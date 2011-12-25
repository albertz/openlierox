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

//
// Color packing and unpacking (mainly grabbed from SDL)
//

INLINE Color Unpack_8(Uint8 px, const SDL_PixelFormat *fmt)  { 
	return Color(fmt->palette->colors[px].r, fmt->palette->colors[px].g, fmt->palette->colors[px].b); 
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
public:
	PixelCopy() : sfmt(NULL), dfmt(NULL) {}
	void setformats(SDL_PixelFormat *srcf, SDL_PixelFormat *dstf) { sfmt = srcf; dfmt = dstf; }
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


//
// Pixel put (ignoring alpha)
//

// 8-bit PutPixel, currently unused
INLINE void PutPixel_8(Uint8 *addr, Uint32 color)  { *addr = (Uint8)color; }
class PixelPut_8 : public PixelPut  {
	void put(Uint8 *addr, Uint32 color)  { PutPixel_8(addr, color); }
};

// 16-bit PutPixel
INLINE void PutPixel_16(Uint8 *addr, Uint32 color)  { *(Uint16 *)addr = (Uint16)color; }
class PixelPut_16 : public PixelPut {
	void put(Uint8 *addr, Uint32 color)	{ PutPixel_16(addr, color); }
};

// 24-bit PutPixel
INLINE void PutPixel_24(Uint8 *addr, Uint32 color)  {
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
	void put(Uint8 *addr, Uint32 color)	{ PutPixel_24(addr, color); }
};

// 32-bit PutPixel
INLINE void PutPixel_32(Uint8 *addr, Uint32 color) {
	*(Uint32 *)addr = color;
}

class PixelPut_32 : public PixelPut {
	void put(Uint8 *addr, Uint32 color)	{ PutPixel_32(addr, color); }
};


//
// Get pixel
//

// 8-bit getpixel, returns palette index, not the color; currently unused
INLINE Uint32 GetPixel_8(const Uint8 *addr)  { return *addr; }
class PixelGet_8 : public PixelGet {
	Uint32 get(Uint8 *addr)  { return GetPixel_8(addr); }
};

// 16-bit getpixel
INLINE Uint32 GetPixel_16(const Uint8 *addr)  { return *(Uint16 *)addr; }
class PixelGet_16 : public PixelGet {
	Uint32 get(Uint8 *addr)	{ return GetPixel_16(addr); }
};

// 24-bit getpixel
INLINE Uint32 GetPixel_24(const Uint8 *addr) {
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
	Uint32 get(Uint8 *addr) { return GetPixel_24(addr); }
};

// 32-bit getpixel
INLINE Uint32 GetPixel_32(const Uint8 *addr)  {
	return *(Uint32 *)addr;
}

class PixelGet_32 : public PixelGet {
	Uint32 get(Uint8 *addr)	{ return GetPixel_32(addr); }
};

class PixelGet_32_nonalpha : public PixelGet {
	Uint32 get(Uint8 *addr)	{
		return Uint32(0xff000000) | GetPixel_32(addr);
	}
};


//
// Copy pixel (from one surface to another)
//
class PixelCopy_8_8 : public PixelCopy  { public:
	void copy(Uint8 *dstaddr, const Uint8 *srcaddr)  { *dstaddr = *srcaddr; }
};

class PixelCopy_16_16 : public PixelCopy  { public:
	void copy(Uint8 *dstaddr, const Uint8 *srcaddr)  { *(Uint16 *)dstaddr = *(Uint16 *)srcaddr; }
};

class PixelCopy_24_24 : public PixelCopy  { public:
	void copy(Uint8 *dstaddr, const Uint8 *srcaddr)  { dstaddr[0] = srcaddr[0]; dstaddr[1] = srcaddr[1]; dstaddr[2] = srcaddr[2]; }
};

class PixelCopy_32_32 : public PixelCopy  { public:
	void copy(Uint8 *dstaddr, const Uint8 *srcaddr)  { *(Uint32 *)dstaddr = *(Uint32 *)srcaddr; }
};

class PixelCopy_32na_32 : public PixelCopy  { public:
	void copy(Uint8 *dstaddr, const Uint8 *srcaddr)  { *(Uint32 *)dstaddr = Uint32(0xff000000) | *(Uint32 *)srcaddr; }
};

class PixelCopy_32key_32 : public PixelCopy  { public:
	void copy(Uint8 *dstaddr, const Uint8 *srcaddr)  {
		if(*(Uint32 *)srcaddr != sfmt->colorkey)
			*(Uint32 *)dstaddr = Uint32(0xff000000) | *(Uint32 *)srcaddr;
	}
};


class PixelCopy_8_32 : public PixelCopy  { public:
	void copy(Uint8 *dstaddr, const Uint8 *srcaddr)  { 
		*(Uint32 *)dstaddr = Pack(Color(sfmt->palette->colors[*srcaddr]), dfmt);
	}
};

class PixelCopy_8_24 : public PixelCopy  { public:
	void copy(Uint8 *dstaddr, const Uint8 *srcaddr)  { 
		PutPixel_24(dstaddr, Pack(Color(sfmt->palette->colors[*srcaddr]), dfmt));
	}
};

class PixelCopy_8_16 : public PixelCopy  { public:
	void copy(Uint8 *dstaddr, const Uint8 *srcaddr)  { 
		*(Uint16 *)dstaddr = (Uint16)Pack(Color(sfmt->palette->colors[*srcaddr]), dfmt);
	}
};

class PixelCopy_16_32 : public PixelCopy  { public:
	void copy(Uint8 *dstaddr, const Uint8 *srcaddr)  { 
		*(Uint32 *)dstaddr = Pack(Unpack_solid(*(Uint16 *)srcaddr, sfmt), dfmt);
	}
};

class PixelCopy_16_24 : public PixelCopy  { public:
	void copy(Uint8 *dstaddr, const Uint8 *srcaddr)  { 
		PutPixel_24(dstaddr, Pack(Unpack_solid(*(Uint16 *)srcaddr, sfmt), dfmt));
	}
};

class PixelCopy_16_8 : public PixelCopy  { public:
	void copy(Uint8 *dstaddr, const Uint8 *srcaddr)  {
		Color c = Unpack_solid(*(Uint16 *)srcaddr, sfmt);
		*dstaddr = (Uint8)SDL_MapRGB(dfmt, c.r, c.g, c.b);
	}
};

class PixelCopy_24_32 : public PixelCopy  { public:
	void copy(Uint8 *dstaddr, const Uint8 *srcaddr)  { 
		*(Uint32 *)dstaddr = Pack(Unpack_solid(GetPixel_24(srcaddr), sfmt), dfmt);
	}
};

class PixelCopy_24_16 : public PixelCopy  { public:
	void copy(Uint8 *dstaddr, const Uint8 *srcaddr)  { 
		*(Uint16 *)dstaddr = Pack(Unpack_solid(GetPixel_24(srcaddr), sfmt), dfmt);
	}
};

class PixelCopy_24_8 : public PixelCopy  { public:
	void copy(Uint8 *dstaddr, const Uint8 *srcaddr)  {
		Color c = Unpack_solid(GetPixel_24(srcaddr), sfmt);
		*dstaddr = (Uint8)SDL_MapRGB(dfmt, c.r, c.g, c.b);
	}
};

class PixelCopy_32_8 : public PixelCopy  { public:
	void copy(Uint8 *dstaddr, const Uint8 *srcaddr)  {
		Color c = Unpack_solid(*(Uint32 *)srcaddr, sfmt);
		*dstaddr = (Uint8)SDL_MapRGB(dfmt, c.r, c.g, c.b);
	}
};

class PixelCopy_32_16 : public PixelCopy  { public:
	void copy(Uint8 *dstaddr, const Uint8 *srcaddr)  { 
		*(Uint16 *)dstaddr = Pack(Unpack_solid(*(Uint32 *)srcaddr, sfmt), dfmt);
	}
};

class PixelCopy_32_24 : public PixelCopy  { public:
	void copy(Uint8 *dstaddr, const Uint8 *srcaddr)  { 
		PutPixel_24(dstaddr, Pack(Unpack_solid(*(Uint32 *)srcaddr, sfmt), dfmt));
	}
};


//
// Special pixel drawing (with alpha)
//

// Blends the pixel with the background pixel, the background pixel is considered opaque
#define BLEND_CHANN_SOLID(chann, bg, fg)  (((255 - fg.a) * bg.chann + fg.a * fg.chann) >> 8)

// 16-bit alpha putpixel
class PixelPutAlpha_SolidBg_16 : public PixelPutAlpha  {
	void put(Uint8 *addr, const SDL_PixelFormat *dstfmt, const Color& col)  {
		Color dest_cl = Unpack_solid(GetPixel_16(addr), dstfmt);

		// Blend and save to dest_cl
		dest_cl.r = BLEND_CHANN_SOLID(r, dest_cl, col);
		dest_cl.g = BLEND_CHANN_SOLID(g, dest_cl, col);
		dest_cl.b = BLEND_CHANN_SOLID(b, dest_cl, col);
		PutPixel_16(addr, Pack(dest_cl, dstfmt));
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
		PutPixel_24(addr, Pack(dest_cl, dstfmt));
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
		PutPixel_32(addr, Pack(dest_cl, dstfmt));
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
		dest_cl.a = 255 - (((255 - col.a) * (255 - dest_cl.a)) >> 8); // Same as MIN(255, dest_cl.a + col.a) but faster (no condition)
		dest_cl.r = BLEND_CHANN_ALPHA(r, dest_cl, col, dest_cl.a);
		dest_cl.g = BLEND_CHANN_ALPHA(g, dest_cl, col, dest_cl.a);
		dest_cl.b = BLEND_CHANN_ALPHA(b, dest_cl, col, dest_cl.a);
		PutPixel_32(addr, Pack(dest_cl, dstfmt));
	}
};

//////////////////////////
// Get the alpha pixel putter for the surface
INLINE PixelPutAlpha& getPixelAlphaPutFunc(const SDL_Surface *surf)  {
	static PixelPutAlpha_SolidBg_16 px16;
	static PixelPutAlpha_SolidBg_24 px24;
	static PixelPutAlpha_SolidBg_32 px32;
	static PixelPutAlpha_AlphaBg_32 px32_a;

	switch (surf->format->BytesPerPixel)  {
	case 1:
		assert(false); // No alpha blending for 8-bit surfaces atm
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
	static PixelGet_32_nonalpha px32nonalpha;
	
	switch (surf->format->BytesPerPixel)  {
	case 1:
		return px8;
	case 2:
		return px16;
	case 3:
		return px24;
	case 4:
		if(surf->format->Amask != 0) return px32;
		else return px32nonalpha;
	default:
		assert(false);
	}

	return px32; // Should not happen
}

/////////////////////
// Returns a pixel copy functor for the given surface
INLINE PixelCopy& getPixelCopyFunc(const SDL_Surface *source_surf, const SDL_Surface *dest_surf)
{
	static PixelCopy_8_8	copy_8_8;	copy_8_8.setformats(source_surf->format, dest_surf->format);
	static PixelCopy_8_16	copy_8_16;	copy_8_16.setformats(source_surf->format, dest_surf->format);
	static PixelCopy_8_24	copy_8_24;	copy_8_24.setformats(source_surf->format, dest_surf->format);
	static PixelCopy_8_32	copy_8_32;	copy_8_32.setformats(source_surf->format, dest_surf->format);
	static PixelCopy_16_8	copy_16_8;	copy_16_8.setformats(source_surf->format, dest_surf->format);
	static PixelCopy_16_16	copy_16_16;	copy_16_16.setformats(source_surf->format, dest_surf->format);
	static PixelCopy_16_24	copy_16_24;	copy_16_24.setformats(source_surf->format, dest_surf->format);
	static PixelCopy_16_32	copy_16_32;	copy_16_32.setformats(source_surf->format, dest_surf->format);
	static PixelCopy_24_8	copy_24_8;	copy_24_8.setformats(source_surf->format, dest_surf->format);
	static PixelCopy_24_16	copy_24_16;	copy_24_16.setformats(source_surf->format, dest_surf->format);
	static PixelCopy_24_24	copy_24_24;	copy_24_24.setformats(source_surf->format, dest_surf->format);
	static PixelCopy_24_32	copy_24_32;	copy_24_32.setformats(source_surf->format, dest_surf->format);
	static PixelCopy_32_8	copy_32_8;	copy_32_8.setformats(source_surf->format, dest_surf->format);
	static PixelCopy_32_16	copy_32_16;	copy_32_16.setformats(source_surf->format, dest_surf->format);
	static PixelCopy_32_24	copy_32_24;	copy_32_24.setformats(source_surf->format, dest_surf->format);
	static PixelCopy_32_32	copy_32_32;	copy_32_32.setformats(source_surf->format, dest_surf->format);
	static PixelCopy_32na_32	copy_32na_32;	copy_32na_32.setformats(source_surf->format, dest_surf->format);
	static PixelCopy_32key_32	copy_32key_32;	copy_32key_32.setformats(source_surf->format, dest_surf->format);
	
#define DEST_SWITCH(sourcebpp)  switch (dest_surf->format->BytesPerPixel)  {\
	case 1: return copy_##sourcebpp##_8;  \
	case 2: return copy_##sourcebpp##_16;  \
	case 3: return copy_##sourcebpp##_24;  \
	case 4: if(sourcebpp == 32 && source_surf->format->Amask == 0) { \
				if(source_surf->flags & SDL_SRCCOLORKEY) \
					return copy_32key_32; \
				else \
					return copy_32na_32; } \
			else return copy_##sourcebpp##_32;  \
	}

	switch (source_surf->format->BytesPerPixel)  {
	case 1:
		DEST_SWITCH(8);
	break;
	case 2:
		DEST_SWITCH(16);
	break;
	case 3:
		DEST_SWITCH(24);
	break;
	case 4:
		DEST_SWITCH(32);
	break;
	}

	return copy_8_8; // Should not happen
}

#endif
