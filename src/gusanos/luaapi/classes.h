#ifndef LUA_CLASSES_H
#define LUA_CLASSES_H

#include "luaapi/context.h"
#include "util/log.h"

template<class T>
struct LuaID
{
};

#define CLASSID(name_, id_) \
template<> struct LuaID<name_> { static int const value = id_; }

class BaseObject;
class BaseWorm;
class Particle;
struct BITMAP;
class Viewport;
struct LuaGameEvent; // virtual
struct LuaPlayerEvent; // virtual
struct LuaWormEvent; // virtual
struct LuaParticleEvent; // virtual
struct Socket; // virtual
class ZCom_BitStream;
class BasePlayer;
class Weapon;
class PartType;
class WeaponType;
class Font;
class Sound;
class SpriteSet;

namespace OmfgGUI
{
class Wnd;
class List;
struct ListNode;
class Edit;
class Check;
class Label;
}

CLASSID(BaseObject, 1);
CLASSID(BaseWorm, 2);
CLASSID(Particle, 3);
CLASSID(BITMAP, 4);
CLASSID(Viewport, 5);
CLASSID(LuaGameEvent, 6);
CLASSID(LuaPlayerEvent, 7);
CLASSID(LuaWormEvent, 8);
CLASSID(LuaParticleEvent, 9);
CLASSID(Socket, 10);
CLASSID(ZCom_BitStream, 11);
CLASSID(BasePlayer, 12);
CLASSID(Weapon, 13);
CLASSID(PartType, 14);
CLASSID(WeaponType, 15);
CLASSID(Font, 16);
CLASSID(Sound, 17);
CLASSID(SpriteSet, 18);
CLASSID(OmfgGUI::Wnd, 19);
CLASSID(OmfgGUI::List, 20);
CLASSID(OmfgGUI::ListNode, 21);
CLASSID(OmfgGUI::Edit, 22);
CLASSID(OmfgGUI::Check, 23);
CLASSID(OmfgGUI::Label, 24);

#undef CLASSID

template<class T>
inline T* getObject(LuaContext& context, int idx)
{
	void* p = lua_touserdata(context, idx);
	if(!p)
		return 0;
	if(!lua_getmetatable(context, idx))
		return 0;
	lua_rawgeti(context, -1, LuaID<T>::value);
	bool b = lua_isnil(context, -1);
	context.pop(2);
	if(!b)
		return *static_cast<T**>(p);
	return 0;
}

template<class T>
inline T* getLObject(LuaContext& context, int idx)
{
	void* p = lua_touserdata(context, idx);
	if(!p)
		return 0;
	if(!lua_getmetatable(context, idx))
		return 0;
	lua_rawgeti(context, -1, LuaID<T>::value);
	bool b = lua_isnil(context, -1);
	context.pop(2);
	if(!b)
		return static_cast<T*>(p);
	return 0;
}

template<class T>
inline T* assertObject(LuaContext& context, int idx, char const* errstr)
{
	if(T* p = getObject<T>(context, idx))
		return p;
	
	lua_pushstring(context, errstr);
	lua_error(context);
	return 0;
}

template<class T>
inline T* assertLObject(LuaContext& context, int idx, char const* errstr)
{
	if(T* p = getLObject<T>(context, idx))
		return p;
	
	lua_pushstring(context, errstr);
	lua_error(context);
	return 0;
}

#define ASSERT_OBJECT(type_, idx_) assertObject<type_>(context, idx_, "Expected object of type " #type_ " as parameter " #idx_)

#define ASSERT_OBJECT_P(type_, idx_, place_) assertObject<type_>(context, idx_, "Expected object of type " #type_ place_)

#define ASSERT_LOBJECT(type_, idx_) assertLObject<type_>(context, idx_, "Expected object of type " #type_ " as parameter " #idx_)

#endif //LUA_CLASSES_H
