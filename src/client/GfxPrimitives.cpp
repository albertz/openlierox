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


#include <iostream>
#include <assert.h>
#include <gd.h>

#include "LieroX.h"
#include "MathLib.h"
#include "GfxPrimitives.h"
#include "CServer.h"
#include "Cache.h"
#include "FindFile.h"
#include "StringUtils.h"

int iSurfaceFormat = SDL_SWSURFACE;

using namespace std;

/////////////////////////
//
// Misc routines
//
//////////////////////////

/////////////////
// Put the pixel alpha blended with the background
void PutPixelA(SDL_Surface *bmpDest, int x, int y, Uint32 colour, float a)  {
	Uint8 R1, G1, B1, A1, R2, G2, B2; 	 
	Uint8* px = (Uint8*)bmpDest->pixels + y * bmpDest->pitch + x * bmpDest->format->BytesPerPixel; 	 
	SDL_GetRGBA(GetPixelFromAddr(px, bmpDest->format->BytesPerPixel), bmpDest->format, &R1, &G1, &B1, &A1); 	 
	SDL_GetRGB(colour, bmpDest->format, &R2, &G2, &B2); 	 
	PutPixelToAddr(px, SDL_MapRGBA(bmpDest->format, 	 
			(Uint8) CLAMP(R1 * (1.0f - a) + R2 * a, 0.0f, 255.0f), 	 
			(Uint8) CLAMP(G1 * (1.0f - a) + G2 * a, 0.0f, 255.0f), 	 
			(Uint8) CLAMP(B1 * (1.0f - a) + B2 * a, 0.0f, 255.0f), 	 
			A1), bmpDest->format->BytesPerPixel);
}


//////////////////////
// Set a color key for alpha surface (SDL_SetColorKey does not work for alpha surfaces)
void SetColorKeyAlpha(SDL_Surface* dst, Uint8 r, Uint8 g, Uint8 b) {
	// Just set transparent alpha to pixels that match the color key
	Uint8* pxr = (Uint8*)dst->pixels;
	Uint8* px;
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

	// Set the colorkey value so we can later check if it was set or not and save some CPU time
	dst->format->colorkey = colorkey;
}


///////////////////////
// Set a pink color key
void SetColorKey(SDL_Surface* dst)  {
	// If there's already a colorkey set, don't set it again
	if (dst->format->colorkey != 0)
		return;

	// Because some graphic editors (old Photoshop and GIMP and possibly more) contain a bug that rounds 255 to 254
	// and some mods have such bugged images. To fix it we have to make a little hack here :(
	Uint32 bugged_colorkey = SDL_MapRGB(dst->format, 254, 0, 254);
	
	// If one of the pixels in edges matches the bugged colorkey, we suppose whole image is bugged
	// Not the best method, but it is fast and works for all cases I have discovered
	bool bugged =	EqualRGB(GetPixel(dst, 0, 0), bugged_colorkey, dst->format) ||
					EqualRGB(GetPixel(dst, dst->w - 1, 0), bugged_colorkey, dst->format) ||
					EqualRGB(GetPixel(dst, 0, dst->h - 1), bugged_colorkey, dst->format) ||
					EqualRGB(GetPixel(dst, dst->w - 1, dst->h - 1), bugged_colorkey, dst->format);


	// Apply the colorkey
	if (dst->flags & SDL_SRCALPHA)
		if (bugged)
			SetColorKeyAlpha(dst, 254, 0, 254);
		else
			SetColorKeyAlpha(dst, 255, 0, 255);
	else
		if (bugged)
			SDL_SetColorKey(dst, SDL_SRCCOLORKEY, bugged_colorkey); 
		else
			SDL_SetColorKey(dst, SDL_SRCCOLORKEY, SDL_MapRGB(dst->format, 255, 0, 255)); 
}

/////////////////////////
//
// Clipping routines
//
/////////////////////////


//
// Line clipping - grabbed from SDL_gfx
//

#define CLIP_LEFT_EDGE   0x1
#define CLIP_RIGHT_EDGE  0x2
#define CLIP_BOTTOM_EDGE 0x4
#define CLIP_TOP_EDGE    0x8
#define CLIP_INSIDE(a)   (!a)
#define CLIP_REJECT(a,b) (a&b)
#define CLIP_ACCEPT(a,b) (!(a|b))

