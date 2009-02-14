/*
 OpenLieroX
 
 tag gamemode
 
 created 2009-02-14
 code under LGPL
 */

#include <iostream>
#include "CTag.h"
#include "CWorm.h"
#include "Options.h"
#include "Consts.h"
#include "CServer.h"
#include "CClient.h"

CTag::CTag(GameServer* server, CWorm* worms)
{
	cServer = server;
	cWorms = worms;
}

CTag::~CTag()
{
}

void CTag::PrepareGame()
{
	fGameStart = tLX->fCurTime;

	bFirstBlood = true;
	for(int i = 0; i < MAX_WORMS; i++) {
		iKillsInRow[i] = 0;
		iDeathsInRow[i] = 0;
	}
}

void CTag::PrepareWorm(CWorm* worm)
{
	worm->setTagIT(false);
	worm->setTagTime(0);
}

bool CTag::Spawn(CWorm* worm, CVec pos)
{
	worm->Spawn(pos);
	return true;
}

void CTag::Kill(CWorm* victim, CWorm* killer)
{
	// Kill or suicide message
	if(networkTexts->sKilled != "<none>") {
		std::string buf;
		if(killer != victim) {
			replacemax(networkTexts->sKilled, "<killer>", killer->getName(), buf, 1);
			replacemax(buf, "<victim>", victim->getName(), buf, 1);
		}
		// TODO: Restore the suicide count message
		else
			replacemax(networkTexts->sCommitedSuicide, "<player>", victim->getName(), buf, 1);
		cServer->SendGlobalText(buf, TXT_NORMAL);
	}
	
	// First blood
	if(bFirstBlood && killer != victim && networkTexts->sFirstBlood != "<none>")  {
		bFirstBlood = false;
		cServer->SendGlobalText(replacemax(networkTexts->sFirstBlood, "<player>",
			killer->getName(), 1), TXT_NORMAL);
	}

	// Kills & deaths in row
	if(killer != victim) {
		killer->AddKill();
		iKillsInRow[killer->getID()]++;
		iDeathsInRow[killer->getID()] = 0;
	}
	iKillsInRow[victim->getID()] = 0;
	iDeathsInRow[victim->getID()]++;

	// Killing spree message
	if(killer != victim) {
		switch(iKillsInRow[killer->getID()]) {
		case 3:
			if(networkTexts->sSpree1 != "<none>")
				cServer->SendGlobalText(replacemax(networkTexts->sSpree1, "<player>",
					killer->getName(), 1), TXT_NORMAL);
			break;
		case 5:
			if(networkTexts->sSpree2 != "<none>")
				cServer->SendGlobalText(replacemax(networkTexts->sSpree2, "<player>",
					killer->getName(), 1), TXT_NORMAL);
			break;
		case 7:
			if(networkTexts->sSpree3 != "<none>")
				cServer->SendGlobalText(replacemax(networkTexts->sSpree3, "<player>",
					killer->getName(), 1), TXT_NORMAL);
			break;
		case 9:
			if(networkTexts->sSpree4 != "<none>")
				cServer->SendGlobalText(replacemax(networkTexts->sSpree4, "<player>",
					killer->getName(), 1), TXT_NORMAL);
			break;
		case 10:
			if(networkTexts->sSpree5 != "<none>")
				cServer->SendGlobalText(replacemax(networkTexts->sSpree5, "<player>",
					killer->getName(), 1), TXT_NORMAL);
			break;
		}
	}

	// Dying spree message
	switch(iDeathsInRow[victim->getID()]) {
	case 3:
		if(networkTexts->sDSpree1 != "<none>")
			cServer->SendGlobalText(replacemax(networkTexts->sDSpree1, "<player>",
				victim->getName(), 1), TXT_NORMAL);
		break;
	case 5:
		if(networkTexts->sDSpree2 != "<none>")
			cServer->SendGlobalText(replacemax(networkTexts->sDSpree2, "<player>",
				victim->getName(), 1), TXT_NORMAL);
		break;
	case 7:
		if(networkTexts->sDSpree3 != "<none>")
			cServer->SendGlobalText(replacemax(networkTexts->sDSpree3, "<player>",
				victim->getName(), 1), TXT_NORMAL);
		break;
	case 9:
		if(networkTexts->sDSpree4 != "<none>")
			cServer->SendGlobalText(replacemax(networkTexts->sDSpree4, "<player>",
				victim->getName(), 1), TXT_NORMAL);
		break;
	case 10:
		if(networkTexts->sDSpree5 != "<none>")
			cServer->SendGlobalText(replacemax(networkTexts->sDSpree5, "<player>",
				victim->getName(), 1), TXT_NORMAL);
		break;
	}

	// Victim is out of the game
	if(victim->Kill()&& networkTexts->sPlayerOut != "<none>")
		cServer->SendGlobalText(replacemax(networkTexts->sPlayerOut, "<player>",
			victim->getName(), 1), TXT_NORMAL);

	// Tag another worm
	if (victim->getTagIT())  {
		if (killer != victim)  {
			if (killer->getLives() == WRM_OUT)  // The killer got killed and out in the meanwhile
				TagRandomWorm();
			else
				TagWorm(killer);
		} else // Suicide
			TagRandomWorm();
	}
}

bool CTag::Shoot(CWorm* worm)
{
	return true;
}

///////////////////
// Returns true if w1 has a better score than w2
bool CTag::CompareWormsScore(CWorm *w1, CWorm *w2)
{
	// Tag time
	if (w1->getTagTime() > w2->getTagTime())
		return true;
	else if (w1->getTagTime() < w2->getTagTime())
		return false;

	// HINT: we get there only when the tag times are equal

	// Lives
	if (w1->getLives() > w2->getLives() && tLXOptions->tGameInfo.iLives >= 0)
		return true;

	// Kills
	if (w1->getKills() > w2->getKills())
		return true;

	// Damage
	return w1->getDamage() > w2->getDamage();
}

