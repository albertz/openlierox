#ifndef LEVEL_EFFECT_H
#define LEVEL_EFFECT_H

#include "resource_list.h"
#include "resource_base.h"
#include "gfx.h"
#include <boost/cstdint.hpp>

#include "gusanos/allegro.h"
#include <string>

class SpriteSet;

class LevelEffect : public ResourceBase
{
public:
	
	LevelEffect();
	~LevelEffect();

	bool load(std::string const& filename);
	
	SpriteSet* mask;
	uint32_t crc;
};

extern ResourceList<LevelEffect> levelEffectList;

#endif // _LEVEL_EFFECT_H_
