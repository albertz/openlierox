/*
 *  Geometry.h
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 09.04.09.
 *  code under LGPL
 *
 */

#ifndef __OLX__GEOMETRY_H__
#define __OLX__GEOMETRY_H__

#include <list>
#include <vector>
#include <SDL.h>
#include "CVec.h"


struct Line {
	VectorD2<int> start;
	VectorD2<int> end;
	
	Line() {}
	Line(VectorD2<int> s, VectorD2<int> e) : start(s), end(e) {}
	
	bool isRightFrom(int x, int y) const;
	bool isParallel(int x, int y) const;
	bool isBeforeStart(int x, int y) const;
	bool isAfterEnd(int x, int y) const;
	bool containsY(int y, int& x, bool aimsDown) const;
};

class CViewport;

struct Polygon2D {
	typedef std::list< VectorD2<int> > Points;
	Points points;
	typedef std::vector< Line > Lines;
	Lines lines;
	bool doReloadLines;
	
	Polygon2D() : doReloadLines(true) {}
	void clear() { points.clear(); lines.clear(); doReloadLines = true; }
	void reloadLines();
	bool getNext(int x, int y, int& nextx, bool inside) const;
	SDL_Rect minOverlayRect() const;
	SDL_Rect minOverlayRect(CViewport* v) const;
	void drawFilled(SDL_Surface* s, int x, int y, Color col);
	void drawFilled(SDL_Surface* s, int x, int y, CViewport* v, Color col); // interpret as ingame coordinates and use viewport
};

void TestPolygonDrawing(SDL_Surface* s);
void TestCircleDrawing(SDL_Surface* s);


#endif
