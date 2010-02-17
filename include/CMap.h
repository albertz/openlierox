/////////////////////////////////////////
//
//                  OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Map class
// Created 21/1/02
// Jason Boettcher


#ifndef __CMAP_H__
#define __CMAP_H__

#include <SDL.h>
#include <string>
#include <set>
#include "ReadWriteLock.h"
#include "SmartPointer.h"
#include "LieroX.h" // for maprandom_t
#include "GfxPrimitives.h" // for Rectangle<>
#include "CClient.h" // only for cClient->getMap() for fastTraceLine()
#include "Color.h"
#include "MathLib.h" // for SIGN
#include "gusanos/level.h"
#include "level/LXMapFlags.h"

class CViewport;
class CCache;


class CWorm;



// Theme structure
struct theme_t {
	std::string	name;
	Color		iDefaultColour;
	SmartPointer<SDL_Surface> bmpFronttile;
	SmartPointer<SDL_Surface> bmpBacktile;

	int			NumStones;
	SmartPointer<SDL_Surface> bmpStones[16];
	SmartPointer<SDL_Surface> bmpHoles[16];
	int			NumMisc;
	SmartPointer<SDL_Surface> bmpMisc[32];

};


class CPlayer;
class MapLoad;
class ML_OrigLiero;
class ML_LieroX;
class ML_CommanderKeen123;
struct ML_Gusanos;
struct VermesLevelLoader;

class CMap {
	friend class MapLoad;
	friend class ML_OrigLiero;
	friend class ML_LieroX;
	friend class ML_CommanderKeen123;
	friend struct ML_Gusanos;
	friend struct VermesLevelLoader;
	
private:
	// just don't do that
	CMap(const CMap&) { assert(false); }
	CMap& operator=(const CMap&) { assert(false); return *this; }
	
public:
	// Constructor
	CMap() {
		Width = 800;
		Height = 700;
		MinimapWidth = 128;
		MinimapHeight = 96;
		Type = MPT_PIXMAP;
        nTotalDirtCount = 0;
		fBlinkTime = TimeDiff();

		Created = false;
		FileName = "";

		bmpImage = NULL;
#ifdef _AI_DEBUG
		bmpDebugImage = NULL;
#endif
		bmpBackImage = NULL;
		bmpBackImageHiRes = NULL;
		bmpMiniMap = NULL;
        bmpGreenMask = NULL;
        bmpShadowMap = NULL;
		
		NumObjects = 0;
		Objects = NULL;

		bMiniMapDirty = true;

		
		AdditionalData.clear();
		
		bMapSavingToMemory = false;
		bmpSavedImage = NULL;
		savedPixelFlags = NULL;
		savedMapCoords.clear();
		
		gusInit();
   	}

	~CMap() {
		Shutdown();
	}

private:
	// Attributes

	std::string	Name;
	std::string FileName;
	int			Type;
	uint		Width;
	uint		Height;
	uint		MinimapWidth;
	uint		MinimapHeight;
	theme_t		Theme;
    uint         nTotalDirtCount;

	bool		Created;
	
	SmartPointer<SDL_Surface> bmpImage;
	SmartPointer<SDL_Surface> bmpDrawImage;
	SmartPointer<SDL_Surface> bmpBackImage;
	SmartPointer<SDL_Surface> bmpBackImageHiRes;
	SmartPointer<SDL_Surface> bmpMiniMap;
	SmartPointer<SDL_Surface> bmpMiniMapTransparent; // Half-transparent minimap for Gusanos
    SmartPointer<SDL_Surface> bmpGreenMask;
    SmartPointer<SDL_Surface> bmpShadowMap;
#ifdef _AI_DEBUG
	SmartPointer<SDL_Surface> bmpDebugImage;
#endif

	// Minimap
	TimeDiff	fBlinkTime;

    //maprandom_t sRandomLayout;

	bool		bMiniMapDirty;

	ReadWriteLock	flagsLock;

	// Objects
	int			NumObjects;
	object_t	*Objects;

