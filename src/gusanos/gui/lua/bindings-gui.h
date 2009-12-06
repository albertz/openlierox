#ifndef LUA_BINDINGS_GUI_H
#define LUA_BINDINGS_GUI_H

#include "../../lua51/luaapi/context.h"
#include "../../lua51/luaapi/types.h"
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
