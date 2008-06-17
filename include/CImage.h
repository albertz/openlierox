// OpenLieroX


// Image
// Created 29/10/06
// Dark Charlie

// code under LGPL


#ifndef __CIMAGE_H__
#define __CIMAGE_H__

#include "InputEvents.h"
#include "GfxPrimitives.h"



// Event types
enum {
	IMG_NONE = -1,
	IMG_CLICK = 0,
};


// Messages
enum {
	IMG_CHANGE
};


class CImage : public CWidget {
public:
	// Constructor
	// TODO: this "cropping" isn't used at all, remove it?
	// Cropping is used in GUI skinning in one place, I thought it would be nice, yet it's safe to remove it.
	CImage(const std::string& Path, int _cropX=0, int _cropY=0, int _cropW=0, int _cropH=0):
			cropX(_cropX), cropY(_cropY), cropW(_cropW), cropH(_cropH) {
		iType = wid_Image;
		sPath = Path;
		tImage = NULL;
		iWidth = iHeight = 0;
		if (Path != "")  {
			tImage = LoadGameImage(Path);
			if( !tImage.get() )
				return;

			if( cropW == 0 && cropX > 0 )
				cropW = tImage.get()->w - cropX;
			if( cropH == 0 && cropY > 0 )
				cropH = tImage.get()->h - cropY;
			if( cropW > tImage.get()->w )
				cropW = tImage.get()->w;
			if( cropH > tImage.get()->h )
				cropH = tImage.get()->h;

			iWidth = cropW > 0 ? cropW : tImage.get()->w;
			iHeight = cropH > 0 ? cropH : tImage.get()->h;

			if( cropX > tImage.get()->w - iWidth )
				cropX = tImage.get()->w - iWidth;
			if( cropY > tImage.get()->h - iHeight )
				cropY = tImage.get()->h - iHeight;
		}
	}

	CImage(SmartPointer<SDL_Surface> img) {
		iType = wid_Image;
		sPath = "";
		tImage = img;
		cropX = cropY = cropW = cropH = 0;
		if (tImage.get())  {
			iWidth = tImage.get()->w;
			iHeight = tImage.get()->h;
		}
	}

private:
    // Attributes
	SmartPointer<SDL_Surface> tImage;
	std::string	sPath;
	int cropX, cropY, cropW, cropH;
	CGuiSkin::CallbackHandler cClick;

public:
    // Methods

    void			Create(void)		{ iType = wid_Image; }
	void			Destroy(void)		{  }

	inline std::string	getPath(void)		{ return sPath; }
	SmartPointer<SDL_Surface> getSurface(void)	{ return tImage; }
	void			Change(const std::string& Path);
	void			Change(SmartPointer<SDL_Surface> bmpImg);

	//These events return an event id, otherwise they return -1
	int		MouseOver(mouse_t *tMouse)				{ return IMG_NONE; }
	int		MouseUp(mouse_t *tMouse, int nDown)		{ return IMG_CLICK; }
	int		MouseDown(mouse_t *tMouse, int nDown)	{ return IMG_NONE; }
	int		MouseWheelDown(mouse_t *tMouse)			{ return IMG_NONE; }
	int		MouseWheelUp(mouse_t *tMouse)			{ return IMG_NONE; }
	int		KeyDown(UnicodeChar c, int keysym, const ModifiersState& modstate)		{ return IMG_NONE; }
	int		KeyUp(UnicodeChar c, int keysym, const ModifiersState& modstate)		{ return IMG_NONE; }

	DWORD SendMessage(int iMsg, DWORD Param1, DWORD Param2);
	DWORD SendMessage(int iMsg, const std::string& sStr, DWORD Param) { return 0; }
	DWORD SendMessage(int iMsg, std::string *sStr, DWORD Param)  { return 0; }

	void	Draw(SDL_Surface * bmpDest);

	void	LoadStyle(void) {}

	static CWidget * WidgetCreator( const std::vector< CScriptableVars::ScriptVar_t > & p, CGuiLayoutBase * layout, int id, int x, int y, int dx, int dy );

	void	ProcessGuiSkinEvent(int iEvent) 
	{
		if( iEvent == IMG_CLICK )
			cClick.Call();
	};
};

#endif  //  __CIMAGE_H__
