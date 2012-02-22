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
#include "game/WormInputHandler.h"
#include "CWormHuman.h"
#include "gusanos/glua.h"
#include "gusanos/luaapi/context.h"
#include "sound/sfx.h"
#include "OLXConsole.h"
#include "game/SinglePlayer.h"
#include "game/SettingsPreset.h"
#include "CGameScript.h"
#include "ProfileSystem.h"
#include "Attr.h"
#include "gusanos/luaapi/classes.h"
#include "gusanos/network.h"
#include "FlagInfo.h"
#include "CWpnRest.h"

#include <boost/shared_ptr.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/bind.hpp>

Game game;

static bool inMainGameLoop = false;

static bool DbgSimulateSlow = false;

static bool bRegisteredDebugVars = CScriptableVars::RegisterVars("Debug.Game")
( DbgSimulateSlow, "SimulateSlow" );



Game::Game() {
	wasPrepared = false;
	menuFrame = 0;
	uniqueObjId = LuaID<Game>::value;
	m_isServer = false;
	m_isLocalGame = false;
	state = S_Inactive;
	m_wpnRest = new CWpnRest();
}

void Game::init() {
	assert(state == S_Inactive);
	prepareMenu();
}

void Game::startServer(bool localGame) {
	m_isServer = true;
	m_isLocalGame = localGame;
	state = S_Lobby;
}

void Game::startClient() {
	m_isServer = false;
	m_isLocalGame = false;
	state = S_Connecting;
}

void Game::startGame() {
	if(!isServer()) {
		errors << "startGame as client" << endl;
		return;
	}

	if(state != Game::S_Lobby)
		warnings << "startGame: expected to be in lobby but game state is " << game.state << endl;

	state = Game::S_Preparing;
	gameOver = false;
	serverFrame = 0;
}

void Game::stop() {
	m_isServer = false;
	m_isLocalGame = false;
	state = S_Inactive;
}

void checkCurrentGameState() {
	if(!cClient || cClient->getStatus() == NET_DISCONNECTED) {
		// S_INACTIVE;
		if(game.state != Game::S_Inactive)
			errors << "client is disconnected but game state is " << game.state << endl;
		return;
	}
	if(game.isClient()) {
		if(cClient->getStatus() == NET_CONNECTING) {
			// S_CLICONNECTING;
			if(game.state != Game::S_Connecting)
				errors << "client is connecting but game state is " << game.state << endl;
			return;
		}
		if(game.state < Game::S_Preparing) {
			// S_CLILOBBY;
			if(game.state != Game::S_Lobby)
				errors << "client is connected and game is not ready but game state is " << game.state << endl;
			return;
		}
		if(cClient->getStatus() == NET_PLAYING) {
			// S_CLIPLAYING;
			if(game.state != Game::S_Playing)
				errors << "client is connected and playing but game state is " << game.state << endl;
			return;
		}
		// S_CLIWEAPONS;
		if(game.state != Game::S_Preparing)
			errors << "client is connected and should be preparing game but game state is " << game.state << endl;
		return;
	}
	if(!cServer->isServerRunning()) {
		// S_INACTIVE;
		if(game.state != Game::S_Inactive)
			errors << "server is not running but game state is " << game.state << endl;
		return;
	}
	//if(!DeprecatedGUI::tMenu || DeprecatedGUI::tMenu->bMenuRunning);
}


void Game::onSettingsUpdate(BaseObject* /* oPt */, const AttrDesc* attrDesc, ScriptVar_t /* oldValue */) {
	assert(attrDesc->objTypeId == LuaID<Settings>::value);
	FeatureIndex featureIndex = Settings::getAttrDescs().getIndex(attrDesc);

	switch(featureIndex) {
	case FT_Map:
	case FT_GameMode:
	case FT_Mod:
		if(game.wasPrepared)
			// We need a re-init. This will do it.
			game.cleanupAfterGameloopEnd();
	default: break; // nop
	}
}

