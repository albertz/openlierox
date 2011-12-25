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
#include "CodeAttributes.h"

struct PixelPutAlpha;
struct PixelGet;


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

	Color f_white;
	Color f_green;

public:
	// Methods

	int					Load(const std::string& fontname, bool _colour);

	INLINE void			Draw(SDL_Surface * dst, int x, int y, Color col, const std::string& txt)  {
		DrawAdv(dst, x, y, 99999, col, txt);
	}
	void				DrawAdv(SDL_Surface * dst, int x, int y, int max_w, Color col, const std::string& txt);
	void				DrawCentre(SDL_Surface * dst, int x, int y, Color col, const std::string& txt);
	void				DrawCentreAdv(SDL_Surface * dst, int x, int y, int min_x, int max_w, Color col, const std::string& txt);
	void				DrawInRect(SDL_Surface * dst, int x, int y, int rectX, int rectY, int rectW, int rectH, Color col, const std::string& txt);
	void				DrawGlyph(SDL_Surface *dst, int x, int y, Color col, UnicodeChar c);

	void				Shutdown();

	INLINE void			SetOutline(bool _o)  {
		OutlineFont = _o;
	}
	INLINE bool			IsOutline()  {
		return OutlineFont;
	}

	int					GetWidth(const std::string& buf);
	int					GetCharacterWidth(UnicodeChar c);
	INLINE int			GetHeight() { return bmpFont.get()->h + VSpacing; }
	int					GetHeight(const std::string& buf);

	// Translates the character to the position in Fontstr array, returns -1 if impossible
	INLINE int			TranslateCharacter(std::string::const_iterator &it, const std::string::const_iterator& last)  {
		UnicodeChar ch = GetNextUnicodeFromUtf8(it, last);
		if (ch > FIRST_CHARACTER + NumCharacters - 1 || ch < FIRST_CHARACTER) return -1;
		return ch - FIRST_CHARACTER;
	}

	INLINE int			TranslateCharacter(UnicodeChar ch)  {
		if (ch > FIRST_CHARACTER + NumCharacters - 1 || ch < FIRST_CHARACTER) return -1;
		return ch - FIRST_CHARACTER;
	}

	INLINE bool			CanDisplayCharacter (UnicodeChar c)  { return (c < FIRST_CHARACTER + NumCharacters) && (c >= FIRST_CHARACTER); }

	INLINE void			SetSpacing(int _s)  {
		Spacing = _s;
	}
	INLINE int			GetSpacing()		 {
		return Spacing;
	}
	INLINE void			SetVSpacing(int _v) {
		VSpacing = _v;
	}
	INLINE int			GetVSpacing()	{
		return VSpacing;
	}
private:
	bool				IsColumnFree(int x);
	void				Parse();
	void				PreCalculate(const SmartPointer<SDL_Surface> & bmpSurf, Color colour);
	
	// Internal functions for glyph drawing, first one for normal fonts, second one for outline fonts
	// These do the fast glyph blit without any additional checks or clipping
	void				DrawGlyphNormal_Internal(SDL_Surface *dst, const SDL_Rect& r, int sx, int sy, Color col, int glyph_index, PixelPutAlpha& putter, PixelGet& getter);
	void				DrawGlyphOutline_Internal(SDL_Surface *dst, const SDL_Rect& r, int sx, int sy, Color col, int glyph_index, PixelPutAlpha& putter, PixelGet& getter);
};








#endif  //  __CFONT_H__
