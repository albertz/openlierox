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


///////////////////
// Flip the screen
void FlipScreen(SDL_Surface *psScreen)
{
    // Take a screenshot?
    // We do this here, because there are so many graphics loops, but this function is common
    // to all of them
    if( cTakeScreenshot.isDownOnce())
        TakeScreenshot(tGameInfo.bTournament);

	// TODO: this better
	if (cServer)  // Automatic screenshot after the game ends
		if (cServer->getTakeScreenshot() )
			TakeScreenshot(tGameInfo.bTournament);

	if (tLXOptions->bOpenGL)  {
		SDL_GL_SwapBuffers();
	}
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
void TakeScreenshot(bool Tournament)
{
	static std::string	picname;
	static std::string	fullname;
	static std::string	extension;
	int			i;

	// Set the extension
	switch (tLXOptions->iScreenshotFormat)  {
	case FMT_BMP: extension = ".bmp"; break;
	case FMT_PNG: extension = ".png"; break;
	case FMT_JPG: extension = ".jpg"; break;
	case FMT_GIF: extension = ".gif"; break;
	default: extension = ".png";
	}

	// Create the file name
    for(i=0; 1; i++) {
		picname = "lierox"+itoa(i)+extension;

		if (Tournament)
			fullname = "tourny_scrshots/"+picname;
		else
			fullname = "scrshots/" + picname;

		if (!IsFileAvailable(fullname,false))
			break;	// file doesn't exist
	}

	// Save the surface
	SaveSurface(SDL_GetVideoSurface(),fullname,tLXOptions->iScreenshotFormat,Tournament && cServer->getTakeScreenshot());
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
