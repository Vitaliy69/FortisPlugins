/*!
    \file channelstats.h
    \author Vitaliy Gribko
    
    Distributed under the GPL v2
    see the file LICENSE for details
    or visit http://www.gnu.org/copyleft/gpl.html
*/

#ifndef _CHANNELSTATS_H_
#define _CHANNELSTATS_H_

#include "plugin.h"

#include "configer.h"
#include "logger.h"
#include "global.h"

#include <string>
#include <sstream>
#include <cstdio>
#include <ctime>
#include <algorithm>

#include <iconv.h>
#include <resolv.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>

#include <curl/curl.h>

#include <openssl/sha.h>

#include <net/if.h>

#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/reboot.h>

/// StatsData
/*!
    \struct StatsData
    \brief Structure with TV watching statistics
*/

struct StatsData
{
    bool            noSignal;
    bool            isLive;
    unsigned short  langId;
    unsigned short  networkId;
    const char*     networkName;
    unsigned short  orgNetId;
    unsigned short  tsId;
    unsigned short  svcId;
    const char*     serviceName;
    const char*     epgText;
};

/// ChannelStats
/*!
    \class ChannelStats
    \brief Class for collecting the TV watching statistics
*/

class ChannelStats
{
public:
    ChannelStats(std::string binaryName);

    virtual ~ChannelStats();

    /// Get a poll period time
    /*!
        \return Time in the seconds
    */
    int getPollPeriod();

    /// Get a poll period update time
    /*!
        \return Time in seconds
    */
    int getUpdatePeriod();

    /// Perform a HTTP request with the statistics
    /*!
        \param data StatsData structure
    */
    void performRequest(StatsData data);

    /// Perform a configuration file update
    void performUpdate();

private:
    Logger      *m_logger;

    std::string m_mac;

    int         m_pollPeriod;
    std::string m_server;
    int         m_port;
    bool        m_appendCname;
    bool        m_appendEpg;
    bool        m_appendTimeStamp;

    int         m_updatePeriod;
    int         m_configVersion;

    bool        m_rebootOnUpdate;

    std::string m_binaryName;

    bool        m_updateInProgress;

    ChannelStats(const ChannelStats&);

    void readConfig();

    void getMacAddress();

    const char* getEncoding(int langId);

    std::string convertToUrl(char *input, const char* encoding);

    void httpRequest(std::string request);

    CURLcode downloadFile(std::string link, std::string output);

    std::string calculateSha256(std::string file);

    void showOsdMessage(std::string message);

    template<typename T>
    std::string toString(const T& value);
};

#endif
