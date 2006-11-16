/////////////////////////////////////////
//
//                  LieroX
//
//     Copyright Auxiliary Software 2002
//
//
/////////////////////////////////////////


// Map class
// Created 22/1/02
// Jason Boettcher


#include "defs.h"
#include "LieroX.h"
#include <zlib.h>


///////////////////
// Create a new map
int CMap::New(int _width, int _height, char *_theme)
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
    strcpy(sRandomLayout.szTheme, Theme.name);

    sRandomLayout.psObjects = new object_t[sRandomLayout.nNumObjects];
    if( !sRandomLayout.psObjects )
        printf("Warning: Out of memory for CMap::ApplyRandom()\n");

	// Spawn 15 random rocks
	for(n=0;n<nNumRocks;n++) {
		x = (int)(fabs(GetRandomNum()) * (float)Width);
		y = (int)(fabs(GetRandomNum()) * (float)Height);
		i = (int)(fabs(GetRandomNum()) * (float)Theme.NumStones);

        // Store the object
        if( sRandomLayout.psObjects ) {
            sRandomLayout.psObjects[objCount].Type = OBJ_STONE;
            sRandomLayout.psObjects[objCount].Size = i;
            sRandomLayout.psObjects[objCount].X = x;
            sRandomLayout.psObjects[objCount].Y = y;
            objCount++;
        }

		PlaceStone(i,CVec((float)x,(float)y));
	}

	// Spawn 20 random misc
	for(n=0;n<nNumMisc;n++) {
		x = (int)(fabs(GetRandomNum()) * (float)Width);
		y = (int)(fabs(GetRandomNum()) * (float)Height);
		i = (int)(fabs(GetRandomNum()) * (float)Theme.NumMisc);

        // Store the object
        if( sRandomLayout.psObjects ) {
            sRandomLayout.psObjects[objCount].Type = OBJ_MISC;
            sRandomLayout.psObjects[objCount].Size = i;
            sRandomLayout.psObjects[objCount].X = x;
            sRandomLayout.psObjects[objCount].Y = y;
            objCount++;
        }

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
            if( sRandomLayout.psObjects ) {
                sRandomLayout.psObjects[objCount].Type = OBJ_HOLE;
                sRandomLayout.psObjects[objCount].Size = 4;
                sRandomLayout.psObjects[objCount].X = x+a;
                sRandomLayout.psObjects[objCount].Y = y+b;
                objCount++;
            }

			CarveHole(4,CVec( (float)(x+a), (float)(y+b)));
		}
	}

    // Calculate the total dirt count
    CalculateDirtCount();

	// Update the mini map
	bMiniMapDirty = true;
	UpdateMiniMap();
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

    // Calculate the total dirt count
    CalculateDirtCount();

	// Update the mini map
	bMiniMapDirty = true;
	UpdateMiniMap();
}


///////////////////
// Load the theme
int CMap::LoadTheme(char *_theme)
{
	char thm[64],buf[64],cfg[64];
	int n,x,y;

	sprintf(thm,"data/themes/%s",_theme);

	strcpy(Theme.name, _theme);
    strcpy(sRandomLayout.szTheme, _theme);

	sprintf(buf,"%s/Backtile.png",thm);
	LOAD_IMAGE_BPP(Theme.bmpBacktile,buf);
	sprintf(buf,"%s/Fronttile.png",thm);
	LOAD_IMAGE_BPP(Theme.bmpFronttile,buf);


	// Stones
	sprintf(cfg,"%s/theme.txt",thm);
	ReadInteger(cfg,"General","NumStones",&Theme.NumStones,0);

	for(n=0;n<Theme.NumStones;n++) {
		sprintf(buf,"%s/Stone%d.png",thm,n+1);
		LOAD_IMAGE_BPP(Theme.bmpStones[n],buf);

		SDL_SetColorKey(Theme.bmpStones[n], SDL_SRCCOLORKEY, SDL_MapRGB(Theme.bmpStones[n]->format,255,0,255));
	}


	// Holes
	for(n=0;n<5;n++) {
		sprintf(buf,"%s/Hole%d.png",thm,n+1);
		LOAD_IMAGE_BPP(Theme.bmpHoles[n],buf);

		SDL_SetColorKey(Theme.bmpHoles[n], SDL_SRCCOLORKEY, SDL_MapRGB(Theme.bmpHoles[n]->format,0,0,0));
	}

	// Calculate the default colour from a non-pink, non-black colour in the hole image
	Theme.iDefaultColour = GetPixel(Theme.bmpFronttile,0,0);
	SDL_Surface *hole = Theme.bmpHoles[0];
	Uint16 pink = (Uint16)MakeColour(255,0,255);
	if(hole) {
		for(y=0; y<hole->h; y++) {
			for(x=0; x<hole->w; x++) {
				Uint32 pixel = GetPixel(hole,x,y);
				if(pixel != 0 && pixel != pink)
					Theme.iDefaultColour = pixel;
			}
		}
	}


	// Misc
	sprintf(cfg,"%s/theme.txt",thm);
	ReadInteger(cfg,"General","NumMisc",&Theme.NumMisc,0);
	for(n=0;n<Theme.NumMisc;n++) {
		sprintf(buf,"%s/misc%d.png",thm,n+1);
		LOAD_IMAGE_BPP(Theme.bmpMisc[n],buf);
	}


    // Load the green dirt mask
    LOAD_IMAGE_BPP(bmpGreenMask, "data/gfx/greenball.png");
	
	return true;
}


