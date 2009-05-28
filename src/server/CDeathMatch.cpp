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
	virtual void Drop(CWorm* worm);
	virtual std::string Name() { return "Death Match"; }
};


void CDeathMatch::Drop(CWorm* worm)
{
	// TODO: why is this needed? Where is this used? CGameMode::Drop() is empty stub - move this code there.
	if (!worm || worm->getID() < 0 || worm->getID() >= MAX_WORMS)
		errors << "Dropped an invalid worm" << endl;

	iKillsInRow[worm->getID()] = 0;
	iDeathsInRow[worm->getID()] = 0;
}



static CDeathMatch gameMode;
CGameMode* gameMode_DeathMatch = &gameMode;
