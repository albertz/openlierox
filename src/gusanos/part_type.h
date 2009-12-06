#ifndef PART_TYPE_H
#define PART_TYPE_H

#include "resource_list.h"
#ifndef DEDICATED_ONLY
#include "gfx.h"
#include "distortion.h"
#include "blitters/context.h"
#include "gusanos/allegro.h"
#endif
#include "events.h"
#include "util/vec.h"
#include "util/angle.h"
//#include "luaapi/types.h"
#include "script.h"
#include "resource_base.h"
#include <boost/cstdint.hpp>

#include <string>
#include <vector>
#include <boost/filesystem/path.hpp>
namespace fs = boost::filesystem;

#ifndef DEDICATED_ONLY
class SpriteSet;
class BaseAnimator;
class Sprite;
#endif
class DetectEvent;
class BasePlayer;
class PartType;
class TimerEvent;

typedef BaseObject* (*NewParticleFunc)(PartType* type, Vec pos_, Vec spd_, int dir, BasePlayer* owner, Angle angle);

BaseObject* newParticle_requested( PartType* type, Vec pos_, Vec spd_, int dir, BasePlayer* owner, Angle angle );

class PartType : public ResourceBase
{
public:
	
	static LuaReference metaTable;
		
	PartType();
	~PartType();
	
	void touch();
	bool isSimpleParticleType();
	bool load(std::string const& filename);
	
	virtual void makeReference();
	virtual void finalize();

#ifndef DEDICATED_ONLY
	BaseAnimator* allocateAnimator();
#endif
	NewParticleFunc newParticle;

	float gravity;
	float bounceFactor;
	float groundFriction;
	float damping;
	float acceleration;
	float maxSpeed;
	float radius; //Hax!
	int repeat;
	AngleDiff angularFriction;
	int colLayer;
	float health;
	uint32_t crc;
	/*
	std::string networkInitName;
	LuaReference networkInit;
	LuaReference getNetworkInit();
	*/
	LazyScript networkInit;
	LazyScript distortionGen;
	IVec distortionSize;
	LazyScript lightGen;
	IVec lightSize;
	
#ifndef DEDICATED_ONLY
	Distortion* distortion;
	float distortMagnitude;
#endif
	int renderLayer;
	int colour;
	int alpha;
#ifndef DEDICATED_ONLY
	SpriteSet* sprite;
	Sprite* lightHax;
#endif
	int animDuration;
	int animType;
	int animOnGround;
	//Blenders blender;
#ifndef DEDICATED_ONLY
	BlitterContext::Type blender;
#endif
	bool line2Origin;
	bool wupixels;
	bool invisible;
	bool culled;
	
	bool syncPos;
	bool syncSpd;
	bool syncAngle;
	bool needsNode;
	
	int simpleParticle_timeout;
	int simpleParticle_timeoutVariation;
	
	std::vector< TimerEvent* > timer;
	std::vector< DetectEvent* > detectRanges;
	std::vector< GameEvent* > customEvents;
	GameEvent *groundCollision;
	GameEvent *creation;
	GameEvent *death;
	
	enum
	{
		ANIM_PINGPONG = 0,
		ANIM_LOOPRIGHT,
		ANIM_RIGHTONCE
	};
	
};

extern ResourceList<PartType> partTypeList;

#endif // _PART_TYPE_H_
