/*!
    \file logger.h
    \author Vitaliy Gribko
    
    Distributed under the GPL v2
    see the file LICENSE for details
    or visit http://www.gnu.org/copyleft/gpl.html
*/

#ifndef _LOGGER_H_
#define _LOGGER_H_

#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>

#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/socket.h>
#include <arpa/inet.h>

/// Logger
/*!
    \class Logger
    \brief Class for logging in textfile and syslog
*/

class Logger 
{

public:
    /// Constructor
    /*!
        \param std::string _dirName Path to text log file
        \param std::string _fileName Log file name
        \param bool _isActiveTextLog Create text log
        \param int _port Port for syslog
        \param std::string _ip IP address for syslog
        \param std::string _moduleName Module name
        \param bool _noModulemessages Not show message for module load and unload
    */
    Logger(std::string _dirName, std::string _fileName, bool _isActiveTextLog = true, int _port = 0, std::string _ip = "", std::string _moduleName = "");

    virtual ~Logger();

    /// Write string to log
    /*!
        \param std::string _str Message
    */
    void writeToLog(std::string _str); 

    /// Write integer to log
    /*!
        \param std::string _val Message
    */
    void writeToLog(int _val);

    /// Write string and integer to log
    /*!
        \param std::string _val Message
    */
    void writeToLog(std::string _str, int _val);

private:
    std::ofstream m_fl;
    std::string m_name;

    bool m_noModuleMessages;
    bool m_activeSyslog;

    struct sockaddr_in si_other;
    int s, slen;

    const std::string getTime();

    void sendNetMessage(std::string _message);
};

#endif
