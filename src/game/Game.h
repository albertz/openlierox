/*
 *  Game.h
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 09.12.09.
 *  code under LGPL
 *
 */

#ifndef __OLX_GAME_H__
#define __OLX_GAME_H__

#include <vector>
#include <list>

#include "types.h"
#include "gusanos/object_grid.h"

class CWormHumanInputHandler;
class CWormInputHandler;
class CWorm;
class CGameScript;

class Game {
public:
	void prepareGameloop();
	void frameOuter();
	void frameInner();
	void cleanupAfterGameloopEnd();
	
	void reset();
	
	void onNewWorm(CWorm* w);
	void onRemoveWorm(CWorm* w);
	void onNewPlayer(CWormInputHandler*);
	void onNewPlayer_Lua(CWormInputHandler*);
	void onNewHumanPlayer(CWormHumanInputHandler*);
	void onNewHumanPlayer_Lua(CWormHumanInputHandler*);
	
	std::vector<CWormHumanInputHandler*> localPlayers;
	std::list<CWormInputHandler*> players;
	
#ifdef USE_GRID
	Grid objects;
#else
	ObjectsList objects;
#endif

	CGameScript* gameScript();
	
private:
	AbsTime oldtime;
};

extern Game game;

#endif
