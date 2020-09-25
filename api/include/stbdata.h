#ifndef __STBDATA_H__
#define  __STBDATA_H__

#define SATELLITE
//#define TERRESTRIAL
//#define CABLE

#define MAX_SAT_NAME	32
#define N_Lnb	64
#define N_Sat	250

//#define N_Tv		7000
//#define N_Radio	3000
//#define N_Tp		8000

#define ALL_Sat	((word)-1)

#define LNB_Default_Ku	0xff
#define LNB_Default_C	0xfe
#define LNB_Default_DP	0xfd

#define MIN_SYMBOLRATE		2000
#define MAX_SYMBOLRATE		45000

#define MIN_FREQ			950
#define MAX_FREQ			2150

#define FREQ_CENTER		11700	// Universal

#define	MAX_AUDIO_NAME		16
#define	MAX_SERVICE_NAME		32
#define	MAX_NETWORK_NAME	32
#define	MAX_PROVIDER_NAME	32

typedef enum
{
	SVCTYPE_Tv,
	SVCTYPE_Radio,
} T_SvcType;

typedef enum
{
	LNB_Single,
	LNB_Universal,
	LNB_Unicable,
	N_LNB_Type,
} T_LnbType;

typedef enum {
	POSITIONER_Type_None,
	POSITIONER_Type_12,
	POSITIONER_Type_Usals,
	N_Positioner_Type
} T_PositionerType;

#ifdef CABLE
typedef enum {
	MODE_QAMAUTO,
	MODE_QAM16,
	MODE_QAM32,
	MODE_QAM64,
	MODE_QAM128,
	MODE_QAM256,
	N_QAMMODE
} T_QamMode;
#endif

#ifdef TERRESTRIAL
typedef enum {
	BW_6Mhz = 0,
	BW_7Mhz,
	BW_8Mhz,
	N_BWMODE
} T_BandWidth;

typedef enum {
	PR_Low,
	PR_High,
	N_PRIORITY
} T_Priority;
#endif

typedef enum{
	TunerCon_Separate,
	TunerCon_Loopthrough,
} T_TunerConnect;

typedef struct
{
	byte		type;
	byte		legacy;
	word	freq;

	word	lowFreq;
	word	highFreq;		// Single LNB : highFreq == 0

	byte		diseqc10;
	byte		diseqc10_2;
	byte		diseqc11;
	byte		positionerIdx;

	dword	lnbPower	: 1;
	dword	v12			: 1;
	dword	kh22		: 1;
	dword	positioner	: 2;	// POSITIONER_Type_None, POSITIONER_Type_12, POSITIONER_Type__Usals
	dword	used		: 1;
	dword	saTCR_pos	: 4;
	dword	saTCR_ub	: 4;
	dword	dodecaSlot	: 8;
	dword	resBit:10;
} S_Lnb;

typedef struct
{
	byte		name[MAX_SAT_NAME];
	short	angle;
	word	position;
	word	satNum;
	byte		lnbIdx;
	byte		lnbIdx1;
} S_Satellite;

typedef struct
{
	dword		freq;	// band width for terrestrial

	word		sr;
	word		orgNetId;
	word		bouquetId;
	word		tsId;
	word		satIdx;
#if defined( SATELLITE ) && defined( TERRESTRIAL ) && defined( CABLE ) //SLOT_TYPE_TUNER
	word		polar : 4;
	word		used : 2;
	word		res:2;
	word		xorTs : 1;
	word		chNum : 7;
#elif defined( SATELLITE ) && defined( TERRESTRIAL )
	word		polar : 2;
	word		used : 2;
	word		echoEit : 1;
	word		bellEit : 2;
	word		res	: 1;
	word		xorTs : 1;
	word		chNum : 7;
#elif defined( SATELLITE )
	word		polar : 2;
	word		used : 2;
	word		echoEit : 1;
	word		bellEit : 2;
	word		xorTs : 1;		// spain 특정 tp에서 ts가 xor되어 들어옴.
	word		res	: 8;
#elif defined( CABLE ) && defined( TERRESTRIAL )
	word		used : 2;
	word		polar: 4;
	word		chNum: 7;
	word		reservedBit:3;
#elif defined( CABLE )
	word		used : 2;
	// satellite의 polar는 cable에서는 qam mode로 사용.
	word		polar : 4;
	word		res : 10;
#elif defined( TERRESTRIAL )
	word		used : 2;
	// satellite의 polar는 terrestrial에서는 priority로 사용.
	word		polar : 2;
	word		res : 3;
	word		chNum : 7;
#endif
	word		nameIndex;
	dword		curCaSystem;	// 부팅후 佯桓?채널 전환을 위한것이므로 굳이 Flash 에 저장할 필요는 없음.

	dword		gmt;
	dword		gmtTick;
	word		timeCalibrated : 2;	// 0 = Not calibrated, 1 = Valid time, 2 = Invalid time
	word		otaTp : 2;
	word		resBits : 12;

	// 스캔 시 필요한 parameter들로 flash에 저장할 필요는 없음.
	word		transmission : 4;
	word		fec : 4;
	word		modulation : 4;
	word		pilot : 4;

	word		networkId;

} S_Tp;

