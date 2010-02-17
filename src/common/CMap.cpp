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
// Created 22/1/02
// Jason Boettcher


#include <cassert>
#include <zlib.h>
#include <list>


#include "LieroX.h"
#include "CViewport.h"
#include "CMap.h"
#include "EndianSwap.h"
#include "MathLib.h"
#include "Error.h"
#include "ConfigHandler.h"
#include "GfxPrimitives.h"
#include "PixelFunctors.h"
#include "FindFile.h"
#include "StringUtils.h"
#include "InputEvents.h"
#include "CWorm.h"
#include "Entity.h"
#include "CServer.h"
#include "ProfileSystem.h"
#include "Cache.h"
#include "Debug.h"
#include "FlagInfo.h"
#include "FileUtils.h"
#include "EndianSwap.h"
#include "MapLoader.h"
#include "game/Level.h"
#include "gusanos/gusgame.h"
#include "game/Game.h"


////////////////////
// Copies all info from the given map to this map
bool CMap::NewFrom(CMap* map)
{
	if (map == NULL || !map->Created)
		return false;

	Name = map->Name;
	FileName = map->FileName;
	Type = map->Type;
	Width = map->Width;
	Height = map->Height;
	MinimapWidth = map->MinimapWidth;
	MinimapHeight = map->MinimapHeight;
	Theme = map->Theme;
	nTotalDirtCount = map->nTotalDirtCount;
	bMiniMapDirty = map->bMiniMapDirty;
	NumObjects = map->NumObjects;
	
	bmpGreenMask = map->bmpGreenMask.get() ? GetCopiedImage(map->bmpGreenMask) : NULL;

	if(map->gusIsLoaded()) {
		if (!MiniCreate(Width, Height, MinimapWidth, MinimapHeight))
			return false;
	} else {
		// Create the map (and bmpImage and friends)
		if (!Create(Width, Height, Theme.name, MinimapWidth, MinimapHeight))
			return false;
		
		destroy_bitmap(material);
	}
	
	m_gusLoaded = map->m_gusLoaded;
	
	image = create_copy_bitmap(map->image);
	background = create_copy_bitmap(map->background);
	paralax = create_copy_bitmap(map->paralax);
	lightmap = create_copy_bitmap(map->lightmap);
	watermap = create_copy_bitmap(map->watermap);
	material = create_copy_bitmap(map->material);
	
	vectorEncoding = map->vectorEncoding;
	intVectorEncoding = map->intVectorEncoding;
	diffVectorEncoding = map->diffVectorEncoding;
	
	m_materialList = map->m_materialList;
	m_config = map->m_config ? new LevelConfig(*map->m_config) : NULL;
	m_firstFrame = true;
	
	m_water = map->m_water;	
	
	// Copy the data
	bmpImage = image ? image->surf : GetCopiedImage(map->bmpImage);
	bmpDrawImage = map->bmpDrawImage.get() ? GetCopiedImage(map->bmpDrawImage) : NULL;
	bmpBackImage = GetCopiedImage(map->bmpBackImage);
	bmpShadowMap = map->bmpShadowMap.get() ? GetCopiedImage(map->bmpShadowMap) : NULL;
#ifdef _AI_DEBUG
	bmpDebugImage = map->bmpDebugImage.get() ? GetCopiedImage(map->bmpDebugImage) : NULL;
#endif
	
	CopySurface(bmpMiniMap.get(), map->bmpMiniMap, 0, 0, 0, 0, bmpMiniMap->w, bmpMiniMap->h);

	if(Objects && map->Objects)
		memcpy(Objects, map->Objects, MAX_OBJECTS * sizeof(object_t));
	bmpBackImageHiRes = NULL;
	if( map->bmpBackImageHiRes.get() )
	{
		bmpBackImageHiRes = gfxCreateSurface(Width*2, Height*2);
		if( ! bmpBackImageHiRes.get() )
		{
			errors("CMap::NewFrom(): ERROR: cannot create bmpBackImageHiRes\n");
			return false;
		}
		DrawImage(bmpBackImageHiRes.get(), map->bmpBackImageHiRes, 0, 0);
	}
	
	AdditionalData = map->AdditionalData;
	
	bMapSavingToMemory = false;
	bmpSavedImage = NULL;
	if( savedPixelFlags )
		delete[] savedPixelFlags;
	savedPixelFlags = NULL;
	savedMapCoords.clear();
	
	Created = true;

	return true;
}


////////////////
// Save this map to cache
void CMap::SaveToCache()
{
	// no map caching for gusanos because the map can be different depending on the mod so we should always reload it
	if(!gusIsLoaded())
		cCache.SaveMap(FileName, this);
}

///////////////
// Try to load the map from cache
bool CMap::LoadFromCache(const std::string& filename)
{
	SmartPointer<CMap> cached = cCache.GetMap(filename);
	if (!cached.get())
		return false;

	return NewFrom(cached.get());
}

/////////////////////
// Returns number of bytes that the map takes in memory
size_t CMap::GetMemorySize()
{
	size_t res = sizeof(CMap) +
		GetSurfaceMemorySize(bmpBackImage.get()) + GetSurfaceMemorySize(bmpImage.get()) +
		GetSurfaceMemorySize(bmpDrawImage.get()) + GetSurfaceMemorySize(bmpGreenMask.get()) +
		GetSurfaceMemorySize(bmpShadowMap.get()) + GetSurfaceMemorySize(bmpMiniMap.get()) +
		GetSurfaceMemorySize(bmpMiniMapTransparent.get()) +
		Width * Height + // Pixel flags
		Name.size() + FileName.size() +
		Theme.name.size();
#ifdef _AI_DEBUG
	res += GetSurfaceMemorySize(bmpDebugImage.get());
#endif
	if( bmpSavedImage.get() != NULL )
		res += GetSurfaceMemorySize(bmpSavedImage.get()) + Width * Height; // Saved pixel flags
	if( bmpBackImageHiRes.get() )
		res += GetSurfaceMemorySize(bmpBackImageHiRes.get());
	return res;
}

///////////////////
// Allocate a new map
bool CMap::Create(uint _width, uint _height, const std::string& _theme, uint _minimap_w, uint _minimap_h)
{
	if(!MiniCreate(_width, _height, _minimap_w, _minimap_h))
		return false;

	// reset it again and load the rest
	Created = false;
	
	Objects = new object_t[MAX_OBJECTS];
	if(Objects == NULL)
	{
		errors << "CMap::New:: cannot create object array" << endl;
		return false;
	}

	// Load the tiles
	if(!LoadTheme(_theme))
	{
		errors("CMap::New:: ERROR: cannot create titles/theme\n");
		return false;
	}

	// Create the pixel flags
	material = create_bitmap_ex(8, Width, Height);
	if(!material)
	{
		errors("CMap::New:: ERROR: cannot create pixel flags\n");
		return false;
	}
	
	// Create the surface
	if(!CreateSurface())
	{
		errors("CMap::New:: ERROR: cannot create surface\n");
		return false;
	}

	Created = true;
	return true;
}

///////////////////
// Allocate a new map
bool CMap::MiniCreate(uint _width, uint _height, uint _minimap_w, uint _minimap_h)
{
	Width = _width;
	Height = _height;
	MinimapWidth = _minimap_w;
	MinimapHeight = _minimap_h;
	
	fBlinkTime = 0;
	
	Objects = NULL;
			
	bmpMiniMap = gfxCreateSurface(MinimapWidth, MinimapHeight);
	if(bmpMiniMap.get() == NULL) {
		SetError("CMap::MiniCreate(): bmpMiniMap creation failed, perhaps out of memory");
		return false;
	}
	
	Created = true;
	
	return true;
}


///////////////////
// Create a new map
bool CMap::New(uint _width, uint _height, const std::string& _theme, uint _minimap_w, uint _minimap_h)
{
	NumObjects = 0;
    nTotalDirtCount = 0;
    //sRandomLayout.bUsed = false;

	// Create the map
	if (!Create(_width, _height, _theme, _minimap_w, _minimap_h))
		return false;

	// Place default tiles
	TileMap();

	// TODO: does that make sense? we haven't loaded anything yet...
	
	/*
	// Update the mini map
	UpdateMiniMap();

    // Calculate the total dirt count
    CalculateDirtCount();

    // Calculate the grid
    calculateGrid();
	*/
	
	Created = true;

	return true;
}


///////////////////
// Create a new map with minimal settings (used for Gusanos)
bool CMap::MiniNew(uint _width, uint _height, uint _minimap_w, uint _minimap_h)
{
	NumObjects = 0;
    nTotalDirtCount = 0;
    //sRandomLayout.bUsed = false;
	
	// Create the map
	if (!MiniCreate(_width, _height, _minimap_w, _minimap_h))
		return false;
	
	// we need the standard theme in any case to make LX-like carving possible (in case of a LX mod)
	if(!LoadTheme("dirt")) {
		warnings << "CMap::MiniNew: cannot load theme" << endl;
		return false;
	}
	
	Created = true;
	
	return true;
}


///////////////////
// Clear the map
void CMap::Clear()
{
	TileMap();
}

///////////////////
// Load the theme
bool CMap::LoadTheme(const std::string& _theme)
{
	// Already loaded
	if (Theme.name == _theme /* && sRandomLayout.szTheme == _theme */) {
		notes << "LoadTheme: Theme " << _theme << " already loaded" << endl;
	} else {
		const std::string thmdir = "data/themes/" + _theme;
		const std::string thmfile = thmdir + "/theme.txt";
		
		Theme.name = _theme;
		//sRandomLayout.szTheme = _theme;

		LOAD_IMAGE(Theme.bmpBacktile, thmdir + "/Backtile.png");
		LOAD_IMAGE(Theme.bmpFronttile, thmdir + "/Fronttile.png");


		// Stones
		ReadInteger(thmfile, "General", "NumStones", &Theme.NumStones, 0);

		for(int n=0;n<Theme.NumStones;n++) {
			LOAD_IMAGE(Theme.bmpStones[n], thmdir + "/Stone" + itoa(n+1) + ".png");
			SetColorKey(Theme.bmpStones[n].get());
		}


		// Holes
		for(int n=0;n<5;n++) {
			LOAD_IMAGE(Theme.bmpHoles[n], thmdir + "/Hole" + itoa(n+1) + ".png");
			SetColorKey(Theme.bmpHoles[n].get(), 0, 0, 0); // use black as colorkey
		}

		// Calculate the default colour from a non-pink, non-black colour in the hole image
		LOCK_OR_FAIL(Theme.bmpFronttile);
		Theme.iDefaultColour = Color(Theme.bmpFronttile->format, GetPixel(Theme.bmpFronttile.get(),0,0));
		UnlockSurface(Theme.bmpFronttile);
		SmartPointer<SDL_Surface> hole = Theme.bmpHoles[0];
		LOCK_OR_FAIL(hole);
		if(hole.get()) {
			for(int y=0; y<hole.get()->h; y++) {
				for(int x=0; x<hole.get()->w; x++) {
					Color pixel = Color(hole.get()->format, GetPixel(hole.get(),x,y));
					if(pixel != tLX->clBlack && pixel != tLX->clPink)  {
						Theme.iDefaultColour = pixel;
						break;
					}
				}
			}
		}
		UnlockSurface(hole);


		// Misc
		ReadInteger(thmfile, "General", "NumMisc", &Theme.NumMisc, 0);
		for(int n = 0; n < Theme.NumMisc; n++) {
			LOAD_IMAGE(Theme.bmpMisc[n], thmdir + "/misc" + itoa(n+1) + ".png");
			SetColorKey(Theme.bmpMisc[n].get());
		}
	}
	
    // Load the green dirt mask
	if(bmpGreenMask.get() == NULL)
		LOAD_IMAGE(bmpGreenMask, std::string("data/gfx/greenball.png"));

	return true;
}


	typedef std::vector<std::string> themelist;
	class ThemesCounter { public:
		themelist* themes;
		ThemesCounter(themelist* t) : themes(t) {}
		inline bool operator() (const std::string& dir) {
			size_t pos = findLastPathSep(dir);
			std::string theme = dir.substr(pos+1);
			if(CMap::validateTheme(theme))
				themes->push_back(theme);
			return true;
		}
	};

///////////////////
// Finds a theme at random and returns the name
std::string CMap::findRandomTheme() {
    // Find directories in the theme dir
	themelist themes;

    // Count the number of themes
	ThemesCounter counter(&themes);
	FindFiles(counter, "data/themes", false, FM_DIR);

	if(themes.size() == 0) {
		// If we get here, then default to dirt
		notes("CMap::findRandomTheme(): no themes found\n");
		notes("                         Defaulting to \"dirt\"\n");
		return "dirt";
	}

    // Get a random number
    int t = GetRandomInt((int)themes.size()-1);
    return themes[t];
}


