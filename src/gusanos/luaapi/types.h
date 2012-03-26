#ifndef LUA_TYPES_H
#define LUA_TYPES_H

#include <cstring> //size_t
#include <boost/shared_ptr.hpp>
#include <map>
#include "CodeAttributes.h"
extern "C" {
#include "lua.h"
}
#include "util/WeakRef.h"

class LuaContext;
class BaseObject;

class LuaReference
{
public:
	typedef int Idx;
private:
	typedef std::map<WeakRef<lua_State>, Idx> IdxMap;
	boost::shared_ptr<IdxMap> idxs;
	void cleanup();

public:
	LuaReference() { idxs.reset(new IdxMap); }

	void create(LuaContext& ctx); // [-1,0]
	void push(LuaContext& ctx) const; // [0,1]
	bool isSet(const LuaContext& ctx) const;

	void destroy();
	void invalidate();
};

struct LuaReferenceLazy {
	WeakRef<BaseObject> obj;
	LuaReference ref;
	LuaReference get(LuaContext& ctx);
};


template<typename T, typename _Lua>
INLINE void* __lua_alloc(_Lua& lua_) {
	lua_.push(T::metaTable);
	return lua_.pushObject(sizeof(T));
}

template<typename T, typename _Lua, typename Meta>
INLINE void* __lua_alloc_meta(_Lua& lua_, Meta& metaTable) {
	lua_.push(metaTable);
	return lua_.pushObject(sizeof(T));
}

template<typename T, typename _Lua>
INLINE T* __lua_new_finalize(T* p, _Lua& lua_) {
	p->luaReference.create(lua_);
	return p;
}

template<typename T, typename _Lua>
INLINE T* __lua_new_finalize_keep(T* p, _Lua& lua_) {
	lua_pushvalue(lua_, -1);
	p->luaReference.create(lua_);
	return p;
}


#define lua_new(t_, param_, lua_) \
	__lua_new_finalize<t_>( new ( __lua_alloc<t_>(lua_) ) t_ param_, lua_ )

#define lua_new_keep(t_, param_, lua_) \
	__lua_new_finalize_keep<t_>( new ( __lua_alloc<t_>(lua_) ) t_ param_, lua_ )

#define lua_new_m(t_, param_, lua_, metatable_) \
	__lua_new_finalize<t_>( new ( __lua_alloc_meta<t_>(lua_, metatable_) ) t_ param_, lua_ )

#define lua_new_m_keep(t_, param_, lua_, metatable_) \
	__lua_new_finalize_keep<t_>( new ( __lua_alloc_meta<t_>(lua_, metatable_) ) t_ param_, lua_ )

template <typename _Context>
void *lua_newuserdata_init(_Context& lua, size_t size)  {
	void *res = lua_newuserdata(lua, size);
	memset(res, 0, size);
	return res;
}

#endif //LUA_TYPES_H
