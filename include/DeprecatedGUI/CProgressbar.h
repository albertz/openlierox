// OpenLieroX

// Progressbar
// Created 5/11/06
// Dark Charlie

// code under LGPL


#ifndef __CPROGRESSBAR_H__DEPRECATED_GUI__
#define __CPROGRESSBAR_H__DEPRECATED_GUI__

#include "DeprecatedGUI/CBar.h"


namespace DeprecatedGUI {

// This only ecapsulates the CBar class to a widget so it can be used in menu

// Progressbar events
enum {
	BAR_NONE=-1
};


class CProgressBar : public CWidget {
public:
	CProgressBar() {}

	// Constructor
	CProgressBar( SmartPointer<SDL_Surface> bmp, int label_x, int label_y, bool label_visible, int numstates) {
		cProgressBar = CBar(bmp, 0, 0, label_x, label_y, BAR_LEFTTORIGHT, numstates);
		cProgressBar.SetLabelVisible(label_visible);
		bRedrawMenu = true;
		iVar = NULL;
	}


private:
	// Attributes
	CBar	cProgressBar;
	int		*iVar;

public:
	// Methods

	void	Create() { cProgressBar.SetX(iX); cProgressBar.SetY(iY); iWidth = cProgressBar.GetWidth(); iHeight = cProgressBar.GetHeight(); }
	void	Destroy() { }

	//These events return an event id, otherwise they return -1
	int		MouseOver(mouse_t *tMouse)			{ return BAR_NONE; }
	int		MouseUp(mouse_t *tMouse, int nDown)		{ return BAR_NONE; }
	int		MouseDown(mouse_t *tMouse, int nDown)	{ return BAR_NONE; }
	int		MouseWheelDown(mouse_t *tMouse)		{ return BAR_NONE; }
	int		MouseWheelUp(mouse_t *tMouse)		{ return BAR_NONE; }
	int		KeyDown(UnicodeChar c, int keysym, const ModifiersState& modstate)	{ return BAR_NONE; }
	int		KeyUp(UnicodeChar c, int keysym, const ModifiersState& modstate)	{ return BAR_NONE; }

	DWORD SendMessage(int iMsg, DWORD Param1, DWORD Param2)	{ 
							return 0;
						}
	DWORD SendMessage(int iMsg, const std::string& sStr, DWORD Param) { return 0; }
	DWORD SendMessage(int iMsg, std::string *sStr, DWORD Param)  { return 0; }

	// Draw the line
	inline void	Draw(SDL_Surface * bmpDest) {
		if (bRedrawMenu)
			redrawBuffer();
		if( iVar )
			SetPosition( *iVar );
		cProgressBar.Draw( bmpDest );
	}

	inline void SetPosition(int _pos)  { cProgressBar.SetPosition(_pos); }
};

}; // namespace DeprecatedGUI

#endif  //  __CPROGRESSBAR_H__DEPRECATED_GUI__
