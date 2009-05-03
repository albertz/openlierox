#ifndef LUA_MACROS_H
#define LUA_MACROS_H


#define METHOD(type_, name_, body_...) \
	int l_##name_(lua_State* L_) { \
		LuaContext context(L_); \
		void* p2 = lua_touserdata (context, 1); \
		if(!p2) return 0; \
		type_* p = *static_cast<type_ **>(p2); \
		body_ }
		
#define METHODC(type_, name_, body_...) \
	int l_##name_(lua_State* L_) { \
		LuaContext context(L_); \
		if(type_* p = getObject<type_>(context, 1)) { \
		body_ } else { \
		lua_pushstring(context, "Method called on invalid object. Did you use '.' instead of ':'?"); \
		lua_error(context); return 0; }  }
			
#define LMETHOD(type_, name_, body_...) \
	int l_##name_(lua_State* L_) { \
		LuaContext context(L_); \
		type_* p = static_cast<type_ *>(lua_touserdata (context, 1)); \
		if(!p) return 0; \
		body_ }
		
#define LMETHODC(type_, name_, body_...) \
	int l_##name_(lua_State* L_) { \
		LuaContext context(L_); \
		if(type_* p = getLObject<type_>(context, 1)) { \
		body_ } else { \
		lua_pushstring(context, "Method called on invalid object. Did you use '.' instead of ':'?"); \
		lua_error(context); return 0; }  }
		
#define BINOP(type_, name_, body_...) \
	int l_##name_(lua_State* L_) { \
		LuaContext context(L_); \
		void* a_ = lua_touserdata (context, 1); \
		if(!a_) return 0; \
		void* b_ = lua_touserdata (context, 2); \
		if(!b_) return 0; \
		type_* a = *static_cast<type_ **>(a_); \
		type_* b = *static_cast<type_ **>(b_); \
		body_ }
		
#define LBINOP(type_, name_, body_...) \
	int l_##name_(lua_State* L_) { \
		LuaContext context(L_); \
		type_* a = static_cast<type_ *>(lua_touserdata (context, 1)); \
		if(!a) return 0; \
		type_* b = static_cast<type_ *>(lua_touserdata (context, 2)); \
		if(!b) return 0; \
		body_ }

#define CLASS(name_, body_...) { \
	lua_newtable(context); \
	lua_pushstring(context, "__index"); \
	lua_newtable(context); \
	context.tableFunctions() \
		body_ ; \
	lua_rawset(context, -3); \
	context.tableSetField(LuaID<name_>::value); \
	name_##MetaTable = context.createReference(); }
	
/*
#define CLASSID(name_, id_, body_) { \
	lua_newtable(context); \
	lua_pushstring(context, "__index"); \
	lua_newtable(context); \
	context.tableFunctions() \
		body_ ; \
	context.tableSetField(id_); \
	lua_rawset(context, -3); \
	name_##MetaTable = context.createReference(); }
	*/
#define CLASS_(name_, body_...) { \
	lua_newtable(context); \
	lua_pushstring(context, "__index"); \
	lua_newtable(context); \
	context.tableFunctions() \
		body_ ; \
	lua_rawset(context, -3); \
	context.tableSetField(LuaID<name_>::value); \
	name_::metaTable = context.createReference(); }
	
#define CLASSM(name_, meta_, body_...) { \
	lua_newtable(context); \
	context.tableFunctions() \
		meta_ ; \
	lua_pushstring(context, "__index"); \
	lua_newtable(context); \
	context.tableFunctions() \
		body_ ; \
	lua_rawset(context, -3); \
	context.tableSetField(LuaID<name_>::value); \
	name_##MetaTable = context.createReference(); }
	
#define CLASSM_(name_, meta_, body_...) { \
	lua_newtable(context); \
	context.tableFunctions() \
		meta_ ; \
	lua_pushstring(context, "__index"); \
	lua_newtable(context); \
	context.tableFunctions() \
		body_ ; \
	lua_rawset(context, -3); \
	context.tableSetField(LuaID<name_>::value); \
	name_::metaTable = context.createReference(); }

#define ENUM(name_, body_) { \
	lua_pushstring(context, #name_); \
	lua_newtable(context); \
	context.tableItems() \
		body_ ; \
	lua_rawset(context, LUA_GLOBALSINDEX); }
	
#define REQUEST_TABLE(name_, func_) { \
	lua_pushstring(context, name_); \
	lua_newtable(context); \
	lua_newtable(context); \
	lua_pushstring(context, "__index"); \
	lua_pushcfunction(context, func_); \
	lua_rawset(context, -3); \
	lua_setmetatable(context, -2); \
	lua_rawset(context, LUA_GLOBALSINDEX); }
	
#define SHADOW_TABLE(name_, get_, set_) { \
	lua_pushstring(context, name_); \
	lua_newtable(context); \
	lua_newtable(context); \
	lua_pushstring(context, "__index"); \
	lua_pushcfunction(context, get_); \
	lua_rawset(context, -3); \
	lua_pushstring(context, "__newindex"); \
	lua_pushcfunction(context, set_); \
	lua_rawset(context, -3); \
	lua_setmetatable(context, -2); \
	lua_rawset(context, LUA_GLOBALSINDEX); }
	


#endif