///////////////////
// Checks if a theme is a valid theme
bool CMap::validateTheme(const std::string& name) {
    // Does simple checks to see if the main files exists
    // Ie 'backtile.png' 'fronttile.png' & 'theme.txt'

    std::string thm,buf;

	thm = std::string("data/themes/") + name;

    // Backtile.png
    buf = thm + "/backtile.png";
    if (!IsFileAvailable(buf,false))
        return false;

    // Fronttile.png
    buf = thm + "/fronttile.png";
    if (!IsFileAvailable(buf,false))
        return false;

    // Theme.txt
    buf = thm + "/theme.txt";
    if (!IsFileAvailable(buf,false))
        return false;

    // All 3 files are good

    return true;
}


///////////////////
// Creates the level surface
bool CMap::CreateSurface()
{
	SDL_PixelFormat *fmt = getMainPixelFormat();
	if(fmt == NULL)
		errors("CMap::CreateSurface: ERROR: fmt is nothing\n");

	bmpImage = gfxCreateSurface(Width, Height);
	if(bmpImage.get() == NULL) {
		SetError("CMap::CreateSurface(): bmpImage creation failed, perhaps out of memory");
		return false;
	}

#ifdef _AI_DEBUG
	bmpDebugImage = gfxCreateSurface(Width*2, Height*2);
	if (bmpDebugImage.get() == NULL)  {
		SetError("CMap::CreateSurface(): bmpDebugImage creation failed perhaps out of memory");
		return false;
	}

	SetColorKey(bmpDebugImage.get());
	FillSurfaceTransparent(bmpDebugImage.get());
#endif

	bmpDrawImage = gfxCreateSurface(Width*2, Height*2);
	if(bmpDrawImage.get() == NULL) {
		SetError("CMap::CreateSurface(): bmpDrawImage creation failed, perhaps out of memory");
		return false;
	}

	bmpBackImage = gfxCreateSurface(Width, Height);
	if(bmpBackImage.get() == NULL) {
		SetError("CMap::CreateSurface(): bmpBackImage creation failed, perhaps out of memory");
		return false;
	}

    bmpShadowMap = gfxCreateSurface(Width, Height);
	if(bmpShadowMap.get() == NULL) {
		SetError("CMap::CreateSurface(): bmpShadowMap creation failed, perhaps out of memory");
		return false;
	}

	// minimap will be allocated in MiniCreate()
	
	return true;
}

////////////////////
// Updates an area according to pixel flags, recalculates minimap, draw image, pixel flags and shadow
void CMap::UpdateArea(int x, int y, int w, int h, bool update_image)
{
	if(bDedicated) return;

	int i, j;

	// When drawing shadows, we have to update a bigger area
	int shadow_update = tLXOptions->bShadows ? SHADOW_DROP : 0;

	x -= shadow_update;
	y -= shadow_update;
	w += 2 * shadow_update;
	h += 2 * shadow_update;


	// Clipping
	if (!ClipRefRectWith(x, y, w, h, (SDLRect&)bmpImage.get()->clip_rect))
		return;

	// Grid
	lockFlags();

	// Update the bmpImage according to pixel flags
	if (update_image && (bmpBackImage.get() || bmpBackImageHiRes.get()))  {

		// Update
		if( bmpBackImageHiRes.get() )
		{
			LOCK_OR_QUIT(bmpDrawImage);
			LOCK_OR_QUIT(bmpBackImageHiRes);
			Uint8 *img_pixel, *back_pixel;
			Uint16 ImgRowStep, ImgRowSize;
			byte bpp = bmpDrawImage.get()->format->BytesPerPixel;
			byte bppX2 = bpp * 2;

			img_pixel = (Uint8 *)bmpDrawImage.get()->pixels + y * 2 * bmpDrawImage.get()->pitch + x * 2 * bpp;
			back_pixel = (Uint8 *)bmpBackImageHiRes.get()->pixels + y * 2 * bmpBackImageHiRes.get()->pitch + x * 2 * bpp;
			uchar** pfline = &material->line[y];
			uchar* pf = &(*pfline)[x];

			ImgRowSize = bmpDrawImage.get()->pitch;
			ImgRowStep = ImgRowSize * 2 - (w * bpp * 2);

			for (i = h; i; --i)  {
				for (j = w; j; --j)  {
					if (m_materialList[*pf].toLxFlags() & PX_EMPTY) // Empty pixel - copy from the background image
					{
						memcpy(img_pixel, back_pixel, bppX2);
						memcpy(img_pixel + ImgRowSize, back_pixel + ImgRowSize, bppX2);
					}

					img_pixel += bppX2;
					back_pixel += bppX2;
					pf++;
				}

				img_pixel += ImgRowStep;
				back_pixel += ImgRowStep;
				pfline++;
				pf = &(*pfline)[x];
			}
			UnlockSurface(bmpDrawImage);
			UnlockSurface(bmpBackImageHiRes);
		}
		else
		{
			LOCK_OR_QUIT(bmpImage);
			LOCK_OR_QUIT(bmpBackImage);

			// Init the variables
			Uint8 *img_pixel, *back_pixel;
			Uint16 ImgRowStep, BackRowStep;
			byte bpp = bmpImage.get()->format->BytesPerPixel;

			img_pixel = (Uint8 *)bmpImage.get()->pixels + y * bmpImage.get()->pitch + x * bpp;
			back_pixel = (Uint8 *)bmpBackImage.get()->pixels + y * bmpBackImage.get()->pitch + x * bpp;
			uchar** pfline = &material->line[y];
			uchar* pf = &(*pfline)[x];

			ImgRowStep = bmpImage.get()->pitch - (w * bpp);
			BackRowStep = bmpBackImage.get()->pitch - (w * bpp);

			for (i = h; i; --i)  {
				for (j = w; j; --j)  {
					if (m_materialList[*pf].toLxFlags() & PX_EMPTY) // Empty pixel - copy from the background image
						memcpy(img_pixel, back_pixel, bpp);

					img_pixel += bpp;
					back_pixel += bpp;
					pf++;
				}

				img_pixel += ImgRowStep;
				back_pixel += BackRowStep;
				pfline++;
				pf = &(*pfline)[x];
			}
			UnlockSurface(bmpImage);
			UnlockSurface(bmpBackImage);
		}
	}

	unlockFlags();

	// Apply shadow
	ApplyShadow(x - shadow_update, y - shadow_update, w + 2 * shadow_update, h + 2 * shadow_update);

	// Update draw image
	UpdateDrawImage(x, y, w, h);

	// Update minimap
	UpdateMiniMapRect(x - shadow_update - 10, y - shadow_update - 10, w + 2 * shadow_update + 20, h + 2 * shadow_update + 20);
}



inline Color Resample2_getColor(SDL_Surface* bmpSrc, int sx, int sy) {
	if(sx < 0 || sx >= bmpSrc->w || sy < 0 || sy >= bmpSrc->h) return Color(0,0,0);
	return Color( bmpSrc->format, GetPixel(bmpSrc, sx, sy) );
}

inline bool Resample2_isDominantColor(SDL_Surface* bmpSrc, int dx, int dy) {
	Color baseC = Resample2_getColor(bmpSrc, dx, dy);
	Color otherC;
	int count = 0, otherColCount = 0;

	Color curC;

	for(short x = -1; x <= 1; x += 1) {
		for(short y = (x == 0) ? -1 : 0; y <= 1; y += 2) {
			curC = Resample2_getColor(bmpSrc, dx + x, dy + y);

			if(baseC == curC) count++;
			else {
				if(otherC != curC) {
					otherColCount++;
					otherC = curC;
				}
			}

		}
	}

	return count >= 3 || (count == 2 && otherColCount >= 2);
}

void DrawImageResampled2(SDL_Surface* bmpDest, SDL_Surface* bmpSrc, int sx, int sy, int w, int h) {
	if (!ClipRefRectWith(sx, sy, w, h, (SDLRect&)bmpSrc->clip_rect))
		return;

	int dx2 = sx*2 + w*2;
	int dy2 = sy*2 + h*2;
	for(int dy = sy*2; dy < dy2; dy++) {
		for(int dx = sx*2; dx < dx2; dx++) {

			Color col;

			if(dx % 2 == 0 && dy % 2 == 0)
				{ col = Resample2_getColor(bmpSrc, dx/2, dy/2); }
			else if(dx % 2 == 1 && dy % 2 == 0)
				{ col = Resample2_getColor(bmpSrc, dx/2, dy/2) * 0.5 + Resample2_getColor(bmpSrc, (dx + 1)/2, dy/2) * 0.5; }
			else if(dx % 2 == 0 && dy % 2 == 1)
				{ col = Resample2_getColor(bmpSrc, dx/2, dy/2) * 0.5 + Resample2_getColor(bmpSrc, dx/2, (dy - 1)/2) * 0.5; }
			else
				{ col =
					Resample2_getColor(bmpSrc, dx/2, dy/2) * 0.25 + Resample2_getColor(bmpSrc, dx/2, (dy - 1)/2) * 0.25 +
					Resample2_getColor(bmpSrc, (dx + 1)/2, dy/2) * 0.25 + Resample2_getColor(bmpSrc, (dx + 1)/2, (dy - 1)/2) * 0.25;
				}

			Uint8* dst_px = (Uint8 *)bmpDest->pixels + dy * bmpDest->pitch + dx * bmpDest->format->BytesPerPixel;
			PutPixelToAddr(dst_px, col.get(bmpDest->format), bmpDest->format->BytesPerPixel);
		}
	}
}


////////////////////
// Updates the bmpDrawImage with data from bmpImage
// X, Y, W, H apply to bmpImage, not bmpDrawImage
void CMap::UpdateDrawImage(int x, int y, int w, int h)
{
	if(bDedicated) return;
	
	if(gusIsLoaded())
		return;

	if( bmpBackImageHiRes.get() )
		return;
		
	if(tLXOptions->bAntiAliasing)
		DrawImageScale2x(bmpDrawImage.get(), bmpImage, x, y, x*2, y*2, w, h);
	else
		DrawImageStretch2(bmpDrawImage.get(), bmpImage, x, y, x*2, y*2, w, h);
}

////////////////
// Set dimensions of the minimap
void CMap::SetMinimapDimensions(uint _w, uint _h)
{
	if(bDedicated) return;

	// check if values make sense
	if(_w >= 320 || _h >= 240) {
		errors << "CMap::SetMinimapDimensions: dimension " << _w << "x" << _h << " too big" << endl;
		// default values from Menu_HostShowMinimap
		_w = 128; _h = 96;
	}
	
	// If already created, reallocate
	if (bmpMiniMap.get())  {
		bmpMiniMap = gfxCreateSurface(_w, _h);
		MinimapWidth = _w;
		MinimapHeight = _h;
		UpdateMiniMap(true);
	// Just set it and CreateSurface will do the rest of the job
	} else {
		// TODO: why can we assure here that the bmpMiniMap has the right dimension?
		MinimapWidth = _w;
		MinimapHeight = _h;
	}
}




///////////////////
// Tile the map
void CMap::TileMap()
{
	if(bDedicated) return;
	
	if(gusIsLoaded()) return;
	
	if(!bmpImage.get()) {
		errors << "CMap::TileMap: map-image not loaded" << endl;
		return;
	}
	
	uint x,y;

	// Place the tiles

	// Place the first row
	if(bmpImage.get())
		for(y=0;y<Height;y+=Theme.bmpFronttile.get()->h) {
			for(x=0;x<Width;x+=Theme.bmpFronttile.get()->w) {
				DrawImage(bmpImage.get(), Theme.bmpFronttile,x,y);
			}
		}

	if(bmpBackImage.get())
		for(y=0;y<Height;y+=Theme.bmpBacktile.get()->h) {
			for(x=0;x<Width;x+=Theme.bmpBacktile.get()->w) {
				DrawImage(bmpBackImage.get(), Theme.bmpBacktile,x,y);
			}
		}

	// Update the draw image
	UpdateDrawImage(0, 0, Width, Height);

	// Set the pixel flags
	lockFlags();
	memset(material->surf->pixels, Material::indexFromLxFlag(PX_DIRT), material->surf->pitch * Height * sizeof(uchar));
	unlockFlags();

    // Calculate the shadowmap
    CalculateShadowMap();

    // Calculate the total dirt count
    nTotalDirtCount = Width * Height;

	bMiniMapDirty = true;
}


///////////////////
// Calculate the dirt count in the level
void CMap::CalculateDirtCount()
{
    nTotalDirtCount = 0;

	for (int y = Height - 1; y >= 0; y--)
		for (int x = Width - 1; x >= 0; x--)
			if(unsafeGetMaterial((uint)x, (uint)y).toLxFlags() & PX_DIRT)
				nTotalDirtCount++;
}


static bool getGroundPos(CMap* cMap, const CVec& pos, CVec* ret, uchar badPX) {
	// TODO: optimise
	
	long x = long(pos.x), y = long(pos.y);
	
	uchar px = cMap->GetPixelFlag(x,y);
	if(px & badPX) { // we are already bad, go up
		while(y >= 0) {
			px = cMap->GetPixelFlag(x,y);
			if(!(px & badPX)) { // found place
				*ret = CVec((float)x, (float)y);
				return true;
			}
			y--;
		}
		// nothing found
		return false;
	}
	
	// go down
	while((unsigned long)y < cMap->GetHeight()) {
		px = cMap->GetPixelFlag(x,y);
		if(px & badPX) { // found place
			*ret = CVec((float)x, (float)(y - 1));
			return true;
		}
		y++;		
	}
	
	// everything free
	*ret = CVec((float)x, (float)(cMap->GetHeight() - 1));
	return true;
}

