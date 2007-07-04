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

///////////////////////
// Copies area from one image to another (not blitting so the alpha values are kept!)
void CopySurface(SDL_Surface* dst, SDL_Surface* src, int sx, int sy, int dx, int dy, int w, int h)
{
	// The surfaces must have same properties
	assert(src->format->Amask == dst->format->Amask);
	assert(src->format->Rmask == dst->format->Rmask);
	assert(src->format->Gmask == dst->format->Gmask);
	assert(src->format->Bmask == dst->format->Bmask);
	assert(src->format->BytesPerPixel == dst->format->BytesPerPixel);
	
	// Source clipping
	if (sx + w > src->w) {
		if (sx >= src->w) return;  // >= because copying area of 0px width makes no sense as well
		w = src->w - sx;
	}
	
	if (sy + h > src->h) {
		if (sy >= src->h) return;
		h = src->h - sy;
	}
	
	// Dest clipping
	if (dx + w > dst->w) {
		if (dx >= dst->w) return;
		w = dst->w - dx;
	}
	
	if (dy + h > dst->h) {
		if (dy >= dst->h) return;
		h = dst->h - dy;
	}
	
	// Initialize
	int lower_bound  = sy + h;
	int byte_bound = (w - 1)*src->format->BytesPerPixel;
	int src_pitch = src->pitch;
	int dst_pitch = dst->pitch;
	register Uint8* srcrow = (Uint8 *)src->pixels
		+ (sy * src->pitch) + (sx * src->format->BytesPerPixel);
	register Uint8* dstrow = (Uint8 *)dst->pixels
		+ (dy * dst->pitch) + (dx * dst->format->BytesPerPixel);
	
	// Copy row by row
	for (register int i = sy; i < lower_bound; ++i)  {
		memcpy(dstrow, srcrow, byte_bound);
		dstrow += dst_pitch;
		srcrow += src_pitch;
	}
}

template<typename T>
inline T force_in_range(T val, T min, T max) {
	return MIN(MAX(val, min), max);
}

inline Uint8 relative(Uint8 v1, Uint8 v2, Uint8 c) {
	return (Uint8)force_in_range(
		(((float)v1 * (float)c) + ((float)v2 * (float)(255 - c))) / 255.0f,
		0.0f, 255.0f);
}

// HINT: not threadsafe!
inline void PutPixelA(SDL_Surface *bmpDest, int x, int y, Uint32 colour, float a)  {
	static Uint8 R1, G1, B1, A1, R2, G2, B2;
	static float not_a;
	not_a = 1.0f - a;
	Uint8* px = (Uint8*)bmpDest->pixels + y * bmpDest->pitch + x * bmpDest->format->BytesPerPixel;
	SDL_GetRGBA(GetPixelFromAddr(px, bmpDest->format->BytesPerPixel), bmpDest->format, &R1, &G1, &B1, &A1);
	SDL_GetRGB(colour, bmpDest->format, &R2, &G2, &B2);
	PutPixelToAddr(px, SDL_MapRGBA(bmpDest->format,
		(Uint8) force_in_range(R1 * a + R2 * not_a, 0.0f, 255.0f),
		(Uint8) force_in_range(G1 * a + G2 * not_a, 0.0f, 255.0f),
		(Uint8) force_in_range(B1 * a + B2 * not_a, 0.0f, 255.0f),
		A1), bmpDest->format->BytesPerPixel);
}

/////////////////
// Put the pixel alpha blended with the background
// NOTICE: colour has to be of the same format as bmpDest->format
void PutPixelA(SDL_Surface *bmpDest, int x, int y, Uint32 colour, Uint8 a)  {
	PutPixelA(bmpDest, x, y, colour, (float)a / 255.0f);
}


