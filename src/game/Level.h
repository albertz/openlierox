/*
 *  Level.h
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 08.12.09.
 *  code under LGPL
 *
 */

#ifndef __OLX_LEVEL_H__
#define __OLX_LEVEL_H__

#include <string>
#include "util/CustomVar.h"
#include "SmartPointer.h"
#include "game/Attr.h"

struct SDL_Surface;

struct LevelInfo : CustomVar {
	LevelInfo();
	static LevelInfo ByPath(const std::string& _p) { LevelInfo info; info.valid = true; info.path = _p; return info; }
	bool valid;
	ATTR(LevelInfo, std::string, name, 1, {})
	ATTR(LevelInfo, std::string, path, 2, {})
	ATTR(LevelInfo, std::string, typeShort, 3, {})
	std::string type;
	
	virtual CustomVar* copy() const { return new LevelInfo(*this); }
	virtual bool operator==(const CustomVar&) const;
	virtual bool operator<(const CustomVar&) const;
	virtual std::string toString() const;
	virtual bool fromString( const std::string & str);	
};

LevelInfo infoForLevel(const std::string& f, bool absolute = false);

SmartPointer<SDL_Surface> minimapForLevel(const std::string& f, bool absolute = false);

#endif

