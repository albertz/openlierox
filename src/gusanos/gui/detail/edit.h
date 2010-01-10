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
	
	Edit(Wnd* parent, /*std::string const& tagLabel, std::string const& className,
	  std::string const& id,*/ std::map<std::string, std::string> const& attributes/*,
	  std::string const& text = std::string("")*/)
	: Wnd(parent, attributes, "edit")
	, m_drag(false), m_caretPos(0), m_selTo(0)
	, m_hscroll(0), m_lock(false)
	{

	}
	
	virtual bool render();
	
	virtual void process();

	void setCaretPos(ulong caretPos)
	{
		if(m_caretPos != caretPos)
		{
			m_caretPos = caretPos;
			//InvalidateWhole();
		}
	}
	
	void setSelTo(ulong selTo)
	{
		if(m_selTo != selTo)
		{
			m_selTo = selTo;
			//InvalidateWhole();
		}
	}
	
	void setLock(bool lock)
	{
		m_lock = lock;
	}
	
	virtual void setText(std::string const& aStr);
	
	virtual bool keyDown(int key);
	//virtual bool keyUp(int key);
	
	virtual bool charPressed(char c, int key);
	
	virtual bool mouseDown(ulong x, ulong y, Context::MouseKey::type button);
	
	virtual bool mouseUp(ulong x, ulong y, Context::MouseKey::type button);
	
	virtual bool mouseMove(ulong x, ulong y);
	
	void assertCaretVisibility(Renderer* renderer);
	
	/*
	virtual bool keyUp(int key);*/
	
	virtual int classID();
	
protected:

	//bool        m_select;
	bool        m_drag;
	ulong		m_caretPos;
	ulong		m_selTo;
	long        m_hscroll;
	bool        m_lock;
	
/*
	bool		m_OnlyNumbers;
	bool		m_ShowUpDownButton;
	long		m_StepSizeI;
	long		m_LowLimitI;
	long		m_HighLimitI;
	
	long		m_ScrollStartY;
	bool		m_Scrolling;*/
};

}

#endif

