/*
 *  GameState.h
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 15.02.12.
 *  code under LGPL
 *
 */

#ifndef OLX_GAMESTATE_H
#define OLX_GAMESTATE_H

#include <vector>
#include <map>
#include "EngineSettings.h"
#include "CScriptableVars.h"
#include "Attr.h"

struct AttribState {
	ScriptVar_t value;
};

struct ObjectState {
	std::map<AttribRef, AttribState> attribs;
};

struct GameStateUpdates {
	std::set<ObjAttrRef> objs;
};

struct GameState
{
	std::vector<ObjectState> objects;
};

#endif // OLX_GAMESTATE_H
