#ifndef LUA_BINDINGS_RESOURCES_H
#define LUA_BINDINGS_RESOURCES_H

#include "../luaapi/types.h"

class LuaContext;

namespace LuaBindings
{
	void initResources(LuaContext& context);
	
	extern LuaReference FontMetaTable;
	extern LuaReference SpriteSetMetaTable;
}

#endif //LUA_BINDINGS_RESOURCES_H
