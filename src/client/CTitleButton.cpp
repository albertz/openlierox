/////////////////////////////////////////
//
//             Liero Xtreme
//
//     Copyright Auxiliary Software 2002
//
//
/////////////////////////////////////////


// Title button
// Created 30/3/03
// Jason Boettcher


#include "defs.h"
#include "LieroX.h"
#include "menu.h"


///////////////////
// Draw the title button
void CTitleButton::Draw(SDL_Surface *bmpDest)
{
    Menu_redrawBufferRect(iX,iY, bmpImage->w,39);

	int y = 10+iImageID*40;
	if(iMouseOver)
		y += 200;
	DrawImageAdv(bmpDest,bmpImage, 10,y, iX,iY, bmpImage->w, 39);
	iMouseOver = false;
}
