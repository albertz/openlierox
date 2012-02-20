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


struct MainLoopTask : LoopTask {
	enum State {
		State_Startup,
		State_Loop,
		State_Quit
	};
	State state;
	MainLoopTask() : state(State_Startup) {}
	Result handle_Startup();
	Result handle_Loop();
	Result handle_Quit();
	Result handleFrame();
};



static bool handleSDLEvent(SDL_Event& ev) {
	if(ev.type == SDL_USEREVENT) {
		switch(ev.user.code) {
			case UE_QuitEventThread:
				return false;
			case UE_DoVideoFrame:
				videoHandler.frame();
				return true;
			case UE_DoSetVideoMode:
				videoHandler.setVideoMode();
				return true;
			case UE_DoActionInMainThread:
				((Action*)ev.user.data1)->handle();
				delete (Action*)ev.user.data1;
				return true;
		}
	}
	if( ev.type == SDL_SYSWMEVENT ) {
		EvHndl_SysWmEvent_MainThread( &ev );
		return true;
	}
	mainQueue->push(ev);
	return true;
}

// Get all SDL events and push them to our event queue.
// We have to do that in the same thread where we inited the video because of SDL.
bool handleSDLEvents(bool wait) {
	assert(isMainThread());

	SDL_Event ev;
	memset( &ev, 0, sizeof(ev) );

	if(wait) {
		if( SDL_WaitEvent(&ev) ) {
			if(!handleSDLEvent(ev))
				return false;
		}
		else {
			notes << "error while waiting for next event" << endl;
			SDL_Delay(200);
			return true;
		}
	}

	while( SDL_PollEvent(&ev) ) {
		if(!handleSDLEvent(ev))
			return false;
	}

	return true;
}

void doMainLoop() {
#ifdef SINGLETHREADED
	MainLoopTask mainLoop;
	while(true) {
		// we handle SDL events in doVideoFrameInMainThread
		if(NegResult r = mainLoop.handleFrame())
			break;
	}
#else
	ThreadPoolItem* mainLoopThread = threadPool->start(new MainLoopTask(), "main loop", false);

	startMainLockDetector();

	if(!bDedicated) {
		while(true) {
			if(!handleSDLEvents(true))
				goto quit;
		}
	}

quit:
	threadPool->wait(mainLoopThread, NULL);
	mainLoopThread = NULL;
#endif
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
	if(isMainThread()) {
		VideoPostProcessor::flipBuffers();
		VideoPostProcessor::process();
		flipRealVideo();
	}
	else
		videoHandler.pushFrame();
}

void doSetVideoModeInMainThread() {
	if(bDedicated) return;
	if(isMainThread())
		videoHandler.setVideoMode();
	else
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

	if(isMainThread()) {
		act->handle();
		delete act;
	}
	else {
		SDL_Event ev;
		ev.type = SDL_USEREVENT;
		ev.user.code = UE_DoActionInMainThread;
		ev.user.data1 = act;
		if(SDL_PushEvent(&ev) != 0) {
			errors << "failed to push custom action event" << endl;
		}
	}
}


Result MainLoopTask::handle_Startup() {
	setCurThreadPriority(0.5f);

	// NOTE: This code is really old and outdated.
	// We might just merge that with the new code.
	// Otherwise, it will never work like this because of
	// missing stuff, e.g. no game.init call (which does
	// also init the old menu, so we cannot simply call it
	// like this right now).
	if( tLXOptions->bNewSkinnedGUI )
	{
		// Just for test - it's not real handler yet
		SkinnedGUI::cMainSkin->Load("default");
		SkinnedGUI::cMainSkin->OpenLayout("test.skn");
		while (!tLX->bQuitGame)  {
			tLX->fDeltaTime = GetTime() - tLX->currentTime;
			tLX->currentTime = GetTime();

			ProcessEvents();
			SkinnedGUI::cMainSkin->Frame();
		}

		return "quit";
	}

	state = State_Loop;
	return true;
}

Result MainLoopTask::handle_Loop() {
	if(tLX->bQuitGame) {
		state = State_Quit;
		return true;
	}

	game.frame();
	return true;
}

Result MainLoopTask::handle_Quit() {
	if(!isMainThread()) {
		SDL_Event quitEv = QuitEventThreadEvent();
		if(!bDedicated)
			while(SDL_PushEvent(&quitEv) < 0) {}
	}
	return "quit";
}

Result MainLoopTask::handleFrame() {
	switch(state) {
	case State_Startup: return handle_Startup();
	case State_Loop: return handle_Loop();
	case State_Quit: return handle_Quit();
	}
	return "invalid state";
}


