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


// Auxiliary library
// Created 12/11/01
// By Jason Boettcher

#include <time.h>
#include <SDL/SDL_syswm.h>
#include "LieroX.h"
#include "AuxLib.h"
#include "Error.h"
#include "CServer.h"
#include "Menu.h"
#include "GfxPrimitives.h"
#include "Cache.h"
#include "FindFile.h"
#include "InputEvents.h"
#include "StringUtils.h"


// Game info
std::string	GameName;

// Config file
std::string	ConfigFile;

// Screen

SDL_Surface *bmpIcon=NULL;

///////////////////
// Initialize the standard Auxiliary Library
int InitializeAuxLib(const std::string& gname, const std::string& config, int bpp, int vidflags)
{
	// Set the game info
	GameName=gname;

	ConfigFile=config;

	// Solves problem with FPS in fullscreen
#ifdef WIN32
	SDL_putenv("SDL_VIDEODRIVER=directx");
#endif

	// Initialize SDL
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_NOPARACHUTE) == -1) {
		SystemError("Failed to initialize the SDL system!\nErrorMsg: %s",SDL_GetError());
#ifdef WIN32
		// retry it with any available video driver	
		SDL_putenv("SDL_VIDEODRIVER=");
		if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_NOPARACHUTE) != -1)
			printf("... but we still have success with the any driver\n");
		else
#endif		
		return false;
	}


	if(!SetVideoMode())
		return false;

    // Enable the system events
    SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);

	// Enable unicode and key repeat
	SDL_EnableUNICODE(1);
	SDL_EnableKeyRepeat(200,20);


    if( !nDisableSound ) {
	    // Initialize sound
		//if(!InitSoundSystem(22050, 1, 512)) {
		if(!InitSoundSystem(44100, 1, 512)) {
		    printf("Warning: Failed the initialize the sound system\n");
		}
    }

	if( tLXOptions->iSoundOn ) {
		StartSoundSystem();
		SetSoundVolume( tLXOptions->iSoundVolume );
	}
	else
		StopSoundSystem();


	// Give a seed to the random number generator
	srand((unsigned int)time(NULL));


	// Initialize the cache
	if(!InitializeCache())
		return false;

	bmpIcon = LoadImage("data/icon.png", true);
	if(bmpIcon)
		SDL_WM_SetIcon(bmpIcon, NULL);


	// Initialize the keyboard & mouse
	ProcessEvents();
	for(int k = 0;k<SDLK_LAST;k++)
		GetKeyboard()->KeyUp[k] = false;

	GetMouse()->Button = 0;
	GetMouse()->Down = 0;
	GetMouse()->FirstDown = 0;
	GetMouse()->Up = 0;

	return true;
}


