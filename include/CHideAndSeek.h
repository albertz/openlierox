/*
 OpenLieroX
 
 hide and seek gamemode
 
 created 2009-02-09
 code under LGPL
 */

#ifndef __CHIDEANDSEEK_H__
#define __CHIDEANDSEEK_H__

#include "CGameMode.h"
#include "Consts.h"
#include "types.h"

class CHideAndSeek : public CGameMode {
public:
	virtual void PrepareGame();
	virtual void PrepareWorm(CWorm* worm);
	virtual bool Spawn(CWorm* worm, CVec pos);
	virtual void Kill(CWorm* victim, CWorm* killer);
	virtual bool Shoot(CWorm* worm);
	virtual void Drop(CWorm* worm);
	virtual void Simulate();
	virtual bool CheckGameOver();
	virtual int  GeneralGameType();
	virtual int  GameTeams();
	virtual int  Winner();
	virtual bool NeedUpdate(CServerConnection* cl, CWorm* worm);
	virtual std::string Name() { return "Hide and Seek"; }
	// Show or hide a worm to/from the opposing team
	void Show(CWorm* worm, bool message = true);
	void Hide(CWorm* worm, bool message = true);
	// Returns true if worm1 can see worm2
	bool CanSee(CWorm* worm1, CWorm* worm2);
	// Generates an approximation of the time needed to finish the game
	void GenerateTimes();

	enum  Team { HIDER = 0, SEEKER = 1 };
	
protected:
	float fGameLength;           // The length of the game
	TimeDiff fWarmupTime[MAX_WORMS]; // The time for which worm should be invisible and untouchable
	TimeDiff fLastAlert[MAX_WORMS]; // The last time the worms were seen by other worms
	bool  bVisible[MAX_WORMS];   // The visibility of the woms
};

#endif

