#ifndef SPRITE_H
#define SPRITE_H

#include "gfx.h"
//#include <allegro.h>

struct BITMAP;
class BlitterContext;

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
	
	Sprite(BITMAP* bitmap, int xPivot, int yPivot);
#ifndef DEDSERV
	Sprite(Sprite const&, Sprite const&, int);
#endif
	Sprite(Sprite const& b, MirrorTag);
	~Sprite();
	
#ifndef DEDSERV
	//void draw(BITMAP *where, int x, int y, bool flipped = false, int Alignment = 0);
	void drawCut(BITMAP *where, int x, int y, BlitterContext const& blender, int alignment, int left, int top, int bottom, int right);
	//void drawBlended(BITMAP *where, int x, int y, bool flipped = false, int alignment = 0, Blenders blender = ADD );
	void draw(BITMAP *where, int x, int y, BlitterContext const& blender/*, bool flipped = false*/, int alignment = 0);
	
	// To ease transition
	void draw(BITMAP *where, int x, int y/*, bool flipped = false*/, int alignment = 0);

#endif

	int getWidth()
	{ return m_bitmap->w; }
	
	int getHeight()
	{ return m_bitmap->h; }
	
	BITMAP *m_bitmap;
	//BITMAP *m_mirror;
	int m_xPivot;
	int m_yPivot;
	
};

Sprite* genLight( int radius );

#endif // _SPRITE_H_
