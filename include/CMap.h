/////////////////////////////////////////
//
//                  LieroX
//
//     Copyright Auxiliary Software 2002
//
//
/////////////////////////////////////////


// Map class
// Created 21/1/02
// Jason Boettcher


#ifndef __CMAP_H__
#define __CMAP_H__



#define		MAP_VERSION	0

// Map types
#define		MPT_PIXMAP	0
#define		MPT_IMAGE	1

// Pixel flags
#define		PX_EMPTY	0x01
#define		PX_DIRT		0x02
#define		PX_ROCK		0x04
#define		PX_SHADOW	0x08
#define		PX_WORM		0x10


// Object types
#define		OBJ_HOLE	0
#define		OBJ_STONE	1
#define		OBJ_MISC	2

#define		MAX_OBJECTS	8192



class CWorm;



// Object structure
typedef struct {
	int		Type;
	int		Size;
	int     X, Y;

} object_t;



// Random map data
typedef struct {

    bool        bUsed;
    char        szTheme[128];
    int         nNumObjects;
    object_t    *psObjects;

} maprandom_t;



// Theme structure
typedef struct {
	char		name[128];
	Uint32		iDefaultColour;
	SDL_Surface	*bmpFronttile;
	SDL_Surface	*bmpBacktile;

	int			NumStones;
	SDL_Surface	*bmpStones[16];
	SDL_Surface	*bmpHoles[16];
	int			NumMisc;
	SDL_Surface	*bmpMisc[32];

} theme_t;


class CPlayer;

class CMap {
public:
	// Constructor
	CMap() {
		Width = 800;
		Height = 700;
		Type = MPT_PIXMAP;
        nTotalDirtCount = 0;

		Created = false;

		bmpImage = NULL;
#ifdef _AI_DEBUG
		bmpDebugImage = NULL;
#endif
		bmpBackImage = NULL;
		bmpMiniMap = NULL;
		PixelFlags = NULL;
        bmpGreenMask = NULL;
        bmpShadowMap = NULL;
        GridFlags = NULL;
		AbsoluteGridFlags = NULL;
		
		NumObjects = 0;
		Objects = NULL;

		bMiniMapDirty = true;
        sRandomLayout.bUsed = false;
        
        flags_mutex = SDL_CreateMutex();
	}

	~CMap() {
		Shutdown();
		SDL_DestroyMutex(flags_mutex);
	}

private:
	// Attributes

	char		Name[64];
	int			Type;
	int			Width;
	int			Height;
	theme_t		Theme;
    int         nTotalDirtCount;

	bool		Created;

	SDL_Surface	*bmpImage;
	SDL_Surface	*bmpDrawImage;
	SDL_Surface	*bmpBackImage;    
	SDL_Surface	*bmpMiniMap;
    SDL_Surface *bmpGreenMask;
	uchar		*PixelFlags;    
    SDL_Surface *bmpShadowMap;
#ifdef _AI_DEBUG
	SDL_Surface *bmpDebugImage;
#endif

    // AI Grid
    int         nGridWidth, nGridHeight;
    int         nGridCols, nGridRows;
	uchar		*GridFlags;
	uchar		*AbsoluteGridFlags;

    maprandom_t sRandomLayout;

	bool		bMiniMapDirty;

	SDL_mutex*	flags_mutex;

	// Objects
	int			NumObjects;
	object_t	*Objects;


	// Water
	int			*m_pnWater1;
	int			*m_pnWater2;



public:
	// Methods

	int			New(int _width, int _height, char *_theme);
	int			Load(char *filename);
	int			LoadOriginal(FILE *fp);
	int			Save(char *name, char *filename);
	int			SaveImageFormat(FILE *fp);
	int			LoadImageFormat(FILE *fp);	
	void		Clear(void);

    void		ApplyRandom(void);
    void        ApplyRandomLayout(maprandom_t *psRandom);

	void		Shutdown(void);

