/////////////////////////////////////////
//
//             Liero Xtreme
//
//     Copyright Auxiliary Software 2002
//
//
/////////////////////////////////////////


// gui scrollbar class
// Created 30/6/02
// Jason Boettcher


#ifndef __CSCROLLBAR_H__
#define __CSCROLLBAR_H__


// Scrollbar events
enum {
	SCR_NONE=-1,
	SCR_CHANGE=0
};

// Scrollbar messages
enum {
    SCM_GETVALUE,
    SCM_SETVALUE,
    SCM_SETITEMSPERBOX,
    SCM_SETMAX,
    SCM_SETMIN
};


class CScrollbar : public CWidget {
public:
	// Constructor
	CScrollbar() {
		iType = wid_Scrollbar;
	}


private:
	// Attributes
	int		iMin;
	int		iMax;
	int		iValue;

	int		iScrollPos;
	int		iItemsperbox;
	int		iSliderGrabbed;
	int		iSliderGrabPos;

	int		iTopButton;
	int		iBotButton;

	int		nButtonsDown;
	float	fMouseNext[3];


public:
	// Methods

	void	Create(void);
	void	Destroy(void) { }

	//These events return an event id, otherwise they return -1
	int		MouseOver(mouse_t *tMouse);
	int		MouseUp(mouse_t *tMouse, int nDown);
	int		MouseDown(mouse_t *tMouse, int nDown);
	int		MouseWheelDown(mouse_t *tMouse);
	int		MouseWheelUp(mouse_t *tMouse);
	int		KeyDown(int c)					{ return SCR_NONE; }
	int		KeyUp(int c)					{ return SCR_NONE; }

	void	Draw(SDL_Surface *bmpDest);

	void	UpdatePos(void);


	void	setMin(int _min)				{ iMin = _min; UpdatePos(); }
	void	setMax(int _max)				{ iMax = _max; UpdatePos(); }
	void	setValue(int _value)			{ iValue = _value; UpdatePos(); }

	void	setItemsperbox(int _i)			{ iItemsperbox = _i; }
    int     getItemsperbox(void)            { return iItemsperbox; }

	int		getValue(void)					{ return iValue; }
	int		getMax(void)					{ return iMax; }

	int		SendMessage(int iMsg, DWORD Param1, DWORD Param2);



};


#endif  //  __CSCROLLBAR_H__