CVec CMap::groundPos(const CVec& pos) {
	CVec ret = pos;
	lockFlags(false);	
	
	if(getGroundPos(this, pos, &ret, PX_DIRT|PX_ROCK)) {
		unlockFlags(false);
		return ret;
	}
	
	getGroundPos(this, pos, &ret, PX_ROCK);
	
	unlockFlags(false);
	return ret;
}


///////////////////
// Draw the map
void CMap::Draw(SDL_Surface *bmpDest, const SDL_Rect& rect, int worldX, int worldY)
{
	if(!bmpDrawImage.get() || !bmpDest) return; // safty

	if(gusIsLoaded())		
		return;
	
	if(!cClient->getGameLobby()->features[FT_InfiniteMap]) {
		DrawImageAdv(bmpDest, bmpDrawImage, worldX*2, worldY*2,rect.x,rect.y,rect.w,rect.h);
#ifdef _AI_DEBUG
		DrawImageAdv(bmpDest, bmpDebugImage, worldX*2, worldY*2,rect.x,rect.y,rect.w,rect.h);
#endif
	}
	else {
		DrawImageTiled(bmpDest, bmpDrawImage, worldX*2, worldY*2, rect.w, rect.h, rect.x, rect.y, rect.w, rect.h);
#ifdef _AI_DEBUG
		DrawImageTiled(bmpDest, bmpDebugImage, worldX*2, worldY*2, rect.w, rect.h,rect.x,rect.y,rect.w,rect.h);
#endif
	}
}

///////////////////
// Draw the map
void CMap::Draw(SDL_Surface * bmpDest, CViewport *view)
{
	SDL_Rect destRect = {view->GetLeft(),view->GetTop(),view->GetWidth()*2,view->GetHeight()*2};
	Draw(bmpDest, destRect, view->GetWorldX(), view->GetWorldY());
}

struct ShadowClipInfo  {
	int map_x;
	int map_y;
	int dest_x;
	int dest_y;
	int obj_x;
	int obj_y;
	int w;
	int h;
};


/////////////////////
// Helper function for DrawObjectShadow
ShadowClipInfo ClipShadow(SDL_Surface * bmpDest, SDL_Surface * bmpObj, SDL_Surface *bmpShadowMap,
						  int sx, int sy, int w, int h, CViewport *view, int wx, int wy)
{
	ShadowClipInfo res;

	res.dest_x = ((wx + SHADOW_DROP - view->GetWorldX()) * 2) + view->GetLeft();
	res.dest_y = ((wy + SHADOW_DROP - view->GetWorldY()) * 2) + view->GetTop();

	// Calculate positions and clipping
	int clip_shift_x = - MIN(0, wx - view->GetWorldX() + SHADOW_DROP) * 2;  // When we clip left/top on dest, we have to
	int clip_shift_y = - MIN(0, wy - view->GetWorldY() + SHADOW_DROP) * 2;  // "shift" the coordinates on other surfaces, too

	res.obj_x = sx + clip_shift_x;
	res.obj_y = sy + clip_shift_y;

	res.map_x = wx + SHADOW_DROP + clip_shift_x;
	res.map_y = wy + SHADOW_DROP + clip_shift_y;
	int shadowmap_real_w = w / 2;
	int shadowmap_real_h = h / 2;

	// Clipping
	ClipRefRectWith(res.dest_x, res.dest_y, w, h, (SDLRect&)bmpDest->clip_rect);
	ClipRefRectWith(res.obj_x, res.obj_y, w, h, (SDLRect&)bmpObj->clip_rect);
	ClipRefRectWith(res.map_x, res.map_y, shadowmap_real_w, shadowmap_real_h, (SDLRect&)bmpShadowMap->clip_rect);

	res.w = MIN(w, shadowmap_real_w * 2);
	res.h = MIN(h, shadowmap_real_h * 2);

	return res;
}

///////////////////
// Checks if the given area is free (no rock/dirt inside)
bool CMap::CheckAreaFree(int x, int y, int w, int h)
{
	// If there is some kind of clipping, the area is not free
	if (x < 0 || x + w >= (int)Width || y < 0 || y + h >= (int)Height)
		return false;

	for(int dy = 0; dy < h; ++dy)
		for(int dx = 0; dx < w; ++dx)
			if(GetPixelFlag(x + dx, y + dy) & (PX_ROCK|PX_DIRT))
				return false;

	return true;
}

///////////////////
// Draw an object's shadow
void CMap::DrawObjectShadow(SDL_Surface * bmpDest, SDL_Surface * bmpObj, SDL_Surface * bmpObjShadow, int sx, int sy, int w, int h, CViewport *view, int wx, int wy)
{
	if(gusIsLoaded()) return;
	
	// TODO: simplify, possibly think up a better algo...
	// TODO: reduce local variables to 5
	if(!bmpDest) {
		errors << "CMap::DrawObjectShadow: bmpDest not set" << endl;
		return;
	}
	
	if(!bmpObj) {
		errors << "CMap::DrawObjectShadow: bmpObj not set" << endl;
		return;
	}
	
	if(!view) {
		errors << "CMap::DrawObjectShadow: view not set" << endl;
		return;
	}
	
	if(!bmpShadowMap.get()) {
		errors << "CMap::DrawObjectShadow: shadow map not initialised" << endl;
		return;
	}

	const ShadowClipInfo i = ClipShadow(bmpDest, bmpObj, bmpShadowMap.get(), sx, sy, w, h, view, wx, wy);
	if (!i.h || !i.w)
		return;

	// If we are bigger, we have to check the whole area of the shadow
	if (CheckAreaFree(i.map_x, i.map_y, i.w, i.h) && bmpObjShadow)  {
		DrawImageAdv(bmpDest, bmpObjShadow, i.obj_x, i.obj_y, i.dest_x, i.dest_y, i.w, i.h);
		return;
	}

	// Lock the surfaces
	LOCK_OR_QUIT(bmpDest);
	LOCK_OR_QUIT(bmpObj);
	LOCK_OR_QUIT(bmpShadowMap);

	// Pixels
	Uint8 *dest_row = (Uint8 *)bmpDest->pixels + (i.dest_y * bmpDest->pitch) + (i.dest_x * bmpDest->format->BytesPerPixel);
	Uint8 *obj_row  = (Uint8 *)bmpObj->pixels + (i.obj_y * bmpObj->pitch) + (i.obj_x * bmpObj->format->BytesPerPixel);
	Uint8 *shadow_row = (Uint8 *)bmpShadowMap->pixels + (i.map_y * bmpShadowMap->pitch) + (i.map_x * bmpShadowMap->format->BytesPerPixel);
	uchar** pfline = &material->line[i.map_y];

	PixelGet& getter = getPixelGetFunc(bmpObj);
	PixelCopy& copier = getPixelCopyFunc(bmpShadowMap.get(), bmpDest);

	// Draw the shadow
	for (int loop_y = i.h; loop_y; --loop_y)  {
		uchar* pf = &(*pfline)[i.map_x];
		Uint8 *shadowmap_px = shadow_row;
		Uint8 *obj_px = obj_row;
		Uint8 *dest_px = dest_row;

		for (int loop_x = i.w; loop_x; --loop_x)  {

			if (m_materialList[*pf].toLxFlags() & PX_EMPTY)  { // Don't draw shadow on solid objects

				// Put pixel if not tranparent
				if (!IsTransparent(bmpObj, getter.get(obj_px)))
					copier.copy(dest_px, shadowmap_px);
			}

			// Update the pixels & flag
			dest_px		 += bmpDest->format->BytesPerPixel;
			obj_px		 += bmpObj->format->BytesPerPixel;
			shadowmap_px += bmpShadowMap->format->BytesPerPixel * (loop_x & 1); // We draw the shadow doubly stretched -> only 1/2 pixel on shadowmap
			pf			 += loop_x & 1;
		}

		// Skip to next row
		dest_row	   += bmpDest->pitch;
		obj_row		   += bmpObj->pitch;
		shadow_row	   += bmpShadowMap->pitch * (loop_y & 1); // We draw the shadow doubly stretched -> only 1/2 row on shadowmap
		pfline		   += loop_y & 1;
	}

	UnlockSurface(bmpShadowMap);
	UnlockSurface(bmpDest);
	UnlockSurface(bmpObj);
}


///////////////////
// Draw a pixel sized shadow
void CMap::DrawPixelShadow(SDL_Surface * bmpDest, CViewport *view, int wx, int wy)
{
	if(bDedicated) return;
	if(gusIsLoaded()) return;
	
    wx += SHADOW_DROP;
    wy += SHADOW_DROP;

	// HINT: Clipping is done by DrawImageAdv/GetPixelFlag

	// Get real coordinates
	// TODO: DrawObjectShadow also doesn't do correct shadow for infinite maps, so let's ignore it here too atm
	//VectorD2<int> p = view->physicToReal(VectorD2<int>(wx,wy), cClient->getGameLobby()->features[FT_InfiniteMap], GetWidth(), GetHeight());
	VectorD2<int> p = view->physicToReal(VectorD2<int>(wx,wy));
	
	// NOTE: if we fix shadow for infinite maps here, we cannot use GetPixel this way!
	if( GetPixelFlag(wx, wy /*, cClient->getGameLobby()->features[FT_InfiniteMap]*/) & PX_EMPTY )  {  // We should check all the 4 pixels, but no one will ever notice it
		LOCK_OR_QUIT(bmpShadowMap);
		Color color = Color(bmpShadowMap->format, GetPixel(bmpShadowMap.get(), wx, wy));
		UnlockSurface(bmpShadowMap);

		DrawRectFill2x2(bmpDest, p.x, p.y, color);
	}
}


///////////////////
// Carve a hole in the map
//
// Returns the number of dirt pixels carved
// IMPORTANT: hole and map must have same gfx format
int CMap::CarveHole(int size, CVec pos, bool wrapAround)
{
	// Just clamp it and continue
	size = MAX(size, 0);
	size = MIN(size, 4);

	// Calculate half
	SmartPointer<SDL_Surface> hole = Theme.bmpHoles[size];
	if (!hole.get())
		return 0;

	int nNumDirt = 0;
	int w = hole.get()->w;
	int h = hole.get()->h;
	int map_x = (int)pos.x - w / 2;
	int map_y = (int)pos.y - h / 2;
	if (wrapAround)  {
		int map_x2 = map_x;
		int map_y2 = map_y;
		if (map_x <= hole->w / 2) 
			map_x2 = map_x + (int)Width;
		if (map_x >= (int)Width - hole->w / 2)
			map_x2 = map_x - (int)Width;
		if (map_y <= hole->h / 2) 
			map_y2 = map_y + (int)Height;
		if (map_y >= (int)Height - hole->h / 2)
			map_y2 = map_y - (int)Height;
		if (map_x2 != map_x || map_y2 != map_y)
			this->CarveHole(size, CVec((float)map_x2 + w / 2, (float)map_y2 + h / 2), false);
	}

	// Clipping
	if (!ClipRefRectWith(map_x, map_y, w, h, (SDLRect&)bmpImage.get()->clip_rect))
		return 0;
			
	SaveToMemoryInternal( map_x, map_y, w, h );

	// Variables
	byte bpp = hole.get()->format->BytesPerPixel;
	
	if (!LockSurface(hole))
		return 0;

	Uint8* hole_px = (Uint8 *)hole.get()->pixels;

	uchar** PixelFlagLine = &material->line[map_y];
	int HoleRowStep = hole.get()->pitch - (w * bpp);

	// Lock
	lockFlags();

	if( bmpBackImageHiRes.get() ) // Hi-res image
	{
		if (!LockSurface(bmpDrawImage))
			return 0;
		Uint8* mapimage_px = (Uint8 *)bmpDrawImage.get()->pixels + map_y * 2 * bmpDrawImage.get()->pitch + map_x * bpp * 2;
		int MapImageRowSize = bmpDrawImage.get()->pitch;
		int MapImageRowStep = bmpDrawImage.get()->pitch * 2 - (w * bpp * 2);
		for(int hy = h; hy; --hy)  
		{
			uchar* PixelFlag = &(*PixelFlagLine)[map_x];
			for(int hx = w; hx; --hx) 
			{
				if (m_materialList[*PixelFlag].toLxFlags() & PX_DIRT)  // Carve only dirt
				{
					Uint32 CurrentPixel = GetPixelFromAddr(hole_px, bpp);
					// Set the flag to empty
					if(CurrentPixel == tLX->clPink.get(hole.get()->format)) 
					{
						// Increase the dirt count
						nNumDirt++;
						*PixelFlag = Material::indexFromLxFlag(PX_EMPTY);
					} 
					else if(CurrentPixel != tLX->clBlack.get(hole.get()->format)) // Put pixels that are not black/pink (eg, brown)
					{
						PutPixelToAddr(mapimage_px, CurrentPixel, bpp);
						PutPixelToAddr(mapimage_px+bpp, CurrentPixel, bpp);
						PutPixelToAddr(mapimage_px+MapImageRowSize, CurrentPixel, bpp);
						PutPixelToAddr(mapimage_px+MapImageRowSize+bpp, CurrentPixel, bpp);
					}
				}
				hole_px += bpp;
				mapimage_px += bpp * 2;
				PixelFlag++;
			}
			hole_px += HoleRowStep;
			mapimage_px += MapImageRowStep;
			PixelFlagLine++;
		}
		UnlockSurface(bmpDrawImage);
	}
	else // Low-res image
	{
		if (!LockSurface(bmpImage))
			return 0;
		Uint8* mapimage_px = (Uint8 *)bmpImage.get()->pixels + map_y * bmpImage.get()->pitch + map_x * bpp;
		Uint8* back_px = (Uint8 *)background->surf->pixels + map_y * background->surf->pitch + map_x * bpp;
		int MapImageRowStep = bmpImage.get()->pitch - (w * bpp);
		for(int hy = h; hy; --hy)  {
			uchar* PixelFlag = &(*PixelFlagLine)[map_x];
			for(int hx = w; hx; --hx) {

				// Carve only dirt
				if (m_materialList[*PixelFlag].toLxFlags() & PX_DIRT)  {

					Uint32 CurrentPixel = GetPixelFromAddr(hole_px, bpp);

					// Set the flag to empty
					if(CurrentPixel == tLX->clPink.get(hole.get()->format)) {

						// Increase the dirt count
						nNumDirt++;

						*PixelFlag = Material::indexFromLxFlag(PX_EMPTY);
						PutPixelToAddr(mapimage_px, GetPixelFromAddr(back_px, bpp), bpp);

					// Put pixels that are not black/pink (eg, brown)
					} else if(CurrentPixel != tLX->clBlack.get(hole.get()->format))
						PutPixelToAddr(mapimage_px, CurrentPixel, bpp);
				}

				hole_px += bpp;
				mapimage_px += bpp;
				back_px += bpp;
				PixelFlag++;

			}

			hole_px += HoleRowStep;
			mapimage_px += MapImageRowStep;
			back_px += MapImageRowStep;
			PixelFlagLine++;
		}
		UnlockSurface(bmpImage);
	}

	unlockFlags();

	UnlockSurface(hole);

	if(nNumDirt)  { // Update only when something has been carved
		UpdateArea(map_x, map_y, w, h, true);
	}

    return nNumDirt;
}