	int			LoadTheme(char *_theme);
	int			CreateSurface(void);
	int			CreatePixelFlags(void);
    bool        createGrid(void);
    void        calculateGrid(void);
    void        calculateGridCell(int x, int y, bool bSkipEmpty);
	void		TileMap(void);
    
    void        CalculateDirtCount();
    void        CalculateShadowMap();

	inline void	lockFlags()		{ SDL_mutexP(flags_mutex); }
	inline void unlockFlags()	{ SDL_mutexV(flags_mutex); }

    char        *findRandomTheme(char *buf);
    bool        validateTheme(char *name);

    void        PutImagePixel(int x, int y, Uint32 colour);

	void		UpdateMiniMap(int force = false);	

	void		Send(CBytestream *bs);

	void		Draw(SDL_Surface *bmpDest, CViewport *view);
    void        DrawObjectShadow(SDL_Surface *bmpDest, SDL_Surface *bmpObj, int sx, int sy, int w, int h, CViewport *view, int wx, int wy);
    void        DrawPixelShadow(SDL_Surface *bmpDest, CViewport *view, int wx, int wy);
    void		DrawMiniMap(SDL_Surface *bmpDest, int x, int y, float dt, CWorm *worms, int gametype);

private:
	// not thread-safe, therefore private	
	inline void	SetPixelFlag(int x, int y, int flag)	
	{
		// Check edges
		if(x < 0 || y < 0)
			return;
		if(x >= Width || y >= Height)
			return;
	
		PixelFlags[y * Width + x] = flag;
	}

public:	
	inline uchar GetPixelFlag(int x, int y)
	{
		// Checking edges
		if(x < 0 || y < 0)
			return PX_ROCK;
		if(x >= Width || y >= Height)
			return PX_ROCK;
	
		return PixelFlags[y * Width + x];
	}

	inline const uchar	*GetPixelFlags(void) const	{ return PixelFlags; }

	inline SDL_Surface	*GetDrawImage(void)		{ return bmpDrawImage; }
	inline SDL_Surface	*GetImage(void)			{ return bmpImage; }
	inline SDL_Surface	*GetMiniMap(void)		{ return bmpMiniMap; }
#ifdef _AI_DEBUG
	inline SDL_Surface *GetDebugImage(void)	{ return bmpDebugImage; }

	void		ClearDebugImage(void)  { if (bmpDebugImage) { DrawRectFill(bmpDebugImage,0,0,bmpDebugImage->w,bmpDebugImage->h,MakeColour(255,0,255));}}
#endif

	void		AddObject(int type, int size, CVec pos);

	int 		CarveHole(int size, CVec pos);
	int 		PlaceDirt(int size, CVec pos);
	void		PlaceStone(int size, CVec pos);
	void		PlaceMisc(int id, CVec pos);
    int         PlaceGreenDirt(CVec pos);
	void		ApplyShadow(int sx, int sy, int w, int h);

	void		DeleteObject(CVec pos);
	void		DeleteStone(object_t *obj);

	inline theme_t		*GetTheme(void)		{ return &Theme; }

	void		DEBUG_DrawPixelFlags(void);

    inline maprandom_t *getRandomLayout(void)  { return &sRandomLayout; }


	inline int			GetWidth(void)		{ return Width; }
	inline int			GetHeight(void)		{ return Height; }
    inline int         GetDirtCount(void)  { return nTotalDirtCount; }

    inline int         getGridCols(void)   { return nGridCols; }
    inline int         getGridRows(void)   { return nGridRows; }
    inline int         getGridWidth(void)  { return nGridWidth; }
    inline int         getGridHeight(void) { return nGridHeight; }
    inline const uchar *getGridFlags(void) { return GridFlags; }
	inline const uchar	*getAbsoluteGridFlags() { return AbsoluteGridFlags; }
	inline int			getCreated(void)	{ return Created; }


};





#endif  //  __CMAP_H__
