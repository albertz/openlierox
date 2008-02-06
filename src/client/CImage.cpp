// OpenLieroX


// Input box
// Created 30/3/03
// Jason Boettcher

// code under LGPL


#include "LieroX.h"
#include "Menu.h"
#include "GfxPrimitives.h"
#include "CImage.h"


///////////////////
// Draw the image
void CImage::Draw(SDL_Surface *bmpDest)
{
	// Don't try to draw non-existing image
	if (!tImage)
		return;

	if (bRedrawMenu)
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
void CImage::Change(const std::string& Path)
{
	if(Path == "")
		return;

	// Copy the new path
	sPath = Path;

	// Load the new image
	tImage = LoadImage(sPath);

	// Update the width and height
	iWidth = tImage->w;
	iHeight = tImage->h;
}

void CImage::Change(SDL_Surface *bmpImg)
{
	// Just re-setup the image-related variables
	sPath = "";
	tImage = bmpImg;
	iWidth = bmpImg->w;
	iHeight = bmpImg->h;
}

/////////////////////
// This widget is a sendmessage
DWORD CImage::SendMessage(int iMsg, DWORD Param1, DWORD Param2)
{
	return 0;
}

static bool CImage_WidgetRegistered = 
	CGuiSkin::RegisterWidget( "image", & CImage::WidgetCreator )
							( "file", CScriptableVars::SVT_STRING )
							( "click", CScriptableVars::SVT_STRING );