void Game::onStateUpdate(BaseObject* oPt, const AttrDesc* attrDesc, ScriptVar_t oldValue) {
	assert(oPt == &game);
	assert(attrDesc == game.state.attrDesc());

	if((int)oldValue >= Game::S_Preparing) {
		if(game.state <= Game::S_Lobby) {
			game.cleanupAfterGameloopEnd();
			game.prepareMenu();
		}
	}
}

void Game::prepareMenu() {
	menuFrame = 0;

	cClient->SetSocketWithEvents(true);
	cServer->SetSocketWithEvents(true);

	if(!bDedicated) {
		if(!DeprecatedGUI::bSkipStart) {
			notes << "Loading main menu" << endl;
			DeprecatedGUI::tMenu->iMenuType = DeprecatedGUI::MNU_MAIN;
			DeprecatedGUI::Menu_MainInitialize();
		} else
			DeprecatedGUI::Menu_RedrawMouse(true);
	}

	DeprecatedGUI::bSkipStart = false;

	tLX->currentTime = menuStartTime = GetTime();
}

Result Game::prepareGameloop() {
	// Pre-game initialization
	if(!bDedicated) FillSurface(VideoPostProcessor::videoSurface(), tLX->clBlack);
	
	// If we go out of the menu, it means the user has selected something.
	// This indicates that everything is fine, so we should restart in case of a crash.
	// Note that we will set this again to false later on in case the user quitted.
	CrashHandler::restartAfterCrash = true;

	cClient->SetSocketWithEvents(false);
	cServer->SetSocketWithEvents(false);

	if(isClient()) {
		// check if we have level
		std::string sMapFilename = "levels/" + cClient->getGameLobby()[FT_Map].as<LevelInfo>()->path;
		if(CMap::GetLevelName(GetBaseFilename(sMapFilename)) == "") {
			cClient->DownloadMap(GetBaseFilename(sMapFilename));  // Download the map
			// we have bDownloadingMap = true when this was successfull
		}

		if(cClient->bDownloadingMod)
			cClient->bWaitingForMod = true;
		if(cClient->bDownloadingMap)
			cClient->bWaitingForMap = true;
	}

	if(NegResult r = game.loadMod())
		return "Error while loading mod: " + r.res.humanErrorMsg;

	if(NegResult r = game.loadMap())
		return "Error while loading map: " + r.res.humanErrorMsg;

	game.gameMap()->SetMinimapDimensions(cClient->tInterfaceSettings.MiniMapW, cClient->tInterfaceSettings.MiniMapH);
	cClient->bMapGrabbed = true;

	// always also load Gusanos engine
	// even with LX-stuff-only, we may access/need it (for network stuff and later probably more)
	if( !game.gameMap()->gusIsLoaded() && (game.isServer() || cClient->getServerVersion() >= OLXBetaVersion(0,59,1) ) ) {
		// WARNING: This may be temporary
		// Right now, we load the gus mod in the map loader (gusGame.changeLevel).
		// Thus, when we don't load a gus level, we must load the mod manually.

		if(!game.gameScript()->gusEngineUsed())
			gusGame.setMod(gusGame.getDefaultPath());
		gusGame.loadModWithoutMap();
	}

	if(gusGame.isEngineNeeded()) {
		// Unprepare all worms first.
		// Some init scripts (e.g. Promode) have trouble when there are players
		// but their bindings.playerInit was not called.
		// As the bindings.playerInit is firstly set here by the init scripts,
		// there is no other way.
		// The worms will get prepared later on again.
		// TODO: do the preparation client/server independently somewhere else
		// TODO: as well as the gus loading/init stuff
		for_each_iterator(CWorm*, w, game.worms())
			w->get()->Unprepare();

		gusGame.runInitScripts();
	}

	if(game.isServer()) {
		std::string errMsg;
		if(!cServer->PrepareGame(&errMsg)) {
			if (game.isLocalGame())
				GotoLocalMenu();
			else
				GotoNetMenu();
			return "starting game in local game failed for reason: " + errMsg;
		}
	}

	while(cClient->getStatus() == NET_CONNECTING) {
		notes << "client not connected yet - waiting" << endl;
		SDL_Delay(10);
		SyncServerAndClient();
	}
	if(cClient->getStatus() == NET_DISCONNECTED) {
		warnings << "prepareGameLoop: something went wrong, client not connected anymore" << endl;
		game.state = Game::S_Inactive;
		return "client did not connect";
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
		gameScript()->lx56modSettings.copyTo( cClient->getGameLobby().overwrite );
	}
		
	if(isServer()) {
		// resend lua event index to everyone
		network.sendEncodedLuaEvents(INVALID_CONN_ID);		
	}
	
	PhysicsEngine::Init();
		
	ClearEntities();

	// in server mode, server would reset this
	if(game.isClient())
		cClient->permanentText = "";

	cClient->flagInfo()->reset();
	for(int i = 0; i < MAX_TEAMS; ++i) {
		cClient->iTeamScores[i] = 0;
	}

	cClient->projPosMap.clear();
	cClient->projPosMap.resize(CClient::MapPosIndex( VectorD2<int>(game.gameMap()->GetWidth(), game.gameMap()->GetHeight())).index(game.gameMap()) );
	cClient->cProjectiles.clear();

	if( GetGlobalIRC() )
		GetGlobalIRC()->setAwayMessage("Playing: " + cClient->getServerName());

	ProcessEvents();
	notes << "MaxFPS is " << tLXOptions->nMaxFPS << endl;
	
	//cCache.ClearExtraEntries(); // Do not clear anything before game started, it may be slow
	
	notes << "GameLoopStart" << endl;
	inMainGameLoop = true;
	if( DedicatedControl::Get() )
		DedicatedControl::Get()->GameLoopStart_Signal();
	
	CrashHandler::recoverAfterCrash = tLXOptions->bRecoverAfterCrash && GetGameVersion().releasetype == Version::RT_NORMAL;
	
	simulationTime = oldtime = GetTime();
	wasPrepared = true;
	return true;
}

