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
//	AttrDesc::AttrId attrId;
	ScriptVar_t value;
};

struct AttribStates {
//	AttrDesc::ObjTypeId objTypeId;
	std::map<AttrDesc::AttrId, AttribState> attribs;
};

struct ObjectState {
	std::map<AttrDesc::ObjTypeId, AttribStates> attribs;
	AttribState& attrib(AttrDesc::ObjTypeId objTypeId, AttrDesc::AttrId attrId) {
		AttribStates& attrsByType = attribs[objTypeId];
		AttribState& attrState = attrsByType.attribs[attrId];
		return attrState;
	}
};

struct GameStateUpdateRef {
	AttribRef attribRef;
	std::string engineSetting;
	enum Type { T_Attrib, T_Setting } type;
};

struct GameState
{
	EngineSettings settings;
	std::vector<ObjectState> objects;
};

#endif // OLX_GAMESTATE_H
