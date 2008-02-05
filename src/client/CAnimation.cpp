// OpenLieroX


// Input box
// Created 30/3/03
// Jason Boettcher

// code under LGPL


#include "LieroX.h"
#include "Menu.h"
#include "GfxPrimitives.h"
#include "CAnimation.h"


//////////////////
// Create
void CAnimation::Create()
{
	iType = wid_Image;
	if (tAnimation)  {
		iWidth = tAnimation->w;
		iHeight = tAnimation->h;
	} else {
		iWidth = iHeight = 0;
	}
}

///////////////////
// Parse the animation
void CAnimation::Parse()
{
	if (!tAnimation)
		return;

	tFrameOffsets.clear();
	tFrameWidths.clear();

	LockSurface(tAnimation);

	tFrameOffsets.push_back(0);

	// Go through and look for pink frame dividing line
	Uint8 *px = (Uint8 *)tAnimation->pixels;
	int last_x = 0;
	iNumFrames = 0;
	Uint32 pink = SDL_MapRGB(tAnimation->format, 255, 0, 255); // Animation is alphasurface, cannot use MakeColour
	for (int x = 0; x < tAnimation->w; x++, px += tAnimation->format->BytesPerPixel) {
		if (EqualRGB(GetPixelFromAddr(px, tAnimation->format->BytesPerPixel), pink, tAnimation->format)) {
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
	tFrameWidths.push_back(tAnimation->w - last_x - 1);
}

///////////////////
// Draw the image
void CAnimation::Draw(SDL_Surface *bmpDest)
{
	// Don't try to draw non-existing image
	if (!tAnimation || !iNumFrames)
		return;

	if (bRedrawMenu)
		Menu_redrawBufferRect(iX,iY, tAnimation->w,tAnimation->h);

	// Check if it's time to change the frame
	if (tLX->fCurTime - fLastFrameChange >= fFrameTime) {
		iCurFrame++;
		if (iCurFrame >= iNumFrames)
			iCurFrame = 0;
		fLastFrameChange = tLX->fCurTime;
	}

	// Draw the image
	DrawImageAdv(bmpDest,tAnimation, tFrameOffsets[iCurFrame], 0, iX, iY, tFrameWidths[iCurFrame], tAnimation->h);
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
	tAnimation = LoadImage(sPath, true);

	// Update the width and height
	if (tAnimation) {
		iWidth = tAnimation->w;
		iHeight = tAnimation->h;
	} else {
		iWidth = iHeight = 0;
	}

	iCurFrame = 0;
	fFrameTime = frametime;
	fLastFrameChange = -9999;

	Parse();
}
