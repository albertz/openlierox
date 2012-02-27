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