///////////////////
// Set the video mode
int SetVideoMode(void)
{
	// TODO: Use DOUBLEBUF and hardware surfaces
	bool HardwareAcceleration = false;
	int DoubleBuf = false;
	int vidflags = 0;

	// Use doublebuf when hardware accelerated
	if (HardwareAcceleration)
		DoubleBuf = true;

	// Check that the bpp is valid
	switch (tLXOptions->iColourDepth) {
	case 0:
	case 16:
	case 24:
	case 32:
		break;
	default: tLXOptions->iColourDepth = 16;
	}
	printf("ColorDepth: %i\n", tLXOptions->iColourDepth);

	// BlueBeret's addition (2007): OpenGL support
	bool opengl = tLXOptions->bOpenGL;

	// Initialize the video
	if(tLXOptions->iFullscreen)  {
		vidflags |= SDL_FULLSCREEN;
	}

	if (opengl) {
		printf("HINT: using OpenGL\n");
		vidflags |= SDL_OPENGL;
		vidflags |= SDL_OPENGLBLIT;

#ifndef MACOSX
		short colorbitsize = (tLXOptions->iColourDepth==16) ? 5 : 8;
		SDL_GL_SetAttribute (SDL_GL_RED_SIZE,   colorbitsize);
		SDL_GL_SetAttribute (SDL_GL_GREEN_SIZE, colorbitsize);
		SDL_GL_SetAttribute (SDL_GL_BLUE_SIZE,  colorbitsize);
		// TODO: why is this commented out?
		//SDL_GL_SetAttribute (SDL_GL_ALPHA_SIZE, colorbitsize);
		//SDL_GL_SetAttribute (SDL_GL_DEPTH_SIZE, tLXOptions->iColourDepth);
#endif
		//SDL_GL_SetAttribute (SDL_GL_ALPHA_SIZE,  8);
		//SDL_GL_SetAttribute (SDL_GL_DEPTH_SIZE, 24);
		//SDL_GL_SetAttribute (SDL_GL_BUFFER_SIZE, 32);
		SDL_GL_SetAttribute (SDL_GL_DOUBLEBUFFER, DoubleBuf);
	} 

	if(HardwareAcceleration)  {
		vidflags |= SDL_HWSURFACE | SDL_HWPALETTE | SDL_HWACCEL;
		if (tLXOptions->iFullscreen)
			iSurfaceFormat = SDL_HWSURFACE;
	}
	else  {
		vidflags |= SDL_SWSURFACE;
		iSurfaceFormat = SDL_SWSURFACE;
	}

	if(DoubleBuf && !opengl)
		vidflags |= SDL_DOUBLEBUF;

#ifdef WIN32
	UnSubclassWindow();  // Unsubclass before doing anything with the window
#endif


#ifdef WIN32
	// Reset the video subsystem under WIN32, else we get a "Could not reset OpenGL context" error when switching mode
	if (opengl && tLX)  {  // Don't reset when we're setting up the mode for first time (OpenLieroX not yet initialized)
		SDL_QuitSubSystem(SDL_INIT_VIDEO);
		SDL_InitSubSystem(SDL_INIT_VIDEO);
	}
#endif

	if( SDL_SetVideoMode(640,480, tLXOptions->iColourDepth,vidflags) == NULL) {
		SystemError("Failed to set the video mode %dx%dx%d\nErrorMsg: %s", 640, 480, tLXOptions->iColourDepth,SDL_GetError());
		return false;
	}

	SDL_WM_SetCaption(GameName.c_str(),NULL);
	SDL_ShowCursor(SDL_DISABLE);

#ifdef WIN32
	// Hint: Reset the mouse state - this should avoid the mouse staying pressed
	GetMouse()->Button = 0;
	GetMouse()->Down = 0;
	GetMouse()->FirstDown = 0;
	GetMouse()->Up = 0;

	if (!tLXOptions->iFullscreen)  {
		SubclassWindow();
	}
#endif

	// Set the change mode flag
	if (tLX)
		tLX->bVideoModeChanged = true;

	return true;
}

#ifdef WIN32
//////////////////////
// Get the window handle
HWND GetWindowHandle(void)
{
	SDL_SysWMinfo info;
	SDL_VERSION(&info.version);
	if(!SDL_GetWMInfo(&info))
		return 0;

	return info.window;
}
#endif


////////////////
// Process any screenshots
void ProcessScreenshots()
{
	// Check if user pressed screenshot key
	if (cTakeScreenshot.isDownOnce())  {
		screenshot_t scr;
		scr.sData = "";
		scr.sDir = "scrshots";
		tLX->tScreenshotQueue.push_back(scr);
	}

	// Process all the screenhots in the queue
	for (std::list<screenshot_t>::iterator it = tLX->tScreenshotQueue.begin(); it != tLX->tScreenshotQueue.end(); it++)  {
		TakeScreenshot(it->sDir, it->sData);
	}

	// Clear the queue
	tLX->tScreenshotQueue.clear();
}


