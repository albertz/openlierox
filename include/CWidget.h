/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Widget class
// Created 30/5/02
// Jason Boettcher


#ifndef __CWIDGET_H__
#define __CWIDGET_H__

#include "InputEvents.h"
#include "types.h"
#ifdef WIN32
#include "windows.h"
#endif //WIN32
#include "CGuiSkin.h"


// Widget messages
#define		WDM_SETENABLE	-1

// Generic event IDs
enum {
	OnMouseOver=0,
	OnMouseOut,
	OnMouseDown,
	OnClick,
	NumEvents
};

// Generic events
class generic_events_t { public:
	char Events[NumEvents][128];
};

// Widget types - not needed actually, we may use RTTI
enum WidgetType_t {
	wid_None=-1,
	wid_Button=0,
	wid_Label,
	wid_Listview,
	wid_Scrollbar,
	wid_Slider,
	wid_Textbox,
	wid_Titlebutton,
	wid_Textbutton,
	wid_Checkbox,
	wid_Inputbox,
	wid_Combobox,
	wid_Image,
	wid_Frame,
	wid_Animation,
	wid_GuiLayout
};

class CGuiLayoutBase;

class CWidget {
public:
	// Constructor
	CWidget() {
		iID = -1;

		iType = wid_None;
		iX = iY = 0;
		iWidth = iHeight = 1;
		bFocused = false;
		bEnabled = true;
		bRedrawMenu = true;
		bCanLoseFocus = true;
	}

    // what is it? what is it??? well yes, it's the destructor!!
    virtual ~CWidget() 
	{
		CGuiSkin::DeRegisterUpdateCallback( this );	// Remove any possible callbacks 'cause widget not exists anymore
	}

protected:
	// Attributes

	int		iX, iY;
	int		iWidth, iHeight;
	bool	bFocused;
	WidgetType_t iType;
	bool	bRedrawMenu;
	bool	bCanLoseFocus;


private:
	int					iID;
	bool				bEnabled;

	generic_events_t	tEvents;
	CGuiLayoutBase		*cParent;

	CWidget				*cNext;
	CWidget				*cPrev;


public:
	// Methods


	// Widget functions
	void			Setup(int id, int x, int y, int w, int h);
	bool			InBox(int x, int y);

    void            redrawBuffer(void);

	void			setNext(CWidget *w)				{ cNext = w; }
	CWidget			*getNext(void)					{ return cNext; }
	void			setPrev(CWidget *w)				{ cPrev = w; }
	CWidget			*getPrev(void)					{ return cPrev; }

	int				getID(void)						{ return iID; }
	void			setID(int _i)					{ iID = _i; }
	WidgetType_t	getType(void)					{ return iType; }

	void			setFocused(bool _f)				{ bFocused = _f; }
	bool			getFocused(void)				{ return bFocused; }

	bool			getEnabled(void)				{ return bEnabled; }
	void			setEnabled(bool _e)				{ bEnabled = _e; }

	bool			getRedrawMenu(void)				{ return bRedrawMenu; }
	void			setRedrawMenu(bool _r)			{ bRedrawMenu = _r; }

	int				getX()							{ return iX; }
	int				getY()							{ return iY; }
	int				getWidth()						{ return iWidth; }
	int				getHeight()						{ return iHeight; }

	CGuiLayoutBase	*getParent(void)				{ return cParent; }
	void			setParent(CGuiLayoutBase *l)	{ cParent = l; }

	bool			CanLoseFocus(void)				{ return bCanLoseFocus; }
	void			setLoseFocus(bool _f)			{ bCanLoseFocus = _f; }

	void			SetupEvents(generic_events_t *Events);	// Not used anywhere, should be removed
	void			ProcessEvent(int Event);	// Not used anywhere, should be removed


	// Virtual functions
	virtual	void	Create(void) = 0;
	virtual void	Destroy(void) = 0;

	//These events return an event id, otherwise they return -1
	virtual	int		MouseOver(mouse_t *tMouse) = 0;
	virtual	int		MouseUp(mouse_t *tMouse, int nDown ) = 0;
	virtual	int		MouseDown(mouse_t *tMouse, int nDown) = 0;
	virtual	int		MouseWheelUp(mouse_t *tMouse ) = 0;
	virtual	int		MouseWheelDown(mouse_t *tMouse) = 0;
	virtual	int		KeyDown(UnicodeChar c, int keysym) = 0;
	virtual	int		KeyUp(UnicodeChar c, int keysym) = 0;

	virtual	void	LoadStyle(void) = 0;	// Not used anywhere
	virtual	void	Draw(SDL_Surface *bmpDest) = 0;

	virtual DWORD	SendMessage(int iMsg, DWORD Param1, DWORD Param2) = 0;
	virtual DWORD	SendMessage(int iMsg, const std::string& sStr, DWORD Param) = 0;
	virtual DWORD	SendMessage(int iMsg, std::string *sStr, DWORD Param) = 0;
	
	virtual void	ProcessGuiSkinEvent(int iEvent) {};
};

// Base class for CGuiLayout and CGuiSkinnedLayout
class CGuiLayoutBase
{
	public:
	virtual ~CGuiLayoutBase() { };
	virtual void		Add(CWidget *widget, int id, int x, int y, int w, int h) = 0;
	// Add more functions here if needed
};

#endif  //  __CWIDGET_H__
