
#ifndef __PLUGIN_H__
#define  __PLUGIN_H__

#include "type.h"
#include "osd.h"
#include "dbg.h"
#include "front.h"
#include "stbdata.h"

extern int WiMainQ;

#define PLUGIN_CMD_LEN		32

typedef enum
{
	PCMD_Key = 0xF1230001,
	PCMD_SetConfig,			// idx, value
	PCMD_GetEpg,			// satIdx, orgNetId, tsId, svcId	- Ack- APPLMSG_EPG_Data_Ack
	PCMD_SvcStop,			// mainSub
	PCMD_SvcStart,			// mainSub, svcType, svcNum, satIdx( 1 + satIdx )
	PCMD_SvcMove,			// mainSub, upDown
	PCMD_ChannelSetIdx,	// tuner, tpIdx
	PCMD_ChannelSet,		// tuner, satIdx, freq, sr, polar
	PCMD_SetVolume,		// volume %
	PCMD_SetAudio,			// (0- manual, 1- isAuto, 2- off), pid, isAc3, soundMode
	PCMD_SetScreen,		// [1]- _display_resolution_ idx, [2]- _display_format_ idx, [3]- _screen_ratio_ idx : value will be applied if it is not (-1).
							// [4]- decode_wh(width<<16|height), [5]- decode_xy(x<<16|y), [6]- view_wh(width<<16|height), [7]- view_xy(x<<16|y) : if _wh is 0 don't apply
	PCMD_BlockKey,			// value
	PCMD_ReleaseKey,		// value
	PCMD_SetState,			// mainState
	PCMD_Blit,				// not support.
	PCMD_FillBox,			// x, y, w, h, color	- 0xf1230010
	PCMD_SaveState, 		// stateidx, state
	PCMD_RestoreState,		// stateidx

	PCMD_WavStart,
	PCMD_WavWrite,
	PCMD_WavStop,

	PCMD_NotUsed4,
	PCMD_NotUsed5,

	PCMD_NotifyMsg,			// send message.
	PCMD_UartCtrl,			// uart control
	PCMD_ProtectedData,	// protected data store.

	PCMD_NormalUnit,		// block Live mode unit: msg[0]- PCMD_NormalUnit, msg[1]- block(1)/release(0), msg[2]- UNIT_xxxx
	PCMD_DestroyAndSaveSubState,	// destroy all popup items and set substate to data1

	PCMD_SetAlarm,			// Use SetAlarmPtr function. (evtId<<16 | svcId), (tsId<<16 | orgNetId), (svcType<<16 | svcIdx), satIdx, sTime, eTime, (alarmType<<24 | alarmMode<<16 | alarmDays<<8 | alarmPwOff)
							// ok- APPLMSG_ALARM_DB_Changed, not ok- APPLMSG_ALARM_Set_Fail
	PCMD_GetAlarm,			// Ack- APPLMSG_ALARM_DB_Changed. main s/w Send APPLMSG_ALARM_DB_Changed message when alarm db is changed too.

	PCMD_FilterStart,
	PCMD_FilterStop,

	PCMD_SetControl,
	PCMD_ClearControl,

	PCMD_EnableKey,			// Internal use only for rcu ctrl
	PCMD_DisableKey,			// Internal use only for rcu ctrl

	PCMD_SvcStartInternalTest,	// Internal use only - mainSub, svcType, svcNum, service_tune_flag
	PCMD_SvcStopInternalTest,	// Internal use only.

	PCMD_StopVideoStream,	// sel
	PCMD_SetVideoStream,	// sel, pid

	PCMD_GetSvcState,		// [1]- sel(main, sub), [2]- pid [ ack- PLUGINMSG_SvcState ]

	PCMD_ExternCi,			// Internal use only. get ciplus apdu to ciplus module.	PCMD_Ciplus_pass
	PCMD_BrandId,			// Internal use only. ciplus

	PCMD_DestroyAllStateAndSaveSubState,	// destroy all menu and popup items and then set substate to data1
	PCMD_BoxInfo,			// Internal use only. Do not send directly. use BoxInfo function.

	PCMD_ShowStillPic,		// Use ShowStillPic( S_BlitImg *img , int dataSize ) function.
	PCMD_HideStillPic,		// Use HideStillPic() function.

	PCMD_SetSubtStream,	// sel, pid

	PCMD_RegionShow,		// Use RegionShow( int id, int on ) function.
	PCMD_ChangeLayer,		// Internal use only.

	PCMD_REQURLPLAY,		// Internal use only.
	PCMD_URLCONTROL,		// Internal use only.

	PCMD_SubtControl,		// Internal use only.
	PCMD_UrlPageLoad,		// Internal use only.

	PCMD_SetKeyCodeMethod,		// If set on should receive APPLMSG_REMOCON insted of MSG_BASE_REMOCON [ 0: APPLMSG_REMOCON 1: CMD_xx 2: MSG_MASK_KEY_REPEAT ]

	PCMD_ExternalStart = 0xF123E001,	// !! UPPER MESSAGE CODE IS ASSIGNED FOR INTERNAL USE.
	PCMD_RestartPlugin,		// restart current plugin on Main application, no param.


} T_PluginCmd;

typedef enum PluginType
{
	PLUGINTYPE_NONE,
	PLUGINTYPE_WIDGET,
	PLUGINTYPE_STANDALONE,
	PLUGINTYPE_WEBBROWSER
} PluginType_t;

// PCMD_NormalUnit support UNIT_InfoBox, UNIT_DirectBar only.
typedef enum
{
	UNIT_InfoBox		= 0x00000001,	// After open detail infobox - STATEIDX_Sub has SUBSTATE_IBox value
	UNIT_VolBar			= 0x00000002,	// Volume bar
	UNIT_SleepBar		= 0x00000004,
	UNIT_DirectBar		= 0x00000008,	// Channel number input bar OR simple service list
	UNIT_WarnBar		= 0x00000010,	// No Signal and so on

	UNIT_PlayBar		= 0x00000020,	// - Has SUBSTATE_PlayBar value

	UNIT_PvrInfoBar		= 0x00000040,	// Recording information, etc.
	UNIT_CasBar		= 0x00000080,	// Smart card message, etc.

	UNIT_AudioDelayBar = 0x00000100,	// - Has SUBSTATE_SetAudioDelay value

} T_NormalUnit;

extern dword GetShownUnit();	// return shown UNIT_xxx value.

typedef enum
{
	DMODE_Section,
	DMODE_SectionRepeat,
	DMODE_SectionRepeatTable,
	DMODE_SectionRepeatECM,
	DMODE_SectionToggle,
	DMODE_Table,
	DMODE_TableToggle,
} T_DmxMode;

extern void SendCommand(dword *cmd); // cmd size = PLUGIN_CMD_LEN
extern void SendCommand(dword cmd, dword data1=0, dword data2=0, dword data3=0, dword data4=0, dword data5=0, dword data6=0, dword data7 =0);
extern void SendCommandAndData(dword cmd, dword msg1, dword msg2, byte *data, dword size);

extern int SendCmdAndWaitAnswer(dword *cmd, dword waitMsg, dword *msg); // cmd & msg size = PLUGIN_CMD_LEN
extern int SendCmdAndDataAndWaitAnswer(dword cmd, dword msg1, dword msg2, byte *data, dword size, dword waitMsg, dword *msg); // msg size = PLUGIN_CMD_LEN

