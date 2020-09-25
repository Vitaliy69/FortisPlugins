/*!
    \file plugin.cpp
    \author Vitaliy Gribko
    
    Distributed under the GPL v2
    see the file LICENSE for details
    or visit http://www.gnu.org/copyleft/gpl.html
*/

#include "global.h"

#include "channelstats.h"

static ChannelStats *channelStats = NULL;
static int          timerStatId = -1;
static int          timerUpdateId = -1;

int PluginMain(int UNUSED(argc), char *argv[])
{
    dword evt[4];

    WiFront_Init();
    FontInit((char *) "/var/etc/swissc.ttf");

    char hwAddress[12];
    memset(&hwAddress, 0, sizeof hwAddress);

    int pollPeriod, updatePeriod;

    channelStats = new ChannelStats(std::string(argv[0]));

    pollPeriod = channelStats->getPollPeriod();
    updatePeriod = channelStats->getUpdatePeriod();

    timerStatId = WiTimer_EveryQueue(pollPeriod * 100, WiMainQ);
    timerUpdateId = WiTimer_EveryQueue(updatePeriod * 100, WiMainQ);

    while(true)
    {
        int ret = WiQueue_Receive(WiMainQ, (u_int8 *) evt, 16, -1);

        if (ret == 0)
        {
            if(evt[0] == PLUGINMSG_StateChanged)
            {
                if(evt[1] == STATE_Normal)
                {
                    if (timerStatId == -1)
                    {
                        timerStatId = WiTimer_EveryQueue(pollPeriod * 100, WiMainQ);
                    }

                    if (timerUpdateId == -1)
                    {
                        timerUpdateId = WiTimer_EveryQueue(updatePeriod * 100, WiMainQ);
                    }
                }

                if(evt[1] == STATE_Standby)
                {
                    if (timerStatId != -1)
                    {
                        WiTimer_Delete(timerStatId);
                        timerStatId = -1;
                    }

                    if (timerUpdateId != -1)
                    {
                        WiTimer_Delete(timerUpdateId);
                        timerUpdateId = -1;
                    }
                }
            }

            if((evt[0] & MSG_BASE_MASK) == MSG_BASE_TIMER)
            {
                if (evt[0] == (dword) timerStatId)
                {
                    StatsData data;
                    memset(&data, 0, sizeof data);

                    S_Service *curSer = GetCurService(SERVICE_Main);
                    S_Tp *tpIdx = GetTp(SERVICE_Main);
                    S_EventInfo *epgData = NULL;

                    if (!curSer || !tpIdx)
                    {
                        data.isLive = false;
                    }
                    else
                    {
                        int tuner, isLock, strength, quality;

                        tuner = GetCurSvcTuner(SERVICE_Main);
                        GetTunerState(tuner, &isLock, &strength, &quality);

                        if (quality == 0)
                        {
                            data.noSignal = true;
                        }
                        else
                        {
                            data.isLive = true;

                            data.networkName = (const char*) GetNetworkName(tpIdx, true);
                            data.networkId = tpIdx->networkId;
                            data.orgNetId = tpIdx->orgNetId;
                            data.tsId = tpIdx->tsId;
                            data.svcId = curSer->svcId;
                            data.serviceName = (const char*) GetSvcName(curSer, false);

                            int eventsCount;
                            bool epg = GetEventDataNow(curSer, &eventsCount, &epgData);

                            if (epg && epgData)
                            {
                                dword cTime = GetCurTime();
                                for (int i = 0; i < eventsCount; i++)
                                {
                                    if((epgData[i].startTime <= cTime) && (epgData[i].endTime > cTime))
                                    {
                                        data.epgText = (const char*) epgData[i].text;
                                        data.langId = epgData[i].codeIdx;
                                        break;
                                    }
                                }
                            }
                            else
                            {
                                data.langId = GetConfig(CFG_MENULANG);
                            }
                        }
                    }

                    channelStats->performRequest(data);

                    if (epgData)
                    {
                        free(epgData);
                        epgData = NULL;
                    }
                }

                if (evt[0] == (dword) timerUpdateId)
                {
                    channelStats->performUpdate();
                }
            }

            if ((timerStatId != -1) && (channelStats->getPollPeriod() != pollPeriod))
            {
                pollPeriod = channelStats->getPollPeriod();

                WiTimer_Delete(timerStatId);
                timerStatId = WiTimer_EveryQueue(pollPeriod * 100, WiMainQ);
            }

            if ((timerUpdateId != -1) && (channelStats->getUpdatePeriod() != updatePeriod))
            {
                updatePeriod = channelStats->getUpdatePeriod();

                WiTimer_Delete(timerUpdateId);
                timerUpdateId = WiTimer_EveryQueue(updatePeriod * 100, WiMainQ);
            }
        }
    }

    return 0;
}

void PluginClose()
{
    if (timerStatId != -1)
    {
        WiTimer_Delete(timerStatId);
    }

    if (timerUpdateId != -1)
    {
        WiTimer_Delete(timerUpdateId);
    }

    if (channelStats)
    {
        delete channelStats;
    }

    FontClose();
    WiFront_Term();
}
