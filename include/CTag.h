/*
 OpenLieroX
 
 team death match gamemode
 
 created 2009-02-14
 code under LGPL
 */

#ifndef __CTAG_H__
#define __CTAG_H__

#include "CGameMode.h"
#include "Consts.h"

class CTag : public CGameMode {
private:
	bool CompareWormsScore(CWorm *w1, CWorm *w2);
protected:
	virtual void TagWorm(CWorm *worm);
	virtual void TagRandomWorm();
public:
	CTag(GameServer* server, CWorm* worms);
	virtual ~CTag();
	
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
	virtual bool NeedUpdate(CServerConnection* cl, CWorm* worm);
	virtual std::string Name() { return "Tag"; }

protected:
	bool bFirstBlood;
	int  iKillsInRow[MAX_WORMS];
	int  iDeathsInRow[MAX_WORMS];
	int  iWinner;
};

#endif  // __CTAG_H__

