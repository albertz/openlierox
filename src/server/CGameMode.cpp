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
#include "CServerConnection.h"
#include "CServerNetEngine.h"
#include "ProfileSystem.h"

int CGameMode::GeneralGameType() {
	if(GameTeams() == 1)
		return GMT_NORMAL;
	else
		return GMT_TEAMS;
}


int CGameMode::GameTeams() {
	return 1;
}

float CGameMode::TimeLimit() {
	return (float)gameSettings[FT_TimeLimit] * 60.0f;
}

std::string CGameMode::TeamName(int t) {
	static const std::string teamnames[4] = { "blue", "red", "green", "yellow" };
	if(t >= 0 && t < 4) return teamnames[t];
	return itoa(t);
}

void CGameMode::PrepareGame()
{
	lastTimeLimitReport = int(TimeLimit());
	lastFragsLeftReport = -1;
	bFirstBlood = true;
	for(int i = 0; i < MAX_WORMS; i++) {
		iKillsInRow[i] = 0;
		iDeathsInRow[i] = 0;
	}
	cServer->SendPlaySound("prepareforbattle");
}

static void playSoundForWorm(CWorm* worm, const std::string& s) {
	if(worm->getType() == PRF_HUMAN && worm->getClient() && worm->getClient()->getNetEngine())
		worm->getClient()->getNetEngine()->SendPlaySound(s);	
}

static void playSoundForWorm(int wormId, const std::string& s) {
	CWorm* w = game.wormById(wormId, false);
	if(w)
		playSoundForWorm(w, s);
}


bool CGameMode::Spawn(CWorm* worm, CVec pos) {
	worm->Spawn(pos);
	return true;
}

