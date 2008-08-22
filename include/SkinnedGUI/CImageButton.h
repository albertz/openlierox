/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Button
// Created 30/6/02
// Jason Boettcher


#ifndef __CIMAGEBUTTON_H__SKINNED_GUI__
#define __CIMAGEBUTTON_H__SKINNED_GUI__

#include "SkinnedGUI/CWidget.h"


namespace SkinnedGUI {

// Image button events
class CImageButton;
typedef void(CWidget::*ImageButtonClickHandler)(CImageButton *sender, MouseButton button, const ModifiersState& modstate);
#define SET_IMGBTNCLICK(imagebutton, func) SET_EVENT(imagebutton, OnClick, ImageButtonClickHandler, func)

class CImageButton : public CWidget {
public:
	// Constructor
	CImageButton(COMMON_PARAMS);
	virtual ~CImageButton() {}

protected:
	// Attributes
	SmartPointer<SDL_Surface>	bmpImage;
	bool		bActive;
	std::string sPath;
	
	DECLARE_EVENT(OnClick, ImageButtonClickHandler);

	// Events
	virtual int	DoMouseMove(int x, int y, int dx, int dy, bool down, MouseButton button, const ModifiersState& modstate);
	virtual int	DoMouseUp(int x, int y, int dx, int dy, MouseButton button, const ModifiersState& modstate);

	virtual void ApplySelector(const CSSParser::Selector& sel, const std::string& prefix);
	virtual void ApplyTag(xmlNodePtr node);
	
public:
	// Events
	EVENT_SETGET(OnClick, ImageButtonClickHandler)

	// Draw the button
	virtual void	DoRepaint();

	void	setActive(bool _a) { bActive = _a; }
	bool	getActive()	{ return bActive; }

	void	setImage(SmartPointer<SDL_Surface> img);

	static const std::string tagName()	{ return "imgbutton"; }
	virtual const std::string getTagName()	{ return CImageButton::tagName(); }
};

}; // namespace SkinnedGUI

#endif  //  __CIMAGEBUTTON_H__SKINNED_GUI__
