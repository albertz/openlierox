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
#include "SinglePlayer.h"
#include "util/macros.h"

GameModeInfo::GameModeInfo() {
	m_mode = GameMode(GM_DEATHMATCH);
	m_generalGameType = 0;
	m_name = "Death Match";
}

bool GameModeInfo::operator==(const CustomVar& o) const {
	const GameModeInfo* oi = dynamic_cast<const GameModeInfo*> (&o);
	return oi && m_name == oi->m_name;
}

bool GameModeInfo::operator<(const CustomVar& o) const {
	const GameModeInfo* oi = dynamic_cast<const GameModeInfo*> (&o);
	if(oi) return m_name < oi->m_name;
	return this < &o;
}

std::string GameModeInfo::toString() const {
	if(!m_name.empty()) return m_name;	
	return guessGeneralGameTypeName(m_generalGameType);
}

bool GameModeInfo::fromString( const std::string & str) {
	m_generalGameType = 0;
	m_name = "";
	
	if(str == "") {
		m_mode = NULL;
		return true;
	}
	
	bool fail = false;
	int num = from_string<int>(str, fail);
	if(!fail) {
		// interpret num as gamemode index
		m_mode = GameMode((GameModeIndex)num);
		if(m_mode) m_name = m_mode->Name();
	}
	else {
		// interpret str as name
		m_name = str;
		m_mode = GameMode(m_name);
	}
	
	if(m_mode) m_generalGameType = m_mode->GeneralGameType();
	return true;
}

GameModeInfo::ModeWrapper& GameModeInfo::ModeWrapper::operator=(CGameMode* m) {
	GameModeInfo* info = __OLX_BASETHIS(GameModeInfo, mode);	
	info->m_mode = m;
	if(m) info->m_generalGameType = m->GeneralGameType();
	if(m) info->m_name = m->Name();
	return *this;
}

GameModeInfo::ModeWrapper::operator CGameMode*() const {
	GameModeInfo* info = __OLX_BASETHIS(GameModeInfo, mode);
	return info->m_mode;
}

GameModeInfo::GenTypeWrapper& GameModeInfo::GenTypeWrapper::operator=(int t) {
	GameModeInfo* info = __OLX_BASETHIS(GameModeInfo, generalGameType);
	info->m_generalGameType = t;
	return *this;
}

GameModeInfo::GenTypeWrapper::operator int() const {
	GameModeInfo* info = __OLX_BASETHIS(GameModeInfo, generalGameType);
	return info->m_generalGameType;
}

GameModeInfo::NameWrapper& GameModeInfo::NameWrapper::operator=(const std::string& str) {
	GameModeInfo* info = __OLX_BASETHIS(GameModeInfo, name);
	
	// NOTE: The difference to fromString is that we don't reset generalgametype/name in case we fail here.
	// Also, we interpret an empty string as the request to reset (gamemodeindex=0).
	
	bool fail = false;
	int num = (str == "") ? 0 : from_string<int>(str, fail);
	if(!fail) {
		// interpret num as gamemode index
		info->m_mode = GameMode((GameModeIndex)num);
		if(info->m_mode) info->m_name = info->m_mode->Name();
	}
	else {
		// interpret str as name
		info->m_name = str;
		info->m_mode = GameMode(info->m_name);
	}
	
	if(info->m_mode) info->m_generalGameType = info->m_mode->GeneralGameType();
	
	return *this;
}

GameModeInfo::NameWrapper::operator std::string() const {
	GameModeInfo* info = __OLX_BASETHIS(GameModeInfo, name);
	return info->toString();
}

GameModeInfo GameModeInfo::fromNetworkModeInt(int m) {
	GameModeInfo info;
	info.generalGameType = m;
	if( info.generalGameType > GMT_MAX || info.generalGameType < 0 )
		info.generalGameType = GMT_NORMAL;
	info.mode = NULL;
	info.name = guessGeneralGameTypeName(info.generalGameType);
	return info;
}

GameModeInfo GameModeInfo::withNewName(const std::string& n) const {
	GameModeInfo info;
	info.name = n;
	return info;
}

GameModeIndex GameModeInfo::actualIndex(GameModeIndex fallback) const {
	if(m_mode == &singlePlayerGame)
		return GetGameModeIndex(singlePlayerGame.standardGameMode, fallback);
	return GetGameModeIndex(m_mode, fallback);
}
