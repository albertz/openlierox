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

// Standard includes
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <ctype.h>
#include <string>
#include <vector>
#include <list>
#include <sstream>

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
#if _MSC_VER <= 1200
inline size_t strnlen(const char *str, size_t maxlen)  { 
	register unsigned int i=0;
	for (;i<maxlen && str[i];i++) {}
	return i;
}
#endif // _MSC_VER <= 1200
inline int strncasecmp(const char *str1, const char *str2, size_t l) {return _strnicmp(str1,str2,l); }
#	define vsnprintf _vsnprintf
#	define snprintf	 _snprintf
#	define stat _stat
inline bool S_ISREG(unsigned short s)  { return (s & S_IFREG) != 0; }
inline bool S_ISDIR(unsigned short d)  { return (d & S_IFDIR) != 0; }
#else
#	include <sys/dir.h>
#	include <unistd.h>
inline void strlwr(char* string) {
	if(string) while( 0 != ( *string++ = (char)tolower( *string ) ) ) ;
}
#endif

#include <SDL/SDL.h>


#include <nl.h>
// workaraound for bad named makros by nl.h
// macros are bad, esp the names (reserved/used by CBytestream)
// TODO: they seem to not work correctly!
// all use of it in CBytestream was removed
inline void nl_writeShort(char* x, int& y, NLushort z)		{ writeShort(x, y, z); }
inline void nl_writeLong(char* x, int& y, NLulong z)		{ writeLong(x, y, z); }
inline void nl_writeFloat(char* x, int& y, NLfloat z)		{ writeFloat(x, y, z); }
inline void nl_writeDouble(char* x, int& y, NLdouble z)		{ writeDouble(x, y, z); }
inline void nl_readShort(char* x, int& y, NLushort z)		{ readShort(x, y, z); }
inline void nl_readLong(char* x, int& y, NLulong z)			{ readLong(x, y, z); }
inline void nl_readFloat(char* x, int& y, NLfloat z)		{ readFloat(x, y, z); }
inline void nl_readDouble(char* x, int& y, NLdouble z)		{ readDouble(x, y, z); }
#undef writeByte
#undef writeShort
#undef writeFloat
#undef writeString
#undef readByte
#undef readShort
#undef readFloat
#undef readString

// Helpful Macros
#define		CMP(str1,str2)  !xmlStrcmp((const xmlChar *)str1,(const xmlChar *)str2)

template <typename T> inline T MIN(T a, T b) { return a<b?a:b; }
template <typename T> inline T MAX(T a, T b) { return a>b?a:b; }
inline unsigned long MIN(unsigned long a, unsigned int b) { return a<b?a:b; }

void d_printf(char *fmt, ...);

// like strcasecmp, but for a char
int chrcasecmp(const char c1, const char c2);

#ifndef WIN32
inline char* itoa(int val, char* buf, int base) {
	int i = 29; // TODO: bad style!
	buf[i+1] = '\0';

	for(; val && i ; --i, val /= base)	
		buf[i] = "0123456789abcdef"[val % base];
	
	return &buf[i+1];
}
#	define		stricmp		strcasecmp
#else // WIN32
#	define		strcasecmp	stricmp
#endif

// --------------------------------------------
// Endian checks and conversions

#ifndef WIN32
#include <endian.h>
#endif

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

#ifdef WIN32
// WIN32 doesn't define this, endian.h doesn't exist there
#define	__BYTE_ORDER	__LITTLE_ENDIAN
#endif

#if !defined(__BYTE_ORDER)
#	error __BYTE_ORDER not defined
#endif
#if __BYTE_ORDER == __LITTLE_ENDIAN
#	define EndianSwap(x)		;
#	define BEndianSwap(x)		ByteSwap5(x);
#	define GetEndianSwapped(x)	(&x)
#elif __BYTE_ORDER == __BIG_ENDIAN
#	define EndianSwap(x)		ByteSwap5(x);
#	define BEndianSwap(x)		;
#	define GetEndianSwapped(x)	(GetByteSwapped(x))
#else
#	error unknown ENDIAN type
#endif

// secure str handling macros
// WARNING: don't use expressions like buf[i++] with the macros, because the "i" variable will be incremented twice in some macros!
#define		fix_markend(chrarray) \
				chrarray[sizeof(chrarray)-1] = '\0';
#define		fix_strnlen(chrarray) \
				strnlen(chrarray,sizeof(chrarray))
#define		fix_strncpy(chrarray, src) \
			{	strncpy(chrarray, src, sizeof(chrarray)); \
			 	chrarray[sizeof(chrarray)-1] = '\0'; }
#define		fix_strncat(chrarray, src) \
			{	size_t destlen = strnlen(chrarray, sizeof(chrarray)); \
				strncpy(&chrarray[destlen], src, sizeof(chrarray)-destlen); \
				chrarray[sizeof(chrarray)-1] = '\0'; }
#define		dyn_markend(dest, len) \
				dest[len-1] = '\0';
#define		dyn_strncpy(dest, src, len) \
			{	strncpy(dest, src, len); \
				dest[len-1] = '\0'; }
#define		dyn_strncat(dest, src, len) \
			{	size_t destlen = strnlen(dest, len); \
				strncpy(&dest[destlen], src, len-destlen); \
				dest[len-1] = '\0'; }


#include "Networking.h"

// XML parsing library
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>


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
