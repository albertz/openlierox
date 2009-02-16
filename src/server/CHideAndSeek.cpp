/*
 OpenLieroX
 
 hide and seek gamemode
 
 created 2009-02-09
 code under LGPL
 */

#include <iostream>
#include "CHideAndSeek.h"
#include "CWorm.h"
#include "Options.h"
#include "Consts.h"
#include "CServer.h"
#include "CClient.h"
#include "CServerConnection.h"
#include "CServerNetEngine.h"
#include "CMap.h"
#include "LieroX.h"

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
	fGameStart = tLX->fCurTime;
	GenerateTimes();
	if(tLXOptions->tGameInfo.fTimeLimit > 0)
		fGameLength = tLXOptions->tGameInfo.fTimeLimit * 60;
	for(int i = 0; i < MAX_WORMS; i++) {
		fLastAlert[i] = 0;
		bVisible[i] = true; // So we can hide
		Hide(&cWorms[i], false);
		cWorms[i].setLives(0);
	}
}

void CHideAndSeek::PrepareWorm(CWorm* worm)
{
	std::string teamhint[2];
	if (networkTexts->sHiderMessage != "<none>")
		replace(networkTexts->sHiderMessage, "<time>", itoa((int)fGameLength), teamhint[0]);
	if (networkTexts->sSeekerMessage != "<none>")
		replace(networkTexts->sSeekerMessage, "<time>", itoa((int)fGameLength), teamhint[1]);

	// Gameplay hints
	worm->getClient()->getNetEngine()->SendText(teamhint[worm->getTeam()], TXT_NORMAL);
}

bool CHideAndSeek::Spawn(CWorm* worm, CVec pos)
{
	pos = cServer->FindSpot();
	worm->Spawn(pos);
	// Worms only spawn visible to their own team
	for(int i = 0; i < MAX_WORMS; i++)  {
		if(cWorms[i].isUsed() && cWorms[i].getTeam() == worm->getTeam())
			cWorms[i].getClient()->getNetEngine()->SendSpawnWorm(worm, pos);
		else
			cWorms[i].getClient()->getNetEngine()->SendHideWorm(worm);
	}
	return false;
}

void CHideAndSeek::Kill(CWorm* victim, CWorm* killer)
{
	if(killer->getTeam() == SEEKER && killer != victim) {
		if (networkTexts->sCaughtMessage != "<none>")  {
			std::string msg;
			replace(networkTexts->sCaughtMessage, "<seeker>", killer->getName(), msg);
			replace(msg, "<hider>", victim->getName(), msg);
			cServer->SendGlobalText(msg, TXT_NORMAL);
		}
		killer->AddKill();
	}
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
	float GameTime = tLX->fCurTime - fGameStart;
	// Game time up
	if(GameTime > fGameLength) {
		for(int i = 0; i < MAX_WORMS; i++)
			if(cWorms[i].isUsed() && cWorms[i].getLives() != WRM_OUT && cWorms[i].getTeam() == SEEKER)
				cServer->killWorm(i, i, cWorms[i].getLives() + 1);
		return;
	}
	// Hiders have some time free from being caught and seen
	if(GameTime < (float)tLXOptions->tGameInfo.features[FT_HS_HideTime])
		return;
	// Check if any of the worms can see eachother
	int i, j;
	for(i = 0; i < MAX_WORMS; i++) {
		if(!cWorms[i].isUsed() || cWorms[i].getLives() == WRM_OUT)
			continue;
		// Hide the worm if the alert time is up
		if(fLastAlert[i] + (float)tLXOptions->tGameInfo.features[FT_HS_AlertTime] < GameTime)
			Hide(&cWorms[i]);
		for(j = 0; j < MAX_WORMS; j++) {
			if(!cWorms[j].isUsed() || cWorms[j].getLives() == WRM_OUT)
				continue;
			if(cWorms[j].getTeam() == cWorms[i].getTeam())
				continue;
			if(CanSee(&cWorms[i], &cWorms[j]))
				Show(&cWorms[j]);
			// Catch the hiders if they are within 10 pixels
			if(cWorms[i].getTeam() == SEEKER && cWorms[j].getTeam() == HIDER)
				if((cWorms[i].getPos() - cWorms[j].getPos()).GetLength() < 10)
					cServer->killWorm(cWorms[j].getID(), cWorms[i].getID(), 0);
		}
	}
}

