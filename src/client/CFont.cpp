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


#include "defs.h"
#include "LieroX.h"
#include "GfxPrimitives.h"
#include "Unicode.h"


//char Fontstr[256] = {" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_'abcdefghijklmnopqrstuvwxyz{|}~ \161\162\163\164\165\166\167\168\169\170\171\172\173\174\175\176\177\178\179\180\181\182\183\184\185\186\187\188\189\190\191\192\193\194\195\196\197\198\199\200\201\202\203\204\205\206\207\208\209\210\211\212\213\214\215\216\217\218\219\220\221\222\223\224\225\226\227\228\229\230\231\232\233\234\235\236\237\238\239\240\241\242\243\244\245\246\247\248\249\250\251\252\253\254\255"};

// When you draw the font, use this to get rid of all possible color "bugs" in it

/*bool SimilarColors(Uint32 color1, Uint32 color2, SDL_PixelFormat *fmt)  {
	static unsigned __int8 r1,g1,b1,r2,g2,b2;
	SDL_GetRGB(color1,fmt,&r1,&g1,&b1);
	SDL_GetRGB(color2,fmt,&r2,&g2,&b2);
	return (abs(r1-r2) <= 20) && (abs(g1-g2) <= 20) && (abs(b1-b2) <= 20);
}

void AdjustFont(SDL_Surface *bmp,const std::string& fname)
{
	SDL_Surface *dst = gfxCreateSurface(bmp->w,bmp->h);
	Uint8 *spxr = (Uint8 *)bmp->pixels;
	Uint8 *spx;
	Uint32 blue = SDL_MapRGB(bmp->format,0,0,255);
	Uint32 pink = SDL_MapRGB(bmp->format,255,0,255);
	Uint32 white = SDL_MapRGB(bmp->format,255,255,255);
	Uint32 black = 0;
	for (int y=0;y<bmp->h;y++)  {
		spx = spxr;
		for (int x=0;x<bmp->w;x++,spx+=bmp->format->BytesPerPixel)  {
			if (SimilarColors(GetPixelFromAddr(spx,bmp->format->BytesPerPixel),pink,bmp->format))
				PutPixel(dst,x,y,pink);
			else if (SimilarColors(GetPixelFromAddr(spx,bmp->format->BytesPerPixel),black,bmp->format)) 
				PutPixel(dst,x,y,black);
			else if (SimilarColors(GetPixelFromAddr(spx,bmp->format->BytesPerPixel),blue,bmp->format)) 
				PutPixel(dst,x,y,blue);
			else if (SimilarColors(GetPixelFromAddr(spx,bmp->format->BytesPerPixel),white,bmp->format)) 
				PutPixel(dst,x,y,white);
			else
				PutPixel(dst,x,y,pink);
		}
		spxr += bmp->pitch;
	}
	SDL_SaveBMP(dst,fname.c_str());	
}*/


///////////////////
// Load a font
int CFont::Load(const std::string& fontname, bool _colour)
{

	LOAD_IMAGE_WITHALPHA(bmpFont,fontname);

	Colorize = _colour;

	//if(!bmpFont) return false;

	//AdjustFont(bmpFont,fontname+"_adj.bmp");

	bmpWhite = gfxCreateSurfaceAlpha(bmpFont->w,bmpFont->h);
	bmpGreen = gfxCreateSurfaceAlpha(bmpFont->w,bmpFont->h);

	// Calculate the width of each character, number of characters and the fontstr
	Parse();

	// Precache some common font colors (but only if this font should be colorized)
	if (Colorize)  {
		PreCalculate(bmpWhite,ConvertColor(tLX->clNormalLabel,SDL_GetVideoSurface()->format,bmpWhite->format));
		PreCalculate(bmpGreen,ConvertColor(tLX->clChatText,SDL_GetVideoSurface()->format,bmpGreen->format));
	}


	// Pre-calculate some colours
	f_pink = ConvertColor(tLX->clPink,SDL_GetVideoSurface()->format,bmpFont->format) | bmpFont->format->Amask;
	f_blue = ConvertColor(tLX->clHeading,SDL_GetVideoSurface()->format,bmpFont->format) | bmpFont->format->Amask;//SDL_MapRGB(bmpFont->format,0,0,255);
	f_white = ConvertColor(tLX->clNormalLabel,SDL_GetVideoSurface()->format,bmpFont->format);//MakeColour(255,255,255);
	f_green = ConvertColor(tLX->clChatText,SDL_GetVideoSurface()->format,bmpFont->format) | bmpFont->format->Amask;//MakeColour(0,255,0);

	// Set the color key for this alpha surface (SDL_SetColorKey does not work for alpha blended surfaces)
	SetColorKeyAlpha(bmpFont, 255,0,255);

	return true;
}


