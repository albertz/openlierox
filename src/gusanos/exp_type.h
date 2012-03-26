#ifndef EXP_TYPE_H
#define EXP_TYPE_H

#include "resource_list.h"
#include "util/BaseObject.h"
#ifndef DEDICATED_ONLY
#include "gfx.h"
#include "blitters/context.h"
#endif

#include "gusanos/allegro.h"
#include <string>
#include <vector>
#include <boost/cstdint.hpp>

class SpriteSet;
class Distortion;
struct GameEvent;
struct DetectEvent;
class Sprite;

class ExpType : public BaseObject
{
public:
	
	ExpType();
	~ExpType();

	bool load(std::string const& filename);

	int timeout;
	int timeoutVariation;

#ifndef DEDICATED_ONLY
	Distortion* distortion;
	float distortMagnitude;
	SpriteSet* sprite;
	BlitterContext::Type blender;
	Sprite* lightHax;
	bool rockHidden;
#endif //DEDICATED_ONLY
	int renderLayer;
	uint32_t colour;
	int alpha;
	int destAlpha;
	uint32_t crc;

	bool wupixels;
	bool invisible;

	std::vector< DetectEvent* > detectRanges;
	GameEvent *creation;
};

extern ResourceList<ExpType> expTypeList;

#endif // _PART_TYPE_H_
