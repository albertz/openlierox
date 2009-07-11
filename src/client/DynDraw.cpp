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

struct SDLSurfDrawCrop : DynDrawIntf {
	SmartPointer<SDL_Surface> surf;
	int srcX, srcY;
	SDLSurfDrawCrop(const SmartPointer<SDL_Surface>& s, int x, int y, int W, int H) : 
		DynDrawIntf(MIN(W, s->w), MIN(H, s->h)), surf(s), srcX(x), srcY(y) {}
	
	virtual void draw(SDL_Surface* bmpDest, int x, int y) 
	{
		DrawImageAdv(bmpDest, surf.get(), srcX, srcY, x, y, w, h);
	}
};

SmartPointer<DynDrawIntf> DynDrawFromSurface(const SmartPointer<SDL_Surface>& surf) {
	if(surf.get()) return new SDLSurfDraw(surf);
	else return NULL;
}

SmartPointer<DynDrawIntf> DynDrawFromSurfaceCrop(const SmartPointer<SDL_Surface>& surf, int x, int y, int w, int h)
{
	if(surf.get()) return new SDLSurfDrawCrop(surf, x, y, w, h);
	return NULL;
}
