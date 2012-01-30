// OpenLieroX

// Main entry point
// Created 28/6/02
// Jason Boettcher

// code under LGPL



#include <cassert>
#include <setjmp.h>
#include <sstream> // for print_binary_string
#include <set>
#include <string>

#include "TeeStdoutHandler.h"
#include "LieroX.h"
#include "IpToCountryDB.h"
#include "AuxLib.h"
#include "CClient.h"
#include "CServer.h"
#include "ConfigHandler.h"
#include "OLXConsole.h"
#include "GfxPrimitives.h"
#include "FindFile.h"
#include "InputEvents.h"
#include "StringUtils.h"
#include "Entity.h"
#include "Error.h"
#include "DedicatedControl.h"
#include "Physics.h"
#include "Version.h"
#include "OLXG15.h"
#include "CrashHandler.h"
#include "Cursor.h"
#include "CssParser.h"
#include "FontHandling.h"
#include "Timer.h"
#include "CChannel.h"
#include "Cache.h"
#include "ProfileSystem.h"
#include "IRC.h"
#include "Music.h"
#include "Debug.h"
#include "TaskManager.h"
#include "CGameMode.h"
#include "ConversationLogger.h"
#include "StaticAssert.h"
#include "OLXCommand.h"
#include "game/Mod.h"
#include "gusanos/gusanos.h"
#include "game/Game.h"
#include "sound/SoundsBase.h"
#include "game/ServerList.h"

#include "DeprecatedGUI/CBar.h"
#include "DeprecatedGUI/Graphics.h"
#include "DeprecatedGUI/Menu.h"
#include "DeprecatedGUI/CChatWidget.h"

#include "SkinnedGUI/CGuiSkin.h"

#include "breakpad/ExtractInfo.h"

#ifndef WIN32
#include <dirent.h>
#include <sys/stat.h>
#endif

#include <libxml/parser.h>

#ifdef __MINGW32_VERSION
#include <SDL_main.h>
#endif

// TODO: i hate globals ...
// we have to create a basic class Game or something
keyboard_t	*kb = NULL;

static std::list<std::string> startupCommands;


//
// Loading screen info and functions
//
class LoadingScreen  {
public:
	LoadingScreen() {
		iBackgroundX = iBackgroundY = 0;
		bmpBackground = NULL;
		cBar = NULL;
	}

	int iBackgroundX;
	int iBackgroundY;
	int iLabelX;
	int iLabelY;
	Uint32 clLabel;
	SmartPointer<SDL_Surface> bmpBackground;
	DeprecatedGUI::CBar *cBar;
};

static LoadingScreen cLoading;

static void InitializeLoading();
static void DrawLoading(byte percentage, const std::string &text);
static void ShutdownLoading();




// TODO: move this out here
void print_binary_string(const std::string& txt) {
	std::string buf;
	std::stringstream str(buf);
	for(std::string::const_iterator it = txt.begin(); it != txt.end(); it++) {
		str << std::hex << (ushort)(uchar)(*it) << " ";
	}
	notes << buf << endl;
}

static void DoSystemChecks() {
	// sadly, these sizeof are directly used in CGameScript.cpp/CMap.cpp
	// TODO: fix this issue
	static_assert(sizeof(char) == 1, sizeof_char__equals1);
	static_assert(sizeof(short) == 2, sizeof_short__equals2);
	static_assert(sizeof(int) == 4, sizeof_int__equals4);
	static_assert(sizeof(float) == 4, sizeof_float__equals4);
	// sometimes the return value of SendMessage is used as a pointer
	static_assert(sizeof(DWORD) == sizeof(void*), sizeof_dword__equals_p);
}


#ifndef WIN32
sigjmp_buf longJumpBuffer;
#endif

static ThreadPoolItem* mainLoopThread = NULL;


static SDL_Event QuitEventThreadEvent() {
	SDL_Event ev;
	ev.type = SDL_USEREVENT;
	ev.user.code = UE_QuitEventThread;	
	return ev;
}


void startMainLockDetector() {
	if(!tLXOptions->bUseMainLockDetector) return;
	
	struct MainLockDetector : Action {
		bool wait(Uint32 time) {
			if(!tLX) return false;
			AbsTime oldTime = tLX->currentTime;
			for(Uint32 t = 0; t < time; t += 100) {
				if(!tLX) return false;
				if(tLX->bQuitGame) return false;						
				if(oldTime != tLX->currentTime) return true;
				SDL_Delay(100);
			}
			return true;
		}
		Result handle() {
			// We should always have tLX!=NULL here as we uninit it after the thread-shutdown now.
			while(tLX && !tLX->bQuitGame) {
				AbsTime oldTime = tLX->currentTime;
				if(!wait(1000)) return true;
				if(!tLX) return true;
				if(tLX->bQuitGame) return true;
				if(IsWaitingForEvent()) continue;
				
				// HINT: Comment that out and you'll find a lot of things in OLX which could be improved.
				// WARNING: This is the only code here which could lead to very rare crashes.
				//if(!cClient || cClient->getStatus() != NET_PLAYING) continue;
				
				// check if the mainthread is hanging
				if(oldTime == tLX->currentTime) {
					warnings << "possible lock of game thread detected" << endl;
					//OlxWriteCoreDump("mainlock");
					//RaiseDebugger();
					
					if(!wait(5*1000)) return true;
					if(tLX && !tLX->bQuitGame && oldTime == tLX->currentTime) {
						hints << "Still locked after 5 seconds. Current threads:" << endl;
						threadPool->dumpState(stdoutCLI());
						hints << "Free system memory: " << (GetFreeSysMemory() / 1024) << " KB" << endl;
						hints << "Cache size: " << (cCache.GetCacheSize() / 1024) << " KB" << endl;
					}
					else continue;
					
					// pause for a while, don't be so hard
					if(!wait(25*1000)) return true;
					if(tLX && !tLX->bQuitGame && oldTime == tLX->currentTime) {
						warnings << "we still are locked after 30 seconds" << endl;
						if(tLXOptions && tLXOptions->bFullscreen) {
							notes << "we are in fullscreen, going to window mode now" << endl;
							tLXOptions->bFullscreen = false;
							doSetVideoModeInMainThread();
							notes << "setting window mode sucessfull" << endl;
						}
					}
					else continue;
				
					// pause for a while, don't be so hard
					if(!wait(25*1000)) return true;
					if(tLX && !tLX->bQuitGame && oldTime == tLX->currentTime) {
						errors << "we still are locked after 60 seconds" << endl;
						if(!AmIBeingDebugged()) {
							errors << "aborting now" << endl;
							abort();
						}
					}
				}
			}
			return true;
		}
	};
	threadPool->start(new MainLockDetector(), "main lock detector", true);
}


