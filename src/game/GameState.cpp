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
	case LuaID<Game>::value:
		assert(r.objId == 1);
		return &game;
	case LuaID<CWorm>::value:
		return game.wormById(r.objId);
	default:
		assert(false);
	}
	return NULL;
}

void GameStateUpdates::handleFromBs(CBytestream* bs) {
	// WARNING/TODO: We have many asserts here, so this will crash
	// for unsafe data. Also, unsafe data isn't really handled
	// well anyway. This has to be fixed in future releases.

	uint16_t creationsNum = bs->readInt16();
	for(uint16_t i = 0; i < creationsNum; ++i) {
		ObjRef r;
		r.readFromBs(bs);
		//BaseObject* o = getClassInfo(r.classId)->createInstance();
		//o->thisRef.classId = r.classId;
		//o->thisRef.objId = r.objId;

		// we only handle/support CWorm objects for now...
		assert(game.isClient());
		assert(r.classId == LuaID<CWorm>::value);
		game.createNewWorm(r.objId, false, NULL, Version());
	}

	uint16_t deletionsNum = bs->readInt16();
	for(uint16_t i = 0; i < deletionsNum; ++i) {
		ObjRef r;
		r.readFromBs(bs);

		// we only handle/support CWorm objects for now...
		assert(game.isClient());
		assert(r.classId == LuaID<CWorm>::value);
		CWorm* w = game.wormById(r.objId);
		game.removeWorm(w);
	}

	uint32_t attrUpdatesNum = bs->readInt(4);
	for(uint32_t i = 0; i < attrUpdatesNum; ++i) {
		ObjAttrRef r;
		r.readFromBs(bs);
		ScriptVar_t v;
		bs->readVar(v);

		// for now, this is somewhat specific to the only types we support
		if(r.obj.classId == LuaID<Settings>::value) {
			assert(game.isClient());
			FeatureIndex fIndex = Settings::getAttrDescs().getIndex(r.attr.getAttrDesc());
			cClient->getGameLobby().overwrite[fIndex] = v;
		}
		else {
			BaseObject* o = getObjFromRef(r.obj);
			assert(o != NULL);
			assert(o->thisRef == r.obj);
			ScriptVarPtr_t p = r.attr.getAttrDesc()->getValueScriptPtr(o);
			p.fromScriptVar(v);
		}
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

