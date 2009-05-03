#ifndef GUSANOS_MENU_H
#define GUSANOS_MENU_H

#ifdef DEDSERV
#error "Can't use this in dedicated server"
#endif //DEDSERV

#include "omfggui.h"
#include "resource_locator.h"
#include "gconsole.h"
#include "blitters/context.h"
#include <boost/filesystem/fstream.hpp>

class Font;
class SpriteSet;
struct BITMAP;

namespace OmfgGUI
{
	
struct GusanosFont : public BaseFont
{
	GusanosFont(Font* font_)
	: font(font_)
	{
	}
	
	Font *font;
};

struct GusanosSpriteSet : public BaseSpriteSet
{
	GusanosSpriteSet(SpriteSet* spriteSet_)
	: spriteSet(spriteSet_)
	{
	}
	
	virtual int getFrameCount() const;
	
	virtual ulong getFrameWidth(int frame, int angle = 0) const;
	virtual ulong getFrameHeight(int frame, int angle = 0) const;
	
	SpriteSet *spriteSet;
	
	
};

struct AllegroRenderer : public Renderer
{
	AllegroRenderer()
	{
	}
	
	// Draws a box
	virtual void drawBox(
		Rect const& rect,
		RGB const& color,
		RGB const& borderLeftColor,
		RGB const& borderTopColor,
		RGB const& borderRightColor,
		RGB const& borderBottomColor);
		
	virtual void drawFrame(
		Rect const& rect,
		RGB const& color);
		
	virtual void drawBox(
		Rect const& rect,
		RGB const& color);
		
	virtual void drawVLine(ulong x, ulong y1, ulong y2, RGB const& color);
	
	// Draws text
	virtual void drawText(BaseFont const& font, std::string const& str, ulong flags, ulong x, ulong y, RGB const& aColor);
	
	virtual std::pair<int, int> getTextDimensions(BaseFont const& font, std::string::const_iterator b, std::string::const_iterator e);
	virtual int getTextCoordToIndex(BaseFont const& font, std::string::const_iterator b, std::string::const_iterator e, int x);
	
	virtual void drawSprite(BaseSpriteSet const& spriteSet, int frame, ulong x, ulong y);
	virtual void drawSprite(BaseSpriteSet const& spriteSet, int frame, ulong x, ulong y, ulong left, ulong top, ulong bottom, ulong right);

	virtual void setClip(Rect const& rect);
	virtual Rect const& getClip();
	virtual Rect const& getViewportRect();
	
	virtual void setAddBlender(int alpha);
	virtual void setAlphaBlender(int alpha);
	virtual void resetBlending();
	
	void drawSkinnedBox(BaseSpriteSet const& skin, Rect const& rect, RGB const& backgroundColor);
	
/*
	void rectfill32(BITMAP* where, int x1, int y1, int x2, int y2, int colour);
	void rectfill16(BITMAP* where, int x1, int y1, int x2, int y2, int colour);*/
	
private:
	Rect clipRect;
	Rect screenRect;
	BlitterContext blitter;
};

//extern GusanosFont defaultMenuFont;

class GContext : public Context
{
public:
	GContext(Renderer* renderer);
	
	void init();
	
	bool eventKeyDown(int k);
	bool eventKeyUp(int k);
	bool eventPrintableChar(char c, int k);
	
	bool eventMouseDown(int b);
	bool eventMouseUp(int b);
	bool eventMouseMove(int x, int y);
	bool eventMouseScroll(int offs);
	
	void clear();
	
	virtual void setFocus(Wnd* wnd);
	virtual void hiddenFocus();
	virtual void shownFocus();
	virtual LuaContext& luaContext();
	
	virtual BaseFont* loadFont(std::string const& name);
	virtual BaseSpriteSet* loadSpriteSet(std::string const& name);
	
	virtual void loadGSSFile(std::string const& name, bool passive);
	virtual Wnd* loadXMLFile(std::string const& name, Wnd* loadTo);
	
	virtual bool keyState(int key);
	
private:
	GConsole::BindingLock bindingLock;
};

extern GContext menu;

}

class XMLFile
{
public:
	operator bool()
	{
		return f;
	}
	
	fs::ifstream f;
};

extern ResourceLocator<XMLFile, false, false> xmlLocator;

class GSSFile
{
public:
	GSSFile()
	: loaded(false)
	{
	}
	
	operator bool()
	{
		//return f;
		return loaded;
	}
	
	//fs::ifstream f;
	
	bool loaded;
};

extern ResourceLocator<GSSFile> gssLocator;

#endif //GUSANOS_MENU_H
