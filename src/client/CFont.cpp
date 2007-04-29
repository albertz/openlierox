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


//char Fontstr[256] = {" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_'abcdefghijklmnopqrstuvwxyz{|}~ \161\162\163\164\165\166\167\168\169\170\171\172\173\174\175\176\177\178\179\180\181\182\183\184\185\186\187\188\189\190\191\192\193\194\195\196\197\198\199\200\201\202\203\204\205\206\207\208\209\210\211\212\213\214\215\216\217\218\219\220\221\222\223\224\225\226\227\228\229\230\231\232\233\234\235\236\237\238\239\240\241\242\243\244\245\246\247\248\249\250\251\252\253\254\255"};
ushort Fontstr[NUM_CHARACTERS];
size_t Fontstr_len = sizeof(Fontstr)/sizeof(short);

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
	register short i;
	for (i=0; i<128; i++)
		Fontstr[i] = i+32;
	for (i=128; i<sizeof(Fontstr)/sizeof(short); i++)
		Fontstr[i] = i+65;

	LOAD_IMAGE(bmpFont,fontname);

	Colour = _colour;

	if(!bmpFont) return false;

	//AdjustFont(bmpFont,fontname+"_adj.bmp");

	bmpWhite = gfxCreateSurface(bmpFont->w,bmpFont->h);
	bmpGreen = gfxCreateSurface(bmpFont->w,bmpFont->h);

	// Calculate the font width for each character
	CalculateWidth();

	PreCalculate(bmpWhite,ConvertColor(tLX->clNormalLabel,SDL_GetVideoSurface()->format,bmpWhite->format));
	PreCalculate(bmpGreen,ConvertColor(tLX->clChatText,SDL_GetVideoSurface()->format,bmpGreen->format));


		// Pre-calculate some colours
	f_pink = ConvertColor(tLX->clPink,SDL_GetVideoSurface()->format,bmpFont->format);
	f_blue = ConvertColor(tLX->clHeading,SDL_GetVideoSurface()->format,bmpFont->format);//SDL_MapRGB(bmpFont->format,0,0,255);
	f_white = ConvertColor(tLX->clNormalLabel,SDL_GetVideoSurface()->format,bmpFont->format);//MakeColour(255,255,255);
	f_green = ConvertColor(tLX->clChatText,SDL_GetVideoSurface()->format,bmpFont->format);//MakeColour(0,255,0);

	// Must do this after PreCalculate
	SDL_SetColorKey(bmpFont, SDL_SRCCOLORKEY, f_pink);

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
	static const Uint32 pink = SDL_MapRGB(bmpFont->format,255,0,255);
	for (ushort i=0; i<bmpFont->h; i++)
		if (GetPixel(bmpFont,x,i) != pink)
			return false;
	return true;
}

