#ifndef OMFG_GUI_BUTTON_H
#define OMFG_GUI_BUTTON_H

#include "wnd.h"

namespace OmfgGUI
{

class Button : public Wnd
{
public:
	static LuaReference metaTable;
	
	Button(Wnd* parent_, /*std::string const& tagLabel, std::string const& className, 
	  std::string const& id,*/ std::map<std::string, std::string> const& properties/*,
	  std::string const& text_ = std::string("")*/)
	: Wnd(parent_, properties, "button"), m_state(false)
	{

	}
	
	virtual bool render();
	
	virtual void process();

	//virtual int classID();
	
private:
	bool m_state;
};

}

#endif //OMFG_GUI_BUTTON_H

