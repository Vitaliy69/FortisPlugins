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

#include <iostream>
#include <sstream>

#include <unistd.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <jpeglib.h>
#include <dirent.h>
#include <errno.h>

#define COL_RED(col)            ((col >> 16) & 0xFF)
#define COL_GREEN(col)          ((col >> 8) & 0xFF)
#define COL_BLUE(col)           ((col) & 0xFF)

#define PATH                    "/tmp"
#define QUAL                    96

#define CORX                    0
#define CORY                    20
#define HEI                     30
#define FCOL                    0xFFFFFFFF
#define BCOL                    0x40000000
#define SIZE                    24
#define SIZEOLD                 12

typedef uint32_t pix;

typedef struct {
    int32_t width;
    int32_t height;
    pix     *buf;
} image_s;

struct my_dst_mgr {
    struct jpeg_destination_mgr jdst;
    JOCTET *buf;
    JOCTET *off;
    size_t sz;
    size_t used;
};

Logger          *logger = NULL;
Configer        *configer = NULL;

image_s         bufs;
dword           *blitAddr;

byte            *imgEcm;

std::string     path;
unsigned int    qual;

static void dst_mgr_init(j_compress_ptr cinfo)
{
    struct my_dst_mgr *dst = (my_dst_mgr*)cinfo->dest;

    dst->used = 0;
    dst->sz = cinfo->image_width
          * cinfo->image_height
          * cinfo->input_components;
    dst->buf = static_cast<JOCTET*>(malloc(dst->sz * sizeof *dst->buf));
    dst->off = dst->buf;
    dst->jdst.next_output_byte = dst->off;
    dst->jdst.free_in_buffer = dst->sz;

    return;
}

static boolean dst_mgr_empty(j_compress_ptr cinfo)
{
    struct my_dst_mgr *dst = (my_dst_mgr*)cinfo->dest;

    dst->sz *= 2;
    dst->used = dst->off - dst->buf;
    dst->buf = static_cast<JOCTET*>(realloc(dst->buf, dst->sz * sizeof *dst->buf));
    dst->off = dst->buf + dst->used;
    dst->jdst.next_output_byte = dst->off;
    dst->jdst.free_in_buffer = dst->sz - dst->used;

    return (boolean) TRUE;
}

static void dst_mgr_term(j_compress_ptr cinfo)
{
    struct my_dst_mgr *dst = (my_dst_mgr*)cinfo->dest;

    dst->used += dst->sz - dst->jdst.free_in_buffer;
    dst->off = dst->buf + dst->used;

    return;
}

static void jpeg_memory_dest(j_compress_ptr cinfo, struct my_dst_mgr* dst)
{
    dst->jdst.init_destination = dst_mgr_init;
    dst->jdst.empty_output_buffer = dst_mgr_empty;
    dst->jdst.term_destination = dst_mgr_term;
    cinfo->dest = (jpeg_destination_mgr*)dst;

    return;
}

unsigned char* image_save_to_jpeg_buf(image_s* pimage, int* size)
{
    struct      jpeg_compress_struct cinfo;
    struct      jpeg_error_mgr jerr;
    JSAMPROW    row_pointer[1];
    int         row_stride;
    char        *data;
    int         i, x;
    struct      my_dst_mgr dst;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);
    jpeg_memory_dest(&cinfo, &dst);
    cinfo.image_width = pimage->width;
    cinfo.image_height = pimage->height;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;
    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, qual, (boolean)TRUE);
    jpeg_start_compress(&cinfo, (boolean)TRUE);
    row_stride = cinfo.image_width * 3;

    if((data = static_cast<char*>(malloc(row_stride))) == NULL)
    {
        free(dst.buf);
        jpeg_destroy_compress(&cinfo);
        return NULL;
    }

    i = 0;

    while(cinfo.next_scanline < cinfo.image_height)
    {
        for(x = 0; x < pimage->width; x++)
        {
            data[x * 3]     = COL_RED(pimage->buf[i]);
            data[x * 3 + 1] = COL_GREEN(pimage->buf[i]);
            data[x * 3 + 2] = COL_BLUE(pimage->buf[i]);
            i++;
        }

        row_pointer[0] = (unsigned char *)data;
        jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }

    jpeg_finish_compress(&cinfo);
    *size = dst.used;
    free(data);
    jpeg_destroy_compress(&cinfo);

    return dst.buf;
}

char* image_save_to_jpeg_file(image_s* pimage, char* path)
{
    int nwritten, size = 0;
    unsigned char *buf;
    FILE *dst_file;

    buf = image_save_to_jpeg_buf(pimage, &size);
    if(!buf) return NULL;

    dst_file = fopen(path, "w");

    if(!dst_file)
    {
        free(buf);
        return NULL;
    }

    nwritten = fwrite(buf, 1, size, dst_file);
    fclose(dst_file);
    free(buf);

    return (nwritten == size) ? path : NULL;
}

void readConfig()
{
    if (access((char *) "/var/etc/OSDScreenshot.cfg", 0))
    {
        logger->writeToLog("The configuration file was not found. Use the default settings.");
        path = PATH;
        qual = QUAL;
    }
    else
    {
        logger->writeToLog("Reading the configuration file...");

        path = configer->getStrValue("jpeg", "path");
        
        if (path.length() < 1)
            path = PATH;

        DIR* dir = opendir((const char*)path.c_str());
        if (dir)
        {
            logger->writeToLog("Screenshots path: " + path);
            closedir(dir);
        }
        else if (ENOENT == errno)
        {
            logger->writeToLog("Path does not exist. Use the default path /tmp.");
            path = PATH;
        }

        qual = configer->getIntValue("jpeg", "qual");

        if ((qual < 5) || (qual > 100))
            qual = QUAL;

        logger->writeToLog("JPEG quality: ", qual);
    }
}

