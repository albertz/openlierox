#include <iostream>
#include <string>
#include "CTeamDeathMatch.h"
#include "CWorm.h"
#include "Options.h"
#include "Consts.h"
#include "CServer.h"
#include "CClient.h"

CTeamDeathMatch::CTeamDeathMatch(GameServer* server, CWorm* worms)
{
	cServer = server;
	cWorms = worms;
}

CTeamDeathMatch::~CTeamDeathMatch()
{
}

void CTeamDeathMatch::PrepareGame()
{
	bFirstBlood = true;
	for(int i = 0; i < MAX_WORMS; i++) {
		iKillsInRow[i] = 0;
		iDeathsInRow[i] = 0;
	}
}

void CTeamDeathMatch::PrepareWorm(CWorm* worm)
{
}

bool CTeamDeathMatch::Spawn(CWorm* worm, CVec pos)
{
	worm->Spawn(pos);
	return true;
}

void CTeamDeathMatch::Kill(CWorm* victim, CWorm* killer)
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
	
	// Teamkiller
	if(killer != victim && killer->getTeam() == victim->getTeam() && networkTexts->sTeamkill != "<none>") {
		cServer->SendGlobalText(replacemax(networkTexts->sTeamkill, "<player>",
			killer->getName(), 1), TXT_NORMAL);
	}
	
	// Kills & deaths in row
	if(killer != victim && killer->getTeam() != victim->getTeam()) {
		killer->AddKill();
		iKillsInRow[killer->getID()]++;
		iDeathsInRow[killer->getID()] = 0;
	}
	iKillsInRow[victim->getID()] = 0;
	iDeathsInRow[victim->getID()]++;

	// Killing spree message
	if(killer != victim && killer->getTeam() != victim->getTeam()) {
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
	if(victim->Kill() && networkTexts->sPlayerOut != "<none>")
		cServer->SendGlobalText(replacemax(networkTexts->sPlayerOut, "<player>",
			victim->getName(), 1), TXT_NORMAL);

	static const std::string teamnames[4] = { "blue", "red", "green", "yellow" };
	int worms[4] = { 0, 0, 0, 0 };
	for(int i = 0; i < MAX_WORMS; i++)
		if(cWorms[i].isUsed() && cWorms[i].getLives() != WRM_OUT)
			worms[cWorms[i].getTeam()]++;
	
	// Victim's team is out of the game
	if(worms[victim->getTeam()] == 0 && networkTexts->sTeamOut != "<none>")
		cServer->SendGlobalText(replacemax(networkTexts->sTeamOut, "<team>",
			teamnames[victim->getTeam()], 1), TXT_NORMAL);
}

bool CTeamDeathMatch::Shoot(CWorm* worm)
{
	return true;
}

void CTeamDeathMatch::Drop(CWorm* worm)
{
}

void CTeamDeathMatch::Simulate()
{
}

bool CTeamDeathMatch::CheckGame()
{
	static const std::string teamnames[4] = { "blue", "red", "green", "yellow" };
	// Empty games, no need to check anything?
	if(tLXOptions->tGameInfo.features[FT_AllowEmptyGames])
		return false;

	int worms[4] = { 0, 0, 0, 0 };
	for(int i = 0; i < MAX_WORMS; i++)
		if(cWorms[i].isUsed() && cWorms[i].getLives() != WRM_OUT)
			worms[cWorms[i].getTeam()]++;
	int team = 0;
	int teams = 0;
	for(int i = 0; i < 4; i++)
		if(worms[i]) {
			teams++;
			team = i;
		}
	if(teams <= 1) {
		if(networkTexts->sTeamHasWon != "<none>")
			cServer->SendGlobalText((replacemax(networkTexts->sTeamHasWon,
				"<team>", teamnames[team], 1)), TXT_NORMAL);
		return true;
	}
	return false;
}

int CTeamDeathMatch::GameType()
{
	return GMT_TEAMS;
}

int CTeamDeathMatch::GameTeams()
{
	return 4;
}

int CTeamDeathMatch::Winner()
{
	// There's no single winner so this will do for now
	return 0;
}

