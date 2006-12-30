/////////////////////////////////////////
//
//             Liero Xtreme
//
//     Copyright Auxiliary Software 2002
//
//
/////////////////////////////////////////


// Worm class - AI
// Created 13/3/03
// Jason Boettcher
// Dark Charlie
// Albert Zeyer

#ifdef WIN32
#pragma warning(disable: 4786)
#endif

#include <set>
#include <list>

#include "defs.h"
#include "LieroX.h"

// we need it here for some debugging...
// we cannot define this globaly because some X11-header also defines this (which is not included here, so this works)
extern	SDL_Surface		*Screen;

static const unsigned short wormsize = 7;


/*
===============================

    Artificial Intelligence

===============================
*/


/*
	this traces the given line
	_action is a functor, used for the checkflag_action
	it will be called, if we have a flag fitting to checkflag, with (int x, int y) as param
	if the returned value is false, the loop will break
*/
template<class _action>
_action fastTraceLine(CVec target, CVec start, CMap *pcMap, uchar checkflag, _action checkflag_action) {
	enum { X_DOM=-1, Y_DOM=1 } dom; // which is dominating?
	CVec dir = target-start;
	if(dir.x == 0 && dir.y == 0)
		return checkflag_action;
		
	float quot;
	int s_x = (dir.x>=0) ? 1 : -1;
	int s_y = (dir.y>=0) ? 1 : -1;	
	// ensure, that |quot| <= 1 (we swap the whole map virtuelly for this, this is, what dom saves)
	if(s_x*dir.x >= s_y*dir.y) {
		dom = X_DOM;
		quot = dir.y/dir.x;
	} else {
		dom = Y_DOM;
		quot = dir.x/dir.y;
	}
	
#ifdef _AI_DEBUG
	//SDL_Surface *bmpDest = pcMap->GetDebugImage();
#endif
	
	const uchar* pxflags = pcMap->GetPixelFlags();
	const uchar* gridflags = pcMap->getAbsoluteGridFlags();
	int map_w = pcMap->GetWidth();
	int map_h = pcMap->GetHeight();	
    int grid_w = pcMap->getGridWidth();
    int grid_h = pcMap->getGridHeight();
	int grid_cols = pcMap->getGridCols();
	
	int start_x = (int)start.x;
	int start_y = (int)start.y;
	int last_gridflag_i = -1;
	int gridflag_i;
	int pos_x, pos_y;
	int grid_x, grid_y;
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
		pos_x = start_x + x;
		pos_y = start_y + y;
			
		// clipping?
		if(pos_x < 0 || pos_x >= map_w
		|| pos_y < 0 || pos_y >= map_h) {
			if(!checkflag_action(pos_x, pos_y))
				break;
			continue;
		}
		
#ifdef _AI_DEBUG
		//PutPixel(bmpDest,pos_x*2,pos_y*2,MakeColour(255,255,0));
#endif
			
		// inside the grid
		grid_x = pos_x / grid_w;
		grid_y = pos_y / grid_h;

		// got we some usefull info from our grid?
		gridflag_i = grid_y*grid_cols + grid_x;
		if(last_gridflag_i != gridflag_i) {
			last_gridflag_i = gridflag_i;
			if(!(gridflags[gridflag_i] & checkflag)) {				
				// yes, we did, no checkflag inside
#ifdef _AI_DEBUG
				//DrawRectFill(bmpDest,grid_x*grid_w*2,grid_y*grid_h*2,(grid_x+1)*grid_w*2+4,(grid_y+1)*grid_h*2,MakeColour(150,150,0));	
#endif
				// go behind this grid-cell
				// the following checks works, because |quot| <= 1
				// make some pictures, then you will belive me :)
				if(dom != Y_DOM) { // X_DOM
					if(s_x > 0) {
						if(s_y > 0) {
							if( pos_x != (grid_x+1)*grid_w &&
							(float(pos_y - (grid_y+1)*grid_h))/float(pos_x - (grid_x+1)*grid_w) <= quot )
								x += int(float((grid_y+1)*grid_h - pos_y)/quot) + 1; // down
							else
								x = (grid_x+1)*grid_w - start_x; // right
						} else { // s_y < 0
							if( pos_x != (grid_x+1)*grid_w &&
							(float(pos_y - grid_y*grid_h))/float(pos_x - (grid_x+1)*grid_w) >= quot )
								x += int(float(grid_y*grid_h - pos_y)/quot) + 1; // up
							else
								x = (grid_x+1)*grid_w - start_x; // right
						}
					} else { // s_x < 0
						if(s_y > 0) {
							if( pos_x != grid_x*grid_w &&
							(float(pos_y - (grid_y+1)*grid_h))/float(pos_x - grid_x*grid_w) >= quot )
								x += int(float((grid_y+1)*grid_h - pos_y)/quot) - 1; // down
							else
								x = grid_x*grid_w - start_x - 1; // left
						} else { // s_y < 0
							if( pos_x != grid_x*grid_w &&
							(float(pos_y - grid_y*grid_h))/float(pos_x - grid_x*grid_w) <= quot )
								x += int(float(grid_y*grid_h - pos_y)/quot) - 1; // up
							else
								x = grid_x*grid_w - start_x - 1; // left
						}
					}
				} else { // Y_DOM
					if(s_y > 0) {
						if(s_x > 0) {
							if( pos_y != (grid_y+1)*grid_h &&
							(float(pos_x - (grid_x+1)*grid_w))/float(pos_y - (grid_y+1)*grid_h) <= quot )
								y += int(float((grid_x+1)*grid_w - pos_x)/quot) + 1; // right
							else
								y = (grid_y+1)*grid_h - start_y; // down
						} else { // s_y < 0
							if( pos_y != (grid_y+1)*grid_h &&
							(float(pos_x - grid_x*grid_w))/float(pos_y - (grid_y+1)*grid_h) >= quot )
								y += int(float(grid_x*grid_w - pos_x)/quot) + 1; // left
							else
								y = (grid_y+1)*grid_h - start_y; // down
						}
					} else { // s_y < 0
						if(s_x > 0) {
							if( pos_y != grid_y*grid_h &&
							(float(pos_x - (grid_x+1)*grid_w))/float(pos_y - grid_y*grid_h) >= quot )
								y += int(float((grid_x+1)*grid_w - pos_x)/quot) - 1; // right
							else
								y = grid_y*grid_h - start_y - 1; // up
						} else { // s_y < 0
							if( pos_y != grid_y*grid_h &&
							(float(pos_x - grid_x*grid_w))/float(pos_y - grid_y*grid_h) <= quot )
								y += int(float(grid_x*grid_w - pos_x)/quot) - 1; // left
							else
								y = grid_y*grid_h - start_y - 1; // up
						}
					}
				}	
			}
			continue;		
		} 
		
		// is the checkflag fitting to our current flag?
		if(pxflags[pos_y*map_w + pos_x] & checkflag)
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
	
	return checkflag_action;
}


// returns the maximal possible free rectangle with the given point inside
// (not in every case, but in most)
// the return-value of type SquareMatrix consists of the top-left and upper-right pixel
// WARNING: if the given point is not in the map, the returned 
// area is also not in the map (but it will handle it correctly)
SquareMatrix<int> getMaxFreeArea(VectorD2<int> p, CMap* pcMap, uchar checkflag) {
	// yes I know, statics are not good style,
	// but I will not recursivly use it and
	// the whole game is singlethreaded
	int map_w = pcMap->GetWidth();
	int map_h = pcMap->GetHeight();	
    int grid_w = pcMap->getGridWidth();
    int grid_h = pcMap->getGridHeight();
	int grid_cols = pcMap->getGridCols();
	const uchar* pxflags = pcMap->GetPixelFlags();
	const uchar* gridflags = pcMap->getAbsoluteGridFlags();
	
	SquareMatrix<int> ret;
	ret.v1 = p; ret.v2 = p;
	
	// just return if we are outside
	if(p.x < 0 || p.x >= map_w
	|| p.y < 0 || p.y >= map_h)
		return ret;
	
	enum { GO_RIGHT=1, GO_DOWN=2, GO_LEFT=4, GO_UP=8 }; short dir;
	unsigned short col;
	register int x, y;
	int grid_x, grid_y;
	bool avoided_all_grids;
	
	// loop over all directions until there is some obstacle
	col = 0; dir = 1;
	while(true) {
		if(col == 15) // got we collisions in all directions?
			break;
		
		// change direction
		do {
			dir = dir << 1;
			if(dir > 8) dir = 1;
		} while(col & dir);		
		
		// set start pos
		switch(dir) {
		case GO_RIGHT: x=ret.v2.x+1; y=ret.v1.y; break;
		case GO_DOWN: x=ret.v1.x; y=ret.v2.y+1; break;
		case GO_LEFT: x=ret.v1.x-1; y=ret.v1.y; break;
		case GO_UP: x=ret.v1.x; y=ret.v1.y-1; break;
		}
		
		// check if still inside the map (than nothing bad can happen)
		if(x < 0 || x >= map_w
		|| y < 0 || y >= map_h) {
			col |= dir;
			continue;			
		}
		
		// loop over all pxflags of the aligned line and check for an obstacle
		avoided_all_grids = true;
		while(true) {
			// break if ready
			if(dir == GO_RIGHT || dir == GO_LEFT) {
				if(y > ret.v2.y) break;
			} else // GO_UP / GO_DOWN
				if(x > ret.v2.x) break;
				
			// check if we can avoid this gridcell
			grid_x = x / grid_w; grid_y = y / grid_h;
			if(!(gridflags[grid_y*grid_cols + grid_x] & checkflag)) {
				// yes we can and do now
				switch(dir) {
				case GO_RIGHT: case GO_LEFT: y=(grid_y+1)*grid_h; break;
				case GO_DOWN: case GO_UP: x=(grid_x+1)*grid_w; break;
				}
				continue;
			} else
				avoided_all_grids = false;

			// is there some obstacle?
			if(pxflags[y*map_w + x] & checkflag) {
				col |= dir;
				break;
			}

			// inc the pos (trace the aligned line)
			switch(dir) {
			case GO_RIGHT: case GO_LEFT: y++; break;
			case GO_DOWN: case GO_UP: x++; break;
			}			
		}
		
		if(!(col & dir)) {
			if(avoided_all_grids) {
				// we can jump to the end of the grids in this case
				// grid_x/grid_y was already set here by the last loop
				switch(dir) {
				case GO_RIGHT: ret.v2.x=MIN((grid_x+1)*grid_w-1,map_w-1); break;
				case GO_DOWN: ret.v2.y=MIN((grid_y+1)*grid_h-1,map_h-1); break;
				case GO_LEFT: ret.v1.x=grid_x*grid_w; break;
				case GO_UP: ret.v1.y=grid_y*grid_h; break;
				}			
			} else { // not avoided_all_grids
				// simple inc 1 pixel in the checked direction
				switch(dir) {
				case GO_RIGHT: ret.v2.x++; break;
				case GO_DOWN: ret.v2.y++; break;
				case GO_LEFT: ret.v1.x--; break;
				case GO_UP: ret.v1.y--; break;
				}
			}	
		}
		
	} // loop over directions
	
	// cut the area if outer space...
	if(ret.v1.x < 0) ret.v1.x = 0;
	if(ret.v1.y < 0) ret.v1.y = 0;
	if(ret.v2.x >= map_w) ret.v2.x = map_w-1;
	if(ret.v2.y >= map_h) ret.v2.y = map_h-1;
	
	return ret;
}




NEW_ai_node_t* createNewAiNode(float x, float y, NEW_ai_node_t* next = NULL, NEW_ai_node_t* prev = NULL) {
	NEW_ai_node_t* tmp = (NEW_ai_node_t*)malloc(sizeof(NEW_ai_node_t));
	tmp->fX = x; tmp->fY = y;
	tmp->psNext = next; tmp->psPrev = prev;
	return tmp;
}

NEW_ai_node_t* createNewAiNode(NEW_ai_node_t* base) {
	if(!base) return NULL;
	NEW_ai_node_t* tmp = (NEW_ai_node_t*)malloc(sizeof(NEW_ai_node_t));
	tmp->fX = base->fX; tmp->fY = base->fY;
	tmp->psNext = base->psNext; tmp->psPrev = base->psPrev;
	return tmp;
}


// returns true, if the given line is free or not
// these function will either go parallel to the x-axe or parallel to the y-axe
// (depends on which of them is the absolute greatest)
inline bool simpleTraceLine(CMap* pcMap, VectorD2<int> start, VectorD2<int> dist, uchar checkflag) {
	const uchar* pxflags = pcMap->GetPixelFlags();
	int map_w = pcMap->GetWidth();
	int map_h = pcMap->GetHeight();	
	
	if(abs(dist.x) >= abs(dist.y)) {
		if(dist.x < 0) { // avoid anoying checks
			start.x += dist.x;
			dist.x = -dist.x;
		}
		if(start.x < 0 || start.x + dist.x >= map_w || start.y < 0 || start.y >= map_h)
			return false;
		for(register int x = 0; x <= dist.x; x++) {
			if(pxflags[start.y*map_w + start.x + x] & checkflag)
				return false;
		}
	} else { // y is greater
		if(dist.y < 0) { // avoid anoying checks
			start.y += dist.y;
			dist.y = -dist.y;
		}
		if(start.y < 0 || start.y + dist.y >= map_h || start.x < 0 || start.x >= map_w)
			return false;
		for(register int y = 0; y <= dist.y; y++) {
			if(pxflags[(start.y+y)*map_w + start.x] & checkflag)
				return false;
		}	
	}
	return true;
}


/*
this class do the whole pathfinding (idea by AZ)
you can use the findPath-function directly,
or you can use the function StartThreadSearch, which
start the search in an own thread; you can ask
for the state with IsReady
*/
class searchpath_base {
public:

// this will define, if we should break the search on the first result
// (we ensure in forEachChecklistItem, that it is not that bad)
#define		BREAK_ON_FIRST	1

	class area_item {
	public:
				
		// needed for area_mset
		class pseudoless {
		public:			
			// this is a well-defined transitive ordering after the v1-vector of the matrix
			inline bool operator()(const area_item* a, const area_item* b) const {
				if(!a || !b) return false; // this isn't handled correctly, but this should never hapen
				return a->area.v1 < b->area.v1;
			}
		};
		
		searchpath_base* base;
		
		// this will save the state, if we still have to check a specific end
		// at the rectangle or not
		short checklistRows;
		short checklistRowStart;
		short checklistColWidth;
		short checklistCols;
		short checklistColStart;
		short checklistRowHeight;
	
		SquareMatrix<int> area;
		bool inUse; // indicates, that some recursive tree is using this area as a base
		NEW_ai_node_t* bestNode; // the best way from here to the target
		VectorD2<int> bestPosInside, bestPosOutside; // the start-pos of the best way		
				
		void initChecklists() {
			VectorD2<int> size = area.v2 - area.v1;
			// ensure here, that the starts are not 0
			checklistCols = (size.x-3) / checklistColWidth + 1;
			checklistRows = (size.y-3) / checklistRowHeight + 1;
			checklistColStart = (size.x - (checklistCols-1)*checklistColWidth) / 2;
			checklistRowStart = (size.y - (checklistRows-1)*checklistRowHeight) / 2;
		}
				
		inline VectorD2<int> getCenter() {
			return (area.v1+area.v2)/2;
		}
		
		area_item(searchpath_base* b) :
			inUse(false),
			base(b),
			bestNode(NULL),
			checklistRows(0),
			checklistRowStart(0),
			checklistRowHeight(5),
			checklistCols(0),
			checklistColStart(0),
			checklistColWidth(5) {}
		
		// iterates over all checklist points
		// calls given action with 2 parameters:
		//    VectorD2<int> p, VectorD2<int> dist
		// p is the point inside of the area, dist the change to the target
		// if the returned value by the action is false, it will break
		template<typename _action>
		void forEachChecklistItem(_action action, VectorD2<int> start) {
			register int i;
			VectorD2<int> p, dist;

			typedef std::multiset< VectorD2<int>, VectorD2__absolute_less<int> > p_mset;			
			// the set point here defines, where the 'best' point is (the sorting depends on it)
			p_mset points(VectorD2__absolute_less<int>((base->target))); // + getCenter()*2 - start) / 2));

			// insert now the points to the list
			// the list will sort itself
			
			// left
			p.x = area.v1.x;
			dist.x = -checklistRowHeight; dist.y = 0;
			for(i = 0; i < checklistRows; i++) {
				p.y = area.v1.y + checklistRowStart + i*checklistRowHeight;
				points.insert(p);
			}
			
			// right
			p.x = area.v2.x;
			dist.x = checklistRowHeight; dist.y = 0;
			for(i = 0; i < checklistRows; i++) {
				p.y = area.v1.y + checklistRowStart + i*checklistRowHeight;
				points.insert(p);
			}
			
			// top
			p.y = area.v1.y;
			dist.x = 0; dist.y = -checklistColWidth;
			for(i = 0; i < checklistCols; i++) {
				p.x = area.v1.x + checklistColStart + i*checklistColWidth;
				points.insert(p);
			}
			
			// bottom
			p.y = area.v2.y;
			dist.x = 0; dist.y = checklistColWidth;
			for(i = 0; i < checklistCols; i++) {
				p.x = area.v1.x + checklistColStart + i*checklistColWidth;
				points.insert(p);
			}			

			// the list is sorted, the closest (to the target) comes first
			for(p_mset::iterator it = points.begin(); it != points.end(); it++) {
				if(it->x == area.v1.x) { // left
					dist.x = -checklistRowHeight; dist.y = 0;
				} else if(it->x == area.v2.x) { // right
					dist.x = checklistRowHeight; dist.y = 0;
				} else if(it->y == area.v1.y) { // top
					dist.x = 0; dist.y = -checklistColWidth;
				} else { // bottom
					dist.x = 0; dist.y = checklistColWidth;
				}
				if(!action(*it, dist)) return;				
			}
			
		}
		
		class check_checkpoint {
		public:
			area_item* myArea;
			searchpath_base* base;
			float bestNodeLen; // this can be done better...
			
			check_checkpoint(searchpath_base* b, area_item* a) :
				base(b),
				myArea(a),
				bestNodeLen(-1) {}
					
			// this will be called by forEachChecklistItem
			// pt is the checkpoint and dist the change to the new target
			// it will search for the best node starting at the specific pos
			inline bool operator()(VectorD2<int> pt, VectorD2<int> dist) {
				bool trace = true;	
				VectorD2<int> dir, start;
				unsigned short left, right;
				if(abs(dist.x) >= abs(dist.y))
					dir.y = 1;
				else
					dir.x = 1;
				start = pt;
				pt += dir;
				
				// ensure, that the way has at least the width of wormsize
				base->pcMap->lockFlags();				
				trace = simpleTraceLine(base->pcMap, pt, dist, PX_ROCK);
				for(left = 0; trace && left < wormsize; pt += dir) {
					trace = simpleTraceLine(base->pcMap, pt, dist, PX_ROCK);
					if(trace) left++;			
				}
				pt = start - dir; trace = true;
				for(right = 0; trace && right < (wormsize-left); right++, pt -= dir) {
					trace = simpleTraceLine(base->pcMap, pt, dist, PX_ROCK);
					if(trace) right++;
				}				
				base->pcMap->unlockFlags();
								
				// is there enough space?
				if(left+right >= wormsize) {
					// we can start a new search from this point (targ)
					NEW_ai_node_t* node = base->findPath(pt + dist);
					if(node) {
						// good, we find a new path
						// check now, if it is better then the last found
#if BREAK_ON_FIRST != 1
						float node_len = get_ai_nodes_length(node);						
						if(bestNodeLen < 0 || node_len < bestNodeLen) {
							// yes, it is better
							
							// save the new info
							bestNodeLen = node_len;
#endif // BREAK_ON_FIRST
							myArea->bestPosInside = pt;
							myArea->bestPosOutside = pt + dist;
							myArea->bestNode = node;
						
#if BREAK_ON_FIRST == 1
							// the result of this return is, that the algo will break with the first found path
							return false;
#else						
						} // better node check
#endif
					}
				}
				
				// continue the search
				return true;
			}
		}; // class check_checkpoint
				
