/*!
    \file osdshell.cpp
    \author Vitaliy Gribko
    
    Distributed under the GPL v2
    see the file LICENSE for details
    or visit http://www.gnu.org/copyleft/gpl.html
*/

#include "osdshell.h"

static bool rscActive = false;
static bool gscActive = false;
static bool yscActive = false;
static bool bscActive = false;

static bool rActive = false;
static bool rBlock = false;
static bool rScript = false;
static bool rConsole = false;

static bool gActive = false;
static bool gBlock = false;
static bool gScript = false;
static bool gConsole = false;

static bool yActive = false;
static bool yBlock = false;
static bool yScript = false;
static bool yConsole = false;

static bool bActive = false;
static bool bBlock = false;
static bool bScript = false;
static bool bConsole = false;

bool isActiveTextLog = true;
int logPort = 0;
std::string logServer = "";

OSDShell::OSDShell() :
    m_logger(NULL), m_config(NULL)
{
    if (!access("/var/etc/OSDShell.cfg", 0))
    {
        m_config = new Configer("/var/etc/OSDShell.cfg");

        isActiveTextLog = m_config->getBoolValue("logging", "enableTextlog");
        logPort = m_config->getIntValue("logging", "syslogport");
        logServer = m_config->getStrValue("logging", "sysloghost");
    }

    m_logger = new Logger("/tmp/", "OSDShell.log", isActiveTextLog, logPort, logServer, "OSDShell, v0.9");

    m_cur = 0;

    if (GetOsdWidth() == 1280) //< STi H205
    {
        m_posX_shift = 20;
        m_posY_shift = 10;
    }
    else
    {
        m_posX_shift = 12;
        m_posY_shift = 8;
    }

    readConfig();

    keyLock();
}

OSDShell::~OSDShell()
{
    if (m_config) delete m_config;
    if (m_logger) delete m_logger;
}

