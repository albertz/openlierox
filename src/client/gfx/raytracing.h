/*
 *  raytracing.h
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 06.04.10.
 *  code under GPL
 *
 */

#ifndef __OLX_RAYTRACING_H__
#define __OLX_RAYTRACING_H__

#include "Color.h"
#include "util/vec.h"

class CGameObject;

struct GamePixelInfo {
	enum {
		GPI_Material,
		GPI_Object
	} type;
	
	struct ObjectInfo {
		CGameObject* obj;
		int relX, relY;
	};
	
	typedef unsigned char MaterialIndex;
	
	union {
		ObjectInfo object;
		MaterialIndex material;
	};
	
	Color color;
};

GamePixelInfo getGamePixelInfo(int x, int y);
Color getGamePixelColor(int x, int y);

#endif
