/*!
    \file plugin.cpp
    \author Vitaliy Gribko
    
    Distributed under the GPL v2
    see the file LICENSE for details
    or visit http://www.gnu.org/copyleft/gpl.html
*/

#include "plugin.h"
#include "osdclock.h"

static OSDClock *osdClock = NULL;
static byte     *imgBuf = NULL;
static int      timerId = -1;

int PluginMain(int UNUSED(argc), char *UNUSED(argv[]))
{
    dword evt[4];

    osdClock = new OSDClock();

    WiFront_Init();
    FontInit((char *)(osdClock->getFamily().c_str()));

    imgBuf = OsdCompress(osdClock->getX(), osdClock->getY(), osdClock->getW(), osdClock->getH());

    if (GetState(STATEIDX_Main) != STATE_PlugInMenu)
        SendCommand(PCMD_SetState, STATE_Normal);

    while(true)
    {
        int ret = WiQueue_Receive(WiMainQ, (u_int8 *)evt, 16, -1);

        if (ret == 0)
        {
            if(evt[0] == PLUGINMSG_StateChanged) 
            {
                if(evt[1] == STATE_Normal)
                {
                    if (timerId == -1)
                    {
                        osdClock->showTime();
                        timerId = WiTimer_EveryQueue(osdClock->getInterval() * 100, WiMainQ);
                    }
                }
                else
                {
                    WiTimer_Delete(timerId);
                    timerId = -1;
                }
            }

            if((evt[0] & MSG_BASE_MASK) == MSG_BASE_TIMER)
                osdClock->showTime();
        }
    }

    return 0;
}

void PluginClose()
{
    FontClose();

    if (timerId != -1)
        WiTimer_Delete(timerId);

    OsdDecompress(imgBuf, osdClock->getX(), osdClock->getY(), osdClock->getW(), osdClock->getH());
    free(imgBuf);
    imgBuf = NULL;
    
    delete osdClock;

    system("killall -9 OSDClock");
}
