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

///////////////////
// Spawn a worm
void GameServer::SpawnWorm(CWorm *Worm, CVec * _pos, CServerConnection * client)
{
	if (bGameOver || Worm->isSpectating())
		return;

	CVec pos;

	if( _pos )
		pos = *_pos;
	else
	{
		pos = FindSpot();
		
		// Spawn worm closer to it's own team and away from other teams
		if( tLXOptions->tGameInfo.bRespawnGroupTeams &&
				  ( getGameMode()->GameTeams() > 1 ) )
		{
			float team_dist = 0;
			CVec pos1;
			
			for( int k=0; k<100; k++ )
			{
				float team_dist1 = 0;
				pos1 = FindSpot();
				CWorm * w = cWorms;
				for(int i = 0; i < MAX_WORMS; i++, w++)
				{
					if( !w->isUsed() || w->getLives() == WRM_OUT || !w->getWeaponsReady() || 
						Worm->getID() == w->getID() || !w->getAlive() )
						continue;
					// sqrt will make sure there's no large dist between team1 and 2 and short dist between 2 and 3
					// The sum will get considerably smaller if any two teams are on short dist
					if( w->getTeam() == Worm->getTeam() )
						team_dist1 -= ( pos1 - w->getPos() ).GetLength() / 10.0f;
					else
						team_dist1 += sqrt( ( pos1 - w->getPos() ).GetLength() );
				}
				if( team_dist1 > team_dist )
				{
					team_dist = team_dist1;
					pos = pos1;
				}
			}
		}
	}

	if(pos.x == -1 && pos.y == -1)
		pos = FindSpot();

	// Allow the game mode to override spawns
	if(!getGameMode()->Spawn(Worm, pos))
		return;

	if( client )	// Spawn all playing worms only for new client for connect-during-game
		client->getNetEngine()->SendSpawnWorm(Worm, Worm->getPos());
	else
	{
		if( DedicatedControl::Get() )
			DedicatedControl::Get()->WormSpawned_Signal(Worm);
		for( int i = 0; i < MAX_CLIENTS; i++ )
			cClients[i].getNetEngine()->SendSpawnWorm(Worm, Worm->getPos());
	}
}

///////////////////
// Find a spot with no rock
CVec GameServer::FindSpot(void)
{
	int	 x, y;
	int	 px, py;
	bool	first = true;
	int	 cols = cMap->getGridCols() - 1;	   // Note: -1 because the grid is slightly larger than the
	int	 rows = cMap->getGridRows() - 1;	   // level size
	int	 gw = cMap->getGridWidth();
	int	 gh = cMap->getGridHeight();

	uchar pf, pf1, pf2, pf3, pf4;
	cMap->lockFlags();
	
	// Find a random cell to start in - retry if failed
	for( int tries = 0; tries < 40; tries++ ) {
		px = (int)(fabs(GetRandomNum()) * (float)cols);
		py = (int)(fabs(GetRandomNum()) * (float)rows);
		x = px; y = py;

		if( x + y < 6 )	// Do not spawn in top left corner
			continue;

		pf = *(cMap->getAbsoluteGridFlags() + y * cMap->getGridCols() + x);
		pf1 = (x>0) ? *(cMap->getAbsoluteGridFlags() + y * cMap->getGridCols() + (x-1)) : PX_ROCK;
		pf2 = (x<cols-1) ? *(cMap->getAbsoluteGridFlags() + y * cMap->getGridCols() + (x+1)) : PX_ROCK;
		pf3 = (y>0) ? *(cMap->getAbsoluteGridFlags() + (y-1) * cMap->getGridCols() + x) : PX_ROCK;
		pf4 = (y<rows-1) ? *(cMap->getAbsoluteGridFlags() + (y+1) * cMap->getGridCols() + x) : PX_ROCK;
		if( !(pf & PX_ROCK) && !(pf1 & PX_ROCK) && !(pf2 & PX_ROCK) && !(pf3 & PX_ROCK) && !(pf4 & PX_ROCK) ) {
			cMap->unlockFlags();
			return CVec((float)x * gw + gw / 2, (float)y * gh + gh / 2);
		}
	}

	// Start from the cell and go through until we get to an empty cell
	while(true) {
		while(true) {
			// If we're on the original starting cell, and it's not the first move we have checked all cells
			// and should leave
			if(!first) {
				if(px == x && py == y) {
					cMap->unlockFlags();
					return CVec((float)x * gw + gw / 2, (float)y * gh + gh / 2);
				}
			}
			first = false;

			pf = *(cMap->getAbsoluteGridFlags() + y * cMap->getGridCols() + x);
			pf1 = (x>0) ? *(cMap->getAbsoluteGridFlags() + y * cMap->getGridCols() + (x-1)) : PX_ROCK;
			pf2 = (x<cols-1) ? *(cMap->getAbsoluteGridFlags() + y * cMap->getGridCols() + (x+1)) : PX_ROCK;
			pf3 = (y>0) ? *(cMap->getAbsoluteGridFlags() + (y-1) * cMap->getGridCols() + x) : PX_ROCK;
			pf4 = (y<rows-1) ? *(cMap->getAbsoluteGridFlags() + (y+1) * cMap->getGridCols() + x) : PX_ROCK;
			if( !(pf & PX_ROCK) && !(pf1 & PX_ROCK) && !(pf2 & PX_ROCK) && !(pf3 & PX_ROCK) && !(pf4 & PX_ROCK) )  {
				cMap->unlockFlags();
				return CVec((float)x * gw + gw / 2, (float)y * gh + gh / 2);
			}

			if(++y >= rows) {
				y = 0;
				break;
			}
		}

		if(++x >= cols) {
			x = 0;
			y = 0;
			break;
		}
	}
	cMap->unlockFlags();

	// Can't get here
	return CVec((float)x, (float)y);
}