///////////////////
// Flip the screen
void FlipScreen(SDL_Surface *psScreen)
{
    // Take a screenshot?
    // We do this here, because there are so many graphics loops, but this function is common
    // to all of them
    ProcessScreenshots();

	if (tLXOptions->bOpenGL)
		SDL_GL_SwapBuffers();

/*
	TODO: application crashed here
	I supose a problem with the screen-surface.
	
	...
	Sending HTTP request lieroxtreme.thegaminguniverse.com/server/svr_list.php
HTTP ERROR: Could not resolve DNS (The address or port are not valid.)
Sending HTTP request nspire.fw.nu/lierox/server/svr_list.php
./start.sh: line 24:  9693 Speicherzugriffsfehler  (core dumped) ../../bin/openlierox
ERROR detected, printing core-info ...
Using host libthread_db library "/lib/libthread_db.so.1".

warning: Can't read pathname for load map: Input/output error.
Core was generated by `../../bin/openlierox'.
Program terminated with signal 11, Segmentation fault.
#0  malloc_consolidate (av=0xb7c71120) at malloc.c:4819
4819    malloc.c: No such file or directory.
        in malloc.c

Thread 1 (process 9693):
#0  malloc_consolidate (av=0xb7c71120) at malloc.c:4819
        fb = (mfastbinptr *) 0xb7c71138
        p = (mchunkptr) 0x2
        nextp = (mchunkptr) 0x2
        unsorted_bin = (mchunkptr) 0xb7c71150
        first_unsorted = <value optimized out>
        nextchunk = (mchunkptr) 0x8b2eba8
        size = 584
        nextsize = 16
        prevsize = <value optimized out>
        bck = <value optimized out>
        fwd = (mchunkptr) 0x248
#1  0xb7bb0cd1 in _int_free (av=0xb7c71120, mem=0x85639f8) at malloc.c:4715
        heap = <value optimized out>
        p = (mchunkptr) 0x8557cd8
        size = 84712
        nextchunk = (mchunkptr) 0x856c7c0
        nextsize = 12104
        prevsize = <value optimized out>
        bck = <value optimized out>
        fwd = <value optimized out>
        errstr = <value optimized out>
#2  0xb7bb0f0f in *__GI___libc_free (mem=0x85639f8) at malloc.c:3622
        ar_ptr = (mstate) 0xb7c71120
        p = (mchunkptr) 0x248
        hook = <value optimized out>
#3  0xb6f50b98 in _mesa_free (ptr=0x85639f8) at main/imports.c:116
No locals.
#4  0xb6fa4130 in free_texgen_data (stage=0x8473890) at tnl/t_vb_texgen.c:600
        store = (struct texgen_stage_data *) 0x8504ec8
        i = 8
#5  0xb6f8f2e4 in _tnl_destroy_pipeline (ctx=0x822d470) at tnl/t_pipeline.c:68
        tnl = (TNLcontext *) 0x846f370
        i = 5
#6  0xb6f8f124 in _tnl_DestroyContext (ctx=0x822d470) at tnl/t_context.c:136
No locals.
#7  0xb6f000e8 in intelDestroyContext (driContextPriv=0x8221078)
    at intel_context.c:425
        __PRETTY_FUNCTION__ = "intelDestroyContext"
#8  0xb6ee6472 in driDestroyContext (dpy=0x8206068, scrn=0, 
    contextPrivate=0x8221078) at ../common/dri_util.c:732
No locals.
#9  0xb7109140 in DestroyContext (dpy=0x8206068, gc=0x8222710) at glxcmds.c:472
        xid = 71303170
        opcode = 142 '\216'
        imported = 0 '\0'
#10 0xb7f9cd16 in X11_GL_Shutdown (this=0x81ff4f8)
    at ./src/video/x11/SDL_x11gl.c:305
No locals.
#11 0xb7fa0960 in X11_DestroyWindow (this=0x81ff4f8, screen=0x248)
    at ./src/video/x11/SDL_x11video.c:576
No locals.
#12 0xb7fa0b65 in X11_VideoQuit (this=0x81ff4f8)
    at ./src/video/x11/SDL_x11video.c:1352
No locals.
#13 0xb7f90ac3 in SDL_VideoQuit () at ./src/video/SDL_video.c:1347
        ready_to_go = <value optimized out>
#14 0xb7f667a6 in SDL_QuitSubSystem (flags=65535) at ./src/SDL.c:200
No locals.
#15 0xb7f667cf in SDL_Quit () at ./src/SDL.c:220
No locals.
#16 0xb7f66f41 in SDL_Parachute (sig=11) at ./src/SDL_fatal.c:41
No locals.
#17 <signal handler called>
No symbol table info available.
#18 _mesa_PopAttrib () at main/attrib.c:1047
        point = (const struct gl_point_attrib *) 0x428b4945
        attr = (struct gl_attrib_node *) 0xaf7007a8
        next = <value optimized out>
        ctx = (GLcontext *) 0x822d470
#19 0xb7f8ff88 in SDL_GL_Unlock () at ./src/video/SDL_video.c:1669
        this = (SDL_VideoDevice *) 0x81ff4f8
#20 0xb7f8ffbd in SDL_GL_UpdateRectsLock (this=0x81ff4f8, numrects=1, 
    rects=0xbfd4aaa4) at ./src/video/SDL_video.c:1530
No locals.
#21 0xb7f90d4d in SDL_UpdateRects (screen=0x8765898, numrects=1, 
    rects=0xbfd4aaa4) at ./src/video/SDL_video.c:1073
        pal = (SDL_Palette *) 0x0
        saved_colors = (SDL_Color *) 0xbfd4aa88
        i = <value optimized out>
        video = (SDL_VideoDevice *) 0x81ff4f8
#22 0xb7f90e9e in SDL_UpdateRect (screen=0x8765898, x=0, y=0, w=0, 
    h=2943354792) at ./src/video/SDL_video.c:1016
        rect = {x = 0, y = 0, w = 640, h = 480}
#23 0xb7f9134e in SDL_Flip (screen=0xaf7007a8) at ./src/video/SDL_video.c:1133
        video = (SDL_VideoDevice *) 0x81ff4f8
#24 0x08107392 in FlipScreen (psScreen=0x8765898)
    at /home/az/Programmierung/openlierox/src/client/AuxLib.cpp:279
No locals.
#25 0x080e4c6a in Menu_Loop ()
    at /home/az/Programmierung/openlierox/src/client/MenuSystem.cpp:346
        oldtime = 17.644001
        fMaxFrameTime = 0.00999999978
#26 0x080e4cce in Menu_Start ()
    at /home/az/Programmierung/openlierox/src/client/MenuSystem.cpp:267
No locals.
#27 0x08091823 in main (argc=1, argv=0xbfd4ac64)
    at /home/az/Programmierung/openlierox/src/main.cpp:256
        oldtime = -1.66149235
        fMaxFrameTime = -2.37226268e-05
        startgame = false
        slashpos = 9
Current language:  auto; currently c


*/
    SDL_Flip( psScreen );
}


