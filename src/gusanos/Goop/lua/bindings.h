#ifndef LUA_BINDINGS_H
#define LUA_BINDINGS_H

#include "luaapi/context.h"
#include "luaapi/types.h"
#include <vector>
#include <string>
#include <list>

/*
class BaseObject;
class BasePlayer;
class BaseWorm;
class Viewport;
*/

namespace LuaBindings
{
	void init();

	//int print(lua_State* state);
	
	//std::string runLua(LuaReference ref, std::list<std::string> const& args);
	
}


#define LUA_ABORT() \
do { lua.pop(2 + params); return ""; } while(0)

#define LUA_CALLBACK(sign_) \
std::string sign_ { \
AssertStack as(lua); \
lua.push(LuaContext::errorReport); \
lua.pushReference(ref); \
if(lua_isnil(lua, -1)) { \
	lua.pop(2); \
	return ""; } \
int params = 0;

#define END_LUA_CALLBACK() \
int r = lua.call(params, 1, -params-2); \
if(r < 0) { \
	lua_pushnil(lua); \
	lua.assignReference(ref); \
	lua.pop(1); \
	return ""; } \
lua_remove(lua, -1-1); \
if(char const* s = lua_tostring(lua, -1)) { \
	std::string ret(s); \
	lua.pop(1); \
	return ret; } \
lua.pop(1); \
return ""; }

#endif //LUA_BINDINGS_H
