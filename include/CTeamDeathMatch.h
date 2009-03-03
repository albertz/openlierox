/*
 OpenLieroX
 
 team death match gamemode
 
 created 2009-02-09
 code under LGPL
 */

#ifndef __CTEAMDEATHMATCH_H__
#define __CTEAMDEATHMATCH_H__

#include "CGameMode.h"
#include "Consts.h"

class CTeamDeathMatch : public CGameMode {
public:
	virtual ~CTeamDeathMatch();
	
	virtual void PrepareGame();
	virtual void PrepareWorm(CWorm* worm);
	virtual bool Spawn(CWorm* worm, CVec pos);
	virtual void Kill(CWorm* victim, CWorm* killer);
	virtual bool Shoot(CWorm* worm);
	virtual void Drop(CWorm* worm);
	virtual void Simulate();
	virtual bool CheckGameOver();
	virtual int  GameType();
	virtual int  GameTeams();
	virtual int  Winner();
	virtual bool NeedUpdate(CServerConnection* cl, CWorm* worm);
	virtual std::string Name() { return "Team Death Match"; }

protected:
	bool bFirstBlood;
	int  iKillsInRow[MAX_WORMS];
	int  iDeathsInRow[MAX_WORMS];
};

#endif

