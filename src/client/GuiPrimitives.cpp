/////////////////////////////////////////
//
//   OpenLieroX
//
//   Auxiliary Software class library
//
//   based on the work of JasonB
//   enhanced by Dark Charlie and Albert Zeyer
//
//   code under LGPL
//
/////////////////////////////////////////


// Primitives for Graphics User Interface
// Created 12/08/08
// By Karel Petranek

#include "GuiPrimitives.h"
#include "GfxPrimitives.h"
#include "MathLib.h"


////////////////////////
// Draws a simple 3D button
void DrawSimpleButton(SDL_Surface *bmpDest, int x, int y, int w, int h, Color face, Color light, Color dark, bool down)
{
	DrawRectFill(bmpDest, x + 1, y + 1, x + w - 1, y + h - 1, face);
	DrawRect(bmpDest, x, y, x + w - 1, y + h - 1, dark);
	if (down)  {
		DrawVLine(bmpDest, y + 1, y + h - 2, x + w - 2, light);
		DrawHLine(bmpDest, x + 1, x + w - 2, y + h - 2, light);
	} else {
		DrawVLine(bmpDest, y + 1, y + h - 2, x + 1, light);
		DrawHLine(bmpDest, x + 1, x + w - 2, y + 1, light);
	}
}

//////////////////////
// Draws a simple filled arrow
void DrawSimpleArrow(SDL_Surface *bmpDest, int x, int y, int w, int h, ArrowDirection dir, Color col)
{

	// Draw the arrow according to the direction
	switch (dir)  {
	case ardDown:  {
		float step = (float)w / (float)(h * 2);
		for (int ly = 0; ly < h; ++ly)
			DrawHLine(bmpDest, x + Round(ly * step), x + w - Round(ly * step), y + ly, col);
	} break;

	case ardUp:  {
		float step = (float)w / (float)(h * 2);
		for (int ly = 0; ly < h; ++ly)
			DrawHLine(bmpDest, x + Round(ly * step), x + w - Round(ly * step), y + h - ly - 1, col);
	} break;

	case ardRight:  {
		float step = (float)h / (float)(w * 2);
		for (int lx = 0; lx < w; ++lx)
			DrawVLine(bmpDest, y + Round(lx * step), y + h - Round(lx * step), x + lx, col);
	} break;

	case ardLeft:  {
		float step = (float)h / (float)(w * 2);
		for (int lx = 0; lx < w; ++lx)
			DrawVLine(bmpDest, y + Round(lx * step), y + h - Round(lx * step), x + w - lx - 1, col);
	} break;
	}
}

/////////////////////////
// Draws a check (for example for checkbox)
void DrawCheck(SDL_Surface *bmpDest, int x, int y, int w, int h, Color col)
{
	int starty = y + h / 2;
	int startx = x;
	int endy = y + h - 1;
	int endx = x + w / 2 - 1;

	DrawLine(bmpDest, startx, starty, endx, endy, col);
	DrawLine(bmpDest, startx, starty + 1, endx - 1, endy, col);

	starty = y + h - 1;
	startx = endx;
	endy = y;
	endx = x + w - 1;

	DrawLine(bmpDest, startx, starty, endx, endy, col);
	DrawLine(bmpDest, startx - 1, starty, endx - 1, endy, col);
}

