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


#ifndef __CSLIDER_H__SKINNED_GUI__
#define __CSLIDER_H__SKINNED_GUI__

#include <string>
#include "SkinnedGUI/CWidget.h"
#include "Color.h"


namespace SkinnedGUI {

class CSlider;
typedef void(CWidget::*SliderChangeHandler)(CSlider *sender, int newvalue, bool& cancel);
#define SET_SLDCHANGE(slider, func)	SET_EVENT(slider, OnEnterPress, SliderChangePressHandler, func)

class CSlider : public CWidget {
public:
	// Constructor
	CSlider(COMMON_PARAMS);
	CSlider(COMMON_PARAMS, int max, int min = 0);


private:
	// Attributes

	int		iValue;
	int		iMax;
	int		iMin;

	StyleVar<Color>		clLight;
	StyleVar<Color>		clDark;
	StyleVar<Color>		clButLight;
	StyleVar<Color>		clButDark;
	StyleVar<Color>		clButFace;
	StyleVar<SmartPointer<SDL_Surface> > bmpButton;

	int		DoMouseDown(int x, int y, int dx, int dy, MouseButton button, const ModifiersState& modstate);
	int		DoMouseMove(int x, int y, int dx, int dy, bool down, MouseButton button, const ModifiersState& modstate);
	void	DoMove(int ms_x, int ms_y);
	void	DoRepaint();
	int		DoCreate();
	void	ApplySelector(const CSSParser::Selector& sel, const std::string& prefix = "");
	void	ApplyTag(xmlNodePtr node);

	DECLARE_EVENT(OnChange, SliderChangeHandler);

public:
	// Events
	EVENT_SETGET(OnChange, SliderChangeHandler)

	void	LoadStyle(void) {}

	int		getValue(void)						{ return iValue; }
	void	setValue(int v)						{ iValue = v; }

	void	setMax(int _m)						{ iMax = _m; }
	void	setMin(int _m)						{ iMin = _m; }

	static const std::string tagName()			{ return "slider"; }
	const std::string getTagName()				{ return CSlider::tagName(); }
};

}; // namespace SkinnedGUI

#endif  //  __CSLIDER_H__
