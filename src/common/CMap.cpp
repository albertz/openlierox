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

#include <assert.h>
#include <zlib.h>
#include <list>

#include "CMap.h"
#include "EndianSwap.h"
#include "MathLib.h"
#include "Error.h"
#include "ConfigHandler.h"
#include "LieroX.h"
#include "GfxPrimitives.h"
#include "FindFile.h"
#include "StringUtils.h"
#include "InputEvents.h"
#include "CWorm.h"
#include "Entity.h"


///////////////////
// Create a new map
int CMap::New(uint _width, uint _height, const std::string& _theme, uint _minimap_w, uint _minimap_h)
{
	if(Created)
		Shutdown();

	Width = _width;
	Height = _height;
	MinimapWidth = _minimap_w;
	MinimapHeight = _minimap_h;
	NumObjects = 0;
    nTotalDirtCount = 0;
    sRandomLayout.bUsed = false;

	Objects = new object_t[MAX_OBJECTS];
	if(Objects == NULL)
	{
		printf("CMap::New:: ERROR: cannot create object array\n");
		return false;
	}

	// Load the tiles
	if(!LoadTheme(_theme))
	{
		printf("CMap::New:: ERROR: cannot create titles/theme\n");
		return false;
	}

	// Create the surface
	if(!CreateSurface())
	{
		printf("CMap::New:: ERROR: cannot create surface\n");
		return false;
	}

	// Create the pixel flags
	if(!CreatePixelFlags())
	{
		printf("CMap::New:: ERROR: cannot create pixel flags\n");
		return false;
	}

    // Create the AI Grid
    if(!createGrid())
	{
		printf("CMap::New:: ERROR: cannot create AI grid\n");
		return false;
	}

	// Place default tiles
	TileMap();

	// Update the mini map
	UpdateMiniMap();

    // Calculate the total dirt count
    CalculateDirtCount();

    // Calculate the grid
    calculateGrid();

	Created = true;



	//---------------------
	// Water test
	//---------------------


	// TODO: valgrind says, this two arrays got lost
	m_pnWater1 = new int[Width];
	m_pnWater2 = new int[Width];

	memset(m_pnWater1, 0, sizeof(int) * Width);
	memset(m_pnWater2, 0, sizeof(int) * Width);



	return true;
}


///////////////////
// Clear the map
void CMap::Clear(void)
{
	TileMap();
}


///////////////////
// Apply a random set to the map
void CMap::ApplyRandom(void)
{
	int n, x, y, i;
    int objCount = 0;

    int nNumRocks = 15;
    int nNumMisc = 30;
    int nNumHoles = 10;
    int nNumHoleGroup = 20;

    // Setup the random layout
    sRandomLayout.bUsed = true;
    sRandomLayout.nNumObjects = nNumRocks + nNumMisc + (nNumHoles * nNumHoleGroup);
    sRandomLayout.szTheme = Theme.name;

    sRandomLayout.psObjects = new object_t[sRandomLayout.nNumObjects];
    if( !sRandomLayout.psObjects )  {
        printf("Warning: Out of memory for CMap::ApplyRandom()\n");
		return;
	}

	// Spawn 15 random rocks
	for(n = 0; n < nNumRocks; n++) {
		x = (int)(fabs(GetRandomNum()) * (float)Width);
		y = (int)(fabs(GetRandomNum()) * (float)Height);
		i = (int)(fabs(GetRandomNum()) * (float)Theme.NumStones);

        // Store the object
        sRandomLayout.psObjects[objCount].Type = OBJ_STONE;
        sRandomLayout.psObjects[objCount].Size = i;
        sRandomLayout.psObjects[objCount].X = x;
        sRandomLayout.psObjects[objCount].Y = y;
        objCount++;

		PlaceStone(i, CVec( (float)x, (float)y ));
	}

	// Spawn 20 random misc
	for(n = 0; n < nNumMisc; n++) {
		x = (int)(fabs(GetRandomNum()) * (float)Width);
		y = (int)(fabs(GetRandomNum()) * (float)Height);
		i = (int)(fabs(GetRandomNum()) * (float)Theme.NumMisc);

        // Store the object
        sRandomLayout.psObjects[objCount].Type = OBJ_MISC;
        sRandomLayout.psObjects[objCount].Size = i;
        sRandomLayout.psObjects[objCount].X = x;
        sRandomLayout.psObjects[objCount].Y = y;
        objCount++;

		PlaceMisc(i, CVec( (float)x, (float)y ));
	}


	// Spawn 10 random holes
	for(n = 0; n < nNumHoles; n++) {
		x = (int)(fabs(GetRandomNum()) * (float)Width);
		y = (int)(fabs(GetRandomNum()) * (float)Height);

		// Spawn a hole group of holes around this hole
		for(int i = 0; i < nNumHoleGroup; i++) {
			int a = (int) (GetRandomNum() * (float)15);
			int b = (int) (GetRandomNum() * (float)15);

            // Store the object
            sRandomLayout.psObjects[objCount].Type = OBJ_HOLE;
            sRandomLayout.psObjects[objCount].Size = 4;
            sRandomLayout.psObjects[objCount].X = x + a;
            sRandomLayout.psObjects[objCount].Y = y + b;
            objCount++;

			CarveHole(4, CVec( (float)(x + a), (float)(y + b) ));
		}
	}

    // Calculate the total dirt count
    CalculateDirtCount();

	// No need to update the minimap, it's updated in CarveHole and PlaceXXX
}


///////////////////
// Apply a random set
void CMap::ApplyRandomLayout(maprandom_t *psRandom)
{
    assert(psRandom);

    // Load the theme
    LoadTheme(psRandom->szTheme);

    // Tile the map
    TileMap();

    // Apply the objects
	if (psRandom->psObjects)  {
		for( int i = 0; i < psRandom->nNumObjects; i++ ) {
			if( psRandom->psObjects ) {

				switch(psRandom->psObjects[i].Type) {

					// Stone
					case OBJ_STONE:
						PlaceStone(psRandom->psObjects[i].Size,
							CVec(
								(float)(psRandom->psObjects[i].X),
								(float)(psRandom->psObjects[i].Y)));
						break;

					// Misc
					case OBJ_MISC:
						PlaceMisc(psRandom->psObjects[i].Size,
							CVec(
								(float)(psRandom->psObjects[i].X),
								(float)(psRandom->psObjects[i].Y)));
						break;

					// Hole
					case OBJ_HOLE:
						CarveHole(psRandom->psObjects[i].Size,
							CVec(
								(float)(psRandom->psObjects[i].X),
								(float)(psRandom->psObjects[i].Y)));
						break;
				}
			}
		}
	}

    // Calculate the total dirt count
    CalculateDirtCount();

	//CalculateShadowMap();
	UpdateMiniMap(true);
}


///////////////////
// Load the theme
int CMap::LoadTheme(const std::string& _theme)
{
	// Already loaded
	if (Theme.name == _theme && sRandomLayout.szTheme == _theme)
		return true;

	static std::string thm,buf,cfg;
	int n,x,y;

	thm = "data/themes/" + _theme;

	Theme.name = _theme;
    sRandomLayout.szTheme = _theme;

	buf = thm + "/Backtile.png";
	LOAD_IMAGE(Theme.bmpBacktile,buf);
	buf = thm + "/Fronttile.png";
	LOAD_IMAGE(Theme.bmpFronttile,buf);


	// Stones
	cfg = thm + "/theme.txt";
	ReadInteger(cfg,"General","NumStones",&Theme.NumStones,0);

	for(n=0;n<Theme.NumStones;n++) {
		buf = thm + "/Stone" + itoa(n+1) + ".png";
		LOAD_IMAGE(Theme.bmpStones[n],buf);
		SetColorKey(Theme.bmpStones[n]);
	}


	// Holes
	for(n=0;n<5;n++) {
		buf = thm + "/Hole" + itoa(n+1) + ".png";
		LOAD_IMAGE(Theme.bmpHoles[n],buf);
		SDL_SetColorKey(Theme.bmpHoles[n], SDL_SRCCOLORKEY, tLX->clBlack);
	}

	// Calculate the default colour from a non-pink, non-black colour in the hole image
	Theme.iDefaultColour = GetPixel(Theme.bmpFronttile,0,0);
	SDL_Surface *hole = Theme.bmpHoles[0];
	Uint32 pixel = 0;
	if(hole) {
		for(y=0; y<hole->h; y++) {
			for(x=0; x<hole->w; x++) {
				pixel = GetPixel(hole,x,y);
				if(pixel != tLX->clBlack && pixel != tLX->clPink)  {
					Theme.iDefaultColour = pixel;
					break;
				}
			}
		}
	}


	// Misc
	cfg = thm + "/theme.txt";
	ReadInteger(cfg,"General","NumMisc",&Theme.NumMisc,0);
	for(n=0;n<Theme.NumMisc;n++) {
		buf = thm + "/misc" + itoa(n+1) + ".png";
		LOAD_IMAGE(Theme.bmpMisc[n],buf);
		SetColorKey(Theme.bmpMisc[n]);
	}


    // Load the green dirt mask
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
	FindFiles(ThemesCounter(&themes), "data/themes", FM_DIR);

	if(themes.size() == 0) {
		// If we get here, then default to dirt
		printf("CMap::findRandomTheme(): no themes found\n");
		printf("                         Defaulting to \"dirt\"\n");
		return "dirt";
	}

    // Get a random number
    int t = GetRandomInt(themes.size()-1);
    return themes[t];
}


