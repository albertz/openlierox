/*
 OpenLieroX
 
 death match gamemode
 
 created 2009-02-09
 code under LGPL
 */

#include <iostream>
#include "game/CWorm.h"
#include "Options.h"
#include "Consts.h"
#include "CServer.h"
#include "CClient.h"
#include "CGameMode.h"
#include "Consts.h"

class CDeathMatch : public CGameMode {
public:
	virtual std::string Name() { return "Death Match"; }
};


static CDeathMatch gameMode;
CGameMode* gameMode_DeathMatch = &gameMode;