void OSDShell::readConfig()
{
    if (access("/var/etc/OSDShell.cfg", 0))
    {
        m_logger->writeToLog("The configuration file was not found. Use the default settings.");

        if (GetOsdWidth() == 1280) //< STi H205
        {
            m_posX = POSX;
            m_posY = POSY;
            m_height = HEIGHT;
            m_width = WIDTH;
            m_size = SIZE;
            m_indent = INDENT;
        }
        else
        {
            m_posX = POSXOLD;
            m_posY = POSYOLD;
            m_height = HEIGHTOLD;
            m_width = WIDTHOLD;
            m_size = SIZEOLD;
            m_indent = INDENTOLD;
        }

        m_time = TIMED;
        m_family = FAMILY;
        m_rPath = m_gPath = m_yPath = m_bPath = PATH;
        m_font = FONT;
        m_back = BACK;

        rActive = gActive = yActive = bActive = ACTIVE;
        rBlock = gBlock = yBlock = bBlock = BLOCK;
        rScript = gScript = yScript = bScript = SCRIPT;
        rConsole = gConsole = yConsole = bConsole = CONSOLE;
    }
    else
    {
        m_logger->writeToLog("Reading the configuration file...");
        m_logger->writeToLog("Reading the virtual console parameters..");

        if (GetOsdWidth() == 1280) //< STi H205
        {
            m_posX = m_config->getIntValue("position", "posX");
            if ((m_posX < 0) || (m_posX > GetOsdWidth()))
                m_posX = POSX;

            m_logger->writeToLog("posX =", m_posX);

            m_posY = m_config->getIntValue("position", "posY");
            if ((m_posY < 0) || (m_posY > GetOsdHeight()))
                m_posY = POSY;

            m_logger->writeToLog("posY =", m_posY);

            m_height = m_config->getIntValue("position", "height");
            if ((m_height < 0) || (m_height > GetOsdHeight()))
                m_height = HEIGHT;

            m_logger->writeToLog("height =", m_height);

            m_width = m_config->getIntValue("position", "width");
            if ((m_width < 0) || (m_width > GetOsdWidth()))
                m_width = WIDTH;

            m_logger->writeToLog("width =", m_width);

            m_size = m_config->getIntValue("font", "size");
            if ((m_size < 6) || (m_size > 42))
                m_size = SIZE;

            m_logger->writeToLog("Font size:", m_size);

            m_indent = m_config->getIntValue("font", "indent");
            if ((m_indent < 2) || (m_indent > 24))
                m_indent = INDENT;

            m_logger->writeToLog("Vertical indent:", m_indent);
        }
        else
        {
            m_posX = m_config->getIntValue("position", "posX");
            if ((m_posX < 0) || (m_posX > GetOsdWidth()))
                m_posX = POSXOLD;

            m_logger->writeToLog("posX =", m_posX);

            m_posY = m_config->getIntValue("position", "posY");
            if ((m_posY < 0) || (m_posY > GetOsdHeight()))
                m_posY = POSYOLD;

            m_logger->writeToLog("posY =", m_posY);

            m_height = m_config->getIntValue("position", "height");
            if ((m_height < 0) || (m_height > GetOsdHeight()))
                m_height = HEIGHTOLD;

            m_logger->writeToLog("height =", m_height);

            m_width = m_config->getIntValue("position", "width");
            if ((m_width < 0) || (m_width > GetOsdWidth()))
                m_width = WIDTHOLD;

            m_logger->writeToLog("width =", m_width);

            m_size = m_config->getIntValue("font", "size");
            if ((m_size < 6) || (m_size > 42))
                m_size = SIZEOLD;

            m_logger->writeToLog("Font size:", m_size);

            m_indent = m_config->getIntValue("font", "indent");
            if ((m_indent < 2) || (m_indent > 24))
                m_indent = INDENTOLD;

            m_logger->writeToLog("Vertical indent:", m_indent);
        }

        m_time = m_config->getIntValue("position", "time");
        if ((m_time < 0) || (m_time > 300))
            m_time = TIMED;

        m_logger->writeToLog("Console show time, s:", m_time);

        m_family = m_config->getStrValue("font", "family");

        if (m_family.length() < 1)
            m_family = FAMILY;

        m_logger->writeToLog("Font path: " + m_family);

        string fontStr = m_config->getStrValue("color", "font");
        if (!m_config->validateColor(fontStr))
        {
            m_font = FONT;
            m_logger->writeToLog("Font color: " + string("0xFFFFFFFF"));
        }
        else
        {
            m_font = strtol(fontStr.c_str(), 0, 0);
            m_logger->writeToLog("Font color: " + fontStr);
        }

        string fontBack = m_config->getStrValue("color", "back");
        if (!m_config->validateColor(fontBack))
        {
            m_back = BACK;
            m_logger->writeToLog("Font background: " + string("0x40000000"));
        }
        else
        {
            m_back = strtol(fontBack.c_str(), 0, 0);
            m_logger->writeToLog("Font background: " + fontBack);
        }

        m_logger->writeToLog("Reading the buttons parameters...");
        m_logger->writeToLog("Reading the RED button parameters...");

        rActive = m_config->getBoolValue("red", "isActive");

        if (rActive)
        {
            rBlock = m_config->getBoolValue("red", "blockDefault");
            if (rBlock)
                m_logger->writeToLog("Standard function of the RED button is disable.");
            else
                m_logger->writeToLog("Standard function of the RED button is enable.");

            rScript = m_config->getBoolValue("red", "runAsSript");
            if (rScript)
                m_logger->writeToLog("Linux script will apply when the RED button pressed.");
            else
                m_logger->writeToLog("Linux shell is disable for the RED button. The command will be execute line by the line.");

            rConsole = m_config->getBoolValue("red", "enableConsole");
            
            if(rConsole && rScript)
            {
                rConsole = false;
                m_logger->writeToLog("Screen message box is disable. Switch off the enableConsole option to execute the command line by line for the RED button.");
            }
            else
            {
                if (rConsole)
                    m_logger->writeToLog("The command's result will show on the screen for the RED button.");
                else
                    m_logger->writeToLog("The command's result will not show on the screen for the RED button.");
            }

            m_rPath = m_config->getStrValue("red", "scriptPath");
            if (m_rPath != "")
                m_logger->writeToLog("Script for the RED button: " + m_rPath);
            else
            {
                m_logger->writeToLog("No script given for the RED button. Button is not active.");
                rActive = false;
            }

            if (access(m_rPath.c_str(), 0))
            {
                rActive = false;
                m_logger->writeToLog("Script for the RED button is not exists. Button is not active.");
            }
        }
        else
            m_logger->writeToLog("The RED button is not active. Skipping...");

        m_logger->writeToLog("Reading the GREEN button parameters...");

        gActive = m_config->getBoolValue("green", "isActive");

        if (gActive)
        {
            gBlock = m_config->getBoolValue("green", "blockDefault");
            if (gBlock)
                m_logger->writeToLog("Standard function of the GREEN button is disable.");
            else
                m_logger->writeToLog("Standard function of the GREEN button is enable.");

            gScript = m_config->getBoolValue("green", "runAsSript");
            if (gScript)
                m_logger->writeToLog("Linux script will apply when GREEN button pressed.");
            else
                m_logger->writeToLog("Linux shell is disable for the GREEN button. The command will be execute line by the line.");

            gConsole = m_config->getBoolValue("green", "enableConsole");

            if(gConsole && gScript)
            {
                gConsole = false;
                m_logger->writeToLog("Screen message box is disable. Switch off the enableConsole option to execute the command line by line for the GREEN button.");
            }
            else
            {
                if (gConsole)
                    m_logger->writeToLog("The command's results will show on the screen for the GREEN button.");
                else
                    m_logger->writeToLog("The command's results will not show on the screen for the GREEN button.");
            }

            m_gPath = m_config->getStrValue("green", "scriptPath");
            if (m_gPath != "")
                m_logger->writeToLog("Script for the GREEN button: " + m_gPath);
            else
            {
                m_logger->writeToLog("No script given for the GREEN button. Button is not active.");
                gActive = false;
            }

            if (access(m_gPath.c_str(), 0))
            {
                gActive = false;
                m_logger->writeToLog("The script for GREEN button is not exists. Button is not active.");
            }
        }
        else
            m_logger->writeToLog("The GREEN button is not active. Skipping...");

        m_logger->writeToLog("Reading the YELLOW button parameters...");

        yActive = m_config->getBoolValue("yellow", "isActive");

        if (yActive)
        {
            yBlock = m_config->getBoolValue("yellow", "blockDefault");
            if (yBlock)
                m_logger->writeToLog("Standard function of the YELLOW button is disable.");
            else
                m_logger->writeToLog("Standard function of the YELLOW button is enable.");

            yScript = m_config->getBoolValue("yellow", "runAsSript");
            if (yScript)
                m_logger->writeToLog("Linux script will be apply when the YELLOW button pressed.");
            else
                m_logger->writeToLog("Linux shell is disable for YELLOW button. The command will be execute line by line.");

            yConsole = m_config->getBoolValue("yellow", "enableConsole");
            
            if(yConsole && yScript)
            {
                yConsole = false;
                m_logger->writeToLog("Screen message box is disable. Switch off the enableConsole option to execute the command line by line for the YELLOW button.");
            }
            else
            {
                if (yConsole)
                    m_logger->writeToLog("Command's results will show on the screen for the YELLOW button.");
                else
                    m_logger->writeToLog("Command's results will not show on the screen for the YELLOW button.");
            }

            m_yPath = m_config->getStrValue("yellow", "scriptPath");
            if (m_yPath != "")
                m_logger->writeToLog("Script for the YELLOW button: " + m_yPath);
            else
            {
                m_logger->writeToLog("No script given for the YELLOW button. Button is not active.");
                yActive = false;
            }

            if (access(m_yPath.c_str(), 0))
            {
                yActive = false;
                m_logger->writeToLog("Script for the YELLOW button is not exists. Button is not active.");
            }
        }
        else
            m_logger->writeToLog("The YELLOW button is not active. Skipping...");

        m_logger->writeToLog("Reading the BLUE button parameters...");

        bActive = m_config->getBoolValue("blue", "isActive");

        if (bActive)
        {
            bBlock = m_config->getBoolValue("blue", "blockDefault");
            if (bBlock)
                m_logger->writeToLog("Standard function of the BLUE button is disable.");
            else
                m_logger->writeToLog("Standard function of the BLUE button is enable.");

            bScript = m_config->getBoolValue("blue", "runAsSript");
            if (bScript)
                m_logger->writeToLog("Linux script will apply when the BLUE button pressed.");
            else
                m_logger->writeToLog("Linux shell is disable for BLUE button. Command will be execute line by line.");

            bConsole = m_config->getBoolValue("blue", "enableConsole");

            if(bConsole && bScript)
            {
                bConsole = false;
                m_logger->writeToLog("Screen message box is disable. Switch off the enableConsole option to execute the command line by line for the BLUE button.");
            }
            else
            {
                if (bConsole)
                    m_logger->writeToLog("Command's results will show on the screen for the BLUE button.");
                else
                    m_logger->writeToLog("Command's results will not show on the screen for the BLUE button.");
            }

            m_bPath = m_config->getStrValue("blue", "scriptPath");
            if (m_bPath != "")
                m_logger->writeToLog("Script for the BLUE button: " + m_bPath);
            else
            {
                m_logger->writeToLog("No script given for the BLUE button. Button is not active.");
                bActive = false;
            }

            if (access(m_bPath.c_str(), 0))
            {
                bActive = false;
                m_logger->writeToLog("Script for the BLUE button is not exists. Button is not active.");
            }
        }
        else
            m_logger->writeToLog("The BLUE button is not active. Skipping...");
    }
}