static bool menu_startgame = false;

struct VideoHandler {
	SDL_mutex* mutex;
	SDL_cond* sign;
	int framesInQueue;
	bool videoModeReady;
	
	VideoHandler() : mutex(NULL), sign(NULL), framesInQueue(0), videoModeReady(true) {
		mutex = SDL_CreateMutex();
		sign = SDL_CreateCond();
	}
	
	~VideoHandler() {
		SDL_DestroyMutex(mutex); mutex = NULL;
		SDL_DestroyCond(sign); sign = NULL;
	}
	
	void frame() {
		{
			ScopedLock lock(mutex);
			if(!videoModeReady) return;
			SDL_CondBroadcast(sign);
			if(framesInQueue > 0) framesInQueue--;
			else return;
			VideoPostProcessor::process();
		}
		
		flipRealVideo();
	}
	
	void setVideoMode() {
		bool makeFrame = false;
		{
			ScopedLock lock(mutex);
			SetVideoMode();
			videoModeReady = true;
			SDL_CondBroadcast(sign);
			
			if(framesInQueue > 0) {
				framesInQueue = 0;
				makeFrame = true;
				VideoPostProcessor::process();
			}
		}
		
		if(makeFrame)
			flipRealVideo();
	}
	
	void pushFrame() {
		// wait for current drawing
		ScopedLock lock(mutex);
	
		while(framesInQueue > 0)
			SDL_CondWait(sign, mutex);
		
		SDL_Event ev;
		ev.type = SDL_USEREVENT;
		ev.user.code = UE_DoVideoFrame;
		if(SDL_PushEvent(&ev) == 0) {
			framesInQueue++;
			VideoPostProcessor::flipBuffers();
		} else
			warnings << "failed to push videoframeevent" << endl;
		
	}
	
	void requestSetVideoMode() {
		ScopedLock lock(mutex);
		SDL_Event ev;
		ev.type = SDL_USEREVENT;
		ev.user.code = UE_DoSetVideoMode;
		if(SDL_PushEvent(&ev) == 0) {
			videoModeReady = false;
			while(!videoModeReady)
				SDL_CondWait(sign, mutex);
		}
		else
			warnings << "failed to push setvideomode event" << endl;
	}
};

static VideoHandler videoHandler;

struct MainLoopTask : LoopTask {
	enum State {
		State_Startup,
		State_BeforeMenu,
		State_Menu,
		State_AfterMenu,
		State_Game,
		State_AfterGame,
		State_Quit
	};
	State state;
	AbsTime menuStartTime;
	bool last_frame_was_because_of_an_event;
	MainLoopTask() : state(State_Startup) {}
	Result handle_Startup();
	Result handle_BeforeMenu();
	Result handle_Menu();
	Result handle_AfterMenu();
	Result handle_Game();
	Result handle_AfterGame();
	Result handle_Quit();
	Result handleFrame();
};


// ParseArguments will set this eventually to true
static bool afterCrash = false;
static bool afterCrashInformedUser = false;

void setBinaryDirAndName(char* argv0);