///////////////////
// Shutdown the standard Auxiliary Library
void ShutdownAuxLib(void)
{
	QuitSoundSystem();

	// Shutdown the error system
	EndError();

#ifdef WIN32
	UnSubclassWindow();
#endif

	// Shutdown the cache
	ShutdownCache();

	// Shutdown the SDL system
	// this is a (non-working) workaround to prevent the default segfault-routine
	try { SDL_Quit(); }
	catch(...) {
		printf("WARNING: ERROR while shutting down SDL\n");
	}
}



///////////////////
// Return the game name
std::string GetGameName(void)
{
	return GameName;
}


///////////////////
// Get text from the clipboard
// Returns the length of the text (0 for no text)
std::string GetClipboardText() {
#ifdef WIN32
	std::string szText;
    
    // Get the window handle
	SDL_SysWMinfo info;
	SDL_VERSION(&info.version);
	if(!SDL_GetWMInfo(&info))
		return "";

	HWND    WindowHandle = info.window;
    HANDLE  CBDataHandle; // handle to the clipboard data
    LPSTR   CBDataPtr;    // pointer to data to send

    // Windows version
    if( IsClipboardFormatAvailable(CF_TEXT) ) {

        if( OpenClipboard(WindowHandle) ) {
            CBDataHandle = GetClipboardData(CF_TEXT);

            if(CBDataHandle) {
                CBDataPtr = (LPSTR)GlobalLock(CBDataHandle);
                szText = CBDataPtr;

                GlobalUnlock(CBDataHandle);
                CloseClipboard();

                return szText;
            }
        }
    }
#else
	// TODO: how to do on linux?
#endif

    return "";
}

