/*
 OpenLieroX
 
 gamemode interface
 
 created 2009-02-09
 code under LGPL
 */

#ifndef __CGAMEMODE_H__
#define __CGAMEMODE_H__

#include "CVec.h"
#include <string>

class CWorm;
class GameServer;
class CServerConnection;

class CGameMode {
protected:
	float	fGameStart;		// The time when the game was started

public:
	CGameMode() : fGameStart(0), cWorms(NULL), cServer(NULL) {}
	virtual ~CGameMode() {}

	virtual void PrepareGame() = 0;
	virtual void PrepareWorm(CWorm* worm) = 0;
	// If Spawn returns false then no spawn packet will be sent
	virtual bool Spawn(CWorm* worm, CVec pos) = 0;
	virtual void Kill(CWorm* victim, CWorm* killer) = 0;
	// If Shoot returns false then no shot will be fired
	virtual bool Shoot(CWorm* worm) = 0;
	virtual void Drop(CWorm* worm) = 0;
	virtual void Simulate() = 0;
	virtual bool CheckGame() = 0;
	virtual int  GameType() = 0; // this is the game type which is sent over network
	virtual int  GameTeams() = 0;
	virtual int  Winner() = 0;
	virtual bool NeedUpdate(CServerConnection* cl, CWorm* worm) = 0;
	// This is the game mode name as shown in the lobby for clients
	virtual std::string Name() = 0;

protected:
	CWorm*      cWorms;
	GameServer* cServer;
};

#endif