		NEW_ai_node_t* process(VectorD2<int> start) {
			// search the best path
			inUse = true;
			forEachChecklistItem(check_checkpoint(base, this), start);
			inUse = false;
			
			// did we find any?
			if(bestNode) {
				// start at the pos inside of the area
				bestNode = createNewAiNode(bestPosOutside.x, bestPosOutside.y, bestNode);
				base->nodes.push_back(bestNode);
				bestNode = createNewAiNode(bestPosInside.x, bestPosInside.y, bestNode);
				base->nodes.push_back(bestNode);
				return bestNode;
			}
			
			// nothing found
			return NULL;
		}
		
	}; // class area_item
	
	typedef std::multiset< area_item*, area_item::pseudoless > area_mset;
	typedef std::list< NEW_ai_node_t* > node_list;
	
	// these neccessary attributes have to be set manually
	CMap* pcMap;
	area_mset areas;
	node_list nodes;
	VectorD2<int> start, target;
	
	searchpath_base() :
		pcMap(NULL),
		thread(NULL),
		thread_is_ready(true),
		resulted_path(NULL),
		break_thread_signal(0) {
		thread_mut = SDL_CreateMutex();
		//printf("starting thread for %i ...\n", (long)this);
		// TODO: it will not freed in every case (says valgrind)
		thread = SDL_CreateThread(threadSearch, this);
	}
	
	~searchpath_base() {
		// thread cleaning up
		//printf("breaking thread for %i ...\n", (long)this);
		breakThreadSignal();
		SDL_WaitThread(thread, NULL);
		//printf("thread for %i finished\n", (long)this);		
		thread = NULL;
		SDL_DestroyMutex(thread_mut);
		
		clear();
	}
		
	void removePathFromList(NEW_ai_node_t* start) {
		for(NEW_ai_node_t* node = start; node; node = node->psNext)
			nodes.remove(node);
	}	
		
private:
	void clear() {
		clear_areas();
		clear_nodes();
	}

	void clear_areas() {
		for(area_mset::iterator it = areas.begin(); it != areas.end(); it++) {
			delete *it;
		}
		areas.clear();	
	}
	
	void clear_nodes() {
		for(node_list::iterator it = nodes.begin(); it != nodes.end(); it++) {
			free(*it);
		}
		nodes.clear();
	}
	
	// searches for an overleading area and returns the first
	// returns NULL, if none found
	area_item* getArea(VectorD2<int> p) {
		// (take a look at pseudoless)
		for(area_mset::iterator it = areas.begin(); it != areas.end() && (*it)->area.v1 <= p; it++) {
			if((*it)->area.v1.x <= p.x && (*it)->area.v1.y <= p.y
			&& (*it)->area.v2.x >= p.x && (*it)->area.v2.y >= p.y)
				return *it;
		}
		
#ifdef _AI_DEBUG
/*		printf("getArea( %i, %i )\n", p.x, p.y);
		printf("  don't find an underlying area\n");
		printf("  areas = {\n");
		for(area_mset::iterator it = areas.begin(); it != areas.end(); it++) {
			printf("		( %i, %i, %i, %i )%s,\n",
				(*it)->area.v1.x, (*it)->area.v1.y,
				(*it)->area.v2.x, (*it)->area.v2.y,
				((*it)->area.v1 <= p) ? "" : " (*)");
		}
		printf("     }\n"); */
#endif		

		return NULL;
	}
		
public:	
	NEW_ai_node_t* findPath(VectorD2<int> start) {
		// lower priority to this thread
		SDL_Delay(1);
		if(shouldBreakThread()) return NULL;
		
		// is the start inside of the map?
		if(start.x < 0 || start.x >= pcMap->GetWidth() 
		|| start.y < 0 || start.y >= pcMap->GetHeight())
			return NULL;
		
		// can we just finish with the search?
		pcMap->lockFlags();
		if(traceWormLine(target, start, pcMap)) {
			pcMap->unlockFlags();
			// yippieh!
			NEW_ai_node_t* ret = createNewAiNode(target.x, target.y);
			nodes.push_back(ret);
			return ret;
		}
		pcMap->unlockFlags();
		
		// look around for an existing area here
		area_item* a = getArea(start);		
		if(a) { // we found an area which includes this point
			// are we started somewhere from here?
			if(a->inUse)
				return NULL;

			// we have already found out the best path from here
			return a->bestNode;
		}
		
		// get the max area (rectangle) around us
		pcMap->lockFlags();
		SquareMatrix<int> area = getMaxFreeArea(start, pcMap, PX_ROCK);
		pcMap->unlockFlags();		
		if(area.v2.x-area.v1.x >= wormsize && area.v2.y-area.v1.y >= wormsize) {
			a = new area_item(this);
			a->area = area;
			a->initChecklists();
			areas.insert(a);		
#ifdef _AI_DEBUG
/*			printf("findPath( %i, %i )\n", start.x, start.y);
			printf("   new area:\n");
			printf("   ( %i, %i, %i, %i )\n", a->area.v1.x, a->area.v1.y, a->area.v2.x, a->area.v2.y); */
/*					
			DrawRectFill(pcMap->GetDebugImage(),a->area.v1.x*2,a->area.v1.y*2,a->area.v2.x*2,a->area.v2.y*2,MakeColour(150,150,0));
			cClient->Draw(Screen); // dirty dirty...
			FlipScreen(Screen); */
#endif			
			// and search 
			return a->process(start);
		}
		
		// the max area around us is to small
		return NULL;
	}
	
	// this function will start the search, if it was not started right now
	void startThreadSearch() {
		// if we are still searching, do nothing
		if(!isReady()) return;
				
		clear();
		resulted_path = NULL;
		
		// this is the signal to start the search
		setReady(false);
	}

private:
	// main-function used by the thread
	static int threadSearch(void* b) {
		searchpath_base* base = (searchpath_base*)b;
		NEW_ai_node_t* ret;
		
		while(true) {
			// sleep a little bit while we have nothing to do...
			while(base->isReady()) {
				SDL_Delay(100);
				if(base->shouldBreakThread()) {
					//printf("got break signal(1) for %i\n", (long)base);
					return 0;				
				}
			}
			
			// start the main search
			ret = base->findPath(base->start);
			base->completeNodesInfo(ret);
			base->simplifyPath(ret);
			base->splitUpNodes(ret, NULL);
			
			// was there a break-signal?
			if(base->shouldBreakThread()) {
				//printf("got break signal(2) for %i\n", (long)base);
				return 0;			
			}

			// we are ready now
			base->resulted_path = ret;
			base->setReady(true);
		}
	}
	
public:	
	inline bool isReady() {
		bool ret = false;
		lock();
		ret = thread_is_ready;
		unlock();
		return ret;
	}

	inline void setReady(bool state) {
		lock();
		thread_is_ready = state;
		unlock();		
	}

	// WARNING: not thread safe; call isReady before
	inline NEW_ai_node_t* resultedPath() {
		return resulted_path;
	}

private:
	NEW_ai_node_t* resulted_path;
	SDL_Thread* thread;
	SDL_mutex* thread_mut;
	bool thread_is_ready;
	int break_thread_signal;
	
	inline void breakThreadSignal() {
		// we don't need more thread-safety here, because this will not fail
		break_thread_signal = 1;
	}

	inline bool shouldBreakThread() {
		return (break_thread_signal != 0);
	}

	inline void lock() {
		SDL_mutexP(thread_mut);	
	}
	
	inline void unlock() {
		SDL_mutexV(thread_mut);
	}
	
	void completeNodesInfo(NEW_ai_node_t* start) {
		NEW_ai_node_t* last = NULL;
		for(NEW_ai_node_t* n = start; n; n = n->psNext) {
			n->psPrev = last;
			last = n;
		}
	}
	
	void splitUpNodes(NEW_ai_node_t* start, NEW_ai_node_t* end) {
		NEW_ai_node_t* tmpnode = NULL;
		short s1, s2;
		static const unsigned short dist = 50;
		for(NEW_ai_node_t* n = start; n && n->psNext && n != end; n = n->psNext) {
			s1 = (n->fX > n->psNext->fX) ? 1 : -1;
			s2 = (n->fY > n->psNext->fY) ? 1 : -1;		
			if(s1*(n->fX - n->psNext->fX) > dist || s2*(n->fY - n->psNext->fY) > dist) {
				tmpnode = (NEW_ai_node_t*)malloc(sizeof(NEW_ai_node_t));
				if(tmpnode) {
					nodes.push_back(tmpnode);
					if(s1*(n->fX - n->psNext->fX) >= s2*(n->fY - n->psNext->fY)) {
						tmpnode->fX = n->fX - s1*dist;
						tmpnode->fY = n->fY
							- s1*dist*(n->fY - n->psNext->fY)/(n->fX - n->psNext->fX);
					} else {
						tmpnode->fY = n->fY - s2*dist;
						tmpnode->fX = n->fX
							- s2*dist*(n->fX - n->psNext->fX)/(n->fY - n->psNext->fY);
					}
					tmpnode->psNext = n->psNext;
					tmpnode->psPrev = n;
					n->psNext->psPrev = tmpnode;
					n->psNext = tmpnode;
				}			
			}
		}
	}
	

	void simplifyPath(NEW_ai_node_t* start) {
		NEW_ai_node_t* node = NULL;
		NEW_ai_node_t* last_node = NULL;
		NEW_ai_node_t* closest_node = NULL;
		unsigned short count = 0;
		float dist, len;
		
		// short up
		for(node = start; node; node = node->psNext)  {
			len = 0;
			last_node = node;
			for(closest_node = node, count = 0; closest_node; closest_node = closest_node->psNext, count++) {
				len += CVec(closest_node->fX - last_node->fX, closest_node->fY - last_node->fY).GetLength2();
				dist = CVec(closest_node->fX - node->fX, closest_node->fY - node->fY).GetLength2();
				if(count >= 3
				&& dist < len
				&& traceWormLine(CVec(closest_node->fX,closest_node->fY),CVec(node->fX,node->fY),pcMap)) {
					node->psNext = closest_node;
					closest_node->psPrev = node;
					len = dist;
				}
				last_node = closest_node;
			}
		}
	
		// simplify
		for(node = start; node; node = node->psNext) {
			// While we see the two nodes, delete all nodes between them and skip to next node
			count = 0;
			while (closest_node && traceWormLine(CVec(closest_node->fX,closest_node->fY),CVec(node->fX,node->fY),pcMap))  {
				if(count >= 2) {
					node->psNext = closest_node;
					closest_node->psPrev = node;
				}
				closest_node = closest_node->psNext;
				count++;
			}
		}
	}
	
}; // class searchpath_base



///////////////////
// Initialize the AI
bool CWorm::AI_Initialize(CMap *pcMap)
{
    assert(pcMap);

    // Because this can be called multiple times, shutdown any previous allocated data
    AI_Shutdown();

    // Allocate the Open/Close grid
    nGridCols = pcMap->getGridCols();
    nGridRows = pcMap->getGridRows();

    pnOpenCloseGrid = new int[nGridCols*nGridRows];
    if(!pnOpenCloseGrid)
        return false;

    psPath = NULL;
    psCurrentNode = NULL;
    fLastCarve = -9999;
    cStuckPos = CVec(-999,-999);
    fStuckTime = -9999;
    fLastPathUpdate = -9999;
	fLastJump = -9999;
	fLastCreated = -9999;
    bStuck = false;
	bPathFinished = true;
	//iAiGameType = GAM_OTHER;
	nAITargetType = AIT_NONE;
	nAIState = AI_THINK;
	fLastFace = -9999;

	fRopeAttachedTime = 0;
	fRopeHookFallingTime = 0;

	pathSearcher = new searchpath_base;
	if(!pathSearcher)
		return false;
	((searchpath_base*)pathSearcher)->pcMap = pcMap;

    return true;
}


///////////////////
// Shutdown the AI stuff
void CWorm::AI_Shutdown(void)
{
	if(pathSearcher) {
		delete ((searchpath_base*)pathSearcher);			
	}
	pathSearcher = NULL;
	
	// in every case, the nodes of the current path is not handled by pathSearcher
	delete_ai_nodes(NEW_psPath);
	
    AI_CleanupPath(psPath);
	NEW_psPath = NULL;
	NEW_psCurrentNode = NULL;
	NEW_psLastNode = NULL;
    psPath = NULL;
    psCurrentNode = NULL;
    if(pnOpenCloseGrid)
        delete[] pnOpenCloseGrid;
    pnOpenCloseGrid = NULL;
}





/*
  Algorithm:
  ----------

  1) Find nearest worm and set it as a target
  2) If we are within a good distance, aim and shoot the target
  3) If we are too far, try and get closer by digging and ninja rope

*/

#ifdef _AI_DEBUG
class debug_print_col {
public:
	SDL_Surface *bmpDest;
	debug_print_col(SDL_Surface* dest=NULL) : bmpDest(dest) {}
	
	bool operator()(int x, int y) const {
		if(!bmpDest)
			return false;
		if(x*2-4 >= 0 && x*2+4 < bmpDest->w
		&& y*2-4 >= 0 && y*2+4 < bmpDest->h)
			DrawRectFill(bmpDest,x*2-4,y*2-4,x*2+4,y*2+4,MakeColour(255,255,0));
		else
			return false;
		return true;
	}
};

void do_some_tests_with_fastTraceLine(CMap *pcMap) {
	CVec start, target;
	start.x = rand() % pcMap->GetWidth();
	start.y = rand() % pcMap->GetHeight();
	target.x = rand() % pcMap->GetWidth();
	target.y = rand() % pcMap->GetHeight();
	
	fastTraceLine(target, start, pcMap, PX_ROCK,  debug_print_col(pcMap->GetDebugImage()));
}
#endif


///////////////////
// Simulate the AI
// TODO: this is global for all worms (which is not what is wanted)
float fLastTurn = 0;  // Time when we last tried to change the direction
void CWorm::AI_GetInput(int gametype, int teamgame, int taggame, CMap *pcMap)
{
	// Behave like humans and don't play immediatelly after spawn
	if ((tLX->fCurTime-fSpawnTime) < 0.4)
		return;

#ifdef _AI_DEBUG
/*	DrawRectFill(pcMap->GetDebugImage(),0,0,pcMap->GetDebugImage()->w,pcMap->GetDebugImage()->h,MakeColour(255,0,255));
	do_some_tests_with_fastTraceLine(pcMap);
	usleep(1000000);	
	return; */
#endif
		
	worm_state_t *ws = &tState;
	gs_worm_t *wd = cGameScript->getWorm();

	// Init the ws
	ws->iCarve = false;
	ws->iMove = false;
	ws->iShoot = false;
	ws->iJump = false;

	iAiGame = gametype;
	iAiTeams = teamgame;
	iAiTag = taggame;

    strcpy(tLX->debug_string, "");

	iAiDiffLevel = tProfile->nDifficulty;
	float dt = tLX->fDeltaTime;

    // Every 3 seconds we run the think function
    if(tLX->fCurTime - fLastThink > 3 && nAIState != AI_THINK)
        nAIState = AI_THINK;

	// check more often if we path isn't finished yet
	if(tLX->fCurTime - fLastThink > 0.5 && !bPathFinished)
		nAIState = AI_THINK;

    // If we have a good shooting 'solution', shoot
	// TODO: join AI_CanShoot and AI_Shoot
    if(AI_CanShoot(pcMap, gametype)) {
   
		// jump, move and carve around
		if (tLX->fCurTime-fLastJump > 1.0f)  {
			ws->iJump = true;
			fLastJump = tLX->fCurTime;
		}
		ws->iMove = true;

		// Don't carve so fast!
		if (tLX->fCurTime-fLastCarve > 0.2f)  {
			fLastCarve = tLX->fCurTime;
			ws->iCarve = true; // Carve
		}
		else  {
			ws->iCarve = false;
		}
   
   		if(CheckOnGround(pcMap))
   			cNinjaRope.Release();
   
   		if(!AI_Shoot(pcMap)) {   			
			// change direction after some time
			if ((tLX->fCurTime-fLastTurn) > 1.0)  {
				iDirection = !iDirection;
				fLastTurn = tLX->fCurTime;
			}
			// don't return here -> do MoveToTarget/Think
    	} else
	        return;
        
    } else {
    
		// Reload weapons when we can't shoot
		AI_ReloadWeapons();
    
    }
      
    // Process depending on our current state
    switch(nAIState) {

        // Think; We spawn in this state
        case AI_THINK:
            AI_Think(gametype, teamgame, taggame, pcMap);
            break;

        // Moving towards a target
        case AI_MOVINGTOTARGET:
            NEW_AI_MoveToTarget(pcMap);
            break;
    }


    //
    // Find a target worm
    //
    /*CWorm *trg = findTarget(gametype, teamgame, taggame, pcMap);

	// If we have no target, do something
    if(!trg)
        // TODO Something
        return;


    //
    // Shoot the target
    //

    // If we can shoot the target, shoot it
    if(AI_CanShoot(trg, pcMap)) {

        // Shoot the target

    } else {

        // If we cannot shoot the target, walk towards the target
        AI_MoveToTarget(trg, pcMap);
    }


	//
	// Shoot the target
	//


	CVec	tgPos = trg->getPos();
	CVec	tgDir = tgPos - vPos;    

	int		iAngleToBig = false;

	float   fDistance = NormalizeVector(&tgDir);
	
	// Make me face the target
	if(tgPos.x > vPos.x)
		iDirection = DIR_RIGHT;
	else
		iDirection = DIR_LEFT;


	// Aim at the target
	float ang = (float)atan2(tgDir.x, tgDir.y);
	ang = RAD2DEG(ang);

	if(iDirection == DIR_LEFT)
		ang+=90;
	else
		ang = -ang + 90;

	// Clamp the angle
	ang = MAX(-90, ang);

	if(ang > 60) {
		ang = 60;
		iAngleToBig = true;
	}


	// AI Difficulty level effects accuracy
	float Levels[] = {12, 6, 3, 0};

	// If we are going to shoot, we need to lower the accuracy a little bit
	ang += GetRandomNum() * Levels[DiffLevel];


	// Move the angle at the same speed humans are allowed to move the angle
	if(ang > fAngle)
		fAngle += wd->AngleSpeed * dt;
	else if(ang < fAngle)
		fAngle -= wd->AngleSpeed * dt;

	// If the angle is within +/- 3 degrees, just snap it
	if( fabs(fAngle - ang) < 3)
		fAngle = ang;

	// Clamp the angle
	fAngle = MIN(60,fAngle);
	fAngle = MAX(-90,fAngle);


	// ???: If the angle is to big, (ie, the target is below us and we can't aim directly at him) don't shoot


	// If we are close enough, shoot the target
	if(fDistance < 200) {

        // Find the best weapon for the job
        int wpn = getBestWeapon( gametype, fDistance, tgPos, pcMap );
        if( wpn >= 0 ) {
            iCurrentWeapon = wpn;
            ws->iShoot = true;
        }
    } else {

        // If we're outside shooting range, cycle through the weapons so they can be reloaded
        iCurrentWeapon = cycleWeapons();
    }


	// If we are real close, jump to get a better angle
	// Only do this if we are better then easy level
	if(fDistance < 10 && DiffLevel > AI_EASY)
		ws->iJump = true;



	// We need to move closer to the target

	// We carve every few milliseconds so we don't go too fast
	if(tLX->fCurTime - fLastCarve > 0.35f) {
		fLastCarve = tLX->fCurTime;
		ws->iCarve = true;
	}

	ws->iMove = true;



	// If the target is above us, we point up & use the ninja rope
	if(fAngle < -60 && (vPos - tgPos).y > 50) {
		
		fAngle = -90;
        CVec dir;
        dir.x=( (float)cos(fAngle * (PI/180)) );
	    dir.y=( (float)sin(fAngle * (PI/180)) );
	    if(iDirection==DIR_LEFT)
		    dir.x=(-dir.x);

        if( !cNinjaRope.isReleased() )
            cNinjaRope.Shoot(vPos,dir);
	}


    // If the hook of the ninja rope is below us, release it
    if( cNinjaRope.isReleased() && cNinjaRope.isAttached() && cNinjaRope.getHookPos().y > vPos.y ) {
        cNinjaRope.Release();
    }

    // If the target is not too far above us, we should release the ninja rope
    if( (vPos - tgPos).y < 30 && cNinjaRope.isAttached() && cNinjaRope.isReleased() ) {
        cNinjaRope.Release();
    }*/
}


