#include "wnd.h"

#include "../../luaapi/types.h"
#include "../../luaapi/context.h"
#include "util/log.h"

#include <iostream>
#include <map>
#include <boost/lexical_cast.hpp>
using boost::lexical_cast;

using std::cout;
using std::cerr;
using std::endl;

namespace OmfgGUI
{
	
LuaReference Wnd::metaTable;

Wnd::~Wnd()
{
	if(m_context)
	{
		//Remove references in context
		luaReference.destroy();
		m_context->deregisterWindow(this);
	}
	else
	{
		cerr << "WARNING: Wnd destructed without context" << endl;
	}
	
	if(m_parent)
	{
		
		m_parent->removeChild(this);
	}

	std::list<Wnd *>::iterator i = m_children.begin(), e = m_children.end();

	for(; i != e;)
	{
		std::list<Wnd *>::iterator next = i; ++next;
		delete (*i);
		i = next;
	}
}

bool Wnd::readSpriteSet(BaseSpriteSet*& dest, std::string const& str)
{
	delete dest;
	dest = 0;

	if(str.size() == 0)
		return true;
	
	dest = m_context->loadSpriteSet(str);
	//cerr << "Loaded: " << str << "(" << dest << ")" << endl;
	if(!dest)
		return false;

	return true;
}

bool Wnd::readSkin(BaseSpriteSet*& dest, std::string const& str)
{
	delete dest;
	dest = 0;

	if(str.size() == 0)
		return true;
	
	dest = m_context->loadSpriteSet(str);

	if(!dest)
		return false;
		
	int frames = dest->getFrameCount();
	
	if(frames < 8)
	{
		cerr << "Not enough sprites in skin " << str << endl;
		delete dest;
		dest = 0;
		return false;
	}
	
	size_t w = dest->getFrameWidth(0);
	size_t h = dest->getFrameHeight(0);
	for(int i = 1; i < 8; ++i)
	{
		if(dest->getFrameWidth(i) != w
		|| dest->getFrameHeight(i) != h)
		{
			cerr << "Sprites in skin " << str << " are not all the same size" << endl;
			delete dest;
			dest = 0;
			return false;
		}
	}
	
	return true;
}

void Wnd::setActivation(bool active)
{
	// Do nothing
}

void Wnd::doSetActivation(bool active)
{
	if(active != m_active)
	{
		if(active)
		{
			m_context->setActive(this);
			focus();
		}
		setActivation(active);
		m_active = active;
	}
}

bool Wnd::doAction()
{
	if(m_callbacks[OnAction].isSet(luaIngame))
	{
		int r = (luaIngame.call(m_callbacks[OnAction], 1), luaReference)();
		if(r == 1)
		{
			bool v = lua_toboolean(luaIngame, -1) != 0;
			luaIngame.pop(1);
			return v;
		}
	}
	return false;
}

void Wnd::setText(std::string const& aStr)
{
	m_text = aStr;
}

void Wnd::pushReference()
{
	luaIngame.push(luaReference);
}

void Wnd::setGroup(std::string newGroup)
{
	m_group = newGroup;
	
	std::list<Wnd *>::iterator i = m_children.begin(), e = m_children.end();
	
	for(; i != e; ++i)
	{
		if((*i)->m_group.empty())
			(*i)->setGroup(newGroup);
	}
}

bool Wnd::getAttrib(std::string const& name, std::string& dest)
{
	std::map<std::string, std::string>::iterator i = m_attributes.find(name);
	if(i == m_attributes.end())
		return false;
	
	dest = i->second;
	return true;
}

void Wnd::setContext_(Context* context)
{
	m_context = context;
	m_context->registerWindow(this);
		
	std::list<Wnd *>::iterator i = m_children.begin(), e = m_children.end();
	
	for(; i != e; ++i)
		(*i)->setContext_(context);
}

int Wnd::classID()
{
	return Context::Unknown;
}

bool Wnd::registerCallback(std::string const& name, LuaReference callb)
{
	LuaCallbacks i = LuaCallbacksMax;
	if(name == "onAction")
		i = OnAction;
	else if(name == "onKeyDown")
		i = OnKeyDown;
	else if(name == "onActivate")
		i = OnActivate;
	if(i != LuaCallbacksMax && !m_callbacks[i].isSet(luaIngame))
	{
		m_callbacks[i] = callb;
		return true;
	}
	return false;
}

bool Wnd::isVisibile()
{
	Wnd* p = this;
	while(p)
	{
		if(!p->m_visible)
			return false;

		p = p->getParent();
	}
	
	return true;
}

bool Wnd::switchTo()
{
	if(!m_parent)
		return false;
	
	std::list<Wnd *>::iterator i = m_parent->m_children.begin(), e = m_parent->m_children.end();
	
	for(; i != e; ++i)
	{
		(*i)->setVisibility(*i == this);
	}
	
	return true;
}

}