void CGameMode::Kill(CWorm* victim, CWorm* killer)
{
	int oldLeadWorm = HighestScoredWorm();
	int oldLeadTeam = HighestScoredTeam();
	bool wasZeroScore = TeamScores(oldLeadTeam) == 0;
	
	// Kill or suicide message
	std::string buf;
	if( killer )
	{
		if( killer == victim )
		{
			// Suicide
			// TODO: Restore the suicide count message
			if( networkTexts->sCommitedSuicide != "<none>" )
				replacemax(networkTexts->sCommitedSuicide, "<player>", victim->getName(), buf, 1);
			killer->addSuicide();
		}
		else if( GameTeams() > 1 && killer->getTeam() == victim->getTeam() )
		{
			if( networkTexts->sTeamkill != "<none>" )
				replacemax(networkTexts->sTeamkill, "<player>", killer->getName(), buf, 1);
			killer->addTeamkill();
		}
		else
		{
			if(networkTexts->sKilled != "<none>")
			{
				replacemax(networkTexts->sKilled, "<killer>", killer->getName(), buf, 1);
				replacemax(buf, "<victim>", victim->getName(), buf, 1);
			}
			killer->addKill();
		}
	}
	else
	{
		// TODO: message if no killer
	}
	if( buf != "" )
		cServer->SendGlobalText(buf, TXT_NORMAL);
	
	// First blood
	if(bFirstBlood && killer && killer != victim && networkTexts->sFirstBlood != "<none>")  {
		bFirstBlood = false;
		cServer->SendGlobalText(replacemax(networkTexts->sFirstBlood, "<player>",
								killer->getName(), 1), TXT_NORMAL);
	}

	// Kills & deaths in row
	if(killer && killer != victim) {
		iKillsInRow[killer->getID()]++;
		iDeathsInRow[killer->getID()] = 0;
	}
	iKillsInRow[victim->getID()] = 0;
	iDeathsInRow[victim->getID()]++;

	// Killing spree message
	if(killer && killer != victim) {
		switch(iKillsInRow[killer->getID()]) {
			case 3:
				playSoundForWorm(killer, "03kills");
				if(networkTexts->sSpree1 != "<none>")
					cServer->SendGlobalText(replacemax(networkTexts->sSpree1, "<player>",
											killer->getName(), 1), TXT_NORMAL);
				break;
			case 5:
				playSoundForWorm(killer, "05kills");
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
				playSoundForWorm(killer, "10kills");
				if(networkTexts->sSpree5 != "<none>")
					cServer->SendGlobalText(replacemax(networkTexts->sSpree5, "<player>",
											killer->getName(), 1), TXT_NORMAL);
				break;
			case 15:
			case 20:
			case 25:
			case 30:
				playSoundForWorm(killer, itoa(iKillsInRow[killer->getID()]) + "kills");
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
	victim->Kill(true);
	if(victim->getLives() == WRM_OUT && networkTexts->sPlayerOut != "<none>") {
		playSoundForWorm(victim, "terminated");
		cServer->SendGlobalText(replacemax(networkTexts->sPlayerOut, "<player>",
								victim->getName(), 1), TXT_NORMAL);
	}
	
	if( GameTeams() > 1 )
	{
		std::vector<int> worms(4, 0);
		for_each_iterator(CWorm*, w, game.worms())
			if(w->get()->getLives() != WRM_OUT)
				if(w->get()->getTeam() >= 0 && w->get()->getTeam() < MAX_TEAMS)
					worms[w->get()->getTeam()]++;
	
		// Victim's team is out of the game
		if(worms[victim->getTeam()] == 0 && networkTexts->sTeamOut != "<none>")
			cServer->SendGlobalText(replacemax(networkTexts->sTeamOut, "<team>",
				TeamName(victim->getTeam()), 1), TXT_NORMAL);
	}
	
	int newLeadWorm = HighestScoredWorm();
	int newLeadTeam = HighestScoredTeam();

	if(oldLeadWorm != newLeadWorm) {
		playSoundForWorm(oldLeadWorm, "leadlost");
		playSoundForWorm(newLeadWorm, "leadgained");		
	}
	
	if(isTeamGame() && (oldLeadTeam != newLeadTeam || wasZeroScore)) {
		// for all
		cServer->SendPlaySound(TeamName(newLeadTeam) + "teamtakeslead");
	}
	
	if((int)gameSettings[FT_KillLimit] > 0) {
		CWorm* w = (newLeadWorm >= 0) ? game.wormById(newLeadTeam, false) : NULL;
		int rest = w ? ((int)gameSettings[FT_KillLimit] - w->getScore()) : 0;
		if(lastFragsLeftReport < 0 || lastFragsLeftReport > rest) {
			if(rest >= 1 && rest <= 3)
				cServer->SendPlaySound(itoa(rest) + "fragsleft");
			lastFragsLeftReport = rest;
		}
	}
	else if(isTeamGame() && (int)gameSettings[FT_TeamScoreLimit] > 0) {
		int rest = (int)gameSettings[FT_TeamScoreLimit] - TeamScores(newLeadTeam);
		if(lastFragsLeftReport < 0 || lastFragsLeftReport > rest) {
			if(rest >= 1 && rest <= 3)
				cServer->SendPlaySound(itoa(rest) + "fragsleft");
			lastFragsLeftReport = rest;
		}		
	}
}

void CGameMode::Drop(CWorm* worm)
{
	if (!worm || worm->getID() < 0 || worm->getID() >= MAX_WORMS) {
		errors << "Dropped an invalid worm" << endl;
		return;
	}
	
	iKillsInRow[worm->getID()] = 0;
	iDeathsInRow[worm->getID()] = 0;
}

static int getWormHitKillLimit() {
	if((int)gameSettings[FT_KillLimit] <= 0) return -1;
	
	for_each_iterator(CWorm*, w, game.worms()) {
		if(w->get()->getScore() >= (int)gameSettings[FT_KillLimit])
			return w->get()->getID();
	}
	
	return -1;
}

bool CGameMode::CheckGameOver() {
	// In game?
	if (!cServer || cServer->getState() == SVS_LOBBY || cServer->getGameOver())
		return true;

	// check for teamscorelimit
	if(GameTeams() > 1 && (int)gameSettings[FT_TeamScoreLimit] > 0) {
		for(int i = 0; i < GameTeams(); ++i) {
			if(TeamScores(i) >= (int)gameSettings[FT_TeamScoreLimit]) {
				// TODO: make configureable
				cServer->SendGlobalText("Team score limit " + itoa((int)gameSettings[FT_TeamScoreLimit]) + " hit by " + TeamName(i), TXT_NORMAL);
				notes << "team score limit hit by team " << i << endl;
				return true;
			}
		}
	}

	// check for maxkills
	if((int)gameSettings[FT_KillLimit] > 0) {
		int w = getWormHitKillLimit();
		if(w >= 0) {
			// TODO: make configureable
			cServer->SendGlobalText("Kill limit " + itoa((int)gameSettings[FT_KillLimit]) + " hit by worm " + game.wormName(w), TXT_NORMAL);
			notes << "worm " << w << " hit the kill limit" << endl;
			return true;
		}
	}
	
	// Check if the timelimit has been reached
	if(TimeLimit() > 0) {		
		if (cServer->getServerTime() > TimeLimit()) {
			cServer->SendPlaySound("timeoutcalled");
			if(networkTexts->sTimeLimit != "<none>")
				cServer->SendGlobalText(networkTexts->sTimeLimit, TXT_NORMAL);
			notes << "time limit (" << TimeLimit() << ") reached with current time " << cServer->getServerTime().seconds();
			notes << " -> game over" << endl;
			return true;
		}
	}
	
	bool allowEmptyGames =
		gameSettings[FT_AllowEmptyGames] &&
		tLXOptions->bAllowConnectDuringGame &&
		tLX->iGameType != GME_LOCAL &&
		(int)gameSettings[FT_Lives] < 0;
	
	if(!allowEmptyGames) {
		// TODO: move that worms-num calculation to GameServer
		int worms = 0;
		for_each_iterator(CWorm*, w, game.worms())
			if (w->get()->getLives() != WRM_OUT)
				worms++;

		int minWormsNeeded = 2;
		if(tLX->iGameType == GME_LOCAL && game.worms()->size() == 1) minWormsNeeded = 1;
		if(worms < minWormsNeeded) {
			// TODO: make configureable
			// HINT: thins is kinda spammy, because in this kind of games everybody knows why it ended
			//cServer->SendGlobalText("Too few players in game", TXT_NORMAL);
			notes << "only " << worms << " players left in game -> game over" << endl;
			return true;
		}
	}

	if(!allowEmptyGames && GameTeams() > 1) {
		// Only one team left?
		int teams = 0;
		for(int i = 0; i < GameTeams(); i++)  {
			if(WormsAliveInTeam(i)) {
				teams++;
			}
		}
		
		// Only one team left
		if(teams <= 1) {
			// TODO: make configureable
			// HINT: this is kinda spammy because everyone with IQ > 50 knows why the game ended
			//cServer->SendGlobalText("Too few teams in game", TXT_NORMAL);
			notes << "Game has finished, all non-winning teams are out!" << endl;
			return true;
		}
	}
	
	return false;
}

void CGameMode::Simulate() {
	// play time remaining sounds
	if(TimeLimit() > 0) {		
		if(cServer->getServerTime() <= TimeLimit()) {
			int restSecs = int( (TimeDiff(TimeLimit()) - cServer->getServerTime()).seconds() );
			// can happen if we set the timelimit while in game
			if(lastTimeLimitReport < 0) lastTimeLimitReport = restSecs + 1;
			
			for(int i = 1; i <= 5; ++i)
				if(restSecs <= i && lastTimeLimitReport > i) {
					cServer->SendPlaySound(itoa(i));
					lastTimeLimitReport = restSecs;
				}
			
			if(restSecs <= 60 && lastTimeLimitReport > 60) {
				cServer->SendPlaySound("1minuteremains");
				lastTimeLimitReport = restSecs;
			}
			else if(restSecs <= 5*60  && lastTimeLimitReport > 5*60) {
				cServer->SendPlaySound("5minutesremain");
				lastTimeLimitReport = restSecs;		
			}
		}
	}	
}

int CGameMode::Winner() {
	// In case of last man standing, that one must win
	CWorm *alive = cServer->getFirstAliveWorm();
	if (cServer->getAliveWormCount() < 2 && alive)  {
		return alive->getID();
	}

	return HighestScoredWorm(); // For other cases (max kills etc.)
}

int CGameMode::HighestScoredWorm() {
	CWorm* highest = NULL;
	for_each_iterator(CWorm*, w, game.worms()) {
		if(!highest) highest = w->get();
		else
			highest = (CompareWormsScore(w->get(), highest) > 0) ? w->get() : highest; // Take the worm with the best score
	}
	
	return highest ? highest->getID() : -1;
}

int CGameMode::CompareWormsScore(CWorm* w1, CWorm* w2) {

	// Lives first
	if((int)cClient->getGameLobby()[FT_Lives] >= 0) {
		// Only exception is max kills - if there's a worm with max kills hit, that one is clearly the best
		int killLimit = (int)cClient->getGameLobby()[FT_KillLimit];
		if (killLimit > 0)  {
			if (w1->getScore() >= killLimit) return 1;
			if (w2->getScore() >= killLimit) return -1;
		}

		if (w1->getLives() > w2->getLives()) return 1;
		if (w1->getLives() < w2->getLives()) return -1;		
	}

	// If one worm is out and another has infinite lives, the infinite lives win
	if (w1->getLives() != WRM_OUT && w2->getLives() == WRM_OUT) return 1;
	if (w2->getLives() != WRM_OUT && w1->getLives() == WRM_OUT) return -1;
	
	// Kills
	if (w1->getScore() > w2->getScore()) return 1;
	if (w1->getScore() < w2->getScore()) return -1;
	
	// Damage
	return Round(w1->getDamage() - w2->getDamage());
}

int CGameMode::WinnerTeam() {
	if(GameTeams() > 1)  {
		// Only one team left, that one must be the winner
		if (cServer->getAliveTeamCount() < 2 && (int)gameSettings[FT_Lives] >= 0)  {
			for_each_iterator(CWorm*, w, game.worms())
				if (w->get()->getLives() != WRM_OUT && w->get()->getTeam() >= 0 && w->get()->getTeam() < MAX_TEAMS)
					return w->get()->getTeam();
		}

		return HighestScoredTeam();
	} else
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
	for_each_iterator(CWorm*, w, game.worms())
		if(w->get()->getLives() != WRM_OUT && w->get()->getTeam() == t)
			c++;
	return c;
}

int CGameMode::TeamScores(int t) {
	return TeamKills(t);
}

int CGameMode::TeamKills(int t) {
	int c = 0;
	for_each_iterator(CWorm*, w, game.worms())
		if(w->get()->getTeam() == t)
			c += w->get()->getScore();
	return c;
}

float CGameMode::TeamDamage(int t) {
	float c = 0;
	for_each_iterator(CWorm*, w, game.worms())
		if(w->get()->getTeam() == t)
			c += w->get()->getDamage();
	return c;
}


int CGameMode::CompareTeamsScore(int t1, int t2) {
	// Lives first
	if((int)gameSettings[FT_Lives] >= 0) {
		int d = WormsAliveInTeam(t1) - WormsAliveInTeam(t2);
		if(d != 0) return d;
	}
	
	// if there is a custom teamscore function
	{
		int d = TeamScores(t1) - TeamScores(t2);
		if(d != 0) return d;
	}
	
	// Kills
	{
		int d = TeamKills(t1) - TeamKills(t2);
		if(d != 0) return d;
	}
	
	// Damage
	return Round(TeamDamage(t1) - TeamDamage(t2));
}


extern CGameMode* gameMode_DeathMatch;
extern CGameMode* gameMode_TeamDeathMatch;
extern CGameMode* gameMode_Tag;
extern CGameMode* gameMode_Demolitions;
extern CGameMode* gameMode_HideAndSeek;
extern CGameMode* gameMode_CaptureTheFlag;
extern CGameMode* gameMode_Race;
extern CGameMode* gameMode_TeamRace;


static CGameMode* gameModes[] = {
	gameMode_DeathMatch,
	gameMode_TeamDeathMatch,
	gameMode_Tag,
	gameMode_Demolitions,
	gameMode_HideAndSeek,
	gameMode_CaptureTheFlag,
	gameMode_Race,
	gameMode_TeamRace
};

static size_t gameModesSize = sizeof(gameModes) / sizeof(CGameMode*);

void InitGameModes() {}

CGameMode* GameMode(GameModeIndex i) {
	if(i < 0 || (uint)i >= gameModesSize) {
		errors << "GameMode(): gamemode index " << i << " requested, we don't have such one" << endl;
		return NULL;
	}
	
	return gameModes[i];
}

CGameMode* GameMode(const std::string& name) {
	for(size_t i = 0; i < gameModesSize; ++i) {
		if(name == gameModes[i]->Name())
			return gameModes[i];
	}
	warnings << "GameMode(): gamemode name " << name << " requested, we don't have such one" << endl;
	return NULL;
}

GameModeIndex GetGameModeIndex(CGameMode* gameMode, GameModeIndex fallback) {
	if(gameMode == NULL) return fallback;
	for(size_t i = 0; i < gameModesSize; ++i) {
		if(gameMode == gameModes[i])
			return (GameModeIndex)i;
	}
	return fallback;
}



Iterator<CGameMode*>::Ref GameModeIterator() {
	return GetConstIterator(Array(gameModes, gameModesSize));
}

std::string guessGeneralGameTypeName(int iGeneralGameType)
{
	if( iGeneralGameType == GMT_NORMAL )
		return "Death Match";
	if( iGeneralGameType == GMT_TEAMS )
		return "Team Death Match";
	if( iGeneralGameType == GMT_TIME )
		return "Tag";
	if( iGeneralGameType == GMT_DIRT )
		return "Demolitions";
	return "";
};
