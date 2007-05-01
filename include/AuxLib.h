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

#include "Unicode.h"


// Initialization sub-systems
#define		AUX_INIT_SDL		0
#define		AUX_INIT_2DVIDEO	1
#define		AUX_INIT_3DVIDEO	2
#define		AUX_INIT_AUDIO		3
#define		AUX_INIT_NETWORK	4
#define		AUX_INIT_TIMER		5
#define		AUX_INIT_GUI		6


#define     MAX_KEYQUEUE        32


class KeyboardEvent {
public:
    UnicodeChar ch;
	bool down;
};

// Keyboard structure
class keyboard_t { public:
	Uint8	*keys;
	UnicodeChar	KeyUp[SDLK_LAST];
	UnicodeChar	KeyDown[SDLK_LAST];
    int     queueLength;
    KeyboardEvent keyQueue[MAX_KEYQUEUE];
};


// Mouse structure
class mouse_t { public:
	int		X,Y;
	int		Button;

	int		Up;
	int		Down;
    int     FirstDown;

	int		WheelUp;
	int		WheelDown;
	int		WheelScrollUp;
	int		WheelScrollDown;
};

// Super global variables
//int iRecordingVideo;



// Routines
int			InitializeAuxLib(const std::string& gname, const std::string& config, int bpp, int vidflags);
void		ShutdownAuxLib(void);
int			SetVideoMode(void);

#ifdef WIN32
HWND		GetWindowHandle(void);
#endif

void		ProcessEvents(void);
void        FlipScreen(SDL_Surface *psScreen);
keyboard_t	*GetKeyboard(void);
mouse_t		*GetMouse(void);
SDL_Event	*GetEvent(void);

std::string	GetGameName(void);

std::string GetConfigFile(void);

std::string GetClipboardText();
int         SetClipboardText(const std::string& szText);
void        TakeScreenshot(bool Tournament);

// Subclass
#ifdef WIN32
extern WNDPROC wpOriginal;
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void		SubclassWindow(void);
void		UnSubclassWindow(void);
#endif


#endif  //  __AUXLIB_H__
