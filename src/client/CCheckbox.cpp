/////////////////////////////////////////
//
//             Liero Xtreme
//
//     Copyright Auxiliary Software 2002
//
//
/////////////////////////////////////////


// Checkbox
// Created 30/3/03
// Jason Boettcher


#include "defs.h"
#include "LieroX.h"
#include "Menu.h"


///////////////////
// Draw the checkbox
void CCheckbox::Draw(SDL_Surface *bmpDest)
{
    Menu_redrawBufferRect( iX,iY, 17,17 );

    if(iValue)
		DrawImageAdv(bmpDest, bmpImage, 17,0,iX,iY,17,17);
	else
	    DrawImageAdv(bmpDest, bmpImage, 0,0,iX,iY,17,17);
}


///////////////////
// Create
void CCheckbox::Create(void)
{
    bmpImage = LoadImage("data/frontend/checkbox.png",16);
}
