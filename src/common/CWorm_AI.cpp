/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Worm class - AI
// Created 13/3/03
// Jason Boettcher
// Dark Charlie
// Albert Zeyer

// TODO: cleanup!!!

#include "LieroX.h"

#ifdef _MSC_VER
#pragma warning(disable: 4786)
#endif

#include <assert.h>
#include <set>

#include "MathLib.h"
#include "CClient.h"
#include "CBonus.h"
#include "GfxPrimitives.h"
#include "StringUtils.h"
#include "CWorm.h"


// we need it here for some debugging...
// we cannot define this globaly because some X11-header also defines this (which is not included here, so this works)
extern	SDL_Surface		*Screen;

static const unsigned short wormsize = 7;


/*
===============================

    Artificial Intelligence

===============================
*/


// returns the biggest possible free rectangle with the given point inside
// (not in every case, but in most it is the biggest; but it is ensured that the given rect is at least free)
// the return-value of type SquareMatrix consists of the top-left and upper-right pixel
// WARNING: if the given point is not in the map, the returned
// area is also not in the map (but it will handle it correctly)
SquareMatrix<int> getMaxFreeArea(VectorD2<int> p, CMap* pcMap, uchar checkflag) {
	// yes I know, statics are not good style,
	// but I will not recursivly use it and
	// the whole game is singlethreaded
	uint map_w = pcMap->GetWidth();
	uint map_h = pcMap->GetHeight();
    uint grid_w = pcMap->getGridWidth();
    uint grid_h = pcMap->getGridHeight();
	uint grid_cols = pcMap->getGridCols();
	const uchar* pxflags = pcMap->GetPixelFlags();
	const uchar* gridflags = pcMap->getAbsoluteGridFlags();

	SquareMatrix<int> ret;
	ret.v1 = p; ret.v2 = p;

	// just return if we are outside
	if(p.x < 0 || (uint)p.x >= map_w
	|| p.y < 0 || (uint)p.y >= map_h)
		return ret;

	enum { GO_RIGHT=1, GO_DOWN=2, GO_LEFT=4, GO_UP=8 }; short dir;
	unsigned short col;
	register int x=0, y=0;
	int grid_x=0, grid_y=0;
	bool avoided_all_grids;

	// loop over all directions until there is some obstacle
	col = 0; dir = 1;
	while(true) {
		if(col == 0xF) // got we collisions in all directions?
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
		if(x < 0 || (uint)x >= map_w
		|| y < 0 || (uint)y >= map_h) {
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
	if((uint)ret.v2.x >= map_w) ret.v2.x = map_w-1;
	if((uint)ret.v2.y >= map_h) ret.v2.y = map_h-1;

	return ret;
}




NEW_ai_node_t* createNewAiNode(float x, float y, NEW_ai_node_t* next = NULL, NEW_ai_node_t* prev = NULL) {
	NEW_ai_node_t* tmp = new NEW_ai_node_t;
	tmp->fX = x; tmp->fY = y;
	tmp->psNext = next; tmp->psPrev = prev;
	return tmp;
}

NEW_ai_node_t* createNewAiNode(NEW_ai_node_t* base) {
	if(!base) return NULL;
	NEW_ai_node_t* tmp = new NEW_ai_node_t;
	tmp->fX = base->fX; tmp->fY = base->fY;
	tmp->psNext = base->psNext; tmp->psPrev = base->psPrev;
	return tmp;
}


// returns true, if the given line is free or not
// these function will either go parallel to the x-axe or parallel to the y-axe
// (depends on which of them is the absolute greatest)
inline bool simpleTraceLine(CMap* pcMap, VectorD2<int> start, VectorD2<int> dist, uchar checkflag) {
	register const uchar* pxflags = pcMap->GetPixelFlags();
	if (!pxflags)  {  // The map has been probably shut down
		printf("WARNING: simpleTraceLine with pxflags==NULL\n");
		return false;
	}
	uint map_w = pcMap->GetWidth();
	uint map_h = pcMap->GetHeight();

	if(abs(dist.x) >= abs(dist.y)) {
		if(dist.x < 0) { // avoid anoying checks
			start.x += dist.x;
			dist.x = -dist.x;
		}
		if(start.x < 0 || (uint)(start.x + dist.x) >= map_w || start.y < 0 || (uint)start.y >= map_h)
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
		if(start.y < 0 || (uint)(start.y + dist.y) >= map_h || start.x < 0 || (uint)start.x >= map_w)
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
		short checklistRowHeight;
		short checklistCols;
		short checklistColStart;
		short checklistColWidth;

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
			base(b),
			checklistRows(0),
			checklistRowStart(0),
			checklistRowHeight(5),
			checklistCols(0),
			checklistColStart(0),
			checklistColWidth(5),
			inUse(false),
			bestNode(NULL) {}

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
			p_mset points(VectorD2__absolute_less<int>( base->target )); // + getCenter()*2 - start) / 2));

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
			searchpath_base* base;
			area_item* myArea;
			float bestNodeLen; // this can be done better...

			check_checkpoint(searchpath_base* b, area_item* a) :
				base(b),
				myArea(a),
				bestNodeLen(-1) {}

			// this will be called by forEachChecklistItem
			// pt is the checkpoint and dist the change to the new target
			// it will search for the best node starting at the specific pos
			inline bool operator() (VectorD2<int> pt, VectorD2<int> dist) {
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
				base->pcMap->lockFlags(false);
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
				base->pcMap->unlockFlags(false);

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
				bestNode = createNewAiNode((float)bestPosOutside.x, (float)bestPosOutside.y, bestNode);
				base->nodes.insert(bestNode);
				bestNode = createNewAiNode((float)bestPosInside.x, (float)bestPosInside.y, bestNode);
				base->nodes.insert(bestNode);
				return bestNode;
			}

			// nothing found
			return NULL;
		}

	}; // class area_item

	typedef std::multiset< area_item*, area_item::pseudoless > area_mset;
	typedef std::set< NEW_ai_node_t* > node_set;

	// these neccessary attributes have to be set manually
	CMap* pcMap;
	area_mset areas;
	node_set nodes;
	VectorD2<int> start, target;

	searchpath_base() :
		pcMap(NULL),
		resulted_path(NULL),
		thread(NULL),
        thread_is_ready(true),
		break_thread_signal(0),
		restart_thread_searching_signal(0) {
		thread_mut = SDL_CreateMutex();
		//printf("starting thread for %i ...\n", (long)this);
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
			nodes.erase(node);
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
		for(node_set::iterator it = nodes.begin(); it != nodes.end(); it++) {
			delete *it;
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
	// WARNING: you should never use this function directly;
	//          it is called inside from our searcher-thread
	// it searches for the path (recursive algo)
	NEW_ai_node_t* findPath(VectorD2<int> start) {
		// lower priority to this thread
		SDL_Delay(1);

		if(shouldBreakThread() || shouldRestartThread() || !pcMap->getCreated()) return NULL;

		// is the start inside of the map?
		if(start.x < 0 || (uint)start.x >= pcMap->GetWidth()
		|| start.y < 0 || (uint)start.y >= pcMap->GetHeight())
			return NULL;

		// can we just finish with the search?
		pcMap->lockFlags(false);
		if(traceWormLine(target, start, pcMap)) {
			pcMap->unlockFlags(false);
			// yippieh!
			NEW_ai_node_t* ret = createNewAiNode((float)target.x, (float)target.y);
			nodes.insert(ret);
			return ret;
		}
		pcMap->unlockFlags(false);

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
		pcMap->lockFlags(false);
		SquareMatrix<int> area = getMaxFreeArea(start, pcMap, PX_ROCK);
		pcMap->unlockFlags(false);
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
	// WARNING: the searcher-thread will clear all current saved nodes
	void startThreadSearch() {

		// if we are still searching, do nothing
		if(!isReady()) return;

		// this is the signal to start the search
		setReady(false);
	}

private:
	// main-function used by the thread
	static int threadSearch(void* b) {

		searchpath_base* base = (searchpath_base*)b;
		NEW_ai_node_t* ret;

		// Name the thread
#ifdef _MSC_VER
		static char name[32];
		sprintf(name,"AI thread %i",(int)b);
		nameThread(-1,name);
#endif // WIN32

		while(true) {
			// sleep a little bit while we have nothing to do...
			while(base->isReady()) {
				// was there a break-signal?
				if(base->shouldBreakThread()) {
					//printf("got break signal(1) for %i\n", (long)base);
					return 0;
				}
				SDL_Delay(100);
			}

			base->resulted_path = NULL;
			base->clear(); // this is save and important here, else we would have invalid pointers
			
			// start the main search
			ret = base->findPath(base->start);
			
			// finishing the result
			base->completeNodesInfo(ret);
			base->simplifyPath(ret);
			base->splitUpNodes(ret, NULL);
			base->resulted_path = ret;

			if(base->shouldRestartThread()) {
				// HINT: both locks (of shouldRestartThread and the following) are seperated
				//       this don't make any trouble, because here is the only place where we
				//       reset it and we have always restart_thread_searching_signal==true here
				base->lock();
				base->restart_thread_searching_signal = 0;
				base->start = base->restart_thread_searching_newdata.start;
				base->target = base->restart_thread_searching_newdata.target;
				base->unlock();
				continue;
			}

			// we are ready now
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

	// HINT: threadSearch is the only function, who should set this to true again!
	// a set to false means for threadSearch, that it should start the search now
	inline void setReady(bool state) {
		lock();
		thread_is_ready = state;
		unlock();
	}

	// WARNING: not thread safe; call isReady before
	inline NEW_ai_node_t* resultedPath() {
		return resulted_path;
	}

	inline void restartThreadSearch(VectorD2<int> newstart, VectorD2<int> newtarget) {
		// set signal
		lock();
		thread_is_ready = false;
		restart_thread_searching_newdata.start = newstart;
		restart_thread_searching_newdata.target = newtarget;
		// HINT: the reading of this isn't synchronized
		restart_thread_searching_signal = 1;
		unlock();
	}

private:
	NEW_ai_node_t* resulted_path;
	SDL_Thread* thread;
	SDL_mutex* thread_mut;
	bool thread_is_ready;
	int break_thread_signal;
	int restart_thread_searching_signal;
	class start_target_pair { public:
		VectorD2<int> start, target; 
	} restart_thread_searching_newdata;

	inline void breakThreadSignal() {
		// we don't need more thread-safety here, because this will not fail
		break_thread_signal = 1;
	}

	inline bool shouldBreakThread() {
		return (break_thread_signal != 0);
	}

public:
	inline bool shouldRestartThread() {
		return (restart_thread_searching_signal != 0);
	}

private:
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
				tmpnode = new NEW_ai_node_t;
				if(tmpnode) {
					nodes.insert(tmpnode);
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
bool CWorm::AI_Initialize() {
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
	fLastThink = -9999;
    bStuck = false;
	bPathFinished = true;
	//iAiGameType = GAM_OTHER;
	nAITargetType = AIT_NONE;
	nAIState = AI_THINK;
	fLastFace = -9999;
	fLastShoot = -9999;
	fLastCompleting = -9999;

	fCanShootTime = 0;

	fRopeAttachedTime = 0;
	fRopeHookFallingTime = 0;

	if(pathSearcher)
		printf("WARNING: pathSearcher is already initialized\n");
	else
		pathSearcher = new searchpath_base;
	if(!pathSearcher) {
		printf("ERROR: cannot initialize pathSearcher\n");
		return false;
	}
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

	// in every case, the nodes of the current path are not handled by pathSearcher
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
	start.x = (float)(rand() % pcMap->GetWidth());
	start.y = (float)(rand() % pcMap->GetHeight());
	target.x = (float)(rand() % pcMap->GetWidth());
	target.y = (float)(rand() % pcMap->GetHeight());

	fastTraceLine(target, start, pcMap, PX_ROCK,  debug_print_col(pcMap->GetDebugImage()));
}
#endif


void CWorm::AI_Respawn() {
	// find new target and reset the path

    // Search for an unfriendly worm
    psAITarget = findTarget(iAiGame, iAiTeams, iAiTag);

    // Any unfriendlies?
    if(psAITarget) {
        // We have an unfriendly target, so change to a 'move-to-target' state
        nAITargetType = AIT_WORM;
        nAIState = AI_MOVINGTOTARGET;
		NEW_AI_CreatePath(true);
	}
}

///////////////////
// Simulate the AI
void CWorm::AI_GetInput(int gametype, int teamgame, int taggame, int VIPgame, int flaggame, int teamflaggame)
{
	// Behave like humans and don't play immediatelly after spawn
	if ((tLX->fCurTime-fSpawnTime) < 0.4)
		return;

#ifdef _AI_DEBUG
/*	DrawRectFill(pcMap->GetDebugImage(),0,0,pcMap->GetDebugImage()->w,pcMap->GetDebugImage()->h,COLORKEY(pcMap->GetDebugImage()));
	do_some_tests_with_fastTraceLine(pcMap);
	usleep(1000000);
	return; */
#endif

	worm_state_t *ws = &tState;

	// If the worm is a flag don't let it move
	if(flaggame && getFlag())
		return;
	if(teamflaggame && getFlag())
		return;

	// Init the ws
	ws->iCarve = false;
	ws->iMove = false;
	ws->iShoot = false;
	ws->iJump = false;

	iAiGame = gametype;
	iAiTeams = teamgame;
	iAiTag = taggame;
	iAiVIP = VIPgame;
	iAiCTF = flaggame;
	iAiTeamCTF = teamflaggame;

    tLX->debug_string = "";

	iRandomSpread = 0;
	fLastRandomChange = -9999;

	iAiDiffLevel = tProfile->nDifficulty;

    // Every 3 seconds we run the think function
    if(tLX->fCurTime - fLastThink > 3 && nAIState != AI_THINK)
        nAIState = AI_THINK;

	// check more often if the path isn't finished yet
	if(tLX->fCurTime - fLastThink > 0.5 && !bPathFinished)
		nAIState = AI_THINK;

    // If we have a good shooting 'solution', shoot
    if(AI_Shoot()) {

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

   		if(CheckOnGround() && fRopeAttachedTime >= 0.3f && !NEW_AI_IsInAir(vPos))
   			cNinjaRope.Release();

    	return;

    } else {

		// Reload weapons when we can't shoot
		AI_ReloadWeapons();

    }

    // Process depending on our current state
    switch(nAIState) {

        // Think; We spawn in this state
        case AI_THINK:
            AI_Think(gametype, teamgame, taggame);
            break;

        // Moving towards a target
        case AI_MOVINGTOTARGET:
            NEW_AI_MoveToTarget();
            break;
    }
}


///////////////////
// Find a target worm
CWorm *CWorm::findTarget(int gametype, int teamgame, int taggame)
{
	CWorm	*w = cClient->getRemoteWorms();
	CWorm	*trg = NULL;
	CWorm	*nonsight_trg = NULL;
	float	fDistance = -1;
	float	fSightDistance = -1;

	int NumTeams = 0;
	uint i;
	for (i=0; i<4; i++)
		if (cClient->getTeamScore(i) > -1)
			NumTeams++;


    //
	// Just find the closest worm
	//

	for(i=0; i<MAX_WORMS; i++, w++) {
		if(w == NULL) break;

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

		// If this is a VIP game target:
		// Red worms if Blue
		// Green & Blue worms if Red
		// Blue worms if green
		if(iAiVIP && iTeam == 0 && w->getTeam() != 1)
			continue;
		if(iAiVIP && iTeam == 1 && w->getTeam() == 1)
			continue;
		if(iAiVIP && iTeam == 2 && w->getTeam() != 0)
			continue;

		// If this is a capture the flag game just aim to get the flag
		if(iAiCTF && !w->getFlag())
			continue;

		// If this is a teams capture the flag game just aim to get the flag
		if(iAiTeamCTF && !w->getFlag())
			continue;

		// Calculate distance between us two
		float l = (w->getPos() - vPos).GetLength2();

		// Prefer targets we have free line of sight to
		float length;
		int type;
		traceLine(w->getPos(),&length,&type,1);
		if (! (type & PX_ROCK))  {
			// Line of sight not blocked
			if (fSightDistance < 0 || l < fSightDistance)  {
				trg = w;
				fSightDistance = l;
				if (fDistance < 0 || l < fDistance)  {
					nonsight_trg = w;
					fDistance = l;
				}
			}
		}
		else
			// Line of sight blocked
			if(fDistance < 0 || l < fDistance) {
				nonsight_trg = w;
				fDistance = l;
			}
	}

	// If the target we have line of sight to is too far, switch back to the closest target
	if (fSightDistance-fDistance > 50.0f || /*TODO: in rifles we can shoot target far away!*/iAiGameType == GAM_RIFLES || trg == NULL)  {
		if (nonsight_trg)
			trg = nonsight_trg;
	}

    return trg;
}


///////////////////
// Think State
void CWorm::AI_Think(int gametype, int teamgame, int taggame)
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
        if(AI_FindHealth())
            return;


    // Search for an unfriendly worm
    psAITarget = findTarget(gametype, teamgame, taggame);

    // Any unfriendlies?
    if(psAITarget) {
        // We have an unfriendly target, so change to a 'move-to-target' state
        nAITargetType = AIT_WORM;
        nAIState = AI_MOVINGTOTARGET;
        //AI_InitMoveToTarget();
		NEW_AI_CreatePath();
        return;
    }
	else
		fLastShoot = -9999;

    // If we're down on health (less than 80%) we should look for a health bonus
    if(iHealth < 80) {
        printf("we should look for health\n");
        if(AI_FindHealth())
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

		printf("something in thinking\n");
		// Nothing todo, so go find some health if we even slightly need it
		if(iHealth < 100) {
			if(AI_FindHealth())
				return;
		}
        return;
	}

    int     cols = pcMap->getGridCols()-1;       // Note: -1 because the grid is slightly larger than the
    int     rows = pcMap->getGridRows()-1;       // level size

    // Find a random spot to go to high in the level
    printf("I don't find any target, so let's get somewhere (high)\n");
    int x, y, c;
    for(c=0; c<10; c++) {
		x = (int)(fabs(GetRandomNum()) * (float)cols);
		y = (int)(fabs(GetRandomNum()) * (float)rows / 5); // little hack to go higher

		uchar pf = *(pcMap->getGridFlags() + y*pcMap->getGridCols() + x);

		if(pf & PX_ROCK)
			continue;

		// Set the target
		cPosTarget = CVec((float)(x*pcMap->getGridWidth()+(pcMap->getGridWidth()/2)), (float)(y*pcMap->getGridHeight()+(pcMap->getGridHeight()/2)));
		nAITargetType = AIT_POSITION;
		nAIState = AI_MOVINGTOTARGET;
		//AI_InitMoveToTarget();
		NEW_AI_CreatePath();
		break;
    }
}


///////////////////
// Find a health pack
// Returns true if we found one
bool CWorm::AI_FindHealth()
{
	if (!tGameInfo.iBonusesOn)
		return false;

    CBonus  *pcBonusList = cClient->getBonusList();
    short     i;
    CBonus  *pcBonus = NULL;
    float   dist2 = -1;
	float d2;

    // Find the closest health bonus
    for(i=0; i<MAX_BONUSES; i++) {
        if(pcBonusList[i].getUsed() && pcBonusList[i].getType() == BNS_HEALTH) {

            d2 = (pcBonusList[i].getPosition() - vPos).GetLength2();

            if(dist2 < 0 || d2 < dist2) {
                pcBonus = &pcBonusList[i];
                dist2 = d2;
            }
        }
    }


    // TODO: Verify that the target is not in some small cavern that is hard to get to


    // If we have found a bonus, setup the state to move towards it
    if(dist2 >= 0) {
        psBonusTarget = pcBonus;
        nAITargetType = AIT_BONUS;
        nAIState = AI_MOVINGTOTARGET;
		NEW_AI_CreatePath();
		return true;
    }

    return false;
}


///////////////////
// Reloads the weapons
void CWorm::AI_ReloadWeapons(void)
{
    ushort  i;

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
void CWorm::AI_InitMoveToTarget()
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
    psPath = AI_ProcessNode(NULL, curX,curY, tarX, tarY);
    psCurrentNode = psPath;

    fLastPathUpdate = tLX->fCurTime;

    // Draw the path
    if(!psPath)
        return;

#ifdef _AI_DEBUG
    pcMap->DEBUG_DrawPixelFlags();
    AI_DEBUG_DrawPath(psPath);
#endif // _AI_DEBUG

}


///////////////////
// Path finding
ai_node_t *CWorm::AI_ProcessNode(ai_node_t *psParent, int curX, int curY, int tarX, int tarY)
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
    if((pf & PX_ROCK) && !bFound) {
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
            node->psChildren[i] = AI_ProcessNode(node, curX+diagCase[i*2], curY+diagCase[i*2+1], tarX, tarY);
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
        h = v = 0;
        
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

        node->psChildren[i] = AI_ProcessNode(node, curX+h, curY+v, tarX, tarY);
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
void CWorm::AI_MoveToTarget()
{
    int     nCurrentCell[2];
    int     nTargetCell[2];
    int     nFinalTarget[2];
    worm_state_t *ws = &tState;

    int     hgw = pcMap->getGridWidth()/2;
    int     hgh = pcMap->getGridHeight()/2;

	// Better target?
	CWorm *newtrg = findTarget(iAiGame, iAiTeams, iAiTag);
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
        AI_InitMoveToTarget();


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
          AI_DEBUG_DrawPath(psPath);
		#endif // _AI_DEBUG

        // If we don't have a path, resort to simpler AI methods
        AI_SimpleMove(psAITarget != NULL);
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
        traceLine(CVec((float)(node->nX*pcMap->getGridWidth()+hgw), (float) (node->nY*pcMap->getGridHeight()+hgh)), &tdist, &type,1);
        if(tdist < 0.75f && (type & PX_ROCK))
            continue;

        // If we are near the node skip ahead to it
        if(abs(node->nX - nCurrentCell[0]) < nNodeProx &&
           abs(node->nY - nCurrentCell[1]) < nNodeProx) {

            psCurrentNode = node;

			#ifdef _AI_DEBUG
				pcMap->DEBUG_DrawPixelFlags();
				AI_DEBUG_DrawPath(node);
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
				AI_DEBUG_DrawPath(node);
			#endif  // _AI_DEBUG
        }
    }

    if(psCurrentNode == NULL || psPath == NULL) {
         // If we don't have a path, resort to simpler AI methods
        AI_SimpleMove(psAITarget != NULL);
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

			if (tLX->fCurTime - fLastFace >= 0.5f)  {
				iDirection = !iDirection;
				fLastFace = tLX->fCurTime;
			}

            fAngle -= 45;
            // Clamp the angle
	        fAngle = MIN((float)60,fAngle);
	        fAngle = MAX((float)-90,fAngle);

            // Recalculate the path
            AI_InitMoveToTarget();
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
                if(cNinjaRope.isAttached()) {
                    if((vPos - cNinjaRope.getHookPos()).GetLength2() < cNinjaRope.getRestLength()*cNinjaRope.getRestLength() && vVelocity.y<-10)
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
    int length = traceLine(v, &traceDist, &type);
    if((float)(length*length) <= (v-vPos).GetLength2() && (type & PX_DIRT)) {
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

		if(!IsEmpty(CELL_DOWN))  {
			if(IsEmpty(CELL_LEFT))
				iDirection = DIR_LEFT;
			else if (IsEmpty(CELL_RIGHT))
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
void CWorm::AI_DEBUG_DrawPath(ai_node_t *node)
{
	return;

    if(!node)
        return;

    int x = node->nX * pcMap->getGridWidth();
    int y = node->nY * pcMap->getGridHeight();

    //DrawRectFill(pcMap->GetImage(), x,y, x+pcMap->getGridWidth(), y+pcMap->getGridHeight(), tLX->clBlack);

    if(node->psPath) {
        int cx = node->psPath->nX * pcMap->getGridWidth();
        int cy = node->psPath->nY * pcMap->getGridHeight();
        DrawLine(pcMap->GetDrawImage(), (x+pcMap->getGridWidth()/2)*2, (y+pcMap->getGridHeight()/2)*2,
                                    (cx+pcMap->getGridWidth()/2)*2,(cy+pcMap->getGridHeight()/2)*2,
                                    tLX->clWhite);
    }
    else {
        // Final target
        DrawRectFill(pcMap->GetDrawImage(), x*2-5,y*2-5,x*2+5,y*2+5, MakeColour(0,255,0));

    }

    AI_DEBUG_DrawPath(node->psPath);
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

        // Position target
        case AIT_POSITION:
            return cPosTarget;

        // Worm target
        case AIT_WORM:
        default:
        	if(nAITargetType != AIT_WORM)
        		nAIState = AI_THINK;
            if(psAITarget) {
                if(!psAITarget->getAlive() || !psAITarget->isUsed())
                    nAIState = AI_THINK;
                return psAITarget->getPos();
            }
            break;

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

	NormalizeVector(&tgDir);

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
void CWorm::AI_SimpleMove(bool bHaveTarget)
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
    traceLine(cPosTarget, &fDist, &type, 1);
    if(fDist < 0.75f || cPosTarget.y < vPos.y) {

        // Change direction
		if (bHaveTarget && (tLX->fCurTime-fLastFace) > 1.0)  {
			iDirection = !iDirection;
			fLastFace = tLX->fCurTime;
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
/*
// TODO: it's deprecated; delete this function
void CWorm::AI_PreciseMove(CMap *pcMap)
{
    worm_state_t *ws = &tState;

    //strcpy(tLX->debug_string, "AI invoked");

    ws->iJump = false;
    ws->iMove = false;
    ws->iCarve = false;


    // If we're insanely close, just stop
    if(fabs(vPos.x - cPosTarget.x) < 10 && fabs(vPos.y - cPosTarget.y) < 10) {
 */		/*if (tLX->fCurTime - fLastDirChange > 2.0f)  {
			if (GetRandomNum() < 0)
				iDirection = DIR_LEFT;
			else
				iDirection = DIR_RIGHT;
			fLastDirChange = tLX->fCurTime;
		}
		ws->iMove = true; */
   /*
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
*/

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
			static std::string name;
			name = tWeapons[i].Weapon->Name;
			stringlwr(name);
			if(strincludes(name,"dirt") || strincludes(name,"napalm") || strincludes(name,"grenade") || strincludes(name,"nuke") || strincludes(name,"mine"))
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




////////////////////
// Returns true, if the weapon can hit the target
bool CWorm::weaponCanHit(int gravity, float speed, CVec cTrgPos)
{
	// Get the target position
	if(!psAITarget)
		return false;

	CVec *from = &vPos;
	CVec *to = &cTrgPos;

	// Convert the alpha to radians
	float alpha = DEG2RAD(fAngle);
	// Get the maximal X
	int max_x = (int)(to->x - from->x);

	wpnslot_t* wpnslot = getWeapon(getCurrentWeapon());
	weapon_t* wpn = wpnslot ? wpnslot->Weapon : NULL;
	proj_t* wpnproj = wpn ? wpn->Projectile : NULL;
	if(!wpnproj) {
		printf("ERROR: cannot determinit wpnproj\n");
		return false;
	}

	// Get the maximal Y
	int max_y = (int)(from->y-to->y);

	/*if (max_y > 0)
		alpha = 180-alpha;*/

	// Check the pixels in the projectile trajectory
	int x,y;

#ifdef _AI_DEBUG
	//pcMap->ClearDebugImage();
	//DrawRectFill(pcMap->GetDebugImage(),cTrgPos.x*2-2,cTrgPos.y*2-2,cTrgPos.x*2+2,cTrgPos.y*2+2,tLX->clWhite);
#endif


	float tmp;
	float cos_alpha = cos(alpha);
	float tan_alpha = tan(alpha);
	y = 0;
	int dy;

	if (max_x == 0) {
		if(speed && fabs(cos_alpha)>0.1f) return false;
		float fDist;
		int nType = PX_EMPTY;
		traceWeaponLine(cTrgPos,&fDist,&nType);
		return (nType == PX_EMPTY);
	}

	if (max_x > 0)  {
		for (x=0;x<max_x;x+=2)  {
			tmp = (2*speed*speed*cos_alpha*cos_alpha);
			if(tmp != 0)
				dy = -x*(int)(tan_alpha+(gravity*x*x)/tmp) - y;
			else
				return false;
			y += dy;

			// If we have reached the target, the trajectory is free
			if (max_y < 0)  {
				if (y < max_y)
					return true;
			} else  {
				if (y > max_y)
					return true;
			}

			// Rock or dirt, trajectory not free
			/*if (pcMap->GetPixelFlag(x+(int)from->x,y+(int)from->y) & (PX_ROCK|PX_DIRT))  {
				return false;
			}*/
			if(CProjectile::CheckCollision(wpnproj,1,pcMap,*from+CVec((float)x, (float)y),CVec(2.0f, (float)dy)))
				return false;

	#ifdef _AI_DEBUG
			//PutPixel(pcMap->GetDebugImage(),x*2+(int)from->x*2,y*2+(int)from->y*2,tLX->clWhite);
	#endif
		}
	}
	else  {
		for (x=0;x>max_x;x-=2)  {
			tmp = (2*speed*speed*cos_alpha*cos_alpha);
			if(tmp != 0)
				dy = -x*(int)(tan_alpha+(gravity*x*x)/tmp) - y;
			else
				return false;
			y += dy;

			// If we have reached the target, the trajectory is free
			if (max_y < 0)  {
				if (y < max_y)
					return true;
			} else  {
				if (y > max_y)
					return true;
			}

			// Rock or dirt, trajectory not free
			/*if (pcMap->GetPixelFlag(x+(int)from->x,y+(int)from->y) & (PX_ROCK|PX_DIRT))  {
				return false;
			}*/
			if(CProjectile::CheckCollision(wpnproj,1,pcMap,*from+CVec((float)x, (float)y), CVec(-2.0f, (float)dy)))
				return false;

	#ifdef _AI_DEBUG
			//PutPixel(pcMap->GetDebugImage(),x*2+(int)from->x*2,y*2+(int)from->y*2,tLX->clWhite);
	#endif
		}
	}

	// Target reached
	return true;
}


bool AI_GetAimingAngle(float v, int g, float x, float y, float *angle)
{
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
bool CWorm::AI_Shoot()
{
    // Make sure the target is a worm
    if(nAITargetType != AIT_WORM)
        return false;

    // Make sure the worm is good
    if(!psAITarget || !psAITarget->getAlive() || !psAITarget->isUsed()) {
        nAIState = AI_THINK;
        return false;
    }


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
    float d = (cTrgPos - vPos).GetLength();
 /*   if(d > 300.0f && iAiGameType != GAM_RIFLES)
        return false; */

/*	// If we're on the target, simply shoot // this doesn't work in every case
	if(d < 10.0f)
		return true;
*/

	// TODO: first choose the weapon, and then use weaponCanHit instead of traceWeaponLine
	// hm, i would say, the other way around is better

    float fDist;
    int nType = -1;
    int length = 0;

	length = traceWeaponLine(cTrgPos, &fDist, &nType);

    // If target is blocked by rock we can't use direct firing
    if(nType & PX_ROCK)  {
		bDirect = false;
	}

	// Don't shoot teammates
	if(tGameInfo.iGameMode == GMT_TEAMDEATH && (nType & PX_WORM)) {
		printf("we don't want shoot teammates\n");
		return false;
	}

	// If target is blocked by large amount of dirt, we can't shoot it
	if (nType & PX_DIRT)  {
		if(d-fDist > 40.0f)
			bDirect = false;
	}

	// In mortar game there must be enough of free cells around us
	if (iAiGameType == GAM_MORTARS)  {
		if (!NEW_AI_CheckFreeCells(1))  {
//			bDirect = false;
			//printf("not enough free cells\n");
			return false;
		}
		if(bDirect && !traceWormLine(cTrgPos,vPos,pcMap))
			bDirect = false;
	}

    // Set the best weapon for the situation
    // If there is no good weapon, we can't shoot
    tLX->debug_float = d;
    int wpn = AI_GetBestWeapon(iAiGameType, d, bDirect, fDist);
    if(wpn < 0) {
        //strcpy(tLX->debug_string, "No good weapon");
        printf("I could not find any useable weapon\n");
        return false;
    }

    iCurrentWeapon = wpn;

    bool bAim = false;

	float alpha = 0;

	gs_worm_t *wd = cGameScript->getWorm();
	if (!wd)
		return false;

    // Aim in the right direction to account of weapon speed, gravity and worm velocity
	weapon_t *weap = getCurWeapon()->Weapon;
	if(weap && weap->Projectile) switch (weap->Projectile->Hit_Type)  {
	//case PJ_NOTHING:
	//case PJ_CARVE:
	case PJ_DIRT:
	case PJ_GREENDIRT:
		printf("hit_type is %i\n", weap->Projectile->PlyHit_Type);
		// don't shoot this shit
		break;
	default:
		CVec direction = (psAITarget->getPos() - vPos).Normalize();
		// speed of target in the direction (moving away from us)
		float targ_speed = direction.Scalar(*psAITarget->getVelocity());
		float my_speed = direction.Scalar(vVelocity);

		// Projectile speed (see CClient::ProcessShot for reference) - targ_speed
		float v = (float)weap->ProjSpeed/* *weap->Projectile->Dampening */ + weap->ProjSpeedVar*100.0f + my_speed;
		if(v < 0) {
			// we have high velocities, danger to shot...
			// if v<0, we would shoot in the wrong direction
			//printf("velocities(%f) too high...\n", v);
		/*	printf("  ProjSpeed = %f\n", (float)weap->ProjSpeed);
			printf("  Dampening = %f\n", (float)weap->Projectile->Dampening);
			printf("  ProjSpeedVar = %f\n", weap->ProjSpeedVar);
			printf("  my_speed = %f\n", my_speed);
			printf("  targ_speed = %f\n", targ_speed); */
			bAim = false;
			break;
		}

		// Distance
		float x = (cTrgPos.x-vPos.x);
		float y = (vPos.y-cTrgPos.y); // no PC-koord but real-world-koords

		// Count with the gravity of the target worm
		/*if (iAiGameType == GAM_RIFLES && psAITarget->CheckOnGround(pcMap))  {
			float flight_time = x*x+y*y/v;
			CVec trg_arriv_speed = CVec(psAITarget->getVelocity()->x,psAITarget->getVelocity()->y+wd->Gravity*flight_time/2);
			v += targ_speed;  // Get rid of the old target speed

			// Add the new target speed
			direction = (psAITarget->getPos() - vPos).Normalize();
			targ_speed =  direction.Scalar(trg_arriv_speed);
			v -= targ_speed;
		}*/


		// how long it takes for hitting the target
		float apriori_time = v ? sqrt(x*x + y*y) / v : 0;
		apriori_time *= 0.7f; // it's no direct line but a polynom, so this could give better results :)
		if(apriori_time < 0) {
			// target is faster than the projectile
			// shoot somewhere in the other direction
//			v = -v; apriori_time = -apriori_time;
//			x = -x; y = -y;
			// perhaps, this is good
//			tState.iJump = true;
			printf("target is too fast! my speed: %f, trg speed: %f, my abs speed: %f, trg abs speed: %f, proj speed: %f+%f\n",my_speed,targ_speed,vVelocity.GetLength(),psAITarget->getVelocity()->GetLength(),(float)weap->ProjSpeed*weap->Projectile->Dampening,weap->ProjSpeedVar*100.0f);

		} else { // apriori_time >= 0
			// where the target would be
			x += apriori_time*psAITarget->getVelocity()->x;
			y -= apriori_time*psAITarget->getVelocity()->y; // HINT: real-world-koords
		}

		// Gravity
		int	g = 100;
		if(weap->Projectile->UseCustomGravity)
			g = weap->Projectile->Gravity;

		proj_t *tmp = weap->Projectile;
		while(tmp)  {
			if (tmp->UseCustomGravity)  {
				if (tmp->Gravity > g)
					g = tmp->Gravity;
			} else
				if (g < 100)
					g = 100;

			// If there are any other projectiles, that are spawned with the main one, try their gravity
			if (tmp->Timer_Projectiles)  {
				if (tmp->Timer_Time >= 0.5f)
					break;
			}
			else if (tmp->Hit_Projectiles || tmp->PlyHit_Projectiles || tmp->Tch_Projectiles)
				break;

			tmp = tmp->Projectile;
		}

		// Get the alpha
		bAim = AI_GetAimingAngle(v,g,x,y,&alpha);
		if (!bAim) {
			//printf("cannot calc the alpha, v=%f, g=%i, x=%f, y=%f\n", v,g,x,y);
			break;
		}

		// AI diff level
		// Don't shoot so exactly on easier skill levels
		int diff[4] = {13,8,3,0};

		if (tLX->fCurTime-fLastRandomChange >= 0.5f)  {
			iRandomSpread = GetRandomInt(diff[iAiDiffLevel]) * SIGN(GetRandomNum());
			fLastRandomChange = tLX->fCurTime;
		}

		alpha += iRandomSpread;

		// Can we hit the target?
		//printf("proj-speed: %f\n", v);
		if (g <= 10 || v >= 200)  {
			// we already have bDirect==false, if we have no direct free way
			bAim = bDirect;
		}
		else {
			bAim = weaponCanHit(g,v,CVec(vPos.x+x,vPos.y-y));
			//if(!bAim) printf("weapon can't hit target, g=%i, v=%f, x=%f, y=%f\n", g,v,x,y);
		}

		if (!bAim)
			break;

		if (fabs(fAngle-alpha) > 5.0)  {
			// Move the angle at the same speed humans are allowed to move the angle
			if(alpha > fAngle)
				fAngle += wd->AngleSpeed * tLX->fDeltaTime;
			else if(alpha < fAngle)
				fAngle -= wd->AngleSpeed * tLX->fDeltaTime;
			// still aiming ...
			bAim = false;
		}
		else
			fAngle = alpha;

		// Face the target
		if (x < 0)
			iDirection = DIR_LEFT;
		else
			iDirection = DIR_RIGHT;

		//if(bAim) printf("shooting!!!\n");

		/*strcpy(tLX->debug_string,weap->Name);
		if (tLX->fCurTime-flast > 1.0f)  {
			tLX->debug_float = alpha;
			flast = tLX->fCurTime;
		}*/
		break;
		//printf("wp type is BEAM %s\n", bAim ? "and we are aiming" : "and no aim");
	}

	//
	// If there's some lag or low FPS, don't shoot in the direction of our flight (avoid suicides)
	//

	// HINT: we don't need this, because we ensure above in the speed-calculation, that we have no problem
	// TODO: avoiding projectiles should not be done by not shooting but by changing MoveToTarget
	if(bAim) if (GetFPS() < 75 || tGameInfo.iGameType == GME_JOIN)  {
		// Get the angle
		float ang = (float)atan2(vVelocity.x, vVelocity.y);
		ang = RAD2DEG(ang);
		if(iDirection == DIR_LEFT)
			ang+=90;
		else
			ang = -ang + 90;

		// Cannot shoot
		if (fabs(fAngle-ang) <= 30 && vVelocity.GetLength2() >= 900.0f && weap->Type != WPN_BEAM)  {
			if (weap->Type == WPN_PROJECTILE)  {
				if (weap->Projectile->PlyHit_Damage > 0)
					return false;
			}
		}
	}

    if(!bAim)  {

  		// we cannot shoot here

		// TODO: do we need this?
		fBadAimTime += tLX->fDeltaTime;
		if((fBadAimTime) > 4) {
			if(IsEmpty(CELL_UP))
				tState.iJump = true;
			fBadAimTime = 0;
		}

		fCanShootTime = 0;

        return false;
	}

	// Reflexes :)
	// TODO: this doesn't work atm
	/*float diff[4] = {0.45f,0.35f,0.25f,0.0f};
	fCanShootTime += tLX->fDeltaTime;
	if (fCanShootTime <= diff[iAiDiffLevel])  {
		return false;
	}*/

	fBadAimTime = 0;

    // Shoot
	tState.iShoot = true;
	fLastShoot = tLX->fCurTime;
	return true;
}


///////////////////
// AI: Get the best weapon for the situation
// Returns weapon id or -1 if no weapon is suitable for the situation
int CWorm::AI_GetBestWeapon(int nGameType, float fDistance, bool bDirect, float fTraceDist) {
	// if we are to close to the target, don't selct any weapon (=> move away)
	/*if(fDistance < 5)
		return -1; */

	float diff[4] = {0.50f,0.30f,0.20f,0.12f};

    // We need to wait a certain time before we change weapon
    if( tLX->fCurTime - fLastWeaponChange > diff[iAiDiffLevel] )
        fLastWeaponChange = tLX->fCurTime;
    else
        return iCurrentWeapon;

	// For rifles and mortars just get the first unreloaded weapon
	if (iAiGameType == GAM_RIFLES || iAiGameType == GAM_MORTARS)  {
		for (int i=0; i<5; i++)
			if (!tWeapons[i].Reloading)
				return i;
		//printf("GAM_RIFLES|GAM_MORTARS: all weapons still are reloading...\n");
		return -1;
	}

    CVec    cTrgPos = AI_GetTargetPos();


	if (iAiGameType == GAM_100LT)  {
		// We're above the worm

		// If we are close enough, shoot the napalm
		if (vPos.y <= cTrgPos.y && (vPos-cTrgPos).GetLength2() < 10000.0f)  {
			if (traceWormLine(cTrgPos,vPos,pcMap) && !tWeapons[1].Reloading)
				if (psAITarget)
					if (psAITarget->CheckOnGround())
						return 1;
		}



		float d = (vPos-cTrgPos).GetLength();
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
				printf("GAM_100LT: i think we should not shoot here\n");
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
        if (!NEW_AI_CheckFreeCells(5)) {
			printf("we should not shoot because of the hints everywhere\n");
			return -1;
        }

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

//	printf("simply everything failed, no luck with that\n");
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
int CWorm::traceLine(CVec target, float *fDist, int *nType, int divs)  {
	int res =  traceLine(target, vPos, nType, divs);
	if (fDist && res) *fDist = (int)(vPos - target).GetLength() / res;
	return res;
}

int CWorm::traceLine(CVec target, CVec start, int *nType, int divs, uchar checkflag)
{
    assert( pcMap );

    // Trace a line from the worm to length or until it hits something
	CVec pos = start;
	CVec dir = target-pos;
    int nTotalLength = (int)NormalizeVector(&dir);

	int divisions = divs;			// How many pixels we go through each check (more = slower)

	if( nTotalLength < divisions)
		divisions = nTotalLength;

	if (nType)
		*nType = PX_EMPTY;

	// Make sure we have at least 1 division
	divisions = MAX(divisions,1);

	int i;
	for(i=0; i<nTotalLength; i+=divisions) {
		uchar px = pcMap->GetPixelFlag( (int)pos.x, (int)pos.y );
		//pcMap->PutImagePixel((int)pos.x, (int)pos.y, MakeColour(255,0,0));

        if(!(px & checkflag)) {
			if (nType)
				*nType = px;
            return i;
        }

        pos = pos + dir * (float)divisions;
    }

    return nTotalLength;
}

///////////////////
// Returns true, if the cell is empty
// Cell can be: CELL_CURRENT, CELL_LEFT,CELL_DOWN,CELL_RIGHT,CELL_UP,CELL_LEFTDOWN,CELL_RIGHTDOWN,CELL_LEFTUP,CELL_RIGHTUP
bool CWorm::IsEmpty(int Cell)
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

  if ((cx < 0 || cx > pcMap->getGridCols()))
	  return false;

  if ((cy < 0 || cy > pcMap->getGridRows()))
	  return false;

  /*int dx = cx*pcMap->getGridCols()*2;
  int dy = pcMap->getGridRows()*2*cy;

  DrawRect(pcMap->GetDrawImage(),dx,dy,dx+(pcMap->GetWidth()/pcMap->getGridCols())*2,dy+(pcMap->GetHeight()/pcMap->getGridRows())*2,tLX->clWhite);

  dx /= 2;
  dy /= 2;
  DrawRect(pcMap->GetImage(),dx,dy,dx+pcMap->GetWidth()/pcMap->getGridCols(),dy+pcMap->GetHeight()/pcMap->getGridRows(),tLX->clWhite);*/

  const uchar   *f = pcMap->getGridFlags() + cy*pcMap->getGridWidth()+cx;
  bEmpty = *f == PX_EMPTY;

  return bEmpty;
}

/////////////////////////////
// TEST TEST TEST
//////////////////
// Finds the nearest free cell in the map and returns coordinates of its midpoint
CVec CWorm::NEW_AI_FindClosestFreeCell(CVec vPoint)
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
int CWorm::traceWeaponLine(CVec target, float *fDist, int *nType)
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
	if (cClient && (tGameInfo.iGameMode == GMT_TEAMDEATH || tGameInfo.iGameMode == GMT_VIP))  {
		CWorm *w = cClient->getRemoteWorms();
		for (i=0;i<MAX_WORMS;i++,w++)  {
			if (w) {
				if(w->isUsed() && w->getAlive() && w->getVIP() && iTeam == 0 && tGameInfo.iGameMode == GMT_VIP)
					WormsPos[WormCount++] = w->getPos();
				if (w->isUsed() && w->getAlive() && w->getTeam() == iTeam && w->getID() != iID)
					WormsPos[WormCount++] = w->getPos();
			}
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
			if ((pos-WormsPos[j]).GetLength2() < 400.0f)  {
				if(nTotalLength != 0)
					*fDist = (float)i / (float)nTotalLength;
				else
					*fDist = 0;
				*nType = *nType | PX_WORM;
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
	CVec start;
	CVec* collision;
	bool hit;

	set_col_and_break(CVec st, CVec* col) : start(st), collision(col), hit(false) {}
	bool operator()(int x, int y) {
		hit = true;
		if(collision && (*collision - start).GetLength2() > (CVec((float)x,(float)y) - start).GetLength2()) {
			collision->x = (float)x;
			collision->y = (float)y;
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
bool CWorm::NEW_AI_CheckFreeCells(int Num)
{
	// Get the cell
	int cellX = (int) fabs((vPos.x)/pcMap->getGridWidth());
	int cellY = (int) fabs((vPos.y)/pcMap->getGridHeight());

	// First of all, check our current cell
	if (*(pcMap->getGridFlags() + cellY*pcMap->getGridCols() +cellX) & PX_ROCK)
		return false;

/*#ifdef _AI_DEBUG
	pcMap->ClearDebugImage();
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
		for (x = cellX - Num - 1; x < cellX; x++)  {
			// Clipping means rock
			if ((x < 0 || x > pcMap->getGridCols()))
				return false;
			for (y = cellY + dir; y < cellY + dir + Num; y++)  {
				// Clipping means rock
				if ((y < 0 || y > pcMap->getGridRows()))
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
			if((x< 0 || x > pcMap->getGridCols()))
				return false;
			for(y=cellY+dir;y<cellY+dir+Num;y++)  {
				// Clipping means rock
				if((y < 0 || y > pcMap->getGridRows()))
					return false;

				// Rock cell
				if(*(pcMap->getGridFlags() + y*pcMap->getGridCols() +x) & PX_ROCK)
					return false;
			}
		}

		return true;
	}

	// Weird, shouldn't happen
	printf("ouh, what???\n");
	return false;
}


////////////////////
// Creates the path
int CWorm::NEW_AI_CreatePath(bool force_break)
{

	CVec trg = AI_GetTargetPos();
	if (psAITarget && nAITargetType == AIT_WORM)
		trg = NEW_AI_FindShootingSpot();  // If our target is an enemy, go to the best spot for shooting

	if(force_break) {
		bPathFinished = false;
		fSearchStartTime = tLX->fCurTime;
		((searchpath_base*)pathSearcher)->restartThreadSearch(vPos, trg);
		
		return false;
	}

	// bPathFinished also implicates, that there is currently no search
	// if bPathFinished is set in the wrong way, this will result in multithreading errors!
	// TODO: something has to be changed here, because the name bPathFinished doesn't indicates this
	if(!bPathFinished) {
		// have we finished a current search?
		if(((searchpath_base*)pathSearcher)->isReady()) {

			bPathFinished = true;
			NEW_ai_node_t* res = ((searchpath_base*)pathSearcher)->resultedPath();

			// have we found something?
			if(res) {
				// in every case, the nodes of the current path are not handled by pathSearcher
				// so we have to delete it here
				delete_ai_nodes(NEW_psPath);

				NEW_psPath = res;
				NEW_psLastNode = get_last_ai_node(NEW_psPath);
				NEW_psCurrentNode = NEW_psPath;

				// prevent it from deleting the current path (it will be deleted, when the new path is found)
				((searchpath_base*)pathSearcher)->removePathFromList(NEW_psPath);

#ifdef _AI_DEBUG
				pcMap->ClearDebugImage();
				NEW_AI_DrawPath();
#endif

				fLastCreated = tLX->fCurTime;

				return true;
			}
			// we don't find anything, so don't return here, but start a new search

		} else { // the searcher is still searching ...

			// restart search in some cases
			if(!((searchpath_base*)pathSearcher)->shouldRestartThread() && (tLX->fCurTime - fSearchStartTime >= 5.0f || !traceWormLine(vPos, ((searchpath_base*)pathSearcher)->start, pcMap) || !traceWormLine(trg, ((searchpath_base*)pathSearcher)->target, pcMap))) {
				fSearchStartTime = tLX->fCurTime;
				((searchpath_base*)pathSearcher)->restartThreadSearch(vPos, trg);
			}

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
	// the searcher-thread is currently ready
	bPathFinished = false;

	// start a new search
	fSearchStartTime = tLX->fCurTime;
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


CVec CWorm::NEW_AI_FindBestFreeSpot(CVec vPoint, CVec vStart, CVec vDirection, CVec vTarget, CVec* vEndPoint) {

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
			//PutPixel(bmpDest,(int)possible_end.x*2,(int)possible_end.y*2,tLX->clPink);
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
CVec CWorm::NEW_AI_FindClosestFreeSpotDir(CVec vPoint, CVec vDirection, int Direction = -1)
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
void CWorm::NEW_AI_DrawPath()
{
	if (!NEW_psPath)
		return;

	SDL_Surface *bmpDest = pcMap->GetDebugImage();
	if (!bmpDest)
		return;

	const int NodeColour = MakeColour(255,0,0);
	const int LineColour = tLX->clWhite;

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

		if(best.x < 0 || (CVec((float)x,(float)y)-target).GetLength2() < (best-target).GetLength2())
			best = CVec((float)x,(float)y);

		return false;
	}
};

/////////////////////////
// Finds the best spot to shoot rope to if we want to get to trg
CVec CWorm::NEW_AI_GetBestRopeSpot(CVec trg)
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
CVec CWorm::NEW_AI_GetNearestRopeSpot(CVec trg)
{
	CVec dir = trg-vPos;
	NormalizeVector(&dir);
	dir = dir*10;
	float restlen2 = cNinjaRope.getRestLength();
	restlen2 *= restlen2;
	while ((vPos-trg).GetLength2() >= restlen2)
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

	// TODO: Note: unoptimized

	int i=1;
	int x=0,y=0;
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
bool CWorm::NEW_AI_IsInAir(CVec pos, int area_a)
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
		if(tmp_pf & (PX_ROCK|PX_DIRT))
			return false;

		x++;
	}

	return true;

}


/////////////////////
// Move to the target
void CWorm::NEW_AI_MoveToTarget()
{
//	printf("Moving to target");

    worm_state_t *ws = &tState;

/*
	// if we are walking through a tunnel and we are passing some other
	// target than our current one, it will stop walking with this,
	// because the passed target is "nearer"; this is absolutly not wanted here
	
	// Better target?
	CWorm *newtrg = findTarget(iAiGame, iAiTeams, iAiTag, pcMap);
	if (psAITarget && newtrg)
		if (newtrg->getID() != psAITarget->getID())
			nAIState = AI_THINK;

	if (!psAITarget && newtrg)  {
		nAIState = AI_THINK;
	}
*/

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

    bool fireNinja = false;

	/*
		Prevent injuries! If any of the projectiles around is heading to us, try to get away from it
	*/
	if (psHeadingProjectile)  {
		// TODO: improve this

		// Go away from the projectile
		if (tLX->fCurTime-fLastFace >= 0.5f)  {
			if (psHeadingProjectile->GetVelocity().x > 0)
				iDirection = DIR_LEFT;  // Move in the opposite direction
			else
				iDirection = DIR_RIGHT;
			fLastFace = tLX->fCurTime;
		}
		ws->iMove = true;


		// If we're on ground, jump
		if(CheckOnGround())  {
			if (tLX->fCurTime - fLastJump > 1.0f)  {
				ws->iJump = true;
				fLastJump = tLX->fCurTime;
			}
		}

		// Release any previous rope
		if (fRopeAttachedTime >= 1.5f)
			cNinjaRope.Release();

		// Shoot the rope
		fireNinja = true;

		// We want to move away
		CVec desired_dir = -psHeadingProjectile->GetVelocity();

		// Choose some point and find the best rope spot to it
		desired_dir = desired_dir.Normalize() * 40.0f;
		CVec cAimPos = NEW_AI_GetBestRopeSpot(vPos+desired_dir);

		// Aim it
		/*
		// TODO: why isn't this used any more?
		fireNinja = AI_SetAim(cAimPos);

		if (fireNinja)
			fireNinja = psHeadingProjectile->GetVelocity().GetLength() > 50.0f;

		if (fireNinja)  {

			// Get the direction
			CVec dir;
			dir.x=( (float)cos(fAngle * (PI/180)) );
			dir.y=( (float)sin(fAngle * (PI/180)) );
			if(iDirection == DIR_LEFT)
				dir.x=(-dir.x);

			// Shoot it
			cNinjaRope.Shoot(vPos,dir);
		}*/

		return;
	}

	// TODO: in general, move away from projectiles

	// prevent suicides
	if (iAiGameType == GAM_MORTARS)  {
		if (tLX->fCurTime - fLastShoot <= 0.5f)  {
			if (fRopeAttachedTime >= 0.1f)
				cNinjaRope.Release();
			return;
		}
	}


    /*
      Move through the path.
      We have a current node that we must get to. If we go onto the node, we go to the next node, and so on.
    */

	//return; // Uncomment this when you don't want the AI to move

    if(NEW_psPath == NULL || NEW_psCurrentNode == NULL) {
        // If we don't have a path, resort to simpler AI methods
        AI_SimpleMove(psAITarget != NULL);
		//printf("Pathfinding problem 2; ");
        return;
    }

//	printf("We should move now...");

	// If some of the next nodes is closer than the current one, just skip to it
	NEW_ai_node_t *next_node = NEW_psCurrentNode->psNext;
	bool newnode = false;
	while(next_node)  {
		if(traceWormLine(CVec(next_node->fX,next_node->fY),vPos,pcMap))  {
			NEW_psCurrentNode = next_node;
			newnode = true;
		}
		next_node = next_node->psNext;
	}
	if(!newnode) {
		// check, if we have a direct connection to the current node
		// else, choose some last node
		// this will work and is in many cases the last chance
		// perhaps, we need a fLastGoBack here
		for(next_node = NEW_psCurrentNode; next_node; next_node = next_node->psPrev) {
			if(traceWormLine(CVec(next_node->fX,next_node->fY),vPos,pcMap))  {
				if(NEW_psCurrentNode != next_node) {
					NEW_psCurrentNode = next_node;
 					newnode = true;
 				}
				break;
			}
		}
	}


	// Get the target node position
    CVec nodePos = CVec(NEW_psCurrentNode->fX,NEW_psCurrentNode->fY);


	// release rope, if it forces us to the wrong direction
	if(cNinjaRope.isAttached() && (cNinjaRope.GetForce(vPos).Normalize() + vPos - nodePos).GetLength2() > (vPos - nodePos).GetLength2())
		cNinjaRope.Release();


#ifdef _AI_DEBUG
	NEW_AI_DrawPath();
#endif

	float dist; int type;
	traceWeaponLine(cPosTarget,&dist,&type);
	bool we_see_the_target = (type & PX_EMPTY);

	/*
	  For rifle games: it's not clever when we go to the battle with non-reloaded weapons
	  If we're close to the target (<= 3 nodes missing), stop and reload weapons if needed

	  This is an advanced check, so simply ignore it if we are "noobs"
	*/
	if (iAiGameType == GAM_RIFLES && iAiDiffLevel >=2)  {
		int num_reloaded=0;
		int i;
		for (i=0;i<5;i++) {
			if (!tWeapons[i].Reloading)
				num_reloaded++;
		}

		if (num_reloaded <= 3)  {

			// If we see the target, fight instead of reloading!
			if (!we_see_the_target)  {
				NEW_ai_node_t *node = NEW_psLastNode;
				for(i=0;node && node != NEW_psCurrentNode;node=node->psPrev,i++) {}
				if (NEW_psLastNode == NULL || i>=3)  {
					// Reload weapons when we're far away from the target or if we don't have any path
					AI_ReloadWeapons();
				}
				if(NEW_psLastNode && (i>=3 && i<=5)) {
					// Stop, if we are not so far away
					if (fRopeAttachedTime >= 0.7f)
						cNinjaRope.Release();
					return;
				}
			}
		}
	}


    /*
      Now that we've done all the boring stuff, our single job here is to reach the node.
      We have walking, jumping, move-through-air, and a ninja rope to help us.
    */

    /*
      If there is dirt between us and the next node, don't shoot a ninja rope
      Instead, carve
    */
    float traceDist = -1;
    CVec v = CVec(NEW_psCurrentNode->fX, NEW_psCurrentNode->fY);
    int length = traceLine(v, &traceDist, &type); // HINT: this is only a line, not the whole worm
														 // NOTE: this can return dirt, even if there's also rock between us two


    //float dist = CalculateDistance(v, vPos);
	// HINT: atm, fireNinja is always false here
    if(!fireNinja && (float)(length*length) <= (v-vPos).GetLength2() && (type & PX_DIRT)) {
		// HINT: as we always carve, we don't need to do it here specially

		// release rope, if it is atached and above
		if(fRopeAttachedTime > 0.5f && cNinjaRope.getHookPos().y - 5.0f > v.y)
			cNinjaRope.Release();

		// Jump, if the node is above us
		if (v.y+10.0f < vPos.y)
			if (tLX->fCurTime - fLastJump > 0.5f)  {
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
					ws->iShoot = true; // TODO: is it ensured here, that we are aiming already correctly?
					// Don't do any crazy things when shooting
					ws->iMove = false;
					ws->iJump = false;
				} /* else
					AI_SimpleMove(pcMap,psAITarget != NULL); */ // no weapon found, so move around
			}
    } else  {
		// If there's no dirt around and we have jetpack in our arsenal, lets use it!
		for (short i=0;i<5;i++) {
			if (tWeapons[i].Weapon->Recoil < 0 && !tWeapons[i].Reloading)  {
				iCurrentWeapon = i;
				ws->iShoot = AI_SetAim(nodePos);
			}
		}

    	fireNinja = true;
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
			if(CheckOnGround()) {
				if (tLX->fCurTime - fLastJump > 1.0f)  {
					ws->iJump = true;
					fLastJump = tLX->fCurTime;
					// Rope will happen soon
				}
			}
		}
	}

	// If we're above the node and the rope is hooked wrong, release the rope
	// TODO: is it better to check only, if we are above it?
	if((vPos.y < nodePos.y) /* && fRopeAttachedTime > 2.0f && cNinjaRope.getHookPos().y-vPos.y < 0.0f */) {
		cNinjaRope.Release();
		fireNinja = false;
	} else if(!CheckOnGround()) // Not on ground? Shoot the rope
		fireNinja = true;

	bool aim = false;

	CVec ropespot;
	if(fireNinja)
		ropespot = NEW_AI_GetBestRopeSpot(nodePos);

	// It has no sense to shoot the rope on short distances
	if(fireNinja && (vPos-ropespot).GetLength2() < 625.0f)
		fireNinja = false;

	// In rifle games: don't continue if we see the final target and are quite close to it
	// If we shot the rope, we wouldnt aim the target, which is the priority now
	if(fireNinja && iAiGameType == GAM_RIFLES && we_see_the_target && (vPos-cPosTarget).GetLength2() <= 3600.0f) {
		fireNinja = false;
	}

    CVec dir;
    if(fireNinja) {
    	// set it to false, only if we pass the following checks, set it to true again
		fireNinja = false;

		// Aim
		aim = AI_SetAim(ropespot);


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
        if(aim || !CheckOnGround()) {
            if(!cNinjaRope.isReleased())
            	fireNinja = true;
            else {
                if(cNinjaRope.isAttached()) {
                    if((vPos-cNinjaRope.getHookPos()).GetLength2() < cNinjaRope.getRestLength()*cNinjaRope.getRestLength() && vVelocity.y<-10)
                    	fireNinja = true;
                }
            }
            ws->iJump = true;
			fRopeHookFallingTime = 0;
			fRopeAttachedTime = 0;
        }
    }

    if(fireNinja) {
    	// the final shoot of the rope...
    	cNinjaRope.Shoot(vPos,dir);

    } else { // not fireNinja

		// Aim at the node
		if(!we_see_the_target) aim = AI_SetAim(nodePos);

		// If the node is above us by a little, jump
		if((vPos.y-NEW_psCurrentNode->fY) <= 30 && (vPos.y-NEW_psCurrentNode->fY) > 0) {
			// Don't jump so often
			if (tLX->fCurTime - fLastJump > 0.5f)  {
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

			if (tLX->fCurTime-fLastFace >= 0.5f)  {
				iDirection = !iDirection;
				fLastFace = tLX->fCurTime;
			}

            fAngle -= cGameScript->getWorm()->AngleSpeed * tLX->fDeltaTime;
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
            // TODO: should we do this in a more general way somewhere other?
            NEW_AI_CreatePath();

            if(!NEW_psCurrentNode || !NEW_psPath)
            	return;
        }
    }
    else {
        bStuck = false;
        fStuckTime = 0;
        cStuckPos = vPos;
    }

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

}

void drawpoint(SDL_Surface *debug_surf, CVec point)
{
	DrawRectFill(debug_surf, (int)point.x * 2, (int)point.y * 2, (int)point.x * 2 + 4, (int)point.y * 2 + 4, MakeColour(0, 255, 0));
}

///////////////////////
// Returns coordinates of a point that is best for shooting the target
CVec CWorm::NEW_AI_FindShootingSpot()
{
	if (psAITarget == NULL)
		return CVec(0,0);

	// If the worm is not on ground, we cannot hit him with a napalm-like weapon (napalm, blaster etc.)
	bool have_straight = false;
	bool have_falling = false;
	bool have_flying = false;

	// Check what weapons we have
	for (int i=0; i < 5; i++)  {
		if (tWeapons[i].Weapon->Projectile)  {
			// Get the gravity
			int gravity = 100;  // Default
			if (tWeapons[i].Weapon->Projectile->UseCustomGravity)
				gravity = tWeapons[i].Weapon->Projectile->Gravity;

			// Change the flags according to the gravity
			if (gravity >= 5)
				have_falling = true;
			else if (gravity >= -5)
				have_straight = true;
			else
				have_flying = true;
		}
	}

	float upper_bound = 0;
	float lower_bound = 0;

	// Falling projectiles, we have to get above a bit
	if (have_falling || (have_straight && psAITarget->CheckOnGround()))  {
		lower_bound = (float)PI/2;
		upper_bound = 3 * (float)PI/2;

	// Flying straight
	} else if (have_straight)  {
		lower_bound = 0;
		upper_bound = 2 * (float)PI;

	// Flying up
	} else {
		lower_bound = (float)-PI/2;
		upper_bound = (float)PI/2;
	}


	// Check a best-distance upper half circle around the target
	CVec possible_pos;
	for (float j=lower_bound; j <= upper_bound; j += 0.3f)  {
		possible_pos.x = (float) (40.0f * sin(j)) + psAITarget->getPos().x;
		possible_pos.y = (float) (40.0f * cos(j)) + psAITarget->getPos().y;
		//PutPixel(pcMap->GetDebugImage(), possible_pos.x * 2, possible_pos.y * 2, MakeColour(255, 0, 0));

		if (NEW_AI_IsInAir(possible_pos, 1) && traceLine(possible_pos, psAITarget->getPos(), NULL) >= 40)  {
			//drawpoint(pcMap->GetDebugImage(), possible_pos);
			return possible_pos;
		}
	}

	// Not found
	return psAITarget->getPos();
}

NEW_ai_node_t* get_last_ai_node(NEW_ai_node_t* n) {
	if(!n) return NULL;
	for(;n->psNext;n=n->psNext) {}
	return n;
}

void delete_ai_nodes(NEW_ai_node_t* start) {
	if(!start) return;
	delete_ai_nodes(start->psNext);
	delete start;
}

void delete_ai_nodes(NEW_ai_node_t* start, NEW_ai_node_t* end) {
	if(!start || start == end) return;
	delete_ai_nodes(start->psNext, end);
	delete start;
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