///////////////////
// Calculate Widths
void CFont::CalculateWidth(void)
{
	int x,n;
	int cur_w;

	// Lock the surface
	if(SDL_MUSTLOCK(bmpFont))
		SDL_LockSurface(bmpFont);

	static const Uint32 blue = SDL_MapRGB(bmpFont->format,0,0,255);

	n=0;
	cur_w = 0;
	for (x=0; x<bmpFont->w; x++)  {
		if (Fontstr[n] != ' ')
			while (IsColumnFree(x) && x<bmpFont->w)  // Ignore any free pixel columns before the character (but don't do this for spaces)
				x++;

		// Read until a blue pixel or end of the image
		while (GetPixel(bmpFont,x,0) != blue && x<bmpFont->w)  {
			x++;
			cur_w++;
		}

		// Blue pixel means end of the character
		FontWidth[n] = cur_w;
		CharacterOffset[n] = x-cur_w;
		n++;
		if (n == Fontstr_len)  // Safety
			break;
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

	DrawRectFill(bmpSurf,0,0,bmpSurf->w,bmpSurf->h,tLX->clPink);
	SDL_BlitSurface(bmpFont,NULL,bmpSurf,NULL);


	// Replace black with the appropriate colour
	if(SDL_MUSTLOCK(bmpSurf))
		SDL_LockSurface(bmpSurf);

	for(y=0;y<bmpSurf->h;y++) {
		for(x=0;x<bmpSurf->w;x++) {
			pixel = GetPixel(bmpSurf,x,y);

			if(pixel == 0)
				PutPixel(bmpSurf,x,y,colour);
		}
	}


	// Unlock the surface
	if(SDL_MUSTLOCK(bmpSurf))
		SDL_UnlockSurface(bmpSurf);

	SDL_SetColorKey(bmpSurf, SDL_SRCCOLORKEY, tLX->clPink);
}


///////////////////
// Set to true, if this is an outline font
void CFont::SetOutline(int Outline) {
	OutlineFont = Outline;
}

///////////////////
// Is this an outline font?
int CFont::IsOutline(void) {
	return OutlineFont;
}

//////////////////
// Draw a text
void CFont::Draw(SDL_Surface *dst, int x, int y, Uint32 col, char *fmt,...) {
	va_list arg;

	va_start(arg, fmt);
	static char buf[512];
	vsnprintf(buf, sizeof(buf),fmt, arg);
	fix_markend(buf);
	va_end(arg);

	DrawAdv(dst,x,y,99999,col,std::string(buf));
}

void CFont::Draw(SDL_Surface *dst, int x, int y, Uint32 col, const std::string& txt) {
	DrawAdv(dst,x,y,99999,col,txt);
}


void CFont::DrawAdv(SDL_Surface *dst, int x, int y, int max_w, Uint32 col, char *fmt,...) {
	va_list arg;
	va_start(arg, fmt);
	static char buf[512];
	vsnprintf(buf, sizeof(buf),fmt, arg);
	fix_markend(buf);
	va_end(arg);
	
	DrawAdv(dst, x, y, max_w, col, std::string(buf));
}

///////////////////
// Draw a font (advanced)
void CFont::DrawAdv(SDL_Surface *dst, int x, int y, int max_w, Uint32 col, const std::string& txt) {
	int pos=0;
	short l;
	Uint32 pixel;
	int i,j;
	int w;
	int a,b;
	int length = Fontstr_len;

	// Clipping rectangle
	SDL_Rect rect = dst->clip_rect;

	int	top = rect.y;
	int left = rect.x;
	int right = rect.x + rect.w;
	int bottom = rect.y + rect.h;


	// Lock the surfaces
	if(SDL_MUSTLOCK(dst))
		SDL_LockSurface(dst);
	if(SDL_MUSTLOCK(bmpFont))
		SDL_LockSurface(bmpFont);


	Uint32 col2 = col;

	pos=0;
	for(std::string::const_iterator p = txt.begin(); p != txt.end(); p++) {
		l = TranslateCharacter(*p);

		// Line break
		if (*p == '\n')  {
			y += bmpFont->h+3;
			pos = 0;
			continue;
		}

		// Maximal width overflowed
		// TODO: doesn't support multiline texts, but it's faster...
		if (pos > max_w)
			break;

        // Ignore unkown characters
        if(l >= length || l < 0)
            continue;

		w=0;
		a=CharacterOffset[l];

		if(!Colour) {
			SDL_SetColorKey(bmpFont, SDL_SRCCOLORKEY, tLX->clPink);
			DrawImageAdv(dst,bmpFont,a,0,x+pos,y,FontWidth[l],bmpFont->h);
			pos+=FontWidth[l]+Spacing;
			continue;
		}

		if (!OutlineFont)  {
			if (col2 == f_white)  {
				DrawImageAdv(dst,bmpWhite,a,0,x+pos,y,FontWidth[l],bmpFont->h);
				pos+=FontWidth[l]+Spacing;
				continue;
			} else if (!col2) {
				DrawImageAdv(dst,bmpFont,a,0,x+pos,y,FontWidth[l],bmpFont->h);
				pos+=FontWidth[l]+Spacing;
				continue;
			} else if( col2 == f_green) {
				DrawImageAdv(dst,bmpGreen,a,0,x+pos,y,FontWidth[l],bmpFont->h);
				pos+=FontWidth[l]+Spacing;
				continue;
			}
		}

		/*if(!Colour) {
			SDL_SetColorKey(bmpFont, SDL_SRCCOLORKEY, tLX->clPink);
			DrawImageAdv(dst,bmpFont,a,0,x+pos,y,FontWidth[l],bmpFont->h);
		}
		else */{
			register Uint8 *src = (Uint8 *)bmpFont->pixels + a * bmpFont->format->BytesPerPixel;
			register Uint8 *p;
			register byte bpp = bmpFont->format->BytesPerPixel;
			for(j=0;j<bmpFont->h;j++) {
				p = src;
				for(i=a,b=0;b<FontWidth[l];i++,b++,p+=bpp) {

					// Clipping
					// TODO: calculate the clipping before entering loops, it will speed this up
					if(x+pos+b < left)
						continue;
					if(y+j < top)
						break;

					if(y+j >= bottom)
						break;
					if(x+pos+b >= right)
						break;

					pixel = GetPixelFromAddr(p,bpp);


					if(OutlineFont)  {
						if (pixel == tLX->clWhite)
							PutPixel(dst,x+pos+b,y+j,col);
						else if (!pixel)
							PutPixel(dst,x+pos+b,y+j,0);
					} else if (!pixel)  // Put only black pixels
						PutPixel(dst,x+pos+b,y+j,col);
				}
				src+= bmpFont->pitch;
			}
		}

		pos+=FontWidth[l]+Spacing;
	}


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
	for(std::string::const_iterator p = buf.begin(); p != buf.end(); p++) {
		l = TranslateCharacter(*p);
		if (l != -1)
			length += FontWidth[l]+Spacing;
	}

	return length;
}

/////////////////////
// Translates the character to the position in Fontstr array, returns -1 if impossible
int CFont::TranslateCharacter(char c)
{
	if (c < 32)  {
#ifdef WIN32
		static char charbuf[2];
		static ushort utfbuf[2];
		charbuf[0] = c;
		charbuf[1] = '\0';
		::MultiByteToWideChar(CP_ACP, 0, charbuf, 1, utfbuf, 1);
		if (utfbuf[0] > NUM_CHARACTERS+65)
			return -1;
		else
			return utfbuf[0]-65;
#else  // LINUX
		return (uchar)c-32; // TODO: does it work?
#endif
	} else
		return c-32;
}


void CFont::DrawCentre(SDL_Surface *dst, int x, int y, Uint32 col, const std::string& txt) {
	int length = GetWidth(txt);
	int pos = x-length/2;
	Draw(dst,pos,y,col,txt);
}

///////////////////
// Draws the text in centre alignment
void CFont::DrawCentre(SDL_Surface *dst, int x, int y, Uint32 col, char *fmt, ...) {
	va_list arg;
	va_start(arg, fmt);
	static char buf[512];
	vsnprintf(buf, sizeof(buf),fmt, arg);
	fix_markend(buf);
	va_end(arg);

	DrawCentre(dst, x,y,col, std::string(buf));
}


void CFont::DrawCentreAdv(SDL_Surface *dst, int x, int y, int min_x, int max_w, Uint32 col, const std::string& txt) {
	int length = GetWidth(txt);
	int pos = MAX(min_x, x-length/2);
	DrawAdv(dst,pos,y,max_w,col,txt);
}

///////////////////
// Draw's the text in centre alignment
void CFont::DrawCentreAdv(SDL_Surface *dst, int x, int y, int min_x, int max_w, Uint32 col, char *fmt, ...) {
	va_list arg;
	va_start(arg, fmt);
	static char buf[512];
	vsnprintf(buf, sizeof(buf),fmt, arg);
	fix_markend(buf);
	va_end(arg);

	DrawCentreAdv(dst, x, y, min_x, max_w, col, std::string(buf));
}
