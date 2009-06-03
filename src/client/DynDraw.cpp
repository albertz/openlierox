/*
 *  DynDraw.cpp
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 03.06.09.
 *  code under LGPL
 *
 */

#include "DynDraw.h"
#include "GfxPrimitives.h"

struct SDLSurfDraw : DynDrawIntf {
	SmartPointer<SDL_Surface> surf;
	SDLSurfDraw(const SmartPointer<SDL_Surface>& s) : DynDrawIntf(s->w, s->h), surf(s) {}
	
	virtual void draw(SDL_Surface* bmpDest, int x, int y) {
		DrawImage(bmpDest, surf.get(), x, y);
	}
};

SmartPointer<DynDrawIntf> DynDrawFromSurface(const SmartPointer<SDL_Surface>& surf) {
	if(surf.get()) return new SDLSurfDraw(surf);
	else return NULL;
}
