// OpenLieroX


// Image
// Created 29/10/06
// Dark Charlie

// code under LGPL


#ifndef __CIMAGE_H__DEPRECATED_GUI__
#define __CIMAGE_H__DEPRECATED_GUI__

#include "InputEvents.h"
#include "GfxPrimitives.h"
#include "DynDraw.h"

namespace DeprecatedGUI {

// Event types
enum {
	IMG_NONE = -1,
	IMG_CLICK = 0,
};


// Messages
enum {
	IMG_CHANGE
};

	// Cropping is used in GUI skinning in one place, I thought it would be nice, yet it's safe to remove it.
	// Note: I removed cropping for now as DynDrawIntf is used now - if you want to reimplement it, just implement a custom DynDrawIntf which does that


class CImage : public CWidget {
public:
	// Constructor
	CImage(const std::string& Path) {
		iType = wid_Image;
		sPath = Path;
		tImage = NULL;
		iWidth = iHeight = 0;
		if (Path != "")  {
			tImage = DynDrawFromSurface(LoadGameImage(Path));
			if( !tImage.get() )
				return;

			iWidth = tImage->w;
			iHeight = tImage->h;
		}
	}

	CImage(const SmartPointer<DynDrawIntf>& img) {
		iType = wid_Image;
		sPath = "";
		tImage = img;
		iWidth = iHeight = 0;
		if (tImage.get())  {
			iWidth = tImage.get()->w;
			iHeight = tImage.get()->h;
		}
	}

	CImage(const SmartPointer<SDL_Surface>& img) {
		iType = wid_Image;
		sPath = "";
		tImage = DynDrawFromSurface(img);
		iWidth = iHeight = 0;
		if (tImage.get())  {
			iWidth = tImage.get()->w;
			iHeight = tImage.get()->h;
		}		
	}
	
private:
    // Attributes
	SmartPointer<DynDrawIntf> tImage;
	std::string	sPath;

public:
    // Methods

    void			Create()		{ iType = wid_Image; }
	void			Destroy()		{  }

	inline std::string	getPath()		{ return sPath; }
	SmartPointer<DynDrawIntf> getSurface()	{ return tImage; }
	void			Change(const std::string& Path);
	void			Change(const SmartPointer<DynDrawIntf>& bmpImg);
	void			Change(const SmartPointer<SDL_Surface>& bmpImg) {
		Change(DynDrawFromSurface(bmpImg));
	}

	//These events return an event id, otherwise they return -1
	int		MouseOver(mouse_t *tMouse)				{ return IMG_NONE; }
	int		MouseUp(mouse_t *tMouse, int nDown)		{ return IMG_CLICK; }
	int		MouseDown(mouse_t *tMouse, int nDown)	{ return IMG_NONE; }
	int		MouseWheelDown(mouse_t *tMouse)			{ return IMG_NONE; }
	int		MouseWheelUp(mouse_t *tMouse)			{ return IMG_NONE; }
	int		KeyDown(UnicodeChar c, int keysym, const ModifiersState& modstate);
	int		KeyUp(UnicodeChar c, int keysym, const ModifiersState& modstate)		{ return IMG_NONE; }

	DWORD SendMessage(int iMsg, DWORD Param1, DWORD Param2);
	DWORD SendMessage(int iMsg, const std::string& sStr, DWORD Param) { return 0; }
	DWORD SendMessage(int iMsg, std::string *sStr, DWORD Param)  { return 0; }

	void	Draw(SDL_Surface * bmpDest);
};

}; // namespace DeprecatedGUI

#endif  //  __CIMAGE_H__DEPRECATED_GUI__
