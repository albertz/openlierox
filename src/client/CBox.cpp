// OpenLieroX


// Input box
// Created 30/3/03
// Jason Boettcher

// code under LGPL


#include "LieroX.h"
#include "MathLib.h"
#include "Menu.h"
#include "GfxPrimitives.h"
#include "StringUtils.h"
#include "CBox.h"


///////////////////
// Draw the frame into a buffer
void CBox::PreDraw(void)
{
	// If the width or height of the box has changed, create new buffer
	if (bmpBuffer)  {
		if (iWidth != bmpBuffer->w || iHeight != bmpBuffer->h)  {
			SDL_FreeSurface(bmpBuffer);
			bmpBuffer = NULL;
		}
	}

	// Create the buffer, if needed
	if (!bmpBuffer)  {
		SDL_PixelFormat *fmt = SDL_GetVideoSurface()->format;
		if (fmt)
		// TODO: why is SDL_CreateRGBSurface used directly here?
			bmpBuffer = SDL_CreateRGBSurface(iSurfaceFormat,iWidth,iHeight,fmt->BitsPerPixel,fmt->Rmask,fmt->Gmask,fmt->Bmask,fmt->Amask);
		else
			bmpBuffer = NULL;

		if (!bmpBuffer)
			return;
	}

	// Set the whole buffer transparent
	SetColorKey(bmpBuffer);
	FillSurfaceTransparent(bmpBuffer);

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
	Uint32 cur_col = iDarkColour;

	// Create gradient
	Uint8 dark_r,dark_g,dark_b;
	GetColour3(iDarkColour,bmpBuffer,&dark_r,&dark_g,&dark_b);
	Uint8 light_r,light_g,light_b;
	GetColour3(iLightColour,bmpBuffer,&light_r,&light_g,&light_b);
	int rstep=0,gstep=0,bstep=0;
	if (iBorder)  {
		rstep = (light_r-dark_r)/iBorder;
		gstep = (light_g-dark_g)/iBorder;
		bstep = (light_b-dark_b)/iBorder;
	}

	// Top line
	int j;
	for (j=0; j<iBorder; j++)  {
		DrawHLine(bmpBuffer,line_left,line_right,j,cur_col);
		cur_col = MakeColour(dark_r+rstep*(j+1),dark_g+gstep*(j+1),dark_b+bstep*(j+1));
	}

	cur_col = iDarkColour;

	// Bottom line
	for (j=0; j<iBorder; j++)  {
		DrawHLine(bmpBuffer,line_left,line_right,iHeight-j-1,cur_col);
		cur_col = MakeColour(dark_r+rstep*(j+1),dark_g+gstep*(j+1),dark_b+bstep*(j+1));
	}

	cur_col = iDarkColour;

	// Left line
	for (j=0; j<iBorder; j++)  {
		DrawVLine(bmpBuffer,line_top,line_bottom,j,cur_col);
		cur_col = MakeColour(dark_r+rstep*(j+1),dark_g+gstep*(j+1),dark_b+bstep*(j+1));
	}

	cur_col = iDarkColour;

	// Right line
	for (j=0; j<iBorder; j++)  {
		DrawVLine(bmpBuffer,line_top,line_bottom,iWidth-j-1,cur_col);
		cur_col = MakeColour(dark_r+rstep*(j+1),dark_g+gstep*(j+1),dark_b+bstep*(j+1));
	}


	//
	// 2. Draw the round borders
	//

	int x,y;

	// Radius has to be bigger than or equal to border
	iRound = MAX(iBorder,iRound);

	// Step for circle drawing
	float step = (float)(1/(PI*(iRound+0.00000001)*(iBorder+0.000000001)));

	// Top left
	// (PI,3/2*PI)
	float i;
	for (i=1.00f;i<1.5f;i+=step)  {
		cur_col = iDarkColour;
		for (j=0; j<iBorder; j++)  {
			x = (int)((iRound-j)*sin(PI*i))-1;
			y = (int)((iRound-j)*cos(PI*i))-1;
			PutPixel(bmpBuffer,x+iRound,y+iRound,cur_col);
			cur_col = MakeColour(dark_r+rstep*(j+1),dark_g+gstep*(j+1),dark_b+bstep*(j+1));
		}
	}

	// Top right
	// (PI/2,PI)
	for (i=0.50f;i<1.0f;i+=step)  {
		cur_col = iDarkColour;
		for (j=0; j<iBorder; j++)  {
			x = (int)((iRound-j)*sin(PI*i));
			y = (int)((iRound-j)*cos(PI*i))-1;
			PutPixel(bmpBuffer,x+iWidth-iRound,y+iRound,cur_col);
			cur_col = MakeColour(dark_r+rstep*(j+1),dark_g+gstep*(j+1),dark_b+bstep*(j+1));
		}
	}

	// Bottom left
	// (3/2*PI,2PI)
	for (i=1.50f;i<2.0f;i+=step)  {
		cur_col = iDarkColour;
		for (j=0; j<iBorder; j++)  {
			x = (int)((iRound-j)*sin(PI*i))-1;
			y = (int)((iRound-j)*cos(PI*i));
			PutPixel(bmpBuffer,x+iRound,y+iHeight-iRound,cur_col);
			cur_col = MakeColour(dark_r+rstep*(j+1),dark_g+gstep*(j+1),dark_b+bstep*(j+1));
		}
	}

	// Bottom right
	// (0,PI)
	for (i=-0.01f;i<0.5f;i+=step)  {
		cur_col = iDarkColour;
		for (j=0; j<iBorder; j++)  {
			x = (int)((iRound-j)*sin(PI*i));
			y = (int)((iRound-j)*cos(PI*i));
			PutPixel(bmpBuffer,x+iWidth-iRound,y+iHeight-iRound,cur_col);
			cur_col = MakeColour(dark_r+rstep*(j+1),dark_g+gstep*(j+1),dark_b+bstep*(j+1));
		}
	}


	//
	//	Draw the background
	//
	if (iBgColour == tLX->clPink)
		return;

	// Draw the center rectangle
	DrawRectFill(bmpBuffer,line_left,iBorder,line_right,iHeight-iBorder,iBgColour);

	// No need to draw more
	if (iRound <= iBorder)
		return;

	DrawRectFill(bmpBuffer,iBorder,line_top,line_left,line_bottom,iBgColour);
	DrawRectFill(bmpBuffer,line_right,line_top,iWidth-iBorder,line_bottom,iBgColour);

	// Top left
	// (1,3/2*PI)
	for (i=1.01f;i<1.49;i+=step)  {
		for (j=iBorder; j<iRound-iBorder+2; j++)  {
			x = (int)((iRound-j)*sin(PI*i))-1;
			y = (int)((iRound-j)*cos(PI*i))-1;
			PutPixel(bmpBuffer,x+iRound,y+iRound,iBgColour);
		}
	}

	// Top right
	// (PI/2,PI)
	for (i=0.51f;i<0.99;i+=step)  {
		for (j=iBorder; j<iRound-iBorder+2; j++)  {
			x = (int)((iRound-j)*sin(PI*i));
			y = (int)((iRound-j)*cos(PI*i))-1;
			PutPixel(bmpBuffer,x+iWidth-iRound,y+iRound,iBgColour);
		}
	}
	
	// Bottom left
	// (3/2*PI,2PI)
	for (i=1.51f;i<1.99;i+=step)  {
		for (j=iBorder; j<iRound-iBorder+2; j++)  {
			x = (int)((iRound-j)*sin(PI*i))-1;
			y = (int)((iRound-j)*cos(PI*i));
			PutPixel(bmpBuffer,x+iRound,y+iHeight-iRound,iBgColour);
		}
	}

	// Bottom right
	// (0,PI)
	for (i=0.01f;i<0.499;i+=step)  {
		for (j=iBorder; j<=iRound-iBorder+2; j++)  {
			x = (int)((iRound-j)*sin(PI*i));
			y = (int)((iRound-j)*cos(PI*i));
			PutPixel(bmpBuffer,x+iWidth-iRound,y+iHeight-iRound,iBgColour);
		}
	}
	
}