///////////////////
// Finds a theme at random and returns the name
char *CMap::findRandomTheme(char *buf)
{
    assert(buf);
    buf[0] = 0;

    // Find directories in the theme dir
	char dir[256];
	char *d;
    int count=-1;
	
    // Count the number of themes
    if(FindFirstDir("data/themes",dir)) {
        count = 0;
		while(1) {
			d = MAX(strrchr(dir,'/'),strrchr(dir, '\\'))+1;
            // Make sure the theme is valid
            if( validateTheme(d) )
			    count++;

			if(!FindNextDir(dir))
				break;
		}
	}

    // Get a random number
    int t = GetRandomInt(count);

    // Count the number of themes
    if(FindFirstDir("data/themes",dir)) {
        count = 0;
		while(1) {
			d = MAX(strrchr(dir,'/'),strrchr(dir, '\\'))+1;
           	// Make sure the theme is valid
            if( validateTheme(d) ) {

                // Is this the theme to choose?
                if( count == t ) {
                    strcpy(buf,d);
                    return buf;
                }
			    count++;
            }

			if(!FindNextDir(dir))
				break;
		}
	}

    // If we get here, then default to dirt
    d_printf("CMap::findRandomTheme(): Couldn't find a random theme\n");
    d_printf("                         Defaulting to \"dirt\"\n");

    strcpy(buf,"dirt");

    return buf;
}


///////////////////
// Checks if a theme is a valid theme
bool CMap::validateTheme(char *name)
{
    // Does simple checks to see if the main files exists
    // Ie 'backtile.png' 'fronttile.png' & 'theme.txt'

    char thm[64],buf[64];	
    FILE *fp = NULL;

	sprintf(thm,"data/themes/%s",name);

    // Backtile.png
    sprintf(buf,"%s/backtile.png", thm);
    fp = fopen_i(buf,"rb");
    if( !fp )
        return false;
    fclose(fp);

    // Fronttile.png
    sprintf(buf,"%s/fronttile.png", thm);
    fp = fopen_i(buf,"rb");
    if( !fp )
        return false;
    fclose(fp);

    // Theme.txt
    sprintf(buf,"%s/theme.txt", thm);
    fp = fopen_i(buf,"rt");
    if( !fp )
        return false;
    fclose(fp);

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
		
	bmpImage = SDL_CreateRGBSurface(SDL_SWSURFACE, Width, Height,
		fmt->BitsPerPixel, fmt->Rmask, fmt->Gmask, fmt->Bmask, fmt->Amask);

	if(bmpImage == NULL) {
		SetError("CMap::CreateSurface(): bmpImage creation failed, perhaps out of memory");
		return false;
	}

	bmpDrawImage = SDL_CreateRGBSurface(SDL_SWSURFACE, Width*2, Height*2, fmt->BitsPerPixel, 
										fmt->Rmask, fmt->Gmask, fmt->Bmask, fmt->Amask);

	if(bmpDrawImage == NULL) {
		SetError("CMap::CreateSurface(): bmpDrawImage creation failed, perhaps out of memory");
		return false;
	}

	bmpBackImage = SDL_CreateRGBSurface(SDL_SWSURFACE, Width, Height, fmt->BitsPerPixel, 
									fmt->Rmask, fmt->Gmask, fmt->Bmask, fmt->Amask);

	if(bmpBackImage == NULL) {
		SetError("CMap::CreateSurface(): bmpBackImage creation failed, perhaps out of memory");
		return false;
	}

	bmpMiniMap = SDL_CreateRGBSurface(SDL_SWSURFACE, 128,96,fmt->BitsPerPixel,
									fmt->Rmask, fmt->Gmask, fmt->Bmask, fmt->Amask);

	if(bmpMiniMap == NULL) {
		SetError("CMap::CreateSurface(): bmpMiniMap creation failed, perhaps out of memory");
		return false;
	}

    bmpShadowMap = SDL_CreateRGBSurface(SDL_SWSURFACE, Width, Height, fmt->BitsPerPixel, 
									fmt->Rmask, fmt->Gmask, fmt->Bmask, fmt->Amask);

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
	PixelFlags = new uchar[Width*Height];
	if(PixelFlags == NULL) {
		SetError("CMap::CreatePixelFlags(): Out of memory");
		return false;
	}

	return true;
}


///////////////////
// Create the AI Grid
bool CMap::createGrid(void)
{
    nGridWidth = 15;
    nGridHeight = 15;

    nGridCols = Width/nGridWidth + 1;
    nGridRows = Height/nGridHeight + 1;

    GridFlags = new uchar[nGridCols * nGridRows];
    if(GridFlags == NULL) {
        SetError("CMap::CreateGrid(): Out of memory");
        return false;
    }

    return true;
}


///////////////////
// Calculate the grid
void CMap::calculateGrid(void)
{
    for(int y=0; y<Height; y+=nGridHeight)  {
        for(int x=0; x<Width; x+=nGridWidth) {
            calculateGridCell(x,y, false);
        }
    }
}


///////////////////
// Calculate a single grid cell
// x & y are pixel locations, not grid cell locations
void CMap::calculateGridCell(int x, int y, bool bSkipEmpty)
{
    int i = x/nGridWidth;
    int j = y/nGridHeight;

    // Clamp it
    i = MAX(i,0);
    j = MAX(j,0);
    i = MIN(i,nGridCols-1);
    j = MIN(j,nGridRows-1);

    x = i*nGridWidth;
    y = j*nGridHeight;

    uchar *cell = GridFlags + j*nGridCols + i;

    // Skip empty cells?
    if(*cell == PX_EMPTY && bSkipEmpty)
        return;

    int dirtCount = 0;
    int rockCount = 0;

    // Go through every pixel in the cell and get a solid flag count
    for(int b=y; b<y+nGridHeight; b++) {
        // Clipping
        if(b>=Height)
            break;
        if(x>=Width)
            break;

        uchar *pf = PixelFlags + b*Width + x;
        for(int a=x; a<x+nGridWidth; a++, pf++) {

            // Clipping
            if(a>=Width)
                break;

            if(*pf & PX_DIRT)
                dirtCount++;
            if(*pf & PX_ROCK)
                rockCount++;
        }
    }
    
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
	int x,y;

	// Place the tiles
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
	DrawImageStretch2(bmpDrawImage,bmpImage,0,0,0,0,bmpImage->w,bmpImage->h);
	
	// Set the pixel flags
	memset(PixelFlags,PX_DIRT,Width*Height*sizeof(uchar));

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
    int x,y,n;

    n=0;
    for( y=0; y<Height; y++ ) {
        for( x=0; x<Width; x++ ) {

            if( PixelFlags[n++] & PX_DIRT )
                nTotalDirtCount++;
        }
    }
}


