// OpenLieroX


// Image
// Created 30/3/03
// Jason Boettcher

// code under LGPL

#if 0

#include "LieroX.h"

//#include "Menu.h"
#include "GfxPrimitives.h"
#include "SkinnedGUI/CImage.h"


namespace SkinnedGUI {

////////////////
// Constructors
	
// TODO: this "cropping" isn't used at all, remove it?
// Cropping is used in GUI skinning in one place, I thought it would be nice, yet it's safe to remove it.
CImage::CImage(COMMON_PARAMS, const std::string& Path, int _cropX=0, int _cropY=0, int _cropW=0, int _cropH=0):
		CALL_DEFAULT_CONSTRUCTOR {
	CLEAR_EVENT(OnClick);
	iType = wid_Image;
	sPath = Path;
	tImage = NULL;

	cropX = _cropX;
	cropY = _cropY;
	cropW = _cropW;
	cropH = _cropH;
	if (Path.size() != 0)  {
		tImage = LoadGameImage(Path);

		if( cropW == 0 && cropX > 0 )
			cropW = tImage->w - cropX;
		if( cropH == 0 && cropY > 0 )
			cropH = tImage->h - cropY;
		if( cropW > tImage->w )
			cropW = tImage->w;
		if( cropH > tImage->h )
			cropH = tImage->h;

		iWidth = cropW > 0 ? cropW : tImage->w;
		iHeight = cropH > 0 ? cropH : tImage->h;

		if( cropX > tImage->w - iWidth )
			cropX = tImage->w - iWidth;
		if( cropY > tImage->h - iHeight )
			cropY = tImage->h - iHeight;
	}
}

CImage::CImage(COMMON_PARAMS, SDL_Surface *img) : CALL_DEFAULT_CONSTRUCTOR {
	CLEAR_EVENT(OnClick);
	iType = wid_Image;
	sPath = "";
	tImage = img;
	cropX = cropY = cropW = cropH = 0;
	if (tImage)  {
		iWidth = tImage->w;
		iHeight = tImage->h;
	}
}

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
		DrawImage(bmpDest,tImage,iX,iY); // Hopefully faster cropping
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
	tImage = LoadGameImage(sPath);
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

CWidget * CImage::WidgetCreator( const std::vector< ScriptVar_t > & p, CGuiLayoutBase * layout, int id, int x, int y, int dx, int dy )
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
							( "file", SVT_STRING )
							( "crop_x", SVT_INT32 )
							( "crop_y", SVT_INT32 )
							( "crop_w", SVT_INT32 )
							( "crop_h", SVT_INT32 )
							( "click", SVT_STRING );

}; // namespace SkinnedGUI

#endif

