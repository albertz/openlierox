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

void CHideAndSeek::PrepareGame()
{
	GenerateTimes();
	if(tLXOptions->tGameInfo.fTimeLimit > 0)
		fGameLength = tLXOptions->tGameInfo.fTimeLimit * 60;
	for(int i = 0; i < MAX_WORMS; i++) {
		fLastAlert[i] = 0;
		// TODO: Maybe we need bVisible[i] = false and no hiding because it is done in CHideAndSeek::Spawn
		bVisible[i] = true; // So we can hide
		Hide(&cServer->getWorms()[i], false);
		fWarmupTime[i] = cServer->getServerTime() + TimeDiff((float)tLXOptions->tGameInfo.features[FT_HS_HideTime]);
		/*
		// Set all the lives to 0
		cWorms[i].setLives(0);
		for(int j = 0; j < MAX_WORMS; j++)
			if(i != j && cWorms[j].isUsed())
				cWorms[j].getClient()->getNetEngine()->SendWormScore(&cWorms[i]);
		*/
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
	worm->getClient()->getNetEngine()->SendText(teamhint[CLAMP(worm->getTeam(),0,1)], TXT_NORMAL);
}

bool CHideAndSeek::Spawn(CWorm* worm, CVec pos)
{
	pos = cServer->FindSpot();
	worm->Spawn(pos);
	bVisible[worm->getID()] = false;
	// Worms only spawn visible to their own team
	for(int i = 0; i < MAX_WORMS; i++)  {
		if(cServer->getWorms()[i].isUsed() && cServer->getWorms()[i].getTeam() == worm->getTeam())
			cServer->getWorms()[i].getClient()->getNetEngine()->SendSpawnWorm(worm, pos);
		else if(cServer->getWorms()[i].isUsed())
			cServer->getWorms()[i].getClient()->getNetEngine()->SendHideWorm(worm, i);
	}
	fWarmupTime[worm->getID()] = cServer->getServerTime() + TimeDiff((float)tLXOptions->tGameInfo.features[FT_HS_HideTime]);
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
	bVisible[victim->getID()] = false;
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
	TimeDiff GameTime = cServer->getServerTime();
	// Game time up
	if(GameTime > fGameLength) {
		for(int i = 0; i < MAX_WORMS; i++)
			if(cServer->getWorms()[i].isUsed() && cServer->getWorms()[i].getLives() != WRM_OUT) {
				if(cServer->getWorms()[i].getTeam() == SEEKER)
					cServer->killWorm(i, i, cServer->getWorms()[i].getLives() + 1);
				else
					Show(&cServer->getWorms()[i], false); // People often want to see where the hiders were
			}
		return;
	}
	// Hiders have some time free from being caught and seen
	if(GameTime < (float)tLXOptions->tGameInfo.features[FT_HS_HideTime])
		return;
	// Check if any of the worms can see eachother
	int i, j;
	for(i = 0; i < MAX_WORMS; i++) 
	{
		if( !cServer->getWorms()[i].isUsed() || cServer->getWorms()[i].getLives() == WRM_OUT || !cServer->getWorms()[i].getAlive() )
			continue;
		// Hide the worm if the alert time is up
		if(fLastAlert[i] + TimeDiff((float)tLXOptions->tGameInfo.features[FT_HS_AlertTime]) < GameTime)
			Hide(&cServer->getWorms()[i]);
		for(j = 0; j < MAX_WORMS; j++) 
		{
			if( !cServer->getWorms()[j].isUsed() || cServer->getWorms()[j].getLives() == WRM_OUT || !cServer->getWorms()[j].getAlive() )
				continue;
			if(cServer->getWorms()[j].getTeam() == cServer->getWorms()[i].getTeam())
				continue;
			if( fWarmupTime[j] > GameTime )
				continue;

			if(CanSee(&cServer->getWorms()[i], &cServer->getWorms()[j]))
				Show(&cServer->getWorms()[j]);
			// Catch the hiders if they are within 10 pixels
			if(cServer->getWorms()[i].getTeam() == SEEKER && cServer->getWorms()[j].getTeam() == HIDER)
				if((cServer->getWorms()[i].getPos() - cServer->getWorms()[j].getPos()).GetLength() < 10)
				{
					int type;
					float length;
					cServer->getWorms()[i].traceLine(cServer->getWorms()[j].getPos(), &length, &type, 1);
					if( type & PX_EMPTY )	// Do not touch through thin wall
						cServer->killWorm(j, i, 0);
				}
		}
	}
}

bool CHideAndSeek::CheckGameOver()
{
	// We ignore the standard GameOver-check, we just do this check.
	// In Simulate, we kill the seeker worms when the timelimit is hit.
	
	int worms[2] = { 0, 0 };
	int winners = -1;
	static const std::string teamname[2] = { "hiding", "seeking" };

	// TODO: move out here
	for(int i = 0; i < MAX_WORMS; i++)
		if(cServer->getWorms()[i].isUsed() && cServer->getWorms()[i].getLives() != WRM_OUT && cServer->getWorms()[i].getTeam() < 2)
			worms[cServer->getWorms()[i].getTeam()]++;
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

int CHideAndSeek::GeneralGameType()
{
	return GMT_TEAMS;
}

int CHideAndSeek::GameTeams()
{
	return 2;
}

int CHideAndSeek::Winner()
{
	// there is no single winner in hideandseek, only the team
	return -1;
}

bool CHideAndSeek::NeedUpdate(CServerConnection* cl, CWorm* worm)
{
	// Clients don't recieve dirt updates without getting a full update
	// No worms, but we don't want the client to see nothing
	if(cl->getNumWorms() == 0)
		return true;

	// Different teams, and invisible so no need I think
	if(cl->getWorm(0)->getTeam() != worm->getTeam() && !bVisible[worm->getID()] && !worm->getWormState()->bCarve)
		return false;

	return true;
}

void CHideAndSeek::Show(CWorm* worm, bool message)
{
	fLastAlert[worm->getID()] = cServer->getServerTime();
	if(bVisible[worm->getID()])
		return;
	bVisible[worm->getID()] = true;

	if(worm->getTeam() == HIDER && message)  {
		if (networkTexts->sHiderVisible != "<none>")
			worm->getClient()->getNetEngine()->SendText(networkTexts->sHiderVisible, TXT_NORMAL);
	}
	// Seekers will know they are seen with this here. This reduces the effectiveness of 'wall-vision'
	/*else {
		if (networkTexts->sSeekerVisible != "<none>")
			worm->getClient()->getNetEngine()->SendText(networkTexts->sSeekerVisible, TXT_NORMAL);
	}*/

	for(int i = 0; i < MAX_WORMS; i++) {
		if(!cServer->getWorms()[i].isUsed() || cServer->getWorms()[i].getTeam() == worm->getTeam())
			continue;
		cServer->getWorms()[i].getClient()->getNetEngine()->SendHideWorm(worm, i, true);
		if (networkTexts->sVisibleMessage != "<none>" && message)  {
			std::string msg;
			replace(networkTexts->sVisibleMessage, "<player>", worm->getName(), msg);
			cServer->getWorms()[i].getClient()->getNetEngine()->SendText(msg, TXT_NORMAL);
		}
	}
}

void CHideAndSeek::Hide(CWorm* worm, bool message)
{
	if(!bVisible[worm->getID()])
		return;
	bVisible[worm->getID()] = false;

	// Removed message for seekers because it is confusing since they don't get the "you are visible" one
	if(networkTexts->sYouAreHidden != "<none>" && message && worm->getTeam() == HIDER)
		worm->getClient()->getNetEngine()->SendText(networkTexts->sYouAreHidden, TXT_NORMAL);
	for(int i = 0; i < MAX_WORMS; i++) {
		if(!cServer->getWorms()[i].isUsed() || cServer->getWorms()[i].getTeam() == worm->getTeam())
			continue;
		cServer->getWorms()[i].getClient()->getNetEngine()->SendHideWorm(worm, i);
		if(networkTexts->sHiddenMessage != "<none>" && message) {
			std::string msg;
			replace(networkTexts->sHiddenMessage, "<player>", worm->getName(), msg);
			cServer->getWorms()[i].getClient()->getNetEngine()->SendText(msg, TXT_NORMAL);
		}
	}
}

bool CHideAndSeek::CanSee(CWorm* worm1, CWorm* worm2)
{
	CVec dist;
	dist = worm1->getPos() - worm2->getPos();
	if(worm1->getTeam() == SEEKER)
	{
		if( dist.GetLength() < (int)tLXOptions->tGameInfo.features[FT_HS_SeekerVisionRangeThroughWalls] )
			return true;
		if( dist.GetLength() < (int)tLXOptions->tGameInfo.features[FT_HS_SeekerVisionRange] )
		{
			int type;
			float length;
			worm1->traceLine(worm2->getPos(), &length, &type, 1);
			if( !( type & PX_EMPTY ) )
				return false;
				
			float angle = worm1->getAngle();
			if(worm1->getDirection() == DIR_LEFT)
				angle = 180.0f - angle;
				
			while( angle > 180.0f )	// Normalize it between 180 and -180
				angle -= 360.0f;
			
			float wormAngle = VectorAngle( worm2->getPos(), worm1->getPos() ) * 180.0f / (float)PI;
			
			float angleDiff = angle - wormAngle;
			
			while( angleDiff > 180.0f ) // Normalize it
				angleDiff -= 360.0f;
			while( angleDiff < -180.0f )
				angleDiff += 360.0f;
			
			return (int)fabs(angleDiff*2.0f) < (int)tLXOptions->tGameInfo.features[FT_HS_SeekerVisionAngle];
		}
		return false;
	}
	else 
	{
		if( dist.GetLength() < (int)tLXOptions->tGameInfo.features[FT_HS_HiderVisionRangeThroughWalls] )
			return true;
		if( dist.GetLength() < (int)tLXOptions->tGameInfo.features[FT_HS_HiderVisionRange] )
		{
			int type;
			float length;
			worm1->traceLine(worm2->getPos(), &length, &type, 1);
			if(type & PX_EMPTY)
				return true;
		}
		return false;
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
	// TODO: move that out here (to GameServer)
	int worms[2] = { 0, 0 };
	for(int i = 0; i < MAX_WORMS; i++)
		if(cServer->getWorms()[i].isUsed() && cServer->getWorms()[i].getTeam() < 2)
			worms[cServer->getWorms()[i].getTeam()]++;
	if(worms[0] != 0 && worms[1] != 0)
		ratio = (float)worms[0] / worms[1];

	// TODO: Is this actually any good? 
	fGameLength = (45 * size * ratio);
}

static CHideAndSeek gameMode;
CGameMode* gameMode_HideAndSeek = &gameMode;
