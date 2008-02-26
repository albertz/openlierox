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
	if( cropX == 0 && cropY == 0 && cropW == 0 && cropH == 0 )
		DrawImage(bmpDest,tImage,0,0); // Hopefully faster cropping
	else
		DrawImageAdv(bmpDest,tImage,cropX,cropY,iX,iY,iWidth,iHeight);
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
	cropX = cropY = cropW = cropH = 0;
	
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
	cropX = cropY = cropW = cropH = 0;
}

/////////////////////
// This widget is a sendmessage
DWORD CImage::SendMessage(int iMsg, DWORD Param1, DWORD Param2)
{
	return 0;
}

CWidget * CImage::WidgetCreator( const std::vector< CScriptableVars::ScriptVar_t > & p, CGuiLayoutBase * layout, int id, int x, int y, int dx, int dy )
{
	CImage * w = new CImage( p[0].s, p[1].i, p[2].i, p[3].i, p[4].i );
	w->cClick.Init( p[5].s, w );
	if( dx == 0 )
		dx = w->iWidth;
	if( dy == 0 )
		dy = w->iHeight;
	layout->Add( w, id, x, y, dx, dy );
	return w;
};

static bool CImage_WidgetRegistered = 
	CGuiSkin::RegisterWidget( "image", & CImage::WidgetCreator )
							( "file", CScriptableVars::SVT_STRING )
							( "crop_x", CScriptableVars::SVT_INT )
							( "crop_y", CScriptableVars::SVT_INT )
							( "crop_w", CScriptableVars::SVT_INT )
							( "crop_h", CScriptableVars::SVT_INT )
							( "click", CScriptableVars::SVT_STRING );
