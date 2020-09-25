/*!
    \file osdshell.h
    \author Vitaliy Gribko
    
    Distributed under the GPL v2
    see the file LICENSE for details
    or visit http://www.gnu.org/copyleft/gpl.html
*/

#ifndef _OSDSHELL_H_
#define _OSDSHELL_H_

#include <iostream>
#include <fstream>
#include <string>

#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

#include "configer.h"
#include "logger.h"
#include "global.h"
#include "plugin.h"
#include "font.h"

using namespace std;

/// OSDShell
/*!
    \class OSDShell
    \brief Class for execute user script activated by color buttons
*/

#define MAX_BUF             512
#define POSX                100
#define POSXOLD             80
#define POSY                40
#define POSYOLD             46
#define HEIGHT              640
#define HEIGHTOLD           500
#define WIDTH               1080
#define WIDTHOLD            560
#define TIMED               5
#define FAMILY              "/var/etc/verdana.ttf"
#define SIZE                24
#define SIZEOLD             18
#define INDENT              12
#define INDENTOLD           10
#define FONT                0xFFFFFFFF
#define BACK                0x40000000
#define ACTIVE              0
#define BLOCK               0
#define SCRIPT              1
#define CONSOLE             0
#define PATH                ""


enum ButCol
{
    Red, Green, Yellow, Blue
};

struct ThrPar
{
    string pathFile;
    bool isShell;
    bool showConsole;
    int x;
    int y;
    int w;
    int h;
    dword font;
    dword back;
    int size;
    int x_shift;
    int y_shift;
    int indent;
    ButCol bc;
    int time;
};

class OSDShell 
{

public:
    OSDShell();

    virtual ~OSDShell();

    /// Get current font
    /*!
        \return std::string Full path to font
    */
    string getFamily();

    /// Buttons blocking
    static void keyLock();

    /// Execute command
    /*!
        \param ButCol Pressed button
    */
    void execCommand(ButCol _but);

private:
    Logger      *m_logger;
    Configer    *m_config;

    int         m_cur, m_posX, m_posY, m_height, m_width, m_time, m_size, m_indent, m_posX_shift, m_posY_shift;
    string      m_family, m_rPath, m_gPath, m_yPath, m_bPath;
    dword       m_font, m_back;

    pthread_t   rThread, gThread, yThread, bThread;

    void readConfig();

    string returnPath(ButCol _but);

    //static void blockAllKeys();

    static void clearStream(void* _arg);

    static void clearFile(void* _arg);

    static void finishThread(void* _arg);

    static void *startThread(void* _args);

    OSDShell(const OSDShell&);
};

#endif