	std::map< std::string, std::string > AdditionalData; // Not used currently, maybe will contain CTF info in Beta10

	// Save/restore from memory, for commit/rollback net mechanism
	bool		bMapSavingToMemory;
	SmartPointer<SDL_Surface> bmpSavedImage;
	uchar *		savedPixelFlags;
	enum { MAP_SAVE_CHUNK = 16 };
	struct SavedMapCoord_t {
		int X, Y;
		SavedMapCoord_t(int x = 0, int y = 0) : X(x), Y(y) {}
		bool operator < (const SavedMapCoord_t & m) const
		{
			if( Y < m.Y ) return true;
			else if( Y > m.Y ) return false;
			else if( X < m.X ) return true;
			else return false;
		}
	};
	std::set< SavedMapCoord_t > savedMapCoords;

private:
	// Update functions
	void		UpdateMiniMap(bool force = false);
	void		UpdateMiniMapRect(int x, int y, int w, int h);
	void		UpdateArea(int x, int y, int w, int h, bool update_image = false);
	void		UpdateMiniMapTransparent();

	friend class CCache;

	bool		NewFrom(CMap *map);
	void		SaveToCache();
	bool		LoadFromCache(const std::string& filename);

public:
	// Methods
	bool		MiniCreate(uint _width, uint _height, uint _minimap_w = 128, uint _minimap_h = 96);
	bool		Create(uint _width, uint _height, const std::string& _theme, uint _minimap_w = 128, uint _minimap_h = 96);
	bool		MiniNew(uint _width, uint _height, uint _minimap_w = 128, uint _minimap_h = 96);
	bool		New(uint _width, uint _height, const std::string& _theme, uint _minimap_w = 128, uint _minimap_h = 96);
	bool		Load(const std::string& filename);
	bool		Save(const std::string& name, const std::string& filename);
	bool		SaveImageFormat(FILE *fp);

	void		Clear();
	bool		isLoaded()	{ return material && (bmpImage.get() || gusIsLoaded()); }
	
	std::string getName()			{ return Name; }
	std::string getFilename()		{ return FileName; }
	static std::string GetLevelName(const std::string& filename, bool abs_filename = false);

	void		UpdateDrawImage(int x, int y, int w, int h);

	void		Shutdown();

	bool		LoadTheme(const std::string& _theme);
	bool		CreateSurface();

	void		TileMap();
    
    void        CalculateDirtCount();
    void        CalculateShadowMap();

	inline void	lockFlags(bool writeAccess = true) {
		if(writeAccess)
			flagsLock.startWriteAccess();
		else
			flagsLock.startReadAccess();
	}
	inline void unlockFlags(bool writeAccess = true) {
		if(writeAccess)
			flagsLock.endWriteAccess();
		else
			flagsLock.endReadAccess(); 
	}

    static std::string findRandomTheme();
    static bool validateTheme(const std::string& name);

    void        PutImagePixel(uint x, uint y, Color colour);

	void		Draw(SDL_Surface *bmpDest, CViewport *view);
	void		Draw(SDL_Surface *bmpDest, const SDL_Rect& rect, int worldX, int worldY);	// For CMapEditor
	
	void        DrawObjectShadow(SDL_Surface * bmpDest, SDL_Surface * bmpObj, SDL_Surface * bmpObjShadow, int sx, int sy, int w, int h, CViewport *view, int wx, int wy);
	void        DrawPixelShadow(SDL_Surface * bmpDest, CViewport *view, int wx, int wy);
	void		DrawMiniMap(SDL_Surface * bmpDest, uint x, uint y, TimeDiff dt, CWorm *worms);
	void		drawOnMiniMap(SDL_Surface* bmpDest, uint miniX, uint miniY, const CVec& pos, Uint8 r, Uint8 g, Uint8 b, bool big, bool special);
	
private:
	// not thread-safe, therefore private	
	inline void	unsafeSetPixelFlag(long x, long y, uchar flag) {
		material->line[y][x] = (char) Material::indexFromLxFlag(flag);
	}
	
