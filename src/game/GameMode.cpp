/*
 *  GameMode.cpp
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 24.02.10.
 *  code under LGPL
 *
 */

#include "CGameMode.h"
#include "GameMode.h"

GameModeInfo::GameModeInfo() {
	mode = GameMode(GM_DEATHMATCH);
	generalGameType = mode->GeneralGameType();
	name = mode->Name();
}

bool GameModeInfo::operator==(const CustomVar& o) const {
	const GameModeInfo* oi = dynamic_cast<const GameModeInfo*> (&o);
	return oi && name == oi->name;
}

bool GameModeInfo::operator<(const CustomVar& o) const {
	const GameModeInfo* oi = dynamic_cast<const GameModeInfo*> (&o);
	if(oi) return name < oi->name;
	return this < &o;
}

std::string GameModeInfo::toString() const {
	return name;
}

bool GameModeInfo::fromString( const std::string & str) {
	generalGameType = 0;
	name = str;
	mode = GameMode(name);
	if(mode) generalGameType = mode->GeneralGameType();
	return true;
}