///////////////////
// Sets a pixel flag for a certain pixel
void CMap::SetPixelFlag(int x, int y, int flag)
{
    // Check edges
	if(x < 0 || y < 0)
		return;
	if(x >= Width || y >= Height)
		return;

	PixelFlags[y * Width + x] = flag;
}


///////////////////
// Gets a pixel flag for a certain pixel
uchar CMap::GetPixelFlag(int x, int y)
{
	// Checking edges
	if(x < 0 || y < 0)
		return PX_ROCK;
	if(x >= Width || y >= Height)
		return PX_ROCK;


	return PixelFlags[y * Width + x];
}



///////////////////
// Draw the map
void CMap::Draw(SDL_Surface *bmpDest, CViewport *view)
{
	// Normal
	if(!tLXOptions->iFiltered)  {
		//DrawImageStretch2(bmpDest, bmpImage, view->GetWorldX(), view->GetWorldY(),
		//								view->GetLeft(),view->GetTop(), view->GetWidth(), view->GetHeight());
		//DEBUG_DrawPixelFlags();
		DrawImageAdv(bmpDest, bmpDrawImage, view->GetWorldX()*2, view->GetWorldY()*2,view->GetLeft(),view->GetTop(),view->GetWidth()*2,view->GetHeight()*2);
	}

	// Filtered
	if(tLXOptions->iFiltered) {	
		int bpp = bmpImage->format->BytesPerPixel;

		// Clamp the viewport for a filtered draw
		view->ClampFiltered(Width, Height);

		Uint8 *ptrSrc = (Uint8 *)bmpImage->pixels + view->GetWorldY()*bmpImage->pitch + view->GetWorldX()*bpp;
		Uint8 *ptrDst = (Uint8 *)bmpDest->pixels + view->GetTop()*bmpDest->pitch + view->GetLeft()*bpp;

		Super2xSaI( ptrSrc, bmpImage->pitch, ptrDst, bmpDest->pitch, view->GetWidth(), view->GetHeight());
	}
}


///////////////////
// Draw an object's shadow
void CMap::DrawObjectShadow(SDL_Surface *bmpDest, SDL_Surface *bmpObj, int sx, int sy, int w, int h, CViewport *view, int wx, int wy)
{
    int v_wx = view->GetWorldX();
	int v_wy = view->GetWorldY();
	int l = view->GetLeft();
	int t = view->GetTop();

    // Drop the shadow
    int Drop = 3;
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
  
    Uint16 pink = (Uint16)MakeColour(255,0,255);

    int x,y,dx,dy,i,j;
    
    for( y=sy,dy=wy,j=0; y<sy+h; y++,j++, dy += (wy+j)&1 ) {
        // World Clipping
        if(dy < 0) continue;
        if(dy >= Height) break;

        // Screen clipping
        if( dty+t+j < c_y ) continue;
        if( dty+t+j >= c_y2 ) break;

        uchar *pf = &PixelFlags[dy * Width + wx];
        Uint16 *srcpix = (Uint16*)bmpShadowMap->pixels + (dy * Width + wx);

        for( x=sx,dx=wx,i=0; x<sx+w; x++,i++, dx+=(wx+i)&1, pf+=(wx+i)&1, srcpix+=(wx+i)&1 ) {

            // Clipping
            if(dx < 0) continue;
            if(dx >= Width) break;

            // Is this pixel solid?            
            if( !(*pf & PX_EMPTY) ) continue;

            // Screen clipping
            if( dtx+l+i < c_x ) continue;
            if( dtx+l+i >= c_x2 ) continue;

            // Is the source pixel see through?
            if( GetPixel(bmpObj, x,y) == pink )
                continue;

            PutPixel(bmpDest, dtx + l+i, dty + t+j, *srcpix);
        }
    }
}