	inline void	SetPixelFlag(long x, long y, uchar flag, bool wrapAround = false) {
		if(!wrapAround) {
			if(x < 0 || y < 0 || (size_t)x >= Width || (size_t)y >= Height) return;
		}
		else {
			x = WrapAroundX(x);
			y = WrapAroundY(y);			
		}
		unsafeSetPixelFlag(x, y, flag);
	}
	
	// Saves region of map to savebuffer for RestoreFromMemory() - called from CarveHole()/PlaceDirt()/PlaceGreenDirt()
	void SaveToMemoryInternal(int x, int y, int w, int h);


public:	

	size_t GetMemorySize();

	int WrapAroundX(int x) const {
		x %= (int)Width;
		if(x < 0) x += Width;
		return x;
	}

	int WrapAroundY(int y) const {
		y %= (int)Height;
		if(y < 0) y += Height;
		return y;
	}

	VectorD2<int> WrapAround(VectorD2<int> p) {
		return VectorD2<int>(WrapAroundX(p.x), WrapAroundY(p.y));
	}

	uchar unsafeGetPixelFlag(long x, long y, bool wrapAround = false) const {
		return unsafeGetMaterial(x, y).toLxFlags();
	}

	uchar GetPixelFlag(long x, long y, bool wrapAround = false) const {
		if(!wrapAround) {
			// Checking edges
			if(x < 0 || y < 0 || (unsigned long)x >= Width || (unsigned long)y >= Height)
				return PX_ROCK;
		}
		else {
			x = WrapAroundX(x);
			y = WrapAroundY(y);
		}
		return unsafeGetMaterial(x, y).toLxFlags();
	}
	uchar GetPixelFlag(const CVec& pos) const { return GetPixelFlag((long)pos.x, (long)pos.y); }

	bool CheckAreaFree(int x, int y, int w, int h);
	
	Color	getColorAt(long x, long y);
	void	putColorTo(long x, long y, Color c);
	void	putSurfaceTo(long x, long y, SDL_Surface* surf, int sx, int sy, int sw, int sh);
	
	SmartPointer<SDL_Surface> GetDrawImage()		{ return bmpDrawImage; }
	SmartPointer<SDL_Surface> GetImage()			{ return bmpImage; }
	SmartPointer<SDL_Surface> GetBackImage()		{ return bmpBackImage; }
	SmartPointer<SDL_Surface> GetMiniMap()		{ return bmpMiniMap; }
#ifdef _AI_DEBUG
	// TODO: the debug image is also usefull for other debugging things, not for AI
	// so make it also available if DEBUG is defined
	SmartPointer<SDL_Surface> GetDebugImage()	{ return bmpDebugImage; }

	void		ClearDebugImage();
#endif

	int 		CarveHole(int size, CVec pos, bool wrapAround);
	int 		PlaceDirt(int size, CVec pos);
	void		PlaceStone(int size, CVec pos);
	void		PlaceMisc(int id, CVec pos);
    int         PlaceGreenDirt(CVec pos);
	void		ApplyShadow(int sx, int sy, int w, int h);

	struct CollisionInfo  {
		CollisionInfo() : occured(false), left(false), top(false), right(false), bottom(false), hitBounds(false), hitRockDirt(false), x(0), y(0) {}
		bool occured;
		bool left, top, right, bottom;
		bool hitBounds;
		bool hitRockDirt;
		int x, y;  // HINT: if inifinite map is set, these coordinates are real world coordinates, i.e. modulo is applied on them
		int moveToX, moveToY;
	};
	CollisionInfo StaticCollisionCheck(const CVec& objpos, int objw, int objh, bool infiniteMap) const;

private:
	void StaticCollisionCheckInfinite(const CVec& objpos, int objw, int objh, CollisionInfo& result) const;
	void StaticCollisionCheckFinite(const CVec& objpos, int objw, int objh, CollisionInfo& result) const;

public:

	CVec		groundPos(const CVec& pos);
	
	// Save/restore from memory, for commit/rollback net mechanism
	void		NewNet_SaveToMemory();
	void		NewNet_RestoreFromMemory();
	void		NewNet_Deinit();

