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



#include <cassert>
#ifndef DEDICATED_ONLY
#include <gd.h>
#endif
#include <SDL.h>

#include "LieroX.h"
#include "Options.h"
#include "Debug.h"
#include "MathLib.h"
#include "GfxPrimitives.h"
#include "Cache.h"
#include "FindFile.h"
#include "StringUtils.h"
#include "AuxLib.h"
#include "PixelFunctors.h"
#include "Utils.h"
#include "CViewport.h"
#include "Geometry.h"
#include "Timer.h"

int iSurfaceFormat = SDL_SWSURFACE;



// Used in various alpha-blending routines, internal
// TODO: remove this and use Color struct
struct RGBA  {
	Uint8 r, g, b, a;
};

/////////////////////////
//
// Misc routines
//
//////////////////////////

/////////////////
// Put the pixel alpha blended with the background
// TODO: this function is now obsolete, remove it
// TODO: why is this function obsolete? what should be used instead?
void PutPixelA(SDL_Surface * bmpDest, int x, int y, Uint32 colour, Uint8 a)  {
	Uint8* px = (Uint8*)bmpDest->pixels + y * bmpDest->pitch + x * bmpDest->format->BytesPerPixel;
	
	PixelPutAlpha& putter = getPixelAlphaPutFunc(bmpDest);
	Color c = Unpack_solid(colour, bmpDest->format); c.a = a;
	putter.put(px, bmpDest->format, c);
}


//////////////////////////
// Set the per-surface alpha
void SetPerSurfaceAlpha(SDL_Surface * dst, Uint8 a) {
	SDL_SetAlpha(dst, SDL_SRCALPHA, a);
}


//////////////////////
// Set a color key for alpha surface (SDL_SetColorKey does not work for alpha surfaces)
static void SetColorKey_Alpha(SDL_Surface * dst, Uint8 r, Uint8 g, Uint8 b) {
	LOCK_OR_QUIT(dst);

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

	UnlockSurface(dst);
}


///////////////////////
// Set a pink color key
void SetColorKey(SDL_Surface * dst)  {
	// If there's already a colorkey set, don't set it again
	if ((dst->flags & SDL_SRCCOLORKEY) &&
		((dst->format->colorkey == SDL_MapRGB(dst->format, 255, 0, 255)) ||
		(dst->format->colorkey == SDL_MapRGB(dst->format, 254, 0, 254))))
		return;

	// Because some graphic editors (old Photoshop and GIMP and possibly more) contain a bug that rounds 255 to 254
	// and some mods have such bugged images. To fix it we have to make a little hack here :(
	Uint32 bugged_colorkey = SDL_MapRGB(dst->format, 254, 0, 254);

	// If one of the pixels in edges matches the bugged colorkey, we suppose whole image is bugged
	// Not the best method, but it is fast and works for all cases I have discovered
	LOCK_OR_QUIT(dst);
	bool bugged =	EqualRGB(GetPixel(dst, 0, 0), bugged_colorkey, dst->format) ||
					EqualRGB(GetPixel(dst, dst->w - 1, 0), bugged_colorkey, dst->format) ||
					EqualRGB(GetPixel(dst, 0, dst->h - 1), bugged_colorkey, dst->format) ||
					EqualRGB(GetPixel(dst, dst->w - 1, dst->h - 1), bugged_colorkey, dst->format);
	UnlockSurface(dst);


	// Apply the colorkey
	if (dst->flags & SDL_SRCALPHA) {
		// see comment in other SetColorKey; the behaviour with SDL_SRCALPHA is different
		if (bugged)
			SetColorKey_Alpha(dst, 254, 0, 254);
		else
			SetColorKey_Alpha(dst, 255, 0, 255);
	}

	// set in both cases the colorkey (for alpha-surfaces just as a info, it's ignored there)
	if (bugged)
		SDL_SetColorKey(dst, SDL_SRCCOLORKEY, bugged_colorkey);
	else
		SDL_SetColorKey(dst, SDL_SRCCOLORKEY, SDL_MapRGB(dst->format, 255, 0, 255));
}

void SetColorKey(SDL_Surface * dst, Uint8 r, Uint8 g, Uint8 b) {
	if(r == 255 && g == 0 && b == 255) { // pink
		SetColorKey(dst); // use this function as it has a workaround included for some old broken mods
		return;
	}

	if (dst->flags & SDL_SRCALPHA)
		// HINT: The behaviour with SDL_SRCALPHA is different
		// and not as you would expect it. The problem is that while blitting to a surface with RGBA,
		// the alpha channel of the destination is untouched (see manpage to SDL_SetAlpha).
		SetColorKey_Alpha(dst, r, g, b);

	// set in both cases the colorkey (for alpha-surfaces just as a info, it's ignored there)
	SDL_SetColorKey(dst, SDL_SRCCOLORKEY, SDL_MapRGB(dst->format, r, g, b));
}

//////////////////////////////
// Create a surface without any alpha channel
SmartPointer<SDL_Surface> gfxCreateSurface(int width, int height, bool forceSoftware)
{
	if (width <= 0 || height <= 0) // Nonsense, can cause trouble
		return NULL;

	SDL_PixelFormat* fmt = getMainPixelFormat();

	SmartPointer<SDL_Surface> result = SDL_CreateRGBSurface(
			forceSoftware ? SDL_SWSURFACE : iSurfaceFormat,
			width, height,
			fmt->BitsPerPixel, fmt->Rmask, fmt->Gmask, fmt->Bmask, fmt->Amask);

	if (result.get() != NULL)  {
		// OpenGL strictly requires the surface to be cleared
		SDL_FillRect(result.get(), NULL, SDL_MapRGBA(result.get()->format, 0, 0, 0, 255));
		SDL_SetAlpha(result.get(), 0, 0); // Remove any alpha
	}

	#ifdef DEBUG
	//printf("gfxCreateSurface() %p %i %i\n", result.get(), width, height );
	#endif

	return result;
}

/////////////////////////
// Create a surface with an alpha channel
SmartPointer<SDL_Surface> gfxCreateSurfaceAlpha(int width, int height, bool forceSoftware)
{
	if (width <= 0 || height <= 0) // Nonsense, can cause trouble
		return NULL;

	SmartPointer<SDL_Surface> result;
	SDL_PixelFormat* fmt = getMainPixelFormat();

	// HINT: in 32bit mode with software surfaces, we have to use the predefined masks because they are hardcoded in SDL
	// (else the blitting is wrong)
	// it seems that for other BPP this is not the case
	if(!forceSoftware && (iSurfaceFormat == SDL_HWSURFACE || fmt->BitsPerPixel != 32) && fmt->Amask != 0) // the main pixel format supports alpha blending
		result = SDL_CreateRGBSurface(
				iSurfaceFormat | SDL_SRCALPHA,
				width, height,
				fmt->BitsPerPixel, fmt->Rmask, fmt->Gmask, fmt->Bmask, fmt->Amask);

	else // no native alpha blending or forced software, so create a software alpha blended surface
		result = SDL_CreateRGBSurface(
				SDL_SWSURFACE | SDL_SRCALPHA,
				width, height, 32,
				ALPHASURFACE_RMASK, ALPHASURFACE_GMASK, ALPHASURFACE_BMASK, ALPHASURFACE_AMASK);

	if (result.get() != NULL)
		// OpenGL strictly requires the surface to be cleared
		SDL_FillRect( result.get(), NULL, SDL_MapRGB(result.get()->format, 0, 0, 0));

	#ifdef DEBUG
	//printf("gfxCreateSurfaceAlpha() %p %i %i\n", result.get(), width, height );
	#endif

	return result;
}