//=========================================================================

typedef enum
{
	SERVICE_Main = 0,
	SERVICE_Sub,
	SERVICE_Rec1,
	SERVICE_Rec2,
	SERVICE_Rec3,
	N_ServiceHandle,
} T_ServiceHandle;

//ETC =========================================================================

typedef void *(* PFNTASK)(void *);
typedef enum task_priority {
	TPrio_Min,
	TPrio_Mid,
	TPrio_Max,
} T_TaskPriority;

//
extern int WiTask_Priority(T_TaskPriority prio);	// don't use bigger than TPrio_Mid.
extern int WiTask_Create(PFNTASK entry, void *arg, int stack_size, int prio, char *name);
extern int WiTask_Delete(int id);
extern int WiTask_Suspend(int id);
extern int WiTask_Resume(int id);
extern int WiTask_Sleep(int milli_sec);
extern int WiTask_Id();
//
extern int WiSem_Create(int init_value);
extern int WiSem_Delete(int id);
extern int WiSem_Take(int id, int tmo);
extern int WiSem_Release(int id);
//
extern unsigned long GetTick(void);

//=== TIMER ===================================================================

#define MAX_TIMER 64

extern int WiTimer_AfterCallback(int ticks, void (*callback)(int, int, int), int arg);
extern int WiTimer_AfterCallbackMs(int ticks, void (*callback)(int, int, int), int arg);
extern int WiTimer_EveryCallback(int ticks, void (*callback)(int, int, int), int arg);
extern int WiTimer_AfterQueue(int ticks, int queue);
extern int WiTimer_AfterQueueMs(int ticks, int queue);
extern int WiTimer_EveryQueue(int ticks, int queue);
extern int WiTimer_Delete(int id);
extern int WiTimer_After(int ticks);
extern int WiTimer_Every(int ticks);
extern long long WiTimer_TicksMs();
extern unsigned int WiTimer_Ticks();

// === TOOLS ==================================================================

extern unsigned short	GetCrc16(unsigned char *data, int size);
extern unsigned long	GetMpegCrc32(unsigned char *data, int size);

extern unsigned long	Get12bit(unsigned char *addr);
extern void			Put12bit(unsigned char *addr, int data);
extern unsigned long	Get13bit(unsigned char *addr);
extern void			Put13bit(unsigned char *addr, int data);
extern unsigned long	Get16bit(unsigned char *addr);
extern void			Put16bit(unsigned char *addr, int data);
extern unsigned long	Get24bit(unsigned char *addr);
extern void			Put24bit(unsigned char *addr, int data);
extern unsigned long	Get32bit(unsigned char *addr);
extern void			Put32bit(unsigned char *addr, int data);

extern char * GetStrBuf(void);	// len- 256

//CI ==========================================================================

enum _type_ci_msg {
	CI_APDU = 1,	/* message : MSG_BASE_CI, (sessNum<<8) | slot, CI_APDU, data */
	CI_CLEAR,		/* message : MSG_BASE_CI, slot, CI_CLEAR, delay */
	CI_STATUS,		/* message : MSG_BASE_CI,                slot, CI_STATUS, status(CI_CAM_INSERT, CI_CAM_ENABLED, CI_CAM_REMOVED) */
	CI_OPENED,		/* message : MSG_BASE_CI, (sessNum<<8) | slot, CI_OPENED, resourceId */
	CI_CLOSED,		/* message : MSG_BASE_CI, (sessNum<<8) | slot, CI_CLOSED, resourceId */
	CI_CONTENTKEY,
	CI_URISET,
	CI_REQCREDENTIAL	/* Request credentails */
};

enum _type_ci_cam_status {
	CI_CAM_INSERT = 1,
	CI_CAM_ENABLED,
	CI_CAM_REMOVED
};

enum _type_ci_slot {
	CI_CAM_SLOT0 = 0,
	CI_CAM_SLOT1
};

extern int WiCi_SendApdu(int slot, int sessNum, int apduLen, u_int8 *apdu);
extern int WiCi_EnableCam(int slot);
extern int WiCi_EnableEachCam(int slot, int on, int direct);
extern int WiCi_AssignTS3(int ts);
extern int WiCi_AssignCam(int ts1, int ts2);

//ICC =========================================================================

/* slot number */
enum {
	SMART_SLOT1,
	SMART_SLOT2
};

// Smart

/* event */
enum {
	SMART_NONE,
	SMART_INSERT,
	SMART_REMOVE
};

/* status */
enum {
	SMART_SLOT_IDLE,
	SMART_SLOT_INITIALIZED,
	SMART_SLOT_INSERTED,
	SMART_SLOT_READY
};

#if 0
extern int WiSmart_Read(int slot, u_int8 *out_buffer, int out_len, u_int8 *in_buffer, int in_len, int tmo, u_int8 *sw1, u_int8 *sw2);
extern int WiSmart_Write(int slot, u_int8 *out_buffer, int out_len, u_int8 *in_buffer, int in_len, int tmo, u_int8 *sw1, u_int8 *sw2);
extern int WiSmart_GetAtr(int slot, u_int8 *buffer, int *len);
extern int WiSmart_Status(int slot, int *status);
extern int WiSmart_GetHandle(int slot);
#endif

// Smart extend

typedef enum
{
   WI_SMART_CNXT_SMC_EMPTY = 0,
   WI_SMART_CNXT_SMC_NOT_READY,
   WI_SMART_CNXT_SMC_READY
} WI_SMART_CNXT_SMC_STATE;

/* card convention */
typedef enum
{
   WI_SMART_CNXT_SMC_CONV_DIRECT,
   WI_SMART_CNXT_SMC_CONV_INVERSE
} WI_SMART_CNXT_SMC_CONVENTION;

/* card protocol */
typedef enum
{
   WI_SMART_CNXT_SMC_PROTOCOL_T0,
   WI_SMART_CNXT_SMC_PROTOCOL_T1,
   WI_SMART_CNXT_SMC_PROTOCOL_T14 = 14
} WI_SMART_CNXT_SMC_PROTOCOL;

/* card config items */
typedef enum
{
   WI_SMART_CNXT_SMC_CONFIG_CONVENTION,
   WI_SMART_CNXT_SMC_CONFIG_PROTOCOL,
   WI_SMART_CNXT_SMC_CONFIG_FI,
   WI_SMART_CNXT_SMC_CONFIG_DI,
   WI_SMART_CNXT_SMC_CONFIG_PI1,
   WI_SMART_CNXT_SMC_CONFIG_PI2,
   WI_SMART_CNXT_SMC_CONFIG_II,
   WI_SMART_CNXT_SMC_CONFIG_N,
   WI_SMART_CNXT_SMC_CONFIG_HISTORICAL,
   WI_SMART_CNXT_SMC_CONFIG_HISTORICAL_LEN,
   WI_SMART_CNXT_SMC_CONFIG_TIMEOUT,
   WI_SMART_CNXT_SMC_CONFIG_RETRY
} WI_SMART_CNXT_SMC_CONFIG_ITEM;

