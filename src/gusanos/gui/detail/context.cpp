#include "context.h"

#include "renderer.h"
#include "wnd.h"
#include "util/macros.h"
#include <iostream>
using std::cerr;
using std::endl;

namespace OmfgGUI
{

void Context::destroy()
{
	delete m_rootWnd; m_rootWnd = 0;
}

void Context::setRoot_(Wnd* wnd)
{
	m_rootWnd = wnd;
	if(m_rootWnd)
		m_rootWnd->setContext_(this);
}

void Context::setFocus(Wnd* aWnd)
{
	if(aWnd)
	{
		while(aWnd->m_lastChildFocus)
		{
			aWnd = aWnd->m_lastChildFocus;
		}
	}
	
	if(aWnd == m_keyboardFocusWnd)
		return;
	
	m_keyboardFocusWnd = aWnd;
	
	if(aWnd)
	{
		aWnd->m_lastChildFocus = 0;
		Wnd* lastChild = aWnd;
		for(Wnd* p = aWnd->m_parent; p; p = p->m_parent)
		{
			p->m_lastChildFocus = lastChild;
			lastChild = p;
		}
	}
}

void Context::setActive(Wnd* wnd)
{
	if(m_activeWnd)
		m_activeWnd->doSetActivation(false);
	m_activeWnd = wnd;
}

void Context::registerWindow(Wnd* wnd)
{
	registerNamedWindow(wnd->m_id, wnd);
}

void Context::registerNamedWindow(std::string const& id, Wnd* wnd)
{
	if(id.empty())
		return;

	m_namedWindows.insert(std::make_pair(id, wnd));
}

void Context::deregisterWindow(Wnd* wnd)
{
	if(getFocus() == wnd)
		setFocus(0);
	if(getMouseCapture() == wnd)
		captureMouse(0);
	if(getRoot() == wnd)
		setRoot((Wnd *)0);
	if(m_activeWnd == wnd)
		m_activeWnd = 0;
	if(m_mouseFocusWnd == wnd)
		m_mouseFocusWnd = 0;
	foreach_delete(i, m_namedWindows)
	{
		if(i->second == wnd)
			m_namedWindows.erase(i);
	}
}


}
