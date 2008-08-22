/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Checkbox
// Created 30/7/02
// Jason Boettcher


#ifndef __CCHECKBOX_H__SKINNED_GUI__
#define __CCHECKBOX_H__SKINNED_GUI__

#include "SkinnedGUI/CWidget.h"


namespace SkinnedGUI {

// Check event
class CCheckbox;
typedef void(CWidget::*CheckChangeHandler)(CCheckbox *sender, bool newstate, bool& cancel);
#define SET_CHKCHANGE(checkbox, func)	SET_EVENT(checkbox, OnClick, CheckChangeHandler, func)


class CCheckbox : public CWidget {
public:
	// Constructor
	CCheckbox(COMMON_PARAMS);
	CCheckbox(COMMON_PARAMS, bool val);

private:
	// Attributes
	StyleVar<bool>			bValue;
	StyleVar<SmartPointer<SDL_Surface> > bmpImage;
	
	// Events
	DECLARE_EVENT(OnChange, CheckChangeHandler);

	int			DoMouseUp(int x, int y, int dx, int dy, MouseButton button, const ModifiersState& modstate);
	void		DoRepaint();
	void		ApplySelector(const CSSParser::Selector& sel, const std::string& prefix);
	void		ApplyTag(xmlNodePtr node);

public:
	// Events
	EVENT_SETGET(OnMouseUp, MouseHandler)
	EVENT_SETGET(OnChange, CheckChangeHandler)

	// Methods

	bool	getValue(void)						{ return bValue; }
	void	setValue(bool _v)					{ bValue.set(_v, HIGHEST_PRIORITY); }


	static const std::string tagName()		{ return "checkbox"; }
	const std::string getTagName()			{ return CCheckbox::tagName(); }
};

}; // namespace SkinnedGUI

#endif  //  __CCHECKBOX_H__SKINNED_GUI__
