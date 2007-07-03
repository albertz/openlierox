/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// CBar source file
// Created 22/6/07
// Dark Charlie

#include "GfxPrimitives.h"
#include "CBar.h"
#include "StringUtils.h"
#include "MathLib.h"
#include "LieroX.h"


//////////////
// Constructor
CBar::CBar(SDL_Surface *bmp, int x, int y, int label_x, int label_y, int dir, int num_states)  {
	bmpBar = bmp;
	X = x;
	Y = y;
	LabelX = label_x;
	LabelY = label_y;
	Direction = dir;
	Position = 100;
	LabelVisible = true;
	NumStates = num_states;
	CurrentState = 0;

	// Default colors
	bgColor = MakeColour(128, 128, 128);
	foreColor = MakeColour(0, 255, 0);
}

//////////////
// Draw the bar
void CBar::Draw(SDL_Surface *dst)  {
	static std::string progress;
	progress = itoa(Position) + " %";
	int pos = MIN(MAX(Position, 0), 100);

	// In the bar image, there's the fully loaded image - it's always the first image starting at 0,0.
	// Then there can be unlimited number of states for various purposes
	// The last image is the background - fully unloaded image

	if (bmpBar)  {  // We got a bitmap - many options and possibilities for drawing

		int bar_h = (bmpBar->h - NumStates) / (NumStates + 1);
		int bar_w = (bmpBar->w - NumStates) / (NumStates + 1);

		Uint32 clLabel = tLX->clWhite;
		
		// Draw the progress bitmap over the background depending on progress direction
		int w, h;
		switch (Direction)  {
		case BAR_LEFTTORIGHT:
			// Background
			DrawImageAdv(dst, bmpBar, 0, bar_h * NumStates, X, Y, bmpBar->w, bar_h); // The last image is the empty one

			DrawImageAdv(dst, bmpBar, 0,  bar_h * CurrentState, X, Y, (bmpBar->w * pos) / 100, bar_h);  // Progress
			clLabel = GetPixel(bmpBar, MAX(0, (bmpBar->w*pos)/100 - NumStates + CurrentState), bmpBar->h - NumStates + CurrentState);
			break;
		case BAR_RIGHTTOLEFT:
			// Background
			DrawImageAdv(dst, bmpBar, 0, bar_h * NumStates, X, Y, bmpBar->w, bar_h); // The last image is the empty one

			w = (bmpBar->w * pos) / 100;
			DrawImageAdv(dst, bmpBar, bmpBar->w - w,  bar_h * CurrentState, X + bmpBar->w - w, Y, w, bar_h);  // Progress
			clLabel = GetPixel(bmpBar, MIN(bmpBar->w - 1, bmpBar->w - w + 1), bmpBar->h - NumStates + CurrentState);
			break;
		case BAR_TOPTOBOTTOM: 
			// Background
			DrawImageAdv(dst, bmpBar, bar_w * NumStates, 0, X, Y, bar_w, bmpBar->h); // The last image is the empty one

			DrawImageAdv(dst, bmpBar, bar_w * CurrentState, 0, X, Y, bar_w, (bmpBar->h / 100) * pos); // Progress
			clLabel = GetPixel(bmpBar, bmpBar->w - NumStates + CurrentState, MAX(0, (bmpBar->h * pos)/100 - 1));
			break;
		case BAR_BOTTOMTOTOP:
			// Background
			DrawImageAdv(dst, bmpBar, bar_w * NumStates, 0, X, Y, bar_w, bmpBar->h); // The last image is the empty one

			h = (bmpBar->w * pos) / 100;
			DrawImageAdv(dst, bmpBar,  bar_w * CurrentState, bmpBar->h - h, X, Y + bmpBar->h - h, bar_w, h);  // Progress
			clLabel = GetPixel(bmpBar, bmpBar->w - NumStates + CurrentState, MIN(bmpBar->h- 1, bmpBar->h - h - 1));
			break;
		default:
			printf("Bad bar type in CBar::Draw");
			return;
		}

		// bmpBar is an alpha surface so we need to convert the color from GetPixel
		Uint8 r, g, b;
		GetColour3(clLabel, bmpBar, &r, &g, &b);
		clLabel = MakeColour(r, g, b);

		if (LabelVisible)
			tLX->cFont.Draw(dst, LabelX, LabelY, clLabel, progress); // Label

		
	} else {  // No bitmap, just draw the simplest bar without any options
		int width = 100;
		int height = 10;

		DrawRectFill(dst, X, Y, (X + width + 1), Y + height, bgColor);
		DrawRectFill(dst, X + 1, Y + 1, X + (width / 100) * pos + 1, Y + height - 1, foreColor);
	}


}

//////////////////
// Get width of this bar
int CBar::GetWidth()
{
	if (bmpBar) {
		switch (Direction)  {
		case BAR_LEFTTORIGHT:
		case BAR_RIGHTTOLEFT:
			return bmpBar->w;
		case BAR_TOPTOBOTTOM:
		case BAR_BOTTOMTOTOP:
			return (bmpBar->w-NumStates)/(NumStates + 1);
		default:
			return bmpBar->w;
		}
	} else {
		return 100;
	}
}

//////////////////
// Get width of this bar
int CBar::GetHeight()
{
	if (bmpBar) {
		switch (Direction)  {
		case BAR_LEFTTORIGHT:
		case BAR_RIGHTTOLEFT:
			return (bmpBar->h-NumStates)/(NumStates + 1);
		case BAR_TOPTOBOTTOM:
		case BAR_BOTTOMTOTOP:
			return bmpBar->h;
		default:
			return bmpBar->h;
		}
	} else {
		return 10;
	}
}
