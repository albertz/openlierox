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
	ObjectState() : objId(-1) {}
	ObjectState(BaseObject* obj_) : objId(obj_->uniqueObjId), obj(obj_->thisWeakRef) {}
	BaseObject::ObjId objId;
	BaseObject::WeakRef obj;
	std::map<AttribRef, AttribState> attribs;
};

struct GameStateUpdates {
	std::vector<BaseObject::ObjId> objDeletions;
	std::map<BaseObject::ObjId, ClassId> objCreations;
	std::set<ObjAttrRef> objs;
};

struct GameState {
	GameState();
	ObjectState& registerObj(BaseObject* obj);
	std::map<BaseObject::ObjId, ObjectState> objs;
};

#endif // OLX_GAMESTATE_H
