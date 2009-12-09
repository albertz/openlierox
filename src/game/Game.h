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

class CWormHumanInputHandler;
class CWormInputHandler;

class Game {
public:
	void prepareGameloop();
	void frameOuter();
	void frameInner();
	void cleanupAfterGameloopEnd();
	
	std::vector<CWormHumanInputHandler*> localPlayers;
	std::list<CWormInputHandler*> players;
	
private:
	AbsTime oldtime;
};

extern Game game;

#endif