/* return values of APIs */
typedef enum
{
   WI_SMART_CNXT_SMC_OK = 0,
   WI_SMART_CNXT_SMC_ALREADY_INIT,
   WI_SMART_CNXT_SMC_NOT_INIT,
   WI_SMART_CNXT_SMC_BAD_UNIT,
   WI_SMART_CNXT_SMC_CLOSED_HANDLE,
   WI_SMART_CNXT_SMC_BAD_HANDLE,
   WI_SMART_CNXT_SMC_BAD_PARAMETER,
   WI_SMART_CNXT_SMC_RESOURCE_ERROR,
   WI_SMART_CNXT_SMC_INTERNAL_ERROR,
   WI_SMART_CNXT_SMC_NOT_AVAILABLE,
   WI_SMART_CNXT_SMC_TIMEOUT
} WI_SMART_CNXT_SMC_STATUS;

/* events */
typedef enum
{
   WI_SMART_CNXT_SMC_EVENT_TERM,
   WI_SMART_CNXT_SMC_EVENT_RESET,
   WI_SMART_CNXT_SMC_EVENT_CARD_INSERTED,
   WI_SMART_CNXT_SMC_EVENT_CARD_RESET_COMPLETE,
   WI_SMART_CNXT_SMC_EVENT_CARD_RESET_TIMEOUT,
   WI_SMART_CNXT_SMC_EVENT_CARD_RW_COMPLETE,
   WI_SMART_CNXT_SMC_EVENT_CARD_RW_TIMEOUT,
   WI_SMART_CNXT_SMC_EVENT_CARD_POWER_DOWN_COMPLETE,
   WI_SMART_CNXT_SMC_EVENT_CARD_POWER_DOWN_TIMEOUT,
   WI_SMART_CNXT_SMC_EVENT_CARD_REMOVED
} WI_SMART_CNXT_SMC_EVENT;

typedef enum
{
  	SC_IS_EXTRACTED,
  	SC_IS_INSERTED,
  	SC_IS_OPERATIONAL
} enSC_State;

typedef struct
{
   u_int32 uLength;
   bool bExclusive;
   u_int32 uUnitNumber;
} WI_SMART_CNXT_SMC_CAPS;

typedef int (* WI_SMART_CNXT_SMC_PFNNOTIFY) (int pHandle,
                                               void            *pUserData,
                                               WI_SMART_CNXT_SMC_EVENT  Event,
                                               void            *pData,
                                               void            *Tag);

extern int WiSmart_smc_get_state(int handle, WI_SMART_CNXT_SMC_STATE  *Status);
extern int WiSmart_smc_powerdown_card(int handle, int bAsync, void *Tag);
extern int WiSmart_smc_get_config(int handle, WI_SMART_CNXT_SMC_CONFIG_ITEM Item, u_int32 *puValue);
extern int WiSmart_smc_read_write(int handle,
							int bAsync,
							void *pOutBuf,
							u_int32 uOutLength,
							void *pInBuf,
							u_int32 *pInLength,
							u_int32 uTimeOut,
							void *Tag,
							u_int8 flush);
extern int WiSmart_smc_read_writeT14(int 	handle,
								 int 	bAsync,
								 void  *pOutBuf,
								 u_int32	uOutLength,
								 void  *pInBuf,
								 u_int32   *pInLength,
								 u_int32	uTimeOut,
								 void  *Tag,
								 u_int8     flush);
extern int WiSmart_smc_set_config(int handle, WI_SMART_CNXT_SMC_CONFIG_ITEM Item, u_int32 uValue);
extern int WiSmart_smc_reset_card(int handle, int bAsync, void *Tag);
extern int WiSmart_smc_get_atr( int handle, void *pAtr, u_int32 *puBufLength);
extern int WiSmart_smc_reset(void *pCfg);
extern int WiSmart_smc_init(void *pCfg);
extern int WiSmart_smc_get_unit_caps(u_int32 uUnitNumber, WI_SMART_CNXT_SMC_CAPS *pCaps);
extern int WiSmart_smc_open(int *pHandle, WI_SMART_CNXT_SMC_CAPS *pCaps, WI_SMART_CNXT_SMC_PFNNOTIFY pNotifyFn, void *pUserData);
extern void WiSmart_smc_close(int handle);
extern void WiSmart_smc_flush( int handle );

extern int WiSmart_smc_get_unit_number(int handle);
extern void WiSmart_smc_set_clock_rate(int rate);

//DEMUX =======================================================================

#define MAX_FILTER			96 // Filter is 8byte Mode. If filter is 16byte Mode, Max is 48. Robin 09.07.30
#define MAX_DESCRAMBLER	16

/* copy of cnxt's genchannel_t in demuxapi.h */
enum channel_type {
    CHANNEL_VIDEO = 1,
    CHANNEL_AUDIO,
    CHANNEL_PES,
    CHANNEL_ES,
    CHANNEL_PSI
};

extern int WiDemux_DescramblerClear(int sel, int pid);
extern int WiDemux_DescramblerKey(int sel, int pid, int odd, u_int8 *key);

extern int WiDemux_FilterStart(int sel, int pid, int size, u_int8 *filter, u_int8 *mask, int sections, int continues, int queue, int callback, int timeOut
									, void (*fcallback)(unsigned char *, void *context)=NULL, void *context=NULL, int fd=-1/*internal use only*/);

extern int WiDemux_FilterStop(int evt);
extern void WiDemux_DisplayFilter();
extern void WiDemux_SetCallback(int evtCode, void (*callback)(unsigned char *, void *context), void *context);

//QUEUE =======================================================================

enum queue_type {
	FIXED_SIZE,
	VARIABLE_SIZE
};

extern int WiQueue_Rx4(int id, u_int32 *data, int tmo);
extern int WiQueue_Tx4(int id, u_int32 a0, u_int32 a1, u_int32 a2, u_int32 a3);
extern int WiQueue_Receive(int id, u_int8 *buf, int length, int tmo);
extern int WiQueue_Send(int id, u_int8 *buf);
extern int WiQueue_Delete(int id);
extern int WiQueue_Create(int elements, int size, int type, char *name);
extern void WiQueue_Init();
extern int WiQueue_GetMaxElements(int id);
extern void WiQue_DeleteElement(int id, int msg0, int msg1, int msg2, int msg3); // Chris

//KEY
//(WiMainQ, MSG_BASE_KEY, key, NULL, NULL);
#define MSG_BASE_KEY						0x01000000

//IRR
//(WiMainQ, MSG_BASE_REMOCON, irKeyCode, NULL, NULL);
//(WiMainQ, MSG_BASE_REMOCON, irKeyCode | MSG_MASK_KEY_REPEAT, NULL, NULL);
#define MSG_BASE_REMOCON					0x02000000

#define MSG_BASE_CI							0x08000000

//msg[1] bits mask...
#define MSG_MASK_KEY_REPEAT					0x00010000
#define MSG_MASK_KEY_CODE					0x000000ff

//(WiMainQ, MSG_BASE_TUNER | Channel_CommandId, 0, NULL); // LOCK
//(WiMainQ, MSG_BASE_TUNER | Channel_CommandId, 1, NULL); // UNLOCK
#define MSG_BASE_TUNER						0x03000000

