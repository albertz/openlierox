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


#include "LieroX.h"
#include "GfxPrimitives.h"
#include "Unicode.h"

//
// IMPORTANT: all surfaces in CFont are alpha blended which means they can (and mostly do) have another format than screen.
// Because of this we cannot use almost any of the color handling functions from GfxPrimitives
// Format of the surfaces is guaranted to be 32bit ARGB
//


// For font drawing use FontGenerator tool in /tools/fontgenerator

///////////////////
// Load a font
int CFont::Load(const std::string& fontname, bool _colour) {

	// Load the font
	LOAD_IMAGE_WITHALPHA(bmpFont, fontname);

	// Set the color key for this alpha surface
	SetColorKey(bmpFont, 255, 0, 255);

	Colorize = _colour;

	bmpWhite = gfxCreateSurfaceAlpha(bmpFont->w, bmpFont->h);
	bmpGreen = gfxCreateSurfaceAlpha(bmpFont->w, bmpFont->h);

	// Calculate the width of each character and number of characters
	Parse();

	// Pre-calculate some colours
	f_white = tLX->clNormalLabel;
	f_green = tLX->clChatText;
	
	// Precache some common font colors (but only if this font should be colorized)
	if (Colorize) {
		PreCalculate(bmpWhite, f_white);
		PreCalculate(bmpGreen, f_green);
	}

	return true;
}


///////////////////
// Shutdown the font
void CFont::Shutdown(void) {
	if (bmpWhite) {
		SDL_FreeSurface(bmpWhite);
		bmpWhite = NULL;
	}
	if (bmpGreen) {
		SDL_FreeSurface(bmpGreen);
		bmpGreen = NULL;
	}
}


//////////////////
// Helper function for CalculateWidth
// Checks, whether a vertical line is free
// NOTE: bmpFont must be locked before calling this
bool CFont::IsColumnFree(int x) {
	// it's only completelly see through
	for (int y = 0; y < bmpFont->h; y++) {
		if ((GetPixel(bmpFont, x, y) & ALPHASURFACE_AMASK) != 0)
			return false;
	}

	return true;
}

///////////////////
// Calculate character widths, number of characters and offsets
void CFont::Parse(void) {
	int x;
	UnicodeChar CurChar = FIRST_CHARACTER;
	int cur_w;

	// Lock the surface
	LOCK_OR_QUIT(bmpFont);

	Uint32 blue = SDL_MapRGB(bmpFont->format, 0, 0, 255);

	// a blue pixel always indicates a new char exept for the first
	
	uint char_start = 0;
	for (x = 0; x < bmpFont->w; x++) {
		// x is always one pixel behind a blue line or x==0
	
		// Ignore any free pixel columns before the character
		char_start = x;
		while (IsColumnFree(x) && x < bmpFont->w)
			++x;

		// If we stopped at next blue line/end, this must be a kind of space
		if (GetPixel(bmpFont, x, 0) == blue || x == bmpFont->w)  {
			cur_w = x - char_start;

		// Non-blank character
		} else {
			char_start = x;
			++x;

			// Read until a blue pixel or end of the image
			cur_w = 1;
			while (GetPixel(bmpFont, x, 0) != blue && x < bmpFont->w) {
				++x;
				++cur_w;
			}

			// Ignore any free pixel columns *after* the character
			uint tmp_x = x - 1; // last line before blue line or end
			while (IsColumnFree(tmp_x) && cur_w) {
				--cur_w;
				--tmp_x;
			}
			
			if(cur_w == 0)
				printf("WARNING: cur_w == 0\n");
		}
		
		// Add the character
		FontWidth.push_back(cur_w);
		CharacterOffset.push_back(char_start);
		NumCharacters++;
		CurChar++;
	}



	// Unlock the surface
	UnlockSurface(bmpFont);
}