/*
uint64_t calcFramesLeft(AbsTime curTime, AbsTime curSimTime) {
	TimeDiff diff = curTime - curSimTime;
	return (diff.milliseconds() + Game::FixedFrameTime - 1) / Game::FixedFrameTime;
}
*/

static TimeDiff simulationDelay() {
	return GetTime() - tLX->currentTime;
	/*
	if(simulationTime > tLX->currentTime) { // inside the 100FPS loop
		curTime = simulationTime;
		curSimTime = tLX->currentTime;
		uint64_t framesLeft = calcFramesLeft(curTime, curSimTime);
		TimeDiff timeLeft = TimeDiff(framesLeft * Game::FixedFrameTime);
	}
	*/
}

bool Game::hasHighSimulationDelay() { return simulationDelay() > TimeDiff(100); }
bool Game::hasSeriousHighSimulationDelay() { return simulationDelay() > TimeDiff(200); }

void Game::frame() {
	SetCrashHandlerReturnPoint("main game loop");

	// Timing
	tLX->currentTime = GetTime();
	tLX->fDeltaTime = tLX->currentTime - oldtime;
	tLX->fRealDeltaTime = tLX->fDeltaTime;
	oldtime = tLX->currentTime;

	ProcessEvents();

	// Main frame
	frameInner();

	if(game.state <= Game::S_Lobby) {
		DeprecatedGUI::Menu_Frame();
		menuFrame++;

		// If we have run fine for >=5 seconds and 10 frames, it is probably
		// safe & make sense to restart the game in case of a crash.
		if(menuFrame >= 10 && tLX->currentTime - menuStartTime >= TimeDiff(5.0f))
			CrashHandler::restartAfterCrash = true;
	}

	if(DbgSimulateSlow) SDL_Delay(700);

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

	// no lobby for local games
	if(game.state == Game::S_Lobby && game.isLocalGame())
		game.startGame();

	if(!wasPrepared && game.state >= Game::S_Preparing) {
		Result r = prepareGameloop();
		if(!r) {
			warnings << "prepageGameloop failed: " << r.humanErrorMsg << endl;
			if(!bDedicated)
				DeprecatedGUI::Menu_MessageBox("Error", "Error while starting game: " + r.humanErrorMsg);
			game.state = Game::S_Lobby;
		}
		else if(!wasPrepared) {
			errors << "prepageGameloop: no error but not prepared" << endl;
			return;
		}
	}

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

	if(state != Game::S_Inactive) {
		cClient->ReadPackets();

		cClient->ProcessMapDownloads();
		cClient->ProcessModDownloads();
		cClient->ProcessUdpUploads();

		cClient->SimulateHud();

		if(isServer() && cServer->isServerRunning()) {
			cServer->ProcessRegister();
			cServer->ProcessGetExternalIP();
			cServer->ReadPackets();
		}
	}

	if(state >= Game::S_Preparing) {
		// We have a separate fixed 100FPS for game simulation.
		// Because much old code uses tLX->{currentTime, fDeltaTime, fRealDeltaTime},
		// we have to set it accordingly.
		AbsTime curTime = tLX->currentTime;
		TimeDiff curDeltaTime = tLX->fDeltaTime;
		tLX->currentTime = simulationTime;
		tLX->fDeltaTime = TimeDiff(Game::FixedFrameTime);
		tLX->fRealDeltaTime = TimeDiff(Game::FixedFrameTime);
		while(tLX->currentTime < curTime) {

			if(hasSeriousHighSimulationDelay()) {
				TimeDiff simDelay = simulationDelay();
				if(simDelay > 0.5f)
					warnings << "deltatime " << simDelay.seconds() << " is too high" << endl;
				// Don't do anything anymore, just skip.
				// Also don't increment serverFrame so clients know about this.
				tLX->currentTime += TimeDiff(simDelay.milliseconds() - simDelay.milliseconds() % Game::FixedFrameTime);
				continue;
			}

			if(game.state < Game::S_Preparing)
				break;

			if(game.state == Game::S_Playing && !isGamePaused())
				serverFrame++;

			// do lua/gus frames in all cases
			{
				// convert speed to lua if needed
				std::vector< SmartPointer<CGameObject::ScopedGusCompatibleSpeed> > scopedSpeeds;
				scopedSpeeds.reserve( game.worms()->size() );
				for_each_iterator(CWorm*, w, game.worms())
						scopedSpeeds.push_back( new CGameObject::ScopedGusCompatibleSpeed(*w->get()) );

				gusLogicFrame();
			}

			cClient->Frame();
			if(isServer())
				cServer->Frame();

			tLX->currentTime += TimeDiff(Game::FixedFrameTime);
		}
		simulationTime = tLX->currentTime;
		tLX->currentTime = curTime;
		tLX->fDeltaTime = curDeltaTime;
	}

	iterAttrUpdates(NULL);

	if(tLX && state >= Game::S_Preparing)
		cClient->Draw(VideoPostProcessor::videoSurface());

	if(state != Game::S_Inactive) {
		// Gusanos network
		network.update();

		cClient->SendPackets();

		// Connecting process
		if (cClient->bConnectingBehindNat)
			cClient->ConnectingBehindNAT();
		else
			cClient->Connecting();

		if(isServer() && cServer->isServerRunning()) {
			cServer->CheckRegister();
			cServer->SendFiles();
			cServer->SendPackets();
		}
	}

	cClient->resetDebugStr();
	
	EnableSystemMouseCursor(false);
}