//LOADING FAIL or TIMEOUT...
//(qTarget, MSG_BASE_FILTER | evtCode, NULL, NULL, NULL); // loading FAIL
//LOADING OK...
//(qTarget, MSG_BASE_FILTER | evtCode, dscr, nSec, nLoadedSec); // loading OK
#define MSG_BASE_FILTER						0x04000000

//WiQueue_Tx4(qTarget, MSG_BASE_TIMER | evtCode, NULL, NULL);
#define MSG_BASE_TIMER						0x0c000000

//base mask value
#define MSG_BASE_MASK						0xff000000

//=======================================================================

#define MSG_BASE_APPL						0xff000000

typedef enum
{
// SI Message
	APPLMSG_SI_BASE 				= MSG_BASE_APPL | 0x00110000,

	// msg[1] = pid, msg[2] = tuner
	APPLMSG_SI_PAT				= APPLMSG_SI_BASE | 0x00001000,
	APPLMSG_SI_PMT				= APPLMSG_SI_BASE | 0x00002000,
	APPLMSG_SI_NIT				= APPLMSG_SI_BASE | 0x00003000,
	APPLMSG_SI_NIT_Other			= APPLMSG_SI_BASE | 0x00003001,
	APPLMSG_SI_SDT				= APPLMSG_SI_BASE | 0x00004000,
	APPLMSG_SI_SDT_Other			= APPLMSG_SI_BASE | 0x00004001,
	APPLMSG_SI_SDT_Repeat			= APPLMSG_SI_BASE | 0x00004002,
	APPLMSG_SI_TDT				= APPLMSG_SI_BASE | 0x00005000,
	APPLMSG_SI_TOT				= APPLMSG_SI_BASE | 0x00005001,
	APPLMSG_SI_BAT				= APPLMSG_SI_BASE | 0x00006000,
	APPLMSG_SI_CAT				= APPLMSG_SI_BASE | 0x00007000,

	APPLMSG_SI_ETC_BASE 			= MSG_BASE_APPL | 0x00120000,

	APPLMSG_SI_SDT_AllLoaded		= APPLMSG_SI_ETC_BASE | 0x00002001,
	APPLMSG_SI_SDT_CurrentLoaded	= APPLMSG_SI_ETC_BASE | 0x00002002,

	APPLMSG_SI_CUR_PMT			= APPLMSG_SI_ETC_BASE | 0x0000F000,
	APPLMSG_SI_ALL_PMT			= APPLMSG_SI_ETC_BASE | 0x0000F001,

// TUNER Message
	APPLMSG_TUNER_BASE			= MSG_BASE_APPL | 0x00200000,

// Positioner
	APPLMSG_MSG_BASE				= MSG_BASE_APPL | 0x00400000,
	APPLMSG_Positioner_Move 		= APPLMSG_MSG_BASE |0x00010000,
	APPLMSG_Positioner_Move_End 	= APPLMSG_MSG_BASE |0x00020000,

// Epg
	APPLMSG_EPG_BASE				= MSG_BASE_APPL | 0x00600000,
	APPLMSG_EPG_Data_Ack			= APPLMSG_EPG_BASE |0x00000200,		// msg[1]- (satIdx<<16 | orgNetId), msg[2]- (tsId<<16 | svcId)

// Ack ( response )
	APPLMSG_ACK_BASE		= MSG_BASE_APPL | 0x00700000,

	APPLMSG_DNG_ACK		= APPLMSG_ACK_BASE | 0x000B0000,
	APPLMSG_DNG_MSG		= APPLMSG_DNG_ACK | 0x00000400,

	APPLMSG_EData			= APPLMSG_DNG_ACK | 0x00000500,
	APPLMSG_EDataAck		= APPLMSG_DNG_ACK | 0x00000501,

	APPLMSG_CS_MSG		= APPLMSG_DNG_ACK | 0x00000700,

// Svc
	APPLMSG_SVC_BASE			= MSG_BASE_APPL | 0x00800000,
	APPLMSG_SVC_Start			= APPLMSG_SVC_BASE | 0x00010000,	// APPLMSG_SVC_Start, svcType, mainSub, svcNum
	APPLMSG_SVC_NewSvcStart 	= APPLMSG_SVC_BASE | 0x00010001,	// APPLMSG_SVC_Start, svcType, mainSub, svcNum
	APPLMSG_SVC_Stop			= APPLMSG_SVC_BASE | 0x00020000,	// APPLMSG_SVC_Stop, mainSub, mainSub
	APPLMSG_SVC_AudioChange 	= APPLMSG_SVC_BASE | 0x00030000,	// APPLMSG_SVC_Start, svcType, mainSub, svcNum

	APPLMSG_Streaming			= APPLMSG_SVC_BASE | 0x00070000,	// T_StreamingErr

// Alarm
	APPLMSG_ALARM_BASE			= MSG_BASE_APPL | 0x00A00000,
	APPLMSG_ALARM_DB_Changed	= APPLMSG_ALARM_BASE |0x00040000,	// param0 - alarmId, if param0 is ALARM_NUM - PCMD_GetAlarm ack
	APPLMSG_ALARM_Set_Fail		= APPLMSG_ALARM_BASE |0x00040001,	// param0 - alarmId, param1 - T_AlarmError, param2 - collision slot

	APPLMSG_ALARM_Set				= APPLMSG_ALARM_BASE |0x00040010,	// Don't use directly. use SetAlarmPtrAndWaitAnswer function. param0 - alarmId, param1 - T_SetAlarmError, param2~ collision slots

//Smart Card and CAS
	APPLMSG_SC_BASE			=  MSG_BASE_APPL | 0x00B00000,
	APPLMSG_SC_INSERTED		=  APPLMSG_SC_BASE |0x00010000,
	APPLMSG_SC_INITIALIZING	=  APPLMSG_SC_BASE |0x00020000,
	APPLMSG_SC_EXTRACTED		=  APPLMSG_SC_BASE |0x00030000,
	APPLMSG_SC_EXTRACTED_CAMENU	=  APPLMSG_SC_BASE |0x00030001,

	APPLMSG_SC_INITIALIZED	=  APPLMSG_SC_BASE |0x00040000,
	APPLMSG_SC_INVALID		=  APPLMSG_SC_BASE |0x00050000,
	APPLMSG_SC_CMDERROR		=  APPLMSG_SC_BASE |0x00050001,
	APPLMSG_SC_REQPIN			=  APPLMSG_SC_BASE |0x00060000,
	APPLMSG_SC_REQMMI		=  APPLMSG_SC_BASE |0x00070000,
	APPLMSG_SC_INOUT			=  APPLMSG_SC_BASE |0x00080000,
	APPLMSG_SC_TPSKEY			= APPLMSG_SC_BASE | 0x00090000,
	APPLMSG_SC_CARDTYPE		= APPLMSG_SC_BASE | 0x000A0000,
	APPLMSG_SC_REINITIALIZED	=  APPLMSG_SC_BASE |0x000B0000,

// CI
	APPLMSG_CI_BASE			= MSG_BASE_APPL | 0x00C00000,
	APPLMSG_CI_Inserted		= APPLMSG_CI_BASE | 0x00010000,
	APPLMSG_CI_Extracted		= APPLMSG_CI_BASE | 0x00010001,
	APPLMSG_CI_AppInfo		= APPLMSG_CI_BASE | 0x00010002,
	APPLMSG_CI_CaInfo			= APPLMSG_CI_BASE | 0x00010003,
	APPLMSG_CI_DateTime		= APPLMSG_CI_BASE | 0x00010004,
	APPLMSG_CI_CloseMmi		= APPLMSG_CI_BASE | 0x00010005,
	APPLMSG_CI_Enq			= APPLMSG_CI_BASE | 0x00010006,
	APPLMSG_CI_Menu			= APPLMSG_CI_BASE | 0x00010007,
	APPLMSG_CI_List			= APPLMSG_CI_BASE | 0x00010008,
	APPLMSG_CI_CaPmtReply		= APPLMSG_CI_BASE | 0x00010009,
	APPLMSG_CI_InputEnd		= APPLMSG_CI_BASE | 0x0001000A,

	APPLMSG_CI_PLUS_Support		= APPLMSG_CI_BASE | 0x00020001,
	APPLMSG_CI_PLUS_APDU			= APPLMSG_CI_BASE | 0x00020002,
	APPLMSG_CI_PLUS_APDU_BACK	= APPLMSG_CI_BASE | 0x00020003,
	APPLMSG_CI_PLUS_HOST_SET		= APPLMSG_CI_BASE | 0x00020004,
	APPLMSG_CI_PLUS_COPYCONTROL	= APPLMSG_CI_BASE | 0x00020005,

// PVR
	APPLMSG_PVR_BASE 		= MSG_BASE_APPL | 0x00D00000,
	APPLMSG_REC_START 	= APPLMSG_PVR_BASE |0x00020001,
	APPLMSG_REC_STOP  	= APPLMSG_PVR_BASE |0x00020002,
	APPLMSG_PLAY_STATE = APPLMSG_PVR_BASE |0x00010000,

// Etc
	APPLMSG_ETC_BASE	= MSG_BASE_APPL | 0x00F00000,
	APPLMSG_FLASHWRITE_OK		= APPLMSG_ETC_BASE | 0x00020000,
	APPLMSG_FLASHFILE_WRITE_OK	= APPLMSG_ETC_BASE | 0x00020001,
	APPLMSG_FLASHWRITE_FAIL 		= APPLMSG_ETC_BASE | 0x00020002,
	APPLMSG_EEPROM_WRITE_OK 	= APPLMSG_ETC_BASE | 0x00020003,
	APPLMSG_FLASHUSER1_WRITE_OK = APPLMSG_ETC_BASE | 0x00020004,

	APPLMSG_REMOCON 				= APPLMSG_ETC_BASE | 0x00030000,
	APPLMSG_REMOCON_RELEASE		= APPLMSG_REMOCON | 0x0000f000,

	APPLMSG_RF_OMT 				= APPLMSG_ETC_BASE | 0x00030002,

	APPLMSG_Destroy				= APPLMSG_ETC_BASE | 0x000c0001, // auto delete mode delete.
	APPLMSG_ReDraw				= APPLMSG_ETC_BASE | 0x000c0002, // Redraw Menu.
	APPLMSG_OpenFirmware			= APPLMSG_ETC_BASE | 0x000c0003, // Firmware Box open. for mainmenu. or need delay menu.
	APPLMSG_OpenOTAPrompt		= APPLMSG_ETC_BASE | 0x000c0004, // OTA Prompt Box open. for OTA Software Update menu.
	APPLMSG_DestroyAllStateAndSaveSubState	= APPLMSG_ETC_BASE | 0x000c0005, // destroy all menu state. msg[1]- reserved sub state after all destroy.

	APPLMSG_PluginSelectedCmd 		= APPLMSG_ETC_BASE | 0x000e0011,

	APPLMSG_PluginReloadCmd 		= APPLMSG_ETC_BASE | 0x000e0012,
	APPLMSG_PluginStart 			= APPLMSG_ETC_BASE | 0x000e0013,
	APPLMSG_PluginStop 			= APPLMSG_ETC_BASE | 0x000e0014,
	APPLMSG_PluginType				= APPLMSG_ETC_BASE | 0x000e0015,

} T_Appl_Msg;

