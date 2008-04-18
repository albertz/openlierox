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

#include "LieroX.h"
#include "GfxPrimitives.h"
#include "CBar.h"
#include "StringUtils.h"
#include "MathLib.h"


//////////////
// Constructor
CBar::CBar(SmartPointer<SDL_Surface> bmp, int x, int y, int label_x, int label_y, int dir, int num_fore_states, int num_bg_states)  {
	bmpBar = bmp;
	X = x;
	Y = y;
	LabelX = label_x;
	LabelY = label_y;
	Direction = dir;
	Position = 100;
	LabelVisible = true;
	NumForeStates = num_fore_states;
	NumBgStates = num_bg_states;
	CurrentForeState = 0;
	CurrentBgState = 0;

	// Default colors
	bgColor = MakeColour(128, 128, 128);
	foreColor = MakeColour(0, 255, 0);
}

//////////////
// Draw the bar
void CBar::Draw(SDL_Surface * dst)  {
	static std::string progress;
	progress = itoa(Position) + " %";
	int pos = CLAMP(Position, 0, 100);

	// In the bar image, there's the fully loaded image - it's always the first image starting at 0,0.
	// Then there can be unlimited number of states for various purposes
	// The last image is the background - fully unloaded image

	if (bmpBar)  {  // We got a bitmap - many options and possibilities for drawing
		int numstates = NumForeStates + NumBgStates;

		int bar_h = (bmpBar->h - NumForeStates) / (numstates);
		int bar_w = (bmpBar->w - NumForeStates) / (numstates);

		Uint32 clLabel = tLX->clWhite;
		
		// Draw the progress bitmap over the background depending on progress direction
		int w, h;
		switch (Direction)  {
		case BAR_LEFTTORIGHT:
			// Background
			DrawImageAdv(dst, bmpBar, 0, bar_h * (NumForeStates + CurrentBgState), X, Y, bmpBar->w, bar_h);

			DrawImageAdv(dst, bmpBar, 0,  bar_h * CurrentForeState, X, Y, (bmpBar->w * pos) / 100, bar_h);  // Progress
			LOCK_OR_QUIT(bmpBar);
			clLabel = GetPixel(bmpBar, MAX(0, (bmpBar->w*pos)/100 - 1), bmpBar->h - NumForeStates + CurrentForeState);
			UnlockSurface(bmpBar);
			break;
		case BAR_RIGHTTOLEFT:
			// Background
			DrawImageAdv(dst, bmpBar, 0, bar_h * (NumForeStates + CurrentBgState), X, Y, bmpBar->w, bar_h);

			w = (bmpBar->w * pos) / 100;
			DrawImageAdv(dst, bmpBar, bmpBar->w - w,  bar_h * CurrentForeState, X + bmpBar->w - w, Y, w, bar_h);  // Progress
			LOCK_OR_QUIT(bmpBar);
			clLabel = GetPixel(bmpBar, MIN(bmpBar->w - 1, bmpBar->w - w + 1), bmpBar->h - NumForeStates + CurrentForeState);
			UnlockSurface(bmpBar);
			break;
		case BAR_TOPTOBOTTOM: 
			// Background
			DrawImageAdv(dst, bmpBar, bar_w * (NumForeStates + CurrentBgState), 0, X, Y, bar_w, bmpBar->h); // The last image is the empty one

			DrawImageAdv(dst, bmpBar, bar_w * CurrentForeState, 0, X, Y, bar_w, (bmpBar->h / 100) * pos); // Progress
			LOCK_OR_QUIT(bmpBar);
			clLabel = GetPixel(bmpBar, bmpBar->w - NumForeStates + CurrentForeState, MAX(0, (bmpBar->h * pos)/100 - 1));
			UnlockSurface(bmpBar);
			break;
		case BAR_BOTTOMTOTOP:
			// Background
			DrawImageAdv(dst, bmpBar, bar_w * (NumForeStates + CurrentBgState), 0, X, Y, bar_w, bmpBar->h); // The last image is the empty one

			h = (bmpBar->w * pos) / 100;
			DrawImageAdv(dst, bmpBar,  bar_w * CurrentForeState, bmpBar->h - h, X, Y + bmpBar->h - h, bar_w, h);  // Progress
			LOCK_OR_QUIT(bmpBar);
			clLabel = GetPixel(bmpBar, bmpBar->w - NumForeStates + CurrentForeState, MIN(bmpBar->h- 1, bmpBar->h - h - 1));
			UnlockSurface(bmpBar);
			break;
		default:
			printf("Bad bar type in CBar::Draw");
			return;
		}

		// bmpBar is an alpha surface so we need to convert the color from GetPixel
		Uint8 r, g, b;
		GetColour3(clLabel, bmpBar->format, &r, &g, &b);
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
		int numstates = NumForeStates + NumBgStates;
		switch (Direction)  {
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
		return 100;
	}
}

//////////////////
// Get width of this bar
int CBar::GetHeight()
{
	if (bmpBar) {
		int numstates = NumForeStates + NumBgStates;
		switch (Direction)  {
		case BAR_LEFTTORIGHT:
		case BAR_RIGHTTOLEFT:
			return (bmpBar->h-NumForeStates)/(numstates);
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
