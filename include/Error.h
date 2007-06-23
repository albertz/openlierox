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

#include <string>

void SetError(char *fmt, ...);
void ShowError(void);
void EndError(void);

void SystemError(char *fmt, ...);

void GuiSkinError(char *fmt, ...);

/*void Hint(const std::string& text) {}
void Debug(const std::string& text) {}
void Warning(const std::string& text) {}
void Error(const std::string& text) {}
void FatalError(const std::string& text) {}*/

void LxSetLastError(const std::string& desc);
std::string LxGetLastError(void);

// WIN32 exception handling
#ifdef _MSC_VER
LONG WINAPI CustomUnhandledExceptionFilter(PEXCEPTION_POINTERS pExInfo);
void InstallExceptionFilter(void);
#endif





#endif  //  __ERROR_H__