///////////////////
// Checks if a theme is a valid theme
bool CMap::validateTheme(const std::string& name) {
    // Does simple checks to see if the main files exists
    // Ie 'backtile.png' 'fronttile.png' & 'theme.txt'

    static std::string thm,buf;

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
int CMap::CreateSurface(void)
{
	SDL_Surface *screen = SDL_GetVideoSurface();
	if(screen == NULL)
		printf("CMap::CreateSurface: ERROR: screen is nothing\n");
	SDL_PixelFormat *fmt = screen->format;
	if(fmt == NULL)
		printf("CMap::CreateSurface: ERROR: fmt is nothing\n");

	bmpImage = gfxCreateSurface(Width, Height);
	if(bmpImage == NULL) {
		SetError("CMap::CreateSurface(): bmpImage creation failed, perhaps out of memory");
		return false;
	}

#ifdef _AI_DEBUG
	bmpDebugImage = gfxCreateSurface(Width*2, Height*2);
	if (bmpDebugImage == NULL)  {
		SetError("CMap::CreateSurface(): bmpDebugImage creation failed perhaps out of memory");
		return false;
	}

	SetColorKey(bmpDebugImage);
	FillSurface(bmpDebugImage, COLORKEY(bmpDebugImage));
#endif

	bmpDrawImage = gfxCreateSurface(Width*2, Height*2);
	if(bmpDrawImage == NULL) {
		SetError("CMap::CreateSurface(): bmpDrawImage creation failed, perhaps out of memory");
		return false;
	}

	bmpBackImage = gfxCreateSurface(Width, Height);
	if(bmpBackImage == NULL) {
		SetError("CMap::CreateSurface(): bmpBackImage creation failed, perhaps out of memory");
		return false;
	}

	bmpMiniMap = gfxCreateSurface(MinimapWidth, MinimapHeight);
	if(bmpMiniMap == NULL) {
		SetError("CMap::CreateSurface(): bmpMiniMap creation failed, perhaps out of memory");
		return false;
	}

    bmpShadowMap = gfxCreateSurface(Width, Height);
	if(bmpShadowMap == NULL) {
		SetError("CMap::CreateSurface(): bmpShadowMap creation failed, perhaps out of memory");
		return false;
	}

	return true;
}

////////////////////
// Updates the bmpDrawImage with data from bmpImage
// X, Y, W, H apply to bmpImage, not bmpDrawImage
void CMap::UpdateDrawImage(int x, int y, int w, int h)
{
	// HINT: we're not using DrawImageResampled when antialiasing is enabled because
	// the blurred level looks weird
	DrawImageStretch2(bmpDrawImage, bmpImage, x, y, x*2, y*2, w, h);
}

////////////////
// Set dimensions of the minimap
void CMap::SetMinimapDimensions(uint _w, uint _h)
{
	// If already created, reallocate
	if (bmpMiniMap)  {
		SDL_FreeSurface(bmpMiniMap);
		bmpMiniMap = gfxCreateSurface(_w, _h);
		MinimapWidth = _w;
		MinimapHeight = _h;
		UpdateMiniMap(true);
	// Just set it and CreateSurface will do the rest of the job
	} else {
		MinimapWidth = _w;
		MinimapHeight = _h;
	}
}


///////////////////
// Creates the level pixel flags
int CMap::CreatePixelFlags(void)
{
	lockFlags();
	PixelFlags = new uchar[Width*Height];
	unlockFlags();
	if(PixelFlags == NULL) {
		SetError("CMap::CreatePixelFlags(): Out of memory");
		return false;
	}

	return true;
}


///////////////////
// Create the AI Grid
bool CMap::createGrid(void) {
    nGridWidth = 15;
    nGridHeight = 15;

    nGridCols = Width/nGridWidth + 1;
    nGridRows = Height/nGridHeight + 1;

    lockFlags();
    GridFlags = new uchar[nGridCols * nGridRows];
    AbsoluteGridFlags = new uchar[nGridCols * nGridRows];
    if(GridFlags == NULL || AbsoluteGridFlags == NULL) {
		unlockFlags();
        SetError("CMap::CreateGrid(): Out of memory");
        return false;
    }
	memset(GridFlags,PX_EMPTY,nGridCols*nGridRows*sizeof(uchar));
	memset(AbsoluteGridFlags,PX_EMPTY,nGridCols*nGridRows*sizeof(uchar));
	unlockFlags();

    return true;
}


///////////////////
// Calculate the grid
void CMap::calculateGrid(void)
{
	lockFlags();
    for(uint y=0; y<Height; y+=nGridHeight)  {
        for(uint x=0; x<Width; x+=nGridWidth) {
            calculateGridCell(x,y, false);
        }
    }
    unlockFlags();
}


///////////////////
// Calculate a single grid cell
// x & y are pixel locations, not grid cell locations
// WARNING: not thread-safe (the caller has to ensure the threadsafty!)
void CMap::calculateGridCell(uint x, uint y, bool bSkipEmpty)
{
    uint i = x / nGridWidth;
    uint j = y / nGridHeight;

    if((int)i >= nGridCols || (int)j >= nGridRows) return;

    x = i * nGridWidth;
    y = j * nGridHeight;

    uchar *cell = GridFlags + j*nGridCols + i;
    uchar *abs_cell = AbsoluteGridFlags + j*nGridCols + i;

    // Skip empty cells?
    if(*cell == PX_EMPTY && bSkipEmpty)
        return;

    int dirtCount = 0;
    int rockCount = 0;

	int clip_h = MIN(y+nGridHeight,Height);
	int clip_w = MIN(x+nGridWidth,Width);
	uchar *pf;

    // Go through every pixel in the cell and get a solid flag count
    for(int b = y; b < clip_h; b++) {

        pf = PixelFlags + b*Width + x;
        for(int a = x; a < clip_w; a++, pf++) {

            if(*pf & PX_DIRT)
                dirtCount++;
            if(*pf & PX_ROCK)
                rockCount++;
        }
    }

    *abs_cell = PX_EMPTY;
    if(dirtCount > 0)
    	*abs_cell |= PX_DIRT;
    if(rockCount > 0)
    	*abs_cell |= PX_ROCK;

    int size = nGridWidth*nGridHeight / 10;

    // If the dirt or rock count is greater than a 10th, the cell is flagged
    *cell = PX_EMPTY;
    if(dirtCount > size)
        *cell = PX_DIRT;
    if(rockCount > size)    // Rock overrides dirt
        *cell = PX_ROCK;
}


///////////////////
// Tile the map
void CMap::TileMap(void)
{
	uint x,y;

	// Place the tiles
	
	// Place the first row
	for(y=0;y<Height;y+=Theme.bmpFronttile->h) {
		for(x=0;x<Width;x+=Theme.bmpFronttile->w) {
			DrawImage(bmpImage, Theme.bmpFronttile,x,y);
		}
	}

	for(y=0;y<Height;y+=Theme.bmpBacktile->h) {
		for(x=0;x<Width;x+=Theme.bmpBacktile->w) {
			DrawImage(bmpBackImage, Theme.bmpBacktile,x,y);
		}
	}

	// Update the draw image
	UpdateDrawImage(0, 0, Width, Height);
	
	// Set the pixel flags
	lockFlags();
	memset(PixelFlags,PX_DIRT,Width*Height*sizeof(uchar));
	unlockFlags();

    // Calculate the shadowmap
    CalculateShadowMap();

    // Calculate the total dirt count
    CalculateDirtCount();

	bMiniMapDirty = true;
}


///////////////////
// Calculate the dirt count in the level
void CMap::CalculateDirtCount(void)
{
    nTotalDirtCount = 0;
	uint n;
	const uint size = Width * Height;

	for (n = 0; n < size; n++)
		nTotalDirtCount += (PixelFlags[n] & PX_DIRT) ? 1 : 0;
}



///////////////////
// Draw the map
void CMap::Draw(SDL_Surface *bmpDest, CViewport *view)
{
		//DEBUG_DrawPixelFlags();
		DrawImageAdv(bmpDest, bmpDrawImage, view->GetWorldX()*2, view->GetWorldY()*2,view->GetLeft(),view->GetTop(),view->GetWidth()*2,view->GetHeight()*2);
#ifdef _AI_DEBUG
		/*if (GetKeyboard()->KeyDown[SDLK_F2])
			DrawImageStretch2(bmpDebugImage,bmpShadowMap,0, 0,0,0,Width,Height);
		else
			ClearDebugImage();*/
		DrawImageAdv(bmpDest, bmpDebugImage, view->GetWorldX()*2, view->GetWorldY()*2,view->GetLeft(),view->GetTop(),view->GetWidth()*2,view->GetHeight()*2);
#endif
}


///////////////////
// Draw an object's shadow
void CMap::DrawObjectShadow(SDL_Surface *bmpDest, SDL_Surface *bmpObj, int sx, int sy, int w, int h, CViewport *view, int wx, int wy)
{
	const int Drop = 3;

	// Calculate positions
	int dest_real_x = ((wx + Drop - view->GetWorldX()) * 2) + view->GetLeft();
	int dest_real_y = ((wy + Drop - view->GetWorldY()) * 2) + view->GetTop();

	int object_real_x = sx;
	int object_real_y = sy;

	int shadowmap_real_x = wx + Drop;
	int shadowmap_real_y = wy + Drop;

	int pixelflags_start_x = wx + Drop;  // X starting for pixelflags
	int pixelflags_y = wy + Drop; // current Y in pixelflags
	
	// Clipping
	ClipRect<int> dst_cliprect = ClipRect<int>(&dest_real_x, &dest_real_y, &w, &h);
	dst_cliprect.IntersectWith(SDLClipRect(&bmpDest->clip_rect), dst_cliprect); // Destination clipping
	
	ClipRect<int> obj_cliprect = ClipRect<int>(&object_real_x, &object_real_y, &w, &h);
	obj_cliprect.IntersectWith(SDLClipRect(&bmpObj->clip_rect), obj_cliprect); // Object clipping

	ClipRect<int> shadowmap_cliprect = ClipRect<int>(&shadowmap_real_x, &shadowmap_real_y, &w, &h);
	shadowmap_cliprect.IntersectWith(SDLClipRect(&bmpShadowMap->clip_rect), shadowmap_cliprect); // Map clipping

	// Pixels
	byte bpp = bmpDest->format->BytesPerPixel;
	Uint8 *dest_px, *obj_px, *shadowmap_px;
	Sint16 DestRowStep, ObjRowStep;
	Uint8 *ShadowmapPxRow;  // We draw shadowmap doubly stretched so we cannot use step there

	dest_px		   = (Uint8 *)bmpDest->pixels + (dest_real_y * bmpDest->pitch) + (dest_real_x * bpp);
	obj_px		   = (Uint8 *)bmpObj->pixels + (object_real_y * bmpObj->pitch) + (object_real_x * bpp);
	ShadowmapPxRow = (Uint8 *)bmpShadowMap->pixels + (shadowmap_real_y * bmpShadowMap->pitch) + (shadowmap_real_x * bpp);

	DestRowStep	 = bmpDest->pitch - (w * bpp);
	ObjRowStep	 = bmpObj->pitch - (w * bpp);

	// Pixelflags
	uchar *PixelFlag;

	// Loop variables
	int loop_x, loop_y;
	int loop_max_x = w;
	int loop_max_y = h;

	// Lock the surfaces
	if (SDL_MUSTLOCK(bmpDest))
		SDL_LockSurface(bmpDest);
	if (SDL_MUSTLOCK(bmpObj))
		SDL_LockSurface(bmpObj);
	if (SDL_MUSTLOCK(bmpShadowMap))
		SDL_LockSurface(bmpShadowMap);

	// Draw the shadow
	for (loop_y = loop_max_y; loop_y; --loop_y)  {
		PixelFlag = &PixelFlags[pixelflags_y * Width + pixelflags_start_x];
		shadowmap_px = ShadowmapPxRow;

		for (loop_x = loop_max_x; loop_x; --loop_x)  {

			if ( (*PixelFlag & PX_EMPTY))  { // Don't draw shadow on solid objects

				// Put pixel of not tranparent
				if (!IsTransparent(bmpObj, GetPixelFromAddr(obj_px, bpp)))
					memcpy(dest_px, shadowmap_px, bpp);
			}

			// Update the pixels & flag
			dest_px		 += bpp;
			obj_px		 += bpp;
			shadowmap_px += bpp * (loop_x & 1); // We draw the shadow doubly stretched -> only 1/2 pixel on shadowmap
			PixelFlag	 += loop_x & 1;
		}

		// Skip to next row
		dest_px		   += DestRowStep;
		obj_px		   += ObjRowStep;
		ShadowmapPxRow += bmpShadowMap->pitch * (loop_y & 1); // We draw the shadow doubly stretched -> only 1/2 row on shadowmap
		pixelflags_y   += loop_y & 1;
	}

	// Unlock the surfaces
	if (SDL_MUSTLOCK(bmpDest))
		SDL_UnlockSurface(bmpDest);
	if (SDL_MUSTLOCK(bmpObj))
		SDL_UnlockSurface(bmpObj);
	if (SDL_MUSTLOCK(bmpShadowMap))
		SDL_UnlockSurface(bmpShadowMap);		  
}


///////////////////
// Draw a pixel sized shadow
void CMap::DrawPixelShadow(SDL_Surface *bmpDest, CViewport *view, int wx, int wy)
{
    const int Drop = 3;
    wx += Drop;
    wy += Drop;

	// HINT: Clipping is done by DrawImageAdv

	// Get real coordinatesf
    int x = (wx - view->GetWorldX()) * 2 + view->GetLeft();
    int y = (wy - view->GetWorldY()) * 2 + view->GetTop();

    if( PixelFlags[wy * Width + wx] & PX_EMPTY )  // We should check all the 4 pixels, but no one will ever notice it
        DrawImageAdv( bmpDest, bmpShadowMap, wx, wy,  x, y, 2, 2 );
}


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
	SDL_Surface* hole = Theme.bmpHoles[size];
	if (!hole)
		return 0;

	int nNumDirt = 0;
	const int map_x = (int)pos.x - (hole->w / 2);
	const int map_y = (int)pos.y - (hole->h / 2);
	
	lockFlags();
	
	if(SDL_MUSTLOCK(hole))
		SDL_LockSurface(hole);
	if(SDL_MUSTLOCK(bmpImage))
		SDL_LockSurface(bmpImage);
	if(SDL_MUSTLOCK(bmpBackImage))
		SDL_LockSurface(bmpBackImage);
	
	for(int hx = 0; hx < hole->w; hx++)
		for(int hy = 0; hy < hole->h; hy++) {
			const int x = map_x + hx;
			const int y = map_y + hy;
			if(y < 0) continue;
			if((uint)y >= Height) break;
			if(x < 0 || (uint)x >= Width) break;			
			uchar* px = PixelFlags + y * Width + x;
			
			if(*px & PX_DIRT) {
				// Set the flag to empty
				if(GetPixel(hole, hx, hy) == tLX->clPink) {
		
					// Increase the dirt count
					nNumDirt++;
		
					*px = PX_EMPTY;
		
				// Put pixels that are not black/pink (eg, brown)
				} else if(GetPixel(hole, hx, hy) != tLX->clBlack)
					PutPixel(bmpImage, x, y, GetPixel(hole, hx, hy));			
			}
		
			if(*px & PX_EMPTY) {
				// redraw background-pixel because perhaps we don't have shadow here any more
				// we will update the shadowed pixel later
				PutPixel(bmpImage, x, y, GetPixel(bmpBackImage, x, y));
			}
		
		}

	unlockFlags();

	if(SDL_MUSTLOCK(hole))
		SDL_UnlockSurface(hole);
	if(SDL_MUSTLOCK(bmpImage))
		SDL_UnlockSurface(bmpImage);
	if(SDL_MUSTLOCK(bmpBackImage))
		SDL_UnlockSurface(bmpBackImage);

	if(!nNumDirt)
		return 0;

	// Apply a shadow
	ApplyShadow(map_x - 5, map_y - 5, hole->w + 25, hole->h + 25);

    // Recalculate the grid
    lockFlags();
	for(int hx = 0; hx < hole->w; hx += nGridWidth)
		for(int hy = 0; hy < hole->h; hy += nGridHeight) {
			const int x = map_x + hx;
			const int y = map_y + hy;
			calculateGridCell(x, y, true);
		}
	unlockFlags();

	// Update the draw image
	UpdateDrawImage(map_x - 5, map_y - 5, hole->w + 25, hole->h + 25);

	UpdateMiniMapRect(map_x, map_y, hole->w, hole->h);

    return nNumDirt;
}