void Game::cleanupAfterGameloopEnd() {
	wasPrepared = false;
	CrashHandler::recoverAfterCrash = false;
	
	// can happen if we have aborted a game
	if(isServer() && !gameOver)
		// call gameover because we may do some important cleanup there
		game.gameMode()->GameOver();
	
	reset();
	gusGame.unload();
	
	PhysicsEngine::UnInit();
	
	notes << "GameLoopEnd" << endl;
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

	if(isServer()) {
		if(!cServer->isServerRunning())
			stop();
	}
	else {
		if(cClient->getStatus() == NET_DISCONNECTED)
			stop();
	}
}



void SetQuitEngineFlag(const std::string& reason) {
	if(game.state == Game::S_Inactive)
		errors << "SetQuitEngineFlag '" << reason << "' in menu" << endl;
	else if(game.state == Game::S_Connecting)
		errors << "SetQuitEngineFlag '" << reason << "' in connecting state" << endl;
	else if(game.state == Game::S_Lobby)
		errors << "SetQuitEngineFlag '" << reason << "' in lobby" << endl;
	else
		game.state = Game::S_Lobby;
}


void Game::onPrepareWorm(CWorm* w) {
	objects.insertImmediately(w, Grid::WormColLayer, Grid::WormRenderLayer);
	objects.insertImmediately(w->getNinjaRope(), 1, 1);
}

