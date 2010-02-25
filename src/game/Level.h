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
struct SDL_Surface;

struct LevelInfo : CustomVar {
	LevelInfo() : valid(false) {}
	LevelInfo(const std::string& _path) : valid(true), path(_path) {}
	bool valid;
	std::string name;
	std::string path;
	std::string type;
	std::string typeShort;
	
	virtual CustomVar* copy() const { return new LevelInfo(*this); }
	virtual bool operator==(const CustomVar&) const;
	virtual bool operator<(const CustomVar&) const;
	virtual std::string toString() const;
	virtual bool fromString( const std::string & str);	
};

LevelInfo infoForLevel(const std::string& f, bool absolute = false);

SmartPointer<SDL_Surface> minimapForLevel(const std::string& f, bool absolute = false);

#endif

