#ifndef LUA_TYPES_H
#define LUA_TYPES_H

#include <cstring> //size_t

struct LuaReference
{
	LuaReference()
	: idx(0)
	{
	}
	
	explicit LuaReference(int idx_)
	: idx(idx_)
	{
	}
	
	operator bool()
	{
		return idx != 0;
	}
	
	void reset()
	{
		idx = 0;
	}
	
	int idx;
};


template<typename T, typename _Lua>
inline void* __lua_alloc(_Lua& lua_) {
	lua_.push(T::metaTable);
	return lua_.pushObject(sizeof(T));
}

template<typename T, typename _Lua, typename Meta>
inline void* __lua_alloc_meta(_Lua& lua_, Meta& metaTable) {
	lua_.push(metaTable);
	return lua_.pushObject(sizeof(T));
}

template<typename T, typename _Lua>
inline T* __lua_new_finalize(T* p, _Lua& lua_) {
	p->luaReference = lua_.createReference();
	return p;
}

template<typename T, typename _Lua>
inline T* __lua_new_finalize_keep(T* p, _Lua& lua_) {
	lua_pushvalue(lua_, -1);
	p->luaReference = lua_.createReference();
	return p;
}


#define LUA_NEW_(t_, param_, lua_) \
({ \
	LuaContext& lua_once_ = lua_; \
	lua_once_.pushRegObject(t_::metaTable); \
	void* space_ = lua_once_.pushObject(sizeof(t_)); \
	t_* p_ = new (space_) t_ param_; \
	p_->luaReference = lua_once_.createReference(); \
	p_; \
})

#define LUA_NEW_KEEP(t_, param_, lua_) \
({ \
	LuaContext& lua_once_ = lua_; \
	lua_once_.pushRegObject(t_::metaTable); \
	void* space_ = lua_once_.pushObject(sizeof(t_)); \
	t_* p_ = new (space_) t_ param_; \
	lua_pushvalue(lua_once_, -1); \
	p_->luaReference = lua_once_.createReference(); \
	p_; \
})

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
