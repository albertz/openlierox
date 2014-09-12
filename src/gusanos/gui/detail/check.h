#ifndef OMFG_GUI_CHECK_H
#define OMFG_GUI_CHECK_H

#include "wnd.h"

namespace OmfgGUI
{

class Check : public Wnd
{
public:
	static LuaReference metaTable;
	
	Check(Wnd* parent_, std::map<std::string, std::string> const& properties)
	: Wnd(parent_, properties, "check"), m_checked(false)
	{}

	virtual void toggleState() { m_checked = !m_checked; }
	bool getState()	{ return m_checked; }
	
private:
	bool m_checked;
};

}

#endif //OMFG_GUI_BUTTON_H