///////////////////
// Find a target worm
CWorm *CWorm::findTarget(int gametype, int teamgame, int taggame, CMap *pcMap)
{
	CWorm	*w = cClient->getRemoteWorms();
	CWorm	*trg = NULL;
	CWorm	*nonsight_trg = NULL;
	float	fDistance = 99999;
	float	fSightDistance = 99999;

	int NumTeams = 0;
	int i;
	for (i=0; i<4; i++)
		if (cClient->getTeamScore(i) > -1)
			NumTeams++;


    //
	// Just find the closest worm
	//

	for(i=0; i<MAX_WORMS; i++, w++) {

		// Don't bother about unused or dead worms
		if(!w->isUsed() || !w->getAlive())
			continue;

		// Make sure i don't target myself
		if(w->getID() == iID)
			continue;

		// If this is a team game, don't target my team mates
		// BUT, if there is only one team, play it like deathmatch
		if(teamgame && w->getTeam() == iTeam && NumTeams > 1)
			continue;

		// If this is a game of tag, only target the worm it (unless it's me)
		if(taggame && !w->getTagIT() && !iTagIT)
			continue;

		// Calculate distance between us two
		float l = CalculateDistance(w->getPos(), vPos);

		// Prefer targets we have free line of sight to
		float length;
		int type;
		traceLine(w->getPos(),pcMap,&length,&type,1);
		if (! (type & PX_ROCK))  {
			// Line of sight not blocked
			if (l < fSightDistance)  {
				trg = w;
				fSightDistance = l;
				if (l < fDistance)  {
					nonsight_trg = w;
					fDistance = l;
				}
			}
		}
		else
			// Line of sight blocked
			if(l < fDistance) {
				nonsight_trg = w;
				fDistance = l;
			}
	}

	// If the target we have line of sight to is too far, switch back to the closest target
	if (fSightDistance-fDistance > 50.0f || iAiGameType == GAM_RIFLES || trg == NULL)  {
		if (nonsight_trg)
			trg = nonsight_trg;
	}

    return trg;
}


///////////////////
// Think State
void CWorm::AI_Think(int gametype, int teamgame, int taggame, CMap *pcMap)
{
    /*
      We start of in an think state. When we're here we decide what we should do.
      If there is an unfriendly worm, or a game target we deal with it.
      In the event that we have no unfriendly worms or game targets we should remain in the idle state.
      While idling we can walk around to appear non-static, reload weapons or grab a bonus.
    */

    // Clear the state
    psAITarget = NULL;
    psBonusTarget = NULL;
    nAITargetType = AIT_NONE;
    fLastThink = tLX->fCurTime;


    // Reload our weapons in idle mode
    AI_ReloadWeapons();


    // If our health is less than 15% (critical), our main priority is health
    if(iHealth < 15)
        if(AI_FindHealth(pcMap))
            return;

    // Search for an unfriendly worm
    psAITarget = findTarget(gametype, teamgame, taggame, pcMap);

    
    // Any unfriendlies?
    if(psAITarget) {
        // We have an unfriendly target, so change to a 'move-to-target' state
        nAITargetType = AIT_WORM;
        nAIState = AI_MOVINGTOTARGET;
        //AI_InitMoveToTarget(pcMap);
		NEW_AI_CreatePath(pcMap);
        return;
    }
	else
		fLastShoot = 0;


    // If we're down on health (less than 80%) we should look for a health bonus
    if(iHealth < 80) {
        if(AI_FindHealth(pcMap))
            return;
    }


    /*
      If we get here that means we have nothing to do
    */


    //
    // Typically, high ground is safer. So if we don't have anything to do, lets up up
    //
    
    // Our target already on high ground?
    if(cPosTarget.y < pcMap->getGridHeight()*5 && nAIState == AI_MOVINGTOTARGET)  {

		// Nothing todo, so go find some health if we even slightly need it
		if(iHealth < 100) {
			if(AI_FindHealth(pcMap))
				return;
		}
        return;
	}

	// TODO:
	// If there's no target worm and bonuses are off, this is run every frame
	// The pathfinding is then run very often and it terribly slows down

    // Find a random spot to go to high in the level
    int x, y;
    for(y=1; y<5 && y<pcMap->getGridRows()-1; y++) {
        for(x=1; x<pcMap->getGridCols()-1; x++) {
            uchar pf = *(pcMap->getGridFlags() + y*pcMap->getGridCols() + x);

            if(pf & PX_ROCK)
                continue;

            // Set the target
            cPosTarget = CVec((float)(x*pcMap->getGridWidth()+(pcMap->getGridWidth()/2)), (float)(y*pcMap->getGridHeight()+(pcMap->getGridHeight()/2)));
            nAITargetType = AIT_POSITION;
            nAIState = AI_MOVINGTOTARGET;
            //AI_InitMoveToTarget(pcMap);
			NEW_AI_CreatePath(pcMap);
            goto break_whole; // year, i hate this, too ...
        }
    }
break_whole:

    /*int     x, y;
    int     px, py;
    bool    first = true;
    int     cols = pcMap->getGridCols()-1;       // Note: -1 because the grid is slightly larger than the
    int     rows = pcMap->getGridRows()-1;       // level size
    int     gw = pcMap->getGridWidth();
    int     gh = pcMap->getGridHeight();
	CVec	resultPos;

    
    // Find a random cell to start in
    px = (int)(fabs(GetRandomNum()) * (float)cols);
	py = (int)(fabs(GetRandomNum()) * (float)rows);

    x = px; y = py;
	bool breakme = false;

    // Start from the cell and go through until we get to an empty cell
    while(1) {
        while(1) {
            // If we're on the original starting cell, and it's not the first move we have checked all cells
            // and should leave
            if(!first) {
                if(px == x && py == y) {
                    resultPos = CVec((float)x*gw+gw/2, (float)y*gh+gh/2);
					breakme = true;
					break;
                }
            }
            first = false;

            uchar pf = *(pcMap->getGridFlags() + y*pcMap->getGridCols() + x);
            if(!(pf & PX_ROCK))  {
                resultPos = CVec((float)x*gw+gw/2, (float)y*gh+gh/2);
				breakme = true;
				break;
			}
            
            if(++x >= cols) {
                x=0;
                break;
            }
        }

		if (breakme)
			break;

        if(++y >= rows) {
            y=0;
            x=0;
        }
    }


   //cPosTarget = resultPos;
   nAITargetType = AIT_POSITION;
   nAIState = AI_MOVINGTOTARGET;
   AI_InitMoveToTarget(pcMap);*/

	return;
}


///////////////////
// Find a health pack
// Returns true if we found one
bool CWorm::AI_FindHealth(CMap *pcMap)
{
	if (!tGameInfo.iBonusesOn)
		return false;

    CBonus  *pcBonusList = cClient->getBonusList();
    int     i;
    bool    bFound = false;
    CBonus  *pcBonus = NULL;
    float   dist = 99999;

    // Find the closest health bonus
    for(i=0; i<MAX_BONUSES; i++) {
        if(pcBonusList[i].getUsed() && pcBonusList[i].getType() == BNS_HEALTH) {
            
            float d = CalculateDistance(pcBonusList[i].getPosition(), vPos);

            if(d < dist) {
                pcBonus = &pcBonusList[i];
                dist = d;
                bFound = true;
            }
        }
    }


    // TODO: Verify that the target is not in some small cavern that is hard to get to

    
    // If we have found a bonus, setup the state to move towards it
    if(bFound) {
        psBonusTarget = pcBonus;
        nAITargetType = AIT_BONUS;
        nAIState = AI_MOVINGTOTARGET;
        //AI_InitMoveToTarget(pcMap);
		NEW_AI_CreatePath(pcMap);
    }

    return bFound;
}


///////////////////
// Reloads the weapons
void CWorm::AI_ReloadWeapons(void)
{
    int     i;

    // Go through reloading the weapons
    for(i=0; i<5; i++) {
        if(tWeapons[i].Reloading) {
            iCurrentWeapon = i;
            break;
        }
    }
}


///////////////////
// Initialize a move-to-target
void CWorm::AI_InitMoveToTarget(CMap *pcMap)
{
    memset(pnOpenCloseGrid, 0, nGridCols*nGridRows*sizeof(int));

    // Cleanup the old path
    AI_CleanupPath(psPath);

    cPosTarget = AI_GetTargetPos();

    // Put the target into a cell position
    int tarX = (int) (cPosTarget.x / pcMap->getGridWidth());
    int tarY = (int) (cPosTarget.y / pcMap->getGridHeight());

    // Current position cell
    int curX = (int) (vPos.x / pcMap->getGridWidth());
    int curY = (int) (vPos.y / pcMap->getGridHeight());

    nPathStart[0] = curX;
    nPathStart[1] = curY;
    nPathEnd[0] = tarX;
    nPathEnd[1] = tarY;
   
    // Go down the path
    psPath = AI_ProcessNode(pcMap, NULL, curX,curY, tarX, tarY);
    psCurrentNode = psPath;

    fLastPathUpdate = tLX->fCurTime;
    
    // Draw the path
    if(!psPath)
        return;

#ifdef _AI_DEBUG
    pcMap->DEBUG_DrawPixelFlags();
    AI_DEBUG_DrawPath(pcMap,psPath);
#endif // _AI_DEBUG

}


///////////////////
// Path finding 
ai_node_t *CWorm::AI_ProcessNode(CMap *pcMap, ai_node_t *psParent, int curX, int curY, int tarX, int tarY)
{
    int     i;
    bool    bFound = false;

    // Bounds checking
    if(curX < 0 || curY < 0 || tarX < 0 || tarY < 0)
        return NULL;
    if(curX >= nGridCols || curY >= nGridRows || tarX >= nGridCols || tarY >= nGridRows)
        return NULL;

    // Are we on target?
    if(curX == tarX && curY == tarY)
        bFound = true;

    // Is this node impassable (rock)?
    uchar pf = *(pcMap->getGridFlags() + (curY*nGridCols) + curX);
    if(pf & PX_ROCK && !bFound) {
        if(psParent != NULL)  // Parent can be on rock
            return NULL;
    }

    int movecost = 1;
    // Dirt is a higher cost
    if(pf & PX_DIRT)
        movecost = 2;

    // Is this node closed?    
    if(pnOpenCloseGrid[curY*nGridCols+curX] == 1)
        return NULL;
    
    // Close it
    pnOpenCloseGrid[curY*nGridCols+curX] = 1;

    // Create a node
    ai_node_t *node = new ai_node_t;
    if(!node)
        return NULL;

    // Fill in the node details
    node->psParent = psParent;
    node->nX = curX;
    node->nY = curY;
    node->nCost = 0;
    node->nCount = 0;
    node->nFound = false;
    node->psPath = NULL;
    for(i=0; i<8; i++)
        node->psChildren[i] = NULL;
    if(psParent) {
        node->nCost = psParent->nCost+movecost;
        node->nCount = psParent->nCount+1;
    }

    // Are we on target?
    if(bFound) {
		tState.iJump = true;
        node->nFound = true;
        // Traverse back up the path
        return node;
    }

    /*DrawRectFill(pcMap->GetImage(), curX*pcMap->getGridWidth(), curY*pcMap->getGridHeight(), 
                                    curX*pcMap->getGridWidth()+pcMap->getGridWidth(),
                                    curY*pcMap->getGridHeight()+pcMap->getGridHeight(), MakeColour(0,0,255));*/


    //
    // TEST
    //

    int closest = -1;
    int cost = 99999;
    /*int norm[] = {-1,-1, 0,-1, 1,-1, 
                  -1,0,  1,0,
                  -1,1,  0,1, 1,1};
    for(i=0; i<8; i++) {
        node->psChildren[i] = AI_ProcessNode(pcMap, node, curX+norm[i*2], curY+norm[i*2+1], tarX, tarY);
        if(node->psChildren[i]) {
            if(node->psChildren[i]->nFound) {
                if(node->psChildren[i]->nCost < cost) {
                    cost = node->psChildren[i]->nCost;
                    closest = i;
                }
            }
        }
    }
    
    // Kill the children
    for(i=0; i<8; i++) {
        if(i != closest) {
            delete node->psChildren[i];
            node->psChildren[i] = NULL;
        }
    }
    
    // Found any path?
    if(closest == -1) {
        delete node;
        return NULL;
    }
    
    // Set the closest path
    node->psPath = node->psChildren[closest];
    node->nFound = true;
    return node;*/



    // Here we go down the children that are closest to the target
    int vertDir = 0;    // Middle
    int horDir = 0;     // Middle

    if(tarY < curY)
        vertDir = -1;   // Go upwards
    else if(tarY > curY)
        vertDir = 1;    // Go downwards
    if(tarX < curX)
        horDir = -1;    // Go left
    else if(tarX > curX)
        horDir = 1;     // Go right


    int diagCase[] = {horDir,vertDir,  horDir,0,  0,vertDir,  -horDir,vertDir,
                      horDir,-vertDir, -horDir,0, 0,-vertDir, -horDir,-vertDir};

    //
    // Diagonal target case
    //
    closest = -1;
    cost = 99999;
    if(horDir != 0 && vertDir != 0) {
        for(i=0; i<8; i++) {
            node->psChildren[i] = AI_ProcessNode(pcMap, node, curX+diagCase[i*2], curY+diagCase[i*2+1], tarX, tarY);
            if(node->psChildren[i]) {
                if(node->psChildren[i]->nFound) {
                    if(node->psChildren[i]->nCost < cost) {
                        cost = node->psChildren[i]->nCost;
                        closest = i;
                    }
                }
            }
        }

        // Kill the children
        for(i=0; i<8; i++) {
            if(i != closest) {
				if(node->psChildren[i]) {
					delete node->psChildren[i];
					node->psChildren[i] = NULL;
				}
            }
        }

        // Found any path?
        if(closest == -1) {
			if(node)
				delete node;
            return NULL;
        }

        // Set the closest path
        node->psPath = node->psChildren[closest];
        node->nFound = true;
        return node;
    }


    //
    // Straight target case
    //
    int upCase[] = {0,-1,  1,-1,  -1,-1,  1,0,  -1,0,  1,1,  -1,1,  0,1};
    int rtCase[] = {1,0,   1,1,   1,-1,   0,1,  0,-1,  -1,1, -1,-1, -1,0};
    
    closest = -1;
    cost = 99999;
    for(i=0; i<8; i++) {
        int h,v;
        // Up/Down
        if(horDir == 0) {
            h = upCase[i*2];
            v = upCase[i*2+1];
            if(vertDir == 1) {
                h = -h;
                v = -v;
            }
        }

        // Left/Right
        if(vertDir == 0) {
            h = rtCase[i*2];
            v = rtCase[i*2+1];
            if(horDir == -1) {
                h = -h;
                v = -v;
            }
        }

        node->psChildren[i] = AI_ProcessNode(pcMap, node, curX+h, curY+v, tarX, tarY);
        if(node->psChildren[i]) {
            if(node->psChildren[i]->nFound) {
                if(node->psChildren[i]->nCost < cost) {
                    cost = node->psChildren[i]->nCost;
                    closest = i;
                }
            }
        }
        
        // Kill the children
        for(i=0; i<8; i++) {
            if(i != closest) {
				if(node->psChildren[i]) {
					delete node->psChildren[i];
					node->psChildren[i] = NULL;
				}
            }
        }
        
        // Found any path?
        if(closest == -1) {
			if(node)
				delete node;
            return NULL;
        }
        
        // Set the closest path
        node->psPath = node->psChildren[closest];
        node->nFound = true;
        return node;
    }
    delete node;
    return NULL;
}


///////////////////
// Cleanup a path
void CWorm::AI_CleanupPath(ai_node_t *node)
{
    if(node) {

        AI_CleanupPath(node->psPath);
        delete node;
    }
}


