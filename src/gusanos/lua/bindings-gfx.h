#ifndef LUA_BINDINGS_GFX_H
#define LUA_BINDINGS_GFX_H

#include "../luaapi/context.h"
#include "../luaapi/types.h"

#ifndef DEDICATED_ONLY
#include "../blitters/context.h"
#endif

namespace LuaBindings
{
	void initGfx();

#ifndef DEDICATED_ONLY
	extern LuaReference CViewportMetaTable;
	extern LuaReference ALLEGRO_BITMAPMetaTable;
	extern BlitterContext blitter;
#endif
}

#endif //LUA_BINDINGS_GFX_H
