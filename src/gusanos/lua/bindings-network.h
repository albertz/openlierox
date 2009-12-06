#ifndef LUA_BINDINGS_NETWORK_H
#define LUA_BINDINGS_NETWORK_H

#include "../lua51/luaapi/context.h"
#include "../lua51/luaapi/types.h"

namespace LuaBindings
{
	extern LuaReference Net_BitStreamMetaTable;

	void initNetwork(LuaContext& context);
}

#endif //LUA_BINDINGS_NETWORK_H