///////////////////
// Main entry point
int main(int argc, char *argv[])
{
	if(DoCrashReport(argc, argv)) return 0;

	teeStdoutInit();
	
	setCurThreadName("Main Thread");
	setCurThreadPriority(0.5f);
	
	hints << GetFullGameName() << " is starting ..." << endl;
#ifdef DEBUG
	hints << "This is a DEBUG build." << endl;
#endif
#ifdef DEDICATED_ONLY
	hints << "This is a DEDICATED_ONLY build." << endl;
#endif
	notes << "Free memory: " << (GetFreeSysMemory() / 1024 / 1024) << " MB" << endl;
	notes << "Current time: " << GetDateTimeText() << endl;
	
	// Initialize the LieroX structure
	tLX = new lierox_t;

	DoSystemChecks();
	if(!InitNetworkSystem())
		errors << "Failed to initialize the network library" << endl;
	InitThreadPool();
	
	setBinaryDirAndName(argv[0]);

	// this has to be done before GameOptions::Init
	InitGameModes();

startpoint:

	InitTaskManager();
	
	// Load options and other settings
	if(!GameOptions::Init()) {
		SystemError("Could not load options");
		return -1;
	}

	teeStdoutFile(GetWriteFullFileName("logs/OpenLieroX - " + GetDateTimeFilename() + ".txt", true));
	CrashHandler::init();

	if(!NetworkTexts::Init()) {
		SystemError("Could not load network strings.");
		return -1;
	}
	
	if(!Taunts::Init()) {
		SystemError("Could not load taunts.");
		return -1;
	}

	CSSParser::test_css();

    // Parse the arguments
	// do it after the loading of the options as this can
	// overwrite the default options
	ParseArguments(argc, argv);

	// Start the G15 support, it's suitable that the display is showing while loading.
#ifdef WITH_G15
	OLXG15 = new OLXG15_t;
	if (!OLXG15->init()) {
		// Error when initialising, can't use it.
		delete OLXG15;
		OLXG15 = NULL;
	}
#endif //WITH_G15

	// Start loading the IP to country database
	// HINT: we do it as soon as possible because the loading has more time then which means better results
	// HINT: the database won't load if it is disabled in options
	tIpToCountryDB = new IpToCountryDB(IP_TO_COUNTRY_FILE);

	// Initialize LX
	if(!InitializeLieroX())  {
		SystemError("Could not initialize LieroX.");
		return -1;
	}
	
	kb = GetKeyboard();
	if (!bDedicated && !VideoPostProcessor::videoSurface()) {
		SystemError("Could not find screen.");
		return -1;
	}

	// Music
	if( !bDedicated && tLXOptions->iMusicVolume > 0 )
		InitializeBackgroundMusic();

	tLX->currentTime = GetTime();

	DrawLoading(70, "Initializing menu system");

	// Initialize menu
	if(!DeprecatedGUI::Menu_Initialize(&menu_startgame)) {
        SystemError("Error: Could not initialize the menu system.\nError when loading graphics files");
		return -1;
	}

	// speedup for Menu_Start()
	DrawLoading(90, "Loading main menu");
	DeprecatedGUI::iSkipStart = true;
	DeprecatedGUI::tMenu->iMenuType = DeprecatedGUI::MNU_MAIN;
	DeprecatedGUI::Menu_MainInitialize();

	// Initialize chat logging
	convoLogger = new ConversationLogger();
	if (tLXOptions->bLogConvos)
		convoLogger->startLogging();

	// Setup the global keys
	tLX->setupInputs();
	
	DrawLoading(100, "Done! Starting menu");

	// Everything loaded, this is not needed anymore
	ShutdownLoading();

	if(startFunction != NULL) {
		if(!startFunction(startFunctionData)) {
			SystemError("ERROR: startfunction not successfull");
			return -1;
		}
		// reset the data; the startfunction is intended to be called only once
		startFunction = NULL;
		startFunctionData = NULL;
	}

	for(std::list<std::string>::iterator i = startupCommands.begin(); i != startupCommands.end(); ++i) {
		// execute as if it would have been entered in ingame console
		notes << "startup command: " << *i << endl;
		Con_Execute(*i);
	}
	startupCommands.clear(); // don't execute them again
		
//#ifndef SINGLETHREADED
	mainLoopThread = threadPool->start(new MainLoopTask(), "main loop", false);

	startMainLockDetector();
//#endif

	if(!bDedicated) {
		// Get all SDL events and push them to our event queue.
		// We have to do that in the same thread where we inited the video because of SDL.
		SDL_Event ev;
		memset( &ev, 0, sizeof(ev) );
		while(true) {
			while( SDL_WaitEvent(&ev) ) {
				if(ev.type == SDL_USEREVENT) {
					switch(ev.user.code) {
						case UE_QuitEventThread:
							goto quit;
						case UE_DoVideoFrame:
							videoHandler.frame();
							continue;
						case UE_DoSetVideoMode:
							videoHandler.setVideoMode();
							continue;
						case UE_DoActionInMainThread:
							((Action*)ev.user.data1)->handle();
							delete (Action*)ev.user.data1;
							continue;
					}
				}
				if( ev.type == SDL_SYSWMEVENT ) {
					EvHndl_SysWmEvent_MainThread( &ev );
					continue;
				}
				mainQueue->push(ev);
			}
			
			notes << "error while waiting for next event" << endl;
			SDL_Delay(200);
		}
	}
	
quit:
	threadPool->wait(mainLoopThread, NULL);
	mainLoopThread = NULL;
	
	if(!bRestartGameAfterQuit)
		CrashHandler::restartAfterCrash = false;
	
	ShutdownLieroX();

	notes << "waiting for all left threads and tasks" << endl;
	taskManager->finishQueuedTasks();
	threadPool->waitAll(); // do that before uniniting task manager because some threads could access it

	// do that after shutting down the timers and other threads
	ShutdownEventQueue();
	
	UnInitTaskManager();

	if(bRestartGameAfterQuit) {
		bRestartGameAfterQuit = false;
		hints << "-- Restarting game --" << endl;
		goto startpoint;
	}

	UnInitThreadPool();

	// Network
	QuitNetworkSystem();

	// LieroX structure
	// HINT: must be after end of all threads because we could access it
	if(tLX) {
		delete tLX;
		tLX = NULL;
	}
	
	notes << "Good Bye and enjoy your day..." << endl;

	// Uninit the crash handler after all other code
	CrashHandler::uninit();

	teeStdoutQuit();
	return 0;
}