/*inline void CarveHole_handlePixel(CMap* map, int& nNumDirt, int map_x, int map_y, Uint32 hole_pixel) {
	uchar* px = map->GetPixelFlags() + map_y * map->GetWidth() + map_x;

	if(*px & PX_DIRT) {
		// Set the flag to empty
		if(hole_pixel == tLX->clPink) {

			// Increase the dirt count
			nNumDirt++;

			*px = PX_EMPTY;

		// Put pixels that are not black/pink (eg, brown)
		} else if(hole_pixel != tLX->clBlack)
			PutPixel(map->GetImage(), map_x, map_y, hole_pixel);
	}

	if(*px & PX_EMPTY) {
		// redraw background-pixel because perhaps we don't have shadow here any more
		// we will update the shadowed pixel later
		CopyPixel_SameFormat(map->GetImage(), map->GetBackImage(), map_x, map_y);
	}

}

class CarveHole_PixelWalker {
public:
	CMap* map; SmartPointer<SDL_Surface> hole; int& nNumDirt;
	int map_left, map_top;

	CarveHole_PixelWalker(CMap* map_, const SmartPointer<SDL_Surface> & hole_, int& nNumDirt_, int map_left_, int map_top_) :
		map(map_), hole(hole_), nNumDirt(nNumDirt_), map_left(map_left_), map_top(map_top_) {}

	inline bool operator()(int map_x, int map_y) {
		CarveHole_handlePixel(map, nNumDirt, map_x, map_y, GetPixel(hole, map_x - map_left, map_y - map_top));
		return true;
	}

};


///////////////////
// Carve a hole in the map
//
// Returns the number of dirt pixels carved
int CMap::CarveHole(int size, CVec pos)
{
	if(size < 0 || size > 4) {
		// Just clamp it and continue
		size = MAX(size, 0);
		size = MIN(size, 4);
	}

	// Calculate half
	SmartPointer<SDL_Surface> hole = Theme.bmpHoles[size];
	if (!hole)
		return 0;

	int nNumDirt = 0;
	int map_left = (int)pos.x - (hole->w / 2);
	int map_top = (int)pos.y - (hole->h / 2);

	lockFlags();

	if (!LockSurface(hole))
		return 0;
	if (!LockSurface(bmpImage))
		return 0;
	if (!LockSurface(bmpBackImage))
		return 0;

	walkPixels(ClipRect<int>(&map_left, &map_top, &hole->w, &hole->h), CarveHole_PixelWalker(this, hole, nNumDirt, map_left, map_top));

	unlockFlags();

	UnlockSurface(hole);
	UnlockSurface(bmpImage);
	UnlockSurface(bmpBackImage);

	if(!nNumDirt)
		return 0;

    // Recalculate the grid
    lockFlags();
	for(int hx = 0; hx < hole->w; hx += nGridWidth)
		for(int hy = 0; hy < hole->h; hy += nGridHeight) {
			const int x = map_left + hx;
			const int y = map_top + hy;
			calculateGridCell(x, y, true);
		}
	unlockFlags();

	// Apply a shadow
	ApplyShadow(map_left - 5, map_top - 5, hole->w + 25, hole->h + 25);

	// Update the draw image
	UpdateDrawImage(map_left - 5, map_top - 5, hole->w + 25, hole->h + 25);

	UpdateMiniMapRect(map_left, map_top, hole->w, hole->h);

    return nNumDirt;
}*/


///////////////////
// Place a bit of dirt
//
// Returns the number of dirt pixels placed
int CMap::PlaceDirt(int size, CVec pos)
{
	SmartPointer<SDL_Surface> hole;
	int dx,dy, sx,sy;
	int x,y;
	int w,h;
	int ix,iy;
	Uint32 pixel, pixel2;

    int nDirtCount = 0;

	// Just clamp it and continue
	size = MAX(size,0);
	size = MIN(size,4);


	// Calculate half
	hole = Theme.bmpHoles[size];
	if(hole.get() == NULL) {
		errors << "CMap::PlaceDirt: hole with size " << size << " unset" << endl;
		return 0;
	}
	
	Uint32 pink = tLX->clPink.get(hole.get()->format);
	w = hole.get()->w;
	h = hole.get()->h;

	sx = (int)pos.x-(hole.get()->w>>1);
	sy = (int)pos.y-(hole.get()->h>>1);


	if (!LockSurface(hole))
		return 0;
	if (!LockSurface(Theme.bmpFronttile))
		return 0;

	short screenbpp = getMainPixelFormat()->BytesPerPixel;

	// Calculate clipping
	int clip_y = MAX(sy, 0);
	int clip_x = MAX(sx, 0);
	int clip_h = MIN(sy+h, bmpImage.get()->h);
	int clip_w = MIN(sx+w, bmpImage.get()->w);
	int hole_clip_y = -MIN(sy,(int)0);
	int hole_clip_x = -MIN(sx,(int)0);
	
	SaveToMemoryInternal( clip_x, clip_y, clip_w, clip_h );

	lockFlags();

	if( bmpBackImageHiRes.get() ) // Hi-res image
	{
		if (!LockSurface(bmpDrawImage))
			return 0;
		// Go through the pixels in the hole, setting the flags to dirt
		int DrawImagePitch = bmpDrawImage.get()->pitch;
		for(y = hole_clip_y, dy = clip_y; dy < clip_h; y++, dy++) {

			Uint8* p = (Uint8 *)hole.get()->pixels + y * hole.get()->pitch + hole_clip_x * hole.get()->format->BytesPerPixel;
			uchar* px = &material->line[dy][clip_x];
			Uint8* p2 = (Uint8 *)bmpDrawImage.get()->pixels + dy * 2 * bmpDrawImage.get()->pitch + clip_x * 2 * bmpDrawImage.get()->format->BytesPerPixel;

			for(x=hole_clip_x,dx=clip_x;dx<clip_w;x++,dx++) {

				pixel = GetPixelFromAddr(p, screenbpp);
				uchar flag = m_materialList[*px].toLxFlags();

				ix = dx % Theme.bmpFronttile.get()->w;
				iy = dy % Theme.bmpFronttile.get()->h;

				// Set the flag to empty
				if(!IsTransparent(hole.get(), pixel) && !(flag & PX_ROCK)) {
                    if( flag & PX_EMPTY )
                        nDirtCount++;

					*(uchar*)px = Material::indexFromLxFlag(PX_DIRT);
					pixel2 = GetPixel(Theme.bmpFronttile.get(), ix, iy);
					// Place the dirt image
					PutPixelToAddr(p2, pixel2, screenbpp);
					PutPixelToAddr(p2+screenbpp, pixel2, screenbpp);
					PutPixelToAddr(p2+DrawImagePitch, pixel2, screenbpp);
					PutPixelToAddr(p2+DrawImagePitch+screenbpp, pixel2, screenbpp);
				}

				// Put pixels that are not black/pink (eg, brown)
                if(!IsTransparent(hole.get(), pixel) && pixel != pink && (flag & PX_EMPTY)) {
					PutPixelToAddr(p2, pixel, screenbpp);
					PutPixelToAddr(p2+screenbpp, pixel, screenbpp);
					PutPixelToAddr(p2+DrawImagePitch, pixel, screenbpp);
					PutPixelToAddr(p2+DrawImagePitch+screenbpp, pixel, screenbpp);
                    *(uchar*)px = Material::indexFromLxFlag(PX_DIRT);
                    nDirtCount++;
                }

				p += screenbpp;
				p2 += screenbpp * 2;
				px++;
			}
		}
		UnlockSurface(bmpDrawImage);
	}
	else // Low-res image
	{
		if (!LockSurface(bmpImage))
			return 0;
		// Go through the pixels in the hole, setting the flags to dirt
		for(y = hole_clip_y, dy = clip_y; dy < clip_h; y++, dy++) {

			Uint8* p = (Uint8 *)hole.get()->pixels + y * hole.get()->pitch + hole_clip_x * hole.get()->format->BytesPerPixel;
			uchar* px = &material->line[dy][clip_x];
			Uint8* p2 = (Uint8 *)bmpImage.get()->pixels + dy * bmpImage.get()->pitch + clip_x * bmpImage.get()->format->BytesPerPixel;

			for(x=hole_clip_x,dx=clip_x;dx<clip_w;x++,dx++) {

				pixel = GetPixelFromAddr(p, screenbpp);
				uchar flag = m_materialList[*px].toLxFlags();

				ix = dx % Theme.bmpFronttile.get()->w;
				iy = dy % Theme.bmpFronttile.get()->h;

				// Set the flag to empty
				if(!IsTransparent(hole.get(), pixel) && !(flag & PX_ROCK)) {
                    if( flag & PX_EMPTY ) {
                        nDirtCount++;
						putpixel(background, dx, dy, getpixel(image, dx, dy));
					}
					
					*(uchar*)px = Material::indexFromLxFlag(PX_DIRT);

					// Place the dirt image
					PutPixelToAddr(p2, GetPixel(Theme.bmpFronttile.get(), ix, iy), screenbpp);
				}

				// Put pixels that are not black/pink (eg, brown)
				if(!IsTransparent(hole.get(), pixel) && pixel != pink && (flag & PX_EMPTY)) {
					PutPixelToAddr(p2, pixel, screenbpp);
					// TODO: this should be obsolete & wrong, shoudln't it?
					//*(uchar*)px = Material::indexFromLxFlag(PX_DIRT);
                    //nDirtCount++;
                }

				p += screenbpp;
				p2 += screenbpp;
				px++;
			}
		}
		UnlockSurface(bmpImage);
	}

	unlockFlags();

	UnlockSurface(hole);
	UnlockSurface(Theme.bmpFronttile);

	UpdateArea(sx, sy, w, h);

    return nDirtCount;
}


