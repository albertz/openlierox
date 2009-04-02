/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Button
// Created 30/6/02
// Jason Boettcher


#ifndef __CBUTTON_H__SKINNED_GUI__
#define __CBUTTON_H__SKINNED_GUI__

#include "SkinnedGUI/CWidget.h"
#include "SkinnedGUI/CBackground.h"
#include "SkinnedGUI/CBorder.h"
#include "GfxPrimitives.h"
#include "FontHandling.h"


namespace SkinnedGUI {

// Button specific events
class CButton;
typedef void(CWidget::*ButtonClickHandler)(CButton *sender, MouseButton button, const ModifiersState& modstate);
#define SET_BTNCLICK(button, func)	SET_EVENT(button, OnClick, ButtonClickHandler, func)

class CButton : public CWidget {
public:
	// Constructors
	CButton(COMMON_PARAMS);
	CButton(COMMON_PARAMS, const std::string& text);

private:
	// Attributes

	// These two behave the same as if the mouse were over/down
	// They are used by external code to force the button to draw a specific state
	bool		bDown;
	bool		bActive;

	CBackground cBackground;
	CBorder cBorder;
	StyleVar<SmartPointer<SDL_Surface> > bmpGlyph;
	StyleVar<bool> bAutoSize;
	CFontStyle cFont;
	CTextProperties cText;

	std::string sText;

	DECLARE_EVENT(OnClick, ButtonClickHandler);

	// Events
	int		DoMouseMove(int x, int y, int dx, int dy, bool down, MouseButton button, const ModifiersState& modstate);
	int		DoMouseUp(int x, int y, int dx, int dy, MouseButton button, const ModifiersState& modstate);
	void	DoRepaint();
	void	UpdateSize();
	int		DoCreate();
	
public:
	CButton& operator =(const CButton& b2)  {
		if (&b2 != this)  {
			cBackground = b2.cBackground;
			cBorder = b2.cBorder;
			bmpGlyph = b2.bmpGlyph;
			cFont = b2.cFont;
			sText = b2.sText;
			bDown = b2.bDown;
			bActive = b2.bActive;

			// TODO: CWidget operator=
			sName = b2.sName;
		}
		return *this;
	}

	// Publish some of the default events
	EVENT_SETGET(OnMouseUp, MouseHandler)
	EVENT_SETGET(OnMouseDown, MouseHandler)
	EVENT_SETGET(OnClick, ButtonClickHandler);

	void	LoadStyle() {}

	void	setActive(bool _a) { bActive = _a; }
	bool	getActive()	{ return bActive; }
	void	setDown(bool d) { bDown = d; }
	bool	getDown() { return bDown; }

	static const std::string tagName()	{ return "button"; }
	const std::string getTagName()	{ return CButton::tagName(); }
	void setGlyph(SmartPointer<SDL_Surface> gl) { bmpGlyph.set(gl, HIGHEST_PRIORITY); UpdateSize(); }

	void	ApplySelector(const CSSParser::Selector& sel, const std::string& prefix = "");
	void	ApplyTag(xmlNodePtr node);
	
};

}; // namespace SkinnedGUI

#endif  //  __CBUTTON_H__SKINNED_GUI__