bool CHideAndSeek::CheckGame()
{
	// Empty games, no need to check anything?
	if(tLXOptions->tGameInfo.features[FT_AllowEmptyGames])
		return false;
	
	// In game?
	if (!cServer || cServer->getState() == SVS_LOBBY)
		return false;

	int worms[2] = { 0, 0 };
	int winners = -1;
	static const std::string teamname[2] = { "hiding", "seeking" };

	for(int i = 0; i < MAX_WORMS; i++)
		if(cWorms[i].isUsed() && cWorms[i].getLives() != WRM_OUT)
			worms[cWorms[i].getTeam()]++;
	if(worms[0] == 0)
		winners = SEEKER;
	else if(worms[1] == 0)
		winners = HIDER;
	if(winners != -1) {
		if(networkTexts->sTeamHasWon != "<none>")
			cServer->SendGlobalText((replacemax(networkTexts->sTeamHasWon, "<team>",
				teamname[winners], 1)), TXT_NORMAL);
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
	return 0;
}

bool CHideAndSeek::NeedUpdate(CServerConnection* cl, CWorm* worm)
{
	// Clients don't recieve dirt updates without getting a full update?
	/*// No worms, but we don't want the client to see nothing
	if(cl->getNumWorms() == 0)
		return true;
	// Different teams, and invisible so no need I think
	if(cl->getWorm(0)->getTeam() != worm->getTeam() && !bVisible[worm->getID()])
		return false;*/
	return true;
}

void CHideAndSeek::Show(CWorm* worm)
{
	fLastAlert[worm->getID()] = tLX->fCurTime - fGameStart;
	if(bVisible[worm->getID()])
		return;
	bVisible[worm->getID()] = true;

	if(worm->getTeam() == HIDER)  {
		if (networkTexts->sHiderVisible != "<none>")
			worm->getClient()->getNetEngine()->SendText(networkTexts->sHiderVisible, TXT_NORMAL);
	}
	// Seekers will know they are seen with this here. This reduces the effectiveness of 'wall-vision'
	/*else {
		if (networkTexts->sSeekerVisible != "<none>")
			worm->getClient()->getNetEngine()->SendText(networkTexts->sSeekerVisible, TXT_NORMAL);
	}*/

	for(int i = 0; i < MAX_WORMS; i++) {
		if(!cWorms[i].isUsed() || cWorms[i].getTeam() == worm->getTeam())
			continue;
		cWorms[i].getClient()->getNetEngine()->SendSpawnWorm(worm, worm->getPos());
		if (networkTexts->sVisibleMessage != "<none>")  {
			std::string msg;
			replace(networkTexts->sVisibleMessage, "<player>", worm->getName(), msg);
			cWorms[i].getClient()->getNetEngine()->SendText(msg, TXT_NORMAL);
		}
	}
}

void CHideAndSeek::Hide(CWorm* worm, bool message)
{
	if(!bVisible[worm->getID()])
		return;
	bVisible[worm->getID()] = false;

	if(networkTexts->sYouAreHidden != "<none>" && message)
		worm->getClient()->getNetEngine()->SendText(networkTexts->sYouAreHidden, TXT_NORMAL);
	for(int i = 0; i < MAX_WORMS; i++) {
		if(!cWorms[i].isUsed() || cWorms[i].getTeam() == worm->getTeam())
			continue;
		cWorms[i].getClient()->getNetEngine()->SendHideWorm(worm);
		if(networkTexts->sHiddenMessage != "<none>" && message) {
			std::string msg;
			replace(networkTexts->sHiddenMessage, "<player>", worm->getName(), msg);
			cWorms[i].getClient()->getNetEngine()->SendText(msg, TXT_NORMAL);
		}
	}
}

bool CHideAndSeek::CanSee(CWorm* worm1, CWorm* worm2)
{
	float length;
	int type;
	CVec dist;
	worm1->traceLine(worm2->getPos(), &length, &type, 1);
	dist = worm1->getPos() - worm2->getPos();
	if(worm1->getTeam() == SEEKER)
	{
		if(type & PX_EMPTY)
			return dist.GetLength() < (int)tLXOptions->tGameInfo.features[FT_HS_SeekerVisionRange];
		else
			return dist.GetLength() < (int)tLXOptions->tGameInfo.features[FT_HS_SeekerVisionRangeThroughWalls];
	}
	else 
	{
		if(type & PX_EMPTY)
			return dist.GetLength() < (int)tLXOptions->tGameInfo.features[FT_HS_HiderVisionRange];
		else
			return dist.GetLength() < (int)tLXOptions->tGameInfo.features[FT_HS_HiderVisionRangeThroughWalls];
	}
}

void CHideAndSeek::GenerateTimes()
{
	CMap* cMap = cServer->getMap();
	int volume = cMap->GetWidth() * cMap->GetHeight();
	enum { SMALL = 1, MEDIUM, LARGE, XLARGE } size;
	// Decide on the level size
	if(volume < 500 * 400)
		size = SMALL;
	else if(volume < 700 * 600)
		size = MEDIUM;
	else if(volume < 900 * 800)
		size = LARGE;
	else
		size = XLARGE;
	// Calculate the ratio of hiders to seekers
	float ratio = 1.0f;
	int worms[2] = { 0, 0 };
	for(int i = 0; i < MAX_WORMS; i++)
		if(cWorms[i].isUsed())
			worms[cWorms[i].getTeam()]++;
	if(worms[0] != 0 && worms[1] != 0)
		ratio = (float)worms[0] / worms[1];

	// TODO: Is this actually any good? 
	fGameLength = (45 * size * ratio);
}