void GameServer::killWorm( int victim, int killer, int suicidesCount )
{
	// If the game is already over, ignore this
	if (bGameOver)  {
		printf("GameServer::killWorm: Game is over, ignoring.\n");
		return;
	}
	// Safety check
	if (victim < 0 || victim >= MAX_WORMS)  {
		printf("GameServer::killWorm: victim ID out of bounds.\n");
		return;
	}
	if (killer < 0 || killer >= MAX_WORMS)  {
		printf("GameServer::killWorm: killer ID out of bounds.\n");
		return;
	}

	if(victim != killer )
		suicidesCount = 0;
	if(victim == killer && suicidesCount == 0 )
		suicidesCount = 1;

	CWorm *vict = &cWorms[victim];
	CWorm *kill = &cWorms[killer];

	// Cheat prevention, game behaves weird if this happens
	if (vict->getLives() < 0 && tLXOptions->tGameInfo.iLives >= 0)  {
		vict->setLives(WRM_OUT);  // Safety
		printf("GameServer::ParseDeathPacket: victim is already out of the game.\n");
		return;
	}

	// Adjust the score if there were multiple suicides
	if (suicidesCount > 1)  {
		if (tLXOptions->tGameInfo.iLives != WRM_UNLIM) // Substracting from infinite makes no sense
			vict->setLives(MAX(WRM_OUT, vict->getLives() - suicidesCount + 1)); // HINT: +1 because one life is substracted in vict->Kill()
	}

	getGameMode()->Kill(vict, kill);
	for(int i = 0; i < MAX_CLIENTS; i++) {
		cClients[i].getNetEngine()->SendWormScore(vict);
		if (killer != victim)
			cClients[i].getNetEngine()->SendWormScore(kill);
	}
	// Let everyone know that the worm is now dead
	for(int i = 0; i < MAX_CLIENTS; i++)
		cClients[i].getNetEngine()->SendWormDied(vict);

	if(DedicatedControl::Get())
		DedicatedControl::Get()->WormDied_Signal(vict,kill);
	RecheckGame();
	return;
};


