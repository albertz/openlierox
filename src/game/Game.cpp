/*
 *  Game.cpp
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 09.12.09.
 *  code under LGPL
 *
 */

#include "Game.h"
#include "AuxLib.h"
#include "Debug.h"
#include "LieroX.h"
#include "GfxPrimitives.h"
#include "Entity.h"
#include "EventQueue.h"
#include "InputEvents.h"
#include "DedicatedControl.h"
#include "CrashHandler.h"
#include "Timer.h"
#include "NewNetEngine.h"
#include "OLXCommand.h"
#include "IRC.h"
#include "CClient.h"
#include "CServer.h"
#include "Physics.h"
#include "DeprecatedGUI/Menu.h"
#include "Cache.h"

Game game;

static bool inMainGameLoop = false;
static std::string quitEngineFlagReason;

void Game::prepareGameloop() {
	// Pre-game initialization
	if(!bDedicated) FillSurface(VideoPostProcessor::videoSurface(), tLX->clBlack);
	
	ClearEntities();
	
	ProcessEvents();
	notes << "MaxFPS is " << tLXOptions->nMaxFPS << endl;
	
	//cCache.ClearExtraEntries(); // Do not clear anything before game started, it may be slow
	
	notes << "GameLoopStart" << endl;
	inMainGameLoop = true;
	if( DedicatedControl::Get() )
		DedicatedControl::Get()->GameLoopStart_Signal();
	
	CrashHandler::recoverAfterCrash = tLXOptions->bRecoverAfterCrash && GetGameVersion().releasetype == Version::RT_NORMAL;
	
	ResetQuitEngineFlag();
	oldtime = GetTime();	
}

void Game::frameOuter() {
	tLX->currentTime = GetTime();
	SetCrashHandlerReturnPoint("main game loop");
	
	// Timing
	tLX->fDeltaTime = tLX->currentTime - oldtime;
	tLX->fRealDeltaTime = tLX->fDeltaTime;
	oldtime = tLX->currentTime;
	
	// cap the delta
	if(tLX->fDeltaTime.seconds() > 0.5f) {
		warnings << "deltatime " << tLX->fDeltaTime.seconds() << " is too high" << endl;
		// only if not in new net mode because it would screw up the gamestate there
		if(!NewNet::Active())
			tLX->fDeltaTime = 0.5f; // don't simulate more than 500ms, it could crash the game
	}
	
	ProcessEvents();
	
	// Main frame
	frameInner();
	
	doVideoFrameInMainThread();
	CapFPS();
}



///////////////////
// Game loop
void Game::frameInner()
{
	HandlePendingCommands();
	
	if(bDedicated)
		DedicatedControl::Get()->GameLoop_Frame();
	
    if(tLX->bQuitEngine)
        return;
	
	// Check if user pressed screenshot key
	if (tLX->cTakeScreenshot.isDownOnce())  {
		PushScreenshot("scrshots", "");
	}
	
	// Switch between window and fullscreen mode
	// Switch only if delta time is low enough. This is because when the game does not
	// respond for >30secs and the user presses cSwitchMode in the meantime, the mainlock-detector
	// would switch to window and here we would switch again to fullscreen which is stupid.
	if( tLX->cSwitchMode.isUp() && tLX && tLX->fRealDeltaTime < 1.0f )  {
		// Set to fullscreen
		tLXOptions->bFullscreen = !tLXOptions->bFullscreen;
		
		// Set the new video mode
		doSetVideoModeInMainThread();
		
		tLX->cSwitchMode.reset();
	}
	
#ifdef WITH_G15
	if (OLXG15)
		OLXG15->gameFrame();
#endif //WITH_G15
	
	if(tLXOptions->bEnableChat)
		ProcessIRC();
	
	// Local
	switch (tLX->iGameType)  {
		case GME_LOCAL:
			cClient->Frame();
			cServer->Frame();
			
			// If we are connected, just start the game straight away (bypass lobby in local)
			if(cClient->getStatus() == NET_CONNECTED) {
				if(cServer->getState() == SVS_LOBBY) {
					std::string errMsg;
					if(!cServer->StartGame(&errMsg)) {
						errors << "starting game in local game failed for reason: " << errMsg << endl;
						DeprecatedGUI::Menu_MessageBox("Error", "Error while starting game: " + errMsg);
						GotoLocalMenu();
						return;
					}
				}
			}
			
			if(tLX && !tLX->bQuitEngine)
				cClient->Draw(VideoPostProcessor::videoSurface());
			break;
			
			
			// Hosting
		case GME_HOST:
			cClient->Frame();
			cServer->Frame();
			
			if(tLX && !tLX->bQuitEngine)
				cClient->Draw(VideoPostProcessor::videoSurface());
			break;
			
			// Joined
		case GME_JOIN:
			cClient->Frame();
			if(tLX && !tLX->bQuitEngine)
				cClient->Draw(VideoPostProcessor::videoSurface());
			break;
			
	} // SWITCH
	
	cClient->resetDebugStr();
	
	EnableSystemMouseCursor(false);
}


void Game::cleanupAfterGameloopEnd() {
	CrashHandler::recoverAfterCrash = false;
	
	PhysicsEngine::Get()->uninitGame();
	
	notes << "GameLoopEnd: " << quitEngineFlagReason << endl;
	inMainGameLoop = false;
	if( DedicatedControl::Get() )
		DedicatedControl::Get()->GameLoopEnd_Signal();		
	
	cCache.ClearExtraEntries(); // Game ended - clear cache	
}




void ResetQuitEngineFlag() {
	tLX->bQuitEngine = false;
}

void SetQuitEngineFlag(const std::string& reason) {
	Warning_QuitEngineFlagSet("SetQuitEngineFlag(" + reason + "): ");
	quitEngineFlagReason = reason;
	tLX->bQuitEngine = true;
	// If we call this from within the menu, the menu should shutdown.
	// It will be restarted then in the next frame.
	// If we are not in the menu (i.e. in maingameloop), this has no
	// effect as we set it to true in Menu_Start().
	if(DeprecatedGUI::tMenu)
		DeprecatedGUI::tMenu->bMenuRunning = false;
	// If we were in menu, because we forced the menu restart above,
	// we must set this, otherwise OLX would quit (because of current maingamelogic).
	if(DeprecatedGUI::bGame)
		*DeprecatedGUI::bGame = true;
}

bool Warning_QuitEngineFlagSet(const std::string& preText) {
	if(tLX->bQuitEngine) {
		hints << preText << endl;
		warnings << "bQuitEngine is set because: " << quitEngineFlagReason << endl;
		return true;
	}
	return false;
}



