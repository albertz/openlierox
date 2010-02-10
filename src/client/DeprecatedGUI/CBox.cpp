// OpenLieroX


// Input box
// Created 30/3/03
// Jason Boettcher

// code under LGPL


#include "LieroX.h"

#include "MathLib.h"
#include "DeprecatedGUI/Menu.h"
#include "GfxPrimitives.h"
#include "StringUtils.h"
#include "DeprecatedGUI/CBox.h"


namespace DeprecatedGUI {

///////////////////
// Draw the frame into a buffer
void CBox::PreDraw()
{
	// TODO: clean up this function (eg, to much different usage of same var, too much most-upper-level vars, ...)
	
	// If the width or height of the box has changed, create new buffer
	if (bmpBuffer.get())  {
		if (iWidth != bmpBuffer.get()->w || iHeight != bmpBuffer.get()->h)  {
			bmpBuffer = NULL;
		}
	}

	// Create the buffer, if needed
	if (!bmpBuffer.get())  {
		SDL_PixelFormat *fmt = getMainPixelFormat();
		if (fmt)
			bmpBuffer = gfxCreateSurface(iWidth, iHeight);
		else
			bmpBuffer = NULL;

		if (!bmpBuffer.get())
			return;
	}

	// Set the whole buffer transparent
	SetColorKey(bmpBuffer.get());
	FillSurfaceTransparent(bmpBuffer.get());

	// Clip the border and radius
	if (iRound < 0 || iBorder < 0)
		return;

	iRound = MIN(MIN(iWidth,iHeight)/2,iRound);
	iBorder = MIN(MIN(iWidth,iHeight)/2,iBorder);

	//
	// 1. Draw the lines
	//
	int line_left,line_right,line_top,line_bottom;

	// Line positions
	line_left = MAX(iRound,iBorder);
	line_right = iWidth - MAX(iRound,iBorder);
	line_top = MAX(iRound,iBorder);
	line_bottom = iHeight - MAX(iRound,iBorder);

	// Color handling
	Color cur_col = iDarkColour;

	// Create gradient
	int rstep=0,gstep=0,bstep=0;
	if (iBorder)  {
		rstep = (iLightColour.r-iDarkColour.r)/iBorder;
		rstep = (iLightColour.g-iDarkColour.g)/iBorder;
		rstep = (iLightColour.b-iDarkColour.b)/iBorder;
	}

	// Top line
	int j;
	for (j=0; j<iBorder; j++)  {
		DrawHLine(bmpBuffer.get(),line_left,line_right,j,cur_col);
		cur_col = Color(iDarkColour.r+rstep*(j+1),iDarkColour.g+gstep*(j+1),iDarkColour.b+bstep*(j+1));
	}

	cur_col = iDarkColour;

	// Bottom line
	for (j=0; j<iBorder; j++)  {
		DrawHLine(bmpBuffer.get(),line_left,line_right,iHeight-j-1,cur_col);
		cur_col = Color(iDarkColour.r+rstep*(j+1),iDarkColour.g+gstep*(j+1),iDarkColour.b+bstep*(j+1));
	}

	cur_col = iDarkColour;

	// Left line
	for (j=0; j<iBorder; j++)  {
		DrawVLine(bmpBuffer.get(),line_top,line_bottom,j,cur_col);
		cur_col = Color(iDarkColour.r+rstep*(j+1),iDarkColour.g+gstep*(j+1),iDarkColour.b+bstep*(j+1));
	}

	cur_col = iDarkColour;

	// Right line
	for (j=0; j<iBorder; j++)  {
		DrawVLine(bmpBuffer.get(),line_top,line_bottom,iWidth-j-1,cur_col);
		cur_col = Color(iDarkColour.r+rstep*(j+1),iDarkColour.g+gstep*(j+1),iDarkColour.b+bstep*(j+1));
	}


	//
	// 2. Draw the round borders
	//

	int x,y;

	// Radius has to be bigger than or equal to border
	iRound = MAX(iBorder,iRound);

	// Step for circle drawing
	float step = (float)(1/(PI*(iRound+0.00000001)*(iBorder+0.000000001)));

	LOCK_OR_QUIT(bmpBuffer);

	// Top left
	// (PI,3/2*PI)
	float i;
	for (i=1.00f;i<1.5f;i+=step)  {
		cur_col = iDarkColour;
		for (j=0; j<iBorder; j++)  {
			x = CLAMP( (int)((iRound-j)*sin(PI*i))-1+iRound, 0, iWidth-1);
			y = CLAMP( (int)((iRound-j)*cos(PI*i))-1+iRound, 0, iHeight-1);
			
			PutPixel(bmpBuffer.get(),x,y,cur_col.get(bmpBuffer->format));
			cur_col = Color(iDarkColour.r+rstep*(j+1),iDarkColour.g+gstep*(j+1),iDarkColour.b+bstep*(j+1));
		}
	}

	// Top right
	// (PI/2,PI)
	for (i=0.50f;i<1.0f;i+=step)  {
		cur_col = iDarkColour;
		for (j=0; j<iBorder; j++)  {
			x = CLAMP( (int)((iRound-j)*sin(PI*i))+iWidth-iRound, 0, iWidth-1);
			y = CLAMP( (int)((iRound-j)*cos(PI*i))-1+iRound, 0, iHeight-1);
			PutPixel(bmpBuffer.get(),x,y,cur_col.get(bmpBuffer->format));
			cur_col = Color(iDarkColour.r+rstep*(j+1),iDarkColour.g+gstep*(j+1),iDarkColour.b+bstep*(j+1));
		}
	}

	// Bottom left
	// (3/2*PI,2PI)
	for (i=1.50f;i<2.0f;i+=step)  {
		cur_col = iDarkColour;
		for (j=0; j<iBorder; j++)  {
			x = CLAMP( (int)((iRound-j)*sin(PI*i))-1+iRound, 0, iWidth-1);
			y = CLAMP( (int)((iRound-j)*cos(PI*i))+iHeight-iRound, 0, iHeight-1);
			PutPixel(bmpBuffer.get(),x,y,cur_col.get(bmpBuffer->format));
			cur_col = Color(iDarkColour.r+rstep*(j+1),iDarkColour.g+gstep*(j+1),iDarkColour.b+bstep*(j+1));
		}
	}

	// Bottom right
	// (0,PI)
	for (i=-0.01f;i<0.5f;i+=step)  {
		cur_col = iDarkColour;
		for (j=0; j<iBorder; j++)  {
			x = CLAMP( (int)((iRound-j)*sin(PI*i))+iWidth-iRound, 0, iWidth-1);
			y = CLAMP( (int)((iRound-j)*cos(PI*i))+iHeight-iRound, 0, iHeight-1);
			PutPixel(bmpBuffer.get(),x,y,cur_col.get(bmpBuffer->format));
			cur_col = Color(iDarkColour.r+rstep*(j+1),iDarkColour.g+gstep*(j+1),iDarkColour.b+bstep*(j+1));
		}
	}


	//
	//	Draw the background
	//
	if (iBgColour == tLX->clPink)
		return;

	// Draw the center rectangle
	DrawRectFill(bmpBuffer.get(),line_left,iBorder,line_right,iHeight-iBorder,iBgColour);

	// No need to draw more
	if (iRound <= iBorder)
		return;

	DrawRectFill(bmpBuffer.get(),iBorder,line_top,line_left,line_bottom,iBgColour);
	DrawRectFill(bmpBuffer.get(),line_right,line_top,iWidth-iBorder,line_bottom,iBgColour);

	// Top left
	// (1,3/2*PI)
	for (i=1.01f;i<1.49;i+=step)  {
		for (j=iBorder; j<iRound-iBorder+2; j++)  {
			x = CLAMP( (int)((iRound-j)*sin(PI*i))-1+iRound, 0, iWidth-1);
			y = CLAMP( (int)((iRound-j)*cos(PI*i))-1+iRound, 0, iHeight-1);
			PutPixel(bmpBuffer.get(),x,y,iBgColour.get(bmpBuffer->format));
		}
	}

	// Top right
	// (PI/2,PI)
	for (i=0.51f;i<0.99;i+=step)  {
		for (j=iBorder; j<iRound-iBorder+2; j++)  {
			x = CLAMP( (int)((iRound-j)*sin(PI*i))+iWidth-iRound, 0, iWidth-1);
			y = CLAMP( (int)((iRound-j)*cos(PI*i))-1+iRound, 0, iHeight-1);
			PutPixel(bmpBuffer.get(),x,y,iBgColour.get(bmpBuffer->format));
		}
	}
	
	// Bottom left
	// (3/2*PI,2PI)
	for (i=1.51f;i<1.99;i+=step)  {
		for (j=iBorder; j<iRound-iBorder+2; j++)  {
			x = CLAMP( (int)((iRound-j)*sin(PI*i))-1+iRound, 0, iWidth-1);
			y = CLAMP( (int)((iRound-j)*cos(PI*i))+iHeight-iRound, 0, iHeight-1);
			PutPixel(bmpBuffer.get(),x,y,iBgColour.get(bmpBuffer->format));
		}
	}

	// Bottom right
	// (0,PI)
	for (i=0.01f;i<0.499;i+=step)  {
		for (j=iBorder; j<=iRound-iBorder+2; j++)  {
			x = CLAMP( (int)((iRound-j)*sin(PI*i))+iWidth-iRound, 0, iWidth-1);
			y = CLAMP( (int)((iRound-j)*cos(PI*i))+iHeight-iRound, 0, iHeight-1);
			PutPixel(bmpBuffer.get(),x,y,iBgColour.get(bmpBuffer->format));
		}
	}

	UnlockSurface(bmpBuffer);
	
}

/////////////////
// Draw the frame
void CBox::Draw(SDL_Surface * bmpDest)
{
	if (bmpBuffer.get())
		DrawImage(bmpDest,bmpBuffer,iX,iY);
}

/////////////////
// Free the widget
void CBox::Destroy()
{
	bmpBuffer = NULL;
}

//////////////////
// Checks if any mouse event happened 
int	CBox::CheckEvent()
{
	// If the mouse is over transparent area, don't fire any event
	// CGuiLayout will then continue processing events for other widgets

	if(bmpBuffer.get() == NULL) {
		errors << "CBox::CheckEvent: bmpBuffer == NULL" << endl;
		return BOX_NONE;
	}
	
	// Get the mouse pos
	mouse_t *Mouse = GetMouse();
	if (!Mouse)
		return BOX_NONE;
	int x = Mouse->X - iX;
	int y = Mouse->Y - iY;

	// Mouse in the buffer
	if (x < iWidth && y < iHeight)  {
		// Mouse over transparent pixel? No event
		if (!LockSurface(bmpBuffer))
			return BOX_NONE;
		if(Color(bmpBuffer.get()->format, GetPixel(bmpBuffer.get(),x,y)) == tLX->clPink)  {
			UnlockSurface(bmpBuffer);
			return BOX_NOEVENT;
		}
		UnlockSurface(bmpBuffer);
		return BOX_MOUSEOVER;
	} else 
		return BOX_NONE;

}

}; // namespace DeprecatedGUI
