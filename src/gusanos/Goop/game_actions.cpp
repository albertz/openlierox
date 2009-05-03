#include "game_actions.h"

#include "game.h"
//#include "particle.h"
#include "part_type.h"
#include "explosion.h"
#include "exp_type.h"
#ifndef DEDSERV
#include "sound.h"
#include "sound1d.h"
#include "sprite_set.h"
#endif
#include "util/text.h"
#include "util/angle.h"
#include "util/vec.h"
#include "util/macros.h"
#include "util/log.h"
#include "base_object.h"
#include "weapon.h"
#include "worm.h"
#include "level_effect.h"

#include "glua.h"
#include "luaapi/context.h"
#include "script.h"

#include "omfg_script.h"

using namespace std;

OmfgScript::ActionFactory gameActions;

template<class T>
BaseAction* newAction(vector<OmfgScript::TokenBase*> const& params )
{
	return new T(params);
}

void registerGameActions()
{
	namespace af = OmfgScript::ActionParamFlags;
	
	gameActions.add("shoot_particles", newAction<ShootParticles>, af::Object)
		("type", false)
		("amount")
		("speed")
		("speed_var")
		("motion_inheritance")
		("amount_var")
		("distribution")
		("angle_offs")
		("distance_offs")
	;
	
	gameActions.add("uniform_shoot_particles", newAction<UniformShootParticles>, af::Object)
		("type", false)
		("amount")
		("speed")
		("speed_var")
		("motion_inheritance")
		("amount_var")
		("distribution")
		("angle_offs")
		("distance_offs")
	;
	
	gameActions.add("remove", newAction<Remove>, af::Object);
	
	gameActions.add("put_particle", newAction<PutParticle>, 0)
		("type", false)
		("x", false)
		("y", false)
		("xspd")
		("yspd")
		("angle")
	;
	
	gameActions.add("create_explosion", newAction<CreateExplosion>, af::Object)
		("type", false)
	;
	
	gameActions.add("play_sound", newAction<PlaySound>, af::Object)
		("sound", false)	
		("loudness")
		("pitch")
		("pitch_var")
	;
	
	gameActions.add("play_sound_static", newAction<PlaySoundStatic>, af::Object)
		("sound", false)	
		("loudness")
		("pitch")
		("pitch_var")
	;
	
	gameActions.add("play_global_sound", newAction<PlayGlobalSound>, 0)
		("sound", false)
		("volume")
		("volume_var")
		("pitch")
		("pitch_var")
	;
	
	gameActions.add("delay_fire", newAction<DelayFire>, af::Weapon)
		("time")
		("time_var")
	;
	
	gameActions.add("use_ammo", newAction<UseAmmo>, af::Weapon)
		("amount")
	;
	
	gameActions.add("add_angle_speed", newAction<AddAngleSpeed>, af::Object)
		("amount")
		("amount_var")
	;
	gameActions.add("add_speed", newAction<AddSpeed>, af::Object)
		("amount")
		("amount_var")
		("offs")
		("offs_var")
	;
	
	gameActions.add("push", newAction<Push>, af::Object | af::Object2)
		("factor")
	;
	
	gameActions.add("damage", newAction<Damage>, af::Object | af::Object2)
		("amount")
		("amount_var")
		("max_distance")
	;
	
	gameActions.add("set_alpha_fade", newAction<SetAlphaFade>, af::Object)
		("frames")
		("dest")
	;
	
	gameActions.add("show_firecone", newAction<ShowFirecone>, af::Object)
		("sprite", false)
		("frames")
		("draw_distance")
	;
	
	gameActions.add("custom_event", newAction<RunCustomEvent>, af::Object2)
		("index", false)
	;
	
	gameActions.add("run_script", newAction<RunScript>, 0)
		("name", false)
	;
	
	gameActions.add("repel", newAction<Repel>, af::Object | af::Object2)
		("max_force")
		("max_distance")
		("min_force")
	;
	gameActions.add("damp", newAction<Damp>, af::Object2)
		("factor")
	;
	
	gameActions.add("apply_map_effect", newAction<ApplyMapEffect>, af::Object)
		("effect", false)
	;
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

ShootParticles::ShootParticles( vector<OmfgScript::TokenBase*> const& params )
{
	type = partTypeList.load(params[0]->toString());
	amount = params[1]->toInt(1);
	speed = params[2]->toDouble(0.0);
	speedVariation = params[3]->toDouble(0.0);
	motionInheritance = params[4]->toDouble(0.0);
	amountVariation = params[5]->toInt(0);
	distribution = Angle(params[6]->toDouble(360.0));
	angleOffset = AngleDiff(params[7]->toDouble(0.0));
	distanceOffset = params[8]->toDouble(0.0);
}

void ShootParticles::run( ActionParams const& params )
{
	if (type)
	{
		int dir = params.object->getDir();
		Angle baseAngle(params.object->getAngle() + angleOffset * dir);
		
		int realAmount = amount + int(rnd()*amountVariation);
		for(int i = 0; i < realAmount; ++i)
		{
			Angle angle = baseAngle;
			if(distribution) angle += distribution * midrnd();
			Vec direction(angle);
			Vec spd(direction * (speed + midrnd()*speedVariation));
			if(motionInheritance)
			{
				spd += params.object->spd * motionInheritance;
				//angle = spd.getAngle(); // Need to recompute angle // <basara> No you dont
				//Perhaps as an option later.
			}

			type->newParticle(type, params.object->pos + direction * distanceOffset, spd, params.object->getDir(), params.object->getOwner(), angle);
		}
	}
}

ShootParticles::~ShootParticles()
{
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

UniformShootParticles::UniformShootParticles( vector<OmfgScript::TokenBase*> const& params )
{
	type = partTypeList.load(params[0]->toString());
	amount = params[1]->toInt(1);
	speed = params[2]->toDouble(0.0);
	speedVariation = params[3]->toDouble(0.0);
	motionInheritance = params[4]->toDouble(0.0);
	amountVariation = params[5]->toInt(0);
	distribution = Angle(params[6]->toDouble(360.0));
	angleOffset = AngleDiff(params[7]->toDouble(0.0));
	distanceOffset = params[8]->toDouble(0.0);
}

void UniformShootParticles::run( ActionParams const& params )
{
	if (type)
	{
		int dir = params.object->getDir();
		Angle angle(params.object->getAngle() + (angleOffset * dir) - ( distribution * 0.5f ) );
		
		
		int realAmount = amount + int(rnd()*amountVariation);
		
		AngleDiff angleIncrease( distribution / realAmount );
				
		for(int i = 0; i < realAmount; ++i, angle += angleIncrease )
		{
			Vec direction(angle);
			Vec spd(direction * (speed + midrnd()*speedVariation));
			if(motionInheritance)
			{
				spd += params.object->spd * motionInheritance;
				//angle = spd.getAngle(); // Need to recompute angle
			}

			type->newParticle(type, params.object->pos + direction * distanceOffset, spd, params.object->getDir(), params.object->getOwner(), angle);
		}
	}
}

UniformShootParticles::~UniformShootParticles()
{
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

PutParticle::PutParticle( vector<OmfgScript::TokenBase*> const& params )
{
	type = partTypeList.load(params[0]->toString());
	x = params[1]->toDouble();
	y = params[2]->toDouble();
	xspd = params[3]->toDouble(0.0);
	yspd = params[4]->toDouble(0.0);
	angle = Angle(params[5]->toDouble(0.0));
}

void PutParticle::run( ActionParams const& params )
{
	if (type)
	{
		type->newParticle(type, Vec(x,y), Vec(xspd,yspd), 1, 0, angle);
	}
}

PutParticle::~PutParticle()
{
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

CreateExplosion::CreateExplosion( vector<OmfgScript::TokenBase*> const& params )
{
	type = expTypeList.load(params[0]->toString());
}

void CreateExplosion::run( ActionParams const& params )
{
	if (type != NULL)
	{
		game.insertExplosion( new Explosion( type, params.object->pos, params.object->getOwner() ) );
	}
}

CreateExplosion::~CreateExplosion()
{
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

Push::Push( vector<OmfgScript::TokenBase*> const& params )
{
	factor = params[0]->toDouble(0.0);
}

void Push::run( ActionParams const& params  )
{
	params.object2->spd += params.object->spd*factor;
}

Push::~Push()
{
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

Repel::Repel( vector<OmfgScript::TokenBase*> const& params )
{
	// Sensible defaults?
	maxForce = params[0]->toDouble(0.0);
	maxDistance = params[1]->toDouble(0.0);
	minForce = params[2]->toDouble(0.0);

	maxDistanceSqr = maxDistance*maxDistance;
	forceDiffScaled = (minForce - maxForce) / maxDistance;
}

void Repel::run( ActionParams const& params )
{
/*
	float distance = ( params.object2->pos - params.object->pos ).length();
	if ( ( distance > 0 ) && ( distance < maxDistance ) )
	{
		params.object2->spd += ( params.object2->pos - params.object->pos ).normal() * ( maxForce + distance * ( minForce - maxForce ) / maxDistance );
	}
*/
	Vec dir( params.object2->pos - params.object->pos );
	double distanceSqr = dir.lengthSqr();
	if ( distanceSqr > 0.f && distanceSqr < maxDistanceSqr )
	{
		double distance = sqrt(distanceSqr);
		dir /= distance; // Normalize
		params.object2->spd += dir * ( maxForce + distance * forceDiffScaled);
	}
}

Repel::~Repel()
{
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

Damp::Damp( vector<OmfgScript::TokenBase*> const& params )
{
	factor = params[0]->toDouble(0.0);
}

void Damp::run( ActionParams const& params )
{
	params.object2->spd *= factor;
}

Damp::~Damp()
{
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

Damage::Damage( vector<OmfgScript::TokenBase*> const& params )
{
	m_damage = params[0]->toDouble(0.0);
	m_damageVariation = params[1]->toDouble(0.0);
	m_maxDistance = params[2]->toDouble(-1.0);
}

void Damage::run( ActionParams const& params )
{
	float damageAmount = m_damage + rnd() * m_damageVariation;
	if ( m_maxDistance > 0 )
	{
		float distance = ( params.object->pos - params.object2->pos ).length();
		if ( distance < m_maxDistance )
			damageAmount *= 1.0 - (distance / m_maxDistance);
		else
			damageAmount = 0;
	}
	params.object2->damage( damageAmount, params.object->getOwner() );
}

Damage::~Damage()
{
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

Remove::Remove( vector<OmfgScript::TokenBase*> const& params )
{
}

void Remove::run( ActionParams const& params )
{
	params.object->remove();
}

Remove::~Remove()
{
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

PlaySound::PlaySound( vector<OmfgScript::TokenBase*> const& params )
{
#ifndef DEDSERV
	if ( params[0]->isString() )
	{
		sounds.push_back( soundList.load(params[0]->toString()) );
	}else if(params[0]->assertList())
	{
		const_foreach(s, params[0]->toList())
		{
			if((*s)->assertString())
				sounds.push_back(soundList.load((*s)->toString()));
		}
	}
	loudness = params[1]->toDouble(100.0);
	pitch = params[2]->toDouble(1.0);
	pitchVariation = params[3]->toDouble(0.0);
#endif
}

void PlaySound::run( ActionParams const& params )
{
#ifndef DEDSERV
	if ( !sounds.empty() )
	{
		int sound = rndInt(sounds.size());
		if ( sounds[sound] )
		{
			sounds[sound]->play2D(params.object,loudness,pitch,pitchVariation);
		}
	}
#endif
}

PlaySound::~PlaySound()
{
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

PlaySoundStatic::PlaySoundStatic( vector<OmfgScript::TokenBase*> const& params )
{
#ifndef DEDSERV
	if ( params[0]->isString() )
	{
		sounds.push_back( soundList.load(params[0]->toString()) );
	}else if(params[0]->assertList())
	{
		const_foreach(s, params[0]->toList())
		{
			if((*s)->assertString())
				sounds.push_back(soundList.load((*s)->toString()));
		}
	}
	loudness = params[1]->toDouble(100.0);
	pitch = params[2]->toDouble(1.0);
	pitchVariation = params[3]->toDouble(0.0);
#endif
}


void PlaySoundStatic::run( ActionParams const& params )
{
#ifndef DEDSERV
	if ( !sounds.empty() )
	{
		int sound = rndInt(sounds.size());
		if ( sounds[sound] )
		{
			sounds[sound]->play2D(params.object->pos,loudness,pitch,pitchVariation);
		}
	}
#endif
}

PlaySoundStatic::~PlaySoundStatic()
{
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

PlayGlobalSound::PlayGlobalSound( vector<OmfgScript::TokenBase*> const& params )
{
#ifndef DEDSERV
	if ( params[0]->isString() )
	{
		sounds.push_back( sound1DList.load(params[0]->toString()) );
	}else if(params[0]->assertList())
	{
		const_foreach(s, params[0]->toList())
		{
			if((*s)->assertString())
				sounds.push_back(sound1DList.load((*s)->toString()));
		}
	}
	volume = params[1]->toDouble(1.0);
	volumeVariation = params[2]->toDouble(0.0);
	pitch = params[3]->toDouble(1.0);
	pitchVariation = params[4]->toDouble(0.0);
#endif
}

void PlayGlobalSound::run( ActionParams const& params )
{
#ifndef DEDSERV
	if ( !sounds.empty() )
	{
		int sound = rndInt(sounds.size());
		if ( sounds[sound] )
		{
			sounds[sound]->play( volume, pitch, volumeVariation, pitchVariation );
		}
	}
#endif
}

PlayGlobalSound::~PlayGlobalSound()
{
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

DelayFire::DelayFire( vector<OmfgScript::TokenBase*> const& params )
{
	delayTime = params[0]->toInt(0);
	delayTimeVariation = params[1]->toInt(0);
}

void DelayFire::run( ActionParams const& params )
{
	if(params.weapon)
	{
		params.weapon->delay( static_cast<int>(delayTime + rnd()*delayTimeVariation) );
	}
}

DelayFire::~DelayFire()
{
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

UseAmmo::UseAmmo( vector<OmfgScript::TokenBase*> const& params )
{
	amount = params[0]->toInt(1);
}

void UseAmmo::run( ActionParams const& params )
{
	if(params.weapon)
	{
		params.weapon->useAmmo(amount);
	}
}

UseAmmo::~UseAmmo()
{
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

ShowFirecone::ShowFirecone( vector<OmfgScript::TokenBase*> const& params )
{
#ifndef DEDSERV
	sprite = spriteList.load(params[0]->toString());
	frames = params[1]->toInt(0);
	drawDistance = params[2]->toDouble(0);
#endif
}

void ShowFirecone::run( ActionParams const& params )
{
#ifndef DEDSERV
	if( BaseWorm* w = dynamic_cast<BaseWorm*>(params.object) )
	{
		w->showFirecone( sprite, frames, drawDistance );
	}
#endif
}

ShowFirecone::~ShowFirecone()
{
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

AddAngleSpeed::AddAngleSpeed( vector<OmfgScript::TokenBase*> const& params )
{
	speed = AngleDiff(params[0]->toDouble(0.0));
	speedVariation = AngleDiff(params[1]->toDouble(0.0));
}

void AddAngleSpeed::run( ActionParams const& params )
{
	if (params.object)
	{
		params.object->addAngleSpeed( speed*params.object->getDir() + speedVariation*rnd() );
	}
}

AddAngleSpeed::~AddAngleSpeed()
{
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

AddSpeed::AddSpeed( vector<OmfgScript::TokenBase*> const& params )
{
	speed = params[0]->toDouble(0.0);
	speedVariation = params[1]->toDouble(0.0);
	offs = AngleDiff(params[2]->toDouble(0.0));
	offsVariation = Angle(params[3]->toDouble(0.0));
}

void AddSpeed::run( ActionParams const& params  )
{
	int dir = params.object->getDir();
	Angle angle(params.object->getAngle() + offs * dir);
	if(offsVariation) angle += offsVariation * midrnd();
	params.object->spd += Vec(angle, speed + midrnd()*speedVariation );
}

AddSpeed::~AddSpeed()
{
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

SetAlphaFade::SetAlphaFade( vector<OmfgScript::TokenBase*> const& params )
{
	frames = params[0]->toInt(0);
	dest = params[1]->toInt(0);
}

void SetAlphaFade::run( ActionParams const& params )
{
#ifndef DEDSERV
	if (params.object)
	{
		params.object->setAlphaFade( frames, dest );
	}
#endif
}

SetAlphaFade::~SetAlphaFade()
{
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

RunCustomEvent::RunCustomEvent( vector<OmfgScript::TokenBase*> const& params )
{
	index = params[0]->toInt(0);
}

void RunCustomEvent::run( ActionParams const& params )
{
	if (params.object2)
	{
		params.object2->customEvent( index );
	}
}

RunCustomEvent::~RunCustomEvent()
{
}


/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

RunScript::RunScript( vector<OmfgScript::TokenBase*> const& params )
//: function(0), scriptName(params[0]->toString())
: script(params[0]->toString())
{
	
}

void RunScript::run( ActionParams const& params )
{
	AssertStack as(lua);
	
	/*
	if(!function)
	{
		if(scriptName.empty())
			return;
		
		function = Script::functionFromString(scriptName);
		
		scriptName.clear();
		
		if(!function)
			return;
	}*/
	LuaReference f = script.get();
	if(!f)
		return;
	
	lua.push(LuaContext::errorReport);
	lua.pushReference(f);
	if(lua_isnil(lua, -1))
	{
		lua.pop(2);
		return;
	}
	
	if(params.object)
		params.object->pushLuaReference();
	else
		lua_pushnil(lua);
		
	if(params.object2)
		params.object2->pushLuaReference();
	else
		lua_pushnil(lua);
		
	if(lua.call(2, 0, -4) < 0)
	{
		//lua.destroyReference(function);
		//lua_pushnil(lua);
		//lua.assignReference(function);
		script.makeNil();
		//function.reset();
	}
	lua.pop(); // Pop error function
}

RunScript::~RunScript()
{
	//lua.destroyReference(function);
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

ApplyMapEffect::ApplyMapEffect( vector<OmfgScript::TokenBase*> const& params )
{
	effect = levelEffectList.load(params[0]->toString());
}

void ApplyMapEffect::run( ActionParams const& params )
{
	if ( effect )
		game.applyLevelEffect(effect, (int)params.object->pos.x, (int)params.object->pos.y);
}

ApplyMapEffect::~ApplyMapEffect()
{
}

