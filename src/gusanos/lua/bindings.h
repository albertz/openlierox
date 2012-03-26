#ifndef LUA_BINDINGS_H
#define LUA_BINDINGS_H

#include "../luaapi/context.h"
#include "../luaapi/types.h"
#include <vector>
#include <string>
#include <list>

namespace LuaBindings
{
	void init(LuaContext& ctx);
}


#define LUA_ABORT() \
do { context.pop(2 + params); return ""; } while(0)

#define LUA_CALLBACK(sign_) \
std::string sign_ { \
AssertStack as(context); \
context.push(LuaContext::errorReport); \
context.push(ref); \
if(lua_isnil(context, -1)) { \
	context.pop(2); \
	return ""; } \
int params = 0;

#define END_LUA_CALLBACK() \
int r = context.call(params, 1, -params-2); \
if(r < 0) { \
	ref.invalidate(); \
	context.pop(1); \
	return ""; } \
lua_remove(context, -1-1); \
if(char const* s = lua_tostring(context, -1)) { \
	std::string ret(s); \
	context.pop(1); \
	return ret; } \
context.pop(1); \
return ""; }

#endif //LUA_BINDINGS_H
