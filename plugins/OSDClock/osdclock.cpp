/*!
    \file osdclock.cpp
    \author Vitaliy Gribko
    
    Distributed under the GPL v2
    see the file LICENSE for details
    or visit http://www.gnu.org/copyleft/gpl.html
*/

#include "osdclock.h"

OSDClock::OSDClock() :
    m_logger(NULL), m_config(NULL)
{
    bool isActiveTextLog = true;
    int logPort = 0;
    std::string logServer = "";

    if (!access("/var/etc/OSDClock.cfg", 0))
    {
        m_config = new Configer("/var/etc/OSDClock.cfg");

        isActiveTextLog = m_config->getBoolValue("logging", "enableTextlog");
        logPort = m_config->getIntValue("logging", "syslogport");
        logServer = m_config->getStrValue("logging", "sysloghost");
    }

    m_logger = new Logger("/tmp/", "OSDClock.log", isActiveTextLog, logPort, logServer, "OSDClock, v0.16");

    readConfig();
}

OSDClock::~OSDClock()
{
    if (m_config) delete m_config;
    if (m_logger) delete m_logger;
}

void OSDClock::readConfig()
{
    if (access("/var/etc/OSDClock.cfg", 0))
    {
        m_logger->writeToLog("The configuration file was not found. Use the default settings.");
        m_posX = POSX;
        m_posY = POSY;
        m_width = WIDHT;
        m_height = HEIGHT;
        m_family = FAMILY;
        m_size = SIZE;
        m_font = FONT;
        m_back = BACK;
        m_time = TIMED;
        m_interval = INTERVAL;
    }
    else
    {
        m_logger->writeToLog("Reading the configuration file...");

        m_posX = m_config->getIntValue("position", "posX");
        if ((m_posX < 0) || (m_posX > GetOsdWidth()))
            m_posX = POSX;

        m_logger->writeToLog("posX =", m_posX);

        m_posY = m_config->getIntValue("position", "posY");
        if ((m_posY < 0) || (m_posY > GetOsdHeight()))
            m_posY = POSY;

        m_logger->writeToLog("posY =", m_posY);
        
        m_width = m_config->getIntValue("position", "width");
        if ((m_width < 50) || (m_width > 200))
            m_width = WIDHT;

        m_logger->writeToLog("width =", m_width);

        m_height = m_config->getIntValue("position", "height");
        if ((m_height < 10) || (m_height > 100))
            m_height = HEIGHT;

        m_logger->writeToLog("height =", m_height);

        m_family = m_config->getStrValue("font", "family");

        if (m_family.length() < 1)
            m_family = FAMILY;
        
        m_logger->writeToLog("Font path: " + m_family);

        m_size = m_config->getIntValue("font", "size");
        if ((m_size < 6) || (m_size > 36))
            m_size = SIZE;

        m_logger->writeToLog("Font size:", m_size);

        std::string fontStr = m_config->getStrValue("color", "font");
        if (!m_config->validateColor(fontStr))
        {
            m_font = FONT;
            m_logger->writeToLog("Font color: = " + std::string("0x1100FF00"));
        }
        else
        {
            m_font = strtol(fontStr.c_str(), 0, 0);
            m_logger->writeToLog("Font color: " + fontStr);
        }

        std::string fontBack = m_config->getStrValue("color", "back");
        if (!m_config->validateColor(fontBack))
        {
            m_back = BACK;
            m_logger->writeToLog("Font background: " + std::string("0x41000000"));
        }
        else
        {
            m_back = strtol(fontBack.c_str(), 0, 0);
            m_logger->writeToLog("Font background: " + fontBack);
        }
        
        m_time = m_config->getStrValue("format", "time");
        
        if (m_time.length() < 1)
            m_time = TIMED;
            
        m_logger->writeToLog("Time format: " + m_time);
        
        std::transform(m_time.begin(), m_time.end(), m_time.begin(), ::tolower);
        
        m_interval = m_config->getIntValue("format", "interval");
        if ((m_interval < 1) || (m_interval > 10))
            m_interval = INTERVAL;

        m_logger->writeToLog("Time refresh interval, s:", m_interval);
    }
}

std::string OSDClock::getFamily()
{
    return m_family;
}

void OSDClock::showTime()
{
    char    *strBuf;
    int     hour, min, sec, mjd;
    dword   tm;

    tm = GetCurTime(&hour, &min, &sec, &mjd);
    tm = Time2Local(tm);
    mjd = (tm >> 16) & 0xFFFF;
    hour = (tm >> 8) & 0xFF;
    min = tm & 0xFF;

    strBuf = GetStrBuf();
    
    if (m_time == "hh:mm")
        sprintf(strBuf, "%02d:%02d", hour, min);

    if (m_time == "hh:mm:ss")
        sprintf(strBuf, "%02d:%02d:%02d", hour, min, sec);

    SetFontSize(m_size);
    DrawText(m_posX, m_posY, m_width, m_height, strBuf, m_font, m_back, ALIGN_Center, VALIGN_Center, 0);
}

void OSDClock::writeToLog(std::string _str)
{
    m_logger->writeToLog(_str);
}

int OSDClock::getX()
{
    return m_posX;
}

int OSDClock::getY()
{
    return m_posY;
}

int OSDClock::getW()
{
    return m_width;
}

int OSDClock::getH()
{
    return m_height;
}

int OSDClock::getInterval()
{
    return m_interval;
}
