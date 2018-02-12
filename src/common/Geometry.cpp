/*
 *  Geometry.cpp
 *  OpenLieroX
 *
 *  Created by Karel Petranek on 11.06.09.
 *  code under LGPL
 *
 */

#include <algorithm>
#include "Geometry.h"
#include "GfxPrimitives.h"
#include "CViewport.h"

//
// Line
//

///////////////////
// Returns true if the given point is in the right from the line or if it is at the line
bool Line::isRightFrom(int x, int y) const {
	VectorD2<int> rel = end - start;
	x -= start.x; y -= start.y;
	return rel.y * x <= y * rel.x;
}

////////////////////
// Returns true if the point is at the line before the starting point
bool Line::isBeforeStart(int x, int y) const {
	VectorD2<int> rel = end - start;
	x -= start.x; y -= start.y;
	return rel.x * x + rel.y * y < 0;
}

////////////////////
// Returns true if the point is at the line after the ending point
bool Line::isAfterEnd(int x, int y) const {
	VectorD2<int> rel = start - end;
	x -= end.x; y -= end.y;
	return rel.x * x + rel.y * y < 0;	
}

bool Line::isParallel(int x, int y) const {
	return !isBeforeStart(x,y) && !isAfterEnd(x,y);
}

bool Line::containsY(int y, int& x, bool aimsDown) const {
	if(start.y == end.y) {
		if(start.y == y) {
			if(!aimsDown) {
				if(start.x < end.x) x = start.x;
				else x = end.x;
			}
			else {
				if(start.x < end.x) x = end.x;
				else x = start.x;				
			}
			return true;
		}
		return false;
	}
	
	if(start.y <= y && end.y >= y && aimsDown) {
		VectorD2<int> rel = end - start;
		y -= start.y;
		x = rel.x * y / rel.y;
		x += start.x;
		return true;
	}
	
	if(start.y >= y && end.y <= y && !aimsDown) {
		VectorD2<int> rel = start - end;
		y -= end.y;
		x = rel.x * y / rel.y;
		x += end.x;
		return true;
	}
	
	return false;	
}

///////////////////
// Returns true if the line is horizontal (dy = 0)
bool Line::isHorizontal() const
{
	return start.y == end.y;
}

//////////////////
// Returns true if the line intersects another line
bool Line::intersects(const Line& l) const
{
	// Get the analytical intersection and check that it is located on one of the lines

	// ax + by + c = 0
	const VectorD2<int> v1 = (end - start).orthogonal(); // a = v1.x, b = v1.y
	const VectorD2<int> v2 = (l.end - l.start).orthogonal();
	const int c1 = -v1.x * start.x - v1.y * start.y;
	const int c2 = -v2.x * l.start.x - v2.y * l.start.y;

	// Use determinants
	// | b1 c1 | 
	// | b2 c2 |
	// --------- = Xp
	// | a1 b1 |
	// | a2 b2 |
	//
	// | c1 a1 | 
	// | c2 a2 |
	// --------- = Yp
	// | a1 b1 |
	// | a2 b2 |
	const int denom = v1.x * v2.y - v2.x * v1.y;
	if (!denom)  // Parallel
		return (float)v1.x / v2.x == (float)c1/c2;  // Check for identity
	const int xp_numer = v1.y * c2 - v2.y * c1;
	const int yp_numer = c1 * v2.x - c2 * v1.x;

	const int xp = xp_numer / denom;
	const int yp = yp_numer / denom;

	int res = abs(end.y - yp) + abs(start.y - yp) - abs(end.y - start.y);
	res += abs(l.end.y - yp) + abs(l.start.y - yp) - abs(l.end.y - l.start.y);
	res += abs(end.x - xp) + abs(start.x - xp) - abs(end.x - start.x);
	res += abs(l.end.x - xp) + abs(l.start.x - xp) - abs(l.end.x - l.start.x);

	return res == 0;
}

///////////////////
// Squared distance of the line from the given point
float Line::distFromPoint2(const VectorD2<int> &pt) const
{
	VectorD2<int> d = pt - start;
	VectorD2<int> e = end - start;
	int e2 = e.GetLength2();
	int ed = e.x * d.x + e.y * d.y;
	float t = CLAMP((float)ed/e2, 0.0f, 1.0f);  // For points that are before/after end of the line we return distance to the ending points

	CVec closest(start.x + t * e.x, start.y + t * e.y);
	return SQR(closest.x - pt.x) + SQR(closest.y - pt.y);
}

///////////////////
// Distance of the line from the given point
float Line::distFromPoint(const VectorD2<int> &pt) const
{
	return sqrtf(distFromPoint2(pt));
}




//
// Polygon2D class
//

