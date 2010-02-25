/*
 *  GameMode.h
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 24.02.10.
 *  code under LGPL
 *
 */

#ifndef __OLX_GAME_GAMEMODE_H__
#define __OLX_GAME_GAMEMODE_H__

#include <string>
#include "util/CustomVar.h"

class CGameMode;

struct GameModeInfo : CustomVar {
	GameModeInfo();
	CGameMode* mode;
	int generalGameType;
	std::string name;

	virtual CustomVar* copy() const { return new GameModeInfo(*this); }
	virtual bool operator==(const CustomVar&) const;
	virtual bool operator<(const CustomVar&) const;
	virtual std::string toString() const;
	virtual bool fromString( const std::string & str);	
};

#endif
