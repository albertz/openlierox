#include "wnd.h"

#include "luaapi/types.h"
#include "luaapi/context.h"
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
		
		lua.destroyReference(luaReference);
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

void Wnd::applyGSSreally(Context::GSSselectors const& style)
{
	std::multimap<int, Context::GSSpropertyMap const*> clauses;
	
	foreach(i, style)
	{
		if(int level = i->matchesWindow(this))
		{
			clauses.insert(std::make_pair(level, &i->props));
		}
	}
	
	// Activate from lowest specifity
	foreach(i, clauses)
	{
		applyFormatting(*(i->second));
	}
}

/*
void Wnd::applyGSSstate(Context::GSSselectors const& style, std::string const& state)
{
	applyGSSnoState(style);
}
*/

void Wnd::applyGSS(Context::GSSselectors const& style)
{
	//cout << "Context: " << m_context << endl;
	if(m_active)
		m_state = "active";
	else if(isFocused())
		m_state = "focused";
	else
		m_state = "";
	
	applyGSSreally(style);
	
		
	//updateFormatting();
}

void Wnd::applyFormatting(Context::GSSpropertyMap const& f)
{
	#define EACH_VALUE(i_) for(std::list<std::string>::const_iterator i_ = i->second.begin(); i_ != i->second.end(); ++i_)
	
	for(Context::GSSpropertyMap::const_iterator i = f.begin(); i != f.end(); ++i)
	{
		//cout << "Applying to " << m_id << ": " << i->first << endl;
		if(i->first == "background")
		{
			EACH_VALUE(v)
			{
				readColor(m_formatting.background.color, *v);
			}
		}
		else if(i->first == "color" || i->first == "colour")
		{
			EACH_VALUE(v)
			{
				readColor(m_formatting.fontColor, *v);
			}
		}
		else if(i->first == "background-image")
		{
			EACH_VALUE(v)
			{
				if(readSpriteSet(m_formatting.background.spriteSet, *v))
					break;
			}
		}
		else if(i->first == "background-invisible")
		{
			EACH_VALUE(v)
			{
				m_formatting.background.invisible = (*v != "0");
			}
		}
		else if(i->first == "skin")
		{
			EACH_VALUE(v)
			{
				if(readSkin(m_formatting.background.skin, *v))
					break;
			}
		}
		else if(i->first == "border")
		{
			RGB color;
			EACH_VALUE(v)
			{
				readColor(color, *v);
			}
			
			m_formatting.borders[0].color = color;
			m_formatting.borders[1].color = color;
			m_formatting.borders[2].color = color;
			m_formatting.borders[3].color = color;
		}
		else if(i->first == "border-left")
		{
			EACH_VALUE(v)
			{
				readColor(m_formatting.borders[0].color, *v);
			}
		}
		else if(i->first == "border-top")
		{
			EACH_VALUE(v)
			{
				readColor(m_formatting.borders[1].color, *v);
			}
		}
		else if(i->first == "border-right")
		{
			EACH_VALUE(v)
			{
				readColor(m_formatting.borders[2].color, *v);
			}
		}
		else if(i->first == "border-bottom")
		{
			EACH_VALUE(v)
			{
				readColor(m_formatting.borders[3].color, *v);
			}
		}
		else if(i->first == "left")
		{
			EACH_VALUE(v)
			{
				m_formatting.rect.x1 = lexical_cast<int>(*v);
			}
			m_formatting.flags |= Formatting::HasLeft;
		}
		else if(i->first == "right")
		{
			EACH_VALUE(v)
			{
				m_formatting.rect.x2 = lexical_cast<int>(*v);
			}
			m_formatting.flags |= Formatting::HasRight;
		}
		else if(i->first == "top")
		{
			EACH_VALUE(v)
			{
				m_formatting.rect.y1 = lexical_cast<int>(*v);
			}
			m_formatting.flags |= Formatting::HasTop;
		}
		else if(i->first == "bottom")
		{
			EACH_VALUE(v)
			{
				m_formatting.rect.y2 = lexical_cast<int>(*v);
			}
			m_formatting.flags |= Formatting::HasBottom;
		}
		else if(i->first == "width")
		{
			EACH_VALUE(v)
			{
				m_formatting.width = lexical_cast<int>(*v);
			}
		}
		else if(i->first == "height")
		{
			EACH_VALUE(v)
			{
				m_formatting.height = lexical_cast<int>(*v);
			}
		}
		else if(i->first == "spacing")
		{
			EACH_VALUE(v)
			{
				m_formatting.spacing = lexical_cast<int>(*v);
			}
		}
		else if(i->first == "padding")
		{
			EACH_VALUE(v)
			{
				m_formatting.padding = lexical_cast<int>(*v);
			}
		}
		else if(i->first == "font-family")
		{
			delete m_font;
			
			EACH_VALUE(v)
			{
				m_font = m_context->loadFont(*v);
				if(m_font)
					break;
			}
		}
		else if(i->first == "alpha")
		{
			EACH_VALUE(v)
			{
				m_formatting.alpha = lexical_cast<int>(*v);
			}
		}
		else if(i->first == "blender")
		{
			EACH_VALUE(v)
			{
				if(*v == "add")
					m_formatting.blender = Formatting::Add;
				else if(*v == "alpha")
					m_formatting.blender = Formatting::Alpha;
				else if(*v == "none")
					m_formatting.blender = Formatting::None;
			}
		}
	}
	
	#undef EACH_VALUE
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
		applyGSS(m_context->m_gss);
	}
}

