#include "sprite.h"

#include "gfx.h"
#include "CodeAttributes.h"
#ifndef DEDICATED_ONLY
#include "blitters/context.h"
#endif


#include "gusanos/allegro.h"
#include <iostream>
#include <algorithm>

using namespace std;

// LIGHTHAX!!
#include "CVec.h"
Sprite* genLight( int radius )
{
	radius *= 2; // doubleRes
	ALLEGRO_BITMAP* lightHax = create_bitmap_ex(8, 2*radius, 2*radius );
	for ( int x = 0; x < lightHax->w; ++x )
		for ( int y = 0; y < lightHax->h; ++y ) {
			int color = (int)(255 - ( 255 * Vec( (float)(radius-x), (float)(radius-y) ).length() ) / (float)radius);
			if ( color < 0 )
				color = 0;
			putpixel(lightHax,x,y,color);
		}
	return new Sprite(lightHax, -1, -1);
}

Sprite::Sprite( ALLEGRO_BITMAP* bitmap, int xPivot, int yPivot)
		: m_bitmap(bitmap)
{
	if ( xPivot == -1 )
		m_xPivot = m_bitmap->w / 2;
	else
		m_xPivot = xPivot * 2;
	if ( yPivot == -1 )
		m_yPivot = m_bitmap->h / 2;
	else
		m_yPivot = yPivot * 2;
}

INLINE int scaleColor(int a, int b, int bmax)
{
	return makecol(
	           getr(a) * b / bmax,
	           getg(a) * b / bmax,
	           getb(a) * b / bmax);
}

INLINE int brightenColor(int a, int b)
{
	return makecol(
	           std::min(getr(a) + b, 255),
	           std::min(getg(a) + b, 255),
	           std::min(getb(a) + b, 255));
}

#ifndef DEDICATED_ONLY

Sprite::Sprite(Sprite const& b, Sprite const& mask, int color)
		: m_bitmap(
		    create_bitmap_ex(
		        bitmap_color_depth(b.m_bitmap),
		        b.m_bitmap->w,
		        b.m_bitmap->h
		    )
		), m_xPivot(b.m_xPivot), m_yPivot(b.m_yPivot)
{
	int colorDepth = bitmap_color_depth(b.m_bitmap);
	LocalSetColorDepth cd(colorDepth);

	int wormColor = makecol(0, 0, 0);

	ALLEGRO_BITMAP* maskBitmap = mask.m_bitmap;
	ALLEGRO_BITMAP* srcBitmap = b.m_bitmap;

	static const int limit = 104;

	for(int y = 0; y < m_bitmap->h; ++y)
		for(int x = 0; x < m_bitmap->w; ++x) {
			int col = getpixel(srcBitmap, x, y);
			int m = getpixel(maskBitmap, x, y);
			if(m == wormColor) {
				int magn = getr(col);
				if(magn <= limit)
					col = scaleColor(color, magn, limit);
				else {
					int fact = 256*limit / magn;
					col = brightenColor(scaleColor(color, fact, 256), 256-fact);
				}
			}

			putpixel_solid(m_bitmap, x, y, col);
		}
}
#endif

Sprite::Sprite(Sprite const& b, MirrorTag)
		: m_bitmap(
		    create_bitmap_ex(
		        bitmap_color_depth(b.m_bitmap),
		        b.m_bitmap->w,
		        b.m_bitmap->h
		    )
		), m_xPivot( (b.m_bitmap->w -1 ) - b.m_xPivot), m_yPivot(b.m_yPivot)
{
	clear_to_color(m_bitmap, makecol(255,0,255));
	draw_sprite_h_flip(m_bitmap, b.m_bitmap, 0, 0);
}

Sprite::~Sprite()
{
	destroy_bitmap( m_bitmap );
}

#ifndef DEDICATED_ONLY

void Sprite::drawCut(ALLEGRO_BITMAP *where, int x, int y, BlitterContext const& blender, int alignment, int left, int top, int bottom, int right)
{
	int realX = x + left, realY = y + top;

	if ( alignment & ALIGN_LEFT ) /* Do nothing */
		;
	else if ( alignment & ALIGN_RIGHT )
		realX += m_bitmap->w;
	else
		realX += m_xPivot;

	if ( alignment & ALIGN_TOP ) /* Do nothing */
		;
	else if ( alignment & ALIGN_BOTTOM )
		realY += m_bitmap->h;
	else
		realY += m_yPivot;

	blender.drawSpriteCut(where, m_bitmap, realX, realY, left, top, right, bottom);

}

void Sprite::draw(ALLEGRO_BITMAP *where, int x, int y, int alignment)
{
	draw(where, x, y, BlitterContext(), alignment);
}

void Sprite::draw(ALLEGRO_BITMAP *where, int x, int y, BlitterContext const& blender, int alignment )
{
	int _x,_y;

	if ( alignment & ALIGN_LEFT )
		_x = 0;
	else if ( alignment & ALIGN_RIGHT )
		_x = m_bitmap->w;
	else
		_x = m_xPivot;

	if ( alignment & ALIGN_TOP )
		_y = 0;
	else if ( alignment & ALIGN_BOTTOM )
		_y = m_bitmap->h;
	else
		_y = m_yPivot;

	blender.drawSprite(where, m_bitmap, x - _x, y - _y);
}

#endif

