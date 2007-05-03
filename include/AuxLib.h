/////////////////////////////////////////
//
//   OpenLieroX
//
//   Auxiliary Software class library
//
//   work by JasonB
//   code under LGPL
//   enhanced by Dark Charlie and Albert Zeyer
//
/////////////////////////////////////////


// Auxiliary Library
// Created 12/11/01
// By Jason Boettcher


#ifndef __AUXLIB_H__
#define __AUXLIB_H__

#ifdef WIN32
#include <windows.h>
#endif

// Initialization sub-systems
#define		AUX_INIT_SDL		0
#define		AUX_INIT_2DVIDEO	1
#define		AUX_INIT_3DVIDEO	2
#define		AUX_INIT_AUDIO		3
#define		AUX_INIT_NETWORK	4
#define		AUX_INIT_TIMER		5
#define		AUX_INIT_GUI		6



// Routines
int			InitializeAuxLib(const UCString& gname, const UCString& config, int bpp, int vidflags);
void		ShutdownAuxLib(void);
int			SetVideoMode(void);

#ifdef WIN32
HWND		GetWindowHandle(void);
#endif

void        FlipScreen(SDL_Surface *psScreen);

UCString	GetGameName(void);

UCString GetConfigFile(void);

UCString GetClipboardText();
int         SetClipboardText(const UCString& szText);
void        TakeScreenshot(bool Tournament);

// Subclass
#ifdef WIN32
extern WNDPROC wpOriginal;
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void		SubclassWindow(void);
void		UnSubclassWindow(void);
#endif


#endif  //  __AUXLIB_H__
