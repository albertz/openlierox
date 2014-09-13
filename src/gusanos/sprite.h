#ifndef SPRITE_H
#define SPRITE_H

#include "gfx.h"
//#include "gusanos/allegro.h"

struct ALLEGRO_BITMAP;
struct BlitterContext;

enum
{
	ALIGN_TOP = 1,
	ALIGN_BOTTOM = 2,
	ALIGN_RIGHT = 4,
	ALIGN_LEFT = 16
};

class Sprite
{
public:
	struct MirrorTag { };
	
	Sprite(ALLEGRO_BITMAP* bitmap, int xPivot, int yPivot);
#ifndef DEDICATED_ONLY
	Sprite(Sprite const&, Sprite const&, int);
#endif
	Sprite(Sprite const& b, MirrorTag);
	~Sprite();
	
#ifndef DEDICATED_ONLY
	void drawCut(ALLEGRO_BITMAP *where, int x, int y, BlitterContext const& blender, int alignment, int left, int top, int bottom, int right);
	void draw(ALLEGRO_BITMAP *where, int x, int y, BlitterContext const& blender, int alignment = 0);
	
	// To ease transition
	void draw(ALLEGRO_BITMAP *where, int x, int y, int alignment = 0);

#endif

	int getWidth()
	{ return m_bitmap->w; }
	
	int getHeight()
	{ return m_bitmap->h; }
	
	ALLEGRO_BITMAP *m_bitmap;
	//ALLEGRO_BITMAP *m_mirror;
	int m_xPivot;
	int m_yPivot;
	
};

Sprite* genLight( int radius );

#endif // _SPRITE_H_
