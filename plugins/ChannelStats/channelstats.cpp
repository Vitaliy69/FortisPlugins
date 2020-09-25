/*!
    \file channelstats.cpp
    \author Vitaliy Gribko
    
    Distributed under the GPL v2
    see the file LICENSE for details
    or visit http://www.gnu.org/copyleft/gpl.html
*/

#include "channelstats.h"

#define MAX_BUF                     512
#define BINARY_VERSION              1

#define POLL_PERIOD                 10
#define SERVER                      "wp.wilhelm.az"
#define PORT                        80
#define APPEND_CNAME                1
#define APPEND_EPG                  0
#define APPEND_TIMESTAMP            0
#define UPDATE_PERIOD               60
#define CONFIG_VERSION              1
#define REBOOT_ON_UPDATE            1

#define COORX                       0
#define COORY                       20
#define HEIGHT                      30
#define FCOLOR                      0xFFFFFFFF
#define BCOLOR                      0x40000000
#define SIZE                        24

ChannelStats::ChannelStats(std::string binaryName) :
    m_logger(NULL), m_updateInProgress(false)
{
    bool isActiveTextLog = true;
    int logPort = 0;
    std::string logServer = "";

    if (!access("/var/etc/ChannelStats.cfg", 0))
    {
        Configer config("/var/etc/ChannelStats.cfg");

        isActiveTextLog = config.getBoolValue("logging", "enableTextlog");
        logPort = config.getIntValue("logging", "syslogport");
        logServer = config.getStrValue("logging", "sysloghost");
    }

    m_logger = new Logger("/tmp/", "ChannelStats.log", isActiveTextLog, logPort, logServer, "ChannelStats, v" + toString(BINARY_VERSION));

    readConfig();
    getMacAddress();

    m_binaryName = "/var/bin/" + binaryName.erase(0, binaryName.find_last_of("/") + 1);
}

ChannelStats::~ChannelStats()
{
    if (m_logger) delete m_logger;
}

int ChannelStats::getPollPeriod()
{
    return m_pollPeriod;
}

int ChannelStats::getUpdatePeriod()
{
    return m_updatePeriod;
}

void ChannelStats::performRequest(StatsData data)
{
    std::string request = "http://";
    request.append(m_server);

    if (data.noSignal)
    {
        request.append("/nosignal.asp?stb=");
        request.append(m_mac);

        if (m_appendTimeStamp)
        {
            request.append("&time=");
            request.append(toString(std::time(0)));
        }
    }
    else
    {
        if (data.isLive)
        {
            request.append("/stat.asp?nid=");
            request.append(toString(data.networkId));

            std::string networkName = convertToUrl((char*) data.networkName, getEncoding(data.langId));
            if (!networkName.empty())
            {
                request.append("&nname=");
                request.append(networkName);
            }

            request.append("&onid=");
            request.append(toString(data.orgNetId));

            request.append("&tsid=");
            request.append(toString(data.tsId));

            request.append("&sid=");
            request.append(toString(data.svcId));

            std::string channelName = convertToUrl((char*) data.serviceName, getEncoding(data.langId));
            if (m_appendCname && !channelName.empty())
            {
                request.append("&cname=");
                request.append(channelName);
            }

            if (m_appendEpg && data.epgText && (strlen(data.epgText) != 0))
            {
                std::string event = convertToUrl((char*) data.epgText, getEncoding(data.langId));
                if (!event.empty())
                {
                    request.append("&event=");
                    request.append(event);
                }
            }

            request.append("&stb=");
            request.append(m_mac);

            if (m_appendTimeStamp)
            {
                request.append("&time=");
                request.append(toString(std::time(0)));
            }
        }
        else
        {
            request.append("/nolive.asp?stb=");
            request.append(m_mac);

            if (m_appendTimeStamp)
            {
                request.append("&time=");
                request.append(toString(std::time(0)));
            }
        }
    }

    httpRequest(request);
}

