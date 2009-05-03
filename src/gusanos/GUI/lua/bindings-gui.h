#ifndef LUA_BINDINGS_GUI_H
#define LUA_BINDINGS_GUI_H

#include "luaapi/context.h"
#include "luaapi/types.h"
#include <vector>

namespace OmfgGUI
{
class Context;
}

namespace LuaBindings
{
	void initGUI(OmfgGUI::Context& gui, LuaContext& context);

	//extern std::vector<LuaReference> guiWndMetaTable;
}

#endif //LUA_BINDINGS_GUI_H
