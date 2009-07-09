/*
 *  DynDraw.h
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 03.06.09.
 *  coder under LGPL
 *
 */

#ifndef __OLX__DYNDRAW_H__
#define __OLX__DYNDRAW_H__

#include "SmartPointer.h"

struct SDL_Surface;

struct DynDrawIntf {
	const int w, h;
	DynDrawIntf(int _w, int _h) : w(_w), h(_h) {}
	virtual ~DynDrawIntf() {}
	virtual void draw(SDL_Surface* bmpDest, int x, int y) = 0;
};

SmartPointer<DynDrawIntf> DynDrawFromSurface(const SmartPointer<SDL_Surface>& surf);
SmartPointer<DynDrawIntf> DynDrawFromSurfaceCrop(const SmartPointer<SDL_Surface>& surf, int x, int y, int w, int h);

#endif
