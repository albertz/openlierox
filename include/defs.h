/////////////////////////////////////////
//
//             Liero Xtreme
//
//     Copyright Auxiliary Software 2002
//
//
/////////////////////////////////////////


// Standard definitions
// Created 28/6/02
// Jason Boettcher


#ifndef __DEFS_H__
#define __DEFS_H__


// Global compile defines
#define DEBUG	= 1;

// Standard includes
//#include <io.h> //TODO: does not exist under Linux
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <time.h>
//#include <direct.h> //TODO: does not exist under Linux

//#include "mmgr.h"

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

//TODO: HINT: include of bass-header was removed here; new soundsystem is needed
#include <nl.h>
// workaraound for bad named makros by nl.h
// makros are bad, esp the names (reserved/used by CBytestream)
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


// Auxiliary Core components
#include "types.h"
#include "AuxLib.h"
#include "Cache.h"
#include "Error.h"
#include "GfxPrimitives.h"
#include "Timer.h"



// Core classes
#include "CInput.h"
#include "CVec.h"


// Network classes
#include "CBytestream.h"
#include "CChannel.h"


// Secondary components (coz they need to be below the classes)
#include "ConfigHandler.h"
#include "MathLib.h"
#include "CFont.h"

#include "FindFile.h"





// Helpful Macros
#define		LOAD_IMAGE(bmp,name) bmp = LoadImage(name,0); if(bmp == NULL) return false
#define		LOAD_IMAGE_BPP(bmp,name) bmp = LoadImage(name,SDL_GetVideoSurface()->format->BitsPerPixel); if(bmp == NULL) return false

#define		MIN(a,b)	(a)<(b) ? (a) : (b)
#define		MAX(a,b)	(a)>(b) ? (a) : (b)

void		d_printf(char *fmt, ...);

#endif  //  __DEFS_H__
