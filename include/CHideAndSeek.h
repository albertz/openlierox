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
	virtual bool CheckGame();
	virtual int  GameType();
	virtual int  GameTeams();
	virtual int  Winner();
	virtual bool NeedUpdate(CServerConnection* cl, CWorm* worm);
	// Show or hide a worm to/from the opposing team
	void Show(CWorm* worm);
	void Hide(CWorm* worm);
	// Returns true if worm1 can see worm2
	bool CanSee(CWorm* worm1, CWorm* worm2);
	// Generates an approximation of the time needed to finish the game
	void GenerateTimes();

protected:
	enum  team { HIDER, SEEKER };
	float fHideLength;           // The length of time the hiders cannot be caught in
	float fGameLength;           // The length of the game
	float fAlertLength;          // The length of time a worm must not be seen for to go invisible
	float fLastAlert[MAX_WORMS]; // The last time the worms were seen by other worms
	bool  bVisible[MAX_WORMS];   // The visibility of the woms
};

#endif

