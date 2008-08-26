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
#include "AuxLib.h"

int iSurfaceFormat = SDL_SWSURFACE;

using namespace std;

// Used in various alpha-blending routines, internal
struct RGBA  {
	Uint8 r, g, b, a;
};


/////////////////////////
//
// Pixel drawing routines
//
/////////////////////////

//
// Color packing and unpacking (mainly grabbed from SDL)
//

// TODO: put these to Color.h or somewhere there and use them instead of SDL_MapRGBA and GetRGBA (the functions below are faster)

Color Unpack_8(Uint8 px, const SDL_PixelFormat *fmt)  {
	return Color(fmt->palette->colors[px].r, fmt->palette->colors[px].g, fmt->palette->colors[px].b);
}

// Unpacks the color, the surface contains alpha information
Color Unpack_alpha(Uint32 px, const SDL_PixelFormat *fmt)  {
	unsigned int v = (px & fmt->Rmask) >> fmt->Rshift;
	const Uint8 r = (v << fmt->Rloss) + (v >> (8 - (fmt->Rloss << 1)));
	v = (px & fmt->Gmask) >> fmt->Gshift;
	const Uint8 g = (v << fmt->Gloss) + (v >> (8 - (fmt->Gloss << 1)));
	v = (px & fmt->Bmask) >> fmt->Bshift;
	const Uint8 b = (v << fmt->Bloss) + (v >> (8 - (fmt->Bloss << 1)));
	v = (px & fmt->Amask) >> fmt->Ashift;
	const Uint8 a = (v << fmt->Aloss) + (v >> (8 - (fmt->Aloss << 1)));
	return Color(r, g, b, a);
}

// Unpack the color, the surface has no alpha information
Color Unpack_solid(Uint32 px, const SDL_PixelFormat *fmt)  {
	unsigned int v = (px & fmt->Rmask) >> fmt->Rshift;
	const Uint8 r = (v << fmt->Rloss) + (v >> (8 - (fmt->Rloss << 1)));
	v = (px & fmt->Gmask) >> fmt->Gshift;
	const Uint8 g = (v << fmt->Gloss) + (v >> (8 - (fmt->Gloss << 1)));
	v = (px & fmt->Bmask) >> fmt->Bshift;
	const Uint8 b = (v << fmt->Bloss) + (v >> (8 - (fmt->Bloss << 1)));
	return Color(r, g, b, SDL_ALPHA_OPAQUE);
}

// Unpack the color
Color Unpack(Uint32 px, const SDL_PixelFormat *fmt)  {
	if (fmt->Amask)
		return Unpack_alpha(px, fmt);
	else
		return Unpack_solid(px, fmt);
}

Uint8 Pack_8(const Color& c, const SDL_PixelFormat *fmt)  { return SDL_MapRGB(fmt, c.r, c.g, c.b); }

// Pack the color, for 16, 24 and 32bit modes
Uint32 Pack(const Color& c, const SDL_PixelFormat *fmt)	{
	// Grabbed from SDL_MapRGBA
    return (c.r >> fmt->Rloss) << fmt->Rshift
    | (c.g >> fmt->Gloss) << fmt->Gshift
    | (c.b >> fmt->Bloss) << fmt->Bshift
    | ((c.a >> fmt->Aloss) << fmt->Ashift & fmt->Amask);
}


//
// Pixel copy (ignoring alpha)
//

// 8-bit PutPixel, currently unused
inline void CopyPixel_8(Uint8 *addr, Uint32 color)  { *addr = (Uint8)color; }
class PixelCopy_8 : public PixelCopy  {
	void put(Uint8 *addr, Uint32 color)  { CopyPixel_8(addr, color); }
};

// 16-bit PutPixel
inline void CopyPixel_16(Uint8 *addr, Uint32 color)  { *(Uint16 *)addr = (Uint16)color; }
class PixelCopy_16 : public PixelCopy {
	void put(Uint8 *addr, Uint32 color)	{ CopyPixel_16(addr, color); }
};

// 24-bit PutPixel
inline void CopyPixel_24(Uint8 *addr, Uint32 color)  {
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		addr[0] = ((Uint8 *)&color)[3];
		addr[1] = ((Uint8 *)&color)[2];
		addr[2] = ((Uint8 *)&color)[1];
#else
		addr[0] = ((Uint8 *)&color)[0];
		addr[1] = ((Uint8 *)&color)[1];
		addr[2] = ((Uint8 *)&color)[2];
#endif
}

