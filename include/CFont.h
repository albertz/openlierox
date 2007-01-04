/////////////////////////////////////////
//
//   Auxiliary Software class library
//
/////////////////////////////////////////


// Font class
// Created 15/7/01
// Jason Boettcher


#ifndef __CFONT_H__
#define __CFONT_H__


class CFont {
public:
	// Constructor
	CFont() {
		bmpFont = NULL;
		Colour = false;
		Width=18;
		OutlineFont = false;
	}


private:
	// Attributes

	SDL_Surface		*bmpFont;
	int				Colour;
	int				Width;
	int				FontWidth[256];
	int				OutlineFont;

	// Common colours
	SDL_Surface		*bmpWhite;
	SDL_Surface		*bmpGreen;
    
    Uint32 f_pink;
    Uint32 f_blue;
    Uint32 f_white;
    Uint32 f_green;



public:
	// Methods

	int				Load(char *fontname, int _colour, int _width);
	void			CalculateWidth(void);

	void			PreCalculate(SDL_Surface *bmpSurf, Uint32 colour);

	void			Draw(SDL_Surface *dst, int x, int y, int col, char *fmt,...);
	void			DrawAdv(SDL_Surface *dst, int x, int y, int max_w, int col, char *fmt,...);
	void			DrawCentre(SDL_Surface *dst, int x, int y, int col, char *fmt,...);
	void			DrawCentreAdv(SDL_Surface *dst, int x, int y, int min_x, int max_w, int col, char *fmt,...);

	void			Shutdown(void);

	void			SetOutline(int Outline);
	int				IsOutline(void);

	int				GetWidth(char *buf);
	int				GetHeight(void)					{ return bmpFont->h; }
};








#endif  //  __CFONT_H__
