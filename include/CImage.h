// OpenLieroX


// Image
// Created 29/10/06
// Dark Charlie

// code under LGPL


#ifndef __CIMAGE_H__
#define __CIMAGE_H__


// Event types
enum {
	IMG_NONE=-1
};


// Messages
enum {
	IMG_CHANGE
};


class CImage : public CWidget {
public:
	// Constructor
	CImage(char *Path) {
		iType = wid_Image;
		sPath = NULL;
		tImage = NULL;
		if (Path)  {
			tImage = LoadImage(Path,0);
			size_t len = strlen(Path);
			sPath = new char[len+1];
			memcpy(sPath,Path,len+1);

			iWidth = tImage->w;
			iHeight = tImage->h;
		}
	}

private:
    // Attributes
	SDL_Surface	*tImage;
	char		*sPath;


public:
    // Methods

    void			Create(void)		{ iType = wid_Image; }
	void			Destroy(void)		{ if(sPath) delete[] sPath; }

	char			*getPath(void)		{ return sPath; }
	SDL_Surface		*getSurface(void)	{ return tImage; }
	void			Change(char *Path);

	//These events return an event id, otherwise they return -1
	int		MouseOver(mouse_t *tMouse)				{ return IMG_NONE; }
	int		MouseUp(mouse_t *tMouse, int nDown)		{ return IMG_NONE; }
	int		MouseDown(mouse_t *tMouse, int nDown)	{ return IMG_NONE; }
	int		MouseWheelDown(mouse_t *tMouse)			{ return IMG_NONE; }
	int		MouseWheelUp(mouse_t *tMouse)			{ return IMG_NONE; }
	int		KeyDown(int c)							{ return IMG_NONE; }
	int		KeyUp(int c)							{ return IMG_NONE; }

	DWORD SendMessage(int iMsg, DWORD Param1, DWORD Param2);

	void	Draw(SDL_Surface *bmpDest);

	void	LoadStyle(void) {}

};


#endif  //  __CIMAGE_H__