class PixelCopy_24 : public PixelCopy {
	void put(Uint8 *addr, Uint32 color)	{ CopyPixel_24(addr, color); }
};

// 32-bit PutPixel
inline void CopyPixel_32(Uint8 *addr, Uint32 color)  { *(Uint32 *)addr = color; }
class PixelCopy_32 : public PixelCopy {
	void put(Uint8 *addr, Uint32 color)	{ CopyPixel_32(addr, color); }
};

//
// Get pixel
//

// 8-bit getpixel, returns palette index, not the color; currently unused
inline Uint32 GetPixel_8(Uint8 *addr)  { return *addr; }
class PixelGet_8 : public PixelGet {
	Uint32 get(Uint8 *addr)  { return GetPixel_8(addr); }
};

// 16-bit getpixel
inline Uint32 GetPixel_16(Uint8 *addr)  { return *(Uint16 *)addr; }
class PixelGet_16 : public PixelGet {
	Uint32 get(Uint8 *addr)	{ return GetPixel_16(addr); }
};

// 24-bit getpixel
inline Uint32 GetPixel_24(Uint8 *addr)  {
		char color[4];
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		color[0] = addr[0];
		color[1] = addr[1];
		color[2] = addr[2];
		color[3] = 0;
#else
		color[2] = addr[2];
		color[1] = addr[1];
		color[0] = addr[0];
		color[3] = 0;
#endif
		return *((Uint32 *)(&color[0]));
}

class PixelGet_24 : public PixelGet  {
	Uint32 get(Uint8 *addr) { return GetPixel_24(addr); }
};

// 32-bit getpixel
inline Uint32 GetPixel_32(Uint8 *addr)  { return *(Uint32 *)addr; }
class PixelGet_32 : public PixelGet {
	Uint32 get(Uint8 *addr)	{ return GetPixel_32(addr); }
};

////////////////////////
// Get the copy pixel functor for the given surface
PixelCopy& getPixelCopy(const SDL_Surface *surf)  {
	static PixelCopy_8 px8;
	static PixelCopy_16 px16;
	static PixelCopy_24 px24;
	static PixelCopy_32 px32;

	switch (surf->format->BytesPerPixel)  {
	case 1:
		return px8;
	case 2:
		return px16;
	case 3:
		return px24;
	case 4:
		return px32;
	default:
		DEBUGASSERT();
	}

	return px32; // Should not happen
}

////////////////////////
// Get the "get pixel" functor for the given surface
PixelGet& getPixelGetter(const SDL_Surface *surf)  {
	static PixelGet_8 px8;
	static PixelGet_16 px16;
	static PixelGet_24 px24;
	static PixelGet_32 px32;

	switch (surf->format->BytesPerPixel)  {
	case 1:
		return px8;
	case 2:
		return px16;
	case 3:
		return px24;
	case 4:
		return px32;
	default:
		DEBUGASSERT();
	}

	return px32; // Should not happen
}

//
// Special pixel drawing (with alpha)
//

// Blends the pixel with the background pixel, the background pixel is considered opaque
#define BLEND_CHANN_SOLID(chann, bg, fg)  (((255 - fg.a) * bg.chann + fg.a * fg.chann) / 255)

// 16-bit alpha putpixel
class PixelPutAlpha_SolidBg_16 : public PixelPutAlpha  {
	void put(Uint8 *addr, const SDL_PixelFormat *dstfmt, const Color& col)  {
		Color dest_cl = Unpack_solid(GetPixel_16(addr), dstfmt);

		// Blend and save to dest_cl
		dest_cl.r = BLEND_CHANN_SOLID(r, dest_cl, col);
		dest_cl.g = BLEND_CHANN_SOLID(g, dest_cl, col);
		dest_cl.b = BLEND_CHANN_SOLID(b, dest_cl, col);
		CopyPixel_16(addr, Pack(dest_cl, dstfmt));
	}
};

