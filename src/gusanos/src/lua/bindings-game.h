#ifndef LUA_BINDINGS_GAME_H
#define LUA_BINDINGS_GAME_H

#include "../lua51/luaapi/context.h"
#include "../lua51/luaapi/types.h"

namespace LuaBindings
{
	void initGame();
	
	extern LuaReference playerIterator;
	extern LuaReference BasePlayerMetaTable;
}

#endif //LUA_BINDINGS_GAME_H
