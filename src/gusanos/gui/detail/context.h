#ifndef OMFG_GUI_CONTEXT_H
#define OMFG_GUI_CONTEXT_H

#include "util/rect.h"
#include "util/common.h"
#include "renderer.h"
#include <iostream>
#include <map>
#include <list>
#include <string>
#include <vector>
#include <stack>

class LuaContext;

namespace OmfgGUI
{

class Wnd;

typedef int KeyType;

class Context
{
public:
	friend class Wnd; //TEMP
	
	enum
	{
		Unknown = 0,
		Button,
		Edit,
		List,
		Group,
		WndCount
	};
	
	struct MouseKey
	{
		enum type
		{
			Left = 0,
			Right,
			Middle
		};
	};
		
	Context()
	: m_mouseCaptureWnd(0), m_rootWnd(0), m_keyboardFocusWnd(0)
	, m_mouseFocusWnd(0)
	{}
	virtual ~Context() {}
			
	void captureMouse(Wnd* aWnd)
	{
		m_mouseCaptureWnd = aWnd;
	}
	
	Wnd* getMouseCapture()
	{
		return m_mouseCaptureWnd;
	}
	
	virtual void setFocus(Wnd* aWnd);
	
	// Called by a window if it has focus and
	// is being hidden.
	virtual void hiddenFocus() {}
	
	// Called by a window if it has focus and
	// is turned visible again.
	virtual void shownFocus() {}
		
	Wnd* getFocus()
	{
		return m_keyboardFocusWnd;
	}
	
	void setActive(Wnd* wnd);
		
	virtual Wnd* loadXMLFile(std::string const&, Wnd* loadTo) = 0;
		
	// This is defined in xml.cpp
	Wnd* buildFromXML(std::istream& s, Wnd* dest);
		
	template<class WndT>
	WndT* setRoot(WndT* wnd)
	{
		setRoot_(wnd);
			
		return wnd;
	}

	Wnd* getRoot()
	{
		return m_rootWnd;
	}

	void registerNamedWindow(std::string const& id, Wnd* wnd);
	void registerWindow(Wnd* wnd);	
	void deregisterWindow(Wnd* wnd);

	Wnd* findNamedWindow(std::string const& id)
	{
		std::map<std::string, Wnd*>::iterator i = m_namedWindows.find(id);
		if(i == m_namedWindows.end())
			return 0;
		return i->second;
	}

	void destroy();
	
	virtual BaseFont* loadFont(std::string const& name) = 0;
	virtual BaseSpriteSet* loadSpriteSet(std::string const& name) = 0;
	
		
protected:
	void setRoot_(Wnd* wnd);
	
	Wnd* m_mouseCaptureWnd;
	Wnd* m_rootWnd;
	Wnd* m_keyboardFocusWnd;
	Wnd* m_activeWnd;
	Wnd* m_mouseFocusWnd;

	std::map<std::string, Wnd*> m_namedWindows;
	
	long m_cursorX;
	long m_cursorY;
};

} //namespace OmfgGUI


#endif //OMFG_GUI_CONTEXT_H
