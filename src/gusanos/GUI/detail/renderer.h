#ifndef OMFG_GUI_RENDERER_H
#define OMFG_GUI_RENDERER_H

#include "util/rect.h"
#include "util/common.h"
#include <string>
#include <algorithm>
#include <utility>

namespace OmfgGUI
{

struct RGB
{
	RGB()
	{
	}
	
	RGB(uchar aR, uchar aG, uchar aB)
		: r(aR), g(aG), b(aB)
	{
	}
	
	RGB blend(RGB const& aB) const
	{
		return RGB(
			uchar(((int)r + aB.r) / 2),
			uchar(((int)g + aB.g) / 2),
			uchar(((int)b + aB.b) / 2));
	}
	
	RGB lighten(unsigned int mod) const
	{
		int r_ = r + mod;
		int g_ = g + mod;
		int b_ = b + mod;
		
		return RGB(
			uchar(std::min(r_, 255)),
			uchar(std::min(g_, 255)),
			uchar(std::min(b_, 255)));
	}
	
	RGB darken(unsigned int mod) const
	{
		int r_ = int(r) - mod;
		int g_ = int(g) - mod;
		int b_ = int(b) - mod;
		
		return RGB(
			uchar(std::max(r_, 0)),
			uchar(std::max(g_, 0)),
			uchar(std::max(b_, 0)));
	}
	
	uchar r;
	uchar g;
	uchar b;
};

struct BaseSpriteSet
{
	virtual int getFrameCount() const = 0;
	
	virtual ulong getFrameWidth(int frame, int angle = 0) const = 0;
	virtual ulong getFrameHeight(int frame, int angle = 0) const = 0;
	
	virtual ~BaseSpriteSet()
	{}
};

struct BaseFont
{
	enum Flags
	{
		CenterH = 1 << 0,
		CenterV = 1 << 1
	};
	
	virtual ~BaseFont()
	{}
};

//Renderer interface
class Renderer
{
public:
	// Draws a box
	virtual void drawBox(
		Rect const& rect,
		RGB const& color,
		RGB const& borderLeftColor,
		RGB const& borderTopColor,
		RGB const& borderRightColor,
		RGB const& borderBottomColor) = 0;
		
	virtual void drawFrame(
		Rect const& rect,
		RGB const& color) = 0;
		
	virtual void drawBox(
		Rect const& rect,
		RGB const& color) = 0;
		
	virtual void drawVLine(ulong x, ulong y1, ulong y2, RGB const& color) = 0;
	
	// Draws text
	virtual void drawText(BaseFont const& font, std::string const& str, ulong flags, ulong x, ulong y, RGB const& aColor) = 0;
	
	virtual std::pair<int, int> getTextDimensions(BaseFont const& font, std::string::const_iterator b, std::string::const_iterator e) = 0;
	virtual int getTextCoordToIndex(BaseFont const& font, std::string::const_iterator b, std::string::const_iterator e, int x) = 0;
	
	virtual void drawSprite(BaseSpriteSet const& spriteSet, int frame, ulong x, ulong y) = 0;
	virtual void drawSprite(BaseSpriteSet const& spriteSet, int frame, ulong x, ulong y, ulong left, ulong top, ulong bottom, ulong right) = 0;

	virtual void setClip(Rect const& rect) = 0;
	virtual Rect const& getClip() = 0;
	virtual Rect const& getViewportRect() = 0;
	
	virtual void setAddBlender(int alpha) = 0;
	virtual void setAlphaBlender(int alpha) = 0;
	virtual void resetBlending() = 0;
	
	virtual void drawSkinnedBox(BaseSpriteSet const& skin, Rect const& rect, RGB const& backgroundColor) = 0;
	
	virtual ~Renderer()
	{}
};

}

#endif //OMFG_GUI_RENDERER_H
