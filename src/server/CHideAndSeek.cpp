#include <iostream>
#include <math.h>
#include "CHideAndSeek.h"
#include "CWorm.h"
#include "Options.h"
#include "Consts.h"
#include "CServer.h"
#include "CClient.h"
#include "CServerConnection.h"
#include "CServerNetEngine.h"

CHideAndSeek::CHideAndSeek(GameServer* server, CWorm* worms)
{
	cServer = server;
	cWorms = worms;
}

CHideAndSeek::~CHideAndSeek()
{
}

void CHideAndSeek::PrepareGame()
{
}

void CHideAndSeek::PrepareWorm(CWorm* worm)
{
	worm->setLives(0);
}

bool CHideAndSeek::Spawn(CWorm* worm, CVec pos)
{
	worm->Spawn(pos);
	// Hiders only spawn on their own client
	if(worm->getTeam() == HIDER) {
		worm->getClient()->getNetEngine()->SendSpawnWorm(worm, pos);
		return false;
	}
	// Seekers spawn for everyone
	return true;
}

void CHideAndSeek::Kill(CWorm* victim, CWorm* killer)
{
	cServer->SendGlobalText(killer->getName() + " caught " + victim->getName(), TXT_NORMAL);
	killer->AddKill();
	victim->Kill();
}

bool CHideAndSeek::Shoot(CWorm* worm)
{
	return false;
}

void CHideAndSeek::Drop(CWorm* worm)
{
}

void CHideAndSeek::Simulate()
{
	// Check if any of the seekers are in range of the hiders
	int i, j;
	CVec dist;
	for(i = 0; i < MAX_WORMS; i++) {
		if(!cWorms[i].isUsed() || cWorms[i].getLives() == WRM_OUT || cWorms[i].getTeam() == HIDER)
			continue;
		for(j = 0; j < MAX_WORMS; j++) {
			if(!cWorms[j].isUsed() || cWorms[j].getLives() == WRM_OUT || cWorms[j].getTeam() == SEEKER)
				continue;
			dist = cWorms[i].getPos() - cWorms[j].getPos();
			// Catch worms within 100 pixels of the seeker
			if(dist.GetLength() < 100)
				cServer->killWorm(j, i, 0);
		}
	}
}

bool CHideAndSeek::CheckGame()
{
	// Empty games, no need to check anything?
	if(tLXOptions->tGameInfo.features[FT_AllowEmptyGames])
		return false;

	int worms = 0;
	int wormid = 0;
	for(int i = 0; i < MAX_WORMS; i++)
		if(cWorms[i].isUsed() && cWorms[i].getLives() != WRM_OUT && cWorms[i].getTeam() == HIDER) {
			worms++;
			wormid = i;
		}
	if(worms == 0) {
		if (networkTexts->sPlayerHasWon != "<none>")
			cServer->SendGlobalText((replacemax(networkTexts->sPlayerHasWon, "<player>",
				cWorms[wormid].getName(), 1)), TXT_NORMAL);
		iWinner = wormid;
		return true;
	}
	return false;
}

int CHideAndSeek::GameType()
{
	return GMT_TEAMS;
}

int CHideAndSeek::GameTeams()
{
	return 2;
}

int CHideAndSeek::Winner()
{
	return iWinner;
}