///////////////////
// Precalculate a font's colour
void CFont::PreCalculate(SDL_Surface *bmpSurf, Uint32 colour) {
	Uint32 pixel;
	int x, y;

	FillSurface(bmpSurf, SDL_MapRGBA(bmpSurf->format, 255, 0, 255, 0));

	// Lock the surfaces
	LOCK_OR_QUIT(bmpSurf);
	LOCK_OR_QUIT(bmpFont);

	Uint8 R, G, B, A;
	Uint8 sr, sg, sb;
	GetColour3(colour, getMainPixelFormat(), &sr, &sg, &sb);

	// Outline font: replace white pixels with appropriate color, put black pixels
	if (OutlineFont) {
		for (y = 0; y < bmpSurf->h; y++) {
			for (x = 0; x < bmpSurf->w; x++) {
				pixel = GetPixel(bmpFont, x, y);
				GetColour4(pixel, bmpFont->format, &R, &G, &B, &A);

				if (R == 255 && G == 255 && B == 255)    // White
					PutPixel(bmpSurf, x, y,
					         SDL_MapRGBA(bmpSurf->format, sr, sg, sb, A));
				else if (!R && !G && !B)   // Black
					PutPixel(bmpSurf, x, y,
					         SDL_MapRGBA(bmpSurf->format, 0, 0, 0, A));
			}
		}
	// Not outline: replace black pixels with appropriate color
	} else {
		for (y = 0; y < bmpSurf->h; y++) {
			for (x = 0; x < bmpSurf->w; x++) {
				pixel = GetPixel(bmpFont, x, y);
				GetColour4(pixel, bmpFont->format, &R, &G, &B, &A);

				if (!R && !G && !B)   // Black
					PutPixel(bmpSurf, x, y,
					         SDL_MapRGBA(bmpSurf->format, sr, sg, sb, A));
			}
		}
	}


	// Unlock the surfaces
	UnlockSurface(bmpSurf);
	UnlockSurface(bmpFont);
}


////////////////////
// Get height of multiline text
int CFont::GetHeight(const std::string& buf) {
	int numlines = 1;
	for (std::string::const_iterator i = buf.begin(); i != buf.end(); i++)
		if (*i == '\n') numlines++;
	return numlines * (bmpFont->h + VSpacing);
}

///////////////////
// Draws a font at X, Y, but visible only in specified rect
// HINT: not thread-safe
void CFont::DrawInRect(SDL_Surface *dst, int x, int y, int rectX, int rectY, int rectW, int rectH, Uint32 col, const std::string &txt)  {
	// Set the special clipping rectangle and then draw the font

	SDL_Rect oldrect, newrect;
	SDL_GetClipRect(dst, &oldrect);  // Save the old rect

	// Fill in the details
	newrect.x = rectX;
	newrect.y = rectY;
	newrect.w = rectW;
	newrect.h = rectH;

	// Special clipping
	SDL_SetClipRect(dst, &newrect);

	// Blit the font
	DrawAdv(dst, x, y, 9999, col, txt);

	// Restore original clipping rect
	SDL_SetClipRect(dst, &oldrect);
}

