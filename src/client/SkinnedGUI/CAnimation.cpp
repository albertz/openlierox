// OpenLieroX


// Input box
// Created 30/3/03
// Jason Boettcher

// code under LGPL


#include "LieroX.h"

#include "GfxPrimitives.h"
#include "MathLib.h"
#include "FindFile.h"
#include "StringUtils.h"
#include "SkinnedGUI/CAnimation.h"
#include "XMLutils.h"


namespace SkinnedGUI {

#define DEFAULT_FRAME_TIME 0.2f

/////////////////////
// Create
CAnimation::CAnimation(COMMON_PARAMS, const std::string& Path) : CALL_DEFAULT_CONSTRUCTOR {
	iType = wid_Animation;
	sPath = Path;
	tImage = NULL;
	if (Path.size())  {
		tImage = LoadGameImage(Path, true);
	}
	fLastFrameChange = -9999;
	iCurFrame = 0;
	fFrameTime = DEFAULT_FRAME_TIME;
	bAnimated = false;
	tTimer = new Timer("CAnimation frame counter", getEventHandler(this, &CAnimation::OnTimer));
	CLEAR_EVENT(OnClick);
}

CAnimation::CAnimation(COMMON_PARAMS, SmartPointer<SDL_Surface> img) : CALL_DEFAULT_CONSTRUCTOR {
	iType = wid_Animation;
	sPath = "";
	tImage = img;
	fLastFrameChange = -9999;
	iCurFrame = 0;
	fFrameTime = DEFAULT_FRAME_TIME;
	bAnimated = false;
	tTimer = new Timer("CAnimation frame counter", getEventHandler(this, &CAnimation::OnTimer));
	CLEAR_EVENT(OnClick);
}

CAnimation::CAnimation(const std::string &name, CContainerWidget *parent) : CWidget(name, parent)
{
	iType = wid_Animation;
	sPath = "";
	tImage = NULL;
	fLastFrameChange = -9999;
	iCurFrame = 0;
	fFrameTime = DEFAULT_FRAME_TIME;
	bAnimated = false;
	tTimer = new Timer("CAnimation frame counter", getEventHandler(this, &CAnimation::OnTimer));
	CLEAR_EVENT(OnClick);
}

//////////////////
// Destroy
CAnimation::~CAnimation()
{
	delete tTimer;
}

///////////////////
// Parse the animation
void CAnimation::ParseAnimation()
{
	if (!tImage.get() || !bAnimated || !bCreated)
		return;

	tFrameOffsets.clear();
	tFrameWidths.clear();

	int width = 0;
	int height = tImage->h;

	LOCK_OR_QUIT(tImage);

	tFrameOffsets.push_back(0);

	// Go through and look for pink frame dividing line
	Uint8 *px = (Uint8 *)tImage->pixels;
	int last_x = 0;
	iNumFrames = 0;
	Uint32 pink = SDL_MapRGB(tImage->format, 255, 0, 255); // Animation is alphasurface, cannot use MakeColour
	for (int x = 0; x < tImage->w; x++, px += tImage->format->BytesPerPixel) {
		if (EqualRGB(GetPixelFromAddr(px, tImage->format->BytesPerPixel), pink, tImage->format)) {
			if (!x) continue; // Ignore lines at the beginning

			int w = x - last_x - 1;
			tFrameOffsets.push_back(x + 1);
			tFrameWidths.push_back(w);
			iNumFrames++;

			if (w > width)
				width = w;

			last_x = x;
		}
	}

	UnlockSurface(tImage);

	// Last frame
	iNumFrames++;
	tFrameWidths.push_back(tImage->w - last_x - 1);

	// Resize the image
	Resize(getX(), getY(), width + cBorder.getLeftW() + cBorder.getRightW(), 
		height + cBorder.getTopW() + cBorder.getBottomW());
}

///////////////////
// Draw the image
void CAnimation::DoRepaint()
{
	CHECK_BUFFER;

	CWidget::DoRepaint();

	// Don't try to draw non-existing image
	if (!tImage.get() || !iNumFrames)
		return;

	// Draw the image
	if (bAnimated)
		CopySurface(bmpBuffer.get(), tImage, tFrameOffsets[iCurFrame], 0, 0, 0, tFrameWidths[iCurFrame], tImage->h);
	else
		CopySurface(bmpBuffer.get(), tImage, 0, 0, 0, 0, tImage->w, tImage->h);

	cBorder.Draw(bmpBuffer.get(), 0, 0, getWidth(), getHeight());
}

/////////////////////
// Set the period
void CAnimation::setFrameTime(float t)
{
	fFrameTime = t;
	if (tTimer)  {
		tTimer->stop();
		tTimer->interval = (int)(t * 1000);
		if (bCreated && bAnimated)
			tTimer->start();
	}
}

/////////////////////////
// Read the information from a xml node
void CAnimation::ApplyTag(xmlNodePtr node)
{
	CWidget::ApplyTag(node);

	std::string base_url = xmlGetBaseURL(node);

	// Read the attributes
	sPath = xmlGetString(node, "src");
	if (sPath.size())
		tImage = LoadGameImage(JoinPaths(base_url, sPath), true);
	bAnimated = xmlGetBool(node, "animated", false);
	setFrameTime(xmlGetFloat(node, "period", fFrameTime));

	if (bAnimated)
		ParseAnimation();
}

//////////////////
// Applies the given CSS selector
void CAnimation::ApplySelector(const CSSParser::Selector &sel, const std::string& prefix)
{
	// Default widget selectors
	CWidget::ApplySelector(sel, prefix);
	
	// Border
	cBorder.ApplySelector(sel, prefix);
}

///////////////////
// Changes the image
void CAnimation::Change(const std::string& Path)
{
	if(Path.size() == 0)
		return;

	// Copy the new path
	sPath = Path;

	// Load the new image
	tImage = LoadGameImage(sPath, true);

	iCurFrame = 0;
	fLastFrameChange = -9999;

	if (bAnimated)
		ParseAnimation();
	AutoSize();
	Repaint();
}

//////////////////
// Automatically adjust the size
void CAnimation::AutoSize()
{
	if (!tImage.get())
		return;

	if (bAnimated && tFrameWidths.size())
		Resize(getX(), getY(), getWidth() ? getWidth() : tFrameWidths[0], getHeight() ? getHeight() : tImage->h);
	else
		Resize(getX(), getY(), getWidth() ? getWidth() : tImage->w, getHeight() ? getHeight() : tImage->h);
}

////////////////
// Create event
int CAnimation::DoCreate()
{
	CWidget::DoCreate();
	if (bAnimated)
		ParseAnimation();
	AutoSize();

	if (bAnimated)
		tTimer->start();

	return WID_PROCESSED;
}

////////////////
// Mouse up event
int	CAnimation::DoMouseUp(int x, int y, int dx, int dy, MouseButton button, const ModifiersState& modstate)
{
	if (RelInBox(x, y))
		CALL_EVENT(OnClick, (this, button, modstate));

	CWidget::DoMouseUp(x, y, dx, dy, button, modstate);
	return WID_PROCESSED;
}

void CAnimation::OnTimer(Timer::EventData ev)
{
	incFrame();
}

}; // namespace SkinnedGUI
