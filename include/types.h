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

typedef unsigned long DWORD;
typedef uchar byte;
typedef unsigned short ushort;

// Class for having a null point, to allow overwriting a function which takes null.
// HINT: Don't ever use this class for something different. Every use of this function
// interprets it to have this parameter null.
class Null {}; extern Null null;

#endif  // __TYPES_H__

