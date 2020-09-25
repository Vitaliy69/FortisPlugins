/*!
    \file plugin.cpp
    \author Vitaliy Gribko
    
    Distributed under the GPL v2
    see the file LICENSE for details
    or visit http://www.gnu.org/copyleft/gpl.html
*/

#include "plugin.h"
#include "logger.h"
#include "configer.h"
#include "global.h"

#include <sstream>

#include <unistd.h>
#include <pthread.h>

#include <sys/stat.h>

#define BUTTON  0x4b

Configer        *configer = NULL;
Logger          *logger = NULL;

std::string     file;
std::ifstream   fl;

pthread_t       wthread;

int             button;
bool            showBox;

word            vpid;
long            modTime;

bool            isActiveTextLog;
int             logPort;
std::string     logServer;

unsigned int    posX, posY, w, h, posX_shift, posY_shift, sat_ft, ecm_ft, str_space, sat_num, sig_wid, ecmStr;

void getEcmStr()
{
    ecmStr = 0;

    if (!access("/tmp/ecm.info", 0))
        file = "/tmp/ecm.info";

    if (!access("/tmp/.ecm.info", 0))
        file = "/tmp/.ecm.info";

    if (!access(file.c_str(), 0))
    {
        try
        {
            fl.open(file.c_str(), std::ios::binary);

            if (fl.is_open())
            {
                fl.clear();
                fl.seekg(0);

                while(!fl.eof())
                {
                    char* buf;
                    buf = GetStrBuf();
                    fl.getline(buf, 255);
                    ecmStr++;
                }

                fl.close();
            }
        }
        catch (std::ifstream::failure) {}
    }

    if (ecmStr > 13)
        ecmStr = 13;

    h = (sat_ft + str_space) * sat_num + (ecm_ft + str_space) * ecmStr + str_space; //< Width for Y axis
}

void InitVars()
{
    isActiveTextLog = true;
    logPort = 0;
    logServer = "";

    if (!access("/var/etc/OSDECMInfo.cfg", 0))
    {
        configer = new Configer("/var/etc/OSDECMInfo.cfg");

        isActiveTextLog = configer->getBoolValue("logging", "enableTextlog");
        logPort = configer->getIntValue("logging", "syslogport");
        logServer = configer->getStrValue("logging", "sysloghost");
    }

    logger = new Logger("/tmp/", "OSDECMInfo.log", isActiveTextLog, logPort, logServer, "OSDECMInfo, v0.16");

    if (access("/var/etc/OSDECMInfo.cfg", 0))
    {
        logger->writeToLog("The configuration file was not found. Use the default settings.");
        button = BUTTON;
    }
    else
    {
        logger->writeToLog("Reading the configuration file...");

        std::string hot = configer->getStrValue("button", "key");

        if (hot.length() != 4)
            button = BUTTON;
        else
            sscanf(hot.c_str(), "%x", &button);

        logger->writeToLog("Hot button key:", button);
    }
    
    file  = "";

    showBox = false;

    vpid = 0;
    modTime = 0;
    ecmStr = 0;

    if (GetOsdWidth() == 1280) //< STi H205
    {
        posX = 100; //< Left space by X asis
        posY = 40; //< Left space by Y asis
        w = 1080; //< Width for X axis

        posX_shift = 20; //< Offset by X axis
        posY_shift = 10; //< Offset by Y axis

        sat_ft = 32; //< Size font for thread info
        ecm_ft = 24; //< Size font for ECM info

        str_space = 8; //< Vertical offset

        sat_num = 6; //< Stings number with thread info

        sig_wid = 120; //< Width for signal quality
    }
    else
    {
        posX = 56;
        posY = 46;
        w = 606;

        posX_shift = 11;
        posY_shift = 8;

        sat_ft = 24;
        ecm_ft = 18;

        str_space = 6;

        sat_num = 6;

        sig_wid = 68;
    }

    getEcmStr();
}

void clearVars()
{
    vpid = 0;
    modTime = 0;
}

