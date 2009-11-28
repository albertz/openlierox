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


#include <iomanip>
#include <time.h>
#include <SDL.h>
#include <SDL_syswm.h>
#ifdef REAL_OPENGL
#include <SDL_opengl.h>
#endif
#include <cstdlib>
#include <sstream>
#include <cstring>

#if defined(__APPLE__)
#include <mach/host_info.h>
#include <mach/mach_host.h>
#include <mach/mach_init.h>
#include <sys/sysctl.h>
#include <mach/mach_traps.h>
#elif defined(WIN32) || defined(WIN64)
#include <windows.h>
#else
#include <cstdio>
#include <unistd.h>
#endif

#ifdef __FREEBSD__
#include <sys/sysctl.h>
#include <vm/vm_param.h>
#include <sys/vmmeter.h>
#endif

#ifndef WIN32
#include <sys/types.h>
#include <sys/resource.h>
#include <unistd.h>
#endif

#include "Cache.h"
#include "Debug.h"
#include "AuxLib.h"
#include "Error.h"
#include "DeprecatedGUI/Menu.h"
#include "GfxPrimitives.h"
#include "FindFile.h"
#include "InputEvents.h"
#include "StringUtils.h"
#include "Sounds.h"
#include "Version.h"
#include "Timer.h"
#include "types.h"
#include "CClient.h"
#include "CServer.h"
#include "Geometry.h"


Null null;	// Used in timer class

// Config file
std::string	ConfigFile;

// Screen

SDL_Surface* videoSurface = NULL;


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
int InitializeAuxLib(const std::string& config, int bpp, int vidflags)
{
	// We have already loaded all options from the config file at this time.

#ifdef linux
	//XInitThreads();	// We should call this before any SDL video stuff and window creation
#endif


	ConfigFile=config;

	if(getenv("SDL_VIDEODRIVER"))
		notes << "SDL_VIDEODRIVER=" << getenv("SDL_VIDEODRIVER") << endl;

	// Solves problem with FPS in fullscreen
#ifdef WIN32
	if(!getenv("SDL_VIDEODRIVER")) {
		notes << "SDL_VIDEODRIVER not set, setting to directx" << endl;
		putenv((char*)"SDL_VIDEODRIVER=directx");
	}
#endif

	// Initialize SDL
	int SDLflags = SDL_INIT_TIMER | SDL_INIT_NOPARACHUTE;
	if(!bDedicated) {
		SDLflags |= SDL_INIT_VIDEO;
	} else {
		hints << "DEDICATED MODE" << endl;
		bDisableSound = true;
		bJoystickSupport = false;
	}

	if(SDL_Init(SDLflags) == -1) {
		errors << "Failed to initialize the SDL system!\nErrorMsg: " << std::string(SDL_GetError()) << endl;
#ifdef WIN32
		// retry it with any available video driver
		unsetenv("SDL_VIDEODRIVER");
		if(SDL_Init(SDLflags) != -1)
			hints << "... but we have success with the any driver" << endl;
		// retry with windib
		else if(putenv((char*)"SDL_VIDEODRIVER=windib") == 0 && SDL_Init(SDLflags) != -1)
			hints << "... but we have success with the windib driver" << endl;
		else
#endif
		return false;
	}

#ifndef DISABLE_JOYSTICK
	if(bJoystickSupport) {
		if(SDL_InitSubSystem(SDL_INIT_JOYSTICK) != 0) {
			warnings << "WARNING: couldn't init joystick subystem: " << SDL_GetError() << endl;
			bJoystickSupport = false;
		}
	}
#endif

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
		    warnings << "Failed the initialize the sound system" << endl;
			bDisableSound = true;
		}
    }
	if(bDisableSound) {
		notes << "soundsystem completly disabled" << endl;
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
		SmartPointer<SDL_Surface> bmpIcon = LoadGameImage("data/icon.png", true);
		if(bmpIcon.get())
			SDL_WM_SetIcon(bmpIcon.get(), NULL);
	}

	InitEventQueue();
	
	// Initialize the keyboard & mouse
	InitEventSystem();

	// Initialize timers
	InitializeTimers();

#ifdef DEBUG
	// Cache
	InitCacheDebug();
#endif


	return true;
}


#ifdef REAL_OPENGL
static void OGL_init();
#endif