Polygon2D::Polygon2D(const Points& pts)
{
	// Add the points one by one
	// We have to do this because the internal representation may vary from what we get
	startPointAdding();
	for (Points::const_iterator it = pts.begin(); it != pts.end(); ++it)
		addPoint(*it);
	endPointAdding();
}

///////////////////
// Updates the convex flag, private
// The three points are last two points added to the polygon
void Polygon2D::checkConvex(const VectorD2<int>& pt1, const VectorD2<int>& pt2, const VectorD2<int>& pt3)
{
	if (!convex)
		return;

	// Get the vertex sign
	// TODO: why does this not work?
	//int sign = (pt1 - pt2).Scalar(pt3 - pt2);  //(pt2.x - pt1.x)*(pt3.y - pt2.y) - (pt2.y - pt1.y) * (pt3.x - pt1.x);
	int sign = (pt1 - pt2).Cross(pt3 - pt2); // TODO: works only for simple polygons
	convex = (lastsign * sign >= 0);
	lastsign = sign;
}

///////////////////
// Add a point to the polygon, startPointAdding() must be called before!
void Polygon2D::addPoint(const VectorD2<int>& pt)
{
	assert(addingPoints);
	points.push_back(pt);
	if (points.size() > 2)  {
		Points::reverse_iterator p1 = points.rbegin();
		p1++; p1++;
		Points::reverse_iterator p2 = points.rbegin();
		p2++;
		checkConvex(*p1, *p2, pt);
	}
}

///////////////////
// Denotes start of polygon initialization (point adding)
void Polygon2D::startPointAdding()
{
	addingPoints = true;
}

///////////////////
// Denotes end of polygon initialization (point adding)
void Polygon2D::endPointAdding()
{
	assert(addingPoints);

	// Build and pre-process the polygon
	if (points.size() < 3)
		return;

	int minx = points.begin()->x;
	int miny = points.begin()->y;
	int maxx = minx;
	int maxy = miny;

	// Create the lines
	Points::iterator it1 = points.begin();
	Points::iterator it2 = points.begin();
	++it2;
	for (; it2 != points.end(); ++it2, ++it1)  {
		// Update min and max for X
		minx = MIN(minx, MIN(it2->x, it1->x));
		maxx = MAX(maxx, MAX(it2->x, it1->x));

		// Make sure it always goes from top to bottom
		if (it2->y < it1->y)  {
			lines.push_back(Line(*it2, *it1));
			miny = MIN(miny, it2->y);
			maxy = MAX(maxy, it1->y);
		} else if (it2->y == it1->y)  {
			horizLines.push_back(Line(*it1, *it2));  // Horizontal lines are treated special
			miny = MIN(miny, it1->y);
			maxy = MAX(maxy, it1->y);
		} else {
			lines.push_back(Line(*it1, *it2));
			miny = MIN(miny, it1->y);
			maxy = MAX(maxy, it2->y);
		}
		
	}

	// Calculate the overlay rect
	overlay.x = minx;
	overlay.y = miny;
	overlay.w = maxx - minx;
	overlay.h = maxy - miny;

	// Close the polygon if necessary
	if (*points.begin() != *points.rbegin())  {
		if (points.begin()->y < points.rbegin()->y)
			lines.push_back(Line(*points.begin(), *points.rbegin()));
		else if (points.begin()->y == points.rbegin()->y)
			horizLines.push_back(Line(*points.begin(), *points.rbegin()));
		else
			lines.push_back(Line(*points.rbegin(), *points.begin()));

		Points::reverse_iterator p1 = points.rbegin();
		p1++;
		checkConvex(*p1, *points.rbegin(), *points.begin());
	}

	addingPoints = false;
}

SDL_Rect Polygon2D::minOverlayRect() const {
	assert(!addingPoints);
	return overlay;
}

SDL_Rect Polygon2D::minOverlayRect(CViewport* v) const {
	assert(!addingPoints);
	const int wx = v->GetWorldX();
	const int wy = v->GetWorldY();
	const int l = v->GetLeft();
	const int t = v->GetTop();

#define Tx(x) ((x - wx) * 2 + l)
#define Ty(y) ((y - wy) * 2 + t)
	
	SDL_Rect r = { (SDLRect::Type) Tx(overlay.x), (SDLRect::Type) Ty(overlay.y), (SDLRect::TypeS) (overlay.w * 2), (SDLRect::TypeS) (overlay.h * 2) };

#undef Tx
#undef Ty	
	return r;
}

