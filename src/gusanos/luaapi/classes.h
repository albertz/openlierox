#ifndef LUA_CLASSES_H
#define LUA_CLASSES_H

#include "context.h"
#include "util/log.h"
#include "CodeAttributes.h"
#include "game/ClassInfo.h"

template<class T> struct LuaID {};
template<uint32_t id> struct LuaClassInfo {
	static const char* name(uint32_t) { assert(false); return NULL; }
};

#define CLASSID(name_, id_) \
template<> struct LuaID<name_> { static uint32_t const value = id_; }; \
template<> struct LuaClassInfo<id_> { \
	static uint32_t const id = id_; \
	static const char* name() { return # name_; } \
	static const char* name(uint32_t id2_) { \
		if(id_ == id2_) return name(); \
		else if(id2_ > id_) { assert(false); return NULL; } \
		else return LuaClassInfo<id_-1>::name(id2_); \
	} \
}

class CGameObject;
class CWorm;
class CNinjaRope;
class Particle;
struct ALLEGRO_BITMAP;
class CViewport;
struct LuaGameEvent; // virtual
struct LuaPlayerEvent; // virtual
struct LuaWormEvent; // virtual
struct LuaParticleEvent; // virtual
struct Socket; // virtual
class BitStream;
class CWormInputHandler;
class Weapon;
class PartType;
class WeaponType;
class Font;
class Sound;
class SpriteSet;
class Settings;
class Game;

namespace OmfgGUI
{
class Wnd;
class List;
struct ListNode;
class Edit;
class Check;
class Label;
}

CLASSID(CGameObject, 1);
CLASSID(CWorm, 2);
CLASSID(Particle, 3);
CLASSID(ALLEGRO_BITMAP, 4);
CLASSID(CViewport, 5);
CLASSID(LuaGameEvent, 6);
CLASSID(LuaPlayerEvent, 7);
CLASSID(LuaWormEvent, 8);
CLASSID(LuaParticleEvent, 9);
CLASSID(Socket, 10);
CLASSID(BitStream, 11);
CLASSID(CWormInputHandler, 12);
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
CLASSID(CNinjaRope, 25);
CLASSID(Settings, 26);
CLASSID(Game, 27);

#undef CLASSID

INLINE const char* LuaClassName(uint32_t id_) {
	return LuaClassInfo<27>::name(id_);
}

template<class T>
INLINE T* getObject(LuaContext& context, int idx)
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
INLINE T* getLObject(LuaContext& context, int idx)
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
INLINE T* assertObject(LuaContext& context, int idx, char const* errstr)
{
	if(T* p = getObject<T>(context, idx))
		return p;
	
	lua_pushstring(context, errstr);
	lua_error(context);
	return 0;
}

template<class T>
INLINE T* assertLObject(LuaContext& context, int idx, char const* errstr)
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
