#ifndef OMFG_GUI_LABEL_H
#define OMFG_GUI_LABEL_H

#include "wnd.h"

namespace OmfgGUI
{

class Label : public Wnd
{
public:
	static LuaReference metaTable;
	
	Label(Wnd* parent_, std::map<std::string, std::string> const& properties)
	: Wnd(parent_, properties, "label")
	{}
	
};

}

#endif //OMFG_GUI_BUTTON_H