// note: important to have const char* here because std::string is too dynamic, could be screwed up when returning
void SetCrashHandlerReturnPoint(const char* name) {
#ifndef WIN32
	if(sigsetjmp(longJumpBuffer, true) != 0) {
		hints << "returned from sigsetjmp in " << name << endl;
		if(!tLXOptions) {
			notes << "we already have tLXOptions uninitialised, exiting now" << endl;
			exit(10);
			return;
		}
		if(tLXOptions->bFullscreen) {
			notes << "we are in fullscreen, going to window mode now" << endl;
			tLXOptions->bFullscreen = false;
			doSetVideoModeInMainThread();
			notes << "back in window mode" << endl;
		}
	}
#endif	
}

void doVideoFrameInMainThread() {
	if(bDedicated) return;
	videoHandler.pushFrame();
}

void doSetVideoModeInMainThread() {
	if(bDedicated) return;
	videoHandler.requestSetVideoMode();
}

void doVppOperation(Action* act) {
	{
		ScopedLock lock(videoHandler.mutex);
		act->handle();
	}
	delete act;
}

void doActionInMainThread(Action* act) {
	if(bDedicated) {
		warnings << "doActionInMainThread cannot work correctly in dedicated mode" << endl;
		// we will just put it to the main queue instead
		mainQueue->push(act);
		return;
	}
	
	SDL_Event ev;
	ev.type = SDL_USEREVENT;
	ev.user.code = UE_DoActionInMainThread;
	ev.user.data1 = act;
	if(SDL_PushEvent(&ev) != 0) {
		errors << "failed to push custom action event" << endl;
	}
}


Result MainLoopTask::handle_Startup() {
	setCurThreadPriority(0.5f);
	tLX->bQuitGame = false;

	if(afterCrash && !afterCrashInformedUser) {
		afterCrashInformedUser = true;
		DeprecatedGUI::Menu_MessageBox("Sorry",
									   "The game has crashed. This should not have happend. "
									   "But it did.\nWe hope we can fix this problem in a future version. "
									   "Or perhaps there is already a new version; check out our "
									   "homepage for more information:\nhttp://openlierox.net\n\n"
									   "If you have an idea why this have happend, please write "
									   "us a mail or post in our forum. This may help us a lot "
									   "for fixing the problem.\n\nThanks!", DeprecatedGUI::LMB_OK);
	}
	
	if( tLXOptions->bNewSkinnedGUI )
	{
		// Just for test - it's not real handler yet
		SkinnedGUI::cMainSkin->Load("default");
		SkinnedGUI::cMainSkin->OpenLayout("test.skn");
		while (!tLX->bQuitGame)  {
			tLX->fDeltaTime = GetTime() - tLX->currentTime;
			tLX->currentTime = GetTime();

			WaitForNextEvent();
			SkinnedGUI::cMainSkin->Frame();
		}

		ShutdownLieroX();
		return "quit";
	}

	state = State_BeforeMenu;
	return true;
}

Result MainLoopTask::handle_BeforeMenu() {
	if(tLX->bQuitGame) {
		state = State_Quit;
		return true;
	}

	SetCrashHandlerReturnPoint("MainLoopThread before lobby");

	cClient->SetSocketWithEvents(true);
	cServer->SetSocketWithEvents(true);
	menu_startgame = false; // the menu has a reference to this variable
	ResetQuitEngineFlag();

	DeprecatedGUI::tMenu->bMenuRunning = true;

	if(!bDedicated) {
		if(!DeprecatedGUI::iSkipStart) {
			notes << "Loading main menu" << endl;
			DeprecatedGUI::tMenu->iMenuType = DeprecatedGUI::MNU_MAIN;
			DeprecatedGUI::Menu_MainInitialize();
		} else
			DeprecatedGUI::Menu_RedrawMouse(true);
	}

	DeprecatedGUI::iSkipStart = false;

	menuStartTime = tLX->currentTime = GetTime();
	last_frame_was_because_of_an_event = true;
	last_frame_was_because_of_an_event = ProcessEvents();

	state = State_Menu;
	return true;
}

Result MainLoopTask::handle_Menu() {
	if(!DeprecatedGUI::tMenu->bMenuRunning) {
		state = State_AfterMenu;
		return true;
	}

	AbsTime oldtime = tLX->currentTime;

	DeprecatedGUI::Menu_Frame();
	if(!DeprecatedGUI::tMenu->bMenuRunning) return true;
	CapFPS();
	SetCrashHandlerReturnPoint("Menu_Loop");

	if(last_frame_was_because_of_an_event || bDedicated) {
		// Use ProcessEvents() here to handle other processes in queue.
		// There aren't probably any but it has also the effect that
		// we run the loop another time after an event which is sometimes
		// because of the current code needed. Sometimes after an event,
		// some new menu elements got initialised but not drawn.
		last_frame_was_because_of_an_event = ProcessEvents();
	} else {
		last_frame_was_because_of_an_event = WaitForNextEvent();
	}

	ProcessIRC();

	tLX->currentTime = GetTime();
	tLX->fDeltaTime = tLX->currentTime - oldtime;
	tLX->fRealDeltaTime = tLX->fDeltaTime;

	// If we have run fine for >=5 seconds, it is probably safe & make sense
	// to restart the game in case of a crash.
	if(tLX->currentTime - menuStartTime >= TimeDiff(5.0f))
		CrashHandler::restartAfterCrash = true;

	return true;
}

