/////////////////////////////////////////
//
//			 OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Server class - Game routines
// Created 11/7/02
// Jason Boettcher

#include "LieroX.h"
#include "CServer.h"
#include "Debug.h"
#include "CServerConnection.h"
#include "CServerNetEngine.h"
#include "StringUtils.h"
#include "Protocol.h"
#include "CWorm.h"
#include "MathLib.h"
#include "DedicatedControl.h"
#include "Physics.h"
#include "DeprecatedGUI/Menu.h"
#include "CGameMode.h"
#include "FlagInfo.h"
#include "WeaponDesc.h"
#include "game/Game.h"
#include "CGameScript.h"



///////////////////
// Spawn a worm
void GameServer::SpawnWorm(CWorm *Worm, CVec * _pos, CServerConnection * client)
{
	if (bGameOver || Worm->isSpectating())
		return;

	if(game.gameScript()->gusEngineUsed())
		// NOTE: only for now
		return;
	
	CVec pos;

	if( _pos )
		pos = *_pos;
	else
	{
		if( cMap->getPredefinedSpawnLocation(Worm, &pos) ) {
			// ok
		}
		else if( game.gameMode() == GameMode(GM_CTF) ) {
			Flag* flag = m_flagInfo->getFlag(Worm->getTeam());
			if(!flag) // no flag yet, choose like in respawngroupteams
				pos = cMap->FindSpotCloseToTeam(Worm->getTeam(), Worm, true);
			else
				pos = cMap->FindSpotCloseToPos(flag->spawnPoint.pos);
		}
		// Spawn worm closer to it's own team and away from other teams
		else if( gameSettings[FT_RespawnGroupTeams] && ( game.gameMode()->GameTeams() > 1 ) )
		{
			pos = cMap->FindSpotCloseToTeam(Worm->getTeam(), Worm);
		}
		else
			pos = cMap->FindSpot();			
	}

	if(pos.x == -1 && pos.y == -1)
		pos = cMap->FindSpot();

	// Allow the game mode to override spawns
	if(!game.gameMode()->Spawn(Worm, pos))
		return;

	Worm->Spawn(pos);
	bool sendWormUpdate = true;
	if(Worm->getLives() == WRM_OUT) {
		if((int)gameSettings[FT_Lives] < 0)
			Worm->setLives(WRM_UNLIM);
		else
			Worm->setLives(0);
		sendWormUpdate = true;
	}
	
	if( client ) { // Spawn all playing worms only for new client for connect-during-game
		client->getNetEngine()->SendSpawnWorm(Worm, Worm->getPos());
		if(sendWormUpdate) client->getNetEngine()->SendWormScore(Worm);		
	}
	else {
		if( DedicatedControl::Get() )
			DedicatedControl::Get()->WormSpawned_Signal(Worm);
		for( int i = 0; i < MAX_CLIENTS; i++ ) {
			cClients[i].getNetEngine()->SendSpawnWorm(Worm, Worm->getPos());
			if(sendWormUpdate) cClients[i].getNetEngine()->SendWormScore(Worm);
		}

		if( gameSettings[FT_EmptyWeaponsOnRespawn] )
			cServer->SendEmptyWeaponsOnRespawn( Worm );
	}
}



void GameServer::killWorm( int victim, int killer, int suicidesCount )
{
	// If the game is already over, ignore this
	if (bGameOver)  {
		warnings << "GameServer::killWorm: Game is over, ignoring." << endl;
		return;
	}
	// Safety check
	if (victim < 0 || victim >= MAX_WORMS)  {
		errors << "GameServer::killWorm: victim ID out of bounds." << endl;
		return;
	}

	if(victim != killer )
		suicidesCount = 0;
	if(victim == killer && suicidesCount == 0 )
		suicidesCount = 1;

	CWorm *vict = &cWorms[victim];
	CWorm *kill = NULL;
	if(killer >= 0 && killer < MAX_WORMS && cWorms[killer].isUsed())
		kill = &cWorms[killer];
	
	// Cheat prevention, game behaves weird if this happens
	if (vict->getLives() < 0 && (int)gameSettings[FT_Lives] >= 0)  {
		vict->setLives(WRM_OUT);  // Safety
		warnings("GameServer::killWorm: victim is already out of the game.\n");
		return;
	}

	// Adjust the score if there were multiple suicides
	if (suicidesCount > 1)  {
		if ((int)gameSettings[FT_Lives] != WRM_UNLIM) // Substracting from infinite makes no sense
			vict->setLives(MAX<int>(0, vict->getLives() - suicidesCount + 1)); // HINT: +1 because one life is substracted in vict->Kill()
	}

	game.gameMode()->Kill(vict, kill);
	for(int i = 0; i < MAX_CLIENTS; i++) {
		if(!cClients[i].isConnected()) continue;		
		cClients[i].getNetEngine()->SendWormScore(vict);
		if (kill && killer != victim)
			cClients[i].getNetEngine()->SendWormScore(kill);
	}
	// Let everyone know that the worm is now dead
	for(int i = 0; i < MAX_CLIENTS; i++) {
		if(!cClients[i].isConnected()) continue;		
		cClients[i].getNetEngine()->SendWormDied(vict);
	}
	
	if(DedicatedControl::Get())
		DedicatedControl::Get()->WormDied_Signal(vict,kill);
	
	SendTeamScoreUpdate();
	
	RecheckGame();
	return;
};