///////////////////
// Draw a font (advanced)
void CFont::DrawAdv(SDL_Surface *dst, int x, int y, int max_w, Uint32 col, const std::string& txt) {

	if (txt.size() == 0)
		return;

	// Clipping rectangle
	SDL_Rect oldrect = dst->clip_rect;
	SDL_Rect newrect = dst->clip_rect;

	// Set the newrect width and use this newrect temporarily to draw the font
	// We use this rect because of precached fonts which use SDL_Blit for drawing (and it takes care of cliprect)
	newrect.w = MIN(oldrect.w, (Uint16)max_w);
	newrect.x = MAX(oldrect.x, (Sint16)x);
	newrect.y = MAX(oldrect.y, (Sint16)y);
	if (!ClipRefRectWith(newrect.x, newrect.y, newrect.w, newrect.h, (SDLRect&)oldrect))
		return;
	SDL_SetClipRect(dst, &newrect);


	// Look in the precached fonts if there's some for this color
	SDL_Surface *bmpCached = NULL;
	if (Colorize) {
		// HINT: if we leave this disabled, the drawing will always be done manually
		// this is a bit (not not much) slower but prevents from the usual errors with CFont (wrong color, invisible, so on)
		// TODO: should we completly remove the caches? how much speed improvements do they give?
		// under MacOSX, it doesn't seem to give any performance improvement at all, at least no change in FPS
		// I have activated it again now as it seem to work at the moment (perhabps because of my change in gfxCreateSurfaceAlpha)
		if (col == f_white)
			bmpCached = bmpWhite;
		else if (col == tLX->clBlack)
			bmpCached = bmpFont;
		else if (col == f_green)
			bmpCached = bmpGreen;
	}
	// Not colourize, bmpFont itself should be blitted without any changes, so it's precached
	else {
		bmpCached = bmpFont;
	}


	// Lock the surfaces
	// If we use cached font, we do not access the pixels
	if (!bmpCached)  {
		LOCK_OR_QUIT(dst);
		LOCK_OR_QUIT(bmpFont);
	}

	Uint8 R, G, B, A;

	// Adjust the color to the dest-suface format
	GetColour3(col, getMainPixelFormat(), &R, &G, &B);
	col = SDL_MapRGB(dst->format, R, G, B);

	// Position at destination surface
	int char_y = y;
	int char_h = bmpFont->h;

	// Vertical clipping
	OneSideClip(char_y, char_h, newrect.y, newrect.h);

	for (std::string::const_iterator p = txt.begin(); p != txt.end();) {

		// Line break
		if (*p == '\n') {
			y += bmpFont->h + VSpacing;
			char_y = y;
			x = newrect.x;
			p++;

			// If any further text wouldn't be drawn, just stop
			if (char_y >= (newrect.y + newrect.h))
				break;
			else  {
				OneSideClip(char_y, char_h, newrect.y, newrect.h);
				continue;
			}
		}
	
		// Translate and ignore unknown
		int l = TranslateCharacter(p, txt.end()); // HINT: increases the iterator
		if (l == -1)
			continue;


		// Check if the current line will be drawn, if not, just proceed
		if (char_h == 0)
			continue;

		// Precached fonts
		if (bmpCached) {
			DrawImageAdv(dst, bmpCached, CharacterOffset[l], 0, x, y, FontWidth[l], bmpFont->h);
			x += FontWidth[l] + Spacing;
			continue;
		}

		// Horizontal clipping
		int char_x = x;
		int char_w = FontWidth[l];
		if (!OneSideClip(char_x, char_w, newrect.x, newrect.w))  {
			x += FontWidth[l] + Spacing;
			continue;
		}


		const short bpp = bmpFont->format->BytesPerPixel;
		Uint8 *src = (Uint8 *) bmpFont->pixels + bmpFont->pitch * (char_y - y) + 
						(CharacterOffset[l] + char_x - x) * bpp;
		Uint8 *px;

		// Outline font
		if (OutlineFont) {
			for (int j = 0; j < char_h; ++j) {
				px = src;
				for (int i = 0; i < char_w; ++i, px += bpp) {

					Uint32 pixel = GetPixelFromAddr(px, bpp);
					GetColour4(pixel, bmpFont->format, &R, &G, &B, &A);

					// Put black pixels and colorize white ones
					if (R == 255 && G == 255 && B == 255)    // White
						PutPixelA(dst, char_x + i, char_y + j, col, A);    // Put the pixel and blend it with background
					else if (!R && !G && !B)    // Black
						PutPixelA(dst, char_x + i, char_y + j, tLX->clBlack, A);
				}
				src += bmpFont->pitch;
			}
		}
		// Not outline
		else {
			for (int j = 0; j < char_h; ++j) {
				px = src;
				for (int i = 0; i < char_w; ++i, px += bpp) {

					Uint32 pixel = GetPixelFromAddr(px, bpp);
					GetColour4(pixel, bmpFont->format, &R, &G, &B, &A);

					// Put only black pixels
					if (!R && !G && !B)
						PutPixelA(dst, char_x + i, char_y + j, col, A);
				}
				src += bmpFont->pitch;
			}
		}

		x += FontWidth[l] + Spacing;
	}

	// Restore the original clipping rect
	SDL_SetClipRect(dst, &oldrect);


	// Unlock the surfaces
	if (!bmpCached)  {
		UnlockSurface(dst);
		UnlockSurface(bmpFont);
	}
}


///////////////////
// Calculate the width of a string of text
int CFont::GetWidth(const std::string& buf) {
	int length = 0;
	short l;

	// Calculate the length of the text
	for (std::string::const_iterator p = buf.begin(); p != buf.end();) {
		l = TranslateCharacter(p, buf.end());
		if (l != -1)
			length += FontWidth[l] + Spacing;
	}

	return length;
}

/////////////////
// Get width of a single character
int CFont::GetCharacterWidth(UnicodeChar c)
{
	int l = TranslateCharacter(c);
	if (l != -1)
		return FontWidth[l];
	else
		return 0;
}

///////////////////
// Draws the text in centre alignment
void CFont::DrawCentre(SDL_Surface *dst, int x, int y, Uint32 col, const std::string& txt) {
	Draw(dst, x - GetWidth(txt) / 2, y, col, txt);
}

///////////////////
// Draw's the text in centre alignment
void CFont::DrawCentreAdv(SDL_Surface *dst, int x, int y, int min_x, int max_w, Uint32 col, const std::string& txt) {
	DrawAdv(dst, MAX(min_x, x - GetWidth(txt) / 2), y, max_w, col, txt);
}
