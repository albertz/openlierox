/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Button
// Created 30/3/03
// Jason Boettcher


#include "LieroX.h"
#include "Menu.h"
#include "GfxPrimitives.h"


///////////////////
// Draw the button
void CButton::Draw(SDL_Surface *bmpDest)
{
	// If we shouldn't redraw the menu, call the other Draw
	if (!bRedrawMenu)  {
		Draw2(bmpDest);
		return;
	}

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
    for( int y=y2; y<=y2+38 && y<bmpImage->h; y++) {
        for( int x=0; x<bmpImage->w; x++ ) {

            Uint32 pixel = GetPixel(bmpImage, x,y);
            GetColour4(pixel, bmpImage, &r,&g,&b,&a);
            if( a != 0 )
                iGoodWidth = MAX(iGoodWidth,x);
        }
    }

    iGoodWidth = MIN(iGoodWidth+1,bmpImage->w);
}
