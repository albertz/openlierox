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


#ifndef __CTOGGLEBUTTON_H__SKINNED_GUI__
#define __CTOGGLEBUTTON_H__SKINNED_GUI__

#include "SkinnedGUI/CImageButton.h"


namespace SkinnedGUI {

// Toggle button events
class CToggleButton;
#define SET_TOGGBTNCLICK SET_IMGBTNCLICK
typedef void(CWidget::*ToggleButtonChangeHandler)(CToggleButton *sender, bool newval, bool& cancel);
#define SET_TOGGBTNCHANGE(togglebutton, func) SET_EVENT(togglebutton, OnChange, ToggleButtonChangeHandler, func)

class CToggleButton : public CImageButton {
public:
	// Constructors
	CToggleButton(COMMON_PARAMS, bool down = false);

private:
	DECLARE_EVENT(OnChange, ToggleButtonChangeHandler);

	bool	bDown;

	// Events
	int		DoMouseUp(int x, int y, int dx, int dy, MouseButton button, const ModifiersState& modstate);
	void	DoRepaint();

public:
	EVENT_SETGET(OnChange, ToggleButtonChangeHandler);

	void	setImage(SDL_Surface* theValue);

	static const std::string tagName()			{ return "togglebtn"; }
	const std::string getTagName()				{ return CToggleButton::tagName(); }	
};

}; // namespace SkinnedGUI

#endif  //  __CIMAGEBUTTON_H__SKINNED_GUI__
