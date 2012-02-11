#include <SDL.h>
#include <setjmp.h>

#include "MainLoop.h"
#include "Mutex.h"
#include "ReadWriteLock.h"
#include "AuxLib.h"
#include "EventQueue.h"
#include "Options.h"
#include "LieroX.h"
#include "Cache.h"
#include "OLXCommand.h"
#include "InputEvents.h"
#include "TaskManager.h"
#include "Timer.h"
#include "DeprecatedGUI/Menu.h"
#include "SkinnedGUI/CGuiSkin.h"
#include "CServer.h"
#include "IRC.h"
#include "CrashHandler.h"




// ParseArguments will set this eventually to true
bool afterCrash = false;
static bool afterCrashInformedUser = false;


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


#ifndef WIN32
sigjmp_buf longJumpBuffer;
#endif

#ifndef SINGLETHREADED
static SDL_Event QuitEventThreadEvent() {
	SDL_Event ev;
	ev.type = SDL_USEREVENT;
	ev.user.code = UE_QuitEventThread;
	return ev;
}
#endif


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


void doMainLoop() {
#ifdef SINGLETHREADED
	MainLoopTask mainLoop;
#else
	ThreadPoolItem* mainLoopThread = threadPool->start(new MainLoopTask(), "main loop", false);

	startMainLockDetector();
#endif

	if(!bDedicated) {
		// Get all SDL events and push them to our event queue.
		// We have to do that in the same thread where we inited the video because of SDL.
		SDL_Event ev;
		memset( &ev, 0, sizeof(ev) );
		while(true) {
#ifdef SINGLETHREADED
			while( SDL_PollEvent(&ev) ) {
#else
			while( SDL_WaitEvent(&ev) ) {
#endif
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

#ifdef SINGLETHREADED
			if(NegResult r = mainLoop.handleFrame())
				break;
#elif
			notes << "error while waiting for next event" << endl;
			SDL_Delay(200);
#endif
		}
	}

quit:
#ifndef SINGLETHREADED
	threadPool->wait(mainLoopThread, NULL);
	mainLoopThread = NULL;
#endif
	{}
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
	ResetQuitEngineFlag();

	DeprecatedGUI::tMenu->bMenuRunning = true;
	DeprecatedGUI::tMenu->bMenuWantsGameStart = false;

	if(!bDedicated) {
		if(!DeprecatedGUI::bSkipStart) {
			notes << "Loading main menu" << endl;
			DeprecatedGUI::tMenu->iMenuType = DeprecatedGUI::MNU_MAIN;
			DeprecatedGUI::Menu_MainInitialize();
		} else
			DeprecatedGUI::Menu_RedrawMouse(true);
	}

	DeprecatedGUI::bSkipStart = false;

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

#ifdef SINGLETHREADED
	last_frame_was_because_of_an_event = ProcessEvents();
#else
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
#endif

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

	if(!DeprecatedGUI::tMenu->bMenuWantsGameStart) {
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
#ifndef SINGLETHREADED
	SDL_Event quitEv = QuitEventThreadEvent();
	if(!bDedicated)
		while(SDL_PushEvent(&quitEv) < 0) {}
#endif
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

