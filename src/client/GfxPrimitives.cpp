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


#include <assert.h>
#include <gd.h>

#include "defs.h"
#include "LieroX.h"
#include "GfxPrimitives.h"
#include "CServer.h"
#include "Cache.h"
#include "FindFile.h"
#include "StringUtils.h"

int iSurfaceFormat = SDL_SWSURFACE;

/////////////////
// Put the pixel alpha blended with the background
void PutPixelA(SDL_Surface *bmpDest, int x, int y, Uint32 colour, Uint8 a)  {
	if (!(byte)(~a))  { // Fully opaque
		PutPixel(bmpDest,x,y,colour);
		return;
	}

	static Uint32 R,G,B,A;
	register Uint64 pixel = GetPixel(bmpDest,x,y);

	#define Rmask bmpDest->format->Rmask
	#define Gmask bmpDest->format->Gmask
	#define Bmask bmpDest->format->Bmask
	#define Amask bmpDest->format->Amask

	R = ((((pixel & Rmask) * (255-a) >> 8) + (( (colour & Rmask) * a >> 8))) & Rmask);
	G = ((((pixel & Gmask) * (255-a) >> 8) + (( (colour & Gmask) * a >> 8))) & Gmask);
	B = ((((pixel & Bmask) * (255-a) >> 8) + (( (colour & Bmask) * a >> 8))) & Bmask);
	A = pixel & Amask;

	pixel = (R|G|B|A);
	
	#undef Rmask
	#undef Gmask
	#undef Bmask
	#undef Amask

	PutPixel(bmpDest,x,y,pixel);
}

//////////////////////
// Set a color key for alpha surface (SDL_SetColorKey does not work for alpha surfaces)
void SetColorKeyAlpha(SDL_Surface *dst, Uint8 r, Uint8 g, Uint8 b)
{
	// Just set transparent alpha to pixels that match the color key
	register Uint8 *pxr = (Uint8 *)dst->pixels;
	register Uint8 *px;
	int x,y;
	Uint8 dr,dg,db;
	Uint32 pixel;
	for (y=0;y<dst->h;y++,pxr+=dst->pitch)  {
		px = pxr;
		for (x=0;x<dst->w;x++,px+=dst->format->BytesPerPixel)  {
			pixel = GetPixelFromAddr(px,dst->format->BytesPerPixel);
			SDL_GetRGB(pixel,dst->format,&dr,&dg,&db);
			if (!(r+g+b-dr-dg-db))  {  // Key?
				pixel &= (dst->format->Amask & SDL_ALPHA_TRANSPARENT) | dst->format->Rmask | dst->format->Gmask | dst->format->Bmask;
				memcpy(px,&pixel,dst->format->BytesPerPixel);
			}
		}
	}
			
}

///////////////////
// Draw the image mirrored with a huge amount of options
void DrawImageAdv_Mirror(SDL_Surface *bmpDest, SDL_Surface *bmpSrc, int sx, int sy, int dx, int dy, int w, int h)
{
	// Warning: Both surfaces have to have same bpp!
	assert(bmpDest->format->BytesPerPixel == bmpSrc->format->BytesPerPixel);

	int x,y,c;

	// Lock the surfaces
	if(SDL_MUSTLOCK(bmpDest))
		SDL_LockSurface(bmpDest);
	if(SDL_MUSTLOCK(bmpSrc))
		SDL_LockSurface(bmpSrc);


	// Warning: Doesn't do clipping on the source surface
	int cx = bmpDest->clip_rect.x;
	int cy = bmpDest->clip_rect.y;
	int cex = cx + bmpDest->clip_rect.w;
	int cey = cy + bmpDest->clip_rect.h;

	// Do clipping on the DEST surface
	if(dx+w<cx || dy+h<cy) return;
	if(dx>=cex || dy>=cey) return;

	if(dx<cx) {
		c = cx-dx;
		dx=cx;
		w-=c;
	}
	if(dy<cy) {
		c = cy-dy;
		dy=cy;
		h-=c;
	}

	if(dx+w > cex) {
		c = dx+w - cex;
		w-=c;
		sx+=c;
	}
	if(dy+h > cey) {
		c = dy+h - cey;
		h-=c;
		sy+=c;
	}

	Uint8 *TrgPix = (Uint8 *)bmpDest->pixels + dy*bmpDest->pitch + dx*bmpDest->format->BytesPerPixel;
	Uint8 *SrcPix = (Uint8 *)bmpSrc->pixels +  sy*bmpSrc->pitch + sx*bmpSrc->format->BytesPerPixel;

	register byte bpp = bmpDest->format->BytesPerPixel;

	register Uint8 *sp,*tp;
	for(y=0;y<h;y++) {

		sp = SrcPix;
		tp = TrgPix + w*bpp;
		for(x=0;x<w;x++) {
			// Copy the pixel
			tp-=bpp;
			memcpy(tp,sp,bpp);
			sp+=bpp;
		}

		SrcPix+=bmpSrc->pitch;
		TrgPix+=bmpDest->pitch;
	}

	// Unlock em
	if(SDL_MUSTLOCK(bmpDest))
		SDL_UnlockSurface(bmpDest);
	if(SDL_MUSTLOCK(bmpSrc))
		SDL_UnlockSurface(bmpSrc);
}