///////////////////
// Shutdown the font
void CFont::Shutdown(void)
{
	if(bmpWhite)  {
		SDL_FreeSurface(bmpWhite);
		bmpWhite = NULL;
	}
	if(bmpGreen)  {
		SDL_FreeSurface(bmpGreen);
		bmpGreen = NULL;
	}
}


//////////////////
// Helper function for CalculateWidth
// Checks, whether a vertical line is free (all pixels pink)
bool CFont::IsColumnFree(int x)
{
	// NOTE: antialiased (alpha blended) fonts use pink as background color, too
	// it's only completelly see through
	static Uint8 R,G,B;
	for (ushort i=0; i < bmpFont->h; i++)  {
		SDL_GetRGB(GetPixel(bmpFont,x,i),bmpFont->format,&R,&G,&B);
		if ((byte)(~R+~B+G))  // ~R+~B+G = 0+0+0 => 0 is pink, otherwise not pink and stop
			return false;
	}

	return true;
}

///////////////////
// Calculate character widths, number of characters, fontstr and offsets
void CFont::Parse(void)
{
	uint x;
	UnicodeChar CurChar=FIRST_CHARACTER;
	int cur_w;

	// Lock the surface
	if(SDL_MUSTLOCK(bmpFont))
		SDL_LockSurface(bmpFont);

	static const Uint32 blue = SDL_MapRGB(bmpFont->format,0,0,255);

	cur_w = 0;
	uint tmp_x = 0;
	for (x=0; (int)x<bmpFont->w; x++)  {

		if (CurChar != ' ')
			while (IsColumnFree(x) && (int)x<bmpFont->w)  // Ignore any free pixel columns before the character (but don't do this for spaces)
				x++;

		// Read until a blue pixel or end of the image
		while (GetPixel(bmpFont,x,0) != blue && (int)x<bmpFont->w)  {
			x++;
			cur_w++;
		}

		// Ignore any free pixel columns *after* the character
		tmp_x = x-1; // -1 - blue line
		if (CurChar != ' ')  {
			while (IsColumnFree(tmp_x))  {
				cur_w--;
				tmp_x--;
			}
		}

		// Blue pixel means end of the character
		FontWidth.push_back(cur_w);
		CharacterOffset.push_back(tmp_x-cur_w+1);
		Fontstr += CurChar;
		NumCharacters++;
		CurChar++;
		cur_w = 0;
	}



	// Unlock the surface
	if(SDL_MUSTLOCK(bmpFont))
		SDL_UnlockSurface(bmpFont);
}


