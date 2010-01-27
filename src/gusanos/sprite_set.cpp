#include "gusanos/sprite_set.h"

#include "gusanos/resource_list.h"
#include "gusanos/gfx.h"
#include "gusanos/sprite.h"
#include "util/macros.h"

#include "gusanos/allegro.h"
#include <string>
#include <vector>
#include <iostream> //TEMP
#include <stdexcept>

#ifndef DEDICATED_ONLY
#include "gusanos/blitters/context.h"
#endif

using namespace std;

ResourceList<SpriteSet> spriteList;

SpriteSet::SpriteSet()
//: m_flipped(0)
#ifndef DEDICATED_ONLY
		:
		m_coloredCache(ColorSpriteSet(*this))
#endif
{
}

#ifndef DEDICATED_ONLY
// This does not copy the colored cache naturally
SpriteSet::SpriteSet(SpriteSet const& b, SpriteSet const& mask, int color)
		: 
		frameCount(b.frameCount),
		angleCount(b.angleCount),
		m_frames(b.m_frames),
		m_angleFactor(b.m_angleFactor),
		m_halfAngleDivisonSize(b.m_halfAngleDivisonSize),
		m_coloredCache(ColorSpriteSet(*this))
{
	std::vector<Sprite *>::const_iterator srci = b.m_frames.begin();
	std::vector<Sprite *>::const_iterator maski = mask.m_frames.begin();
	std::vector<Sprite *>::iterator desti = m_frames.begin();
	for (; desti != m_frames.end();
	        ++srci, ++maski, ++desti) {
		if((*srci)->m_bitmap->w != (*maski)->m_bitmap->w
		        || (*srci)->m_bitmap->h != (*maski)->m_bitmap->h)
			throw std::runtime_error("Mask sprite is not the same size as source sprite");

		*desti = new Sprite(**srci, **maski, color);
	}
}
#endif

SpriteSet::~SpriteSet()
{
	foreach(frame, m_frames)
	delete *frame;
	foreach(frame, m_flippedFrames)
	delete *frame;
}

bool SpriteSet::load(std::string const& filename)
{
	//cerr << "Loading sprite set: " << filename.native_file_string() << endl;

	ALLEGRO_BITMAP *tempBitmap = gfx.loadBitmap(filename.c_str(), true);

	if (!tempBitmap)
		return false;

	LocalSetColorConversion cc(0/*COLORCONV_NONE*/);
	LocalSetColorDepth cd(bitmap_color_depth(tempBitmap));

	if ( (tempBitmap->w > 1) && (tempBitmap->h > 1) ) {
		int lastY = 1;
		int pivotY = -1;
		angleCount = 0;

		for (int y = 1; y < tempBitmap->h; ++y) {
			if( gfx.compareRGB(getpixel(tempBitmap,0,y),makecol(255,0,0)) ) // Red pixel marks the pivot of the sprite
			{
				pivotY = y-lastY;
			} else if( gfx.compareRGB(getpixel(tempBitmap,0,y), 0) || y == tempBitmap->h - 1 ) {
				++angleCount;

				int lastX = 1;
				int pivotX = -1;
				frameCount = 0;

				for (int x = 1; x < tempBitmap->w; ++x) {
					// Pivot again but for X axis
					if( gfx.compareRGB(getpixel(tempBitmap,x,0), makecol(255,0,0)) ) {
						pivotX = x-lastX;
					} else if(gfx.compareRGB(getpixel(tempBitmap,x,0), 0) || x == tempBitmap->w - 1 ) {
						ALLEGRO_BITMAP* spriteFrame = create_bitmap(x-lastX+1, y-lastY+1);
						blit(tempBitmap, spriteFrame, lastX, lastY, 0, 0, spriteFrame->w, spriteFrame->h);
						//m_frames.back().push_back(new Sprite( spriteFrame, pivotX, pivotY ) );
						m_frames.push_back(new Sprite( spriteFrame, pivotX, pivotY ) );
						++frameCount;

						pivotX = -1;

						lastX = x + 1;
					}
				}

				pivotY = -1;
				lastY = y + 1;
			}
		}

		// Fill the other 180 with the sprites but mirrored.

	}

	destroy_bitmap(tempBitmap);

	m_angleFactor = (angleCount - 1) * 2;
	m_halfAngleDivisonSize = (1 << 15) / angleCount / 2;

	return true;
}

