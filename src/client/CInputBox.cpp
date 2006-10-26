/////////////////////////////////////////
//
//             Liero Xtreme
//
//     Copyright Auxiliary Software 2002
//
//
/////////////////////////////////////////


// Input box
// Created 30/3/03
// Jason Boettcher


#include "defs.h"
#include "LieroX.h"
#include "Menu.h"


///////////////////
// Draw the input box
void CInputbox::Draw(SDL_Surface *bmpDest)
{
    Menu_redrawBufferRect(iX,iY, bmpImage->w,17);

	int y=0;
	if(iMouseOver)
		y=17;
	DrawImageAdv(bmpDest,bmpImage, 0,y, iX,iY, bmpImage->w,17);
	iMouseOver = false;
    tLX->cFont.DrawCentre(bmpDest, iX+25, iY+1, 0xffff,"%s", sText);
}