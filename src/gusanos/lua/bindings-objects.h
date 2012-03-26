#ifndef LUA_BINDINGS_OBJECTS_H
#define LUA_BINDINGS_OBJECTS_H

#include "../luaapi/types.h"

class LuaContext;

namespace LuaBindings
{
	void initObjects(LuaContext& ctx);
}

#endif //LUA_BINDINGS_OBJECTS_H