bool Wnd::doAction()
{
	if(m_callbacks[OnAction])
	{
		int r = (lua.call(m_callbacks[OnAction], 1), luaReference)();
		if(r == 1)
		{
			bool v = lua_toboolean(lua, -1);
			lua.pop(1);
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
	lua.push(luaReference);
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
	applyGSS(m_context->m_gss); // Apply GSS
	updatePlacement(); // Place window
	
	std::list<Wnd *>::iterator i = m_children.begin(), e = m_children.end();
	
	// Update GSS for children
	for(; i != e; ++i)
		(*i)->doUpdateGSS();
}

//Sends a cursor relocation event
bool Wnd::doMouseMove(ulong newX, ulong newY)
{
	if(!m_visible || !m_rect.isInside(newX, newY))
		return true;
		
	std::list<Wnd *>::reverse_iterator i = m_children.rbegin(), e = m_children.rend();
	
	for(; i != e; ++i)
		if(!(*i)->doMouseMove(newX, newY))
			return false;

	return mouseMove(newX, newY);
}

//Sends a mouse button down event
bool Wnd::doMouseDown(ulong newX, ulong newY, Context::MouseKey::type button)
{
	if(!m_visible || !m_rect.isInside(newX, newY))
		return true;
		
	std::list<Wnd *>::reverse_iterator i = m_children.rbegin(), e = m_children.rend();
	
	for(; i != e; ++i)
	{
		if(!(*i)->doMouseDown(newX, newY, button))
			return false;
	}
	
	bool res = mouseDown(newX, newY, button);
	if(!res)
		m_context->m_mouseFocusWnd = this;

	return res;
}

//Sends a mouse button up event
bool Wnd::doMouseUp(ulong newX, ulong newY, Context::MouseKey::type button)
{
	if(!m_visible || !m_rect.isInside(newX, newY))
		return true;
		
	std::list<Wnd *>::reverse_iterator i = m_children.rbegin(), e = m_children.rend();
	
	for(; i != e; ++i)
		if(!(*i)->doMouseUp(newX, newY, button))
			return false;
		
	return mouseUp(newX, newY, button);
}

bool Wnd::doMouseScroll(ulong newX, ulong newY, int offs)
{
	if(!m_visible || !m_rect.isInside(newX, newY))
		return true;
		
	std::list<Wnd *>::iterator i = m_children.begin(), e = m_children.end();
	
	for(; i != e; ++i)
		if(!(*i)->doMouseScroll(newX, newY, offs))
			return false;
		
	return mouseScroll(newX, newY, offs);
}

//Sends a cursor relocation event
bool Wnd::mouseMove(ulong newX, ulong newY)
{
	return true;
}

//Sends a mouse button down event
bool Wnd::mouseDown(ulong newX, ulong newY, Context::MouseKey::type button)
{
	return true;
}

//Sends a mouse button up event
bool Wnd::mouseUp(ulong newX, ulong newY, Context::MouseKey::type button)
{
	return true;
}

bool Wnd::mouseScroll(ulong newX, ulong newY, int offs)
{
	return true;
}

bool Wnd::doKeyDown(int key)
{
	if(m_callbacks[OnKeyDown])
	{
		int r = (lua.call(m_callbacks[OnKeyDown], 1), luaReference, key)();
		if(r == 1)
		{
			bool v = lua_toboolean(lua, -1);
			lua.pop(1);
			if(v)
				return false;
		}
	}
	if(!keyDown(key))
	{
		return false;
	}
	if(m_parent)
		return m_parent->doKeyDown(key);
	return true;
}
	
bool Wnd::doKeyUp(int key)
{
	if(!keyUp(key))
		return false;
	if(m_parent)
		return m_parent->doKeyUp(key);
	return true;
}

bool Wnd::keyDown(int key)
{
	return true;
}
	
bool Wnd::keyUp(int key)
{
	return true;
}

bool Wnd::charPressed(char c, int key)
{
	return true;
}

void Wnd::setContext_(Context* context)
{
	m_context = context;
	m_context->registerWindow(this);
	
	applyGSS();
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
	if(i != LuaCallbacksMax && !m_callbacks[i])
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
