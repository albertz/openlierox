/////////////////////////////////////////
//
//             Liero Xtreme
//
//     Copyright Auxiliary Software 2002
//
//
/////////////////////////////////////////


// Slider
// Created 30/6/02
// Jason Boettcher


#include "defs.h"
#include "LieroX.h"
#include "Menu.h"


///////////////////
// Draw the slider
void CSlider::Draw(SDL_Surface *bmpDest)
{
    Menu_redrawBufferRect(iX,iY, iWidth,iHeight);

	DrawHLine( bmpDest, iX+5, iX+iWidth-5, iY+iHeight/2-1, MakeColour(90,90,90));
	DrawHLine( bmpDest, iX+5, iX+iWidth-5, iY+iHeight/2,   MakeColour(115,115,115));
	DrawHLine( bmpDest, iX+5, iX+iWidth-5, iY+iHeight/2+1, MakeColour(90,90,90));

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
