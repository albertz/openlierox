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


#ifndef __GUIPRIMITIVES_H__
#define __GUIPRIMITIVES_H__

#include <SDL.h>
#include "Color.h"

enum ArrowDirection  {
	ardUp,
	ardDown,
	ardLeft,
	ardRight
};

void DrawSimpleButton(SDL_Surface *bmpDest, int x, int y, int w, int h, Color face, Color light, Color dark, bool down);
void DrawArrow(SDL_Surface *bmpDest, int x, int y, int w, int h, ArrowDirection dir, Color col);


#endif  // __GUIPRIMITIVES_H__
