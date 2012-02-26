/*
 *  GameState.cpp
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 15.02.12.
 *  code under LGPL
 *
 */

#include "GameState.h"
#include "Game.h"
#include "Settings.h"

GameState::GameState() {
	// register singletons which are always there
	registerObj(&game);
	registerObj(&gameSettings);
}

ObjectState& GameState::registerObj(BaseObject* obj) {
	return objs[obj->thisRef] = ObjectState(obj);
}
