/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Slider
// Created 30/6/02
// Jason Boettcher


#ifndef __CSLIDER_H__
#define __CSLIDER_H__


// Slider events
enum {
	SLD_NONE=-1,
	SLD_CHANGE=0
};


// Slider messages
enum {
	SLM_GETVALUE=0,
	SLM_SETVALUE
};


class CSlider : public CWidget {
public:
	// Constructor
	CSlider(int max, int min = 0) {
		Create();
		iMax = max;
		iMin = min;
		iType = wid_Slider;
	}


private:
	// Attributes

	int		iValue;
	int		iMax;
	int		iMin;


public:
	// Methods

	void	Create(void) { iValue=0; }
	void	Destroy(void) { }

	//These events return an event id, otherwise they return -1
	int		MouseOver(mouse_t *tMouse)			{ return SLD_NONE; }
	int		MouseUp(mouse_t *tMouse, int nDown)		{ return SLD_NONE; }
	int		MouseDown(mouse_t *tMouse, int nDown);
	int		MouseWheelDown(mouse_t *tMouse)		{ return SLD_NONE; }
	int		MouseWheelUp(mouse_t *tMouse)		{ return SLD_NONE; }
	int		KeyDown(int c)						{ return SLD_NONE; }
	int		KeyUp(int c)						{ return SLD_NONE; }

	void	Draw(SDL_Surface *bmpDest);

	void	LoadStyle(void) {}

	DWORD SendMessage(int iMsg, DWORD Param1, DWORD Param2);
	DWORD SendMessage(int iMsg, const tString& sStr, DWORD Param) { return 0; }
	DWORD SendMessage(int iMsg, tString *sStr, DWORD Param)  { return 0; }

	int		getValue(void)						{ return iValue; }
	void	setValue(int v)						{ iValue = v; }

	void	setMax(int _m)						{ iMax = _m; }
	void	setMin(int _m)						{ iMin = _m; }
};



#endif  //  __CSLIDER_H__