void ChannelStats::performUpdate()
{
    if (m_updateInProgress)
        return;
    else
        m_updateInProgress = true;

    std::string link = "http://" + m_server + "/updates/Update.cfg";
    std::string output = "/tmp/Update.cfg";

    if (downloadFile(link, output) == CURLE_OK)
    {
        if (!access("/tmp/Update.cfg", 0))
        {
            Configer config("/tmp/Update.cfg");

            int configVersion = config.getIntValue("update", "configVersion");

            int binaryVersion = config.getIntValue("update", "binaryVersion");
            std::string binarySha256 = config.getStrValue("update", "binarySha256");

            std::transform(binarySha256.begin(), binarySha256.end(), binarySha256.begin(), ::tolower);

            if (m_configVersion < configVersion)
            {
                link = "http://" + m_server + "/updates/ChannelStats.cfg";
                output = "/var/etc/ChannelStats.cfg.tmp";

                if (downloadFile(link, output) == CURLE_OK)
                {
                    unlink("/var/etc/ChannelStats.cfg");

                    int res = rename(output.c_str(), "/var/etc/ChannelStats.cfg");

                    if (!res)
                    {
                        m_logger->writeToLog("Configuration file was successfully updated...");
                        readConfig(); //< Attention! The new configuration will used below for update the executable file.

                        showOsdMessage("Configuration file for the ChannelStats plugin was successfully updated...");
                    }
                }
            }

            sleep(1);

            if (abs(BINARY_VERSION) < binaryVersion)
            {
                link = "http://" + m_server + "/updates/ChannelStats";
                output = m_binaryName + ".tmp";

                unlink(output.c_str());

                if (downloadFile(link, output) == CURLE_OK)
                {
                    if (calculateSha256(output) == binarySha256)
                    {
                        unlink(m_binaryName.c_str());

                        int res = rename(output.c_str(), std::string(m_binaryName).c_str());
                        res += chmod(m_binaryName.c_str(), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);

                        if (!res)
                        {
                            m_logger->writeToLog("The binary file was successfully updated...");

                            if (m_rebootOnUpdate)
                            {
                                showOsdMessage("Plugin ChannelStats was updated. Please, wait while the STB will reboot...");

                                sync();
                                sleep(3);

                                reboot(RB_AUTOBOOT);
                            }
                            else
                            {
                                showOsdMessage("Plugin ChannelStats was updated. Please, reboot the STB...");
                            }
                        }
                    }
                    else
                    {
                        unlink(output.c_str());
                    }
                }
            }

            unlink("/tmp/Update.cfg");
        }
    }

    m_updateInProgress = false;
}

void ChannelStats::readConfig()
{
    if (access("/var/etc/ChannelStats.cfg", 0))
    {
        m_logger->writeToLog("The configuration was file not found. Use the default settings.");

        m_pollPeriod = POLL_PERIOD;
        m_server = SERVER;
        m_port = PORT;
        m_appendCname = APPEND_CNAME;
        m_appendEpg = APPEND_EPG;
        m_appendTimeStamp = APPEND_TIMESTAMP;

        m_updatePeriod = UPDATE_PERIOD;
        m_pollPeriod = POLL_PERIOD;

        m_rebootOnUpdate = REBOOT_ON_UPDATE;
    }
    else
    {
        m_logger->writeToLog("Reading the configuration file...");

        Configer config("/var/etc/ChannelStats.cfg");

        m_pollPeriod = config.getIntValue("settings", "pollPeriod");
        if ((m_pollPeriod < 1) || (m_pollPeriod > 3600))
            m_pollPeriod = POLL_PERIOD;

        m_logger->writeToLog("Poll period:", m_pollPeriod);

        m_server = config.getStrValue("settings", "server");
        if (m_server.empty())
            m_server = SERVER;

        m_logger->writeToLog("Server: " + m_server);

        m_port = config.getIntValue("settings", "port");
        if ((m_port < 1) || (m_port > 65535))
            m_port = PORT;

        m_logger->writeToLog("Port:", m_port);

        m_appendCname = config.getBoolValue("settings", "appendCname");

        if (m_appendCname)
            m_logger->writeToLog("Channel name will append.");
        else
            m_logger->writeToLog("Channel name will not append.");

        m_appendEpg = config.getBoolValue("settings", "appendEpg");

        if (m_appendEpg)
            m_logger->writeToLog("EPG data will append.");
        else
            m_logger->writeToLog("EPG data will not append.");

        m_appendTimeStamp = config.getBoolValue("settings", "appendTimeStamp");

        if (m_appendTimeStamp)
            m_logger->writeToLog("Time stamp will append.");
        else
            m_logger->writeToLog("Time stamp will not append.");

        m_updatePeriod = config.getIntValue("update", "updatePeriod");
        if ((m_updatePeriod < 60) || (m_pollPeriod > 3600 * 24))
            m_updatePeriod = UPDATE_PERIOD;

        m_logger->writeToLog("Update period:", m_updatePeriod);

        m_configVersion = config.getIntValue("update", "configVersion");
        if ((m_configVersion < 0) || (m_configVersion > INT_MAX))
            m_configVersion = CONFIG_VERSION;

        m_logger->writeToLog("Configuration version:", m_configVersion);

        m_rebootOnUpdate = config.getBoolValue("update", "rebootOnUpdate");

        if (m_rebootOnUpdate)
            m_logger->writeToLog("STB will reboot on the binary update.");
        else
            m_logger->writeToLog("STB will not reboot on the binary update.");
    }
}