void CTag::Drop(CWorm* worm)
{
	if (!worm || worm->getID() < 0 || worm->getID() >= MAX_WORMS)
		errors << "Dropped an invalid worm" << endl;

	iKillsInRow[worm->getID()] = 0;
	iDeathsInRow[worm->getID()] = 0;

	// Tag another worm
	if (worm->getTagIT())
		TagRandomWorm();
}

void CTag::Simulate()
{
	// Increase the tag time
	bool have_it = false;
	for (int i = 0; i < MAX_WORMS; i++)  {
		if (cWorms[i].isUsed() && cWorms[i].getTagIT() && cWorms[i].getLives() != WRM_OUT)  {
			if (cWorms[i].getAlive())
				cWorms[i].incrementTagTime(tLX->fRealDeltaTime);
			have_it = true;
			break;
		}
	}

	// No worm is tagged (happens at the beginning of the game)
	if (!have_it)
		TagRandomWorm();

	// Check if any of the worms reached the maximum tag time
	if (tLXOptions->tGameInfo.iTagLimit > 0)
		for (int i = 0; i < MAX_WORMS; i++)
			if(cWorms[i].isUsed() && cWorms[i].getTagTime() >= tLXOptions->tGameInfo.iTagLimit * 60.0f)
				cServer->RecheckGame();
}

bool CTag::CheckGame()
{
	// Check if the timelimit has been reached
	bool timelimit = tLXOptions->tGameInfo.fTimeLimit > 0 &&
			cServer->getServerTime() > tLXOptions->tGameInfo.fTimeLimit*60.0;

	// Empty games, no need to check anything?
	if(tLXOptions->tGameInfo.features[FT_AllowEmptyGames] && !timelimit)
		return false;

	// In game?
	if (!cServer || cServer->getState() == SVS_LOBBY)
		return false;

	int worms = 0;
	int wormid = 0;
	for(int i = 0; i < MAX_WORMS; i++)
		if(cWorms[i].isUsed()) {
			if (cWorms[i].getLives() != WRM_OUT)
				worms++;
			wormid = CompareWormsScore(&cWorms[i], &cWorms[wormid]) ? i : wormid; // Take the worm with the best score
		}

	// Zero or one worm left, end the game
	if(worms <= 1 || timelimit) {
		if (networkTexts->sPlayerHasWon != "<none>")
			cServer->SendGlobalText((replacemax(networkTexts->sPlayerHasWon, "<player>",
				cWorms[wormid].getName(), 1)), TXT_NORMAL);
		iWinner = wormid;

		// Timelimit message
		if (timelimit)
			cServer->SendGlobalText(networkTexts->sTimeLimit, TXT_NORMAL);

		return true;
	}

	// Check if any of the worms reached the maximum tag time
	if (tLXOptions->tGameInfo.iTagLimit > 0)  {
		if(cWorms[wormid].isUsed() && 
			cWorms[wormid].getTagTime() >= tLXOptions->tGameInfo.iTagLimit * 60.0f) {

			notes << cWorms[wormid].getName() << " has reached the maximum tag time" << endl;

			iWinner = wormid;

			// Message
			if (networkTexts->sPlayerHasWon != "<none>")
				cServer->SendGlobalText((replacemax(networkTexts->sPlayerHasWon, "<player>",
					cWorms[wormid].getName(), 1)), TXT_NORMAL);

			return true;
		}
	}

	return false;
}

int CTag::GameType()
{
	return GMT_TIME;
}

int CTag::GameTeams()
{
	return 0;
}

int CTag::Winner()
{
	return iWinner;
}

void CTag::TagWorm(CWorm *worm)
{
	// Safety check
	if(!worm || cServer->getState() != SVS_PLAYING)
		return;

	// Go through all the worms, setting their tag to false
	for(short i = 0; i < MAX_WORMS; i++)
		if(cWorms[i].isUsed())
			cWorms[i].setTagIT(false);

	// Some safety warnings
	if (!worm->isUsed())  {
		errors << "Tagging an unused worm (ID " << worm->getID() << ")" << endl;
	}


	worm->setTagIT(true);

	// Let everyone know this guy is tagged
	cServer->SendWormTagged(worm);

	// Take care of the <none> tag
	if (networkTexts->sWormIsIt != "<none>")  {
		cServer->SendGlobalText((replacemax(networkTexts->sWormIsIt, "<player>", worm->getName(), 1)),
						TXT_NORMAL);
	}
}

////////////////
// Tag a random worm
void CTag::TagRandomWorm()
{
	float time = 99999;
	std::vector<int> all_lowest;

	// Go through finding the worm with the lowest tag time
	// A bit more fairer then random picking
	unsigned short i;
	for(i=0;i<MAX_WORMS;i++) {
		if(cWorms[i].isUsed() && cWorms[i].getLives() != WRM_OUT) {
			if(cWorms[i].getTagTime() < time) {
				time = cWorms[i].getTagTime();
			}
		}
	}

	// Find all the worms that have the lowest time
	for (i=0;i<MAX_WORMS;i++)  {
		if (cWorms[i].isUsed() && cWorms[i].getLives() != WRM_OUT)  {
			if (cWorms[i].getTagTime() == time)  {
				all_lowest.push_back(i);
			}
		}
	}

	// Choose a random worm from all those having the lowest time
	int random_lowest = GetRandomInt((int)all_lowest.size()-1);


	// Tag the lowest tagged worm
	TagWorm(&cWorms[all_lowest[random_lowest]]);
}

