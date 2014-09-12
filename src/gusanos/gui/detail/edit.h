#ifndef OMFG_GUI_EDIT
#define OMFG_GUI_EDIT

#include "util/rect.h"
#include "util/common.h"
#include <string>
#include "wnd.h"

namespace OmfgGUI
{

class Edit : public Wnd
{
public:
	static LuaReference metaTable;
	
	Edit(Wnd* parent, std::map<std::string, std::string> const& attributes)
	: Wnd(parent, attributes, "edit")
	, m_drag(false), m_caretPos(0), m_selTo(0)
	, m_hscroll(0), m_lock(false)
	{}
	
	void setCaretPos(ulong caretPos)
	{
		if(m_caretPos != caretPos)
		{
			m_caretPos = caretPos;
		}
	}
	
	void setSelTo(ulong selTo)
	{
		if(m_selTo != selTo)
		{
			m_selTo = selTo;
		}
	}
	
	void setLock(bool lock)
	{
		m_lock = lock;
	}
	
	virtual void setText(std::string const& aStr);
		
	void assertCaretVisibility(Renderer* renderer);
		
	virtual int classID();
	
protected:

	bool        m_drag;
	ulong		m_caretPos;
	ulong		m_selTo;
	long        m_hscroll;
	bool        m_lock;
};

}

#endif

