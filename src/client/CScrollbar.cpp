/////////////////////////////////////////
//
//             Liero Xtreme
//
//     Copyright Auxiliary Software 2002
//
//
/////////////////////////////////////////


// gui scrollbar class
// Created 30/6/02
// Jason Boettcher


#include "defs.h"
#include "LieroX.h"
#include "Menu.h"


///////////////////
// Create the scrollbar
void CScrollbar::Create(void)
{
	iMin=0;
	iMax=0;
	iValue=0;
	iItemsperbox = 1;
	iScrollPos = 0;
	iSliderGrabbed = false;
	iSliderGrabPos = 0;
	iTopButton=false;
	iBotButton=false;

	nButtonsDown = 0;
}


///////////////////
// Draw the scrollbar
void CScrollbar::Draw(SDL_Surface *bmpDest)
{
	int x=0;
	int length;
    
	// Top arrow
	if(iTopButton)
		x=15;
	DrawImageAdv(bmpDest, gfxGUI.bmpScrollbar, x,0, iX,iY, 15,14);

	// Bottom arrow
	x=0;
	if(iBotButton)
		x=15;
	DrawImageAdv(bmpDest, gfxGUI.bmpScrollbar, x,14, iX,iY+iHeight-14, 15,14);

	// Main bit
	DrawRectFill(bmpDest, iX,iY+14, iX+iWidth, iY+iHeight-14, MakeColour(0,66,100));
	DrawVLine(bmpDest, iY+14, iY+iHeight-14, iX,MakeColour(0,75,113));
	DrawVLine(bmpDest, iY+14, iY+iHeight-14, iX+iWidth,MakeColour(0,75,113));

	// Slider
	if(iMax >= iItemsperbox && iMax > 0) {
		length = (int)((float)iItemsperbox/(float)iMax * iHeight-30);
		/*if(iMax == iItemsperbox)
			length -= 3;*/
        length = MAX(length, 12);
        
		int pos = iScrollPos;
		if(pos+length > iHeight-30)
			pos=iHeight-30-length;

        Uint32 shine = MakeColour(25,155,255);
        Uint32 middle = MakeColour(0,100,180);
        Uint32 dark = MakeColour(0,40,65);
		
		DrawRectFill(bmpDest, iX+2, iY+15+pos, iX+iWidth-1, iY+15+pos+length, middle);

        // Shine
        DrawVLine(bmpDest, iY+15+pos, iY+15+pos+length, iX+1, shine);
        DrawHLine(bmpDest, iX+1, iX+iWidth-1, iY+15+pos, shine);
        // Dark
        DrawVLine(bmpDest, iY+15+pos, iY+15+pos+length, iX+iWidth-1, dark);
        DrawHLine(bmpDest, iX+1, iX+iWidth-1, iY+15+pos+length, dark);
	}

	// Slight hack
	if( !GetMouse()->Button ) {
		iTopButton=false;
		iBotButton=false;
	}
		
}


///////////////////
// Mouse up
int CScrollbar::MouseUp(mouse_t *tMouse, int nDown)
{
	iSliderGrabbed = false;
	iTopButton=false;
	iBotButton=false;
	nButtonsDown = 0;
	return SCR_NONE;
}


///////////////////
// Mouse over
int CScrollbar::MouseOver(mouse_t *tMouse)
{
	if(!tMouse->Down) {
		iSliderGrabbed = false;
		iTopButton=false;
		iBotButton=false;
		nButtonsDown = 0;
	}
	
	return SCR_NONE;
}