///////////////////
// Move towards a target
void CWorm::AI_MoveToTarget(CMap *pcMap)
{
    int     nCurrentCell[2];
    int     nTargetCell[2];
    int     nFinalTarget[2];
    worm_state_t *ws = &tState;

    int     hgw = pcMap->getGridWidth()/2;
    int     hgh = pcMap->getGridHeight()/2;

	// Better target?
	CWorm *newtrg = findTarget(iAiGame, iAiTeams, iAiTag, pcMap);
	if (psAITarget && newtrg)
		if (newtrg->getID() != psAITarget->getID())
			nAIState = AI_THINK;

    // Clear the state
	ws->iCarve = false;
	ws->iMove = false;
	ws->iShoot = false;
	ws->iJump = false;

    cPosTarget = AI_GetTargetPos();

/*
    // this don't make sense here; what if there is a wall between them?
    // AI_CanShoot should make the ness checks, if we can shoot at the target
    // if not, move to the next node; the last node has direct access to the target
    // If we're really close to the target, perform a more precise type of movement
    if(fabs(vPos.x - cPosTarget.x) < 20 && fabs(vPos.y - cPosTarget.y) < 20) {
        AI_PreciseMove(pcMap);
        return;
    }
*/

    // If we're stuck, just get out of wherever we are
    if(bStuck) {
        ws->iMove = true;
        ws->iJump = true;
        cNinjaRope.Release();
        if(tLX->fCurTime - fStuckPause > 2)
            bStuck = false;
        return;
    }



    /*
      First, if the target or ourself has deviated from the path target & start by a large enough amount:
      Or, enough time has passed since the last path update:
      recalculate the path
    */
    int     i;
    int     nDeviation = 2;     // Maximum deviation allowance
    bool    recalculate = false;

    
    // Cell positions
    nTargetCell[0] = (int)cPosTarget.x / pcMap->getGridWidth();
    nTargetCell[1] = (int)cPosTarget.y / pcMap->getGridHeight();
    nCurrentCell[0] = (int)vPos.x / pcMap->getGridWidth();
    nCurrentCell[1] = (int)vPos.y / pcMap->getGridHeight();
    nFinalTarget[0] = (int)cPosTarget.x / pcMap->getGridWidth();
    nFinalTarget[1] = (int)cPosTarget.y / pcMap->getGridHeight();

    for(i=0 ;i<2; i++) {
        if(abs(nPathStart[i] - nCurrentCell[i]) > nDeviation ||
            abs(nPathEnd[i] - nTargetCell[i]) > nDeviation) {
            recalculate = true;
        }
    }

    if(tLX->fCurTime - fLastPathUpdate > 3)
        recalculate = true;

    // Re-calculate the path?
    if(recalculate)
        AI_InitMoveToTarget(pcMap);

    
    // We carve every few milliseconds so we don't go too fast
	if(tLX->fCurTime - fLastCarve > 0.35f) {
		fLastCarve = tLX->fCurTime;
		ws->iCarve = true;
	}


    /*
      Move through the path.
      We have a current node that we must get to. If we go onto the node, we go to the next node, and so on.
      Because the path finding isn't perfect we can sometimes skip a whole group of nodes.
      If we are close to other nodes that are _ahead_ in the list, we can skip straight to that node.
    */

    int     nNodeProx = 3;      // Maximum proximity to a node to skip to

    if(psCurrentNode == NULL || psPath == NULL) {
        #ifdef _AI_DEBUG
          pcMap->DEBUG_DrawPixelFlags();
          AI_DEBUG_DrawPath(pcMap, psPath);
		#endif // _AI_DEBUG

        // If we don't have a path, resort to simpler AI methods
        AI_SimpleMove(pcMap,psAITarget != NULL);
        return;
    }

    float xd = (float)(nCurrentCell[0] - psCurrentNode->nX);
    float yd = (float)(nCurrentCell[1] - psCurrentNode->nY);
    float fCurDist = fastSQRT(xd*xd + yd*yd);
    
    // Go through the path
    ai_node_t *node = psPath;    
    for(; node; node=node->psPath) {

        // Don't go back down the list
        if(node->nCount <= psCurrentNode->nCount)
            continue;

         // If the node is blocked, don't skip ahead
        float tdist;
        int type;
        traceLine(CVec((float)(node->nX*pcMap->getGridWidth()+hgw), (float) (node->nY*pcMap->getGridHeight()+hgh)), pcMap, &tdist, &type,1);
        if(tdist < 0.75f && (type & PX_ROCK))
            continue;

        // If we are near the node skip ahead to it
        if(abs(node->nX - nCurrentCell[0]) < nNodeProx &&
           abs(node->nY - nCurrentCell[1]) < nNodeProx) {

            psCurrentNode = node;

			#ifdef _AI_DEBUG
				pcMap->DEBUG_DrawPixelFlags();
				AI_DEBUG_DrawPath(pcMap, node);
			#endif  // _AI_DEBUG
            continue;
        }

        // If we're closer to this node than to our current node, skip to this node
        xd = (float)(nCurrentCell[0] - node->nX);
        yd = (float)(nCurrentCell[1] - node->nY);
        float dist = fastSQRT(xd*xd + yd*yd);

        if(dist < fCurDist) {
            psCurrentNode = node;
			#ifdef _AI_DEBUG
				pcMap->DEBUG_DrawPixelFlags();
				AI_DEBUG_DrawPath(pcMap, node);
			#endif  // _AI_DEBUG
        }
    }

    if(psCurrentNode == NULL || psPath == NULL) {
         // If we don't have a path, resort to simpler AI methods
        AI_SimpleMove(pcMap);
        return;
    }

    nTargetCell[0] = psCurrentNode->nX;
    nTargetCell[1] = psCurrentNode->nY;


    /*
      We get the 5 next nodes in the list and generate an average position to get to
    */
    int avgCell[2] = {0,0};
    node = psCurrentNode;    
    for(i=0; node && i<5; node=node->psPath, i++) {
        avgCell[0] += node->nX;
        avgCell[1] += node->nY;
    }
    if(i != 0) {
    	avgCell[0] /= i;
    	avgCell[1] /= i;
    }
    //nTargetCell[0] = avgCell[0];
    //nTargetCell[1] = avgCell[1];
    
    tLX->debug_pos = CVec((float)(nTargetCell[0]*pcMap->getGridWidth()+hgw), (float)(nTargetCell[1]*pcMap->getGridHeight()+hgh));



    /*
      Now that we've done all the boring stuff, our single job here is to reach the node.
      We have walking, jumping, move-through-air, and a ninja rope to help us.
    */
    int     nRopeHeight = 3;                // Minimum distance to release rope    


    // Aim at the node
    bool aim = AI_SetAim(CVec((float)(nTargetCell[0]*pcMap->getGridWidth()+hgw), (float)(nTargetCell[1]*pcMap->getGridHeight()+hgh)));

    // If we are stuck in the same position for a while, take measures to get out of being stuck
    if(fabs(cStuckPos.x - vPos.x) < 25 && fabs(cStuckPos.y - vPos.y) < 25) {
        fStuckTime += tLX->fDeltaTime;

        // Have we been stuck for a few seconds?
        if(fStuckTime > 4/* && !ws->iJump && !ws->iMove*/) {
            // Jump, move, switch directions and release the ninja rope
            ws->iJump = true;
            ws->iMove = true;
            bStuck = true;
            fStuckPause = tLX->fCurTime;
            cNinjaRope.Release();

			iDirection = !iDirection;
            
            fAngle -= 45;
            // Clamp the angle
	        fAngle = MIN((float)60,fAngle);
	        fAngle = MAX((float)-90,fAngle);

            // Recalculate the path
            AI_InitMoveToTarget(pcMap);
            fStuckTime = 0;
        }
    }
    else {
        bStuck = false;
        fStuckTime = 0;
        cStuckPos = vPos;
    }


    // If the ninja rope hook is falling, release it & walk
    if(!cNinjaRope.isAttached() && !cNinjaRope.isShooting()) {
        cNinjaRope.Release();
        ws->iMove = true;
    }

    // Walk only if the target is a good distance on either side
    if(abs(nCurrentCell[0] - nTargetCell[0]) > 3)
        ws->iMove = true;

    
    // If the node is above us by a lot, we should use the ninja rope
    if(nTargetCell[1] < nCurrentCell[1]) {
        
        // If we're aimed at the point, just leave and it'll happen soon
        if(!aim)
            return;

        bool fireNinja = true;

        CVec dir;
        dir.x=( (float)cos(fAngle * (PI/180)) );
	    dir.y=( (float)sin(fAngle * (PI/180)) );
	    if(iDirection == DIR_LEFT)
		    dir.x=(-dir.x);
       
        /*
          Got aim, so shoot a ninja rope
          We shoot a ninja rope if it isn't shot
          Or if it is, we make sure it has pulled us up and that it is attached
        */
        if(fireNinja) {
            if(!cNinjaRope.isReleased())
                cNinjaRope.Shoot(vPos,dir);
            else {
                float length = CalculateDistance(vPos, cNinjaRope.getHookPos());
                if(cNinjaRope.isAttached()) {
                    if(length < cNinjaRope.getRestLength() && vVelocity.y<-10)
                        cNinjaRope.Shoot(vPos,dir);
                }
            }
        }
    }    


    /*
      If there is dirt between us and the next node, don't shoot a ninja rope
      Instead, carve or use some clearing weapon
    */
    float traceDist = -1;
    int type = 0;
	if (!psCurrentNode)
		return;
    CVec v = CVec((float)(psCurrentNode->nX*pcMap->getGridWidth()+hgw), (float)(psCurrentNode->nY*pcMap->getGridHeight()+hgh));
    int length = traceLine(v, pcMap, &traceDist, &type);
    float dist = CalculateDistance(v, vPos);
    if((float)length <= dist && (type & PX_DIRT)) {
        ws->iJump = true;
        ws->iMove = true;
		ws->iCarve = true; // Carve
        
        // Shoot a path
        /*int wpn;
        if((wpn = AI_FindClearingWeapon()) != -1) {
            iCurrentWeapon = wpn;
            ws->iShoot = true;
            cNinjaRope.Release();
        }*/
    }



    // If we're above the node, let go of the rope and move towards to node
    if(nTargetCell[1] >= nCurrentCell[1]+nRopeHeight) {
        // Let go of any rope
        cNinjaRope.Release();

        // Walk in the direction of the node
        ws->iMove = true;
    }

    if(nTargetCell[1] >= nCurrentCell[1]) {

		if(!IsEmpty(CELL_DOWN,pcMap))  {
			if(IsEmpty(CELL_LEFT,pcMap))
				iDirection = DIR_LEFT;
			else if (IsEmpty(CELL_RIGHT,pcMap))
				iDirection = DIR_RIGHT;
		}

        // Walk in the direction of the node
        ws->iMove = true;
    }


    // If we're near the height of the final target, release the rope
    if(abs(nFinalTarget[0] - nCurrentCell[0]) < 2 && abs(nFinalTarget[1] - nCurrentCell[1]) < 2) {
        // Let go of any rope
        cNinjaRope.Release();

        // Walk in the direction of the target
        ws->iMove = true;

        // If we're under the final target, jump
        if(nFinalTarget[1] < nCurrentCell[1])
            ws->iJump = true;
    }
    


}


///////////////////
// DEBUG: Draw the path
void CWorm::AI_DEBUG_DrawPath(CMap *pcMap, ai_node_t *node)
{
	return;

    if(!node)
        return;

    int x = node->nX * pcMap->getGridWidth();
    int y = node->nY * pcMap->getGridHeight();
    
    //DrawRectFill(pcMap->GetImage(), x,y, x+pcMap->getGridWidth(), y+pcMap->getGridHeight(), 0);
    
    if(node->psPath) {
        int cx = node->psPath->nX * pcMap->getGridWidth();
        int cy = node->psPath->nY * pcMap->getGridHeight();
        DrawLine(pcMap->GetDrawImage(), (x+pcMap->getGridWidth()/2)*2, (y+pcMap->getGridHeight()/2)*2, 
                                    (cx+pcMap->getGridWidth()/2)*2,(cy+pcMap->getGridHeight()/2)*2,
                                    0xffff);
    }
    else {
        // Final target
        DrawRectFill(pcMap->GetDrawImage(), x*2-5,y*2-5,x*2+5,y*2+5, MakeColour(0,255,0));

    }

    AI_DEBUG_DrawPath(pcMap, node->psPath);
}


///////////////////
// Get the target's position
// Also checks the target and resets to a think state if needed
CVec CWorm::AI_GetTargetPos(void)
{
    // Put the target into a position
    switch(nAITargetType) {

        // Bonus target
        case AIT_BONUS:
            if(psBonusTarget) {
                if(!psBonusTarget->getUsed())
                    nAIState = AI_THINK;
                return psBonusTarget->getPosition();
            }
            break;

        // Worm target
        case AIT_WORM:
            if(psAITarget) {
                if(!psAITarget->getAlive() || !psAITarget->isUsed())
                    nAIState = AI_THINK;
                return psAITarget->getPos();
            }
            break;

        // Position target
        case AIT_POSITION:
            return cPosTarget;
    }

    // No target
    nAIState = AI_THINK;
    return CVec(0,0);
}


///////////////////
// Aim at a spot
// Returns true if we're aiming at it
bool CWorm::AI_SetAim(CVec cPos)
{
    float   dt = tLX->fDeltaTime;
    CVec	tgPos = cPos;
	CVec	tgDir = tgPos - vPos;
    bool    goodAim = false;
    gs_worm_t *wd = cGameScript->getWorm();

	float   fDistance = NormalizeVector(&tgDir);
	
	if (tLX->fCurTime - fLastFace > 0.5f)  {  // prevent turning
	// Make me face the target
		if(tgPos.x > vPos.x)
			iDirection = DIR_RIGHT;
		else
			iDirection = DIR_LEFT;

		fLastFace = tLX->fCurTime;
	}

	// Aim at the target
	float ang = (float)atan2(tgDir.x, tgDir.y);
	ang = RAD2DEG(ang);

	if(iDirection == DIR_LEFT)
		ang+=90;
	else
		ang = -ang + 90;

	// Clamp the angle
	ang = MAX((float)-90, ang);

/*
	printf("AI_SetAim\n");
	printf("   ang = %f\n", ang);
	printf("   fAngle = %f\n", fAngle);
*/

	// If the angle is within +/- 3 degrees, just snap it
    if( fabs(fAngle - ang) < 3) {
		fAngle = ang;
        goodAim = true;
    }

	// Move the angle at the same speed humans are allowed to move the angle
	if(ang > fAngle)
		fAngle += wd->AngleSpeed * dt;
	else if(ang < fAngle)
		fAngle -= wd->AngleSpeed * dt;

	// If the angle is within +/- 3 degrees, just snap it
    if( fabs(fAngle - ang) < 3) {
		fAngle = ang;
        goodAim = true;
    }

	// Clamp the angle
	fAngle = MIN((float)60,fAngle);
	fAngle = MAX((float)-90,fAngle);

    return goodAim;
}


///////////////////
// A simpler method to get to a target
// Used if we have no path
 // TODO: this is global for all worms (which is not what is wanted)
float fLastJump = 0;  // Time when we last tried to jump
void CWorm::AI_SimpleMove(CMap *pcMap, bool bHaveTarget)
{
    worm_state_t *ws = &tState;

    // Simple    
	ws->iMove = true;
	ws->iShoot = false;
	ws->iJump = false;

    //strcpy(tLX->debug_string, "AI_SimpleMove invoked");

    cPosTarget = AI_GetTargetPos();
    
    // Aim at the node
    bool aim = AI_SetAim(cPosTarget);

    // If our line is blocked, try some evasive measures
    float fDist = 0;
    int type = 0;
    int nLength = traceLine(cPosTarget, pcMap, &fDist, &type, 1);
    if(fDist < 0.75f || cPosTarget.y < vPos.y) {

        // Change direction
		if (bHaveTarget && (tLX->fCurTime-fLastTurn) > 1.0)  {
			iDirection = !iDirection;
			fLastTurn = tLX->fCurTime;
		}

        // Look up for a ninja throw
        aim = AI_SetAim(vPos+CVec(GetRandomNum()*10,GetRandomNum()*10+10));
        if(aim) {
            CVec dir;
            dir.x=( (float)cos(fAngle * (PI/180)) );
	        dir.y=( (float)sin(fAngle * (PI/180)) );
	        if(iDirection==DIR_LEFT)
		        dir.x=(-dir.x);

            cNinjaRope.Shoot(vPos,dir);
        }

		// Jump and move
		else  {
			if (tLX->fCurTime-fLastJump > 3.0)  {
				ws->iJump = true;
				fLastJump = tLX->fCurTime;
			}
			ws->iMove = true;
			cNinjaRope.Release();
		}

        return;
    }

    // Release the ninja rope
    cNinjaRope.Release();
}

float fLastDirChange = 99999;
///////////////////
// Perform a precise movement
void CWorm::AI_PreciseMove(CMap *pcMap)
{
    worm_state_t *ws = &tState;

    //strcpy(tLX->debug_string, "AI invoked");

    ws->iJump = false;
    ws->iMove = false;
    ws->iCarve = false;
    
 
    // If we're insanely close, just stop
    if(fabs(vPos.x - cPosTarget.x) < 10 && fabs(vPos.y - cPosTarget.y) < 10) {
 		/*if (tLX->fCurTime - fLastDirChange > 2.0f)  {
			if (GetRandomNum() < 0)
				iDirection = DIR_LEFT;
			else
				iDirection = DIR_RIGHT;
			fLastDirChange = tLX->fCurTime;
		}
		ws->iMove = true; */
   
        return;
    }

    // Aim at the target
    //bool aim = AI_SetAim(cPosTarget);
	bool aim = AI_CanShoot(pcMap,iAiGame);
    if(aim) {
        // Walk towards the target
        ws->iMove = true;

        // If the target is above us, jump
        if(fabs(vPos.x - cPosTarget.x) < 10 && vPos.y - cPosTarget.y > 5)
            ws->iJump = true;
    } else  {
		ws->iJump = true;
		// Randomly change direction
		if (tLX->fCurTime - fLastDirChange > 2.0f)  {
			if (GetRandomNum() < 0)
				iDirection = DIR_LEFT;
			else
				iDirection = DIR_RIGHT;
			fLastDirChange = tLX->fCurTime;
		}
		ws->iMove = true;
	}
}


///////////////////
// Finds a suitable 'clearing' weapon
// A weapon used for making a path
// Returns -1 on failure
int CWorm::AI_FindClearingWeapon(void)
{
	if(iAiGameType == GAM_MORTARS)
		return -1;
	int type = PJ_EXPLODE;
	
	// search a good projectile weapon
	int i = 0;
    for (i=0; i<5; i++) {
    	if(tWeapons[i].Weapon->Type == WPN_PROJECTILE) {
			// TODO: not really all cases...
			type = tWeapons[i].Weapon->Projectile->Hit_Type;

			// Nothing that could fall back onto us
			if (tWeapons[i].Weapon->ProjSpeed < 100.0f) {
				if (!tWeapons[i].Weapon->Projectile->UseCustomGravity || tWeapons[i].Weapon->Projectile->Gravity > 30)
					continue;
			}

			// Suspicious
			char name[256];
			strcpy(name,tWeapons[i].Weapon->Name);
			strlwr(name);
			if (strstr("dirt",name) || strstr("napalm",name) || strstr("grenade",name) || strstr("nuke",name) || strstr("mine",name))
				continue;

			// Nothing explosive or dirty
			if (type != PJ_DIRT && type != PJ_GREENDIRT)
				if(!tWeapons[i].Reloading)
					return i;
		}
	}

	// accept also beam-weapons as a second choice
    for (i=0; i<5; i++)
 		if(tWeapons[i].Weapon->Type == WPN_BEAM)
			return i;

    // No suitable weapons
    return -1;
}



///////////////////
// Can we shoot the target
bool CWorm::AI_CanShoot(CMap *pcMap, int nGameType)
{

    // Make sure the target is a worm
    if(nAITargetType != AIT_WORM)
        return false;
    
    // Make sure the worm is good
    if(!psAITarget)
        return false;
    if(!psAITarget->getAlive() || !psAITarget->isUsed())
        return false;
    

    CVec    cTrgPos = psAITarget->getPos();
    bool    bDirect = true;


    /*
      Here we check if we have a line of sight with the target.
      If we do, and our 'direct' firing weapons are loaded, we shoot.

      If the line of sight is blocked, we use 'indirect' shooting methods.
      We have to be careful with indirect, because if we're in a confined space, or the target is above us
      we shouldn't shoot at the target

      The aim of the game is killing worms, so we put a higher priority on shooting rather than
      thinking tactically to prevent injury to self

	  If we play a mortar game, make sure there's enough free space to shoot - avoid suicides
    */


    // If the target is too far away we can't shoot at all (but not when a rifle game)
    float d = CalculateDistance(cTrgPos, vPos);
    if(d > 300.0f && iAiGameType != GAM_RIFLES)
        return false;

/*	// If we're on the target, simply shoot // this doesn't work in every case
	if(d < 10.0f)
		return true;
*/

    float fDist;
    int nType = -1;
    int length = 0;

	length = traceWeaponLine(cTrgPos, pcMap, &fDist, &nType);

    // If target is blocked by rock we can't use direct firing
    if(nType & PX_ROCK)  {
		return false;
	}

	// Don't shoot teammates
	if(tGameInfo.iGameMode == GMT_TEAMDEATH && nType == PX_WORM)
		return false;

	// If target is blocked by large amount of dirt, we can't shoot it with rifle
	if (nType & PX_DIRT)  {
		if(d-fDist > 40.0f)
			return false;
	}

	// In mortar game there must be enough of free cells around us
	if (iAiGameType == GAM_MORTARS)  {
		if (!NEW_AI_CheckFreeCells(1,pcMap))  {
			return false;
		}
		if (!traceWormLine(cTrgPos,vPos,pcMap))
			return false;
	}

	// If our velocity is big and we shoot in the direction of the flight, we can suicide
	// We will avoid this here
	/*if (vVelocity.y > 30 && fAngle >= 50)
		return false;
	if (vVelocity.y < -30 && fAngle <= 10)
		return false;
	if (vVelocity.x < -30 && iDirection == DIR_LEFT && fAngle > 20)
		return false;
	if (vVelocity.x > 30 && iDirection == DIR_RIGHT && fAngle > 20)
		return false;*/

    // Set the best weapon for the situation
    // If there is no good weapon, we can't shoot
    tLX->debug_float = d;
    int wpn = AI_GetBestWeapon(nGameType, d, bDirect, pcMap, fDist);
    if(wpn == -1) {
        //strcpy(tLX->debug_string, "No good weapon");
        return false;
    }     

    iCurrentWeapon = wpn;

    // Shoot
    return true;
}