void Game::onUnprepareWorm(CWorm* w) {
	// We must unlink the object now from the list because this destructor
	// is not called from Gusanos but from CClient.
	// NOTE: Not really the best way but I don't know a better way
	// Game.onNewWorm has inserted the object into the list.
	objects.unlink(w);
	objects.unlink(w->getNinjaRope());
}

void Game::onRemoveWorm(CWorm* w) {
	if(w->isPrepared())
		w->Unprepare(); // also to call onUnprepareWorm and to unlink it
	std::map<int,CWorm*>::iterator i = m_worms.find(w->getID());
	assert(i->second == w);
	m_worms.erase(i);
}

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

void Game::onRemovePlayer(CWormInputHandler* p) {
	foreach(p2, players) {
		if(*p2 == p) {
			players.erase(p2);
			break;
		}
	}
	foreach(p2, localPlayers) {
		if(*p2 == p) {
			localPlayers.erase(p2);
			break;
		}
	}
}

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


void Game::reset() {
	notes << "Game::reset" << endl;
	
	// Delete all players
	std::vector<CWormInputHandler*> playersCopy(players); // copy to avoid problems
	foreach ( p, playersCopy )
		(*p)->deleteThis();
	players.clear();
	localPlayers.clear();
	
	// we must call this first because the references to weapons, ninjarope and what may be deleted
	for_each_iterator(CWorm*, w, worms())
		w->get()->Unprepare();
	
	// Delete all objects
	objects.clear();
}

void Game::resetWorms() {
	for_each_iterator(CWorm*, w, FullCopyIterator(worms()))
		w->get()->deleteThis();
	m_worms.clear();	
}

CMap* Game::gameMap() { return m_gameMap.get(); }

CGameScript* Game::gameScript() { return m_gameMod.get(); }

CGameMode* Game::gameMode() {
	if(tLX) {
		if(game.isServer()) return gameSettings[FT_GameMode].as<GameModeInfo>()->mode;
		return cClient->getGameLobby()[FT_GameMode].as<GameModeInfo>()->mode;
	}
	return NULL;
}

CWpnRest* Game::weaponRestrictions() { return m_wpnRest.get(); }

bool Game::needProxyWormInputHandler() {
	return isServer();
}

bool Game::needToCreateOwnWormInputHandlers() {
	return isServer() || (cClient->getServerVersion() < OLXBetaVersion(0,59,1));
}

bool Game::isTeamPlay() {
	return cClient->isTeamGame();
}


bool CClient::getGamePaused() {
	if(!game.isLocalGame()) return false; // pause only allowed in local game
	return bViewportMgr || bGameMenu || Con_IsVisible();
}

bool Game::isGamePaused() {
	// If we're in a menu & a local game, don't do simulation
	return cClient->getGamePaused();
}

bool Game::shouldDoPhysicsFrame() {
	if(isGamePaused()) return false;
	if(!cClient->canSimulate()) return false;
	if(gameOver) {
		// We stop a few seconds after the actual game over
		if(gameOverTime().seconds() > GAMEOVER_WAIT)
			return false;
	}
	return true;
}


