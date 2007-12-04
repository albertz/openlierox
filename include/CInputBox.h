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


#ifndef __CINPUTBOX_H__
#define __CINPUTBOX_H__

#include "InputEvents.h"
#include "CGuiSkinnedLayout.h"


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
	CInputbox(int val, const std::string& _text, SDL_Surface *img, const std::string& name) {
		iKeyvalue = val;
		sText = _text;
		sName = name;

		bmpImage = img;
		iType = wid_Inputbox;
		iMouseOver = false;
		sVar = NULL;
	}


private:
	// Attributes

	int			iKeyvalue;
	std::string	sText;
	SDL_Surface	*bmpImage;
	int			iMouseOver;
	std::string	sName;

	std::string		*sVar;

public:
	// Methods

	void	Create(void) { }
	void	Destroy(void) { }

	//These events return an event id, otherwise they return -1
	inline int		MouseOver(mouse_t *tMouse)			{ iMouseOver = true; return INB_NONE; }
	inline int		MouseUp(mouse_t *tMouse, int nDown)		{ return INB_MOUSEUP; }
	inline int		MouseDown(mouse_t *tMouse, int nDown)	{ return INB_NONE; }
	inline int		MouseWheelDown(mouse_t *tMouse)		{ return INB_NONE; }
	inline int		MouseWheelUp(mouse_t *tMouse)		{ return INB_NONE; }
	inline int		KeyDown(UnicodeChar c, int keysym)	{ return INB_NONE; }
	inline int		KeyUp(UnicodeChar c, int keysym)	{ return INB_NONE; }


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
	void	Draw(SDL_Surface *bmpDest);

	inline void	LoadStyle(void) {}


	inline int		getValue(void)						{ return iKeyvalue; }
	inline void	setValue(int _v)					{ iKeyvalue = _v; }
	inline std::string	getText(void)				{ return sText; }
	inline void	setText(const std::string& _t)		{ sText = _t; }
	inline std::string	getName(void)				{ return sName; }

	static CWidget * WidgetCreator( const std::vector< CGuiSkin::WidgetVar_t > & p );
	void	ProcessGuiSkinEvent(int iEvent);

	static CInputbox * InputBoxSelected;
	static std::string InputBoxLabel;	// "GUI.InputBoxLabel" skin string
	friend class CInputboxInput;
};

class CInputboxInput: public CInputbox	// InputBoxDialog.xml should contain exactly one such control at the end
{
	private:
	int		iSkipFirstFrame;

	public:
	CInputboxInput();
	void	Create(void) 
	{ 
		iX = iY = 0;	// Fullscreen to capture MouseOver() event on every frame
		iWidth = 640;
		iHeight = 480;
	};
	void	Draw(SDL_Surface *bmpDest) {};
	int		MouseOver(mouse_t *tMouse);

	static CWidget * WidgetCreator( const std::vector< CGuiSkin::WidgetVar_t > & p )
	{
		CInputboxInput * w = new CInputboxInput();
		return w;
	};
	
	void	ProcessGuiSkinEvent(int iEvent) { };
};

#endif  //  __CINPUTBOX_H__