static inline int clipEncode(int x, int y, int left, int top, int right, int bottom)
{
    int code = 0;

    if (x < left) {
		code |= CLIP_LEFT_EDGE;
    } else if (x > right) {
		code |= CLIP_RIGHT_EDGE;
    }

    if (y < top) {
		code |= CLIP_TOP_EDGE;
    } else if (y > bottom) {
		code |= CLIP_BOTTOM_EDGE;
    }

    return code;
}

/////////////////////
// Clip the line to the surface
bool ClipLine(SDL_Surface * dst, int * x1, int * y1, int * x2, int * y2)
{
    int left, right, top, bottom;
    Uint32 code1, code2;
    bool draw = false;
    int swaptmp;
    float m;

	// No line
	if (*x1 == *x2 && *y1 == *y2)
		return false;

    // Get clipping boundary 
    left = dst->clip_rect.x;
    right = dst->clip_rect.x + dst->clip_rect.w - 1;
    top = dst->clip_rect.y;
    bottom = dst->clip_rect.y + dst->clip_rect.h - 1;

    while (true) {
		code1 = clipEncode(*x1, *y1, left, top, right, bottom);
		code2 = clipEncode(*x2, *y2, left, top, right, bottom);
		if (CLIP_ACCEPT(code1, code2)) {
			draw = true;
			break;
		} else if (CLIP_REJECT(code1, code2))
			break;
		else {
			if (CLIP_INSIDE(code1)) {
				swaptmp = *x2;
				*x2 = *x1;
				*x1 = swaptmp;
				swaptmp = *y2;
				*y2 = *y1;
				*y1 = swaptmp;
				swaptmp = code2;
				code2 = code1;
				code1 = swaptmp;
			}
			
			if (*x2 != *x1) {
				m = (*y2 - *y1) / (float) (*x2 - *x1);
			} else {
				m = 1.0f;
			}
			
			if (code1 & CLIP_LEFT_EDGE) {
				*y1 += (int) ((left - *x1) * m);
				*x1 = left;
			} else if (code1 & CLIP_RIGHT_EDGE) {
				*y1 += (int) ((right - *x1) * m);
				*x1 = right;
			} else if (code1 & CLIP_BOTTOM_EDGE) {
				if (*x2 != *x1) {
					*x1 += (int) ((bottom - *y1) / m);
				}
				*y1 = bottom;
			} else if (code1 & CLIP_TOP_EDGE) {
				if (*x2 != *x1) {
					*x1 += (int) ((top - *y1) / m);
				}
				*y1 = top;
			}
		}
    }

    return draw && (*x1 != *x2 || *y1 != *y2);
}


/////////////////////////
// Performs one side (horizontal or vertical) clip
// c - x or y; d - width or height
bool OneSideClip(int& c, int& d, const int clip_c, const int clip_d)  {
	if (c < clip_c)  {
		d += c - clip_c;
		c = clip_c;
		if (d <= 0)  {
			d = 0;
			return false;
		}
	}

	if (c + d >= clip_c + clip_d)  {
		if (c >= clip_c + clip_d)  {
			d = 0;
			return false;
		}

		d = clip_c + clip_d - c;
	}

	return true;
}


//////////////////
//
// Blitting and drawing routines
//
//////////////////


inline void CopySurfaceFast(SDL_Surface* dst, SDL_Surface* src, int sx, int sy, int dx, int dy, int w, int h) {
	// Initialize
	size_t byte_bound = w * src->format->BytesPerPixel;
	int src_pitch = src->pitch;
	int dst_pitch = dst->pitch;
	Uint8* srcrow = (Uint8 *)src->pixels
		+ (sy * src_pitch) + (sx * src->format->BytesPerPixel);
	Uint8* dstrow = (Uint8 *)dst->pixels
		+ (dy * dst_pitch) + (dx * dst->format->BytesPerPixel);
	
	// Copy row by row
	for (register int i = 0; i < h; ++i)  {
		memcpy(dstrow, srcrow, byte_bound);
		dstrow += dst_pitch;
		srcrow += src_pitch;
	}
}