Iterator<CWorm*>::Ref Game::worms() {
	return GetIterator_second(m_worms);
}

Iterator<CWorm*>::Ref Game::localWorms() {
	return GetFilterIterator(worms())(&CWorm::getLocal);
}

Iterator<CWorm*>::Ref Game::aliveWorms() {
	return GetFilterIterator(worms())(&CWorm::getAlive);
}

Iterator<CWorm*>::Ref Game::wormsOfClient(const CServerConnection* cl) {
	return GetFilterIterator(worms())( boost::bind(&CWorm::getClient, _1) == cl );
}

CWorm* Game::wormById(int wormId, bool assertExisting) {
	std::map<int,CWorm*>::iterator i = m_worms.find(wormId);
	if(i == m_worms.end()) {
		if(assertExisting)
			assert(false);
		return NULL;
	}
	return i->second;
}

CWorm* Game::firstLocalHumanWorm() {
	for_each_iterator(CWorm*, w, localWorms())
		if( w->get()->getType() == PRF_HUMAN )
			return w->get();
	return NULL;
}

CWorm* Game::findWormByName(const std::string& name) {
	for_each_iterator(CWorm*, w, worms())
		if(stringcasecmp(w->get()->getName(), name) == 0)
			return w->get();
	return NULL;
}

CWorm* Game::createNewWorm(int wormId, bool local, const SmartPointer<profile_t>& profile, const Version& clientVersion) {
	assert(wormById(wormId, false) == NULL);
	CWorm* w = new CWorm();
	w->setID(wormId);
	w->fLastSimulationTime = GetPhysicsTime(); 
	w->iTotalWins = w->iTotalLosses = w->iTotalKills = w->iTotalDeaths = w->iTotalSuicides = 0;
	w->setClient(NULL); // Local worms won't get CServerConnection owner
	w->setName(profile->sName);
	w->setSkin(profile->cSkin);
	w->setTeam(profile->iTeam);
	w->setType(WormType::fromInt(profile->iType));
	if(local && bDedicated && w->getType() == PRF_HUMAN) {
		warnings << "createNewWorm: local human worm creation on dedicated server -> make it a bot instead" << endl;
		w->setType(PRF_COMPUTER);
	}
	w->setLocal(local);
	w->setClientVersion(clientVersion);
	w->setProfile(profile);
	m_worms[wormId] = w;
	return w;
}

int Game::getNewUniqueWormId() {
	int lastId = -1;
	foreach(w, m_worms) {
		if(w->first > lastId + 1) // there is at least one ID free
			return lastId + 1;
		lastId = w->first;
	}
	return lastId + 1;
}

void Game::removeWorm(CWorm* w) {
	assert(w != NULL);
	w->deleteThis();
	// onRemoveWorm will be called and will remove the worm from the list
}

static std::string _wormName(CWorm* w) { return itoa(w->getID()) + ":" + w->getName(); }

std::string Game::wormName(int wormId) {
	return ifWorm<std::string>(wormId, _wormName, itoa(wormId) + ":<unknown-worm>");
}

bool Game::allowedToSleepForEvent() {
	if(state > Game::S_Inactive)
		// we are in connecting, lobby, game or so -> don't sleep
		// In theory, in connecting/lobby, network events should
		// be enough. But that has been buggy earlier, so don't sleep for now.
		return false;

	// in menu
	if(processedEvent)
		// LX GUI code sucks. It sometimes needs a refresh right after
		// some event was handled and thus, don't sleep for the next frame.
		// We reset processedEvent in the next ProcessEvents() call.
		return false;

	if(menuFrame < 2)
		// Again, LX GUI code sucks. It needs some refresh right at startup.
		return false;

	return true;
}


int oldLXStateInt() {
	switch(game.state) {
	case Game::S_Lobby: return 0;
	case Game::S_Preparing: return 1;
	case Game::S_Playing: return 2;
	default: warnings << "oldLXStateInt: bad game state " << game.state << endl;
	}
	return 0;
}

