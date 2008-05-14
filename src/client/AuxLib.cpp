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

#ifdef _MSC_VER
#pragma warning(disable: 4996)
#endif

#include <iostream>
#include <iomanip>
#include <time.h>
#include <SDL.h>
#include <SDL_syswm.h>
#include <stdlib.h>

#include "AuxLib.h"
#include "Error.h"
#include "CServer.h"
#include "Menu.h"
#include "GfxPrimitives.h"
#include "Cache.h"
#include "FindFile.h"
#include "InputEvents.h"
#include "StringUtils.h"
#include "Sounds.h"


// Game info
std::string	GameName;

// Config file
std::string	ConfigFile;

// Screen

SmartPointer<SDL_Surface> bmpIcon=NULL;


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

using namespace std;


///////////////////
// Initialize the standard Auxiliary Library
int InitializeAuxLib(const std::string& gname, const std::string& config, int bpp, int vidflags)
{
	// Set the game info
	GameName=gname;

	ConfigFile=config;

	if(getenv("SDL_VIDEODRIVER"))
		printf("SDL_VIDEODRIVER=%s\n", getenv("SDL_VIDEODRIVER"));
	
	// Solves problem with FPS in fullscreen
#ifdef WIN32
	if(!getenv("SDL_VIDEODRIVER")) {
		printf("SDL_VIDEODRIVER not set, setting to directx\n");
		putenv("SDL_VIDEODRIVER=directx");
	}
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
		SystemError("Failed to initialize the SDL system!\nErrorMsg: " + std::string(SDL_GetError()));
#ifdef WIN32
		// retry it with any available video driver	
		unsetenv("SDL_VIDEODRIVER");
		if(SDL_Init(SDLflags) != -1)
			printf("... but we have success with the any driver\n");
		else
#endif		
		return false;
	}

	if(bJoystickSupport) {
		if(SDL_InitSubSystem(SDL_INIT_JOYSTICK) != 0) {
			printf("WARNING: couldn't init joystick subystem: %s\n", SDL_GetError());
			bJoystickSupport = false;
		}
	}

	if(!bDedicated && !SetVideoMode())
		return false;

    // Enable the system events
    SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);
	SDL_EventState(SDL_VIDEOEXPOSE, SDL_ENABLE);
	
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
	}
	else
		StopSoundSystem();


	// Give a seed to the random number generator
	srand((unsigned int)time(NULL));

	if(!bDedicated) {
		bmpIcon = LoadImage("data/icon.png", true);
		if(bmpIcon.get())
			SDL_WM_SetIcon(bmpIcon.get(), NULL);
	}

	// Initialize the keyboard & mouse
	InitEventSystem();
	
	
	return true;
}


static void DumpPixelFormat(const SDL_PixelFormat* format) {
	cout << "PixelFormat:" << endl
		<< "  BitsPerPixel: " << (int)format->BitsPerPixel << ","
		<< "  BytesPerPixel: " << (int)format->BytesPerPixel << endl
		<< "  R/G/B/A mask: " << hex
			<< (uint)format->Rmask << "/"
			<< (uint)format->Gmask << "/"
			<< (uint)format->Bmask << "/"
			<< (uint)format->Amask << endl
		<< "  R/G/B/A loss: "
			<< (uint)format->Rloss << "/"
			<< (uint)format->Gloss << "/"
			<< (uint)format->Bloss << "/"
			<< (uint)format->Aloss << endl << dec
		<< "  Colorkey: " << (uint)format->colorkey << ","
		<< "  Alpha: " << (int)format->alpha << endl;
}

