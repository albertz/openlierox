/*
 *  Level.cpp
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 08.12.09.
 *  code under LGPL
 *
 */

#include <memory>
#include "Level.h"
#include "MapLoader.h"
#include "StringUtils.h"
#include "GfxPrimitives.h"

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

SmartPointer<SDL_Surface> minimapForLevel(const std::string& f, bool absolute) {
	LevelInfo info = infoForLevel(f, absolute);
	
	std::auto_ptr<MapLoad> loader( MapLoad::open(absolute ? f : ("levels/" + f), absolute, false) );
	if(!loader.get()) {
		SmartPointer<SDL_Surface> minimap = gfxCreateSurface(128,96);
		DrawCross(minimap.get(), 0, 0, 128, 96, Color(0,0,255));
		return minimap;
	}
	
	return loader->getMinimap();
}