//////////////////
// Resets the alpha-channel and the colorkey
void ResetAlpha(SDL_Surface * dst)
{
	SDL_SetColorKey(dst, 0, 0); // Remove the colorkey
	SDL_SetAlpha(dst, 0, 0); // Remove the persurface-alpha

	PixelPut& putter = getPixelPutFunc(dst);
	PixelGet& getter = getPixelGetFunc(dst);

	LOCK_OR_QUIT(dst);
	Uint8 *px = (Uint8 *)dst->pixels;
	int gap = dst->pitch - dst->w * dst->format->BytesPerPixel;

	int x, y;
	for(y = dst->h; y; --y, px += gap)
		for(x = dst->w; x; --x, px += dst->format->BytesPerPixel)
			putter.put(px, getter.get(px) | dst->format->Amask);

	UnlockSurface(dst);
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



//////////////////
//
// Blitting and drawing routines
//
//////////////////


// TODO: not used, remove?
inline void CopySurfaceFast(SDL_Surface * dst, SDL_Surface * src, int sx, int sy, int dx, int dy, int w, int h) {
	LOCK_OR_QUIT(dst);
	LOCK_OR_QUIT(src);

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

	UnlockSurface(src);
	UnlockSurface(dst);
}


///////////////////////
// Copies area from one image to another (not blitting so the alpha values are kept!)
void CopySurface(SDL_Surface * dst, SDL_Surface * src, int sx, int sy, int dx, int dy, int w, int h)
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

/////////////////////
// Performs a "correct" blit of RGBA surfaces to RGB or RGBA surfaces
static void DrawRGBA(SDL_Surface * bmpDest, SDL_Surface * bmpSrc, SDL_Rect& rDest, SDL_Rect& rSrc)
{
	// RGBA surfaces are only 32bit
	assert(bmpSrc->format->BytesPerPixel == 4);

	// Clip
	{
		int old_x = rDest.x;
		int old_y = rDest.y;
		rDest.w = rSrc.w;
		rDest.h = rSrc.h;
		if (!ClipRefRectWith(rDest, (SDLRect&)bmpDest->clip_rect))
			return;

		rSrc.x += rDest.x - old_x;
		rSrc.y += rDest.y - old_y;
		if (!ClipRefRectWith(rSrc, (SDLRect&)bmpSrc->clip_rect))
			return;
		rDest.w = MIN(rSrc.w, rDest.w);
		rDest.h = MIN(rSrc.h, rDest.h);
	}

	LOCK_OR_QUIT(bmpDest);
	LOCK_OR_QUIT(bmpSrc);

	static const int sbpp = 4;
	const int dbpp = bmpDest->format->BytesPerPixel;
	Uint8 *src = ((Uint8 *)bmpSrc->pixels + rSrc.y * bmpSrc->pitch + rSrc.x * sbpp);
	Uint8 *dst = ((Uint8 *)bmpDest->pixels + rDest.y * bmpDest->pitch + rDest.x * dbpp);
	int srcgap = bmpSrc->pitch - rDest.w * sbpp;
	int dstgap = bmpDest->pitch - rDest.w * dbpp;

	PixelPutAlpha& putter = getPixelAlphaPutFunc(bmpDest);

	for (int y = rDest.h; y; --y, dst += dstgap, src += srcgap)
		for (int x = rDest.w; x; --x, dst += dbpp, src += sbpp)  {
			Color c = Unpack_alpha(GetPixel_32(src), bmpSrc->format);
			c.a = (c.a * bmpSrc->format->alpha) / 255;  // Add the per-surface alpha to the source pixel alpha
			putter.put(dst, bmpDest->format, c);
		}

	UnlockSurface(bmpDest);
	UnlockSurface(bmpSrc);
}

/////////////////////
// Draws the image
void DrawImageAdv(SDL_Surface * bmpDest, SDL_Surface * bmpSrc, SDL_Rect& rDest, SDL_Rect& rSrc)
{
	bool src_isrgba = bmpSrc->format->Amask != 0 && (bmpSrc->flags & SDL_SRCALPHA);
	bool dst_isrgba = bmpDest->format->Amask != 0 && (bmpDest->flags & SDL_SRCALPHA);

	// RGB -> RGB
	// RGB -> RGBA
	if (!src_isrgba)  {
		SDL_BlitSurface(bmpSrc, &rSrc, bmpDest, &rDest);

	// RGBA -> RGB
	} else if (src_isrgba && !dst_isrgba)  {
		switch (bmpSrc->format->alpha)  {
		case SDL_ALPHA_OPAQUE:
			SDL_BlitSurface(bmpSrc, &rSrc, bmpDest, &rDest);
		break;
		case SDL_ALPHA_TRANSPARENT:
		return;
		default:
			DrawRGBA(bmpDest, bmpSrc, rDest, rSrc); // To handle the per-surface alpha correctly
		}

	// RGBA -> RGBA
	} else {
		DrawRGBA(bmpDest, bmpSrc, rDest, rSrc);
	}
}

/////////////////////////
// Draw the image tiled on the dest surface
void DrawImageTiled(SDL_Surface *bmpDest, SDL_Surface *bmpSrc, int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh)
{
	// Place the tiles
	for (int y = 0; y < dh; y += sh)
		for (int x = 0; x < dw; x += sw)
			DrawImageAdv(bmpDest, bmpSrc, sx, sy, dx + x, dy + y, sw, sh);
}

/////////////////////////
// Draw the image tiled on the dest surface, only in the X direction
void DrawImageTiledX(SDL_Surface *bmpDest, SDL_Surface *bmpSrc, int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh)
{
	// Place the tiles
	for (int x = 0; x < dw; x += sw)
		DrawImageAdv(bmpDest, bmpSrc, sx, sy, dx + x, dy, sw, MIN(dh, sh));
}

/////////////////////////
// Draw the image tiled on the dest surface, only in the Y direction
void DrawImageTiledY(SDL_Surface *bmpDest, SDL_Surface *bmpSrc, int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh)
{
	// Place the tiles
	for (int y = 0; y < dh; y += sh)
		DrawImageAdv(bmpDest, bmpSrc, sx, sy, dx, dy + y, MIN(dw, sw), sh);
}


///////////////////
// Draw the image mirrored with a huge amount of options
void DrawImageAdv_Mirror(SDL_Surface * bmpDest, SDL_Surface * bmpSrc, int sx, int sy, int dx, int dy, int w, int h)
{
	// Warning: Both surfaces have to have same bpp!
	assert(bmpDest->format->BytesPerPixel == bmpSrc->format->BytesPerPixel);

	// Clipping on source surface
	if (!ClipRefRectWith(sx, sy, w, h, (SDLRect&)bmpSrc->clip_rect))
		return;

	// Clipping on dest surface
	if (!ClipRefRectWith(dx, dy, w, h, (SDLRect&)bmpDest->clip_rect))
		return;

	int x,y;

	// Lock the surfaces
	LOCK_OR_QUIT(bmpDest);
	LOCK_OR_QUIT(bmpSrc);


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
	UnlockSurface(bmpDest);
	UnlockSurface(bmpSrc);
}


///////////////////
// Draws a sprite doubly stretched
void DrawImageStretch2(SDL_Surface * bmpDest, SDL_Surface * bmpSrc, int sx, int sy, int dx, int dy, int w, int h)
{
	// HINT: since the new copy-functors have been available, this function accepts surfaces of any format
	// However, passing surfaces with same format (bpp) makes this function faster

	// Clipping
	{
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
	}

	// Lock the surfaces
	LOCK_OR_QUIT(bmpDest);
	LOCK_OR_QUIT(bmpSrc);

	Uint8 *TrgPix = (Uint8 *)bmpDest->pixels + dy*bmpDest->pitch + dx*bmpDest->format->BytesPerPixel;
	Uint8 *SrcPix = (Uint8 *)bmpSrc->pixels +  sy*bmpSrc->pitch + sx*bmpSrc->format->BytesPerPixel;

	int doublepitch = bmpDest->pitch*2;
	int sbpp = bmpSrc->format->BytesPerPixel;
	int dbpp = bmpDest->format->BytesPerPixel;
	PixelCopy& copier = getPixelCopyFunc(bmpSrc, bmpDest);

    for(int y = h; y; --y) {

		Uint8 *sp = SrcPix;
		Uint8 *tp_x1 = TrgPix;
		Uint8 *tp_x2 = tp_x1 + bmpDest->pitch;
		for(int x = w; x; --x) {
            // Copy the 1 source pixel into a 4 pixel block on the destination surface
			copier.copy(tp_x1, sp); tp_x1 += dbpp;
			copier.copy(tp_x1, sp); tp_x1 += dbpp;
			copier.copy(tp_x2, sp); tp_x2 += dbpp;
			copier.copy(tp_x2, sp); tp_x2 += dbpp;
			sp += sbpp;
		}
		TrgPix += doublepitch;
		SrcPix += bmpSrc->pitch;
	}

	// Unlock em
	UnlockSurface(bmpDest);
	UnlockSurface(bmpSrc);
}


///////////////////
// Draws a sprite doubly stretched with colour key
// HINT: doesn't work with alpha-surfaces
void DrawImageStretch2Key(SDL_Surface * bmpDest, SDL_Surface * bmpSrc, int sx, int sy, int dx, int dy, int w, int h)
{
	// HINT: since the new copy-functors have been available, this function accepts surfaces of any format
	// However, passing surfaces with same format (bpp) makes this function faster

	// Clipping
	{
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
	}

	// Lock the surfaces
	LOCK_OR_QUIT(bmpDest);
	LOCK_OR_QUIT(bmpSrc);

	Uint8 *TrgPix = (Uint8 *)bmpDest->pixels + dy*bmpDest->pitch + dx*bmpDest->format->BytesPerPixel;
	Uint8 *SrcPix = (Uint8 *)bmpSrc->pixels +  sy*bmpSrc->pitch + sx*bmpSrc->format->BytesPerPixel;

	int doublepitch = bmpDest->pitch*2;
	int sbpp = bmpSrc->format->BytesPerPixel;
	int dbpp = bmpDest->format->BytesPerPixel;
	PixelCopy& copier = getPixelCopyFunc(bmpSrc, bmpDest);
	PixelGet& getter = getPixelGetFunc(bmpSrc);

    for(int y = h; y; --y) {

		Uint8 *sp = SrcPix;
		Uint8 *tp_x1 = TrgPix;
		Uint8 *tp_x2 = tp_x1 + bmpDest->pitch;
		for(int x = w; x; --x) {
			if (IsTransparent(bmpSrc, getter.get(sp)))  {
				// Skip the transparent pixel
				tp_x1 += dbpp * 2;
				tp_x2 += dbpp * 2;
			} else {
				// Copy the 1 source pixel into a 4 pixel block on the destination surface
				copier.copy(tp_x1, sp); tp_x1 += dbpp;
				copier.copy(tp_x1, sp); tp_x1 += dbpp;
				copier.copy(tp_x2, sp); tp_x2 += dbpp;
				copier.copy(tp_x2, sp); tp_x2 += dbpp;
			}
			sp += sbpp;
		}
		TrgPix += doublepitch;
		SrcPix += bmpSrc->pitch;
	}

	// Unlock em
	UnlockSurface(bmpDest);
	UnlockSurface(bmpSrc);
}


///////////////////
// Draws a sprite mirrored doubly stretched with colour key
void DrawImageStretchMirrorKey(SDL_Surface *bmpDest, SDL_Surface * bmpSrc, int sx, int sy, int dx, int dy, int w, int h)
{
	// Clipping
	{
		int dw = w * 2;
		int dh = h * 2;

		// Source clipping
		if (!ClipRefRectWith(sx, sy, w, h, (SDLRect&)bmpSrc->clip_rect))
			return;

		// Clipping on dest surface
		if (!ClipRefRectWith(dx, dy, dw, dh, (SDLRect&)bmpDest->clip_rect))
			return;

		// Clipping could change w or h
		w = MIN(w, dw / 2);
		h = MIN(h, dh / 2);
	}


	// Lock the surfaces
	LOCK_OR_QUIT(bmpDest);
	LOCK_OR_QUIT(bmpSrc);


	Uint8 *TrgPix = (Uint8 *)bmpDest->pixels + dy*bmpDest->pitch + dx*bmpDest->format->BytesPerPixel;
	Uint8 *SrcPix = (Uint8 *)bmpSrc->pixels + sy*bmpSrc->pitch + sx*bmpSrc->format->BytesPerPixel;

	// Pre-calculate some things, so the loop is faster
	int doublepitch = bmpDest->pitch*2;
	int sbpp = bmpSrc->format->BytesPerPixel;
	int dbpp = bmpDest->format->BytesPerPixel;
	
	PixelGet& getter = getPixelGetFunc(bmpSrc);
	PixelCopy& copier = getPixelCopyFunc(bmpSrc, bmpDest);

    for(int y = h; y; --y) {

		Uint8 *sp = SrcPix;
		Uint8 *tp_x1 = TrgPix + w * 2 * dbpp;
		Uint8 *tp_x2 = tp_x1 + bmpDest->pitch;
		for(int x = w; x; --x) {
			if (!IsTransparent(bmpSrc, getter.get(sp)))  {
				// Non-transparent
				// Copy the 1 source pixel into a 4 pixel block on the destination surface
				copier.copy(tp_x1, sp); tp_x1 -= dbpp;
				copier.copy(tp_x1, sp); tp_x1 -= dbpp;
				copier.copy(tp_x2, sp); tp_x2 -= dbpp;
				copier.copy(tp_x2, sp); tp_x2 -= dbpp;
			} else {
				// Skip to next pixel
				tp_x1 -= dbpp * 2;
				tp_x2 -= dbpp * 2;
			}
			sp += sbpp;
		}
		TrgPix += doublepitch;
		SrcPix += bmpSrc->pitch;
	}


	// Unlock em
	UnlockSurface(bmpDest);
	UnlockSurface(bmpSrc);
}

/////////////////////////
// Draw the image resized
void DrawImageResizedAdv(SDL_Surface * bmpDest, SDL_Surface * bmpSrc, int sx, int sy, int dx, int dy, int sw, int sh, int dw, int dh)
{
	// Source clipping
	if (!ClipRefRectWith(sx, sy, sw, sh, (SDLRect&)bmpSrc->clip_rect))
		return;

	// Dest clipping
	if (!ClipRefRectWith(dx, dy, dw, dh, (SDLRect&)bmpDest->clip_rect))
		return;

	float xstep = (float)sw/(float)dw; // X step we'll do on the source surface
	float ystep = (float)sh/(float)dh; // Y step we'll do on the source surface

	// Update the widths/heights according to clipping
	dw = MIN(dw, Round((float)sw * xstep));
	dh = MIN(dh, Round((float)sh * ystep));

	// Lock the surfaces
	LOCK_OR_QUIT(bmpSrc);
	LOCK_OR_QUIT(bmpDest);

	// Pixels
	Uint8 *dst_px = NULL;
	Uint8 *dst_pxrow = (Uint8 *)bmpDest->pixels + (dy * bmpDest->pitch) + (dx * bmpDest->format->BytesPerPixel);
	int sbpp = bmpSrc->format->BytesPerPixel;
	int dbpp = bmpDest->format->BytesPerPixel;
	PixelCopy& copier = getPixelCopyFunc(bmpSrc, bmpDest);

	// Resize
	int dest_y = 0;
	for (float src_y = (float)sy; dest_y < dh; dest_y++)  {
		Uint8 *src_pxrow = (Uint8 *)bmpSrc->pixels + (int)(src_y) * bmpSrc->pitch;
		dst_px = dst_pxrow;

		int dest_x = dw;

		// Copy the row
		for (float src_x = (float)sx; dest_x; --dest_x)  {
			const Uint8 *src_px = src_pxrow + (int)(src_x) * sbpp;
			copier.copy(dst_px, src_px);
			src_x += xstep;
			dst_px += dbpp;
		}

		src_y += ystep;
		dst_pxrow += bmpDest->pitch;
	}

	// Unlock
	UnlockSurface(bmpDest);
	UnlockSurface(bmpSrc);
}

/////////////////////////
// Draw the image resized
void DrawImageResizedAdv(SDL_Surface * bmpDest, SDL_Surface * bmpSrc, int sx, int sy, int dx, int dy, int sw, int sh, float xratio, float yratio)
{
	int dw = Round((float)sw * xratio);
	int dh = Round((float)sh * yratio);
	DrawImageResizedAdv(bmpDest, bmpSrc, sx, sy, dx, dy, sw, sh, dw, dh);
}

////////////////////////
// Draws the image nicely resampled
void DrawImageResampledAdv(SDL_Surface * bmpDest, SDL_Surface * bmpSrc, int sx, int sy, int dx, int dy, int sw, int sh, float xratio, float yratio)
{
	if(!bmpSrc || !bmpDest) return;
	
	SDL_Rect src = { sx, sy, sw, sh };
	SDL_Rect dst = { dx, dy, (int)((float)sw * xratio), (int)((float)sh * yratio) };
	
	// Source clipping
	if (!ClipRefRectWith((SDLRect&)src, (SDLRect&)bmpSrc->clip_rect))
		return;

	// Dest clipping
	if (!ClipRefRectWith((SDLRect&)dst, (SDLRect&)bmpDest->clip_rect))
		return;
	
	if (dst.w != 0 && dst.h != 0)
		SDL_SoftStretch(bmpSrc, &src, bmpDest, &dst);
}

////////////////////////
// Draws the image nicely resampled
void DrawImageResampledAdv(SDL_Surface * bmpDest, SDL_Surface * bmpSrc, int sx, int sy, int dx, int dy, int sw, int sh, int dw, int dh)
{
	if(!bmpSrc || !bmpDest) return;

	SDL_Rect src = { sx, sy, sw, sh };
	SDL_Rect dst = { dx, dy, dw, dh };
	
	// Source clipping
	if (!ClipRefRectWith((SDLRect&)src, (SDLRect&)bmpSrc->clip_rect))
		return;

	// Dest clipping
	if (!ClipRefRectWith((SDLRect&)dst, (SDLRect&)bmpDest->clip_rect))
		return;
	
	if (dw != 0 && dh != 0)
		SDL_SoftStretch(bmpSrc, &src, bmpDest, &dst);
}

//
// Helper functions for the Scale2x algorithm
//

/*
	[A] [B] [C]
	[D] [E] [F]
	[G] [H] [I]
*/

// HINT: the A, C, G, I pixels are not used by the Scale2x algorithm
enum { B = 0, D, E, F, H };

////////////////////
// Does the source and destination surface clipping
static bool Clip2x(SDL_Surface* bmpDest, SDL_Surface* bmpSrc, int& sx, int& sy, int& dx, int& dy, int& w, int& h)
{
	if (!ClipRefRectWith(sx, sy, w, h, (SDLRect&)bmpSrc->clip_rect))
		return false;

	int dw = w * 2;
	int dh = h * 2;

	if (!ClipRefRectWith(dx, dy, dw, dh, (SDLRect&)bmpDest->clip_rect))
		return false;

	w = MIN(w, dw/2);
	h = MIN(h, dh/2);

	return true;
}

static void Scale2xPixel(SDL_Surface *bmpDest, SDL_Surface *bmpSrc, int dx, int dy, Uint32 *colors, PixelPut& putter)
{
	// Put the 2x2 square on the dest surface
	const int bpp = bmpDest->format->BytesPerPixel;

	Uint8 *pixel = GetPixelAddr(bmpDest, dx, dy);

	// Get the four colors to put on the dest surface
	Uint32 dstE[4];
	if (colors[B] != colors[H] && colors[D] != colors[F]) {
		dstE[0] = colors[D] == colors[B] ? colors[D] : colors[E];
		dstE[1] = colors[B] == colors[F] ? colors[F] : colors[E];
		dstE[2] = colors[D] == colors[H] ? colors[D] : colors[E];
		dstE[3] = colors[H] == colors[F] ? colors[F] : colors[E];

		// Put the four pixels
		putter.put(pixel, dstE[0]);
		putter.put(pixel + bpp, dstE[1]);

		pixel += bmpDest->pitch;
		putter.put(pixel, dstE[2]);
		putter.put(pixel + bpp, dstE[3]);
	} else {
		// One, solid pixel
		putter.put(pixel, colors[E]);
		putter.put(pixel + bpp, colors[E]);

		pixel += bmpDest->pitch;
		putter.put(pixel, colors[E]);
		putter.put(pixel + bpp, colors[E]);
	}
}

static void Scale2xPixel_FF(SDL_Surface *bmpDest, SDL_Surface *bmpSrc, 
							int sx, int sy, int dx, int dy, PixelGet& getter, PixelPut& putter)
{
	Uint8 *addr = GetPixelAddr(bmpSrc, sx, sy);
	Uint32 colors[5];
	colors[B] = colors[D] = colors[E] = getter.get(addr);
	colors[F] = getter.get(addr + bmpSrc->format->BytesPerPixel); // x + 1
	colors[H] = getter.get(addr + bmpSrc->pitch);  // y + 1
	Scale2xPixel(bmpDest, bmpSrc, dx, dy, colors, putter);
}

static void Scale2xPixel_LF(SDL_Surface *bmpDest, SDL_Surface *bmpSrc,
							int sx, int sy, int dx, int dy, PixelGet& getter, PixelPut& putter)
{
	Uint8 *addr = GetPixelAddr(bmpSrc, sx, sy);
	Uint32 colors[5];
	colors[B] = colors[E] = colors[F] = getter.get(addr);
	colors[D] = getter.get(addr - bmpSrc->format->BytesPerPixel);
	colors[H] = getter.get(addr + bmpSrc->pitch);
	Scale2xPixel(bmpDest, bmpSrc, dx, dy, colors, putter);
}

static void Scale2xPixel_FL(SDL_Surface *bmpDest, SDL_Surface *bmpSrc,
							int sx, int sy, int dx, int dy, PixelGet& getter, PixelPut& putter)
{
	Uint8 *addr = GetPixelAddr(bmpSrc, sx, sy);
	Uint32 colors[5];
	colors[D] = colors[E] = colors[H] = getter.get(addr);
	colors[B] = getter.get(addr - bmpSrc->pitch);
	colors[F] = getter.get(addr + bmpSrc->format->BytesPerPixel);
	Scale2xPixel(bmpDest, bmpSrc, dx, dy, colors, putter);
}

static void Scale2xPixel_LL(SDL_Surface *bmpDest, SDL_Surface *bmpSrc,
							int sx, int sy, int dx, int dy, PixelGet& getter, PixelPut& putter)
{
	Uint8 *addr = GetPixelAddr(bmpSrc, sx, sy);
	Uint32 colors[5];
	colors[E] = colors[F] = colors[H] = getter.get(addr);
	colors[B] = getter.get(addr - bmpSrc->pitch);
	colors[D] = getter.get(addr - bmpSrc->format->BytesPerPixel);
	Scale2xPixel(bmpDest, bmpSrc, dx, dy, colors, putter);
}

static void Scale2xPixel_F(SDL_Surface *bmpDest, SDL_Surface *bmpSrc,
						   int sx, int sy, int dx, int dy, PixelGet& getter, PixelPut& putter)
{
	Uint8 *addr = GetPixelAddr(bmpSrc, sx, sy);
	Uint32 colors[5];
	colors[D] = colors[E] = getter.get(addr);
	colors[B] = getter.get(addr - bmpSrc->pitch);
	colors[F] = getter.get(addr + bmpSrc->format->BytesPerPixel);
	colors[H] = getter.get(addr + bmpSrc->pitch);
	Scale2xPixel(bmpDest, bmpSrc, dx, dy, colors, putter);
}

static void Scale2xPixel_L(SDL_Surface *bmpDest, SDL_Surface *bmpSrc,
						   int sx, int sy, int dx, int dy, PixelGet& getter, PixelPut& putter)
{
	Uint8 *addr = GetPixelAddr(bmpSrc, sx, sy);
	Uint32 colors[5];
	colors[E] = colors[F] = getter.get(addr);
	colors[B] = getter.get(addr - bmpSrc->pitch);
	colors[D] = getter.get(addr - bmpSrc->format->BytesPerPixel);
	colors[H] = getter.get(addr + bmpSrc->pitch);
	Scale2xPixel(bmpDest, bmpSrc, dx, dy, colors, putter);
}

/////////////////////////
// Draws the image double-sized using the scale2x algorithm
// This algo is taken from http://scale2x.sourceforge.net/algorithm.html
// Thanks go to the AdvanceMAME team!
void DrawImageScale2x(SDL_Surface* bmpDest, SDL_Surface* bmpSrc, int sx, int sy, int dx, int dy, int w, int h)
{
	PixelPut& putter = getPixelPutFunc(bmpDest);
	PixelGet& getter = getPixelGetFunc(bmpSrc);
	const unsigned sbpp = bmpSrc->format->BytesPerPixel;
	Uint8 *px1, *px2;

	// Lock
	LOCK_OR_QUIT(bmpDest);
	LOCK_OR_QUIT(bmpSrc);

	// Clipping
	if (!Clip2x(bmpDest, bmpSrc, sx, sy, dx, dy, w, h))
		return;

	if (w < 2 || h < 2)  {
		DrawImageStretch2(bmpDest, bmpSrc, sx, sy, dx, dy, w, h);
		return;
	}

	// Variables
	int sx2 = sx + w - 1;
	int sy2 = sy + h - 1;

	Uint32 colors[5];

	// First pixel, first line
	Scale2xPixel_FF(bmpDest, bmpSrc, sx, sy, dx, dy, getter, putter);

	// Last pixel, first line
	Scale2xPixel_LF(bmpDest, bmpSrc, sx2, sy, dx + w * 2 - 2, dy, getter, putter);

	// First line
	px1 = GetPixelAddr(bmpSrc, sx + 1, sy);
	px2 = px1 + bmpSrc->pitch;
	for(int x = 1; x < w - 1; ++x, px1 += sbpp, px2 += sbpp)  {
		colors[B] = colors[E] = getter.get(px1);
		colors[D] = getter.get(px1 - sbpp);
		colors[F] = getter.get(px1 + sbpp);
		colors[H] = getter.get(px2);

		Scale2xPixel(bmpDest, bmpSrc, dx + x * 2, dy, colors, putter);
	}

	// First pixel, last line
	Scale2xPixel_FL(bmpDest, bmpSrc, sx, sy2, dx, dy + h * 2 - 2, getter, putter);

	// Last pixel, last line
	Scale2xPixel_LL(bmpDest, bmpSrc, sx2, sy2, dx + w * 2 - 2, dy + h * 2 - 2, getter, putter);


	// Last line
	px1 = GetPixelAddr(bmpSrc, sx + 1, sy2 - 1);
	px2 = px1 + bmpSrc->pitch;
	for(int x = 1; x < w - 1; ++x, px1 += sbpp, px2 += sbpp)  {
		colors[E] = colors[H] = getter.get(px2);
		colors[B] = getter.get(px1);
		colors[D] = getter.get(px2 - sbpp);
		colors[F] = getter.get(px2 + sbpp);

		Scale2xPixel(bmpDest, bmpSrc, dx + x * 2, dy + h * 2 - 2, colors, putter);
	}

	// Rest of the image
	for(int y = 1; y < h - 1; ++y) {
		const int y2 = y * 2;

		// First & last pixel
		Scale2xPixel_F(bmpDest, bmpSrc, sx, sy + y, dx, dy + y2, getter, putter);
		Scale2xPixel_L(bmpDest, bmpSrc, sx2, sy + y, dx + w * 2 - 2, dy + y2, getter, putter);

		// Rest of the line
		Uint8 *px = GetPixelAddr(bmpSrc, sx + w - 2, sy + y);
		for(int x = w - 2; x; --x, px -= sbpp) {
			colors[B] = getter.get(px - bmpSrc->pitch);
			colors[D] = getter.get(px - sbpp);
			colors[E] = getter.get(px);
			colors[F] = getter.get(px + sbpp);
			colors[H] = getter.get(px + bmpSrc->pitch);

			Scale2xPixel(bmpDest, bmpSrc, dx + x * 2, dy + y2, colors, putter);
		}
	}

	// Unlock
	UnlockSurface(bmpDest);
	UnlockSurface(bmpSrc);
}

///////////////////////
// Helper function for HalfImageScaleHalf
static Uint32 HalfBlendPixel(Uint32 p1, Uint32 p2, Uint32 p3, Uint32 p4, SDL_PixelFormat *fmt)
{
/*#define PIXEL_IMPACT(pixel, channel) (((pixel & fmt->##channel##mask) >> fmt->##channel##shift)/4)
	return ((PIXEL_IMPACT(p1, R) + PIXEL_IMPACT(p2, R) + PIXEL_IMPACT(p3, R) + PIXEL_IMPACT(p4, R)) << fmt->Rshift) |
		((PIXEL_IMPACT(p1, G) + PIXEL_IMPACT(p2, G) + PIXEL_IMPACT(p3, G) + PIXEL_IMPACT(p4, G)) << fmt->Gshift) |
		((PIXEL_IMPACT(p1, B) + PIXEL_IMPACT(p2, B) + PIXEL_IMPACT(p3, B) + PIXEL_IMPACT(p4, B)) << fmt->Bshift);*/

	// TODO: does this work on big endian systems? (there could be an overflow in the red channel)
#define PIXEL_CHANNEL(pixel, channel) ( (Uint32) (pixel & fmt-> channel##mask) )
	return (Uint32) (
		((((PIXEL_CHANNEL(p1, R) + PIXEL_CHANNEL(p2, R) + PIXEL_CHANNEL(p3, R) + PIXEL_CHANNEL(p4, R))) / 4) & fmt->Rmask) |
		((((PIXEL_CHANNEL(p1, G) + PIXEL_CHANNEL(p2, G) + PIXEL_CHANNEL(p3, G) + PIXEL_CHANNEL(p4, G))) / 4) & fmt->Gmask) |
		((((PIXEL_CHANNEL(p1, B) + PIXEL_CHANNEL(p2, B) + PIXEL_CHANNEL(p3, B) + PIXEL_CHANNEL(p4, B))) / 4) & fmt->Bmask) ) |
		fmt->Amask; // HINT: this doesn't support semi-transparent surfaces
}

void DrawImageScaleHalf(SDL_Surface* bmpDest, SDL_Surface* bmpSrc) {

	// Lock
	LOCK_OR_QUIT(bmpDest);
	LOCK_OR_QUIT(bmpSrc);

	short bpp = bmpDest->format->BytesPerPixel;
	Uint8 *srcpx_1 = (Uint8 *)bmpSrc->pixels;
	Uint8 *srcpx_2 = (Uint8 *)bmpSrc->pixels + bmpSrc->pitch;
	int srcgap = bmpSrc->pitch * 2 - bmpSrc->w * bmpSrc->format->BytesPerPixel; // *2 because we skip two lines on the source surface
	Uint8 *dstpx = (Uint8 *)bmpDest->pixels;
	int dstgap = bmpDest->pitch - bmpDest->w * bmpDest->format->BytesPerPixel;

	int w = bmpSrc->w / 2;
	int h = bmpSrc->h / 2;
	for(int y = h; y; --y, srcpx_1 += srcgap, srcpx_2 += srcgap, dstpx += dstgap)
		for(int x = w; x; --x, srcpx_1 += bpp * 2, srcpx_2 += bpp * 2, dstpx += bpp) {
			const Uint32 px1 = GetPixelFromAddr(srcpx_1, bpp);  // x, y
			const Uint32 px2 = GetPixelFromAddr(srcpx_1 + bpp, bpp); // x + 1, y
			const Uint32 px3 = GetPixelFromAddr(srcpx_2, bpp);  // x, y + 1
			const Uint32 px4 = GetPixelFromAddr(srcpx_2 + bpp, bpp);  // x + 1, y + 1
			PutPixelToAddr(dstpx, HalfBlendPixel(px1, px2, px3, px4, bmpSrc->format), bpp);
		}

	// Unlock
	UnlockSurface(bmpDest);
	UnlockSurface(bmpSrc);

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
void RopePutPixelA(SDL_Surface * bmpDest, int x, int y, Uint32 colour, Uint8 alpha)
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
void RopePutPixel(SDL_Surface * bmpDest, int x, int y, Uint32 colour) {
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
void BeamPutPixelA(SDL_Surface * bmpDest, int x, int y, Uint32 colour, Uint8 alpha)
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

void BeamPutPixel(SDL_Surface * bmpDest, int x, int y, Uint32 colour) { // For compatibility with perform_line
	BeamPutPixelA(bmpDest, x, y, colour, 255);
}



///////////////////
// Put a laser-sight pixel on the surface
void LaserSightPutPixel(SDL_Surface * bmpDest, int x, int y, Uint32 colour)
{
	static int laseralt = 0;
	laseralt++;
	laseralt %= GetRandomInt(35)+1;

	if(laseralt != 0)
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
inline void perform_line(SDL_Surface * bmp, int x1, int y1, int x2, int y2, Color col, void (*proc)(SDL_Surface * , int, int, Uint32, Uint8))
{
	int dx = x2-x1;
	int dy = y2-y1;
	int i1, i2;
	int x, y;
	int dd;
	
	Uint32 d = col.get(bmp->format);

   LOCK_OR_QUIT(bmp);

   /* worker macro */
   #define DO_LINE(pri_sign, pri_c, pri_cond, sec_sign, sec_c, sec_cond)     \
   {                                                                         \
      if (d##pri_c == 0) {                                                   \
		 proc(bmp, x1, y1, d, col.a);                                               \
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
		 proc(bmp, x, y, d, col.a);                                                 \
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

#undef DO_LINE
	
   UnlockSurface(bmp);
}

inline void perform_line(SDL_Surface * bmp, int x1, int y1, int x2, int y2, Color col, void (*proc)(SDL_Surface * , int, int, Uint32))
{
	int dx = x2-x1;
	int dy = y2-y1;
	int i1, i2;
	int x, y;
	int dd;
	
	Uint32 d = col.get(bmp->format);
	
	LOCK_OR_QUIT(bmp);
	
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
	
	#undef DO_LINE
	
	UnlockSurface(bmp);
}



inline void secure_perform_line(SDL_Surface * bmpDest, int x1, int y1, int x2, int y2, Color color, void (*proc)(SDL_Surface *, int, int, Uint32, Uint8)) {
	if (!ClipLine(bmpDest, &x1, &y1, &x2, &y2)) // Clipping
		return;

	perform_line(bmpDest, x1, y1, x2, y2, color, proc);
}

inline void secure_perform_line(SDL_Surface * bmpDest, int x1, int y1, int x2, int y2, Color color, void (*proc)(SDL_Surface *, int, int, Uint32)) {
	if (!ClipLine(bmpDest, &x1, &y1, &x2, &y2)) // Clipping
		return;
	
	perform_line(bmpDest, x1, y1, x2, y2, color, proc);
}


////////////////////////
// Draw horizontal line
void DrawHLine(SDL_Surface * bmpDest, int x, int x2, int y, Color colour) {

	if (bmpDest->flags & SDL_HWSURFACE)  {
		DrawRectFill(bmpDest, x, y, x2, y + 1, colour); // In hardware mode this is much faster, in software it is slower
		return;
	}

	// Swap the ends if necessary
	if (x2 < x)  {
		int tmp;
		tmp = x;
		x = x2;
		x2 = tmp;
	}

	const SDL_Rect& r = bmpDest->clip_rect;

	// Clipping
	if (y < r.y) return;
	if (y >= (r.y + r.h)) return;

	if (x < r.x)
		x = r.x;
	else if (x >= (r.x + r.w))
		return;

	if (x2 < r.x)
		return;
	else if (x2 >= (r.x + r.w))
		x2 = r.x + r.w - 1;

	// Lock
	LOCK_OR_QUIT(bmpDest);
	byte bpp = (byte)bmpDest->format->BytesPerPixel;
	Uint8 *px2 = (uchar *)bmpDest->pixels+bmpDest->pitch*y+bpp*x2;

	// Draw depending on the alpha
	switch (colour.a)  {
	case SDL_ALPHA_OPAQUE:  
	{
		// Solid (no alpha) drawing
		PixelPut& putter = getPixelPutFunc(bmpDest);
		Uint32 packed_cl = Pack(colour, bmpDest->format);
		for (Uint8* px = (Uint8*)bmpDest->pixels + bmpDest->pitch * y + bpp * x; px <= px2; px += bpp)
			putter.put(px, packed_cl);
	} break;
	case SDL_ALPHA_TRANSPARENT:
	break;
	default:
	{
		// Draw the line alpha-blended with the background
		PixelPutAlpha& putter = getPixelAlphaPutFunc(bmpDest);
		for (Uint8* px = (Uint8*)bmpDest->pixels + bmpDest->pitch * y + bpp * x; px <= px2; px += bpp)
			putter.put(px, bmpDest->format, colour);
	}
	}

	UnlockSurface(bmpDest);

}

// Draw vertical line
void DrawVLine(SDL_Surface * bmpDest, int y, int y2, int x, Color colour) {
	if (bmpDest->flags & SDL_HWSURFACE)  {
		DrawRectFill(bmpDest, x, y, x + 1, y2, colour); // In hardware mode this is much faster, in software it is slower
		return;
	}

	// Swap the ends if necessary
	if (y2 < y)  {
		int tmp;
		tmp = y;
		y = y2;
		y2 = tmp;
	}

	const SDL_Rect& r = bmpDest->clip_rect;

	// Clipping
	if (x < r.x) return;
	if (x >= (r.x + r.w)) return;

	if (y < r.y)
		y = r.y;
	else if (y >= (r.y + r.h))
		return;

	if (y2 < r.y)
		return;
	else if (y2 >= (r.y + r.h))
		y2 = r.y + r.h - 1;

	LOCK_OR_QUIT(bmpDest);
	ushort pitch = (ushort)bmpDest->pitch;
	byte bpp = (byte)bmpDest->format->BytesPerPixel;
	Uint8 *px2 = (Uint8 *)bmpDest->pixels+pitch*y2+bpp*x;

	// Draw depending on the alpha
	switch (colour.a)  {
	case SDL_ALPHA_OPAQUE:  
	{
		// Solid (no alpha) drawing
		PixelPut& putter = getPixelPutFunc(bmpDest);
		Uint32 packed_cl = Pack(colour, bmpDest->format);
		for (Uint8 *px= (Uint8 *)bmpDest->pixels+pitch*y + bpp*x; px <= px2; px+=pitch)
			putter.put(px, packed_cl);
	} break;
	case SDL_ALPHA_TRANSPARENT:
	break;
	default:
	{
		// Draw the line alpha-blended with the background
		PixelPutAlpha& putter = getPixelAlphaPutFunc(bmpDest);
		for (Uint8 *px= (Uint8 *)bmpDest->pixels+pitch*y + bpp*x; px <= px2; px+=pitch)
			putter.put(px, bmpDest->format, colour);
	}
	}

	UnlockSurface(bmpDest);
}

///////////////////
// Line drawing
void DrawLine(SDL_Surface * dst, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Color color) {
	secure_perform_line(dst, x1, y1, x2, y2, color, PutPixelA);
}

//////////////////
// Fast routine for drawing 2x2 filled rects
void DrawRectFill2x2_NoClip(SDL_Surface *bmpDest, int x, int y, Color color)
{
	LOCK_OR_QUIT(bmpDest);

	Uint8 *row1 = (Uint8 *)bmpDest->pixels + y * bmpDest->pitch + x * bmpDest->format->BytesPerPixel;
	Uint8 *row2 = row1 + bmpDest->pitch;

	PixelPutAlpha& putter = getPixelAlphaPutFunc(bmpDest);
	putter.put(row1, bmpDest->format, color);
	putter.put(row1 + bmpDest->format->BytesPerPixel, bmpDest->format, color);
	putter.put(row2, bmpDest->format, color);
	putter.put(row2 + bmpDest->format->BytesPerPixel, bmpDest->format, color);

	UnlockSurface(bmpDest);
}

///////////////////////
// Draws the 2x2 filled rectangle, does clipping
void DrawRectFill2x2(SDL_Surface *bmpDest, int x, int y, Color color)
{
	if(color.a == SDL_ALPHA_TRANSPARENT) return;
	
	if (x < 0 || x + 2 >= bmpDest->clip_rect.x + bmpDest->clip_rect.w)
		return;
	if (y < 0 || y + 2 >= bmpDest->clip_rect.y + bmpDest->clip_rect.h)
		return;

	DrawRectFill2x2_NoClip(bmpDest, x, y, color);
}

//////////////////////
// Draw antialiased line with a putpixel callback
// Code basis taken from CTGraphics by darkoman (http://www.codeproject.com/gdi/CTGraphics.asp)
void AntiAliasedLine(SDL_Surface * dst, int x1, int y1, int x2, int y2, Color color, void (*proc)(SDL_Surface *, int, int, Uint32, Uint8))
{
	// Clipping
	if (!ClipLine(dst, &x1, &y1, &x2, &y2))
		return;

	// Calculate line params
	int dx = (x2 - x1);
	int dy = (y2 - y1);
	int k;
	Uint16 distance;
	Uint8 alpha = color.a;
	color.a = SDL_ALPHA_OPAQUE;
	LOCK_OR_QUIT(dst);

	Uint32 packed_c = color.get(dst->format);

	// Set start pixel
	proc(dst, x1, y1, packed_c, alpha);

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
			proc(dst, xs, yt / 256, packed_c, (Uint8) ((255 - distance) * alpha / 255));
			proc(dst, xs, yt / 256 + 1, packed_c, (Uint8) (distance * alpha / 255));

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
			proc(dst, xt / 256, ys, packed_c, (Uint8) ((255 - distance) * alpha / 255));
			proc(dst, xt / 256 + 1, ys, packed_c, (Uint8) (distance * alpha / 255));

			xt += k;
			distance = xt & 255;  // Same as: xt % 256
		}
	}

	UnlockSurface(dst);
}

static void DrawRectFill_Overlay(SDL_Surface *bmpDest, const SDL_Rect& r, Color color)
{
	// Clipping
	if (!ClipRefRectWith((SDLRect&)r, (SDLRect&)bmpDest->clip_rect))
		return;

	const int bpp = bmpDest->format->BytesPerPixel;
	Uint8 *px = (Uint8 *)bmpDest->pixels + r.y * bmpDest->pitch + r.x * bpp;
	int step = bmpDest->pitch - r.w * bpp;

	// Draw the fill rect
	PixelPutAlpha& putter = getPixelAlphaPutFunc(bmpDest);
	for (int y = r.h; y; --y, px += step)
		for (int x = r.w; x; --x, px += bpp)  {
			putter.put(px, bmpDest->format, color);
		}

}

//////////////////////
// Draws a filled rectangle
void DrawRectFill(SDL_Surface *bmpDest, int x, int y, int x2, int y2, Color color)
{
	SDL_Rect r = { x, y, x2 - x, y2 - y };

	switch (color.a)  {
		case SDL_ALPHA_OPAQUE:
			SDL_FillRect(bmpDest,&r,color.get(bmpDest->format));
			break;
		case SDL_ALPHA_TRANSPARENT:
			break;
		default:
			DrawRectFill_Overlay(bmpDest, r, color);
	}
}


void DrawCircleFilled(SDL_Surface* bmpDest, int x, int y, int rx, int ry, Color color) {
	if(color.a == SDL_ALPHA_TRANSPARENT) return;
	if(rx <= 0 || ry <= 0) return;
	if(rx == 1) { DrawVLine(bmpDest, y - ry, y + ry, x, color); return; }
	if(ry == 1) { DrawHLine(bmpDest, x - rx, x + rx, y, color); return; }
	
	int innerRectW = int(rx / sqrt(2.0));
	int innerRectH = int(ry / sqrt(2.0));
	DrawRectFill(bmpDest, x - innerRectW, y - innerRectH, x + innerRectW + 1, y + innerRectH + 1, color);
	
	float f = float(rx) / float(ry);
	for(int _y = innerRectH + 1; _y < ry; _y++) {
		int w = int(f * sqrt(float(ry*ry - _y*_y)));
			
		DrawHLine(bmpDest, x - w, x + w, y - _y, color);
		DrawHLine(bmpDest, x - w, x + w, y + _y, color);
	}

	f = 1.0f / f;
	for(int _x = innerRectW + 1; _x < rx; _x++) {
		int h = int(f * sqrt(float(rx*rx - _x*_x)));
			
		DrawVLine(bmpDest, y - h, y + h, x - _x, color);
		DrawVLine(bmpDest, y - h, y + h, x + _x, color);
	}
}

void TestCircleDrawing(SDL_Surface* s) {
	for(int i = 0; i < 10; i += 3) {
		DrawCircleFilled(s, i*50, i*25, i*25, i*25, Color(i*20,255,0,128));
	}
}


/////////////////////
// Draws a simple linear gradient
void DrawLinearGradient(SDL_Surface *bmpDest, int x, int y, int w, int h, Color cl1, Color cl2, GradientDirection dir)
{
	if (!ClipRefRectWith(x, y, w, h, (SDLRect&)bmpDest->clip_rect))
		return;

	float rstep, gstep, bstep, astep;

	switch (dir)  {
		case grdVertical:
			rstep = (float)(cl2.r - cl1.r) / (float)h;
			gstep = (float)(cl2.g - cl1.g) / (float)h;
			bstep = (float)(cl2.b - cl1.b) / (float)h;
			astep = (float)(cl2.a - cl1.a) / (float)h;

			for (int gy = 0; gy < h; gy++)  {
				Uint8 r = cl1.r + Round(gy * rstep);
				Uint8 g = cl1.g + Round(gy * gstep);
				Uint8 b = cl1.b + Round(gy * bstep);
				Uint8 a = cl1.a + Round(gy * astep);
				DrawHLine(bmpDest, x, x + w - 1, y + gy, Color(r, g, b, a));
			}
		break;

		case grdHorizontal:
			rstep = (float)(cl2.r - cl1.r) / (float)w;
			gstep = (float)(cl2.g - cl1.g) / (float)w;
			bstep = (float)(cl2.b - cl1.b) / (float)w;
			astep = (float)(cl2.a - cl1.a) / (float)w;

			for (int gx = 0; gx < w; gx++)  {
				Uint8 r = cl1.r + Round(gx * rstep);
				Uint8 g = cl1.g + Round(gx * gstep);
				Uint8 b = cl1.b + Round(gx * bstep);
				Uint8 a = cl1.a + Round(gx * astep);
				DrawVLine(bmpDest, y, y + h - 1, x + gx, Color(r, g, b, a));
			}
		break;
	}
}


///////////////////
// Draws a rope line
void DrawRope(SDL_Surface * bmp, int x1, int y1, int x2, int y2, Color color)
{
	ropealt = 0;
	ropecolour = 0;

	// Clipping
	if (!ClipLine(bmp, &x1, &y1, &x2, &y2))
		return;

	// Because we are drawing 2x stretched, we have to make sure we don't draw the boundary pixels
	x1 -= x1 & 1;
	y1 -= y1 & 1;
	x2 -= x2 & 1;
	y2 -= y2 & 1;

	if (tLXOptions->bAntiAliasing)
		AntiAliasedLine(bmp, x1, y1, x2, y2, color, RopePutPixelA);
	else
		perform_line(bmp, x1, y1, x2, y2, color.get(bmp->format), RopePutPixel);
}


///////////////////
// Draws a beam
void DrawBeam(SDL_Surface * bmp, int x1, int y1, int x2, int y2, Color color)
{
	// Clipping
	if (!ClipLine(bmp, &x1, &y1, &x2, &y2))
		return;

	// Because we are drawing 2x stretched, we have to make sure we don't draw the boundary pixels
	x1 -= x1 & 1;
	y1 -= y1 & 1;
	x2 -= x2 & 1;
	y2 -= y2 & 1;

	if (tLXOptions->bAntiAliasing)
		AntiAliasedLine(bmp, x1, y1, x2, y2, color, BeamPutPixelA);
	else {
		if(color.a != SDL_ALPHA_OPAQUE)
			perform_line(bmp, x1, y1, x2, y2, color, BeamPutPixelA);
		else
			perform_line(bmp, x1, y1, x2, y2, color, BeamPutPixel);
	}
}


///////////////////
// Draws a laser sight
void DrawLaserSight(SDL_Surface * bmp, int x1, int y1, int x2, int y2, Color color)
{
	// Clipping
	if (!ClipLine(bmp, &x1, &y1, &x2, &y2))
		return;

	// Because we are drawing 2x stretched, we have to make sure we don't draw the boundary pixels
	x1 -= x1 & 1;
	y1 -= y1 & 1;
	x2 -= x2 & 1;
	y2 -= y2 & 1;

	perform_line(bmp, x1, y1, x2, y2, color, LaserSightPutPixel);
}



bool Line::isRightFrom(int x, int y) const {
	VectorD2<int> rel = end - start;
	x -= start.x; y -= start.y;
	return rel.y * x <= y * rel.x;
}

bool Line::isBeforeStart(int x, int y) const {
	VectorD2<int> rel = end - start;
	x -= start.x; y -= start.y;
	return rel.x * x + rel.y * y < 0;
}

bool Line::isAfterEnd(int x, int y) const {
	VectorD2<int> rel = start - end;
	x -= end.x; y -= end.y;
	return rel.x * x + rel.y * y < 0;	
}

bool Line::isParallel(int x, int y) const {
	return !isBeforeStart(x,y) && !isAfterEnd(x,y);
}

bool Line::containsY(int y, int& x, bool aimsDown) const {
	if(start.y == end.y) {
		if(start.y == y) {
			if(!aimsDown) {
				if(start.x < end.x) x = start.x;
				else x = end.x;
			}
			else {
				if(start.x < end.x) x = end.x;
				else x = start.x;				
			}
			return true;
		}
		return false;
	}
	
	if(start.y <= y && end.y >= y && aimsDown) {
		VectorD2<int> rel = end - start;
		y -= start.y;
		x = rel.x * y / rel.y;
		x += start.x;
		return true;
	}
	
	if(start.y >= y && end.y <= y && !aimsDown) {
		VectorD2<int> rel = start - end;
		y -= end.y;
		x = rel.x * y / rel.y;
		x += end.x;
		return true;
	}
	
	return false;	
}

void Polygon2D::reloadLines() {
	if(points.size() == 0) {
		warnings << "Polygon2D::reloadLines: no points set" << endl;
		return;
	}
	doReloadLines = false;
	lines.clear();
	lines.reserve(points.size() - 1);
	Points::const_iterator j = points.begin(); ++j;
	for(Points::const_iterator i = points.begin(); j != points.end(); ++i, ++j) {
		Line l; l.start = *i; l.end = *j;
		lines.push_back(l);
	}
}


bool Polygon2D::getNext(int x, int y, int& nextx, bool inside) const {
	bool haveAny = false;
	for(Lines::const_iterator l = lines.begin(); l != lines.end(); ++l) {
		int lx;
		if(l->containsY(y, lx, !inside) && lx >= x) {
			if(!haveAny || lx < nextx) {
				haveAny = true;
				if(lx == x && !inside)
					nextx = lx + 1; // hack to go forward in such cases
				else
					nextx = lx;
			}
		}
	}
	return haveAny;
}


SDL_Rect Polygon2D::minOverlayRect() const {
	SDL_Rect r = {0,0,0,0};
	for(Points::const_iterator i = points.begin(); i != points.end(); ++i) {
		if(i == points.begin()) {
			r.x = i->x;
			r.y = i->y;
		}
		else {
			if(r.x > i->x) {
				r.w += r.x - i->x;
				r.x = i->x;
			} else if(r.x + r.w < i->x) {
				r.w = i->x - r.x;
			}
			if(r.y > i->y) {
				r.h += r.y - i->y;
				r.y = i->y;
			} else if(r.y + r.h < i->y) {
				r.h = i->y - r.y;
			}
		}
	}
	return r;
}

SDL_Rect Polygon2D::minOverlayRect(CViewport* v) const {
	int wx = v->GetWorldX();
	int wy = v->GetWorldY();
	int l = v->GetLeft();
	int t = v->GetTop();

#define Tx(x) ((x - wx) * 2 + l)
#define Ty(y) ((y - wy) * 2 + t)
	
	SDL_Rect r = {0,0,0,0};
	for(Points::const_iterator i = points.begin(); i != points.end(); ++i) {
		if(i == points.begin()) {
			r.x = Tx(i->x);
			r.y = Ty(i->y);
		}
		else {
			if(r.x > Tx(i->x)) {
				r.w += r.x - Tx(i->x);
				r.x = Tx(i->x);
			} else if(r.x + r.w < Tx(i->x)) {
				r.w = Tx(i->x) - r.x;
			}
			if(r.y > Ty(i->y)) {
				r.h += r.y - Ty(i->y);
				r.y = Ty(i->y);
			} else if(r.y + r.h < Ty(i->y)) {
				r.h = Ty(i->y) - r.y;
			}
		}
	}

#undef Tx
#undef Ty	
	return r;
}

void Polygon2D::drawFilled(SDL_Surface* bmpDest, int x, int y, Color col) {
	SDL_Rect r = minOverlayRect(); r.x += x; r.y += y;
	
	// Clipping (only y, x cannot be done because algo doesn't work otherwise)
	if(!OneSideClip(r.y, r.h, bmpDest->clip_rect.y, bmpDest->clip_rect.h)) return;
	if(r.x + r.w < bmpDest->clip_rect.x) return;
	if(r.x >= bmpDest->clip_rect.x + bmpDest->clip_rect.w) return;
	r.w = MIN(r.w, bmpDest->clip_rect.x + bmpDest->clip_rect.w - r.x);
	
	if(doReloadLines) reloadLines();
		
	const int bpp = bmpDest->format->BytesPerPixel;

	LOCK_OR_QUIT(bmpDest);
	
	// Draw the fill rect
	r.x -= x; r.y -= y;
	PixelPutAlpha& putter = getPixelAlphaPutFunc(bmpDest);
	for (int _y = r.y; _y < r.y + r.h; ++_y) {
		int _x = r.x;
		while(_x < r.x + r.w && getNext(_x, _y, _x, true)) {
			int endx = r.x + r.w;
			getNext(_x, _y, endx, false); endx = MIN(endx, r.x + r.w);
			Uint8 *px = (Uint8 *)bmpDest->pixels + (_y + y) * bmpDest->pitch + (_x + x) * bpp;
			for(; _x < endx; ++_x, px += bpp) {
				if(_x + x < bmpDest->clip_rect.x) continue;
				putter.put(px, bmpDest->format, col);
			}
		}
	}
	
	UnlockSurface(bmpDest);
}

void Polygon2D::drawFilled(SDL_Surface* bmpDest, int x, int y, CViewport* v, Color col) {
	SDL_Rect r = minOverlayRect(v); r.x += x*2; r.y += y*2; 
	
	// Clipping (only y, x cannot be done because algo doesn't work otherwise)
	if(!OneSideClip(r.y, r.h, bmpDest->clip_rect.y, bmpDest->clip_rect.h)) return;
	if(r.x + r.w < bmpDest->clip_rect.x) return;
	if(r.x >= bmpDest->clip_rect.x + bmpDest->clip_rect.w) return;
	r.w = MIN(r.w, bmpDest->clip_rect.x + bmpDest->clip_rect.w - r.x);
	
	const int bpp = bmpDest->format->BytesPerPixel;
	int wx = v->GetWorldX();
	int wy = v->GetWorldY();
	int l = v->GetLeft();
	int t = v->GetTop();
	
	// transform back
	r.x -= x*2; r.y -= y*2;
	r.x = (r.x - l) / 2 + wx;
	r.y = (r.y - t) / 2 + wy;
	r.w /= 2; r.h /= 2;
	
#define Tx(x) ((x - wx) * 2 + l)
#define Ty(y) ((y - wy) * 2 + t)

	if(doReloadLines) reloadLines();

	LOCK_OR_QUIT(bmpDest);
	
	// Draw the fill rect
	PixelPutAlpha& putter = getPixelAlphaPutFunc(bmpDest);
	for (int _y = r.y; _y < r.y + r.h; ++_y) {
		int _x = r.x;
		while(_x < r.x + r.w && getNext(_x, _y, _x, true)) {
			int endx = r.x + r.w;
			getNext(_x, _y, endx, false); endx = MIN(endx, r.x + r.w);
			Uint8 *px = (Uint8 *)bmpDest->pixels + Ty(_y + y) * bmpDest->pitch + Tx(_x + x) * bpp;
			for(; _x < endx; ++_x, px += bpp * 2) {
				if(Tx(_x + x) < bmpDest->clip_rect.x) continue;
				putter.put(px, bmpDest->format, col);
				putter.put(px + bpp, bmpDest->format, col);
				putter.put(px + bmpDest->pitch, bmpDest->format, col);
				putter.put(px + bmpDest->pitch + bpp, bmpDest->format, col);
			}
		}
	}
	
	UnlockSurface(bmpDest);
	
#undef Tx
#undef Ty
}


void TestPolygonDrawing(SDL_Surface* surf) {
	// star
	Polygon2D p;
	p.points.push_back( VectorD2<int>(100, 0) );
	p.points.push_back( VectorD2<int>(110, 90) );
	p.points.push_back( VectorD2<int>(200, 100) );
	p.points.push_back( VectorD2<int>(110, 110) );
	p.points.push_back( VectorD2<int>(100, 200) );
	p.points.push_back( VectorD2<int>(90, 110) );
	p.points.push_back( VectorD2<int>(0, 100) );
	p.points.push_back( VectorD2<int>(90, 90) );
	p.points.push_back( VectorD2<int>(100, 0) );	
	p.drawFilled(surf, 0, 0, Color(0,255,0,128));
	
	// triangle
	Polygon2D q;
	q.points.push_back( VectorD2<int>(310, 10) );
	q.points.push_back( VectorD2<int>(400, 20) );
	q.points.push_back( VectorD2<int>(390, 100) );
	q.points.push_back( VectorD2<int>(310, 10) );
	q.drawFilled(surf, 0, 0, Color(0,255,0,128));
	
	// rectangle
	Polygon2D r;
	r.points.push_back( VectorD2<int>(10, 240) );
	r.points.push_back( VectorD2<int>(200, 240) );
	r.points.push_back( VectorD2<int>(200, 400) );
	r.points.push_back( VectorD2<int>(10, 400) );
	r.points.push_back( VectorD2<int>(10, 240) );
	r.drawFilled(surf, 0, 0, Color(0,255,0,128));

	// quadrangle (like worms beam)
	Polygon2D s;
	s.points.push_back( VectorD2<int>(230, 240) );
	s.points.push_back( VectorD2<int>(250, 230) );
	s.points.push_back( VectorD2<int>(300, 380) );
	s.points.push_back( VectorD2<int>(260, 400) );
	s.points.push_back( VectorD2<int>(230, 240) );
	s.drawFilled(surf, 0, 0, Color(0,255,0,128));

	// rotating long triangle
	Polygon2D t;
	float ta = (GetTime() - AbsTime()).seconds();
	MatrixD2<float> rot = MatrixD2<float>::Rotation(cos(ta), sin(ta));
	notes << "M:" << rot.v1.x << "," << rot.v1.y << ";" << rot.v2.x << "," << rot.v2.y << endl;
	//rot = MatrixD2<float>(1.0f);
	VectorD2<int> t1(0, -20); t1 = rot * t1;
	VectorD2<int> t2(200, 0); t2 = rot * t2;
	VectorD2<int> t3(0, 20); t3 = rot * t3;
	notes << t1.x << "," << t1.y << ";" << t2.x << "," << t2.y << ";" << t3.x << "," << t3.y << endl;
	t.points.push_back(t1);
	t.points.push_back(t2);
	t.points.push_back(t3);
	t.points.push_back(t1);
	t.drawFilled(surf, 70, 400, Color(0,0,255,160));
	
}







////////////////////////
//
//  Image loading/saving routines
//
////////////////////////

///////////////////
// Loads an image, and converts it to the same colour depth as the screen (speed)
SmartPointer<SDL_Surface> LoadGameImage(const std::string& _filename, bool withalpha)
{
	{
		// Try cache first
		SmartPointer<SDL_Surface> ImageCache = cCache.GetImage(_filename);
		if( ImageCache.get() )
			return ImageCache;
	}
	
	// Load the image
	std::string fullfname = GetFullFileName(_filename);
	if(fullfname.size() == 0)
		return NULL;

	SmartPointer<SDL_Surface> img = IMG_Load(Utf8ToSystemNative(fullfname).c_str());

	if(!img.get())  {
		return NULL;
	}

	SmartPointer<SDL_Surface> Image;
	if(bDedicated || !VideoPostProcessor::videoSurface()) {
		if(!bDedicated)
			// we haven't initialized the screen yet
			warnings << "LoadGameImage: screen not initialized yet while loading image" << endl;	

		if (withalpha)
			Image = gfxCreateSurfaceAlpha(img.get()->w, img.get()->h);
		else
			Image = gfxCreateSurface(img.get()->w, img.get()->h);			
		CopySurface(Image.get(), img, 0, 0, 0, 0, img.get()->w, img.get()->h);
	}
	else {
		// Convert the image to the screen's colour depth
		if (withalpha)  {
			Image = gfxCreateSurfaceAlpha(img.get()->w, img.get()->h);
			CopySurface(Image.get(), img, 0, 0, 0, 0, img.get()->w, img.get()->h);
		} else {
			SDL_PixelFormat fmt = *(getMainPixelFormat());
			img.get()->flags &= ~SDL_SRCALPHA; // Remove the alpha flag here, ConvertSurface will remove the alpha completely later
			img.get()->flags &= ~SDL_SRCCOLORKEY; // Remove the colorkey here, we don't want it (normally it shouldn't be activated here, so only for safty)
			Image = SDL_ConvertSurface(img.get(), &fmt, iSurfaceFormat);
			Image.get()->flags &= ~SDL_SRCALPHA; // we explicitly said that we don't want alpha, so remove it
		}
	}

	if(!Image.get()) {
		errors << "LoadGameImage: cannot create new surface" << endl;
		return NULL;
	}

	// Save to cache
	#ifdef DEBUG
	//printf("LoadImage() %p %s\n", Image.get(), _filename.c_str() );
	#endif
	cCache.SaveImage(_filename, Image);
	return Image;
}

#ifndef DEDICATED_ONLY
///////////////////////
// Converts the SDL_surface to gdImagePtr
static gdImagePtr SDLSurface2GDImage(SDL_Surface * src) {
	gdImagePtr gd_image = gdImageCreateTrueColor(src->w,src->h);
	if(!gd_image)
		return NULL;

	Uint32 rmask, gmask, bmask;
	// format of gdImage
	rmask=0x00FF0000; gmask=0x0000FF00; bmask=0x000000FF;

	SmartPointer<SDL_Surface> formated = SDL_CreateRGBSurface(SDL_SWSURFACE, src->w, src->h, 32, rmask, gmask, bmask, 0);
	if(!formated.get())
		return NULL;
	#ifdef DEBUG
	//printf("SDLSurface2GDImage() %p\n", formated.get() );
	#endif
	// convert it to the new format (32 bpp)
	CopySurface(formated.get(), src, 0, 0, 0, 0, src->w, src->h);

	if (!LockSurface(formated))
		return NULL;

	for(int y = 0; y < src->h; y++) {
		memcpy(gd_image->tpixels[y], (uchar*)formated.get()->pixels + y*formated.get()->pitch, formated.get()->pitch);
	}

	UnlockSurface(formated);

	return gd_image;
}
#endif //DEDICATED_ONLY

///////////////////////
// Saves the surface into the specified file with the specified format
bool SaveSurface(SDL_Surface * image, const std::string& FileName, int Format, const std::string& Data)
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
	warnings << "SaveSurface: cannot use something else than BMP in dedicated-only-mode" << endl;
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
#endif // !DEDICATED_ONLY
}


void test_Clipper() {
	SDL_Rect r1 = {52, 120, 7, 14};
	SDL_Rect r2 = {52, 162, 558, 258};

	ClipRefRectWith(r1, (SDLRect&)r2);

	notes << r1.x << "," << r1.y << "," << r1.w << "," << r1.h << endl;
}

template <> void SmartPointer_ObjectDeinit<SDL_Surface> ( SDL_Surface * obj )
{
	#ifdef DEBUG
	//printf("SmartPointer_ObjectDeinit<SDL_Surface>() %p\n", obj);
	#endif

	SDL_FreeSurface(obj);
}

#ifdef DEBUG
SDL_mutex *SmartPointer_CollMutex = NULL;
std::map< void *, SDL_mutex * > * SmartPointer_CollisionDetector = NULL;
#endif
