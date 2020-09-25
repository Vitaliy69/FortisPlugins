/*!
    \file osdclock.h
    \author Vitaliy Gribko
    
    Distributed under the GPL v2
    see the file LICENSE for details
    or visit http://www.gnu.org/copyleft/gpl.html
*/

#ifndef _OSDCLOCK_H_
#define _OSDCLOCK_H_

#include <algorithm>

#include <unistd.h>

#include "configer.h"
#include "logger.h"
#include "global.h"
#include "plugin.h"
#include "font.h"

/// OSDClock
/*!
    \class OSDClock
    \brief Class for time showing
*/

#define POSX            25
#define POSY            14
#define WIDHT           70
#define HEIGHT          30
#define FAMILY          "/var/etc/LCD-BOLD.TTF"
#define SIZE            24
#define FONT            0x8000FF00
#define BACK            0x40000000
#define TIMED           "hh:mm"
#define INTERVAL        2

class OSDClock 
{
    
public:
    OSDClock();

    virtual ~OSDClock();

    /// Reread plugin settings
    void readConfig();

    /// Get current font
    /*!
        \return std::string Full path to font
    */
    std::string getFamily();

    /// Show current time
    void showTime();

    /// Write string to logfile
    /*!
        \param std::string _str Message
    */
    void writeToLog(std::string _str);

    /// Get X cor
    /*!
        \return X cor
    */
    int getX();

    /// Get Y cor
    /*!
        \return Y cor
    */
    int getY();

    /// Get width
    /*!
        \return Width
    */
    int getW();

    /// Get hight
    /*!
        \return Hight
    */
    int getH();

    /// Get time refresh period
    /*!
        \return Refresh period
    */
    int getInterval();

private:
    Logger      *m_logger;
    Configer    *m_config;

    int         m_posX, m_posY, m_width, m_height, m_size, m_interval;
    std::string m_family, m_time;
    dword       m_font, m_back;

    OSDClock(const OSDClock&);
};

#endif
