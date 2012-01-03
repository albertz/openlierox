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
#include "Consts.h"

class CGameMode;

class GameModeInfo : public CustomVar {
private:
	CGameMode* m_mode;
	int m_generalGameType;
	std::string m_name;
	
public:	
	GameModeInfo();
	static GameModeInfo fromNetworkModeInt(int m);
	
	struct ModeWrapper {
		ModeWrapper& operator=(CGameMode* m);
		operator CGameMode*() const;
		CGameMode* operator->() const { return (CGameMode*)(*this); }
	} mode;
	
	struct GenTypeWrapper {
		GenTypeWrapper& operator=(int t);
		operator int() const;
	} generalGameType;
	
	struct NameWrapper {
		NameWrapper& operator=(const std::string& n);
		operator std::string() const;
	} name;
	
	GameModeInfo withNewName(const std::string& n) const;
	GameModeIndex actualIndex(GameModeIndex fallback = (GameModeIndex)-1 /*undefined*/) const; // GameMode(actualIndex) can differ from m_mode. use this for bots or so
	
// CustomVar interface	
	virtual CustomVar* copy() const { return new GameModeInfo(*this); }
	virtual bool operator==(const CustomVar&) const;
	virtual bool operator<(const CustomVar&) const;
	virtual std::string toString() const;
	virtual bool fromString( const std::string & str);	
};

#endif
