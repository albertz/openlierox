// OpenLieroX

// Line
// Created 5/11/06
// Dark Charlie

// code under LGPL


#ifndef __CLINE_H__DEPRECATED_GUI__
#define __CLINE_H__DEPRECATED_GUI__

#include "Color.h"


namespace DeprecatedGUI {

// Line events
enum {
	LIN_NONE=-1
};


class CLine : public CWidget {
public:
	// Constructor
	CLine(int x1, int y1, int dx, int dy, Color col) {
		iX = x1;
		iY = y1;
		iWidth = dx;
		iHeight = dy;
		iColour = col;
	}


private:
	// Attributes
	Color	iColour;

public:
	// Methods

	void	Create() { }
	void	Destroy() { }

	//These events return an event id, otherwise they return -1
	int		MouseOver(mouse_t *tMouse)			{ return LIN_NONE; }
	int		MouseUp(mouse_t *tMouse, int nDown)		{ return LIN_NONE; }
	int		MouseDown(mouse_t *tMouse, int nDown)	{ return LIN_NONE; }
	int		MouseWheelDown(mouse_t *tMouse)		{ return LIN_NONE; }
	int		MouseWheelUp(mouse_t *tMouse)		{ return LIN_NONE; }
	int		KeyDown(UnicodeChar c, int keysym, const ModifiersState& modstate)	{ return LIN_NONE; }
	int		KeyUp(UnicodeChar c, int keysym, const ModifiersState& modstate)	{ return LIN_NONE; }

	DWORD SendMessage(int iMsg, DWORD Param1, DWORD Param2)	{ 
							return 0;
						}
	DWORD SendMessage(int iMsg, const std::string& sStr, DWORD Param) { return 0; }
	DWORD SendMessage(int iMsg, std::string *sStr, DWORD Param)  { return 0; }

	void	ChangeColour(Color col)			{ iColour = col; }

	// Draw the line
	void	Draw(SDL_Surface * bmpDest) {
		DrawLine(bmpDest, iX, iY, iX + iWidth, iY + iHeight, iColour); 
	}
};

}; // namespace DeprecatedGUI

#endif  //  __CLINE_H__DEPRECATED_GUI__
