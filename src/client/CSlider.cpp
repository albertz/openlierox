/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Slider
// Created 30/6/02
// Jason Boettcher


#include "LieroX.h"
#include "Graphics.h"
#include "Menu.h"
#include "GfxPrimitives.h"
#include "CSlider.h"


///////////////////
// Draw the slider
void CSlider::Draw(SDL_Surface *bmpDest)
{
	if (bRedrawMenu)
		Menu_redrawBufferRect(iX,iY, iWidth,iHeight);

	DrawHLine( bmpDest, iX+5, iX+iWidth-5, iY+iHeight/2-1, tLX->clSliderDark);
	DrawHLine( bmpDest, iX+5, iX+iWidth-5, iY+iHeight/2,   tLX->clSliderLight);
	DrawHLine( bmpDest, iX+5, iX+iWidth-5, iY+iHeight/2+1, tLX->clSliderDark);

	// Draw the button
	int x = iX+5;
	int w = iWidth - 10;
	int val = (int)( ((float)w/(float)iMax) * (float)iValue ) + x;

	int y = (iY+iHeight/2) - gfxGUI.bmpSliderBut->h/2;
	DrawImage(bmpDest,gfxGUI.bmpSliderBut,val-3,y);
}


///////////////////
// Mouse down on the slider
int CSlider::MouseDown(mouse_t *tMouse, int nDown)
{
	int x = iX+5;
	int w = iWidth - 10;

	int val = (int)( (float)iMax / ( (float)w / (float)(tMouse->X-x)) );
	iValue = val;

	if(tMouse->X > x+w)
		iValue = iMax;
	if(tMouse->X < x)
		iValue = 0;

	// Clamp the value
	iValue = MAX(0,iValue);
	iValue = MIN(iMax,iValue);

	return SLD_CHANGE;
}


///////////////////
// This widget is send a message
DWORD CSlider::SendMessage(int iMsg, DWORD Param1, DWORD Param2)
{
	switch(iMsg) {

		// Get the value
		case SLM_GETVALUE:
			return iValue;

		// Set the value
		case SLM_SETVALUE:
			iValue = Param1;
			break;

	}

	return 0;
}
