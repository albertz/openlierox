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
#include "CWorm.h"
#include "CBytestream.h"
#include "ClassInfo.h"
#include "ProfileSystem.h"

ScriptVar_t ObjectState::getValue(AttribRef a) const {
	Attribs::const_iterator it = attribs.find(a);
	if(it == attribs.end())
		return a.getAttrDesc()->defaultValue;
	return it->second.value;
}

GameStateUpdates::operator bool() const {
	if(!objs.empty()) return true;
	if(!objCreations.empty()) return true;
	if(!objDeletions.empty()) return true;
	return false;
}

void GameStateUpdates::writeToBs(CBytestream* bs) const {
	bs->writeInt16((uint16_t)objCreations.size());
	foreach(o, objCreations) {
		o->writeToBs(bs);
	}

	bs->writeInt16((uint16_t)objDeletions.size());
	foreach(o, objDeletions) {
		o->writeToBs(bs);
	}

	bs->writeInt((uint32_t)objs.size(), 4);
	foreach(a, objs) {
		a->writeToBs(bs);
		bs->writeVar(a->get());
	}
}

static BaseObject* getObjFromRef(ObjRef r) {
	switch(r.classId) {
	case LuaID<Game>::value: return &game;
	case LuaID<CWorm>::value: return game.wormById(r.objId);
	default: break;
	}
	return NULL;
}

static bool attrBelongsToClass(const ClassInfo* classInfo, const AttrDesc* attrDesc) {
	if(attrDesc->objTypeId == classInfo->id) return true;
	if(classInfo->superClassId != ClassId(-1)) {
		const ClassInfo* superClassInfo = getClassInfo(classInfo->superClassId);
		assert(superClassInfo != NULL); // if there is a superClassId, we always should have the ClassInfo
		return attrBelongsToClass(superClassInfo, attrDesc);
	}
	return false;
}

void GameStateUpdates::handleFromBs(CBytestream* bs) {
	uint16_t creationsNum = bs->readInt16();
	for(uint16_t i = 0; i < creationsNum; ++i) {
		ObjRef r;
		r.readFromBs(bs);
		//BaseObject* o = getClassInfo(r.classId)->createInstance();
		//o->thisRef.classId = r.classId;
		//o->thisRef.objId = r.objId;

		// we only handle/support CWorm objects for now...
		if(game.isServer()) {
			errors << "GameStateUpdates::handleFromBs: got obj creation as server" << endl;
			bs->SkipAll();
			return;
		}
		if(r.classId != LuaID<CWorm>::value) {
			errors << "GameStateUpdates::handleFromBs: obj-creation: invalid class-id " << r.classId << endl;
			bs->SkipAll();
			return;
		}
		if(game.wormById(r.objId, false) != NULL) {
			errors << "GameStateUpdates::handleFromBs: worm-creation: worm " << r.objId << " already exists" << endl;
			bs->SkipAll();
			return;
		}
		game.createNewWorm(r.objId, false, NULL, Version());
	}

	uint16_t deletionsNum = bs->readInt16();
	for(uint16_t i = 0; i < deletionsNum; ++i) {
		ObjRef r;
		r.readFromBs(bs);

		// we only handle/support CWorm objects for now...
		if(game.isServer()) {
			errors << "GameStateUpdates::handleFromBs: got obj deletion as server" << endl;
			bs->SkipAll();
			return;
		}
		if(r.classId != LuaID<CWorm>::value) {
			errors << "GameStateUpdates::handleFromBs: obj-deletion: invalid class-id " << r.classId << endl;
			bs->SkipAll();
			return;
		}
		CWorm* w = game.wormById(r.objId, false);
		if(!w) {
			errors << "GameStateUpdates::handleFromBs: obj-deletion: worm " << r.objId << " does not exist" << endl;
			bs->SkipAll();
			return;
		}
		game.removeWorm(w);
	}

	uint32_t attrUpdatesNum = bs->readInt(4);
	for(uint32_t i = 0; i < attrUpdatesNum; ++i) {
		ObjAttrRef r;
		r.readFromBs(bs);
		ScriptVar_t v;
		bs->readVar(v);

		const AttrDesc* attrDesc = r.attr.getAttrDesc();
		if(attrDesc == NULL) {
			errors << "GameStateUpdates::handleFromBs: AttrDesc for update not found" << endl;
			bs->SkipAll();
			return;
		}

		const ClassInfo* classInfo = getClassInfo(r.obj.classId);
		if(classInfo == NULL) {
			errors << "GameStateUpdates::handleFromBs: class " << r.obj.classId << " for obj-update unknown" << endl;
			bs->SkipAll();
			return;
		}

		if(!attrBelongsToClass(classInfo, attrDesc)) {
			errors << "GameStateUpdates::handleFromBs: attr " << attrDesc->description() << " does not belong to class " << r.obj.classId << " for obj-update" << endl;
			bs->SkipAll();
			return;
		}

		// for now, this is somewhat specific to the only types we support
		if(r.obj.classId == LuaID<Settings>::value) {
			if(game.isServer()) {
				errors << "GameStateUpdates::handleFromBs: got settings update as server" << endl;
				bs->SkipAll();
				return;
			}
			if(!Settings::getAttrDescs().belongsToUs(attrDesc)) {
				errors << "GameStateUpdates::handleFromBs: settings update AttrDesc " << attrDesc->description() << " is not a setting attr" << endl;
				bs->SkipAll();
				return;
			}
			FeatureIndex fIndex = Settings::getAttrDescs().getIndex(attrDesc);
			cClient->getGameLobby().overwrite[fIndex] = v;
		}
		else {
			BaseObject* o = getObjFromRef(r.obj);
			if(o == NULL) {
				errors << "GameStateUpdates::handleFromBs: object for attr update not found" << endl;
				bs->SkipAll();
				return;
			}
			if(o->thisRef != r.obj) {
				errors << "GameStateUpdates::handleFromBs: object-ref for attr update invalid" << endl;
				bs->SkipAll();
				return;
			}
			ScriptVarPtr_t p = attrDesc->getValueScriptPtr(o);
			p.fromScriptVar(v);
		}

		notes << "game state update: <" << r.obj.description() << "> " << attrDesc->attrName << " to " << v.toString() << endl;
	}
}

