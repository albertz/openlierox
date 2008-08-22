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

#include "LieroX.h" // for printf
#include "SkinnedGUI/CLabel.h"
#include "XMLutils.h"
#include "MathLib.h"


namespace SkinnedGUI {

//////////////////
// Create
CLabel::CLabel(COMMON_PARAMS, const std::string& text) : CWidget(name, parent) {
	sText = text;
	iType = wid_Label;
	bAutoSize.set(true, DEFAULT_PRIORITY);
}

CLabel::CLabel(const std::string &name, CContainerWidget *parent) : CWidget(name, parent)
{
	sText = "";
	iType = wid_Label;
	bAutoSize.set(true, DEFAULT_PRIORITY);
}

///////////////////
// Draw the label
void CLabel::DoRepaint()
{
	CHECK_BUFFER;

	CWidget::DoRepaint();

	// Background
	cBackground.Draw(bmpBuffer, 0, 0, getWidth(), getHeight());

	// Text
	SDL_Rect r = { cBorder.getLeftW(), cBorder.getTopW(), getWidth() - cBorder.getLeftW() - cBorder.getRightW(),
		getHeight() - cBorder.getTopW() - cBorder.getBottomW()};
	cText.tFontRect = &r;
	DrawGameText(bmpBuffer.get(), sText, cFont, cText);

	// Border
	cBorder.Draw(bmpBuffer, 0, 0, getWidth(), getHeight());
}

/////////////////////
// Automatically setup the size
void CLabel::ReadjustSize()
{
	if (bAutoSize)
		Resize(getX(), getY(), MAX(1, GetTextWidth(cFont, sText) + cBorder.getLeftW() + cBorder.getRightW()),
			MAX(1, GetTextHeight(cFont, sText) + cBorder.getTopW() + cBorder.getBottomW()));
}

/////////////////////
// Create the label
int CLabel::DoCreate()
{
	ReadjustSize();
	return CWidget::DoCreate();
}


////////////////////
// Apply the given selector
void CLabel::ApplySelector(const CSSParser::Selector& sel, const std::string& prefix)
{
	CWidget::ApplySelector(sel, prefix);
	cBorder.ApplySelector(sel, prefix);
	cBackground.ApplySelector(sel, prefix);
	cFont.ApplySelector(sel, prefix);
	cText.ApplySelector(sel, prefix);

	FOR_EACH_ATTRIBUTE(sel, it)  {
		if (it->getName() == prefix + "auto-size")
			bAutoSize.set(it->getFirstValue().getBool(), it->getPriority());
	}
}

////////////////////
// Apply the given tag
void CLabel::ApplyTag(xmlNodePtr node)
{
	CWidget::ApplyTag(node);

	sText = xmlGetString(node, "text", sText); // Text can be either in the text property
	sText = xmlNodeText(node, sText); // or between <label></label>
	if (xmlPropExists(node, "color"))
		cFont.iColor.set(xmlGetColour(node, "color", cFont.iColor), TAG_ATTR_PRIORITY);
	if (xmlPropExists(node, "autosize"))
		bAutoSize.set(xmlGetBool(node, "autosize", bAutoSize), TAG_ATTR_PRIORITY);

	ReadjustSize();

}

}; // namespace SkinnedGUI