bool Polygon2D::isInside(int x, int y) const
{
	// Check the overlay rect first
	if (x < overlay.x || x >= overlay.x + overlay.w || y < overlay.y || y >= overlay.y + overlay.h)
		return false;

	// Run one scanline in the level of the point and check that the point is inside
	std::vector<int> isc; isc.reserve(lines.size());
	for (Lines::const_iterator it = lines.begin(); it != lines.end(); ++it)  {
		if (it->start.y <= y && y < it->end.y)  {
			const float slope = (float)(it->start.x - it->end.x) / (it->start.y - it->end.y);
			isc.push_back( (int)(slope * (y - it->start.y)) + it->start.x ); // Calculate the intersection
		}
	}

	std::sort(isc.begin(), isc.end());
	for (unsigned i = 0; i < isc.size(); i += 2)
		if (isc[i] <= x && x <= isc[i + 1])  {
			return true;
		}

	// Check horizontal lines
	for (Lines::const_iterator it = horizLines.begin(); it != horizLines.end(); ++it)
		if (y == it->start.y && it->start.x <= x && x <= it->end.x)
			return true;

	return false;
}

bool Polygon2D::intersectsConvex(const Polygon2D& poly) const
{
	// Collision check using the Separating Axis Theorem

	// TODO...

	// Horizontal lines
	/*for (Lines::iterator it = horizLines.begin(); it != horizLines.end(); it++)  {
		int minx = poly.horizLines.begin()->start.x;
		int maxx = minx;
		for (Lines::iterator it = poly.horizLines.begin(); it != poly.horizLines.end(); it++)  {
			if (it->start.x < minx)
				minx = it->start.x;
			if (it->start.x > maxx)
				maxx = it->start.x;
			if (it->end.x > maxx)
				maxx = it->start.x;
			if (it->end.x < minx)
				minx = it->start.y;
		}
	}*/
	return false;
}

bool Polygon2D::intersects(const Polygon2D& poly) const
{

	/*if (convex && poly.convex)  {
		return intersectsConvex(poly);
	}*/

	// First check the overlay rect
	SDL_Rect tmp = overlay;
	if (!ClipRefRectWith(tmp.x, tmp.y, tmp.w, tmp.h, (SDLRect)poly.overlay))
		return false;

	// Check for line intersection
	for (Lines::const_iterator it1 = lines.begin(); it1 != lines.end(); ++it1)  {
		for (Lines::const_iterator it2 = poly.lines.begin(); it2 != poly.lines.end(); ++it2)
			if (it1->intersects(*it2))
				return true;
		for (Lines::const_iterator it2 = poly.horizLines.begin(); it2 != poly.horizLines.end(); ++it2)
			if (it1->intersects(*it2))
				return true;
	}

	// Horizontal lines
	for (Lines::const_iterator it1 = horizLines.begin(); it1 != horizLines.end(); ++it1)  {
		for (Lines::const_iterator it2 = poly.lines.begin(); it2 != poly.lines.end(); ++it2)
			if (it1->intersects(*it2))
				return true;
		for (Lines::const_iterator it2 = poly.horizLines.begin(); it2 != poly.horizLines.end(); ++it2)  {
			if (it1->start.y == it2->start.y && it1->intersects(*it2))
				return true;
		}
	}

	// Can happen when the whole polygon is inside the other one
	return isInside(poly.points.begin()->x, poly.points.begin()->y) 
		|| poly.isInside(points.begin()->x, points.begin()->y);
}

/////////////////////
// Checks for rect intersection
bool Polygon2D::intersectsRect(const SDL_Rect &r) const
{
	// First check the overlay rect
	SDL_Rect tmp = overlay;
	if (!ClipRefRectWith(tmp.x, tmp.y, tmp.w, tmp.h, (SDLRect)r))
		return false;

	Line l1(VectorD2<int>(r.x, r.y), VectorD2<int>(r.x + r.w, r.y));
	Line l2(VectorD2<int>(r.x + r.w, r.y), VectorD2<int>(r.x + r.w, r.y + r.h));
	Line l3(VectorD2<int>(r.x + r.w, r.y + r.h), VectorD2<int>(r.x, r.y + r.h));
	Line l4(VectorD2<int>(r.x, r.y + r.h), VectorD2<int>(r.x, r.y));

	for (Lines::const_iterator it = lines.begin(); it != lines.end(); it++)  {
		if (it->intersects(l1) || it->intersects(l2) || it->intersects(l3) || it->intersects(l4))
			return true;
	}

	for (Lines::const_iterator it = horizLines.begin(); it != horizLines.end(); ++it)  {
		if (it->intersects(l1) || it->intersects(l2) || it->intersects(l3) || it->intersects(l4))
			return true;		
	}

	return false;
}

//////////////////
// Checks for circle intersection
bool Polygon2D::intersectsCircle(VectorD2<int> &midpoint, int radius) const
{

	for (Lines::const_iterator it = lines.begin(); it != lines.end(); it++)  {
		if (it->distFromPoint(midpoint) <= radius)
			return true;
	}

	for (Lines::const_iterator it = horizLines.begin(); it != horizLines.end(); ++it)  {
		if (it->distFromPoint(midpoint) <= radius)
			return true;		
	}

	return false;
}