///////////////////
// Place a blob of green dirt (greenball)
//
// Returns the number of dirt pixels placed
int CMap::PlaceGreenDirt(CVec pos)
{
	if (!bmpGreenMask.get()) {
		warnings << "CMap::PlaceGreenDirt: bmpGreenMask not loaded" << endl;
		return 0;
	}
	
 	int dx,dy, sx,sy;
	int x,y;
	int w,h;
	Uint32 pixel;
    const Uint32 green = MakeColour(0,255,0);
	const Uint32 pink = MakeColour(255,0,255);
    const Uint32 greens[4] = {MakeColour(148,136,0),
				              MakeColour(136,124,0),
						      MakeColour(124,112,0),
							  MakeColour(116,100,0)};

    int nGreenCount = 0;

	// Calculate half
	w = bmpGreenMask.get()->w;
	h = bmpGreenMask.get()->h;

	sx = (int)pos.x-(w>>1);
	sy = (int)pos.y-(h>>1);


	if (!LockSurface(bmpGreenMask))
		return 0;

	Uint32 gr;

	// Calculate clipping
	int clip_y = MAX(sy, 0);
	int clip_x = MAX(sx, 0);
	int clip_h = MIN(sy+h, bmpImage.get()->h);
	int clip_w = MIN(sx+w, bmpImage.get()->w);
	int green_clip_y = -MIN(sy,(int)0);
	int green_clip_x = -MIN(sx,(int)0);
	
	SaveToMemoryInternal( clip_x, clip_y, clip_w, clip_h );

	short screenbpp = getMainPixelFormat()->BytesPerPixel;

	lockFlags();

	if( bmpBackImageHiRes.get() ) // Hi-res image
	{
		if (!LockSurface(bmpDrawImage))
			return 0;
		// Go through the pixels in the hole, setting the flags to dirt
		int DrawImagePitch = bmpDrawImage.get()->pitch;
		for(y = green_clip_y, dy=clip_y; dy < clip_h; y++, dy++) {

			Uint8* p = (Uint8*)bmpGreenMask.get()->pixels
				+ y * bmpGreenMask.get()->pitch
				+ green_clip_x * bmpGreenMask.get()->format->BytesPerPixel;
			uchar* px = &material->line[dy][clip_x];
			Uint8* p2 = (Uint8 *)bmpDrawImage.get()->pixels + dy * 2 * bmpDrawImage.get()->pitch + clip_x * 2 * bmpDrawImage.get()->format->BytesPerPixel;

			for(x = green_clip_x, dx=clip_x; dx < clip_w; x++, dx++) {

				pixel = GetPixelFromAddr(p,screenbpp);
				uchar flag = m_materialList[*px].toLxFlags();

				// Set the flag to dirt
				if(pixel == green && (flag & PX_EMPTY)) {
					*(uchar*)px = Material::indexFromLxFlag(PX_DIRT);
					nGreenCount++;

					// Place a random green pixel
					gr = greens[ GetRandomInt(3) ];

					// Place the green pixel
					PutPixelToAddr(p2, gr, screenbpp);
					PutPixelToAddr(p2+screenbpp, gr, screenbpp);
					PutPixelToAddr(p2+DrawImagePitch, gr, screenbpp);
					PutPixelToAddr(p2+DrawImagePitch+screenbpp, gr, screenbpp);
				}

				// Put pixels that are not green/pink (eg, dark green)
                if(pixel != green && pixel != pink && (flag & PX_EMPTY)) {
					PutPixelToAddr(p2, pixel, screenbpp);
					PutPixelToAddr(p2+screenbpp, pixel, screenbpp);
					PutPixelToAddr(p2+DrawImagePitch, pixel, screenbpp);
					PutPixelToAddr(p2+DrawImagePitch+screenbpp, pixel, screenbpp);
					*(uchar*)px = Material::indexFromLxFlag(PX_DIRT);
					nGreenCount++;
                }

				p += screenbpp;
				p2 += screenbpp * 2;
				px++;
			}
		}
		UnlockSurface(bmpDrawImage);
	}
	else // Low-res image
	{
		if (!LockSurface(bmpImage))
			return 0;
		// Go through the pixels in the hole, setting the flags to dirt
		for(y = green_clip_y, dy=clip_y; dy < clip_h; y++, dy++) {

			Uint8* p = (Uint8*)bmpGreenMask.get()->pixels
				+ y * bmpGreenMask.get()->pitch
				+ green_clip_x * bmpGreenMask.get()->format->BytesPerPixel;
			uchar* px = &material->line[dy][clip_x];
			Uint8* p2 = (Uint8 *)bmpImage.get()->pixels + dy * bmpImage.get()->pitch + clip_x * bmpImage.get()->format->BytesPerPixel;

			for(x = green_clip_x, dx=clip_x; dx < clip_w; x++, dx++) {

				pixel = GetPixelFromAddr(p,screenbpp);
				uchar flag = m_materialList[*px].toLxFlags();

				// Set the flag to dirt
				if(pixel == green && (flag & PX_EMPTY)) {
					putpixel(background, dx, dy, getpixel(image, dx, dy));
					*(uchar*)px = Material::indexFromLxFlag(PX_DIRT);
                    nGreenCount++;

                    // Place a random green pixel
                    gr = greens[ GetRandomInt(3) ];

					// Place the green pixel
					PutPixelToAddr(p2, gr, screenbpp);
				}

				// Put pixels that are not green/pink (eg, dark green)
                if(pixel != green && pixel != pink && (flag & PX_EMPTY)) {
					putpixel(background, dx, dy, getpixel(image, dx, dy));
					PutPixelToAddr(p2, pixel, screenbpp);
					*(uchar*)px = Material::indexFromLxFlag(PX_DIRT);
                    nGreenCount++;
                }

				p += screenbpp;
				p2 += screenbpp;
				px++;
			}
		}
		UnlockSurface(bmpImage);
	}

	unlockFlags();

	UnlockSurface(bmpGreenMask);

	// Nothing placed, no need to update
	if (nGreenCount)  {
		UpdateArea(sx, sy, w, h);
	}

    return nGreenCount;
}

////////////////////
// Checks for an object collision with the map
CMap::CollisionInfo CMap::StaticCollisionCheck(const CVec &objpos, int objw, int objh, bool infiniteMap) const
{
	CollisionInfo result;
	result.moveToX = (int)objpos.x;
	result.moveToY = (int)objpos.y;
	
	if (infiniteMap)
		StaticCollisionCheckInfinite(objpos, objw, objh, result);
	else
		StaticCollisionCheckFinite(objpos, objw, objh, result);

	return result;
}

//////////////////////
// Collision check for finite map (with bounds hits)
void CMap::StaticCollisionCheckFinite(const CVec &objpos, int objw, int objh, CMap::CollisionInfo &result) const
{
	// Bounds
	if (objpos.x - objw / 2 <= 0)  {
		result.moveToX = objw / 2 + 1;
		result.occured = result.left = result.hitBounds = true;
	}

	if (objpos.x + objw / 2 >= Width)  {
		result.moveToX = Width - objw / 2 - 1;
		result.occured = result.right = result.hitBounds = true;
	}

	if (objpos.y - objh / 2 <= 0)  {
		result.moveToY = objh / 2 + 1;
		result.occured = result.top = result.hitBounds = true;
	}

	if (objpos.y + objh / 2 >= Height)  {
		result.moveToY = Height - objh / 2 - 1;
		result.occured = result.bottom = result.hitBounds = true;
	}

	// Cross check, taken from worm collision
	SDL_Rect coll_r = { (int)objpos.x - objw / 2, (int)objpos.y - objh / 2, objw, objh };
	if (!ClipRefRectWith(coll_r.x, coll_r.y, coll_r.w, coll_r.h, (SDLRect&)bmpImage->clip_rect))
		return;
	result.x = coll_r.x + coll_r.w / 2;
	result.y = coll_r.y + coll_r.h / 2;

	// Top to bottom
	for (int y = coll_r.y; y < coll_r.y + coll_r.h; y++)  {
		if (unsafeGetPixelFlag(coll_r.x + coll_r.w / 2, y) & (PX_ROCK|PX_DIRT))  {
			result.occured = true;
			result.hitRockDirt = true;
			result.y = y;

			if (y < coll_r.y + coll_r.h / 2)  {
				result.top = true;
				result.moveToY = y + objh / 2 + 1;
			} else {
				result.bottom = true;
				result.moveToY = y - objh / 2 - 1;
			}
		}
	}

	// Left to right
	for (int x = coll_r.x; x < coll_r.x + coll_r.w; x++)  {
		if (unsafeGetPixelFlag(x, coll_r.y + coll_r.h / 2) & (PX_ROCK|PX_DIRT))  {
			result.occured = true;
			result.hitRockDirt = true;
			result.x = x;

			if (x < coll_r.x + coll_r.w / 2)  {
				result.left = true;
				result.moveToX = x + objw / 2 + 1;
			} else {
				result.right = true;
				result.moveToX = x - objw / 2 - 1;
			}
		}
	}
}

//////////////////////////
// Collision check for infinite map
void CMap::StaticCollisionCheckInfinite(const CVec &objpos, int objw, int objh, CMap::CollisionInfo &result) const
{
	// Cross check, taken from worm collision
	SDL_Rect coll_r = { (int)objpos.x - objw / 2, (int)objpos.y - objh / 2, objw, objh };
	result.x = coll_r.x + coll_r.w / 2;
	result.y = coll_r.y + coll_r.h / 2;

	// Top to bottom
	const int wx = coll_r.x + coll_r.w / 2;
	for (int y = coll_r.y; y < coll_r.y + coll_r.h; y++)  {
		if (GetPixelFlag(wx, y, true) & (PX_ROCK|PX_DIRT))  {
			result.occured = true;
			result.hitRockDirt = true;
			result.y = WrapAroundY(y);

			if (y < coll_r.y + coll_r.h / 2)  {
				result.top = true;
				result.moveToY = WrapAroundY(y + objh / 2 + 1);
			} else {
				result.bottom = true;
				result.moveToY = WrapAroundY(y - objh / 2 - 1);
			}
		}
	}

	// Left to right
	const int wy = coll_r.y + coll_r.h / 2;
	for (int x = coll_r.x; x < coll_r.x + coll_r.w; x++)  {
		if (GetPixelFlag(x, wy, true) & (PX_ROCK|PX_DIRT))  {
			result.occured = true;
			result.hitRockDirt = true;
			result.x = WrapAroundX(x);

			if (x < coll_r.x + coll_r.w / 2)  {
				result.left = true;
				result.moveToX = WrapAroundX(x + objw / 2 + 1);
			} else {
				result.right = true;
				result.moveToX = WrapAroundX(x - objw / 2 - 1);
			}
		}
	}
}


///////////////////
// Apply a shadow to an area
void CMap::ApplyShadow(int sx, int sy, int w, int h)
{
	if(bDedicated) return;
	// Draw shadows?
	if(!tLXOptions->bShadows) return;
	if(gusIsLoaded()) return;
	if(!bmpShadowMap.get()) return;
	
	int x, y, n;
	uint ox,oy;
	Uint32 offset;

	Uint8 *pixel,*src;

	int screenbpp = getMainPixelFormat()->BytesPerPixel;

	LOCK_OR_QUIT(bmpShadowMap);

	lockFlags();

	int clip_y = MAX(sy, (int)0);
	int clip_x = MAX(sx, (int)0);
	int clip_h = MIN(sy + h, Height);
	int clip_w = MIN(sx + w, Width);

	if( bmpBackImageHiRes.get() ) // Hi-res image
	{
		LOCK_OR_QUIT(bmpDrawImage);
		int DrawImagePitch = bmpDrawImage.get()->pitch;
		int ShadowMapPitch = bmpShadowMap.get()->pitch;
		for(y = clip_y; y < clip_h; y++) 
		{
			uchar* px = &material->line[y][clip_x];
			for(x = clip_x; x < clip_w; x++) 
			{
				uchar flag = m_materialList[*(uchar *)px].toLxFlags();
				// Edge hack
				//if(x==0 || y==0 || x==Width-1 || y==Height-1)
					//flag = PX_EMPTY;
				if(!(flag & PX_EMPTY)) 
				{
					ox = x+1; oy = y+1;
					// Draw the shadow
					for(n = 0; n < SHADOW_DROP; n++) 
					{
						// Clipping
						if(ox >= Width) break;
						if(oy >= Height) break;

						if(!( unsafeGetPixelFlag(ox, oy) & PX_EMPTY))
							break;

						pixel = (Uint8*)bmpDrawImage.get()->pixels + oy*2*DrawImagePitch + ox*2*screenbpp;
						src = (Uint8*)bmpShadowMap.get()->pixels + oy*ShadowMapPitch + ox*screenbpp;
						memcpy(pixel, src, screenbpp);
						memcpy(pixel+screenbpp, src, screenbpp);
						memcpy(pixel+DrawImagePitch, src, screenbpp);
						memcpy(pixel+DrawImagePitch+screenbpp, src, screenbpp);

						ox++; oy++;
					}
				}

				px++;
			}
		}
		UnlockSurface(bmpDrawImage);
	}
	else // Low-res image
	{
		LOCK_OR_QUIT(bmpImage);
		for(y = clip_y; y < clip_h; y++) {

			uchar* px = &material->line[y][clip_x];

			for(x = clip_x; x < clip_w; x++) {

				uchar flag = m_materialList[*(uchar *)px].toLxFlags();

				// Edge hack
				//if(x==0 || y==0 || x==Width-1 || y==Height-1)
					//flag = PX_EMPTY;

				if(!(flag & PX_EMPTY)) {
					ox = x+1; oy = y+1;

					// Draw the shadow
					for(n = 0; n < SHADOW_DROP; n++) {

						// Clipping
						if(ox >= Width) break;
						if(oy >= Height) break;

						if(!( unsafeGetPixelFlag(ox, oy) & PX_EMPTY))
							break;

                        offset = oy*bmpImage.get()->pitch + ox*screenbpp;
                        pixel = (Uint8*)bmpImage.get()->pixels + offset;
                        src = (Uint8*)bmpShadowMap.get()->pixels + offset;
						memcpy(pixel, src, screenbpp);

						ox++; oy++;
					}
				}

				px++;
			}
		}
		UnlockSurface(bmpImage);
	}

	unlockFlags();

	UnlockSurface(bmpShadowMap);

	bMiniMapDirty = true;
}


