#ifndef LUA_BINDINGS_GAME_H
#define LUA_BINDINGS_GAME_H

#include "../luaapi/context.h"
#include "../luaapi/types.h"

namespace LuaBindings
{
	void initGame(LuaContext& context);
	
	extern LuaReference playerIterator;
}

#endif //LUA_BINDINGS_GAME_H
