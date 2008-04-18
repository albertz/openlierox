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
#include "CButton.h"


///////////////////
// Draw the button
void CButton::Draw(const SmartPointer<SDL_Surface> & bmpDest)
{
	// Don't draw empty image
	if (!bmpImage)
		return;

	if (iButtonType == BUT_MENU)  {
		if (bRedrawMenu)
			Menu_redrawBufferRect(iX,iY, iGoodWidth, 18);

		int y2 = 5+iImageID*40;
		if(bMouseOver)
			y2+=20;

		DrawImageAdv(bmpDest,bmpImage, 5,y2, iX,iY, iGoodWidth, 18);
		bMouseOver = false;	
	} else { // Two state and three state buttons
		if (bRedrawMenu)
			Menu_redrawBufferRect(iX,iY, iGoodWidth, bmpImage->h);

		int numstates = iButtonType == BUT_TWOSTATES ? 2: 3;

		int x2 = 0;
		if (bMouseOver)
			x2 = (numstates - 2) * bmpImage->w / numstates;
		if (bMouseDown)
			x2 = (numstates - 1) * bmpImage->w / numstates;

		DrawImageAdv(bmpDest, bmpImage, x2, 0, iX, iY, bmpImage->w / numstates, bmpImage->h);

		bMouseOver = false;
	}
}


///////////////////
// Draw the button, without the buffer update
void CButton::Draw2(const SmartPointer<SDL_Surface> & bmpDest)
{  
	// Temporarily disable buffer update and draw
	bool old_redraw = bRedrawMenu;
	bRedrawMenu = false;
	Draw(bmpDest);
	bRedrawMenu = old_redraw;
}


///////////////////
// Create
// TODO: what is this good for? the WidgetCreator for example never calls this
void CButton::Create(void)
{
    int y2 = 5+iImageID*40;
    Uint8 r,g,b,a;

    // Find the smallest width
	LOCK_OR_QUIT(bmpImage);
    iGoodWidth = 0;
    for( int y=y2; y<=y2+38 && y<bmpImage->h; y++) {
        for( int x=0; x<bmpImage->w; x++ ) {

            Uint32 pixel = GetPixel(bmpImage, x,y);
            GetColour4(pixel, bmpImage->format, &r,&g,&b,&a);
            if( a != 0 )
                iGoodWidth = MAX(iGoodWidth,x);
        }
    }
	UnlockSurface(bmpImage);

    iGoodWidth = MIN(iGoodWidth+1,bmpImage->w);
    
    initWidthHeight();
}

static bool CButton_WidgetRegistered = 
	CGuiSkin::RegisterWidget( "button", & CButton::WidgetCreator )
							( "textid", CScriptableVars::SVT_INT )
							( "click", CScriptableVars::SVT_STRING );

static bool CImageButton_WidgetRegistered = 
	CGuiSkin::RegisterWidget( "imagebutton", & CButton::WidgetCreator_Image )
							( "file", CScriptableVars::SVT_STRING )
							( "click", CScriptableVars::SVT_STRING );