////////////////////
// Returns true, if the weapon can hit the target
bool CWorm::weaponCanHit(int gravity,float speed,CMap *pcMap)
{
	// Get the target position
	if(!psAITarget)
		return false;
    CVec cTrgPos = psAITarget->getPos();

	CVec *from = &vPos;
	CVec *to = &cTrgPos;

	// Convert the alpha to radians
	float alpha = DEG2RAD(fAngle);
	// Get the maximal X
	int max_x = (int)(to->x-from->x);
	// If we're in the X coordinate of the target, we can shoot (else we wouldn't be called)
	if (max_x == 0)
		return true;
	// Get the maximal Y
	int max_y = (int)(from->y-to->y);

	/*if (max_y > 0)
		alpha = 180-alpha;*/

	// Check the pixels in the projectile trajectory
	int x,y;

#ifdef _AI_DEBUG
	//DrawRectFill(pcMap->GetDebugImage(),0,0,pcMap->GetDebugImage()->w,pcMap->GetDebugImage()->h,MakeColour(255,0,255));
#endif


	int tmp;
	if (max_x > 0)  {
		for (x=0;x<max_x;x++)  {
			tmp = (int)(2*speed*speed*cos(alpha)*cos(alpha));
			if(tmp != 0)
				y = -x*(int)tan(alpha)+(gravity*x*x)/tmp;
			else
				y = 0;
			// If we have reached the target, the trajectory is free
			if (max_y < 0)  {
				if (y < max_y)
					return true;
			} else  {
				if (y > max_y)
					return true;
			}

			// Rock or dirt, trajectory not free
			if (pcMap->GetPixelFlag(x+(int)from->x,y+(int)from->y) & PX_ROCK)  {
				return false;
			}

			if (pcMap->GetPixelFlag(x+(int)from->x,y+(int)from->y) & PX_DIRT)  {
				return false;
			}

	#ifdef _AI_DEBUG
			//PutPixel(pcMap->GetDebugImage(),x*2+(int)from->x*2,y*2+(int)from->y*2,0xffff);
	#endif
		}
	}
	else  {
		for (x=0;x>max_x;x--)  {
			tmp = (int)(2*speed*speed*cos(alpha)*cos(alpha));
			if(tmp != 0)
				y = -x*(int)tan(alpha)+(gravity*x*x)/tmp;
			else
				y = 0;
			// If we have reached the target, the trajectory is free
			if (max_y < 0)  {
				if (y < max_y)
					return true;
			} else  {
				if (y > max_y)
					return true;
			}

			// Rock, trajectory not free
			if (pcMap->GetPixelFlag(x+(int)from->x,y+(int)from->y) & PX_ROCK)  {
				return false;
			}

	#ifdef _AI_DEBUG
			//PutPixel(pcMap->GetDebugImage(),x*2+(int)from->x*2,y*2+(int)from->y*2,0xffff);
	#endif
		}
	}

	// Target reached
	return true;
}


bool AI_GetAimingAngle(float v, int g, float x, float y, float *angle)
{

	// The target is too far to aim
	float vy = v*(float)sin(PI/4);
	float d = 2*vy*vy/g;
	if (fabs(x) >= d)  {
		*angle = 0;
		return false;
	}

	float v2 = v*v;
	float x2 = x*x;
	float g2 = (float)(g*g);
	float y2 = y*y;

	// Small hack - for small y-distance we want positive numbers
	if (fabs(y) < 3)  {
		if (y <= 0)
			y = -y;
	}

	float tmp1 = (float)fabs(-2*v2*y*g-g2*x2+v2*v2);

	// Validity check
	if (tmp1 < 0)  {
		*angle = 0;
		return false;
	}

	float tmp2 = -x2*(float)sqrt(tmp1)+x2*y*g+v2*(x2+2*y2);

	// Validity check
	if (tmp2 < 0)  {
		*angle = 0;
		return false;
	}

	float tmp3 = (float)sqrt(tmp1)-y*g+v2;

	// Validity check
	if (tmp3 <= 0 || x == 0)  {
		*angle = 60;
		return false;
	}

	// Get the angle
	*angle = (float)atan((float)sqrt(tmp2)/(x*(float)sqrt(tmp3)));
	if (x < 0)
		*angle = -(*angle);
	if (y > 0)
		*angle = -(*angle);

	// Convert to degrees
	*angle *= R2D;

	// Clamp the angle
	if (*angle > 60)  {
		*angle = 60;
		return false;
	}
	if (*angle < -90)  {
		*angle = -90;
		return false;
	}

	// Ok!
	return true;

}

///////////////////
// Shoot!
bool CWorm::AI_Shoot(CMap *pcMap)
{
	if(!psAITarget)
		return false;
    CVec    cTrgPos = psAITarget->getPos();

    //
    // Aim at the target
    //
    bool    bAim = false;//AI_SetAim(cTrgPos);
	
	// Distance
	float x = (cTrgPos.x-vPos.x);
	float y = (vPos.y-cTrgPos.y);
	
	float alpha = 0;

	gs_worm_t *wd = cGameScript->getWorm();
	if (!wd)
		return false;

    // Aim in the right direction to account of weapon speed, gravity and worm velocity
	weapon_t *weap = getCurWeapon()->Weapon;
	switch (weap->Type)  {
	case WPN_BEAM:
		// Direct aim
		bAim = AI_SetAim(cTrgPos);
		//printf("wp type is BEAM %s\n", bAim ? "and we are aiming" : "and no aim");
		break;
	case WPN_PROJECTILE:  {
		switch (weap->Projectile->Hit_Type)  {
		//case PJ_NOTHING:
		//case PJ_CARVE: 
		case PJ_DIRT:
		case PJ_GREENDIRT:
			//printf("hit_type is %i\n", weap->Projectile->PlyHit_Type);
			break;
		default:
			// TODO: count with the worm velocities

			// Worm speed
			float MySpeed = vVelocity.GetLength();
			// Enemy speed
			float EnemySpeed = psAITarget->getVelocity()->GetLength();
			// Projectile speed
			float v = (float)weap->ProjSpeed*weap->Projectile->Dampening;
			// Gravity
			int	  g = 100;
			if (weap->Projectile->UseCustomGravity)
				g = weap->Projectile->Gravity;
			if (iAiGameType == GAM_MORTARS)
				g = 100;

			// Get the alpha
			bAim = AI_GetAimingAngle(v,g,x,y,&alpha);
			if (!bAim)
				break;
			
			// Can we hit the target?
			if (g <= 5)  {
				int type = PX_EMPTY;
				float d;
				traceWeaponLine(cTrgPos,pcMap,&d,&type);
				bAim = type == PX_EMPTY;
			}
			else
				bAim = weaponCanHit(g,v,pcMap);
			if (!bAim)
				break;

			if (fabs(fAngle-alpha) > 5.0)  {
				// Move the angle at the same speed humans are allowed to move the angle
				if(alpha > fAngle)
					fAngle += wd->AngleSpeed * tLX->fDeltaTime;
				else if(alpha < fAngle)
					fAngle -= wd->AngleSpeed * tLX->fDeltaTime;
			}
			else
				fAngle = alpha;

			// Face the target
			if (x < 0)
				iDirection = DIR_LEFT;
			else
				iDirection = DIR_RIGHT;

			/*strcpy(tLX->debug_string,weap->Name);
			if (tLX->fCurTime-flast > 1.0f)  {
				tLX->debug_float = alpha;
				flast = tLX->fCurTime;
			}*/
			break;
		} // switch over Hit_Type
		} // case WPN_PROJECTILE
		break;
	default: // weap->Type
		// it's WPN_SPECIAL (should)
		// don't know, but this could possible be good
		// or else, this is perhaps some kamikaze action :)
		bAim = true;
	}

    if(!bAim)  {

		// In mortars we can hit the target below us
		if (iAiGameType == GAM_MORTARS)  {
			if (cTrgPos.y > (vPos.y-20.0f)) {
				// aim
				if (fabs(fAngle-alpha) > 5.0)  {
					// Move the angle at the same speed humans are allowed to move the angle
					if(alpha > fAngle)
						fAngle += wd->AngleSpeed * tLX->fDeltaTime;
					else if(alpha < fAngle)
						fAngle -= wd->AngleSpeed * tLX->fDeltaTime;
				}
				else
					fAngle = alpha;
					
				// Face the target
				if (x < 0)
					iDirection = DIR_LEFT;
				else
					iDirection = DIR_RIGHT;
					
				tState.iShoot = true;			
				
				return true;
			}
		}

  		// we cannot shoot here

		fBadAimTime += tLX->fDeltaTime;
		if((fBadAimTime) > 4) {
			if(IsEmpty(CELL_UP,pcMap))
				tState.iJump = true;
			fBadAimTime = 0;
		}

        return false;
	}

	fBadAimTime = 0;

    // Shoot
	tState.iShoot = true;
	fLastShoot = tLX->fCurTime;
	return true;
}


///////////////////
// AI: Get the best weapon for the situation
// Returns weapon id or -1 if no weapon is suitable for the situation
int CWorm::AI_GetBestWeapon(int nGameType, float fDistance, bool bDirect, CMap *pcMap, float fTraceDist)
{
	// if we are to close to the target, don't selct any weapon (=> move away)
	if(fDistance < 5)
		return -1;

    // We need to wait a certain time before we change weapon
    if( tLX->fCurTime - fLastWeaponChange > 0.15f )
        fLastWeaponChange = tLX->fCurTime;
    else
        return iCurrentWeapon;

	// For rifles and mortars just get the first unreloaded weapon
	if (iAiGameType == GAM_RIFLES || iAiGameType == GAM_MORTARS)  {
		for (int i=0; i<5; i++)
			if (!tWeapons[i].Reloading)  
				return i;
		return -1;
	}

    CVec    cTrgPos = AI_GetTargetPos();


	if (iAiGameType == GAM_100LT)  {
		// We're above the worm

		// If we are close enough, shoot the napalm
		if (vPos.y <= cTrgPos.y && CalculateDistance(vPos,cTrgPos) < 100.0f)  {
			if (traceWormLine(cTrgPos,vPos,pcMap) && !tWeapons[1].Reloading)
				if (psAITarget)
					if (psAITarget->CheckOnGround(pcMap))
						return 1;
		}



		float d = CalculateDistance(vPos,cTrgPos);
		// We're close to the target
		if (d < 50.0f)  {
			// We see the target
			if(bDirect)  {
				// Super shotgun
				if (!tWeapons[0].Reloading)
					return 0;

				// Chaingun
				if (!tWeapons[4].Reloading)  
					return 4;
				

				// Doomsday
				if (!tWeapons[3].Reloading)
					return 3;

				// Let's try cannon
				if (!tWeapons[2].Reloading)
				// Don't use cannon when we're on ninja rope, we will avoid suicides
					if (!cNinjaRope.isAttached())  {
						tState.iMove = false;  // Don't move, avoid suicides
						return 2;
					}
			}
			// We don't see the target
			else  {
				tState.iJump = true; // Jump, we might get better position
				return -1;
			}
		}

		// Not close, not far
		if (d > 50.0f && d<=300.0f)  {
			if (bDirect)  {

				// Chaingun is the best weapon for this situation
				if (!tWeapons[4].Reloading)  {
					return 4;
				}

				// Let's try cannon
				if (!tWeapons[2].Reloading)
					// Don't use cannon when we're on ninja rope, we will avoid suicides
					if (!cNinjaRope.isReleased())  {
						tState.iMove = false;  // Don't move, avoid suicides
						return 2;
					}

				// Super Shotgun makes it sure
				if (!tWeapons[0].Reloading)
					return 0;

				// As for almost last, try doomsday
				if (!tWeapons[3].Reloading)
					// Don't use doomsday when we're on ninja rope, we will avoid suicides
					if (!cNinjaRope.isAttached())  {
						tState.iMove = false;  // Don't move, avoid suicides
						return 3;
					}
			} // End of direct shooting weaps

			//return -1;
		}

		// Quite far
		if (d > 300.0f && bDirect)  {

			// First try doomsday
			if (!tWeapons[3].Reloading)  {
				// Don't use doomsday when we're on ninja rope, we will avoid suicides
				if (!cNinjaRope.isAttached())  {
					tState.iMove = false;  // Don't move, avoid suicides
					return 3;
				}
			}

			// Super Shotgun
			if (!tWeapons[0].Reloading)
				return 0;

			// Chaingun
			if (!tWeapons[4].Reloading)
				return 4;

			// Cannon, the worst possible for this
			if (!tWeapons[2].Reloading)
				// Don't use cannon when we're on ninja rope, we will avoid suicides
				if (!cNinjaRope.isReleased())  {
					// Aim a bit up
					// AI_SetAim(CVec(cTrgPos.x,cTrgPos.y+5.0f)); // don't do aiming here
					tState.iMove = false;  // Don't move, avoid suicides
					return 2;
				}
		}
					
		//return -1;

	}

    /*
    0: "minigun"
	1: "shotgun"
	2: "chiquita bomb"
	3: "blaster"
	4: "big nuke"
    */


    /*
    
      Special firing cases
    
    */

    //
    // Case 1: The target is on the bottom of the level, a perfect spot to lob an indirect weapon
    //
    if(cTrgPos.y > pcMap->GetHeight()-50 && fDistance < 200) {
		for (int i=0; i<5; i++)
			if (!tWeapons[i].Reloading)  
				if (tWeapons[i].Weapon->Type == WPN_PROJECTILE)
					if (tWeapons[i].Weapon->Projectile->Hit_Type == PJ_EXPLODE)
						return i;
    }



    /*
    
      Direct firing weapons
    
    */

    //
    // If we're close, use some beam or projectile weapon
    //
    if(fDistance < 100 && bDirect) {
        // First try beam
		int i;
		for (i=0; i<5; i++)
			if (!tWeapons[i].Reloading)  
				if (tWeapons[i].Weapon->Type == WPN_BEAM)
					return i;

		// If beam not available, try projectile
		for (i=0; i<5; i++)
			if (!tWeapons[i].Reloading)  
				if (tWeapons[i].Weapon->Type == WPN_PROJECTILE)
					if( tWeapons[i].Weapon->Projectile->Hit_Type != PJ_DIRT
					&& tWeapons[i].Weapon->Projectile->Hit_Type != PJ_GREENDIRT)
					//if (tWeapons[i].Weapon->Projectile->Type == PRJ_PIXEL)
					return i;
 		
 		// don't return here, try selection by other, not optimal fitting cases
   }


    //
    // If we're at a medium distance, use any weapon, but prefer the exact ones
    //
    if(fDistance < 150 && bDirect) {

		// First try beam
		int i;
		for (i=0; i<5; i++)
			if (!tWeapons[i].Reloading)  
				if (tWeapons[i].Weapon->Type == WPN_BEAM)
					return i;

		// If beam not available, try projectile
		for (i=0; i<5; i++)
			if (!tWeapons[i].Reloading)  
				if (tWeapons[i].Weapon->Type == WPN_PROJECTILE)
					if( tWeapons[i].Weapon->Projectile->Hit_Type != PJ_DIRT
					&& tWeapons[i].Weapon->Projectile->Hit_Type != PJ_GREENDIRT)
					/*if (tWeapons[i].Weapon->Projectile->Type == PRJ_PIXEL || tWeapons[i].Weapon->Projectile->Hit_Type == PJ_BOUNCE)*/
						return i;

		// don't return here, try selection by other, not optimal fitting cases
    }


    //
    // Any greater distance for direct firing uses a projectile weapon first
    //
    if(bDirect) {
		// First try projectile
		int i;
		for (i=0; i<5; i++)
			if (!tWeapons[i].Reloading)  
				if (tWeapons[i].Weapon->Type == WPN_PROJECTILE)
					if( tWeapons[i].Weapon->Projectile->Hit_Type != PJ_DIRT
					&& tWeapons[i].Weapon->Projectile->Hit_Type != PJ_GREENDIRT)
						return i;

		// If projectile not available, try beam
		for (i=0; i<5; i++)
			if (!tWeapons[i].Reloading)  
				if (tWeapons[i].Weapon->Type == WPN_BEAM)
					return i;

		// If everything fails, try some random weapons
		int num=0;
		for (i=0; i<5; i++, num=GetRandomInt(4))
			if (!tWeapons[num].Reloading)  
				return num;
				
		//return -1;
    }


    //
    // Indirect firing weapons
    //


    // If we're above the target, try any special weapon, for Liero mod try napalm
    // BUT only if our health is looking good
    // AND if there is no rock/dirt nearby
    if(fDistance > 190 && iHealth > 25 && fTraceDist > 0.5f && (cTrgPos.y-20) > vPos.y ) {
        if (!NEW_AI_CheckFreeCells(5,pcMap))
			return -1;

		// try projectile weapons
		for (int i=0; i<5; i++)
			if (!tWeapons[i].Reloading && tWeapons[i].Weapon->Type == WPN_PROJECTILE)
				if (tWeapons[i].Weapon->Projectile->Hit_Type == PJ_EXPLODE || tWeapons[i].Weapon->Projectile->Hit_Type == PJ_BOUNCE)
					return i;
					
    }


    //
    // Last resort
    //

    // Shoot a beam (we cant suicide with that)
	int i;
	for (i=0; i<5; i++)
		if (!tWeapons[i].Reloading && tWeapons[i].Weapon->Type == WPN_BEAM)  
			return i;
    
    
    
    // No suitable weapon found
	/*for (i=0;i<5;i++)
		if (!tWeapons[i].Reloading)
		{	
			return i;
		}*/
		
	// If everything fails, try some random weapons
	int num = GetRandomInt(4);
	for (i=0; i<5; i++, num=GetRandomInt(4))
		if (!tWeapons[num].Reloading)  
			return num;

	// If everything fails, try all weapons
	for (i=0; i<5; i++)
		if (!tWeapons[i].Reloading)  
			return num;

    return -1;
}


///////////////////
// Return any unloaded weapon in the list
/*int CWorm::cycleWeapons(void)
{
    // Don't do this in easy mode
    if( tProfile->nDifficulty == 0 )
        return iCurrentWeapon;

    // Find the first reloading weapon
    for( int i=0; i<5; i++) {
        if( tWeapons[i].Reloading )
            return i;
    }

    // Default case (all weapons loaded)
    return 0;
}*/


