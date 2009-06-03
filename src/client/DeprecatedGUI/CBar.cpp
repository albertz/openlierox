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
#include "DeprecatedGUI/CBar.h"
#include "StringUtils.h"
#include "MathLib.h"


namespace DeprecatedGUI {

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
	bgColor = Color(128, 128, 128);
	foreColor = Color(0, 255, 0);
}

//////////////
// Draw the bar
void CBar::Draw(SDL_Surface * dst)  {
	std::string progress;
	progress = itoa(Position) + " %";
	int pos = CLAMP(Position, 0, 100);

	// In the bar image, there's the fully loaded image - it's always the first image starting at 0,0.
	// Then there can be unlimited number of states for various purposes
	// The last image is the background - fully unloaded image

	if (bmpBar.get())  {  // We got a bitmap - many options and possibilities for drawing
		int numstates = NumForeStates + NumBgStates;

		int bar_h = (bmpBar.get()->h - NumForeStates) / (numstates);
		int bar_w = (bmpBar.get()->w - NumForeStates) / (numstates);

		Uint32 clLabel = 0; // same format as bmpBar
		
		// Draw the progress bitmap over the background depending on progress direction
		int w, h;
		switch (Direction)  {
		case BAR_LEFTTORIGHT:
			// Background
			DrawImageAdv(dst, bmpBar, 0, bar_h * (NumForeStates + CurrentBgState), X, Y, bmpBar.get()->w, bar_h);

			DrawImageAdv(dst, bmpBar, 0,  bar_h * CurrentForeState, X, Y, (bmpBar.get()->w * pos) / 100, bar_h);  // Progress
			LOCK_OR_QUIT(bmpBar);
			clLabel = GetPixel(bmpBar.get(), MAX(0, (bmpBar.get()->w*pos)/100 - 1), bmpBar.get()->h - NumForeStates + CurrentForeState);
			UnlockSurface(bmpBar);
			break;
		case BAR_RIGHTTOLEFT:
			// Background
			DrawImageAdv(dst, bmpBar, 0, bar_h * (NumForeStates + CurrentBgState), X, Y, bmpBar.get()->w, bar_h);

			w = (bmpBar.get()->w * pos) / 100;
			DrawImageAdv(dst, bmpBar, bmpBar.get()->w - w,  bar_h * CurrentForeState, X + bmpBar.get()->w - w, Y, w, bar_h);  // Progress
			LOCK_OR_QUIT(bmpBar);
			clLabel = GetPixel(bmpBar.get(), MIN(bmpBar.get()->w - 1, bmpBar.get()->w - w + 1), bmpBar.get()->h - NumForeStates + CurrentForeState);
			UnlockSurface(bmpBar);
			break;
		case BAR_TOPTOBOTTOM: 
			// Background
			DrawImageAdv(dst, bmpBar, bar_w * (NumForeStates + CurrentBgState), 0, X, Y, bar_w, bmpBar.get()->h); // The last image is the empty one

			DrawImageAdv(dst, bmpBar, bar_w * CurrentForeState, 0, X, Y, bar_w, (bmpBar.get()->h / 100) * pos); // Progress
			LOCK_OR_QUIT(bmpBar);
			clLabel = GetPixel(bmpBar.get(), bmpBar.get()->w - NumForeStates + CurrentForeState, MAX(0, (bmpBar.get()->h * pos)/100 - 1));
			UnlockSurface(bmpBar);
			break;
		case BAR_BOTTOMTOTOP:
			// Background
			DrawImageAdv(dst, bmpBar, bar_w * (NumForeStates + CurrentBgState), 0, X, Y, bar_w, bmpBar.get()->h); // The last image is the empty one

			h = (bmpBar.get()->h * pos) / 100;
			DrawImageAdv(dst, bmpBar,  bar_w * CurrentForeState, bmpBar.get()->h - h, X, Y + bmpBar.get()->h - h, bar_w, h);  // Progress
			LOCK_OR_QUIT(bmpBar);
			clLabel = GetPixel(bmpBar.get(), bmpBar.get()->w - NumForeStates + CurrentForeState, MIN(bmpBar.get()->h- 1, bmpBar.get()->h - h - 1));
			UnlockSurface(bmpBar);
			break;
		default:
			warnings("Bad bar type in CBar::Draw\n");
			return;
		}

		if (LabelVisible)
			tLX->cFont.Draw(dst, LabelX, LabelY, Color(bmpBar->format, clLabel), progress); // Label

		
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
	if (bmpBar.get()) {
		int numstates = NumForeStates + NumBgStates;
		switch (Direction)  {
		case BAR_LEFTTORIGHT:
		case BAR_RIGHTTOLEFT:
			return bmpBar.get()->w;
		case BAR_TOPTOBOTTOM:
		case BAR_BOTTOMTOTOP:
			return (bmpBar.get()->w-numstates)/(numstates + 1);
		default:
			return bmpBar.get()->w;
		}
	} else {
		return 100;
	}
}

//////////////////
// Get width of this bar
int CBar::GetHeight()
{
	if (bmpBar.get()) {
		int numstates = NumForeStates + NumBgStates;
		switch (Direction)  {
		case BAR_LEFTTORIGHT:
		case BAR_RIGHTTOLEFT:
			return (bmpBar.get()->h-NumForeStates)/(numstates);
		case BAR_TOPTOBOTTOM:
		case BAR_BOTTOMTOTOP:
			return bmpBar.get()->h;
		default:
			return bmpBar.get()->h;
		}
	} else {
		return 10;
	}
}

}; // namespace DeprecatedGUI