///////////////////
// Draw a pixel sized shadow
void CMap::DrawPixelShadow(SDL_Surface *bmpDest, CViewport *view, int wx, int wy)
{
    int v_wx = view->GetWorldX();
	int v_wy = view->GetWorldY();
	int l = view->GetLeft();
	int t = view->GetTop();

    int Drop = 3;
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
	int dx,dy, sx,sy;
	int bx,by;
	int x,y;
	int w,h;
	Uint32 pixel;
	uchar flag;
	Uint32 pink = MakeColour(255,0,255);

    int nNumDirt = 0;

	if(size < 0 || size > 4) {
		// Just clamp it and continue
		size = MAX(size,0);
		size = MIN(size,4);
	}


	// Calculate half
	hole = Theme.bmpHoles[size];
	w = hole->w;
	h = hole->h;

	sx = (int)pos.GetX()-(hole->w>>1);
	sy = (int)pos.GetY()-(hole->h>>1);

	
	if(SDL_MUSTLOCK(hole))
		SDL_LockSurface(hole);


	// WARNING: This requires the holes to be loaded as 16bpp surfaces
	Uint8 *p;
	uchar *px;
	Uint8 *p2;


	// Go through the pixels in the hole, setting the flags to empty
	for(y=0,dy=sy,by=16;y<hole->h;y++,dy++,by++) {

		// Clipping
		if(dy<0)			continue;
		if(dy>=bmpImage->h)	break;

		p = (Uint8 *)hole->pixels + y * hole->pitch;
		px = PixelFlags + dy * Width + sx;
		p2 = (Uint8 *)bmpImage->pixels + dy * bmpImage->pitch + sx * bmpImage->format->BytesPerPixel;

		for(x=0,dx=sx,bx=16;x<hole->w;x++,dx++,bx++) {

			// Clipping
			if(dx<0) {	p+=2; p2+=2; px++;	continue; }
			if(dx>=bmpImage->w)				break;


			pixel = *(Uint16 *)p;
			flag = *(uchar *)px;

			// Set the flag to empty
            if(pixel == pink && !(flag & PX_ROCK)) {

                // Increase the dirt count
                if( flag & PX_DIRT )
                    nNumDirt++;

				*(uchar *)px = PX_EMPTY;
            }

			// Put pixels that are not black/pink (eg, brown)
			if(pixel != 0 && pixel != pink && (flag & PX_DIRT))
				*(Uint16 *)p2 = (Uint16)pixel;

			p+=2;
			p2+=2;
			px++;
		}
	}

	if(SDL_MUSTLOCK(hole))
		SDL_UnlockSurface(hole);



	// Go through and clean up the hole
	sx-=5;
	sy-=5;
	
	if(SDL_MUSTLOCK(bmpImage))
		SDL_LockSurface(bmpImage);

	for(y=sy;y<sy+h+20;y++) {

		// Clipping
		if(y<0)			continue;
		if(y>=bmpImage->h)	break;

		p = (Uint8 *)bmpImage->pixels + y * bmpImage->pitch + sx * bmpImage->format->BytesPerPixel;
		p2 = (Uint8 *)bmpBackImage->pixels + y * bmpBackImage->pitch + sx * bmpBackImage->format->BytesPerPixel;
		px = PixelFlags + y * Width + sx;

		for(x=sx;x<sx+w+20;x++) {

			// Clipping
			if(x<0) {	p+=2; p2+=2; px++;	continue; }
			if(x>=bmpImage->w)		break;

			if(*px & PX_EMPTY)
				*(Uint16 *)p = *(Uint16 *)p2;

			p+=2;
			p2+=2;
			px++;
		}
	}

	
	if(SDL_MUSTLOCK(bmpImage))
		SDL_UnlockSurface(bmpImage);

	// Apply a shadow
	ApplyShadow(sx-5,sy-5,w+25,h+25);

    // Recalculate the grid
    int hw = w/2;
    int hh = h/2;
    for(y=sy-hh; y<sy+h+hh; y+=nGridHeight/2) {
        for(x=sx-hw; x<sx+w+hw; x+=nGridWidth/2) {
            calculateGridCell(x, y, true);
        }
    }

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
    

	bMiniMapDirty = true;

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
	Uint32 pink = MakeColour(255,0,255);

    int nDirtCount = 0;

	if(size < 0 || size > 4) {
		// Just clamp it and continue
		size = MAX(size,0);
		size = MIN(size,4);
	}


	// Calculate half
	hole = Theme.bmpHoles[size];
	w = hole->w;
	h = hole->h;

	sx = (int)pos.GetX()-(hole->w>>1);
	sy = (int)pos.GetY()-(hole->h>>1);

	
	if(SDL_MUSTLOCK(hole))
		SDL_LockSurface(hole);


	// WARNING: This requires the holes & themes to be loaded as 16bpp surfaces
	Uint8 *p;
	uchar *px;
	Uint8 *p2;


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
			if(dx<0) {	p+=2; p2+=2; px++;	continue; }
			if(dx>=bmpImage->w)				break;

			pixel = *(Uint16 *)p;
			flag = *(uchar *)px;

			int ix = dx % Theme.bmpFronttile->w;
			int iy = dy % Theme.bmpFronttile->h;

			// Set the flag to empty
			if(pixel == pink && !(flag & PX_ROCK)) {
                if( flag & PX_EMPTY )
                    nDirtCount++;

				*(uchar *)px = PX_DIRT;
				
				// Place the dirt image
				*(Uint16 *)p2 = (Uint16)GetPixel(Theme.bmpFronttile,ix,iy);
			}

			// Put pixels that are not black/pink (eg, brown)
            if(pixel != 0 && pixel != pink && flag & PX_EMPTY) {
				*(Uint16 *)p2 = (Uint16)pixel;
                *(uchar *)px = PX_DIRT;
                nDirtCount++;
            }

			p+=2;
			p2+=2;
			px++;
		}
	}

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

	bMiniMapDirty = true;

    return nDirtCount;
}


///////////////////
// Place a blob of green dirt (greenball)
//
// Returns the number of dirt pixels placed
int CMap::PlaceGreenDirt(CVec pos)
{
 	int dx,dy, sx,sy;
	int bx,by;
	int x,y;
	int w,h;
	Uint32 pixel;
	uchar flag;
	Uint32 pink = MakeColour(255,0,255);
    Uint32 green = MakeColour(0,255,0);
    Uint32 greens[4] = {MakeColour(148,136,0), 
                        MakeColour(136,124,0), 
                        MakeColour(124,112,0), 
                        MakeColour(116,100,0)};

    int nGreenCount = 0;


	// Calculate half
	w = bmpGreenMask->w;
	h = bmpGreenMask->h;

	sx = (int)pos.GetX()-(w>>1);
	sy = (int)pos.GetY()-(h>>1);

	
	if(SDL_MUSTLOCK(bmpGreenMask))
		SDL_LockSurface(bmpGreenMask);


	// WARNING: This requires the holes & themes to be loaded as 16bpp surfaces
	Uint8 *p;
	uchar *px;
	Uint8 *p2;


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
			if(dx<0) {	p+=2; p2+=2; px++;	continue; }
			if(dx>=bmpImage->w)				break;

			pixel = *(Uint16 *)p;
			flag = *(uchar *)px;

			// Set the flag to empty
			if(pixel == green && flag & PX_EMPTY) {
				*(uchar *)px = PX_DIRT;
                nGreenCount++;

                // Place a random green pixel
                Uint32 gr = greens[ GetRandomInt(3) ];
				
				// Place the dirt image
				*(Uint16 *)p2 = (Uint16)gr;
			}

			// Put pixels that are not green/pink (eg, dark green)
            if(pixel != green && pixel != pink && flag & PX_EMPTY) {
				*(Uint16 *)p2 = (Uint16)pixel;
                *(uchar *)px = PX_DIRT;
                nGreenCount++;
            }

			p+=2;
			p2+=2;
			px++;
		}
	}

	if(SDL_MUSTLOCK(bmpGreenMask))
		SDL_UnlockSurface(bmpGreenMask);

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

	bMiniMapDirty = true;

    return nGreenCount;
}