///////////////////
// Simulate the game stuff
void GameServer::SimulateGame(void)
{
	if(iState != SVS_PLAYING)
		return;

	// If this is a remote game, and game over,
	// and we've seen the scoreboard for a certain amount of time, go back to the lobby
	if(bGameOver
	&& (tLX->fCurTime - fGameOverTime > LX_ENDWAIT || (bDedicated && iNumPlayers <= 1)) // dedicated server should go to lobby immediatly if alone
	&& iState != SVS_LOBBY
	&& tLX->iGameType == GME_HOST) {
		gotoLobby();
		return;
	}

	// Don't process if the game is paused (local play only)
	if (cClient->getGamePaused())
		return;

	// Process worms
	CWorm *w = cWorms;
	short i ;

	for(i=0;i<MAX_WORMS;i++,w++) {
		if(!w->isUsed())
			continue;

		if(!w->getAlive() && w->getLives() != WRM_OUT && w->getWeaponsReady()) {
			// Check to see if they have been dead for longer then 2.5 seconds
			if(tLX->fCurTime - w->getTimeofDeath() > tLXOptions->tGameInfo.fRespawnTime )
			{
				SpawnWorm(w);
				if( tLXOptions->tGameInfo.bEmptyWeaponsOnRespawn )
					SendEmptyWeaponsOnRespawn(w);
			}
		}

		// Simulate the worm's weapons
		// TODO: why are we doing this? we are not simulating the worm but why the weapon?
		// please try to remove this here and then remove also the dt parameter in PhysicsEngine
		if( w->getAlive() )
			PhysicsEngine::Get()->simulateWormWeapon(tLX->fRealDeltaTime * (float)tLXOptions->tGameInfo.features[FT_GameSpeed], w);
	}

	// Check if any bonuses have been in for too long and need to be destroyed
	if (tLXOptions->tGameInfo.bBonusesOn)  {
		for(i=0; i<MAX_BONUSES; i++) {
			if(!cBonuses[i].getUsed())
				continue;

			// If it's been here too long, destroy it
			if( tLX->fCurTime - cBonuses[i].getSpawnTime() > tLXOptions->tGameInfo.fBonusLife ) {
				CBytestream bs;
				bs.Clear();
				cBonuses[i].setUsed(false);

				// TODO: move this out here
				bs.writeByte(S2C_DESTROYBONUS);
				bs.writeByte((byte)i);
				SendGlobalPacket(&bs);
			}
		}
	}



	// Check if we need to spawn a bonus
	if(tLX->fCurTime - fLastBonusTime > tLXOptions->tGameInfo.fBonusFreq && tLXOptions->tGameInfo.bBonusesOn && !bGameOver) {

		SpawnBonus();

		fLastBonusTime = tLX->fCurTime;
	}

	// Simulate anything needed by the game mode
	getGameMode()->Simulate();

	if( tLXOptions->tGameInfo.fTimeLimit > 0 && fServertime > tLXOptions->tGameInfo.fTimeLimit*60.0 )
		RecheckGame();
}


///////////////////
// Spawn a bonus
void GameServer::SpawnBonus(void)
{
	// Find an empty spot for the bonus
	CVec pos = FindSpot();

	// Carve a hole for it
	//cMap->CarveHole(SPAWN_HOLESIZE,pos);

	// NOTE: Increase to 2 when we want to use the fullcharge bonus
	int type = (GetRandomInt(999) >= tLXOptions->tGameInfo.fBonusHealthToWeaponChance * 1000.0f) ? BNS_HEALTH : BNS_WEAPON;
	int wpn = GetRandomInt(cGameScript.get()->GetNumWeapons()-1);


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


	// Check if the weapon chosen is enabled or a 'bonus' weapon in the restrictions
	int orig = wpn;
	while(1) {
		int state = cWeaponRestrictions.getWeaponState( (cGameScript.get()->GetWeapons()+wpn)->Name );

		if( state != wpr_banned )
			break;

		wpn++;
		if( wpn >= cGameScript.get()->GetNumWeapons())
			wpn=0;

		// No good weapons? Just leave with original choice
		if( wpn == orig )
			break;
	}


	b->Spawn(pos, type, wpn, cGameScript.get());


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


///////////////////
// Worm is shooting
void GameServer::WormShoot(CWorm *w, GameServer* gameserver)
{
	// Don't shoot when the game is over
	if (cClient->isGameOver())
		return;

	if(!gameserver->getGameMode()->Shoot(w))
		return;

	wpnslot_t *Slot = w->getCurWeapon();

	if(Slot->Reloading)
		return;

	if(Slot->LastFire>0)
		return;

	// Don't shoot with banned weapons
	if (!Slot->Enabled)
		return;

	if(!Slot->Weapon) {
		printf("WARNING: trying to shoot with an unitialized weapon!\n");
		return;
	}

	// TODO: what is the effect of this?
	Slot->LastFire = Slot->Weapon->ROF;


	// Must be a projectile
	if(Slot->Weapon->Type != WPN_PROJECTILE && Slot->Weapon->Type != WPN_BEAM)
		return;

	// Get the direction angle
	float Angle = w->getAngle();
	if(w->getDirection() == DIR_LEFT)
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
		CVec vel = *w->getVelocity();
		speed = NormalizeVector( &vel );
	}
	
	if(gameserver) {
		float time = gameserver->getServerTime();
		if( w->hasOwnServerTime() )
			time = w->serverTime();
		
		// Add the shot to ALL the connected clients shootlist
		CServerConnection *cl = gameserver->getClients();
		for(short i=0; i<MAX_CLIENTS; i++,cl++) {
			if(cl->getStatus() == NET_DISCONNECTED)
				continue;

			cl->getShootList()->addShoot(time, speed, (int)Angle, w);
		}
	}
	

	//
	// Note: Drain does NOT have to use a delta time, because shoot timing is controlled by the ROF
	// (ROF = Rate of Fire)
	//

	// Drain the Weapon charge
	Slot->Charge -= Slot->Weapon->Drain / 100;
	if(Slot->Charge <= 0) {
		Slot->Charge = 0;
		Slot->Reloading = true;
	}
}




