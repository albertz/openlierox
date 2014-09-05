#ifndef VERMES_MENU_H
#define VERMES_MENU_H

#ifdef DEDICATED_ONLY
#error "Can't use this in dedicated server"
#endif //DEDICATED_ONLY

#include "gui/omfggui.h"
#include "resource_locator.h"
#include "gconsole.h"
#include "blitters/context.h"

class Font;
class SpriteSet;
struct ALLEGRO_BITMAP;

namespace OmfgGUI
{
	
struct GusanosFont : public BaseFont
{
	GusanosFont(Font* font_)
	: font(font_)
	{}
	
	Font *font;
};

struct GusanosSpriteSet : public BaseSpriteSet
{
	GusanosSpriteSet(SpriteSet* spriteSet_)
	: spriteSet(spriteSet_)
	{}
	
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
	
private:
	Rect clipRect;
	Rect screenRect;
	BlitterContext blitter;
};

class GContext : public Context
{
public:
	GContext(Renderer* renderer);
	
	void init();
	void clear();
	
	virtual LuaContext& luaContext();
	
	virtual BaseFont* loadFont(std::string const& name);
	virtual BaseSpriteSet* loadSpriteSet(std::string const& name);
	
	virtual void loadGSSFile(std::string const& name, bool passive);
	virtual Wnd* loadXMLFile(std::string const& name, Wnd* loadTo);
};

extern GContext menu;

}

class XMLFile
{
public:
	operator bool()
	{
		return (bool)f;
	}
	
	std::ifstream f;
};

extern ResourceLocator<XMLFile, false, false> xmlLocator;

class GSSFile
{
public:
	GSSFile()
	: loaded(false)
	{}
	
	operator bool()
	{
		return loaded;
	}
		
	bool loaded;
};

extern ResourceLocator<GSSFile> gssLocator;

#endif //VERMES_MENU_H
