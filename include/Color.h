/*
	OpenLieroX

	color type and related functions

	code under LGPL
	created 10-01-2007
*/

#ifndef __COLOR_H__
#define __COLOR_H__

#include <SDL.h>


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


struct Color {
	double r, g, b;

	Color(Uint8 r_ = 0, Uint8 g_ = 0, Uint8 b_ = 0) : r(r_/255.0), g(g_/255.0), b(b_/255.0) {}
	Color(double r_, double g_, double b_) : r(r_), g(g_), b(b_) {}
	Color(Uint32 px, SDL_PixelFormat* fmt) {
		r = GetR(px, fmt) / 255.0;
		g = GetG(px, fmt) / 255.0;
		b = GetB(px, fmt) / 255.0;
	}

	Color operator+(const Color& c) const { return Color(r+c.r, g+c.g, b+c.b); }
	Color operator-(const Color& c) const { return (*this) + c * -1.0; }
	Color operator*(double f) const { return Color(r*f, g*f, b*f); }
	Color operator/(double f) const { return (*this) * (1.0/f); }
	bool operator==(const Color& c) const { return r==c.r && g==c.g && b==c.b; }
	bool operator!=(const Color& c) const { return ! ((*this) == c); }

	Uint8 r8() const { if(r >= 1.0) return 255; else if(r <= 0.0f) return 0; else return (Uint8)(r * 255.0f); }
	Uint8 g8() const { if(g >= 1.0) return 255; else if(g <= 0.0f) return 0; else return (Uint8)(g * 255.0f); }
	Uint8 b8() const { if(b >= 1.0) return 255; else if(b <= 0.0f) return 0; else return (Uint8)(b * 255.0f); }
	Uint32 pixel(SDL_PixelFormat* fmt) const { return SDL_MapRGB(fmt, r8(), g8(), b8()); }
};

#endif
