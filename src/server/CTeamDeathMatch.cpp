/*
 OpenLieroX
 
 team death match gamemode
 
 created 2009-02-09
 code under LGPL
 */

#include <iostream>
#include <string>
#include "game/CWorm.h"
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
	virtual int  GameType() { return GMT_TEAMS; }
	virtual int  GameTeams() { return MAX_TEAMS; }
	virtual int  Winner() {
		// There's no single winner so this will do for now
		return -1;	
	}
	virtual std::string Name() { return "Team Death Match"; }
	virtual int TeamScores(int t)
	{
		if(t >= 0 && t < MAX_TEAMS) return teamScore[t];
		return -1;
	};

	void ChangeTeamScore(int t, int diff);
	
protected:
	int teamScore[MAX_TEAMS];
};

void CTeamDeathMatch::PrepareGame()
{
	CGameMode::PrepareGame();
	for( int i = 0; i < MAX_TEAMS; i++ )
		teamScore[i] = 0;
}

void CTeamDeathMatch::Kill(CWorm* victim, CWorm* killer)
{
	int oldVictimScore = victim->getScore();
	int oldKillerScore = 0;
	if(killer)
		oldKillerScore = killer->getScore();
		
	CGameMode::Kill(victim, killer);

	ChangeTeamScore( victim->getTeam(), victim->getScore() - oldVictimScore );
	if( killer )
		ChangeTeamScore( killer->getTeam(), killer->getScore() - oldKillerScore );
}

void CTeamDeathMatch::ChangeTeamScore(int t, int diff) 
{
	if( t >= 0 && t < MAX_TEAMS )
		teamScore[t] += diff;
	else
		errors << "CTeamDeathMatch::ChangeTeamScore: invalid team nr " << t << endl;
}

static CTeamDeathMatch gameMode;
CGameMode* gameMode_TeamDeathMatch = &gameMode;
