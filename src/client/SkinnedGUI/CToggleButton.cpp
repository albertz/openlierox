/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Toggle Button
// Created 9/5/08
// Karel Petranek

#include "SkinnedGUI/CToggleButton.h"
#include "LieroX.h"
#include "debug.h"


namespace SkinnedGUI {

/////////////////////
// Constructors
CToggleButton::CToggleButton(COMMON_PARAMS, bool down) :
CImageButton(name, parent)
{
	bDown = down;
	CLEAR_EVENT(OnChange);
}

////////////////////
// Draw the toggle button
void CToggleButton::DoRepaint()
{
	CHECK_BUFFER;

	// HINT: we're using the image button's draw function
	// If we're toggled down, just set the bMouseDown variable to true, call paren't draw and then restore the variable
	bool old_mouseover = bMouseOver;
	bool old_mousedown = bMouseDown;
	if (bDown)  {
		bMouseOver = false;
		bMouseDown = true;
	}

	CImageButton::DoRepaint();

	bMouseOver = old_mouseover;
	bMouseDown = old_mousedown;
}

///////////////////////
// Click event
int CToggleButton::DoMouseUp(int x, int y, int dx, int dy, MouseButton button, const ModifiersState &modstate)
{
	if (InBox(x, y))  {
		bool cancel = false;
		CALL_EVENT(OnChange, (this, !bDown, cancel));
		if (!cancel)
			bDown = !bDown;
	}

	return CImageButton::DoMouseUp(x, y, dx, dy, button, modstate);
}

}; // namespace SkinnedGUI
