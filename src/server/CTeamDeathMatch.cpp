/*
 OpenLieroX
 
 team death match gamemode
 
 created 2009-02-09
 code under LGPL
 */

#include <iostream>
#include <string>
#include "CWorm.h"
#include "Options.h"
#include "Consts.h"
#include "CServer.h"
#include "CClient.h"
#include "CGameMode.h"
#include "Consts.h"

class CTeamDeathMatch : public CGameMode {
public:
	
	virtual void PrepareGame();
	virtual void Kill(CWorm* victim, CWorm* killer);
	virtual void Drop(CWorm* worm);
	virtual bool CheckGameOver();
	virtual int  GameType() { return GMT_TEAMS; }
	virtual int  GameTeams() { return MAXTEAMS; }
	virtual int  Winner() {
		// There's no single winner so this will do for now
		return -1;	
	}
	virtual std::string Name() { return "Team Death Match"; }
	
	virtual int TeamScores(int t) {
		if(t >= 0 && t < MAXTEAMS) return teamScore[t];
		return -1;
	}
	
	bool isValidTeam(int t) { return t >= 0 && t < MAXTEAMS; }
	
	void ChangeTeamScore(int t, int diff) {
		if(!isValidTeam(t)) {
			errors << "CTeamDeathMatch::ChangeTeamScore: invalid team nr " << t << endl;
			return;
		}
		teamScore[t] += diff;
	}
	
protected:
	static const int MAXTEAMS = 4;
	int teamScore[MAXTEAMS];
	bool bFirstBlood;
	int  iKillsInRow[MAX_WORMS];
	int  iDeathsInRow[MAX_WORMS];
};

void CTeamDeathMatch::PrepareGame()
{
	bFirstBlood = true;
	for(int i = 0; i < MAX_WORMS; i++) {
		iKillsInRow[i] = 0;
		iDeathsInRow[i] = 0;
	}

	for(int i = 0; i < MAXTEAMS; i++) {
		teamScore[i] = 0;
	}
}

void CTeamDeathMatch::Kill(CWorm* victim, CWorm* killer)
{
	// TODO: move that to CGameMode
	// TODO: share code with CGameMode::Kill()
	
	// Kill or suicide message
	if(killer && networkTexts->sKilled != "<none>") {
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
	if(bFirstBlood && killer && killer != victim && networkTexts->sFirstBlood != "<none>")  {
		bFirstBlood = false;
		cServer->SendGlobalText(replacemax(networkTexts->sFirstBlood, "<player>",
			killer->getName(), 1), TXT_NORMAL);
	}
	
	// Teamkiller
	if(killer && killer != victim && killer->getTeam() == victim->getTeam() && networkTexts->sTeamkill != "<none>") {
		cServer->SendGlobalText(replacemax(networkTexts->sTeamkill, "<player>",
			killer->getName(), 1), TXT_NORMAL);
	}
	
	// Kills & deaths in row
	if(killer && killer != victim) {
		if(killer->getTeam() != victim->getTeam()) {
			killer->AddKill();
			iKillsInRow[killer->getID()]++;
			iDeathsInRow[killer->getID()] = 0;
			ChangeTeamScore(killer->getTeam(), 1);
		}
		else { // killed team mate
			if(tLXOptions->tGameInfo.features[FT_SuicideDecreasesScore]) {
				ChangeTeamScore(killer->getTeam(), -1);
			}
		}
	}
	iKillsInRow[victim->getID()] = 0;
	iDeathsInRow[victim->getID()]++;

	// Killing spree message
	if(killer && killer != victim && killer->getTeam() != victim->getTeam()) {
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

	int worms[4] = { 0, 0, 0, 0 };
	for(int i = 0; i < MAX_WORMS; i++)
		if(cServer->getWorms()[i].isUsed() && cServer->getWorms()[i].getLives() != WRM_OUT)
			if(isValidTeam(cServer->getWorms()[i].getTeam()))
				worms[cServer->getWorms()[i].getTeam()]++;
	
	// Victim's team is out of the game
	if(worms[victim->getTeam()] == 0 && networkTexts->sTeamOut != "<none>")
		cServer->SendGlobalText(replacemax(networkTexts->sTeamOut, "<team>",
			TeamName(victim->getTeam()), 1), TXT_NORMAL);
}

void CTeamDeathMatch::Drop(CWorm* worm)
{
	if (!worm || worm->getID() < 0 || worm->getID() >= MAX_WORMS) {
		errors << "Dropped an invalid worm" << endl;
		return;
	}
	
	iKillsInRow[worm->getID()] = 0;
	iDeathsInRow[worm->getID()] = 0;
}

bool CTeamDeathMatch::CheckGameOver()
{
	return CGameMode::CheckGameOver();
}


static CTeamDeathMatch gameMode;
CGameMode* gameMode_TeamDeathMatch = &gameMode;