void* showInfo(void*)
{
    while (true)
    {
        struct stat stm;
        int shift, tuner, isLock, strength, quality, st, qt;
        std::string strINFO;
        char *charINFO;

        charINFO = GetStrBuf();

        tuner = GetCurSvcTuner(SERVICE_Main);
        GetTunerState(tuner, &isLock, &strength, &quality);
        qt = (quality * 1000) / 256;
        st = (strength * 1000) / 256;

        S_Service *curSer = GetCurService(SERVICE_Main);
        if (!curSer)
        {
            sleep(1);
            continue;
        }

        word servidPid = curSer->videoPid;
        word seraudPid = curSer->audioPid;
        word serpcrPid = curSer->pcrPid;
        word serpmtPid = curSer->pmtPid;  

        S_Tp *tpIdx = GetTp(SERVICE_Main);
        if (!tpIdx)
        {
            sleep(1);
            continue;
        }

        word trafreq = tpIdx->freq;
        word trasr = tpIdx->sr;
        word trapolar = tpIdx->polar;
        word trasatIdx = tpIdx->satIdx;

        if(vpid != servidPid)
        {
            int stb, qtb;

            vpid = servidPid;

            shift = 0;

            SetFontSize(sat_ft);

            DrawRect(posX - 2, posY - 1, w + 4, h + 2, 0xFFFFFFFF);
            FillBox(posX, posY, w, h, 0x41000000);
            sprintf((char*)strINFO.c_str(), "%s / %dE", (char *)GetSatName(trasatIdx), GetSatAngle(trasatIdx)/10);
            charINFO = (char *)strINFO.c_str();
            DrawText(posX + posX_shift, posY + shift + posY_shift, w - posX_shift * 2, sat_ft, charINFO, 0xFFDAA520, 0x41000000, ALIGN_Center, VALIGN_Center, 0);

            shift += (sat_ft + str_space);

            sprintf((char*)strINFO.c_str(), "%d MHz / %c / %d Ksps", trafreq, ((trapolar == 0/*POL_VERTICAL*/) ? 'V' : 'H'), trasr);
            charINFO = (char *)strINFO.c_str(); 
            DrawText(posX + posX_shift, posY + shift + posY_shift, w - posX_shift * 2, sat_ft, charINFO, 0xFFDAA520, 0x41000000, ALIGN_Center, VALIGN_Center, 0);

            shift += (sat_ft + str_space);

            sprintf((char*)strINFO.c_str(), "VPID %d / APID %d / P-PID %d / PMT %d", servidPid, seraudPid, serpcrPid, serpmtPid);
            charINFO = (char *)strINFO.c_str();
            DrawText(posX + posX_shift, posY + shift + posY_shift, w - posX_shift * 2, sat_ft, charINFO, 0xFFDAA520, 0x41000000, ALIGN_Center, VALIGN_Center, 0);

            shift += (sat_ft + str_space);

            sprintf((char*)strINFO.c_str(), "%s / %s", GetNetworkName(tpIdx, true), (char *)GetSvcName(curSer, false));
            charINFO = (char *)strINFO.c_str();
            DrawText(posX + posX_shift, posY + shift + posY_shift, w - posX_shift * 2, sat_ft, charINFO, 0xFFFA8072, 0x41000000, ALIGN_Center, VALIGN_Center, 0);

            shift += (sat_ft + str_space);

            sprintf((char*)strINFO.c_str(), "LE %d%c", (st/10), '%');
            charINFO = (char *)strINFO.c_str();
            DrawText(posX + posX_shift, posY + shift + posY_shift, sig_wid, sat_ft, charINFO, 0xFFFA8072, 0x41000000, ALIGN_Left, VALIGN_Center, 0);

            stb = (w - (posX_shift * 3 + sig_wid)) * (st/10) / 100;

            FillBox(posX + sig_wid + 2 * posX_shift, posY + shift + posY_shift, stb, sat_ft, 0xFF00FF00);
            
            shift += (sat_ft + str_space);

            sprintf((char*)strINFO.c_str(), "QT %d%c", (qt/10), '%');
            charINFO = (char *)strINFO.c_str();
            DrawText(posX + posX_shift, posY + shift + posY_shift, sig_wid, sat_ft, charINFO, 0xFFFA8072, 0x41000000, ALIGN_Left, VALIGN_Center, 0);

            qtb = (w - (posX_shift * 3 + sig_wid)) * (qt/10) / 100;

            FillBox(posX + sig_wid + 2 * posX_shift, posY + shift + posY_shift, qtb, sat_ft, 0xFFD2691E);
        }

        if (!access(file.c_str(), 0))
        {
            stat(file.c_str(), &stm);
            long tm = (long)stm.st_mtim.tv_sec;

            if(modTime != tm)
            {
                modTime = tm;

                try
                {
                    fl.open(file.c_str(), std::ios::binary);

                    if (fl.is_open())
                    {
                        fl.clear();
                        fl.seekg(0);

                        shift = (sat_ft + str_space) * sat_num;

                        SetFontSize(ecm_ft);

                        while(!fl.eof())
                        {
                            char* buf;

                            buf = GetStrBuf();

                            fl.getline(buf, 255);

                            DrawText(posX + posX_shift, posY + shift + posY_shift, w - posX_shift * 2, ecm_ft, buf, 0xFFFFFFFF, 0x41000000, ALIGN_Center, VALIGN_Center, 0);

                            shift += (ecm_ft + str_space);
                        }

                        fl.close();
                    }
                }
                catch (std::ifstream::failure) {}
            }
        }

        sleep(1);
    }

    pthread_exit(0);
}

void hideInfo()
{
    showBox = false;

    pthread_cancel(wthread);

    FillBox(0, 0, GetOsdWidth(), GetOsdHeight(), 0);
}

void showWindow()
{
    SendCommand(PCMD_Key, REM_EXIT);
    SendCommand(PCMD_Key, REM_EXIT);
    SendCommand(PCMD_Key, REM_EXIT);

    sleep(1);

    getEcmStr();
    clearVars();

    showBox = true;

    pthread_create(&wthread, NULL, showInfo, NULL);
    pthread_detach(wthread);
}

int PluginMain(int UNUSED(argc), char *UNUSED(argv[]))
{
    dword   evt[4];
    int     key = 0;
    bool    isNormal = true;

    WiFront_Init();
    FontInit((char *) "/var/etc/swissc.ttf");

    InitVars();

    SendCommand(PCMD_BlockKey, button, Rem_Monitoring);

    while(true)
    {
        int ret = WiQueue_Receive(WiMainQ, (u_int8 *)evt, 16, -1);

        if (ret == 0)
        {
            if(evt[0] == PLUGINMSG_StateChanged) 
            {
                if(evt[1] == STATE_Normal)
                    isNormal = true;
                else
                    isNormal = false;
            }

            if((evt[0] & MSG_BASE_MASK) == MSG_BASE_REMOCON)
            {
                key = evt[1] & MSG_MASK_KEY_CODE;

                if(evt[1] == STATE_Standby) 
                {
                    if (showBox)
                        hideInfo();
                }

                if (key == button && isNormal)
                {
                    if (showBox)
                        hideInfo();
                    else
                        showWindow();
                }

                if (key != button && showBox)
                    hideInfo();
            }
        }
    }

    return 0;
}

void PluginClose()
{
    SendCommand(PCMD_ReleaseKey, (dword)-1);

    WiFront_Term();
    FontClose();

    if (configer) delete configer;
    if (logger) delete logger;
}
