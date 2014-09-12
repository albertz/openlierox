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

struct BaseSpriteSet
{
	virtual int getFrameCount() const = 0;
	
	virtual ulong getFrameWidth(int frame, int angle = 0) const = 0;
	virtual ulong getFrameHeight(int frame, int angle = 0) const = 0;
	
	virtual ~BaseSpriteSet()
	{}
};

}

#endif //OMFG_GUI_RENDERER_H