//////////////////////
// Set a color key for alpha surface (SDL_SetColorKey does not work for alpha surfaces)
void SetColorKeyAlpha(SDL_Surface* dst, Uint8 r, Uint8 g, Uint8 b) {
	// Just set transparent alpha to pixels that match the color key
	register Uint8* pxr = (Uint8*)dst->pixels;
	register Uint8* px;
	int x, y;
	Uint32 colorkey = SDL_MapRGBA(dst->format, r, g, b, 0);
	for(y = 0; y < dst->h; y++, pxr += dst->pitch)  {
		px = pxr;
		for(x = 0; x < dst->w; x++, px += dst->format->BytesPerPixel)  {
			if(EqualRGB(
					colorkey,
					GetPixelFromAddr(px, dst->format->BytesPerPixel),
					dst->format))
				PutPixelToAddr(px, colorkey, dst->format->BytesPerPixel);
		}
	}

	// Makes the dst->format->colorkey to match specified colorkey
	// TODO: why are we doing this here? is it realy safe? and nevertheless, it's useless, isn't it?
	// It's not useless, because without this the COLORKEY() macro doesn't work and it's necessary in
	// some parts of code (mainly worm graphics) and this won't cause any harm at all (look at source
	// of SDL_SetColorKey)
	SDL_SetColorKey(dst, SDL_SRCCOLORKEY, colorkey);

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

	short bpp = bmpDest->format->BytesPerPixel;

	register Uint8 *sp,*tp;
	for(y=0;y<h;y++) {

		sp = SrcPix;
		tp = TrgPix + w*bpp;
		for(x = 0; x < w; x++) {
			// Copy the pixel
			tp -= bpp;
			memcpy(tp, sp, bpp);
			sp += bpp;
		}

		SrcPix += bmpSrc->pitch;
		TrgPix += bmpDest->pitch;
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
	if (sx >= bmpSrc->w || (sx + w) <= 0) return;
	if (sy >= bmpSrc->h || (sy + h) <= 0) return;
	if (sx < 0)  { w+=sx; sx=0; }
	else if((sx+w) > bmpSrc->w)  { w = bmpSrc->w -sx; }
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
	key = SDLColourToNativeColour(key);

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
	key = SDLColourToNativeColour(key);

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

/////////////////////////
// Draw the image resized
void DrawImageResizedAdv( SDL_Surface *bmpDest, SDL_Surface *bmpSrc, int sx, int sy, int dx, int dy, int sw, int sh, float xratio, float yratio)
{
	int dw = Round((float)sw * xratio);
	int dh = Round((float)sh * yratio);

	// Clipping rectangle
	SDL_Rect src_cliprect, dst_cliprect;
	SDL_GetClipRect(bmpSrc, &src_cliprect);
	SDL_GetClipRect(bmpDest, &dst_cliprect);

	// Source clipping
	if (sx > (src_cliprect.x + src_cliprect.w) || (sx + sw) <= 0) return;
	if (sy > (src_cliprect.y + src_cliprect.h) || (sy + sh) <= 0) return;
	if ( (sx + sw) > (src_cliprect.x + src_cliprect.w) )
		sw = (src_cliprect.x + src_cliprect.w) - sx;
	if ( (sy + sh) > (src_cliprect.y + src_cliprect.h) )
		sh = (src_cliprect.y + src_cliprect.h) - sy;
	if (sx < 0)  {
		sw += sx;
		sx = 0;
	}
	if (sy < 0)  {
		sh += sy;
		sy = 0;
	}

	// Destination clipping
	if (dx > (dst_cliprect.x + dst_cliprect.w) || (dx + dw) <= 0) return;
	if (dy > (dst_cliprect.y + dst_cliprect.h) || (dy + dh) <= 0) return;
	if ( (dx + dw) > (dst_cliprect.x + dst_cliprect.w) )
		dw = (dst_cliprect.x + dst_cliprect.w) - dx;
	if ( (dy + dh) > (dst_cliprect.y + dst_cliprect.h) )
		dh = (dst_cliprect.y + dst_cliprect.h) - dy;
	if (dx < 0)  {
		dw += dx;
		dx = 0;
	}
	if (dy < 0)  {
		dh += dy;
		dy = 0;
	}

	float xstep = (float)sw/(float)dw; // X step we'll do on the source surface
	float ystep = (float)sh/(float)dh; // Y step we'll do on the source surface
	float src_xstart = Round((float)sx/xstep) * xstep; // Starting X coordinate on source surface
	float src_ystart = Round((float)sy/ystep) * ystep; // Starting Y coordinate on source surface
	float src_x, src_y;  // Current coordinates on the source surface
	int dest_x = dx; // Current X on dest surface
	int dest_y = dy; // Current Y on dest surface
	int dest_x2 = dx + dw;  // Right bound
	int dest_y2 = dy + dh;  // Bottom bound

	// Lock the surfaces
	if (SDL_MUSTLOCK(bmpSrc))
		SDL_LockSurface(bmpSrc);
	if (SDL_MUSTLOCK(bmpDest))
		SDL_LockSurface(bmpDest);

	// Pixels
	register Uint8 *src_px = NULL;
	register Uint8 *src_pxrow = NULL;
	register Uint8 *dst_px = NULL;
	register Uint8 *dst_pxrow = (Uint8 *)bmpDest->pixels + (dy * bmpDest->pitch) + (dx * bmpDest->format->BytesPerPixel);
	register byte bpp = (byte)bmpDest->format->BytesPerPixel;

	for (src_y = src_ystart; dest_y < dest_y2; dest_y++)  {
		src_pxrow = (Uint8 *)bmpSrc->pixels + Round(src_y) * bmpSrc->pitch;
		dst_px = dst_pxrow;

		// Copy the row
		for (dest_x = dx, src_x = src_xstart; dest_x < dest_x2; dest_x++)  {
			src_px = src_pxrow + Round(src_x) * bpp;
			memcpy(dst_px, src_px, bpp);
			src_x += xstep;
			dst_px += bpp;
		}

		src_y += ystep;
		dst_pxrow += bmpDest->pitch;
	}

	// Unlock
	if (SDL_MUSTLOCK(bmpSrc))
		SDL_UnlockSurface(bmpSrc);
	if (SDL_MUSTLOCK(bmpDest))
		SDL_UnlockSurface(bmpDest);
}

////////////////////////
// Draws the image nicely resampled
// blur - the greater the value is, the more will be the destination image blurred
void DrawImageResampledAdv( SDL_Surface *bmpDest, SDL_Surface *bmpSrc, int sx, int sy, int dx, int dy, int sw, int sh, float xratio, float yratio, float blur)
{
	// How this works:
	// We take four neighbour pixels from the source and make average of them

	// For 1px width/height no one will notice the difference and it will 
	// avoid many checks here
	if (sw < 2 || sh < 2)  {
		DrawImageResizedAdv(bmpDest, bmpSrc, sx, sy, dx, dy, sw, sh, xratio, yratio);
		return;
	}

	int dw = Round((float)sw * xratio);
	int dh = Round((float)sh * yratio);

	// Clipping rectangle
	SDL_Rect src_cliprect, dst_cliprect;
	SDL_GetClipRect(bmpSrc, &src_cliprect);
	SDL_GetClipRect(bmpDest, &dst_cliprect);

	// Source clipping
	if (sx > (src_cliprect.x + src_cliprect.w) || (sx + sw) <= 0) return;
	if (sy > (src_cliprect.y + src_cliprect.h) || (sy + sh) <= 0) return;
	if ( (sx + sw) > (src_cliprect.x + src_cliprect.w) )
		sw = (src_cliprect.x + src_cliprect.w) - sx;
	if ( (sy + sh) > (src_cliprect.y + src_cliprect.h) )
		sh = (src_cliprect.y + src_cliprect.h) - sy;
	if (sx < 0)  {
		sw += sx;
		sx = 0;
	}
	if (sy < 0)  {
		sh += sy;
		sy = 0;
	}

	// Destination clipping
	if (dx > (dst_cliprect.x + dst_cliprect.w) || (dx + dw) <= 0) return;
	if (dy > (dst_cliprect.y + dst_cliprect.h) || (dy + dh) <= 0) return;
	if ( (dx + dw) > (dst_cliprect.x + dst_cliprect.w) )
		dw = (dst_cliprect.x + dst_cliprect.w) - dx;
	if ( (dy + dh) > (dst_cliprect.y + dst_cliprect.h) )
		dh = (dst_cliprect.y + dst_cliprect.h) - dy;
	if (dx < 0)  {
		dw += dx;
		dx = 0;
	}
	if (dy < 0)  {
		dh += dy;
		dy = 0;
	}

	float xstep = (float)sw/(float)dw; // X step we'll do on the source surface
	float ystep = (float)sh/(float)dh; // Y step we'll do on the source surface
	float src_x = (float)sx; // Current X on source surface
	float src_y = (float)sy; // Current Y on source surface
	int dest_x = dx; // Current X on dest surface
	int dest_y = dy; // Current Y on dest surface
	int dest_x2 = dx + dw;  // Right bound
	int dest_y2 = dy + dh;  // Bottom bound
	float src_right_bound = sx + sw -1;  // Right bound on source
	float src_bottom_bound = sy + sh - 1;  // Bottom bound on source
	blur = 1.0f/blur;  // Actually, in the loop it's flipped
	float avg_divide = 5.0f + blur - 1;// To make the loop a bit faster, we precalculate this value, 5 = number of neighbor pixels + current pixel

	// Lock the surfaces
	if (SDL_MUSTLOCK(bmpSrc))
		SDL_LockSurface(bmpSrc);
	if (SDL_MUSTLOCK(bmpDest))
		SDL_LockSurface(bmpDest);

	// Pixels
	register Uint8 *src_px = NULL;
	register Uint8 *src_pxrow = NULL;
	register Uint8 *dst_px = NULL;
	register Uint8 *dst_pxrow = (Uint8 *)bmpDest->pixels + (dy * bmpDest->pitch) + (dx * bmpDest->format->BytesPerPixel);
	register byte bpp = (byte)bmpDest->format->BytesPerPixel;

	static Uint32 leftpixel, toppixel, rightpixel, bottompixel, curpixel;
	static Uint8 rleft, rtop, rright, rbottom, rcurrent, gleft, gtop, gright, gbottom, gcurrent, bleft, btop, bright, bbottom, bcurrent;
	static Uint32 avg_r, avg_g, avg_b;
	register byte first_row, first_col; // 0 when first row/column, 1 when not
	first_row = first_col = 0;

	for (; dest_y < dest_y2; dest_y++)  {
		src_pxrow = (Uint8 *)bmpSrc->pixels + (int)(src_y) * bmpSrc->pitch;
		dst_px = dst_pxrow;

		first_col = 0;

		// Copy the row
		for (dest_x = dx, src_x = (float)sx; dest_x < dest_x2; dest_x++)  {
			src_px = src_pxrow + (int)(src_x) * bpp;

			// If we're reading last row/column, instead of the pixel that is outside the source surface
			// (left, right, top or bottom) we use the current pixel
			// This ensures the  "* (expression)" where expression is 0 when we read the last source row or column
			curpixel = GetPixelFromAddr(src_px, bpp);
			leftpixel = GetPixelFromAddr(src_px - bpp*first_col, bpp);
			toppixel = GetPixelFromAddr(src_px - bmpSrc->pitch*first_row, bpp);
			rightpixel = GetPixelFromAddr(src_px + bpp*(src_x < src_right_bound), bpp);
			bottompixel = GetPixelFromAddr(src_px + bmpSrc->pitch*(src_y < src_bottom_bound), bpp);

			// Get the R/G/B values for each of the pixels
			SDL_GetRGB(curpixel, bmpSrc->format, &rcurrent, &gcurrent, &bcurrent);
			SDL_GetRGB(leftpixel, bmpSrc->format, &rleft, &gleft, &bleft);
			SDL_GetRGB(toppixel, bmpSrc->format, &rtop, &gtop, &btop);
			SDL_GetRGB(rightpixel, bmpSrc->format, &rright, &gright, &bright);
			SDL_GetRGB(bottompixel, bmpSrc->format, &rbottom, &gbottom, &bbottom);

			// Count the average color and put it on the dest surface
			// Blur depends on how much we prefer current pixel to the neighbours
			avg_r = (int)( (float)( (float)rcurrent*blur + rleft + rright + rtop + rbottom ) / avg_divide); 
			avg_g = (int)( (float)( (float)gcurrent*blur + gleft + gright + gtop + gbottom ) / avg_divide);
			avg_b = (int)( (float)( (float)bcurrent*blur + bleft + bright + btop + bbottom ) / avg_divide);
			PutPixelToAddr(dst_px, SDL_MapRGB(bmpDest->format, (Uint8)avg_r, (Uint8)avg_g, (Uint8)avg_b), bpp);


			src_x += xstep;
			dst_px += bpp;
			first_col = src_x > 0.5f; // Round(src_x) must be greater than zero
		}

		src_y += ystep;
		first_row = src_y > 0.5f;  // Round(src_y) must be greater than zero
		dst_pxrow += bmpDest->pitch;
	}

	// Unlock
	if (SDL_MUSTLOCK(bmpSrc))
		SDL_UnlockSurface(bmpSrc);
	if (SDL_MUSTLOCK(bmpDest))
		SDL_UnlockSurface(bmpDest);
}

/*
 * Special line and pixel drawing
 */


int ropecolour = 0;
int ropealt = 0;

///////////////////
// Put a pixel on the surface (while checking for clipping)
inline void RopePutPixelA(SDL_Surface *bmpDest, int x, int y, Uint32 colour, float alpha)
{
	// Warning: lock the surface before calling this!
	// Warning: passing NULL surface will cause a segfault

	//ropealt = !ropealt;

	/*if(ropealt)
		return;*/

	// Snap to nearest 2nd pixel
//	x -= x % 2;
//	y -= y % 2;

	if( x >= bmpDest->clip_rect.x+bmpDest->clip_rect.w || y >= bmpDest->clip_rect.y+bmpDest->clip_rect.h )
		return;

	static Uint32 ropecols[2] = { MakeColour(160,80,0), MakeColour(200,100,0) };
	if (ropealt)
		ropecolour = !ropecolour;
	colour = ropecols[ropecolour];
	//colour = SDLColourToNativeColour(colour);

	// No alpha weight, use direct pixel access (faster)
	if (alpha == 1)  {
		Uint8 *px = (Uint8 *)bmpDest->pixels+bmpDest->pitch*y+x*bmpDest->format->BytesPerPixel;
		Uint8 *px2 = px+bmpDest->pitch;
		memcpy(px,&colour,bmpDest->format->BytesPerPixel);
		memcpy(px+bmpDest->format->BytesPerPixel,&colour,bmpDest->format->BytesPerPixel);
		memcpy(px2,&colour,bmpDest->format->BytesPerPixel);
		memcpy(px2+bmpDest->format->BytesPerPixel,&colour,bmpDest->format->BytesPerPixel);
	} else { // Alpha, use special routine
		PutPixelA(bmpDest, x, y, colour, alpha);
		PutPixelA(bmpDest, x+1, y, colour, alpha);
		PutPixelA(bmpDest, x, y+1, colour, alpha);
		PutPixelA(bmpDest, x+1, y+1, colour, alpha);
	}
}

inline void RopePutPixel(SDL_Surface *bmpDest, int x, int y, Uint32 colour) { // For compatibility with perform_line
	RopePutPixelA(bmpDest, x, y, colour, 1); }


int beamalt = 0;

///////////////////
// Put a pixel on the surface (while checking for clipping)
inline void BeamPutPixelA(SDL_Surface *bmpDest, int x, int y, Uint32 colour, float alpha)
{
	/*beamalt = !beamalt;

	if(beamalt)
		return;*/

	// Snap to nearest 2nd pixel
	//x -= x % 2;
	//y -= y % 2;

	if( x < bmpDest->clip_rect.x || y < bmpDest->clip_rect.y )
		return;
	if( x >= bmpDest->clip_rect.x+bmpDest->clip_rect.w || y >= bmpDest->clip_rect.y+bmpDest->clip_rect.h )
		return;
	
	//colour = SDLColourToNativeColour(colour);

	// No alpha weight, use direct pixel access
	if (alpha == 1)  {
		Uint8 *px = (Uint8 *)bmpDest->pixels+bmpDest->pitch*y+x*bmpDest->format->BytesPerPixel;
		Uint8 *px2 = px+bmpDest->pitch;
		memcpy(px,&colour,bmpDest->format->BytesPerPixel);
		memcpy(px+bmpDest->format->BytesPerPixel,&colour,bmpDest->format->BytesPerPixel);
		memcpy(px2,&colour,bmpDest->format->BytesPerPixel);
		memcpy(px2+bmpDest->format->BytesPerPixel,&colour,bmpDest->format->BytesPerPixel);
	} else { // Alpha weight, use special routine
		PutPixelA(bmpDest, x, y, colour, alpha);
		PutPixelA(bmpDest, x+1, y, colour, alpha);
		PutPixelA(bmpDest, x, y+1, colour, alpha);
		PutPixelA(bmpDest, x+1, y+1, colour, alpha);
	}
}

inline void BeamPutPixel(SDL_Surface *bmpDest, int x, int y, Uint32 colour) { // For compatibility with perform_line
	BeamPutPixelA(bmpDest, x, y, colour, 1); }


int laseralt = 0;

///////////////////
// Put a pixel on the surface (while checking for clipping)
inline void LaserSightPutPixel(SDL_Surface *bmpDest, int x, int y, Uint32 colour)
{
	laseralt++;
	laseralt %= GetRandomInt(35)+1;

	if(laseralt)
		return;

	static Uint32 laseraltcols[] = { MakeColour(190,0,0), MakeColour(160,0,0) };
	colour = laseraltcols[ GetRandomInt(1) ];
	colour = SDLColourToNativeColour(colour);

	// Snap to nearest 2nd pixel
	x -= x % 2;
	y -= y % 2;

	if( x < bmpDest->clip_rect.x || y < bmpDest->clip_rect.y )
		return;
	if( x >= bmpDest->clip_rect.x+bmpDest->clip_rect.w || y >= bmpDest->clip_rect.y+bmpDest->clip_rect.h )
		return;

	// Laser sight is never antialiased
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

	static Uint8 r,g,b;
	GetColour3(colour,SDL_GetVideoSurface(),&r,&g,&b);
	Uint32 friendly_col = SDL_MapRGB(bmpDest->format,r,g,b);

	register byte bpp = (byte)bmpDest->format->BytesPerPixel;
	register uchar *px2 = (uchar *)bmpDest->pixels+bmpDest->pitch*y+bpp*x2;
	friendly_col = SDLColourToNativeColour(friendly_col);
	
	SDL_LockSurface(bmpDest);
	for (register uchar *px= (uchar *)bmpDest->pixels+bmpDest->pitch*y+bpp*x;px <= px2;px+=bpp)
		memcpy(px,&friendly_col,bpp);
	
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

	static Uint8 r,g,b;
	GetColour3(colour,SDL_GetVideoSurface(),&r,&g,&b);
	Uint32 friendly_col = SDL_MapRGB(bmpDest->format,r,g,b);

	register ushort pitch = (ushort)bmpDest->pitch;
	register byte bpp = (byte)bmpDest->format->BytesPerPixel;
	register uchar *px2 = (uchar *)bmpDest->pixels+pitch*y2+bpp*x;
	friendly_col = SDLColourToNativeColour(friendly_col);

	SDL_LockSurface(bmpDest);
	for (register uchar *px= (uchar *)bmpDest->pixels+pitch*y+bpp*x;px <= px2;px+=pitch)
		memcpy(px,&friendly_col,bpp);

	SDL_UnlockSurface(bmpDest);
}

// Line drawing
void DrawLine(SDL_Surface *dst, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint32 color) {
	// TODO: does this need more improvement/optimisation ?
	secure_perform_line(dst, x1, y1, x2, y2, color, PutPixel);
}

//////////////////////
// Draw antialiased line with an putpixel callback
// Code taken from CTGraphics by darkoman (http://www.codeproject.com/gdi/CTGraphics.asp)
void AntiAliasedLine(SDL_Surface * dst, int x1, int y1, int x2, int y2, int thickness, Uint32 color, void (*proc)(SDL_Surface *, int, int, Uint32, float))
{
	// Calculate line params
	int dx = (x2 - x1);
	int dy = (y2 - y1);
	int temp;
	float k;
	float distance;

	int half_thickness = thickness/2;

	// Set start pixel
	proc(dst, x1, y1, color, SDL_ALPHA_OPAQUE);

	// X-dominant line
	if (abs(dx) > abs(dy))
	{
		// Ex-change line end points
		if (dx < 0)
		{
			temp = x1;
			x1 = x2;
			x2 = temp;

			temp = y1;
			y1 = y2;
			y2 = temp;
		}
		k = (float)dy / (float)dx;
		k *= (float) thickness;

		// Set middle pixels
		int xs;
		float yt = (float)y1 + k;
		for (xs=x1+thickness; xs<x2; xs += thickness)
		{
			distance = (float)(yt - (int)(yt));

			proc(dst, xs, (int)yt, color, distance);
			proc(dst, xs, (int)yt+half_thickness, color, 1.0f-distance);

			yt += k;
		}
	}
	// Y-dominant line
	else
	{
		// Ex-change line end points
		if (dy < 0)
		{
			temp = x1;
			x1 = x2;
			x2 = temp;

			temp = y1;
			y1 = y2;
			y2 = temp;
		}
		k = (float)dx / (float)dy;
		k *= (float) thickness;

		// Set middle pixels
		int ys;
		float xt = (float)x1 + k;
		for (ys=y1+thickness; ys<y2; ys += thickness)
		{
			distance = (float)(xt - (int)(xt));

			proc(dst, (int)xt, ys, color, distance);
			proc(dst, (int)xt+half_thickness, ys, color, 1.0f-distance);			

			xt += k;
		}
	}

	// Set end pixel
	proc(dst, x2, y2, color, SDL_ALPHA_OPAQUE);
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


	if (tLXOptions->bAntiAliasing)
		AntiAliasedLine(bmp, x1, y1, x2, y2, 2, color, RopePutPixelA);
	else
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

	if (tLXOptions->bAntiAliasing)
		AntiAliasedLine(bmp, x1, y1, x2, y2, 2, color, BeamPutPixelA);
	else
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
