#ifndef LUA_BINDINGS_GFX_H
#define LUA_BINDINGS_GFX_H

#include "luaapi/context.h"
#include "luaapi/types.h"

#ifndef DEDSERV
#include "../blitters/context.h"
#endif

namespace LuaBindings
{
	void initGfx();

#ifndef DEDSERV
	extern LuaReference ViewportMetaTable;
	extern LuaReference BITMAPMetaTable;
	extern BlitterContext blitter;
#endif
}

#endif //LUA_BINDINGS_GFX_H
