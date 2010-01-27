#ifndef OMFG_GUI_CHECK_H
#define OMFG_GUI_CHECK_H

#include "wnd.h"

namespace OmfgGUI
{

class Check : public Wnd
{
public:
	static LuaReference metaTable;
	
	Check(Wnd* parent_, /*std::string const& tagLabel, std::string const& className, 
	  std::string const& id,*/ std::map<std::string, std::string> const& properties/*,
	  std::string const& text_ = std::string("")*/)
	: Wnd(parent_, properties, "check"), m_checked(false)
	{

	}
	
	virtual bool render();
	
	virtual void process();

	virtual bool mouseDown(ulong newX, ulong newY, Context::MouseKey::type button);

	virtual bool mouseUp(ulong newX, ulong newY, Context::MouseKey::type button);
	
	virtual bool keyDown(int key);
	
	virtual void toggleState();
	virtual void applyGSS(Context::GSSselectors const& style);
	
	bool getState()
	{ return m_checked; }
	
private:
	bool m_checked;
};

}

#endif //OMFG_GUI_BUTTON_H