///////////////////
// Simulate the game stuff
void GameServer::SimulateGame()
{
	if(iState != SVS_PLAYING)
		return;

	if( gameSettings[FT_NewNetEngine] )
		return;

	// If this is a remote game, and game over,
	// and we've seen the scoreboard for a certain amount of time, go back to the lobby
	if(bGameOver
	&& (tLX->currentTime - fGameOverTime > LX_ENDWAIT || (bDedicated && iNumPlayers <= 1)) // dedicated server should go to lobby immediatly if alone
	&& iState != SVS_LOBBY
	&& tLX->iGameType == GME_HOST) {
		gotoLobby(true, "timeout for gameover scoreboard");
		return;
	}

	// Don't process if the game is paused (local play only)
	if (cClient->getGamePaused())
		return;

	// Process worms
	CWorm *w = cWorms;
	for(short i=0;i<MAX_WORMS;i++,w++) {
		if(!w->isUsed())
			continue;

		// HINT: this can happen when a new client connects during game and has not selected weapons yet
		// We just skip him
		if (w->getClient())
			// TODO: why do we check getGameReady() and not getWeaponsReady() here? what's the difference?
			if (!w->getClient()->getGameReady())
				continue;

		if(!w->getAlive() && w->getLives() != WRM_OUT && w->getWeaponsReady()) {

			// Check to see if they have been dead for longer than fRespawnTime (originally 2.5 seconds)
			if(tLX->currentTime > w->getTimeofDeath() + TimeDiff((float)gameSettings[FT_RespawnTime]) )
			{
				SpawnWorm(w);
			}
		}

		// Simulate the worm's weapons
		// TODO: why are we doing this? we are not simulating the worm but why the weapon?
		// please try to remove this here and then remove also the dt parameter in PhysicsEngine
		if( w->getAlive() )
			PhysicsEngine::Get()->simulateWormWeapon(w);
	}

	// Check if any bonuses have been in for too long and need to be destroyed
	if (gameSettings[FT_Bonuses])  {
		for(short i=0; i<MAX_BONUSES; i++) {
			if(!cBonuses[i].getUsed())
				continue;

			// If it's been here too long, destroy it
			if( tLX->currentTime - cBonuses[i].getSpawnTime() > gameSettings[FT_BonusLife] ) {
				cBonuses[i].setUsed(false);

				// TODO: move this out here
				CBytestream bs;
				bs.writeByte(S2C_DESTROYBONUS);
				bs.writeByte((byte)i);
				SendGlobalPacket(&bs);
			}
		}
	}

	// Check if we need to spawn a bonus
	if(tLX->currentTime - fLastBonusTime > gameSettings[FT_BonusFreq] && gameSettings[FT_Bonuses] && !bGameOver) {
		SpawnBonus();
		fLastBonusTime = tLX->currentTime;
	}

	// check for flag
	w = cWorms;
	for(short i=0;i<MAX_WORMS;i++,w++) {
		if(!w->isUsed())
			continue;
		flagInfo()->checkWorm(w);
	}
	
	// Simulate anything needed by the game mode
	game.gameMode()->Simulate();

	if( game.gameMode()->TimeLimit() > 0 && fServertime > game.gameMode()->TimeLimit() )
		RecheckGame();
}


