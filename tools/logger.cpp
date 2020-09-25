/*!
    \file logger.cpp
    \author Vitaliy Gribko
    
    Distributed under the GPL v2
    see the file LICENSE for details
    or visit http://www.gnu.org/copyleft/gpl.html
*/

#include "logger.h"

template<typename T>
std::string to_string(const T& value)
{
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

Logger::Logger(std::string _dirName, std::string _fileName, bool _isActiveTextLog, int _port, std::string _ip, std::string _moduleName) :
    m_name (_dirName + _fileName)
{
    m_activeSyslog = false;

    if (_moduleName.empty())
        m_noModuleMessages = true;
    else
        m_noModuleMessages = false;

    if (_port > 0 && !_ip.empty())
    {
        slen = sizeof(si_other);

        memset((char *) &si_other, 0, sizeof(si_other));
        si_other.sin_family = AF_INET;
        si_other.sin_port = htons(_port);

        if (inet_aton(_ip.c_str(), &si_other.sin_addr) == 0)
            perror("Error address convert!");
        else
        {
            if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
                perror("Error socket create!");
            else
                m_activeSyslog = true;
        }
    }

    if (_isActiveTextLog)
    {
        try
        {
            m_fl.open(m_name.c_str(), std::ios::app);
        }
        catch (std::ofstream::failure)
        {
            std::cerr << "Error open file!\n";
        }
    }

    if (!m_noModuleMessages)
    {
        std::string txt1, txt2;
        txt1 = "\n------------------------------------------------------ \n";
        txt2 = getTime() + ": Plugin \"" + _moduleName + "\" successfully loaded!\n";
        txt2 += getTime() + ": Author is Vitaliy69. Bug reports and suggestions to email vitaliy69@gmail.com.\n";

        if (m_fl.is_open())
        {
            m_fl << txt1;
            m_fl << txt2;

            m_fl.flush();
        }

        if (m_activeSyslog)
        {
            sendNetMessage(txt1);
            sendNetMessage(txt2);
        }
    }
}

Logger::~Logger()
{
    std::string txt = getTime() + ": Plugin unload successfully!\n";

    if (m_fl.is_open())
    {
        if (!m_noModuleMessages)
        {
            m_fl << txt;

            m_fl.flush();
        }

        m_fl.close();
    }

    if (m_activeSyslog)
    {
        if (!m_noModuleMessages)
            sendNetMessage(txt);

        close(s);
    }
}

void Logger::writeToLog(std::string _str)
{
    std::string txt = getTime() + ": " + _str.c_str() + "\n";
    
    if (m_fl.is_open())
    {
        m_fl << txt;

        m_fl.flush();
    }

    if (m_activeSyslog)
        sendNetMessage(txt);
}

void Logger::writeToLog(int _val)
{
    std::string txt = getTime() + ": " + to_string(_val) + "\n";

    if (m_fl.is_open())
    {
        m_fl << txt;

        m_fl.flush();
    }

    if (m_activeSyslog)
        sendNetMessage(txt);
}

void Logger::writeToLog(std::string _str, int _val)
{
    std::string txt = getTime() + ": " + _str + " " + to_string(_val) + "\n";

    if (m_fl.is_open())
    {
        m_fl << txt;

        m_fl.flush();
    }

    if (m_activeSyslog)
        sendNetMessage(txt);
}

const std::string Logger::getTime()
{
    struct tm tstruct;
    char buf[20];

    time_t now = time(0);
    tstruct = *localtime(&now);

    strftime(buf, sizeof(buf), "%d.%m.%Y %X", &tstruct);

    return buf;
}

void Logger::sendNetMessage(std::string _message)
{
    if (sendto(s, _message.c_str(), strlen(_message.c_str()), 0, (struct sockaddr*) &si_other, slen) == -1)
        perror("Send data error!");
}
