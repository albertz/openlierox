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


#include "SmartPointer.h"


// Routines
int			InitializeAuxLib(const std::string& config, int bpp, int vidflags);
void		ShutdownAuxLib();
bool		SetVideoMode(); // only call from main thread; use doSetVideoModeInMainThread elsewhere

#ifdef WIN32
void		*GetWindowHandle();
#endif

void        FlipScreen();
void		CapFPS();

char*		GetAppPath();

std::string GetConfigFile();


void		PushScreenshot(const std::string& dir, const std::string& data);
void		ProcessScreenshots();

void		SetCrashHandlerReturnPoint(const char* name);
void		OpenLinkInExternBrowser(const std::string& url);
void		setCurThreadName(const std::string& name);
void		setCurThreadPriority(float p); // p in [-1,1], whereby 0 is standard

size_t		GetFreeSysMemory(); // returnes available physical memory in bytes
std::string	GetDateTimeText();	// Returns human-readable time
std::string	GetDateTimeFilename(); // Returns time for use in filename, so newer files will get alpha-sorted last

#ifdef DEBUG
bool		HandleDebugCommand(const std::string& cmd);
#else
#define		HandleDebugCommand(cmd) (false)
#endif


void doVideoFrameInMainThread();
void doSetVideoModeInMainThread();
void doActionInMainThread(Action* act);
void doVppOperation(Action* act);

void flipRealVideo();

// Asynchronously enable/disable mouse cursor in window manager, may be called from any thread
// Use this function instead of SDL_ShowCursor()
void EnableSystemMouseCursor(bool enable = true);

class VideoPostProcessor {
protected:
	SmartPointer<SDL_Window> m_window;
	SmartPointer<SDL_Renderer> m_renderer;
	SmartPointer<SDL_Texture> m_videoTexture;
	SmartPointer<SDL_Surface> m_videoSurface;
	static VideoPostProcessor instance;
	
public:
	// IMPORTANT: only call these from the main thread
	static void process();
	static void cloneBuffer();
	
public:
	static VideoPostProcessor* get() { return &instance; }
	static void uninit();

	bool initWindow();
	bool resetVideo(); // this is called from SetVideoMode
	void render();
	
	int screenWidth() { return 640; }
	int screenHeight() { return 480; }

	static SDL_Surface* videoSurface() { return get()->m_videoSurface.get(); };
	
	static void transformCoordinates_ScreenToVideo( int& x, int& y );
};


// Subclass
#ifdef WIN32
void		SubclassWindow();
void		UnSubclassWindow();
#endif

#ifdef WIN32
int unsetenv(const char *name);
#endif


#endif  //  __AUXLIB_H__