typedef struct
{
	word	svcId;
	word	logicalChNum;

	word	pmtPid;
	word	pcrPid;

	word	videoPid;
	word 	isAc3		: 1;
	word	audioAuto	: 1;
	word	audioRes	: 1;
	word	audioPid	: 13;

	word	nameIndex;
	word	tpIdx;

	dword	favIdx;
	dword	caSystem;		// Logical CA_xxx
	dword	curCaSystem;

	dword	resbits2		: 2;
	dword	isSkip		: 1;
	dword	isCas		: 1;
	dword	isLock		: 1;
	dword	isRenamed	: 1;
	dword	isUsedLcn	: 1;	// for the Dr.Edit
	dword	isHd		: 1;
	dword	soundMode	: 3;
	dword	soundLevel	: 2;
	dword	markBit		: 1;
	dword	delCheck 	: 1;
	dword	subtPid		: 13;
	/* [ only used multifeed */
	dword	multifeed	: 1;
	dword	multifeedIdx : 3;
	/* ] */

	byte		videoStreamType;
	byte		audioStreamType;
	word	resbits4		: 4;
	word	tunerNum	: 4;
	word	resbits8		: 8;

	dword	soid;			// Just for temporary use, Do Not Write To FLASH!!
	dword	resbits32;

	word	resbits16;
	word	tempChNum;		// 이 부분은 LCN과 Svc Index의 Option 화를 위하여 사용한다.

	dword	cwKeyIndex;

	dword	epgChAddr;
} S_Service;

// SI  -----------------------------------

#define	MAX_SVC_INFO		96	// _s_dataSizeInfo[DataId_MaxSvcInfo]
#define	MAX_SCAN_SVC		256	// _s_dataSizeInfo[DataId_MaxScanSvc]
#define	MAX_NIT_TP			128 // _s_dataSizeInfo[DataId_MaxNitTp] // 64

typedef byte *T_SiDescriptor;
typedef byte *T_SecData;

typedef struct
{
	byte		streamType;
	word	pid;
	word	numOfDescriptor;
	T_SiDescriptor	*descriptor;
} S_SiPmtEs;

typedef struct
{
	word 	pid;
	word	pageNum;
	word 	compoPageId;
	word	ancilPageId;
	byte		lan[3];
	byte		isDvbSubt;
	byte		componentTag;
} S_SubtInfo;

typedef struct
{
	word 	pid;
	byte		 lan[3];
	byte		isAc3 : 1;
	byte		soundMode : 3;
	byte		res : 4;
	byte		componentTag;
	byte		audioStreamType;
	byte		name[MAX_AUDIO_NAME];
} S_AudioInfo;

typedef struct
{
	void	*thisP;		// this
	int 	dataSize;

	byte 	tableId;
	byte		versionNumber;
	word	svcId;
	word	pcrPid;
	dword	oldCrc32;
	dword 	crc32;
	word	numOfDescriptor;
	T_SiDescriptor	*descriptor;
	word	numOfEsStream;
	S_SiPmtEs	*pmtEs;
} S_SiPmt;

