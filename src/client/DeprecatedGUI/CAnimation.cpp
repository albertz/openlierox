// OpenLieroX


// Input box
// Created 30/3/03
// Jason Boettcher

// code under LGPL


#include "LieroX.h"
#include "DeprecatedGUI/Menu.h"
#include "GfxPrimitives.h"
#include "DeprecatedGUI/CAnimation.h"


namespace DeprecatedGUI {

//////////////////
// Create
void CAnimation::Create()
{
	iType = wid_Image;
	if (tAnimation.get())  {
		iWidth = tAnimation.get()->w;
		iHeight = tAnimation.get()->h;
	} else {
		iWidth = iHeight = 0;
	}
}

///////////////////
// Parse the animation
void CAnimation::Parse()
{
	if (!tAnimation.get())
		return;

	tFrameOffsets.clear();
	tFrameWidths.clear();

	LOCK_OR_QUIT(tAnimation);

	tFrameOffsets.push_back(0);

	// Go through and look for pink frame dividing line
	Uint8 *px = (Uint8 *)tAnimation.get()->pixels;
	int last_x = 0;
	iNumFrames = 0;
	Uint32 pink = SDL_MapRGB(tAnimation.get()->format, 255, 0, 255); // Animation is alphasurface, cannot use MakeColour
	for (int x = 0; x < tAnimation.get()->w; x++, px += tAnimation.get()->format->BytesPerPixel) {
		if (EqualRGB(GetPixelFromAddr(px, tAnimation.get()->format->BytesPerPixel), pink, tAnimation.get()->format)) {
			if (!x) continue; // Ignore lines at the beginning

			tFrameOffsets.push_back(x + 1);
			tFrameWidths.push_back(x - last_x - 1);
			iNumFrames++;

			last_x = x;
		}
	}

	UnlockSurface(tAnimation);

	// Last frame
	iNumFrames++;
	tFrameWidths.push_back(tAnimation.get()->w - last_x - 1);
}

///////////////////
// Draw the image
void CAnimation::Draw(SDL_Surface * bmpDest)
{
	// Don't try to draw non-existing image
	if (!tAnimation.get() || !iNumFrames)
		return;

	if (bRedrawMenu)
		Menu_redrawBufferRect(iX,iY, tAnimation.get()->w,tAnimation.get()->h);

	// Check if it's time to change the frame
	if (tLX->fCurTime - fLastFrameChange >= fFrameTime) {
		iCurFrame++;
		if (iCurFrame >= iNumFrames)
			iCurFrame = 0;
		fLastFrameChange = tLX->fCurTime;
	}

	// Draw the image
	DrawImageAdv(bmpDest,tAnimation, tFrameOffsets[iCurFrame], 0, iX, iY, tFrameWidths[iCurFrame], tAnimation.get()->h);
}

///////////////////
// Changes the image
void CAnimation::Change(const std::string& Path, float frametime)
{
	if(Path == "")
		return;

	// Copy the new path
	sPath = Path;

	// Load the new image
	tAnimation = LoadGameImage(sPath, true);

	// Update the width and height
	if (tAnimation.get()) {
		iWidth = tAnimation.get()->w;
		iHeight = tAnimation.get()->h;
	} else {
		iWidth = iHeight = 0;
	}

	iCurFrame = 0;
	fFrameTime = frametime;
	fLastFrameChange = -9999;

	Parse();
}

}; // namespace DeprecatedGUI
