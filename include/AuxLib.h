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
bool		SetVideoMode(); // only call from main thread; use doSetVideoModeInMainThread elsewhere

#ifdef WIN32
HWND		GetWindowHandle();
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
inline bool	HandleDebugCommand(const std::string& cmd) { return false; }
#endif


void doVideoFrameInMainThread();
void doSetVideoModeInMainThread();
void doActionInMainThread(Action* act);
void doVppOperation(Action* act);

void flipRealVideo();


class VideoPostProcessor {
protected:
	static SDL_Surface* m_videoSurface;
	static SDL_Surface* m_videoBufferSurface;
	static VideoPostProcessor* instance;
	
public:
	// IMPORTANT: only call these from the main thread
	static void flipBuffers() { SDL_Surface* tmp = m_videoSurface; m_videoSurface = m_videoBufferSurface; m_videoBufferSurface = tmp; }
	static void process();
	static void cloneBuffer();
	
public:
	virtual ~VideoPostProcessor() {}
	static VideoPostProcessor* get() { return instance; }
	static void init();
	static void uninit();

	virtual void resetVideo() { m_videoSurface = m_videoBufferSurface = SDL_GetVideoSurface(); } // this dummy just uses the real video surface directly; it is called from SetVideoMode
	virtual void processToScreen() {} // should process m_videoSurface to real video surface; this is run within an own thread
	virtual int screenWidth() { return 640; }
	virtual int screenHeight() { return 480; }

	static SDL_Surface* videoSurface() { return m_videoSurface; };
	static SDL_Surface* videoBufferSurface() { return m_videoBufferSurface; };
	
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