	theme_t		*GetTheme()		{ return &Theme; }

	void		DEBUG_DrawPixelFlags(int x, int y, int w, int h);

	uint			GetWidth() const	{ return Width; }
	uint			GetHeight()	const	{ return Height; }
	uint			GetMinimapWidth() const { return MinimapWidth; }
	uint			GetMinimapHeight() const { return MinimapHeight; }
	void				SetMinimapDimensions(uint _w, uint _h);
    uint         GetDirtCount() const { return nTotalDirtCount; }

	bool			getCreated()	{ return Created; }
	
	
	// TODO: this needs to be made much more general to be as fast as the current routines
	
	// _F has to be a functor with provides compatible functions to:
	//   bool operator()(int x, int y, int adr_offset); // handle one point; if returns false, break
	template<typename _T, typename _F>
	void walkPixels(OLXRect<_T> r, _F walker) {
		if(!r.clipWith(SDLRect(0, 0, Width, Height)))
			return;
		
		for(int y = r.y(); y < r.y2(); y++)
			for(int x = r.x(); x < r.x2(); x++)
				if(!walker(x, y)) return;
	}
	
	
	
	CVec FindSpot();
	CVec FindSpotCloseToPos(const std::list<CVec>& goodPos, const std::list<CVec>& badPos, bool keepDistanceToBad);	
	CVec FindSpotCloseToPos(const CVec& goodPos) {
		std::list<CVec> good; good.push_back(goodPos);
		std::list<CVec> bad;
		return FindSpotCloseToPos(good, bad, true);
	}
	CVec FindSpotCloseToTeam(int t, CWorm* exceptionWorm = NULL, bool keepDistanceToEnemy = true);
	CVec FindNearestSpot(CGameObject *o) { return FindSpotCloseToPos(o->pos()); }
	
	
	struct __PixelFlagAccessBase {
		CMap* map;
		__PixelFlagAccessBase(CMap* m) : map(m) {} 
	};
	
	struct __PixelFlagReaders : __PixelFlagAccessBase {
		__PixelFlagReaders(CMap* m) : __PixelFlagAccessBase(m) {}

		uchar get(long x, long y, bool wrapAround = false) { return map->GetPixelFlag(x,y,wrapAround); }
		
		typedef uchar (*CombiFunc) (uchar, uchar);
		static uchar Or(uchar a, uchar b) { return a | b; }
		static uchar And(uchar a, uchar b) { return a & b; }
		
		template<CombiFunc func>
		uchar getLineHoriz(long x, long y, long x2, bool wrapAround = false) {
			uchar ret = 0;
			for(; x < x2; ++x)
				ret = (*func)(ret, get(x,y,wrapAround));
			return ret;
		}
		
		uchar getLineHoriz_Or(long x, long y, long x2, bool wrapAround = false) { return getLineHoriz<Or>(x,y,x2,wrapAround); }
		
		template<CombiFunc func>
		uchar getArea(long x, long y, long x2, long y2, bool wrapAround = false) {
			uchar ret = 0;
			for(; y < y2; ++y)
				ret = (*func)(ret, getLineHoriz<func>(x,y,x2,wrapAround));
			return ret;
		}
		
		uchar getArea_Or(long x, long y, long w, long h, bool wrapAround = false) { return getArea<Or>(x,y,w,h,wrapAround); }
		
		typedef bool (*CheckFunc) (uchar);
		template<uchar flags> static bool Have(uchar a) { return (bool)(a & flags); }
		template<uchar flags> static bool HaveNot(uchar a) { return !(bool)(a & flags); }
		
		template<CheckFunc func>
		bool checkLineHoriz_All(long x, long y, long x2, bool wrapAround = false) {
			for(; x < x2; ++x)
				if(!(*func)(get(x,y,wrapAround))) return false;
			return true;
		}
		
