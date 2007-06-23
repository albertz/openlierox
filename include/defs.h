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




#if (defined(WIN32) && (!defined(_MSC_VER) || _MSC_VER <= 1200)) || defined(MACOSX)
#include <stddef.h> // for size_t
// TODO: remove this from whole code! (we use std::string now)
inline size_t strnlen(const char *str, size_t maxlen)  { 
	register unsigned int i=0;
	for (;i<maxlen && str[i];i++) {}
	return i;
}
#endif




void d_printf(char *fmt, ...);




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
