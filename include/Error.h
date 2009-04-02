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

#include <string>

void SetError(const std::string& text);
void ShowError();
void EndError();

void SystemError(const std::string&text);

void GuiSkinError(const std::string& text);

void LxSetLastError(const std::string& desc);
std::string LxGetLastError();



#endif  //  __ERROR_H__
