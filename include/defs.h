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
#include <io.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <direct.h>

//#include "mmgr.h"

#include <SDL/SDL.h>
#include <SDL/SDL_Image.h>

#include "bass.h"
#include "nl.h"


// Auxiliary Core components
#include "AuxLib.h"
#include "Cache.h"
#include "Error.h"
#include "GfxPrimitives.h"
#include "Timer.h"
#include "types.h"



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