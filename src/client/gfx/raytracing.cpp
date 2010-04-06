/*
 *  raytracing.cpp
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 06.04.10.
 *  code under GPL
 *
 */

#include "raytracing.h"

GamePixelInfo getGamePixelInfo(int x, int y) {
	
}

Color getGamePixelColor(int x, int y) {
	return getGamePixelInfo(x, y).color;
}