///////////////////
// Apply a shadow to an area
void CMap::ApplyShadow(int sx, int sy, int w, int h)
{
	int Drop = 3;
	int x,y,n;
	int bpp = bmpImage->format->BytesPerPixel;
	uchar *px;
	uchar *p;
	int ox,oy;
	char flag;


	// Draw shadows?
	if(!tLXOptions->iShadows)
		return;


	Uint32 Rmask = bmpImage->format->Rmask, Gmask = bmpImage->format->Gmask, Bmask = bmpImage->format->Bmask, Amask = bmpImage->format->Amask;
	//Uint32 R,G,B,A = 0;
	Uint8 alpha = 64;
	Uint32 color = 0;

	if(SDL_MUSTLOCK(bmpImage))
		SDL_LockSurface(bmpImage);

	// WARNING: Assumes 16bpp

	for(y=sy;y<sy+h;y++) {

		// Clipping
		if(y<0)				continue;
		else if(y>=Height)	break;

		px = PixelFlags + y * Width + sx;

		for(x=sx;x<sx+w;x++) {

			// Clipping
			if(x<0) {	px++;	continue; }
			else if(x>=Width)	break;
			
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

                    Uint32 offset = oy*bmpImage->pitch/2 + ox;
                    Uint16 *pixel = (Uint16 *)bmpImage->pixels + offset;
                    Uint16 *src = (Uint16 *)bmpShadowMap->pixels + offset;
                    *pixel = *src;
					
					// Transparency
					/*Uint16 *pixel = (Uint16 *)bmpImage->pixels + oy*bmpImage->pitch/2 + ox;

					Uint16 *srcpixel = (Uint16 *)bmpBackImage->pixels + oy*bmpBackImage->pitch/2 + ox;
					Uint32 dc = *srcpixel;

					R = ((dc & Rmask) + (( (color & Rmask) - (dc & Rmask) ) * alpha >> 8)) & Rmask;
					G = ((dc & Gmask) + (( (color & Gmask) - (dc & Gmask) ) * alpha >> 8)) & Gmask;
					B = ((dc & Bmask) + (( (color & Bmask) - (dc & Bmask) ) * alpha >> 8)) & Bmask;
					if( Amask )
						A = ((dc & Amask) + (( (color & Amask) - (dc & Amask) ) * alpha >> 8)) & Amask;

					*pixel= R | G | B | A;*/



					//PutPixel(bmpImage,ox,oy,0);

					*(uchar *)p |= PX_EMPTY | PX_SHADOW;
					ox++; oy++;
				}
			}

			px++;
		}
	}


	if(SDL_MUSTLOCK(bmpImage))
		SDL_UnlockSurface(bmpImage);

	bMiniMapDirty = true;
}


///////////////////
// Calculate the shadow map
void CMap::CalculateShadowMap(void)
{
    int x,y;
    int bpp = bmpImage->format->BytesPerPixel;

    Uint32 Rmask = bmpImage->format->Rmask, Gmask = bmpImage->format->Gmask, Bmask = bmpImage->format->Bmask, Amask = bmpImage->format->Amask;
	Uint32 R,G,B,A = 0;
	Uint8 alpha = 100;
	Uint32 color = 0;

    Uint16 *pixel = (Uint16 *)bmpShadowMap->pixels;
    Uint16 *srcpixel = (Uint16 *)bmpBackImage->pixels;

	if(SDL_MUSTLOCK(bmpBackImage))
		SDL_LockSurface(bmpBackImage);
    if(SDL_MUSTLOCK(bmpShadowMap))
		SDL_LockSurface(bmpShadowMap);

    for( y=0; y<Height; y++ ) {

        for( x=0; x<Width; x++, pixel++, srcpixel++ ) {

            // Transparency
			Uint32 dc = *srcpixel;

			R = ((dc & Rmask) + (( (color & Rmask) - (dc & Rmask) ) * alpha >> 8)) & Rmask;
			G = ((dc & Gmask) + (( (color & Gmask) - (dc & Gmask) ) * alpha >> 8)) & Gmask;
			B = ((dc & Bmask) + (( (color & Bmask) - (dc & Bmask) ) * alpha >> 8)) & Bmask;
			if( Amask )
				A = ((dc & Amask) + (( (color & Amask) - (dc & Amask) ) * alpha >> 8)) & Amask;

			*pixel= (Uint16)(R | G | B | A);
        }
    }

    if(SDL_MUSTLOCK(bmpBackImage))
		SDL_UnlockSurface(bmpBackImage);
    if(SDL_MUSTLOCK(bmpShadowMap))
		SDL_UnlockSurface(bmpShadowMap);
}


///////////////////
// Place a stone
void CMap::PlaceStone(int size, CVec pos)
{
	SDL_Surface *stone;
	int dx,dy, sx,sy;
	int x,y;
	int w,h;

	Uint32 pink = MakeColour(255,0,255);

	if(size < 0 || size >= Theme.NumStones) {
		// TODO: Bail out or warning of overflow
		d_printf("Bad stone size\n");
		return;
	}

	
	// Add the stone to the object list
	if(NumObjects+1 < MAX_OBJECTS) {
		object_t *o = &Objects[NumObjects++];
		o->Type = OBJ_STONE;
		o->Size = size;
		o->X = (int) pos.GetX();
        o->Y = (int) pos.GetY();
	}



	// Calculate half
	stone = Theme.bmpStones[size];
	w = stone->w;
	h = stone->h;

	sx = (int)pos.GetX()-(stone->w>>1);
	sy = (int)pos.GetY()-(stone->h>>1);


	if(SDL_MUSTLOCK(stone))
		SDL_LockSurface(stone);


	// WARNING: This requires the stones to be loaded as 16bpp surfaces
	Uint8 *p = (Uint8 *)stone->pixels;
	uchar *px = PixelFlags;

	// Go through the pixels in the stone
	for(y=0,dy=sy;y<stone->h;y++,dy++) {

		// Clipping
		if(dy<0)			continue;
		if(dy>=bmpImage->h)	break;

		p = (Uint8 *)stone->pixels + y * stone->pitch;
		px = PixelFlags + dy * Width + sx;

		for(x=0,dx=sx;x<stone->w;x++,dx++) {

			// Clipping
			if(dx<0) {	p+=2; px++;	continue; }
			if(dx>=bmpImage->w)		break;


			// Put the pixel down
			if(*(Uint16 *)p != pink) {

				PutPixel(bmpImage,dx,dy,*(Uint16 *)p);
				*(uchar *)px = PX_ROCK;
			}

			p+=2;
			px++;
		}
	}

	if(SDL_MUSTLOCK(stone))
		SDL_UnlockSurface(stone);

	// Apply the shadow
	ApplyShadow(sx-5,sy-5,w+10,h+10);

	// Update the draw image
	DrawImageStretch2(bmpDrawImage,bmpImage,0,0,0,0,bmpImage->w,bmpImage->h);

    // Calculate the total dirt count
    CalculateDirtCount();

	bMiniMapDirty = true;
}


