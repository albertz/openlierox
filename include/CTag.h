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
protected:
	virtual void TagWorm(CWorm *worm);
	virtual void TagRandomWorm();
public:
	virtual ~CTag();
	
	virtual void PrepareGame();
	virtual void PrepareWorm(CWorm* worm);
	virtual bool Spawn(CWorm* worm, CVec pos);
	virtual void Kill(CWorm* victim, CWorm* killer);
	virtual bool Shoot(CWorm* worm);
	virtual void Drop(CWorm* worm);
	virtual void Simulate();
	virtual bool CheckGameOver();
	virtual int  GeneralGameType();
	virtual int CompareWormsScore(CWorm *w1, CWorm *w2);
	virtual int  Winner();
	virtual bool NeedUpdate(CServerConnection* cl, CWorm* worm);
	virtual std::string Name() { return "Tag"; }

protected:
	bool bFirstBlood;
	int  iKillsInRow[MAX_WORMS];
	int  iDeathsInRow[MAX_WORMS];
};

#endif  // __CTAG_H__

