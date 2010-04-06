/*
 *  raytracing.cpp
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 06.04.10.
 *  code under GPL
 *
 */

#include "raytracing.h"

// basically, this is CClient::DrawViewport_Game backwards
GamePixelInfo getGamePixelInfo(int x, int y) {
	GamePixelInfo info;
	
	
	return info;
}

Color getGamePixelColor(int x, int y) {
	return getGamePixelInfo(x, y).color;
}
