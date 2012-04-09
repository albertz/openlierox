/////////////////////////////////////////
//
//   OpenLieroX
//
//   Primitives for Graphics User Interface
//
//   based on the work of JasonB
//   enhanced by Karel Petranek and Albert Zeyer
//
//   Created 12/08/08
//   By Karel Petranek
//
//   code under LGPL
//
/////////////////////////////////////////



#ifndef __GUIPRIMITIVES_H__
#define __GUIPRIMITIVES_H__

struct SDL_Surface;
#include "Color.h"

enum ArrowDirection  {
	ardUp,
	ardDown,
	ardLeft,
	ardRight
};

void DrawSimpleButton(SDL_Surface *bmpDest, int x, int y, int w, int h, Color face, Color light, Color dark, bool down);
void DrawSimpleArrow(SDL_Surface *bmpDest, int x, int y, int w, int h, ArrowDirection dir, Color col);
void DrawCheck(SDL_Surface *bmpDest, int x, int y, int w, int h, Color col);


#endif  // __GUIPRIMITIVES_H__
