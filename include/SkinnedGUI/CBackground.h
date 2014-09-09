/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Background for widgets
// Created 11/5/08
// Karel Petranek

#ifndef __CBACKGROUND_H__SKINNED_GUI__
#define __CBACKGROUND_H__SKINNED_GUI__

#include <SDL.h>
#include "CssParser.h"
#include "Color.h"
#include "SmartPointer.h"


namespace SkinnedGUI {

enum BackgroundRepeat  {
	bgNoRepeat,
	bgRepeat,
	bgRepeatX,
	bgRepeatY
};

class CBackground  {
public:
	CBackground();
	CBackground(const CBackground& oth)  { operator=(oth); }

private:
	StyleVar<Color>	clMain;
	StyleVar<Color>	clGradient1;
	StyleVar<Color>	clGradient2;
	StyleVar<bool>		bUseGradient;
	StyleVar<int>		iGradientDirection;
	StyleVar<SmartPointer<SDL_Surface> > bmpMain;
	StyleVar<BackgroundRepeat> iRepeat;

public:
	CBackground& operator=(const CBackground& b)  {
		if (&b != this)  {
			clMain = b.clMain;
			clGradient1 = b.clGradient1;
			clGradient2 = b.clGradient2;
			bUseGradient = b.bUseGradient;
			iGradientDirection = b.iGradientDirection;
			bmpMain = b.bmpMain;
		}
		return *this;
	}

public:
	void setColor(const Color& cl, uint32_t prio = HIGHEST_PRIORITY)	{ clMain.set(cl, prio); }
	Color getColor()							{ return clMain; }

	void setGradient(const Color& cl1, const Color& cl2)	{ clGradient1.set(cl1, HIGHEST_PRIORITY); clGradient2.set(cl2, HIGHEST_PRIORITY); bUseGradient.set(true, HIGHEST_PRIORITY); }

	void setGradientDirection(int _d)			{ iGradientDirection.set(_d, HIGHEST_PRIORITY); }
	int getDradientDirection()					{ return iGradientDirection; }

	void setImage(SmartPointer<SDL_Surface> s)	{ bmpMain.set(s, HIGHEST_PRIORITY); }
	SmartPointer<SDL_Surface> getImage()		{ return bmpMain; }

	// Methods
	void ApplySelector(const CSSParser::Selector& sel, const std::string& prefix = "");
	void Draw(SDL_Surface *bmpDest, int x, int y, int w, int h, const SDL_Rect *cliprect = NULL);
	void Draw(const SmartPointer<SDL_Surface>& bmpDest, int x, int y, int w, int h, const SDL_Rect *cliprect = NULL)  { Draw(bmpDest.get(), x, y, w, h, cliprect); }
};

}; // namespace SkinnedGUI

#endif // __CBACKGROUND_H__SKINNED_GUI__
