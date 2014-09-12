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

void Context::render()
{
	if(m_rootWnd)
	{
		Rect oldClip(renderer()->getClip());
		m_rootWnd->doRender(renderer()->getViewportRect());
		renderer()->setClip(oldClip);
		renderer()->resetBlending();
	}       
}

void Context::process()
{
	if(m_rootWnd)
		m_rootWnd->doProcess();
}
	

void Context::setRoot_(Wnd* wnd)
{
	m_rootWnd = wnd;
	if(m_rootWnd)
		m_rootWnd->setContext_(this);
}

void Context::updateGSS()
{
	if(m_rootWnd)
		m_rootWnd->doUpdateGSS();
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
	
	Wnd* oldFocus = m_keyboardFocusWnd;
	m_keyboardFocusWnd = aWnd;
	if(oldFocus)
		oldFocus->applyGSS(m_gss);
	if(m_keyboardFocusWnd)
		m_keyboardFocusWnd->applyGSS(m_gss);
	
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

int Context::GSSselector::matchesWindow(Wnd* w) const
{
	int matchLevel = 1;
	
	for (std::list<Condition>::const_iterator c = cond.begin(); c != cond.end(); c++)
	{
		switch(c->type)
		{
			case Condition::Tag:
				if(w->m_tagLabel != c->v)
					return 0;
				matchLevel += 1;
			break;

			case Condition::Class:
				if(w->m_className != c->v)
					return 0;
				matchLevel += 2;
			break;
			
			case Condition::ID:
				if(w->m_id != c->v)
					return 0;
				matchLevel += 4;
			break;
			
			case Condition::State:
				if(w->m_state != c->v)
					return 0;
				matchLevel += 8;
			break;
			
			case Condition::Group:
				if(w->m_group != c->v)
					return 0;
				matchLevel += 16;
			break;
		}
	}
		
	return matchLevel;
}


}