#define	APPLMSG_BASE_MASK		( MSG_BASE_APPL | 0x00f00000 )

typedef enum
{
	PLUGINMSG_StateChanged	= 	APPLMSG_ETC_BASE | 0x0000FF01,
	PLUGINMSG_SVC_Starting,
	PLUGINMSG_WavDone,
	PLUGINMSG_StateChangeStart,
	PLUGINMSG_DisplayChanged,
	PLUGINMSG_NU2,

	PLUGINMSG_FilterStarted,
	PLUGINMSG_FilterLoaded,	/* from appl	: PLUGINMSG_FilterLoaded, evtCode, 0, size, [data]
									to MainQ	: PLUGINMSG_FilterLoaded, evtCode, size, [data] */
	PLUGINMSG_FilterTimeout,

	PLUGINMSG_SvcState,		// PCMD_GetSvcState ack : 1-[0- error, 1- ok], 2-[handle]
	PLUGINMSG_BoxInfo,			// Internal use only. PCMD_BoxInfo ack

} T_PluginMsg;

#define	APPLMSG_SI_General(tuner)	 (APPLMSG_SI_BASE | 0x00000000 | (tuner) )

// TUNER Message
#define	APPLMSG_TUNER_LOCK( tuner ) ( APPLMSG_TUNER_BASE | tuner )
#define	APPLMSG_TUNER_UNLOCK( tuner ) ( APPLMSG_TUNER_BASE | 0x00010000 | tuner )
#define	APPLMSG_TUNER_SAME( tuner ) ( APPLMSG_TUNER_BASE | 0x00020000 | tuner )

//=======================================================================

typedef enum { Ratio_4_3, Ratio_16_9 } _screen_ratio_;

typedef enum {
	Resol_480i,		// not supported
	Resol_480p,		// not supported
	Resol_576i,
	Resol_576p,
	Resol_720p,
	Resol_1080i,
	Resol_1080p,	// after 7105
	Resol_Auto
} _display_resolution_;

typedef enum {
	Format_LetterBox,
	Format_Full = Format_LetterBox,
	Format_PanScan,
	Format_PillarBox = Format_PanScan,
} _display_format_;