typedef struct
{
	void	*thisP;		// this
	int 	dataSize;

	byte 	tableId;
	byte		versionNumber;
	dword 	crc32;
	word	numOfDescriptor;
	T_SiDescriptor	*descriptor;
	T_SecData	secData;
} S_SiCat;

typedef struct
{
	word	svcId;
	word	pmtPid;
	word 	ttxPid;
	word 	pcrPid;
	word	videoPid;
	word	citPid;
	dword	caSystem;

	byte		isCas : 1;
	byte		notused : 1;
	byte		runStatus : 3;
	byte		lcnHidden : 1;
	byte		isRenamed : 1;
	byte		isHd : 1;			// for component descriptor
	byte		ttxComponentTag;
	word	ttxPageNum;

	word	logicalChNum;

	S_AudioInfo 	*audioInfo;			// DO NOT USE THIS POINTER. USE GetAudioInfo()
	S_SubtInfo	*subtInfo;			// DO NOT USE THIS POINTER. USE GetSubtInfo()

	byte		nAudio;
	byte		nSubt;

	byte		svcType;	// NOT  tv, radio.
	byte		svcName[MAX_SERVICE_NAME + 1];

	byte		pmtLoaded;
	byte		videoStreamType;
	S_SiPmt *pmtData;				// DO NOT USE THIS POINTER. USE GetPmtData()

	int		pmtShareIdx;
	int		audioInfoShareIdx;
	int		subtInfoShareIdx;

} S_ServiceInfo;

typedef struct
{
	word	svcId;
	word	svcNum;
} S_LogicalChNum;

typedef struct
{
	dword	freq;
	word	sr;
	word	orgNetId;
	word	tsId;
	byte		polar;	// cable에서는 qam mode로 사용하고, terrestrial에서는 band width로 사용한다.
} S_NitTp;

typedef struct
{
	// basic table receive check
	byte			allPmtLoaded;
	byte			sdtLoaded;
	byte			nitLoaded;
	byte			tdtLoaded;
	byte			batLoaded;

	word		degree;		// always from east
	word		orgNetId;		// from SDT

	word		tsId;			// from PAT
	word		nNitTp;

	word		nLCN;
	byte			patLoaded;
	byte 		catLoaded;
	S_SiCat *	catData;		// DO NOT USE THIS POINTER. USE GetCatInfo()

	int			nPmtCnt;
	int			nSvc;
	int			nNitSvcListDescr;	// <= MAX_NIT_TP

	byte			notUsed1 : 2;
	byte			res : 6;

	char		networkName[MAX_NETWORK_NAME+1];
	char		providerName[MAX_PROVIDER_NAME+1];

	S_ServiceInfo	svcInfo[MAX_SVC_INFO];	// PAT 와 SDT 의 서비스의 수가 다를 수 있으므로 배열로 바꿈.
	S_LogicalChNum	logicalChNum[MAX_SCAN_SVC];
	S_NitTp	nitTp[MAX_NIT_TP];

	int		catShareIdx;
	word	networkId;

} S_SiInfo;

typedef enum
{
	SI_NotLoaded,
	SI_Loaded,
	SI_TimeOut,
	SI_NoNeed,
} T_SiLoaded;

// --- EPG ------------------------------------------

typedef struct
{
	word	svcId;				// service ID
	word	duration	;

	word	evtId;				// event ID
	word	orgNetId;

	word	tsId;
	word	extTextLen;

	dword	startTime;			// total seconds
	dword	endTime;

	byte		descNum;
	byte		lastDescNum;
	byte		evtNameLen; 		// length of event name in "text"
	byte		shortTextLen;

	byte		*text;
	byte		*extText;

	byte		parentalRating; 		// parental Rating

	byte		itemNum;			// number of item
	byte		extFlag:1;
	byte		reserve2:2;
	byte 	runningStatus:5;
	byte 	eitcontentsNibble1:4;
	byte 	eitcontentsNibble2:4;

	byte		isoCode[3];
	byte		codeIdx;

}S_EventInfo;

// --- Alarm ------------------------------------------

typedef enum {
	AlarmMode_Off,
	AlarmMode_Once,
	AlarmMode_Daily,
	AlarmMode_Weekly,
	AlarmMode_Disable
} T_AlarmMode;

