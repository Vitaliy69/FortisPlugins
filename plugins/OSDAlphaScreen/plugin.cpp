/*!
    \file plugin.cpp
    \author Vitaliy Gribko
    
    Distributed under the GPL v2
    see the file LICENSE for details
    or visit http://www.gnu.org/copyleft/gpl.html
*/

#include "plugin.h"
#include "configer.h"
#include "logger.h"

#include <string>
#include <sstream>

#include <err.h>
#include <dirent.h>
#include <errno.h>
#include <pthread.h>
#include <strings.h>
#include <stdlib.h>
#include <unistd.h>
#include <png.h>

#include <arpa/inet.h>

#define COL_ALPHA(col)          ((col >> 24) & 0xFF)
#define COL_RED(col)            ((col >> 16) & 0xFF)
#define COL_GREEN(col)          ((col >> 8) & 0xFF)
#define COL_BLUE(col)           ((col) & 0xFF)

#define COMPRESS                5
#define PORT                    7777
#define BUTTON                  0x52
#define PATH                    "/tmp"
#define COMMENT                 "Openbox SX Screenshot by Vitaliy69"
#define BUFSIZE                 2048

using namespace std;

int                 compressl, wport, button, one, sock;
string              path, comment, lastName;
struct sockaddr_in  svr_addr, cli_addr;
socklen_t           sin_len;
pthread_t           wthread, fthread;

struct Params
{
    char*   filename;
    int     width;
    int     height;
    dword   *buffer;
    char*   title;
};

char rootPage[] = 
"HTTP/1.1 200 OK\r\n"
"Content-Type: text/html; charset=UTF-8\r\n\r\n"
"<doctype !html>\r\n"
"<html>\r\n"
"\t<head>\r\n"
"\t\t<title>OSDAlphaScreen</title>\r\n"
"\t</head>\r\n"
"\t<style>\r\n"
"\t\tbody { background-color: #F0F0F0 }\r\n"
"\t\th1 { font-size:1cm; text-align: center; }\r\n"
"\t</style>\r\n"
"\t<body>\r\n"
"\t\t<a href=\"screenshot.html\" style=\"color: #696969\"><h1>Make screenshot!</h1></a>\r\n"
"\t</body>\r\n"
"</html>\r\n";

char screenPage[] = 
"HTTP/1.1 200 OK\r\n"
"Content-Type: text/html; charset=UTF-8\r\n\r\n"
"<doctype !html>\r\n"
"<html>\r\n"
"\t<head>\r\n"
"\t\t<title>OSDAlphaScreen</title>\r\n"
"\t</head>\r\n"
"\t<style>\r\n"
"\t\tbody { background-color: #F0F0F0 }\r\n"
"\t\th1 { font-size:1cm; text-align: center; }\r\n"
"\t</style>\r\n"
"\t<body>\r\n"
"\t\t<a href=\"screenshot.html\" style=\"color: #696969\"><h1>Make screenshot!</h1></a>\r\n"
"\t\t<br><br>\r\n"
"\t\t<center><img style = \"width: 100%; max-height: 100%\" src=\"image.png\"></center>\r\n"
"\t</body>\r\n"
"</html>\r\n";

char notFoundPage[] =
"HTTP/1.1 404 Not Found\r\n"
"Content-Type: text/html; charset=UTF-8\r\n\r\n"
"<doctype !html>\r\n"
"<html>\r\n"
"\t<head>\r\n"
"\t\t<title>OSDAlphaScreen</title>\r\n"
"\t</head>\r\n"
"\t<body>\r\n"
"\t\tFile Not found!\r\n"
"\t</body>\r\n"
"</html>\r\n";

Configer    *configer = NULL;
Logger      *logger = NULL;

template<typename T>
string to_string(const T& value)
{
    ostringstream oss;
    oss << value;
    return oss.str();
}