///////////////////
// Calculate the shadow map
void CMap::CalculateShadowMap()
{
	if(bmpBackImage.get() && bmpShadowMap.get()) {
		// This should be faster
		SDL_BlitSurface(bmpBackImage.get(), NULL, bmpShadowMap.get(), NULL);
		DrawRectFillA(bmpShadowMap.get(),0,0,bmpShadowMap.get()->w,bmpShadowMap.get()->h,tLX->clBlack,96);
	}
}


///////////////////
// Place a stone
void CMap::PlaceStone(int size, CVec pos)
{
	SmartPointer<SDL_Surface> stone;
	short dy, sx,sy;
	short x,y;
	short w,h;

	if(size < 0 || size >= Theme.NumStones) {
		warnings("WARNING: Bad stone size\n");
		size = CLAMP(size, 0, Theme.NumStones - 1);
	}


	// Add the stone to the object list
	if(NumObjects+1 < MAX_OBJECTS) {
		object_t *o = &Objects[NumObjects++];
		o->Type = OBJ_STONE;
		o->Size = size;
		o->X = (int) pos.x;
        o->Y = (int) pos.y;
	}



	// Calculate half
	stone = Theme.bmpStones[size];
	w = stone.get()->w;
	h = stone.get()->h;

	sx = (int)pos.x - (stone.get()->w >> 1);
	sy = (int)pos.y - (stone.get()->h >> 1);

	// Blit the stone to the surface
	DrawImage(bmpImage.get(), stone, sx, sy);

	LOCK_OR_QUIT(stone);

	lockFlags();

	// Calculate the clipping bounds, so we don't have to check each loop then
	short clip_h = MIN(sy+stone.get()->h, bmpImage.get()->h) - sy;
	short clip_w = MIN(sx+stone.get()->w, bmpImage.get()->w) - sx;
	short clip_y = 0;
	short clip_x = 0;
	if (sy<0)
		clip_y = abs(sy);
	if (sx<0)
		clip_x = abs(sx);

	// Pixels
	Uint8 *p = NULL;
	Uint8 *PixelRow = (Uint8*)stone.get()->pixels + clip_y*stone.get()->pitch;
	short pf_tmp = MAX((short)0, sx);
	short p_tmp = clip_x * stone.get()->format->BytesPerPixel;

	// Go through the pixels in the stone and update pixel flags
	for(y = clip_y, dy = MAX((short)0, sy); y < clip_h; y++, dy++, PixelRow += stone.get()->pitch) {

		p = PixelRow+p_tmp;
		uchar* px = &material->line[dy][pf_tmp];

		for(x = clip_x; x < clip_w; x++) {

			// Rock?
			if(!IsTransparent(stone.get(), GetPixelFromAddr(p, stone.get()->format->BytesPerPixel))) {
				*(uchar*)px = Material::indexFromLxFlag(PX_ROCK);
			}

			p += stone.get()->format->BytesPerPixel;
			px++;
		}
	}

	unlockFlags();

	UnlockSurface(stone);

	UpdateArea(sx, sy, w, h);

    // Calculate the total dirt count
    CalculateDirtCount();
}


///////////////////
// Place a miscellaneous item
void CMap::PlaceMisc(int id, CVec pos)
{

	SmartPointer<SDL_Surface> misc;
	short dy,dx, sx,sy;
	short x,y;
	short w,h;

	if(id < 0 || id >= Theme.NumMisc) {
		warnings("Bad misc size\n");
		if(id < 0) id = 0;
		else id = Theme.NumMisc-1;
	}

	// Add the misc to the object list
	if(NumObjects+1 < MAX_OBJECTS) {
		object_t *o = &Objects[NumObjects++];
		o->Type = OBJ_MISC;
		o->Size = id;
		o->X = (int) pos.x;
        o->Y = (int) pos.y;
	}


	// Calculate half
	misc = Theme.bmpMisc[id];
	w = misc.get()->w;
	h = misc.get()->h;

	sx = (int)pos.x-(misc.get()->w>>1);
	sy = (int)pos.y-(misc.get()->h>>1);


	LOCK_OR_QUIT(misc);
	LOCK_OR_QUIT(bmpImage);

	lockFlags();

	// Calculate the clipping bounds, so we don't have to check each loop then
	short clip_h = MIN(sy+misc.get()->h,bmpImage.get()->h)-sy;
	short clip_w = MIN(sx+misc.get()->w,bmpImage.get()->w)-sx;
	short clip_y = 0;
	short clip_x = 0;
	if (sy<0)
		clip_y = -sy;
	if (sx<0)
		clip_x = -sx;

	Uint8 *p = NULL;
	Uint8 *PixelRow = (Uint8 *)misc.get()->pixels+clip_y*misc.get()->pitch;

	// Temps for better performance
	short pf_tmp = MAX((short)0,sx);
	short p_tmp = clip_x*misc.get()->format->BytesPerPixel;
	short dx_tmp = MAX((short)0,sx);

	// Go through the pixels in the misc item
	for(y = clip_y, dy = MAX((short)0, sy); y < clip_h; y++, dy++, PixelRow += misc.get()->pitch) {

		p = PixelRow + p_tmp;
		uchar* px = &material->line[dy][pf_tmp];

		for(x = clip_x, dx = dx_tmp; x < clip_w; dx++, x++) {

			// Put the pixel down
			if(!IsTransparent(misc.get(), GetPixelFromAddr(p, misc.get()->format->BytesPerPixel)) && (m_materialList[*px].toLxFlags() & PX_DIRT)) {
				PutPixel(bmpImage.get(), dx, dy, GetPixelFromAddr(p, misc.get()->format->BytesPerPixel));
				*(uchar*)px = Material::indexFromLxFlag(PX_DIRT);
			}

			p += misc.get()->format->BytesPerPixel;
			px++;
		}
	}

	unlockFlags();

	UnlockSurface(bmpImage);
	UnlockSurface(misc);

	// Update the draw image
	UpdateDrawImage(sx, sy, clip_w, clip_h);

	bMiniMapDirty = true;
}


///////////////////
// Put a pixel onto the front image buffer
// TODO: atm, this isnt used at all; some outcommented usage is in debug parts of AI; so shouldn't we put the pixel on the debug image then?
void CMap::PutImagePixel(uint x, uint y, Color colour)
{
    // Checking edges
	if(x >= Width || y >= Height)
		return;

	LOCK_OR_QUIT(bmpImage)
    PutPixel(bmpImage.get(), x, y, colour.get(bmpImage.get()->format));
	UnlockSurface(bmpImage);

	x *= 2;
	y *= 2;
	DrawRectFill2x2_NoClip(bmpDrawImage.get(), x, y, colour);
}


///////////////////
// Update the minimap
void CMap::UpdateMiniMap(bool force)
{
	if(bDedicated) return;
	if(!bMiniMapDirty && !force) return;

	if(!bmpMiniMap.get()) {
		errors << "CMap::UpdateMiniMap: minimap surface not initialised" << endl;
		return;
	}
	
	if(gusIsLoaded()) {
		gusUpdateMinimap(0, 0, Width, Height);
	}
	else {
		if(!bmpDrawImage.get()) {
			errors << "CMap::UpdateMiniMap: drawimage surface not initialised" << endl;
			return;
		}
		
		if( bmpBackImageHiRes.get() )
		{
			if (tLXOptions->bAntiAliasing)
				DrawImageResampledAdv(bmpMiniMap.get(), bmpDrawImage, 0, 0, 0, 0, bmpDrawImage.get()->w, bmpDrawImage.get()->h, bmpMiniMap->w, bmpMiniMap->h);
			else
				DrawImageResizedAdv(bmpMiniMap.get(), bmpDrawImage, 0, 0, 0, 0, bmpDrawImage.get()->w, bmpDrawImage.get()->h, bmpMiniMap->w, bmpMiniMap->h);
		}
		else
		{
			if (tLXOptions->bAntiAliasing)
				DrawImageResampledAdv(bmpMiniMap.get(), bmpImage, 0, 0, 0, 0, bmpImage.get()->w, bmpImage.get()->h, bmpMiniMap->w, bmpMiniMap->h);
			else
				DrawImageResizedAdv(bmpMiniMap.get(), bmpImage, 0, 0, 0, 0, bmpImage.get()->w, bmpImage.get()->h, bmpMiniMap->w, bmpMiniMap->h);
		}
	}

	// Not dirty anymore
	bMiniMapDirty = false;
	UpdateMiniMapTransparent();
}

///////////////////
// Update an area of the minimap
// X, Y, W and H apply to the bmpImage, not bmpMinimap
void CMap::UpdateMiniMapRect(int x, int y, int w, int h)
{
	if(bDedicated) return;

	// If the minimap is going to be fully repainted, just move on
	if (bMiniMapDirty)
		return;

	if(gusIsLoaded()) {
		gusUpdateMinimap(x, y, w, h);
		return;
	}

	if( bmpBackImageHiRes.get() )
	{
		// Calculate ratios
		const float xratio = (float)bmpMiniMap.get()->w / (float)bmpDrawImage.get()->w;
		const float yratio = (float)bmpMiniMap.get()->h / (float)bmpDrawImage.get()->h;

		const int dx = (int)((float)x * xratio);
		const int dy = (int)((float)y * yratio);

		if (tLXOptions->bAntiAliasing)
			DrawImageResampledAdv(bmpMiniMap.get(), bmpDrawImage, x - 1, y - 1, dx, dy, w + 1, h + 1, xratio, yratio);
		else
			DrawImageResizedAdv(bmpMiniMap.get(), bmpDrawImage, x - 1, y - 1, dx, dy, w + 1, h + 1, xratio, yratio);
	}
	else
	{
		// Calculate ratios
		const float xratio = (float)bmpMiniMap.get()->w / (float)bmpImage.get()->w;
		const float yratio = (float)bmpMiniMap.get()->h / (float)bmpImage.get()->h;

		const int dx = (int)((float)x * xratio);
		const int dy = (int)((float)y * yratio);

		if (tLXOptions->bAntiAliasing)
			DrawImageResampledAdv(bmpMiniMap.get(), bmpImage, x - 1, y - 1, dx, dy, w + 1, h + 1, xratio, yratio);
		else
			DrawImageResizedAdv(bmpMiniMap.get(), bmpImage, x - 1, y - 1, dx, dy, w + 1, h + 1, xratio, yratio);
	}
	UpdateMiniMapTransparent();
}

void CMap::UpdateMiniMapTransparent()
{
	if( !( game.gameScript() && game.gameScript()->gusEngineUsed() ) )
		return;
	if( bmpMiniMapTransparent.get() == NULL || 
		bmpMiniMapTransparent.get()->w != bmpMiniMap.get()->w || 
		bmpMiniMapTransparent.get()->h != bmpMiniMap.get()->h )
	{
		bmpMiniMapTransparent = gfxCreateSurface(bmpMiniMap.get()->w, bmpMiniMap.get()->h);
		SetPerSurfaceAlpha(bmpMiniMapTransparent.get(), 128); // 128 should be optimised in SDL
		FillSurface(bmpMiniMapTransparent.get(), MakeColour(0, 0, 0, 128)); // Enable per-pixel alpha
	}
	SetPerSurfaceAlpha(bmpMiniMap.get(), 255); // Make it not overwrite per-pixel alpha
	DrawImage(bmpMiniMapTransparent.get(), bmpMiniMap, 0, 0);
	//SetPerSurfaceAlpha(bmpMiniMapTransparent.get(), 128); // Just in case
}