///////////////////
// Precalculate a font's colour
void CFont::PreCalculate(SDL_Surface *bmpSurf, Uint32 colour)
{
	register Uint32 pixel;
	int x,y;

	DrawRectFill(bmpSurf,0,0,bmpSurf->w,bmpSurf->h,SDL_MapRGBA(bmpSurf->format,255,0,255,SDL_ALPHA_TRANSPARENT));

	// Lock the surface
	if(SDL_MUSTLOCK(bmpSurf))
		SDL_LockSurface(bmpSurf);

	Uint8 R,G,B;
	Uint32 alpha;
	Uint8 sr,sg,sb;
	SDL_GetRGB(colour,bmpFont->format,&sr,&sg,&sb);

	// Outline font: replace white pixels with appropriate color, put black pixels
	if (OutlineFont)  {
		for(y=0;y<bmpSurf->h;y++) {
			for(x=0;x<bmpSurf->w;x++) {
				pixel = GetPixel(bmpFont,x,y);
				SDL_GetRGB(pixel,bmpSurf->format,&R,&G,&B);
				alpha = bmpSurf->format->Amask-(pixel & bmpSurf->format->Amask);

				if(!(byte)(~R+~G+~B))  // White
					PutPixel(bmpSurf,x,y,colour - alpha);
				else if (!(R+G+B)) // Black
					PutPixel(bmpSurf,x,y,pixel); // "pixel", not 0, because "pixel" containst alpha info
			}
		}
	// Not outline: replace black pixels with appropriate color
	} else {
		for(y=0;y<bmpSurf->h;y++) {
			for(x=0;x<bmpSurf->w;x++) {
				pixel = GetPixel(bmpFont,x,y);
				SDL_GetRGB(pixel,bmpSurf->format,&R,&G,&B);
				alpha = bmpSurf->format->Amask-(pixel & bmpSurf->format->Amask);

				if(!(R+G+B))
					PutPixel(bmpSurf,x,y,colour - alpha);
			}
		}
	}


	// Unlock the surface
	if(SDL_MUSTLOCK(bmpSurf))
		SDL_UnlockSurface(bmpSurf);
}


////////////////////
// Get height of multiline text
int CFont::GetHeight(const std::string& buf)
{
	int numlines=1;
	for (std::string::const_iterator i=buf.begin();i!=buf.end();i++)
		if (*i == '\n') numlines++;
	return numlines*bmpFont->h;
}

