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

struct GameState;
class CBytestream;

struct AttribState {
	ScriptVar_t value;
};

struct ObjectState {
	typedef std::map<AttribRef, AttribState> Attribs;

	ObjRef obj;
	Attribs attribs;

	ObjectState() {}
	ObjectState(ObjRef obj_) : obj(obj_) {}
	ObjectState(BaseObject* obj_) : obj(obj_->thisRef) {}
	ScriptVar_t getValue(AttribRef) const;
};

struct GameStateUpdates {
	typedef std::set<ObjAttrRef> Objs;

	std::set<ObjRef> objDeletions;
	std::set<ObjRef> objCreations;
	Objs objs;

	operator bool() const;
	void writeToBs(CBytestream* bs) const;
	static void handleFromBs(CBytestream* bs);
	void pushObjAttrUpdate(ObjAttrRef);
	void pushObjCreation(ObjRef);
	void pushObjDeletion(ObjRef);
	void reset();
	void diffFromStateToCurrent(const GameState& s);
};

struct GameState {
	typedef std::map<ObjRef, ObjectState> Objs;

	Objs objs;

	GameState();
	static GameState Current();
	void reset();
	void updateToCurrent();
	void addObject(ObjRef);

	bool haveObject(ObjRef) const;
	ScriptVar_t getValue(ObjAttrRef) const;
};


#endif // OLX_GAMESTATE_H
