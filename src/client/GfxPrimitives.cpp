/////////////////////////////////////////
//
//   OpenLieroX
//
//   Auxiliary Software class library
//
//   based on the work of JasonB
//   enhanced by Dark Charlie and Albert Zeyer
//
//   code under LGPL
//
/////////////////////////////////////////


// Graphics primitives
// Created 12/11/01
// By Jason Boettcher


#include <gd.h>

#include "defs.h"
#include "LieroX.h"
#include "GfxPrimitives.h"



int ropecolour = 0;
int ropealt = 0;

///////////////////
// Put a pixel on the surface (while checking for clipping)
void RopePutPixel(SDL_Surface *bmpDest, int x, int y, Uint32 colour)
{
	ropealt = !ropealt;

	if(ropealt)
		return;

	if(!bmpDest)
		return;

	// Snap to nearest 2nd pixel
	x -= x % 2;
	y -= y % 2;

	if( x < 0 || y < 0 )
		return;
	if( x < bmpDest->clip_rect.x || y < bmpDest->clip_rect.y )
		return;
	if( x >= bmpDest->clip_rect.x+bmpDest->clip_rect.w || y >= bmpDest->clip_rect.y+bmpDest->clip_rect.h )
		return;

	Uint32 cols[2];
	cols[0] = MakeColour(160,80,0);
	cols[1] = MakeColour(200,100,0);

	ropecolour = !ropecolour;
	colour = cols[ropecolour];

	boxColor(bmpDest, x,y,x+1,y+1, colour);
}


int beamalt = 0;

///////////////////
// Put a pixel on the surface (while checking for clipping)
void BeamPutPixel(SDL_Surface *bmpDest, int x, int y, Uint32 colour)
{
	beamalt = !beamalt;

	if(beamalt)
		return;

	if(!bmpDest)
		return;

	// Snap to nearest 2nd pixel
	x -= x % 2;
	y -= y % 2;

	if( x < 0 || y < 0 )
		return;
	if( x < bmpDest->clip_rect.x || y < bmpDest->clip_rect.y )
		return;
	if( x >= bmpDest->clip_rect.x+bmpDest->clip_rect.w || y >= bmpDest->clip_rect.y+bmpDest->clip_rect.h )
		return;

	boxColor(bmpDest, x,y,x+1,y+1, colour);
}


int laseralt = 0;

///////////////////
// Put a pixel on the surface (while checking for clipping)
void LaserSightPutPixel(SDL_Surface *bmpDest, int x, int y, Uint32 colour)
{
	laseralt++;
	laseralt %= GetRandomInt(35)+1;

	if(laseralt)
		return;

	if(!bmpDest)
		return;

	Uint32 Colours[] = { MakeColour(190,0,0), MakeColour(160,0,0) };
	colour = Colours[ GetRandomInt(1) ];

	// Snap to nearest 2nd pixel
	x -= x % 2;
	y -= y % 2;

	if( x < 0 || y < 0 )
		return;
	if( x < bmpDest->clip_rect.x || y < bmpDest->clip_rect.y )
		return;
	if( x >= bmpDest->clip_rect.x+bmpDest->clip_rect.w || y >= bmpDest->clip_rect.y+bmpDest->clip_rect.h )
		return;

	boxColor(bmpDest, x,y,x+1,y+1, colour);
}


///////////////////






#define ABS(a) (((a)<0) ? -(a) : (a))

/*
 * Perform a line draw using a put pixel callback
 * Grabbed from allegro
 */
