/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Font Handling
// Created 22/7/08
// Karel Petranek

#ifndef __FONTHANDLING_H__
#define __FONTHANDLING_H__

#include "CssParser.h"
#include "SmartPointer.h"

// TODO: just a temporary cache, move to CCache later
void ShutdownFontCache();

// TODO: many of these features are not implemented yet, if you have a spare while, make some of them
class CFontStyle  {
public:
	CFontStyle() {
		sFontName.set("default.png", DEFAULT_PRIORITY);
		iColor.set(Color(255, 255, 255), DEFAULT_PRIORITY);
		iBackgroundColor.set(Color(255, 255, 255, SDL_ALPHA_TRANSPARENT), DEFAULT_PRIORITY);
		bBold.set(false, DEFAULT_PRIORITY);
		bUnderline.set(false, DEFAULT_PRIORITY);
		bItalics.set(false, DEFAULT_PRIORITY);
		bOverline.set(false, DEFAULT_PRIORITY);
		bStrikeThrough.set(false, DEFAULT_PRIORITY);
		iSize.set(12, DEFAULT_PRIORITY);
		iVSpacing.set(1, DEFAULT_PRIORITY);
		iHSpacing.set(0, DEFAULT_PRIORITY);
		bmpTexture.set(NULL, DEFAULT_PRIORITY);
	}

	CFontStyle& operator=(const CFontStyle& s)  {
		if (&s != this)  {
			sFontName = s.sFontName;
			iColor = s.iColor;
			bUnderline = s.bUnderline;
			bItalics = s.bItalics;
			bOverline = s.bOverline;
			bStrikeThrough = s.bStrikeThrough;
			iSize = s.iSize;
			iVSpacing = s.iVSpacing;
			iHSpacing = s.iHSpacing;
			bmpTexture = s.bmpTexture;
		}
		return *this;
	}

	StyleVar<std::string> sFontName;
	StyleVar<Color> iColor;
	StyleVar<Color> iBackgroundColor;
	StyleVar<bool> bBold;
	StyleVar<bool> bUnderline;
	StyleVar<bool> bItalics;
	StyleVar<bool> bOverline;
	StyleVar<bool> bStrikeThrough;
	StyleVar<int> iSize;
	StyleVar<int> iVSpacing;
	StyleVar<int> iHSpacing;
	StyleVar<SmartPointer<SDL_Surface> > bmpTexture;

	void ApplySelector(const CSSParser::Selector& sel, const std::string& prefix = "");
};

enum TextHAlignment  {
	algLeft,
	algRight,
	algCenter,
	algJustify
};

enum TextVAlignment  {
	algTop,
	algBottom,
	algMiddle
};

class CTextProperties  {
public:
	CTextProperties() : tFontRect(NULL), iCentreMinX(-0xFFFF),
		iCentreMinY(-0xFFFF)
	{ 
		iHAlignment.set(algLeft, DEFAULT_PRIORITY);
		iVAlignment.set(algTop, DEFAULT_PRIORITY);
		bThreeDotsEnd.set(false, DEFAULT_PRIORITY);
		bWrap.set(false, DEFAULT_PRIORITY);
		iCentreMinX = 0;
		iCentreMinY = 0;
	}

	CTextProperties(SDL_Rect *rect, TextHAlignment halign = algLeft, TextVAlignment valign = algTop) : 
		tFontRect(rect), iCentreMinX(-0xFFFF),
		iCentreMinY(-0xFFFF)
	{ 
		iHAlignment.set(halign, DEFAULT_PRIORITY);
		iVAlignment.set(valign, DEFAULT_PRIORITY);
		bThreeDotsEnd.set(false, DEFAULT_PRIORITY);
		bWrap.set(false, DEFAULT_PRIORITY);
		iCentreMinX = 0;
		iCentreMinY = 0;
	}

	SDL_Rect *tFontRect;  // Clipping rectangle
	StyleVar<TextHAlignment> iHAlignment; // According to the rect
	StyleVar<TextVAlignment> iVAlignment; // According to the rect
	int iCentreMinX;  // The minimal X coordinate the x-centered text can have
	int iCentreMinY;  // The minimal Y coordinate the y-centered text can have
	StyleVar<bool> bThreeDotsEnd;  // If the text doesn't fit the rect, it will strip it and add a "..." at the end
	StyleVar<bool> bWrap;	// Automatically wrap the text

	void ApplySelector(const CSSParser::Selector& sel, const std::string& prefix = "");
};

void DrawGameText(SDL_Surface *bmpDest, const std::string& text, const CFontStyle& style, const CTextProperties& prop);
void DrawGameText(SDL_Surface *bmpDest, int x, int y, const std::string& text);
int GetTextWidth(const CFontStyle& style, const std::string& text);
int GetTextHeight(const CFontStyle& style, const std::string& text);

#endif // __FONTHANDLING_H__