///////////////////
// Place a bit of dirt
//
// Returns the number of dirt pixels placed
int CMap::PlaceDirt(int size, CVec pos)
{
	SDL_Surface *hole;
	int dx,dy, sx,sy;
	int x,y;
	int w,h;
	int ix,iy;
	Uint32 pixel;
	uchar flag;

    int nDirtCount = 0;

	if(size < 0 || size > 4) {
		// Just clamp it and continue
		size = MAX(size,0);
		size = MIN(size,4);
	}


	// Calculate half
	hole = Theme.bmpHoles[size];
	Uint32 pink = tLX->clPink;
	w = hole->w;
	h = hole->h;

	sx = (int)pos.x-(hole->w>>1);
	sy = (int)pos.y-(hole->h>>1);


	if(SDL_MUSTLOCK(hole))
		SDL_LockSurface(hole);

	Uint8 *p;
	uchar *px;
	Uint8 *p2;

	short screenbpp = SDL_GetVideoSurface()->format->BytesPerPixel;

	// Calculate clipping
	int clip_y = MAX(sy, 0);
	int clip_x = MAX(sx, 0);
	int clip_h = MIN(sy+h, bmpImage->h);
	int clip_w = MIN(sx+w, bmpImage->w);
	int hole_clip_y = -MIN(sy,(int)0);
	int hole_clip_x = -MIN(sx,(int)0);

	lockFlags();

	// Go through the pixels in the hole, setting the flags to dirt
	for(y = hole_clip_y, dy = clip_y; dy < clip_h; y++, dy++) {

		p = (Uint8 *)hole->pixels + y * hole->pitch + hole_clip_x * hole->format->BytesPerPixel;
		px = PixelFlags + dy * Width + clip_x;
		p2 = (Uint8 *)bmpImage->pixels + dy * bmpImage->pitch + clip_x * bmpImage->format->BytesPerPixel;

		for(x=hole_clip_x,dx=clip_x;dx<clip_w;x++,dx++) {

			pixel = GetPixelFromAddr(p, screenbpp);
			flag = *(uchar *)px;

			ix = dx % Theme.bmpFronttile->w;
			iy = dy % Theme.bmpFronttile->h;

			// Set the flag to empty
			if(!IsTransparent(hole, pixel) && !(flag & PX_ROCK)) {
                if( flag & PX_EMPTY )
                    nDirtCount++;

				*(uchar*)px = PX_DIRT;

				// Place the dirt image
				PutPixelToAddr(p2, GetPixel(Theme.bmpFronttile, ix, iy), screenbpp);
			}

			// Put pixels that are not black/pink (eg, brown)
            if(!IsTransparent(hole, pixel) && pixel != pink && (flag & PX_EMPTY)) {
				PutPixelToAddr(p2, pixel, screenbpp);
                *(uchar*)px = PX_DIRT;
                nDirtCount++;
            }

			p += screenbpp;
			p2 += screenbpp;
			px++;
		}
	}

	unlockFlags();

	if(SDL_MUSTLOCK(hole))
		SDL_UnlockSurface(hole);

	// Apply a shadow
	ApplyShadow(sx-5,sy-5,w+25,h+25);

	// Update the draw image
	int draw_x = sx-5;
	int draw_y = sy-5;
	// Clipping
	if(draw_x+w+25 > bmpImage->w)
		draw_x = bmpImage->w-w-25;
	if(draw_y+h+25 > bmpImage->h)
		draw_y = bmpImage->h-h-25;
	if (draw_x < 0)
		draw_x = 0;
	if (draw_y < 0)
		draw_y = 0;
	UpdateDrawImage(draw_x, draw_y, w+25, h+25);

	UpdateMiniMapRect(draw_x,draw_y,w+25,h+25);
	//bMiniMapDirty = true;


	// Recalculate the grid
	lockFlags();
	for(y=sy; y<sy+h+nGridHeight; y+=nGridHeight) {
		for(x=sx; x<sx+w+nGridWidth; x+=nGridWidth) {
			calculateGridCell(x, y, false);
		}
	}
	unlockFlags();


    return nDirtCount;
}