///////////////////
// Spawn a bonus
void GameServer::SpawnBonus()
{
	// Find an empty spot for the bonus
	CVec pos = game.gameMap()->FindSpot();

	// Carve a hole for it
	//cMap->CarveHole(SPAWN_HOLESIZE,pos);

	// NOTE: Increase to 2 when we want to use the fullcharge bonus
	int type = (GetRandomInt(999) >= (float)gameSettings[FT_BonusHealthToWeaponChance] * 1000.0f) ? BNS_HEALTH : BNS_WEAPON;

	// Find a free bonus spot
	CBonus *b = cBonuses;
	int spot = -1;
	for(short i=0;i<MAX_BONUSES;i++,b++) {
		if(b->getUsed())
			continue;

		spot = i;
		break;
	}

	// No spots
	if(spot == -1)
		return;

	std::vector<int> goodWpns;
	for(int i = 0; i < game.gameScript()->GetNumWeapons(); ++i)
		if(game.weaponRestrictions()->getWeaponState( (game.gameScript()->GetWeapons()+i)->Name ) != wpr_banned)
			goodWpns.push_back(i);
	
	int wpn = (goodWpns.size() > 0) ? randomChoiceFrom(goodWpns) : -1;
	if(type == BNS_WEAPON && wpn < 0) return; // no weapon found
	
	b->Spawn(pos, type, wpn, game.gameScript());


	// Send the spawn the everyone
	CBytestream bs;

	bs.writeByte(S2C_SPAWNBONUS);
	bs.writeInt(type,1);
	if(type == BNS_WEAPON)
		bs.writeInt(wpn,1);
	bs.writeInt(spot,1);

	bs.writeInt((int)pos.x, 2);
	bs.writeInt((int)pos.y, 2);

	SendGlobalPacket(&bs);
}


void GameServer::WormShootEnd(CWorm* w, const weapon_t* wpn) {
	// Don't shoot when the game is over
	if (cClient->isGameOver())
		return;
	
	if(!game.gameMode()->Shoot(w))
		return;
	
	if(!wpn) {
		warnings << "WormShootEnd: trying to shoot with an unitialized weapon!" << endl;
		return;
	}
	
	if(!wpn->FinalProj.isSet())
		return;

	// Get the direction angle
	float Angle = w->getAngle();
	if(w->getFaceDirectionSide() == DIR_LEFT)
		Angle=180-Angle;
	
	if(Angle < 0)
		Angle+=360;
	if(Angle > 360)
		Angle-=360;
	if(Angle == 360)
		Angle=0;
	
	float speed = 0.0f;
	
	CVec vel = w->getVelocity();
	speed = NormalizeVector( &vel );
	
	TimeDiff time = getServerTime();
	if( w->hasOwnServerTime() )
		time = w->serverTime();
	
	// Add the shot to ALL the connected clients shootlist
	CServerConnection *cl = getClients();
	for(short i=0; i<MAX_CLIENTS; i++,cl++) {
		if(cl->getStatus() == NET_DISCONNECTED)
			continue;
		
		cl->getShootList()->addShoot(wpn->ID, time, speed, (int)Angle, w, true);
	}
}

///////////////////
// Worm is shooting
void GameServer::WormShoot(CWorm *w)
{
	// Don't shoot when the game is over
	if (cClient->isGameOver())
		return;

	if(!game.gameMode()->Shoot(w))
		return;

	wpnslot_t *Slot = w->getCurWeapon();

	if(Slot->Reloading)
		return;

	if(Slot->LastFire>0)
		return;

	// Don't shoot with disabld weapons
	if (!Slot->Enabled)
		return;

	if(!Slot->Weapon) {
		warnings << "WormShoot: trying to shoot with an unitialized weapon!" << endl;
		return;
	}

	// Safe the ROF time (Rate of Fire). That's the pause time after each shot.
	// In simulateWormWeapon, we decrease this by deltatime and
	// the next shot is allowed if lastfire<=0.
	Slot->LastFire = Slot->Weapon->ROF;


	// Must be a projectile or beam.
	// We send also beam via the shootlist. See CClient::ProcessShot which is calling CClient::ProcessShot_Beam in that case.
	if(Slot->Weapon->Type != WPN_PROJECTILE && Slot->Weapon->Type != WPN_BEAM) {
		// It's not a projectile, so the client is handling this, but we take track of charge to disable weapon if needed.
		
		// Drain the Weapon charge
		Slot->Charge -= Slot->Weapon->Drain / 100;
		if(Slot->Charge <= 0) {
			Slot->Charge = 0;
			
			if(gameSettings[FT_DisableWpnsWhenEmpty]) {
				Slot->Enabled = false;
				Slot->Weapon = NULL;
				// TODO: move that out here
				CBytestream bs;
				bs.writeByte(S2C_WORMWEAPONINFO);
				w->writeWeapons(&bs);
				cServer->SendGlobalPacket(&bs);			
			} else
				Slot->Reloading = true;
		}
		return;
	}
	
	// Get the direction angle
	float Angle = w->getAngle();
	if(w->getFaceDirectionSide() == DIR_LEFT)
		Angle=180-Angle;

	if(Angle < 0)
		Angle+=360;
	if(Angle > 360)
		Angle-=360;
	if(Angle == 360)
		Angle=0;

	float speed = 0.0f;

	// only projectile wpns have speed; Beam weapons have no speed
	if(Slot->Weapon->Type == WPN_PROJECTILE) {
		// Add the shot to the shooting list
		CVec vel = w->getVelocity();
		speed = NormalizeVector( &vel );
	}
	
	TimeDiff time = getServerTime();
	if( w->hasOwnServerTime() )
		time = w->serverTime();
	
	// Add the shot to ALL the connected clients shootlist
	CServerConnection *cl = getClients();
	for(short i=0; i<MAX_CLIENTS; i++,cl++) {
		if(cl->getStatus() == NET_DISCONNECTED)
			continue;

		cl->getShootList()->addShoot(Slot->Weapon->ID, time, speed, (int)Angle, w, false);
	}

	

	//
	// Note: Drain does NOT have to use a delta time, because shoot timing is controlled by the ROF
	// (ROF = Rate of Fire)
	//

	// Drain the Weapon charge
	Slot->Charge -= Slot->Weapon->Drain / 100;
	if(Slot->Charge <= 0) {
		Slot->Charge = 0;
		
		if(gameSettings[FT_DisableWpnsWhenEmpty]) {
			Slot->Enabled = false;
			Slot->Weapon = NULL;
			// TODO: move that out here
			CBytestream bs;
			bs.writeByte(S2C_WORMWEAPONINFO);
			w->writeWeapons(&bs);
			cServer->SendGlobalPacket(&bs);			
		} else
			Slot->Reloading = true;
	}
}