typedef enum {
	 			 CONFIG_32BitService
				,CFG_LASTSVCTYPE			= CONFIG_32BitService		// Main : TV(0), Radio(1)
				,CFG_SUB_LASTSVCTYPE		// Sub : TV(0), Radio(1)
				,CFG_LASTLIST				// SatIdx(0 ~ N_Sat - 1), FavGroup(N_Sat ~ N_Sat + _nFav - 1), AllSat( (word)-1 )
				,CFG_SUB_LASTLIST			// SatIdx(0 ~ N_Sat - 1), FavGroup(N_Sat ~ N_Sat + _nFav - 1), AllSat( (word)-1 )
				,CFG_LASTTVSVC				// All Sat : Last tv channel - slot no of _tvSvc
				,CFG_LASTRADIOSVC			// All Sat : Last radio channel - slot no of _radioSvc -5th

				,CONFIG_32BitSystem
				,CFG_FIRSTBOOT				= CONFIG_32BitSystem		// First(0) or Not(1) - 6th
				,CFG_INSTALLLOCK			// UnLock(0), Lock(1)
				,CFG_EDITSVCLOCK
				,CFG_CONFIGLOCK
				,CFG_GAMELOCK
				,CFG_GMTUSE				// T_TimeSetMode
				,CFG_SUMMERTIME
				,CFG_SvcListPreview
				,CFG_MenuSound
				,CFG_QUICKCHSEARCH
				,CFG_KeepLastViewingSvc
				,CFG_VIDENC				// N_VideoEnc
				,CFG_RATIO					// _aspect_ratio_
				,CFG_SCREEN				// _video_format_
				,CFG_VIDOUT
				,CFG_COLORMODE
				,CFG_RFTYPE
				,CFG_SOUNDMODE			// _audio_output_type_
				,CFG_VIDENCCFG				// N_VideoEnc
				,CFG_LASTFILTER			// None(0), Name(1), Tp(2).. and so on.
				,CFG_MENULANG				// OSD menu language
				,CFG_AUDLANG				// Audio language
				,CFG_AUDLANG2				// Audio2 language
				,CFG_SUBTLANG				// SubTitle language
				,CFG_SUBTLANG2			// SubTitle2 language - 30th
				,CFG_RFCHANNEL			// 0(21) ~ 48(69)
				,CFG_BRIGHTNESS
				,CFG_CONTRAST
				,CFG_SATURATION
				,CFG_VOLUME				// Volume(0 ~ MAX_VOLUME - 1)
				,CFG_VOLTIMEOUT
				,CFG_INFOTIMEOUT
				,CFG_TIMEZONE				// -12:00(0) ~ +12:00
				,CFG_INFOPOS
				,CFG_LOCALOFFSET			// 0 or 1. Ahead or Behind form GMT - 40th
				,CFG_PINCODE
				,CFG_OSDCOLOR				// 0 ~ 0xFFFF
				,CFG_COUNTRYCODE			// (T_ISO3166)-8 bits * 4
				,CFG_REGIONID				// (RegionID)-6 bits * 5
				,CFG_CLOCKMODE			// HOUR12, HOUR24
				,CFG_TTXLANG
				,CFG_PARENTALRATE
				,CFG_OSDTRANSPARENCY
				,CFG_LASTFILTERROW		// 0 or not
				,CFG_UserFont
				,CFG_UserFontSize
				,CFG_SORTBYNUM
				,CFG_BEEPWITHSGNAL		// Beep with signal, beep(1) no(0)
				,CFG_UNLOCKLOCKCHAFTERPIN // Unlock, lock ch apter PIN, lock(1) no(0)
				,CFG_NetMenuDefault			// 0 : eth, 1 : wifi
				,CFG_AUTOCHUPDATE		// Auto ch/tp update, update(1) no(0)
				,CFG_NotUsed1
				,CFG_NotUsed2
				,CFG_NotUsed3
				,CFG_PLAYNUMERIC				// 1 : percentage move, 0 : commercial skip
				,CFG_SvcListFavLock
				,CFG_FavGroupLock
				,CFG_ViewOnlyFilteredList
				,CFG_CloseCaption
				,CFG_Logitude
				,CFG_Latitude
				,CFG_IpAddrWifi
				,CFG_TimeShift
				,CFG_TogglePositionerDirection	//
				,CFG_SliderTime					// Slider show time, "1, 5, 10, 20, 30- 70th
				,CFG_SliderContinue				// onece(0), continue(1)
				,CFG_RcvLock
				,CFG_TimeShift_Device			// NAND(0), USB External(1)
				,CFG_AutoChChange
				,CFG_ForceBackup
				,CFG_Dns1Wifi
				,CFG_Dns2Wifi
				,CFG_CiMessage
				,CFG_NetMaskWifi
				,CFG_GatewayWifi
				,CFG_SSUReservedTime
				,CFG_SSUPrompt
				,CFG_SSUTime
				,CFG_USBDetect
				,CFG_TimeShiftMsg
				,CFG_RecordStopMsg
				,CFG_OsdType
				,CFG_EpgTimeRange
				,CFG_ZappingMode
				,CFG_Active5v
				,CFG_ExtraRecording
				,CFG_MP3Order
				,CFG_SlideSubtitle
				,CFG_SlideDownScale
				,CFG_AC3Default
				,CFG_EPGLANG 		 // EPG language
				,CFG_VfdScroll
				,CFG_Sw12v
				,CFG_NotUsed4			//
				,CFG_NotUsed5					// 100th
				,CFG_ResolDesire
				,CFG_ResolCfg
				,CFG_TunerConnect
				,CFG_RecDevice
				,CFG_Dhcp
				,CFG_IpAddr
				,CFG_NetMask
				,CFG_Gateway
				,CFG_Dns1
				,CFG_Dns2						// 110th
				,CFG_TunerSort		// '0' is All tuner.
				,CFG_PreviewSpeed
				,CFG_LCNOnOff
				,CFG_LEDTime
				,CFG_LEDBright
				,CFG_EpgListType
				,CFG_HdGrouping
				,CFG_InsideMapping
				,CFG_DDnsAutoStart
				,CFG_PlayfileListSort				// 120th
				,CFG_StandbyPower
				,CFG_HDMIAVOUT
				,CFG_PlaybackPosJump
				,CFG_AlarmConfirmMsg
				,CFG_BlindOption
				,CFG_StandbyHddUsb
				,CFG_TimeshiftStart
				,CFG_PluginConfig
				,CFG_MrCode
				,CFG_INFOEventType
				,CFG_AudioDelay
				,CFG_PluginLock
				,CFG_YOUTUBE_LOCATION
				,CFG_AUTOUPDATEVER
				,CFG_SubtOsdDisable

				,CONFIG_Max
} T_Config;

extern int GetConfig( int cfgIdx );

//=======================================================================

typedef enum
{
	PtData_ReservedStart	= 0xFD257000,
	PtData_ReservedEnd 	= 0xFD257040,

	PtData_Max				= 0xFD257080,
} T_PtDataId;

typedef struct
{
	dword val[8];	// '0' - id, '1' - reserved, '2' ~ '6' - data, '7' - crc('2' ~ '6')
} S_PtGetData;

extern bool GetProtData(T_PtDataId id, S_PtGetData *data);

typedef struct
{
	dword val[5];
} S_PtSetData;

extern void SetProtData(T_PtDataId id, dword idkey, S_PtSetData data);		// idkey - same T_PtDataId exception control.

typedef struct
{
	char		baseModelName[32];
	char		modelName[64];
	char		applVerStr[16];		// %d.%x%x.%x%x - (ver>>16)&0x0f, (ver>>12)&0x0f, (ver>>8)&0x0f, (ver>>4)&0x0f, (ver)&0x0f
	char		apiVerStr[16];		// __DATE__
	byte		tunerCnt;
	byte		usbCnt;
	byte		res1;
	byte		res2;
	dword	systemId;
	byte		eth0[6];
	dword	firmwareId;
	dword	chipId_h;			// H205 series only
	dword	chipId_l;			// H205 series only
} S_BoxInfo;

extern S_BoxInfo * GetBoxInfo();	// if fail return NULL.
extern void GetDeviceInfo(int device, int *total, int *used, int *free);

//=======================================================================

