#ifndef GAME_ACTIONS_H
#define GAME_ACTIONS_H

#include "base_action.h"
#include "util/angle.h"
//#include "luaapi/types.h"
#include "script.h"
#include <string>
#include <vector>

class PartType;
class ExpType;
class Sound;
class Sound1D;
class SpriteSet;
class BaseObject;
class LevelEffect;

namespace OmfgScript
{
struct TokenBase;
struct ActionFactory;
}

extern OmfgScript::ActionFactory gameActions;

#define GAME_ACTION(name_, body_) \
class name_ : public BaseAction { \
public: \
	name_( std::vector<OmfgScript::TokenBase*> const& params ); \
	~name_(); \
	virtual void run( ActionParams const& params ); \
private: \
	body_ };

void registerGameActions();

GAME_ACTION(ShootParticles,
	PartType *type;
	
	int amount;
	int amountVariation;
	
	float motionInheritance;
	float speed;
	float speedVariation;
	Angle distribution;
	AngleDiff angleOffset;
	float distanceOffset;
)

GAME_ACTION(UniformShootParticles,
	PartType *type;

	int amount;
	int amountVariation;

	float motionInheritance;
	float speed;
	float speedVariation;
	Angle distribution;
	AngleDiff angleOffset;
	float distanceOffset;
)

GAME_ACTION(PutParticle,
	PartType *type;

	float x;
	float y;
	float xspd;
	float yspd;
	Angle angle;
)

GAME_ACTION(CreateExplosion,
	ExpType *type;
)

GAME_ACTION(Push,
	float factor;
)

GAME_ACTION(Repel,
	float maxForce;
	float minForce;
	float maxDistance;
	
	// Precomputed constants
	float maxDistanceSqr;
	float forceDiffScaled;
)

GAME_ACTION(Damp,
	float factor;
)

GAME_ACTION(Damage,
	float m_damage;
	float m_damageVariation;
	float m_maxDistance;
)

GAME_ACTION(Remove,
)

GAME_ACTION(PlaySound,
	std::vector<Sound*> sounds;
	float pitch;
	float pitchVariation;
	float loudness;
)

GAME_ACTION(PlaySoundStatic,
	std::vector<Sound*> sounds;
	float pitch;
	float pitchVariation;
	float loudness;
)

GAME_ACTION(PlayGlobalSound,
	std::vector<Sound1D*> sounds;
	float volume;
	float volumeVariation;
	float pitch;
	float pitchVariation;
)

GAME_ACTION(DelayFire,
	int delayTime;
	int delayTimeVariation;
)

GAME_ACTION(UseAmmo,
	int amount;
)

GAME_ACTION(ShowFirecone,
	int frames;
	float drawDistance;
	SpriteSet* sprite;
)

GAME_ACTION(AddAngleSpeed,
	AngleDiff speed;
	AngleDiff speedVariation;
)

GAME_ACTION(AddSpeed,
	float speed;
	float speedVariation;
	AngleDiff offs;
	Angle offsVariation;
)

GAME_ACTION(SetAlphaFade,
	int frames;
	int dest;
)

GAME_ACTION(RunCustomEvent,
	size_t index;
)

GAME_ACTION(RunScript,
	/*
	LuaReference function;
	std::string scriptName;*/
	LazyScript script;
)

GAME_ACTION(ApplyMapEffect,
		LevelEffect* effect;
)


#endif  // _GAME_ACTIONS_H_