void GameStateUpdates::pushObjAttrUpdate(ObjAttrRef a) {
	objs.insert(a);
}

void GameStateUpdates::pushObjCreation(ObjRef o) {
	objDeletions.erase(o);
	objCreations.insert(o);
}

void GameStateUpdates::pushObjDeletion(ObjRef o) {
	Objs::iterator itStart = objs.lower_bound(ObjAttrRef::LowerLimit(o));
	Objs::iterator itEnd = objs.upper_bound(ObjAttrRef::UpperLimit(o));
	objs.erase(itStart, itEnd);
	objCreations.erase(o);
	objDeletions.insert(o);
}

void GameStateUpdates::reset() {
	objs.clear();
	objCreations.clear();
	objDeletions.clear();
}

void GameStateUpdates::diffFromStateToCurrent(const GameState& s) {
	reset();
	for_each_iterator(CWorm*, w, game.worms()) {
		ObjRef o = w->get()->thisRef;
		if(!s.haveObject(o))
			pushObjCreation(o);
		Objs& curObjs = game.gameStateUpdates->objs;
		Objs::iterator itStart = curObjs.lower_bound(ObjAttrRef::LowerLimit(o));
		Objs::iterator itEnd = curObjs.upper_bound(ObjAttrRef::UpperLimit(o));
		for(Objs::iterator it = itStart; it != itEnd; ) {
			Objs::iterator next = it; next++;
			ScriptVar_t curValue = it->get();
			ScriptVar_t stateValue = it->attr.getAttrDesc()->defaultValue;
			if(s.haveObject(it->obj))
				stateValue = s.getValue(*it);
			if(curValue != stateValue)
				pushObjAttrUpdate(*it);
			it = next;
		}
	}
}

static ObjectState& GameState_registerObj(GameState& s, BaseObject* obj) {
	return s.objs[obj->thisRef] = ObjectState(obj);
}

GameState::GameState() {
	// register singletons which are always there
	GameState_registerObj(*this, &game);
	GameState_registerObj(*this, &gameSettings);
}

GameState GameState::Current() {
	GameState s;
	s.updateToCurrent();
	return s;
}

void GameState::reset() {
	*this = GameState();
}

void GameState::updateToCurrent() {
	reset();
	for_each_iterator(CWorm*, w, game.worms()) {
		GameState_registerObj(*this, w->get());
	}
}

void GameState::addObject(ObjRef o) {
	assert(!haveObject(o));
	objs[o] = ObjectState(o);
}

bool GameState::haveObject(ObjRef o) const {
	Objs::const_iterator it = objs.find(o);
	return it != objs.end();
}

ScriptVar_t GameState::getValue(ObjAttrRef a) const {
	Objs::const_iterator it = objs.find(a.obj);
	assert(it != objs.end());
	assert(it->second.obj == a.obj);
	return it->second.getValue(a.attr);
}

