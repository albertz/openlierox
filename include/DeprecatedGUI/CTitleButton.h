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


#ifndef __CTITLEBUTTON_H__DEPRECATED_GUI__
#define __CTITLEBUTTON_H__DEPRECATED_GUI__

#include "InputEvents.h"
#include "Cursor.h"
#include "DeprecatedGUI/CWidget.h"
#include "types.h"
#include "DeprecatedGUI/CGuiLayout.h"

namespace DeprecatedGUI {

// Event types
enum {
	TBT_NONE=-1,
	TBT_MOUSEUP=0,
	TBT_MOUSEOVER,
	TBT_CLICKED
};


class CTitleButton : public CWidget {
public:
	// Constructor
	CTitleButton(int imgid, SmartPointer<SDL_Surface> image) {
		iImageID = imgid;
		bmpImage = image;
		bMouseOver = false;
		iType = wid_Titlebutton;
	}

private:
	// Attributes

	bool		bMouseOver;
	int			iImageID;
	SmartPointer<SDL_Surface> bmpImage;

public:
	// Methods

	void	Create() { }
	void	Destroy() { }

	//These events return an event id, otherwise they return -1
	int		MouseOver(mouse_t *tMouse)			{ bMouseOver=true; SetGameCursor(CURSOR_HAND); return TBT_MOUSEOVER; }
	int		MouseUp(mouse_t *tMouse, int nDown)		{ return TBT_MOUSEUP; }
	int		MouseDown(mouse_t *tMouse, int nDown)	{ bMouseOver=true; return TBT_NONE; }
	int		MouseClicked(mouse_t *tMouse, int nDown)	{ return TBT_CLICKED; }
	int		MouseWheelDown(mouse_t *tMouse)		{ return TBT_NONE; }
	int		MouseWheelUp(mouse_t *tMouse)		{ return TBT_NONE; }
	int		KeyDown(UnicodeChar c, int keysym, const ModifiersState& modstate);
	int		KeyUp(UnicodeChar c, int keysym, const ModifiersState& modstate)	{ return TBT_NONE; }

	DWORD SendMessage(int iMsg, DWORD Param1, DWORD Param2)	{ return 0; }
	DWORD SendMessage(int iMsg, const std::string& sStr, DWORD Param) { return 0; }
	DWORD SendMessage(int iMsg, std::string *sStr, DWORD Param)  { return 0; }

	// Draw the title button
	void	Draw(SDL_Surface * bmpDest);
};

}; // namespace DeprecatedGUI

#endif  //  __CTITLEBUTTON_H__DEPRECATED_GUI__
