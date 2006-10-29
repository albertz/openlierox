/////////////////////////////////////////
//
//   Auxiliary Software class library
//
/////////////////////////////////////////


// Type definitions
// Created on 29/11/01
// By Jason Boettcher

// changed by US


#ifndef __TYPES_H__
#define __TYPES_H__

//#include <SDL/SDL_stdinc.h>
#include <SDL/SDL.h>


typedef unsigned int	uint;
typedef unsigned char	uchar;
typedef unsigned long	ulong;

// used by sound-system; TODO: replace it
typedef int HSAMPLE;
// BASS also typedefed this; TODO: this has to be changed!
//typedef char byte;

#ifndef WIN32
typedef int DWORD;
#endif


#endif  // __TYPES_H__

