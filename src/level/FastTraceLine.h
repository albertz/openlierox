//
//  FastTraceLine.h
//  OpenLieroX
//
//  Created by Albert Zeyer on 02.01.12.
//  code under LGPL
//

#ifndef OpenLieroX_FastTraceLine_h
#define OpenLieroX_FastTraceLine_h

#include "CodeAttributes.h"
#include "CVec.h"
#include "CMap.h"
#include "game/Game.h"

/*
 this traces the given line
 _action is a functor, used for the checkflag_action
 it will be called, if we have a flag fitting to checkflag, with (int x, int y) as param; it also will be called on clipping with the edges
 if the returned value is false, the loop will break
 */
template<class _action>
void fastTraceLine(CVec target, CVec start, uchar checkflag, _action& checkflag_action) {
	enum { X_DOM=-1, Y_DOM=1 } dom; // which is dominating?
	CVec dir = target-start;
	if(dir.x == 0 && dir.y == 0)
		return;
	
	float quot;
	int s_x = (dir.x >= 0) ? 1 : -1;
	int s_y = (dir.y >= 0) ? 1 : -1;	
	// ensure, that |quot| <= 1 (we swap the whole map virtuelly for this, this is, what dom saves)
	if(s_x * dir.x >= s_y * dir.y) {
		dom = X_DOM;
		quot = dir.y / dir.x;
	} else {
		dom = Y_DOM;
		quot = dir.x / dir.y;
	}
	
#ifdef _AI_DEBUG
	//SmartPointer<SDL_Surface> bmpDest = game.gameMap()->GetDebugImage();
#endif
	
	CMap* map = game.gameMap();
	if(!map || !map->isLoaded()) return;
	
	int map_w = map->GetWidth();
	int map_h = map->GetHeight();	
	
	int start_x = (int)start.x;
	int start_y = (int)start.y;
	register int x = 0;
	register int y = 0;
	while(true) {
		if(dom != Y_DOM) { // X_DOM
			y = (int)(quot*(float)x);
		} else { // Y_DOM
			x = (int)(quot*(float)y);
		}
		
		// is all done?
		if(s_x*x > s_x*(int)dir.x
		   || s_y*y > s_y*(int)dir.y) {
			break;
		}
		
		// this is my current pos
		int pos_x = start_x + x;
		int pos_y = start_y + y;
		
		// clipping?
		if(pos_x < 0 || pos_x >= map_w
		   || pos_y < 0 || pos_y >= map_h) {
			if(!checkflag_action(pos_x, pos_y))
				break;
			continue;
		}
		
#ifdef _AI_DEBUG
		//PutPixel(bmpDest,pos_x*2,pos_y*2,Color(255,255,0));
#endif
		
		// is the checkflag fitting to our current flag?
		if(map->unsafeGetMaterial(pos_x,pos_y).toLxFlags() & checkflag)
			// do the given action; break if false
			if(!checkflag_action(pos_x, pos_y))
				break;
		
		// go ahead
		if(dom != Y_DOM) { // X_DOM
			x += s_x;
		} else { // Y_DOM
			y += s_y;
		}
	}
}

struct SimpleTracelineCheck {
	bool result;
	SimpleTracelineCheck() : result(false) {}
	bool operator() (int, int) { result = true; return false; }
};

INLINE bool fastTraceLine_hasAnyCollision(CVec target, CVec start, uchar checkflag) {
	SimpleTracelineCheck ret;
	fastTraceLine(target, start, checkflag, ret);
	return ret.result;
}


#endif