Result MainLoopTask::handle_AfterMenu() {
	// If we go out of the menu, it means the user has selected something.
	// This indicates that everything is fine, so we should restart in case of a crash.
	// Note that we will set this again to false later on in case the user quitted.
	CrashHandler::restartAfterCrash = true;

	cClient->SetSocketWithEvents(false);
	cServer->SetSocketWithEvents(false);
		
	if(!menu_startgame) {
		// Quit
		tLX->bQuitGame = true;
		state = State_Quit;
		return true;
	}

	if(tLX->bQuitEngine) {
		// If we already set the quitengine flag, we want to go back to the menu.
		// Sometimes, when we get both a PrepareGame and a GotoLobby packet in
		// a single menu-frame, we quit the menu and set the quitengine flag
		state = State_BeforeMenu;
		return true;
	}

	game.prepareGameloop();

	state = State_Game;
	return true;
}

Result MainLoopTask::handle_Game() {
	if(tLX->bQuitEngine) {
		state = State_AfterGame;
		return true;
	}

	game.frameOuter();
	return true;
}

Result MainLoopTask::handle_AfterGame() {
	game.cleanupAfterGameloopEnd();
	state = State_BeforeMenu;
	return true;
}

Result MainLoopTask::handle_Quit() {
	SDL_Event quitEv = QuitEventThreadEvent();
	if(!bDedicated)
		while(SDL_PushEvent(&quitEv) < 0) {}
	return "quit";
}

Result MainLoopTask::handleFrame() {
	switch(state) {
	case State_Startup: return handle_Startup();
	case State_BeforeMenu: return handle_BeforeMenu();
	case State_Menu: return handle_Menu();
	case State_AfterMenu: return handle_AfterMenu();
	case State_Game: return handle_Game();
	case State_AfterGame: return handle_AfterGame();
	case State_Quit: return handle_Quit();
	}
	return "invalid state";
}


///////////////////
// Parse the arguments
void ParseArguments(int argc, char *argv[])
{
    // Parameters passed to OpenLieroX overwrite the loaded options
    char *a;
    for(int i=1; i<argc; i++) {
        a = argv[i];

        // -opengl
        // Turns OpenGL on
        if( stricmp(a, "-opengl") == 0 ) {
            tLXOptions->bOpenGL = true;
        } else

        // -nojoystick
        // Turns joystick off
        if( stricmp(a, "-nojoystick") == 0 ) {
            bJoystickSupport = false;
        } else


        // -noopengl
        // Turns OpenGL off
        if( stricmp(a, "-noopengl") == 0 ) {
            tLXOptions->bOpenGL = false;
        } else

        // -nosound
        // Turns off the sound
        if( stricmp(a, "-nosound") == 0 ) {
            bDisableSound = true;
        } else

        // -dedicated
        // Turns on dedicated mode (no gfx, no sound)
        if( stricmp(a, "-dedicated") == 0 ) {
            bDedicated = true;
			bDisableSound = true;
        } else

		// -dedscript
		// set dedicated script (next param)
		if( stricmp(a, "-script") == 0 ) {
			if(argv[i + 1] != NULL) {
				bDedicated = true;
				bDisableSound = true;
				// these settings will be temporarly because we don't save options at end in dedicated mode
				tLXOptions->sDedicatedScript = argv[++i];
			}
			else
				warnings << "-script needs an additinal parameter" << endl;
		} else
				
		// -connect
		// connect to server (next param)
		if( stricmp(a, "-connect") == 0 ) {
			if(argv[i + 1] != NULL) {
				startupCommands.push_back("connect \"" + std::string(argv[++i]) + "\"");
			}
			else
				warnings << "-connect needs an additinal parameter" << endl;
		} else

		// -exec
		// pushes a startup command
		if( stricmp(a, "-exec") == 0 ) {
			if(argv[i + 1] != NULL)
				startupCommands.push_back(argv[++i]);
			else
				warnings << "-exec needs an additinal parameter" << endl;
		} else
				
        // -window
        // Turns fullscreen off
        if( stricmp(a, "-window") == 0 ) {
            tLXOptions->bFullscreen = false;
        } else

        // -fullscreen
        // Turns fullscreen on
        if( stricmp(a, "-fullscreen") == 0 ) {
            tLXOptions->bFullscreen = true;
        } else

        // -skin
        // Turns new skinned GUI on
        if( stricmp(a, "-skin") == 0 ) {
            tLXOptions->bNewSkinnedGUI = true;
        } else

        // -noskin
        // Turns new skinned GUI off
        if( stricmp(a, "-noskin") == 0 ) {
            tLXOptions->bNewSkinnedGUI = false;
        } else
		
		if( stricmp(a, "-aftercrash") == 0) {
			afterCrash = true;
		}

#ifdef WIN32
		// -console
		// Attaches a console window to the main LX window
		if (stricmp(a, "-console") == 0)  {
			if (AllocConsole())  {
			  FILE *con = freopen("CONOUT$", "w", stdout);
			  if (con) {
				*stdout = *con;
				setvbuf(stdout, NULL, _IONBF, 0);
			  }
			  SetConsoleTitle("OpenLieroX Console");
			}
		} else
#endif

		// -help
		// Displays help and quits
        if( !stricmp(a, "-h") || !stricmp(a, "-help") || !stricmp(a, "--help") || !stricmp(a, "/?")) {
        	printf("available parameters:\n");
			printf("   -connect srv  Connects to the server\n");
			printf("   -exec cmd     Executes the command in console\n");
     		printf("   -opengl       OpenLieroX will use OpenGL for drawing\n");
     		printf("   -noopengl     Explicitly disable using OpenGL\n");
     		printf("   -dedicated    Dedicated mode\n");
     		printf("   -nojoystick   Disable Joystick support\n");
     		printf("   -nosound      Disable sound\n");
     		printf("   -window       Run in window mode\n");
     		printf("   -fullscreen   Run in fullscreen mode\n");
			#ifdef WIN32
			printf("   -console      Attach a console window to the main OpenLieroX window\n");
			#endif
			#ifdef DEBUG
     		printf("   -nettest      Test CChannel reliability\n");
			#endif
     		printf("   -skin         Turns on new skinned GUI - it's unfinished yet\n");
     		printf("   -noskin       Turns off new skinned GUI\n");

			// Shutdown and quit
			// ShutdownLieroX() works only correct when everything was inited because ProcessEvents() is used.
			// Therefore we just ignore a good clean up and just quit here.
			// This is not nice but still nicer than getting a segfault.
			// It is not worth to fix this in a nicer way as it is fixed anyway when we use the new engine system.
     		exit(0);
        }
		#ifdef DEBUG
		if( !stricmp(a, "-nettest") )
		{
			InitializeLieroX();
			TestCChannelRobustness();
			ShutdownLieroX();
     		exit(0);
		}
		#endif
    }
}