///////////////////
// Set the video mode
bool SetVideoMode()
{
	if(bDedicated) {
		notes << "SetVideoMode: dedicated mode, ignoring" << endl;
		return true; // ignore this case
	}

	if (!tLXOptions)  {
		warnings << "SetVideoMode: Don't know what video mode to set, ignoring" << endl;
		return false;
	}

	bool resetting = false;

	// Check if already running
	if (VideoPostProcessor::videoSurface())  {
		resetting = true;
		notes << "resetting video mode" << endl;

		// seems to be a win-only problem, it works without problems here under MacOSX
#ifdef WIN32
		// using hw surfaces?
		if ((VideoPostProcessor::videoSurface()->flags & SDL_HWSURFACE) != 0) {
			warnings << "cannot change video mode because current mode uses hardware surfaces" << endl;
			// TODO: you would have to reset whole game, this is not enough!
			// The problem is in all allocated surfaces - they are hardware and when you switch
			// to window, you will most probably get software rendering
			// Also, hardware surfaces are freed from the video memory when reseting video mode
			// so you would first have to convert all surfaces to software and then perform this
			// TODO: in menu_options, restart the game also for fullscreen-change if hw surfaces are currently used
			return false;
		}
#endif
	} else {
		notes << "setting video mode" << endl;
	}

	// uninit first to ensure that the video thread is not running
	VideoPostProcessor::uninit();

	bool HardwareAcceleration = false;
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
	notes << "ColorDepth: " << tLXOptions->iColourDepth << endl;

	// BlueBeret's addition (2007): OpenGL support
	bool opengl = tLXOptions->bOpenGL;

	// Initialize the video
	if(tLXOptions->bFullscreen)  {
		vidflags |= SDL_FULLSCREEN;
	}

	if (opengl) {
		vidflags |= SDL_OPENGL;
#ifndef REAL_OPENGL
		vidflags |= SDL_OPENGLBLIT; // SDL will behave like normally
#endif
		// HINT: it seems that with OGL activated, SDL_SetVideoMode will already set the OGL depth size
		// though this main pixel format of the screen surface was always 32 bit for me in OGL under MacOSX
		//#ifndef MACOSX
		/*
		 short colorbitsize = (tLXOptions->iColourDepth==16) ? 5 : 8;
		 SDL_GL_SetAttribute (SDL_GL_RED_SIZE,   colorbitsize);
		 SDL_GL_SetAttribute (SDL_GL_GREEN_SIZE, colorbitsize);
		 SDL_GL_SetAttribute (SDL_GL_BLUE_SIZE,  colorbitsize);
		 SDL_GL_SetAttribute (SDL_GL_ALPHA_SIZE, colorbitsize);
		 //SDL_GL_SetAttribute (SDL_GL_DEPTH_SIZE, tLXOptions->iColourDepth);
		 */
		//#endif
		//SDL_GL_SetAttribute (SDL_GL_ALPHA_SIZE,  8);
		//SDL_GL_SetAttribute (SDL_GL_DEPTH_SIZE, 24);
		//SDL_GL_SetAttribute (SDL_GL_BUFFER_SIZE, 32);
		SDL_GL_SetAttribute (SDL_GL_DOUBLEBUFFER, 1); // always use double buffering in OGL mode
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

	VideoPostProcessor::init();
	int scrW = VideoPostProcessor::get()->screenWidth();
	int scrH = VideoPostProcessor::get()->screenHeight();
setvideomode:
	if( SDL_SetVideoMode(scrW, scrH, tLXOptions->iColourDepth, vidflags) == NULL) {
		if (resetting)  {
			errors << "Failed to reset video mode"
					<< " (ErrorMsg: " << SDL_GetError() << "),"
					<< " let's wait a bit and retry" << endl;
			SDL_Delay(500);
			resetting = false;
			goto setvideomode;
		}

		if(tLXOptions->iColourDepth != 0) {
			errors << "Failed to use " << tLXOptions->iColourDepth << " bpp"
					<< " (ErrorMsg: " << SDL_GetError() << "),"
					<< " trying automatic bpp detection ..." << endl;
			tLXOptions->iColourDepth = 0;
			goto setvideomode;
		}

		if(vidflags & SDL_OPENGL) {
			errors << "Failed to use OpenGL"
					<< " (ErrorMsg: " << SDL_GetError() << "),"
					<< " trying without ..." << endl;
			vidflags &= ~(SDL_OPENGL | SDL_OPENGLBLIT | SDL_HWSURFACE | SDL_HWPALETTE | SDL_HWACCEL);
			goto setvideomode;
		}
		
		if(vidflags & SDL_FULLSCREEN) {
			errors << "Failed to set full screen video mode "
					<< scrW << "x" << scrH << "x" << tLXOptions->iColourDepth
					<< " (ErrorMsg: " << SDL_GetError() << "),"
					<< " trying window mode ..." << endl;
			vidflags &= ~SDL_FULLSCREEN;
			goto setvideomode;
		}

		SystemError("Failed to set the video mode " + itoa(scrW) + "x" + itoa(scrH) + "x" + itoa(tLXOptions->iColourDepth) + "\nErrorMsg: " + std::string(SDL_GetError()));
		return false;
	}

	SDL_WM_SetCaption(GetGameVersion().asHumanString().c_str(),NULL);
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
	
#ifdef REAL_OPENGL	
	if((SDL_GetVideoSurface()->flags & SDL_OPENGL)) {
		static SDL_PixelFormat OGL_format32 =
		{
			NULL, //SDL_Palette *palette;
			32, //Uint8  BitsPerPixel;
			4, //Uint8  BytesPerPixel;
			0, 0, 0, 0, //Uint8  Rloss, Gloss, Bloss, Aloss;
#if SDL_BYTEORDER == SDL_LIL_ENDIAN /* OpenGL RGBA masks */
			0, 8, 16, 24, //Uint8  Rshift, Gshift, Bshift, Ashift;
			0x000000FF,
			0x0000FF00,
			0x00FF0000,
			0xFF000000,
#else
			24, 16, 8, 0, //Uint8  Rshift, Gshift, Bshift, Ashift;
			0xFF000000,
			0x00FF0000,
			0x0000FF00,
			0x000000FF,
#endif
			0, //Uint32 colorkey;
			255 //Uint8  alpha;
		};
		// some GFX stuff in OLX seems very slow when this is used
		// (probably the blit from alpha surf to this format is slow)
	/*	static SDL_PixelFormat OGL_format24 =
		{
			NULL, //SDL_Palette *palette;
			24, //Uint8  BitsPerPixel;
			3, //Uint8  BytesPerPixel;
			0, 0, 0, 0, //Uint8  Rloss, Gloss, Bloss, Aloss;
			#if SDL_BYTEORDER == SDL_LIL_ENDIAN // OpenGL RGBA masks
			0, 8, 16, 0, //Uint8  Rshift, Gshift, Bshift, Ashift;
			0x000000FF,
			0x0000FF00,
			0x00FF0000,
			0x00000000,
			#else
			16, 8, 0, 0, //Uint8  Rshift, Gshift, Bshift, Ashift;
			0x00FF0000,
			0x0000FF00,
			0x000000FF,
			0x00000000,
			#endif
			0, //Uint32 colorkey;
			255 //Uint8  alpha;
		}; */
		//if(tLXOptions->iColourDepth == 32)
			mainPixelFormat = &OGL_format32;
		//else
		//	mainPixelFormat = &OGL_format24;
	} else
#endif		
		mainPixelFormat = SDL_GetVideoSurface()->format;
	DumpPixelFormat(mainPixelFormat);
	if(SDL_GetVideoSurface()->flags & SDL_DOUBLEBUF)
		notes << "using doublebuffering" << endl;

	// Correct the surface format according to SDL
#ifdef REAL_OPENGL
	if(((SDL_GetVideoSurface()->flags & SDL_OPENGL) != 0)) {
		iSurfaceFormat = SDL_SWSURFACE;
	} else
#endif	
	if((SDL_GetVideoSurface()->flags & SDL_HWSURFACE) != 0)  {
		iSurfaceFormat = SDL_HWSURFACE;
		notes << "using hardware surfaces" << endl;
	}
	else {
		iSurfaceFormat = SDL_SWSURFACE; // HINT: under MacOSX, it doesn't seem to make any difference in performance
		if (HardwareAcceleration)
			hints << "Unable to use hardware surfaces, falling back to software." << endl;
		notes << "using software surfaces" << endl;
	}

	if(SDL_GetVideoSurface()->flags & SDL_OPENGL) {
		hints << "using OpenGL" << endl;
		
#ifdef REAL_OPENGL
		OGL_init();
#else
		FillSurface(SDL_GetVideoSurface(), Color(0, 0, 0));		
#endif
	}
	else
		FillSurface(SDL_GetVideoSurface(), Color(0, 0, 0));
	
	VideoPostProcessor::get()->resetVideo();

	notes << "video mode was set successfully" << endl;
	return true;
}

#ifdef WIN32
//////////////////////
// Get the window handle
HWND GetWindowHandle()
{
	SDL_SysWMinfo info;
	SDL_VERSION(&info.version);
	if(!SDL_GetWMInfo(&info))
		return 0;

	return info.window;
}
#endif


void CapFPS() {
	const TimeDiff fMaxFrameTime = TimeDiff( (tLXOptions->nMaxFPS > 0) ? (1.0f / (float)tLXOptions->nMaxFPS) : 0.0f );
	const AbsTime currentTime = GetTime();
	// tLX->currentTime is old time

	// Cap the FPS
	if(currentTime - tLX->currentTime < fMaxFrameTime)
		SDL_Delay((Uint32)(fMaxFrameTime - (currentTime - tLX->currentTime)).milliseconds());
	else
		// do at least one small break, else it's possible that we never receive signals from our OS
		SDL_Delay(1);
}


// Screenshot structure
struct screenshot_t {
	std::string sDir;
	std::string	sData;
};

struct ScreenshotQueue {
	std::list<screenshot_t> queue;
	SDL_mutex* mutex;
	ScreenshotQueue() : mutex(NULL) { mutex = SDL_CreateMutex(); }
	~ScreenshotQueue() { SDL_DestroyMutex(mutex); mutex = NULL; }
};

static ScreenshotQueue screenshotQueue;

void PushScreenshot(const std::string& dir, const std::string& data) {
	screenshot_t scr; scr.sDir = dir; scr.sData = data;
	ScopedLock lock(screenshotQueue.mutex);
	screenshotQueue.queue.push_back(scr);
}

static void TakeScreenshot(const std::string& scr_path, const std::string& additional_data);

////////////////
// Process any screenshots
void ProcessScreenshots()
{
	std::list<screenshot_t> scrs;
	{
		ScopedLock lock(screenshotQueue.mutex);
		scrs.swap(screenshotQueue.queue);
	}
	
	// Process all the screenhots in the queue
	for (std::list<screenshot_t>::iterator it = scrs.begin(); it != scrs.end(); it++)  {
		TakeScreenshot(it->sDir, it->sData);
	}
}




#ifdef REAL_OPENGL

static GLuint OGL_createTexture() {
	GLuint textureid;
	
	// create one texture name
	glGenTextures(1, &textureid);

	// tell opengl to use the generated texture name
	glBindTexture(GL_TEXTURE_2D, textureid);

	// these affect how this texture is drawn later on...
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

	return textureid;
}

static void OGL_setupScreenForSingleTexture(GLuint textureid, int w, int h) {

	glPushAttrib(GL_ENABLE_BIT | GL_VIEWPORT_BIT | GL_POLYGON_BIT);
	glDisable(GL_DEPTH_TEST);
	glDisable (GL_LIGHTING);
	glDisable (GL_LINE_SMOOTH);
	glDisable (GL_CULL_FACE);
	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glViewport(0, 0, w, h);
	glMatrixMode (GL_PROJECTION);
	glPushMatrix ();
	glLoadIdentity ();
	glOrtho (0, (GLdouble)w, 0, (GLdouble)h, -1, 1);
	glMatrixMode (GL_MODELVIEW);
	glPushMatrix ();
	glLoadIdentity ();

	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glBindTexture (GL_TEXTURE_2D, textureid);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	int x = 0, y = 0;
	glPushMatrix ();
	glTranslated (x, y, 0);
	glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
	
	int realw = 1; while(realw < 640) realw <<= 1;
	int realh = 1; while(realh < 480) realh <<= 1;
	float texW = 640 / (float)realw;
	float texH = 480 / (float)realh;
	
	glBegin(GL_QUADS);
	{
		glTexCoord2f (0, 0);
		glVertex2i (0, h);
		glTexCoord2f (texW, 0);
		glVertex2i (w, h);
		glTexCoord2f (texW, texH);
		glVertex2i (w, 0);
		glTexCoord2f (0, texH);
		glVertex2i (0, 0);
	}
	glEnd();
	
	glPopMatrix ();

	/* Leave "2D mode" */
	glMatrixMode (GL_PROJECTION);
	glPopMatrix ();
	glMatrixMode (GL_MODELVIEW);
	glPopMatrix();
	glPopAttrib();

}

static GLuint OGL_screenBuf = 0;

static void OGL_init() {
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // set black background

	OGL_screenBuf = OGL_createTexture();
}

static void OGL_draw(SDL_Surface* src) {
	// tell opengl to use the generated texture name
	glBindTexture(GL_TEXTURE_2D, OGL_screenBuf);

	int mode = 0; 
	if(src->format->BytesPerPixel == 3)
		mode = GL_RGB;
	else
		mode = GL_RGBA;
	
	// this reads from the sdl surface and puts it into an opengl texture
	glTexImage2D(GL_TEXTURE_2D, 0, mode, src->w, src->h, 0, mode, GL_UNSIGNED_BYTE, src->pixels);

	int w = VideoPostProcessor::get()->screenWidth();
	int h = VideoPostProcessor::get()->screenHeight();
	OGL_setupScreenForSingleTexture(OGL_screenBuf, w, h);
}
#endif



// ---------------- VideoPostProcessor ---------------------------------------------------------

VideoPostProcessor voidVideoPostProcessor; // this one does nothing

SDL_Surface* VideoPostProcessor::m_videoSurface = NULL;
SDL_Surface* VideoPostProcessor::m_videoBufferSurface = NULL;
VideoPostProcessor* VideoPostProcessor::instance = &voidVideoPostProcessor;

///////////////////
// Flip the screen
void flipRealVideo() {
	SDL_Surface* psScreen = SDL_GetVideoSurface();
	if(psScreen == NULL) return;
	
	//TestCircleDrawing(psScreen);
	//TestPolygonDrawing(psScreen);
	//DrawLoadingAni(psScreen, 320, 260, 50, 50, Color(128,128,128), Color(128,128,128,128), LAT_CIRCLES);
	//DrawLoadingAni(psScreen, 320, 260, 10, 10, Color(255,0,0), Color(0,255,0), LAT_CAKE);
	
#ifdef REAL_OPENGL	
	if((psScreen->flags & SDL_OPENGL))
		glFlush();
	else
#endif
		SDL_Flip( psScreen );

	if(psScreen->flags & SDL_OPENGL)
		SDL_GL_SwapBuffers();
}

// base class for your video post processor
// this base manages the video surface and its buffer
class BasicVideoPostProcessor : public VideoPostProcessor {
public:
	SmartPointer<SDL_Surface> m_screenBuf[2];

	virtual void resetVideo() {
		// IMPORTANT: Don't reallocate if we already have the buffers.
		// If we would do, the old surfaces would get deleted. This is bad
		// because other threads could use it right now.
		if(m_screenBuf[0].get()) return;
		
		// create m_screenBuf here to ensure that we have initialised the correct surface parameters like pixel format
#ifdef REAL_OPENGL
		if((SDL_GetVideoSurface()->flags & SDL_OPENGL)) {
			// get smallest power-of-2 dimension which is bigger than src
			int w = 1; while(w < 640) w <<= 1;
			int h = 1; while(h < 480) h <<= 1;
			m_screenBuf[0] = gfxCreateSurface(w, h);
			m_screenBuf[1] = gfxCreateSurface(w, h);
		} else
#endif
		{
			m_screenBuf[0] = gfxCreateSurface(640, 480);
			m_screenBuf[1] = gfxCreateSurface(640, 480);
		}

		m_videoSurface = m_screenBuf[0].get();
		m_videoBufferSurface = m_screenBuf[1].get();
	}
};

// There is one usage of this: If you want to let OLX manage the double buffering.
// In this case, all the flipping and copying is done in the video thread.
// Without a post processor, the flipping is done in the main thread.
// There are some rare situations where the flipping of the screen surface is slow
// and in this situations, it can be faster to use this dummy post processor.
class DummyVideoPostProc : public BasicVideoPostProcessor {
public:
	DummyVideoPostProc() {
		notes << "using Dummy video post processor" << endl;
	}

	virtual void processToScreen() {
#ifdef REAL_OPENGL
		if((SDL_GetVideoSurface()->flags & SDL_OPENGL)) {
			OGL_draw(m_videoBufferSurface);
		}
		else
#endif			
			DrawImageAdv(SDL_GetVideoSurface(), m_videoBufferSurface, 0, 0, 0, 0, 640, 480);
	}

};

class StretchHalfPostProc : public BasicVideoPostProcessor {
public:
	static const int W = 320;
	static const int H = 240;

	StretchHalfPostProc() {
		notes << "using StretchHalf video post processor" << endl;
	}

	virtual void processToScreen() {
		DrawImageScaleHalf(SDL_GetVideoSurface(), m_videoBufferSurface);
		//DrawImageResizedAdv(SDL_GetVideoSurface(), m_videoBufferSurface, 0, 0, 0, 0, 640, 480, W, H);
		//DrawImageResampledAdv(SDL_GetVideoSurface(), m_videoBufferSurface, 0, 0, 0, 0, 640, 480, W, H);
	}

	virtual int screenWidth() { return W; }
	virtual int screenHeight() { return H; }

};

class Scale2XPostProc : public BasicVideoPostProcessor {
public:
	static const int W = 640 * 2;
	static const int H = 480 * 2;

	Scale2XPostProc() {
		notes << "using Scale2x video post processor" << endl;
	}

	virtual void processToScreen() {
		DrawImageScale2x(SDL_GetVideoSurface(), m_videoBufferSurface, 0, 0, 0, 0, 640, 480);
	}

	virtual int screenWidth() { return W; }
	virtual int screenHeight() { return H; }

};




// IMPORTANT: this has to be called from main thread!
void VideoPostProcessor::process() {
	ProcessScreenshots();
	VideoPostProcessor::get()->processToScreen();
}

void VideoPostProcessor::cloneBuffer() {
	// TODO: don't hardcode screen width/height here
	DrawImageAdv(m_videoBufferSurface, m_videoSurface, 0, 0, 0, 0, 640, 480);
}

void VideoPostProcessor::init() {
	notes << "VideoPostProcessor initialisation ... ";

	std::string vppName = tLXOptions->sVideoPostProcessor;
	TrimSpaces(vppName); stringlwr(vppName);
	if(vppName == "stretchhalf")
		instance = new StretchHalfPostProc();
	else if(vppName == "scale2x")
		instance = new Scale2XPostProc();
	else if(vppName == "dummy")
		instance = new DummyVideoPostProc();
	else {
		if(vppName != "")
			notes << "\"" << tLXOptions->sVideoPostProcessor << "\" unknown; ";
		// notes << "none used, drawing directly on screen" << endl;
		//instance = &voidVideoPostProcessor;
		instance = new DummyVideoPostProc();
	}
}

void VideoPostProcessor::uninit() {
	if(instance != &voidVideoPostProcessor && instance != NULL)
		delete instance;
	instance = &voidVideoPostProcessor;

	m_videoSurface = NULL; // should never be used before resetVideo() is called
}



void VideoPostProcessor::transformCoordinates_ScreenToVideo( int& x, int& y ) {
	x = (int)((float)x * 640.0f / (float)get()->screenWidth());
	y = (int)((float)y * 480.0f / (float)get()->screenHeight());
}


// ---------------------------------------------------------------------------------------------





///////////////////
// Shutdown the standard Auxiliary Library
void ShutdownAuxLib()
{
#ifdef WIN32
	UnSubclassWindow();
#endif

	// free all cached stuff like surfaces and sounds
	// HINT: we have to do it before we uninit the specific engines
	cCache.Clear();

	VideoPostProcessor::uninit();
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
// Return the config filename
std::string GetConfigFile()
{
	return ConfigFile;
}


//////////////////
// Helper funtion for screenshot taking
static std::string GetPicName(const std::string& prefix, size_t i, const std::string& ext)
{
	return prefix + (i == 0 ? std::string("") : itoa(i)) + ext;
}


////////////////////
// Helper function for TakeScreenshot
static std::string GetScreenshotFileName(const std::string& scr_path, const std::string& extension)
{
	std::string path = scr_path;

	// Append a slash if not present
	if (path[path.size() - 1] != '/' && path[path.size() - 1] != '\\')  {
		path += '/';
	}
	
	
	std::string filePrefix = GetDateTimeFilename();
	filePrefix += "-";
	if( tLX )
	{
		if( tLX->iGameType == GME_LOCAL )
			filePrefix += "local";
		else if( tLX->iGameType == GME_HOST && cServer )
			filePrefix += cServer->getName();
		else if( cClient )
			filePrefix += cClient->getServerName();
	}
	
	// Make filename more fileststem-friendly
	if( filePrefix.size() > 64 )
		filePrefix.resize(64);

#define S_LETTER_UPPER "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define S_LETTER_LOWER "abcdefghijklmnopqrstuvwxyz"
#define S_LETTER S_LETTER_UPPER S_LETTER_LOWER
#define S_NUMBER "0123456789"
#define S_SYMBOL ". -_&+"	// No "\\" symbol, no tab.
#define S_VALID_FILENAME S_LETTER_UPPER S_LETTER_LOWER S_NUMBER S_SYMBOL
	while( filePrefix.find_first_not_of(S_VALID_FILENAME) != std::string::npos )
		filePrefix[ filePrefix.find_first_not_of(S_VALID_FILENAME) ] = '-';

	static const size_t step = 256; // Step; after how many files we check if the filename still exists

	// We start at range from 1 to step
	size_t lower_bound = 0;
	size_t upper_bound = step;

	std::string fullname(path + GetPicName(filePrefix, upper_bound, extension));

	// Find a raw range of where the screenshot filename could be
	// For example: between lierox1000.png and lierox1256.png
	while (IsFileAvailable(fullname, false))  {
		lower_bound = upper_bound;
		upper_bound += step;

		fullname = path + GetPicName(filePrefix, upper_bound, extension);
	}

	// First file?
	if (!IsFileAvailable(path + GetPicName(filePrefix, lower_bound, extension)))
		return path + GetPicName(filePrefix, lower_bound, extension);

	// Use binary search on the given range to find the exact file name
	size_t i = (lower_bound + upper_bound) / 2;
	while (true)  {
		if (IsFileAvailable(path + GetPicName(filePrefix, i, extension), false))  {
			// If the current (i) filename exists, but the i+1 does not, we're done
			if (!IsFileAvailable(path + GetPicName(filePrefix, i + 1, extension)))
				return path + GetPicName(filePrefix, i + 1, extension);
			else  {
				// The filename is somewhere in the interval (i, upper_bound)
				lower_bound = i;
				i = (lower_bound + upper_bound) / 2;
			}
		} else {
			// The filename is somewhere in the interval (lower_bound, i)
			upper_bound = i;
			i = (lower_bound + upper_bound) / 2;
		}
	}

	return ""; // Should not happen
}

///////////////////
// Take a screenshot
static void TakeScreenshot(const std::string& scr_path, const std::string& additional_data)
{
	if (scr_path.empty()) // Check
		return;

	notes << "Save screenshot to " << scr_path << endl;

	std::string	extension;

	// Set the extension
	switch (tLXOptions->iScreenshotFormat)  {
	case FMT_BMP: extension = ".bmp"; break;
	case FMT_PNG: extension = ".png"; break;
	case FMT_JPG: extension = ".jpg"; break;
	case FMT_GIF: extension = ".gif"; break;
	default: extension = ".png";
	}

	// Save the surface
	SaveSurface(VideoPostProcessor::videoBufferSurface(), GetScreenshotFileName(scr_path, extension),
		tLXOptions->iScreenshotFormat, additional_data);
}

#ifdef WIN32
LONG wpOriginal;
bool Subclassed = false;

////////////////////
// Subclass the window (control the incoming Windows messages)
void SubclassWindow()
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
void UnSubclassWindow()
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




#ifdef DEBUG
#include "CClient.h"

// TODO: move this to console (it was only nec. via chat because we didn't had the console globally before)
// HINT: This is called atm from CClientNetEngine::SendText().
// HINT: This is just a hack to do some testing in lobby or whatever.
// WARNING: These stuff is not intended to be stable, it's only for testing!
// HINT: Don't rely on this. If we allow the console later somehow in the lobby,
// this debug stuff will probably move there.
bool HandleDebugCommand(const std::string& text) {
	if(text.size() >= 3 && text.substr(0,3) == "///") {
		cClient->getChatbox()->AddText("DEBUG COMMAND", tLX->clNotice, TXT_NOTICE, tLX->currentTime);

		std::string cmd = text.substr(3);
		stringlwr(cmd);

		if(cmd == "reconnect") {
			notes << "DEBUG CMD: reconnect local client to " << cClient->getServerAddress() << endl;
			cClient->Connect(cClient->getServerAddress());
		} else if(cmd == "msgbox") {
			Menu_MessageBox("Test",
							"This is a very long text, a very long text, a very long text, a very long text, "
							"a very long text, a very long text, a very long text, a very long text, a very long text, "
							"a very long text, a very long text, a very long text, a very long text, a very long text, "
							"a very long text, a very long text, a very long text, a very long text, a very long text.\n"
							"Yes really, this text is very long, very long, very long, very long, very long, "
							"very very long, very very long, very very long, very very long, very very long.",
							DeprecatedGUI::LMB_OK);
		} else if(cmd == "register") {
			cServer->RegisterServer();
		} else
			notes << "DEBUG CMD unknown" << endl;

		return true;
	}
	return false;
}
#endif



//////////////////
// Gives a name to the thread
// Code taken from http://www.codeproject.com/KB/threads/Name_threads_in_debugger.aspx
void setCurThreadName(const std::string& name)
{
#ifdef _MSC_VER // Currently only for MSVC, haven't found a Windows-general way (I doubt there is one)
	typedef struct tagTHREADNAME_INFO
	{
		DWORD dwType; // Must be 0x1000.
		LPCSTR szName; // Pointer to name (in user addr space).
		DWORD dwThreadID; // Thread ID (-1=caller thread).
		DWORD dwFlags; // Reserved for future use, must be zero.
	} THREADNAME_INFO;
	
	THREADNAME_INFO info;
	{
		info.dwType = 0x1000;
		info.szName = name.c_str();
		info.dwThreadID = (DWORD)-1;
		info.dwFlags = 0;
	}
	
	__try
	{
		RaiseException( 0x406D1388 /* MSVC EXCEPTION */, 0, sizeof(info)/sizeof(DWORD), (DWORD*)&info );
	}
	__except (EXCEPTION_CONTINUE_EXECUTION)
	{
	}
#else
	// TODO: similar for other systems
#endif
}

void setCurThreadPriority(float p) {
#ifdef WIN32
	
#elif defined(__APPLE__)
	//int curp = getpriority(PRIO_DARWIN_THREAD, 0); 
	//int newp = p >= 0 ? 0 : 1;
	//notes << "curp:" << curp << ", newp:" << newp << endl;
	//setpriority(PRIO_DARWIN_THREAD, 0, newp);
#endif
}



size_t GetFreeSysMemory() {
#if defined(__APPLE__)
	vm_statistics_data_t page_info;
	vm_size_t pagesize;
	mach_msg_type_number_t count;
	kern_return_t kret;
	
	pagesize = 0;
	kret = host_page_size (mach_host_self(), &pagesize);
	count = HOST_VM_INFO_COUNT;
	
	kret = host_statistics (mach_host_self(), HOST_VM_INFO,(host_info_t)&page_info, &count);
	return page_info.free_count * pagesize;
#elif defined(__WIN64__)
	MEMORYSTATUSEX memStatex;
	memStatex.dwLength = sizeof (memStatex);
	::GlobalMemoryStatusEx (&memStatex);
	return memStatex.ullAvailPhys;
#elif defined(__WIN32__)
	MEMORYSTATUS memStatus;
	memStatus.dwLength = sizeof(MEMORYSTATUS);
	::GlobalMemoryStatus(&memStatus);
	return memStatus.dwAvailPhys;
#elif defined(__SUN__) && defined(_SC_AVPHYS_PAGES)
	return sysconf(_SC_AVPHYS_PAGES) * sysconf(_SC_PAGESIZE);
#elif defined(__FREEBSD__)
	int vm_vmtotal[] = { CTL_VM, VM_METER };
	struct vmtotal vmdata;

	size_t len = sizeof(vmdata);
        int result = sysctl(vm_vmtotal, sizeof(vm_vmtotal) / sizeof(*vm_vmtotal), &vmdata, &len, NULL, 0);
	if(result != 0) return 0;

	return vmdata.t_free * sysconf(_SC_PAGESIZE);
#elif defined(__linux__)

	// get it from /proc/meminfo
	FILE *fp = fopen("/proc/meminfo", "r");
	if ( fp )
	{
		unsigned long freeMem = 0;
		unsigned long buffersMem = 0;
		unsigned long cachedMem = 0;
		struct SearchTerm { const char* name; unsigned long* var; };
		SearchTerm terms[] = { {"MemFree:", &freeMem}, {"Buffers:", &buffersMem}, {"Cached:", &cachedMem} };
				
		char buf[1024];
		int n = fread(buf, sizeof(char), sizeof(buf) - 1, fp);
		buf[sizeof(buf)-1] = '\0';
		if(n > 0) {
			for(unsigned int i = 0; i < sizeof(terms) / sizeof(SearchTerm); ++i) {
				char* p = strstr(buf, terms[i].name);
				if(p) {
					p += strlen(terms[i].name);
					*terms[i].var = strtoul(p, NULL, 10);
				}
			}
		}
				
		fclose(fp);
		// it's written in KB in meminfo
		return ((size_t)freeMem + (size_t)buffersMem + (size_t)cachedMem) * 1024;
	}
	
	return 0;
#else
#warning No GetFreeSysMemory implementation for this arch/sys
	return 50 * 1024 * 1024; // return 50 MB (really randomly made up, but helps for cache system)
#endif
}



void lierox_t::setupInputs() {
	if(!tLXOptions) {
		errors << "lierox_t::setupInputs: tLXOptions not set" << endl;
		return;
	}
	
	// Setup global keys
	cTakeScreenshot.Setup(tLXOptions->sGeneralControls[SIN_SCREENSHOTS]);
	cSwitchMode.Setup(tLXOptions->sGeneralControls[SIN_SWITCHMODE]);
	cIrcChat.Setup(tLXOptions->sGeneralControls[SIN_IRCCHAT]);
	cConsoleToggle.Setup(tLXOptions->sGeneralControls[SIN_CONSOLETOGGLE]);

	if(cClient)
		cClient->SetupGameInputs();
	else
		warnings << "lierox_t::setupInputs: cClient not set" << endl;
}

bool lierox_t::isAnyControlKeyDown() const {
	return cTakeScreenshot.isDown() || cSwitchMode.isDown() || cIrcChat.isDown() || cConsoleToggle.isDown();
}



std::string GetDateTimeText()
{
	time_t t = time(NULL);

	if (t == (time_t)-1)
		return "TIME-ERROR1";
	
	char* timeCstr = ctime(&t);
	if(timeCstr == NULL)
		return "TIME-ERROR2";
	
	std::string timeStr(timeCstr);
	TrimSpaces(timeStr);
	return timeStr;
}

std::string GetDateTimeFilename()
{
	// Add date and server name to screenshot filename
	time_t curtime1 = time(NULL);
	if (curtime1 == (time_t)-1)
		return "TIME-ERROR1";
	struct tm *curtime = localtime(&curtime1);
	if (curtime == NULL)
		return "TIME-ERROR2";
	char filePrefixTime[200] = {0};
	strftime(filePrefixTime, sizeof(filePrefixTime), "%Y%m%d-%H%M", curtime);
	return filePrefixTime;
}

void EnableSystemMouseCursor(bool enable)
{
	if( bDedicated )
		return;
	struct EnableMouseCursor: public Action
	{
		bool Enable;
		
		EnableMouseCursor(bool b): Enable(b) {};
		int handle()
		{
			SDL_ShowCursor(Enable ? SDL_ENABLE : SDL_DISABLE ); // Should be called from main thread, or you'll get race condition with libX11
			return 0;
		} 
	};
	doActionInMainThread( new EnableMouseCursor(enable) );
};
