/*
 *  CRace.cpp
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 10.05.09.
 *  code under LGPL
 *
 */

#include <algorithm>
#include "CGameMode.h"
#include "CServer.h"
#include "game/CWorm.h"
#include "FlagInfo.h"
#include "CServerConnection.h"
#include "CServerNetEngine.h"
#include "game/Game.h"

struct Race : public CGameMode {
	
	virtual std::string Name() {
		return "Race";
	}
	
	virtual GameInfoGroup getGameInfoGroupInOptions() { return GIG_Race; }
	
	virtual Version MinNeededVersion() { return OLXBetaVersion(0,58,1); }
	
	virtual void reset() {
		wayPoints.clear();
		wormSpawnPoints.clear();
		nextGoals.clear();
	}
	
	Race() { reset(); }
	
	static const unsigned int WPNUM = 4;
	std::vector<CVec> wayPoints;
	
	std::string flagName(int t) {
		return "way point " + itoa(t + 1);
	}

	void sendWormScoreUpdate(CWorm* w) {
		for(int ii = 0; ii < MAX_CLIENTS; ii++) {
			if(cServer->getClients()[ii].getStatus() != NET_CONNECTED) continue;
			if(cServer->getClients()[ii].getNetEngine() == NULL) continue;
			cServer->getClients()[ii].getNetEngine()->SendWormScore( w );
		}		
	}
	
	virtual void PrepareGame() {
		CGameMode::PrepareGame();
		reset();
		
		wayPoints.resize(WPNUM);
		for(int x = 0; x <= 1; x++)
			for(int y = 0; y <= 1; y++) {
				std::list<CVec> goodPos;
				goodPos.push_back(CVec(
								  (game.gameMap()->GetWidth() * 0.8f * x + game.gameMap()->GetWidth() * 0.2f),
								  (game.gameMap()->GetHeight() * 0.8f * y + game.gameMap()->GetHeight() * 0.2f)));
				std::list<CVec> badPos;
				badPos.push_back(CVec(
								  (game.gameMap()->GetWidth() * 0.2f * x + game.gameMap()->GetWidth() * 0.8f),
								  (game.gameMap()->GetHeight() * 0.2f * y + game.gameMap()->GetHeight() * 0.8f)));
				int t = (y == 0) ? x : (3 - x);
				wayPoints[t] = game.gameMap()->FindSpotCloseToPos(goodPos, badPos, false);
				
				if(!gameSettings[FT_InstantAirJump])
					// set the place to the ground
					wayPoints[t] = game.gameMap()->groundPos(wayPoints[t]) - CVec(0, (float)(cServer->flagInfo()->getHeight()/4));
			}
	}
	
	virtual int getWormFlag(CWorm* worm) {
		return worm->getID();
	}
	
	typedef int WormID;
	std::map<WormID,CVec> wormSpawnPoints;
	typedef int FlagID;
	typedef int WayPointID;
	std::map<FlagID,WayPointID> nextGoals;
	
	WayPointID getNextGoal(FlagID id) {
		std::map<FlagID,WayPointID>::iterator f = nextGoals.find(id);
		if(f == nextGoals.end()) return 1;
		else return f->second;
	}
	
	virtual bool Spawn(CWorm* worm, CVec pos) {
		if(wayPoints.size() == 0) {
			errors << "Race::Spawn: not prepared yet" << endl;
			return true;
		}
		
		std::map<WormID,CVec>::iterator sp = wormSpawnPoints.find(worm->getID());
		if(sp == wormSpawnPoints.end())
			pos = wayPoints[0];
		else
			pos = sp->second;
		
		if(!cServer->flagInfo()->getFlag(getWormFlag(worm))) {
			// we have to create the new flag, there isn't any yet
			cServer->flagInfo()->applyInitFlag(getWormFlag(worm), wayPoints[1]);
			nextGoals[getWormFlag(worm)] = 1;
		}
		
		worm->Spawn(pos);
		
		return true;
	}
	
	virtual void Kill(CWorm* victim, CWorm* killer) {
		// Victim is out of the game
		victim->Kill(true);
		if(victim->getLives() == WRM_OUT && networkTexts->sPlayerOut != "<none>")
			cServer->SendGlobalText(replacemax(networkTexts->sPlayerOut, "<player>",
											   victim->getName(), 1), TXT_NORMAL);
		
		wormSpawnPoints[victim->getID()] = victim->getPos();
	}
	