///////////////////
// Initialize the game
int InitializeLieroX()
{
	notes << "Hello there, I am initializing me now..." << endl;

	LIBXML_TEST_VERSION;

	// Initialize the aux library
	if(!InitializeAuxLib("config.cfg",16,0)) {
        SystemError("strange problems with the aux library");
		return false;
	}

	// Setup the HTTP proxy
	AutoSetupHTTPProxy();

	tLX->bVideoModeChanged = false;
	tLX->bQuitGame = false;
	tLX->bQuitCtrlC = false;
	tLX->debug_string = "";
	tLX->currentTime = 0;
	tLX->fDeltaTime = 0;
	tLX->bHosted = false;

	if (!SkinnedGUI::InitializeGuiSkinning())
		return false;

	// Initialize the game colors (must be called after SDL_GetVideoSurface is not NULL and tLX is not NULL)
	DeprecatedGUI::InitializeColors();

	// Load the fonts (must be after colors, because colors are used inside CFont::Load)
	if (!DeprecatedGUI::LoadFonts())  {
		SystemError("Could not load the fonts");
		return false;
	}

	// Initialize the loading screen
	InitializeLoading();

	DrawLoading(5, "Initializing client and server");

	// Allocate the client & server
	cClient = new CClient;
	if(cClient == NULL) {
		SystemError("Error: InitializeLieroX() Out of memory");
		return false;
	}
	cClient->Clear();
	cClient->setLocalClient(true);



	cServer = new GameServer;
	if(cServer == NULL) {
		SystemError("Error: InitializeLieroX() Out of memory on creating GameServer");
		return false;
	}

	
	DrawLoading(10, "Loading Gusanos engine");
	gusInitBase();

	
	DrawLoading(15, "Initializing game entities");

	// Initialize the entities
	if(!InitializeEntities()) {
		SystemError("Error: InitializeEntities() Out of memory on initializing the entities");
		return false;
	}

	DrawLoading(20, "Loading graphics");


	// Load the graphics
	if(!DeprecatedGUI::LoadGraphics()) {
		SystemError("Error: Error loading graphics");
		return false;
	}

	DrawLoading(40, "Initializing console");

	// Initialize the console GUI
	if(!Con_Initialize()) {
		SystemError("Error: Could not initialize the console");
		return false;
	}

	DrawLoading(45, "Loading sounds");

	// Load the sounds
	LoadSounds();

	DrawLoading(55, "Loading player profiles");

	// Load the profiles
	LoadProfiles();

	if(bDedicated)
		if(!DedicatedControl::Init()) {
			errors << "couldn't init dedicated control" << endl;
			return false;
		}

	updateFileListCaches();
	
	notes << "Initializing ready" << endl;

	return true;
}


///////////////////
// Quit back to the menu
void QuittoMenu()
{
	SetQuitEngineFlag("QuittoMenu");
    DeprecatedGUI::Menu_SetSkipStart(false);
	cClient->Disconnect();
}

//////////////////
// Go to local menu
void GotoLocalMenu()
{
	if(tLX->iGameType == GME_HOST) {
		warnings << "called GotoLocalMenu as host, ignoring..." << endl;
		return;
	}

	if(tLX->iGameType == GME_JOIN) {
		warnings << "called GotoLocalMenu as client, ignoring..." << endl;
		return;
	}
	
	SetQuitEngineFlag("GotoLocalMenu");
	cClient->Disconnect();
	cServer->Shutdown();
	cClient->Shutdown();
	if(!bDedicated) {
		DeprecatedGUI::Menu_SetSkipStart(true);
		DeprecatedGUI::Menu_LocalInitialize();
	}
}

//////////////////
// Go to local menu
void GotoNetMenu()
{
	notes << "GotoNetMenu" << endl;
	SetQuitEngineFlag("GotoNetMenu");
	cClient->Disconnect();
	if(!bDedicated) {
		DeprecatedGUI::Menu_SetSkipStart(true);
		DeprecatedGUI::Menu_NetInitialize();
	}
}