///////////////////
// Draws a sprite doubly stretched
void DrawImageStretch2(SDL_Surface *bmpDest, SDL_Surface *bmpSrc, int sx, int sy, int dx, int dy, int w, int h)
{
	assert(bmpDest->format->BytesPerPixel == bmpSrc->format->BytesPerPixel);
	
	int x,y;

	// Lock the surfaces
	if(SDL_MUSTLOCK(bmpDest))
		SDL_LockSurface(bmpDest);
	if(SDL_MUSTLOCK(bmpSrc))
		SDL_LockSurface(bmpSrc);

	// Source clipping
	if (sx < 0)  { w+=sx; sx=0; }
	else if((sx+w) > bmpSrc->w)  { w = bmpSrc->w -sx; } // TODO: not exactly what perhaps is intended...
	if (sy < 0) { h+=sy; sy=0;}
	else if((sy+h) > bmpSrc->h)  { h = bmpSrc->h -sy; }

	// Dest clipping
	if (dx<0)  { sx-=dx; w+=dx; dx=0;}
	else if (dx+w > bmpDest->w)  { w = bmpDest->w-dx; }
	if (dy<0)  { sy-=dy; h+=dy; dy=0;}
	else if (dy+h > bmpDest->h)  { h = bmpDest->h-dy; }

	Uint8 *TrgPix = (Uint8 *)bmpDest->pixels + dy*bmpDest->pitch + dx*bmpDest->format->BytesPerPixel;
	Uint8 *SrcPix = (Uint8 *)bmpSrc->pixels +  sy*bmpSrc->pitch + sx*bmpSrc->format->BytesPerPixel;

	register Uint8 *sp,*tp_x,*tp_y;
	int doublepitch = bmpDest->pitch*2;
	register byte bpp = bmpDest->format->BytesPerPixel;

    for(y=0;y<h;y++) {

		sp = SrcPix;
		tp_x = TrgPix;
		tp_y = tp_x+bmpDest->pitch;
		for(x=0;x<w;x++) {
            // Copy the 1 source pixel into a 4 pixel block on the destination surface
			memcpy(tp_x,sp,bpp);
			tp_x += bpp;
			memcpy(tp_x,sp,bpp);
			memcpy(tp_y,sp,bpp);
			tp_y += bpp;
			memcpy(tp_y,sp,bpp);
			tp_x += bpp;
			tp_y += bpp;
			sp+=bpp;
		}
		TrgPix += doublepitch;
		SrcPix += bmpSrc->pitch;
	}

	// Unlock em
	if(SDL_MUSTLOCK(bmpDest))
		SDL_UnlockSurface(bmpDest);
	if(SDL_MUSTLOCK(bmpSrc))
		SDL_UnlockSurface(bmpSrc);
}