void InitScreen()
{
    bool isActiveTextLog = true;
    int logPort = 0;
    std::string logServer = "";

    if (!access("/var/etc/OSDScreenshot.cfg", 0))
    {
        configer = new Configer("/var/etc/OSDScreenshot.cfg");

        isActiveTextLog = configer->getBoolValue("logging", "enableTextlog");
        logPort = configer->getIntValue("logging", "syslogport");
        logServer = configer->getStrValue("logging", "sysloghost");
    }

    logger = new Logger("/tmp/", "OSDScrenshot.log", isActiveTextLog, logPort, logServer, "OSDScrenshot, v0.8");

    blitAddr = NULL;

    int mem_size = 3 * GetOsdWidth() * GetOsdHeight();

    bufs.buf = new pix[mem_size];
    blitAddr =  new dword[mem_size];

    imgEcm = NULL;

    readConfig();
}

void keyLock(bool bLock)
{
    if (bLock)
        SendCommand(PCMD_BlockKey, (dword)-1);
    else
    {
        SendCommand(PCMD_ReleaseKey, (dword)-1);
        SendCommand(PCMD_BlockKey, REM_SWAP, Rem_Monitoring);
    }
}

void blockAllKeys()
{
    SendCommand(PCMD_BlockKey, REM_UP, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_DOWN, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_RIGHT, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_LEFT, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_MENU, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_OK, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_FAVORITE, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_SAT, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_MUTE, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_GUIDE, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_EXIT, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_INFO, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_TVRADIO, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_PLAYLIST, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_AUDIOTRK, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_SUBT, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_0, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_2, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_3, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_4, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_5, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_6, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_7, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_8, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_9, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_F1, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_F2, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_F3, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_F4, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_SLEEP, Rem_Block);
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
    SendCommand(PCMD_BlockKey, REM_VOLUP, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_VOLDOWN, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_CHUP, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_CHDOWN, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_PIP, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_PIPLIST, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_SWAP, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_POINTMARK, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_FRONT_UP, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_FRONT_DOWN, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_FRONT_RIGHT, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_FRONT_LEFT, Rem_Block);
    SendCommand(PCMD_BlockKey, REM_FRONT_OK, Rem_Block);
}

char* getFullTime()
{
    char    *strBuf;
    int     hour, min, sec, mjd;
    int     year, mon, day, dayOfWeek;
    dword   tm;

    tm = GetCurTime(&hour, &min, &sec, &mjd);
    tm = Time2Local(tm);
    mjd = (tm >> 16) & 0xFFFF;
    hour = (tm >> 8) & 0xFF;
    min = tm & 0xFF;
    Mjd2Date(mjd, &year, &mon, &day, &dayOfWeek);

    strBuf = GetStrBuf();

    sprintf(strBuf, "%d-%d-%d-%02d-%02d-%02d", day, mon, year, hour, min, sec);

    return strBuf;
}

std::string makeScreenshot()
{
    std::string fullName;
    int w = GetOsdWidth();
    int h = GetOsdHeight();

    GetPixmap(0, 0, w, h, blitAddr);

    bufs.width = w;
    bufs.height = h;
    bufs.buf = (pix*)blitAddr;

    fullName = path + "/screenshot-" + getFullTime() + ".jpg";

    image_save_to_jpeg_file(&bufs, (char*)fullName.c_str());

    imgEcm = OsdCompress(CORX, CORY, w, HEI);

    return fullName;
}

void hideMessage()
{
    OsdDecompress(imgEcm, CORX, CORY, GetOsdWidth(), HEI);
    free(imgEcm);
    imgEcm = NULL;
}

void checkFile(std::string message)
{
    std::string text;

    if (access(message.c_str(), 0))
        text = "Save error! Check the path!";
    else
        text = "Screenshot " + message + " made successfully.";

    if (GetOsdWidth() == 1280)
        SetFontSize(SIZE);
    else
        SetFontSize(SIZEOLD);

    DrawText(CORX, CORY, GetOsdWidth(), HEI, (char *)text.c_str(), FCOL, BCOL, ALIGN_Center, VALIGN_Center, 0);

    logger->writeToLog(text);
}

int PluginMain(int UNUSED(argc), char *UNUSED(argv[]))
{
    dword evt[4];
    int key = 0;

    WiFront_Init();
    FontInit((char *) "/var/etc/swissc.ttf");

    InitScreen();

    keyLock(false);

    while(true)
    {
        int ret = WiQueue_Receive(WiMainQ, (u_int8 *)evt, 16, -1);

        if (ret == 0)
        {
            if((evt[0] & MSG_BASE_MASK) == MSG_BASE_REMOCON)
            {
                key = evt[1] & MSG_MASK_KEY_CODE;

                if (key == REM_SWAP)
                {
                    blockAllKeys();
                    sleep(1);
                    checkFile(makeScreenshot());
                    sleep(2);
                    hideMessage();
                    keyLock(false);
                }
            }
        }
    }

    return 0;
}

void PluginClose()
{
    keyLock(true);

    delete[] blitAddr;
    
    if (configer) delete configer;
    if (logger) delete logger;

    FontClose();
    WiFront_Term();
}
