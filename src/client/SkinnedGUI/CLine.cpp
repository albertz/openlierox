/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Line class
// Created 2007
// Karel Petranek

#include "LieroX.h"
#include "debug.h"
#include "SkinnedGUI/CLine.h"
#include "GfxPrimitives.h"
#include "XMLutils.h"


namespace SkinnedGUI {

CLine::CLine(const std::string &name, CContainerWidget *parent) : CWidget(name, parent)
{
	iColour.set(Color(0, 0, 0), DEFAULT_PRIORITY);
}

////////////////
// Paint the line
void CLine::DoRepaint()
{
	CHECK_BUFFER;

	CWidget::DoRepaint();

	DrawLine(bmpBuffer.get(), 0, 0, getWidth(), getHeight(), iColour); 
}

///////////////////////
// Applies the given CSS selector
void CLine::ApplySelector(const CSSParser::Selector &sel, const std::string& prefix)
{
	CWidget::ApplySelector(sel, prefix);

	// Go through the attributes
	FOR_EACH_ATTRIBUTE(sel, it)  {
		if (it->getName() == "color" || it->getName() == "colour")
			iColour.set(it->getFirstValue().getColor(Color(255, 255, 255)), it->getPriority());
	}
}

////////////////////
// Apply the given tag
void CLine::ApplyTag(xmlNodePtr node)
{
	CWidget::ApplyTag(node);

	if (xmlPropExists(node, "color"))
		iColour.set(xmlGetColour(node, "color", iColour), TAG_ATTR_PRIORITY);
}

}; // namespace SkinnedGUI