///////////////////
// Set the video mode
bool SetVideoMode()
{
	if(bDedicated) {
		printf("SetVideoMode: dedicated mode, ignoring\n");
		return true; // ignore this case
	}

	// Check if already running
	if (SDL_GetVideoSurface())  {
		printf("resetting video mode\n");

		// seems to be a win-only problem, it works without problems here under MacOSX
#ifdef WIN32
		// using hw surfaces?
		if ((SDL_GetVideoSurface()->flags & SDL_HWSURFACE) != 0) {
			printf("WARNING: cannot change video mode because current mode uses hardware surfaces\n");
			// TODO: you would have to reset whole game, this is not enough!
			// The problem is in all allocated surfaces - they are hardware and when you switch
			// to window, you will most probably get software rendering
			// Also, hardware surfaces are freed from the video memory when reseting video mode
			// so you would first have to convert all surfaces to software and then perform this
			// TODO: in menu_options, restart the game also for fullscreen-change if hw surfaces are currently used
			return false;
		}
#endif
	}
	
#ifdef WIN32
	bool HardwareAcceleration = false;
#else
	// in OpenGL mode, this is faster; in non-OGL, it's faster without
	// HINT: this has probably nothing to do with hardware acceleration itself because
	// on UNIX systems only root user can run hardware accelerated applications
	bool HardwareAcceleration = tLXOptions->bOpenGL;
#endif
	int DoubleBuf = false;
	int vidflags = 0;

	// it is faster with doublebuffering in hardware accelerated mode
	// also, it seems that it's possible that there are effects in hardware mode with double buf disabled
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

		// HINT: it seems that with OGL activated, SDL_SetVideoMode will already set the OGL depth size
		// though this main pixel format of the screen surface was always 32 bit for me in OGL under MacOSX
//#ifndef MACOSX
/*
		short colorbitsize = (tLXOptions->iColourDepth==16) ? 5 : 8;
		SDL_GL_SetAttribute (SDL_GL_RED_SIZE,   colorbitsize);
		SDL_GL_SetAttribute (SDL_GL_GREEN_SIZE, colorbitsize);
		SDL_GL_SetAttribute (SDL_GL_BLUE_SIZE,  colorbitsize);
		SDL_GL_SetAttribute (SDL_GL_ALPHA_SIZE, colorbitsize);
		SDL_GL_SetAttribute (SDL_GL_DEPTH_SIZE, tLXOptions->iColourDepth);
*/
//#endif
		//SDL_GL_SetAttribute (SDL_GL_ALPHA_SIZE,  8);
		//SDL_GL_SetAttribute (SDL_GL_DEPTH_SIZE, 24);
		//SDL_GL_SetAttribute (SDL_GL_BUFFER_SIZE, 32);
		SDL_GL_SetAttribute (SDL_GL_DOUBLEBUFFER, DoubleBuf);
	} 

	if(HardwareAcceleration)  {
		vidflags |= SDL_HWSURFACE | SDL_HWPALETTE | SDL_HWACCEL;
		// Most (perhaps all) systems use software drawing for their stuff (windows etc.)
		// Because of that we cannot have hardware accelerated support in window - OS screen
		// is software surface. How would you make the window hardware, if it's on the screen?
		// Anyway, SDL takes care of this by istelf and disables the flag when needed
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

	if( SDL_SetVideoMode(640, 480, tLXOptions->iColourDepth, vidflags) == NULL) {
		SystemError("Failed to set the video mode 640x480x" + itoa(tLXOptions->iColourDepth) + "\nErrorMsg: " + std::string(SDL_GetError()));
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
	DumpPixelFormat(mainPixelFormat);
	if(SDL_GetVideoSurface()->flags & SDL_DOUBLEBUF)
		cout << "using doublebuffering" << endl;
	
	// Correct the surface format according to SDL
	if ((SDL_GetVideoSurface()->flags & SDL_HWSURFACE) != 0)  {
		iSurfaceFormat = SDL_HWSURFACE;
		printf("using hardware surfaces\n");
	} else {
		iSurfaceFormat = SDL_SWSURFACE; // HINT: under MacOSX, it doesn't seem to make any difference in performance
		if (HardwareAcceleration)
			printf("HINT: Unable to use hardware surfaces, falling back to software.\n");
		printf("using software surfaces\n");
	}

	FillSurface(SDL_GetVideoSurface(), MakeColour(0, 0, 0));

	cout << "video mode was set successfully" << endl;
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
	else
		// do at least one small break, else it's possible that we never receive signals from our OS
		SDL_Delay(1);
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
void FlipScreen( SDL_Surface * psScreen)
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
void ShutdownAuxLib()
{
	// Process the last events (mainly because of timers that will free the allocated memory)
	ProcessEvents();
	
	// free all cached stuff like surfaces and sounds
	// HINT: we have to do it before we uninit the specific engines
	cCache.Clear();
	
	// quit video at this point to not get stuck in a fullscreen not responding game in case that it crashes in further quitting
	// in the case it wasn't inited at this point, this also doesn't hurt
	SDL_QuitSubSystem( SDL_INIT_VIDEO );

	QuitSoundSystem();

	// Shutdown the error system
	EndError();

#ifdef WIN32
	UnSubclassWindow();
#endif

	// Shutdown the SDL system
	// HINT: Sometimes we get a segfault here. Because
	// all important stuff is already closed and save here, it's not that
	// important to do any more cleanup.
#if SDLMIXER_WORKAROUND_RESTART == 1
	if(bRestartGameAfterQuit)
		// quit everything but audio
		SDL_QuitSubSystem( SDL_WasInit(0) & (~SDL_INIT_AUDIO) );
	else
#endif		
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
	SaveSurface(GetVideoSurface(), fullname, tLXOptions->iScreenshotFormat, additional_data);
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

//////////////////////
// unsetenv for WIN32, taken from libc source
int unsetenv(const char *name)  
{
  size_t len;
  char **ep;

  if (name == NULL || *name == '\0' || strchr (name, '=') != NULL)
    {
      return -1;
    }

  len = strlen (name);

  ep = _environ;
  while (*ep != NULL)
    if (!strncmp (*ep, name, len) && (*ep)[len] == '=')
      {
	/* Found it.  Remove this pointer by moving later ones back.  */
	char **dp = ep;

	do
	  dp[0] = dp[1];
	while (*dp++);
	/* Continue the loop in case NAME appears again.  */
      }
    else
      ++ep;

  return 0;
}
#endif
