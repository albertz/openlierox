/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Title button
// Created 30/6/02
// Jason Boettcher


#ifndef __CTITLEBUTTON_H__SKINNED_GUI__
#define __CTITLEBUTTON_H__SKINNED_GUI__

#include "SkinnedGUI/CImageButton.h"


namespace SkinnedGUI {

#define SET_TITBTNCLICK SET_IMGBTNCLICK

class CTitleButton : public CImageButton {
public:
	// Constructor
	// TODO: these won't work!
	CTitleButton(COMMON_PARAMS, SDL_Surface *image, const std::string& text);
	CTitleButton(COMMON_PARAMS, const std::string& path, const std::string& text) {
		CTitleButton(name, parent, LoadGameImage(path, true), text);
	}

private:
	// Attributes
	std::string sText;
	Uint32 iNormalColor;
	Uint32 iOverColor;
	Uint32 iDownColor;
	int iTextX;
	int iTextY;

public:
	// Methods
	void setNormalColor(Uint32 _c)  { iNormalColor = _c; }
	Uint32 getNormalColor() { return iNormalColor; }

	void setOverColor(Uint32 _c)  { iOverColor = _c; }
	Uint32 getOverColor() { return iOverColor; }

	void setDownColor(Uint32 _c)  { iDownColor = _c; }
	Uint32 getDownColor() { return iDownColor; }

	void setTextX(int _x)  { iTextX = _x; }
	int getTextX()  { return iTextX; }

	// Draw the title button
	void	Draw(SDL_Surface *bmpDest);
};

}; // namespace SkinnedGUI

#endif  //  __CTITLEBUTTON_H__SKINNED_GUI__