	virtual void addScore(CWorm* w) {
		w->addKill();
		sendWormScoreUpdate(w);
		cServer->SendGlobalText(TeamNameForWorm(w) + " is in round " + itoa(Round(w)), TXT_NORMAL);
	}
	
	virtual std::string TeamNameForWorm(CWorm* w) {
		return w->getName();
	}
	
	virtual int Round(CWorm* w) {
		return w->getKills();
	}
	
	virtual void hitFlagSpawnPoint(CWorm* worm, Flag* flag) {
		if(getWormFlag(worm) == flag->id) {
			if(flag->holderWorm == worm->getID())
				cServer->flagInfo()->applySetBack(flag);
			else
				cServer->flagInfo()->applyHolderWorm(flag, worm->getID());
			
			nextGoals[getWormFlag(worm)] = (nextGoals[getWormFlag(worm)] + 1) % WPNUM;
			cServer->flagInfo()->applySpawnPos(flag, wayPoints[nextGoals[getWormFlag(worm)]]);
						
			if(nextGoals[getWormFlag(worm)] == 1) {
				// we made one round
				addScore(worm);
				cServer->RecheckGame();
			}
		}
	}
	
	virtual float FlagPointRadius() { return gameSettings[FT_Race_CheckPointRadius]; }
	virtual float FlagRadius() { return gameSettings[FT_Race_CheckPointRadius]; }
	
	virtual bool Shoot(CWorm* worm) {
		// get that information from client because both client&server can check this
		return cClient->getGameLobby()[FT_Race_AllowWeapons];
	}
	
	virtual bool CheckGameOver() {
		if(int(gameSettings[FT_Race_Rounds]) > 0) {
			for_each_iterator(CWorm*, w, game.worms()) {
				if(w->get()->getKills() >= int(gameSettings[FT_Race_Rounds])) {
					return true;
				}
			}
		}

		bool allOut = true;
		for_each_iterator(CWorm*, w, game.worms())
			if(w->get()->getLives() != WRM_OUT || w->get()->getAlive()) {
				allOut = false;
				break;
			}
		if(allOut) return true;
		
		// Check if the timelimit has been reached
		if(TimeLimit() > 0) {
			if (game.serverTime() > TimeLimit()) {
				if(networkTexts->sTimeLimit != "<none>")
					cServer->SendGlobalText(networkTexts->sTimeLimit, TXT_NORMAL);
				notes << "time limit (" << ((float)gameSettings[FT_TimeLimit]*60.0f) << ") reached with current time " << game.serverTime().seconds();
				notes << " -> game over" << endl;
				return true;
			}
		}
		
		return false;
	}

	struct CompareGoals {
		Race* race;
		CompareGoals(Race* r) : race(r) {}
		
		int comp(CWorm* w1, CWorm* w2) {
			// this is if we want to do the compare but the game has not started yet
			if(race->wayPoints.size() == 0) return 0;
			
			{
				// compare next goals
				int lastGoal1 = (race->getNextGoal(race->getWormFlag(w1)) + WPNUM - 1) % WPNUM;
				int lastGoal2 = (race->getNextGoal(race->getWormFlag(w2)) + WPNUM - 1) % WPNUM;
				if(lastGoal1 > lastGoal2) return 1;
				if(lastGoal1 < lastGoal2) return -1;
			}
			
			{
				// compare distance to next goals (they should be the same for both worms)
				float dist1 = (race->wayPoints[race->getNextGoal(race->getWormFlag(w1))] - w1->getPos()).GetLength2();
				float dist2 = (race->wayPoints[race->getNextGoal(race->getWormFlag(w2))] - w2->getPos()).GetLength2();
				long distdiff = (long) (dist2 - dist1);
				if(distdiff >= 1) return 1; // it's better to be more close
				if(distdiff <= -1) return -1;
			}
			
			return 0;			
		}
		
		bool operator()(CWorm* w1, CWorm* w2) {
			return comp(w1, w2) > 0;
		}
	};
	
	int CompareWormsScore(CWorm* w1, CWorm* w2) {
		// Kills very first (that is the amount of rounds)
		if (w1->getKills() > w2->getKills()) return 1;
		if (w1->getKills() < w2->getKills()) return -1;		

		{
			// compare goals; we can only do that if we are server, because we don't have the waypoints otherwise
			int d = (cServer && cServer->isServerRunning()) ? CompareGoals(this).comp(w1, w2) : 0;
			if(d != 0) return d;
		}
		
		// fallback (very unprobable that this will be used in game)
		return CGameMode::CompareWormsScore(w1, w2);
	}

