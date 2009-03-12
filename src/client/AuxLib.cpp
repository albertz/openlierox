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
#include <stdlib.h>
#include <sstream>

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
		else
#endif
		return false;
	}

	if(bJoystickSupport) {
		if(SDL_InitSubSystem(SDL_INIT_JOYSTICK) != 0) {
			warnings << "WARNING: couldn't init joystick subystem: " << SDL_GetError() << endl;
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


static void DumpPixelFormat(const SDL_PixelFormat* format) {
	std::string buf;
	std::stringstream str(buf);
	str << "PixelFormat:" << std::endl
		<< "  BitsPerPixel: " << (int)format->BitsPerPixel << ","
		<< "  BytesPerPixel: " << (int)format->BytesPerPixel << std::endl
		<< "  R/G/B/A mask: " << std::hex
			<< (uint)format->Rmask << "/"
			<< (uint)format->Gmask << "/"
			<< (uint)format->Bmask << "/"
		<< (uint)format->Amask << std::endl
		<< "  R/G/B/A loss: "
			<< (uint)format->Rloss << "/"
			<< (uint)format->Gloss << "/"
			<< (uint)format->Bloss << "/"
		<< (uint)format->Aloss << std::endl << std::dec
		<< "  Colorkey: " << (uint)format->colorkey << ","
		<< "  Alpha: " << (int)format->alpha << std::endl;
	notes << buf << endl;
}

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
	notes << "ColorDepth: " << tLXOptions->iColourDepth << endl;

	// BlueBeret's addition (2007): OpenGL support
	bool opengl = tLXOptions->bOpenGL;

	// Initialize the video
	if(tLXOptions->bFullscreen)  {
		vidflags |= SDL_FULLSCREEN;
	}

	if (opengl) {
		hints << "using OpenGL" << endl;
		vidflags |= SDL_OPENGL;
		vidflags |= SDL_OPENGLBLIT; // Without that flag SDL_GetVideoSurface()->pixels will be NULL, which is weird

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

	mainPixelFormat = SDL_GetVideoSurface()->format;
	DumpPixelFormat(mainPixelFormat);
	if(SDL_GetVideoSurface()->flags & SDL_DOUBLEBUF)
		notes << "using doublebuffering" << endl;

	// Correct the surface format according to SDL
	if ((SDL_GetVideoSurface()->flags & SDL_HWSURFACE) != 0)  {
		iSurfaceFormat = SDL_HWSURFACE;
		notes << "using hardware surfaces" << endl;
	} else {
		iSurfaceFormat = SDL_SWSURFACE; // HINT: under MacOSX, it doesn't seem to make any difference in performance
		if (HardwareAcceleration)
			hints << "Unable to use hardware surfaces, falling back to software." << endl;
		notes << "using software surfaces" << endl;
	}

	VideoPostProcessor::get()->resetVideo();
	FillSurface(SDL_GetVideoSurface(), MakeColour(0, 0, 0));

	notes << "video mode was set successfully" << endl;
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

////////////////
// Process any screenshots
void ProcessScreenshots()
{
	// Check if user pressed screenshot key
	if (cTakeScreenshot && cTakeScreenshot->isDownOnce())  {
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






// ---------------- VideoPostProcessor ---------------------------------------------------------

VideoPostProcessor voidVideoPostProcessor; // this one does nothing

SDL_Surface* VideoPostProcessor::m_videoSurface = NULL;
SDL_Surface* VideoPostProcessor::m_videoBufferSurface = NULL;
VideoPostProcessor* VideoPostProcessor::instance = &voidVideoPostProcessor;

///////////////////
// Flip the screen
static void flipRealVideo() {
	SDL_Surface* psScreen = SDL_GetVideoSurface();
	if(psScreen == NULL) return;

	SDL_Flip( psScreen );

	if (tLXOptions->bOpenGL)
		SDL_GL_SwapBuffers();
}

// base class for your video post processor
// this base manages the video surface and its buffer
class BasicVideoPostProcessor : public VideoPostProcessor {
public:
	SmartPointer<SDL_Surface> m_screenBuf[2];

	virtual void resetVideo() {
		// create m_screenBuf here to ensure that we have initialised the correct surface parameters like pixel format
		if(!m_screenBuf[0].get()) {
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
	flipRealVideo();
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
std::string GetConfigFile(void)
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
	
	
	// Add date and server name to screenshot filename
	time_t curtime1 = time(NULL);
	struct tm *curtime = localtime(&curtime1);
	char filePrefixTime[200];
	strftime(filePrefixTime, sizeof(filePrefixTime), "%y%m%d-%H%M", curtime);
	std::string filePrefix = filePrefixTime;
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
void TakeScreenshot(const std::string& scr_path, const std::string& additional_data)
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
void nameThread(const std::string& name)
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
