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
	CImage(const std::string& Path) {
		iType = wid_Image;
		sPath = Path;
		tImage = NULL;
		if (Path != "")  {
			tImage = LoadImage(Path);

			iWidth = tImage->w;
			iHeight = tImage->h;
		}
	}

	CImage(SDL_Surface *img) {
		iType = wid_Image;
		sPath = "";
		tImage = img;
		if (tImage)  {
			iWidth = tImage->w;
			iHeight = tImage->h;
		}
	}

private:
    // Attributes
	SDL_Surface	*tImage;
	std::string	sPath;
	CGuiSkin::CallbackHandler cClick;

public:
    // Methods

    void			Create(void)		{ iType = wid_Image; }
	void			Destroy(void)		{  }

	inline std::string	getPath(void)		{ return sPath; }
	SDL_Surface		*getSurface(void)	{ return tImage; }
	void			Change(const std::string& Path);
	void			Change(SDL_Surface *bmpImg);

	//These events return an event id, otherwise they return -1
	int		MouseOver(mouse_t *tMouse)				{ return IMG_NONE; }
	int		MouseUp(mouse_t *tMouse, int nDown)		{ return IMG_CLICK; }
	int		MouseDown(mouse_t *tMouse, int nDown)	{ return IMG_NONE; }
	int		MouseWheelDown(mouse_t *tMouse)			{ return IMG_NONE; }
	int		MouseWheelUp(mouse_t *tMouse)			{ return IMG_NONE; }
	int		KeyDown(UnicodeChar c, int keysym)		{ return IMG_NONE; }
	int		KeyUp(UnicodeChar c, int keysym)		{ return IMG_NONE; }

	DWORD SendMessage(int iMsg, DWORD Param1, DWORD Param2);
	DWORD SendMessage(int iMsg, const std::string& sStr, DWORD Param) { return 0; }
	DWORD SendMessage(int iMsg, std::string *sStr, DWORD Param)  { return 0; }

	void	Draw(SDL_Surface *bmpDest);

	void	LoadStyle(void) {}

	static CWidget * WidgetCreator( const std::vector< CGuiSkin::WidgetVar_t > & p, CGuiLayoutBase * layout, int id, int x, int y, int dx, int dy )
	{
		CImage * w = new CImage( p[0].s );
		w->cClick.Init( p[1].s, w );
		layout->Add( w, id, x, y, dx, dy );
		return w;
	};

	void	ProcessGuiSkinEvent(int iEvent) 
	{
		if( iEvent == IMG_CLICK )
			cClick.Call();
	};
};

static bool CImage_WidgetRegistered = 
	CGuiSkin::RegisterWidget( "image", & CImage::WidgetCreator )
							( "file", CGuiSkin::WVT_STRING )
							( "click", CGuiSkin::WVT_STRING );

#endif  //  __CIMAGE_H__