///////////////////
// Trace a line from this worm to the target
// Returns the distance the trace went
int CWorm::traceLine(CVec target, CMap *pcMap, float *fDist, int *nType, int divs)
{
    assert( pcMap );

    // Trace a line from the worm to length or until it hits something
	CVec    pos = vPos;
	CVec    dir = target-pos;
    int     nTotalLength = (int)NormalizeVector(&dir);

	int divisions = divs;			// How many pixels we go through each check (more = slower)

	if( nTotalLength < divisions)
		divisions = nTotalLength;

    *nType = PX_EMPTY;

	// Make sure we have at least 1 division
	divisions = MAX(divisions,1);

	int i;
	for(i=0; i<nTotalLength; i+=divisions) {
		uchar px = pcMap->GetPixelFlag( (int)pos.x, (int)pos.y );
		//pcMap->PutImagePixel((int)pos.x, (int)pos.y, MakeColour(255,0,0));

        if((px & PX_DIRT) || (px & PX_ROCK)) {
        	if(nTotalLength != 0)
            	*fDist = (float)i / (float)nTotalLength;
            else
            	*fDist = 0;
            *nType = px;
            return i;
        }

        pos = pos + dir * (float)divisions;
    }


    // Full length    
	if(nTotalLength != 0)
		*fDist = (float)i / (float)nTotalLength;
	else
		*fDist = 0;
    return nTotalLength;
}

///////////////////
// Returns true, if the cell is empty
// Cell can be: CELL_CURRENT, CELL_LEFT,CELL_DOWN,CELL_RIGHT,CELL_UP,CELL_LEFTDOWN,CELL_RIGHTDOWN,CELL_LEFTUP,CELL_RIGHTUP
bool CWorm::IsEmpty(int Cell, CMap *pcMap)
{
  bool bEmpty = false;
  int cx = (int)(vPos.x / pcMap->getGridWidth());
  int cy = (int)(vPos.y / pcMap->getGridHeight());

  switch (Cell)  {
  case CELL_LEFT:
	  cx--;
	  break;
  case CELL_DOWN:
	  cy++;
	  break;
  case CELL_RIGHT:
	  cx++;
	  break;
  case CELL_UP:
	  cy--;
	  break;
  case CELL_LEFTDOWN:
	  cx--;
	  cy++;
	  break;
  case CELL_RIGHTDOWN:
	  cx++;
	  cy++;
	  break;
  case CELL_LEFTUP:
	  cx--;
	  cy--;
	  break;
  case CELL_RIGHTUP:
	  cx++;
	  cy--;
	  break;
  }

  if (cx < 0 || cx > pcMap->getGridCols())
	  return false;

  if (cy < 0 || cy > pcMap->getGridRows())
	  return false;

  /*int dx = cx*pcMap->getGridCols()*2;
  int dy = pcMap->getGridRows()*2*cy;

  DrawRect(pcMap->GetDrawImage(),dx,dy,dx+(pcMap->GetWidth()/pcMap->getGridCols())*2,dy+(pcMap->GetHeight()/pcMap->getGridRows())*2,0xFFFF);

  dx /= 2;
  dy /= 2;
  DrawRect(pcMap->GetImage(),dx,dy,dx+pcMap->GetWidth()/pcMap->getGridCols(),dy+pcMap->GetHeight()/pcMap->getGridRows(),0xFFFF);*/

  const uchar   *f = pcMap->getGridFlags() + cy*pcMap->getGridWidth()+cx;
  bEmpty = *f == PX_EMPTY;

  return bEmpty;
}

/////////////////////////////
// TEST TEST TEST
//////////////////
// Finds the nearest free cell in the map and returns coordinates of its midpoint
CVec CWorm::NEW_AI_FindClosestFreeCell(CVec vPoint, CMap *pcMap)
{
	// NOTE: highly unoptimized, looks many times to the same cells

	// Get the cell
	int cellX = (int) fabs((vPoint.x)/pcMap->getGridWidth());
	int cellY = (int) fabs((vPoint.y)/pcMap->getGridHeight());	

	int cellsSearched = 1;
	const int numCells = pcMap->getGridCols() * pcMap->getGridRows();
	int i=1;
	int x,y;
	uchar tmp_pf = PX_ROCK;
	while (cellsSearched < numCells) {
		for (y=cellY-i;y<=cellY+i;y++)  {

			// Clipping
			if (y > pcMap->getGridRows())
				break;
			if (y < 0)
				continue;

			for (x=cellX-i;x<=cellX+i;x++)  {
				// Don't check the entry cell
				if (x == cellX && y == cellY)
					continue;

				// Clipping
				if (x > pcMap->getGridCols())
					break;
				if (x < 0)
					continue;

				tmp_pf = *(pcMap->getGridFlags() + y*pcMap->getGridCols() +x);
				if (!(tmp_pf & PX_ROCK))
					return CVec((float)x*pcMap->getGridWidth()+pcMap->getGridWidth()/2, (float)y*pcMap->getGridHeight()+pcMap->getGridHeight()/2);
			}
		}
		i++;
		cellsSearched++;
	}

	// Can't get here
	return CVec(0,0);
}

/////////////////////
// Trace the line for the current weapon
int CWorm::traceWeaponLine(CVec target, CMap *pcMap, float *fDist, int *nType)
{
   assert( pcMap );

    // Trace a line from the worm to length or until it hits something
	CVec    pos = vPos;
	CVec    dir = target-pos;
    int     nTotalLength = (int)NormalizeVector(&dir);

	int first_division = 7;	// How many pixels we go through first check (we can shoot through walls)		
	int divisions = 5;		// How many pixels we go through each check (more = slower)
	
	//
	// Predefined divisions
	//

	// Beam
	if (tWeapons[iCurrentWeapon].Weapon->Type == WPN_BEAM)  {
		first_division = 1;
		divisions = 1;
	}

	// Rifles
	else if (iAiGameType == GAM_RIFLES)  {
		first_division = 10;
		divisions = 6;
	}

	// Mortars
	else if (iAiGameType == GAM_MORTARS)  {
		first_division = 7;
		divisions = 2;
	}

	// 100lt
	else if (iAiGameType == GAM_100LT)  {
		first_division = 6;
		divisions = 3;
	}

	// Add the worm thickness
	first_division += 5;

	// Check
	if( nTotalLength < divisions)
		divisions = nTotalLength;

    *nType = PX_EMPTY;

	// Make sure we have at least 1 division
	divisions = MAX(divisions,1);
	first_division = MAX(first_division,1);

	// Check that we don't hit any teammate
	CVec WormsPos[MAX_WORMS];
	int	WormCount = 0;
	int i;
	if (cClient && tGameInfo.iGameMode == GMT_TEAMDEATH)  {
		CWorm *w = cClient->getRemoteWorms();
		for (i=0;i<MAX_WORMS;i++,w++)  {
			if (w)
				if (w->isUsed() && w->getAlive() && w->getTeam() == iTeam)
					WormsPos[WormCount++] = w->getPos();
		}
	}


	// Trace the line
	int divs = first_division;
	int j;
	for(i=0; i<nTotalLength; i+=divs) {
		uchar px = pcMap->GetPixelFlag( (int)pos.x, (int)pos.y );

		if (i>first_division)  // we aren't close to a wall, so we can shoot through only thin wall
			divs = divisions;

		// Dirt or rock
		if((px & PX_DIRT) || (px & PX_ROCK)) {
        	if(nTotalLength != 0)
            	*fDist = (float)i / (float)nTotalLength;
            else
            	*fDist = 0;
			*nType = px;
			return i;
		}

		// Friendly worm
		for (j=0;j<WormCount;j++) {
			if (CalculateDistance(pos,WormsPos[j]) < 20.0f)  {
				if(nTotalLength != 0)
					*fDist = (float)i / (float)nTotalLength;
				else
					*fDist = 0;
				*nType = PX_WORM;
				return i;
			}
		}

		pos += dir * (float)divs;
	}

	// Full length
	if(nTotalLength != 0)
		*fDist = (float)i / (float)nTotalLength;
	else
		*fDist = 0;
	return nTotalLength;
}




class set_col_and_break {
public:
	CVec* collision;
	CVec start;
	bool hit;
	
	set_col_and_break(CVec st, CVec* col) : start(st), collision(col), hit(false) {}
	bool operator()(int x, int y) {
		hit = true;
		if(collision && (*collision - start).GetLength2() > (CVec(x,y) - start).GetLength2()) {
			collision->x = x;
			collision->y = y;			
		}
		return false;
	}
};


////////////////////
// Trace the line with worm width
int traceWormLine(CVec target, CVec start, CMap *pcMap, CVec* collision)
{	
	// At least three, else it goes through walls in jukke
	static const unsigned short wormsize = 3;

	if(collision) {
		collision->x = target.x;
		collision->y = target.y;
	}
	
	CVec dir = CVec(target.y-start.y,start.x-target.x); // rotate clockwise by 90 deg
	NormalizeVector(&dir);
	set_col_and_break action = set_col_and_break(start - dir*(wormsize-1)/2, collision);
	target -= dir*(wormsize-1)/2;
	for(register unsigned short i = 0; i < wormsize; i++, action.start += dir, target += dir)
		action = fastTraceLine(target, action.start, pcMap, (uchar)PX_ROCK, action);
	
	return !action.hit;
	
/*	
	// Get the positions
	CVec    pos;
	CVec    dir = target-start;
	int     nTotalLength = (int)NormalizeVector(&dir);

	uchar* pxflags = pcMap->GetPixelFlags();
	int map_w = pcMap->GetWidth();
	int map_h = pcMap->GetHeight();
	

	pos = start;
	
	// Trace the line
	int i;
	uchar px;
	for(i=0; i<nTotalLength; i++) {
		if( (int)pos.x < 0 || (int)pos.x >= map_w 
		|| (int)pos.y < 0 || (int)pos.y >= map_h )
			px = PX_ROCK;
		else
			px = pxflags[(int)pos.x + map_w*(int)pos.y];

		if(px & PX_ROCK) {
			if(collision) {
				collision->x=(pos.x-dir.x);
				collision->y=(pos.y-dir.y);					
			}
			return false;				
		}

		pos += dir;
	}

	if(collision) {
		collision->x=(target.x);
		collision->y=(target.y);
	}

	return true;
*/
}

////////////////////////
// Checks if there is enough free cells around us to shoot
bool CWorm::NEW_AI_CheckFreeCells(int Num,CMap *pcMap)
{
	// Get the cell
	int cellX = (int) fabs((vPos.x)/pcMap->getGridWidth());
	int cellY = (int) fabs((vPos.y)/pcMap->getGridHeight());

	// First of all, check our current cell
	if (*(pcMap->getGridFlags() + cellY*pcMap->getGridCols() +cellX) & PX_ROCK)
		return false;
	
/*#ifdef _AI_DEBUG
	SDL_Surface *bmpDest = pcMap->GetDebugImage();
	if (!bmpDest)
		return false;
	DrawRectFill(bmpDest,0,0,bmpDest->w,bmpDest->h,MakeColour(255,0,255));
#endif*/

	// Direction to left
	if (iDirection == DIR_LEFT)  {
		int dir = 0;
		if (fAngle > 210)
			dir = 1;
		if (fAngle < 210 && fAngle > 160)
			dir = -Num/2;
		if (fAngle < 160)
			dir = -1*Num;

/*#ifdef _AI_DEBUG
		int dX = (cellX-Num-1)*pcMap->getGridWidth()+pcMap->getGridWidth()/2;
		int dY = (cellY+dir)*pcMap->getGridHeight()+pcMap->getGridHeight()/2;
		int dX2 = (cellX-1)*pcMap->getGridWidth()+pcMap->getGridWidth()/2;
		int dY2 = (cellY+dir+Num)*pcMap->getGridHeight()+pcMap->getGridHeight()/2;
		DrawRect(bmpDest,dX*2,dY*2,dX2*2,dY2*2,MakeColour(255,0,0));
#endif*/

		// Check the Num*Num square
		int x,y;
		for (x=cellX-Num-1;x<cellX;x++)  {
			// Clipping means rock
			if (x < 0 || x > pcMap->getGridCols())
				return false;
			for (y=cellY+dir;y<cellY+dir+Num;y++)  {
				// Clipping means rock
				if (y < 0 || y > pcMap->getGridRows())
					return false;

				// Clipping means rock
				if (*(pcMap->getGridFlags() + y*pcMap->getGridCols() +x) & PX_ROCK)
					return false;
			}
		}

		return true;
	// Direction to right
	}  else  {
		int dir = 0;
		if (fAngle > 20)
			dir = 1;
		else if (fAngle < 20 && fAngle > -20)
			dir = -Num/2;
		else if (fAngle < -20)
			dir = -1*Num;

/*#ifdef _AI_DEBUG
		int dX = (cellX)*pcMap->getGridWidth()+pcMap->getGridWidth()/2;
		int dY = (cellY+dir)*pcMap->getGridHeight()+pcMap->getGridHeight()/2;
		int dX2 = (cellX+Num)*pcMap->getGridWidth()+pcMap->getGridWidth()/2;
		int dY2 = (cellY+dir+Num)*pcMap->getGridHeight()+pcMap->getGridHeight()/2;
		DrawRect(bmpDest,dX*2,dY*2,dX2*2,dY2*2,MakeColour(255,0,0));
#endif*/

		// Check the square Num*Num
		int x,y;
		for (x=cellX;x<=cellX+Num;x++)  {
			// Clipping means rock
			if (x< 0 || x > pcMap->getGridCols())
				return false;
			for (y=cellY+dir;y<cellY+dir+Num;y++)  {
				// Clipping means rock
				if (y < 0 || y > pcMap->getGridRows())
					return false;

				// Rock cell
				if (*(pcMap->getGridFlags() + y*pcMap->getGridCols() +x) & PX_ROCK)
					return false;
			}
		}

		return true;
	}

	// Weird, shouldn't happen
	return false;
}


////////////////////
// Creates the path
int CWorm::NEW_AI_CreatePath(CMap *pcMap)
{	
	CVec trg = AI_GetTargetPos();
	
	if(!bPathFinished) {
		// have we finished a current search?
		if(((searchpath_base*)pathSearcher)->isReady()) {
			
			NEW_ai_node_t* res = ((searchpath_base*)pathSearcher)->resultedPath();
						
			// have we found something?
			if(res) {
				// in every case, the nodes of the current path is not handled by pathSearcher
				// so we have to delete it here
				delete_ai_nodes(NEW_psPath);
				
				NEW_psPath = res;
				NEW_psLastNode = get_last_ai_node(NEW_psPath);
				NEW_psCurrentNode = NEW_psPath;
				bPathFinished = true;								
				
				// prevent it from deleting the current path (it will be deleted, when the new path is found)
				((searchpath_base*)pathSearcher)->removePathFromList(NEW_psPath);				
				
#ifdef _AI_DEBUG
				pcMap->ClearDebugImage();	
				NEW_AI_DrawPath(pcMap);
#endif
								
				fLastCreated = tLX->fCurTime;
				
				return true;
			}
			// we don't find anything, so don't return here, but start a new search
			
		} else { // the searcher is still searching ...
		
    		// TODO: if the search takes very long (this is, if it doesn't find something), start a new one after some time		
			return false;
		}		
	}
	
	// don't start a new search, if the current end-node still has direct access to it
	// however, we have to have access somewhere to the path
	if(NEW_psLastNode && traceWormLine(CVec(NEW_psLastNode->fX, NEW_psLastNode->fY), trg, pcMap)) {
		for(NEW_ai_node_t* node = NEW_psPath; node; node = node->psNext)
			if(traceWormLine(CVec(node->fX,node->fY),vPos,pcMap,NULL)) {
				NEW_psCurrentNode = node;
				NEW_psLastNode->psNext = createNewAiNode(trg.x, trg.y, NULL, NEW_psLastNode);
				NEW_psLastNode = NEW_psLastNode->psNext;
				return true;
			}
	}
	
	// Don't create the path so often!
	if (tLX->fCurTime - fLastCreated <= 0.5f)  {
		return NEW_psPath != NULL;
	}
	
	// if we are here, we want to start a new search
	bPathFinished = false;	
	
	// start a new search
	((searchpath_base*)pathSearcher)->target.x = (int)trg.x;
	((searchpath_base*)pathSearcher)->target.y = (int)trg.y;
	((searchpath_base*)pathSearcher)->start = VectorD2<int>(vPos);
	((searchpath_base*)pathSearcher)->startThreadSearch();
	
	return false;
}


int GetRockBetween(CVec pos,CVec trg, CMap *pcMap)
{
    assert( pcMap );

	int result = 0;

    // Trace a line from the worm to the target
	CVec    dir = trg-pos;
    int     nTotalLength = (int)NormalizeVector(&dir);

	const int divisions = 4;			// How many pixels we go through each check (less = slower)

	int i;
	uchar px = PX_EMPTY;
	for(i=0; i<nTotalLength; i+=divisions) {
		px = pcMap->GetPixelFlag( (int)pos.x, (int)pos.y );
		//pcMap->PutImagePixel((int)pos.x, (int)pos.y, MakeColour(255,0,0));

        if (px & PX_ROCK)
			result++;

        pos = pos + dir * (float)divisions;
    }

	return result;
}

/*CVec NEW_AI_FindBestSpot(CVec trg, CVec pos, CMap *pcMap)
{
	// Get the midpoint
	CVec middle = CVec((pos.x+trg.x)/2,(pos.y+trg.y)/2);
	
	float a = CalculateDistance(middle,pos);
	float b = a;
	float step = DEG2RAD(20);
	int min = 99999999;
	CVec result = CVec(0,0);

	// We make a circle and check from the points the way with least rock
	float i;
	float x,y;
	for (;b > 10.0f; b-=b/2)
		for (i=0;i<2*PI; i+=step)  {
			x = a*(float)sin(2*PI*i)+middle.x;
			y = b*(float)cos(2*PI*i)+middle.y;
			CVec point = CVec(x,y);
			if (pcMap->GetPixelFlag( (int)pos.x, (int)pos.y ) & PX_ROCK)
				continue;

			int rock_pixels = GetRockBetween(point,pos,pcMap)+GetRockBetween(point,trg,pcMap);
			if (rock_pixels < min)  {
				min = rock_pixels;
				result.x=(point.x);
				result.y=(point.y);

				if (!min)
					return result;
			}
		}

	return result;
}*/


CVec CWorm::NEW_AI_FindBestFreeSpot(CVec vPoint, CVec vStart, CVec vDirection, CVec vTarget, CVec* vEndPoint, CMap *pcMap) {
	
	/*
		TODO: the algo can made a bit more general, which would increase the finding		
	*/
	
#ifdef _AI_DEBUG
	//SDL_Surface *bmpDest = pcMap->GetDebugImage();
#endif

	unsigned short i = 0;
	int map_w = pcMap->GetWidth();
	int map_h = pcMap->GetHeight();
	const uchar* pxflags = pcMap->GetPixelFlags();
	CVec pos = vStart;
	CVec best = vStart;
	CVec end = pos;
	CVec possible_end;
	CVec backdir = vStart - vTarget;
	backdir = backdir / backdir.GetLength();
	end -= backdir*6;
	float best_length = -1; // the higher it is, the better it is
	vDirection = vDirection / vDirection.GetLength();
	bool lastWasObstacle = false;
	bool lastWasMissingCon = false;
	while(true) {
#ifdef _AI_DEBUG
		//PutPixel(bmpDest,(int)pos.x*2,(int)pos.y*2,MakeColour(255,255,0));
#endif

		if(!lastWasObstacle && !lastWasMissingCon) pos += vDirection;
		i++;

		// make the search-dist greater, if we are far away
		if(i >= 32 && i % 16 == 0) {
			backdir = backdir * 2;
			vDirection = vDirection * 2;
		}

		// don't search to wide
		if(i > 100)
			break;

		// Clipping		
		if( (int)pos.x < 0 || (int)pos.x >= map_w 
		|| (int)pos.y < 0 || (int)pos.y >= map_h )
			break;

		// obstacle...
		if(PX_ROCK & pxflags[(int)pos.x + map_w*(int)pos.y]) {
			pos += backdir;
			lastWasObstacle = true;
			continue;
		} else
			lastWasObstacle = false;
					

		if(i % 4 == 0 || lastWasMissingCon) {
			// do we still have a direct connection to the point?
			if(!traceWormLine(vPoint,pos,pcMap)) {
				// perhaps we are behind an edge (because auf backdir)
				// then go a little more to backdir
				pos += backdir;
				lastWasMissingCon = true;
				continue;
			} else
				lastWasMissingCon = false;
		}

		// don't check to often
		if(i % 4 == 0) {
			// this is the parallel to backdir
			traceWormLine(pos-backdir*1000,pos,pcMap,&possible_end);
			possible_end += backdir*5/backdir.GetLength();
#ifdef _AI_DEBUG
			//PutPixel(bmpDest,(int)possible_end.x*2,(int)possible_end.y*2,MakeColour(255,0,255));
#endif
			// 'best' is, if we have much free way infront of pos
			// and if we are not so far away from the start
			if((possible_end-pos).GetLength2()/(pos-vPoint).GetLength2() > best_length) {
				end = possible_end;
				best = pos;
				best_length = (possible_end-pos).GetLength2()/(pos-vPoint).GetLength2();
			}
		}
	}
	
	if(vEndPoint)
		*vEndPoint = end;
	
	return best;
}

