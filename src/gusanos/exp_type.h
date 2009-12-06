#ifndef EXP_TYPE_H
#define EXP_TYPE_H

#include "resource_list.h"
#include "glua.h"
#ifndef DEDICATED_ONLY
#include "gfx.h"
#include "blitters/context.h"
#endif

#include "gusanos/allegro.h"
#include <string>
#include <vector>
#include <boost/cstdint.hpp>
#include <boost/filesystem/path.hpp>
namespace fs = boost::filesystem;

class SpriteSet;
class Distortion;
class GameEvent;
class DetectEvent;
class Sprite;

class ExpType : public LuaObject
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
	int colour;
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
