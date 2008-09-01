/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// CProgressBar source file
// Created 22/6/07
// Dark Charlie

#include "LieroX.h"
#include "GfxPrimitives.h"
#include "SkinnedGUI/CProgressbar.h"
#include "StringUtils.h"
#include "MathLib.h"
#include "XMLutils.h"
#include "FindFile.h"


namespace SkinnedGUI {

//////////////
// Constructor
CProgressBar::CProgressBar(COMMON_PARAMS, int num_fore_states, int num_bg_states) :
CWidget(name, parent)  {
	bmpBar.set(NULL, DEFAULT_PRIORITY);
	iDirection.set(BAR_LEFTTORIGHT, DEFAULT_PRIORITY);
	iPosition = 100;
	iNumForeStates = num_fore_states;
	iNumBgStates = num_bg_states;
	iCurrentForeState = 0;
	iCurrentBgState = 0;

	// Default colors
	bgColor.set(Color(128, 128, 128), DEFAULT_PRIORITY);
	foreColor.set(Color(0, 255, 0), DEFAULT_PRIORITY);
}

////////////////
// Set the progressbar position
void CProgressBar::setPosition(int _p)
{
	if (cLabel)
		cLabel->setText(itoa(_p));
	iPosition = _p;
	Repaint();
}

/////////////////////
// Set the current foreground state
void CProgressBar::setCurrentForeState(int _s)
{
	iCurrentForeState = MIN(iNumForeStates-1, _s);
	Repaint();
}

////////////////////////
// Set the current background state
void CProgressBar::setCurrentBgState(int _s)
{
	iCurrentBgState = MIN(iNumBgStates-1, _s);
	Repaint();
}

//////////////
// Draw the bar
void CProgressBar::DoRepaint()
{
	CHECK_BUFFER;

	std::string progress = itoa(iPosition) + " %";
	int pos = CLAMP(iPosition, 0, 100);

	// In the bar image, there's the fully loaded image - it's always the first image starting at 0,0.
	// Then there can be unlimited number of states for various purposes
	// The last image is the background - fully unloaded image

	if (bmpBar.get().get())  {  // We got a bitmap - many options and possibilities for drawing
		int numstates = iNumForeStates + iNumBgStates;

		int bar_h = (bmpBar->h - iNumForeStates) / (numstates);
		int bar_w = (bmpBar->w - iNumForeStates) / (numstates);

		Uint32 clLabel = cLabel ? cLabel->getColor().get(bmpBuffer) : SDL_MapRGB(bmpBuffer->format, 255, 255, 255);
		
		// Draw the progress bitmap over the background depending on progress direction
		int w, h;
		switch (iDirection)  {
		case BAR_LEFTTORIGHT:
			// Background
			DrawImageAdv(bmpBuffer.get(), bmpBar, 0, bar_h * (iNumForeStates + iCurrentBgState), 0, 0, bmpBar->w, bar_h);

			DrawImageAdv(bmpBuffer.get(), bmpBar, 0,  bar_h * iCurrentForeState, 0, 0, (bmpBar->w * pos) / 100, bar_h);  // Progress
			LOCK_OR_QUIT(bmpBar);
			clLabel = GetPixel(bmpBar.get().get(), MAX(0, (bmpBar->w*pos)/100 - 1), bmpBar->h - iNumForeStates + iCurrentForeState);
			UnlockSurface(bmpBar);
			break;
		case BAR_RIGHTTOLEFT:
			// Background
			DrawImageAdv(bmpBuffer.get(), bmpBar, 0, bar_h * (iNumForeStates + iCurrentBgState), 0, 0, bmpBar->w, bar_h);

			w = (bmpBar->w * pos) / 100;
			DrawImageAdv(bmpBuffer.get(), bmpBar, bmpBar->w - w,  bar_h * iCurrentForeState, bmpBar->w - w, 0, w, bar_h);  // Progress
			LOCK_OR_QUIT(bmpBar);
			clLabel = GetPixel(bmpBar.get().get(), MIN(bmpBar->w - 1, bmpBar->w - w + 1), bmpBar->h - iNumForeStates + iCurrentForeState);
			UnlockSurface(bmpBar);
			break;
		case BAR_TOPTOBOTTOM: 
			// Background
			DrawImageAdv(bmpBuffer.get(), bmpBar, bar_w * (iNumForeStates + iCurrentBgState), 0, 0, 0, bar_w, bmpBar->h); // The last image is the empty one

			DrawImageAdv(bmpBuffer.get(), bmpBar, bar_w * iCurrentForeState, 0, 0, 0, bar_w, (bmpBar->h / 100) * pos); // Progress
			LOCK_OR_QUIT(bmpBar);
			clLabel = GetPixel(bmpBar.get().get(), bmpBar->w - iNumForeStates + iCurrentForeState, MAX(0, (bmpBar->h * pos)/100 - 1));
			UnlockSurface(bmpBar);
			break;
		case BAR_BOTTOMTOTOP:
			// Background
			DrawImageAdv(bmpBuffer.get(), bmpBar, bar_w * (iNumForeStates + iCurrentBgState), 0, 0, 0, bar_w, bmpBar->h); // The last image is the empty one

			h = (bmpBar->h * pos) / 100;
			DrawImageAdv(bmpBuffer.get(), bmpBar,  bar_w * iCurrentForeState, bmpBar->h - h, 0, bmpBar->h - h, bar_w, h);  // Progress
			LOCK_OR_QUIT(bmpBar);
			clLabel = GetPixel(bmpBar.get().get(), bmpBar->w - iNumForeStates + iCurrentForeState, MIN(bmpBar->h - 1, bmpBar->h - h - 1));
			UnlockSurface(bmpBar);
			break;
		default:
			printf("Bad bar type in CProgressBar::DoRepaint");
			return;
		}

		// Set the label color
		if (cLabel)
			cLabel->setColor(Color(bmpBuffer, clLabel));

		
	} else {  // No bitmap, just draw the simplest bar without any options
		DrawRectFill(bmpBuffer.get(), 0, 0, getWidth(), getHeight(), bgColor);
		DrawRectFill(bmpBuffer.get(), 1, 1, (getWidth() / 100) * pos + 1, getHeight() - 1, foreColor);
	}
}

//////////////////
// Apply the given selector
void CProgressBar::ApplySelector(const CSSParser::Selector& sel, const std::string& prefix)
{
	CWidget::ApplySelector(sel, prefix);

	FOR_EACH_ATTRIBUTE(sel, it)  {
		if (it->getName() == prefix + "direction")  {
			if (stringcaseequal(it->getFirstValue().getString(), "lefttoright"))
				iDirection.set(BAR_LEFTTORIGHT, it->getPriority());
			else if (stringcaseequal(it->getFirstValue().getString(), "righttoleft"))
				iDirection.set(BAR_RIGHTTOLEFT, it->getPriority());
			else if (stringcaseequal(it->getFirstValue().getString(), "toptobottom"))
				iDirection.set(BAR_TOPTOBOTTOM, it->getPriority());
			else if (stringcaseequal(it->getFirstValue().getString(), "bottomtotop"))
				iDirection.set(BAR_BOTTOMTOTOP, it->getPriority());
			else
				printf("Unknown bar direction in the CSS\n");
		} else if (it->getName() == prefix + "image")  {
			bmpBar.set(LoadGameImage(JoinPaths(sel.getBaseURL(), it->getFirstValue().getURL()), true), it->getPriority());
		} else if (it->getName() == prefix + "background-color")  {
			bgColor.set(it->getFirstValue().getColor(bgColor), it->getPriority());
		} else if (it->getName() == prefix + "foreground-color")  {
			foreColor.set(it->getFirstValue().getColor(foreColor), it->getPriority());
		}
	}
}

////////////////////////
// Apply the given node
void CProgressBar::ApplyTag(xmlNodePtr node)
{
	CWidget::ApplyTag(node);

	// Direction
	if (xmlPropExists(node, "dir"))  {
		std::string dir = xmlGetString(node, "dir", "lefttoright");
		if (stringcaseequal(dir, "lefttoright"))
			iDirection.set(BAR_LEFTTORIGHT, TAG_ATTR_PRIORITY);
		else if (stringcaseequal(dir, "righttoleft"))
			iDirection.set(BAR_RIGHTTOLEFT, TAG_ATTR_PRIORITY);
		else if (stringcaseequal(dir, "toptobottom"))
			iDirection.set(BAR_TOPTOBOTTOM, TAG_ATTR_PRIORITY);
		else if (stringcaseequal(dir, "bottomtotop"))
			iDirection.set(BAR_BOTTOMTOTOP, TAG_ATTR_PRIORITY);
	}

	// Image
	if (xmlPropExists(node, "src"))  {
		bmpBar.set(LoadGameImage(JoinPaths(xmlGetBaseURL(node), xmlGetString(node, "src"))), TAG_ATTR_PRIORITY);
	}

	// Bgcolor
	if (xmlPropExists(node, "bgcolor"))  {
		bgColor.set(xmlGetColour(node, "bgcolor", bgColor), TAG_ATTR_PRIORITY);
	}

	// Fore color
	if (xmlPropExists(node, "color"))  {
		foreColor.set(xmlGetColour(node, "color", foreColor), TAG_ATTR_PRIORITY);
	}

	// Position
	iPosition = CLAMP(xmlGetInt(node, "progress", iPosition), 0, 100);

	// States
	iNumForeStates = MAX(0, xmlGetInt(node, "forestates", 1));
	iNumBgStates = MAX(0, xmlGetInt(node, "bgstates", 1));

}

//////////////////
// Get width of this bar
int CProgressBar::CalculateWidth()
{
	if (bmpBar.get().get()) {
		int numstates = iNumForeStates + iNumBgStates;
		switch (iDirection)  {
		case BAR_LEFTTORIGHT:
		case BAR_RIGHTTOLEFT:
			return bmpBar->w;
		case BAR_TOPTOBOTTOM:
		case BAR_BOTTOMTOTOP:
			return (bmpBar->w-numstates)/(numstates + 1);
		default:
			return bmpBar->w;
		}
	} else {
		return getWidth() > 0 ? getWidth() : 100;
	}
}

//////////////////
// Get width of this bar
int CProgressBar::CalculateHeight()
{
	if (bmpBar.get().get()) {
		int numstates = iNumForeStates + iNumBgStates;
		switch (iDirection)  {
		case BAR_LEFTTORIGHT:
		case BAR_RIGHTTOLEFT:
			return (bmpBar->h-iNumForeStates)/(numstates);
		case BAR_TOPTOBOTTOM:
		case BAR_BOTTOMTOTOP:
			return bmpBar->h;
		default:
			return bmpBar->h;
		}
	} else {
		return getHeight() > 0 ? getHeight() : 10;
	}
}

////////////////////
// Create event
int CProgressBar::DoCreate()
{
	CWidget::DoCreate();
	Resize(getX(), getY(), CalculateWidth(), CalculateHeight());

	return WID_PROCESSED;
}

}; // namespace SkinnedGUI
