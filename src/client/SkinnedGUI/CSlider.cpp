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


#include "LieroX.h"

#include "MathLib.h"
#include "FindFile.h"
#include "StringUtils.h"
#include "GfxPrimitives.h"
#include "SkinnedGUI/CSlider.h"
#include "XMLutils.h"
#include "GuiPrimitives.h"


namespace SkinnedGUI {

#define BUTTON_W 5

////////////////////
// Create
CSlider::CSlider(COMMON_PARAMS, int max, int min)  : CWidget(name, parent)
{
	iMax = max;
	iMin = min;
	iValue = 0;
	iType = wid_Slider;
	clButFace.set(Color(191, 191, 191), DEFAULT_PRIORITY);
	clButLight.set(Color(255, 255, 255), DEFAULT_PRIORITY);
	clButDark.set(Color(127, 127, 127), DEFAULT_PRIORITY);
	CLEAR_EVENT(OnChange);
}

CSlider::CSlider(const std::string &name, CContainerWidget *parent) : CWidget(name, parent)
{
	iMax = iMin = iValue = 0;
	iType = wid_Slider;
	clButFace.set(Color(191, 191, 191), DEFAULT_PRIORITY);
	clButLight.set(Color(255, 255, 255), DEFAULT_PRIORITY);
	clButDark.set(Color(127, 127, 127), DEFAULT_PRIORITY);
	CLEAR_EVENT(OnChange);
}

///////////////////
// Draw the slider
void CSlider::DoRepaint()
{
	CHECK_BUFFER;
	CWidget::DoRepaint();

	const int button_w = bmpButton.get().get() ? bmpButton->w / 2 : BUTTON_W;

	DrawHLine( bmpBuffer.get(), button_w / 2, getWidth() - button_w, getHeight()/2 - 1, clDark.get().get(bmpBuffer->format));
	DrawHLine( bmpBuffer.get(), button_w / 2, getWidth() - button_w, getHeight()/2, clLight.get().get(bmpBuffer->format));
	DrawHLine( bmpBuffer.get(), button_w / 2, getWidth() - button_w, getHeight()/2 + 1, clDark.get().get(bmpBuffer->format));

	// Draw the button
	if (bmpButton.get().get())  {
		//int x = button_w / 2; // TODO: unused
		int w = getWidth() - button_w;
		int val = (int)( ((float)w/(float)iMax) * (float)iValue );

		int y = (getHeight() - bmpButton->h)/2;
		DrawImageAdv(bmpBuffer.get(), bmpButton, bMouseOver ? button_w : 0, 0, val, y, button_w, bmpButton->h);
	} else {
		int w = getWidth() - button_w;
		int val = (int)( ((float)w/(float)iMax) * (float)iValue );
		DrawSimpleButton(bmpBuffer.get(), val - button_w / 2, 0, button_w, getHeight(), clButFace, clButLight, clButDark, false);
	}
}

//////////////////
// Apply the given selector to the slider
void CSlider::ApplySelector(const CSSParser::Selector& sel, const std::string& prefix)
{
	CWidget::ApplySelector(sel, prefix);

	FOR_EACH_ATTRIBUTE(sel, it)  {
		if (it->getName() == prefix + "button-image")  {
			bmpButton.set(LoadGameImage(JoinPaths(sel.getBaseURL(), it->getFirstValue().getURL()), true), it->getPriority());
		} else if (it->getName() == prefix + "highlight-color" || it->getName() == prefix + "highlight-colour") {
			clLight.set(it->getFirstValue().getColor(clLight), it->getPriority());
		} else if (it->getName() == prefix + "shadow-color" || it->getName() == prefix + "shadow-colour") {
			clLight.set(it->getFirstValue().getColor(clDark), it->getPriority());
		} else if (it->getName() == prefix + "button-highlight-color" || it->getName() == prefix + "button-highlight-colour")  {
			clButLight.set(it->getFirstValue().getColor(clButLight), it->getPriority());
		} else if (it->getName() == prefix + "button-shadow-color" || it->getName() == prefix + "button-shadow-colour")  {
			clButDark.set(it->getFirstValue().getColor(clButDark), it->getPriority());
		} else if (it->getName() == prefix + "button-color" || it->getName() == prefix + "button-colour")  {
			clButFace.set(it->getFirstValue().getColor(clButFace), it->getPriority());
		}
	}
}

//////////////////
// Apply the tag
void CSlider::ApplyTag(xmlNodePtr node)
{
	CWidget::ApplyTag(node);

	iMin = MAX(0, xmlGetInt(node, "min"));
	iMax = MAX(iMin + 1, xmlGetInt(node, "max", iMin + 1));
	iValue = CLAMP(xmlGetInt(node, "value", iMin), iMin, iMax);

	if (xmlPropExists(node, "highcolor"))
		clLight.set(xmlGetColour(node, "highcolor"), TAG_ATTR_PRIORITY);
	if (xmlPropExists(node, "shadowcolor"))
		clDark.set(xmlGetColour(node, "shadowcolor"), TAG_ATTR_PRIORITY);
	if (xmlPropExists(node, "button"))
		bmpButton.set(LoadGameImage(JoinPaths(xmlGetBaseURL(node), xmlGetString(node, "button"))), TAG_ATTR_PRIORITY);
}

///////////////////
// Moves the slider, takes mouse coordinates
void CSlider::DoMove(int ms_x, int ms_y)
{
	const int button_w = bmpButton.get().get() ? bmpButton->w / 2 : BUTTON_W;

	int x = button_w / 2;
	int w = getWidth() - button_w;

	int val = (int)( (float)iMax / ( (float)w / (float)(ms_x - x)) );

	if(ms_x > x + w)
		val = iMax;
	if(ms_x <= x)
		val = 0;

	// Clamp the value
	val = CLAMP(val, 0, iMax);

	if (val != iValue)  {
		bool cancel = false;
		CALL_EVENT(OnChange, (this, val, cancel));
		if (!cancel)  {
			iValue = val;
			Repaint();
		}
	}
}

//////////////
// Mouse down event
int CSlider::DoMouseDown(int x, int y, int dx, int dy, MouseButton button, const ModifiersState& modstate)
{
	DoMove(x, y);
	CWidget::DoMouseDown(x, y, dx, dy, button, modstate);
	return WID_PROCESSED;
}

//////////////
// Mouse move event
int	CSlider::DoMouseMove(int x, int y, int dx, int dy, bool down, MouseButton button, const ModifiersState& modstate)
{
	if (down)  {
		DoMove(x, y);
	}

	CWidget::DoMouseMove(x, y, dx, dy, down, button, modstate);
	return WID_PROCESSED;
}

//////////////////
// Create event
int CSlider::DoCreate()
{
	CWidget::DoCreate();

	if (getWidth() == 0 || getHeight() == 0)  {
		if (bmpButton.get().get())
			Resize(getX(), getY(), 100, bmpButton->h);
		else
			Resize(getX(), getY(), 100, 10);
	}

	return WID_PROCESSED;
}

}; // namespace SkinnedGUI
