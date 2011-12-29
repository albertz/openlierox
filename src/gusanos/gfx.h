#ifndef GFX_H
#define GFX_H

#include "gusanos/allegro.h"

#include <string>
//#include <list>

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
		drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
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
	
	ALLEGRO_BITMAP* buffer;

	int m_distortionAA;
#endif

	INLINE bool compareRGB( Uint32 c1, Uint32 c2 )
	{
		return ( getr(c1) == getr(c2) && getg(c1) == getg(c2) && getb(c1) == getb(c2) );
	}
	
	ALLEGRO_BITMAP* loadBitmap(const std::string &filename, bool keepAlpha = false);
	bool saveBitmap(const std::string &filename, ALLEGRO_BITMAP* image);
	
	operator bool(); // Returns true if it's safe to use this object

};

#ifndef DEDICATED_ONLY
//std::string screenShot(const std::list<std::string> &args);
#endif

extern Gfx gfx;

#endif // _GFX_H_
