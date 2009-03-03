/*
	OpenLieroX

	death match gamemode

	created 2009-02-09
	code under LGPL
*/

#ifndef __CDEATHMATCH_H__
#define __CDEATHMATCH_H__

#include "CGameMode.h"
#include "Consts.h"

class CDeathMatch : public CGameMode {
public:
	virtual void PrepareWorm(CWorm* worm);
	virtual bool Shoot(CWorm* worm);
	virtual void Drop(CWorm* worm);
	virtual void Simulate();
	virtual bool CheckGameOver();
	virtual bool NeedUpdate(CServerConnection* cl, CWorm* worm);
	virtual std::string Name() { return "Death Match"; }
};

#endif