void CMap::drawOnMiniMap(SDL_Surface* bmpDest, uint miniX, uint miniY, const CVec& pos, Uint8 r, Uint8 g, Uint8 b, bool big, bool special) {
	if(bDedicated) return;
	
	float xstep,ystep;
	float mx,my;
	int mw = bmpMiniMap.get()->w;
	int mh = bmpMiniMap.get()->h;
	
	// Calculate steps
	xstep = (float)Width/ (float)mw;
	ystep = (float)Height / (float)mh;
	
	byte dr,dg,db;
	dr = ~r;
	dg = ~g;
	db = ~b;
	
	r += (int)( (float)dr*(fBlinkTime.seconds()*2.0f));
	g += (int)( (float)dg*(fBlinkTime.seconds()*2.0f));
	b += (int)( (float)db*(fBlinkTime.seconds()*2.0f));
	
	
	
	mx = pos.x/xstep;
	my = pos.y/ystep;
	mx = (float)floor(mx);
	my = (float)floor(my);
	
	mx = MIN(mw-(float)1,mx); mx = MAX((float)0,mx);
	my = MIN(mh-(float)1,my); my = MAX((float)0,my);
	int i=(int)mx + miniX;
	int j=(int)my + miniY;
	// Snap it to the nearest 2nd pixel (prevent 'jumping')
	//x -= x % 2;
	//y -= y % 2;
	
	Color col = Color(r,g,b);
	
	{
		int x = MAX((int)miniX, i-1);
		int y = MAX((int)miniY, j-1);
		
		if(!big && !special)
			DrawRectFill2x2(bmpDest, x, y, col);
		else if(!big && special) {
			if(fBlinkTime > 0.25f) {
				DrawLine(bmpDest, x-1, y-1, x+1, y+1, col);
				DrawLine(bmpDest, x-1, y+1, x+1, y-1, col);				
			} else
				DrawRectFill2x2(bmpDest, x, y, col);
		}			
		else if(big && !special)
			DrawRectFill(bmpDest, x, y, i+2, j+2, col);
		else if(big && special) {
			if(fBlinkTime > 0.25f) {
				DrawLine(bmpDest, x-1, y-1, i+2, j+2, col);
				DrawLine(bmpDest, x-1, j+2, i+2, y-1, col);
			} else {
				DrawRectFill(bmpDest, x, y, i+2, j+2, col);				
			}
		}
	}
}

///////////////////
// Draw & Simulate the minimap
void CMap::DrawMiniMap(SDL_Surface * bmpDest, uint x, uint y, TimeDiff dt, CWorm *worms)
{
	if(worms == NULL || bmpMiniMap.get() == NULL)
		return;

	// Update the minimap (only if dirty)
	if(bMiniMapDirty)
		UpdateMiniMap();

	// Draw the minimap
	if( game.gameScript() && game.gameScript()->gusEngineUsed() )
		DrawImage(bmpDest, bmpMiniMapTransparent, x, y);
	else
		DrawImage(bmpDest, bmpMiniMap, x, y);

	SetPerSurfaceAlpha(bmpMiniMap.get(), 255);

	fBlinkTime+=dt;
	if(fBlinkTime>0.5f)
		fBlinkTime=TimeDiff();
	

	// Show worms
	CWorm *w = worms;
	for(int n=0;n<MAX_WORMS;n++,w++) {
		if(!w->getAlive() || !w->isUsed() || !cClient->isWormVisibleOnAnyViewport(n))
			continue;

		Color gameCol = w->getGameColour();

		// Our worms are bigger
		bool big = false;
		
		// Tagged worms or local players are bigger, depending on the game type
		if(cClient->getGeneralGameType() != GMT_TIME) {
			big = (w->getType() == PRF_HUMAN && w->getLocal());
		} else {
			big = w->getTagIT()!=0;
		}
		
		drawOnMiniMap(bmpDest, x, y, w->getPos(), gameCol.r, gameCol.g, gameCol.b, big, false);
	}
	
	if(cClient && cClient->getGameReady()) {
		cClient->flagInfo()->drawOnMiniMap(this, bmpDest, x, y);
	}
}

///////////////////
// Load the map
bool CMap::Load(const std::string& filename)
{
	// TODO: remove that as soon as we do the map loading in a seperate thread
	ScopedBackgroundLoadingAni backgroundLoadingAni(320, 280, 50, 50, Color(128,128,128), Color(64,64,64));
	
	// Weird
	if (filename == "") {
		warnings("WARNING: loading unnamed map, ignoring ...\n");
		return true;
	}
	
	// Already loaded?
	if (FileName == filename && Created)  {
		notes("HINT: map " + filename + " is already loaded\n");
		return true;
	}
	
	
	FileName = filename;
	
	// try loading a previously cached map
	if(LoadFromCache(FileName)) {
		// everything is ok
		notes << "reusing cached map for " << filename << endl;
		return true;
	}
	
	MapLoad* loader = MapLoad::open(filename);
	if(!loader) {
		warnings << "level " << filename << " couldn't be loaded" << endl;
		return false;
	}
	
	{
		bool res = loader->parseData(this);
		delete loader;
		if(!res) {
			warnings << "level loader for " << filename << " returned with error" << endl;
			return false;
		}
		
		if(!material) {
			errors << "level loader for " << filename << " is behaving wrong" << endl;
			return false;
		}
	}
	
	bMiniMapDirty = true;
	Created = true;
    //sRandomLayout.bUsed = false;
	
	if(!gusIsLoaded()) {
		// Calculate the shadowmap
		CalculateShadowMap();
		
		// Apply the shadow
		ApplyShadow(0, 0, Width, Height);

		// Update the draw image
		UpdateDrawImage(0, 0, bmpImage->w, bmpImage->h);
	}
	
    // Calculate the total dirt count
    CalculateDirtCount();

	// Save the map to cache
	SaveToCache();

	return true;
}


///////////////////
// Save the map
bool CMap::Save(const std::string& name, const std::string& filename)
{
	FILE *fp = OpenGameFile(filename,"wb");
	if(fp == NULL)
		return false;

	Type = MPT_IMAGE;

	// Header
	int		version = MAP_VERSION;

	fwrite("LieroX Level",	32,	fp);
	fwrite_endian_compat((version),	sizeof(int),	1,	fp);
	fwrite(name,	64,	fp);
	fwrite_endian_compat((Width),		sizeof(int),	1,	fp);
	fwrite_endian_compat((Height),		sizeof(int),	1,	fp);
	fwrite_endian_compat((Type),		sizeof(int),	1,	fp);
	fwrite(Theme.name,	32,	fp);
	fwrite_endian_compat((NumObjects),	sizeof(int),	1,	fp);


	// Save the images if in an image format
	if(Type == MPT_IMAGE)
		return SaveImageFormat(fp);

	lockFlags();

	// Dirt map
	for(int y = 0; (size_t)y < Height; ++y) for(int x = 0; (size_t)x < Width; ++x) {
		uchar t = 0;

		// 1 bit == 1 pixel with a yes/no dirt flag
		for(ushort i=0;i<8;i++) {
			uchar value = (unsafeGetPixelFlag(x,y) & PX_EMPTY);
			t |= (value << i);
		}

		fwrite(&t,	sizeof(uchar),	1,	fp);
	}

	unlockFlags();

	// Objects
	object_t *o = Objects;
	for(int i=0;i<NumObjects;i++,o++) {
		fwrite_endian_compat((o->Type),sizeof(int),	1,	fp);
		fwrite_endian_compat((o->Size),sizeof(int),	1,	fp);
		fwrite_endian_compat((o->X),	sizeof(int),	1,	fp);
        fwrite_endian_compat((o->Y),	sizeof(int),	1,	fp);
	}

	fclose(fp);

	return true;
}


///////////////////
// Save the images
bool CMap::SaveImageFormat(FILE *fp)
{
	uint x,y,p;
	Uint8 r,g,b,a;

	// The images are saved in a raw 24bit format.
	// 8 bits per r,g,b channel

	if( !bmpBackImage.get() || !bmpImage.get() || !material )
		return false;

	// Write out the images & pixeflags to memory, compress the data & save the compressed data
	Uint32 size = (Width*Height * 3) * 2 + (Width*Height) + 1;
	Uint32 destsize = size + (size / 8) + 12;

	uchar *pSource = new uchar[size];
	uchar *pDest = new uchar[destsize];

	if(!pSource || !pDest) {
		errors("CMap::SaveImageFormat: ERROR: not enough memory for pSource and pDest\n");
		fclose(fp);
		return false;
	}


	// Save the back image
	p=0;
	LOCK_OR_FAIL(bmpBackImage);
	for(y=0; y<Height; y++) {
		for(x=0; x<Width; x++) {
			GetColour4( GetPixel(bmpBackImage.get(),x,y), bmpBackImage.get()->format, &r, &g, &b ,&a );

			pSource[p++] = r;
			pSource[p++] = g;
			pSource[p++] = b;
		}
	}
	UnlockSurface(bmpBackImage);


	// Save the front image
	LOCK_OR_FAIL(bmpImage);
	for(y=0; y<Height; y++) {
		for(x=0; x<Width; x++) {
			GetColour4( GetPixel(bmpImage.get(),x,y), bmpImage.get()->format, &r, &g, &b ,&a );
			pSource[p++] = r;
			pSource[p++] = g;
			pSource[p++] = b;
		}
	}
	UnlockSurface(bmpImage);

	// Save the pixel flags
	lockFlags();
	for(int y = 0; (size_t)y < Height; ++y) for(int x = 0; (size_t)x < Width; ++x) {
		uchar t = PX_EMPTY;
		if(unsafeGetPixelFlag(x,y) & PX_DIRT) t = PX_DIRT;
		if(unsafeGetPixelFlag(x,y) & PX_ROCK) t = PX_ROCK;

		pSource[p++] = t;
	}
	unlockFlags();

	// Compress it
	ulong lng_dsize = destsize;
	if( compress( pDest, &lng_dsize, pSource, size) != Z_OK ) {
		errors("Failed compressing\n");
		fclose(fp);
		delete[] pSource;
		delete[] pDest;
		return false;
	}
	destsize = lng_dsize; // WARNING: possible overflow ; TODO: do a check for it?

	// Write out the details & the data
	fwrite_endian_compat((destsize), sizeof(Uint32), 1, fp);
	fwrite_endian_compat((size), sizeof(Uint32), 1, fp);
	fwrite(pDest, sizeof(uchar), destsize, fp);

	delete[] pSource;
	delete[] pDest;

	// TODO: save hi-res image here
	
	fclose(fp);
	return true;
}


///////////////////
// DEBUG: Draw the pixel flags
void CMap::DEBUG_DrawPixelFlags(int x, int y, int w, int h)
{

    /*for(y=0;y<Height;y+=30) {
		for(x=0;x<Width;x+=30) {
            CarveHole(5,CVec(x,y));

        }
    }*/


/*	n=0;
	for(y=0;y<Height;y++) {
		for(x=0;x<Width;x++) {

			switch(PixelFlags[n]) { //NOOO

				case PX_EMPTY:	PutPixel(bmpImage,x,y,tLX->clBlack);	break;
				case PX_DIRT:	PutPixel(bmpImage,x,y,MakeColour(255,0,0));	break;
				case PX_ROCK:	PutPixel(bmpImage,x,y,MakeColour(128,128,128));	break;
			}
			n++;
		}
	}*/
	/*SDL_Rect oldrect = bmpDrawImage->clip_rect;
	SDL_Rect newrect = { x * 2, y * 2, w * 2, h * 2 };
	SDL_SetClipRect(bmpDrawImage.get(), &newrect);*/

	//SDL_SetClipRect(bmpDrawImage.get(), &oldrect);
}


void CMap::NewNet_SaveToMemory()
{
	if( bMapSavingToMemory )
	{
		errors("Error: calling CMap::SaveToMemory() twice\n");
		return;
	}
	bMapSavingToMemory = true;
	if( bmpSavedImage.get() == NULL )
	{
		if( bmpBackImageHiRes.get() )
			bmpSavedImage = gfxCreateSurface(Width*2, Height*2);
		else
			bmpSavedImage = gfxCreateSurface(Width, Height);
			
		if( bmpSavedImage.get() == NULL )
		{
			errors("Error: CMap::SaveToMemory(): cannot allocate GFX surface\n");
			return;
		}
		savedPixelFlags = new uchar[Width * Height];
	}
	
	savedMapCoords.clear();
}

void CMap::NewNet_RestoreFromMemory()
{
	if( ! bMapSavingToMemory )
	{
		errors("Error: calling CMap::RestoreFromMemory() twice\n");
		return;
	}
	if( bmpSavedImage.get() == NULL || savedPixelFlags == NULL )
	{
		errors("Error: CMap::RestoreFromMemory(): bmpSavedImage is NULL\n");
		return;
	}
	
	for( std::set< SavedMapCoord_t > :: iterator it = savedMapCoords.begin();
			it != savedMapCoords.end(); it++ )
	{
		int startX = it->X*MAP_SAVE_CHUNK;
		int sizeX = MIN( MAP_SAVE_CHUNK, Width - startX );
		int startY = it->Y*MAP_SAVE_CHUNK;
		int sizeY = MIN( MAP_SAVE_CHUNK, Height - startY  );

		LOCK_OR_QUIT(bmpSavedImage);
		lockFlags();
	
		if( bmpBackImageHiRes.get() )
		{
			LOCK_OR_QUIT(bmpDrawImage);
			DrawImageAdv( bmpDrawImage.get(), bmpSavedImage, startX*2, startY*2, startX*2, startY*2, sizeX*2, sizeY*2 );
			UnlockSurface(bmpDrawImage);
		}
		else
		{
			LOCK_OR_QUIT(bmpImage);
			DrawImageAdv( bmpImage.get(), bmpSavedImage, startX, startY, startX, startY, sizeX, sizeY );
			UnlockSurface(bmpImage);
		}

		for( int y=startY; y<startY+sizeY; y++ )
			memcpy( (char*)material->surf->pixels + y*material->surf->pitch + startX, savedPixelFlags + y*material->surf->pitch + startX, sizeX*sizeof(uchar) );
	
		unlockFlags();
		UnlockSurface(bmpSavedImage);
		
		if( tLXOptions->bShadows )
		{
			UpdateArea(startX, startY, sizeX, sizeY, true);
		}
		else
		{
			UpdateDrawImage(startX, startY, sizeX, sizeY);
			UpdateMiniMapRect(startX-10, startY-10, sizeX+20, sizeY+20);
		}
	}

	bMapSavingToMemory = false;
	savedMapCoords.clear();
}

