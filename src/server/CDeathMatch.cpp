#include <iostream>
#include "CDeathMatch.h"
#include "CWorm.h"
#include "Options.h"
#include "Consts.h"
#include "CServer.h"
#include "CClient.h"

CDeathMatch::CDeathMatch(GameServer* server, CWorm* worms)
{
	cServer = server;
	cWorms = worms;
}

CDeathMatch::~CDeathMatch()
{
}

void CDeathMatch::PrepareGame()
{
	bFirstBlood = true;
	for(int i = 0; i < MAX_WORMS; i++) {
		iKillsInRow[i] = 0;
		iDeathsInRow[i] = 0;
	}
}

void CDeathMatch::PrepareWorm(CWorm* worm)
{
}

bool CDeathMatch::Spawn(CWorm* worm, CVec pos)
{
	worm->Spawn(pos);
	return true;
}

void CDeathMatch::Kill(CWorm* victim, CWorm* killer)
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
}

bool CDeathMatch::Shoot(CWorm* worm)
{
	return true;
}

void CDeathMatch::Drop(CWorm* worm)
{
}

void CDeathMatch::Simulate()
{
}

bool CDeathMatch::CheckGame()
{
	// Empty games, no need to check anything?
	if(tLXOptions->tGameInfo.features[FT_AllowEmptyGames])
		return false;

	int worms = 0;
	int wormid = 0;
	for(int i = 0; i < MAX_WORMS; i++)
		if(cWorms[i].isUsed() && cWorms[i].getLives() != WRM_OUT) {
			worms++;
			wormid = i;
		}
	if(worms <= 1) {
		if (networkTexts->sPlayerHasWon != "<none>")
			cServer->SendGlobalText((replacemax(networkTexts->sPlayerHasWon, "<player>",
				cWorms[wormid].getName(), 1)), TXT_NORMAL);
		iWinner = wormid;
		return true;
	}
	return false;
}

int CDeathMatch::GameType()
{
	return GMT_NORMAL;
}

int CDeathMatch::GameTeams()
{
	return 0;
}

int CDeathMatch::Winner()
{
	return iWinner;
}