////////////////////
// Initialize the loading screen
static void InitializeLoading()  {
	if(bDedicated) return; // ignore this case

	FillSurface(VideoPostProcessor::videoSurface(), Color(0,0,0));

	int bar_x, bar_y, bar_label_x, bar_label_y,bar_dir;
	bool bar_visible;
	std::string bar_direction;

	// Load the details
	ReadInteger("data/frontend/frontend.cfg", "Loading", "LoadingBarX", &bar_x, 210);
	ReadInteger("data/frontend/frontend.cfg", "Loading", "LoadingBarY", &bar_y, 230);
	ReadString ("data/frontend/frontend.cfg", "Loading", "LoadingBarDirection", bar_direction, "lefttoright");
	ReadInteger("data/frontend/frontend.cfg", "Loading", "PercentageLabelX", &bar_label_x, 0);
	ReadInteger("data/frontend/frontend.cfg", "Loading", "PercentageLabelY", &bar_label_y, 0);
	ReadKeyword("data/frontend/frontend.cfg", "Loading", "PercentageLabelVisible", &bar_visible, false);
	ReadInteger("data/frontend/frontend.cfg", "Loading", "BackgroundX", &cLoading.iBackgroundX, 190);
	ReadInteger("data/frontend/frontend.cfg", "Loading", "BackgroundY", &cLoading.iBackgroundY, 170);
	ReadInteger("data/frontend/frontend.cfg", "Loading", "LabelX", &cLoading.iLabelX, 235);
	ReadInteger("data/frontend/frontend.cfg", "Loading", "LabelY", &cLoading.iLabelY, 190);

	// Convert the loading direction
	if (!stringcasecmp(bar_direction,"lefttoright"))
		bar_dir = DeprecatedGUI::BAR_LEFTTORIGHT;
	else if (!stringcasecmp(bar_direction,"righttoleft"))
		bar_dir = DeprecatedGUI::BAR_RIGHTTOLEFT;
	else if (!stringcasecmp(bar_direction,"toptobottom"))
		bar_dir = DeprecatedGUI::BAR_TOPTOBOTTOM;
	else if (!stringcasecmp(bar_direction,"bottomtotop"))
		bar_dir = DeprecatedGUI::BAR_BOTTOMTOTOP;
	else
		bar_dir = DeprecatedGUI::BAR_LEFTTORIGHT;


	// Allocate bar
	cLoading.cBar = new DeprecatedGUI::CBar(LoadGameImage("./data/frontend/loading_bar.png",true), bar_x, bar_y, bar_label_x, bar_label_y, bar_dir);
	if (cLoading.cBar)
		cLoading.cBar->SetLabelVisible(bar_visible);

	// Load the background
	cLoading.bmpBackground = LoadGameImage("./data/frontend/background_loading.png", true);
}

/////////////////////
// Draw the loading
static void DrawLoading(byte percentage, const std::string &text)  {
	notes << "Loading (" << (int)percentage << "%): " << text << endl;

	if(bDedicated)
		return;
	if(cLoading.bmpBackground.get() == NULL)
		return;

	// Update the repainted area
	int x = MIN(cLoading.iBackgroundX, cLoading.cBar->GetX());
	int y = MIN(cLoading.cBar->GetY(), cLoading.iBackgroundY);
	int w = MAX(cLoading.bmpBackground.get()->w, cLoading.cBar->GetWidth());
	int h = MAX(cLoading.bmpBackground.get()->h, cLoading.cBar->GetHeight());
	DrawRectFill(VideoPostProcessor::videoSurface(), x, y, x+w, y+h, Color(0,0,0));

	if (cLoading.bmpBackground.get() != NULL)
		DrawImage(VideoPostProcessor::videoSurface(), cLoading.bmpBackground, cLoading.iBackgroundX, cLoading.iBackgroundY);

	if (cLoading.cBar)  {
		cLoading.cBar->SetPosition(percentage);
		cLoading.cBar->Draw( VideoPostProcessor::videoSurface() );
	}

	tLX->cFont.Draw(VideoPostProcessor::videoSurface(), cLoading.iLabelX, cLoading.iLabelY, tLX->clLoadingLabel, text);

	// we are in the main thread, so we can call this directly
	VideoPostProcessor::flipBuffers();
	VideoPostProcessor::process();
	flipRealVideo();
}

////////////////////
// Shutdown the loading screen
static void ShutdownLoading()  {
	if (cLoading.cBar)
		delete cLoading.cBar;
	cLoading.cBar = NULL;
}

