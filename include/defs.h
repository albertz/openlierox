/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Standard definitions
// Created 28/6/02
// Jason Boettcher


#ifndef __DEFS_H__
#define __DEFS_H__

// Global compile defines
#ifndef DEBUG
#define DEBUG	1
#endif

#ifdef _MSC_VER
#ifndef _DEBUG
#undef DEBUG
#endif // _DEBUG
#endif // _MSC_VER

// Disable this silly warning
#ifdef _MSC_VER
#pragma warning(disable: 4786)
#pragma warning(disable: 4996)
#endif




// file input/output

#include <sys/types.h>
#include <sys/stat.h>

#ifdef WIN32
#	include <windows.h>
#	include <io.h>
#	include <direct.h>
	// wrappers to provide the standards
	inline int mkdir(const char *path, int mode) { return _mkdir(path); }
#	define stat _stat
inline bool S_ISREG(unsigned short s)  { return (s & S_IFREG) != 0; }
inline bool S_ISDIR(unsigned short d)  { return (d & S_IFDIR) != 0; }
#endif

#if (defined(WIN32) && _MSC_VER <= 1200) || defined(MACOSX)
// TODO: remove this from whole code! (we use std::string now)
inline size_t strnlen(const char *str, size_t maxlen)  { 
	register unsigned int i=0;
	for (;i<maxlen && str[i];i++) {}
	return i;
}
#endif


#include <SDL/SDL.h>


void d_printf(char *fmt, ...);

// --------------------------------------------
// Endian checks and conversions

#define ByteSwap5(x) ByteSwap((unsigned char *) &x,sizeof(x))

void ByteSwap(unsigned char * b, int n);
extern unsigned char byteswap_buffer[16];
template <typename T>
inline T* GetByteSwapped(const T b)
{
	*((T*)byteswap_buffer) = b;
	ByteSwap(byteswap_buffer, sizeof(T));
	return (T*)byteswap_buffer;
}

#if !defined(SDL_BYTEORDER)
#	error SDL_BYTEORDER not defined
#endif
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
#	define EndianSwap(x)		;
#	define BEndianSwap(x)		ByteSwap5(x);
#	define GetEndianSwapped(x)	(&x)
#elif SDL_BYTEORDER == SDL_BIG_ENDIAN
#	define EndianSwap(x)		ByteSwap5(x);
#	define BEndianSwap(x)		;
#	define GetEndianSwapped(x)	(GetByteSwapped(x))
#else
#	error unknown ENDIAN type
#endif



#include "Networking.h"



// Auxiliary Core components
#include "types.h"
#include "AuxLib.h"
#include "Error.h"
#include "Timer.h"
#include "Options.h"



// Core classes
#include "CInput.h"
#include "CVec.h"



// Secondary components (coz they need to be below the classes)
#include "ConfigHandler.h"
#include "MathLib.h"
#include "CFont.h"

#include "CssParser.h"




#endif  //  __DEFS_H__