typedef enum
{
	STATE_None,
	STATE_Standby,
	STATE_Logo,
	STATE_RcvLock,

	STATE_Normal,
	STATE_SvcList,
	STATE_MainMenu,
	// Installation
	STATE_DishMenu,
	STATE_PosMenu,
	STATE_ScanMenu,
	STATE_DVBCScanMenu,
	STATE_DVBTScanMenu,
	STATE_OptionMenu,
	STATE_Restoration,
	STATE_DataTransfer,
	STATE_SysInfoMenu,

	// Configuration
	STATE_TimeSetMenu,
	STATE_TimerMgr,
	STATE_UiMenu,
	STATE_ParentalMenu,
	STATE_LangMenu,
	STATE_AvOutputMenu,

	// Edit Service
	STATE_SvcListMgr,
	STATE_SvcListOption,

	STATE_Epg,
	STATE_Teletext,
	STATE_Mosaic,

	STATE_Scanning,
	STATE_DiagMenu,

	STATE_Usb,
	STATE_MP3Player,
	STATE_JpgViewer,

	STATE_Calendar,
	STATE_Calculator,

	STATE_RecMenu,
	STATE_CasMenu,
	STATE_StorageMenu,

	STATE_GameMenu,
	STATE_CakeHexa,
	STATE_Tetris,

	STATE_FreeTV,

	STATE_KeyMgr,
	STATE_LangInstall,
	STATE_TimeInstall,
	STATE_UHF,

	STATE_Subtitle,

	STATE_FilePlayList,

	STATE_Cas,
	STATE_CiMenu,
	STATE_CiMenuWin,

	STATE_OTAMenu,

	STATE_NetSetMenu,

	STATE_TdtGet,
	STATE_PlugInMenu,

	STATE_PluginWebBrowser,
	STATE_StorageCheck,

	STATE_PluginStandAlone,
	STATE_SsuGet,
	STATE_SSUMenu,

	STATE_NVOD,
	STATE_MaturityRateMenu,

	STATE_ShortCutMenu,
	STATE_InternetTV,
	STATE_InternetRadio,
	STATE_WeatherForecast,
	STATE_FstUpdate,
	STATE_Rss,
	STATE_SyncPlayer,
	STATE_ReadyStandby,

	STATE_ProdcutListMenu,
	STATE_KartinaTV,
	STATE_NetInstall,
	STATE_SvcAddonInstall,
	STATE_AMyTV,
	STATE_Portal,
	STATE_FstInstall,
	STATE_QuranicAudio,

	STATE_SpotifyLogin,
	STATE_SpotifyMainMenu,
	STATE_SpotifyPlayListMenu,
	STATE_SpotifyTrackListMenu,
	STATE_SpotifyPlayerMenu,

	STATE_PortalByWebzeal,


	SUBSTATE_IBox 						= 512,
	SUBSTATE_Volume,
	SUBSTATE_MainMenu,
	SUBSTATE_ScanMenu,
	SUBSTATE_DVBCScanMenu,
	SUBSTATE_DVBTScanMenu,
	SUBSTATE_Scan,
	SUBSTATE_SvcListMgr,
	SUBSTATE_Mp3,
	SUBSTATE_Image,
	SUBSTATE_SvcListIndex,
	SUBSTATE_SvcListOpt,
	SUBSTATE_ExpressEpg,
	SUBSTATE_NotUsed,
	SUBSTATE_SvcListSetFav,
	SUBSTATE_SvcFind,
	SUBSTATE_PopList,
	SUBSTATE_LineEdit,
	SUBSTATE_SvcSelect,
	SUBSTATE_TimerEdit,
	SUBSTATE_Keyboard,
	SUBSTATE_FavGroupList,
	SUBSTATE_SatGroupList,
	SUBSTATE_RecallList,
	SUBSTATE_Calendar,
	SUBSTATE_LocalArea,
	SUBSTATE_ProgressBox,
	SUBSTATE_MsgBox,
	SUBSTATE_SWUpgrade,
	SUBSTATE_NetDrvMount,
	SUBSTATE_Calculator,

	SUBSTATE_ConaxSubscriptorBox,
	SUBSTATE_ConaxTokenBox,
	SUBSTATE_ConaxEventBox,
	SUBSTATE_ConaxCaPinBox,
	SUBSTATE_ConaxSignPinBox,
	SUBSTATE_ConaxMaturityBox,
	SUBSTATE_ConaxAboutBox,

	SUBSTATE_PlayBar,
	SUBSTATE_RecordEdit,

	SUBSTATE_AudioControlTrack,
	SUBSTATE_SWDownloading,
	SUBSTATE_SatAutoDetect,
	SUBSTATE_FactoryReset,
	SUBSTATE_TpEdit,
	SUBSTATE_SlideOption,
	SUBSTATE_ExtendedSignal,
	SUBSTATE_DiSEqC,
	SUBSTATE_MenuWin,
	SUBSTATE_PosWin,
	SUBSTATE_Serialmenu,
	SUBSTATE_EpgSearch,
	SUBSTATE_Category,
	SUBSTATE_SvcEdit,
	SUBSTATE_SsuPrompt,

	SUBSTATE_SaTCRRecognition,
	SUBSTATE_SubSvcList,

	SUBSTATE_DynDNS,
	SUBSTATE_PluginPop,

	SUBSTATE_SetAudioDelay,
	SUBSTATE_WebPopMenu,

	SUBSTATE_IrdetoAppl,
	SUBSTATE_IRPPVBox,

	SUBSTATE_PluginDownload,
	SUBSTATE_TextWin,
	SUBSTATE_IconWin,

	SUBSTATE_3gModem,
	SUBSTATE_EpgOption,
	SUBSTATE_MediaServerWin,
	SUBSTATE_Mp3List,
	SUBSTATE_Mp3Browser,
	SUBSTATE_URLSelect,
	SUBSTATE_URLAlarmPlay,
	SUBSTATE_PeopleMeter,
	SUBSTATE_Pincode,
	SUBSTATE_DataTransferWin,
	SUBSTATE_Mheg,

	N_State,
	SUBSTATE_None = STATE_None,
} T_ApplState;

#define SUBSTATE_Offset	1000

//=======================================================================

extern void GetTunerState(int tuner, int *isLock, int *strength, int *quality);

//=======================================================================

typedef enum
{
	STATEIDX_Main,
	STATEIDX_Sub,
	N_StateIdx
} T_StateIdx;

extern int GetState(T_StateIdx stateIdx);

extern void SaveState(T_StateIdx stateIdx, T_ApplState state);
extern void RestoreState(T_StateIdx stateIdx);

//=======================================================================

extern S_Service * GetCurService( int mainSub );
extern int GetSvcCnt( int svcType );
extern int GetSvcIndexCnt( int svcType );
extern int GetSvcIdxByIndex( int svcType, int index );	// error- return (-1)
extern int GetSvcIdxById( int svcType, word orgNetId, word tsId, word svcId, int *index=NULL );	// error- return (-1)
extern S_Service * GetSvcById( int svcType, word orgNetId, word tsId, word svcId );
extern S_Service * GetSvc( byte tvRadio, word svcIdx );
extern word GetSvcId( byte tvRadio, word svcIdx );
extern word GetSvcPmtPid( byte tvRadio, word svcIdx );
extern word GetSvcOrgNetId( byte tvRadio, word svcIdx );
extern word GetSvcTsId( byte tvRadio, word svcIdx );
extern word GetSvcTpIdx( byte tvRadio, word svcIdx );
extern word GetSvcSatIdx( byte tvRadio, word svcIdx );
extern word GetSvcSatIdx( S_Service *svc );
extern byte * GetSvcName( byte tvRadio, word svcIdx, bool shortName );
extern byte * GetSvcName( S_Service *svc, bool shortName );

