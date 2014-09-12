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
	
	delete m_font;
}

void Wnd::getCoord(int& dx, int& dy, int x, int y)
{
	if(!m_parent)
	{
		dx = x;
		dy = y;
		return;
	}
	
	if(x >= 0)
		dx = m_parent->getRect().x1 + x;
	else
		dx = m_parent->getRect().x2 + x;
		
	if(y >= 0)
		dy = m_parent->getRect().y1 + y;
	else
		dy = m_parent->getRect().y2 + y;
		
	return;
}

void Wnd::getCoordX(int& dx, int x)
{
	if(!m_parent)
	{
		if(x >= 0)
			dx = x;
		else
			dx = 320 + x; //TODO: Get the real viewport height
		return;
	}
	
	if(x >= 0)
		dx = m_parent->getRect().x1 + x;
	else
		dx = m_parent->getRect().x2 + x;

	return;
}

void Wnd::getCoordY(int& dy, int y)
{
	if(!m_parent)
	{
		if(y >= 0)
			dy = y;
		else
			dy = 240 + y; //TODO: Get the real viewport width
		return;
	}
	
	if(y >= 0)
		dy = m_parent->getRect().y1 + y;
	else
		dy = m_parent->getRect().y2 + y;

	return;
}

static int fromHex(char h)
{
	if(h >= '0' && h <= '9')
		return h - '0';
	else if(h >= 'a' && h <= 'f')
		return h - 'a' + 10;
	else if(h >= 'A' && h <= 'F')
		return h - 'A' + 10;
	else
		return -1;
}

bool Wnd::readColor(RGB& dest, std::string const& str)
{
	if(str.size() >= 7 && str[0] == '#')
	{
		dest.r = fromHex(str[1]) * 16 + fromHex(str[2]);
		dest.g = fromHex(str[3]) * 16 + fromHex(str[4]);
		dest.b = fromHex(str[5]) * 16 + fromHex(str[6]);
		return true;
	}
	return false;
}

bool Wnd::readSpriteSet(BaseSpriteSet*& dest, std::string const& str)
{
	if(str.size() == 0)
	{
		dest = 0;
		return true;
	}
	
	delete dest;
	dest = m_context->loadSpriteSet(str);
	//cerr << "Loaded: " << str << "(" << dest << ")" << endl;
	if(!dest)
		return false;

	return true;
}