void ChannelStats::getMacAddress()
{
    char hwAddress[12];
    memset(&hwAddress, 0, sizeof hwAddress);

    struct ifreq netdev;
    int fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (fd != -1)
    {
        strcpy(netdev.ifr_name, "eth0");
        if (0 == ioctl(fd, SIOCGIFHWADDR, &netdev))
        {
            for (int i = 0; i < 6; ++i)
            {
                sprintf(hwAddress + i * 2, "%02x", (unsigned char) netdev.ifr_ifru.ifru_hwaddr.sa_data[i]);
            }
        }

        close(fd);
    }

    m_mac = std::string(hwAddress);

    m_logger->writeToLog("STB ethernet MAC address: " + m_mac);
}

const char* ChannelStats::getEncoding(int langId)
{
    const char* result = "ISO_8859-1";

    switch (langId)
    {
        case LANG_FRENCH:
        case LANG_GERMAN:
        case LANG_DUTCH:
        case LANG_SPANISH:
        case LANG_ITALIAN:
        case LANG_PORTUGUESE:
        case LANG_TURK:
        case LANG_SWEDISH:
        case LANG_FINNISH:
        case LANG_DANISH:
        case LANG_NORWEGIAN:
        case LANG_KURDISH:
            result = "ISO_8859-9";
            break;
        case LANG_GREEK:
            result = "ISO_8859-7";
            break;
        case LANG_ARABIC:
        case LANG_PERSIAN:
            result = "ISO_8859-6";
            break;
        case LANG_BULGARIAN:
        case LANG_RUSSIAN:
        case LANG_UKRAINIAN:
        case LANG_MACEDONIAN:
            result = "ISO_8859-5";
            break;
        case LANG_POLISH:
        case LANG_CZECH:
        case LANG_SLOVAK:
        case LANG_SLOVENIAN:
        case LANG_ROMANIAN:
        case LANG_SERBIAN:
        case LANG_HUNGARIAN:
            result = "ISO_8859-2";
            break;
        case LANG_LITHUANIAN:
        case LANG_LATVIAN:
            result = "ISO_8859-13";
            break;
        case LANG_THAI:
            result = "ISO_8859-11";
            break;
        case LANG_HEBREW:
            result = "ISO_8859-8";
            break;
        default:
            break;
    }

    return result;
}

std::string ChannelStats::convertToUrl(char *input, const char *encoding)
{
    iconv_t cd = iconv_open("UTF-8", encoding);

    char *in_buf = &input[0];
    size_t in_left = strlen(input);

    char output[MAX_BUF];
    char *out_buf = &output[0];
    size_t out_left = sizeof(output) - 1;

    do
    {
        if (iconv(cd, &in_buf, &in_left, &out_buf, &out_left) == (size_t) -1)
        {
            m_logger->writeToLog("Iconv conversion failed!");
            iconv_close(cd);
            return "";
        }
    } while (in_left > 0 && out_left > 0);
    *out_buf = 0;

    std::string result = std::string(output);
    result.erase(std::remove(result.begin(), result.end(), '\01'), result.end());

    iconv_close(cd);

    CURL *curl = curl_easy_init();
    if(curl)
    {
        char *url = curl_easy_escape(curl, result.c_str(), result.length());
        if(url)
        {
            result = url;
            curl_free(url);
        }
        else
        {
            result = "";
            m_logger->writeToLog("URL conversion failed!");
        }

        curl_easy_cleanup(curl);
    }

    return result;
}