//////////////////
// Finds the closest free spot, looking only in one direction
CVec CWorm::NEW_AI_FindClosestFreeSpotDir(CVec vPoint, CVec vDirection, CMap *pcMap, int Direction = -1)
{
#ifdef _AI_DEBUG
//	SDL_Surface *bmpDest = pcMap->GetDebugImage();
#endif

	NormalizeVector(&vDirection);
	//CVec vDev = CVec(vDirection.x*5,vDirection.y*5);
	CVec vDev = CVec(0,0);

	int i;
	int emptyPixels = 0;
	CVec pos = vPoint+vDev;
	int firstClosest = 9999;
	int secondClosest = 9999;
	CVec rememberPos1 = CVec(0,0);
	CVec rememberPos2 = CVec(0,0);

	for(i=0; 1; i++) {
		uchar px = pcMap->GetPixelFlag( (int)pos.x, (int)pos.y );

		// Empty pixel? Add it to the count
		if(!(px & PX_ROCK)) {
#ifdef _AI_DEBUG
			//PutPixel(bmpDest,pos.x*2,pos.y*2,MakeColour(255,255,0));
#endif
			emptyPixels++;
		}
		// Rock pixel? This spot isn't good
		else {
#ifdef _AI_DEBUG
			//PutPixel(bmpDest,pos.x*2,pos.y*2,MakeColour(255,0,0));
#endif
			if (emptyPixels >= 10)
				break;
			emptyPixels = 0;
			rememberPos1.x=(0);
			rememberPos1.y=(0);
		}

		// Good spot
		if (emptyPixels >= 10)  {
			firstClosest = i-emptyPixels;
			if (emptyPixels >= 30)
				break;
			rememberPos1.x=(rememberPos1.x+vDirection.x);
			rememberPos1.y=(rememberPos1.y+vDirection.y);
		}

		if (emptyPixels == 5)  {
			rememberPos1.x=(pos.x);
			rememberPos1.y=(pos.y);
		}

		pos = pos + vDirection;
		// Clipping
		if (pos.x > pcMap->GetWidth() || pos.x < 0)
			break;
		if (pos.y > pcMap->GetHeight() || pos.y < 0)
			break;
	}

	if (firstClosest != 9999 && Direction == DIR_LEFT)
		return rememberPos1;

	emptyPixels = 0;
	vDirection.y=(-vDirection.y);
	vDirection.x=(-vDirection.x);
	vDev.x=(-vDev.x);
	vDev.y=(-vDev.y);
	pos = vPoint+vDev;

	for(i=0; 1; i++) {
		uchar px = pcMap->GetPixelFlag( (int)pos.x, (int)pos.y );

		// Empty pixel? Add it to the count
		if(!(px & PX_ROCK)) {
#ifdef _AI_DEBUG
			//PutPixel(bmpDest,pos.x*2,pos.y*2,MakeColour(255,255,0));
#endif
			emptyPixels++;
		}
		// Rock pixel? This spot isn't good
		else {
#ifdef _AI_DEBUG
			//PutPixel(bmpDest,pos.x*2,pos.y*2,MakeColour(255,0,0));
#endif
			if (emptyPixels >= 10)  
				break;

			rememberPos2.x=(0);
			rememberPos2.y=(0);
			emptyPixels = 0;
		}

		// Good spot
		if (emptyPixels > 10)  {
			secondClosest = i-emptyPixels;
			if (emptyPixels >= 30)
				break;
			rememberPos2.x=(rememberPos2.x+vDirection.x);
			rememberPos2.y=(rememberPos2.y+vDirection.y);
		}

		// Remember this special position (in the middle of possible free spot)
		if (emptyPixels == 5)  {
			rememberPos2.x=(pos.x);
			rememberPos2.y=(pos.y);
		}

		pos = pos + vDirection;
		// Clipping
		if (pos.x > pcMap->GetWidth() || pos.x < 0)
			break;
		if (pos.y > pcMap->GetHeight() || pos.y < 0)
			break;
	}

	if (secondClosest != 9999 && Direction == DIR_RIGHT)
		return rememberPos2;

	// In what direction was the closest spot?
	if (firstClosest < secondClosest)
		return rememberPos1;
	else
		return rememberPos2;
}


#ifdef _AI_DEBUG
///////////////////
// Draw the AI path
void CWorm::NEW_AI_DrawPath(CMap *pcMap)
{
	if (!NEW_psPath)
		return;

	SDL_Surface *bmpDest = pcMap->GetDebugImage();
	if (!bmpDest)
		return;

	const int NodeColour = MakeColour(255,0,0);
	const int LineColour = 0xffff;
	const int transparent = MakeColour(255,0,255);

	//(bmpDest,0,0,bmpDest->w,bmpDest->h,transparent);

	// Go down the path
	NEW_ai_node_t *node = NEW_psCurrentNode;
	int node_x = 0;
	int node_y = 0;
	for (;node;node=node->psNext)  {

		// Get the node position
		node_x = Round(node->fX*2);
		node_y = Round(node->fY*2);

		// Clipping
		if (node_x-4 < 0 || node_x+4 > bmpDest->w)
			continue;
		if (node_y-4 < 0 || node_y+4 > bmpDest->h)
			continue;

		// Draw the node
		DrawRectFill(bmpDest,node_x-4,node_y-4,node_x+4,node_y+4,NodeColour);

		// Draw the line
		if (node->psNext)
			DrawLine(bmpDest,MIN(Round(node->psNext->fX*2),bmpDest->w),MIN(Round(node->psNext->fY*2),bmpDest->h),node_x,node_y,LineColour);
	}
	
}
#endif


class bestropespot_collision_action {
public:
	CVec target, best;
	bestropespot_collision_action(CVec t) : target(t), best(-1,-1) {}
	
#ifdef _AI_DEBUG
	//CMap* pcMap;
#endif
	
	bool operator()(int x, int y) {
#ifdef _AI_DEBUG		
		//DrawRectFill(pcMap->GetDebugImage(),x*2-4,y*2-4,x*2+4,y*2+4,MakeColour(0,240,0));		
#endif		
	
		if(best.x < 0 || (CVec(x,y)-target).GetLength2() < (best-target).GetLength2())
			best = CVec(x,y);
				
		return false;
	}
};

/////////////////////////
// Finds the best spot to shoot rope to if we want to get to trg
CVec CWorm::NEW_AI_GetBestRopeSpot(CVec trg, CMap *pcMap)
{
	// Get the direction angle
	CVec dir = trg-vPos;
	dir = dir*cNinjaRope.getMaxLength()/dir.GetLength();
	dir = CVec(-dir.y,dir.x); // rotate reverse-clockwise by 90 deg
	
	// Variables
	float step = 0.05f*(float)PI;
	float ang = 0;
	
	SquareMatrix<float> step_m = SquareMatrix<float>::RotateMatrix(-step);
	bestropespot_collision_action action(trg+CVec(0,-50));
#ifdef _AI_DEBUG
	//action.pcMap = pcMap;	
#endif

	for(ang=0; ang<(float)PI; dir=step_m(dir),ang+=step) {
		action = fastTraceLine(vPos+dir, vPos, pcMap, PX_ROCK|PX_DIRT, action);
	}

	if(action.best.x < 0) // we don't find any spot
		return trg;
	else
		return action.best;
}

////////////////////
// Finds the nearest spot to the target, where the rope can be hooked
CVec CWorm::NEW_AI_GetNearestRopeSpot(CVec trg, CMap *pcMap)
{
	CVec dir = trg-vPos;
	NormalizeVector(&dir);
	dir = dir*10;
	while (CalculateDistance(vPos,trg) >= cNinjaRope.getRestLength()) 
		trg = trg-dir;

	//
	// Find the nearest cell with rock or dirt
	//

	// Get the current cell
	uchar tmp_pf = PX_ROCK;
	int cellX = (int) (trg.x)/pcMap->getGridWidth();
	int cellY = (int) (trg.y)/pcMap->getGridHeight();
	
	// Clipping means rock
	if (cellX > pcMap->getGridCols() || cellX < 0)
		return trg;
	if (cellY > pcMap->getGridRows() || cellY < 0)
		return trg;

	// Check the current cell first
	tmp_pf = *(pcMap->getGridFlags() + cellY*pcMap->getGridCols() +cellX);
	if ((tmp_pf & PX_ROCK) || (tmp_pf & PX_DIRT))
		return trg;

	// Note: unoptimized

	const int numCells = pcMap->getGridCols() * pcMap->getGridRows();
	int i=1;
	int x,y;
	bool bFound = false;
	while (!bFound) {
		for (y=cellY-i;y<=cellY+i;y++)  {

			// Clipping means rock
			if (y > pcMap->getGridRows())  {
				bFound = true;
				break;
			}
			if (y < 0)  {
				bFound = true;
				break;
			}


			for (x=cellX-i;x<=cellX+i;x++)  {
				// Don't check the entry cell
				if (x == cellX && y == cellY)
					continue;

				// Clipping means rock
				if (x > pcMap->getGridCols())  {
					bFound = true;
					break;
				}
				if (x < 0)  {
					bFound = true;
					break;
				}

				// Get the pixel flag of the cell
				tmp_pf = *(pcMap->getGridFlags() + y*pcMap->getGridCols() +x);
				if ((tmp_pf & PX_ROCK) || (tmp_pf & PX_DIRT))  {
					bFound = true;
					break;
				}	
			}
		}
		i++;
	}

	return CVec((float)x*pcMap->getGridWidth()+pcMap->getGridWidth()/2, (float)y*pcMap->getGridHeight()+pcMap->getGridHeight()/2);

}

///////////////
// Returns true if the point has the specified amount of air around itself
bool CWorm::NEW_AI_IsInAir(CVec pos, CMap *pcMap, int area_a)
{
	// Get the current cell
	uchar tmp_pf = PX_ROCK;
	int startX = (int) (pos.x)/pcMap->getGridWidth()-(int)floor((double)area_a/2);
	int startY = (int) (pos.y)/pcMap->getGridHeight()-(int)floor((double)area_a/2);
	
	int x,y,i;
	x=startX;
	y=startY;

	for (i=0;i<area_a*area_a;i++) {
		if (x > area_a)  {
			x = startX;
			y++;
		}

		// Rock or dirt - not in air
		tmp_pf = *(pcMap->getGridFlags() + y*pcMap->getGridCols() +x);
		if (tmp_pf & PX_ROCK || tmp_pf & PX_DIRT)
			return false;

		x++;
	}

	return true;

}


/////////////////////
// Move to the target
float fLastCompleting = -9999; // TODO: this is global for all worms (which is not what is wanted)
void CWorm::NEW_AI_MoveToTarget(CMap *pcMap)
{
/*	NEW_AI_MoveToTargetDC(pcMap);
	return; */
//	printf("Moving to target");

    worm_state_t *ws = &tState;

	// Better target?
	CWorm *newtrg = findTarget(iAiGame, iAiTeams, iAiTag, pcMap);
	if (psAITarget && newtrg)
		if (newtrg->getID() != psAITarget->getID())
			nAIState = AI_THINK;

	if (!psAITarget && newtrg)  {
		nAIState = AI_THINK;
	}

	// No target?
	if (nAITargetType == AIT_NONE || (nAITargetType == AIT_WORM && !psAITarget))  {
		nAIState = AI_THINK;
		return;
	}

    // Clear the state
	ws->iCarve = false;
	ws->iMove = false;
	ws->iShoot = false;
	ws->iJump = false;

	// If the rope hook is attached, increase the attached time
	if (cNinjaRope.isAttached())
		fRopeAttachedTime += tLX->fDeltaTime;
	else
		fRopeAttachedTime = 0;
			
	// release the rope if we used it to long
	if (fRopeAttachedTime > 5.0f)
		cNinjaRope.Release();
			
	if(cNinjaRope.isShooting() && !cNinjaRope.isAttached())
		fRopeHookFallingTime += tLX->fDeltaTime;
	else
		fRopeHookFallingTime = 0;
			
	if (fRopeHookFallingTime >= 2.0f)  {
		// Release & walk
		cNinjaRope.Release();
        ws->iMove = true;
		fRopeHookFallingTime = 0;
	}

    cPosTarget = AI_GetTargetPos();

/*    
    // this don't make sense here; what if there is a wall between them?
    // AI_CanShoot should make the ness checks, if we can shoot at the target
    // if not, move to the next node; the last node has direct access to the target
    // If we're really close to the target, perform a more precise type of movement
    if(fabs(vPos.x - cPosTarget.x) < 20 && fabs(vPos.y - cPosTarget.y) < 20) {
        AI_PreciseMove(pcMap);
        return;
    }
*/

	// HINT: carve always; bad hack, but it works good
	// Don't carve so fast!
	if (tLX->fCurTime-fLastCarve > 0.2f)  {
		fLastCarve = tLX->fCurTime;
		ws->iCarve = true; // Carve
	}
	else  {
		ws->iCarve = false;
	}

    // If we're stuck, just get out of wherever we are
    if(bStuck) {
//		printf("Stucked");

        ws->iMove = true;
		if (tLX->fCurTime-fLastJump > 1.0f)  {
			ws->iJump = true;
			fLastJump = tLX->fCurTime;
		}

/*		// Don't carve so fast!
		if (tLX->fCurTime-fLastCarve > 0.2f)  {
			fLastCarve = tLX->fCurTime;
			ws->iCarve = true; // Carve
		}
		else  {
			ws->iCarve = false;
		} */

        if(tLX->fCurTime - fStuckPause > 2.0f)
            bStuck = false;
        
        // return here, because we force this stuck-action
        // if we would not return here, the stuck-check makes no sense
        return;
    }

	/*

		Prevent injuries! If any of the projectiles around is heading to us, try to get away of it

	*/
	if (psHeadingProjectile)  {
		// TODO: improve this (add rope using etc.)

		// Get the mutual speed
		CVec mutual_speed = psHeadingProjectile->GetVelocity() - vVelocity;

		// Go away from the projectile
		if (tLX->fCurTime-fLastTurn >= 0.5f)  {
			if (mutual_speed.x > 0)
				iDirection = DIR_RIGHT;
			else
				iDirection = DIR_LEFT;
			fLastTurn = tLX->fCurTime;
		}
		ws->iMove = true;


		// If we're on ground, jump
		if (CheckOnGround(pcMap))
			if (tLX->fCurTime - fLastJump > 1.0f)  {
				ws->iJump = true;
				fLastJump = tLX->fCurTime;
			}

		return;
	}


    /*
      First, if the target or ourself has deviated from the path target & start by a large enough amount:
      Or, enough time has passed since the last path update:
      recalculate the path
    */
    
    int     nDeviation = 40;     // Maximum deviation allowance
    bool    recalculate = false;  

	// Deviated?
	if (psAITarget && NEW_psPath && NEW_psLastNode)  {
		// Don't check when the velocity is big
		if (psAITarget->getVelocity()->GetLength() < 30 && vVelocity.GetLength() < 30)   {
			if(fabs(NEW_psPath->fX-vPos.x) > nDeviation || fabs(NEW_psPath->fY-vPos.y) > nDeviation)
				recalculate = true;

			if(fabs(NEW_psLastNode->fX-cPosTarget.x) > nDeviation || fabs(NEW_psLastNode->fY-cPosTarget.y) > nDeviation)
				recalculate = true;
		}
	}

    // Re-calculate the path?
    if(recalculate && bPathFinished)
        NEW_AI_CreatePath(pcMap);


    /*
      Move through the path.
      We have a current node that we must get to. If we go onto the node, we go to the next node, and so on.
    */

	//return; // Uncomment this when you don't want the AI to move

    if(NEW_psPath == NULL || NEW_psCurrentNode == NULL) {
        // If we don't have a path, resort to simpler AI methods
        AI_SimpleMove(pcMap,psAITarget != NULL);
		//printf("Pathfinding problem 2; ");
        return;
    }

//	printf("We should move now...");

	// If some of the next nodes is closer than the current one, just skip to it
	NEW_ai_node_t *next_node = NEW_psCurrentNode->psNext;
	bool newnode = false;
	while (next_node)  {
		if(traceWormLine(CVec(next_node->fX,next_node->fY),vPos,pcMap))  {
			NEW_psCurrentNode = next_node;
			newnode = true;
		}
		next_node = next_node->psNext;
	}
	if(newnode) {
	} else {
		// check, if we have a direct connection to the current node
		// else, chose some last node
		// this will work and is in many cases the last chance
		// perhaps, we need a fLastGoBack here
		for(next_node = NEW_psCurrentNode; next_node; next_node = next_node->psPrev) {
			if(traceWormLine(CVec(next_node->fX,next_node->fY),vPos,pcMap))  {
//				if(NEW_psCurrentNode != next_node)
	//				cNinjaRope.Release();				
				NEW_psCurrentNode = next_node;
				newnode = true;
				break;
			}		
		}
	}

	// Get the target node position
    CVec nodePos = CVec(NEW_psCurrentNode->fX,NEW_psCurrentNode->fY);

	// If we are close to the current node, just continue climbing the path
	// this will not work, because there could be some wall between the current and the next node
	// there shouldn't be any wall between the two nodes - else the path is wrong :P
	/*if (CalculateDistance(nodePos,vPos) <= 20.0f)  {
		if (NEW_psCurrentNode->psNext)  {
			NEW_psCurrentNode = NEW_psCurrentNode->psNext;
			nodePos = CVec(NEW_psCurrentNode->fX,NEW_psCurrentNode->fY);
		}
		else {
			// We are at the target
			nodePos = cPosTarget;
		}
	} */
		
	// release rope, if it forces us to the wrong direction
	if((cNinjaRope.GetForce(vPos).Normalize() + vPos - nodePos).GetLength2() > (vPos - nodePos).GetLength2())
		if (cNinjaRope.isAttached())
			cNinjaRope.Release();


#ifdef _AI_DEBUG
	NEW_AI_DrawPath(pcMap);
#endif
	
	


    /*
      Now that we've done all the boring stuff, our single job here is to reach the node.
      We have walking, jumping, move-through-air, and a ninja rope to help us.
    */   

    /*
      If there is dirt between us and the next node, don't shoot a ninja rope
      Instead, carve
    */
    bool fireNinja = true;
    float traceDist = -1;
    int type = 0;
    CVec v = CVec(NEW_psCurrentNode->fX, NEW_psCurrentNode->fY);
    int length = traceLine(v, pcMap, &traceDist, &type); // HINT: this is only a line, not the whole worm
														 // NOTE: this can return dirt, even if there's also rock between us two
    float dist = CalculateDistance(v, vPos);
    if((float)length <= dist && (type & PX_DIRT)) {
		// release rope, if it is atached and above
		if(cNinjaRope.isAttached() && cNinjaRope.getHookPos().y - 5.0f > v.y)
			cNinjaRope.Release();
		fireNinja = false;
		
		// Jump, if the node is above us
		if (v.y+10.0f < vPos.y)
			if (tLX->fCurTime - fLastJump > 1.0f)  {
				ws->iJump = true;
				fLastJump = tLX->fCurTime;
			}

        ws->iMove = true;

		// If the node is right above us, use a carving weapon
		if (fabs(v.x-vPos.x) <= 50) 
			if (v.y < vPos.y)  {
				int wpn;
				if((wpn = AI_FindClearingWeapon()) != -1) {
					iCurrentWeapon = wpn;
					AI_SetAim(v); // aim at the dirt
					ws->iShoot = true;
				} /* else
					AI_SimpleMove(pcMap,psAITarget != NULL); */ // no weapon found, so move around
			}
    }
 
     
	//
	//	Shooting the rope
	//

    // If the node is above us by a lot, we should use the ninja rope
	// If the node is far, jump and use the rope, too
	if(fireNinja) {
		fireNinja = (NEW_psCurrentNode->fY+20 < vPos.y);
		if (!fireNinja && (fabs(NEW_psCurrentNode->fX-vPos.x) >= 50))  {
			// On ground? Jump
			if(CheckOnGround(pcMap)) {
				if (tLX->fCurTime - fLastJump > 1.0f)  {
					ws->iJump = true;
					fLastJump = tLX->fCurTime;
					// Rope will happen soon
				}
			}
		}
	}
			
	// If we're above the node and the rope is hooked wrong, release the rope
	if((vPos.y < nodePos.y) && fRopeAttachedTime > 2.0f && cNinjaRope.getHookPos().y-vPos.y < 0.0f) {
		cNinjaRope.Release();
		fireNinja = false;
	}
	
	// Not on ground? Shoot the rope
	if(!CheckOnGround(pcMap))
		fireNinja = true;
	
	bool aim = false;

	// Get the best spot to shoot the rope to
	CVec cAimPos = NEW_AI_GetBestRopeSpot(nodePos,pcMap);

	// It has no sense to shoot the rope on short distances
	if (CalculateDistance(vPos,cAimPos) < 25.0f)
		fireNinja = false;

    if(fireNinja) {

		// Aim
		aim = AI_SetAim(cAimPos);


        CVec dir;
        dir.x=( (float)cos(fAngle * (PI/180)) );
	    dir.y=( (float)sin(fAngle * (PI/180)) );
	    if(iDirection == DIR_LEFT)
		    dir.x=(-dir.x);
       
        /*
          Got aim, so shoot a ninja rope
          also shoot, if we are not on ground
          We shoot a ninja rope if it isn't shot
          Or if it is, we make sure it has pulled us up and that it is attached
        */
        if(aim || !CheckOnGround(pcMap)) {
            if(!cNinjaRope.isReleased())
                cNinjaRope.Shoot(vPos,dir);
            else {
                float length = CalculateDistance(vPos, cNinjaRope.getHookPos());
                if(cNinjaRope.isAttached()) {
                    if(length < cNinjaRope.getRestLength() && vVelocity.y<-10)
                        cNinjaRope.Shoot(vPos,dir);
                }
            }
            ws->iJump = true;
			fRopeHookFallingTime = 0;
			fRopeAttachedTime = 0;
        }
        
    } else { // not fireNinja
        
		// Aim at the node
		aim = AI_SetAim(nodePos);
	
		// If the node is above us by a little, jump
		if(vPos.y - 10 <= NEW_psCurrentNode->fY && (vPos.y-NEW_psCurrentNode->fY) > 5) {
			// Don't jump so often
			if (tLX->fCurTime - fLastJump > 1.0f)  {
				ws->iJump = true;
				fLastJump = tLX->fCurTime;
			} else
				ws->iMove = true; // if we should not jump, move
		}
	}
	

    // If we are stuck in the same position for a while, take measures to get out of being stuck
    if(fabs(cStuckPos.x - vPos.x) < 5 && fabs(cStuckPos.y - vPos.y) < 5) {
        fStuckTime += tLX->fDeltaTime;

        // Have we been stuck for a few seconds?
        if(fStuckTime > 3) {
            // Jump, move, carve, switch directions and release the ninja rope
			if (tLX->fCurTime-fLastJump > 1.0f)  {
				ws->iJump = true;
				fLastJump = tLX->fCurTime;
			}
            ws->iMove = true;

/*			// Don't carve so fast!
			if (tLX->fCurTime-fLastCarve > 0.2f)  {
				fLastCarve = tLX->fCurTime;
				ws->iCarve = true; // Carve
			}
			else  {
				ws->iCarve = false;
			} */

            bStuck = true;
            fStuckPause = tLX->fCurTime;

			if (tLX->fCurTime-fLastTurn >= 0.5f)  {
				iDirection = !iDirection;
				fLastTurn = tLX->fCurTime;
			}
            
            fAngle -= cGameScript->getWorm()->AngleSpeed*tLX->fDeltaTime;
            // Clamp the angle
	        fAngle = MIN((float)60,fAngle);
	        fAngle = MAX((float)-90,fAngle);

			// Stucked too long?
			if (fStuckTime >= 5.0f)  {
				// Try the previous node
				if (NEW_psCurrentNode->psPrev)
					NEW_psCurrentNode = NEW_psCurrentNode->psPrev;
				fStuckTime = 0;
			}

            // Recalculate the path
            NEW_AI_CreatePath(pcMap);

            if(!NEW_psCurrentNode || !NEW_psPath)
            	return;
        }
    }
    else {
        bStuck = false;
        fStuckTime = 0;
        cStuckPos = vPos;
    }


	// If the rope is hooked wrong, release it
	/*if (cNinjaRope.isAttached())  {
		if (cNinjaRope.getHookPos().x+20 < vPos.x && cNinjaRope.getHookPos().x+20 < NEW_psCurrentNode->fX)
			cNinjaRope.Release();
		else if (cNinjaRope.getHookPos().x-20 > vPos.x && cNinjaRope.getHookPos().x-20 > NEW_psCurrentNode->fX)
			cNinjaRope.Release();
	}*/

	ws->iMove = true;


/*
	// If the next node is above us by a little, jump too
	NEW_ai_node_t *nextNode = NEW_psCurrentNode->psNext;
	if (nextNode)  {
		if ((vPos.y-nextNode->fY) <= 30 && (vPos.y-nextNode->fY) > 0)
			// Don't jump so often
			if (tLX->fCurTime - fLastJump > 1.0f)  {
				ws->iJump = true;
				fLastJump = tLX->fCurTime;
			} else
				ws->iMove = true; // if we should not jump, move
	}
*/

    




/*
    // If we're above the node, let go of the rope and move towards to node
    if(NEW_psCurrentNode->fY >= vPos.y) {
        // Let go of any rope
        cNinjaRope.Release();

        // Walk in the direction of the node
        ws->iMove = true;
    }
*/
  
/*	
	// TODO: do we realy need this? there is a loop above, which checks for direct connections to new nodes
	// Move to next node, if we arrived at the current
	if(CalculateDistance(vPos,CVec(NEW_psCurrentNode->fX,NEW_psCurrentNode->fY)) < 10)
		if(NEW_psCurrentNode->psNext)
			NEW_psCurrentNode = NEW_psCurrentNode->psNext;
*/	
}

