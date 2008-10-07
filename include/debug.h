/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// debug header file
// Contains tools for detecting memleaks and similar debug stuff


#ifndef __DEBUG_H__
#define __DEBUG_H__

#ifdef DEBUG

// Leak checking for MS Visual C++
#ifdef _MSC_VER
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW

#endif // _MSC_VER

#endif  // DEBUG

#endif  // __DEBUG_H__