string OSDShell::getFamily()
{
    return m_family;
}

void OSDShell::keyLock()
{
    SendCommand(PCMD_ReleaseKey, (dword)-1);

    if (rActive)
    {
        if (rBlock)
            SendCommand(PCMD_BlockKey, REM_F1, Rem_Block);
        else
            SendCommand(PCMD_BlockKey, REM_F1, Rem_Monitoring);
    }

    if (gActive)
    {
        if (gBlock)
            SendCommand(PCMD_BlockKey, REM_F2, Rem_Block);
        else
            SendCommand(PCMD_BlockKey, REM_F2, Rem_Monitoring);
    }

    if (yActive)
    {
        if (yBlock)
            SendCommand(PCMD_BlockKey, REM_F3, Rem_Block);
        else
            SendCommand(PCMD_BlockKey, REM_F3, Rem_Monitoring);
    }

    if (bActive)
    {
        if (bBlock)
            SendCommand(PCMD_BlockKey, REM_F4, Rem_Block);
        else
            SendCommand(PCMD_BlockKey, REM_F4, Rem_Monitoring);
    }
}

void OSDShell::execCommand(ButCol _but)
{
    if (_but == Red && !rActive)
        return;

    if (_but == Green && !gActive)
        return;

    if (_but == Yellow && !yActive)
        return;

    if (_but == Blue && !bActive)
        return;

    if (_but !=  Red && rThread && rscActive && rConsole && !rScript)
    {
        m_logger->writeToLog("Script " + returnPath(_but) + " is cancel because screen is busy.");
        return;
    }

    if (_but !=  Green && gThread && gscActive && gConsole && !gScript)
    {
        m_logger->writeToLog("Script " + returnPath(_but) + " is cancel because screen is busy.");
        return;
    }

    if (_but !=  Yellow && yThread && yscActive && yConsole && !yScript)
    {
        m_logger->writeToLog("Script " + returnPath(_but) + " is cancel because screen is busy.");
        return;
    }

    if (_but !=  Blue && bThread && bscActive && bConsole && !bScript)
    {
        m_logger->writeToLog("Script " + returnPath(_but) + " is cancel because screen is busy.");
        return;
    }

    ThrPar* params = new ThrPar();
    params->x = m_posX;
    params->y = m_posY;
    params->w = m_width;
    params->h = m_height;
    params->font = m_font;
    params->back = m_back;
    params->size = m_size;
    params->x_shift = m_posX_shift;
    params->y_shift = m_posY_shift;
    params->indent = m_indent;
    params->bc = _but;
    params->time = m_time;

    if (_but == Red)
    {
        params->pathFile = m_rPath;
        params->isShell = rScript;
        params->showConsole = rConsole;

        if (rscActive)
        {
            rscActive = false;
            pthread_cancel(rThread);
            m_logger->writeToLog("Manual cancellation for the script: " + returnPath(_but));
        }
        else
        {
            rscActive = true;
            rThread = 0;
            pthread_create(&rThread, NULL, startThread, (void*)params);
            pthread_detach(rThread);
        }
    }

    if (_but == Green)
    {
        params->pathFile = m_gPath;
        params->isShell = gScript;
        params->showConsole = gConsole;

        if (gscActive)
        {
            gscActive = false;
            pthread_cancel(gThread);
            m_logger->writeToLog("Manual cancellation for the script: " + returnPath(_but));
        }
        else
        {
            gscActive = true;
            gThread = 0;
            pthread_create(&gThread, NULL, startThread, (void*)params);
            pthread_detach(gThread);
        }
    }

    if (_but == Yellow)
    {
        params->pathFile = m_yPath;
        params->isShell = yScript;
        params->showConsole = yConsole;

        if (yscActive)
        {
            yscActive = false;
            pthread_cancel(yThread);
            m_logger->writeToLog("Manual cancellation for the script: " + returnPath(_but));
        }
        else
        {
            yscActive = true;
            yThread = 0;
            pthread_create(&yThread, NULL, startThread, (void*)params);
            pthread_detach(yThread);
        }
    }

    if (_but == Blue)
    {
        params->pathFile = m_bPath;
        params->isShell = bScript;
        params->showConsole = bConsole;

        if (bscActive)
        {
            bscActive = false;
            pthread_cancel(bThread);
            m_logger->writeToLog("Manual cancellation for the script: " + returnPath(_but));
        }
        else
        {
            bscActive = true;
            bThread = 0;
            pthread_create(&bThread, NULL, startThread, (void*)params);
            pthread_detach(bThread);
        }
    }

    return;
}

