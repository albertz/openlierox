/////////////////////////////////////////
//
//   OpenLieroX
//
//   Auxiliary Software class library
//
//   based on the work of JasonB
//   enhanced by Dark Charlie and Albert Zeyer
//
//   code under LGPL
//
/////////////////////////////////////////


// Font class
// Created 15/7/01
// Jason Boettcher


#ifndef __CFONT_H__
#define __CFONT_H__

#include <string>


#define NUM_CHARACTERS 468

class CFont {
public:
	// Constructor
	CFont() {
		bmpFont = NULL;
		Colour = false;
		OutlineFont = false;
		Spacing = 1;
	}


private:
	// Attributes

	SDL_Surface		*bmpFont;
	bool			Colour;
	int				FontWidth[NUM_CHARACTERS];
	int				CharacterOffset[NUM_CHARACTERS];
	int				OutlineFont;
	int				Spacing;

	// Common colours
	SDL_Surface		*bmpWhite;
	SDL_Surface		*bmpGreen;
    
    Uint32 f_pink;
    Uint32 f_blue;
    Uint32 f_white;
    Uint32 f_green;

public:
	// Methods

	int				Load(const UCString& fontname, bool _colour);
	void			CalculateWidth(void);

	void			PreCalculate(SDL_Surface *bmpSurf, Uint32 colour);

	//void			Draw(SDL_Surface *dst, int x, int y, Uint32 col, char *fmt,...);
	void			Draw(SDL_Surface *dst, int x, int y, Uint32 col, const UCString& txt);
	//void			DrawAdv(SDL_Surface *dst, int x, int y, int max_w, Uint32 col, char *fmt,...);
	void			DrawAdv(SDL_Surface *dst, int x, int y, int max_w, Uint32 col, const UCString& txt);
	//void			DrawCentre(SDL_Surface *dst, int x, int y, Uint32 col, char *fmt,...);
	void			DrawCentre(SDL_Surface *dst, int x, int y, Uint32 col, const UCString& txt);
	//void			DrawCentreAdv(SDL_Surface *dst, int x, int y, int min_x, int max_w, Uint32 col, char *fmt,...);
	void			DrawCentreAdv(SDL_Surface *dst, int x, int y, int min_x, int max_w, Uint32 col, const UCString& txt);

	void			Shutdown(void);

	void			SetOutline(int Outline);
	int				IsOutline(void);

	int				GetWidth(const UCString& buf);
	int				GetHeight(void)					{ return bmpFont->h; }
	int				TranslateCharacter(UCString::const_iterator &it, const UCString::const_iterator& last);

	inline void		SetSpacing(int _s)  { Spacing = _s; }
	inline int		GetSpacing()		 { return Spacing; }
private:
	bool			IsColumnFree(int x);
};








#endif  //  __CFONT_H__
