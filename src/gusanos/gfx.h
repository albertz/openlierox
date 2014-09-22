#ifndef GFX_H
#define GFX_H

#include <string>
#include "gusanos/allegro.h"

class Gfx
{
public:
	void registerInConsole();
	void loadResources();

#ifndef DEDICATED_ONLY	
	int m_distortionAA;
#endif

	INLINE bool compareRGB( Uint32 c1, Uint32 c2 )
	{
		return ( getr(c1) == getr(c2) && getg(c1) == getg(c2) && getb(c1) == getb(c2) );
	}
	
	ALLEGRO_BITMAP* loadBitmap(const std::string &filename, bool keepAlpha = false, bool stretch2 = true);
	SmartPointer<SDL_Surface> loadBitmapSDL(const std::string &filename, bool keepAlpha = false, bool stretch2 = true);
	
};

extern Gfx gfx;

#endif // _GFX_H_
