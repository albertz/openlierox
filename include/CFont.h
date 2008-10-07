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

#include <SDL.h>
#include <vector>
#include "Unicode.h"
#include "SmartPointer.h"
#include "Color.h"

class PixelPutAlpha;
class PixelGet;


#define FIRST_CHARACTER 32 // space

class CFont {
public:
	// Constructor
	CFont() {
		bmpFont = bmpWhite = bmpGreen = NULL;
		Colorize = false;
		OutlineFont = false;
		Spacing = 1;
		VSpacing = 3;
		NumCharacters = 0;
	}


private:
	// Attributes

	SmartPointer<SDL_Surface>		bmpFont;
	bool							Colorize;
	std::vector<int>				FontWidth;
	std::vector<int>				CharacterOffset;
	bool							OutlineFont;
	int								Spacing;
	int								VSpacing;
	size_t							NumCharacters;

	// Common colours
	SmartPointer<SDL_Surface> bmpWhite;
	SmartPointer<SDL_Surface> bmpGreen;

	Uint32 f_white;
	Uint32 f_green;

public:
	// Methods

	int					Load(const std::string& fontname, bool _colour);

	inline void			Draw(SDL_Surface * dst, int x, int y, Uint32 col, const std::string& txt)  {
		DrawAdv(dst, x, y, 99999, col, txt);
	}
	void				DrawAdv(SDL_Surface * dst, int x, int y, int max_w, Uint32 col, const std::string& txt);
	void				DrawCentre(SDL_Surface * dst, int x, int y, Uint32 col, const std::string& txt);
	void				DrawCentreAdv(SDL_Surface * dst, int x, int y, int min_x, int max_w, Uint32 col, const std::string& txt);
	void				DrawInRect(SDL_Surface * dst, int x, int y, int rectX, int rectY, int rectW, int rectH, Uint32 col, const std::string& txt);
	void				DrawGlyph(SDL_Surface *dst, int x, int y, Color col, UnicodeChar c);

	void				Shutdown(void);

	inline void			SetOutline(bool _o)  {
		OutlineFont = _o;
	}
	inline bool			IsOutline(void)  {
		return OutlineFont;
	}

	int					GetWidth(const std::string& buf);
	int					GetCharacterWidth(UnicodeChar c);
	inline int			GetHeight(void) { return bmpFont.get()->h + VSpacing; }
	int					GetHeight(const std::string& buf);

	// Translates the character to the position in Fontstr array, returns -1 if impossible
	inline int			TranslateCharacter(std::string::const_iterator &it, const std::string::const_iterator& last)  {
		UnicodeChar ch = GetNextUnicodeFromUtf8(it, last);
		if (ch > FIRST_CHARACTER + NumCharacters - 1 || ch < FIRST_CHARACTER) return -1;
		return ch - FIRST_CHARACTER;
	}

	inline int			TranslateCharacter(UnicodeChar ch)  {
		if (ch > FIRST_CHARACTER + NumCharacters - 1 || ch < FIRST_CHARACTER) return -1;
		return ch - FIRST_CHARACTER;
	}

	inline bool			CanDisplayCharacter (UnicodeChar c)  { return (c < FIRST_CHARACTER + NumCharacters) && (c >= FIRST_CHARACTER); }

	inline void			SetSpacing(int _s)  {
		Spacing = _s;
	}
	inline int			GetSpacing()		 {
		return Spacing;
	}
	inline void			SetVSpacing(int _v) {
		VSpacing = _v;
	}
	inline int			GetVSpacing()	{
		return VSpacing;
	}
private:
	bool				IsColumnFree(int x);
	void				Parse(void);
	void				PreCalculate(const SmartPointer<SDL_Surface> & bmpSurf, Uint32 colour);
	
	// Internal functions for glyph drawing, first one for normal fonts, second one for outline fonts
	// These do the fast glyph blit without any additional checks or clipping
	void				DrawGlyphNormal_Internal(SDL_Surface *dst, const SDL_Rect& r, int sx, int sy, Color col, int glyph_index, PixelPutAlpha& putter, PixelGet& getter);
	void				DrawGlyphOutline_Internal(SDL_Surface *dst, const SDL_Rect& r, int sx, int sy, Color col, int glyph_index, PixelPutAlpha& putter, PixelGet& getter);
};








#endif  //  __CFONT_H__
