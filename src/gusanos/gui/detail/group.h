#ifndef OMFG_GUI_GROUP_H
#define OMFG_GUI_GROUP_H

#include "wnd.h"

namespace OmfgGUI
{

class Group : public Wnd
{
public:
	static LuaReference metaTable;
	
	Group(Wnd* parent_, /*std::string const& tagLabel, std::string const& className, 
	  std::string const& id, */std::map<std::string, std::string> const& properties,
	  std::string const& text_ = std::string(""))
	: Wnd(parent_, properties, "group")
	{

	}
	
	virtual bool render();
	
	virtual void process();

	virtual int classID();

};

}

#endif //OMFG_GUI_GROUP_H

