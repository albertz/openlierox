// OpenLieroX


// Input box
// Created 30/3/03
// Jason Boettcher

// code under LGPL


#include "defs.h"
#include "LieroX.h"
#include "Menu.h"


///////////////////
// Draw the image
void CImage::Draw(SDL_Surface *bmpDest)
{
	// Don't try to draw non-existing image
	if (!tImage)
		return;

    Menu_redrawBufferRect(iX,iY, tImage->w,tImage->h);

	// Clipping
	if(iX+iWidth > bmpDest->w)
		iX = bmpDest->w-iWidth;
	if(iX < 0)
		iX = 0;
	if(iY+iHeight > bmpDest->h)
		iY = bmpDest->h-iHeight;
	if(iY < 0)
		iY=0;

	// Draw the image
	DrawImage(bmpDest,tImage,iX,iY);
}

///////////////////
// Changes the image
void CImage::Change(char *Path)
{
	if(!Path)
		return;

	// Delete the old path
	delete[] sPath;

	// Copy the new path
	size_t len = strlen(Path);
	sPath = new char[len+1];
	if(sPath) memcpy(sPath,Path,len+1);

	// Free the current image
	SDL_FreeSurface(tImage);

	// Load the new image
	tImage = LoadImage(sPath,0);

	// Update the width and height
	iWidth = tImage->w;
	iHeight = tImage->h;
}

/////////////////////
// This widget is a sendmessage
DWORD CImage::SendMessage(int iMsg, DWORD Param1, DWORD Param2)
{
	return 0;
}