///////////////////
// Mouse down
int CScrollbar::MouseDown(mouse_t *tMouse, int nDown)
{
	if(tMouse->X < iX || tMouse->X > iX+iWidth)
		return SCR_NONE;

	int length = (int)((float)iItemsperbox/(float)iMax * iHeight-30);
	length = MAX(length, 12);
    // Stabalise
    //int length = (int)((float)iItemsperbox/(float)iMax * iHeight-30-length);
	//length = MAX(length, 12);
	
	// Grab the slider
	if(tMouse->Y > iY+15+iScrollPos && tMouse->Y < iY+15+iScrollPos+length) {		
		if(!iSliderGrabbed) {
			iSliderGrabbed = true;
			iSliderGrabPos = tMouse->Y - (iY+15+iScrollPos);
			nButtonsDown = 0;
			return SCR_NONE;
		}
	}

	// Move the slider
	if(iSliderGrabbed) {
		float dist = (float)tMouse->Y - (iY+15+iScrollPos + iSliderGrabPos);
		float increment = (float)iHeight/(float)iMax;
		if(increment)
			iValue += (int)(dist / increment);
		UpdatePos();
		nButtonsDown = 0;
		return SCR_CHANGE;
	}

	
	// Do a key repeat
	bool Down = false;

	for(int i=0; i<3; i++) {
		if(tMouse->Down & SDL_BUTTON(i)) {

			if( nButtonsDown & SDL_BUTTON(i) ) {
				if( fMouseNext[i] < tLX->fCurTime ) {
					Down = true;
					fMouseNext[i] = tLX->fCurTime + 0.05f;
				}
			} else {
				Down = true;
				nButtonsDown |= SDL_BUTTON(i);
				fMouseNext[i] = tLX->fCurTime + 0.25f;
			}
		}
	}


	if( !Down )
		return SCR_NONE;

	// Top arrow
	if(tMouse->Y > iY && tMouse->Y < iY+14) {
		iTopButton = true;

		// Move up
		iValue--;
		UpdatePos();
		return SCR_CHANGE;
	}

	// Bottom arrow
	if(tMouse->Y > iY+iHeight-14 && tMouse->Y < iY+iHeight) {
		iBotButton = true;

		// Move down
		iValue++;
		UpdatePos();
		return SCR_CHANGE;
	}


	// Background click

	length = (int)((float)iItemsperbox/(float)iMax * iHeight-30);
    length = MAX(length, 12);
        
	// Background click
	int pos = iScrollPos;
	if(pos+length > iHeight-30)
		pos=iHeight-30-length;

	if (tMouse->Y > iY+15+pos+length)
	  iValue+=2;
	else
	  iValue-=2;

	UpdatePos();


	return SCR_CHANGE;


	return SCR_NONE;
}

///////////////////
// Mouse wheel down
int CScrollbar::MouseWheelDown(mouse_t *tMouse)
{
	iValue++;
	UpdatePos();
	
	return SCR_CHANGE;
}

///////////////////
// Mouse wheel up
int CScrollbar::MouseWheelUp(mouse_t *tMouse)
{
	iValue--;
	UpdatePos();
	
	return SCR_CHANGE;
}


///////////////////
// Update the slider pos
void CScrollbar::UpdatePos(void)
{
    iMax = MAX(iMax,0);
    iMin = MAX(iMin,0);

    if(iMax < iItemsperbox) {
        iValue = 0;
        iScrollPos = 0;
        return;
    }

    int mx = iMax-iItemsperbox+1;

	if(iValue > mx)
		iValue = mx;
	if(iValue < 0)
		iValue = 0;

    // Prevent div by zero errors
    if(mx == 0)
        return;

    int length = (int)((float)iItemsperbox/(float)iMax * iHeight-30);
    length = MAX(length, 12);

	iScrollPos = (int)((float)(iHeight-30-length)/(float)mx * iValue);
}


///////////////////
// Process a sent message
int CScrollbar::SendMessage(int iMsg, DWORD Param1, DWORD Param2)
{
    switch( iMsg ) {

        // Get the value
        case SCM_GETVALUE:
            return iValue;
            break;

        // Set the value
        case SCM_SETVALUE:
            iValue = Param1;
            break;

        // Set the number of items per box
        case SCM_SETITEMSPERBOX:
            iItemsperbox = Param1;
            break;

        // Set the min
        case SCM_SETMIN:
            iMin = Param1;
            break;

        // Set the max
        case SCM_SETMAX:
            iMax = Param1;
            break;
    }

    UpdatePos();

    return 0;
}