string OSDShell::returnPath(ButCol _but)
{
    if (_but == Red)
        return m_rPath;

    if (_but == Green)
        return m_gPath;

    if (_but == Yellow)
        return m_yPath;

    if (_but == Blue)
        return m_bPath;

    return "";
}

/*void OSDShell::blockAllKeys()
{
    SendCommand(PCMD_BlockKey, REM_UP, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_DOWN, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_RIGHT, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_LEFT, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_MENU, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_OK, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_FAVORITE, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_SAT, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_GUIDE, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_EXIT, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_INFO, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_TVRADIO, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_PLAYLIST, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_AUDIOTRK, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_0, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_2, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_3, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_4, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_5, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_6, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_7, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_8, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_9, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_SLEEP, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_TELETEXT, Rem_Block);
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
    SendCommand(PCMD_BlockKey, REM_CHUP, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_CHDOWN, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_PIP, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_PIPLIST, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_SUBT, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_FRONT_UP, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_FRONT_DOWN, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_FRONT_RIGHT, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_FRONT_LEFT, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_FRONT_OK, Rem_Block);
}*/

void OSDShell::clearStream(void* _arg)
{
    ifstream *fl = (ifstream*) _arg;

    if (fl->is_open())
    {
        fl->close();
        delete fl;
    }
}

