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


// Functors for putting/getting and copying pixels
// Created 12/11/01
// By Jason Boettcher


#ifndef __PIXELFUNCTORS_H__
#define __PIXELFUNCTORS_H__

#include <cassert>
#include <SDL.h>
#include "Color.h"
#include "CodeAttributes.h"
#include "GfxPrimitives.h"

// Alpha blends two colors, both are considered semi-transparent, the comp_a value should be MIN(255, fg.a + bg.a)
#define BLEND_CHANN_ALPHA(chann, bg, fg, comp_a) (((comp_a - fg.a) * bg.chann + fg.a * fg.chann) / comp_a)

// Blends the pixel with the background pixel, the background pixel is considered opaque
#define BLEND_CHANN_SOLID(chann, bg, fg)  (((255 - fg.a) * bg.chann + fg.a * fg.chann) >> 8)

//
// Color packing and unpacking (mainly grabbed from SDL)
//

INLINE Color Unpack_8(Uint8 px, const SDL_PixelFormat *fmt)  { 
	return Color(
		fmt->palette->colors[px].r,
		fmt->palette->colors[px].g,
		fmt->palette->colors[px].b
		); 
}

// Unpacks the color, the surface contains alpha information
INLINE Color Unpack_alpha(Uint32 px, const SDL_PixelFormat *fmt)  {
	Uint8 v;
	v = (px & fmt->Rmask) >> fmt->Rshift;
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
INLINE Color Unpack_solid(Uint32 px, const SDL_PixelFormat *fmt)  {
	Uint8 v;
	v = (px & fmt->Rmask) >> fmt->Rshift;
	const Uint8 r = (v << fmt->Rloss) + (v >> (8 - (fmt->Rloss << 1)));
	v = (px & fmt->Gmask) >> fmt->Gshift;
	const Uint8 g = (v << fmt->Gloss) + (v >> (8 - (fmt->Gloss << 1)));
	v = (px & fmt->Bmask) >> fmt->Bshift;
	const Uint8 b = (v << fmt->Bloss) + (v >> (8 - (fmt->Bloss << 1)));
	return Color(r, g, b, SDL_ALPHA_OPAQUE);
}

// Unpack the color
INLINE Color Unpack(Uint32 px, const SDL_PixelFormat *fmt)  {
	if (fmt->Amask)
		return Unpack_alpha(px, fmt);
	else
		return Unpack_solid(px, fmt);
}

template<bool alpha>
INLINE Color Unpack_(Uint32 px, const SDL_PixelFormat *fmt)  {
	if (alpha)
		return Unpack_alpha(px, fmt);
	else
		return Unpack_solid(px, fmt);
}

INLINE Uint8 Pack_8(const Color& c, SDL_PixelFormat *fmt)  { return SDL_MapRGB(fmt, c.r, c.g, c.b); }

// Pack the color, for 16, 24 and 32bit modes
INLINE Uint32 Pack(const Color& c, const SDL_PixelFormat *fmt)	{
	// Grabbed from SDL_MapRGBA
    return (c.r >> fmt->Rloss) << fmt->Rshift
    | (c.g >> fmt->Gloss) << fmt->Gshift
    | (c.b >> fmt->Bloss) << fmt->Bshift
    | ((c.a >> fmt->Aloss) << fmt->Ashift & fmt->Amask);
}


//
// Basic functor prototypes
//

// Basic prototype of the put pixel functor
struct PixelPut  {
	virtual void put(Uint8 *addr, Uint32 color) = 0;
	virtual ~PixelPut() {}
};

// Basic prototype of the pixel copy functor (copying pixels between surfaces)
class PixelCopy  {
protected:
	SDL_PixelFormat *sfmt; // Source surface format
	SDL_PixelFormat *dfmt; // Dest surface format
	Color colorkey;
public:
	PixelCopy() : sfmt(NULL), dfmt(NULL) {}
	void setformats(SDL_PixelFormat *srcf, SDL_PixelFormat *dstf, Color key) { sfmt = srcf; dfmt = dstf; colorkey = key; }
	virtual void copy(Uint8 *dstaddr, const Uint8 *srcaddr) = 0;
	virtual ~PixelCopy() {}
};

// Basic prototype of the alpha-blended putpixel functor
struct PixelPutAlpha  {
	virtual void put(Uint8 *addr, const SDL_PixelFormat *dstfmt, const Color& col) = 0;
	virtual ~PixelPutAlpha() {}
};

// Basic prototype of the getpixel functor
struct PixelGet  {
	virtual Uint32	get(Uint8 *addr) = 0;
	virtual ~PixelGet() {}
};

template<int bitspp>
INLINE void PutPixel_(Uint8 *addr, Uint32 color) { assert(false); }

//
// Pixel put (ignoring alpha)
//

// 8-bit PutPixel, currently unused
template<>
INLINE void PutPixel_<8>(Uint8 *addr, Uint32 color)  { *addr = (Uint8)color; }
class PixelPut_8 : public PixelPut  {
	void put(Uint8 *addr, Uint32 color)  { PutPixel_<8>(addr, color); }
};

// 16-bit PutPixel
template<>
INLINE void PutPixel_<16>(Uint8 *addr, Uint32 color)  { *(Uint16 *)addr = (Uint16)color; }
class PixelPut_16 : public PixelPut {
	void put(Uint8 *addr, Uint32 color)	{ PutPixel_<16>(addr, color); }
};

// 24-bit PutPixel
template<>
INLINE void PutPixel_<24>(Uint8 *addr, Uint32 color)  {
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		addr[0] = ((Uint8 *)&color)[1];
		addr[1] = ((Uint8 *)&color)[2];
		addr[2] = ((Uint8 *)&color)[3];
#else
		addr[0] = ((Uint8 *)&color)[0];
		addr[1] = ((Uint8 *)&color)[1];
		addr[2] = ((Uint8 *)&color)[2];
#endif
}

class PixelPut_24 : public PixelPut {
	void put(Uint8 *addr, Uint32 color)	{ PutPixel_<24>(addr, color); }
};

// 32-bit PutPixel
template<>
INLINE void PutPixel_<32>(Uint8 *addr, Uint32 color) {
	*(Uint32 *)addr = color;
}

class PixelPut_32 : public PixelPut {
	void put(Uint8 *addr, Uint32 color)	{ PutPixel_<32>(addr, color); }
};


//
// Get pixel
//

template<int bitspp>
INLINE Uint32 GetPixel_(const Uint8* addr) { assert(false); return 0; }

// 8-bit getpixel, returns palette index, not the color; currently unused
template<>
INLINE Uint32 GetPixel_<8>(const Uint8 *addr)  { return *addr; }
class PixelGet_8 : public PixelGet {
	Uint32 get(Uint8 *addr)  { return GetPixel_<8>(addr); }
};

// 16-bit getpixel
template<>
INLINE Uint32 GetPixel_<16>(const Uint8 *addr)  { return *(Uint16 *)addr; }
class PixelGet_16 : public PixelGet {
	Uint32 get(Uint8 *addr)	{ return GetPixel_<16>(addr); }
};

// 24-bit getpixel
template<>
INLINE Uint32 GetPixel_<24>(const Uint8 *addr) {
		union { Uint8 u8[4]; Uint32 u32; } col;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		col.u8[0] = 255;
		col.u8[1] = addr[2];
		col.u8[2] = addr[1];
		col.u8[3] = addr[0];
#else
		col.u8[0] = addr[0];
		col.u8[1] = addr[1];
		col.u8[2] = addr[2];
		col.u8[3] = 255;
#endif
		return col.u32;
}

class PixelGet_24 : public PixelGet  {
	Uint32 get(Uint8 *addr) { return GetPixel_<24>(addr); }
};

// 32-bit getpixel
template<>
INLINE Uint32 GetPixel_<32>(const Uint8 *addr)  {
	return *(Uint32 *)addr;
}

class PixelGet_32 : public PixelGet {
	Uint32 get(Uint8 *addr)	{ return GetPixel_<32>(addr); }
};

//------


template<bool alpha, int bpp>
INLINE Color _GetPixel(SDL_PixelFormat* format, const Uint8* addr) {
	assert(bpp >= 1 && bpp <= 4);
	if(bpp == 1) return Unpack_8(GetPixel_<8>(addr), format);
	if(alpha)
		return Unpack_alpha(GetPixel_<bpp*8>(addr), format);
	else
		return Unpack_solid(GetPixel_<bpp*8>(addr), format);
	assert(false);
}

template<bool alpha, bool alphablend, int bpp>
INLINE void _PutPixel(SDL_PixelFormat* format, Uint8* addr, Color col) {
	assert(bpp >= 1 && bpp <= 4);
	Color dest_cl;
	if(alphablend) {
		if(alpha && !col.a) return; // Prevent div by zero error
		dest_cl = _GetPixel<alpha,bpp>(format, addr);
		if(alpha) {
			dest_cl.a = 255 - (((255 - col.a) * (255 - dest_cl.a)) >> 8); // Same as MIN(255, dest_cl.a + col.a) but faster (no condition)
			dest_cl.r = BLEND_CHANN_ALPHA(r, dest_cl, col, dest_cl.a);
			dest_cl.g = BLEND_CHANN_ALPHA(g, dest_cl, col, dest_cl.a);
			dest_cl.b = BLEND_CHANN_ALPHA(b, dest_cl, col, dest_cl.a);
		}
		else {
			dest_cl.r = BLEND_CHANN_SOLID(r, dest_cl, col);
			dest_cl.g = BLEND_CHANN_SOLID(g, dest_cl, col);
			dest_cl.b = BLEND_CHANN_SOLID(b, dest_cl, col);
		}
	}
	else { // no alpha-blending
		dest_cl = col;
	}
	
	if(bpp == 1)
		PutPixel_<8>(addr, Pack_8(dest_cl, format));
	else
		PutPixel_<bpp*8>(addr, Pack(dest_cl, format));
}



template<
bool issameformat,
bool alphablend,
bool colorkeycheck,
bool srchasalpha,
bool dsthasalpha,
int srcbytespp,
int dstbytespp
>
INLINE void PixelCopy_(SDL_PixelFormat* dstformat, SDL_PixelFormat* srcformat, Uint8 *dstaddr, const Uint8 *srcaddr, Color colorkey) {
	if(colorkeycheck) {
		if(colorkey == _GetPixel<srchasalpha,srcbytespp>(srcformat, srcaddr)) return;
	}
	if(issameformat && !alphablend) {
		assert(srchasalpha == dsthasalpha);
		assert(srcbytespp == dstbytespp);
		if(srcbytespp == 1) *dstaddr = *srcaddr;
		else if(srcbytespp == 2) *(Uint16 *)dstaddr = *(Uint16 *)srcaddr;
		else if(srcbytespp == 3) { dstaddr[0] = srcaddr[0]; dstaddr[1] = srcaddr[1]; dstaddr[2] = srcaddr[2]; }
		else if(srcbytespp == 4) *(Uint32 *)dstaddr = *(Uint32 *)srcaddr;
		else assert(false);
	}
	else {
		Color c = _GetPixel<srchasalpha,srcbytespp>(srcformat, srcaddr);
		_PutPixel<dsthasalpha,alphablend,dstbytespp>(dstformat, dstaddr, c);
	}
}

template<
bool issameformat,
bool alphablend,
bool colorkeycheck,
bool srchasalpha,
bool dsthasalpha,
int srcbytespp,
int dstbytespp
>
class PixelCopy_Class : public PixelCopy {
public:
	void copy(Uint8 *dstaddr, const Uint8 *srcaddr) {
		PixelCopy_<
		issameformat,alphablend,colorkeycheck,srchasalpha,dsthasalpha,srcbytespp,dstbytespp
		>(dfmt, sfmt, dstaddr, srcaddr, colorkey);
	}
	static PixelCopy& getInstance(SDL_PixelFormat* sfmt, SDL_PixelFormat* dfmt, Color colorkey) {
		static PixelCopy_Class copier;
		copier.setformats(sfmt, dfmt, colorkey);
		return copier;
	}
};



//
// Special pixel drawing (with alpha)
//


template<int bpp>
class PixelPutAlpha_SolidBg_ : public PixelPutAlpha  {
	void put(Uint8 *addr, const SDL_PixelFormat *dstfmt, const Color& col)  {
		Color dest_cl = Unpack_solid(GetPixel_<bpp>(addr), dstfmt);

		// Blend and save to dest_cl
		dest_cl.r = BLEND_CHANN_SOLID(r, dest_cl, col);
		dest_cl.g = BLEND_CHANN_SOLID(g, dest_cl, col);
		dest_cl.b = BLEND_CHANN_SOLID(b, dest_cl, col);
		PutPixel_<bpp>(addr, Pack(dest_cl, dstfmt));
	}
};


// RGBA -> RGBA 32bit pixel putter
// HINT: only this functor is necessary because RGBA surfaces are always 32bit
template<int bpp>
class PixelPutAlpha_AlphaBg_ : public PixelPutAlpha  {
	void put(Uint8 *addr, const SDL_PixelFormat *dstfmt, const Color& col)  {
		if (!col.a) // Prevent div by zero error
			return;

		Color dest_cl = Unpack_alpha(GetPixel_<bpp>(addr), dstfmt);

		// Blend and save to dest_cl
		dest_cl.a = 255 - (((255 - col.a) * (255 - dest_cl.a)) >> 8); // Same as MIN(255, dest_cl.a + col.a) but faster (no condition)
		dest_cl.r = BLEND_CHANN_ALPHA(r, dest_cl, col, dest_cl.a);
		dest_cl.g = BLEND_CHANN_ALPHA(g, dest_cl, col, dest_cl.a);
		dest_cl.b = BLEND_CHANN_ALPHA(b, dest_cl, col, dest_cl.a);
		PutPixel_<bpp>(addr, Pack(dest_cl, dstfmt));
	}
};

//////////////////////////
// Get the alpha pixel putter for the surface
INLINE PixelPutAlpha& getPixelAlphaPutFunc(const SDL_Surface *surf)  {
	static PixelPutAlpha_SolidBg_<16> px16;
	static PixelPutAlpha_SolidBg_<24> px24;
	static PixelPutAlpha_SolidBg_<32> px32;
	static PixelPutAlpha_AlphaBg_<32> px32_a;

	switch (surf->format->BytesPerPixel)  {
	case 1:
		assert(false); // No alpha blending for 8-bit surfaces atm
		break;
	case 2:
		return px16;  // 16-bit surfaces have no alpha
	case 3:
		return px24;	// 24-bit surfaces have no alpha
	case 4:
		if (Surface_HasBlendMode(surf))
			return px32_a;
		return px32;
	default:
		assert(false);
	}

	return px32; // Should not happen
}

//
// Functions for getting the appropriate functors
//

////////////////////////
// Get the "put pixel" functor for the given surface
INLINE PixelPut& getPixelPutFunc(const SDL_Surface *surf)  {
	static PixelPut_8 px8;
	static PixelPut_16 px16;
	static PixelPut_24 px24;
	static PixelPut_32 px32;
	static PixelPut* pxs[5] = { &px32, &px8, &px16, &px24, &px32 };

	assert( surf->format->BytesPerPixel >= 1 && surf->format->BytesPerPixel <= 4 );
	return *pxs[surf->format->BytesPerPixel];
}

////////////////////////
// Get the "get pixel" functor for the given surface
INLINE PixelGet& getPixelGetFunc(const SDL_Surface *surf)  {
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
		assert(false);
	}

	return px32; // Should not happen
}

/////////////////////
// Returns a pixel copy functor for the given surface
PixelCopy& getPixelCopyFunc(const SDL_Surface *source_surf, const SDL_Surface *dest_surf);

#endif