void perform_line(SDL_Surface *bmp, int x1, int y1, int x2, int y2, int d, void (*proc)(SDL_Surface *, int, int, Uint32))
{
   int dx = x2-x1;
   int dy = y2-y1;
   int i1, i2;
   int x, y;
   int dd;

   /* worker macro */
   #define DO_LINE(pri_sign, pri_c, pri_cond, sec_sign, sec_c, sec_cond)     \
   {                                                                         \
      if (d##pri_c == 0) {                                                   \
	 proc(bmp, x1, y1, d);                                               \
	 return;                                                             \
      }                                                                      \
									     \
      i1 = 2 * d##sec_c;                                                     \
      dd = i1 - (sec_sign (pri_sign d##pri_c));                              \
      i2 = dd - (sec_sign (pri_sign d##pri_c));                              \
									     \
      x = x1;                                                                \
      y = y1;                                                                \
									     \
      while (pri_c pri_cond pri_c##2) {                                      \
	 proc(bmp, x, y, d);                                                 \
									     \
	 if (dd sec_cond 0) {                                                \
	    sec_c sec_sign##= 1;                                             \
	    dd += i2;                                                        \
	 }                                                                   \
	 else                                                                \
	    dd += i1;                                                        \
									     \
	 pri_c pri_sign##= 1;                                                \
      }                                                                      \
   }

   if (dx >= 0) {
      if (dy >= 0) {
	 if (dx >= dy) {
	    /* (x1 <= x2) && (y1 <= y2) && (dx >= dy) */
	    DO_LINE(+, x, <=, +, y, >=);
	 }
	 else {
	    /* (x1 <= x2) && (y1 <= y2) && (dx < dy) */
	    DO_LINE(+, y, <=, +, x, >=);
	 }
      }
      else {
	 if (dx >= -dy) {
	    /* (x1 <= x2) && (y1 > y2) && (dx >= dy) */
	    DO_LINE(+, x, <=, -, y, <=);
	 }
	 else {
	    /* (x1 <= x2) && (y1 > y2) && (dx < dy) */
	    DO_LINE(-, y, >=, +, x, >=);
	 }
      }
   }
   else {
      if (dy >= 0) {
	 if (-dx >= dy) {
	    /* (x1 > x2) && (y1 <= y2) && (dx >= dy) */
	    DO_LINE(-, x, >=, +, y, >=);
	 }
	 else {
	    /* (x1 > x2) && (y1 <= y2) && (dx < dy) */
	    DO_LINE(+, y, <=, -, x, <=);
	 }
      }
      else {
	 if (-dx >= -dy) {
	    /* (x1 > x2) && (y1 > y2) && (dx >= dy) */
	    DO_LINE(-, x, >=, -, y, <=);
	 }
	 else {
	    /* (x1 > x2) && (y1 > y2) && (dx < dy) */
	    DO_LINE(-, y, >=, -, x, <=);
	 }
      }
   }
}


///////////////////
// Draws a rope line
void DrawRope(SDL_Surface *bmp, int x1, int y1, int x2, int y2, Uint32 color)
{
	int sx, sy, dx, dy, t;
	ropealt = 0;
	ropecolour = 0;

	SDL_Rect rect = bmp->clip_rect;
	int	ct = rect.y;
	int cl = rect.x;
	int cr = rect.x + rect.w;
	int cb = rect.y + rect.h;

	sx = x1;
    sy = y1;
    dx = x2;
	dy = y2;

    if (sx > dx) {
		t = sx;
		sx = dx;
		dx = t;
    }

    if (sy > dy) {
		t = sy;
	    sy = dy;
	    dy = t;
    }

    if ((sx >= cr) || (sy >= cb) || (dx < cl) || (dy < ct))
		return;

	t = true;


	perform_line(bmp, x1, y1, x2, y2, color, RopePutPixel);
}


///////////////////
// Draws a beam
void DrawBeam(SDL_Surface *bmp, int x1, int y1, int x2, int y2, Uint32 color)
{
	int sx, sy, dx, dy, t;
	beamalt = 0;

	SDL_Rect rect = bmp->clip_rect;
	int	ct = rect.y;
	int cl = rect.x;
	int cr = rect.x + rect.w;
	int cb = rect.y + rect.h;

	sx = x1;
    sy = y1;
    dx = x2;
	dy = y2;

    if (sx > dx) {
		t = sx;
		sx = dx;
		dx = t;
    }

    if (sy > dy) {
		t = sy;
	    sy = dy;
	    dy = t;
    }

    if ((sx >= cr) || (sy >= cb) || (dx < cl) || (dy < ct))
		return;

	t = true;

	perform_line(bmp, x1, y1, x2, y2, color, BeamPutPixel);
}


///////////////////
// Draws a laser sight
void DrawLaserSight(SDL_Surface *bmp, int x1, int y1, int x2, int y2, Uint32 color)
{
	int sx, sy, dx, dy, t;
	laseralt = GetRandomInt(2);

	SDL_Rect rect = bmp->clip_rect;
	int	ct = rect.y;
	int cl = rect.x;
	int cr = rect.x + rect.w;
	int cb = rect.y + rect.h;

	sx = x1;
    sy = y1;
    dx = x2;
	dy = y2;

    if (sx > dx) {
		t = sx;
		sx = dx;
		dx = t;
    }

    if (sy > dy) {
		t = sy;
	    sy = dy;
	    dy = t;
    }

    if ((sx >= cr) || (sy >= cb) || (dx < cl) || (dy < ct))
		return;

	t = true;

	perform_line(bmp, x1, y1, x2, y2, color, LaserSightPutPixel);
}


/*
 *  Image saving routines
 */

///////////////////////
// Converts the SDL_surface to gdImagePtr
gdImagePtr SDLSurface2GDImage(SDL_Surface* src) {
	gdImagePtr gd_image = gdImageCreateTrueColor(src->w,src->h);
	if(!gd_image)
		return NULL;

	Uint32 rmask, gmask, bmask;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	rmask=0x00FF0000; gmask=0x0000FF00; bmask=0x000000FF;
#else	
	rmask=0x000000FF; gmask=0x0000FF00; bmask=0x00FF0000;	
#endif
	
	SDL_Surface* formated = SDL_CreateRGBSurface(
		SDL_SWSURFACE, src->w, src->h, 32, rmask, gmask, bmask, 0);
	if(!formated)
		return NULL;

	// convert it to the new format (24 bpp)
	DrawImageEx(formated, src, 0, 0, src->w, src->h);
	
	for(int y = 0; y < src->h; y++) {
		memcpy(gd_image->tpixels[y], (uchar*)formated->pixels + y*formated->pitch, formated->pitch);	
	}
	
	SDL_FreeSurface(formated);
	
	return gd_image;
}

///////////////////////
// Saves the surface into the specified file with the specified format
bool SaveSurface(SDL_Surface *image, const std::string& FileName, int Format, bool Tournament)
{
  if (Format == FMT_BMP)  {
		std::string abs_fn = GetWriteFullFileName(FileName, true);
		SDL_SaveBMP(image, abs_fn.c_str());

		// Log
		if (Tournament && cServer)  {
			cServer->setTakeScreenshot(false);
			cServer->setScreenshotToken(true);

			FILE *f = OpenGameFile(FileName,"ab");
			if (!f)
				return false;
			if (!cServer->WriteLogToFile(f))  {
				fclose(f);
				return false;
			}
			fclose(f);

		}

		return true;
  }

  gdImagePtr gd_image = NULL;

  gd_image = SDLSurface2GDImage(image);
  if (!gd_image)
	  return false;

  // Save the image
  FILE *out;
  int size;
  char *data = NULL;
  out = OpenGameFile(FileName, "wb");
  if (!out) {
    return false;
  }

  switch (Format) {
  case FMT_PNG:
	//strcat(FileName,".png");
	data = (char *) gdImagePngPtr(gd_image, &size);
	break;
  case FMT_JPG:
	//strcat(FileName,".jpg");
	data = (char *) gdImageJpegPtr(gd_image, &size,tLXOptions->iJpegQuality);
	break;
  case FMT_GIF:
	//strcat(FileName,".gif");
	data = (char *) gdImageGifPtr(gd_image, &size);
	break;
  default:
	  data = (char *) gdImagePngPtr(gd_image, &size);
	  break;
  }

  if (!data) {
    return false;
  }
  if ((int)fwrite(data, 1, size, out) != size) {
    return false;
  }

  // Write info about the game
  if (Tournament && cServer)  {
	  cServer->setTakeScreenshot(false);
	  cServer->setScreenshotToken(true);
	  if (!cServer->WriteLogToFile(out))
		  return false;
  }

  if (fclose(out) != 0) {
    return false;
  }

  // Free everything
  gdFree(data);

  gdImageDestroy(gd_image);

  return true;
}