Sprite* SpriteSet::getSprite( size_t frame )
{
	if ( frame > frameCount ) {
		frame = 0;
	}
	return getSprite_(frame, 0);
}

Sprite* SpriteSet::getSprite( size_t frame, Angle angle )
{	
	//angle.clamp();
	if ( frame > frameCount ) {
		frame = 0;
	} //??

	bool flipped = false;
	if(angle > Angle(180.0)) {
		angle = Angle(360.0) - angle;
		flipped = true;
	}

	angle.clamp();

	//TODO: warning: left shift count >= width of type
	size_t angleFrame = ((angle.adjust<16>() + m_halfAngleDivisonSize) * m_angleFactor) >> 16;

	if ( angleFrame >= angleCount ) {
		angleFrame = angleCount - 1;
	}

	if(flipped)
		return getFlippedSprite_(frame, angleFrame);
	else
		return getSprite_(frame, angleFrame);
}

void SpriteSet::think()
{
#ifndef DEDICATED_ONLY
	m_coloredCache.think();
#endif
}

#ifndef DEDICATED_ONLY
SpriteSet* SpriteSet::ColorSpriteSet::operator()(ColorKey const& key)
{
	return new SpriteSet(parent, *key.first, key.second);
}

void SpriteSet::DeleteSpriteSet::operator()(SpriteSet* spriteSet)
{
	delete spriteSet;
}


Sprite* SpriteSet::getColoredSprite( size_t frame, SpriteSet* mask, int color, Angle angle )
{
	if(!mask) {
		return getSprite(frame, angle);
	}

	if(mask->frameCount != frameCount
	        || mask->angleCount != angleCount) {
		return 0;
	}

	//Disabled - cache wasn't returning anything
	//SpriteSet* s = m_coloredCache[std::make_pair(mask, color)];
	//return s->getSprite(frame, angle);
	
	return getSprite(frame, angle);
}


void SpriteSet::drawSkinnedBox(ALLEGRO_BITMAP* b, BlitterContext& blitter, Rect const& rect, int backgroundColor)
{
	int skinWidth = getSprite(0)->getWidth(), skinHeight = getSprite(0)->getHeight();

	int x, y;

	blitter.rectfill(b, rect.x1 + skinWidth, rect.y1 + skinHeight, rect.x2 - skinWidth, rect.y2 - skinHeight, backgroundColor);
	int lim = rect.y2 - skinHeight * 2 + 1;
	for(y = rect.y1 + skinHeight; y < lim; y += skinHeight) {
		getSprite(4)->draw(b, rect.x1, y, blitter);
		getSprite(5)->draw(b, rect.x2 - skinWidth + 1, y, blitter);
	}

	int cutOff = y - lim;
	getSprite(4)->drawCut(b, rect.x1, y, blitter, 0, 0, 0, cutOff, 0);
	getSprite(5)->drawCut(b, rect.x2 - skinWidth + 1, y, blitter, 0, 0, 0, cutOff, 0);

	lim = rect.x2 - skinWidth * 2 + 1;
	for(x = rect.x1 + skinWidth; x < lim; x += skinWidth) {
		getSprite(6)->draw(b, x, rect.y1, blitter);
		getSprite(7)->draw(b, x, rect.y2 - skinHeight + 1, blitter);
	}

	cutOff = x - lim;
	getSprite(6)->drawCut(b, x, rect.y1, blitter, 0, 0, 0, 0, cutOff);
	getSprite(7)->drawCut(b, x, rect.y2 - skinWidth + 1, blitter, 0, 0, 0, 0, cutOff);

	getSprite(0)->draw(b, rect.x1, rect.y1, blitter);
	getSprite(1)->draw(b, rect.x2 - skinWidth + 1, rect.y1, blitter);
	getSprite(2)->draw(b, rect.x1, rect.y2 - skinHeight + 1, blitter);
	getSprite(3)->draw(b, rect.x2 - skinWidth + 1, rect.y2 - skinHeight + 1, blitter);
}

#endif

void SpriteSet::flipSprites()
{
	assert(m_flippedFrames.empty());

	const_foreach(src, m_frames) {
		m_flippedFrames.push_back(new Sprite(**src, Sprite::MirrorTag()));
	}
}