		template<CheckFunc func>
		bool checkArea_All(long x, long y, long x2, long y2, bool wrapAround = false) {
			for(; y < y2; ++y)
				if(!checkLineHoriz_All<func>(x,y,x2,wrapAround)) return false;
			return true;
		}
		
		template<uchar flags>
		bool checkArea_AllHaveNot(long x, long y, long x2, long y2, bool wrapAround = false) {
			return checkArea_All< HaveNot<flags> >(x,y,x2,y2,wrapAround);
		}
	};
	
	struct __PixelFlagWriters : __PixelFlagReaders {
		__PixelFlagWriters(CMap* m) : __PixelFlagReaders(m) {}
		
		void set(long x, long y, uchar flag) { map->SetPixelFlag((uint)x, (uint)y, flag); }
	};
	
	struct PixelFlagAccess : __PixelFlagReaders {
		PixelFlagAccess(CMap* m) : __PixelFlagReaders(m) { map->lockFlags(false); }
		~PixelFlagAccess() { map->unlockFlags(false); }
	};

	struct PixelFlagWriteAccess : __PixelFlagWriters {
		PixelFlagWriteAccess(CMap* m) : __PixelFlagWriters(m) { map->lockFlags(true); }
		~PixelFlagWriteAccess() { map->unlockFlags(true); }
	};
	
	static bool IsGoodSpawnPoint(PixelFlagAccess& flags, long x, long y) { return flags.checkArea_AllHaveNot<PX_ROCK>(x-3, y-3, x+3, y+3); }
	static bool IsGoodSpawnPoint(PixelFlagAccess& flags, CVec pos) { return IsGoodSpawnPoint(flags, (long)pos.x, (long)pos.y); }
	static bool IsEmptyForWorm(PixelFlagAccess& flags, long x, long y) { return flags.checkArea_AllHaveNot<PX_ROCK|PX_DIRT>(x-3, y-3, x+3, y+3); }
	static bool IsEmptyForWorm(PixelFlagAccess& flags, CVec pos) { return IsEmptyForWorm(flags, (long)pos.x, (long)pos.y); }
	bool IsEmptyForWorm(CVec pos) { PixelFlagAccess flags(this); return IsEmptyForWorm(flags, pos); }

	
	
	// ------------------------------------------------------------------------------
	// ------------------------------------------------------------------------------
	// ---------------------------      Gusanos       -------------------------------
	// ------------------------------------------------------------------------------
	// ------------------------------------------------------------------------------

private:
	void gusInit();
	void gusShutdown();
	
public:
	void gusThink();
	bool gusIsLoaded() { return m_gusLoaded; }
#ifndef DEDICATED_ONLY
	void gusDraw(ALLEGRO_BITMAP* where, int x, int y);
#endif
	void gusUpdateMinimap(int x, int y, int w, int h);
	
	bool getPredefinedSpawnLocation(CWorm* worm, CVec* v);
	Vec getSpawnLocation(CWormInputHandler* player);
	
	Material const& getMaterial(unsigned int x, unsigned int y) const
	{
		if(x < static_cast<unsigned int>(material->w) && y < static_cast<unsigned int>(material->h))
			return m_materialList[(unsigned char)material->line[y][x]];
		else
			return m_materialList[0];
	}
	
	Material const& unsafeGetMaterial(unsigned int x, unsigned int y) const
	{
		return m_materialList[(unsigned char)material->line[y][x]];
	}
	
	unsigned char getMaterialIndex(unsigned int x, unsigned int y) const
	{
		if(x < static_cast<unsigned int>(material->w) && y < static_cast<unsigned int>(material->h))
			return (unsigned char)material->line[y][x];
		else
			return 0;
	}
	
	void putMaterial( unsigned char index, unsigned int x, unsigned int y )
	{
		if(x < static_cast<unsigned int>(material->w) && y < static_cast<unsigned int>(material->h))
			material->line[y][x] = index;
	}
	
	void putMaterial( Material const& mat, unsigned int x, unsigned int y )
	{
		if(x < static_cast<unsigned int>(material->w) && y < static_cast<unsigned int>(material->h))
			material->line[y][x] = mat.index;
	}
	