///////////////////
// Place a blob of green dirt (greenball)
//
// Returns the number of dirt pixels placed
int CMap::PlaceGreenDirt(CVec pos)
{
	if (!bmpGreenMask)
		return 0;

 	int dx,dy, sx,sy;
	int x,y;
	int w,h;
	Uint32 pixel;
	uchar flag;
    const Uint32 green = MakeColour(0,255,0);
	const Uint32 pink = MakeColour(255,0,255);
    const Uint32 greens[4] = {MakeColour(148,136,0),
				              MakeColour(136,124,0),
						      MakeColour(124,112,0),
							  MakeColour(116,100,0)};

    int nGreenCount = 0;

	// Calculate half
	w = bmpGreenMask->w;
	h = bmpGreenMask->h;

	sx = (int)pos.x-(w>>1);
	sy = (int)pos.y-(h>>1);


	if(SDL_MUSTLOCK(bmpGreenMask))
		SDL_LockSurface(bmpGreenMask);


	Uint8 *p;
	uchar *px;
	Uint8 *p2;
	Uint32 gr;

	// Calculate clipping
	int clip_y = MAX(sy, 0);
	int clip_x = MAX(sx, 0);
	int clip_h = MIN(sy+h, bmpImage->h);
	int clip_w = MIN(sx+w, bmpImage->w);
	int green_clip_y = -MIN(sy,(int)0);
	int green_clip_x = -MIN(sx,(int)0);

	short screenbpp = SDL_GetVideoSurface()->format->BytesPerPixel;

	lockFlags();

	// Go through the pixels in the hole, setting the flags to dirt
	for(y = green_clip_y, dy=clip_y; dy < clip_h; y++, dy++) {

		p = (Uint8*)bmpGreenMask->pixels
			+ y * bmpGreenMask->pitch
			+ green_clip_x * bmpGreenMask->format->BytesPerPixel;
		px = PixelFlags + dy * Width + clip_x;
		p2 = (Uint8 *)bmpImage->pixels + dy * bmpImage->pitch + clip_x * bmpImage->format->BytesPerPixel;

		for(x = green_clip_x, dx=clip_x; dx < clip_w; x++, dx++) {

			pixel = GetPixelFromAddr(p,screenbpp);
			flag = *(uchar*)px;

			// Set the flag to dirt
			if(pixel == green && (flag & PX_EMPTY)) {
				*(uchar*)px = PX_DIRT;
                nGreenCount++;

                // Place a random green pixel
                gr = greens[ GetRandomInt(3) ];

				// Place the green pixel
				PutPixelToAddr(p2, gr, screenbpp);
			}

			// Put pixels that are not green/pink (eg, dark green)
            if(pixel != green && pixel != pink && (flag & PX_EMPTY)) {
				PutPixelToAddr(p2, pixel, screenbpp);
                *(uchar*)px = PX_DIRT;
                nGreenCount++;
            }

			p += screenbpp;
			p2 += screenbpp;
			px++;
		}
	}

	unlockFlags();

	if(SDL_MUSTLOCK(bmpGreenMask))
		SDL_UnlockSurface(bmpGreenMask);

	// Nothing placed, no need to update
	if (nGreenCount == 0)
		return 0;

	if (!bmpDrawImage)
		return nGreenCount;

	// Apply a shadow
	ApplyShadow(sx-5,sy-5,w+25,h+25);

	// Update the draw image
	int draw_x = sx-5;
	int draw_y = sy-5;
	// Clipping
	if(draw_x + w + 25 > bmpImage->w)
		draw_x = bmpImage->w - w - 25;
	if(draw_y + h + 25 > bmpImage->h)
		draw_y = bmpImage->h - h - 25;
	if (draw_x < 0)
		draw_x = 0;
	if (draw_y < 0)
		draw_y = 0;
	UpdateDrawImage(draw_x, draw_y, w + 25, h + 25);

	UpdateMiniMapRect(draw_x, draw_y, w + 25, h + 25);
	//bMiniMapDirty = true;

	// Recalculate the grid
	lockFlags();
	for(y = sy; y < sy + h + nGridHeight; y += nGridHeight)
		for(x = sx; x < sx + w + nGridWidth; x += nGridWidth)
			calculateGridCell(x, y, false);
	unlockFlags();

    return nGreenCount;
}


///////////////////
// Apply a shadow to an area
void CMap::ApplyShadow(int sx, int sy, int w, int h)
{
	// Draw shadows?
	if(!tLXOptions->iShadows)
		return;

	static const ushort Drop = 3;
	int x, y, n;
	uchar *px;
	uchar *p;
	uint ox,oy;
	uchar flag;
	Uint32 offset;

	Uint8 *pixel,*src;

	int screenbpp = SDL_GetVideoSurface()->format->BytesPerPixel;

	if(SDL_MUSTLOCK(bmpImage))
		SDL_LockSurface(bmpImage);

	lockFlags();

	int clip_y = MAX(sy, (int)0); 
	int clip_x = MAX(sx, (int)0);
	int clip_h = MIN(sy + h, Height);
	int clip_w = MIN(sx + w, Width);

	for(y = clip_y; y < clip_h; y++) {

		px = PixelFlags + y * Width + clip_x;

		for(x = clip_x; x < clip_w; x++) {

			flag = *(uchar *)px;

			// Edge hack
			//if(x==0 || y==0 || x==Width-1 || y==Height-1)
				//flag = PX_EMPTY;

			if(!(flag & PX_EMPTY)) {
				ox = x+1; oy = y+1;

				// Draw the shadow
				for(n = 0; n < Drop; n++) {

					// Clipping
					if(ox >= Width) break;
					if(oy >= Height) break;

					p = PixelFlags + oy * Width + ox;
					if(!( (*(uchar *)p) & PX_EMPTY))
						break;

                    offset = oy*bmpImage->pitch + ox*screenbpp;
                    pixel = (Uint8*)bmpImage->pixels + offset;
                    src = (Uint8*)bmpShadowMap->pixels + offset;
					memcpy(pixel, src, screenbpp);

					*(uchar *)p |= PX_EMPTY | PX_SHADOW;
					ox++; oy++;
				}
			}

			px++;
		}
	}

	unlockFlags();

	if(SDL_MUSTLOCK(bmpImage))
		SDL_UnlockSurface(bmpImage);

	bMiniMapDirty = true;
}


