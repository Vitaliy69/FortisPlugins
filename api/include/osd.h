#ifndef __OSD_H__
#define __OSD_H__

#include "type.h"

#define PIXELTYPE 			dword

int GetOsdWidth();
int GetOsdHeight();

#define ARGB( a, r, g, b )	(((a)<<24) | ((r)<<16) | ((g)<<8) | ((b)))
#define OSDWIDTH		GetOsdWidth()
#define OSDHEIGHT		GetOsdHeight()

typedef enum
{
	BLIT_OP_COPY,
	BLIT_OP_SPRITE,
	BLIT_OP_SPRITE_NOALPHA,
	BLIT_OP_MIX,		// mix with background image.
	BLIT_OP_BLACK,
	BLIT_OP_SW,
} T_BlitMode;

typedef struct
{
	int x;
	int y;
	int w;
	int h;
 	dword mask;
 	dword mode;			// T_BlitMode
	PIXELTYPE data[1];	// blit data start from here
} S_BlitImg;

bool OsdInit( dword *kerAddr, dword *blitAddr );
void OsdClose( void );
bool IsOsdAvail();

//obsolete. dword *GetOsdAddr();
//obsolete. void *GetBlitBuf();
void Blit( int x, int y, int w, int h, void *image, dword mask, int mode );	// Not supported
void Blit( S_BlitImg *img, word dstw=0, word dsth=0 );	// dstw, dsth are for stretch image.

void ShowStillPic( S_BlitImg *img , int dataSize, int esType = 0 );
void HideStillPic();

byte *ImageCompress( PIXELTYPE *source, int x, int y, int width, int height, int stride = OSDWIDTH, int *size = NULL );
int ImageDecompress( PIXELTYPE *dest, byte *source, int x, int y, int width, int height, int stride = OSDWIDTH );

byte *OsdCompress( int x, int y, int width, int height, int *size = NULL );
int OsdDecompress( byte *source, int x, int y, int width, int height );
void FillBox( int x, int y, int w, int h, dword color );
void DrawRect( int x, int y, int w, int h, dword color, int t = 1 );

void GetPixmap( int x, int y, int w, int h, dword *buf );
void PutPixmap( int x, int y, int w, int h, dword *buf );
void PutPartPixmap( int x, int y, int w, int h, dword *buf, int bw );

bool FontInit( char *fontFile = NULL );
void FontClose();
void SetFontSize( int size );
void SetFontAttr( int setFlag, int clrFlag );
void DrawText( int x, int y, int width, int height, char *text, PIXELTYPE fColor = ARGB(0xff,0xff,0xff,0xff), PIXELTYPE bColor = 0
	,int al = 0, int val = 0, int indent = 0 );
void GetStrWidth( const char *text, int cc, int flags, int *width, int *height, int *base );

void RegionShow( int id, int on );
void ChangeLayer( int option );

enum Alignment
{
	ALIGN_Left,
	ALIGN_Center,
	ALIGN_Right
};

enum VAlignment
{
	VALIGN_Top,
	VALIGN_Center,
	VALIGN_Bottom
};

#define OSD_MODE_4ARGB			0
#define OSD_MODE_4AYUV			1
#define OSD_MODE_8ARGB			2
#define OSD_MODE_8AYUV			3
#define OSD_MODE_16ARGB			4
#define OSD_MODE_16AYUV			5
#define OSD_MODE_16RGB				6
#define OSD_MODE_16YUV655			7
#define OSD_MODE_16YUV422			8
#define OSD_MODE_16ARGB1555		9
#define OSD_MODE_16AYUV2644		10
#define OSD_MODE_32ARGB			11
#define OSD_MODE_32AYUV			12

#endif	// __OSD_H__

