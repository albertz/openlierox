/*
 *  CaptureTheFlag.cpp
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 20.03.09.
 *  code under LGPL
 *
 */

#include "CGameMode.h"
#include "CServer.h"
#include "CWorm.h"
#include "FlagInfo.h"
#include "gusanos/gusgame.h"

struct CaptureTheFlag : public CGameMode {
	
	virtual std::string Name() {
		return "Capture The Flag";
	}

	virtual GameInfoGroup getGameInfoGroupInOptions() { return GIG_CaptureTheFlag; }

	virtual Version MinNeededVersion() { return OLXBetaVersion(0,58,1); }
	
	static const int MAXTEAMS = 4;
	int teamScore[MAXTEAMS];
	
	virtual int GameTeams() {
		return MAXTEAMS;
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

	CaptureTheFlag() { reset(); }
	
	void reset() {
		for(int i = 0; i < MAXTEAMS; ++i)
			teamScore[i] = 0;		
	}
	
	bool isTeamUsed(int t) {
		return !cServer->isTeamEmpty(t);
	}
	
	void initFlag(int t, const CVec& pos) {
		// currently, just use this easy method to find a spot for the flag
		CVec spawnPoint = cServer->getMap()->groundPos(pos) - CVec(0, (float)(cServer->flagInfo()->getHeight()/4));
		cServer->flagInfo()->applyInitFlag(t, spawnPoint);
	}
	
	std::string flagName(int t) {
		return TeamName(t) + " flag";
	}
	
	virtual void PrepareGame() {
		CGameMode::PrepareGame();
		reset();
	}
	
	void wormCatchFlag_Handler(CWorm* worm, Flag* flag) {
		if(!(bool)tLXOptions->tGameInfo.features[FT_CTF_AllowRopeForCarrier])
			cServer->SetWormCanUseNinja(worm->getID(), false);
		cServer->SetWormSpeedFactor(worm->getID(), tLXOptions->tGameInfo.features[FT_CTF_SpeedFactorForCarrier]);
	}
	
	void wormLooseFlag_Handler(CWorm* worm, Flag* flag) {
		if(!(bool)tLXOptions->tGameInfo.features[FT_CTF_AllowRopeForCarrier])
			cServer->SetWormCanUseNinja(worm->getID(), true);
		cServer->SetWormSpeedFactor(worm->getID(), tLXOptions->tGameInfo.features[FT_WormSpeedFactor]);		
	}
	
	void wormDropFlag(CWorm* worm) {
		Flag* flag = cServer->flagInfo()->getFlagOfWorm(worm->getID());
		if(flag) {
			wormLooseFlag_Handler(worm, flag);
			CVec pos = cServer->getMap()->groundPos(worm->getPos()) - CVec(0,(float)(cServer->flagInfo()->getHeight()/4));
			cServer->flagInfo()->applyCustomPos(flag, pos);
			cServer->SendGlobalText(worm->getName() + " lost " + flagName(flag->id), TXT_NORMAL);			
		}		
	}
	
	CVec getTeamBasePos(int team, CVec fallback) {
		for(size_t i = 0; i < gusGame.level().config()->teamBases.size(); ++i) {
			if(gusGame.level().config()->teamBases[i].team == team + 1) // Gus team numbers
				return CVec(gusGame.level().config()->teamBases[i].pos);
		}
		
		return fallback;
	}
	
	virtual bool Spawn(CWorm* worm, CVec pos) {
		if(!CGameMode::Spawn(worm, pos)) return false;
		
		if(!cServer->flagInfo()->getFlag(worm->getTeam())) {
			// we have to create the new flag, there isn't any yet
			initFlag(worm->getTeam(), getTeamBasePos(worm->getTeam(), pos));
		}
		return true;
	}
	
	virtual void Kill(CWorm* victim, CWorm* killer) {
		wormDropFlag(victim);
		
		if(killer && killer != victim)
			killer->addKill();
		
		// Victim is out of the game
		if(victim->Kill() && networkTexts->sPlayerOut != "<none>")
			cServer->SendGlobalText(replacemax(networkTexts->sPlayerOut, "<player>",
											   victim->getName(), 1), TXT_NORMAL);
	}
	
	virtual void Drop(CWorm* worm) {
		wormDropFlag(worm);
	}	
	
	virtual void hitFlag(CWorm* worm, Flag* flag) {
		if(flag->id == worm->getTeam()) { // own flag
			if(!flag->atSpawnPoint && flag->holderWorm < 0) {
				cServer->flagInfo()->applySetBack(flag);
				cServer->SendGlobalText(worm->getName() + " returned " + flagName(flag->id), TXT_NORMAL);
			}
		}
		else { // enemy flag
			cServer->flagInfo()->applyHolderWorm(flag, worm->getID());
			cServer->SendGlobalText(worm->getName() + " caught " + flagName(flag->id), TXT_NORMAL);
			wormCatchFlag_Handler(worm, flag);
		}
	}
	
	virtual void hitFlagSpawnPoint(CWorm* worm, Flag* flag) {
		if(worm->getTeam() == flag->id) { // own base
			Flag* wormsFlag = cServer->flagInfo()->getFlagOfWorm(worm->getID());
			if(wormsFlag && flag->atSpawnPoint) {
				// yay, we scored!
				teamScore[worm->getTeam()]++;
				wormLooseFlag_Handler(worm, wormsFlag);
				cServer->flagInfo()->applySetBack(wormsFlag);
				cServer->SendGlobalText(worm->getName() + " scored for " + TeamName(worm->getTeam()), TXT_NORMAL);
				cServer->SendTeamScoreUpdate();
				cServer->RecheckGame();
			}
		}
	}	
	
	
};

static CaptureTheFlag gameMode;
CGameMode* gameMode_CaptureTheFlag = &gameMode;


