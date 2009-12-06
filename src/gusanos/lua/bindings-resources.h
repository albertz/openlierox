#ifndef LUA_BINDINGS_RESOURCES_H
#define LUA_BINDINGS_RESOURCES_H

#include "../lua51/luaapi/types.h"

namespace LuaBindings
{
	void initResources();
	
	extern LuaReference FontMetaTable;
	//extern LuaReference PartTypeMetaTable;
	//extern LuaReference WeaponTypeMetaTable;
	extern LuaReference SpriteSetMetaTable;
}

#endif //LUA_BINDINGS_RESOURCES_H