	bool isInside(unsigned int x, unsigned int y) const
	{
		if(x < static_cast<unsigned int>(material->w) && y < static_cast<unsigned int>(material->h))
			return true;
		else
			return false;
	}
	
	template<class PredT>
	bool trace(long srcx, long srcy, long destx, long desty, PredT predicate);
	
#ifndef DEDICATED_ONLY
	void specialDrawSprite(Sprite* sprite, ALLEGRO_BITMAP* where, const IVec& pos, const IVec& matPos, BlitterContext const& blitter );
	
	void culledDrawSprite(Sprite* sprite, CViewport* viewport, const IVec& pos, int alpha);
	void culledDrawLight(Sprite* sprite, CViewport* viewport, const IVec& pos, int alpha);
	
#endif
	// applies the effect and returns true if it actually changed something on the map
	bool applyEffect( LevelEffect* effect, int x, int y);
	
	void loaderSucceeded();
	
	
#ifndef DEDICATED_ONLY
	ALLEGRO_BITMAP* image;
	ALLEGRO_BITMAP* background;
	ALLEGRO_BITMAP* paralax;
	ALLEGRO_BITMAP* lightmap; // This has to be 8 bit.
	ALLEGRO_BITMAP* watermap; // How water looks in each pixel of the map
#endif
	ALLEGRO_BITMAP* material;
	Encoding::VectorEncoding vectorEncoding;
	Encoding::VectorEncoding intVectorEncoding;
	Encoding::DiffVectorEncoding diffVectorEncoding;
	
	struct ParticleBlockPredicate
	{
		bool operator()(Material const& m)
		{
			return !m.particle_pass;
		}
	};
			
	void setEvents( LevelConfig* events )
	{
		delete m_config;
		m_config = events;
	}
	
	LevelConfig* config()
	{ return m_config; }
	
	Material& materialForIndex(uchar index) { return m_materialList[index]; }
	array<Material,256>& materialArray() { return m_materialList; }
	
private:
	
	void checkWBorders( int x, int y );
	
	array<Material, 256> m_materialList;
	
	LevelConfig *m_config;
	bool m_firstFrame;
	bool m_gusLoaded;
	
	std::list<WaterParticle> m_water;
	
};



template<class PredT>
bool CMap::trace(long x, long y, long destx, long desty, PredT predicate)
{
	if(!isInside(x, y))
	{
		if(predicate(m_materialList[0]))
			return true;
		else
		{
			return true; //TODO: Clip the beginning of the line instead of returning
		}
	}
	if(!isInside(destx, desty))
	{
		if(predicate(m_materialList[0]))
			return true;
		else
		{
			return true; //TODO: Clip the end of the line instead of returning
		}
	}
		
	long xdiff = destx - x;
	long ydiff = desty - y;
	
	long sx = SIGN(xdiff);
	long sy = SIGN(ydiff);

	xdiff = labs(xdiff);
	ydiff = labs(ydiff);
	
	#define WORK(a, b) { \
		long i = a##diff >> 1; \
		long c = a##diff; \
		while(c-- >= 0) { \
			if(predicate(unsafeGetMaterial(x, y))) return true; \
			i -= b##diff; \
			a += s##a; \
			if(i < 0) b += s##b, i += a##diff; } }
	
	if(xdiff > ydiff)
		WORK(x, y)
	else
		WORK(y, x)

	#undef WORK

	return false;
}



// TODO: make this a member function of CMap
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
	//SmartPointer<SDL_Surface> bmpDest = cClient->getMap()->GetDebugImage();
#endif
	
	CMap* map = cClient->getMap();
	if(!map->isLoaded()) return;

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

inline bool fastTraceLine_hasAnyCollision(CVec target, CVec start, uchar checkflag) {
	SimpleTracelineCheck ret;
	fastTraceLine(target, start, checkflag, ret);
	return ret.result;
}


int		CheckCollision(CVec trg, CVec pos, uchar checkflags);
int 	CarveHole(CVec pos);


#endif  //  __CMAP_H__