///////////////////
// Place a miscellaneous item
void CMap::PlaceMisc(int id, CVec pos)
{
	
	SDL_Surface *misc;
	int dx,dy, sx,sy;
	int x,y;
	int w,h;

	Uint32 pink = MakeColour(255,0,255);

	if(id < 0 || id >= Theme.NumMisc) {
		// TODO: Bail out or warning of overflow
		d_printf("Bad misc size\n");
		return;
	}

	// Add the misc to the object list
	if(NumObjects+1 < MAX_OBJECTS) {
		object_t *o = &Objects[NumObjects++];
		o->Type = OBJ_MISC;
		o->Size = id;
		o->X = (int) pos.GetX();
        o->Y = (int) pos.GetY();
	}


	// Calculate half
	misc = Theme.bmpMisc[id];
	w = misc->w;
	h = misc->h;

	sx = (int)pos.GetX()-(misc->w>>1);
	sy = (int)pos.GetY()-(misc->h>>1);


	if(SDL_MUSTLOCK(misc))
		SDL_LockSurface(misc);


	// WARNING: This requires the misc items to be loaded as 16bpp surfaces
	Uint8 *p = (Uint8 *)misc->pixels;
	uchar *px = PixelFlags;

	// Go through the pixels in the misc item
	for(y=0,dy=sy;y<misc->h;y++,dy++) {

		// Clipping
		if(dy<0)			continue;
		if(dy>=bmpImage->h)	break;

		p = (Uint8 *)misc->pixels + y * misc->pitch;
		px = PixelFlags + dy * Width + sx;

		for(x=0,dx=sx;x<misc->w;x++,dx++) {

			// Clipping
			if(dx<0) {	p+=2; px++;	continue; }
			if(dx>=bmpImage->w)		break;


			// Put the pixel down
			if(*(Uint16 *)p != pink && *px & PX_DIRT) {

				PutPixel(bmpImage,dx,dy,*(Uint16 *)p);
				*(uchar *)px = PX_DIRT;
			}

			p+=2;
			px++;
		}
	}

	if(SDL_MUSTLOCK(misc))
		SDL_UnlockSurface(misc);

	// Update the draw image
	DrawImageStretch2(bmpDrawImage,bmpImage,0,0,0,0,bmpImage->w,bmpImage->h);

	bMiniMapDirty = true;
}


