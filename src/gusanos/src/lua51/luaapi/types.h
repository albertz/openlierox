#ifndef LUA_TYPES_H
#define LUA_TYPES_H

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
({ \
	LuaContext& lua_once_ = lua_; \
	lua_once_.push(t_::metaTable); \
	void* space_ = lua_once_.pushObject(sizeof(t_)); \
	t_* p_ = new (space_) t_ param_; \
	p_->luaReference = lua_once_.createReference(); \
	p_; \
})

#define lua_new_keep(t_, param_, lua_) \
({ \
	LuaContext& lua_once_ = lua_; \
	lua_once_.push(t_::metaTable); \
	void* space_ = lua_once_.pushObject(sizeof(t_)); \
	t_* p_ = new (space_) t_ param_; \
	lua_pushvalue(lua_once_, -1); \
	p_->luaReference = lua_once_.createReference(); \
	p_; \
})

#define lua_new_m(t_, param_, lua_, metatable_) \
({ \
	LuaContext& lua_once_ = lua_; \
	lua_once_.push(metatable_); \
	void* space_ = lua_once_.pushObject(sizeof(t_)); \
	t_* p_ = new (space_) t_ param_; \
	p_->luaReference = lua_once_.createReference(); \
	p_; \
})

#define lua_new_m_keep(t_, param_, lua_, metatable_) \
({ \
	LuaContext& lua_once_ = lua_; \
	lua_once_.push(metatable_); \
	void* space_ = lua_once_.pushObject(sizeof(t_)); \
	t_* p_ = new (space_) t_ param_; \
	lua_pushvalue(lua_once_, -1); \
	p_->luaReference = lua_once_.createReference(); \
	p_; \
})

#endif //LUA_TYPES_H