///////////////////////
// Copies area from one image to another (not blitting so the alpha values are kept!)
void CopySurface(SDL_Surface* dst, SDL_Surface* src, int sx, int sy, int dx, int dy, int w, int h)
{	
	// Copying is a normal blit without colorkey and alpha
	// If the surface has alpha or colorkey set, we have to remove them and then put them back

	// Save alpha values
	bool HasAlpha = false;
	Uint8 PerSurfaceAlpha = SDL_ALPHA_OPAQUE;
	if (src->flags & SDL_SRCALPHA)  {
		HasAlpha = true;
		PerSurfaceAlpha = src->format->alpha;
	}

	// Save colorkey values
	bool HasColorkey = false;
	Uint32 Colorkey = 0;
	if (src->flags & SDL_SRCCOLORKEY)  {
		HasColorkey = true;
		Colorkey = src->format->colorkey;
	}

	// Remove alpha and colorkey
	SDL_SetAlpha(src, 0, 0);
	SDL_SetColorKey(src, 0, 0);

	// Blit
	DrawImageAdv(dst, src, sx, sy, dx, dy, w, h);

	// Return back alpha and colorkey
	if (HasAlpha)
		SDL_SetAlpha(src, SDL_SRCALPHA, PerSurfaceAlpha);
	if (HasColorkey)
		SDL_SetColorKey(src, SDL_SRCCOLORKEY, Colorkey);
}


