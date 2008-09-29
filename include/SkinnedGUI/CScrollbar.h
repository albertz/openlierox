/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// gui scrollbar class
// Created 30/6/02
// Jason Boettcher


#ifndef __CSCROLLBAR_H__SKINNED_GUI__
#define __CSCROLLBAR_H__SKINNED_GUI__

#include "SkinnedGUI/CWidget.h"
#include "Timer.h"
#include "Color.h"


namespace SkinnedGUI {

class CScrollbar;
typedef void(CWidget::*ScrollHandler)(CScrollbar *sender, int newval, bool cancel);
#define SET_SCROLLCHANGE(scrollbar, func)		SET_EVENT(scrollbar, OnScroll, ScrollHandler, func)


// Scrollbar directions
enum {
	scrVertical,
	scrHorizontal
};

class CScrollbar : public CWidget {
public:
	// Constructor
	CScrollbar(COMMON_PARAMS);
	CScrollbar(COMMON_PARAMS, int direction);
	~CScrollbar();

private:
	void Init();


private:
	// Attributes
	int		iMin;
	int		iMax;
	int		iValue;
	StyleVar<int> iDirection;

	int		iScrollPos;
	int		iItemsperbox;
	bool	bSliderGrabbed;
	int		iSliderGrabPos;

	StyleVar<SmartPointer<SDL_Surface> > bmpTop;
	StyleVar<SmartPointer<SDL_Surface> > bmpBottom;
	StyleVar<SmartPointer<SDL_Surface> > bmpSliderTop;
	StyleVar<SmartPointer<SDL_Surface> > bmpSliderBottom;
	StyleVar<SmartPointer<SDL_Surface> > bmpSliderMiddle;
	StyleVar<SmartPointer<SDL_Surface> > bmpBackground;
	StyleVar<Color>			iColor;
	StyleVar<Color>			iHighlightColor;
	StyleVar<Color>			iShadowColor;
	StyleVar<Color>			iBackgroundColor;
	StyleVar<Color>			iArrowColor;

	bool	bTopButtonDown;
	bool	bBotButtonDown;
	bool	bSliderDown;
	bool	bBackgroundDown;
	bool	bTopButtonOver;
	bool	bBotButtonOver;
	bool	bSliderOver;
	bool	bBackgroundOver;

	Timer	*tTimer; // Used for scrolling when user is holding the mouse
	struct TimerData;
	TimerData	*pTimerData;

	DECLARE_EVENT(OnScroll, ScrollHandler);

private:
	int	CalculateWidth();
	int	CalculateHeight();
	void DoRepaintVertical();
	void DoRepaintHorizontal();

	int getTopH();
	int getBottomH();

	bool InSlider(int x, int y);
	bool InTop(int x, int y);
	bool InBottom(int x, int y);

	void	OnTimer(Timer::EventData ev);

public:
	EVENT_SETGET(OnScroll, ScrollHandler);


	void	ScrollDown();
	void	ScrollUp();

	void	UpdatePos(void);


	void	setMin(int _min)				{ iMin = _min; UpdatePos(); }
	void	setMax(int _max)				{ iMax = _max; UpdatePos(); }
	void	setValue(int _value)			{ iValue = _value; UpdatePos(); }

	int		getDirection()					{ return iDirection; }
	void	setDirection(int _d)			{ iDirection.set(_d, HIGHEST_PRIORITY); Repaint(); }

	void	setItemsperbox(int _i)			{ iItemsperbox = _i; Repaint(); }
    int     getItemsperbox(void)            { return iItemsperbox; }

	int		getValue(void)					{ return iValue; }
	int		getMax(void)					{ return iMax; }
	bool	getGrabbed(void)				{ return bSliderGrabbed; }

	// Events
	int		DoMouseMove(int x, int y, int dx, int dy, bool down, MouseButton button, const ModifiersState& modstate);
	int		DoMouseUp(int x, int y, int dx, int dy, MouseButton button, const ModifiersState& modstate);
	int		DoMouseDown(int x, int y, int dx, int dy, MouseButton button, const ModifiersState& modstate);
	int		DoMouseWheelDown(int x, int y, int dx, int dy, const ModifiersState& modstate);
	int		DoMouseWheelUp(int x, int y, int dx, int dy, const ModifiersState& modstate);
	int		DoKeyDown(UnicodeChar c, int keysym, const ModifiersState& modstate);
	void	DoRepaint();
	int		DoCreate();

	static const std::string tagName()		{ return "scrollbar"; }
	const std::string getTagName()			{ return CScrollbar::tagName(); }

	void	ApplySelector(const CSSParser::Selector& sel, const std::string& prefix = "");
	void	ApplyTag(xmlNodePtr node);
};

}; // namespace SkinnedGUI

#endif  //  __CSCROLLBAR_H__SKINNED_GUI__