extern S_ServiceInfo * GetCurServiceInfo( int mainSub );
extern S_ServiceInfo * GetServiceInfo( S_Service *svc );
extern S_ServiceInfo * GetServiceInfo( int svcType, int svcNum );

extern bool IsServiceStarted( int svcHandle );

extern S_SiCat * GetCatData( int svcHandle );
extern S_SiPmt * GetPmtData( int svcHandle );
extern S_AudioInfo * GetAudioInfo( S_ServiceInfo *svcInfo );
extern S_AudioInfo * GetAudioInfo( int svcHandle );
extern S_SubtInfo * GetSubtInfo( S_ServiceInfo *svcInfo );
extern S_SubtInfo * GetSubtInfo( int svcHandle );

extern int GetCurSvcType( int svcHandle );
extern int GetCurSvcTuner( int svcHandle );
extern int GetCurSvcNum( int svcHandle );
extern int GetCurSvcDmxChannel( int svcHandle );
extern void LastSvcInfo( byte *type, word *satIdx, word *svcIdx );

typedef enum
{
	RECMODE_None,
	RECMODE_Recording,
	RECMODE_TempRecording,
	RECMODE_CopyRecording,
	RECMODE_KeyWaiting
} T_RecMode;
extern void GetRecSvcInfo(byte svcType, word svcIdx, int *duration, int *recTime, int *device, int *recMode);

//=======================================================================

extern S_Tp * GetTp( word tpIdx );
extern S_Tp * GetTp( int satIdx, word orgNetId, word tsId );
extern S_Tp * GetTp( int svcHandle );
extern S_Tp * GetTp( S_Service *service );
extern int GetTpIdx( int satIdx, word orgNetId, word tsId );
extern char * GetTpName(word tpIdx );
extern dword GetTpFreq( word tpIdx );
extern word GetTpSr( word tpIdx );
extern word GetTpPolar( word tpIdx );
extern word GetTpOrgNetId( word tpIdx );
extern word GetTpTsId( word tpIdx );
extern word GetTpSatIdx( word tpIdx );
extern bool IsSameTp( const S_Tp &tp1, const S_Tp &tp2 );
extern bool IsSameTp( const S_Tp *tp1, const S_Tp *tp2 );
extern S_Tp * FindTpInSat( word satIdx, dword freq, byte polar );
extern int FindTpIdx( word satIdx, dword freq, dword sr, byte polar );
extern word GetTpNumInSat( word satIdx );

//=======================================================================

extern short GetSatAngle( word satIdx );
extern byte * GetSatName( word satIdx );

//=======================================================================

extern char * GetFavGroupName( int listIdx );
extern bool IsFavGroup( int listIdx );
extern bool IsLockFavGroup( int listIdx );
extern int GetFavGroupCnt();

//=======================================================================

extern byte * GetNetworkName( int tpIdx );
extern byte * GetNetworkName( S_Tp *tp , bool real );
extern byte * GetNetworkName( S_Service *svc , bool real );

//=======================================================================

// --- EPG -------------------------------------

extern bool GetEventInfo(S_Service *svc);				// Call ReadEventData(...) after receive APPLMSG_EPG_Data_Ack
extern bool GetEventInfo(int svcType, int svcNum);		// Call ReadEventData(...) after receive APPLMSG_EPG_Data_Ack
extern bool ReadEventData(int satIdx, int orgNetId, int tsId, int svcId, int *eventCount, S_EventInfo **eventInfo);
extern bool ReadEventData(int svcType, int svcNum , int *eventCount, S_EventInfo **eventInfo);
extern bool ReadEventData(S_Service *svc, int *eventCount, S_EventInfo **eventInfo);

extern bool RemoveEventData(int satIdx, int orgNetId, int tsId, int svcId);
extern char * GetEventName(S_EventInfo *evt);
extern char * GetShortText(S_EventInfo *evt);
extern char * GetExtendedText(S_EventInfo *evt);

extern bool GetEventDataNow(S_Service *svc, int *eventCount, S_EventInfo **eventInfo);
extern bool GetEventDataNow(int svcType, int svcNum, int *eventCount, S_EventInfo **eventInfo);

// --- Alarm -------------------------------------

extern int FindMatchAlarmSlot(word evtId, word svcId, word tsId, word orgNetId); // return: alarmId, fail: -1
extern S_Alarm * GetAlarmPtr(int alarmId); // if (alarmId >= ALARM_NUM) || (alarmId == 0) return NULL

// if (alarmId >= ALARM_NUM) return. if (alarmId == 0) new alarm
extern void SetAlarmPtr(int alarmId, S_Alarm *alarm);		// to check response have to wait APPLMSG_ALARM_DB_Changed or APPLMSG_ALARM_Set_Fail message

// synchronous function . if (alarmId >= ALARM_NUM) return false. if (alarmId == 0) new alarm
extern bool SetAlarmPtrAndWaitAnswer(byte alarmId, S_Alarm *alarm, dword *msg);	// msg[0] = APPLMSG_ALARM_Set, msg[1] = alarmId, msg[2] = T_SetAlarmError, msg[3]~ collision slots.

extern char *GetAlarmEvtName( byte alarmId ); // if (alarmId >= ALARM_NUM) || (alarmId == 0) return NULL
extern char *GetSeriesName( byte seriesId ); // seriesId = 1 ~ 7

//=======================================================================

extern void Mjd2Date(int MJD, int *year, int *mon, int *day, int *dayOfWeek);
extern int  Date2Mjd(int year, int month, int day);
extern dword GetCurTime(int *hour=NULL, int *min=NULL, int *sec=NULL, int *mjd=NULL);
extern void strlwr(char *s);
extern dword Time2Local(dword time);		// mjd(16bit) + hour(8bit) + min(8bit)
extern dword Local2Time(dword time);		// mjd(16bit) + hour(8bit) + min(8bit)

//=======================================================================

extern void DmxStart(int pid, byte *value, byte *mask, int timeout, int mode);
extern void DescramblerKey(int svcHandle, int pid, byte *even, byte *odd);
extern void DescramblerClear(int svcHandle);

//=======================================================================

#define GetBitValue( value, bitMask )			( ( value & bitMask ) ? 1 : 0 )
#define SetBitValue( value, bitValue, TF ) 		( value = ( TF ? ( value | bitValue ) : ( value & ~bitValue ) ) )
#ifndef min
#define min(a,b) (((a)<(b))?(a):(b))
#endif
#ifndef max
#define max(a,b) (((a)>(b))?(a):(b))
#endif

#define countof( array ) ( sizeof( array )/sizeof( array[0] ) )

//=======================================================================

extern void WavStart();
extern void WavWrite(void *data, int size);
extern void WavStop();

//=======================================================================

dword ChangeRcuKeyFromFrontKey(dword frontKey);

#endif	// __PLUGIN_H__


