/*
 OpenLieroX
 
 death match gamemode
 
 created 2009-02-09
 code under LGPL
 */

#include <iostream>
#include "CWorm.h"
#include "Options.h"
#include "Consts.h"
#include "CServer.h"
#include "CClient.h"
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



void CDeathMatch::PrepareWorm(CWorm* worm)
{
}


bool CDeathMatch::Shoot(CWorm* worm)
{
	return true;
}


void CDeathMatch::Drop(CWorm* worm)
{
	if (!worm || worm->getID() < 0 || worm->getID() >= MAX_WORMS)
		errors << "Dropped an invalid worm" << endl;

	iKillsInRow[worm->getID()] = 0;
	iDeathsInRow[worm->getID()] = 0;
}

void CDeathMatch::Simulate()
{
}

bool CDeathMatch::CheckGameOver()
{
	return CGameMode::CheckGameOver();
}

bool CDeathMatch::NeedUpdate(CServerConnection* cl, CWorm* worm)
{
	return true;
}

static CDeathMatch gameMode;
CGameMode* gameMode_DeathMatch = &gameMode;
