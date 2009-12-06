#ifndef SPRITE_SET_H
#define SPRITE_SET_H

#include "resource_list.h"
#include "util/angle.h"
#include "util/GusCache.h"
#include "util/rect.h"
#include "glua.h"
#include <allegro.h>
#include <string>
#include <vector>
#include <utility>
#include <map>
/*#include <boost/filesystem/path.hpp>
namespace fs = boost::filesystem;*/

class Sprite;

class BlitterContext;

class SpriteSet : public LuaObject
{
public:
		
	SpriteSet();
#ifndef DEDICATED_ONLY
	SpriteSet(SpriteSet const&, SpriteSet const& mask, int color);
#endif
	~SpriteSet();

	bool load(std::string const& filename);
	
	void think();
	
	Sprite* getSprite( size_t frame, Angle angle );
	Sprite* getSprite( size_t frame = 0 );
#ifndef DEDICATED_ONLY
	Sprite* getColoredSprite( size_t frame, SpriteSet* mask, int color, Angle angle = Angle(0.0) );
	
	void drawSkinnedBox(BITMAP* b, BlitterContext& blitter, Rect const& rect, int backgroundColor);
#endif

	void flipSprites();

	size_t getFramesWidth()
	{
		return frameCount;
	}
			
	size_t frameCount;
	size_t angleCount;		
	
private:
	Sprite* getSprite_(size_t frame, size_t angleFrame)
	{
		Sprite* s = m_frames.at(angleFrame*frameCount + frame);
		assert(s);
		return s;
	}
	
	Sprite* getFlippedSprite_(size_t frame, size_t angleFrame)
	{
		if(m_flippedFrames.empty())
		{
			flipSprites();
			assert(!m_flippedFrames.empty());
		}
			
		Sprite* s = m_flippedFrames.at(angleFrame*frameCount + frame);
		assert(s);
		return s;
	}
	
	typedef std::pair<SpriteSet*, int> ColorKey;

	std::vector< Sprite* > m_frames;
	std::vector< Sprite* > m_flippedFrames;
	long m_angleFactor;
	long m_halfAngleDivisonSize;

	
#ifndef DEDICATED_ONLY
	struct ColorSpriteSet
	{
		ColorSpriteSet(SpriteSet& parent_)
		: parent(parent_)
		{
		}
		
		SpriteSet* operator()(ColorKey const& key);
		
		SpriteSet& parent;
	};
	
	struct DeleteSpriteSet
	{
		void operator()(SpriteSet* spriteSet);
	};

	Cache<ColorKey, SpriteSet*, ColorSpriteSet, DeleteSpriteSet> m_coloredCache;
#endif

};

extern ResourceList<SpriteSet> spriteList;

#endif // _SPRITE_SET_H_
