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
#include <cmath>
#include <cstdlib>
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


// This class is basically for collision checks (used in projectile simulation).
// Everything should be kept so simple that basically everything here will be optimised out.
template<typename T>
struct Shape {
	enum Type { ST_RECT, ST_CIRCLE } type;
	VectorD2<T> pos;
	VectorD2<T> radius;
	Shape() : type(ST_RECT) {}
	static Shape Circle(const VectorD2<T>& p, const VectorD2<T>& r) { Shape ret; ret.type = ST_CIRCLE; ret.pos = p; ret.radius = r; return ret; }
	static Shape Rect(const VectorD2<T>& p, const VectorD2<T>& r) { Shape ret; ret.type = ST_RECT; ret.pos = p; ret.radius = r; return ret; }	

	bool CollisionWith(const Shape& s) const {
		if(type == ST_RECT && s.type == ST_RECT) {
			bool overlapX = std::abs(pos.x - s.pos.x) < radius.x + s.radius.x;
			bool overlapY = std::abs(pos.y - s.pos.y) < radius.y + s.radius.y;
			return overlapX && overlapY;
		} else if(type == ST_RECT && s.type == ST_CIRCLE) {
			VectorD2<int> nearest;
			if(pos.x + radius.x <= s.pos.x)
				nearest.x = pos.x + radius.x;
			else if(pos.x - radius.x <= s.pos.x)
				nearest.x = s.pos.x;				
			else
				nearest.x = pos.x - radius.x;
			if(pos.y + radius.y <= s.pos.y)
				nearest.y = pos.y + radius.y;
			else if(pos.y - radius.y <= s.pos.y)
				nearest.y = s.pos.y;
			else
				nearest.y = pos.y - radius.y;
			return (nearest - s.pos).GetLength2() < s.radius.GetLength2();
		} else if(type == ST_CIRCLE && s.type == ST_RECT) {
			return s.CollisionWith(*this);
		} else { // both are circles
			return (pos - s.pos).GetLength2() < (radius + s.radius).GetLength2();
		}
	}
};


#endif
