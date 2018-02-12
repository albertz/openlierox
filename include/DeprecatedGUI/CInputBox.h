/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Input box
// Created 31/7/02
// Jason Boettcher


#ifndef __CINPUTBOX_H__DEPRECATED_GUI__
#define __CINPUTBOX_H__DEPRECATED_GUI__

#include "InputEvents.h"
#include "DeprecatedGUI/CGuiSkinnedLayout.h"

namespace DeprecatedGUI {

// Inputbox events
enum {
	INB_NONE=-1,
	INB_MOUSEUP
};


// inputbox messages
enum {
	INM_GETVALUE=0,
	INS_GETTEXT
};


class CInputbox : public CWidget {
public:
	// Constructor
	CInputbox(int val, const std::string& _text, SmartPointer<SDL_Surface> img, const std::string& name) {
		iKeyvalue = val;
		sText = _text;
		sName = name;

		bmpImage = img;
		iType = wid_Inputbox;
		bMouseOver = false;
		sVar = NULL;
	}


private:
	// Attributes

	int			iKeyvalue;
	std::string	sText;
	SmartPointer<SDL_Surface> bmpImage;
	bool		bMouseOver;
	std::string	sName;

	std::string		*sVar;

public:
	// Methods

	void	Create() { }
	void	Destroy() { }

	//These events return an event id, otherwise they return -1
	int		MouseOver(mouse_t *tMouse)			{ bMouseOver = true; return INB_NONE; }
	int		MouseUp(mouse_t *tMouse, int nDown)		{ return INB_MOUSEUP; }
	int		MouseDown(mouse_t *tMouse, int nDown)	{ return INB_NONE; }
	int		MouseWheelDown(mouse_t *tMouse)		{ return INB_NONE; }
	int		MouseWheelUp(mouse_t *tMouse)		{ return INB_NONE; }
	int		KeyDown(UnicodeChar c, int keysym, const ModifiersState& modstate)	{ return INB_NONE; }
	int		KeyUp(UnicodeChar c, int keysym, const ModifiersState& modstate); // return event on key up so we won't process that same event inside key reader


	// Process a message sent
	inline DWORD SendMessage(int iMsg, DWORD Param1, DWORD Param2) {

				if(iMsg == INM_GETVALUE) {
						return iKeyvalue;
				}

				return 0;
			}

	DWORD SendMessage(int iMsg, const std::string& sStr, DWORD Param) { return 0; }
	DWORD SendMessage(int iMsg, std::string *sStr, DWORD Param)  {
		if (iMsg == INS_GETTEXT)  {
			*sStr = sText;
		}
		return 0;
	}

	// Draw the title button
	void	Draw(SDL_Surface * bmpDest);


	inline int		getValue()						{ return iKeyvalue; }
	inline void	setValue(int _v)					{ iKeyvalue = _v; }
	inline std::string	getText()				{ return sText; }
	inline void	setText(const std::string& _t)		{ sText = _t; }
	inline std::string	getName()				{ return sName; }

	static CInputbox * InputBoxSelected;
	static std::string InputBoxLabel;	// "GUI.InputBoxLabel" skin string
};

} // namespace DeprecatedGUI

#endif  //  __CINPUTBOX_H__DEPRECATED_GUI__