void OSDShell::clearFile(void* _arg)
{
    FILE* stream = (FILE*) _arg;
    
    if (stream)
        pclose(stream);
}

void OSDShell::finishThread(void* _arg)
{
    ButCol flag = * (ButCol*) _arg;
    
    if (flag == Red)
    {
        rscActive = false;
        if (rConsole && !rScript)
        {
            FillBox(0, 0, GetOsdWidth(), GetOsdHeight(), 0x00000000);
            keyLock();
        }
    }

    if (flag == Green)
    {
        gscActive = false;
        if (gConsole && !gScript)
        {
            FillBox(0, 0, GetOsdWidth(), GetOsdHeight(), 0x00000000);
            keyLock();
        }
    }

    if (flag == Yellow)
    {
        yscActive = false;
        if (yConsole && !yScript)
        {
            FillBox(0, 0, GetOsdWidth(), GetOsdHeight(), 0x00000000);
            keyLock();
        }
    }

    if (flag == Blue)
    {
        bscActive = false;
        if (bConsole && !bScript)
        {
            FillBox(0, 0, GetOsdWidth(), GetOsdHeight(), 0x00000000);
            keyLock();
        }
    }
}

void* OSDShell::startThread(void* _args)
{
    Logger log("/tmp/", "OSDShell.log", isActiveTextLog, logPort, logServer);

    // pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,  NULL);

    ThrPar  *input_args = static_cast<ThrPar*>(_args);
    string  pathFile = input_args->pathFile;
    bool    isShell = input_args->isShell;
    bool    showConsole = input_args->showConsole;
    int     x = input_args->x;
    int     y = input_args->y;
    int     w = input_args->w;
    int     h = input_args->h;
    dword   font = input_args->font;
    dword   back = input_args->back;
    int     size = input_args->size;
    int     x_shift = input_args->x_shift;
    int     y_shift = input_args->y_shift;
    int     indent = input_args->indent;
    ButCol  bc = input_args->bc;
    int     time = input_args->time;
    int     max = (h - 2 * y_shift) / (size + indent);

    delete input_args;

    pthread_cleanup_push(finishThread, (void *) &bc);
        
    if (isShell)
    {
        log.writeToLog("Run script " + pathFile + " as linux shell script.");
        system((char *)pathFile.c_str());
    }
    else
    {
        ifstream *fl = NULL;

        fl = new ifstream();
        if (fl)
        {
            pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
            fl->open(pathFile.c_str(), ios::binary);
            pthread_cleanup_push(clearStream, fl);
            pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

            try
            {
                if (fl->is_open())
                {
                    fl->clear();
                    fl->seekg(0);

                    log.writeToLog("Run the script " + pathFile + " line by the line with the screen message box...");

                    while(!fl->eof())
                    {
                        FILE* stream = NULL;
                        char buf[MAX_BUF];
                        fl->getline(buf, MAX_BUF);
                        string rdStr(buf);

                        pthread_cleanup_push(clearFile, stream);

                        if (rdStr != "")
                        {
                            rdStr.append(" 2>&1");

                            pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
                            stream = popen(rdStr.c_str(), "r");
                            pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

                            if (stream) 
                            {
                                char buffer[MAX_BUF];
                                int cur = 0;
                                while (!feof(stream))
                                if (fgets(buffer, MAX_BUF, stream) != NULL)
                                {
                                    if (cur == 0 && showConsole)
                                    {
                                        //blockAllKeys();
                                        DrawRect(x - 2, y - 1, w + 4, h + 2, font);
                                        FillBox(x, y, w, h, back);
                                    }

                                    if (cur == max)
                                    {
                                        FillBox(x, y, w, h, back);
                                        cur = 0;
                                    }

                                    string data(buffer);

                                    if (showConsole)
                                    {
                                        SetFontSize(size);
                                        DrawText(x + x_shift, y + y_shift + cur * (size + indent), w - x_shift * 2, size, (char *)data.c_str(), font, back, ALIGN_Center, VALIGN_Center, 0); // здесь падает на координатах
                                    }

                                    data.resize(strcspn(data.c_str(), "\n"));
                                    log.writeToLog(data);

                                    cur++;
                                }

                                pclose(stream);
                            }
                        }
                        pthread_cleanup_pop(1);
                    }
                    fl->close();
                }
            }
            catch (ifstream::failure)
            {
                cerr << "Error the script file " + pathFile + " open!\n";
            }
            
            pthread_cleanup_pop(1);
        }

        if (fl)
        {
            delete fl;
            fl = NULL;
        }
    }

    sleep(time);
    finishThread((void *)&bc);

    pthread_cleanup_pop(1);
    pthread_exit(0);
}
