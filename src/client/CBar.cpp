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

//////////////
// Constructor
CBar::CBar(SDL_Surface *bmp, int x, int y, int label_x, int label_y, int dir)  {
	bmpBar = bmp;
	X = x;
	Y = y;
	LabelX = label_x;
	LabelY = label_y;
	Direction = dir;
	Position = 100;
	LabelVisible = true;

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

	if (bmpBar)  {  // We got a bitmap - many options and possibilities for drawing

		int bar_h = (bmpBar->h - 1)/2;
		int bar_w = (bmpBar->w - 1)/2;

		// Background
		DrawImageAdv(dst,bmpBar, 0, bar_h, X, Y, bmpBar->w, bar_h);

		Uint32 clLabel = tLX->clWhite;
		
		// Draw the progress bitmap over the background depending on progress direction
		switch (Direction)  {
		case BAR_LEFTTORIGHT:
			DrawImageAdv(dst,bmpBar, 0, 0, X, Y, (bmpBar->w*pos)/100, bar_h);  // Progress
			clLabel = GetPixel(bmpBar, MAX(0, (bmpBar->w*pos)/100 - 1), bmpBar->h - 1);
			break;
		case BAR_RIGHTTOLEFT: {
			int w = (bmpBar->w * pos) / 100;
			DrawImageAdv(dst, bmpBar, bmpBar->w - w, 0, X + bmpBar->w - w, Y, w, bar_h);  // Progress
			clLabel = GetPixel(bmpBar, MIN(bmpBar->w - 1, bmpBar->w - w + 1), bmpBar->h - 1);
			}
			break;
		case BAR_TOPTOBOTTOM: 
			DrawImageAdv(dst, bmpBar, 0, 0, X, Y, bar_w, (bmpBar->h/100)*pos); // Progress
			clLabel = GetPixel(bmpBar, bmpBar->w - 1, MAX(0, (bmpBar->h*pos)/100 - 1));
			break;
		case BAR_BOTTOMTOTOP:  {
			int h = (bmpBar->w * pos) / 100;
			DrawImageAdv(dst, bmpBar, 0, bmpBar->h - h, X, Y + bmpBar->h - h, bar_w, h);  // Progress
			clLabel = GetPixel(bmpBar, bmpBar->w - 1, MIN(bmpBar->h - 1, bmpBar->h - h - 1));
			}
			break;
		default:
			printf("Bad bar type in CBar::Draw");
			return;
		}

		// bmpBar is an alpha surface so we need to convert the color from GetPixel
		static Uint8 r, g, b;
		GetColour3(clLabel, bmpBar, &r, &g, &b);
		clLabel = MakeColour(r, g, b);

		if (LabelVisible)
			tLX->cFont.Draw(dst, LabelX, LabelY, clLabel, progress); // Label

		
	} else {  // No bitmap, just draw the simplest bar without any options
		int width = 100;
		int height = 10;

		DrawRectFill(dst, X, Y, (X + width + 1), Y+height, bgColor);
		DrawRectFill(dst, X+1, Y+1, X + (width / 100) * pos + 1, Y+height-1, foreColor);
	}


}