///////////////////
// Draws a sprite doubly stretched with colour key
void DrawImageStretch2Key(SDL_Surface *bmpDest, SDL_Surface *bmpSrc, int sx, int sy, int dx, int dy, int w, int h, Uint32 key)
{
	assert(bmpDest->format->BytesPerPixel == bmpSrc->format->BytesPerPixel);

	int x,y;
	int c;

	// Source clipping
	if (sx < 0)  { w+=sx; sx=0; }
	else if((sx+w) > bmpSrc->w)  { w = bmpSrc->w -sx; } // TODO: not exactly what perhaps is intended...
	if (sy < 0) { h+=sy; sy=0;}
	else if((sy+h) > bmpSrc->h)  { h = bmpSrc->h -sy; }
	
	int cx = bmpDest->clip_rect.x;
	int cy = bmpDest->clip_rect.y;
	int cex = cx+bmpDest->clip_rect.w;
	int cey = cy+bmpDest->clip_rect.h;

	// Do clipping on the DEST surface
	if(dx+w*2<cx || dy+h*2<cy) return;
	if(dx>=cex || dy>=cey) return;

	if(dx<cx) {	c=cx-dx;	dx=cx; c/=2; sx+=c;	w-=c; }
	if(dy<cy) {	c=cy-dy;	dy=cy; c/=2; sy+=c;	h-=c; }

	if(dx+w*2>cex) {	c=(dx+w*2)-cex;	c/=2; w-=c; }
	if(dy+h*2>cey) {	c=(dy+h*2)-cey;	c/=2; h-=c; }

	// Lock the surfaces
	if(SDL_MUSTLOCK(bmpDest))
		SDL_LockSurface(bmpDest);
	if(SDL_MUSTLOCK(bmpSrc))
		SDL_LockSurface(bmpSrc);

	Uint8 *TrgPix = (Uint8 *)bmpDest->pixels + dy*bmpDest->pitch + dx*bmpDest->format->BytesPerPixel;
	Uint8 *SrcPix = (Uint8 *)bmpSrc->pixels +  sy*bmpSrc->pitch + sx*bmpSrc->format->BytesPerPixel;

	register Uint8 *sp,*tp_x,*tp_y;

	// Pre-calculate some things, so the loop is faster
	int doublepitch = bmpDest->pitch*2;
	register byte bpp = bmpDest->format->BytesPerPixel;
	register byte doublebpp = bpp*2;

    for(y=0;y<h;y++) {

		sp = SrcPix;
		tp_x = TrgPix;
		tp_y = tp_x+bmpDest->pitch;
		for(x=0;x<w;x++) {
			if (memcmp(&key,sp,bpp))  {
				// Copy the 1 source pixel into a 4 pixel block on the destination surface
				memcpy(tp_x,sp,bpp);
				tp_x += bpp;
				memcpy(tp_x,sp,bpp);
				tp_x += bpp;
				memcpy(tp_y,sp,bpp);
				tp_y += bpp;
				memcpy(tp_y,sp,bpp);
				tp_y += bpp;
				sp += bpp;
			} else {
				// Skip the transparent pixel
				sp+=bpp;
				tp_x += doublebpp;
				tp_y += doublebpp;
			}
		}
		TrgPix += doublepitch;
		SrcPix += bmpSrc->pitch;
	}


	// Unlock em
	if(SDL_MUSTLOCK(bmpDest))
		SDL_UnlockSurface(bmpDest);
	if(SDL_MUSTLOCK(bmpSrc))
		SDL_UnlockSurface(bmpSrc);
}


