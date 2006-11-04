/////////////////////////////////////////
//
//             Liero Xtreme
//
//     Copyright Auxiliary Software 2002
//
//
/////////////////////////////////////////


// Button
// Created 30/3/03
// Jason Boettcher


#include "defs.h"
#include "LieroX.h"
#include "Menu.h"


///////////////////
// Draw the button
void CButton::Draw(SDL_Surface *bmpDest)
{
	// Don't draw empty image
	if (!bmpImage)
		return;

	int y2 = 5+iImageID*40;
    if(iMouseOver)
	    y2+=20;

    Menu_redrawBufferRect(iX,iY, iGoodWidth,18);

	DrawImageAdv(bmpDest,bmpImage, 5,y2, iX,iY, iGoodWidth, 18);
    iMouseOver = false;	
}


///////////////////
// Draw the button, without the buffer update
void CButton::Draw2(SDL_Surface *bmpDest)
{  
	// Don't draw empty image
	if (!bmpImage)
		return;

	int y2 = 5+iImageID*40;
    if(iMouseOver)
	    y2+=20;

	DrawImageAdv(bmpDest,bmpImage, 5,y2, iX,iY, iGoodWidth, 18);
    iMouseOver = false;	
}


///////////////////
// Create
void CButton::Create(void)
{
    int y2 = 5+iImageID*40;
    Uint8 r,g,b,a;

    // Find the smallest width
    iGoodWidth = 0;
    for( int y=y2; y<=y2+38; y++) {
        for( int x=0; x<bmpImage->w; x++ ) {

            Uint32 pixel = GetPixel(bmpImage, x,y);
            GetColour4(pixel, bmpImage, &r,&g,&b,&a);
            if( a != 0 )
                iGoodWidth = MAX(iGoodWidth,x);
        }
    }

    iGoodWidth = MIN(iGoodWidth+1,bmpImage->w);
}