///////////////////
// Calculate the shadow map
void CMap::CalculateShadowMap(void)
{
	// This should be faster
	SDL_BlitSurface(bmpBackImage, NULL, bmpShadowMap, NULL);
	DrawRectFillA(bmpShadowMap,0,0,bmpShadowMap->w,bmpShadowMap->h,tLX->clBlack,96);
	
 /*   int x,y;

    Uint32 Rmask = bmpImage->format->Rmask, Gmask = bmpImage->format->Gmask, Bmask = bmpImage->format->Bmask, Amask = bmpImage->format->Amask;
	Uint32 R,G,B,A = 0;
	Uint8 alpha = 100;
	Uint32 color = 0;
	Uint32 dc = 0;

	int screenbpp = SDL_GetVideoSurface()->format->BytesPerPixel;

    Uint8 *pixel = (Uint8 *)bmpShadowMap->pixels;
    Uint8 *srcpixel = (Uint8 *)bmpBackImage->pixels;

	if(SDL_MUSTLOCK(bmpBackImage))
		SDL_LockSurface(bmpBackImage);
    if(SDL_MUSTLOCK(bmpShadowMap))
		SDL_LockSurface(bmpShadowMap);

    for( y=0; y<Height; y++ ) {

		for( x=0; x<Width; x++, pixel += screenbpp, srcpixel+=screenbpp ) {

			// Transparency
			//Uint32 dc = *srcpixel;
			dc=0;
			memcpy(&dc,srcpixel,screenbpp);

			R = ((dc & Rmask) + (( (color & Rmask) - (dc & Rmask) ) * alpha >> 8)) & Rmask;
			G = ((dc & Gmask) + (( (color & Gmask) - (dc & Gmask) ) * alpha >> 8)) & Gmask;
			B = ((dc & Bmask) + (( (color & Bmask) - (dc & Bmask) ) * alpha >> 8)) & Bmask;
			if( Amask )
				A = ((dc & Amask) + (( (color & Amask) - (dc & Amask) ) * alpha >> 8)) & Amask;

			// TODO: endian
			dc = (R | G | B | A);
			memcpy(pixel,&dc,screenbpp);
			// *pixel= (Uint16)dc;
		}

    }

    if(SDL_MUSTLOCK(bmpBackImage))
		SDL_UnlockSurface(bmpBackImage);
    if(SDL_MUSTLOCK(bmpShadowMap))
		SDL_UnlockSurface(bmpShadowMap);*/
}


