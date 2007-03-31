/////////////////////////////////////////
//
//   OpenLieroX
//
//   Auxiliary Software class library
//
//   based on the work of JasonB
//   enhanced by Dark Charlie and Albert Zeyer
//
//   code under LGPL
//
/////////////////////////////////////////


// Type definitions
// Created on 29/11/01
// By Jason Boettcher

// changed by US


#ifndef __TYPES_H__
#define __TYPES_H__


typedef unsigned int	uint;
typedef unsigned char	uchar;
typedef unsigned long	ulong;
typedef unsigned char	uint24[3];

#ifndef WIN32
// WIN32 defines this in windows.h
typedef long DWORD;
typedef uchar byte;
#else  // WIN32
typedef unsigned short ushort;
#endif



#endif  // __TYPES_H__

