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
	enum { MAXTEAMS = 4 } ;
	
	virtual void PrepareGame();
	virtual void Kill(CWorm* victim, CWorm* killer);
	virtual int  GameType() { return GMT_TEAMS; }
	virtual int  GameTeams() { return MAXTEAMS; }
	virtual int  Winner() {
		// There's no single winner so this will do for now
		return -1;	
	}
	virtual std::string Name() { return "Team Death Match"; }
	virtual int TeamScores(int t)
	{
		if(t >= 0 && t < MAXTEAMS) return teamScore[t];
		return -1;
	};

	void ChangeTeamScore(int t, int diff);
	
protected:
	int teamScore[MAXTEAMS];
};

void CTeamDeathMatch::PrepareGame()
{
	CGameMode::PrepareGame();
	for( int i = 0; i < MAXTEAMS; i++ )
		teamScore[i] = 0;
}

void CTeamDeathMatch::Kill(CWorm* victim, CWorm* killer)
{
	CGameMode::Kill(victim, killer);

	if( ! killer )
		return;

	if( killer == victim )
	{
		if( tLXOptions->tGameInfo.features[FT_SuicideDecreasesScore] )
			ChangeTeamScore( killer->getTeam(), -1 );
	}
	else if( killer->getTeam() == victim->getTeam() )
	{
		if( tLXOptions->tGameInfo.features[FT_TeamkillDecreasesScore] )
			ChangeTeamScore( killer->getTeam(), -1 );
		if( tLXOptions->tGameInfo.features[FT_CountTeamkills] )
			ChangeTeamScore( killer->getTeam(), 1 );
	}
	else
		ChangeTeamScore( killer->getTeam(), 1 );
	// Team score does not count FT_DeathDecreasesScore feature because it's float,
	// I thought of re-calculating teamscore by summing up worm scores
	// but if someone leaves he takes away part of teamscore then
}

void CTeamDeathMatch::ChangeTeamScore(int t, int diff) 
{
	if( t >= 0 && t < MAXTEAMS )
		teamScore[t] += diff;
	else
		errors << "CTeamDeathMatch::ChangeTeamScore: invalid team nr " << t << endl;
}

static CTeamDeathMatch gameMode;
CGameMode* gameMode_TeamDeathMatch = &gameMode;