bool Wnd::readSkin(BaseSpriteSet*& dest, std::string const& str)
{
	if(str.size() == 0)
	{
		dest = 0;
		return true;
	}
	
	delete dest;
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

void Wnd::updatePlacement()
{
	switch(m_formatting.flags & (Formatting::HasLeft | Formatting::HasRight))
	{
		case 0:
		{
			//m_rect.x1 = formatting.x;
			if(m_parent)
			{
				m_parent->allocateSpace(m_rect.x1, m_rect.y1, m_formatting.width, m_formatting.height);
				m_rect.x2 = m_rect.x1 + m_formatting.width;
				m_rect.y2 = m_rect.y1 + m_formatting.height;
			}
			else
			{
				m_rect = Rect(0, 0, 320-1, 240-1); //TODO: Replace with real dimensions
			}
		}
		break;
		
		case Formatting::HasLeft:
		{
			getCoordX(m_rect.x1, m_formatting.rect.x1);
			m_rect.x2 = m_rect.x1 + m_formatting.width;
		}
		break;
		
		case Formatting::HasRight:
		{
			getCoordX(m_rect.x2, m_formatting.rect.x2);
			m_rect.x1 = m_rect.x2 - m_formatting.width;
		}
		break;
		
		default:
		{
			getCoordX(m_rect.x1, m_formatting.rect.x1);
			getCoordX(m_rect.x2, m_formatting.rect.x2);
		}
	}
	
	switch(m_formatting.flags & (Formatting::HasTop | Formatting::HasBottom))
	{
		case 0:
		{
			//m_rect.y1 = windows.top().curY;
		}
		break;
		
		case Formatting::HasTop:
		{
			getCoordY(m_rect.y1, m_formatting.rect.y1);
			m_rect.y2 = m_rect.y1 + m_formatting.height;
		}
		break;
		
		case Formatting::HasBottom:
		{
			getCoordY(m_rect.y2, m_formatting.rect.y2);
			m_rect.y1 = m_rect.y2 - m_formatting.height;
		}
		break;
		
		default:
		{
			getCoordY(m_rect.y1, m_formatting.rect.y1);
			getCoordY(m_rect.y2, m_formatting.rect.y2);
		}
	}
	
	m_freeNextX = m_freeRect.x1 = m_rect.x1 + m_formatting.padding;
	m_freeNextY = m_freeRect.y1 = m_rect.y1 + m_formatting.padding;
	m_freeRect.x2 = m_rect.x2 - m_formatting.padding;
	m_freeRect.y2 = m_rect.y2 - m_formatting.padding;

}

void Wnd::allocateSpace(int& x, int& y, int width, int height)
{
	if(m_freeNextX + width > m_freeRect.x2)
	{
		m_freeRect.y1 = m_freeNextY;
		m_freeNextX = m_freeRect.x1;
	}
	
	x = m_freeNextX;
	y = m_freeRect.y1;
	
	m_freeNextX += width + m_formatting.spacing;
	
	int bottom = m_freeRect.y1 + height + m_formatting.spacing;
	if(bottom > m_freeNextY)
		m_freeNextY = bottom;
}

Wnd* Wnd::findClosestChild(Wnd* org, Dir direction)
{
	std::list<Wnd *>::iterator i = m_children.begin(), e = m_children.end();
	
	long  minDist = 0x7FFFFFFF;
	Wnd  *minWnd = 0;
	
	int orgx = org->getRect().centerX();
	int orgy = org->getRect().centerY();
	
	for(; i != e; ++i)
	{
		if(*i != org && (*i)->m_focusable && (*i)->m_visible)
		{
			Dir thisDir;
			int x = (*i)->getRect().centerX() - orgx;
			int y = (*i)->getRect().centerY() - orgy;
			if(x > 0)
			{
				if(y < -x) thisDir = Up;
				else if(y >= x) thisDir = Down;
				else thisDir = Right;
			}
			else
			{
				if(y <= x) thisDir = Up;
				else if(y > -x) thisDir = Down;
				else thisDir = Left;
			}
			
			if(thisDir == direction)
			{
				int dist = x*x + y*y;
				if(dist < minDist)
				{
					minWnd = *i;
					minDist = dist;
				}
			}
		}
	}
	
	return minWnd;
}

bool Wnd::doRender(Rect const& clip)
{
	//Render parent first
	if(!m_visible)
		return false;
	
	Renderer* renderer = context()->renderer();
		
	Rect rect(clip);
	rect.intersect(m_rect);
	if(!rect.isValid())
		return false;
	renderer->setClip(rect);
	
	switch(m_formatting.blender)
	{
		case Formatting::Add:
			if(m_formatting.alpha > 0)
				renderer->setAddBlender(m_formatting.alpha);
			else
				renderer->resetBlending();
		break;
		
		case Formatting::Alpha:
			if(m_formatting.alpha < 255)
				renderer->setAlphaBlender(m_formatting.alpha);
			else
				renderer->resetBlending();
		break;
		default: renderer->resetBlending();	
	}
	
	if(!render())
		return false;

	std::list<Wnd *>::iterator i = m_children.begin(), e = m_children.end();
	
	for(; i != e; ++i)
	{
		(*i)->doRender(rect);
	}
	
	return true;
}

void Wnd::doProcess()
{
	//Process children first
	std::list<Wnd *>::iterator i = m_children.begin(), e = m_children.end();
	
	for(; i != e; ++i)
	{
		(*i)->doProcess();
	}
	
	process();
}

bool Wnd::render()
{
	return true;
}

void Wnd::process()
{
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

void Wnd::notifyHide()
{
	if(!m_context)
		return;
	
	if(m_context->getFocus() == this)
		m_context->hiddenFocus();
	
	std::list<Wnd *>::iterator i = m_children.begin(), e = m_children.end();
	
	for(; i != e; ++i)
	{
		(*i)->notifyHide();
	}
}

void Wnd::notifyShow()
{
	if(!m_context)
		return;
	
	if(m_context->getFocus() == this)
		m_context->shownFocus();
		
	std::list<Wnd *>::iterator i = m_children.begin(), e = m_children.end();
	
	for(; i != e; ++i)
	{
		(*i)->notifyShow();
	}
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

/*
std::string const& Wnd::getText() const
{
	return m_text;
}*/

bool Wnd::getAttrib(std::string const& name, std::string& dest)
{
	std::map<std::string, std::string>::iterator i = m_attributes.find(name);
	if(i == m_attributes.end())
		return false;
	
	dest = i->second;
	return true;
}

void Wnd::doUpdateGSS()
{
	m_formatting = Formatting(); // Reset formatting to default
	updatePlacement(); // Place window
	
	std::list<Wnd *>::iterator i = m_children.begin(), e = m_children.end();
	
	// Update GSS for children
	for(; i != e; ++i)
		(*i)->doUpdateGSS();
}

void Wnd::setContext_(Context* context)
{
	m_context = context;
	m_context->registerWindow(this);
	
	updatePlacement();
	
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