///////////////////
// Shutdown the game
void ShutdownLieroX()
{
	notes << "Shutting me down..." << endl;
	
	// Options
	// Save already here in case some other method crashes
	if(!bDedicated) // only save if not in dedicated mode
		tLXOptions->SaveToDisc();
		
	DeprecatedGUI::CChatWidget::GlobalDestroy();	
	ShutdownIRC(); // Disconnect from IRC

	if(bDedicated)
		DedicatedControl::Uninit();

	if( ! bDedicated )
		ShutdownBackgroundMusic();

	ShutdownSounds();

    Con_Shutdown();

	ShutdownLoading();  // In case we're called when an error occured

	DeprecatedGUI::ShutdownGraphics();
	SkinnedGUI::ShutdownGuiSkinning();

	ShutdownFontCache();

	ServerList::get()->shutdown();

	DeprecatedGUI::Menu_Shutdown();
	// Only do the deregistration for widgets if we are not restarting.
	// The problem is that we have registered most widgets globally (not by any init-function)
	// so we would not reinit them.
	if(!bRestartGameAfterQuit)
		DeprecatedGUI::CGuiSkin::DeInit();
	ShutdownProfiles();

	// Free the IP to Country DB
	if (tIpToCountryDB) {
		delete tIpToCountryDB;
		tIpToCountryDB = NULL;
	}

	// Free the client & server
	if(cClient) {
		delete cClient;
		cClient = NULL;
	}

	if(cServer) {
		cServer->Shutdown();
		delete cServer;
		cServer = NULL;
	}

	// End logging
	if (convoLogger)  {
		if (tLXOptions->bLogConvos)
			convoLogger->endLogging();
		delete convoLogger;
		convoLogger = NULL;
	}

#ifdef WITH_G15
	if (OLXG15)
	{
		delete OLXG15;
		OLXG15 = NULL;
	}
#endif //WITH_G15
	// Entitites
	ShutdownEntities();

	// Event system
	ShutdownEventSystem();

	gusQuit();

	// SDL, Cache and other small stuff
	ShutdownAuxLib();

	// HINT: must be after shutting down the event system to free the timer correctly
#ifdef DEBUG
	ShutdownCacheDebug();
#endif
	
	// Save and clear options

	// HINT: save the options again because some could get changed in CServer/CClient destructors and shutdown functions
	// TODO: like what changes? why are there options saved both in CServer/CClient structure and in options?
	if(!bDedicated) // only save if not in dedicated mode
		tLXOptions->SaveToDisc();

	ShutdownOptions();

	// Only do the deregistration for variables if we are not restarting.
	// The problem is that we have registered most vars globally (not by any init-function)
	// so we would not reinit them.
	// Multiple registration of a var is also not a problem because we are
	// using a map for all vars and with a new registration, we would just overwrite
	// the old registration.
	if(!bRestartGameAfterQuit)
	{
		CScriptableVars::DeInit();
	}

	// Shutdown the timers
	ShutdownTimers();

	xmlCleanupParser();

	notes << "Everything was shut down" << endl;
}



struct CheckDirForMod {
	typedef FileListCacheIntf::FileList List;
	void operator()(List& filelist, const std::string& abs_filename) {
		ModInfo info = infoForMod(abs_filename, true);
		if(info.valid)
			filelist.insert( List::value_type(info.path, info.name) );
	}
};
static FileListCache<CheckDirForMod> modListInstance("mod", "", false, FM_DIR);
FileListCacheIntf* modList = &modListInstance;


struct CheckFileForSkin {
	typedef FileListCacheIntf::FileList List;
	void operator()(List& filelist, const std::string& abs_filename) {
		std::string ext = GetFileExtension(abs_filename);
		if(stringcasecmp(ext, "png")==0
		   || stringcasecmp(ext, "bmp")==0
		   || stringcasecmp(ext, "tga")==0
		   || stringcasecmp(ext, "pcx")==0) {
			std::string file = abs_filename;
			size_t slash = findLastPathSep(file);
			if(slash != std::string::npos)
				file.erase(0, slash+1);			
			std::string name = file.substr(0, file.size()-4); // the size-calcing here is safe
			filelist.insert( List::value_type(file, name) );
		}
	}
};
static FileListCache<CheckFileForSkin> skinListInstance("skin", "skins");
FileListCacheIntf* skinList = &skinListInstance;


struct CheckFileForSettingsPreset {
	typedef FileListCacheIntf::FileList List;
	void operator()(List& filelist, const std::string& abs_filename) {
		if(stringcaseequal(GetFileExtension(abs_filename), "cfg")) {
			std::string savedConfigSection;
			ReadString(abs_filename, "ConfigFileInfo", "Section", savedConfigSection, "", true);
			if(stringcaseequal(savedConfigSection, "GameOptions.GameInfo"))
				filelist.insert( List::value_type(GetBaseFilename(abs_filename), GetBaseFilenameWithoutExt(abs_filename)) );
		}
	}
};
static FileListCache<CheckFileForSettingsPreset> settingsPresetListInstance("settings preset", "cfg/presets");
FileListCacheIntf* settingsPresetList = &settingsPresetListInstance;


void updateFileListCaches() {
	// just create one single task for all because it wouldn't make it faster by doing that parallel
	struct Updater : Task {
		Updater() { name = "updateFileListCaches"; }
		Result handle() {
			if(breakSignal) return "break";
			mapList->update();
			if(breakSignal) return "break";
			modList->update();
			if(breakSignal) return "break";
			skinList->update();
			if(breakSignal) return "break";
			settingsPresetList->update();
			return true;
		}
		std::string statusText() {
			return "Updating file list cache ...";
		}
	};
	taskManager->start(new Updater(), TaskManager::QT_QueueToSameTypeAndBreakCurrent);
}



GameState currentGameState() {
	if(!cClient || cClient->getStatus() == NET_DISCONNECTED) return S_INACTIVE;
	if(tLX->iGameType == GME_JOIN) {
		if(cClient->getStatus() == NET_CONNECTING) return S_CLICONNECTING;
		if(!cClient->getGameReady()) return S_CLILOBBY;
		if(cClient->getStatus() == NET_PLAYING) return S_CLIPLAYING;
		return S_CLIWEAPONS;
	}
	if(!cServer->isServerRunning()) return S_INACTIVE;
	//if(!DeprecatedGUI::tMenu || DeprecatedGUI::tMenu->bMenuRunning);
	if(cServer->getState() == SVS_LOBBY) return S_SVRLOBBY;
	if(cServer->getState() == SVS_GAME) return S_SVRWEAPONS;
	return S_SVRPLAYING;
}