void CWorm::NEW_AI_MoveToTargetDC(CMap *pcMap)
{
	/*

		Finding the target
		
	*/

	// Better target?
	CWorm *newtrg = findTarget(iAiGame, iAiTeams, iAiTag, pcMap);
	if (psAITarget && newtrg)
		if (newtrg->getID() != psAITarget->getID())
			nAIState = AI_THINK;

	if (!psAITarget && newtrg)  {
		nAIState = AI_THINK;
	}



	// No target?
	if (nAITargetType == AIT_NONE || (nAITargetType == AIT_WORM && !psAITarget))  {
		nAIState = AI_THINK;
		return;
	}




	/*

		Initialization

	*/

	// Get the worm state
    worm_state_t *ws = &tState;

    // Clear the state
	ws->iCarve = false;
	ws->iMove = false;
	ws->iShoot = false;
	ws->iJump = false;

	// Pathfinding problem, resort to simpler AI methods
	if (!NEW_psPath || !NEW_psCurrentNode || !NEW_psLastNode)  {
		AI_SimpleMove(pcMap);
		return;
	}

	// Get the final target position
	cPosTarget = AI_GetTargetPos();	

	// Get position of the node we're moving to
	CVec nodePos = CVec(NEW_psCurrentNode->fX,NEW_psCurrentNode->fY);




	/*

		Basic rope things that need to be done here

	*/

	// If the rope hook is attached, increase the attached time
	if (cNinjaRope.isAttached())
		fRopeAttachedTime += tLX->fDeltaTime;
	else
		fRopeAttachedTime = 0;
			
	// If the rope is falling, increase the falling time			
	if(cNinjaRope.isShooting() && !cNinjaRope.isAttached())
		fRopeHookFallingTime += tLX->fDeltaTime;
	else
		fRopeHookFallingTime = 0;
		




	/*

		Check, if we need to recalculate the path

	*/

    const int	nDeviation = 40;     // Maximum deviation allowance
    bool		recalculate = false; // Recalculate?  

	// Target deviated
	if (psAITarget && NEW_psPath && NEW_psLastNode)  {
		// Don't check when the worm is moving
		if (psAITarget->getVelocity()->GetLength() < 10.0f)   {
			// Deviated?
			if(CalculateDistance(cPosTarget,CVec(NEW_psLastNode->fX,NEW_psLastNode->fY)) > nDeviation)
				// Don't recalculate if the final target can be seen from end of the path
				if (!traceWormLine(AI_GetTargetPos(),CVec(NEW_psLastNode->fX,NEW_psLastNode->fY),pcMap,NULL))
					recalculate = true;
		}
	}

	// We can't see the current node
	if (!traceWormLine(nodePos,vPos,pcMap,NULL))
		recalculate = true;

    // Re-calculate the path?
    if(recalculate)  {
        NEW_AI_CreatePath(pcMap);
		return; // Quit
	}




	/*

		Get the position to get to

	*/

	float tolerance = 10.0f;  // How close we have to be to skip to the next node

	// First of all: if we see the final target, move to it
	if (traceWormLine(cPosTarget,vPos,pcMap,NULL))  {
		nodePos = cPosTarget;
	} else {
		// We don't see the final target

		// If we see any of the *next* nodes, skip to it
		NEW_ai_node_t *node = NEW_psCurrentNode;
		for(;node;node = node->psNext)  {
			if (traceWormLine(CVec(node->fX,node->fY),vPos,pcMap,NULL))  {
				NEW_psCurrentNode = node;
			}
		}

		// Update the node position (if needed)
		nodePos = CVec(NEW_psCurrentNode->fX,NEW_psCurrentNode->fY);

		// If the node is in the air, it's hard to "hit" it, so it's ok if we increase the tolerance
		if (NEW_AI_IsInAir(nodePos,pcMap))  {
			tolerance = 30.0f;
			if (NEW_AI_IsInAir(nodePos,pcMap,5))
				tolerance = 50.0f;
		}

		// If we reached the node, skip to next one
		if (CalculateDistance(vPos,nodePos) <= tolerance)  {
			if (NEW_psCurrentNode->psNext)  {
				NEW_psCurrentNode = NEW_psCurrentNode->psNext;
				nodePos = CVec(NEW_psCurrentNode->fX,NEW_psCurrentNode->fY);
			}
			else  {
				// Reached the end of the path
				// Don't quit, move to the target worm
				nodePos = cPosTarget;
			}
		}

	}

	/*

		Stucked

	*/

    // If we are stuck in the same position for a while, take measures to get out of being stuck
    if(fabs(cStuckPos.x - vPos.x) < 5 && fabs(cStuckPos.y - vPos.y) < 5) {
        fStuckTime += tLX->fDeltaTime;

		// Stucked
		if (fStuckTime > 2.0f)  {

			// Move
			ws->iMove = true;

			// Jump
			if (tLX->fCurTime - fLastJump > 1.0f)  {
				ws->iJump = true;
				fLastJump = tLX->fCurTime;
			}

			// Carve
			if (tLX->fCurTime-fLastCarve > 0.15f)  { // Not so fast
				fLastCarve = tLX->fCurTime;
				ws->iCarve = true; // Carve
			}
			else  {
				ws->iCarve = false;
			}

			// We cannot be here too long...
			if (fStuckTime > 5.0f)  {
				fStuckTime = 0;
				cStuckPos = vPos;
			}

			return;

		} // Stucked

	// Not stucked
	} else {
		cStuckPos = vPos;
		fStuckTime = 0;
	}




	/*

		Basic movement

	*/

	// Face the node
	if (tLX->fCurTime - fLastFace > 0.5f)  {  // Don't face so often
		if (nodePos.x < vPos.x)
			iDirection = DIR_LEFT;
		else
			iDirection = DIR_RIGHT;

		fLastFace = tLX->fCurTime;
	}

	// If the node is above us by a little, jump
	if((vPos.y-nodePos.y) <= 20.0f && (vPos.y-nodePos.y) > 5) {
		// Don't jump so often
		if (tLX->fCurTime - fLastJump > 1.0f)  {
			ws->iJump = true;
			fLastJump = tLX->fCurTime;
		} else
			ws->iMove = true; // If we should not jump, move
	}

	// If our velocity is small, move
	if (vVelocity.GetLength() < 20.0f)
		ws->iMove = true;


	// Carve
	if (tLX->fCurTime-fLastCarve > 0.2f)  { // Not so fast
		fLastCarve = tLX->fCurTime;
		ws->iCarve = true; // Carve
	}
	else  {
		ws->iCarve = false;
	}

	// Check, if there's some dirt between us and the next node
	float dist = 0;
	int type = PX_EMPTY;
	int len = traceLine(nodePos,pcMap,&dist,&type);
	if (type & PX_DIRT)  {
		// If the node is above us, find a suitable clearing weapon
		// TODO: use angle instead of this
		if (fabs(nodePos.x-vPos.x) < 50.0f && nodePos.y < vPos.y)  {
			int weap = AI_FindClearingWeapon();
			// Shoot the path
			if (weap != -1)  {
				iCurrentWeapon = weap;
				ws->iShoot = true;
			}
		}

		// Aim (so we carve in the right direction)
		AI_SetAim(nodePos);

		// When the dirt is close to us, don't shoot the rope or anything else
		if (len <= 20.0f)  {
			// Release the rope
			if (fRopeAttachedTime >= 1.9f)
				cNinjaRope.Release();

			// Don't do anything more
			return;
		}
	}

	// If we're heading to the node, but our velocity is too high, dampen
	bool heading = (vVelocity.x  < 0.0f && iDirection == DIR_LEFT) || (vVelocity.x  > 0.0f && iDirection == DIR_RIGHT);
	if (heading && vVelocity.GetLength() > 60.0f && CalculateDistance(nodePos,vPos) < 80.0f)  {

		// Move in the opposite direction
		iDirection = !iDirection;
		ws->iMove = true;

		// Let go the rope
		if (cNinjaRope.isAttached())
			cNinjaRope.Release();

		// We don't want the rope shoot to happen
		return;
	}



	/*

		Rope releasing and shooting

	*/

	//
	// Rope releasing 
	//

	// Release rope, if it forces us to the wrong direction
	if((cNinjaRope.GetForce(vPos).Normalize() + vPos - nodePos).GetLength2() > (vPos - nodePos).GetLength2())
		if (fRopeAttachedTime > 0.5f)
			cNinjaRope.Release();

	// If we're above the node and the rope is hooked wrong, release the rope
	if((vPos.y < nodePos.y) && fRopeAttachedTime > 2.0f && cNinjaRope.getHookPos().y-vPos.y < 0.0f) {
		cNinjaRope.Release();
	}

	// If the rope is falling too long, release it & walk
	if (fRopeHookFallingTime >= 2.0f)  {
		cNinjaRope.Release();
        ws->iMove = true;
		fRopeHookFallingTime = 0;
	}




	//
	//	Rope shooting
	//

    // If the node is above us by a lot, we should use the ninja rope
	// If the node is far, jump and use the rope, too
	bool fireNinja = (nodePos.y+35.0f < vPos.y);
	if (!fireNinja && (fabs(nodePos.x-vPos.x) >= 60.0f))  {
		// On ground? Jump
		if(CheckOnGround(pcMap)) {
			if (tLX->fCurTime - fLastJump > 1.0f)  {
				ws->iJump = true;
				fLastJump = tLX->fCurTime;
				// Rope will happen soon
			}
		}
		// Not on ground? Fire the ninja rope
		else
			fireNinja = true;
	}

	// Shoot the rope
    if(fireNinja) {
 
		// Get the best spot to shoot the rope to
		CVec cAimPos = NEW_AI_GetBestRopeSpot(nodePos,pcMap);

		// Don't use rope for small distances
		if (CalculateDistance(cAimPos,vPos) <= 20.0f)
			return;

		// Aim
		bool aim = AI_SetAim(cAimPos);


		// Get the direction vector
        CVec dir;
        dir.x=( (float)cos(fAngle * (PI/180)) );
	    dir.y=( (float)sin(fAngle * (PI/180)) );
	    if(iDirection == DIR_LEFT)
		    dir.x=(-dir.x);
       
        
        //  Got aim, so shoot a ninja rope
        //  Also shoot, if we are not on ground

        //  We shoot a ninja rope if it isn't shot
        //  Or if it is, we make sure it has pulled us up and that it is attached
        if(aim || !CheckOnGround(pcMap)) {
            if(!cNinjaRope.isReleased())
                cNinjaRope.Shoot(vPos,dir);
            else {
                float length = CalculateDistance(vPos, cNinjaRope.getHookPos());
                if(cNinjaRope.isAttached()) {
                    if(length < cNinjaRope.getRestLength() && vVelocity.y<-10)
                        cNinjaRope.Shoot(vPos,dir);
                }
            }
			fRopeHookFallingTime = 0;
			fRopeAttachedTime = 0;
        }
	} // fireNinja
}

NEW_ai_node_t* get_last_ai_node(NEW_ai_node_t* n) {
	if(!n) return NULL;
	for(;n->psNext;n=n->psNext) {}
	return n;
}

void delete_ai_nodes(NEW_ai_node_t* start) {
	if(!start) return;
	delete_ai_nodes(start->psNext);
	free(start);
}

void delete_ai_nodes(NEW_ai_node_t* start, NEW_ai_node_t* end) {
	if(!start || start == end) return;
	delete_ai_nodes(start->psNext, end);
	free(start);
}

float get_ai_nodes_length(NEW_ai_node_t* start) {
	float l,dx,dy;
	for(l=0;start;start=start->psNext) {
		if(start->psNext) {
			dx = start->fX - start->psNext->fX;
			dy = start->fY - start->psNext->fY;			
			l += sqrt(dx*dx + dy*dy);		
		} else
			break;
	}
	return l;
}

float get_ai_nodes_length2(NEW_ai_node_t* start) {
	float l,dx,dy;
	for(l=0;start;start=start->psNext) {
		if(start->psNext) {
			dx = start->fX - start->psNext->fX;
			dy = start->fY - start->psNext->fY;			
			l += dx*dx + dy*dy;
		} else
			break;
	}
	return l;
}
