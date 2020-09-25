
#ifndef __DBG_H__
#define __DBG_H__

#if 0	// confidential
#define DEBUG_TO_SCREEN
void DbgSaveToFile();
#endif

//#define DEBUG
//#define NO_DEBUG

extern void Puts( const char *str );

extern void _Printf( const char *format, ... );
extern void _PrintfN( const char *format, ... );
extern void	_DumpMemory( const void *mem, int size, const char *name );
extern void _PrintfMod( int mod, const char *format, ... );
extern void	_DumpMemoryMod( int mod, const void *mem, int size, const char *format, ... );


#if defined(DEBUG) && !defined(NO_DEBUG)

#define Printf		_Printf
#define PrintfN		_PrintfN
#define PrintfMod		_PrintfMod
#define DumpMemory			_DumpMemory
#define DumpMemoryMod		_DumpMemoryMod
#define PrintError	_Printf
#define Mprint( name, addr, len )	_DumpMemory( addr, len, name );

#else

#define Printf(...)
#define PrintfN(...)
#define PrintfMod(...)
#define DumpMemory(...)
#define DumpMemoryMod(...)
#define PrintError	_Printf
#define Mprint(...)

#endif


#endif	// __DBG_H__

