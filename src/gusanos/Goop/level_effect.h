#ifndef LEVEL_EFFECT_H
#define LEVEL_EFFECT_H

#include "resource_list.h"
#include "resource_base.h"
#include "gfx.h"
#include <boost/cstdint.hpp>

#include <allegro.h>
#include <string>
#include <boost/filesystem/path.hpp>
namespace fs = boost::filesystem;

class SpriteSet;

class LevelEffect : public ResourceBase
{
public:
	
	LevelEffect();
	~LevelEffect();

	bool load(fs::path const& filename);
	
	SpriteSet* mask;
	boost::uint32_t crc;
};

extern ResourceList<LevelEffect> levelEffectList;

#endif // _LEVEL_EFFECT_H_
