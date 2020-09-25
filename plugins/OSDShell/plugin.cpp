/*!
    \file plugin.cpp
    \author Vitaliy Gribko
    
    Distributed under the GPL v2
    see the file LICENSE for details
    or visit http://www.gnu.org/copyleft/gpl.html
*/

#include "plugin.h"
#include "font.h"
#include "type.h"
#include "osdshell.h"

static OSDShell *osdShell = NULL;

int PluginMain(int UNUSED(argc), char *UNUSED(argv[]))
{
    dword evt[4];
    int key = 0;

    osdShell = new OSDShell();

    WiFront_Init();
    FontInit((char *)(osdShell->getFamily().c_str()));

    SendCommand(PCMD_ReleaseKey, (dword)-1);

    osdShell->keyLock();

    while(true)
    {
        int ret = WiQueue_Receive(WiMainQ, (u_int8 *)evt, 16, -1);

        if (ret == 0)
        {
            if(evt[0] == PLUGINMSG_StateChanged) 
            {
                if(evt[1] == STATE_Normal)
                    osdShell->keyLock();
                else
                    SendCommand(PCMD_ReleaseKey, (dword)-1);
            }

            if((evt[0] & MSG_BASE_MASK) == MSG_BASE_REMOCON)
            {
                key = evt[1] & MSG_MASK_KEY_CODE;

                if (GetState(STATEIDX_Main) != STATE_PlugInMenu)
                {
                    SendCommand(PCMD_Key, REM_EXIT);
                    SendCommand(PCMD_Key, REM_EXIT);
                    SendCommand(PCMD_Key, REM_EXIT);

                    if (key == REM_F1)
                    {
                        osdShell->execCommand(Red);
                    }

                    if (key == REM_F2)
                    {
                        osdShell->execCommand(Green);
                    }

                    if (key == REM_F3)
                    {
                        osdShell->execCommand(Yellow);
                    }

                    if (key == REM_F4)
                    {
                        osdShell->execCommand(Blue);
                    }
                }
            }
        }
    }

    return 0;
}

void PluginClose()
{
    FontClose();
    WiFront_Term();

    delete osdShell;
}
