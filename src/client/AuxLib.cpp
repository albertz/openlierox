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
#include <SDL.h>
#include <SDL_syswm.h>
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


SDL_PixelFormat defaultFallbackFormat = 
	{
         NULL, //SDL_Palette *palette;
         32, //Uint8  BitsPerPixel;
         4, //Uint8  BytesPerPixel;
         0, 0, 0, 0, //Uint8  Rloss, Gloss, Bloss, Aloss;
         24, 16, 8, 0, //Uint8  Rshift, Gshift, Bshift, Ashift;
         0xff000000, 0xff0000, 0xff00, 0xff, //Uint32 Rmask, Gmask, Bmask, Amask;
         0, //Uint32 colorkey;
         255 //Uint8  alpha;
	};

SDL_PixelFormat* mainPixelFormat = &defaultFallbackFormat;



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
	int SDLflags = SDL_INIT_TIMER | SDL_INIT_NOPARACHUTE;
	if(!bDedicated) {
		SDLflags |= SDL_INIT_VIDEO;
	} else {
		printf("DEDICATED MODE\n");
		bDisableSound = true;
		bJoystickSupport = false;
	}
	
	if(SDL_Init(SDLflags) == -1) {
		SystemError("Failed to initialize the SDL system!\nErrorMsg: %s",SDL_GetError());
#ifdef WIN32
		// retry it with any available video driver	
		SDL_putenv("SDL_VIDEODRIVER=");
		if(SDL_Init(SDLflags) != -1)
			printf("... but we still have success with the any driver\n");
		else
#endif		
		return false;
	}

	if(bJoystickSupport) {
		if(SDL_Init(SDL_INIT_JOYSTICK) != 0) {
			printf("WARNING: couldn't init joystick subystem: %s\n", SDL_GetError());
			bJoystickSupport = false;
		}
	}

	if(!bDedicated && !SetVideoMode())
		return false;

    // Enable the system events
    SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);

	// Enable unicode and key repeat
	SDL_EnableUNICODE(1);
	SDL_EnableKeyRepeat(200,20);


    if( !bDisableSound ) {
	    // Initialize sound
		//if(!InitSoundSystem(22050, 1, 512)) {
		if(!InitSoundSystem(44100, 1, 512)) {
		    printf("Warning: Failed the initialize the sound system\n");
			bDisableSound = true;
		}
    } 	
	if(bDisableSound) {
		printf("soundsystem completly disabled\n");
		tLXOptions->bSoundOn = false;
	}
	
	if( tLXOptions->bSoundOn ) {
		StartSoundSystem();
		SetSoundVolume( tLXOptions->iSoundVolume );
	}
	else
		StopSoundSystem();


	// Give a seed to the random number generator
	srand((unsigned int)time(NULL));

	if(!bDedicated) {
		bmpIcon = LoadImage("data/icon.png", true);
		if(bmpIcon)
			SDL_WM_SetIcon(bmpIcon, NULL);
	}

	// Initialize the keyboard & mouse
	InitEventSystem();
	
	
	return true;
}


///////////////////
// Set the video mode
int SetVideoMode(void)
{
	if(bDedicated) {
		printf("SetVideoMode: dedicated mode, ignoring\n");
		return true; // ignore this case
	}
	
	// TODO: Use DOUBLEBUF and hardware surfaces
#ifdef WIN32
	bool HardwareAcceleration = false;
#else
	bool HardwareAcceleration = true; // false;
#endif
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
	if(tLXOptions->bFullscreen)  {
		vidflags |= SDL_FULLSCREEN;
	}

	if (opengl) {
		printf("HINT: using OpenGL\n");
		vidflags |= SDL_OPENGL;
		vidflags |= SDL_OPENGLBLIT;

//#ifndef MACOSX
		short colorbitsize = (tLXOptions->iColourDepth==16) ? 5 : 8;
		SDL_GL_SetAttribute (SDL_GL_RED_SIZE,   colorbitsize);
		SDL_GL_SetAttribute (SDL_GL_GREEN_SIZE, colorbitsize);
		SDL_GL_SetAttribute (SDL_GL_BLUE_SIZE,  colorbitsize);
		// TODO: why is this commented out?
		//SDL_GL_SetAttribute (SDL_GL_ALPHA_SIZE, colorbitsize);
		//SDL_GL_SetAttribute (SDL_GL_DEPTH_SIZE, tLXOptions->iColourDepth);
//#endif
		//SDL_GL_SetAttribute (SDL_GL_ALPHA_SIZE,  8);
		//SDL_GL_SetAttribute (SDL_GL_DEPTH_SIZE, 24);
		//SDL_GL_SetAttribute (SDL_GL_BUFFER_SIZE, 32);
		SDL_GL_SetAttribute (SDL_GL_DOUBLEBUFFER, DoubleBuf);
	} 

	if(HardwareAcceleration)  {
		vidflags |= SDL_HWSURFACE | SDL_HWPALETTE | SDL_HWACCEL;
		if (tLXOptions->bFullscreen)
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

	if (!tLXOptions->bFullscreen)  {
		SubclassWindow();
	}
#endif

	// Set the change mode flag
	if (tLX)
		tLX->bVideoModeChanged = true;

	mainPixelFormat = SDL_GetVideoSurface()->format;

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


void CapFPS() {
	const float fMaxFrameTime = (tLXOptions->nMaxFPS > 0) ? (1.0f / (float)tLXOptions->nMaxFPS) : 0;
	const float fCurTime = GetMilliSeconds();
	// tLX->fCurTime is old time
	
	// Cap the FPS
	if(fCurTime - tLX->fCurTime < fMaxFrameTime)
		SDL_Delay((int)((fMaxFrameTime - fCurTime + tLX->fCurTime)*1000));
}

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
	if(psScreen == NULL) return;
	
    // Take a screenshot?
    // We do this here, because there are so many graphics loops, but this function is common
    // to all of them
    ProcessScreenshots();

	SDL_Flip( psScreen );

	if (tLXOptions->bOpenGL)
		SDL_GL_SwapBuffers();
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

	// Process the last events (mainly because of timers that will free the allocated memory)
	ProcessEvents();

	// Shutdown the SDL system
	// HINT: Sometimes we get a segfault here. Because
	// all important stuff is already closed and save here, it's not that
	// important to do any more cleanup.
	SDL_Quit();
}



///////////////////
// Return the game name
std::string GetGameName(void)
{
	return GameName;
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
