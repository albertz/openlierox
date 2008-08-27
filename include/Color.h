/*
	OpenLieroX

	color type and related functions

	code under LGPL
	created 10-01-2007
*/

#ifndef __COLOR_H__
#define __COLOR_H__

#include <SDL.h>
#include "SmartPointer.h"


///////////////////
// If you want to use the adress of some Uint32 directly with memcpy or similar, use this
inline Uint32 SDLColourToNativeColour(Uint32 pixel, short bpp) {
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	return (pixel << (32 - 8 * bpp));
#else
	return pixel;
#endif
}

/////////////////
// If you copied some data directly with memcpy into an Uint32, use this
inline Uint32 NativeColourToSDLColour(Uint32 pixel, short bpp) {
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	return (pixel >> (32 - 8 * bpp));
#else
	return pixel;
#endif
}



///////////////
// Get the specified component from the pixel (grabbed from SDL)
inline Uint8 GetR(Uint32 pixel, SDL_PixelFormat *fmt)  {
	return  (((pixel & fmt->Rmask) >> fmt->Rshift) << fmt->Rloss) +
			(((pixel & fmt->Rmask) >> fmt->Rshift) >> (8 - (fmt->Rloss << 1)));
}

inline Uint8 GetG(Uint32 pixel, SDL_PixelFormat *fmt)  {
	return  (((pixel & fmt->Gmask) >> fmt->Gshift) << fmt->Gloss) +
			(((pixel & fmt->Gmask) >> fmt->Gshift) >> (8 - (fmt->Gloss << 1)));
}

inline Uint8 GetB(Uint32 pixel, SDL_PixelFormat *fmt)  {
	return  (((pixel & fmt->Bmask) >> fmt->Bshift) << fmt->Bloss) +
			(((pixel & fmt->Bmask) >> fmt->Bshift) >> (8 - (fmt->Bloss << 1)));
}

inline Uint8 GetA(Uint32 pixel, SDL_PixelFormat *fmt)  {
	return  (((pixel & fmt->Amask) >> fmt->Ashift) << fmt->Aloss) +
			(((pixel & fmt->Amask) >> fmt->Ashift) >> (8 - (fmt->Aloss << 1)));
}

extern SDL_PixelFormat* mainPixelFormat;

inline SDL_PixelFormat* getMainPixelFormat() {
	return mainPixelFormat;
}

///////////////
// Returns true if the two colors are the same, ignoring the alpha
// HINT: both colors have to be in the same pixelformat
inline bool EqualRGB(Uint32 p1, Uint32 p2, SDL_PixelFormat* fmt) {
	return ((p1|fmt->Amask) == (p2|fmt->Amask));
}

///////////////
// Creates a int colour based on the 3 components
// HINT: format is that one from videosurface!
inline Uint32 MakeColour(Uint8 r, Uint8 g, Uint8 b) {
	return SDL_MapRGB(getMainPixelFormat(), r, g, b);
}

///////////////
// Creates a int colour based on the 4 components
// HINT: format is that one from videosurface!
inline Uint32 MakeColour(Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
	return SDL_MapRGBA(getMainPixelFormat(), r, g, b, a);
}



// Device-independent color
class Color  { public:
	Color() : r(0), g(0), b(0), a(0) {}
	Color(Uint8 _r, Uint8 _g, Uint8 _b) : r(_r), g(_g), b(_b), a(SDL_ALPHA_OPAQUE) {}
	Color(Uint8 _r, Uint8 _g, Uint8 _b, Uint8 _a) : r(_r), g(_g), b(_b), a(_a) {}
	Color(SDL_Surface *s, Uint32 cl)	{ SDL_GetRGBA(cl, s->format, &r, &g, &b, &a); }
	Color(const SmartPointer<SDL_Surface>& s, Uint32 cl)	{ SDL_GetRGBA(cl, s->format, &r, &g, &b, &a); }
	Color(Uint32 cl)	{ set(SDL_GetVideoSurface(), cl); }
	Color(const SDL_Color& cl) { r = cl.r; g = cl.g; b = cl.b; }

	Uint8 r;
	Uint8 g;
	Uint8 b;
	Uint8 a;

	Uint32 get(SDL_Surface *s)  { return SDL_MapRGBA(s->format, r, g, b, a); }
	Uint32 get(const SmartPointer<SDL_Surface>& s)  { return SDL_MapRGBA(s->format, r, g, b, a); }
	void set(SDL_Surface *s, Uint32 cl)	{ SDL_GetRGBA(cl, s->format, &r, &g, &b, &a); }
	void set(const SmartPointer<SDL_Surface>& s, Uint32 cl)	{ SDL_GetRGBA(cl, s->format, &r, &g, &b, &a); }
	
	bool operator == ( const Color & c ) const { return r == c.r && g == c.g && b == c.b && a == c.a; };
	bool operator != ( const Color & c ) const { return ! ( *this == c ); };
	
	Color operator * ( float f ) const { return Color( Uint8(r*f), Uint8(g*f), Uint8(b*f), a ); };
	Color operator + ( const Color & c ) const { return Color( r+c.r, g+c.g, b+c.b, (a+c.a)/2 ); };
};

#endif