///////////////////
// Go back to the lobby
void GameServer::gotoLobby(void)
{
	printf("gotoLobby\n");

	// in lobby we need the events again
	AddSocketToNotifierGroup( tSocket );
	for( int f=0; f<MAX_CLIENTS; f++ )
		if(IsSocketStateValid(tNatTraverseSockets[f]))
			AddSocketToNotifierGroup(tNatTraverseSockets[f]);

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
			cWorms[i].getLobby()->bReady = false;
			cWorms[i].setGameReady(false);
			cWorms[i].setTagIT(false);
			cWorms[i].setTagTime(0);
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
		if(cWorms[i].getFlag()) {
			cWorms[i].setUsed(false);
			cWorms[i].setFlag(false);
			CBytestream bs;
			bs.writeByte(S2C_WORMSOUT);
			bs.writeByte(1);
			bs.writeByte((uchar)i);
			SendGlobalPacket(&bs);
			iNumPlayers--;
		}
	}
	UpdateGameLobby();

	for(i=0; i<MAX_CLIENTS; i++) {
		cClients[i].setGameReady(false);
	}

	fLastUpdateSent = -9999;
	fGameOverTime = -9999;

	SendWormLobbyUpdate();
	
	if(bUpdateWorms)
		UpdateWorms();

	if( DedicatedControl::Get() )
		DedicatedControl::Get()->BackToServerLobby_Signal();

	// Goto the host lobby
	DeprecatedGUI::Menu_Net_GotoHostLobby();

	for( i=0; i<MAX_CLIENTS; i++ )
		cClients[i].getUdpFileDownloader()->allowFileRequest(true);

	// Re-register the server to reflect the state change
	if( tLXOptions->bRegServer && tLX->iGameType == GME_HOST )
		RegisterServerUdp();

	// HINT: the gamescript is shut down by the cache
}




///////////////////
// Recheck the game status
// Called iff game state has changed (e.g. player left etc.)
void GameServer::RecheckGame(void)
{
	if(bGameOver || iState != SVS_PLAYING)
		return;
	if(getGameMode()->CheckGameOver())
		GameOver();
}


///////////////////
// Checks if all the clients are ready to play
void GameServer::CheckReadyClient(void)
{
	short c;

	if(iState != SVS_GAME)
		return;

	bool allready = true;
	CServerConnection *client = cClients;
	for(c=0; c<MAX_CLIENTS; c++, client++) {
		if(client->getStatus() == NET_DISCONNECTED || client->getStatus() == NET_ZOMBIE || client->getNumWorms() == 0)
			continue;

		if(!client->getGameReady())
			allready = false;
	}

	// All ready to go?
	if(allready)
		BeginMatch();
}
