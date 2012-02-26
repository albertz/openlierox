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
#include "util/BaseObject.h"
#include "game/ClassInfo.h"

struct AttribState {
	ScriptVar_t value;
};

struct ObjectState {
	ObjectState() {}
	ObjectState(BaseObject* obj_) : obj(obj_->thisRef) {}
	ObjRef obj;
	std::map<AttribRef, AttribState> attribs;
};

struct GameStateUpdates {
	std::vector<ObjRef> objDeletions;
	std::map<ObjRef, ClassId> objCreations;
	std::set<ObjAttrRef> objs;
};

struct GameState {
	GameState();
	ObjectState& registerObj(BaseObject* obj);
	std::map<ObjRef, ObjectState> objs;
};

#endif // OLX_GAMESTATE_H