///////////////////
// Set text to the clipboard
// Returns the length of the text (0 for no text)
int SetClipboardText(const std::string& szText)
{
#ifdef WIN32
    // Get the window handle
	SDL_SysWMinfo info;
	SDL_VERSION(&info.version);
	if(!SDL_GetWMInfo(&info))
		return 0;

	HWND    WindowHandle = info.window;

	// Open clipboard
	if(!OpenClipboard(WindowHandle))
		return 0;

	// Empty clipboard
	EmptyClipboard();


	// Allocate memory
	LPTSTR  lptstrCopy;
	size_t cch = szText.size();
	HGLOBAL hglbCopy = GlobalAlloc(GMEM_SHARE | GMEM_MOVEABLE, (cch + 1) * sizeof(TCHAR));
    if (hglbCopy == NULL)
    {
        CloseClipboard();
        return 0;
    }

	// Copy the text to the memory
    lptstrCopy = (char *) GlobalLock(hglbCopy);
    memcpy(lptstrCopy, szText.c_str(), cch * sizeof(TCHAR));
    lptstrCopy[cch] = (TCHAR) 0;    // null character
    GlobalUnlock(hglbCopy);

	// Put to clipboard
	SetClipboardData(CF_TEXT, hglbCopy);

	// Close the clipboard
	CloseClipboard();

    return (int)szText.size();
#else
	// TODO: what is with linux here?
	return 0;
#endif
}

///////////////////
// Return the config filename
std::string GetConfigFile(void)
{
	return ConfigFile;
}


///////////////////
// Take a screenshot
void TakeScreenshot(const std::string& scr_path, const std::string& additional_data)
{
	if (scr_path.empty()) // Check
		return;

	std::string	picname;
	std::string	fullname;
	std::string	extension;
	std::string path = scr_path;

	// Append a slash if not present
	if (path[path.size() - 1] != '/' && path[path.size() - 1] != '\\')  {
		path += '/';
	}

	// Set the extension
	switch (tLXOptions->iScreenshotFormat)  {
	case FMT_BMP: extension = ".bmp"; break;
	case FMT_PNG: extension = ".png"; break;
	case FMT_JPG: extension = ".jpg"; break;
	case FMT_GIF: extension = ".gif"; break;
	default: extension = ".png";
	}

	// Create the file name
    for(unsigned int i=0; true; i++) {
		picname = "lierox" + itoa(i) + extension;

		fullname = path + picname;

		if (!IsFileAvailable(fullname,false))
			break;	// file doesn't exist
	}

	// Save the surface
	SaveSurface(SDL_GetVideoSurface(), fullname, tLXOptions->iScreenshotFormat, additional_data);
}

#ifdef WIN32
LONG wpOriginal;
bool Subclassed = false;

////////////////////
// Subclass the window (control the incoming Windows messages)
void SubclassWindow(void)
{
	if (Subclassed)
		return;

#pragma warning(disable:4311)  // Temporarily disable, the typecast is OK here
	wpOriginal = SetWindowLong(GetWindowHandle(),GWL_WNDPROC,(LONG)(&WindowProc));
#pragma warning(default:4311) // Enable the warning
	Subclassed = true;
}

////////////////////
// Remove the subclassing
void UnSubclassWindow(void)
{
	if (!Subclassed)
		return;

	SetWindowLong(GetWindowHandle(),GWL_WNDPROC, wpOriginal);

	Subclassed = false;
}

/////////////////////
// Subclass callback
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// Ignore the unwanted messages
	switch (uMsg)  {
	case WM_ENTERMENULOOP:
		return 0;
	case WM_INITMENU:
		return 0;
	case WM_MENUSELECT:
		return 0;
	case WM_SYSKEYUP:
		return 0;
	}

#pragma warning(disable:4312)
	return CallWindowProc((WNDPROC)wpOriginal,hwnd,uMsg,wParam,lParam);
#pragma warning(default:4312)
}
#endif