void readSettings()
{
    bool isActiveTextLog = true;
    int logPort = 0;
    std::string logServer = "";

    if (!access("/var/etc/OSDAlphaScreen.cfg", 0))
    {
        configer = new Configer("/var/etc/OSDAlphaScreen.cfg");

        isActiveTextLog = configer->getBoolValue("logging", "enableTextlog");
        logPort = configer->getIntValue("logging", "syslogport");
        logServer = configer->getStrValue("logging", "sysloghost");
    }

    logger = new Logger("/tmp/", "OSDAlphaScreen.log", isActiveTextLog, logPort, logServer, "OSDAlphaScreen, v0.6");

    if (access("/var/etc/OSDAlphaScreen.cfg", 0))
    {
        logger->writeToLog("The configuration file was not found. Use the default settings.");

        compressl = COMPRESS;
        wport = PORT;
        button = BUTTON;
        path = PATH;
        comment = COMMENT;
    }
    else
    {
        logger->writeToLog("Reading the configuration file...");

        compressl = configer->getIntValue("png", "compress");
        if ((compressl < 0) || (compressl > 9))
            compressl = COMPRESS;

        wport = configer->getIntValue("web", "port");
        if ((wport < 1024) || (wport > 65535))
            wport = PORT;

        string hot = configer->getStrValue("button", "key");

        if (hot.length() != 4)
            button = BUTTON;
        else
            sscanf(hot.c_str(), "%x", &button);

        path = configer->getStrValue("png", "path");
        
        if (path.length() < 1)
            path = PATH;

        DIR* dir = opendir((const char*)path.c_str());
        if (dir)
            closedir(dir);
        else if (ENOENT == errno)
            path = PATH;

        comment = configer->getStrValue("png", "comment", true);

        logger->writeToLog("Screenshots save path: " + path);
        logger->writeToLog("Z-compress level:", compressl);
        logger->writeToLog("Screenshots comments: " + comment);
        logger->writeToLog("Web-server port:", wport);
        logger->writeToLog("Hot button key:", button);
    }

    lastName = "";
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

void* writeImage(void* params)
{
    Params  *args = static_cast<Params*>(params);
    char    *filename = args->filename;
    int     width = args->width;
    int     height = args->height;
    dword   *buffer = args->buffer;
    char    *title = args->title;

    int x = 0, y = 0;
    FILE *fp;
    png_structp png_ptr;
    png_infop info_ptr = NULL;
    png_bytep row = NULL;

    fp = fopen(filename, "wb");
    if (fp == NULL) 
    {
        logger->writeToLog("Error creating file " + string(filename) + "!");
        goto finalise;
    }

    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (png_ptr == NULL) 
    {
        logger->writeToLog("Memory creating error for PNG structure!");
        goto finalise;
    }

    info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL) 
    {
        logger->writeToLog("Memory creating error for PNG information structure!");
        goto finalise;
    }

    if (setjmp(png_jmpbuf(png_ptr))) 
    {
        logger->writeToLog("Error creating PNG!");
        goto finalise;
    }

    png_init_io(png_ptr, fp);
    png_set_compression_level(png_ptr, compressl);
    png_set_filter(png_ptr, 0, PNG_FILTER_NONE);

    png_set_IHDR(png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

    if (title != NULL) 
    {
        png_text title_text;
        title_text.compression = PNG_TEXT_COMPRESSION_zTXt;
        title_text.key = (char*)"Title";
        title_text.text = title;
        png_set_text(png_ptr, info_ptr, &title_text, 1);
    }

    png_write_info(png_ptr, info_ptr);

    row = (png_bytep) malloc(4 * width * sizeof(png_byte));

    for (y = 0 ; y < height; y++) 
    {
        for (x = 0 ; x < width ; x++) 
        {
            row[x * 4] = COL_RED(buffer[y * width + x]);
            row[x * 4 + 1] = COL_GREEN(buffer[y * width + x]);
            row[x * 4 + 2] = COL_BLUE(buffer[y * width + x]);
            row[x * 4 + 3] = COL_ALPHA(buffer[y * width + x]);
        }
        png_write_row(png_ptr, row);
    }

    png_write_end(png_ptr, NULL);

    finalise:
    if (fp != NULL) fclose(fp);
    if (info_ptr != NULL) png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
    if (png_ptr != NULL) png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
    if (row != NULL) free(row);
    free(buffer);
    delete args;

    if (access(filename, 0))
        logger->writeToLog("Error screenshot creating!");
    else
        logger->writeToLog("Screenshot " + string(filename) + " made successfully " + string(getFullTime()) + "!");

    pthread_exit(0);
}

string makeScreenshot(bool join = false)
{
    int w = GetOsdWidth();
    int h = GetOsdHeight();

    pthread_t sthread;
    dword *blitAddr = (dword*)malloc(4 * w * h);
    GetPixmap(0, 0, w, h, blitAddr);
    std::string name(path + "/screenshot-" + getFullTime() + ".png");
    Params *params = new Params();
    params->filename = (char*)name.c_str();
    params->width = w;
    params->height = h;
    params->buffer = blitAddr;
    params->title = (char*)comment.c_str();
    pthread_create(&sthread, NULL, writeImage, (void*)params);

    if (join)
        pthread_join(sthread, NULL);
    else
        pthread_detach(sthread);

    return name;
}

void httpdInit()
{
    one = 1;
    sin_len = sizeof(cli_addr);
    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock < 0)
        logger->writeToLog("Can not create the socket!");

    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int));

    bzero((char*) &svr_addr, sizeof(svr_addr));

    int port = wport;
    svr_addr.sin_family = AF_INET;
    svr_addr.sin_addr.s_addr = INADDR_ANY;
    svr_addr.sin_port = htons(port);

    if (bind(sock, (struct sockaddr *) &svr_addr, sizeof(svr_addr)) == -1) 
    {
        close(sock);
        logger->writeToLog("Can not bind to the port!");
    }

    listen(sock, 5);
}

