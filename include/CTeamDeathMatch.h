#ifndef __CTEAMDEATHMATCH_H__
#define __CTEAMDEATHMATCH_H__

#include "CGameMode.h"
#include "Consts.h"

class CTeamDeathMatch : public CGameMode {
public:
	CTeamDeathMatch(GameServer* server, CWorm* worms);
	virtual ~CTeamDeathMatch();
	
	virtual void PrepareGame();
	virtual void PrepareWorm(CWorm* worm);
	virtual bool Spawn(CWorm* worm, CVec pos);
	virtual void Kill(CWorm* victim, CWorm* killer);
	virtual bool Shoot(CWorm* worm);
	virtual void Drop(CWorm* worm);
	virtual void Simulate();
	virtual bool CheckGame();
	virtual int  GameType();
	virtual int  GameTeams();
	virtual int  Winner();

protected:
	bool bFirstBlood;
	int  iKillsInRow[MAX_WORMS];
	int  iDeathsInRow[MAX_WORMS];
};

#endif

