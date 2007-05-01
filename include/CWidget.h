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

class CWidget {
public:
	// Constructor
	CWidget() {
		iID = -1;

		iType = -1;
		iX = iY = 0;
		iWidth = iHeight = 1;
		iFocused = false;
		iEnabled = true;

		iCanLoseFocus = true;
	}

    // what is it? what is it??? well yes, it's the destructor!!
    virtual ~CWidget() {}

public:
	// Attributes

	int		iX, iY;
	int		iWidth, iHeight;
	int		iFocused;
	int		iType;
	int		iCanLoseFocus;


private:
	int					iID;
	int					iEnabled;

	generic_events_t	tEvents;
	void				*cParent;

	CWidget				*cNext;
	CWidget				*cPrev;


public:
	// Methods


	// Widget functions
	void			Setup(int id, int x, int y, int w, int h);
	int				InBox(int x, int y);

    void            redrawBuffer(void);

	void			setNext(CWidget *w)				{ cNext = w; }
	CWidget			*getNext(void)					{ return cNext; }
	void			setPrev(CWidget *w)				{ cPrev = w; }
	CWidget			*getPrev(void)					{ return cPrev; }

	int				getID(void)						{ return iID; }
	int				getType(void)					{ return iType; }

	void			setFocused(int _f)				{ iFocused = _f; }
	int				getFocused(void)				{ return iFocused; }

	int				getEnabled(void)				{ return iEnabled; }
	void			setEnabled(int _e)				{ iEnabled = _e; }

	void			*getParent(void)				{ return cParent; }
	void			setParent(void *l)				{ cParent = l; }

	int				CanLoseFocus(void)				{ return iCanLoseFocus; }
	void			setLoseFocus(int _f)			{ iCanLoseFocus = _f; }

	void			SetupEvents(generic_events_t *Events);
	void			ProcessEvent(int Event);


	// Virtual functions
	virtual	void	Create(void) = 0;
	virtual void	Destroy(void) = 0;

	//These events return an event id, otherwise they return -1
	virtual	int		MouseOver(mouse_t *tMouse) = 0;
	virtual	int		MouseUp(mouse_t *tMouse, int nDown ) = 0;
	virtual	int		MouseDown(mouse_t *tMouse, int nDown) = 0;
	virtual	int		MouseWheelUp(mouse_t *tMouse ) = 0;
	virtual	int		MouseWheelDown(mouse_t *tMouse) = 0;
	virtual	int		KeyDown(UnicodeChar c) = 0;
	virtual	int		KeyUp(UnicodeChar c) = 0;

	virtual	void	LoadStyle(void) = 0;
	virtual	void	Draw(SDL_Surface *bmpDest) = 0;

	virtual DWORD	SendMessage(int iMsg, DWORD Param1, DWORD Param2) = 0;
	virtual DWORD	SendMessage(int iMsg, const std::string& sStr, DWORD Param) = 0;
	virtual DWORD	SendMessage(int iMsg, std::string *sStr, DWORD Param) = 0;
};




#endif  //  __CWIDGET_H__
