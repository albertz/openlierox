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

#include "defs.h"
#include "LieroX.h"
#include "GfxPrimitives.h"
#include "FindFile.h"
#include "StringUtils.h"


///////////////////
// Create a new map
int CMap::New(uint _width, uint _height, const std::string& _theme)
{
	if(Created)
		Shutdown();

	Width = _width;
	Height = _height;
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
	int n,x,y,i;
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
	for(n=0;n<nNumRocks;n++) {
		x = (int)(fabs(GetRandomNum()) * (float)Width);
		y = (int)(fabs(GetRandomNum()) * (float)Height);
		i = (int)(fabs(GetRandomNum()) * (float)Theme.NumStones);

        // Store the object
        sRandomLayout.psObjects[objCount].Type = OBJ_STONE;
        sRandomLayout.psObjects[objCount].Size = i;
        sRandomLayout.psObjects[objCount].X = x;
        sRandomLayout.psObjects[objCount].Y = y;
        objCount++;

		PlaceStone(i,CVec((float)x,(float)y));
	}

	// Spawn 20 random misc
	for(n=0;n<nNumMisc;n++) {
		x = (int)(fabs(GetRandomNum()) * (float)Width);
		y = (int)(fabs(GetRandomNum()) * (float)Height);
		i = (int)(fabs(GetRandomNum()) * (float)Theme.NumMisc);

        // Store the object
        sRandomLayout.psObjects[objCount].Type = OBJ_MISC;
        sRandomLayout.psObjects[objCount].Size = i;
        sRandomLayout.psObjects[objCount].X = x;
        sRandomLayout.psObjects[objCount].Y = y;
        objCount++;

		PlaceMisc(i,CVec((float)x,(float)y));
	}


	// Spawn 10 random holes
	for(n=0;n<nNumHoles;n++) {
		x = (int)(fabs(GetRandomNum()) * (float)Width);
		y = (int)(fabs(GetRandomNum()) * (float)Height);

		// Spawn a hole group of holes around this hole
		for(int i=0;i<nNumHoleGroup;i++) {
			int a = (int) (GetRandomNum() * (float)15);
			int b = (int) (GetRandomNum() * (float)15);

            // Store the object
            sRandomLayout.psObjects[objCount].Type = OBJ_HOLE;
            sRandomLayout.psObjects[objCount].Size = 4;
            sRandomLayout.psObjects[objCount].X = x+a;
            sRandomLayout.psObjects[objCount].Y = y+b;
            objCount++;

			CarveHole(4,CVec( (float)(x+a), (float)(y+b)));
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
		for( int i=0; i<psRandom->nNumObjects; i++ ) {
			if( psRandom->psObjects ) {

				switch(psRandom->psObjects[i].Type) {

					// Stone
					case OBJ_STONE:
						PlaceStone(psRandom->psObjects[i].Size,CVec((float)(psRandom->psObjects[i].X), (float)(psRandom->psObjects[i].Y)));
						break;

					// Misc
					case OBJ_MISC:
						PlaceMisc(psRandom->psObjects[i].Size,CVec((float)(psRandom->psObjects[i].X), (float)(psRandom->psObjects[i].Y)));
						break;

					// Hole
					case OBJ_HOLE:
						CarveHole(psRandom->psObjects[i].Size,CVec((float)(psRandom->psObjects[i].X), (float)(psRandom->psObjects[i].Y)));
						break;
				}
			}
		}
	}

    // Calculate the total dirt count
    CalculateDirtCount();

	// No need to update minimap, because it's updated in CarveHole and PlaceXXX
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
		SDL_SetColorKey(Theme.bmpHoles[n], SDL_SRCCOLORKEY, SDL_MapRGB(Theme.bmpHoles[n]->format,0,0,0));
	}

	// Calculate the default colour from a non-pink, non-black colour in the hole image
	Theme.iDefaultColour = GetPixel(Theme.bmpFronttile,0,0);
	SDL_Surface *hole = Theme.bmpHoles[0];
	Uint32 pink = SDL_MapRGB(Theme.bmpHoles[0]->format,255,0,255);
	Uint32 black = SDL_MapRGB(Theme.bmpHoles[0]->format,0,0,0);
	Uint32 pixel = 0;
	if(hole) {
		for(y=0; y<hole->h; y++) {
			for(x=0; x<hole->w; x++) {
				pixel = GetPixel(hole,x,y);
				if(pixel != 0 && pixel != pink)  {
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
		d_printf("CMap::findRandomTheme(): no themes found\n");
		d_printf("                         Defaulting to \"dirt\"\n");
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
	DrawRectFill(bmpDebugImage,0,0,bmpDebugImage->w,bmpDebugImage->h,COLORKEY(bmpDebugImage));
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

	bmpMiniMap = gfxCreateSurface(128, 96);
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
    uint i = x/nGridWidth;
    uint j = y/nGridHeight;

    // Clamp it
    i = MIN(i,nGridCols-1);
    j = MIN(j,nGridRows-1);

    x = i*nGridWidth;
    y = j*nGridHeight;

    uchar *cell = GridFlags + j*nGridCols + i;
    uchar *abs_cell = AbsoluteGridFlags + j*nGridCols + i;

    // Skip empty cells?
    if(*cell == PX_EMPTY && bSkipEmpty)
        return;

    int dirtCount = 0;
    int rockCount = 0;

    // Go through every pixel in the cell and get a solid flag count
    for(uint b=y; b<y+nGridHeight; b++) {
        // Clipping
        if(b>=Height)
            break;
        if(x>=Width)
            break;

        uchar *pf = PixelFlags + b*Width + x;
        for(uint a=x; a<x+nGridWidth; a++, pf++) {

            // Clipping
            if(a>=Width)
                break;

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
	DrawImageStretch(bmpDrawImage,bmpImage,0,0);
	
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
	const uint size = Width*Height;

	for (n=0;n<size;n++)
		nTotalDirtCount += (PixelFlags[n] & PX_DIRT) ? 1 : 0;
}



///////////////////
// Draw the map
void CMap::Draw(SDL_Surface *bmpDest, CViewport *view)
{
		//DEBUG_DrawPixelFlags();
		DrawImageAdv(bmpDest, bmpDrawImage, view->GetWorldX()*2, view->GetWorldY()*2,view->GetLeft(),view->GetTop(),view->GetWidth()*2,view->GetHeight()*2);
#ifdef _AI_DEBUG
		//if (GetKeyboard()->KeyDown[SDLK_F2])
		//	DrawImageStretch2(bmpDebugImage,bmpShadowMap,0, 0,0,0,Width,Height);
		DrawImageAdv(bmpDest, bmpDebugImage, view->GetWorldX()*2, view->GetWorldY()*2,view->GetLeft(),view->GetTop(),view->GetWidth()*2,view->GetHeight()*2);
#endif
}


///////////////////
// Draw an object's shadow
void CMap::DrawObjectShadow(SDL_Surface *bmpDest, SDL_Surface *bmpObj, uint sx, uint sy, uint w, uint h, CViewport *view, uint wx, uint wy)
{
	// TODO: optimize

    int v_wx = view->GetWorldX();
	int v_wy = view->GetWorldY();
	int l = view->GetLeft();
	int t = view->GetTop();

    // Drop the shadow
    static const int Drop = 3;
    wx += Drop;
    wy += Drop;

    // Clipping rectangle
	SDL_Rect rect = bmpDest->clip_rect;
	int	c_y = rect.y;
	int c_x = rect.x;
	int c_x2 = rect.x + rect.w;
	int c_y2 = rect.y + rect.h;


    int dtx = (wx - v_wx)*2;
    int dty = (wy - v_wy)*2;

	// Screen bytes per pixel
	int screenbpp = SDL_GetVideoSurface()->format->BytesPerPixel;

	// Lock the surfaces
	if (SDL_MUSTLOCK(bmpDest))
		SDL_LockSurface(bmpDest);
	if (SDL_MUSTLOCK(bmpObj))
		SDL_LockSurface(bmpObj);
	if (SDL_MUSTLOCK(bmpShadowMap))
		SDL_LockSurface(bmpShadowMap);

	static int x,y,dx,dy,i,j;
	uchar *pf = NULL;

	Uint8 *destpix,*srcpix,*objpix;
	Uint8 *DestPixel,*SrcPixel,*ObjPixel;

	DestPixel = (Uint8 *)bmpDest->pixels + ((dty+t)*bmpDest->pitch+(dtx+l)*screenbpp);
	SrcPixel = (Uint8*)bmpShadowMap->pixels + (wy * bmpShadowMap->pitch + wx*screenbpp);
	ObjPixel = (Uint8 *)bmpObj->pixels + (sy*bmpObj->pitch+sx*screenbpp);

	/*int y_start = sy;
	if (wy < 0) y_start -= wy;
	if (dty+t < c_y) y_start += c_y-dty-t;
	int y_end = (int)(sy+h);
	if ((int)(dy+h/2) >= Height)  {
		if (dty+t+h >= c_y2) y_end -= MAX((dy+h/2)-Height,c_y2-dty-t-h);
		else y_end -= (dy+h/2)-Height;
	} else if (dty+t+h >= c_y2) {
		y_end -= c_y2-dty-t-h;
	}

	int x_start = sx;
	if (wx < 0) x_start = -wx;
	if (dtx+l < c_x) x_start += c_x-dtx-l;
	int x_end = (int)(sx+w);
	if ((int)(dx+w/2) >= Width)  {
		if (dtx+l+w >= c_x2) x_end -= MAX((dx+w/2)-Width,c_x2-dtx-l-w);
		else x_end -= (dx+w/2)-Width;
	} else if (dtx+l+w >= c_x2) {
		x_end -= c_x2-dtx-l-w;
	}*/

	for( y=/*y_start*/sy,dy=wy/*+y_start*/,j=0; y</*y_end*/(int)(sy+h); y++,j++, dy += (wy+j)&1,DestPixel+=bmpDest->pitch,SrcPixel+=((wy+j)&1)*bmpShadowMap->pitch,ObjPixel+=bmpObj->pitch ) {
		// World Clipping
		if(dy < 0) continue;
		if((uint)dy >= Height) break;

		// Screen clipping
		if( dty+t+j < c_y ) continue;
		if( dty+t+j >= c_y2 ) break;

		// Get the first pixel adresses for each of the three images
		pf = &PixelFlags[dy * Width + wx];
		srcpix = SrcPixel;
		destpix = DestPixel;
		objpix = ObjPixel;

		for( x=/*x_start*/sx,dx=wx/*+x_start*/,i=0; x<(int)(sx+w)/*x_end*/; x++,i++, dx+=(wx+i)&1, pf+=(wx+i)&1, destpix+=screenbpp, objpix+=screenbpp, srcpix+=((wx+i)&1)*screenbpp ) {

			// Clipping
			if(dx < 0) continue;
			if((uint)dx >= Width) break;

			// Is this pixel solid?
			if( !(*pf & PX_EMPTY) ) continue;

			// Screen clipping
			if( dtx+l+i < c_x ) continue;
			if( dtx+l+i >= c_x2 ) continue;

			// Put the pixel, if it's not transparent
			if (memcmp(objpix,&COLORKEY(bmpObj),screenbpp))
				memcpy(destpix,srcpix,screenbpp);
		}
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
void CMap::DrawPixelShadow(SDL_Surface *bmpDest, CViewport *view, uint wx, uint wy)
{
    uint v_wx = view->GetWorldX();
	uint v_wy = view->GetWorldY();
	uint l = view->GetLeft();
	uint t = view->GetTop();

    static const int Drop = 3;
    wx += Drop;
    wy += Drop;

    // Clipping
    if( wx<0 || wy<0 ) return;
    if( wx>=Width || wy>=Height ) return;

    int x = (wx - v_wx)*2;
    int y = (wy - v_wy)*2;

    if( PixelFlags[wy * Width + wx] & PX_EMPTY )
        DrawRectFill( bmpDest, x+l, y+t, x+l+2, y+t+2, GetPixel(bmpShadowMap,wx,wy) );
}


///////////////////
// Carve a hole in the map
//
// Returns the number of dirt pixels carved
int CMap::CarveHole(int size, CVec pos)
{
	SDL_Surface *hole;
	int dy, sx,sy;
	//int bx,by;
	int x,y;
	int w,h;

    int nNumDirt = 0;

	if(size < 0 || size > 4) {
		// Just clamp it and continue
		size = MAX(size,0);
		size = MIN(size,4);
	}


	// Calculate half
	hole = Theme.bmpHoles[size];
	if (!hole)
		return 0;
	w = hole->w;
	h = hole->h;

	Uint32 pink = SDL_MapRGB(hole->format,255,0,255);

	sx = (int)pos.x-(hole->w>>1);
	sy = (int)pos.y-(hole->h>>1);

	if(SDL_MUSTLOCK(hole))
		SDL_LockSurface(hole);
	if(SDL_MUSTLOCK(bmpImage))
		SDL_LockSurface(bmpImage);


	// Calculate the clipping bounds, so we don't have to check each loop then
	int clip_h = MIN(sy+hole->h,bmpImage->h)-sy;
	int clip_w = MIN(sx+hole->w,bmpImage->w)-sx;
	int clip_y = 0; 
	int clip_x = 0; 
	if (sy<0) 
		clip_y = abs(sy);
	if (sx<0) 
		clip_x = abs(sx);

	// Some temps to make the loop faster
	int src_tmp = clip_x*hole->format->BytesPerPixel;
	int pf_tmp = sx+clip_x; 
	int dst_tmp = (sx+clip_x)*bmpImage->format->BytesPerPixel;
	static Uint32 black = 0;

	// Pixels
	Uint8 *srcpix;
	Uint8 *SrcRow = (Uint8 *)hole->pixels+(clip_y)*hole->pitch;
	Uint8 *dstpix;
	Uint8 *DstRow = (Uint8 *)bmpImage->pixels+(sy+clip_y)*bmpImage->pitch;
	uchar *px;


	lockFlags();

	// Go through the pixels in the hole, setting the flags to empty
	for(y=clip_y,dy=MAX((int)0,sy);y<clip_h;y++,dy++,SrcRow+=hole->pitch,DstRow+=bmpImage->pitch) {

		srcpix = SrcRow+src_tmp;
		px = PixelFlags + dy * Width + pf_tmp;
		dstpix = DstRow+dst_tmp;

		for(x=clip_x;x<clip_w;x++,px++,srcpix+=hole->format->BytesPerPixel,dstpix+=bmpImage->format->BytesPerPixel) {

			if (*px & PX_ROCK) continue;

			// Set the flag to empty
			if(!memcmp(srcpix,&pink,hole->format->BytesPerPixel)) {

				// Increase the dirt count
				nNumDirt+=((*px & PX_DIRT) != 0) ? 1 : 0;

				*px = PX_EMPTY;

			// Put pixels that are not black/pink (eg, brown)
			} else {	
				if((*px & PX_DIRT) != 0 && memcmp(srcpix,&black,hole->format->BytesPerPixel))
					memcpy(dstpix,srcpix,bmpImage->format->BytesPerPixel);
			}
		}
	}

	unlockFlags();

	if(SDL_MUSTLOCK(hole))
		SDL_UnlockSurface(hole);
	if(SDL_MUSTLOCK(bmpImage))
		SDL_UnlockSurface(bmpImage);

	// If nothing has been carved, we don't have to bother with updating the state
	if (!nNumDirt)
		return 0;


	// Go through and clean up the hole
	sx-=5;
	sy-=5;

	// Calculate the clipping bounds, so we don't have to check each loop then
	clip_h = MIN(sy+hole->h+20,bmpImage->h)-sy;
	clip_w = MIN(sx+hole->w+20,bmpImage->w)-sx;
	clip_y = 0; 
	clip_x = 0; 
	if (sy<0) 
		clip_y = abs(sy);
	if (sx<0) 
		clip_x = abs(sx);


	if(SDL_MUSTLOCK(bmpImage))
		SDL_LockSurface(bmpImage);
	if(SDL_MUSTLOCK(bmpBackImage))
		SDL_LockSurface(bmpBackImage);

	lockFlags();

	// Some temps to make the loop faster
	src_tmp = (sx+clip_x)*bmpBackImage->format->BytesPerPixel;
	pf_tmp = sx+clip_x; 
	dst_tmp = (sx+clip_x)*bmpImage->format->BytesPerPixel;

	// Initialize pixels
	SrcRow = (Uint8 *)bmpBackImage->pixels + (sy+clip_y)*bmpBackImage->pitch;
	DstRow = (Uint8 *)bmpImage->pixels + (sy+clip_y)*bmpImage->pitch;

	for(y=clip_y;y<clip_h;y++,SrcRow+=bmpBackImage->pitch,DstRow+=bmpImage->pitch) {

		srcpix = SrcRow+src_tmp;
		dstpix = DstRow+dst_tmp;
		px = PixelFlags + (sy+y) * Width + pf_tmp;

		for(x=clip_x;x<clip_w;x++,px++,srcpix+=bmpBackImage->format->BytesPerPixel,dstpix+=bmpImage->format->BytesPerPixel) {		
			memcpy(dstpix,srcpix,bmpImage->format->BytesPerPixel*((*px & PX_EMPTY) != 0)); // when not PX_EMPTY, copies 0 bytes
		}
	}

	unlockFlags();

	if(SDL_MUSTLOCK(bmpImage))
		SDL_UnlockSurface(bmpImage);
	if(SDL_MUSTLOCK(bmpBackImage))
		SDL_UnlockSurface(bmpBackImage);

	// Apply a shadow
	ApplyShadow(sx-5,sy-5,w+25,h+25);

    // Recalculate the grid
    int hw = w/2;
    int hh = h/2;
    lockFlags();
    for(y=sy-hh; y<sy+h+hh; y+=nGridHeight/2) {
        for(x=sx-hw; x<sx+w+hw; x+=nGridWidth/2) {
            calculateGridCell(x, y, true);
        }
    }
	unlockFlags();

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
	DrawImageStretch2(bmpDrawImage,bmpImage,draw_x,draw_y,draw_x*2,draw_y*2,w+25,h+25);

	UpdateMiniMapRect(MAX(0,sx-5), MAX(0,sy-5), w+25, h+25);
	//bMiniMapDirty = true;

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
	int bx,by;
	int x,y;
	int w,h;
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
	Uint32 pink = SDL_MapRGB(hole->format,255,0,255);
	w = hole->w;
	h = hole->h;

	sx = (int)pos.x-(hole->w>>1);
	sy = (int)pos.y-(hole->h>>1);


	if(SDL_MUSTLOCK(hole))
		SDL_LockSurface(hole);

	Uint8 *p;
	uchar *px;
	Uint8 *p2;
	Uint32 tmp=0;

	int screenbpp = SDL_GetVideoSurface()->format->BytesPerPixel;

	lockFlags();

	// Go through the pixels in the hole, setting the flags to dirt
	for(y=0,dy=sy,by=16;y<hole->h;y++,dy++,by++) {

		// Clipping
		if(dy<0)			continue;
		if(dy>=bmpImage->h)	break;

		p = (Uint8 *)hole->pixels + y * hole->pitch;
		px = PixelFlags + dy * Width + sx;
		p2 = (Uint8 *)bmpImage->pixels + dy * bmpImage->pitch + sx * bmpImage->format->BytesPerPixel;

		for(x=0,dx=sx,bx=16;x<hole->w;x++,dx++,bx++) {

			// Clipping
			if(dx<0) {	p+=screenbpp; p2+=screenbpp; px++;	continue; }
			if(dx>=bmpImage->w)				break;

			//pixel = *(Uint16 *)p;
			// TODO: endian
			pixel = 0;
			memcpy(&pixel,p,screenbpp); // bpp independent
			flag = *(uchar *)px;

			int ix = dx % Theme.bmpFronttile->w;
			int iy = dy % Theme.bmpFronttile->h;

			// Set the flag to empty
			if(pixel == pink && !(flag & PX_ROCK)) {
                if( flag & PX_EMPTY )
                    nDirtCount++;

				*(uchar *)px = PX_DIRT;

				// Place the dirt image
				tmp = GetPixel(Theme.bmpFronttile,ix,iy);
				// TODO: endian
				memcpy(p2,&tmp,screenbpp);
				//*(Uint16 *)p2 = (Uint16)GetPixel(Theme.bmpFronttile,ix,iy);
			}

			// Put pixels that are not black/pink (eg, brown)
            if(pixel != 0 && pixel != pink && flag & PX_EMPTY) {
				//*(Uint16 *)p2 = (Uint16)pixel;
				// TODO: endian
				memcpy(p2,&pixel,screenbpp);
                *(uchar *)px = PX_DIRT;
                nDirtCount++;
            }

			p+=screenbpp;
			p2+=screenbpp;
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
	DrawImageStretch2(bmpDrawImage,bmpImage,draw_x,draw_y,draw_x*2,draw_y*2,w+25,h+25);

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
	int bx,by;
	int x,y;
	int w,h;
	Uint32 pixel;
	uchar flag;
    Uint32 green = SDL_MapRGB(bmpGreenMask->format,0,255,0);
	Uint32 pink = SDL_MapRGB(bmpGreenMask->format,255,0,255);
    Uint32 greens[4] = {SDL_MapRGB(bmpImage->format,148,136,0),
                        SDL_MapRGB(bmpImage->format,136,124,0),
                        SDL_MapRGB(bmpImage->format,124,112,0),
                        SDL_MapRGB(bmpImage->format,116,100,0)};

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

	int screenbpp = SDL_GetVideoSurface()->format->BytesPerPixel;

	lockFlags();

	// Go through the pixels in the hole, setting the flags to dirt
	for(y=0,dy=sy,by=16; y<h; y++,dy++,by++) {

		// Clipping
		if(dy<0)			continue;
		if(dy>=bmpImage->h)	break;

		p = (Uint8 *)bmpGreenMask->pixels + y * bmpGreenMask->pitch;
		px = PixelFlags + dy * Width + sx;
		p2 = (Uint8 *)bmpImage->pixels + dy * bmpImage->pitch + sx * bmpImage->format->BytesPerPixel;

		for(x=0,dx=sx,bx=16; x<w; x++,dx++,bx++) {

			// Clipping
			if(dx<0) {	p+=screenbpp; p2+=screenbpp; px++;	continue; }
			if(dx>=bmpImage->w)				break;

			//pixel = *(Uint16 *)p;
			pixel = 0;
			memcpy(&pixel,p,screenbpp);
			flag = *(uchar *)px;

			// Set the flag to empty
			if(pixel == green && flag & PX_EMPTY) {
				*(uchar *)px = PX_DIRT;
                nGreenCount++;

                // Place a random green pixel
                Uint32 gr = greens[ GetRandomInt(3) ];

				// Place the dirt image
				//*(Uint16 *)p2 = (Uint16)gr;
				// TODO: endian
				memcpy(p2,&gr,screenbpp);
			}

			// Put pixels that are not green/pink (eg, dark green)
            if(pixel != green && pixel != pink && flag & PX_EMPTY) {
				//*(Uint16 *)p2 = (Uint16)pixel;
				// TODO: endian
				memcpy(p2,&pixel,screenbpp);
                *(uchar *)px = PX_DIRT;
                nGreenCount++;
            }

			p+=screenbpp;
			p2+=screenbpp;
			px++;
		}
	}

	unlockFlags();

	if(SDL_MUSTLOCK(bmpGreenMask))
		SDL_UnlockSurface(bmpGreenMask);

	if (!bmpDrawImage)
		return nGreenCount;

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
	DrawImageStretch2(bmpDrawImage,bmpImage,draw_x,draw_y,draw_x*2,draw_y*2,w+25,h+25);

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

    return nGreenCount;
}


///////////////////
// Apply a shadow to an area
void CMap::ApplyShadow(uint sx, uint sy, uint w, uint h)
{
	// Draw shadows?
	if(!tLXOptions->iShadows)
		return;

	static const ushort Drop = 3;
	uint x,y,n;
	uchar *px;
	uchar *p;
	uint ox,oy;
	uchar flag;

	Uint8 *pixel,*src;

	int screenbpp = SDL_GetVideoSurface()->format->BytesPerPixel;

	if(SDL_MUSTLOCK(bmpImage))
		SDL_LockSurface(bmpImage);

	lockFlags();

	for(y=sy;y<sy+h;y++) {

		// Clipping
//		if(y < 0) continue; else 
		if(y>=Height)	break;

		px = PixelFlags + y * Width + sx;

		for(x=sx;x<sx+w;x++) {

			// Clipping
//			if(x<0) {	px++;	continue; } else
			if(x>=Width)	break;

			flag = *(uchar *)px;

			// Edge hack
			//if(x==0 || y==0 || x==Width-1 || y==Height-1)
				//flag = PX_EMPTY;

			if(!(flag & PX_EMPTY)) {
				ox = x+1; oy = y+1;

				// Draw the shadow
				for(n=0;n<Drop;n++) {

					// Clipping
					if(ox>=Width)	break;
					if(oy>=Height)	break;

					p = PixelFlags + oy * Width + ox;
					if(!(*(uchar *)p & PX_EMPTY))
						break;

                    Uint32 offset = oy*bmpImage->pitch + ox*screenbpp;
                    pixel = (Uint8 *)bmpImage->pixels + offset;
                    src = (Uint8 *)bmpShadowMap->pixels + offset;
                    //*pixel = *src;
					memcpy(pixel,src,screenbpp);

					//PutPixel(bmpImage,ox,oy,0);

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
	SDL_Surface *tmp = gfxCreateSurface(bmpImage->w,bmpImage->h);
	if (!tmp)
		return;
	SDL_BlitSurface(bmpBackImage,NULL,bmpShadowMap,NULL);
	SDL_SetAlpha(tmp,SDL_SRCALPHA | SDL_RLEACCEL, 100);
	SDL_BlitSurface(tmp,NULL,bmpShadowMap,NULL);
	SDL_FreeSurface(tmp);
	
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
		d_printf("Bad stone size\n");
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

	Uint32 pink = COLORKEY(stone);

	sx = (int)pos.x-(stone->w>>1);
	sy = (int)pos.y-(stone->h>>1);

	// Blit the stone to the surface
	DrawImage(bmpImage,stone,sx,sy);

	if(SDL_MUSTLOCK(stone))
		SDL_LockSurface(stone);

	int screenbpp = SDL_GetVideoSurface()->format->BytesPerPixel;

	lockFlags();

	// Calculate the clipping bounds, so we don't have to check each loop then
	short clip_h = MIN(sy+stone->h,bmpImage->h)-sy;
	short clip_w = MIN(sx+stone->w,bmpImage->w)-sx;
	short clip_y = 0; 
	short clip_x = 0; 
	if (sy<0) 
		clip_y = abs(sy);
	if (sx<0) 
		clip_x = abs(sx);

	// Pixels
	Uint8 *p = NULL;
	Uint8 *PixelRow = (Uint8 *)stone->pixels + clip_y*stone->pitch;
	uchar *px = PixelFlags;
	short pf_tmp = MAX((short)0,sx);
	short p_tmp = clip_x*stone->format->BytesPerPixel;

	// Go through the pixels in the stone and update pixel flags
	for(y=clip_y,dy=MAX((short)0,sy);y<clip_h;y++,dy++,PixelRow+=stone->pitch) {

		p = PixelRow+p_tmp;
		px = PixelFlags + dy * Width + pf_tmp;

		for(x=clip_x;x<clip_w;x++) {

			// Rock?
			if (memcmp(p,&pink,screenbpp))  {
				*(uchar *)px = PX_ROCK;
			}

			p+=stone->format->BytesPerPixel;
			px++;
		}
	}

	unlockFlags();

	if(SDL_MUSTLOCK(stone))
		SDL_UnlockSurface(stone);

	// Apply the shadow
	ApplyShadow(sx-5,sy-5,w+10,h+10);

	// Update the draw image
	short draw_x = MAX(sx-5,0);
	short draw_y = MAX(sy-5,0);
	DrawImageStretch2(bmpDrawImage,bmpImage,draw_x,draw_y,draw_x*2,draw_y*2,stone->w+10,stone->h+10);

    // Calculate the total dirt count
    CalculateDirtCount();

	// Update the minimap rectangle
	UpdateMiniMapRect(draw_x, draw_y, stone->w+10, stone->h+10);
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
		d_printf("Bad misc size\n");
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

	Uint32 pink = COLORKEY(misc);

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
		clip_y = abs(sy);
	if (sx<0) 
		clip_x = abs(sx);

	Uint8 *p = NULL;
	Uint8 *PixelRow = (Uint8 *)misc->pixels+clip_y*misc->pitch;
	uchar *px = PixelFlags;

	// Temps for better performance
	short pf_tmp = MAX((short)0,sx);
	short p_tmp = clip_x*misc->format->BytesPerPixel;
	short dx_tmp = MAX((short)0,sx);

	// Go through the pixels in the misc item
	for(y=clip_y,dy=MAX((short)0,sy);y<clip_h;y++,dy++,PixelRow+=misc->pitch) {

		p = PixelRow+p_tmp;
		px = PixelFlags + dy * Width + pf_tmp;

		for(x=clip_x,dx=dx_tmp;x<clip_w;dx++,x++) {

			// Put the pixel down
			if(memcmp(p,&pink,misc->format->BytesPerPixel) && *px & PX_DIRT) {
				static Uint32 tmp;
				tmp = 0;
				memcpy(&tmp,p,misc->format->BytesPerPixel);

				PutPixel(bmpImage,dx,dy,tmp);
				*(uchar *)px = PX_DIRT;
			}

			p+=misc->format->BytesPerPixel;
			px++;
		}
	}

	unlockFlags();

	if(SDL_MUSTLOCK(misc))
		SDL_UnlockSurface(misc);

	// Update the draw image
	short draw_x = MAX(sx-5,0);
	short draw_y = MAX(sy-5,0);
	DrawImageStretch2(bmpDrawImage,bmpImage,draw_x,draw_y,draw_x*2,draw_y*2,clip_w+10,clip_h+10);

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
	float xstep,ystep;
	float mx,my;
	uint mmx,mmy;
	uint mw = bmpMiniMap->w;
	uint mh = bmpMiniMap->h;

	if(!bMiniMapDirty && !force)
		return;


	// Calculate steps
	xstep = (float)Width/ (float)mw;
	ystep = (float)Height / (float)mh;

	// Lock
	if (SDL_MUSTLOCK(bmpImage))
		SDL_LockSurface(bmpImage);
	if (SDL_MUSTLOCK(bmpMiniMap))
		SDL_LockSurface(bmpMiniMap);

	Uint8 *sp,*tp,*tmp1,*tmp2;

	for(my=0,mmy=0;my<Height;my+=ystep,mmy++) {

		if(mmy >= mh)
			break;

		// Save the pointer to the first pixel in current line to make the loop faster
		tmp1 = (Uint8 *)bmpImage->pixels + (int)my*bmpImage->pitch;
		tmp2 = (Uint8 *)bmpMiniMap->pixels + mmy*bmpMiniMap->pitch;

		for(mx=0,mmx=0;mx<Width;mx+=xstep,mmx++) {

			if(mmx >= mw)
				break;

			// Copy the pixel
			sp = tmp1+(int)mx*bmpImage->format->BytesPerPixel;
			tp = tmp2+mmx*bmpMiniMap->format->BytesPerPixel;

			// TODO: endian
			memcpy(tp,sp,bmpMiniMap->format->BytesPerPixel);

			//PutPixel(bmpMiniMap,mmx,mmy,GetPixel(bmpImage,(int)mx,(int)my));  // Slow
		}
	}

	// Unlock
	if (SDL_MUSTLOCK(bmpImage))
		SDL_UnlockSurface(bmpImage);
	if (SDL_MUSTLOCK(bmpMiniMap))
		SDL_UnlockSurface(bmpMiniMap);

	// Not dirty anymore
	bMiniMapDirty = false;
}

///////////////////
// Update an area of the minimap
// X, Y, W and H apply to the bmpImage, not bmpMinimap
void CMap::UpdateMiniMapRect(ushort x, ushort y, ushort w, ushort h)
{
	float xstep,ystep;
	float mx,my;  // bmpImage coordinates
	ushort mmx,mmy;  // bmpMiniMap coordinates
	ushort mw = bmpMiniMap->w;
	ushort mh = bmpMiniMap->h;

	// Calculate steps
	xstep = (float)Width/ (float)mw;
	ystep = (float)Height / (float)mh;

	// Lock
	if (SDL_MUSTLOCK(bmpImage))
		SDL_LockSurface(bmpImage);
	if (SDL_MUSTLOCK(bmpMiniMap))
		SDL_LockSurface(bmpMiniMap);

	Uint8 *sp,*tp,*tmp1,*tmp2;
	ushort mmx_tmp = (ushort)((float)x/xstep);
	float mx_tmp = (float)((short)(x/xstep)*xstep);  // This makes sure the shrink will be the same as in UpdateMinimap
	ushort y2 = y+h;
	ushort x2 = x+w;

	for(my=(float)((short)(y/ystep)*ystep),mmy=(short)((float)y/ystep);my<y2;my+=ystep,mmy++) {

		if(mmy >= mh)
			break;

		// Save the pointer to the first pixel in current line to make the loop faster
		tmp1 = (Uint8 *)bmpImage->pixels + (short)my*bmpImage->pitch;
		tmp2 = (Uint8 *)bmpMiniMap->pixels + mmy*bmpMiniMap->pitch;

		for(mx=mx_tmp,mmx=mmx_tmp;mx<x2;mx+=xstep,mmx++) {

			if(mmx >= mw)
				break;

			// Copy the pixel
			sp = tmp1+(short)mx*bmpImage->format->BytesPerPixel;
			tp = tmp2+mmx*bmpMiniMap->format->BytesPerPixel;

			memcpy(tp,sp,bmpMiniMap->format->BytesPerPixel);

			//PutPixel(bmpMiniMap,mmx,mmy,GetPixel(bmpImage,(int)mx,(int)my));  // Slow
		}
	}

	// Unlock
	if (SDL_MUSTLOCK(bmpImage))
		SDL_UnlockSurface(bmpImage);
	if (SDL_MUSTLOCK(bmpMiniMap))
		SDL_UnlockSurface(bmpMiniMap);

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
	static byte dr,dg,db;
	static Uint32 col;
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
	if(!New(Width, Height, Theme_Name)) {
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
			uchar value = PixelFlags[n++] & PX_EMPTY;
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
	Uint8 *curpixel = (Uint8 *)bmpBackImage->pixels;
	Uint8 *PixelRow = curpixel;

	// TODO: check if pDest is big enough

	// Load the back image
	for (y=0;y<Height;y++,PixelRow+=bmpBackImage->pitch)  {
		curpixel = PixelRow;
		for (x=0;x<Width;x++,curpixel+=bmpBackImage->format->BytesPerPixel)  {
			curcolor = SDL_MapRGB(bmpBackImage->format,pDest[p],pDest[p+1],pDest[p+2]);
			p+=3;
			memcpy(curpixel,&curcolor,bmpBackImage->format->BytesPerPixel);
		}
	}

	// Load the front image
	curpixel = (Uint8 *)bmpImage->pixels;
	PixelRow = curpixel;
	for (y=0;y<Height;y++,PixelRow+=bmpImage->pitch)  {
		curpixel = PixelRow;
		for (x=0;x<Width;x++,curpixel+=bmpImage->format->BytesPerPixel)  {
			curcolor = SDL_MapRGB(bmpImage->format,pDest[p],pDest[p+1],pDest[p+2]);
			p+=3;
			memcpy(curpixel,&curcolor,bmpImage->format->BytesPerPixel);
		}
	}

	//SDL_SaveBMP(bmpImage, "image.bmp");
	//SDL_SaveBMP(bmpBackImage, "backimage.bmp");

	// TEMP
	//SDL_PixelFormat *fmt = bmpImage->format;
	//SDL_Surface *pxf = SDL_CreateRGBSurface(iSurfaceFormat, Width, Height, fmt->BitsPerPixel,
	//								fmt->Rmask, fmt->Gmask, fmt->Bmask, fmt->Amask);

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
			memcpy(curpixel,backpixel,bmpImage->format->BytesPerPixel*(PixelFlags[n] & PX_EMPTY)); // If the pixelflag isn't empty, copies 0 bytes
			nTotalDirtCount += (PixelFlags[n] & PX_DIRT) ? 1 : 0;
			n++;
			/*if(t & PX_ROCK)
				PutPixel(pxf, x,y, MakeColour(128,128,128));
			else if(t & PX_DIRT)
				PutPixel(pxf, x,y, MakeColour(128,64,64));*/
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
	DrawImageStretch2(bmpDrawImage,bmpImage,0,0,0,0,bmpImage->w,bmpImage->h);

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

/*
	// Dump the palette
	for(n=0;n<256;n++) {
		//printf("Index: %d =  %d, %d, %d\n",n,palette[n*3], palette[n*3+1], palette[n*3+2]);
	}
*/

	// Apply shadow
	ApplyShadow(0,0,Width,Height);

	delete[] palette;
	delete[] bytearr;

	fclose(fp);

	// Update the draw image
	DrawImageStretch2(bmpDrawImage,bmpImage,0,0,0,0,bmpImage->w,bmpImage->h);

    // Calculate the total dirt count
    CalculateDirtCount();

    // Calculate the shadowmap
    CalculateShadowMap();

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

				case PX_EMPTY:	PutPixel(bmpImage,x,y,0);	break;
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
			uchar value = PixelFlags[n++] & 0x01;
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
void CMap::ClearDebugImage()   { if (bmpDebugImage) { DrawRectFill(bmpDebugImage,0,0,bmpDebugImage->w,bmpDebugImage->h,COLORKEY(bmpDebugImage));}}
#endif

