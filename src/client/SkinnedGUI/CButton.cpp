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
// Created 30/3/03
// Jason Boettcher


#include "LieroX.h"
#include "debug.h"
#include "Cursor.h"
#include "MathLib.h"
#include "FindFile.h"
#include "StringUtils.h"
#include "GfxPrimitives.h"
#include "SkinnedGUI/CButton.h"
#include "XMLutils.h"


namespace SkinnedGUI {

#define SPACING 2
#define GLYPH_SPACING 5

//////////////////
// Constructors
CButton::CButton(COMMON_PARAMS, const std::string& text) : CALL_DEFAULT_CONSTRUCTOR {
	CLEAR_EVENT(OnClick);
	bmpGlyph.set(NULL, DEFAULT_PRIORITY);
	bAutoSize.set(true, DEFAULT_PRIORITY);
	cText.iVAlignment.set(algMiddle, DEFAULT_PRIORITY);
	cText.iHAlignment.set(algCenter, DEFAULT_PRIORITY);
	bDown = false;
	bActive = false;
	iType = wid_Button;
	UpdateSize();
}

CButton::CButton(COMMON_PARAMS) : CWidget(name, parent)
{
	CLEAR_EVENT(OnClick);
	bmpGlyph.set(NULL, DEFAULT_PRIORITY);
	cText.iVAlignment.set(algMiddle, DEFAULT_PRIORITY);
	cText.iHAlignment.set(algCenter, DEFAULT_PRIORITY);
	bDown = false;
	bActive = false;
	iType = wid_Button;
}

//////////////////
// Changes the button size according to the settings
void CButton::UpdateSize()
{
	if (bAutoSize)  {
		Resize(getX(), getY(),
			GetTextWidth(cFont, sText) + 2*SPACING + (bmpGlyph.get().get() ? bmpGlyph->w + SPACING : 0), // Width
		MAX(bmpGlyph.get().get() ? bmpGlyph->h : 0, GetTextHeight(cFont, sText))); // Height
	} else {
		int w = GetTextWidth(cFont, sText) + 2*SPACING + (bmpGlyph.get().get() ? bmpGlyph->w + SPACING : 0);
		int h = MAX(bmpGlyph.get().get() ? bmpGlyph->h : 0, GetTextHeight(cFont, sText));
		Resize(getX(), getY(), (getWidth() == 0 ? w : getWidth()), (getHeight() == 0 ? h : getHeight()));
	}
}

//////////////////
// Applies the given CSS selector
void CButton::ApplySelector(const CSSParser::Selector &sel, const std::string& prefix)
{
	CWidget::ApplySelector(sel, prefix);
	cBorder.ApplySelector(sel, prefix);
	cBackground.ApplySelector(sel, prefix);
	cFont.ApplySelector(sel, prefix);
	cText.ApplySelector(sel, prefix);

	FOR_EACH_ATTRIBUTE(sel, it)  {
		if (it->getName() == prefix + "glyph")
			bmpGlyph.set(LoadGameImage(JoinPaths(sel.getBaseURL(), it->getFirstValue().getURL()), true), it->getPriority());
		else if (it->getName() == prefix + "auto-size")
			bAutoSize.set(it->getFirstValue().getBool(), it->getPriority());
	}
}

/////////////////////////
// Read the information from a xml node
void CButton::ApplyTag(xmlNodePtr node)
{
	CWidget::ApplyTag(node);

	// Read the attributes
	if (xmlPropExists(node, "glyph"))  {
		std::string glyph_path = xmlGetString(node, "glyph");
		if (glyph_path.size())
			bmpGlyph.set(LoadGameImage(JoinPaths(xmlGetBaseURL(node), glyph_path), true), TAG_ATTR_PRIORITY);
	}

	if (xmlPropExists(node, "autosize"))
		bAutoSize.set(xmlGetBool(node, "autosize", bAutoSize), TAG_ATTR_PRIORITY);

	sText = xmlGetString(node, "value", sText);
	sText = xmlNodeText(node, sText);
}

///////////////////
// Draw the button
void CButton::DoRepaint()
{
	CHECK_BUFFER;

	CWidget::DoRepaint();

	// Background
	cBackground.Draw(bmpBuffer, 0, 0, getWidth(), getHeight());

	// Glyph
	int x = cBorder.getLeftW() + SPACING;
	if (bmpGlyph.get().get())  {
		// Draw the glyph in front of the text
		int img_x = x;
		switch (cText.iHAlignment)  {
		case algJustify:
		case algLeft:
			break;
		case algCenter:
			img_x = x + (getWidth() - x - cBorder.getRightW() - SPACING - GetTextWidth(cFont, sText)) / 2 - bmpGlyph->w - GLYPH_SPACING;
			break;
		case algRight:
			img_x = getWidth() - 1 - cBorder.getRightW() - SPACING - GetTextWidth(cFont, sText) - GLYPH_SPACING - bmpGlyph->w;
			break;
		}
		DrawImage(bmpBuffer.get(), bmpGlyph, img_x, (getHeight() - bmpGlyph->h) / 2);
		x += bmpGlyph->w + GLYPH_SPACING;
	}

	// Draw the text
	SDL_Rect r = { x, cBorder.getTopW(), getWidth() - x - cBorder.getRightW() - cBorder.getLeftW() - SPACING, getHeight() - cBorder.getTopW() - cBorder.getBottomW() };
	cText.tFontRect = &r;
	DrawGameText(bmpBuffer.get(), sText, cFont, cText);

	// Border
	cBorder.Draw(bmpBuffer, 0, 0, getWidth(), getHeight());
}

////////////////
// Create event
int CButton::DoCreate()
{
	UpdateSize();
	CWidget::DoCreate();

	return WID_PROCESSED;
}

/////////////////
// Mouse move event
int	CButton::DoMouseMove(int x, int y, int dx, int dy, bool down, MouseButton button, const ModifiersState& modstate)
{
	SetGameCursor(CURSOR_HAND);
	CWidget::DoMouseMove(x, y, dx, dy, down, button, modstate);
	return WID_PROCESSED;
}

/////////////////
// Mouse up event
int	CButton::DoMouseUp(int x, int y, int dx, int dy, MouseButton button, const ModifiersState& modstate)
{
	// Click?
	if (RelInBox(x, y))
		CALL_EVENT(OnClick, (this, button, modstate));

	CWidget::DoMouseUp(x, y, dx, dy, button, modstate); // Calls the user defined function
	return WID_PROCESSED;
}

}; // namespace SkinnedGUI