void CMap::NewNet_Deinit()
{
		bMapSavingToMemory = false;
		bmpSavedImage = NULL;
		if( savedPixelFlags )
			delete[] savedPixelFlags;
		savedPixelFlags = NULL;
		savedMapCoords.clear();
}

void CMap::SaveToMemoryInternal(int x, int y, int w, int h)
{
	if( ! bMapSavingToMemory )
		return;
	if( bmpSavedImage.get() == NULL || savedPixelFlags == NULL )
	{
		errors("Error: CMap::SaveToMemoryInternal(): bmpSavedImage is NULL\n");
		return;
	}

	int gridX = x / MAP_SAVE_CHUNK;
	int gridMaxX = 1 + (x+w) / MAP_SAVE_CHUNK;
	
	int gridY = y / MAP_SAVE_CHUNK;
	int gridMaxY = 1 + (y+h) / MAP_SAVE_CHUNK;

	for( int fy = gridY; fy < gridMaxY; fy++ )
		for( int fx = gridX; fx < gridMaxX; fx++ )
			if( savedMapCoords.count( SavedMapCoord_t( fx, fy ) ) == 0 )
			{
				savedMapCoords.insert( SavedMapCoord_t( fx, fy ) );
				
				int startX = fx*MAP_SAVE_CHUNK;
				int sizeX = MIN( MAP_SAVE_CHUNK, Width - startX );
				int startY = fy*MAP_SAVE_CHUNK;
				int sizeY = MIN( MAP_SAVE_CHUNK, Height - startY  );

				LOCK_OR_QUIT(bmpSavedImage);
				lockFlags();
				
				if( bmpBackImageHiRes.get() )
				{
					LOCK_OR_QUIT(bmpDrawImage);
					DrawImageAdv( bmpSavedImage.get(), bmpDrawImage, startX*2, startY*2, startX*2, startY*2, sizeX*2, sizeY*2 );
					UnlockSurface(bmpDrawImage);
				}
				else
				{
					LOCK_OR_QUIT(bmpImage);
					DrawImageAdv( bmpSavedImage.get(), bmpImage, startX, startY, startX, startY, sizeX, sizeY );
					UnlockSurface(bmpImage);
				}

				for( int y=startY; y<startY+sizeY; y++ )
					memcpy( savedPixelFlags + y*material->surf->pitch + startX, (char*)material->surf->pixels + y*material->surf->pitch + startX, sizeX*sizeof(uchar) );

				unlockFlags();
				UnlockSurface(bmpSavedImage);
			}
}


///////////////////
// Shutdown the map
void CMap::Shutdown()
{
	lockFlags();

	if(Created) {

		//notes << "Some created map is shutting down..." << endl;

		bmpImage = NULL;
		bmpDrawImage = NULL;
#ifdef _AI_DEBUG
		bmpDebugImage = NULL;
#endif
		bmpBackImage = NULL;
		bmpShadowMap = NULL;
		bmpMiniMap = NULL;
		bmpBackImageHiRes = NULL;

		if(Objects)
			delete[] Objects;
		Objects = NULL;
		NumObjects = 0;
		AdditionalData.clear();

		bMapSavingToMemory = false;
		bmpSavedImage = NULL;
		if( savedPixelFlags )
			delete[] savedPixelFlags;
		savedPixelFlags = NULL;
		savedMapCoords.clear();
	}
	// Safety
	else  {
		bmpImage = NULL;
		bmpDrawImage = NULL;
#ifdef _AI_DEBUG
		bmpDebugImage = NULL;
#endif
		bmpBackImage = NULL;
		bmpBackImageHiRes = NULL;
		bmpShadowMap = NULL;
		bmpMiniMap = NULL;
		Objects = NULL;
		AdditionalData.clear();
		bMapSavingToMemory = false;
		bmpSavedImage = NULL;
		savedPixelFlags = NULL;
		savedMapCoords.clear();
	}

	gusShutdown();
	Created = false;
	FileName = "";

	unlockFlags();
}


#ifdef _AI_DEBUG
void CMap::ClearDebugImage() {
	if (bmpDebugImage.get()) {
		FillSurfaceTransparent(bmpDebugImage.get());
	}
}
#endif

///////////////////
// Carve a hole
// Returns the number of dirt pixels carved
// TODO: why do we have this function? why not CMap::CarveHole?
int CarveHole(CVec pos)
{
	if(!cClient->getMap()) {
		errors << "CarveHole: client map not loaded" << endl;
		return 0;
	}
	
	int x,y;

	// Go through until we find dirt to throw around
	y = MAX(MIN((int)pos.y, (int)cClient->getMap()->GetHeight() - 1), 0);

	for(x=(int)pos.x-2; x<=(int)pos.x+2; x++) {
		// Clipping
		if(x < 0) continue;
		if((uint)x >= cClient->getMap()->GetWidth())	break;

		if(cClient->getMap()->GetPixelFlag(x,y) & PX_DIRT) {
			Color col = cClient->getMap()->getColorAt(x, y);
			for(short n=0; n<3; n++)
				SpawnEntity(ENT_PARTICLE,0,pos,CVec(GetRandomNum()*30,GetRandomNum()*10),col,NULL);
			break;
		}
	}

	// Just carve a hole for the moment
	return cClient->getMap()->CarveHole(3,pos,cClient->getGameLobby()->features[FT_InfiniteMap]);
}





/*
===========================

    Collision Detection

===========================
*/

class set_col_and_break {
public:
	CVec collision;
	bool hit;

	set_col_and_break() : hit(false) {}
	bool operator()(int x, int y) {
		hit = true;
		collision.x = (float)x;
		collision.y = (float)y;
		return false;
	}
};



///////////////////
// Check for a collision
// HINT: this function is not used at the moment; and it is incomplete...
int CheckCollision(float dt, CVec pos, CVec vel, uchar checkflags)
{
/*	set_col_and_break col_action;
	fastTraceLine(trg, pos, map, checkflags, col_action);
	if(col_action.hit) {

	}*/
	assert(false);
	return 0;

/*	int		CollisionSide = 0;
	int		mw = map->GetWidth();
	int		mh = map->GetHeight();
	int		px,py, x,y, w,h;
	int		top,bottom,left,right;

	px=(int)pos.x;
	py=(int)pos.y;

	top=bottom=left=right=0;

	w = width;
	h = height;

	// Hit edges
	// Check the collision side
	if(px-w<0)
		CollisionSide |= COL_LEFT;
	if(py-h<0)
		CollisionSide |= COL_TOP;
	if(px+w>=mw)
		CollisionSide |= COL_RIGHT;
	if(py+h>=mh)
		CollisionSide |= COL_BOTTOM;
	if(CollisionSide) return CollisionSide;




	for(y=py-h;y<=py+h;y++) {

		// Clipping means that it has collided
		if(y<0)	{
			CollisionSide |= COL_TOP;
			return CollisionSide;
		}
		if(y>=mh) {
			CollisionSide |= COL_BOTTOM;
			return CollisionSide;
		}


		const uchar *pf = map->GetPixelFlags() + y*mw + px-w;

		for(x=px-w;x<=px+w;x++) {

			// Clipping
			if(x<0) {
				CollisionSide |= COL_LEFT;
				return CollisionSide;
			}
			if(x>=mw) {
				CollisionSide |= COL_RIGHT;
				return CollisionSide;
			}

			if(*pf & PX_DIRT || *pf & PX_ROCK) {
				if(y<py)
					top++;
				if(y>py)
					bottom++;
				if(x<px)
					left++;
				if(x>px)
					right++;

				//return Collision(*pf);
			}

			pf++;
		}
	}

	// Check for a collision
	if(top || bottom || left || right) {
		CollisionSide = 0;


		// Find the collision side
		if( (left>right || left>2) && left>1 && vel.x < 0)
			CollisionSide = COL_LEFT;

		if( (right>left || right>2) && right>1 && vel.x > 0)
			CollisionSide = COL_RIGHT;

		if(top>1 && vel.y < 0)
			CollisionSide = COL_TOP;

		if(bottom>1 && vel.y > 0)
			CollisionSide = COL_BOTTOM;
	}


	return CollisionSide; */
}

template <> void SmartPointer_ObjectDeinit<CMap> ( CMap * obj )
{
	delete obj;
}



///////////////////
// Get the level name from specified file
std::string CMap::GetLevelName(const std::string& filename, bool abs_filename)
{
	LevelInfo info = infoForLevel(filename, abs_filename);
	if(info.valid) return info.name;
	return "";
}



Color CMap::getColorAt(long x, long y) {
	if(gusIsLoaded()) {
		return Color(getpixel(image, x, y));
	}
	else if(bmpImage.get()) {
		if(!LockSurface(bmpImage)) return Color();
		return Color(bmpImage->format, GetPixel(bmpImage.get(), x, y));
		UnlockSurface(bmpImage);		
	}
	return Color();
}

void CMap::putColorTo(long x, long y, Color c) {
	if(gusIsLoaded()) {
		return putpixel(image, x, y, c.get());
	}
	else if(bmpImage.get()) {
		if(!LockSurface(bmpImage)) return;
		PutPixel(bmpImage.get(),x, y, c.get(bmpImage->format));
		UnlockSurface(bmpImage);		

		DrawRectFill2x2(GetDrawImage().get(), x*2, y*2, c);
	}
}

void CMap::putSurfaceTo(long x, long y, SDL_Surface* surf, int sx, int sy, int sw, int sh) {
	if(gusIsLoaded()) {
		DrawImageAdv(image->surf.get(), surf, sx, sy, x, y, sw, sh);
	}
	else if(bmpImage.get()) {
		DrawImageAdv(GetImage().get(), surf, sx, sy, x, y, sw, sh);
		DrawImageStretch2(GetDrawImage().get(), GetImage(), x, y, x*2, y*2, sw, sh);
	}
}





///////////////////
// Find a spot with no rock
CVec CMap::FindSpot()
{
	CVec pos;
	PixelFlagAccess flags(this);
	
	size_t tries = 1000;
	do {
		pos.x = (float)rnd() * Width;
		pos.y = (float)rnd() * Height;
		tries--;
	} while ( tries > 0 && !IsGoodSpawnPoint(flags, pos) );
	
	if(tries == 0) errors << "FindSpot(): strange error!" << endl;
	return pos;
}



CVec CMap::FindSpotCloseToPos(const std::list<CVec>& goodPos, const std::list<CVec>& badPos, bool keepDistanceToBad) {
	// TODO: optimise this!
	
	float team_dist = -9999999.0f;
	CVec pos = FindSpot();
	CVec pos1;
	
	for( int k=0; k<100; k++ )
	{
		float team_dist1 = 0;
		pos1 = FindSpot();
		for(std::list<CVec>::const_iterator i = goodPos.begin(); i != goodPos.end(); ++i)
			team_dist1 -= ( pos1 - *i ).GetLength() / (goodPos.size() * 10.0f);
		for(std::list<CVec>::const_iterator i = badPos.begin(); i != badPos.end(); ++i) {
			if(keepDistanceToBad)
				team_dist1 += 2.0f * ( ( pos1 - *i ).GetLength() ) / badPos.size();
			else
				// sqrt will make sure there's no large dist between team1 and 2 and short dist between 2 and 3
				// The sum will get considerably smaller if any two teams are on short dist
				team_dist1 += sqrt( ( pos1 - *i ).GetLength() ) / badPos.size();			
		}
		
		if( team_dist1 > team_dist )
		{
			team_dist = team_dist1;
			pos = pos1;
		}
	}
	
	return pos;	
}

CVec CMap::FindSpotCloseToTeam(int t, CWorm* exceptionWorm, bool keepDistanceToEnemy) {
	std::list<CVec> goodPos;
	std::list<CVec> badPos;
	std::vector<bool> coveredTeam(4, false);
	
	CWorm * w = cClient->getRemoteWorms();
	for(int i = 0; i < MAX_WORMS; i++, w++) {
		if( !w->isUsed() || w->getLives() == WRM_OUT || !w->getWeaponsReady() || !w->getAlive())
			continue;
		if(exceptionWorm && exceptionWorm->getID() == w->getID())
			continue;
		if(w->getTeam() < 0 || (size_t)w->getTeam() >= coveredTeam.size())
			continue;
		if(coveredTeam[w->getTeam()])
			continue;
		
		coveredTeam[w->getTeam()] = true;
		if(w->getTeam() == t)
			goodPos.push_back(w->getPos());
		else
			badPos.push_back(w->getPos());
	}
	
	return FindSpotCloseToPos(goodPos, badPos, keepDistanceToEnemy);
}


