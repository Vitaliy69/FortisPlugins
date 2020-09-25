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

#include <string>

#include <unistd.h>

#define X               440
#define XOLD            180
#define Y               240
#define YOLD            275
#define W               400
#define WOLD            360
#define H               35
#define HOLD            40
#define FAMILY          "/var/etc/swissc.ttf"
#define SIZE            24
#define SIZEOLD         18
#define FONT            0xFFFFFFFF
#define BACK            0x40000000
#define SHOWTEXT        1
#define CDWAIT          6
#define RSWAIT          6
#define RTCOUNT         500
#define SCRIPT          "/var/scripts/wicardd-restart.sh"

using namespace std;

int         x, y, w, h, size, cdWt, rsWt, rtCn;
string      family, script;
dword       font, back;
bool        showText;
byte        *osd;

Logger      *logger = NULL;
Configer    *configer = NULL;

void InitPlugin()
{
    bool isActiveTextLog = true;
    int logPort = 0;
    std::string logServer = "";

    if (!access("/var/etc/EMURestarter.cfg", 0))
    {
        configer = new Configer("/var/etc/EMURestarter.cfg");

        isActiveTextLog = configer->getBoolValue("logging", "enableTextlog");
        logPort = configer->getIntValue("logging", "syslogport");
        logServer = configer->getStrValue("logging", "sysloghost");
    }
    
    logger = new Logger("/tmp/", "EMURestarter.log", isActiveTextLog, logPort, logServer, "EMURestarter, v0.8");
    
    if (access("/var/etc/EMURestarter.cfg", 0))
    {
        logger->writeToLog("The configuration file was not found. Use the default settings.");

        if (GetOsdWidth() == 1280) //< STi H205
        {
            x = X;
            y = Y;
            w = W;
            h = H;
        }
        else
        {
            x = XOLD;
            y = YOLD;
            w = WOLD;
            h = HOLD;
        }

        size = SIZE;
        family = FAMILY;
        font = FONT;
        back = BACK;
        showText = SHOWTEXT;
        cdWt = CDWAIT;
        rsWt = RSWAIT;
        rtCn = RTCOUNT;
        script = SCRIPT;
    }
    else
    {
        logger->writeToLog("Reading the configuration file...");

        if (GetOsdWidth() == 1280) //< STi H205
        {
            x = configer->getIntValue("position", "posX");
            if ((x < 0) || (x > GetOsdWidth()))
                x = X;

            logger->writeToLog("posX =", x);

            y = configer->getIntValue("position", "posY");
            if ((y < 0) || (y > GetOsdHeight()))
                y = Y;

            logger->writeToLog("posY =", y);

            w = configer->getIntValue("position", "width");
            if ((w < 0) || (w > GetOsdWidth()))
                w = W;

            logger->writeToLog("width =", w);

            h = configer->getIntValue("position", "height");
            if ((h < 0) || (h > GetOsdHeight()))
                h = H;

            logger->writeToLog("height =", h);
        }
        else
        {
            x = configer->getIntValue("position", "posX");
            if ((x < 0) || (x > GetOsdWidth()))
                x = XOLD;

            logger->writeToLog("posX =", x);

            y = configer->getIntValue("position", "posY");
            if ((y < 0) || (y > GetOsdHeight()))
                y = YOLD;

            logger->writeToLog("posY =", y);

            w = configer->getIntValue("position", "width");
            if ((w < 0) || (w > GetOsdWidth()))
                w = WOLD;

            logger->writeToLog("width =", w);

            h = configer->getIntValue("position", "height");
            if ((h < 0) || (h > GetOsdHeight()))
                h = HOLD;

            logger->writeToLog("height =", h);
        }

        size = configer->getIntValue("font", "size");
        if ((size < 6) || (size > 96))
            size = SIZE;

        logger->writeToLog("Font size:", size);
        
        family = configer->getStrValue("font", "family");

        if (family.length() < 1 || access(family.c_str(), 0))
            family = FAMILY;

        logger->writeToLog("Font path: " + family);

        string fontStr = configer->getStrValue("color", "font");
        if (!configer->validateColor(fontStr))
        {
            font = FONT;
            logger->writeToLog("Font color: " + string("0xFFFFFFFF"));
        }
        else
        {
            font = strtol(fontStr.c_str(), 0, 0);
            logger->writeToLog("Font color: " + fontStr);
        }

        string fontBack = configer->getStrValue("color", "back");
        if (!configer->validateColor(fontBack))
        {
            back = BACK;
            logger->writeToLog("Font background: " + string("0x40000000"));
        }
        else
        {
            back = strtol(fontBack.c_str(), 0, 0);
            logger->writeToLog("Font background: " + fontBack);
        }

        showText = configer->getBoolValue("options", "showText");
        if (showText)
            logger->writeToLog("Message box will show the information at the screen.");
        else
            logger->writeToLog("Message box will not show the information at the screen.");

        cdWt = configer->getIntValue("options", "codeWait");
        if ((cdWt < 0) || (cdWt > 60))
            cdWt = CDWAIT;

        logger->writeToLog("Wait time before to start after encrypted channel message appear, s:", cdWt);
        
        rsWt = configer->getIntValue("options", "restarWait");
        if ((rsWt < 3) || (rsWt > 60))
            rsWt = RSWAIT;

        logger->writeToLog("Wait time before to check for the encrypted channel, s:", rsWt);
                
        rtCn = configer->getIntValue("options", "restartCount");
        if ((rtCn < 3) || (rtCn > 10000))
            rtCn = RTCOUNT;

        logger->writeToLog("Restarts count:", rtCn);

        script = configer->getStrValue("options", "restartScript");

        if (script.length() < 1 || access(script.c_str(), 0))
            script = SCRIPT;
        
        logger->writeToLog("Script path: " + script);
    }

    osd = NULL;
}

