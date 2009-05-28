/*
 OpenLieroX
 
 team death match gamemode
 
 created 2009-02-09
 code under LGPL
 */

#include <iostream>
#include <string>
#include "CWorm.h"
#include "Options.h"
#include "Consts.h"
#include "CServer.h"
#include "CClient.h"
#include "CGameMode.h"
#include "Consts.h"

class CTeamDeathMatch : public CGameMode {
public:
	
	virtual void Drop(CWorm* worm);
	virtual int  GameType() { return GMT_TEAMS; }
	virtual int  GameTeams() { return MAX_TEAMS; }
	virtual int  Winner() {
		// There's no single winner so this will do for now
		return -1;	
	}
	virtual std::string Name() { return "Team Death Match"; }
	
protected:
};

void CTeamDeathMatch::Drop(CWorm* worm)
{
	// TODO: why is this needed? Where is this used? CGameMode::Drop() is empty - move this code there.
	if (!worm || worm->getID() < 0 || worm->getID() >= MAX_WORMS) {
		errors << "Dropped an invalid worm" << endl;
		return;
	}
	
	iKillsInRow[worm->getID()] = 0;
	iDeathsInRow[worm->getID()] = 0;
}


static CTeamDeathMatch gameMode;
CGameMode* gameMode_TeamDeathMatch = &gameMode;