///////////////////
// Go back to the lobby
void GameServer::gotoLobby(bool alsoWithMenu, const std::string& reason)
{
	notes << "GameServer: gotoLobby (" << reason << ")" << endl;

	if(getState() == SVS_LOBBY) {
		notes << "already in lobby, doing nothing" << endl;
		return;
	}
	
	short i;

	// Tell all the clients
	CBytestream bs;
	bs.writeByte(S2C_GOTOLOBBY);
	SendGlobalPacket(&bs);

	// Clear the info
	iState = SVS_LOBBY;
	bool bUpdateWorms = false;
	for(i=0;i<MAX_WORMS;i++) {
		if(cWorms[i].isUsed()) {
			cWorms[i].Unprepare();
			cWorms[i].setLobbyReady( false );
			cWorms[i].setGameReady(false);
			cWorms[i].setTagIT(false);
			cWorms[i].setTagTime(TimeDiff(0));
			if( cWorms[i].getAFK() == AFK_TYPING_CHAT )
			{
				cWorms[i].setAFK(AFK_BACK_ONLINE, "");
				CBytestream bs;
				bs.writeByte( S2C_AFK );
				bs.writeByte( (uchar)i );
				bs.writeByte( AFK_BACK_ONLINE );
				bs.writeString( "" );
	
				CServerConnection *cl;
				int i;
				for( i=0, cl=cClients; i < MAX_CLIENTS; i++, cl++ )
					if( cl->getStatus() == NET_CONNECTED && cl->getClientVersion() >= OLXBetaVersion(7) )
						cl->getNetEngine()->SendPacket(&bs);
			}
		}
	}
	UpdateGameLobby();

	for(i=0; i<MAX_CLIENTS; i++) {
		cClients[i].setGameReady(false);
	}

	fLastUpdateSent = AbsTime();
	fGameOverTime = AbsTime();

	SendWormLobbyUpdate();
	
	if(bUpdateWorms)
		UpdateWorms();

	if( DedicatedControl::Get() )
		DedicatedControl::Get()->BackToServerLobby_Signal();

	// Goto the host lobby
	if(alsoWithMenu)
		DeprecatedGUI::Menu_Net_GotoHostLobby();

	for( i=0; i<MAX_CLIENTS; i++ )
		cClients[i].getUdpFileDownloader()->allowFileRequest(true);

	// Re-register the server to reflect the state change
	if( tLXOptions->bRegServer && tLX->iGameType == GME_HOST )
		RegisterServerUdp();

	CheckForFillWithBots();
	
	// HINT: the gamescript is shut down by the cache
}




///////////////////
// Recheck the game status
// Called iff game state has changed (e.g. player left etc.)
void GameServer::RecheckGame()
{
	if(iState == SVS_GAME)
		// Check if all the clients are ready
		CheckReadyClient();
	
	if(!bGameOver && iState == SVS_PLAYING)
		if(game.gameMode()->CheckGameOver())
			GameOver();
}


///////////////////
// Checks if all the clients are ready to play
void GameServer::CheckReadyClient()
{
	if(iState != SVS_GAME)
		return;

	bool allready = true;
	CServerConnection *client = cClients;
	for(short c=0; c<MAX_CLIENTS; c++, client++) {
		if(client->getStatus() == NET_DISCONNECTED || client->getStatus() == NET_ZOMBIE || client->getNumWorms() == 0)
			continue;

		if(!client->getGameReady()) {
			notes << "CheckReadyClient: " << client->debugName() << " is not ready yet" << endl;
			allready = false;
			break;
		}
	}

	// All ready to go?
	if(allready)
		BeginMatch();
}
