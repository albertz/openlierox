/*
 *  CRace.cpp
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 10.05.09.
 *  code under LGPL
 *
 */

#include "CGameMode.h"
#include "CServer.h"
#include "CWorm.h"
#include "FlagInfo.h"
#include "CServerConnection.h"
#include "CServerNetEngine.h"

struct Race : public CGameMode {
	
	virtual std::string Name() {
		return "Race";
	}
	
	virtual GameInfoGroup getGameInfoGroupInOptions() { return GIG_Race; }
	
	virtual void reset() {
		wayPoints.clear();
		wormSpawnPoints.clear();
		nextGoals.clear();
	}
	
	Race() { reset(); }
	
	std::vector<CVec> wayPoints;
	
	void initFlag(int t, const CVec& pos) {
		// currently, just use this easy method to find a spot for the flag
		CVec spawnPoint = cServer->getMap()->groundPos(pos) - CVec(0, (float)(cServer->flagInfo()->getHeight()/4));
		cServer->flagInfo()->applyInitFlag(t, spawnPoint);
	}
	
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
		
		wayPoints.resize(4);
		for(int x = 0; x <= 1; x++)
			for(int y = 0; y <= 1; y++) {
				std::list<CVec> goodPos;
				goodPos.push_back(CVec(
								  (cServer->getMap()->GetWidth() * 0.8 * x + cServer->getMap()->GetWidth() * 0.2),
								  (cServer->getMap()->GetHeight() * 0.8 * y + cServer->getMap()->GetHeight() * 0.2)));
				std::list<CVec> badPos;
				badPos.push_back(CVec(
								  (cServer->getMap()->GetWidth() * 0.2 * x + cServer->getMap()->GetWidth() * 0.8),
								  (cServer->getMap()->GetHeight() * 0.2 * y + cServer->getMap()->GetHeight() * 0.8)));
				int t = (y == 0) ? x : (3 - x);
				wayPoints[t] = cServer->FindSpotCloseToPos(goodPos, badPos, false);
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
			initFlag(getWormFlag(worm), wayPoints[1]);
			nextGoals[getWormFlag(worm)] = 1;
		}
		
		worm->Spawn(pos);
		
		return true;
	}
	
	virtual void Kill(CWorm* victim, CWorm* killer) {
		// Victim is out of the game
		if(victim->Kill() && networkTexts->sPlayerOut != "<none>")
			cServer->SendGlobalText(replacemax(networkTexts->sPlayerOut, "<player>",
											   victim->getName(), 1), TXT_NORMAL);
		
		wormSpawnPoints[victim->getID()] = victim->getPos();
	}
	
	virtual void addScore(CWorm* w) {
		w->AddKill();
		sendWormScoreUpdate(w);
	}
	
	virtual void hitFlagSpawnPoint(CWorm* worm, Flag* flag) {
		if(getWormFlag(worm) == flag->id) { // own base			
			nextGoals[getWormFlag(worm)] = (nextGoals[getWormFlag(worm)] + 1) % 4;
			cServer->flagInfo()->applySpawnPos(flag, wayPoints[nextGoals[getWormFlag(worm)]]);

			if(nextGoals[getWormFlag(worm)] == 1) {
				// we made one round
				addScore(worm);
				cServer->RecheckGame();
			}
		}
	}
	
	
};

struct TeamRace : public Race {
	
	virtual std::string Name() {
		return "Team Race";
	}
	
	virtual GameInfoGroup getGameInfoGroupInOptions() { return GIG_Race; }
	
	static const int MAXTEAMS = 4;
	int teamScore[MAXTEAMS];
	
	virtual int GameTeams() {
		return MAXTEAMS;
	}

	void reset() {
		Race::reset();
		for(int i = 0; i < MAXTEAMS; ++i)
			teamScore[i] = 0;		
	}

	TeamRace() { reset(); }
	
	virtual void PrepareGame() {
		Race::PrepareGame();
		
	}

	virtual void addScore(CWorm* w) {
		teamScore[w->getTeam()]++;
		cServer->SendTeamScoreUpdate();
		//cServer->SendGlobalText(worm->getName() + " scored for " + TeamName(worm->getTeam()), TXT_NORMAL);
	}
	
	virtual int getWormFlag(CWorm* worm) {
		return worm->getTeam();
	}
	
	bool isTeamUsed(int t) {
		return !cServer->isTeamEmpty(t);
	}
	
	virtual int TeamScores(int t) {
		if(t >= 0 && t < MAXTEAMS) return teamScore[t];
		return -1;
	}
	
	virtual int CompareTeamScore(int t1, int t2) {
		// team score is most important in CTF, more important than left lifes
		{
			int d = TeamScores(t1) - TeamScores(t2);
			if(d != 0) return d;
		}
		
		// ok, fallback to default
		return CGameMode::CompareTeamsScore(t1, t2);
	}
	
	virtual bool CheckGameOver() {
		// currently we use killlimit as teamscorelimit
		if(tLXOptions->tGameInfo.iKillLimit >= 0) {
			for(int i = 0; i < MAXTEAMS; ++i) {
				if(teamScore[i] >= tLXOptions->tGameInfo.iKillLimit) {
					return true;
				}
			}
		}
		
		bool allOut = true;
		for(int i = 0; i < MAX_WORMS; i++)
			if(cServer->getWorms()[i].isUsed()) {
				if(cServer->getWorms()[i].getLives() != WRM_OUT || cServer->getWorms()[i].getAlive()) {
					allOut = false;
					break;
				}
			}
		if(allOut) return true;
		
		// Check if the timelimit has been reached
		if(TimeLimit() > 0) {
			if (cServer->getServerTime() > TimeLimit()) {
				if(networkTexts->sTimeLimit != "<none>")
					cServer->SendGlobalText(networkTexts->sTimeLimit, TXT_NORMAL);
				notes << "time limit (" << (tLXOptions->tGameInfo.fTimeLimit*60.0f) << ") reached with current time " << cServer->getServerTime().seconds();
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

