// OpenLieroX


// Input box
// Created 30/3/03
// Jason Boettcher


#include "defs.h"
#include "LieroX.h"
#include "Menu.h"


///////////////////
// Draw the frame into a buffer
void CFrame::PreDraw(void)
{
	// Create the buffer, if needed
	if (!bmpBuffer)  {
		SDL_PixelFormat *fmt = SDL_GetVideoSurface()->format;
		if (fmt)
			bmpBuffer = SDL_CreateRGBSurface(SDL_SWSURFACE,iWidth,iHeight,fmt->BitsPerPixel,fmt->Rmask,fmt->Gmask,fmt->Bmask,fmt->Amask);
		else
			bmpBuffer = NULL;

		if (!bmpBuffer)
			return;
	}

	// Set the whole buffer transparent
	SDL_SetColorKey(bmpBuffer, SDL_SRCCOLORKEY, SDL_MapRGB(bmpBuffer->format,255,0,255));
	DrawRectFill(bmpBuffer,0,0,iWidth,iHeight,MakeColour(255,0,255));

	//
	// 1. Draw the lines
	//
	int line_left,line_right,line_top,line_bottom;

	line_left = iRound;
	line_right = iWidth - iRound;
	line_top = iRound;
	line_bottom = iHeight - iRound;

	Uint32 cur_col = iDarkColour;
	Uint8 dark_r,dark_g,dark_b;
	SDL_GetRGB(iDarkColour,bmpBuffer->format,&dark_r,&dark_g,&dark_b);
	Uint8 light_r,light_g,light_b;
	SDL_GetRGB(iLightColour,bmpBuffer->format,&light_r,&light_g,&light_b);
	int rstep,gstep,bstep;
	if (iBorder)  {
		rstep = (light_r-dark_r)/iBorder;
		gstep = (light_g-dark_g)/iBorder;
		bstep = (light_b-dark_b)/iBorder;
	}

	// Top line
	for (int j=0; j<iBorder; j++)  {
		DrawHLine(bmpBuffer,line_left,line_right,iY+j,cur_col);
		cur_col = MakeColour(dark_r+rstep*(j+1),dark_g+gstep*(j+1),dark_b+bstep*(j+1));
	}

	cur_col = iDarkColour;

	// Bottom line
	for (j=0; j<iBorder; j++)  {
		DrawHLine(bmpBuffer,line_left,line_right,iY+iHeight-j,cur_col);
		cur_col = MakeColour(dark_r+rstep*(j+1),dark_g+gstep*(j+1),dark_b+bstep*(j+1));
	}

	cur_col = iDarkColour;

	// Left line
	for (j=0; j<iBorder; j++)  {
		DrawVLine(bmpBuffer,line_top,line_bottom,iX+j,cur_col);
		cur_col = MakeColour(dark_r+rstep*(j+1),dark_g+gstep*(j+1),dark_b+bstep*(j+1));
	}

	cur_col = iDarkColour;

	// Right line
	for (j=0; j<iBorder; j++)  {
		DrawVLine(bmpBuffer,line_top,line_bottom,iX+iWidth-j,cur_col);
		cur_col = MakeColour(dark_r+rstep*(j+1),dark_g+gstep*(j+1),dark_b+bstep*(j+1));
	}


	//
	// 2. Draw the round borders
	//

	int x,y;

	float fStart = GetMilliSeconds();

	// Top left
	// (1,3/2*PI)
	float step = 1/(PI*iRound*(iBorder+0.0000001));
	for (float i=1.00;i<1.5;i+=step)  {
		cur_col = iDarkColour;
		for (j=0; j<iBorder; j++)  {
			x = (iRound-j)*sin(PI*i)-1;
			y = (iRound-j)*cos(PI*i)-1;
			PutPixel(bmpBuffer,x+iRound,y+iRound,cur_col);
			cur_col = MakeColour(dark_r+rstep*(j+1),dark_g+gstep*(j+1),dark_b+bstep*(j+1));
		}
	}

	// Top right
	// (PI/2,PI)
	for (i=0.50;i<1;i+=step)  {
		cur_col = iDarkColour;
		for (j=0; j<iBorder; j++)  {
			x = (iRound-j)*sin(PI*i)+1;
			y = (iRound-j)*cos(PI*i)-1;
			PutPixel(bmpBuffer,x+iWidth-iRound,y+iRound,cur_col);
			cur_col = MakeColour(dark_r+rstep*(j+1),dark_g+gstep*(j+1),dark_b+bstep*(j+1));
		}
	}

	// Bottom left
	// (3/2*PI,2PI)
	for (i=1.50;i<2;i+=step)  {
		cur_col = iDarkColour;
		for (j=0; j<iBorder; j++)  {
			x = (iRound-j)*sin(PI*i)-1;
			y = (iRound-j)*cos(PI*i)+1;
			PutPixel(bmpBuffer,x+iRound,y+iHeight-iRound,cur_col);
			cur_col = MakeColour(dark_r+rstep*(j+1),dark_g+gstep*(j+1),dark_b+bstep*(j+1));
		}
	}

	// Bottom right
	// (0,PI)
	for (i=-0.01;i<0.5;i+=step)  {
		cur_col = iDarkColour;
		for (j=0; j<iBorder; j++)  {
			x = (iRound-j)*sin(PI*i)+1;
			y = (iRound-j)*cos(PI*i)+1;
			PutPixel(bmpBuffer,x+iWidth-iRound,y+iHeight-iRound,cur_col);
			cur_col = MakeColour(dark_r+rstep*(j+1),dark_g+gstep*(j+1),dark_b+bstep*(j+1));
		}
	}

	float fLength = (GetMilliSeconds()-fStart)*1000;


	
}

/////////////////
// Draw the frame
void CFrame::Draw(SDL_Surface *bmpDest)
{
	if (bmpBuffer)
		DrawImage(bmpDest,bmpBuffer,iX,iY);
}

/////////////////
// Free the widget
void CFrame::Destroy(void)
{
	if(bmpBuffer) { 
		SDL_FreeSurface(bmpBuffer); 
		bmpBuffer = NULL;
	}
}