typedef enum {
	AlarmType_Play,
	AlarmType_Recording,
	AlarmType_Max
} T_AlarmType;

// APPLMSG_ALARM_Set_Fail message param1
typedef enum {
	AlarmErr_None,
	AlarmErr_SlotAllocationFail,	// no free slot
	AlarmErr_Collision,			// collision slot param2... if param2 is '0' time error
	AlarmErr_Running,
	AlarmErr_WrongValue,
	AlarmErr_UnKnown
} T_SetAlarmError;

#define AlarmPwOff_Reset	0
#define AlarmPwOff_Set		1

extern int ALARM_NUM;	// alarmIdx-0 is not used.

typedef struct
{
	dword	alarmSTime;
	dword	alarmETime;

	word	alarmSat;
	word	alarmSvc;

	word	alarmEvtId;
	byte		extraRecStart;			// when Init appl get it from recording setting.

	byte		notUsedAlarmTuner	: 2;
	byte		notUsedUsingTuner	: 2;
	byte		confirmMsg		: 2;
	byte		condition		: 2;

	byte		alarmSvcType	: 2;		// Tv(0), Radio(1)
	byte		alarmPwOff		: 2;		// Not used. /* Power off when timer end time - Turn Off(1), or not(0) */
	byte		alarmMode		: 4;		// T_AlarmMode - Off(0), Once(1), Daily(2), Weekly(3), Disable(4)

	byte		alarmType		: 4;		// Play(0), Recording(1)
	byte		seriesId			: 3;
	byte		resbits1			: 1;

	byte		alarmDays;				// 0xFE - " 0Bit-0. 1Bit-Mon. 2Bit-Tue. 3Bit-Wed. 4Bit-Thu. 5Bit-Fri. 6Bit-Sat. 7Bit-Sun. - from LSB. "

	byte		extraRecEnd;			// when Init appl get it from recording setting.

	dword	alarmTuner		:4;
	dword	usingTuner		:4;
	dword	resbits8			:8;
	dword	resbits16		:16;

	dword	resbits32;
} S_Alarm;

typedef enum
{
	LANG_ENGLISH,
	LANG_FRENCH,			// EXTFONT_8859_9
	LANG_GERMAN,			// EXTFONT_8859_9
	LANG_DUTCH,			// EXTFONT_8859_9
	LANG_GREEK,			// EXTFONT_8859_7
	LANG_SPANISH,			// EXTFONT_8859_9
	LANG_ARABIC,			// EXTFONT_8859_6
	LANG_ITALIAN,			// EXTFONT_8859_9
	LANG_PORTUGUESE,		// EXTFONT_8859_9
	LANG_TURK,				// EXTFONT_8859_9
	LANG_BULGARIAN,		// EXTFONT_8859_5
	LANG_RUSSIAN,			// EXTFONT_8859_5
	LANG_UKRAINIAN,		// EXTFONT_8859_5
	LANG_POLISH,			// EXTFONT_8859_2
	LANG_LITHUANIAN,		// EXTFONT_8859_13
	LANG_CZECH,			// EXTFONT_8859_2
	LANG_SWEDISH,			// EXTFONT_8859_9
	LANG_FINNISH,			// EXTFONT_8859_9
	LANG_DANISH,			// EXTFONT_8859_9
	LANG_NORWEGIAN,		// EXTFONT_8859_9
	LANG_SLOVAK,			// EXTFONT_8859_2
	LANG_SLOVENIAN,		// EXTFONT_8859_2
	LANG_MACEDONIAN,		// EXTFONT_8859_5
	LANG_ROMANIAN,		// EXTFONT_8859_2
	LANG_SERBIAN,			// EXTFONT_8859_2
	LANG_HUNGARIAN,		// EXTFONT_8859_2
	LANG_LATVIAN,			// EXTFONT_8859_13
	LANG_THAI,				// EXTFONT_8859_11
	LANG_KURDISH,			// EXTFONT_8859_9
	LANG_PERSIAN,			// EXTFONT_8859_6
	LANG_UZBEKISTAN,
	LANG_HEBREW,			//EXTFONT_8859_8

	N_Language,

} T_Language;

#endif	// __STBDATA_H__

