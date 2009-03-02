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
	virtual bool CheckGameOver();
	virtual int  GameType();
	virtual int  GameTeams();
	virtual int  Winner();
	virtual bool NeedUpdate(CServerConnection* cl, CWorm* worm);
	virtual std::string Name() { return GAMEMODE_NAME; }
	// Show or hide a worm to/from the opposing team
	void Show(CWorm* worm, bool message = true);
	void Hide(CWorm* worm, bool message = true);
	// Returns true if worm1 can see worm2
	bool CanSee(CWorm* worm1, CWorm* worm2);
	// Generates an approximation of the time needed to finish the game
	void GenerateTimes();

	static const char * GAMEMODE_NAME;

protected:
	enum  team { HIDER, SEEKER };
	float fGameLength;           // The length of the game
	float fWarmupTime[MAX_WORMS]; // The time for which worm should be invisible and untouchable
	float fLastAlert[MAX_WORMS]; // The last time the worms were seen by other worms
	bool  bVisible[MAX_WORMS];   // The visibility of the woms
};

#endif