///////////////////
// Draws a sprite mirrored doubly stretched with colour key
void DrawImageStretchMirrorKey(SDL_Surface *bmpDest, SDL_Surface *bmpSrc, int sx, int sy, int dx, int dy, int w, int h, Uint32 key)
{
	assert(bmpDest->format->BytesPerPixel == bmpSrc->format->BytesPerPixel);
	
	int x,y;
	int c;

	// Warning: Doesn't do clipping on the source surface

	int cx = bmpDest->clip_rect.x;
	int cy = bmpDest->clip_rect.y;
	int cex = cx+bmpDest->clip_rect.w;
	int cey = cy+bmpDest->clip_rect.h;

	// Do clipping on the DEST surface
	if(dx+w*2<cx || dy+h*2<cy) return;
	if(dx>=cex || dy>=cey) return;

	if(dx<cx) {	c=cx-dx; dx+=c; w-=c;}
	if(dy<cy) {	c=cy-dy; dy+=c; h-=c;}

	if(dx+w*2>cex) {	c=(dx+w*2)-cex;	c/=2; sx+=c; w-=c;}
	if(dy+h*2>cey) {	c=(dy+h*2)-cey;	c/=2; sy+=c; h-=c;}


	// Lock the surfaces
	if(SDL_MUSTLOCK(bmpDest))
		SDL_LockSurface(bmpDest);
	if(SDL_MUSTLOCK(bmpSrc))
		SDL_LockSurface(bmpSrc);


	Uint8 *TrgPix = (Uint8 *)bmpDest->pixels + dy*bmpDest->pitch + dx*bmpDest->format->BytesPerPixel;
	Uint8 *SrcPix = (Uint8 *)bmpSrc->pixels + sy*bmpSrc->pitch + sx*bmpSrc->format->BytesPerPixel;

	register Uint8 *sp,*tp_x,*tp_y;

	// Pre-calculate some things, so the loop is faster
	int doublepitch = bmpDest->pitch*2;
	register byte bpp = bmpDest->format->BytesPerPixel;
	register byte doublebpp = bpp*2;
	int realw = w*bpp;

    for(y=0;y<h;y++) {

		sp = SrcPix;
		tp_x = TrgPix+realw;
		tp_y = tp_x+bmpDest->pitch;
		for(x=0;x<w;x++) {
			if (memcmp(&key,sp,bmpDest->format->BytesPerPixel))  {
				// Non-transparent
				// Copy the 1 source pixel into a 4 pixel block on the destination surface
				memcpy(tp_x,sp,bpp);
				memcpy(tp_x,sp,bpp);
				memcpy(tp_y,sp,bpp);
				memcpy(tp_y,sp,bpp);
			}
			// Skip to next pixel
			sp+=bpp;
			tp_x -= doublebpp;
			tp_y -= doublebpp;
		}
		TrgPix += doublepitch;
		SrcPix += bmpSrc->pitch;
	}


	// Unlock em
	if(SDL_MUSTLOCK(bmpDest))
		SDL_UnlockSurface(bmpDest);
	if(SDL_MUSTLOCK(bmpSrc))
		SDL_UnlockSurface(bmpSrc);
}

/*
 * Special line drawing
 */

int ropecolour = 0;
int ropealt = 0;

///////////////////
// Put a pixel on the surface (while checking for clipping)
inline void RopePutPixel(SDL_Surface *bmpDest, int x, int y, Uint32 colour)
{
	// Warning: lock the surface before calling this!

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

	static Uint32 ropecols[2] = { MakeColour(160,80,0), MakeColour(200,100,0) };
	ropecolour = !ropecolour;
	colour = ropecols[ropecolour];

	//boxColor(bmpDest, x,y,x+1,y+1, colour);
	//DrawRectFill(bmpDest,x,y,x+2,y+2,colour);
	Uint8 *px = (Uint8 *)bmpDest->pixels+bmpDest->pitch*y+x*bmpDest->format->BytesPerPixel;
	Uint8 *px2 = px+bmpDest->pitch;
	memcpy(px,&colour,bmpDest->format->BytesPerPixel);
	memcpy(px+bmpDest->format->BytesPerPixel,&colour,bmpDest->format->BytesPerPixel);
	memcpy(px2,&colour,bmpDest->format->BytesPerPixel);
	memcpy(px2+bmpDest->format->BytesPerPixel,&colour,bmpDest->format->BytesPerPixel);
}


int beamalt = 0;

///////////////////
// Put a pixel on the surface (while checking for clipping)
inline void BeamPutPixel(SDL_Surface *bmpDest, int x, int y, Uint32 colour)
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

	//boxColor(bmpDest, x,y,x+1,y+1, colour);
	Uint8 *px = (Uint8 *)bmpDest->pixels+bmpDest->pitch*y+x*bmpDest->format->BytesPerPixel;
	Uint8 *px2 = px+bmpDest->pitch;
	memcpy(px,&colour,bmpDest->format->BytesPerPixel);
	memcpy(px+bmpDest->format->BytesPerPixel,&colour,bmpDest->format->BytesPerPixel);
	memcpy(px2,&colour,bmpDest->format->BytesPerPixel);
	memcpy(px2+bmpDest->format->BytesPerPixel,&colour,bmpDest->format->BytesPerPixel);
}


int laseralt = 0;