/////////////////
// Draw the frame
void CBox::Draw(SDL_Surface *bmpDest)
{
	if (bmpBuffer)
		DrawImage(bmpDest,bmpBuffer,iX,iY);
}

/////////////////
// Free the widget
void CBox::Destroy(void)
{
	if(bmpBuffer) { 
		SDL_FreeSurface(bmpBuffer); 
		bmpBuffer = NULL;
	}
}

//////////////////
// Checks if any mouse event happened 
int	CBox::CheckEvent(void)
{
	// If the mouse is over transparent area, don't fire any event
	// CGuiLayout will then continue processing events for other widgets

	// Get the mouse pos
	mouse_t *Mouse = GetMouse();
	if (!Mouse)
		return BOX_NONE;
	int x = Mouse->X - iX;
	int y = Mouse->Y - iY;

	// Mouse in the buffer
	if (x < iWidth && y < iHeight)  {
		// Mouse over transparent pixel? No event
		if(GetPixel(bmpBuffer,x,y) == tLX->clPink)
			return BOX_NOEVENT;
		else
			return BOX_NONE;
	} else 
		return BOX_NONE;

}

///////////////////
// Load the style
void CBox::LoadStyle(void/*node_t *cssNode*/)
{
	node_t *cssNode = NULL;
	// Find the default checkbox class, if none specified
	if (!cssNode) {
		cssNode = cWidgetStyles.FindNode("box");
		if (!cssNode)
			return;
	}

	// Read properties
	property_t *prop = cssNode->tProperties;
	for(;prop;prop=prop->tNext) {
		// Border
		if (!stringcasecmp(prop->sName,"thickness"))  {
			iBorder = atoi(prop->sValue);
		}
		// Round
		if (!stringcasecmp(prop->sName,"round"))  {
			iRound = atoi(prop->sValue);
		}
		// Light colour
		if (!stringcasecmp(prop->sName,"lightcolor"))  {
			iLightColour = StrToCol(prop->sValue);
		}
		// Dark colour
		if (!stringcasecmp(prop->sName,"darkcolor"))  {
			iDarkColour = StrToCol(prop->sValue);
		}
		// Bg colour
		if (!stringcasecmp(prop->sName,"bgcolor"))  {
			iBgColour = StrToCol(prop->sValue);
		}
		// Unknown
		else {
			printf("Warning: Unknown property %s in main Checkbox class", prop->sName.c_str());
		}
	}
}
