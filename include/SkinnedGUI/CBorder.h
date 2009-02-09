/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Border for widgets
// Created 11/5/08
// Karel Petranek

#ifndef __CBORDER_H__SKINNED_GUI__
#define __CBORDER_H__SKINNED_GUI__

#include "CssParser.h"
#include "Color.h"
#include "SmartPointer.h"


namespace SkinnedGUI {

enum BorderStyle  {
	brdSolid,
	brdOutset,
	brdInset,
};

class CBorder  {
public:
	CBorder() {}

	struct BorderLineSettings  {
		BorderLineSettings() {
							iStyle.set(brdSolid, DEFAULT_PRIORITY);
							iThickness.set(0, DEFAULT_PRIORITY);
							bmpLine.set(NULL, DEFAULT_PRIORITY);
							}

		StyleVar<Color>			clLight;
		StyleVar<Color>			clDark;
		StyleVar<int>			iThickness;
		StyleVar<BorderStyle>	iStyle;
		StyleVar<SmartPointer<SDL_Surface> >	bmpLine;
	};

	struct BorderCornerSettings  {
		BorderCornerSettings() { iRoundRadius.set(0, DEFAULT_PRIORITY);
							bmpCorner.set(NULL, DEFAULT_PRIORITY); }


		StyleVar<int>			iRoundRadius;
		StyleVar<SmartPointer<SDL_Surface> >	bmpCorner;
	};

	// Settings
	BorderLineSettings BorderLeft;
	BorderLineSettings BorderTop;
	BorderLineSettings BorderBottom;
	BorderLineSettings BorderRight;

	BorderCornerSettings CornerTopLeft;
	BorderCornerSettings CornerTopRight;
	BorderCornerSettings CornerBottomLeft;
	BorderCornerSettings CornerBottomRight;

private:
	void ApplySelectorToBorder(const CSSParser::Selector& sel, const std::string& prefix, const std::string& side, BorderLineSettings& sett);
	void ApplySelectorToCorner(const CSSParser::Selector& sel, const std::string& prefix, const std::string& corner, BorderCornerSettings& sett);
	void getCornerDimensions(const BorderCornerSettings& sett, const BorderLineSettings& hrz, const BorderLineSettings& vrt, int& w, int& h);
	SDL_Rect getTopLeftR(const SDL_Rect& border_r);
	SDL_Rect getTopRightR(const SDL_Rect& border_r);
	SDL_Rect getBottomLeftR(const SDL_Rect& border_r);
	SDL_Rect getBottomRightR(const SDL_Rect& border_r);

	void DrawTopLeftCorner(SDL_Surface *bmpDest, const SDL_Rect& r);
	void DrawTopRightCorner(SDL_Surface *bmpDest, const SDL_Rect& r);
	void DrawBottomLeftCorner(SDL_Surface *bmpDest, const SDL_Rect& r);
	void DrawBottomRightCorner(SDL_Surface *bmpDest, const SDL_Rect& r);

	void DrawLeftLine(SDL_Surface *bmpDest, const SDL_Rect& r);
	void DrawRightLine(SDL_Surface *bmpDest, const SDL_Rect& r);
	void DrawTopLine(SDL_Surface *bmpDest, const SDL_Rect& r);
	void DrawBottomLine(SDL_Surface *bmpDest, const SDL_Rect& r);
public:


	// Methods
	void Draw(SDL_Surface *bmpDest, int x, int y, int w, int h, const SDL_Rect *cliprect = NULL);
	void Draw(const SmartPointer<SDL_Surface>& bmpDest, int x, int y, int w, int h, const SDL_Rect *cliprect = NULL) { Draw(bmpDest.get(), x, y, w, h, cliprect); }

public:
	CBorder& operator=(const CBorder& b)  {
		if (&b != this)  {
			BorderLeft = b.BorderLeft;
			BorderTop = b.BorderTop;
			BorderBottom = b.BorderBottom;
			BorderRight = b.BorderRight;
		}
		return *this;
	}

	void ApplySelector(const CSSParser::Selector& sel, const std::string& prefix = "");

	int getLeftW()		{ return BorderLeft.bmpLine.get().get() ? BorderLeft.bmpLine->w : BorderLeft.iThickness; }
	int getTopW()		{ return BorderTop.bmpLine.get().get() ? BorderTop.bmpLine->h : BorderTop.iThickness; }
	int getRightW()		{ return BorderRight.bmpLine.get().get() ? BorderRight.bmpLine->w : BorderRight.iThickness; }
	int getBottomW()	{ return BorderBottom.bmpLine.get().get() ? BorderBottom.bmpLine->h : BorderBottom.iThickness; }
};

}; // namespace SkinnedGUI

#endif // __CBORDER_H__SKINNED_GUI__
