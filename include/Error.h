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


// Error system
// Created 12/11/01
// By Jason Boettcher


#ifndef __ERROR_H__
#define	__ERROR_H__

#ifdef WIN32
#include <dbghelp.h>
#endif


void SetError(char *fmt, ...);
void ShowError(void);
void EndError(void);

void SystemError(char *fmt, ...);

void GuiSkinError(char *fmt, ...);

void LxSetLastError(char *desc);
char *LxGetLastError(void);

// WIN32 exception handling
#ifdef WIN32
LONG WINAPI CustomUnhandledExceptionFilter(PEXCEPTION_POINTERS pExInfo);
void InstallExceptionFilter(void);
#endif





#endif  //  __ERROR_H__