int PluginMain(int UNUSED(argc), char *UNUSED(argv[]))
{
    dword evt[4];

    InitPlugin();
    FontInit((char *)(family.c_str()));

    while(true)
    {
        int ret = WiQueue_Receive(WiMainQ, (u_int8 *)evt, 16, -1);

        if (ret == 0)
        {
            int tuner, isLock, strength, quality;
            tuner = GetCurSvcTuner(SERVICE_Main);
            GetTunerState(tuner, &isLock, &strength, &quality);

            while ((GetShownUnit() == UNIT_WarnBar) && (quality > 0))
            {
                logger->writeToLog("Encrypted channel is detect.");
                sleep(cdWt);

                if (GetShownUnit() != UNIT_WarnBar)
                {
                    logger->writeToLog("Restart is not required.");
                    break;
                }

                logger->writeToLog("Emu restarting...");

                if (showText)
                {
                    osd = OsdCompress(x, y, w, h);
                    SetFontSize(size);
                    DrawRect(x, y, w, h, font);    
                    DrawText(x + 2, y + 1, w - 4, h - 2, (char *)"Emu restarting...", font, back, ALIGN_Center, VALIGN_Center, 0);
                    sleep(1);
                }

                for (int cn = 0; cn < rtCn; cn++)
                {
                    logger->writeToLog("Count #", cn + 1);
                    system((char *)script.c_str());
                    sleep(rsWt);

                    if (GetShownUnit() != UNIT_WarnBar)
                    {
                        if (osd)
                        {
                            OsdDecompress(osd, x, y, w, h);
                            free(osd);
                            osd = NULL;
                            FillBox(0, 0, GetOsdWidth(), GetOsdHeight(), 0);
                        }
                        logger->writeToLog("Restart is successful!");
                        sleep(1);
                        break;
                    }
                }

                if (GetShownUnit() == UNIT_WarnBar)
                {
                    logger->writeToLog("Failed to restart!");
                    OsdDecompress(osd, x, y, w, h);
                    free(osd);
                    osd = NULL;
                    FillBox(0, 0, GetOsdWidth(), GetOsdHeight(), 0);
                }

                break;
            }

            sleep(3);
        }
    }

    return 0;
}

void PluginClose()
{
    FontClose();

    if (configer) delete configer;
    if (logger) delete logger;
}