///////////////////
// Delete an object
void CMap::DeleteObject(CVec pos)
{
	int w,h;

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

		if((int)fabs(pos.GetX() - obj->X) < w &&
			(int)fabs(pos.GetY() - obj->Y) < h) {

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
void CMap::PutImagePixel(int x, int y, Uint32 colour)
{
    // Checking edges
	if(x < 0 || y < 0)
		return;
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
void CMap::UpdateMiniMap(int force)
{
	float xstep,ystep;
	float mx,my;
	int mmx,mmy;
	int mw = bmpMiniMap->w;
	int mh = bmpMiniMap->h;

	if(!bMiniMapDirty && !force)
		return;


	// Calculate steps
	xstep = (float)Width/ (float)mw;
	ystep = (float)Height / (float)mh;


	Uint16 *sp,*tp,*tmp1,*tmp2;
	
	for(my=0,mmy=0;my<Height;my+=ystep,mmy++) {

		if(mmy >= mh)
			break;

		// Save the pointer to the first pixel in current line to make the loop faster
		tmp1 = (Uint16 *)bmpImage->pixels + (int)my*bmpImage->pitch/2;
		tmp2 = (Uint16 *)bmpMiniMap->pixels + mmy*bmpMiniMap->pitch/2;
		
		for(mx=0,mmx=0;mx<Width;mx+=xstep,mmx++) {

			if(mmx >= mw)
				break;

			// Copy the pixel
			sp = tmp1+(int)mx*bmpImage->format->BytesPerPixel/2;
			tp = tmp2+mmx*bmpMiniMap->format->BytesPerPixel/2;

			*tp = *sp;

			//PutPixel(bmpMiniMap,mmx,mmy,GetPixel(bmpImage,(int)mx,(int)my));  // Slow
		}
	}

	// Not dirty anymore
	bMiniMapDirty = false;
}


///////////////////
// Draw & Simulate the minimap
void CMap::DrawMiniMap(SDL_Surface *bmpDest, int x, int y, float dt, CWorm *worms, int gametype)
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
	for(n=0;n<MAX_WORMS;n++,w++) {
		if(!w->getAlive() || !w->isUsed())
			continue;
		
		GetColour4(w->getColour(), bmpMiniMap, &r,&g,&b,&a);

		int dr = 255-r;
		int dg = 255-g;
		int db = 255-b;

		r = r + (int)( (float)dr*(time*2.0f));
		g = g + (int)( (float)dg*(time*2.0f));
		b = b + (int)( (float)db*(time*2.0f));

		mx = w->getPos().GetX()/xstep;
		my = w->getPos().GetY()/ystep;
        mx = (float)floor(mx);
        my = (float)floor(my);

		mx = MIN(mw-(float)1,mx); mx = MAX((float)0,mx);
		my = MIN(mh-(float)1,my); my = MAX((float)0,my);
		i=(int)mx + x;
		j=(int)my + y;
		// Snap it to the nearest 2nd pixel (prevent 'jumping')
		//x -= x % 2;
		//y -= y % 2;

		Uint32 col = MakeColour(r,g,b);
		
		//PutPixel(bmpMiniMap,x,y,col);
		DrawRectFill(bmpDest,i-1,j-1, i+1,j+1, col);

		// Our worms are bigger
		int big = false;

		// Tagged worms or local players are bigger, depending on the game type
		if(gametype != GMT_TAG) {
			if(w->getType() == PRF_HUMAN && w->getLocal())
				big = true;
		} else {
			if(w->getTagIT())
				big = true;
		}

		if(big)
			DrawRectFill(bmpDest,i-1,j-1, i+2,j+2,col);
	}
}


///////////////////
// Load the map
int CMap::Load(char *filename)
{
	// Weird
	if (strlen(filename) == 0)
		return true;

	FILE *fp = fopen_i(filename,"rb");
	if(fp == NULL)
		return false;

	bMiniMapDirty = true;
    sRandomLayout.bUsed = false;

	// Check if it's an original liero level
	if( stricmp(filename + strlen(filename) - 4, ".lev") == 0 ) {
		return LoadOriginal(fp);
	}


	// Header
	char	id[32];
	int		version;
	int		numobj;
	char	Theme_Name[32];

	fread(id,			sizeof(char),	32,	fp);
	fread(&version,		sizeof(int),	1,	fp);
	EndianSwap(version);
	
	// Check to make sure it's a valid level file
	if(strcmp(id,"LieroX Level") != 0 || version != MAP_VERSION) {
		printf("CMap::Load (%s): ERROR: not a valid level file or wrong version\n", filename);
		fclose(fp);
		return false;
	}

	fread(Name,			sizeof(char),	64,	fp);
	fread(&Width,		sizeof(int),	1,	fp);
	EndianSwap(Width);
	fread(&Height,		sizeof(int),	1,	fp);
	EndianSwap(Height);
	fread(&Type,		sizeof(int),	1,	fp);
	EndianSwap(Type);
	fread(Theme_Name,	sizeof(char),	32,	fp);
	fread(&numobj,		sizeof(int),	1,	fp);
	EndianSwap(numobj);

	printf("Level info:\n");
	printf("  id = %s\n", id);
	printf("  version = %i\n", version);
	printf("  Name = %s\n", Name);
	printf("  Width = %i\n", Width);
	printf("  Height = %i\n", Height);
	printf("  Type = %i\n", Type);
	printf("  Theme_Name = %s\n", Theme_Name);
	printf("  numobj = %i\n", numobj);
	
	// Create the map
	if(!New(Width, Height, Theme_Name)) {
		printf("CMap::Load (%s): ERROR: cannot create map\n", filename);
		fclose(fp);
		return false;
	}

	// Load the images if in an image format
	if(Type == MPT_IMAGE)
	{
		printf("CMap::Load (%s): HINT: level is in image format\n", filename);
		return LoadImageFormat(fp);
	}			
				

	// Dirt map
	int n;
	Uint16 *p1 = (Uint16 *)bmpImage->pixels;
	Uint16 *p2 = (Uint16 *)bmpBackImage->pixels;

	for(n=0;n<Width*Height;) {
		uchar t;

		fread(&t,		sizeof(uchar),	1,	fp);	
		EndianSwap(t);
		
		// 1 bit == 1 pixel with a yes/no dirt flag
		for(int i=0;i<8;i++) {

			if(t & (1 << i))
				PixelFlags[n] = PX_EMPTY;

			// Set the image
			if(PixelFlags[n] & PX_EMPTY)
				*p1 = *p2;
			
			n++;
			p1++;
			p2++;
		}
	}


	// Objects
	object_t o;
	NumObjects = 0;
	for(int i=0;i<numobj;i++) {
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
		if(o.Type == OBJ_MISC)
			PlaceMisc(o.Size,CVec((float)o.X, (float)o.Y));
	}


	ApplyShadow(0,0,Width,Height);

	fclose(fp);

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
int CMap::Save(char *name, char *filename)
{
	FILE *fp = fopen_i(filename,"wb");
	if(fp == NULL)
		return false;

	Type = MPT_IMAGE;

	// Header
	char	id[32];
	int		version = MAP_VERSION;
	strcpy(id,"LieroX Level");

	fwrite(id,			sizeof(char),	32,	fp);
	fwrite(GetEndianSwapped(version),	sizeof(int),	1,	fp);
	fwrite(name,		sizeof(char),	64,	fp);
	fwrite(GetEndianSwapped(Width),		sizeof(int),	1,	fp);
	fwrite(GetEndianSwapped(Height),		sizeof(int),	1,	fp);
	fwrite(GetEndianSwapped(Type),		sizeof(int),	1,	fp);
	fwrite(Theme.name,	sizeof(char),	32,	fp);
	fwrite(GetEndianSwapped(NumObjects),	sizeof(int),	1,	fp);


	// Save the images if in an image format
	if(Type == MPT_IMAGE)
		return SaveImageFormat(fp);
				


	// Dirt map
	int n;
	for(n=0;n<Width*Height;) {
		uchar t = 0;

		// 1 bit == 1 pixel with a yes/no dirt flag
		for(int i=0;i<8;i++) {
			int value = PixelFlags[n++] & 0x01;
			t |= (value << i);
		}

		fwrite(GetEndianSwapped(t),	sizeof(uchar),	1,	fp);	
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
	int x,y,n,p;
	Uint8 r,g,b,a;

	// The images are saved in a raw 24bit format.
	// 8 bits per r,g,b channel

	if( !bmpBackImage || !bmpImage || !PixelFlags )
		return false;

	// Write out the images & pixeflags to memory, compress the data & save the compressed data
	ulong size = (Width*Height * 3) * 2 + (Width*Height) + 1;
	ulong destsize = size + (size / 8) + 12;

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

			pSource[p++] = *GetEndianSwapped(r);
			pSource[p++] = *GetEndianSwapped(g);
			pSource[p++] = *GetEndianSwapped(b);
		}
	}

	// Save the front image
	for(y=0; y<Height; y++) {
		for(x=0; x<Width; x++) {
			GetColour4( GetPixel(bmpImage,x,y), bmpImage, &r, &g, &b ,&a );
			pSource[p++] = *GetEndianSwapped(r);
			pSource[p++] = *GetEndianSwapped(g);
			pSource[p++] = *GetEndianSwapped(b);
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
	if( compress( pDest, &destsize, pSource, size) != Z_OK ) {
		printf("Failed compressing\n");
		fclose(fp);
		delete[] pSource;
		delete[] pDest;
		return false;
	}
	
	// Write out the details & the data
	fwrite(GetEndianSwapped(destsize), sizeof(ulong), 1, fp);
	fwrite(GetEndianSwapped(size), sizeof(ulong), 1, fp);
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
	ulong size, destsize;
	int x,y,n,p;
	Uint8 r,g,b;

	fread(&size, sizeof(ulong), 1, fp);
	EndianSwap(size);
	fread(&destsize, sizeof(ulong), 1, fp);
	EndianSwap(destsize);
	
	// Allocate the memory
	uchar *pSource = new uchar[size];
	uchar *pDest = new uchar[destsize];

	if(!pSource || !pDest) {
		fclose(fp);
		return false;
	}

	fread(pSource, sizeof(uchar), size, fp);

	if( uncompress( pDest, &destsize, pSource, size ) != Z_OK ) {		
		printf("Failed decompression\n");
		fclose(fp);
		delete[] pSource;
		delete[] pDest;
		return false;
	}
		

	// Translate the data

	// Load the back image
	p=0;
	for(y=0; y<Height; y++) {
		for(x=0; x<Width; x++) {
			r = *GetEndianSwapped(pDest[p++]);
			g = *GetEndianSwapped(pDest[p++]);
			b = *GetEndianSwapped(pDest[p++]);

			PutPixel( bmpBackImage, x,y, MakeColour(r,g,b));
		}
	}

	// Save the front image
	for(y=0; y<Height; y++) {
		for(x=0; x<Width; x++) {
			r = *GetEndianSwapped(pDest[p++]);
			g = *GetEndianSwapped(pDest[p++]);
			b = *GetEndianSwapped(pDest[p++]);

			PutPixel( bmpImage, x,y, MakeColour(r,g,b));
		}
	}

	//SDL_SaveBMP(bmpImage, "image.bmp");
	//SDL_SaveBMP(bmpBackImage, "backimage.bmp");

	// TEMP
	//SDL_PixelFormat *fmt = bmpImage->format;
	//SDL_Surface *pxf = SDL_CreateRGBSurface(SDL_SWSURFACE, Width, Height, fmt->BitsPerPixel, 
	//								fmt->Rmask, fmt->Gmask, fmt->Bmask, fmt->Amask);

	// Load the pixel flags
	n=0;
	for(y=0; y<Height; y++) {
		for(x=0; x<Width; x++) {
		
			uchar t = pDest[p++];
			PixelFlags[n++] = t;

			if(t == PX_EMPTY)
				PutPixel(bmpImage, x,y, GetPixel(bmpBackImage,x,y));
			/*if(t == PX_ROCK)
				PutPixel(pxf, x,y, MakeColour(128,128,128));
			if(t == PX_DIRT)
				PutPixel(pxf, x,y, MakeColour(128,64,64));*/
		}
	}

	//SDL_SaveBMP(pxf, "mat.bmp");

    delete[] pSource;
	delete[] pDest;

	fclose(fp);

    // Calculate the shadowmap
    CalculateShadowMap();

	ApplyShadow(0,0,Width,Height);
	
	// Update the draw image
	DrawImageStretch2(bmpDrawImage,bmpImage,0,0,0,0,bmpImage->w,bmpImage->h);

    // Calculate the total dirt count
    CalculateDirtCount();

    // Calculate the grid
    calculateGrid();
    
	return true;
}


///////////////////
// Load an original version of a liero leve;
int CMap::LoadOriginal(FILE *fp)
{
	int Powerlevel = false;
	uchar *palette = NULL;
	int x,y,n;

	// Validate the liero level
	fseek(fp,0,SEEK_END);
	long length = ftell(fp);

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
		FILE *pal = fopen_i("data/lieropal.act","rb");
		if(!pal) {
			fclose(fp);
			return false;
		}

		fread(palette,sizeof(uchar),768,pal);
		fclose(pal);
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
		char id[11];
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
	n=0;
	for(y=0;y<Height;y++) {
		for(x=0;x<Width;x++) {
			uchar p = bytearr[n];
			int type = PX_EMPTY;
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
				if( (p >= 19 && p <= 29) ||
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

	// LOL :)
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

			switch(PixelFlags[n]) {

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
            if(GridFlags[y*nGridCols+x] == PX_DIRT)
                DrawRectFill(bmpDrawImage,x*nGridWidth*2,y*nGridHeight*2,(x*nGridWidth+nGridWidth)*2,(y*nGridHeight+nGridHeight)*2, MakeColour(255,0,0));
            if(GridFlags[y*nGridCols+x] == PX_ROCK)
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
	fp = fopen_i("tempmap.dat","wb");

	int n;
	
	for(n=0;n<Width*Height;) {
		uchar t = 0;

		for(int i=0;i<8;i++) {
			int value = PixelFlags[n++] & 0x01;
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

	for(n=0;n<3;n++)
		fwrite(&b,sizeof(uchar),1,fp);

	fclose(fp);

}


///////////////////
// Shutdown the map
void CMap::Shutdown(void)
{
	if(Created) {

		if(bmpImage)
			SDL_FreeSurface(bmpImage);
		bmpImage = NULL;

		if(bmpDrawImage)
			SDL_FreeSurface(bmpDrawImage);
		bmpDrawImage = NULL;

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

		if(Objects)
			delete[] Objects;
		Objects = NULL;
		NumObjects = 0;

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
		bmpBackImage = NULL;
		bmpShadowMap = NULL;
		bmpMiniMap = NULL;
		PixelFlags = NULL;
		GridFlags = NULL;
		Objects = NULL;
		sRandomLayout.psObjects = NULL;
	}

	Created = false;
}
