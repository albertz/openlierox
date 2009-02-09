#ifndef __CHIDEANDSEEK_H__
#define __CHIDEANDSEEK_H__

#include "CGameMode.h"
#include "Consts.h"

class CHideAndSeek : public CGameMode {
public:
	CHideAndSeek(GameServer* server, CWorm* worms);
	virtual ~CHideAndSeek();
	
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
	enum team { HIDER, SEEKER };
	int  iWinner;
};

#endif

