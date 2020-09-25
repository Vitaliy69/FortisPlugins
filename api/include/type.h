
#ifndef TYPE_H
#define TYPE_H

typedef bool BOOLEAN;
typedef unsigned char u_int8;
typedef unsigned short u_int16;
typedef unsigned int u_int32;

typedef char int8;
typedef short int16;
typedef int int32;

#ifndef byte
typedef unsigned char byte;
#endif

#ifndef word
typedef unsigned short word;
#endif

#ifndef dword
typedef unsigned long dword;
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef NULL
#define NULL 0
#endif

#define 	WAIT_FOREVER	(-1)

#endif /* TYPE_H */