///////////////////
// Put a pixel on the surface (while checking for clipping)
inline void LaserSightPutPixel(SDL_Surface *bmpDest, int x, int y, Uint32 colour)
{
	laseralt++;
	laseralt %= GetRandomInt(35)+1;

	if(laseralt)
		return;

	if(!bmpDest)
		return;


	static Uint32 laseraltcols[] = { MakeColour(190,0,0), MakeColour(160,0,0) };
	colour = laseraltcols[ GetRandomInt(1) ];

	// Snap to nearest 2nd pixel
	x -= x % 2;
	y -= y % 2;

	if( x < 0 || y < 0 )
		return;
	if( x < bmpDest->clip_rect.x || y < bmpDest->clip_rect.y )
		return;
	if( x >= bmpDest->clip_rect.x+bmpDest->clip_rect.w || y >= bmpDest->clip_rect.y+bmpDest->clip_rect.h )
		return;

	//boxColor(bmpDest, x,y,x+1,y+1, colour);
	Uint8 *px = (Uint8 *)bmpDest->pixels+bmpDest->pitch*y+x*bmpDest->format->BytesPerPixel;
	Uint8 *px2 = px+bmpDest->pitch;
	memcpy(px,&colour,bmpDest->format->BytesPerPixel);
	memcpy(px+bmpDest->format->BytesPerPixel,&colour,bmpDest->format->BytesPerPixel);
	memcpy(px2,&colour,bmpDest->format->BytesPerPixel);
	memcpy(px2+bmpDest->format->BytesPerPixel,&colour,bmpDest->format->BytesPerPixel);
}

////////////////////
// Perform a line draw using a put pixel callback
// Grabbed from allegro
inline void perform_line(SDL_Surface *bmp, int x1, int y1, int x2, int y2, int d, void (*proc)(SDL_Surface *, int, int, Uint32))
{
   int dx = x2-x1;
   int dy = y2-y1;
   int i1, i2;
   int x, y;
   int dd;

   if (SDL_MUSTLOCK(bmp))
	   SDL_LockSurface(bmp);

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

   if (SDL_MUSTLOCK(bmp))
	   SDL_UnlockSurface(bmp);
}


inline void secure_perform_line(SDL_Surface* bmpDest, int x1, int y1, int x2, int y2, Uint32 color, void (*proc)(SDL_Surface *, int, int, Uint32)) {
	int sx, sy, dx, dy, t;
	
	SDL_Rect rect = bmpDest->clip_rect;
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

	perform_line(bmpDest, x1, y1, x2, y2, color, proc);
}

// Draw horizontal line
void DrawHLine(SDL_Surface *bmpDest, int x, int x2, int y, Uint32 colour) {
	// TODO: does this need more improvement/optimisation ?
	//secure_perform_line(bmpDest, x, y, x2, y, colour, PutPixel);  // slow
	if (x < 0) x = 0;
	else if (x >= bmpDest->w) x=bmpDest->w-1;

	if (x2 < 0) x2 = 0;
	else if (x2 >= bmpDest->w) x2=bmpDest->w-1;

	if (y < 0) y = 0;
	else if (y >= bmpDest->h) y=bmpDest->h-1;

	if (x2 < x)  {
		static int tmp;
		tmp = x;
		x = x2;
		x2 = tmp;
	}

	register byte bpp = (byte)bmpDest->format->BytesPerPixel;
	register uchar *px2 = (uchar *)bmpDest->pixels+bmpDest->pitch*y+bpp*x2;

	SDL_LockSurface(bmpDest);
	for (register uchar *px= (uchar *)bmpDest->pixels+bmpDest->pitch*y+bpp*x;px <= px2;px+=bpp)  {
		memcpy(px,&colour,bpp);
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		EndianSwap(px,bpp);
#endif
	}

	SDL_UnlockSurface(bmpDest);

}

// Draw vertical line
void DrawVLine(SDL_Surface *bmpDest, int y, int y2, int x, Uint32 colour) {
	// TODO: does this need more improvement/optimisation ?
//	secure_perform_line(bmpDest, x, y, x, y2, colour, PutPixel);  // slow
	if (x < 0) x = 0;
	else if (x >= bmpDest->w) x=bmpDest->w-1;

	if (y < 0) y = 0;
	else if (y >= bmpDest->h) y=bmpDest->h-1;

	if (y2 < 0) y2 = 0;
	else if (y2 >= bmpDest->h) y2=bmpDest->h-1;

	if (y2 < y)  {
		static int tmp;
		tmp = y;
		y = y2;
		y2 = tmp;
	}

	register ushort pitch = (ushort)bmpDest->pitch;
	register byte bpp = (byte)bmpDest->format->BytesPerPixel;
	register uchar *px2 = (uchar *)bmpDest->pixels+pitch*y2+bpp*x;

	SDL_LockSurface(bmpDest);
	for (register uchar *px= (uchar *)bmpDest->pixels+pitch*y+bpp*x;px <= px2;px+=pitch)
		memcpy(px,&colour,bpp);

	SDL_UnlockSurface(bmpDest);
}

