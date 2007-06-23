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


#ifndef __CTITLEBUTTON_H__
#define __CTITLEBUTTON_H__

#include "InputEvents.h"
#include "Cursor.h"


// Event types
enum {
	TBT_NONE=-1,
	TBT_MOUSEUP=0,
	TBT_MOUSEOVER
};


class CTitleButton : public CWidget {
public:
	// Constructor
	CTitleButton(int imgid, SDL_Surface *image) {
		iImageID = imgid;
		bmpImage = image;
		iMouseOver = false;
		iType = wid_Titlebutton;
	}

private:
	// Attributes

	int			iMouseOver;
	int			iImageID;
	SDL_Surface *bmpImage;


public:
	// Methods

	void	Create(void) { }
	void	Destroy(void) { }

	//These events return an event id, otherwise they return -1
	int		MouseOver(mouse_t *tMouse)			{ iMouseOver=true; SetGameCursor(CURSOR_HAND); return TBT_MOUSEOVER; }
	int		MouseUp(mouse_t *tMouse, int nDown)		{ return TBT_MOUSEUP; }
	int		MouseDown(mouse_t *tMouse, int nDown)	{ iMouseOver=true; return TBT_NONE; }
	int		MouseWheelDown(mouse_t *tMouse)		{ return TBT_NONE; }
	int		MouseWheelUp(mouse_t *tMouse)		{ return TBT_NONE; }
	int		KeyDown(UnicodeChar c)						{ return TBT_NONE; }
	int		KeyUp(UnicodeChar c)						{ return TBT_NONE; }

	DWORD SendMessage(int iMsg, DWORD Param1, DWORD Param2)	{ return 0; }
	DWORD SendMessage(int iMsg, const std::string& sStr, DWORD Param) { return 0; }
	DWORD SendMessage(int iMsg, std::string *sStr, DWORD Param)  { return 0; }

	// Draw the title button
	void	Draw(SDL_Surface *bmpDest);

	void	LoadStyle(void) {}



};




#endif  //  __CTITLEBUTTON_H__
