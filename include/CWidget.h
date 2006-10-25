/////////////////////////////////////////
//
//             Liero Xtreme
//
//     Copyright Auxiliary Software 2002
//
//
/////////////////////////////////////////


// Widget class
// Created 30/5/02
// Jason Boettcher


#ifndef __CWIDGET_H__
#define __CWIDGET_H__



// Widget messages
#define		WDM_SETENABLE	-1



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


public:
	// Attributes

	int		iX, iY;
	int		iWidth, iHeight;
	int		iFocused;
	int		iType;
	int		iCanLoseFocus;


private:
	int		iID;
	int		iEnabled;

	CWidget	*cNext;
	CWidget	*cPrev;


public:
	// Methods


	// Widget functions
	void			Setup(int id, int x, int y, int w, int h);
	int				InBox(int x, int y);

    void            redrawBuffer(void);

	void			setNext(CWidget *w)		{ cNext = w; }
	CWidget			*getNext(void)			{ return cNext; }
	void			setPrev(CWidget *w)		{ cPrev = w; }
	CWidget			*getPrev(void)			{ return cPrev; }

	int				getID(void)				{ return iID; }
	int				getType(void)			{ return iType; }

	void			setFocused(int _f)		{ iFocused = _f; }
	int				getFocused(void)		{ return iFocused; }

	int				getEnabled(void)		{ return iEnabled; }
	void			setEnabled(int _e)		{ iEnabled = _e; }

	int				CanLoseFocus(void)		{ return iCanLoseFocus; }
	void			setLoseFocus(int _f)	{ iCanLoseFocus = _f; }


	// Virtual functions
	virtual	void	Create(void) = 0;
	virtual void	Destroy(void) = 0;

	//These events return an event id, otherwise they return -1
	virtual	int		MouseOver(mouse_t *tMouse) = 0;
	virtual	int		MouseUp(mouse_t *tMouse, int nDown ) = 0;
	virtual	int		MouseDown(mouse_t *tMouse, int nDown) = 0;
	virtual	int		MouseWheelUp(mouse_t *tMouse ) = 0;
	virtual	int		MouseWheelDown(mouse_t *tMouse) = 0;
	virtual	int		KeyDown(int c) = 0;
	virtual	int		KeyUp(int c) = 0;

	virtual	void	Draw(SDL_Surface *bmpDest) = 0;

	virtual int		SendMessage(int iMsg, DWORD Param1, DWORD Param2) = 0;
};




#endif  //  __CWIDGET_H__
