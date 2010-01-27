/*
 *  Level.cpp
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 08.12.09.
 *  code under LGPL
 *
 */

#include "Level.h"
#include "MapLoader.h"
#include "StringUtils.h"

LevelInfo infoForLevel(const std::string& f, bool absolute) {
	MapLoad* loader = MapLoad::open(absolute ? f : ("levels/" + f), absolute, false);
	if(!loader) return LevelInfo();

	LevelInfo info;
	info.valid = true;
	info.name = loader->header().name;
	info.path = GetBaseFilename(f);
	info.type = loader->format();
	info.typeShort = loader->formatShort();
	
	delete loader;
	
	return info;
}