///////////////////
// Draw the image mirrored with a huge amount of options
void DrawImageAdv_Mirror(SDL_Surface *bmpDest, SDL_Surface *bmpSrc, int sx, int sy, int dx, int dy, int w, int h)
{
	// Warning: Both surfaces have to have same bpp!
	assert(bmpDest->format->BytesPerPixel == bmpSrc->format->BytesPerPixel);

	// Warning: Doesn't do clipping on the source surface

	// Clipping on dest surface
	if (!ClipRefRectWith(dx, dy, w, h, (SDLRect&)bmpDest->clip_rect))
		return;

	int x,y;

	// Lock the surfaces
	if(SDL_MUSTLOCK(bmpDest))
		SDL_LockSurface(bmpDest);
	if(SDL_MUSTLOCK(bmpSrc))
		SDL_LockSurface(bmpSrc);


	Uint8 *TrgPix = (Uint8 *)bmpDest->pixels + dy*bmpDest->pitch + dx*bmpDest->format->BytesPerPixel;
	Uint8 *SrcPix = (Uint8 *)bmpSrc->pixels +  sy*bmpSrc->pitch + sx*bmpSrc->format->BytesPerPixel;

	short bpp = bmpDest->format->BytesPerPixel;

	register Uint8 *sp,*tp;
	for(y = h; y; --y) {

		sp = SrcPix;
		tp = TrgPix + w*bpp;
		for(x = w; x; --x) {
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
	// TODO: recode this; avoid this amount of variables, only use ~5 local variables in a function!
	
	assert(bmpDest->format->BytesPerPixel == bmpSrc->format->BytesPerPixel);
	
	int x,y;
	int dw = w * 2;
	int dh = h * 2;

	// Lock the surfaces
	if(SDL_MUSTLOCK(bmpDest))
		SDL_LockSurface(bmpDest);
	if(SDL_MUSTLOCK(bmpSrc))
		SDL_LockSurface(bmpSrc);

	// Source clipping
	if (!ClipRefRectWith(sx, sy, w, h, (SDLRect&)bmpSrc->clip_rect))
		return;

	// Dest clipping
	if (!ClipRefRectWith(dx, dy, w, h, (SDLRect&)bmpDest->clip_rect))
		return;

	w = MIN(w, dw/2);
	h = MIN(h, dh/2);

	Uint8 *TrgPix = (Uint8 *)bmpDest->pixels + dy*bmpDest->pitch + dx*bmpDest->format->BytesPerPixel;
	Uint8 *SrcPix = (Uint8 *)bmpSrc->pixels +  sy*bmpSrc->pitch + sx*bmpSrc->format->BytesPerPixel;

	register Uint8 *sp,*tp_x,*tp_y;
	int doublepitch = bmpDest->pitch*2;
	byte bpp = bmpDest->format->BytesPerPixel;

    for(y = h; y; --y) {

		sp = SrcPix;
		tp_x = TrgPix;
		tp_y = tp_x+bmpDest->pitch;
		for(x = w; x; --x) {
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
// HINT: doesn't work with alpha-surfaces
void DrawImageStretch2Key(SDL_Surface *bmpDest, SDL_Surface *bmpSrc, int sx, int sy, int dx, int dy, int w, int h)
{
	// TODO: recode this; avoid this amount of variables, only use ~5 local variables in a function!
	
	assert(bmpDest->format->BytesPerPixel == bmpSrc->format->BytesPerPixel);

	int x,y;

	int dw = w * 2;
	int dh = h * 2;

	// Source clipping
	if (!ClipRefRectWith(sx, sy, w, h, (SDLRect&)bmpSrc->clip_rect))
		return;

	// Dest clipping
	if (!ClipRefRectWith(dx, dy, dw, dh, (SDLRect&)bmpDest->clip_rect))
		return;


	w = MIN(w, dw/2);
	h = MIN(h, dh/2);


	// Lock the surfaces
	if(SDL_MUSTLOCK(bmpDest))
		SDL_LockSurface(bmpDest);
	if(SDL_MUSTLOCK(bmpSrc))
		SDL_LockSurface(bmpSrc);

	Uint8 *TrgPix = (Uint8 *)bmpDest->pixels + dy*bmpDest->pitch + dx*bmpDest->format->BytesPerPixel;
	Uint8 *SrcPix = (Uint8 *)bmpSrc->pixels +  sy*bmpSrc->pitch + sx*bmpSrc->format->BytesPerPixel;

	register Uint8 *sp, *tp_x, *tp_y;

	// Pre-calculate some things, so the loop is faster
	int doublepitch = bmpDest->pitch * 2;
	byte bpp = bmpDest->format->BytesPerPixel;
	byte doublebpp = (byte)(bpp * 2);

    for(y=h; y ; --y) {

		sp = SrcPix;
		tp_x = TrgPix;
		tp_y = tp_x+bmpDest->pitch;
		for(x = w; x; --x) {
			if (IsTransparent(bmpSrc, GetPixelFromAddr(sp, bpp)))  {
				// Skip the transparent pixel
				sp += bpp;
				tp_x += doublebpp;
				tp_y += doublebpp;
			} else {
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
void DrawImageStretchMirrorKey(SDL_Surface *bmpDest, SDL_Surface *bmpSrc, int sx, int sy, int dx, int dy, int w, int h)
{
	// TODO: recode this; avoid this amount of variables, only use ~5 local variables in a function!
	
	assert(bmpDest->format->BytesPerPixel == bmpSrc->format->BytesPerPixel);
	
	int x,y;

	int dw = w * 2;
	int dh = h * 2;

	// Warning: Doesn't do clipping on the source surface

	// Clipping on dest surface
	if (!ClipRefRectWith(dx, dy, w, h, (SDLRect&)bmpDest->clip_rect))
		return;

	// Clipping could change w or h
	w = dw / 2;
	h = dh / 2;


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
	byte bpp = bmpDest->format->BytesPerPixel;
	byte doublebpp = (byte)(bpp*2);
	int realw = w*bpp;

    for(y = h; y; --y) {

		sp = SrcPix;
		tp_x = TrgPix+realw;
		tp_y = tp_x+bmpDest->pitch;
		for(x = w; x; --x) {
			if (!IsTransparent(bmpSrc, GetPixelFromAddr(sp, bpp)))  {
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
void DrawImageResizedAdv( SDL_Surface *bmpDest, SDL_Surface *bmpSrc, float sx, float sy, int dx, int dy, int sw, int sh, float xratio, float yratio)
{
	// TODO: recode this; avoid this amount of variables, only use ~5 local variables in a function!
	
	int dw = Round((float)sw * xratio);
	int dh = Round((float)sh * yratio);

	// Source clipping
	float clip_sw = (float)sw;
	float clip_sh = (float)sh;
	if (!ClipRefRectWith(sx, sy, clip_sw, clip_sh, (SDLRect&)bmpSrc->clip_rect))
		return;

	// Dest clipping
	if (!ClipRefRectWith(dx, dy, dw, dh, (SDLRect&)bmpDest->clip_rect))
		return;

	// Update the widths/heights according to clipping
	sw = (int)clip_sw;
	sh = (int)clip_sh;
	dw = MIN(dw, Round(clip_sw * xratio));
	dh = MIN(dh, Round(clip_sh * yratio));
	

	float xstep = (float)sw/(float)dw; // X step we'll do on the source surface
	float ystep = (float)sh/(float)dh; // Y step we'll do on the source surface
	float src_xstart = (int)((float)sx/xstep) * xstep; // Starting X coordinate on source surface
	float src_ystart = (int)((float)sy/ystep) * ystep; // Starting Y coordinate on source surface
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
	byte bpp = (byte)bmpDest->format->BytesPerPixel;

	for (src_y = src_ystart; dest_y < dest_y2; dest_y++)  {
		src_pxrow = (Uint8 *)bmpSrc->pixels + (int)(src_y) * bmpSrc->pitch;
		dst_px = dst_pxrow;

		// Copy the row
		for (dest_x = dx, src_x = src_xstart; dest_x < dest_x2; dest_x++)  {
			src_px = src_pxrow + (int)(src_x) * bpp;
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
void DrawImageResampledAdv( SDL_Surface *bmpDest, SDL_Surface *bmpSrc, float sx, float sy, int dx, int dy, int sw, int sh, float xratio, float yratio, float blur)
{
	// TODO: recode this; avoid this amount of variables, only use ~5 local variables in a function!

	// How this works:
	// We take four neighbour pixels from the source and make average of them

	// For 1px width/height no one will notice the difference and it will 
	// avoid many checks here
	if (sw < 2.0f || sh < 2.0f)  {
		DrawImageResizedAdv(bmpDest, bmpSrc, sx, sy, dx, dy, sw, sh, xratio, yratio);
		return;
	}

	int dw = Round((float)sw * xratio);
	int dh = Round((float)sh * yratio);

	// Source clipping
	float clip_sw = (float)sw;
	float clip_sh = (float)sh;
	if (!ClipRefRectWith(sx, sy, clip_sw, clip_sh, (SDLRect&)bmpSrc->clip_rect))
		return;

	// Dest clipping
	if (!ClipRefRectWith(dx, dy, dw, dh, (SDLRect&)bmpDest->clip_rect))
		return;

	// Update the widths/heights according to clipping
	sw = (int)clip_sw;
	sh = (int)clip_sh;
	dw = MIN(dw, Round(clip_sw * xratio));
	dh = MIN(dh, Round(clip_sh * yratio));



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
	Uint8 *src_px = NULL;
	Uint8 *src_pxrow = NULL;
	Uint8 *dst_px = NULL;
	Uint8 *dst_pxrow = (Uint8 *)bmpDest->pixels + (dy * bmpDest->pitch) + (dx * bmpDest->format->BytesPerPixel);
	byte bpp = (byte)bmpDest->format->BytesPerPixel;

	static Uint32 leftpixel, toppixel, rightpixel, bottompixel, curpixel;
	static Uint8 rleft, rtop, rright, rbottom, rcurrent, gleft, gtop, gright, gbottom, gcurrent, bleft, btop, bright, bbottom, bcurrent;
	static Uint32 avg_r, avg_g, avg_b;
	byte first_row, first_col; // 0 when first row/column, 1 when not
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

/////////////////
//
// Special line and pixel drawing
//
/////////////////


int ropecolour = 0;
int ropealt = 0;

///////////////////
// Put a pixel on the surface (while checking for clipping)
void RopePutPixelA(SDL_Surface *bmpDest, int x, int y, Uint32 colour, Uint8 alpha)
{
	// Warning: lock the surface before calling this!
	// Warning: passing NULL surface will cause a segfault

	ropealt++;
	ropealt %= 3;

	if (ropealt == 2)
		ropecolour = !ropecolour;
	colour = tLX->clRopeColors[ropecolour];

	// No alpha weight, use direct pixel access (faster)
	if (alpha == 255)  {
		colour = SDLColourToNativeColour(colour, bmpDest->format->BytesPerPixel);
		Uint8 *px = (Uint8 *)bmpDest->pixels + bmpDest->pitch*y + x*bmpDest->format->BytesPerPixel;
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

// For compatibility with perform_line
// NOTE: it's slightly different, to keep the rope looking the same
void RopePutPixel(SDL_Surface *bmpDest, int x, int y, Uint32 colour) {
	ropealt = !ropealt;

	if (ropealt)
		ropecolour = !ropecolour;
	colour = tLX->clRopeColors[ropecolour];

	// Put the pixel
	colour = SDLColourToNativeColour(colour, bmpDest->format->BytesPerPixel);
	Uint8 *px = (Uint8 *)bmpDest->pixels + bmpDest->pitch*y + x*bmpDest->format->BytesPerPixel;
	Uint8 *px2 = px+bmpDest->pitch;
	memcpy(px,&colour,bmpDest->format->BytesPerPixel);
	memcpy(px+bmpDest->format->BytesPerPixel,&colour,bmpDest->format->BytesPerPixel);
	memcpy(px2,&colour,bmpDest->format->BytesPerPixel);
	memcpy(px2+bmpDest->format->BytesPerPixel,&colour,bmpDest->format->BytesPerPixel);
}


///////////////////
// Put a pixel on the surface (while checking for clipping)
void BeamPutPixelA(SDL_Surface *bmpDest, int x, int y, Uint32 colour, Uint8 alpha)
{
	// No alpha, use direct pixel access
	if (alpha == 255)  {
		colour = SDLColourToNativeColour(colour, bmpDest->format->BytesPerPixel);
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
 
void BeamPutPixel(SDL_Surface *bmpDest, int x, int y, Uint32 colour) { // For compatibility with perform_line
	BeamPutPixelA(bmpDest, x, y, colour, 255); }


int laseralt = 0;

///////////////////
// Put a laser-sight pixel on the surface
void LaserSightPutPixel(SDL_Surface *bmpDest, int x, int y, Uint32 colour)
{
	laseralt++;
	laseralt %= GetRandomInt(35)+1;

	if(laseralt)
		return;

	colour = tLX->clLaserSightColors[ GetRandomInt(1) ];
	colour = SDLColourToNativeColour(colour, bmpDest->format->BytesPerPixel);

	// Snap to nearest 2nd pixel
	x -= x % 2;
	y -= y % 2;

	// Put the pixel (laser sight is never antialiased)
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
	if (!ClipLine(bmpDest, &x1, &y1, &x2, &y2)) // Clipping
		return;

	perform_line(bmpDest, x1, y1, x2, y2, color, proc);
}

// Draw horizontal line
void DrawHLine(SDL_Surface *bmpDest, int x, int x2, int y, Uint32 colour) {

	if (x < 0) x = 0;
	else if (x >= bmpDest->w) x=bmpDest->w-1;

	if (x2 < 0) x2 = 0;
	else if (x2 >= bmpDest->w) x2=bmpDest->w-1;

	if (y < 0) y = 0;
	else if (y >= bmpDest->h) y=bmpDest->h-1;

	if (x2 < x)  {
		int tmp;
		tmp = x;
		x = x2;
		x2 = tmp;
	}

	Uint8 r,g,b;
	GetColour3(colour,getMainPixelFormat(),&r,&g,&b);
	Uint32 friendly_col = SDLColourToNativeColour(
		SDL_MapRGB(bmpDest->format, r, g, b), bmpDest->format->BytesPerPixel);

	byte bpp = (byte)bmpDest->format->BytesPerPixel;
	uchar *px2 = (uchar *)bmpDest->pixels+bmpDest->pitch*y+bpp*x2;
	
	SDL_LockSurface(bmpDest);
	for (uchar* px = (uchar*)bmpDest->pixels + bmpDest->pitch * y + bpp * x; px <= px2; px += bpp)
		memcpy(px, &friendly_col, bpp);
	
	SDL_UnlockSurface(bmpDest);

}

// Draw vertical line
void DrawVLine(SDL_Surface *bmpDest, int y, int y2, int x, Uint32 colour) {
	if (x < 0) x = 0;
	else if (x >= bmpDest->w) x=bmpDest->w-1;

	if (y < 0) y = 0;
	else if (y >= bmpDest->h) y=bmpDest->h-1;

	if (y2 < 0) y2 = 0;
	else if (y2 >= bmpDest->h) y2=bmpDest->h-1;

	if (y2 < y)  {
		int tmp;
		tmp = y;
		y = y2;
		y2 = tmp;
	}

	Uint8 r,g,b;
	GetColour3(colour, getMainPixelFormat(), &r, &g, &b);
	Uint32 friendly_col = SDLColourToNativeColour(
		SDL_MapRGB(bmpDest->format, r, g, b), bmpDest->format->BytesPerPixel);

	ushort pitch = (ushort)bmpDest->pitch;
	byte bpp = (byte)bmpDest->format->BytesPerPixel;
	uchar *px2 = (uchar *)bmpDest->pixels+pitch*y2+bpp*x;

	SDL_LockSurface(bmpDest);
	for (uchar *px= (uchar *)bmpDest->pixels+pitch*y + bpp*x; px <= px2; px+=pitch)
		memcpy(px, &friendly_col, bpp);

	SDL_UnlockSurface(bmpDest);
}

// Line drawing
void DrawLine(SDL_Surface *dst, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint32 color) {
	secure_perform_line(dst, x1, y1, x2, y2, color, PutPixel);
}

//////////////////////
// Draw antialiased line with an putpixel callback
// Code basis taken from CTGraphics by darkoman (http://www.codeproject.com/gdi/CTGraphics.asp)
void AntiAliasedLine(SDL_Surface * dst, int x1, int y1, int x2, int y2, Uint32 color, void (*proc)(SDL_Surface *, int, int, Uint32, Uint8))
{
	// Calculate line params
	int dx = (x2 - x1);
	int dy = (y2 - y1);
	int k;
	int distance;

	// Set start pixel
	proc(dst, x1, y1, color, 255);

	// X-dominant line
	if (abs(dx) > abs(dy))
	{
		// Ex-change line end points
		if (dx < 0)
		{
			int temp;

			temp = x1;
			x1 = x2;
			x2 = temp;

			temp = y1;
			y1 = y2;
			y2 = temp;
		}
		k = (dy * 255) / dx;

		// Set middle pixels
		int xs;
		int yt = y1 * 256 + k;
		distance = 0;
		for (xs = x1 + 1; xs < x2; ++xs)
		{
			proc(dst, xs, yt / 256, color, (Uint8) (255 - distance));
			proc(dst, xs, yt / 256 + 1, color, (Uint8) distance);

			yt += k;
			distance = yt & 255; // Same as: yt % 256
		}
	}
	// Y-dominant line
	else
	{
		// Ex-change line end points
		if (dy < 0)
		{
			int temp;

			temp = x1;
			x1 = x2;
			x2 = temp;

			temp = y1;
			y1 = y2;
			y2 = temp;
		}
		k = (dx * 255) / dy;

		// Set middle pixels
		int ys;
		int xt = x1 * 256 + k;
		distance = 0;
		for (ys=y1+1; ys<y2; ++ys)
		{
			proc(dst, xt / 256, ys, color, (Uint8) (255 - distance));
			proc(dst, xt / 256 + 1, ys, color, (Uint8) distance);			

			xt += k;
			distance = xt & 255;  // Same as: xt % 256
		}
	}
}


///////////////////
// Draws a rope line
void DrawRope(SDL_Surface *bmp, int x1, int y1, int x2, int y2, Uint32 color)
{
	ropealt = 0;
	ropecolour = 0;

	// Clipping
	if (!ClipLine(bmp, &x1, &y1, &x2, &y2))
		return;

	if (tLXOptions->bAntiAliasing)
		AntiAliasedLine(bmp, x1, y1, x2, y2, color, RopePutPixelA);
	else
		perform_line(bmp, x1, y1, x2, y2, color, RopePutPixel);
}


///////////////////
// Draws a beam
void DrawBeam(SDL_Surface *bmp, int x1, int y1, int x2, int y2, Uint32 color)
{
	// Clipping
	if (!ClipLine(bmp, &x1, &y1, &x2, &y2))
		return;

	if (tLXOptions->bAntiAliasing)
		AntiAliasedLine(bmp, x1, y1, x2, y2, color, BeamPutPixelA);
	else
		perform_line(bmp, x1, y1, x2, y2, color, BeamPutPixel);
}


///////////////////
// Draws a laser sight
void DrawLaserSight(SDL_Surface *bmp, int x1, int y1, int x2, int y2, Uint32 color)
{
	// Clipping
	if (!ClipLine(bmp, &x1, &y1, &x2, &y2))
		return;

	perform_line(bmp, x1, y1, x2, y2, color, LaserSightPutPixel);
}


////////////////////////
//
//  Image loading/saving routines
//
////////////////////////

///////////////////
// Loads an image, and converts it to the same colour depth as the screen (speed)
SDL_Surface *LoadImage(const std::string& _filename, bool withalpha)
{
	// Try cache first
	SDL_Surface* Image = cCache.GetImage(_filename);
	if (Image)
		return Image;

	// Load the image
	std::string fullfname = GetFullFileName(_filename);
	if(fullfname.size() == 0)
		return NULL;

	SDL_Surface* img = IMG_Load(fullfname.c_str());

	if(!img)
		return NULL;

	if(SDL_GetVideoSurface()) {
		// Convert the image to the screen's colour depth
		SDL_PixelFormat fmt = *(getMainPixelFormat());
		if (withalpha)  {
			Image = gfxCreateSurfaceAlpha(img->w, img->h);
			CopySurface(Image, img, 0, 0, 0, 0, img->w, img->h);
		} else {
			img->flags &= ~SDL_SRCALPHA; // Remove the alpha flag here, ConvertSurface will remove the alpha completely later
			Image = SDL_ConvertSurface(img, &fmt, iSurfaceFormat);
		}
	
		SDL_FreeSurface(img);
	} else {
		// we haven't initialized the screen yet
		if(!bDedicated)
			printf("WARNING: screen not initialized yet while loading image\n");
		Image = img;
	}
	
	if(!Image) {
		printf("ERROR: LoadImgBPP: cannot create new surface\n");
		return NULL;
	}

	// Save to cache
	cCache.SaveImage(_filename, Image);
	
	return Image;
}

#ifndef DEDICATED_ONLY 
///////////////////////
// Converts the SDL_surface to gdImagePtr
static gdImagePtr SDLSurface2GDImage(SDL_Surface* src) {
	gdImagePtr gd_image = gdImageCreateTrueColor(src->w,src->h);
	if(!gd_image)
		return NULL;

	Uint32 rmask, gmask, bmask;
	// format of gdImage
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
#endif

///////////////////////
// Saves the surface into the specified file with the specified format
bool SaveSurface(SDL_Surface *image, const std::string& FileName, int Format, const std::string& Data)
{
	//
	// BMP
	//

	// We use standard SDL function for saving BMPs
	if (Format == FMT_BMP)  {

		// Save the image
		std::string abs_fn = GetWriteFullFileName (FileName, true);  // SDL requires full paths
		SDL_SaveBMP(image, abs_fn.c_str());

		// Append any additional data
		if (!Data.empty())  {

			FILE *f = OpenGameFile (FileName, "ab");
			if (!f)
				return false;

			fwrite(Data.data(), 1, Data.size(), f);
			fclose (f);
		}

		return true;
	}

	#ifdef DEDICATED_ONLY
	printf("WARNING: SaveSurface: cannot use something else than BMP in dedicated-only-mode\n");
	return false;
	#else
	
	//
	// JPG, PNG, GIF
	//

	// We use GD for saving these formats
	gdImagePtr gd_image = NULL;

	// Convert the surface
	gd_image = SDLSurface2GDImage ( image );
	if ( !gd_image )
		return false;

	// Save the image
	int s;
	char *data = NULL;
	FILE *out = OpenGameFile (FileName, "wb");
	if ( !out )
		return false;

	// Get the data depending on the format
	switch (Format)
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

	// Check
	if (!data)
		return false;

	// Size of the data
	size_t size = s > 0 ? s : -s;

	// Write the image data
	if (fwrite(data, 1, size, out) != size)
		return false;

	// Write any additional data
	if (!Data.empty())
		fwrite(Data.data(), 1, Data.size(), out);

	// Free everything
	gdFree ( data );
	gdImageDestroy ( gd_image );

	// Close the file and quit
	return fclose(out) == 0;
	#endif
}


void test_Clipper() {
	SDL_Rect r1 = {52, 120, 7, 14};
	SDL_Rect r2 = {52, 162, 558, 258};
	
	ClipRefRectWith(r1, (SDLRect&)r2);
	
	cout << r1.x << "," << r1.y << "," << r1.w << "," << r1.h << endl;
}
