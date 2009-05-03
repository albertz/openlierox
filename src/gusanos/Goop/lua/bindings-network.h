#ifndef LUA_BINDINGS_NETWORK_H
#define LUA_BINDINGS_NETWORK_H

#include "luaapi/context.h"
#include "luaapi/types.h"

namespace LuaBindings
{
	extern LuaReference ZCom_BitStreamMetaTable;

	void initNetwork(LuaContext& context);
}

#endif //LUA_BINDINGS_NETWORK_H