// 24-bit alpha putpixel
class PixelPutAlpha_SolidBg_24 : public PixelPutAlpha  {
	void put(Uint8 *addr, const SDL_PixelFormat *dstfmt, const Color& col)  {
		Color dest_cl = Unpack_solid(GetPixel_24(addr), dstfmt);

		// Blend and save to dest_cl
		dest_cl.r = BLEND_CHANN_SOLID(r, dest_cl, col);
		dest_cl.g = BLEND_CHANN_SOLID(g, dest_cl, col);
		dest_cl.b = BLEND_CHANN_SOLID(b, dest_cl, col);
		CopyPixel_24(addr, Pack(dest_cl, dstfmt));
	}
};

// 32-bit alpha putpixel
class PixelPutAlpha_SolidBg_32 : public PixelPutAlpha  {
	void put(Uint8 *addr, const SDL_PixelFormat *dstfmt, const Color& col)  {
		Color dest_cl = Unpack_solid(GetPixel_32(addr), dstfmt);

		// Blend and save to dest_cl
		dest_cl.r = BLEND_CHANN_SOLID(r, dest_cl, col);
		dest_cl.g = BLEND_CHANN_SOLID(g, dest_cl, col);
		dest_cl.b = BLEND_CHANN_SOLID(b, dest_cl, col);
		CopyPixel_32(addr, Pack(dest_cl, dstfmt));
	}
};

// Alpha blends two colors, both are considered semi-transparent, the comp_a value should be MIN(255, fg.a + bg.a)
#define BLEND_CHANN_ALPHA(chann, bg, fg, comp_a) (((comp_a - fg.a) * bg.chann + fg.a * fg.chann) / comp_a)

// RGBA -> RGBA 32bit pixel putter
// HINT: only this functor is necessary because RGBA surfaces are always 32bit
class PixelPutAlpha_AlphaBg_32 : public PixelPutAlpha  {
	void put(Uint8 *addr, const SDL_PixelFormat *dstfmt, const Color& col)  {
		if (!col.a) // Prevent div by zero error
			return;

		Color dest_cl = Unpack_alpha(GetPixel_32(addr), dstfmt);

		// Blend and save to dest_cl
		dest_cl.a = 255 - (255 - col.a) * (255 - dest_cl.a) / 255; // Same as MIN(255, dest_cl.a + col.a) but faster (no condition)
		dest_cl.r = BLEND_CHANN_ALPHA(r, dest_cl, col, dest_cl.a);
		dest_cl.g = BLEND_CHANN_ALPHA(g, dest_cl, col, dest_cl.a);
		dest_cl.b = BLEND_CHANN_ALPHA(b, dest_cl, col, dest_cl.a);
		CopyPixel_32(addr, Pack(dest_cl, dstfmt));
	}
};

//////////////////////////
// Get the alpha pixel putter for the surface
PixelPutAlpha& getAlphaPut(const SDL_Surface *surf)  {
	static PixelPutAlpha_SolidBg_16 px16;
	static PixelPutAlpha_SolidBg_24 px24;
	static PixelPutAlpha_SolidBg_32 px32;
	static PixelPutAlpha_AlphaBg_32 px32_a;

	switch (surf->format->BytesPerPixel)  {
	case 1:
		DEBUGASSERT(); // No alpha blending for 8-bit surfaces atm
		break;
	case 2:
		return px16;  // 16-bit surfaces have no alpha
	case 3:
		return px24;	// 24-bit surfaces have no alpha
	case 4:
		if (surf->flags & SDL_SRCALPHA)
			return px32_a;
		return px32;
	default:
		DEBUGASSERT();
	}

	return px32; // Should not happen
}

/////////////////////////
//
// Misc routines
//
//////////////////////////

/////////////////
// Put the pixel alpha blended with the background
// TODO: this function is now obsolete, remove it
void PutPixelA(SDL_Surface * bmpDest, int x, int y, Uint32 colour, Uint8 a)  {
	Uint8* px = (Uint8*)bmpDest->pixels + y * bmpDest->pitch + x * bmpDest->format->BytesPerPixel;

	PixelPutAlpha& putter = getAlphaPut(bmpDest);
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

	#ifdef DEBUG_SMARTPTR
	printf("gfxCreateSurface() %p %i %i\n", result.get(), width, height );
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

	#ifdef DEBUG_SMARTPTR
	printf("gfxCreateSurfaceAlpha() %p %i %i\n", result.get(), width, height );
	#endif

	return result;
}

