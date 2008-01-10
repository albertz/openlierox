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

///////////////
// Returns true if the two colors are the same, ignoring the alpha
inline bool EqualRGB(Uint32 p1, Uint32 p2, SDL_PixelFormat* fmt) {
	return ((p1|fmt->Amask) == (p2|fmt->Amask));
}

///////////////
// Creates a int colour based on the 3 components
// HINT: format is that one from videosurface!
inline Uint32 MakeColour(Uint8 r, Uint8 g, Uint8 b) {
	return SDL_MapRGB(SDL_GetVideoSurface()->format,r,g,b);
}


#endif
