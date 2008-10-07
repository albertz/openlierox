/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Marquee
// Created 20/4/08
// Karel Petranek

#include "SkinnedGUI/CMarquee.h"
#include "LieroX.h"

#include "XMLutils.h"


namespace SkinnedGUI {

///////////////////
// Create
CMarquee::CMarquee(COMMON_PARAMS, const std::string& text) : CWidget(name, parent)
{
	sText = text;
	iFrame = 0;
	iDirection = 1;
	iTextWidth = 0;
	bAtEnd = false;
	tTimer = new Timer(getEventHandler(this, &CMarquee::OnTimer), NULL, (Uint32)(MARQUEE_TIME * 1000));
}

CMarquee::CMarquee(const std::string &name, CContainerWidget *parent) : CWidget(name, parent)
{
	sText = "";
	iFrame = 0;
	iDirection = 1;
	iTextWidth = 0;
	bAtEnd = false;
	tTimer = new Timer(getEventHandler(this, &CMarquee::OnTimer), NULL, (Uint32)(MARQUEE_TIME * 1000));	
}

///////////////////
// Destroy
CMarquee::~CMarquee()
{
	delete tTimer;
}

////////////////////
// Apply the given selector
void CMarquee::ApplySelector(const CSSParser::Selector &sel, const std::string &prefix)
{
	CWidget::ApplySelector(sel, prefix);
	cBorder.ApplySelector(sel, prefix);
	cBackground.ApplySelector(sel, prefix);
	cFont.ApplySelector(sel, prefix);
	cText.ApplySelector(sel, prefix);

	iTextWidth = GetTextWidth(cFont, sText);
}

///////////////////
// Apply the given tag
void CMarquee::ApplyTag(xmlNodePtr node)
{
	CWidget::ApplyTag(node);

	sText = xmlGetString(node, "text", sText); // Either as the text property
	sText = xmlNodeText(node, sText);  // Or between <marquee></marquee>
	iTextWidth = GetTextWidth(cFont, sText);
}

/////////////////////
// Draws the marquee
void CMarquee::DoRepaint()
{
	CHECK_BUFFER;

	CWidget::DoRepaint();

	cBackground.Draw(bmpBuffer, 0, 0, getWidth(), getHeight());

	if (iTextWidth <= getWidth())  {
		SDL_Rect r = { cBorder.getLeftW(), cBorder.getTopW(), 
			getWidth() - cBorder.getLeftW() - cBorder.getRightW(),
			getHeight() - cBorder.getTopW() - cBorder.getBottomW()};
		cText.tFontRect = &r;
		DrawGameText(bmpBuffer.get(), sText, cFont, cText);
		
	} else {
		// Draw the font passed into the widget rect
		SDL_Rect r = { -iFrame + cBorder.getLeftW(), cBorder.getTopW(), 
			iTextWidth,
			getHeight() - cBorder.getTopW() - cBorder.getBottomW()};
		cText.tFontRect = &r;
		DrawGameText(bmpBuffer.get(), sText, cFont, cText);
	}
}

//////////////////
// Automatically set the dimensions
void CMarquee::AutoSize()
{
	if (getHeight() == 0)
		Resize(getX(), getY(), getWidth(), GetTextHeight(cFont, sText));
	if (getWidth() == 0)
		Resize(getX(), getY(), 100, getHeight());
}

////////////////////
// Create event
int CMarquee::DoCreate()
{
	CWidget::DoCreate();
	AutoSize();
	tTimer->start();

	return WID_PROCESSED;
}

/////////////////////
// Change the text
void CMarquee::setText(const std::string& text)
{ 
	sText = text;
	iTextWidth = GetTextWidth(cFont, text);
	iFrame = 0;
	Repaint();
}

////////////////////
// Resize the marquee
void CMarquee::Resize(int x, int y, int w, int h)
{
	iMinWidth = cBorder.getLeftW() + cBorder.getRightW();
	iMinHeight = cBorder.getTopW() + cBorder.getBottomW();
	h = GetTextHeight(cFont, sText) + cBorder.getTopW() + cBorder.getBottomW();
	CWidget::Resize(x, y, w, h);
}

////////////////////
// Timer event
void CMarquee::OnTimer(Timer::EventData ev)
{
	// This occurs when we should leave the "At End" state
	if (bAtEnd)  {
		iDirection = -iDirection;
		bAtEnd = false;

		// Change the timer interval
		tTimer->stop();
		tTimer->interval = (Uint32)(MARQUEE_TIME * 1000);
		tTimer->start();

		return;
	}

	// Move
	iFrame += MARQUEE_STEP * iDirection;

	// Reached the right end?
	if (iFrame + getWidth() >= iTextWidth + 4)  {  // Leave some space behind, looks better
		// Change the timer interval
		tTimer->stop();
		tTimer->interval = (Uint32)(MARQUEE_ENDWAIT * 1000);
		tTimer->start();

		bAtEnd = true;
	}

	// Reached the left end?
	if (iFrame <= 0 && iDirection == -1)  {
		// Change the timer interval
		tTimer->stop();
		tTimer->interval = (Uint32)(MARQUEE_ENDWAIT * 1000);
		tTimer->start();

		bAtEnd = true;
	}

	// Repaint
	Repaint();
}

}; // namespace SkinnedGUI
