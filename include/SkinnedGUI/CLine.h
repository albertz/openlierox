// OpenLieroX

// Line
// Created 5/11/06
// Dark Charlie

// code under LGPL


#ifndef __CLINE_H__SKINNED_GUI__
#define __CLINE_H__SKINNED_GUI__

#include "SkinnedGUI/CWidget.h"
#include "Color.h"


namespace SkinnedGUI {

class CLine : public CWidget {
public:
	// Constructor
	CLine(const std::string &name, CContainerWidget *parent);


private:
	// Attributes
	StyleVar<Color>	iColour;

	void	ApplySelector(const CSSParser::Selector& sel, const std::string& prefix);
	void	ApplyTag(xmlNodePtr node);

public:
	// Methods
	void	ChangeColour(const Color& col)			{ iColour.set(col, HIGHEST_PRIORITY); Repaint(); }

	// Draw the line
	void	DoRepaint();

	static const std::string tagName()	{ return "line"; }
	const std::string getTagName()	{ return CLine::tagName(); }
};

}; // namespace SkinnedGUI

#endif  //  __CLINE_H__SKINNED_GUI__
