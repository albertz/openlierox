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

#include <SDL.h>
#include <string>

#ifdef WIN32
#include <windows.h>
#endif

#include "SmartPointer.h"


// Routines
int			InitializeAuxLib(const std::string& config, int bpp, int vidflags);
void		ShutdownAuxLib();
bool		SetVideoMode();

#ifdef WIN32
HWND		GetWindowHandle();
#endif

void        FlipScreen();
void		CapFPS();

std::string	GetGameName();
char*		GetAppPath();

std::string GetConfigFile();

void		ProcessScreenshots();
void        TakeScreenshot(const std::string& scr_path, const std::string& additional_data);


class VideoPostProcessor {
protected:
	static SDL_Surface* m_videoSurface;
	static VideoPostProcessor* instance;
	static void flipRealVideo();

public:
	virtual ~VideoPostProcessor() {}
	static VideoPostProcessor* get() { return instance; }
	static void init();
	static void uninit();

	virtual void resetVideo() { m_videoSurface = SDL_GetVideoSurface(); } // this dummy just uses the real video surface directly; it is called from SetVideoMode
	virtual void processToScreen() {} // should process m_videoSurface to real video surface
	virtual int screenWidth() { return 640; }
	virtual int screenHeight() { return 480; }

	static SDL_Surface* videoSurface() { return m_videoSurface; };
	static void process() { ProcessScreenshots(); get()->processToScreen(); flipRealVideo(); }

	static void transformCoordinates_ScreenToVideo( int& x, int& y );
};


// Subclass
#ifdef WIN32
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void		SubclassWindow();
void		UnSubclassWindow();
#endif

#ifdef WIN32
int unsetenv(const char *name);
#endif


#endif  //  __AUXLIB_H__
