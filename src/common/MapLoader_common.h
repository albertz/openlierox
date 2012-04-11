/*
 *  MapLoader_common.h
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 11.4.12.
 *  code under LGPL
 *
 */

#ifndef __OLX__MAPLOADER_COMMON_H__
#define __OLX__MAPLOADER_COMMON_H__

#include "MapLoader.h"
#include "GfxPrimitives.h"

struct ParseFinalizer {
	MapLoad* mapLoad;
	CMap* map;
	ParseFinalizer(MapLoad* _load, CMap* _map) : mapLoad(_load), map(_map) {}
	~ParseFinalizer() {
		mapLoad->parseDataFinalize(map);
	}
};

inline static void setMinimapErrorGraphic(SmartPointer<SDL_Surface>& minimap) {
	minimap = gfxCreateSurface(128,96);
	DrawCross(minimap.get(), 0, 0, 128, 96, Color(255,0,0));	
}

#endif