///////////////////
// Place a stone
void CMap::PlaceStone(int size, CVec pos)
{
	SDL_Surface *stone;
	short dy, sx,sy;
	short x,y;
	short w,h;

	if(size < 0 || size >= Theme.NumStones) {
		// TODO: Bail out or warning of overflow
		printf("Bad stone size\n");
		if(size < 0) size = 0;
		else size = Theme.NumStones-1;
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
	w = stone->w;
	h = stone->h;

	sx = (int)pos.x - (stone->w >> 1);
	sy = (int)pos.y - (stone->h >> 1);

	// Blit the stone to the surface
	DrawImage(bmpImage, stone, sx, sy);

	if(SDL_MUSTLOCK(stone))
		SDL_LockSurface(stone);

	lockFlags();

	// Calculate the clipping bounds, so we don't have to check each loop then
	short clip_h = MIN(sy+stone->h, bmpImage->h) - sy;
	short clip_w = MIN(sx+stone->w, bmpImage->w) - sx;
	short clip_y = 0; 
	short clip_x = 0; 
	if (sy<0) 
		clip_y = abs(sy);
	if (sx<0) 
		clip_x = abs(sx);

	// Pixels
	Uint8 *p = NULL;
	Uint8 *PixelRow = (Uint8*)stone->pixels + clip_y*stone->pitch;
	uchar *px = PixelFlags;
	short pf_tmp = MAX((short)0, sx);
	short p_tmp = clip_x * stone->format->BytesPerPixel;

	// Go through the pixels in the stone and update pixel flags
	for(y = clip_y, dy = MAX((short)0, sy); y < clip_h; y++, dy++, PixelRow += stone->pitch) {

		p = PixelRow+p_tmp;
		px = PixelFlags + dy * Width + pf_tmp;

		for(x = clip_x; x < clip_w; x++) {

			// Rock?
			if(!IsTransparent(stone, GetPixelFromAddr(p, stone->format->BytesPerPixel))) {
				*(uchar*)px = PX_ROCK;
			}

			p += stone->format->BytesPerPixel;
			px++;
		}
	}

	unlockFlags();

	if(SDL_MUSTLOCK(stone))
		SDL_UnlockSurface(stone);

	// Apply the shadow
	ApplyShadow(sx - 5, sy - 5, w + 10, h + 10);

	// Update the draw image
	short draw_x = MAX(sx - 5, 0);
	short draw_y = MAX(sy - 5, 0);
	UpdateDrawImage(draw_x, draw_y,	stone->w + 10, stone->h + 10);

    // Calculate the total dirt count
    CalculateDirtCount();

	// Update the minimap rectangle
	UpdateMiniMapRect(draw_x, draw_y, stone->w + 10, stone->h + 10);
	//bMiniMapDirty = true;
}


///////////////////
// Place a miscellaneous item
void CMap::PlaceMisc(int id, CVec pos)
{

	SDL_Surface *misc;
	short dy,dx, sx,sy;
	short x,y;
	short w,h;

	if(id < 0 || id >= Theme.NumMisc) {
		printf("Bad misc size\n");
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
	w = misc->w;
	h = misc->h;

	sx = (int)pos.x-(misc->w>>1);
	sy = (int)pos.y-(misc->h>>1);


	if(SDL_MUSTLOCK(misc))
		SDL_LockSurface(misc);

	lockFlags();

	// Calculate the clipping bounds, so we don't have to check each loop then
	short clip_h = MIN(sy+misc->h,bmpImage->h)-sy;
	short clip_w = MIN(sx+misc->w,bmpImage->w)-sx;
	short clip_y = 0; 
	short clip_x = 0; 
	if (sy<0) 
		clip_y = -sy;
	if (sx<0) 
		clip_x = -sx;

	Uint8 *p = NULL;
	Uint8 *PixelRow = (Uint8 *)misc->pixels+clip_y*misc->pitch;
	uchar *px = PixelFlags;

	// Temps for better performance
	short pf_tmp = MAX((short)0,sx);
	short p_tmp = clip_x*misc->format->BytesPerPixel;
	short dx_tmp = MAX((short)0,sx);

	// Go through the pixels in the misc item
	for(y = clip_y, dy = MAX((short)0, sy); y < clip_h; y++, dy++, PixelRow += misc->pitch) {

		p = PixelRow + p_tmp;
		px = PixelFlags + dy * Width + pf_tmp;

		for(x = clip_x, dx = dx_tmp; x < clip_w; dx++, x++) {

			// Put the pixel down
			if(!IsTransparent(misc, GetPixelFromAddr(p, misc->format->BytesPerPixel)) && (*px & PX_DIRT)) {
				PutPixel(bmpImage, dx, dy, GetPixelFromAddr(p, misc->format->BytesPerPixel));
				*(uchar*)px = PX_DIRT;
			}

			p += misc->format->BytesPerPixel;
			px++;
		}
	}

	unlockFlags();

	if(SDL_MUSTLOCK(misc))
		SDL_UnlockSurface(misc);

	// Update the draw image
	short draw_x = MAX(sx-5,0);
	short draw_y = MAX(sy-5,0);
	UpdateDrawImage(draw_x, draw_y, clip_w+10, clip_h+10);

	UpdateMiniMapRect(draw_x, draw_y, clip_w+10, clip_h+10);
	//bMiniMapDirty = true;
}


///////////////////
// Delete an object
void CMap::DeleteObject(CVec pos)
{
	int w=0,h=0;

	// Go through the objects, last to first and see if one is under the mouse
	for(int o=NumObjects-1;o>=0;o--) {
		object_t *obj = &Objects[o];

		switch(obj->Type) {

			// Stone
			case OBJ_STONE:
				w = Theme.bmpStones[obj->Size]->w << 1;
				h = Theme.bmpStones[obj->Size]->h << 1;
				break;

			// Misc
			case OBJ_MISC:
				w = Theme.bmpMisc[obj->Size]->w << 1;
				h = Theme.bmpMisc[obj->Size]->h << 1;
				break;
		}

		if((int)fabs(pos.x - obj->X) < w &&
			(int)fabs(pos.y - obj->Y) < h) {

			if(obj->Type == OBJ_STONE)
				DeleteStone(obj);

			// Remove this object from this list
			for(int i=o;i<NumObjects-1;i++)
				Objects[i] = Objects[i+1];
			NumObjects--;


			break;
		}
	}

	bMiniMapDirty = true;
}


///////////////////
// Delete a stone from the map
void CMap::DeleteStone(object_t *obj)
{
	// Replace the stone pixels with dirt pixels

}


///////////////////
// Put a pixel onto the front image buffer
void CMap::PutImagePixel(uint x, uint y, Uint32 colour)
{
    // Checking edges
	if(x >= Width || y >= Height)
		return;

    PutPixel(bmpImage, x,y, colour);
	PutPixel(bmpDrawImage, x*2,y*2, colour);
	PutPixel(bmpDrawImage, x*2+1,y*2, colour);
	PutPixel(bmpDrawImage, x*2,y*2+1, colour);
	PutPixel(bmpDrawImage, x*2+1,y*2+1, colour);
}


///////////////////
// Update the minimap
void CMap::UpdateMiniMap(bool force)
{
	if(!bMiniMapDirty && !force)
		return;

	// Calculate ratios
	float xratio = (float)bmpMiniMap->w / (float)bmpImage->w;
	float yratio = (float)bmpMiniMap->h / (float)bmpImage->h;

	if (tLXOptions->bAntiAliasing)
		DrawImageResampledAdv(bmpMiniMap, bmpImage, 0.0f, 0.0f, 0, 0, bmpImage->w, bmpImage->h, xratio, yratio, MINIMAP_BLUR);
	else
		DrawImageResizedAdv(bmpMiniMap, bmpImage, 0.0f, 0.0f, 0, 0, bmpImage->w, bmpImage->h, xratio, yratio);

	// Not dirty anymore
	bMiniMapDirty = false;
}

///////////////////
// Update an area of the minimap
// X, Y, W and H apply to the bmpImage, not bmpMinimap
void CMap::UpdateMiniMapRect(int x, int y, int w, int h)
{
	// Calculate ratios
	const float xratio = (float)bmpMiniMap->w / (float)bmpImage->w;
	const float yratio = (float)bmpMiniMap->h / (float)bmpImage->h;

	const float sx = (float)((int)(x * xratio) / xratio);
	const float sy = (float)((int)(y * yratio) / yratio);
	const int dx = (int)((float)x * xratio);
	const int dy = (int)((float)y * yratio);

	if (tLXOptions->bAntiAliasing)
		DrawImageResampledAdv(bmpMiniMap, bmpImage, sx, sy, dx, dy, w, h, xratio, yratio, MINIMAP_BLUR);
	else
		DrawImageResizedAdv(bmpMiniMap, bmpImage, sx, sy, dx, dy, w, h, xratio, yratio);

	// Not dirty anymore
	bMiniMapDirty = false;
}


///////////////////
// Draw & Simulate the minimap
void CMap::DrawMiniMap(SDL_Surface *bmpDest, uint x, uint y, float dt, CWorm *worms, int gametype)
{
	int i,j;
	float xstep,ystep;
	float mx,my;
	int n;
	static float time = 0;
	int mw = bmpMiniMap->w;
	int mh = bmpMiniMap->h;

	// Calculate steps
	xstep = (float)Width/ (float)mw;
	ystep = (float)Height / (float)mh;

	if(worms == NULL)
		return;

	time+=dt;
	if(time>0.5f)
		time=0;

	Uint8 r,g,b,a;

	// Update the minimap (only if dirty)
	if(bMiniMapDirty)
		UpdateMiniMap();


	// Draw the minimap
	DrawImage(bmpDest, bmpMiniMap, x, y);


	// Show worms
	CWorm *w = worms;
	byte dr,dg,db;
	Uint32 col;
	bool big=false;
	for(n=0;n<MAX_WORMS;n++,w++) {
		if(!w->getAlive() || !w->isUsed())
			continue;

		GetColour4(w->getColour(), bmpMiniMap, &r,&g,&b,&a);

		dr = ~r;
		dg = ~g;
		db = ~b;

		r += (int)( (float)dr*(time*2.0f));
		g += (int)( (float)dg*(time*2.0f));
		b += (int)( (float)db*(time*2.0f));

		mx = w->getPos().x/xstep;
		my = w->getPos().y/ystep;
        mx = (float)floor(mx);
        my = (float)floor(my);

		mx = MIN(mw-(float)1,mx); mx = MAX((float)0,mx);
		my = MIN(mh-(float)1,my); my = MAX((float)0,my);
		i=(int)mx + x;
		j=(int)my + y;
		// Snap it to the nearest 2nd pixel (prevent 'jumping')
		//x -= x % 2;
		//y -= y % 2;

		col = MakeColour(r,g,b);

		//PutPixel(bmpMiniMap,x,y,col);
		DrawRectFill(bmpDest,i-1,j-1, i+1,j+1, col);

		// Our worms are bigger
		big = false;

		// Tagged worms or local players are bigger, depending on the game type
		if(gametype != GMT_TAG) {
			big = (w->getType() == PRF_HUMAN && w->getLocal());
		} else {
			big = w->getTagIT()!=0;
		}

		if(big)
			DrawRectFill(bmpDest,i-1,j-1, i+2,j+2,col);
	}
}

///////////////////
// Load the map
int CMap::Load(const std::string& filename)
{
	// Weird
	if (filename == "")
		return true;

	FILE *fp = OpenGameFile(filename,"rb");
	if(fp == NULL)
		return false;

	bMiniMapDirty = true;
    sRandomLayout.bUsed = false;

	// Check if it's an original liero level
	if( stringcasecmp(GetFileExtension(filename), "lev") == 0 ) {
		return LoadOriginal(fp);
	}


	// Header
	static std::string id;
	id = freadfixedcstr(fp, 32);
	int		version;
	fread(&version,		sizeof(int),	1,	fp);
	EndianSwap(version);

	// Check to make sure it's a valid level file
	if(id != "LieroX Level" || version != MAP_VERSION) {
		printf("CMap::Load (%s): ERROR: not a valid level file or wrong version\n", filename.c_str());
		fclose(fp);
		return false;
	}

	Name = freadfixedcstr(fp, 64);
	fread(&Width,		sizeof(int),	1,	fp);
	EndianSwap(Width);
	fread(&Height,		sizeof(int),	1,	fp);
	EndianSwap(Height);
	fread(&Type,		sizeof(int),	1,	fp);
	EndianSwap(Type);
	static std::string Theme_Name;
	Theme_Name = freadfixedcstr(fp, 32);
	int		numobj;
	fread(&numobj,		sizeof(int),	1,	fp);
	EndianSwap(numobj);

/*
	printf("Level info:\n");
	printf("  id = %s\n", id);
	printf("  version = %i\n", version);
	printf("  Name = %s\n", Name);
	printf("  Width = %i\n", Width);
	printf("  Height = %i\n", Height);
	printf("  Type = %i\n", Type);
	printf("  Theme_Name = %s\n", Theme_Name);
	printf("  numobj = %i\n", numobj);
*/
	// Create the map
	if(!New(Width, Height, Theme_Name, MinimapWidth, MinimapHeight)) {
		printf("CMap::Load (%s): ERROR: cannot create map\n", filename.c_str());
		fclose(fp);
		return false;
	}

	// Load the images if in an image format
	if(Type == MPT_IMAGE)
	{
		printf("CMap::Load (%s): HINT: level is in image format\n", filename.c_str());
		return LoadImageFormat(fp);
	}


	// Lock the surfaces
	if (SDL_MUSTLOCK(bmpImage))
		SDL_LockSurface(bmpImage);
	if (SDL_MUSTLOCK(bmpBackImage))
		SDL_LockSurface(bmpBackImage);

	// Dirt map
	size_t n,i,j,x=0;
	Uint8 *p1 = (Uint8 *)bmpImage->pixels;
	Uint8 *p2 = (Uint8 *)bmpBackImage->pixels;
	Uint8 *dstrow = p1;
	Uint8 *srcrow = p2;

	// Load the bitmask, 1 bit == 1 pixel with a yes/no dirt flag
	uint size = Width*Height/8;
	uchar *bitmask = new uchar[size];
	if (!bitmask)  {
		printf("CMap::Load: Could not create bit mask\n");
		return false;
	}
	fread(bitmask,size,1,fp);
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	static const unsigned char mask[] = {128,64,32,16,8,4,2,1};
#else
	static const unsigned char mask[] = {1,2,4,8,16,32,64,128};
#endif

	nTotalDirtCount = Width*Height;  // Calculate the dirt count

	lockFlags();
	
	for(n=0,i=0;i<size;i++,x+=8) {
		if (x>=Width)  {
			srcrow += bmpBackImage->pitch;
			dstrow += bmpImage->pitch;
			p1 = dstrow;
			p2 = srcrow;
			x=0;
		}

		// 1 bit == 1 pixel with a yes/no dirt flag
		for(j=0;j<8;j++,n++,p1+=bmpImage->format->BytesPerPixel,p2+=bmpBackImage->format->BytesPerPixel) {

			if(bitmask[i] & mask[j])  {
				PixelFlags[n] = PX_EMPTY;
				nTotalDirtCount--;
				memcpy(p1,p2,bmpImage->format->BytesPerPixel);
			}
		}
	}

	unlockFlags();
	
	delete[] bitmask;

	// Unlock the surfaces
	if (SDL_MUSTLOCK(bmpImage))
		SDL_UnlockSurface(bmpImage);
	if (SDL_MUSTLOCK(bmpBackImage))
		SDL_UnlockSurface(bmpBackImage);

	// Objects
	object_t o;
	NumObjects = 0;
	for(i=0;(int)i<numobj;i++) {
		fread(&o.Type,	sizeof(int),	1,	fp);
		EndianSwap(o.Type);
		fread(&o.Size,	sizeof(int),	1,	fp);
		EndianSwap(o.Size);
		fread(&o.X,	    sizeof(int),	1,	fp);
		EndianSwap(o.X);
        fread(&o.Y,	    sizeof(int),	1,	fp);
		EndianSwap(o.Y);

		// Place the object
		if(o.Type == OBJ_STONE)
			PlaceStone(o.Size,CVec((float)o.X, (float)o.Y));
		else if(o.Type == OBJ_MISC)
			PlaceMisc(o.Size,CVec((float)o.X, (float)o.Y));
	}

	// Close the file
	fclose(fp);


	// Apply the shadow
	ApplyShadow(0,0,Width,Height);

	// Update the draw image
	DrawImageStretch(bmpDrawImage,bmpImage,0,0);

	// Update the minimap
	UpdateMiniMap(true);

    // Calculate the total dirt count
    CalculateDirtCount();

    // Calculate the shadowmap
    CalculateShadowMap();

    // Calculate the grid
    calculateGrid();

	return true;
}


///////////////////
// Save the map
int CMap::Save(const std::string& name, const std::string& filename)
{
	FILE *fp = OpenGameFile(filename,"wb");
	if(fp == NULL)
		return false;

	Type = MPT_IMAGE;

	// Header
	int		version = MAP_VERSION;

	fwrite("LieroX Level",	32,	fp);
	fwrite(GetEndianSwapped(version),	sizeof(int),	1,	fp);
	fwrite(name,	64,	fp);
	fwrite(GetEndianSwapped(Width),		sizeof(int),	1,	fp);
	fwrite(GetEndianSwapped(Height),		sizeof(int),	1,	fp);
	fwrite(GetEndianSwapped(Type),		sizeof(int),	1,	fp);
	fwrite(Theme.name,	32,	fp);
	fwrite(GetEndianSwapped(NumObjects),	sizeof(int),	1,	fp);


	// Save the images if in an image format
	if(Type == MPT_IMAGE)
		return SaveImageFormat(fp);



	// Dirt map
	uint n;
	for(n=0;n<Width*Height;) {
		uchar t = 0;

		// 1 bit == 1 pixel with a yes/no dirt flag
		for(ushort i=0;i<8;i++) {
			uchar value = (PixelFlags[n++] & PX_EMPTY);
			t |= (value << i);
		}

		fwrite(&t,	sizeof(uchar),	1,	fp);
	}


	// Objects
	object_t *o = Objects;
	for(int i=0;i<NumObjects;i++,o++) {
		fwrite(GetEndianSwapped(o->Type),sizeof(int),	1,	fp);
		fwrite(GetEndianSwapped(o->Size),sizeof(int),	1,	fp);
		fwrite(GetEndianSwapped(o->X),	sizeof(int),	1,	fp);
        fwrite(GetEndianSwapped(o->Y),	sizeof(int),	1,	fp);
	}


	fclose(fp);

	return true;
}


///////////////////
// Save the images
int CMap::SaveImageFormat(FILE *fp)
{
	uint x,y,n,p;
	Uint8 r,g,b,a;

	// The images are saved in a raw 24bit format.
	// 8 bits per r,g,b channel

	if( !bmpBackImage || !bmpImage || !PixelFlags )
		return false;

	// Write out the images & pixeflags to memory, compress the data & save the compressed data
	Uint32 size = (Width*Height * 3) * 2 + (Width*Height) + 1;
	Uint32 destsize = size + (size / 8) + 12;

	uchar *pSource = new uchar[size];
	uchar *pDest = new uchar[destsize];

	if(!pSource || !pDest) {
		printf("CMap::SaveImageFormat: ERROR: not enough memory for pSource and pDest\n");
		fclose(fp);
		return false;
	}

	// Save the back image
	p=0;
	for(y=0; y<Height; y++) {
		for(x=0; x<Width; x++) {
			GetColour4( GetPixel(bmpBackImage,x,y), bmpBackImage, &r, &g, &b ,&a );

			pSource[p++] = r;
			pSource[p++] = g;
			pSource[p++] = b;
		}
	}

	// Save the front image
	for(y=0; y<Height; y++) {
		for(x=0; x<Width; x++) {
			GetColour4( GetPixel(bmpImage,x,y), bmpImage, &r, &g, &b ,&a );
			pSource[p++] = r;
			pSource[p++] = g;
			pSource[p++] = b;
		}
	}

	// Save the pixel flags
	for(n=0;n<Width*Height;n++) {
		uchar t = PX_EMPTY;

		if(PixelFlags[n] & PX_DIRT)
			t = PX_DIRT;
		if(PixelFlags[n] & PX_ROCK)
			t = PX_ROCK;

		pSource[p++] = t;
	}

	// Compress it
	ulong lng_dsize = destsize;
	if( compress( pDest, &lng_dsize, pSource, size) != Z_OK ) {
		printf("Failed compressing\n");
		fclose(fp);
		delete[] pSource;
		delete[] pDest;
		return false;
	}
	destsize = lng_dsize; // WARNING: possible overflow ; TODO: do a check for it?

	// Write out the details & the data
	fwrite(GetEndianSwapped(destsize), sizeof(Uint32), 1, fp);
	fwrite(GetEndianSwapped(size), sizeof(Uint32), 1, fp);
	fwrite(pDest, sizeof(uchar), destsize, fp);

	delete[] pSource;
	delete[] pDest;

	fclose(fp);
	return true;
}


///////////////////
// Load the image format
int CMap::LoadImageFormat(FILE *fp)
{
	// Load the details
	Uint32 size, destsize;
	uint x,y,n,p;

	fread(&size, sizeof(Uint32), 1, fp);
	EndianSwap(size);
	fread(&destsize, sizeof(Uint32), 1, fp);
	EndianSwap(destsize);

	// Allocate the memory
	uchar *pSource = new uchar[size];
	uchar *pDest = new uchar[destsize];

	if(!pSource || !pDest) {
		fclose(fp);
		return false;
	}

	fread(pSource, size*sizeof(uchar), 1, fp);

	ulong lng_dsize = destsize;
	if( uncompress( pDest, &lng_dsize, pSource, size ) != Z_OK ) {
		printf("Failed decompression\n");
		fclose(fp);
		delete[] pSource;
		delete[] pDest;
		return false;
	}
	destsize = lng_dsize; // WARNING: possible overflow ; TODO: do a check for it?

	delete[] pSource;  // not needed anymore

	//
	// Translate the data
	//

	// Lock surfaces
	if (SDL_MUSTLOCK(bmpBackImage))
		SDL_LockSurface(bmpBackImage);
	if (SDL_MUSTLOCK(bmpImage))
		SDL_LockSurface(bmpImage);

	p=0;
	Uint32 curcolor=0;
	Uint8* curpixel = (Uint8*)bmpBackImage->pixels;
	Uint8* PixelRow = curpixel;

	// TODO: Check if pDest is big enough

	// Load the back image
	for (y = 0; y < Height; y++, PixelRow += bmpBackImage->pitch)  {
		curpixel = PixelRow;
		for (x = 0; x < Width; x++, curpixel += bmpBackImage->format->BytesPerPixel)  {
			curcolor = MakeColour(pDest[p], pDest[p+1], pDest[p+2]);
			p += 3;
			PutPixelToAddr(curpixel, curcolor, bmpBackImage->format->BytesPerPixel);
		}
	}

	// Load the front image
	curpixel = (Uint8 *)bmpImage->pixels;
	PixelRow = curpixel;
	for (y = 0; y < Height; y++, PixelRow += bmpImage->pitch)  {
		curpixel = PixelRow;
		for (x = 0;x < Width; x++, curpixel += bmpImage->format->BytesPerPixel)  {
			curcolor = MakeColour(pDest[p], pDest[p+1], pDest[p+2]);
			p += 3;
			PutPixelToAddr(curpixel, curcolor, bmpImage->format->BytesPerPixel);
		}
	}


	// Load the pixel flags and calculate dirt count
	n=0;
	nTotalDirtCount = 0;

	curpixel = (Uint8 *)bmpImage->pixels;
	PixelRow = curpixel;
	Uint8 *backpixel = (Uint8 *)bmpBackImage->pixels;
	Uint8 *BackPixelRow = backpixel;

	lockFlags();
	
	for(y=0; y<Height; y++,PixelRow+=bmpImage->pitch,BackPixelRow+=bmpBackImage->pitch) {
		curpixel = PixelRow;
		backpixel = BackPixelRow;
		for(x=0; x<Width; x++,curpixel+=bmpImage->format->BytesPerPixel,backpixel+=bmpBackImage->format->BytesPerPixel) {
			PixelFlags[n] = pDest[p++];
			if(PixelFlags[n] & PX_EMPTY)
				memcpy(curpixel, backpixel, bmpImage->format->BytesPerPixel);
			nTotalDirtCount += (PixelFlags[n] & PX_DIRT) ? 1 : 0;
			n++;
		}
	}
	unlockFlags();

	// Unlock the surfaces
	if (SDL_MUSTLOCK(bmpBackImage))
		SDL_UnlockSurface(bmpBackImage);
	if (SDL_MUSTLOCK(bmpImage))
		SDL_UnlockSurface(bmpImage);

	//SDL_SaveBMP(pxf, "mat.bmp");

	// Delete the data
	delete[] pDest;

	fclose(fp);

    // Calculate the shadowmap
    CalculateShadowMap();

	ApplyShadow(0,0,Width,Height);

	// Update the draw image
	UpdateDrawImage(0, 0, bmpImage->w, bmpImage->h);

	// Update the minimap
	UpdateMiniMap(true);

    // Calculate the grid
    calculateGrid();

	return true;
}

///////////////////
// Load an original version of a liero leve;
int CMap::LoadOriginal(FILE *fp)
{
	bool Powerlevel = false;
	uchar *palette = NULL;
	uint x,y,n;

	// Validate the liero level
	fseek(fp,0,SEEK_END);
	size_t length = ftell(fp);

	// check for powerlevel
	if(length != 176400 && length != 176402) {
		if(length == 177178)
			Powerlevel = true;
		else {
			// bad file
			fclose(fp);
			return false;
		}
	}

	fseek(fp,0,SEEK_SET);

	// Default is a dirt theme for the background & dirtballs
	if( !New(504,350,"dirt") ) {
		fclose(fp);
		return false;
	}

	// Image type of map
	Type = MPT_IMAGE;

	palette = new uchar[768];
	if( palette == NULL) {
		printf("CMap::LoadOriginal: ERROR: not enough memory for palette\n");
		fclose(fp);
		return false;
	}

	// Load the palette
	if(!Powerlevel) {
		FILE *fpal = OpenGameFile("data/lieropal.act","rb");
		if(!fpal) {
			fclose(fp);
			return false;
		}

		fread(palette,sizeof(uchar),768,fpal);
		fclose(fpal);
	}

	// Load the image map
	uchar *bytearr = new uchar[Width*Height];
	if(bytearr == NULL) {
		printf("CMap::LoadOriginal: ERROR: not enough memory for bytearr\n");
		delete[] palette;
		fclose(fp);
		return false;
	}

	fread(bytearr,sizeof(uchar),Width*Height,fp);

	// Load the palette from the same file if it's a powerlevel
	if(Powerlevel) {
		static char id[11];
		// Load id
		fread(id,sizeof(uchar),10,fp);
		id[10] = '\0';
		if(stricmp(id,"POWERLEVEL") != 0) {
			delete[] palette;
			delete[] bytearr;
			fclose(fp);
			return false;
		}

		// Load the palette
		fread(palette,sizeof(uchar),768,fp);

		// Convert the 6bit colours to 8bit colours
		for(n=0;n<768;n++) {
			float f = (float)palette[n] / 63.0f * 255.0f;
			palette[n] = (int)f;
		}
	}

	// Set the image
	lockFlags();
	n=0;
	for(y=0;y<Height;y++) {
		for(x=0;x<Width;x++) {
			uchar p = bytearr[n];
			uchar type = PX_EMPTY;
			//if(p >= 0 && p <= 255) {

				// Dirt
				if( (p >= 12 && p <= 18) ||
					(p >= 55 && p <= 58) ||
					(p >= 82 && p <= 84) ||
					(p >= 94 && p <= 103) ||
					(p >= 120 && p <= 123) ||
					(p >= 176 && p <= 180))
					type = PX_DIRT;

				// Rock
				else if( (p >= 19 && p <= 29) ||
					(p >= 59 && p <= 61) ||
					(p >= 85 && p <= 87) ||
					(p >= 91 && p <= 93) ||
					(p >= 123 && p <= 125) ||
					p==104)
					type = PX_ROCK;

				PutPixel(bmpImage,x,y, MakeColour(palette[p*3], palette[p*3+1], palette[p*3+2]));
				if(type == PX_EMPTY)
					PutPixel(bmpBackImage,x,y, MakeColour(palette[p*3], palette[p*3+1], palette[p*3+2]));
				SetPixelFlag(x,y,type);
			//}
			n++;
		}
	}
	unlockFlags();

	delete[] palette;
	delete[] bytearr;

	fclose(fp);

	// Update the minimap
	UpdateMiniMap(true);

	// Update the draw image
	UpdateDrawImage(0, 0, bmpImage->w, bmpImage->h);

    // Calculate the total dirt count
    CalculateDirtCount();

    // Calculate the shadowmap
    CalculateShadowMap();

	// Apply shadow
	ApplyShadow(0,0,Width,Height);

    // Calculate the grid
    calculateGrid();

	return true;
}


///////////////////
// DEBUG: Draw the pixel flags
void CMap::DEBUG_DrawPixelFlags(void)
{
	//int x,y;


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

    /*for(y=0; y<nGridRows; y++) {
        for(x=0; x<nGridCols; x++) {

            if(GridFlags[y*nGridCols+x] == PX_EMPTY)
                DrawRectFill(bmpDrawImage,x*nGridWidth*2,y*nGridHeight*2,(x*nGridWidth+nGridWidth)*2,(y*nGridHeight+nGridHeight)*2, MakeColour(0,0,128));
            if(GridFlags[y*nGridCols+x] & PX_DIRT)
                DrawRectFill(bmpDrawImage,x*nGridWidth*2,y*nGridHeight*2,(x*nGridWidth+nGridWidth)*2,(y*nGridHeight+nGridHeight)*2, MakeColour(255,0,0));
            if(GridFlags[y*nGridCols+x] & PX_ROCK)
                DrawRectFill(bmpDrawImage,x*nGridWidth*2,y*nGridHeight*2,(x*nGridWidth+nGridWidth)*2,(y*nGridHeight+nGridHeight)*2, MakeColour(128,128,128));
            //DrawRect(bmpDrawImage,x*nGridWidth*2,y*nGridHeight*2,(x*nGridWidth+nGridWidth)*2,(y*nGridHeight+nGridHeight)*2, 0);
			//DrawImageStretch2(bmpDrawImage,bmpImage,0,0,0,0,bmpImage->w,bmpImage->h);
        }
    }*/
}


///////////////////
// Send the map data to clients
void CMap::Send(CBytestream *bs)
{
	// At the moment, just write the data to a file and compress it to see how big it is
	FILE *fp;
	fp = OpenGameFile("tempmap.dat","wb");

	uint n;

	for(n=0;n<Width*Height;) {
		uchar t = 0;

		for(ushort i=0;i<8;i++) {
			uchar value = (PixelFlags[n++] & PX_EMPTY);
			t |= (value << i);
		}

		fwrite(&t,sizeof(uchar),1,fp);
	}

	// 100 objects
	for(n=0;n<100;n++) {
		uchar b;

		b = (int)(fabs(GetRandomNum())*10.0f);

		unsigned short p1,p2;
		p1 = (int)(fabs(GetRandomNum())*1024.0f);
		p2 = (int)(fabs(GetRandomNum())*1024.0f);


		fwrite(&b,sizeof(uchar),1,fp);
		fwrite(GetEndianSwapped(p1),sizeof(unsigned short),1,fp);
		fwrite(GetEndianSwapped(p2),sizeof(unsigned short),1,fp);

	}
	uchar b;

	// TODO: what the f*?
	for(n=0;n<3;n++)
		fwrite(&b,sizeof(uchar),1,fp);

	fclose(fp);

}


///////////////////
// Shutdown the map
void CMap::Shutdown(void)
{
	lockFlags();

	if(Created) {

		//printf("some created map is shutting down...\n");

		if(bmpImage)
			SDL_FreeSurface(bmpImage);
		bmpImage = NULL;

		if(bmpDrawImage)
			SDL_FreeSurface(bmpDrawImage);
		bmpDrawImage = NULL;

#ifdef _AI_DEBUG
		if (bmpDebugImage)
			SDL_FreeSurface(bmpDebugImage);
		bmpDebugImage = NULL;
#endif

		if(bmpBackImage)
			SDL_FreeSurface(bmpBackImage);
		bmpBackImage = NULL;

        if(bmpShadowMap)
			SDL_FreeSurface(bmpShadowMap);
		bmpShadowMap = NULL;

		if(bmpMiniMap)
			SDL_FreeSurface(bmpMiniMap);
		bmpMiniMap = NULL;

		if(PixelFlags)
			delete[] PixelFlags;
		PixelFlags = NULL;

        if(GridFlags)
            delete[] GridFlags;
        GridFlags = NULL;

        if(AbsoluteGridFlags)
            delete[] AbsoluteGridFlags;
        AbsoluteGridFlags = NULL;


		if(Objects)
			delete[] Objects;
		Objects = NULL;
		NumObjects = 0;

		if (m_pnWater1)
			delete[] m_pnWater1;
		if (m_pnWater2)
			delete[] m_pnWater2;
		m_pnWater1 = NULL;
		m_pnWater2 = NULL;

        if( sRandomLayout.bUsed ) {
            sRandomLayout.bUsed = false;
            if( sRandomLayout.psObjects )
                delete[] sRandomLayout.psObjects;
            sRandomLayout.psObjects = NULL;
        }

	}
	// Safety
	else  {
		bmpImage = NULL;
		bmpDrawImage = NULL;
#ifdef _AI_DEBUG
		bmpDebugImage = NULL;
#endif
		bmpBackImage = NULL;
		bmpShadowMap = NULL;
		bmpMiniMap = NULL;
		PixelFlags = NULL;
		GridFlags = NULL;
		AbsoluteGridFlags = NULL;
		Objects = NULL;
		sRandomLayout.psObjects = NULL;
	}

	Created = false;

	unlockFlags();
}


#ifdef _AI_DEBUG
void CMap::ClearDebugImage() {
	if (bmpDebugImage) {
		DrawRectFill(bmpDebugImage, 0, 0, bmpDebugImage->w, bmpDebugImage->h, COLORKEY(bmpDebugImage));
	}
}
#endif

///////////////////
// Carve a hole
// Returns the number of dirt pixels carved
int CarveHole(CMap *cMap, CVec pos)
{
	int x,y;
	Uint32 Colour = cMap->GetTheme()->iDefaultColour;

	// Go through until we find dirt to throw around
	y = MAX(MIN((int)pos.y, (int)cMap->GetHeight() - 1), 0);

	for(x=(int)pos.x-2; x<=(int)pos.x+2; x++) {
		// Clipping
		if(x < 0) continue;
		if((uint)x >= cMap->GetWidth())	break;

		if(cMap->GetPixelFlag(x,y) & PX_DIRT) {
			Colour = GetPixel(cMap->GetImage(),x,(int)pos.y);
			for(short n=0; n<3; n++)
				SpawnEntity(ENT_PARTICLE,0,pos,CVec(GetRandomNum()*30,GetRandomNum()*10),Colour,NULL);
			break;
		}
	}

	// Just carve a hole for the moment
	return cMap->CarveHole(3,pos);
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
int CheckCollision(float dt, CVec pos, CVec vel, uchar checkflags, CMap *map)
{
/*	set_col_and_break col_action;
	col_action = fastTraceLine(trg, pos, map, checkflags, col_action);
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
