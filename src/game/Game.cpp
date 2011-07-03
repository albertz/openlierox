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
#include "gusanos/gusanos.h"
#include "gusanos/gusgame.h"
#include "gusanos/ninjarope.h"
#include "game/WormInputHandler.h"
#include "CWormHuman.h"
#include "gusanos/glua.h"
#include "gusanos/luaapi/context.h"
#include "sound/sfx.h"
#include "gusanos/network.h"
#include "OLXConsole.h"
#include "game/SinglePlayer.h"
#include "game/SettingsPreset.h"

#include <boost/shared_ptr.hpp>

Game game;

static bool inMainGameLoop = false;
static std::string quitEngineFlagReason;

static bool DbgSimulateSlow = false;

static bool bRegisteredDebugVars = CScriptableVars::RegisterVars("Debug.Game")
( DbgSimulateSlow, "SimulateSlow" );


void Game::prepareGameloop() {
	// Pre-game initialization
	if(!bDedicated) FillSurface(VideoPostProcessor::videoSurface(), tLX->clBlack);
	
	while(cClient->getStatus() != NET_CONNECTED) {
		notes << "client not connected yet - waiting" << endl;
		SDL_Delay(10);
		SyncServerAndClient();
	}
	
	if(tLX->iGameType != GME_JOIN) {
		if(cServer->getState() == SVS_LOBBY) {
			notes << "prepareGameloop: starting game" << endl;
			std::string errMsg;
			if(!cServer->StartGame(&errMsg)) {
				errors << "starting game in local game failed for reason: " << errMsg << endl;
				DeprecatedGUI::Menu_MessageBox("Error", "Error while starting game: " + errMsg);
				if (tLX->iGameType == GME_LOCAL)
					GotoLocalMenu();
				else
					GotoNetMenu();
				return;
			}
		}
		else
			warnings << "prepareGameloop: server was not in lobby" << endl;
	}

	// we need the gamescript in physics init
	while(gameScript() == NULL) {
		notes << "gamescript not loaded yet - waiting" << endl;
		SDL_Delay(10);
		SyncServerAndClient();
	}
	
	if(isClient() && cClient->getServerVersion() < OLXBetaVersion(0,59,6)) {
		// All the custom settings we may set in the game mod were unknown (as feature array settings)
		// to earlier versions. Thus we overwrite it this way.
		gameScript()->lx56modSettings.copyTo( cClient->getGameLobby() );
	}
		
	if(isServer()) {
		// resend lua event index to everyone
		network.sendEncodedLuaEvents(INVALID_CONN_ID);		
	}
	
	PhysicsEngine::Init();
		
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
	if(DbgSimulateSlow) SDL_Delay(700);
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

	//if(gusGame.isEngineNeeded()) {
	// do lua frames in all cases
	{
		// convert speed to lua if needed
		boost::shared_ptr<CGameObject::ScopedGusCompatibleSpeed> scopedSpeeds[MAX_WORMS];
		for(int i = 0; i < MAX_WORMS; ++i)
			if(cClient->getRemoteWorms()[i].isUsed())
				scopedSpeeds[i].reset( new CGameObject::ScopedGusCompatibleSpeed(cClient->getRemoteWorms()[i]) );
		
		gusLogicFrame();
		
		if(gameScript()->gusEngineUsed() && game.isServer()) {
			// copy worm states to CServer worms because they are never updated in this mode
			for(int i = 0; i < MAX_WORMS; ++i) {
				if(cServer->getWorms()[i].isUsed() && cClient->getRemoteWorms()[i].isUsed()) {
					cServer->getWorms()[i].health = cClient->getRemoteWorms()[i].health;
					cServer->getWorms()[i].vPos = cClient->getRemoteWorms()[i].vPos;
					cServer->getWorms()[i].vVelocity = cClient->getRemoteWorms()[i].vVelocity;
					cServer->getWorms()[i].bAlive = cClient->getRemoteWorms()[i].bAlive;					
				}
			}
		}
	}
	/*} else {
		// do stuff here which we took from Gusanos, which is done in gusLogicFrame and should be done in any case
		sfx.think();
	}*/
	
	// Local
	switch (tLX->iGameType)  {
		case GME_LOCAL:
			cClient->Frame();
			cServer->Frame();
			
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
	
	// can happen if we have aborted a game
	if(isServer() && !cServer->getGameOver())
		// call gameover because we may do some important cleanup there
		game.gameMode()->GameOver();
	
	gusGame.reset(GusGame::ServerQuit);
	
	PhysicsEngine::UnInit();
	
	notes << "GameLoopEnd: " << quitEngineFlagReason << endl;
	inMainGameLoop = false;
	if( DedicatedControl::Get() )
		DedicatedControl::Get()->GameLoopEnd_Signal();		
	
	cCache.ClearExtraEntries(); // Game ended - clear cache	

	// Cleanup settings layer so that while being in lobby,
	// no outdated settings are used for GameServer::isVersionCompatible.
	// This can (and should) be removed once we have the settings already loaded
	// immediately in lobby.
	modSettings.makeSet(false);
	gamePresetSettings.makeSet(false);
	
	cleanupCallbacks();
	cleanupCallbacks.disconnect_all_slots();
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


void Game::onNewWorm(CWorm* w) {
	//if(!game.gameScript()->gusEngineUsed()) return;

	objects.insertImmediately(w, Grid::WormColLayer, Grid::WormRenderLayer);
	if(w->getNinjaRopeObj()) objects.insertImmediately(w->getNinjaRopeObj(), 1, 1);
}

void Game::onRemoveWorm(CWorm* w) {
/*
	objects.(w, Grid::WormColLayer, Grid::WormRenderLayer);
	if(w->getNinjaRopeObj()) objects.insertImmediately(w->getNinjaRopeObj(), 1, 1);
*/
}

///////////////////
// Adds a new player to vector of players.
void Game::onNewPlayer(CWormInputHandler* player) {
	players.push_back( player );	
}

void Game::onNewPlayer_Lua(CWormInputHandler* p) {
	if(game.gameScript()->gusEngineUsed()) {
		EACH_CALLBACK(i, playerInit)
		{
			(lua.call(*i), p->getLuaReference())();
		}	
	}
}

///////////////////
// Adds a new player to vector of human local players.
void Game::onNewHumanPlayer(CWormHumanInputHandler* player) {
	localPlayers.push_back( player );	
	player->local = true;
}

void Game::onNewHumanPlayer_Lua(CWormHumanInputHandler* player) {
	if(game.gameScript()->gusEngineUsed()) {
		EACH_CALLBACK(i, localplayerInit)
		{
			(lua.call(*i), player->getLuaReference())();
		}
	}
}

///////////////////
// Removes players and clears all game objects.
void Game::reset() {
	notes << "Game::reset" << endl;
	
	// Delete all players
	for ( std::vector<CWormInputHandler*>::iterator iter = players.begin(); iter != players.end(); ++iter)
	{
		(*iter)->deleteThis();
	}
	players.clear();
	localPlayers.clear();
	
	// we must call this first because the references to weapons, ninjarope and what may be deleted
	if(cClient && cClient->getRemoteWorms())
		for(int i = 0; i < MAX_WORMS; ++i)
			cClient->getRemoteWorms()[i].gusShutdown();
	
	// Delete all objects
	objects.clear();
}

CMap* Game::gameMap() {
	return &gusGame.level();
}

CGameScript* Game::gameScript() {
	if(tLX) {
		if(tLX->iGameType == GME_JOIN) return cClient->getGameScript().get();
		return cServer->getGameScript().get();
	}
	return NULL;
}

CGameMode* Game::gameMode() {
	if(tLX) {
		if(tLX->iGameType != GME_JOIN) return gameSettings[FT_GameMode].as<GameModeInfo>()->mode;
		return cClient->getGameLobby()[FT_GameMode].as<GameModeInfo>()->mode;
	}
	return NULL;
}

CWpnRest* Game::weaponRestrictions() {
	if(isServer() && cServer) return cServer->getWeaponRestrictions();
	if(isClient() && cClient) return cClient->getWeaponRestrictions();
	return NULL;
}

///////////////////
// Returns true if the game is a server or false otherwise.
bool Game::isServer() {
	return tLX->iGameType != GME_JOIN;
}

bool Game::needProxyWormInputHandler() {
	return isServer();
}

bool Game::needToCreateOwnWormInputHandlers() {
	return isServer() || (cClient->getServerVersion() < OLXBetaVersion(0,59,1));
}

///////////////////
// Returns true if game is team game or false otherwise.
bool Game::isTeamPlay() {
	return cClient->isTeamGame();
}


bool CClient::getGamePaused() {
	return (tLX->iGameType == GME_LOCAL) && (bGameOver || bViewportMgr || bGameMenu || Con_IsVisible());
}

bool Game::isGamePaused() {
	// If we're in a menu & a local game, don't do simulation
	return cClient->getGamePaused();
}

bool Game::shouldDoPhysicsFrame() {
	return !isGamePaused() && cClient->canSimulate() &&
    // We stop a few seconds after the actual game over
	!(cClient->bGameOver && (tLX->currentTime - cClient->fGameOverTime).seconds() > GAMEOVER_WAIT);
}

