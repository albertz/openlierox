/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Label
// Created 30/6/02
// Jason Boettcher


#ifndef __CLABEL_H__SKINNED_GUI__
#define __CLABEL_H__SKINNED_GUI__


#include "SkinnedGUI/CWidget.h"
#include "GfxPrimitives.h"
#include "SkinnedGUI/CBorder.h"
#include "SkinnedGUI/CBackground.h"
#include "FontHandling.h"


namespace SkinnedGUI {

class CLabel : public CWidget {
public:
	// Constructor
	CLabel(COMMON_PARAMS);
	CLabel(COMMON_PARAMS, const std::string& text);

private:
	// Attributes

	std::string	sText;
	CFontStyle cFont;
	CTextProperties cText;
	CBorder cBorder;
	CBackground cBackground;
	StyleVar<bool> bAutoSize;

	// Methods
	int		DoCreate();
	void	ReadjustSize();
	void	ApplySelector(const CSSParser::Selector& sel, const std::string& prefix = "");
	void	ApplyTag(xmlNodePtr node);

public:
	// Publish some of the default events
	EVENT_SETGET(OnMouseUp, MouseHandler)
	EVENT_SETGET(OnMouseDown, MouseHandler)

	// Methods

	// Draw the label
	void	DoRepaint();

	void	setText(const std::string& text)  { sText = text; ReadjustSize(); }
	const std::string& getText() const { return sText; }
	static const std::string tagName()		{ return "label"; }
	const std::string getTagName()			{ return CLabel::tagName(); }
	void	setColor(const Color& cl)		{ cFont.iColor.set(cl, HIGHEST_PRIORITY); }
	Color	getColor()						{ return cFont.iColor; }
};

}; // namespace SkinnedGUI

#endif  //  __CLABEL_H__SKINNED_GUI__
