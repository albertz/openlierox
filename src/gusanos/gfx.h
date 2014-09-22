#ifndef GFX_H
#define GFX_H

#include <string>
#include "gusanos/allegro.h"

enum Blenders
{
	ALPHA,
	ADD,
	NONE
};

class Gfx
{
public:
		
	Gfx();
	~Gfx();
	
	void init();
	void shutDown();
	void registerInConsole();
	void loadResources();

#ifndef DEDICATED_ONLY
	INLINE void setBlender( Blenders blender, int alpha )
	{
		switch ( blender )
		{
			case ALPHA:
				set_trans_blender(255, 255, 255, alpha);
			break;
			
			case ADD:
				set_add_blender( 255,255,255, alpha);
			break;
			
			case NONE:
				solid_mode();
			break;
		}
	}
	
	int m_distortionAA;
#endif

	INLINE bool compareRGB( Uint32 c1, Uint32 c2 )
	{
		return ( getr(c1) == getr(c2) && getg(c1) == getg(c2) && getb(c1) == getb(c2) );
	}
	
	ALLEGRO_BITMAP* loadBitmap(const std::string &filename, bool keepAlpha = false, bool stretch2 = true);
	SmartPointer<SDL_Surface> loadBitmapSDL(const std::string &filename, bool keepAlpha = false, bool stretch2 = true);
	
	operator bool(); // Returns true if it's safe to use this object

};

extern Gfx gfx;

#endif // _GFX_H_
