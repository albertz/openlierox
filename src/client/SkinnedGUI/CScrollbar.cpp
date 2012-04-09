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


#include "LieroX.h"

#include "GfxPrimitives.h"
#include "MathLib.h"
#include "FindFile.h"
#include "StringUtils.h"
#include "XMLutils.h"
#include "GuiPrimitives.h"
#include "SkinnedGUI/CScrollbar.h"


namespace SkinnedGUI {

#define MIN_SLIDER_LENGTH 12 // Minimum length of the slider in pixels

// Dimensions of the arrow
#define ARROW_W  7
#define ARROW_H  4

struct CScrollbar::TimerData {
	TimerData(CScrollbar *s, bool dwn, int stp) : scr(s), down(dwn), step(stp) {}
	CScrollbar *scr;
	bool down; // scrolling down?
	int step;
};

///////////////////
// Create the scrollbar
CScrollbar::CScrollbar(COMMON_PARAMS, int direction) : CWidget(name, parent)
{
	Init();
	iDirection.set(scrVertical, DEFAULT_PRIORITY);
}

CScrollbar::CScrollbar(const std::string &name, CContainerWidget *parent) : CWidget(name, parent)
{
	Init();
}

//////////////////
// Init the scrollbar (used by constructors)
void CScrollbar::Init()
{
	iType = wid_Scrollbar;
	iMin = 0;
	iMax = 1;
	iValue = 0;
	iItemsperbox = 1;
	iScrollPos = 0;
	bSliderGrabbed = false;
	iSliderGrabPos = 0;
	bTopButtonDown = false;
	bBotButtonDown = false;
	bSliderDown = false;
	bTopButtonOver = false;
	bBotButtonOver = false;
	bSliderOver = false;
	bBackgroundDown = false;
	bBackgroundOver = false;
	iDirection.set(scrVertical, DEFAULT_PRIORITY);
	pTimerData = new TimerData(this, false, 0);
	tTimer = new Timer("CScrollbar", getEventHandler(this, &CScrollbar::OnTimer), pTimerData, 120);

	bmpTop.set(NULL, DEFAULT_PRIORITY);
	bmpBottom.set(NULL, DEFAULT_PRIORITY);
	bmpSliderTop.set(NULL, DEFAULT_PRIORITY);
	bmpSliderMiddle.set(NULL, DEFAULT_PRIORITY);
	bmpSliderBottom.set(NULL, DEFAULT_PRIORITY);
	bmpBackground.set(NULL, DEFAULT_PRIORITY);
	iColor.set(Color(191, 191, 191), DEFAULT_PRIORITY);
	iHighlightColor.set(Color(255, 255, 255), DEFAULT_PRIORITY);
	iShadowColor.set(Color(127, 127, 127), DEFAULT_PRIORITY);
	iBackgroundColor.set(Color(0, 0, 0), DEFAULT_PRIORITY);
	iArrowColor.set(Color(0, 0, 0), DEFAULT_PRIORITY);
}

CScrollbar::~CScrollbar()
{
	delete pTimerData;
	delete tTimer;
}

///////////////////
// Calculates the scrollbar width
int CScrollbar::CalculateWidth()
{
	int top_w = bmpTop.get().get() ? bmpTop->w/3 : 0;
	int bottom_w = bmpBottom.get().get() ? bmpBottom->w/3 : 0;
	int slider_top_w = bmpSliderTop.get().get() ? bmpSliderTop->w/3 : 0;
	int slider_middle_w = bmpSliderMiddle.get().get() ? bmpSliderMiddle->w/3 : 0;
	int slider_bottom_w = bmpSliderBottom.get().get() ? bmpSliderBottom->w/3 : 0;
	return MAX(MAX(MAX(top_w, bottom_w), MAX(slider_top_w, slider_bottom_w)), MAX(16, slider_middle_w));
}

///////////////////
// Calculates the scrollbar height
int CScrollbar::CalculateHeight()
{
	int top_h = bmpTop.get().get() ? bmpTop->h : 0;
	int bottom_h = bmpBottom.get().get() ? bmpBottom->h : 0;
	int slider_top_h = bmpSliderTop.get().get() ? bmpSliderTop->h : 0;
	int slider_middle_h = bmpSliderMiddle.get().get() ? bmpSliderMiddle->h : 0;
	int slider_bottom_h = bmpSliderBottom.get().get() ? bmpSliderBottom->h : 0;
	return MAX(MAX(MAX(top_h, bottom_h), MAX(slider_top_h, slider_bottom_h)), MAX(16, slider_middle_h));
}

////////////////////
// Create event
int CScrollbar::DoCreate()
{
	CWidget::DoCreate();

	if (iDirection == scrVertical)
		Resize(getX(), getY(), CalculateWidth(), getHeight());
	else
		Resize(getX(), getY(), getWidth(), CalculateHeight());

	return WID_PROCESSED;
}

/////////////////////
// Repaint the vertical part of the scrollbar
void CScrollbar::DoRepaintVertical()
{
	int top_h = getWidth();
	int bot_h = getWidth();

	// Top arrow
	if (bmpTop.get().get())  {
		top_h = bmpTop->h;
		int sx = 0;
		if (bTopButtonOver)
			sx = bmpTop->w / 3;
		if (bTopButtonDown)
			sx = bmpTop->w * 2 / 3;
		DrawImageAdv(bmpBuffer.get(), bmpTop, sx,0, 0, 0, bmpTop->w/3, bmpTop->h);
	} else {
		DrawSimpleButton(bmpBuffer.get(), 0, 0, getWidth(), getWidth(), iColor, iHighlightColor, iShadowColor, bTopButtonDown);
		DrawSimpleArrow(bmpBuffer.get(), (getWidth() - ARROW_W) / 2, (getWidth() - ARROW_H) / 2, ARROW_W, ARROW_H, ardUp, iArrowColor);
	}

	// Bottom arrow
	if (bmpBottom.get().get())  {
		bot_h = bmpBottom->h;
		int sx = 0;
		if (bBotButtonOver)
			sx = bmpBottom->w / 3;
		if (bBotButtonDown)
			sx = bmpBottom->w * 2 / 3;
		DrawImageAdv(bmpBuffer.get(), bmpBottom, sx,0, 0, getHeight() - bmpBottom->h, bmpBottom->w/3, bmpBottom->h);
	} else {
		DrawSimpleButton(bmpBuffer.get(), 0, getHeight() - getWidth(), getWidth(), getWidth(), iColor, iHighlightColor, iShadowColor, bBotButtonDown);
		DrawSimpleArrow(bmpBuffer.get(), (getWidth() - ARROW_W) / 2, getHeight() - (getWidth() + ARROW_H) / 2 - 1, ARROW_W, ARROW_H, ardDown, iArrowColor);
	}

	// Slider
	if(iMax > iItemsperbox && iMax > 0) {
		int length = (int)((float)iItemsperbox/(float)iMax * getHeight()-30);
		length = MAX(length, MIN_SLIDER_LENGTH);

		// Get the pos
		int pos = iScrollPos;
		if(pos+length > getHeight() - top_h - bot_h)
			pos = getHeight() - top_h - bot_h - length;
		if (pos < 0)
			pos = 0;

		// Have bitmaps?
		if (bmpSliderTop.get().get() && bmpSliderBottom.get().get() && bmpSliderMiddle.get().get())  {
			length = MAX(length, bmpSliderTop->h + bmpSliderBottom->h);

			int top_sx = 0;
			int main_sx = 0;
			int bot_sx = 0;
			if (bSliderOver)  {
				top_sx = bmpSliderTop->w / 3;
				bot_sx = bmpSliderBottom->w / 3;
				main_sx = bmpSliderMiddle->w / 3;
			}

			if (bSliderDown)  {
				top_sx = bmpSliderTop->w * 2 / 3;
				bot_sx = bmpSliderBottom->w / 3;
				main_sx = bmpSliderMiddle->w / 3;
			}

			// Top part
			DrawImageAdv(bmpBuffer.get(), bmpSliderTop, top_sx, 0, 0, top_h + pos, bmpSliderTop->w/3, bmpSliderTop->h);

			// Main part
			DrawImageTiled(bmpBuffer.get(), bmpSliderMiddle, main_sx, 0, bmpSliderMiddle->w/3, bmpSliderMiddle->h, 0, top_h + pos + bmpSliderTop->h, getWidth(), MAX(0, length - bmpSliderTop->h - bmpSliderBottom->h));

			// Bottom part
			DrawImageAdv(bmpBuffer.get(), bmpSliderBottom, bot_sx, 0, 0, pos + length - bmpSliderBottom->h, bmpSliderBottom->w/3, bmpSliderBottom->h);
		} else {
			DrawSimpleButton(bmpBuffer.get(), 0, top_h + pos, getWidth(), length, iColor, iHighlightColor, iShadowColor, false);
		}
	}
}

///////////////////////
// Repaint the horizontal scrollbar
void CScrollbar::DoRepaintHorizontal()
{
	int left_w = getHeight();
	int right_w = getHeight();

	// Left button
	if (bmpTop.get().get())  {
		left_w = bmpTop->w / 3;
		int sx = 0;
		if (bTopButtonOver)
			sx = bmpTop->w / 3;
		if (bTopButtonDown)
			sx = bmpTop->w * 2 / 3;
		DrawImageAdv(bmpBuffer.get(), bmpTop, sx, 0, 0, 0, bmpTop->w/3, bmpTop->h);
	} else {
		DrawSimpleButton(bmpBuffer.get(), 0, 0, getHeight(), getHeight(), iColor, iHighlightColor, iShadowColor, bTopButtonDown);
		DrawSimpleArrow(bmpBuffer.get(), (getHeight() - ARROW_H) / 2, (getHeight() - ARROW_W) / 2, ARROW_H, ARROW_W, ardLeft, iArrowColor);
	}

	// Right button
	if (bmpBottom.get().get())  {
		right_w = bmpBottom->w / 3;
		int sx = 0;
		if (bBotButtonOver)
			sx = bmpBottom->w / 3;
		if (bBotButtonDown)
			sx = bmpBottom->w * 2 / 3;
		DrawImageAdv(bmpBuffer.get(), bmpBottom, sx, 0, getWidth() - right_w, 0, bmpBottom->w/3, bmpBottom->h);
	} else {
		DrawSimpleButton(bmpBuffer.get(), getWidth() - getHeight(), 0, getWidth(), getHeight(), iColor, iHighlightColor, iShadowColor, bBotButtonDown);
		DrawSimpleArrow(bmpBuffer.get(), getWidth() - (getHeight() + ARROW_H) / 2, (getHeight() - ARROW_W) / 2, ARROW_H, ARROW_W, ardRight, iArrowColor);
	}

	// Slider
	if(iMax > iItemsperbox && iMax > 0) {
		int length = (int)((float)iItemsperbox/(float)iMax * getHeight()-30);
		length = MAX(length, MIN_SLIDER_LENGTH);

		// Get the pos
		int pos = iScrollPos;
		if(pos+length > getWidth() - left_w - right_w)
			pos = getWidth() - left_w - right_w - length;
		if (pos < 0)
			pos = 0;

		// Have bitmaps?
		if (bmpSliderTop.get().get() && bmpSliderBottom.get().get() && bmpSliderMiddle.get().get())  {
			length = MAX(length, bmpSliderTop->h + bmpSliderBottom->h);

			int left_sx = 0;
			int main_sx = 0;
			int right_sx = 0;
			if (bSliderOver)  {
				left_sx = bmpSliderTop->w / 3;
				main_sx = bmpSliderBottom->w / 3;
				right_sx = bmpSliderMiddle->w / 3;
			}

			if (bSliderDown)  {
				left_sx = bmpSliderTop->w * 2 / 3;
				right_sx = bmpSliderBottom->w / 3;
				main_sx = bmpSliderMiddle->w / 3;
			}

			// Left part
			DrawImageAdv(bmpBuffer.get(), bmpSliderTop, left_sx, 0, left_w + pos, 0, bmpSliderTop->w/3, bmpSliderTop->h);

			// Main part
			DrawImageTiled(bmpBuffer.get(), bmpSliderMiddle, main_sx, 0, bmpSliderMiddle->w/3, bmpSliderMiddle->h, left_w + pos + bmpSliderTop->w/3, 0, MAX(0, length - bmpSliderTop->w/3 - bmpSliderBottom->w/3), getHeight());

			// Right part
			DrawImageAdv(bmpBuffer.get(), bmpSliderBottom, right_sx, 0, pos + length - bmpSliderBottom->w/3, 0, bmpSliderBottom->w/3, bmpSliderBottom->h);
		} else {
			DrawSimpleButton(bmpBuffer.get(), left_w + pos, 0, length, getHeight(), iColor, iHighlightColor, iShadowColor, false);
		}
	}
}


///////////////////
// Draw the scrollbar
void CScrollbar::DoRepaint()
{
	CHECK_BUFFER;

	CWidget::DoRepaint();

	// Background
	if (bmpBackground.get().get())  {
		int sx = 0;
		if (bBackgroundOver)
			sx = bmpBackground->w / 3;
		if (bBackgroundDown)
			sx = bmpBackground->w * 2 / 3;
		DrawImageTiled(bmpBuffer.get(), bmpBackground, sx, 0, bmpBackground->w/3, bmpBackground->h, 0, 0, getWidth(), getHeight());
	} else {
		DrawRectFill(bmpBuffer.get(), 0, 0, getWidth(), getHeight(), iBackgroundColor);
	}

	switch (iDirection)  {
	case scrVertical:  {
			DoRepaintVertical();
		}
		break;
	case scrHorizontal:  {
			DoRepaintHorizontal();
		}
		break;
	default:
		warnings << "CScrollbar::Draw, unknown scrollbar type" << endl;
	}

}

/////////////////
// Scroll up/left
void CScrollbar::ScrollDown()
{
	bool cancel = false;
	int newval = CLAMP(iValue+1, 0, iMax - iItemsperbox);
	if (newval == iValue)
		return;

	CALL_EVENT(OnScroll, (this, newval, cancel));
	if (!cancel)  {
		iValue = newval;
		UpdatePos();
	}

	Repaint();
}

///////////////
// Scroll down/right
void CScrollbar::ScrollUp()
{
	bool cancel = false;
	int newval = CLAMP(iValue-1, 0, iMax - iItemsperbox);
	if (newval == iValue)
		return;

	CALL_EVENT(OnScroll, (this, newval, cancel));
	if (!cancel)  {
		iValue = newval;
		UpdatePos();
	}

	Repaint();
}

/////////////////////
// Returns true if the given point is inside the slider
bool CScrollbar::InSlider(int x, int y)
{
	if (iDirection == scrVertical)  {
		int top_h = bmpTop.get().get() ? bmpTop->h : getWidth();
		int bottom_h = bmpBottom.get().get() ? bmpBottom->h : getWidth();
		int length = (int)((float)iItemsperbox/(float)iMax * getHeight() - top_h - bottom_h);
		length = MAX(length, MIN_SLIDER_LENGTH);
		if (bmpSliderBottom.get().get() && bmpSliderTop.get().get() && bmpSliderMiddle.get().get())
			length = MAX(length, bmpSliderTop->h + bmpSliderBottom->h);

		return (y > top_h + iScrollPos && y < top_h + iScrollPos + length) && (x >= 0 && x < getWidth());
	} else {
		int left_w = bmpTop.get().get() ? bmpTop->w/3 : getHeight();
		int right_w = bmpBottom.get().get() ? bmpBottom->w/3 : getHeight();
		int length = (int)((float)iItemsperbox/(float)iMax * getWidth() - left_w - right_w);
		length = MAX(length, MIN_SLIDER_LENGTH);
		if (bmpSliderBottom.get().get() && bmpSliderTop.get().get() && bmpSliderMiddle.get().get())
			length = MAX(length, bmpSliderTop->w/3 + bmpSliderBottom->w/3);

		return (x > left_w + iScrollPos && x < left_w + iScrollPos + length) && (y >= 0 && y < getHeight());
	}
}

///////////////////////
// Resturns true if the point is in the top resp. left button
bool CScrollbar::InTop(int x, int y)
{
	if (iDirection == scrVertical)  {
		int h = bmpTop.get().get() ? bmpTop->h : getWidth();
		return (y > 0 && y < h) && (x >= 0 && x < getWidth());
	} else {
		int w = bmpTop.get().get() ? bmpTop->w/3 : getHeight();
		return (x > 0 && x < w) && (y >= 0 && y < getHeight());
	}
}

///////////////////////
// Resturns true if the point is in the bottom resp. right button
bool CScrollbar::InBottom(int x, int y)
{
	if (iDirection == scrVertical)  {
		int h = bmpBottom.get().get() ? bmpBottom->h : getWidth();
		return (y > getHeight() - h && y < getHeight()) && (x >= 0 && x < getWidth());
	} else {
		int w = bmpBottom.get().get() ? bmpBottom->w/3 : getHeight();
		return (x > getWidth() - w && x < getWidth()) && (y >= 0 && y < getHeight());
	}
}

///////////////////////
// Apply a selector to the scrollbar
void CScrollbar::ApplySelector(const CSSParser::Selector &sel, const std::string &prefix)
{
	CWidget::ApplySelector(sel, prefix);

	const std::string& base = sel.getBaseURL();
	FOR_EACH_ATTRIBUTE(sel, it)  {
		if (it->getName() == prefix + "top-button")  {
			bmpTop.set(LoadGameImage(JoinPaths(base, it->getFirstValue().getURL()), true), it->getPriority());
		} else if (it->getName() == prefix + "bottom-button")  {
			bmpBottom.set(LoadGameImage(JoinPaths(base, it->getFirstValue().getURL()), true), it->getPriority());
		} else if (it->getName() == prefix + "background-image")  {
			bmpBackground.set(LoadGameImage(JoinPaths(base, it->getFirstValue().getURL())), it->getPriority());
		} else if (it->getName() == prefix + "slider-top-image")  {
			bmpSliderTop.set(LoadGameImage(JoinPaths(base, it->getFirstValue().getURL())), it->getPriority());
		} else if (it->getName() == prefix + "slider-bottom-image")  {
			bmpSliderBottom.set(LoadGameImage(JoinPaths(base, it->getFirstValue().getURL())), it->getPriority());
		} else if (it->getName() == prefix + "slider-main-image")  {
			bmpSliderMiddle.set(LoadGameImage(JoinPaths(base, it->getFirstValue().getURL())), it->getPriority());
		} else if (it->getName() == prefix + "face-color" || it->getName() == prefix + "face-colour")  {
			iColor.set(it->getFirstValue().getColor(iColor), it->getPriority());
		} else if (it->getName() == prefix + "background-color" || it->getName() == prefix + "background-colour")  {
			iBackgroundColor.set(it->getFirstValue().getColor(iBackgroundColor), it->getPriority());
		} else if (it->getName() == prefix + "highlight-color" || it->getName() == prefix + "highlight-colour")  {
			iHighlightColor.set(it->getFirstValue().getColor(iHighlightColor), it->getPriority());
		} else if (it->getName() == prefix + "shadow-color" || it->getName() == prefix + "shadow-colour")  {
			iShadowColor.set(it->getFirstValue().getColor(iShadowColor), it->getPriority());
		} else if (it->getName() == prefix + "arrow-color" || it->getName() == prefix + "arrow-colour")  {
			iArrowColor.set(it->getFirstValue().getColor(iArrowColor), it->getPriority());
		} else if (it->getName() == prefix + "direction")  {
			if (it->getFirstValue().getString() == "vertical")
				iDirection.set(scrVertical, it->getPriority());
			else if (it->getFirstValue().getString() == "horizontal")
				iDirection.set(scrHorizontal, it->getPriority());
			else
				iDirection.set(scrVertical, DEFAULT_PRIORITY); // Defaults to vertical
		}
	}
}

///////////////////
// Apply the given tag
void CScrollbar::ApplyTag(xmlNodePtr node)
{
	CWidget::ApplyTag(node);

	iMin = MAX(0, xmlGetInt(node, "min"));
	iMax = MAX(iMin + 1, xmlGetInt(node, "max", iMin + 1));
	iValue = CLAMP(xmlGetInt(node, "value"), iMin, iMax);
	iItemsperbox = CLAMP(xmlGetInt(node, "itemsperbox"), 0, iMax);

	std::string base = xmlGetBaseURL(node);
	if (xmlPropExists(node, "topbtn"))
		bmpTop.set(LoadGameImage(JoinPaths(base, xmlGetString(node, "topbtn"))), TAG_ATTR_PRIORITY);
	if (xmlPropExists(node, "bottombtn"))
		bmpBottom.set(LoadGameImage(JoinPaths(base, xmlGetString(node, "bottombtn"))), TAG_ATTR_PRIORITY);
	if (xmlPropExists(node, "slidertop"))
		bmpSliderTop.set(LoadGameImage(JoinPaths(base, xmlGetString(node, "slidertop"))), TAG_ATTR_PRIORITY);
	if (xmlPropExists(node, "sliderbottom"))
		bmpSliderBottom.set(LoadGameImage(JoinPaths(base, xmlGetString(node, "sliderbottom"))), TAG_ATTR_PRIORITY);
	if (xmlPropExists(node, "slidermain"))
		bmpSliderMiddle.set(LoadGameImage(JoinPaths(base, xmlGetString(node, "slidermain"))), TAG_ATTR_PRIORITY);
	if (xmlPropExists(node, "color"))
		iColor.set(xmlGetColour(node, "color", iColor), TAG_ATTR_PRIORITY);
	if (xmlPropExists(node, "highcolor"))
		iHighlightColor.set(xmlGetColour(node, "highcolor", iHighlightColor), TAG_ATTR_PRIORITY);
	if (xmlPropExists(node, "shadowcolor"))
		iShadowColor.set(xmlGetColour(node, "shadowcolor", iShadowColor), TAG_ATTR_PRIORITY);
	if (xmlPropExists(node, "bgcolor"))
		iBackgroundColor.set(xmlGetColour(node, "bgcolor", iBackgroundColor), TAG_ATTR_PRIORITY);
	if (xmlPropExists(node, "arrowcolor"))
		iArrowColor.set(xmlGetColour(node, "arrowcolor", iArrowColor), TAG_ATTR_PRIORITY);
	if (xmlPropExists(node, "dir"))  {
		std::string dir = xmlGetString(node, "dir", "vertical");
		if (stringcaseequal(dir, "vertical"))
			iDirection.set(scrVertical, TAG_ATTR_PRIORITY);
		else if (stringcaseequal(dir, "horizontal"))
			iDirection.set(scrHorizontal, TAG_ATTR_PRIORITY);
	}
}

///////////////////
// Get the top button height, resp. left button width
int CScrollbar::getTopH()
{
	if (iDirection == scrVertical)
		return bmpTop.get().get() ? bmpTop->h : getWidth();
	else
		return bmpTop.get().get() ? bmpTop->w/3 : getHeight();
}

/////////////////////
// Get the bottom button height, resp. right button width
int CScrollbar::getBottomH()
{
	if (iDirection == scrVertical)
		return bmpBottom.get().get() ? bmpBottom->h : getWidth();
	else
		return bmpBottom.get().get() ? bmpBottom->w/3 : getHeight();
}

///////////////////
// Mouse up
int CScrollbar::DoMouseUp(int x, int y, int dx, int dy, MouseButton button, const ModifiersState& modstate)
{
	bSliderGrabbed = false;
	bTopButtonDown = bBotButtonDown = bSliderDown = bBackgroundDown = false;
	tTimer->stop();
	Repaint();
	CWidget::DoMouseUp(x, y, dx, dy, button, modstate);
	return WID_PROCESSED;
}


///////////////////
// Mouse over
int CScrollbar::DoMouseMove(int x, int y, int dx, int dy, bool down, MouseButton button, const ModifiersState& modstate)
{
	if(!down) {
		bSliderGrabbed = false;
		bTopButtonDown = bBotButtonDown = bSliderDown = bBackgroundDown = false;
		tTimer->stop();

		bSliderOver = InSlider(x, y);
		bTopButtonOver = InTop(x, y);
		bBotButtonOver = InTop(x, y);
		bBackgroundOver = RelInBox(x, y) && !bSliderOver && !bTopButtonOver && !bBotButtonOver;
	} else {
		bSliderDown = bSliderGrabbed;
		int dist = (iDirection == scrVertical ? dy : dx);

		// Move the slider
		if(bSliderGrabbed && dist != 0) {
			int newval = iValue;

			if (iDirection == scrVertical)  {
				int w = getHeight() - getTopH() - getBottomH();
				if (w > 0)
					newval = CLAMP(((iMax + 1) * (y - getTopH())) / w, 0, iMax);
			} else {
				int w = getWidth() - getTopH() - getBottomH();
				if (w > 0)
					newval = CLAMP(((iMax + 1) * (x - getTopH())) / w, 0, iMax);
			}

			// Scroll event
			if (newval != iValue)  {
				bool cancel = false;
				CALL_EVENT(OnScroll, (this, newval, cancel));
				if (!cancel)  {
					iValue = newval;
					UpdatePos();
				}
			}
		}
	}

	Repaint();

	CWidget::DoMouseMove(x, y, dx, dy, down, button, modstate);
	return WID_PROCESSED;
}


///////////////////
// Mouse down
int CScrollbar::DoMouseDown(int x, int y, int dx, int dy, MouseButton button, const ModifiersState& modstate)
{
	int top_h = bmpTop.get().get() ? bmpTop->h : getWidth();
	int bot_h = bmpBottom.get().get() ? bmpBottom->h : getWidth();
	if (iDirection == scrHorizontal)  {
		top_h = bmpTop.get().get() ? bmpTop->w/3 : getHeight();
		bot_h = bmpBottom.get().get() ? bmpBottom->w/3 : getHeight();
	}

	if(InSlider(x, y)) {
		bSliderDown = true;
		if(!bSliderGrabbed) {
			bSliderGrabbed = true;
			if (iDirection == scrVertical)
				iSliderGrabPos = y - (top_h + iScrollPos);
			else
				iSliderGrabPos = x - (top_h + iScrollPos);
			Repaint();
			CWidget::DoMouseDown(x, y, dx, dy, button, modstate);
			return WID_PROCESSED;
		}
	} else
		bSliderDown = false;

	// Top arrow
	if(InTop(x, y)) {
		bTopButtonDown = true;
		((TimerData *)tTimer->userData)->down = false;
		((TimerData *)tTimer->userData)->step = 1;
		tTimer->start();

		// Move up
		ScrollUp();
		Repaint();
		CWidget::DoMouseDown(x, y, dx, dy, button, modstate);
		return WID_PROCESSED;
	} else
		bTopButtonDown = false;

	// Bottom arrow
	if(InBottom(x, y)) {
		bSliderDown = bTopButtonDown = false;
		bBotButtonDown = true;
		((TimerData *)tTimer->userData)->down = true;
		((TimerData *)tTimer->userData)->step = 1;
		tTimer->start();

		// Move down
		ScrollDown();
		Repaint();
		CWidget::DoMouseDown(x, y, dx, dy, button, modstate);
		return WID_PROCESSED;
	} else
		bBotButtonDown = false;


	// Background click
	int scroll_size = iDirection == scrVertical ? getHeight() : getWidth();

	int length = (int)((float)iItemsperbox/(float)iMax * scroll_size - top_h - bot_h);
    length = MAX(length, MIN_SLIDER_LENGTH);

	// Background click
	bBackgroundDown = true;
	int pos = iScrollPos;
	if(pos+length > getHeight() - bot_h - top_h)
		pos = getHeight() - bot_h - length;

	bool scrolldownright = false;
	if (iDirection == scrVertical)
		scrolldownright = y > top_h + pos + length;
	else
		scrolldownright = x > top_h + pos + length;

	if (scrolldownright)  {
		((TimerData *)tTimer->userData)->down = true;
		((TimerData *)tTimer->userData)->step = 2;
		tTimer->start();
		ScrollDown();
		ScrollDown();
	} else {
		((TimerData *)tTimer->userData)->down = false;
		((TimerData *)tTimer->userData)->step = 2;
		tTimer->start();
		ScrollUp();
		ScrollUp();
	}

	Repaint();
	CWidget::DoMouseDown(x, y, dx, dy, button, modstate);
	return WID_PROCESSED;
}

///////////////////
// Mouse wheel down
int CScrollbar::DoMouseWheelDown(int x, int y, int dx, int dy, const ModifiersState& modstate)
{
	if ((iDirection == scrVertical && !modstate.bShift) || (iDirection == scrHorizontal && modstate.bShift))
		ScrollDown();
	CWidget::DoMouseWheelDown(x, y, dx, dy, modstate);
	return WID_PROCESSED;
}

///////////////////
// Mouse wheel up
int CScrollbar::DoMouseWheelUp(int x, int y, int dx, int dy, const ModifiersState& modstate)
{
	if ((iDirection == scrVertical && !modstate.bShift) || (iDirection == scrHorizontal && modstate.bShift))
		ScrollUp();
	CWidget::DoMouseWheelDown(x, y, dx, dy, modstate);
	return WID_PROCESSED;
}

//////////////////
// Key down
int CScrollbar::DoKeyDown(UnicodeChar c, int keysym, const ModifiersState& modstate)
{
	switch (keysym)  {
	case SDLK_UP:
		if (iDirection == scrVertical)
			ScrollUp();
		break;

	case SDLK_DOWN:
		if (iDirection == scrVertical)
			ScrollDown();
		break;

	case SDLK_LEFT:
		if (iDirection == scrHorizontal)
			ScrollUp();
		break;

	case SDLK_RIGHT:
		if (iDirection == scrHorizontal)
			ScrollDown();
		break;
	}

	CWidget::DoKeyDown(c, keysym, modstate);
	return WID_NOT_PROCESSED;
}


///////////////////
// Update the slider pos
void CScrollbar::UpdatePos()
{
    iMax = MAX(iMax,1);
    iMin = MAX(iMin,0);

    if(iMax < iItemsperbox) {
        iValue = 0;
        iScrollPos = 0;
        return;
    }

    int mx = iMax-iItemsperbox;

	iValue = CLAMP(iValue, 0, mx);

    // Prevent div by zero errors
    if(mx == 0)
        return;

    int length = (int)((float)iItemsperbox/(float)iMax * getHeight()-30);
    length = MAX(length, 12);

	int mainbitlen = (iDirection == scrVertical ? getHeight() : getWidth());
	iScrollPos = (int)((float)(mainbitlen-30-length)/(float)mx * iValue);
	Repaint();
}

void CScrollbar::OnTimer(Timer::EventData ev)
{
	// TODO: why do we need a timer for scrolling?
	if (ev.userData)  {
		TimerData *d = (TimerData *)ev.userData;
		if (d->down)  {
			for (int i=0; i < d->step; i++)
				d->scr->ScrollDown();
		} else {
			for (int i=0; i < d->step; i++)
				d->scr->ScrollUp();
		}
	}
}

}; // namespace SkinnedGUI