	virtual bool HaveTargetPos(CWorm* w) { return true; }
	virtual CVec TargetPos(CWorm* w) {
		int flagId = getWormFlag(w);
		Flag* flag = cClient->flagInfo()->getFlag(flagId);
		if(flag) return flag->spawnPoint.pos;
		return CVec();
	}

};

struct TeamRace : public Race {
	
	virtual std::string Name() {
		return "Team Race";
	}
	
	virtual GameInfoGroup getGameInfoGroupInOptions() { return GIG_Race; }
	
	int teamScore[MAX_TEAMS];
	
	virtual int GameTeams() {
		return MAX_TEAMS;
	}

	void reset() {
		Race::reset();
		for(int i = 0; i < MAX_TEAMS; ++i)
			teamScore[i] = 0;		
	}

	TeamRace() { reset(); }
	
	virtual void PrepareGame() {
		Race::PrepareGame();
	}

	virtual void addScore(CWorm* w) {
		teamScore[CLAMP(w->getTeam(),0,MAX_TEAMS-1)]++;
		cServer->SendTeamScoreUpdate();
		// also add score for this specifc worm
		Race::addScore(w);
	}
	
	virtual int getWormFlag(CWorm* worm) {
		return worm->getTeam();
	}
	
	bool isTeamUsed(int t) {
		return !cServer->isTeamEmpty(t);
	}

	virtual std::string TeamNameForWorm(CWorm* w) {
		return TeamName(w->getTeam());
	}
	
	virtual int Round(CWorm* w) {
		return TeamScores(w->getTeam());
	}
	
	virtual int TeamScores(int t) {
		if(t >= 0 && t < MAX_TEAMS) return teamScore[t];
		return -1;
	}
	
	std::vector<CWorm*> getTeamWorms(int t) {
		std::vector<CWorm*> worms;
		for_each_iterator(CWorm*, w, game.worms())
			if(w->get()->getTeam() != t)
				worms.push_back(w->get());
		return worms;
	}
	
	CWorm* getTeamBestWorm(int t) {
		std::vector<CWorm*> teamWorms = getTeamWorms(t); 
		if(teamWorms.size() == 0) return NULL;
		std::sort(teamWorms.begin(), teamWorms.end(), CompareGoals(this));
		return teamWorms.front();
	}
	
	virtual int CompareTeamScore(int t1, int t2) {
		// team score is most important in CTF, more important than left lifes
		{
			int d = TeamScores(t1) - TeamScores(t2);
			if(d != 0) return d;
		}

		{
			// compare the goals of the best worms of each team
			CWorm* w1 = getTeamBestWorm(t1);
			CWorm* w2 = getTeamBestWorm(t2);
			
			int d = (w1 && w2) ? CompareGoals(this).comp(w1, w2) : 0;
			if(d != 0) return d;
		}
		
		// ok, fallback to default
		return CGameMode::CompareTeamsScore(t1, t2);
	}
	
	virtual bool CheckGameOver() {
		if(int(gameSettings[FT_Race_Rounds]) > 0) {
			for(int i = 0; i < MAX_TEAMS; ++i) {
				if(teamScore[i] >= int(gameSettings[FT_Race_Rounds])) {
					return true;
				}
			}
		}
		
		bool allOut = true;
		for_each_iterator(CWorm*, w, game.worms())
			if(w->get()->getLives() != WRM_OUT || w->get()->getAlive()) {
				allOut = false;
				break;
			}
		if(allOut) return true;
		
		// Check if the timelimit has been reached
		if(TimeLimit() > 0) {
			if (game.serverTime() > TimeLimit()) {
				if(networkTexts->sTimeLimit != "<none>")
					cServer->SendGlobalText(networkTexts->sTimeLimit, TXT_NORMAL);
				notes << "time limit (" << ((float)gameSettings[FT_TimeLimit]*60.0f) << ") reached with current time " << game.serverTime().seconds();
				notes << " -> game over" << endl;
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
	
	
	
};

static Race _gameMode_Race;
static TeamRace _gameMode_TeamRace;
CGameMode* gameMode_Race = &_gameMode_Race;
CGameMode* gameMode_TeamRace = &_gameMode_TeamRace;

