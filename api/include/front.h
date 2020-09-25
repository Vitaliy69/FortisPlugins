
#ifndef __FRONT_H__
#define  __FRONT_H__

int WiFront_Init();
int WiFront_Term();

#if 0
int WiVfd_LedBright(int idx, int runBr, int idleBr);
int WiVfd_SetDimming(int quality);
int WiVfd_Display(char *buf, int size);
#endif

#if 0
int Front_IrRepeatConfig(int wait, int repeat);
int Front_IrSetCode(int rCustomCode, int rPowerCode);
int Front_IrGetCode(int *rCustomCode, int *rPowerCode);
#endif

typedef enum {
	Rem_Block,
	Rem_Monitoring,
} T_RemRegisterOpt;

#define REM_UP			0x0
#define REM_DOWN		0x1
#define REM_RIGHT		0x2
#define REM_LEFT		0x3
#define REM_MENU		0x4
#define REM_OK			0x1f
#define REM_FAVORITE	0x41
#define REM_SAT			0x05
//#define REM_POWER	0xff
#define REM_MUTE		0xc
#define REM_GUIDE		0x8
#define REM_EXIT		0x1c
#define REM_INFO		0x6
#define REM_TVRADIO		0x1a
#define REM_PLAYLIST	0x40
#define REM_AUDIOTRK	0x49
#define REM_SUBT		0x0b
#define REM_0			0x10
#define REM_1			0x11
#define REM_2			0x12
#define REM_3			0x13
#define REM_4			0x14
#define REM_5			0x15
#define REM_6			0x16
#define REM_7			0x17
#define REM_8			0x18
#define REM_9			0x19
#define REM_F1			0x4b
#define REM_F2			0x4a
#define REM_F3			0x49	// same as REM_AUDIOTRK
#define REM_F4			0x48
//#define REM_UHF		0x
#define REM_SLEEP		0x1e
#define REM_TELETEXT	0x0d
#define REM_RECALL		0x09
#define REM_PGUP		0x44
#define REM_PGDOWN		0x43
//#define REM_TVSAT		0x
//#define REM_TEST		0x
#define REM_PAUSE		0x07
#define REM_PREV		0x50
#define REM_NEXT		0x4c
#define REM_FastBackward	0x58
#define REM_FastForward	0x5c
#define REM_Record		0x56
#define REM_PLAY		0x55
#define REM_STOP		0x54
#define REM_RESOLUTION	0x0f
#define REM_VFORMAT	0x0e
#define REM_VOLUP		0x4e
#define REM_VOLDOWN	0x4f
#define REM_CHUP		0x5e
#define REM_CHDOWN		0x5f
#define REM_PIP			0x51
#define REM_PIPLIST		0x53
#define REM_SWAP		0x52
#define REM_POINTMARK	0x42

#define REM_FRONT_UP		0x1
#define REM_FRONT_DOWN	0x2
#define REM_FRONT_RIGHT	0x3
#define REM_FRONT_LEFT		0x4
#define REM_FRONT_OK		0x6

// APPLMSG_REMOCON
enum {
CMD_UP	= 0x10,
CMD_DOWN,
CMD_RIGHT,
CMD_LEFT,
CMD_SAT,
CMD_OK,
CMD_FAVORITE,
CMD_POWER,
CMD_MUTE,
CMD_MENU,
CMD_GUIDE,
CMD_EXIT,
CMD_INFO,
CMD_TVRADIO,
CMD_PLAYLIST,
CMD_AUDIOTRK,
CMD_SUBT,
CMD_0,
CMD_1,
CMD_2,
CMD_3,
CMD_4,
CMD_5,
CMD_6,
CMD_7,
CMD_8,
CMD_9,

CMD_F1,
CMD_F2,
CMD_F3,
CMD_F4,

CMD_UHF,
CMD_SLEEP,
CMD_TELETEXT,

CMD_RECALL,

CMD_PGUP,
CMD_PGDOWN,

CMD_TVSAT,

CMD_TEST,
CMD_PAUSE,

CMD_PREV,
CMD_NEXT,

CMD_FastBackward,
CMD_FastForward,
CMD_Record,

CMD_PLAY,
CMD_STOP,

CMD_RESOLUTION,
CMD_VFORMAT,
CMD_VOLUP,
CMD_VOLDOWN,
CMD_CHUP,
CMD_CHDOWN,
CMD_PIP,
CMD_PIPLIST,
CMD_SWAP,
CMD_POINTMARK,
};

#endif