//////////////////
// Resets the alpha-channel and the colorkey
void ResetAlpha(SDL_Surface * dst)
{
	SDL_SetColorKey(dst, 0, 0); // Remove the colorkey
	SDL_SetAlpha(dst, 0, 0); // Remove the persurface-alpha

	PixelCopy& putter = getPixelCopy(dst);
	PixelGet& getter = getPixelGetter(dst);

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
	rDest.w = rSrc.w;
	rDest.h = rSrc.h;
	if (!ClipRefRectWith(rDest, (SDLRect&)bmpDest->clip_rect))
		return;
	if (!ClipRefRectWith(rSrc, (SDLRect&)bmpSrc->clip_rect))
		return;
	rDest.w = MIN(rSrc.w, rDest.w);
	rDest.h = MIN(rSrc.h, rDest.h);

	LOCK_OR_QUIT(bmpDest);
	LOCK_OR_QUIT(bmpSrc);

	static const int sbpp = 4;
	const int dbpp = bmpDest->format->BytesPerPixel;
	Uint8 *src = ((Uint8 *)bmpSrc->pixels + rSrc.y * bmpSrc->pitch + rSrc.x * sbpp);
	Uint8 *dst = ((Uint8 *)bmpDest->pixels + rDest.y * bmpDest->pitch + rDest.x * dbpp);
	int srcgap = bmpSrc->pitch - rDest.w * sbpp;
	int dstgap = bmpDest->pitch - rDest.w * dbpp;

	PixelPutAlpha& putter = getAlphaPut(bmpDest);

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

	// Warning: Doesn't do clipping on the source surface

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
	// TODO: recode this; avoid this amount of variables, only use ~5 local variables in a function!

	assert(bmpDest->format->BytesPerPixel == bmpSrc->format->BytesPerPixel);

	int x,y;
	int dw = w * 2;
	int dh = h * 2;

	// Lock the surfaces
	LOCK_OR_QUIT(bmpDest);
	LOCK_OR_QUIT(bmpSrc);

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
	UnlockSurface(bmpDest);
	UnlockSurface(bmpSrc);
}


///////////////////
// Draws a sprite doubly stretched with colour key
// HINT: doesn't work with alpha-surfaces
void DrawImageStretch2Key(SDL_Surface * bmpDest, SDL_Surface * bmpSrc, int sx, int sy, int dx, int dy, int w, int h)
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
	LOCK_OR_QUIT(bmpDest);
	LOCK_OR_QUIT(bmpSrc);

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
	UnlockSurface(bmpDest);
	UnlockSurface(bmpSrc);
}