///////////////////
// Draw a font (advanced)
void CFont::DrawAdv(SDL_Surface *dst, int x, int y, int max_w, Uint32 col, const std::string& txt) {
	int pos=0; // Offset, x+pos is current character position
	short l;
	Uint32 pixel;
	int i,j;
	int w;
	int a,b; // a = offset in bmpFont
	static const Uint32 black = SDL_MapRGBA(bmpFont->format,0,0,0,SDL_ALPHA_OPAQUE);
	static const Uint32 white = SDL_MapRGBA(bmpFont->format,255,255,255,SDL_ALPHA_OPAQUE);
	static const Uint32 pink = SDL_MapRGBA(bmpFont->format,255,0,255,SDL_ALPHA_OPAQUE);

	// Clipping rectangle
	SDL_Rect oldrect = dst->clip_rect;
	SDL_Rect newrect = dst->clip_rect;

	int	top = oldrect.y;
	int left = oldrect.x;
	int right = MIN(oldrect.x + oldrect.w,x+max_w);
	int bottom = oldrect.y + oldrect.h;

	// Set the newrect width and use this newrect temporarily to draw the font
	// We use this rect because of precached fonts which use SDL_Blit for drawing (and it takes care of cliprect)
	newrect.w = right-left;
	SDL_SetClipRect(dst,&newrect);


	// Look in the precached fonts if there's some for this color
	SDL_Surface *bmpCached = NULL;
	if (Colorize)  {
		Uint32 col2 = col | bmpFont->format->Amask;
		if (col2 == f_white)  
			bmpCached = bmpWhite;
		else if (col2 == black)
			bmpCached = bmpFont;
		else if (col2 == f_green)
			bmpCached = bmpGreen;
	}
	// Not colourize, bmpFont itself should be blitted without any changes, so it's precached
	else  {
		bmpCached = bmpFont;
	}

	// Lock the surfaces
	if(SDL_MUSTLOCK(dst))
		SDL_LockSurface(dst);
	if(SDL_MUSTLOCK(bmpFont) && !bmpCached)  // If we use cached font, we do not access the pixels
		SDL_LockSurface(bmpFont);

	Uint8 R,G,B,A;
	short clip_x,clip_y,clip_w,clip_h;

	pos=0;
	for(std::string::const_iterator p = txt.begin(); p != txt.end(); ) {
		// Line break
		if (*p == '\n')  {
			y += bmpFont->h+VSpacing;
			pos = 0;
			p++;
			continue;
		}
		
		// Translate and ignore unknown
		l = TranslateCharacter(p, txt.end());
		if (l == -1)
			continue;

		w=0;
		a=CharacterOffset[l];

		// Precached fonts
		if (bmpCached)  {
			DrawImageAdv(dst,bmpCached,a,0,x+pos,y,FontWidth[l],bmpFont->h);
			pos+=FontWidth[l]+Spacing;
			continue;
		}

		// Calculate clipping
		clip_x = MAX(left-x-pos,0);
		clip_y = MAX(top-y,0);
		clip_w = FontWidth[l];
		if (x+pos+clip_w >= right)
			clip_w = right-x-pos;
		clip_h = bmpFont->h;
		if (y+clip_h >= bottom)
			clip_h = bottom-y;

		register Uint8 *src = (Uint8 *)bmpFont->pixels + a * bmpFont->format->BytesPerPixel;
		register Uint8 *p;
		register byte bpp = bmpFont->format->BytesPerPixel;

		// Outline font
		if (OutlineFont)  {
			for(j=clip_y;j<clip_h;j++) {
				p = src;
				for(i=a+clip_x,b=clip_x;b<clip_w;i++,b++,p+=bpp) {

					pixel = GetPixelFromAddr(p,bpp);
					SDL_GetRGBA(pixel,bmpFont->format,&R,&G,&B,&A);

					// Put black pixels and colorize white ones
					if (!(byte)(~R+~G+~B))  // White
						PutPixelA(dst,x+pos+b,y+j,col,A); // Put the pixel and blend it with background
					else if (!(R+G+B))  // Black
						PutPixelA(dst,x+pos+b,y+j,0,A);
				}
				src+= bmpFont->pitch;
			}
		}
		// Not outline
		else {
			for(j=clip_y;j<clip_h;j++) {
				p = src;
				for(i=a+clip_x,b=clip_x;b<clip_w;i++,b++,p+=bpp) {

					pixel = GetPixelFromAddr(p,bpp);
					SDL_GetRGBA(pixel,bmpFont->format,&R,&G,&B,&A);

					// Put only black pixels
					if (!(R+G+B))  
						PutPixelA(dst,x+pos+b,y+j,col,A);
				}
				src+= bmpFont->pitch;
			}
		}

		pos+=FontWidth[l]+Spacing;
	}

	// Restore the original clipping rect
	SDL_SetClipRect(dst,&oldrect);


	// Unlock the surfaces
	if(SDL_MUSTLOCK(dst))
		SDL_UnlockSurface(dst);
	if(SDL_MUSTLOCK(bmpFont))
		SDL_UnlockSurface(bmpFont);

}


///////////////////
// Calculate the width of a string of text
int CFont::GetWidth(const std::string& buf) {
	int length = 0;
	short l;
	
	// Calculate the length of the text
	for(std::string::const_iterator p = buf.begin(); p != buf.end(); ) {
		l = TranslateCharacter(p, buf.end());
		if (l != -1)
			length += FontWidth[l]+Spacing;
	}

	return length;
}

///////////////////
// Draws the text in centre alignment
void CFont::DrawCentre(SDL_Surface *dst, int x, int y, Uint32 col, const std::string& txt) {
	int length = GetWidth(txt);
	int pos = x-length/2;
	Draw(dst,pos,y,col,txt);
}

///////////////////
// Draw's the text in centre alignment
void CFont::DrawCentreAdv(SDL_Surface *dst, int x, int y, int min_x, int max_w, Uint32 col, const std::string& txt) {
	int length = GetWidth(txt);
	int pos = MAX(min_x, x-length/2);
	DrawAdv(dst,pos,y,max_w,col,txt);
}