// Line drawing
void DrawLine(SDL_Surface *dst, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint32 color) {
	// TODO: does this need more improvement/optimisation ?
	secure_perform_line(dst, x1, y1, x2, y2, color, PutPixel);
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
 *  Image loading/saving routines
 */

///////////////////
// Load an image
SDL_Surface *LoadImage(const std::string& _filename, bool withalpha)
{
	// Has this been already loaded?
	for (std::vector<CCache>::iterator it = Cache.begin(); it != Cache.end(); it++)  {
		if (it->getType() == CCH_IMAGE)  {
			if (stringcasecmp(it->getFilename(),_filename) == 0)
				if (it->GetImage())
					return it->GetImage();
		}
	}

	// Didn't find one already loaded? Create a new one
	CCache tmp;
	SDL_Surface *result = tmp.LoadImgBPP(_filename,withalpha);
	Cache.push_back(tmp);



	return result;
}

///////////////////////
// Converts the SDL_surface to gdImagePtr
gdImagePtr SDLSurface2GDImage(SDL_Surface* src) {
	gdImagePtr gd_image = gdImageCreateTrueColor(src->w,src->h);
	if(!gd_image)
		return NULL;

	Uint32 rmask, gmask, bmask;
	// Works also for little endian
	rmask=0x00FF0000; gmask=0x0000FF00; bmask=0x000000FF;
	
	SDL_Surface* formated = SDL_CreateRGBSurface(SDL_SWSURFACE, src->w, src->h, 32, rmask, gmask, bmask, 0);
	if(!formated)
		return NULL;

	// convert it to the new format (32 bpp)
	DrawImageEx(formated, src, 0, 0, src->w, src->h);
	
	for(int y = 0; y < src->h; y++) {
		memcpy(gd_image->tpixels[y], (uchar*)formated->pixels + y*formated->pitch, formated->pitch);	
	}
	
	SDL_FreeSurface(formated);
	
	return gd_image;
}

///////////////////////
// Saves the surface into the specified file with the specified format
bool SaveSurface ( SDL_Surface *image, const std::string& FileName, int Format, bool Tournament )
{
	if ( Format == FMT_BMP )
	{
		std::string abs_fn = GetWriteFullFileName ( FileName, true );
		SDL_SaveBMP ( image, abs_fn.c_str() );

		// Log
		if ( Tournament && cServer )
		{
			cServer->setTakeScreenshot ( false );
			cServer->setScreenshotToken ( true );

			FILE *f = OpenGameFile ( FileName,"ab" );
			if ( !f )
				return false;
			if ( !cServer->WriteLogToFile ( f ) )
			{
				fclose ( f );
				return false;
			}
			fclose ( f );

		}

		return true;
	}

	gdImagePtr gd_image = NULL;

	gd_image = SDLSurface2GDImage ( image );
	if ( !gd_image )
		return false;

	// Save the image
	FILE *out;
	int s;
	char *data = NULL;
	out = OpenGameFile ( FileName, "wb" );
	if ( !out )
	{
		return false;
	}

	switch ( Format )
	{
		case FMT_PNG:
			data = ( char * ) gdImagePngPtr ( gd_image, &s );
			break;
		case FMT_JPG:
			data = ( char * ) gdImageJpegPtr ( gd_image, &s,tLXOptions->iJpegQuality );
			break;
		case FMT_GIF:
			data = ( char * ) gdImageGifPtr ( gd_image, &s );
			break;
		default:
			data = ( char * ) gdImagePngPtr ( gd_image, &s );
			break;
	}

	size_t size = s>0?s:-s;
	if ( !data )
	{
		return false;
	}
	if ( fwrite ( data, 1, size, out ) != size )
	{
		return false;
	}

	// Write info about the game
	if ( Tournament && cServer )
	{
		cServer->setTakeScreenshot ( false );
		cServer->setScreenshotToken ( true );
		if ( !cServer->WriteLogToFile ( out ) )
			return false;
	}

	if ( fclose ( out ) != 0 )
	{
		return false;
	}

	// Free everything
	gdFree ( data );

	gdImageDestroy ( gd_image );

	return true;
}