///////////////////
// Draws a sprite mirrored doubly stretched with colour key
void DrawImageStretchMirrorKey(SDL_Surface *bmpDest, SDL_Surface * bmpSrc, int sx, int sy, int dx, int dy, int w, int h)
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
	LOCK_OR_QUIT(bmpDest);
	LOCK_OR_QUIT(bmpSrc);


	Uint8 *TrgPix = (Uint8 *)bmpDest->pixels + dy*bmpDest->pitch + dx*bmpDest->format->BytesPerPixel;
	Uint8 *SrcPix = (Uint8 *)bmpSrc->pixels + sy*bmpSrc->pitch + sx*bmpSrc->format->BytesPerPixel;

	Uint8 *sp,*tp_x,*tp_y;

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
	int bpp = (byte)bmpDest->format->BytesPerPixel;

	// Resize
	int dest_y = 0;
	for (float src_y = (float)sy; dest_y < dh; dest_y++)  {
		Uint8 *src_pxrow = (Uint8 *)bmpSrc->pixels + (int)(src_y) * bmpSrc->pitch;
		dst_px = dst_pxrow;

		int dest_x = 0;

		// Copy the row
		for (float src_x = (float)sx; dest_x < dw; dest_x++)  {
			Uint8 *src_px = src_pxrow + (int)(src_x) * bpp;
			memcpy(dst_px, src_px, bpp);
			src_x += xstep;
			dst_px += bpp;
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
	SDL_Rect src = { sx, sy, sw, sh };
	SDL_Rect dst = { dx, dy, (int)((float)sw * xratio), (int)((float)sh * yratio) };
	if (dst.w != 0 && dst.h != 0)
		SDL_SoftStretch(bmpSrc, &src, bmpDest, &dst);
}

////////////////////////
// Draws the image nicely resampled
void DrawImageResampledAdv(SDL_Surface * bmpDest, SDL_Surface * bmpSrc, int sx, int sy, int dx, int dy, int sw, int sh, int dw, int dh)
{
	SDL_Rect src = { sx, sy, sw, sh };
	SDL_Rect dst = { dx, dy, dw, dh };
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

static void Scale2xPixel(SDL_Surface *bmpDest, SDL_Surface *bmpSrc, int dx, int dy, Uint32 *colors)
{
	// Get the four colors to put on the dest surface
	Uint32 dstE[4];
	if (colors[B] != colors[H] && colors[D] != colors[F]) {
		dstE[0] = colors[D] == colors[B] ? colors[D] : colors[E];
		dstE[1] = colors[B] == colors[F] ? colors[F] : colors[E];
		dstE[2] = colors[D] == colors[H] ? colors[D] : colors[E];
		dstE[3] = colors[H] == colors[F] ? colors[F] : colors[E];
	} else {
		dstE[0] = dstE[1] = dstE[2] = dstE[3] = colors[E];
	}

	// Put the 2x2 square on the dest surface
	int bpp = bmpDest->format->BytesPerPixel;

	Uint8 *pixel = (Uint8 *)bmpDest->pixels + dy * bmpDest->pitch + dx * bpp;
	PutPixelToAddr(pixel, dstE[0], bpp);
	PutPixelToAddr(pixel + bpp, dstE[1], bpp);

	pixel += bmpDest->pitch;
	PutPixelToAddr(pixel, dstE[2], bpp);
	PutPixelToAddr(pixel + bpp, dstE[3], bpp);
}

static void Scale2xPixel_FF(SDL_Surface *bmpDest, SDL_Surface *bmpSrc, int sx, int sy, int dx, int dy)
{
	Uint32 colors[5];
	colors[B] = colors[D] = colors[E] = GetPixel(bmpSrc, sx, sy);;
	colors[F] = GetPixel(bmpSrc, sx + 1, sy);
	colors[H] = GetPixel(bmpSrc, sx, sy + 1);
	Scale2xPixel(bmpDest, bmpSrc, dx, dy, colors);
}

static void Scale2xPixel_LF(SDL_Surface *bmpDest, SDL_Surface *bmpSrc, int sx, int sy, int dx, int dy)
{
	Uint32 colors[5];
	colors[B] = colors[E] = colors[F] = GetPixel(bmpSrc, sx, sy);
	colors[D] = GetPixel(bmpSrc, sx - 1, sy);
	colors[H] = GetPixel(bmpSrc, sx, sy + 1);
	Scale2xPixel(bmpDest, bmpSrc, dx, dy, colors);
}

static void Scale2xPixel_FL(SDL_Surface *bmpDest, SDL_Surface *bmpSrc, int sx, int sy, int dx, int dy)
{
	Uint32 colors[5];
	colors[D] = colors[E] = colors[H] = GetPixel(bmpSrc, sx, sy);
	colors[B] = GetPixel(bmpSrc, sx, sy - 1);
	colors[F] = GetPixel(bmpSrc, sx + 1, sy);
	Scale2xPixel(bmpDest, bmpSrc, dx, dy, colors);
}

static void Scale2xPixel_LL(SDL_Surface *bmpDest, SDL_Surface *bmpSrc, int sx, int sy, int dx, int dy)
{
	Uint32 colors[5];
	colors[E] = colors[F] = colors[H] = GetPixel(bmpSrc, sx, sy);
	colors[B] = GetPixel(bmpSrc, sx, sy - 1);
	colors[D] = GetPixel(bmpSrc, sx - 1, sy);
	Scale2xPixel(bmpDest, bmpSrc, dx, dy, colors);
}

static void Scale2xPixel_F(SDL_Surface *bmpDest, SDL_Surface *bmpSrc, int sx, int sy, int dx, int dy)
{
	Uint32 colors[5];
	colors[D] = colors[E] = GetPixel(bmpSrc, sx, sy);
	colors[B] = GetPixel(bmpSrc, sx, sy - 1);
	colors[F] = GetPixel(bmpSrc, sx + 1, sy);
	colors[H] = GetPixel(bmpSrc, sx, sy + 1);
	Scale2xPixel(bmpDest, bmpSrc, dx, dy, colors);
}

static void Scale2xPixel_L(SDL_Surface *bmpDest, SDL_Surface *bmpSrc, int sx, int sy, int dx, int dy)
{
	Uint32 colors[5];
	colors[E] = colors[F] = GetPixel(bmpSrc, sx, sy);
	colors[B] = GetPixel(bmpSrc, sx, sy - 1);
	colors[D] = GetPixel(bmpSrc, sx - 1, sy);
	colors[H] = GetPixel(bmpSrc, sx, sy + 1);
	Scale2xPixel(bmpDest, bmpSrc, dx, dy, colors);
}

/////////////////////////
// Draws the image double-sized using the scale2x algorithm
// This algo is taken from http://scale2x.sourceforge.net/algorithm.html
// Thanks go to the AdvanceMAME team!
void DrawImageScale2x(SDL_Surface* bmpDest, SDL_Surface* bmpSrc, int sx, int sy, int dx, int dy, int w, int h)
{
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
	Scale2xPixel_FF(bmpDest, bmpSrc, sx, sy, dx, dy);

	// Last pixel, first line
	Scale2xPixel_LF(bmpDest, bmpSrc, sx2, sy, dx + w * 2 - 2, dy);

	// First line
	for(int x = 1; x < w - 1; ++x)  {
		colors[B] = colors[E] = GetPixel(bmpSrc, sx + x, sy);
		colors[D] = GetPixel(bmpSrc, sx + x - 1, sy);
		colors[F] = GetPixel(bmpSrc, sx + x + 1, sy);
		colors[H] = GetPixel(bmpSrc, sx + x, sy + 1);

		Scale2xPixel(bmpDest, bmpSrc, dx + x * 2, dy, colors);
	}

	// First pixel, last line
	Scale2xPixel_FL(bmpDest, bmpSrc, sx, sy2, dx, dy + h * 2 - 2);

	// Last pixel, last line
	Scale2xPixel_LL(bmpDest, bmpSrc, sx2, sy2, dx + w * 2 - 2, dy + h * 2 - 2);


	// Last line
	for(int x = 1; x < w - 1; ++x)  {
		colors[E] = colors[H] = GetPixel(bmpSrc, sx + x, sy2);
		colors[B] = GetPixel(bmpSrc, sx + x, sy2 - 1);
		colors[D] = GetPixel(bmpSrc, sx + x - 1, sy2);
		colors[F] = GetPixel(bmpSrc, sx + x + 1, sy2);

		Scale2xPixel(bmpDest, bmpSrc, dx + x * 2, dy + h * 2 - 2, colors);
	}

	// Rest of the image
	for(int y = 1; y < h - 1; ++y) {
		// First & last pixel
		Scale2xPixel_F(bmpDest, bmpSrc, sx, sy + y, dx, dy + y * 2);
		Scale2xPixel_L(bmpDest, bmpSrc, sx2, sy + y, dx + w * 2 - 2, dy + y * 2);

		// Rest of the line
		for(int x = 1; x < w - 1; ++x) {
			colors[B] = GetPixel(bmpSrc, sx + x		, sy + y - 1);
			colors[D] = GetPixel(bmpSrc, sx + x - 1	, sy + y);
			colors[E] = GetPixel(bmpSrc, sx + x		, sy + y);
			colors[F] = GetPixel(bmpSrc, sx + x + 1	, sy + y);
			colors[H] = GetPixel(bmpSrc, sx + x		, sy + y + 1);

			Scale2xPixel(bmpDest, bmpSrc, dx + x * 2, dy + y * 2, colors);
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
	BeamPutPixelA(bmpDest, x, y, colour, 255); }


int laseralt = 0;

///////////////////
// Put a laser-sight pixel on the surface
void LaserSightPutPixel(SDL_Surface * bmpDest, int x, int y, Uint32 colour)
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
inline void perform_line(SDL_Surface * bmp, int x1, int y1, int x2, int y2, int d, void (*proc)(SDL_Surface * , int, int, Uint32))
{
   int dx = x2-x1;
   int dy = y2-y1;
   int i1, i2;
   int x, y;
   int dd;

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

   UnlockSurface(bmp);
}


inline void secure_perform_line(SDL_Surface * bmpDest, int x1, int y1, int x2, int y2, Color color, void (*proc)(SDL_Surface *, int, int, Uint32)) {
	if (!ClipLine(bmpDest, &x1, &y1, &x2, &y2)) // Clipping
		return;

	perform_line(bmpDest, x1, y1, x2, y2, color.get(bmpDest), proc);
}

////////////////////////
// Draw horizontal line
void DrawHLine(SDL_Surface * bmpDest, int x, int x2, int y, Color colour) {

	if (bmpDest->flags & SDL_HWSURFACE)  {
		DrawRectFill(bmpDest, x, y, x2, y + 1, colour); // In hardware mode this is much faster, in software it is slower
		return;
	}

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

	LOCK_OR_QUIT(bmpDest);
	byte bpp = (byte)bmpDest->format->BytesPerPixel;
	Uint8 *px2 = (uchar *)bmpDest->pixels+bmpDest->pitch*y+bpp*x2;

	// Draw depending on the alpha
	switch (colour.a)  {
	case SDL_ALPHA_OPAQUE:
	{
		// Solid (no alpha) drawing
		PixelCopy& putter = getPixelCopy(bmpDest);
		Uint32 packed_cl = Pack(colour, bmpDest->format);
		for (Uint8* px = (Uint8*)bmpDest->pixels + bmpDest->pitch * y + bpp * x; px <= px2; px += bpp)
			putter.put(px, packed_cl);
	} break;
	case SDL_ALPHA_TRANSPARENT:
	break;
	default:
	{
		// Draw the line alpha-blended with the background
		PixelPutAlpha& putter = getAlphaPut(bmpDest);
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

	LOCK_OR_QUIT(bmpDest);
	ushort pitch = (ushort)bmpDest->pitch;
	byte bpp = (byte)bmpDest->format->BytesPerPixel;
	Uint8 *px2 = (Uint8 *)bmpDest->pixels+pitch*y2+bpp*x;

	// Draw depending on the alpha
	switch (colour.a)  {
	case SDL_ALPHA_OPAQUE:
	{
		// Solid (no alpha) drawing
		PixelCopy& putter = getPixelCopy(bmpDest);
		Uint32 packed_cl = Pack(colour, bmpDest->format);
		for (Uint8 *px= (Uint8 *)bmpDest->pixels+pitch*y + bpp*x; px <= px2; px+=pitch)
			putter.put(px, packed_cl);
	} break;
	case SDL_ALPHA_TRANSPARENT:
	break;
	default:
	{
		// Draw the line alpha-blended with the background
		PixelPutAlpha& putter = getAlphaPut(bmpDest);
		for (Uint8 *px= (Uint8 *)bmpDest->pixels+pitch*y + bpp*x; px <= px2; px+=pitch)
			putter.put(px, bmpDest->format, colour);
	}
	}

	UnlockSurface(bmpDest);
}

///////////////////
// Line drawing
void DrawLine(SDL_Surface * dst, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Color color) {
	secure_perform_line(dst, x1, y1, x2, y2, color.get(dst), PutPixel);
}

//////////////////
// Fast routine for drawing 2x2 filled rects
void DrawRectFill2x2_NoClip(SDL_Surface *bmpDest, int x, int y, Color color)
{
	LOCK_OR_QUIT(bmpDest);

	Uint8 *row1 = (Uint8 *)bmpDest->pixels + y * bmpDest->pitch + x * bmpDest->format->BytesPerPixel;
	Uint8 *row2 = row1 + bmpDest->pitch;

	const Uint32 packed_color = Pack(color, bmpDest->format);

	PixelCopy& putter = getPixelCopy(bmpDest);
	putter.put(row1, packed_color);
	putter.put(row1 + bmpDest->format->BytesPerPixel, packed_color);
	putter.put(row2, packed_color);
	putter.put(row2 + bmpDest->format->BytesPerPixel, packed_color);

	UnlockSurface(bmpDest);
}

///////////////////////
// Draws the 2x2 filled rectangle, does clipping
void DrawRectFill2x2(SDL_Surface *bmpDest, int x, int y, Color color)
{
	if (x < 0 || x + 2 >= bmpDest->clip_rect.x + bmpDest->clip_rect.w)
		return;
	if (y < 0 || y + 2 >= bmpDest->clip_rect.y + bmpDest->clip_rect.h)
		return;

	DrawRectFill2x2_NoClip(bmpDest, x, y, color);
}

//////////////////////
// Draw antialiased line with an putpixel callback
// Code basis taken from CTGraphics by darkoman (http://www.codeproject.com/gdi/CTGraphics.asp)
void AntiAliasedLine(SDL_Surface * dst, int x1, int y1, int x2, int y2, Color color, void (*proc)(SDL_Surface *, int, int, Uint32, Uint8))
{
	// Calculate line params
	int dx = (x2 - x1);
	int dy = (y2 - y1);
	int k;
	int distance;

	LOCK_OR_QUIT(dst);

	Uint32 packed_c = color.get(dst);

	// Set start pixel
	proc(dst, x1, y1, packed_c, 255);

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
			proc(dst, xs, yt / 256, packed_c, (Uint8) (255 - distance));
			proc(dst, xs, yt / 256 + 1, packed_c, (Uint8) distance);

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
			proc(dst, xt / 256, ys, packed_c, (Uint8) (255 - distance));
			proc(dst, xt / 256 + 1, ys, packed_c, (Uint8) distance);

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
	PixelPutAlpha& putter = getAlphaPut(bmpDest);
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
		SDL_FillRect(bmpDest,&r,color.get(bmpDest));
	break;
	case SDL_ALPHA_TRANSPARENT:
	break;
	default:
		DrawRectFill_Overlay(bmpDest, r, color);
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

	if (tLXOptions->bAntiAliasing)
		AntiAliasedLine(bmp, x1, y1, x2, y2, color, RopePutPixelA);
	else
		perform_line(bmp, x1, y1, x2, y2, color.get(bmp), RopePutPixel);
}


///////////////////
// Draws a beam
void DrawBeam(SDL_Surface * bmp, int x1, int y1, int x2, int y2, Color color)
{
	// Clipping
	if (!ClipLine(bmp, &x1, &y1, &x2, &y2))
		return;

	if (tLXOptions->bAntiAliasing)
		AntiAliasedLine(bmp, x1, y1, x2, y2, color, BeamPutPixelA);
	else
		perform_line(bmp, x1, y1, x2, y2, color.get(bmp), BeamPutPixel);
}


///////////////////
// Draws a laser sight
void DrawLaserSight(SDL_Surface * bmp, int x1, int y1, int x2, int y2, Color color)
{
	// Clipping
	if (!ClipLine(bmp, &x1, &y1, &x2, &y2))
		return;

	perform_line(bmp, x1, y1, x2, y2, color.get(bmp), LaserSightPutPixel);
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
	// Try cache first
	SmartPointer<SDL_Surface> ImageCache = cCache.GetImage(_filename);
	if( ImageCache.get() )
		return ImageCache;

	SmartPointer<SDL_Surface> Image;
	// Load the image
	std::string fullfname = GetFullFileName(_filename);
	if(fullfname.size() == 0)
		return NULL;

	SmartPointer<SDL_Surface> img = IMG_Load(Utf8ToSystemNative(fullfname).c_str());

	if(!img.get())  {
		return NULL;
	}

	if(VideoPostProcessor::videoSurface()) {
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

	} else {
		// we haven't initialized the screen yet
		if(!bDedicated)
			printf("WARNING: screen not initialized yet while loading image\n");
		Image = img;
	}

	if(!Image.get()) {
		printf("ERROR: LoadImgBPP: cannot create new surface\n");
		return NULL;
	}

	// Save to cache
	#ifdef DEBUG_SMARTPTR
	printf("LoadImage() %p %s\n", Image.get(), _filename.c_str() );
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
	#ifdef DEBUG_SMARTPTR
	printf("SDLSurface2GDImage() %p\n", formated.get() );
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
	printf("WARNING: SaveSurface: cannot use something else than BMP in dedicated-only-mode\n");
	return false;
	#endif //DEDICATED_ONLY

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
}


void test_Clipper() {
	SDL_Rect r1 = {52, 120, 7, 14};
	SDL_Rect r2 = {52, 162, 558, 258};

	ClipRefRectWith(r1, (SDLRect&)r2);

	cout << r1.x << "," << r1.y << "," << r1.w << "," << r1.h << endl;
}

template <> void SmartPointer_ObjectDeinit<SDL_Surface> ( SDL_Surface * obj )
{
	#ifdef DEBUG_SMARTPTR
	printf("SmartPointer_ObjectDeinit<SDL_Surface>() %p\n", obj);
	#endif

	SDL_FreeSurface(obj);
};

#ifdef DEBUG_SMARTPTR
std::map< void *, SDL_mutex * > SmartPointer_CollisionDetector;
#endif
