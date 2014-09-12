#ifndef DEDICATED_ONLY

#include "menu.h"
#include "gfx.h"
#include "blitters/blitters.h"
#include "font.h"
#include "sprite_set.h"
#include "sprite.h"
#include "script.h"
#include "luaapi/context.h"
#include "gui/omfggui_windows.h"
#include "gconsole.h"
#include <boost/bind.hpp>
#include <iostream>
#include <list>
#include <string>
#include <fstream>
#include <sstream>
using std::cout;
using std::endl;

ResourceLocator<XMLFile, false, false> xmlLocator;

namespace OmfgGUI
{

AllegroRenderer renderer;
GContext menu(&renderer);

std::string cmdLoadXML(std::list<std::string> const& args)
{
	// Not really used. We don't draw anything of it.
	// However, we load to trick old Gusanos mods.
	
	if(args.size() > 0)
	{
		std::string ret;

		std::list<std::string>::const_iterator i = args.begin();
		std::string path;
		Wnd* loadTo = menu.getRoot();
		
		path = *i++;
		if(i != args.end())
			loadTo = menu.findNamedWindow(*i++);
			
		if(!loadTo)
			return "DESTINATION WINDOW NOT FOUND";
			
		XMLFile f;
		if(xmlLocator.load(&f, path))
		{
			menu.buildFromXML(f.f, loadTo);
			return "";
		}
		else
			return "ERROR LOADING \"" + path + '"';
	}
	
	return "GUI_LOADXML <FILE> [<DEST>] : LOADS AN XML FILE INTO A WINDOW (ROOT BY DEFAULT)";
}

std::string cmdLoadGSS(std::list<std::string> const& args)
{
	// Ignore without error.
	// This were old Gusanos style cheets for Gusanos menu,
	// which is not used (except dummy compatibility code).
	return "";
}

std::string cmdGSS(std::list<std::string> const& args)
{
	return ""; // ignore without error. see cmdLoadGSS()
}

std::string cmdFocus(std::list<std::string> const& args)
{
	return ""; // ignore without error
}

int GusanosSpriteSet::getFrameCount() const
{
	return spriteSet->getFramesWidth();
}

ulong GusanosSpriteSet::getFrameWidth(int frame, int angle) const
{
	return spriteSet->getSprite(frame)->getWidth();
}

ulong GusanosSpriteSet::getFrameHeight(int frame, int angle) const
{
	return spriteSet->getSprite(frame)->getHeight();
}

GContext::GContext(Renderer* renderer)
: Context(renderer)
{}

void GContext::init()
{
	console.registerCommands()
		("GUI_LOADXML", cmdLoadXML)
		("GUI_LOADGSS", cmdLoadGSS)
		("GUI_GSS", cmdGSS)
		("GUI_FOCUS", cmdFocus)
	;
}

LuaContext& GContext::luaContext()
{
	return luaIngame;
}

void GContext::clear()
{
	delete m_rootWnd;
	m_rootWnd = 0;

	// Gusanos menu not used.
	// However, create dummy root wnd so that some old Gusanos mods work.
		
	std::map<std::string, std::string> attributes;
	attributes["id"] = "root";
	Wnd* root = lua_new(Wnd, (0, attributes), luaIngame);
	setRoot(root);
}

BaseFont* GContext::loadFont(std::string const& name)
{
	Font* f = fontLocator.load(name);
	if(!f)
		return 0;
	return new GusanosFont(f);
}

BaseSpriteSet* GContext::loadSpriteSet(std::string const& name)
{
	SpriteSet *s = spriteList.load(name);
	if(!s)
		return 0;
	return new GusanosSpriteSet(s);
}

Wnd* GContext::loadXMLFile(std::string const& name, Wnd* loadTo)
{
	XMLFile f;
	if(xmlLocator.load(&f, name))
	{
		return buildFromXML(f.f, loadTo);
	}
	
	return 0;
}

int allegroColor(RGB const& rgb)
{
	return makecol(rgb.r, rgb.g, rgb.b);
}

// Draws a box
void AllegroRenderer::drawBox(
	Rect const& rect,
	RGB const& color,
	RGB const& borderLeftColor,
	RGB const& borderTopColor,
	RGB const& borderRightColor,
	RGB const& borderBottomColor)
{
	blitter.rectfill(gfx.buffer, rect.x1, rect.y1, rect.x2, rect.y2, allegroColor(color));
	hline(gfx.buffer, rect.x1, rect.y2, rect.x2, allegroColor(borderBottomColor));
	vline(gfx.buffer, rect.x2, rect.y1, rect.y2, allegroColor(borderRightColor));
	vline(gfx.buffer, rect.x1, rect.y1, rect.y2, allegroColor(borderLeftColor));
	hline(gfx.buffer, rect.x1, rect.y1, rect.x2, allegroColor(borderTopColor));
}

void AllegroRenderer::drawFrame(
	Rect const& rect,
	RGB const& color)
{
	int c = allegroColor(color);
	hline(gfx.buffer, rect.x1, rect.y2, rect.x2, c);
	vline(gfx.buffer, rect.x2, rect.y1, rect.y2, c);
	vline(gfx.buffer, rect.x1, rect.y1, rect.y2, c);
	hline(gfx.buffer, rect.x1, rect.y1, rect.x2, c);
}

// Draws a box
void AllegroRenderer::drawBox(
	Rect const& rect,
	RGB const& color)
{
	blitter.rectfill(gfx.buffer, rect.x1, rect.y1, rect.x2, rect.y2, allegroColor(color));
}

void AllegroRenderer::drawVLine(ulong x, ulong y1, ulong y2, RGB const& color)
{
	vline(gfx.buffer, x, y1, y2, allegroColor(color));
}

// Draws text
void AllegroRenderer::drawText(BaseFont const& font, std::string const& str, ulong flags, ulong x, ulong y, RGB const& aColor)
{
	const int spacing = 0;

	if(GusanosFont const* f = dynamic_cast<GusanosFont const*>(&font))
	{
		if(flags & (BaseFont::CenterH | BaseFont::CenterV))
		{
			std::pair<int, int> dim = f->font->getDimensions(str);
			
			if(flags & BaseFont::CenterH)
				x -= (dim.first - 1) / 2;
			if(flags & BaseFont::CenterV)
				y -= (dim.second - 1) / 2;
		}
		
		f->font->draw(gfx.buffer, str, x, y, spacing, 255, aColor.r, aColor.g, aColor.b);
	}
}

std::pair<int, int> AllegroRenderer::getTextDimensions(BaseFont const& font, std::string::const_iterator b, std::string::const_iterator e)
{
	if(GusanosFont const* f = dynamic_cast<GusanosFont const*>(&font))
	{
		return f->font->getDimensions(b, e);
	}
	return std::make_pair(0, 0);
}

int AllegroRenderer::getTextCoordToIndex(BaseFont const& font, std::string::const_iterator b, std::string::const_iterator e, int x)
{
	if(GusanosFont const* f = dynamic_cast<GusanosFont const*>(&font))
	{
		return f->font->getTextCoordToIndex(b, e, x);
	}
	return 0;
}

void AllegroRenderer::drawSprite(BaseSpriteSet const& spriteSet, int frame, ulong x, ulong y)
{
	if(GusanosSpriteSet const* s = dynamic_cast<GusanosSpriteSet const*>(&spriteSet))
	{
		s->spriteSet->getSprite(frame)->draw(gfx.buffer, x, y, blitter);
	}
}

void AllegroRenderer::drawSprite(BaseSpriteSet const& spriteSet, int frame, ulong x, ulong y, ulong left, ulong top, ulong bottom, ulong right)
{
	if(GusanosSpriteSet const* s = dynamic_cast<GusanosSpriteSet const*>(&spriteSet))
	{
		s->spriteSet->getSprite(frame)->drawCut(gfx.buffer, x, y, blitter, 0, left, top, bottom, right);
	}
}

void AllegroRenderer::setClip(Rect const& rect)
{
	set_clip_rect(gfx.buffer, rect.x1, rect.y1, rect.x2, rect.y2);
}

Rect const& AllegroRenderer::getClip()
{
	get_clip_rect(gfx.buffer, &clipRect.x1, &clipRect.y1, &clipRect.x2, &clipRect.y2);
	return clipRect;
}

Rect const& AllegroRenderer::getViewportRect()
{
	screenRect = Rect(0, 0, SCREEN_W - 1, SCREEN_H - 1);
	return screenRect;
}

void AllegroRenderer::setAddBlender(int alpha)
{
	blitter.set(BlitterContext::Add, alpha);
}

void AllegroRenderer::setAlphaBlender(int alpha)
{
	blitter.set(BlitterContext::Alpha, alpha);
}

void AllegroRenderer::resetBlending()
{
	blitter.set(BlitterContext::none());
}

void AllegroRenderer::drawSkinnedBox(BaseSpriteSet const& skin, Rect const& rect, RGB const& backgroundColor)
{
	if(GusanosSpriteSet const* s = dynamic_cast<GusanosSpriteSet const*>(&skin))
	{
		s->spriteSet->drawSkinnedBox(gfx.buffer, blitter, rect, allegroColor(backgroundColor));
	}
}

}

#endif
