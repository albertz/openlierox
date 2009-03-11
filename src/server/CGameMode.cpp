/*
 *  CGameMode.cpp
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 02.03.09.
 *  code under LGPL
 *
 */

#include "CGameMode.h"
#include "CServer.h"
#include "Options.h"
#include "CWorm.h"
#include "Protocol.h"

int CGameMode::GeneralGameType() {
	if(GameTeams() == 1)
		return GMT_NORMAL;
	else
		return GMT_TEAMS;
}


int CGameMode::GameTeams() {
	return 1;
}

std::string CGameMode::TeamName(int t) {
	static const std::string teamnames[4] = { "blue", "red", "green", "yellow" };
	if(t >= 0 && t < 4) return teamnames[t];
	return itoa(t);
}

void CGameMode::PrepareGame()
{
	bFirstBlood = true;
	for(int i = 0; i < MAX_WORMS; i++) {
		iKillsInRow[i] = 0;
		iDeathsInRow[i] = 0;
	}
}

bool CGameMode::Spawn(CWorm* worm, CVec pos) {
	worm->Spawn(pos);
	return true;
}

void CGameMode::Kill(CWorm* victim, CWorm* killer)
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

bool CGameMode::CheckGameOver() {
	// In game?
	if (!cServer || cServer->getState() == SVS_LOBBY || cServer->getGameOver())
		return true;

	// Check if the timelimit has been reached
	if(tLXOptions->tGameInfo.fTimeLimit > 0) {
		if (cServer->getServerTime() > tLXOptions->tGameInfo.fTimeLimit*60.0) {
			if(networkTexts->sTimeLimit != "<none>")
				cServer->SendGlobalText(networkTexts->sTimeLimit, TXT_NORMAL);
			notes << "time limit (" << (tLXOptions->tGameInfo.fTimeLimit*60.0) << ") reached with current time " << cServer->getServerTime().seconds();
			notes << " -> game over" << endl;
			return true;
		}
	}
	
	if(!tLXOptions->tGameInfo.features[FT_AllowEmptyGames] || tLX->iGameType == GME_LOCAL) {
		// TODO: move that to GameServer
		int worms = 0;
		for(int i = 0; i < MAX_WORMS; i++)
			if(cServer->getWorms()[i].isUsed()) {
				if (cServer->getWorms()[i].getLives() != WRM_OUT)
					worms++;
			}

		int minWormsNeeded = 2;
		if(tLX->iGameType == GME_LOCAL && cServer->getNumPlayers() == 1) minWormsNeeded = 1;
		if(worms < minWormsNeeded) {
			notes << "only " << worms << " players left in game -> game over" << endl;
			return true;
		}
	}

	if(GameTeams() > 1) {
		// Only one team left?
		int teams = 0;
		for(int i = 0; i < GameTeams(); i++)  {
			if(WormsAliveInTeam(i)) {
				teams++;
			}
		}
		
		// Only one team left
		if(teams <= 1) {
			return true;
		}
	}
	
	return false;
}

int CGameMode::Winner() {
	return HighestScoredWorm();
}

int CGameMode::HighestScoredWorm() {
	int wormid = -1;
	for(int i = 0; i < MAX_WORMS; i++)
		if(cServer->getWorms()[i].isUsed()) {
			if(wormid < 0) wormid = i;
			else
				wormid = CompareWormsScore(&cServer->getWorms()[i], &cServer->getWorms()[wormid]) > 0 ? i : wormid; // Take the worm with the best score
		}
	
	return wormid;
}

int CGameMode::CompareWormsScore(CWorm* w1, CWorm* w2) {
	// Lives first
	if(tLXOptions->tGameInfo.iLives >= 0) {
		if (w1->getLives() > w2->getLives()) return 1;
		if (w1->getLives() < w2->getLives()) return -1;		
	}
	
	// Kills
	if (w1->getKills() > w2->getKills()) return 1;
	if (w1->getKills() < w2->getKills()) return -1;
	
	// Damage
	return w1->getDamage() - w2->getDamage();
}

int CGameMode::WinnerTeam() {
	if(GameTeams() > 1)
		return HighestScoredTeam();
	else
		return -1;
}

int CGameMode::HighestScoredTeam() {
	int team = -1;
	for(int i = 0; i < GameTeams(); i++)
		if(team < 0) team = i;
		else
			team = CompareTeamsScore(i, team) > 0 ? i : team; // Take the team with the best score
	
	return team;
}

int CGameMode::WormsAliveInTeam(int t) {
	int c = 0;
	for(int i = 0; i < MAX_WORMS; i++)
		if(cServer->getWorms()[i].isUsed() && cServer->getWorms()[i].getLives() != WRM_OUT && cServer->getWorms()[i].getTeam() == t)
			c++;
	return c;
}

int CGameMode::TeamKills(int t) {
	int c = 0;
	for(int i = 0; i < MAX_WORMS; i++)
		if(cServer->getWorms()[i].isUsed() && cServer->getWorms()[i].getTeam() == t)
			c += cServer->getWorms()[i].getKills();
	return c;
}

long CGameMode::TeamDamage(int t) {
	long c = 0;
	for(int i = 0; i < MAX_WORMS; i++)
		if(cServer->getWorms()[i].isUsed() && cServer->getWorms()[i].getTeam() == t)
			c += cServer->getWorms()[i].getDamage();
	return c;
}


int CGameMode::CompareTeamsScore(int t1, int t2) {
	// Lives first
	if(tLXOptions->tGameInfo.iLives >= 0) {
		int d = WormsAliveInTeam(t1) - WormsAliveInTeam(t2);
		if(d != 0) return d;
	}
	
	// Kills
	{
		int d = TeamKills(t1) - TeamKills(t2);
		if(d != 0) return d;
	}
	
	// Damage
	return TeamDamage(t1) - TeamDamage(t2);
}



extern CGameMode* gameMode_DeathMatch;
extern CGameMode* gameMode_TeamDeathMatch;
extern CGameMode* gameMode_Tag;
extern CGameMode* gameMode_Demolitions;
extern CGameMode* gameMode_HideAndSeek;


static std::vector<CGameMode*> gameModes;

void InitGameModes() {
	gameModes.resize(5);
	gameModes[0] = gameMode_DeathMatch;
	gameModes[1] = gameMode_TeamDeathMatch;
	gameModes[2] = gameMode_Tag;
	gameModes[3] = gameMode_Demolitions;
	gameModes[4] = gameMode_HideAndSeek;	
}

CGameMode* GameMode(GameModeIndex i) {
	if(i < 0 || (uint)i >= gameModes.size()) {
		errors << "gamemode " << i << " requested" << endl;
		return NULL;
	}
	
	return gameModes[i];
}

CGameMode* GameMode(const std::string& name) {
	for(std::vector<CGameMode*>::iterator i = gameModes.begin(); i != gameModes.end(); ++i) {
		if(name == (*i)->Name())
			return *i;
	}
	return NULL;
}

GameModeIndex GetGameModeIndex(CGameMode* gameMode) {
	if(gameMode == NULL) return GM_DEATHMATCH;
	int index = 0;
	for(std::vector<CGameMode*>::iterator i = gameModes.begin(); i != gameModes.end(); ++i, ++index) {
		if(gameMode == *i)
			return (GameModeIndex)index;
	}
	return GM_DEATHMATCH;
}



Iterator<CGameMode* const&>::Ref GameModeIterator() {
	return GetConstIterator(gameModes);
}

