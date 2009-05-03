#include "bindings-objects.h"

#include "bindings-resources.h"
#include "luaapi/types.h"
#include "luaapi/macros.h"
#include "luaapi/classes.h"

#include "../base_player.h"
#include "../player.h"
#include "../base_worm.h"
#include "../particle.h"
#include "../weapon.h"
#include "../weapon_type.h"
#include "../game.h"
#include "../glua.h"
#include "util/log.h"

#include <cmath>
#include <iostream>
#include <allegro.h>
using std::cerr;
using std::endl;
#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>
using boost::lexical_cast;

namespace LuaBindings
{

//LuaReference wormMetaTable;
//LuaReference baseObjectMetaTable;
//LuaReference particleMetaTable;
LuaReference WeaponMetaTable;

namespace ParticleRep
{
	enum type
	{
		Position
	};
}

int shootFromObject(lua_State* L, BaseObject* object)
{
	/*
	void* typeP = lua_touserdata (L, 2);
	if(!typeP)
		return 0;
	PartType* p = *static_cast<PartType **>(typeP);
	*/
	LuaContext context(L);
	PartType* p = ASSERT_OBJECT(PartType, 2);
	
	int amount = 1;
	int amountVariation = 0;
	lua_Number speed = 0;
	lua_Number speedVariation = 0;
	lua_Number motionInheritance = 0;
	lua_Number distanceOffset = 0;
	AngleDiff distribution(360.0);
	AngleDiff angleOffset(0.0);
	
	int params = lua_gettop(L);
	switch(params)
	{
		default: if(params < 3) return 0;
		case 10: distanceOffset = lua_tonumber(L, 10);
		case 9:  angleOffset = AngleDiff(lua_tonumber(L, 9));
		case 8:  distribution = AngleDiff(lua_tonumber(L, 8));
		case 7:  amountVariation = lua_tointeger(L, 7);
		case 6:  motionInheritance = lua_tonumber(L, 6);
		case 5:  speedVariation = lua_tonumber(L, 5);
		case 4:  speed = lua_tonumber(L, 4);
		case 3:  amount = lua_tointeger(L, 3);
	}

	char dir = object->getDir();
	Angle baseAngle(object->getAngle() + angleOffset * dir);
	
	BaseObject* last = 0;
	
	int realAmount = amount + rndInt(amountVariation); // int(rnd()*amountVariation);
	for(int i = 0; i < realAmount; ++i)
	{
		Angle angle = baseAngle + distribution * midrnd();
		Vec direction(angle);
		Vec spd(direction * (speed + midrnd()*speedVariation));
		if(motionInheritance)
		{
			spd += object->spd * motionInheritance;
			angle = spd.getAngle(); // Need to recompute angle
		}
		//game.insertParticle( new Particle( p, object->getPos() + direction * distanceOffset, spd, object->getDir(), object->getOwner(), angle ));
		last = p->newParticle(p, object->pos + direction * distanceOffset, spd, object->getDir(), object->getOwner(), angle);
	}
	
	if(last)
	{
		last->pushLuaReference();
		return 1;
	}
	
	return 0;
}

//! Worm inherits Object

/*
int l_worm_getPlayer(lua_State* L)
{
	BaseWorm* p = static_cast<BaseWorm *>(lua_touserdata (L, 1));
	if(!p->getOwner())
		return 0;
	lua.pushReference(p->getOwner()->luaReference);
	return 1;
}
*/

/*! Worm:health()

	(Known as get_health before 0.9c)
	Returns the health of this worm.
*/
METHODC(BaseWorm, worm_getHealth,
	context.push(p->getHealth());
	return 1;
)

#ifndef NO_DEPRECATED
int l_worm_getHealth_depr(lua_State* L)
{
	LuaContext context(L);
	LUA_WLOG_ONCE("get_health is deprecated, use the health method instead");
	return l_worm_getHealth(L);
}
#endif

METHODC(BaseWorm, worm_isChanging,
	context.push(p->isChanging());
	return 1;
)


/*@ Worm:set_weapon(slot, weapon)

	Sets the weapon of a slot in [1, max] to the WeaponType //weapon//.
*/
/* TODO
LMETHOD(BaseWorm, worm_setWeapon,
	p->m_weapons*/
	
/*
int l_worm_remove(lua_State* L)
{
	BaseWorm* p = static_cast<BaseWorm *>(lua_touserdata (L, 1));
	p->deleteMe = true;
	return 0;
}

int l_worm_pos(lua_State* L)
{
	BaseWorm* p = static_cast<BaseWorm *>(lua_touserdata (L, 1));
	lua_pushnumber(L, p->pos.x);
	lua_pushnumber(L, p->pos.y);
	return 2;
}

int l_worm_spd(lua_State* L) //
{
	BaseWorm* p = static_cast<BaseWorm *>(lua_touserdata (L, 1));
	lua_pushnumber(L, p->spd.x);
	lua_pushnumber(L, p->spd.y);
	return 2;
}

int l_worm_push(lua_State* L) //
{
	BaseWorm* p = static_cast<BaseWorm *>(lua_touserdata (L, 1));
	p->spd.x += lua_tonumber(L, 2);
	p->spd.y += lua_tonumber(L, 3);
	return 0;
}

int l_worm_data(lua_State* L)
{
	BaseWorm* p = static_cast<BaseWorm *>(lua_touserdata (L, 1));
	if(p->luaData)
	{
		lua.pushReference(p->luaData);
	}
	else
	{
		lua_newtable(L);
		lua_pushvalue(L, -1);
		p->luaData = lua.createReference();
	}
	
	return 1;
}

int l_worm_shoot(lua_State* L)
{
	BaseWorm* object = static_cast<BaseWorm *>(lua_touserdata (L, 1));
	
	return shootFromObject(L, object);
}*/

METHODC(BaseWorm, worm_current_weapon,
	if(Weapon* w = p->getCurrentWeapon())
	{
		//context.pushFullReference(*w, WeaponMetaTable);
		w->pushLuaReference();
		return 1;
	}
	return 0;
)

/*
LBINOP(BaseWorm, worm_eq,
	context.push(a == b);
	return 1;
)*/

METHOD(BaseWorm, worm_destroy,
	delete p;
	return 0;
)

/*! Object:angle()

	(Known as get_angle before 0.9c)
	Returns the current angle of the object.
*/

METHODC(BaseObject, baseObject_getAngle,
	lua_pushnumber(context, p->getAngle().toDeg());
	return 1;
)

#ifndef NO_DEPRECATED
int l_baseObject_getAngle_depr(lua_State* L)
{
	LuaContext context(L);
	LUA_WLOG_ONCE("get_angle is deprecated, use the angle method instead");
	return l_baseObject_getAngle(L);
}
#endif

/*! Object:remove()

	Removes the object in the next frame.
*/
METHODC(BaseObject, baseObject_remove,
	p->deleteMe = true;
	return 0;
)

/*! Object:pos()

	Returns the position of this object as a tuple.
	i.e.:
	<code>
	local x, y = object:pos()
	</code>
*/

METHODC(BaseObject, baseObject_pos,
	context.push(p->pos.x);
	context.push(p->pos.y);
	return 2;
)

/*! Object:set_pos(x, y)

	Moves the object to the location (x, y)
	e.g.:
	<code>
	object:set_spd(0, 0) -- Moves the object to the upper-left corner
	</code>
*/
METHODC(BaseObject, baseObject_setPos,
	p->setPos(Vec(lua_tonumber(context, 2), lua_tonumber(context, 3))); 
	return 0;
)

/*! Object:spd()

	Returns the speed of this object as a tuple.
	e.g.:
	<code>
	local vx, vy = object:spd()
	</code>
*/
METHODC(BaseObject, baseObject_spd,
	context.push(p->spd.x);
	context.push(p->spd.y);
	return 2;
)

//! version 0.9c

/*! Object:set_spd(x, y)

	Changes the speed of this object to (x, y)
	e.g.:
	<code>
	object:set_spd(10, 0) -- Makes the object move to the right
	</code>
*/
METHODC(BaseObject, baseObject_setSpd,
	p->spd.x = lua_tonumber(context, 2);
	p->spd.y = lua_tonumber(context, 3); 
	return 0;
)

//! version any

/*! Object:push(x, y)
	
	Accelerates the object in the direction (x, y)
	e.g.:
	<code>
	object:push(0, 10) -- Accelerates the object downwards
	</code>
*/
METHODC(BaseObject, baseObject_push,
	p->spd.x += lua_tonumber(context, 2);
	p->spd.y += lua_tonumber(context, 3);
	return 0;
)

/*! Object:data()

	Returns a table associated with this object that can
	be used by Lua scripts to store values.
*/

METHODC(BaseObject, baseObject_data,
	if(p->luaData)
	{
		lua.pushReference(p->luaData);
	}
	else
	{
		lua_newtable(context);
		context.pushvalue(-1);
		p->luaData = lua.createReference();
	}
	
	return 1;
)

/*! Object:player()

	(Known as get_player before 0.9c)
	
	Returns a Player object of the player that owns this object.
*/
METHODC(BaseObject, baseObject_getPlayer,
	if(!p->getOwner())
		return 0;
	lua.pushReference(p->getOwner()->getLuaReference());
	return 1;
)

#ifndef NO_DEPRECATED
int l_baseObject_getPlayer_depr(lua_State* L)
{
	LuaContext context(L);
	LUA_WLOG_ONCE("get_player is deprecated, use the player method instead");
	return l_baseObject_getPlayer(L);
}
#endif

//! version 0.9c

/*! Object:damage(amount[, player])

	Causes damage to the object. //player// is the player inflicting the damage.
	If //player// isn't specified or nil, the damage is anonymous.
*/
METHODC(BaseObject, baseObject_damage,
	lua_Number amount = lua_tonumber(context, 2);
	//BasePlayer* player = *static_cast<BasePlayer **>(lua_touserdata(context, 3));
	BasePlayer* player = getObject<BasePlayer>(context, 3);
	p->damage(amount, player);
	return 1;
)

//! version any

/*! Object:closest_worm()

	(Known as get_closest_worm before 0.9c)

	Returns the closest worm that fulfills these requirements:
	
	  * It is not the owner of the object.
	  * It is visible and active.
	  * The straight path to it from the object is not, for particles, blocked.
*/
METHODC(BaseObject, baseObject_getClosestWorm,

	Vec from = p->pos;
	
	int fromx = int(from.x);
	int fromy = int(from.y);
	
	BaseWorm* minWorm = 0;
	float minDistSqr = 10000000.f;
	
	for(std::list<BasePlayer*>::iterator playerIter = game.players.begin(); playerIter != game.players.end(); ++playerIter)
	{
		if(p->getOwner() != *playerIter)
		{
			BaseWorm* worm = (*playerIter)->getWorm();
		
			if(worm->isActive())
			//if(worm->isActive())
			{
				float distSqr = (worm->pos - from).lengthSqr();
				if(distSqr < minDistSqr && !game.level.trace(fromx, fromy, int(worm->pos.x), int(worm->pos.y), Level::ParticleBlockPredicate()))
				{
					minDistSqr = distSqr;
					minWorm = worm;
				}
			}
		}
	}
	
	if(!minWorm)
		return 0;
		
	minWorm->pushLuaReference();

	return 1;
)

#ifndef NO_DEPRECATED
int l_baseObject_getClosestWorm_depr(lua_State* L)
{
	LuaContext context(L);
	LUA_WLOG_ONCE("get_closest_worm is deprecated, use the closest_worm method instead");
	return l_baseObject_getClosestWorm(L);
}
#endif

/*! Object:shoot(type, amount, speed, speedVariation, motionInheritance, amountVariation, distribution, angleOffset, distanceOffset)

	Shoots an object of ParticleType 'type'. All parameters except 'type' are optional.
*/

METHODC(BaseObject, baseObject_shoot, 
	return shootFromObject(context, p);
)

/*

METHOD(BaseObject, baseObject_getAngle,
	lua_pushnumber(context, p->getAngle().toDeg());
	return 1;
)
*/

//! Particle inherits Object

/*! Particle:set_angle(angle)

	Changes the angle of the particle to //angle//.
*/

METHODC(Particle, particle_setAngle,
	p->setAngle(Angle((double)lua_tonumber(context, 2)));
	return 0;
)

//! version 0.9c

/*! Particle:set_replication(type, state)

	If the particle is network-aware, this function turns on or off replication
	of different aspects of the particle's state.
	
	Note that this function can only affects things for
	which replication is activated in the omfgScript of the particle type.
	
	Since the server has authority over particle replication, this function will
	have no affect on clients.
	
	//type// can be one of the following:
	Particle.Position : replication of position.
	
	//state// is either true or false, where true turns on replication and false turns it off.
*/

METHODC(Particle, particle_set_replication,
	
	int mask = 0;
	switch(lua_tointeger(context, 2))
	{
		case ParticleRep::Position:
			mask = Particle::RepPos;
		break;
		default:
			return 0;
	}
	
	if(lua_toboolean(context, 3))
		p->setFlag(mask);
	else
		p->resetFlag(mask);

	return 0;
)

//! version any

METHOD(Particle, particle_destroy,
	delete p;
	return 0;
)

/*! Weapon:is_reloading()

	Returns true if this weapon is reloading.
*/

METHODC(Weapon, weaponinst_reloading,
	context.push(p->reloading);
	return 1;
)

/*! Weapon:reload_time()

	Returns the reload time left on this weapon.
	Does only make sense if is_reloading() is true.
*/
METHODC(Weapon, weaponinst_reload_time,
	context.push(p->getReloadTime());
	return 1;
)

/*! Weapon:ammo()

	Returns the amount of ammo left in this weapon.
*/
METHODC(Weapon, weaponinst_ammo,
	context.push(p->getAmmo());
	return 1;
)

/*! Weapon:type()

	Returns the weapon type in the form of a WeaponType object.
*/
METHODC(Weapon, weaponinst_type,
	//context.pushFullReference(*p->getType(), WeaponTypeMetaTable);
	p->getType()->pushLuaReference();
	return 1;
)

METHOD(Weapon, weaponinst_destroy,
	assert(!p->luaReference);
	delete p;
	return 1;
)

void addBaseObjectFunctions(LuaContext& context)
{
	context.tableFunctions()
		("remove", l_baseObject_remove)
		("pos", l_baseObject_pos)
		("spd", l_baseObject_spd)
		("push", l_baseObject_push)
#ifndef NO_DEPRECATED
		("get_player", l_baseObject_getPlayer_depr)
		("get_closest_worm", l_baseObject_getClosestWorm_depr)
		("get_angle", l_baseObject_getAngle_depr)
#endif
		("player", l_baseObject_getPlayer)
		("closest_worm", l_baseObject_getClosestWorm)
		("data", l_baseObject_data)
		("shoot", l_baseObject_shoot)
		("angle", l_baseObject_getAngle)
		("set_pos", l_baseObject_setPos)
		("set_spd", l_baseObject_setSpd)
		("damage", l_baseObject_damage)
	;
}

void initObjects()
{
	LuaContext& context = lua;
	
	// BaseObject method and metatable
	lua_newtable(context); 
	lua_pushstring(context, "__index");
	
	lua_newtable(context);
	
	addBaseObjectFunctions(context);
	
	lua_rawset(context, -3);
	context.tableSetField(LuaID<BaseObject>::value);
	BaseObject::metaTable = context.createReference();
	
	// Particle method and metatable
	
	lua_newtable(context);
	context.tableFunctions()
		("__gc", l_particle_destroy)
	;
	lua_pushstring(context, "__index");
	
	lua_newtable(context);
	
	addBaseObjectFunctions(context);

	context.tableFunctions()
		("set_angle", l_particle_setAngle)
		("set_replication", l_particle_set_replication)
	;
	
	lua_rawset(context, -3);
	context.tableSetField(LuaID<Particle>::value);
	context.tableSetField(LuaID<BaseObject>::value);
	Particle::metaTable = context.createReference();
	
	// Worm method and metatable
	
	lua_newtable(context);
	context.tableFunctions()
		("__gc", l_worm_destroy)
	;
	lua_pushstring(context, "__index");
	
	lua_newtable(context);
	
	addBaseObjectFunctions(context); // BaseWorm inherits from BaseObject

	context.tableFunctions()
#ifndef NO_DEPRECATED
		("get_health", l_worm_getHealth_depr)
#endif
		("health", l_worm_getHealth)
		("current_weapon", l_worm_current_weapon)
		("is_changing", l_worm_isChanging)
	;
	
	lua_rawset(context, -3);
	context.tableSetField(LuaID<BaseWorm>::value);
	context.tableSetField(LuaID<BaseObject>::value);
	BaseWorm::metaTable = context.createReference();
	
	CLASSM_(Weapon,
		("__gc", l_weaponinst_destroy)
	,
		("is_reloading", l_weaponinst_reloading)
		("reload_time", l_weaponinst_reload_time)
		("ammo", l_weaponinst_ammo)
		("type", l_weaponinst_type)
	)
	
	ENUM(Particle,
		("Position", ParticleRep::Position)
	)
}

}