void ChannelStats::httpRequest(std::string request)
{
    CURL *curl = curl_easy_init();
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_DNS_CACHE_TIMEOUT, 0);
        curl_easy_setopt(curl, CURLOPT_URL, request.c_str());
        curl_easy_setopt(curl, CURLOPT_PORT, m_port);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Openbox/1.0");
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK)
        {
            res_init();
            m_logger->writeToLog("URL request failed: " + std::string(curl_easy_strerror(res)));
        }

        curl_easy_cleanup(curl);
    }
}

CURLcode ChannelStats::downloadFile(std::string link, std::string output)
{
    CURLcode res = CURLE_FAILED_INIT;

    CURL *curl = curl_easy_init();
    if (curl)
    {
        FILE *fp = fopen(output.c_str(), "wb");

        if (fp)
        {
            curl_easy_setopt(curl, CURLOPT_DNS_CACHE_TIMEOUT, 0);
            curl_easy_setopt(curl, CURLOPT_URL, link.c_str());
            curl_easy_setopt(curl, CURLOPT_PORT, 80);
            curl_easy_setopt(curl, CURLOPT_USERAGENT, "Openbox/1.0");
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

            res = curl_easy_perform(curl);
            if (res != CURLE_OK)
            {
                res_init();
                m_logger->writeToLog("URL download error: " + std::string(curl_easy_strerror(res)));
            }

            fclose(fp);
        }

        curl_easy_cleanup(curl);
    }

    return res;
}

std::string ChannelStats::calculateSha256(std::string file)
{
    int bytes = 0;

    unsigned char data[MAX_BUF];
    unsigned char sha256sum[SHA256_DIGEST_LENGTH];

    char sha256[SHA256_DIGEST_LENGTH * 2];

    memset(&data, 0, sizeof data);
    memset(&sha256sum, 0, sizeof sha256sum);

    memset(&sha256, 0, sizeof sha256);

    FILE *inFile = fopen(file.c_str(), "rb");
    if (inFile)
    {
        SHA256_CTX shaContext;
        SHA256_Init(&shaContext);

        while ((bytes = fread(data, 1, MAX_BUF, inFile)) != 0)
        {
            SHA256_Update(&shaContext, data, bytes);
        }

        SHA256_Final(sha256sum, &shaContext);

        for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
        {
            sprintf(sha256 + i * 2, "%02x", (unsigned char) sha256sum[i]);
        }

        fclose(inFile);
    }

    return std::string(sha256);
}

void ChannelStats::showOsdMessage(std::string message)
{
    SendCommand(PCMD_BlockKey, REM_UP, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_DOWN, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_RIGHT, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_LEFT, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_MENU, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_OK, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_FAVORITE, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_SAT, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_MUTE, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_GUIDE, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_EXIT, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_INFO, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_TVRADIO, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_PLAYLIST, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_AUDIOTRK, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_SUBT, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_0, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_2, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_3, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_4, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_5, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_6, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_7, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_8, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_9, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_F1, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_F2, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_F3, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_F4, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_SLEEP, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_POINTMARK, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_RECALL, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_PGUP, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_PGDOWN, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_PAUSE, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_PREV, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_NEXT, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_FastBackward, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_FastForward, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_Record, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_PLAY, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_STOP, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_RESOLUTION, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_VFORMAT, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_VOLUP, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_VOLDOWN, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_CHUP, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_CHDOWN, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_PIP, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_PIPLIST, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_SWAP, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_POINTMARK, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_FRONT_UP, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_FRONT_DOWN, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_FRONT_RIGHT, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_FRONT_LEFT, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_FRONT_OK, Rem_Block);

    byte *m_imgEcm = OsdCompress(COORX, COORY, GetOsdWidth(), HEIGHT);

    SetFontSize(SIZE);
    DrawText(COORX, COORY, GetOsdWidth(), HEIGHT, (char *) message.c_str(), FCOLOR, BCOLOR, ALIGN_Center, VALIGN_Center, 0);

    sleep(5);

    OsdDecompress(m_imgEcm, COORX, COORY, GetOsdWidth(), HEIGHT);
    free(m_imgEcm);

    SendCommand(PCMD_ReleaseKey, (dword) -1);
}

template<typename T>
std::string ChannelStats::toString(const T& value)
{
    std::ostringstream oss;
    oss << value;
    return oss.str();
}