void httpdNotFound(int client_fd)
{
    write(client_fd, notFoundPage, sizeof(notFoundPage) - 1);
    close(client_fd);
}

void httpdImg(int client_fd)
{
    if (access(lastName.c_str(), 0))
        httpdNotFound(client_fd);
    else
    {
        FILE    *requested_file;
        char    *sendbuf;
        long    fileLength, result;
        string  head;

        requested_file = fopen(lastName.c_str(), "rb");
        fseek (requested_file, 0, SEEK_END);
        fileLength = ftell(requested_file);
        rewind(requested_file);

        head  = "HTTP/1.1 200 OK\r\nContent-Type: image/png; Content-Transfer-Encoding: binary; Content-Length: " + to_string(fileLength) + "; charset=UTF-8 \r\n\r\n";

        write(client_fd, head.c_str(), head.length());

        sendbuf = (char*) malloc (sizeof(char)*BUFSIZE);
        while ((result = fread(sendbuf, 1, BUFSIZE, requested_file)) > 0)
            send(client_fd, sendbuf, result, 0);

        free(sendbuf);
        fclose(requested_file);
        close(client_fd);
    }
}

void httpdScreen(int client_fd)
{
    logger->writeToLog("Making the screenshot via Web page...");
    lastName = makeScreenshot(true);
    write(client_fd, screenPage, sizeof(screenPage) - 1);
    close(client_fd);
}

void httpdRoot(int client_fd)
{
    write(client_fd, rootPage, sizeof(rootPage) - 1);
    close(client_fd);
}

void* httpdThread(void* param)
{
    int client_fd = *((int*)param);

    char buffer[BUFSIZE];
    bzero(buffer, 2048);
    int n = read(client_fd, buffer, BUFSIZE);
    if (n  > 0)
    {
        std::string str(buffer);
        if (str.find("GET /image.png HTTP/1.1") != std::string::npos)
            httpdImg(client_fd);
        else if (str.find("GET /screenshot.html HTTP/1.1") != std::string::npos)
            httpdScreen(client_fd);
        else if (str.find("GET / HTTP/1.1") != std::string::npos)
            httpdRoot(client_fd);
        else
            httpdNotFound(client_fd);
    }

    pthread_exit(0);
}
 
void* httpd(void*)
{
    httpdInit();

    while (1)
    {
        int client_fd = accept(sock, (struct sockaddr *) &cli_addr, &sin_len);

        if (client_fd == -1)
            continue;

        void* arg = &client_fd;

        pthread_create(&fthread, NULL, httpdThread, arg);
        pthread_detach(fthread);
    }

    pthread_exit(0);
}

int PluginMain(int argc, char *argv[])
{
    dword evt[4];
    int key = 0;

    readSettings();

    if (argc > 1)
    {
        if (strcmp(argv[1], "--manual") == 0)
        {
            logger->writeToLog("Making screenshot within the console...");
            makeScreenshot(true);
            return 0;
        }
    }

    WiFront_Init();

    SendCommand(PCMD_ReleaseKey, (dword)-1);
    SendCommand(PCMD_BlockKey, button, Rem_Monitoring);

    pthread_create(&wthread, NULL, httpd, NULL);
    pthread_detach(wthread);

    while(true)
    {
        int ret = WiQueue_Receive(WiMainQ, (u_int8 *)evt, 16, -1);

        if (ret == 0)
        {
            if((evt[0] & MSG_BASE_MASK) == MSG_BASE_REMOCON)
            {
                key = evt[1] & MSG_MASK_KEY_CODE;

                if (key == button)
                {
                    logger->writeToLog("Making the screenshot by the hot button...");
                    makeScreenshot();
                }
            }
        }
    }

    return 0;
}

void PluginClose()
{
    if (wthread != 0)
        pthread_cancel(wthread);

    SendCommand(PCMD_ReleaseKey, (dword)-1);

    WiFront_Term();

    if (configer) delete configer;
    if (logger) delete logger;
}
