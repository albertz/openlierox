#ifndef OMFG_GUI_RENDERER_H
#define OMFG_GUI_RENDERER_H

#include "util/rect.h"
#include "util/common.h"
#include <string>
#include <algorithm>
#include <utility>

#ifdef min  // stupid <windows.h> defines this
#undef min
#endif
#ifdef max  // stupid <windows.h> defines this
#undef max
#endif
#ifdef RGB
#undef RGB
#endif

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

}

#endif //OMFG_GUI_RENDERER_H
