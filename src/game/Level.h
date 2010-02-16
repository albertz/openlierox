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
#include "SmartPointer.h"
struct SDL_Surface;

struct LevelInfo {
	bool valid;
	std::string name;
	std::string path;
	std::string type;
	std::string typeShort;
	LevelInfo() : valid(false) {}
};

LevelInfo infoForLevel(const std::string& f, bool absolute = false);

SmartPointer<SDL_Surface> minimapForLevel(const std::string& f, bool absolute = false);

#endif

