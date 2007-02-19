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


char Fontstr[256] = {" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_'abcdefghijklmnopqrstuvwxyz{|}~~"};//\161\162\163\164\165\166\167\168\169\170\171\172\173\174\175\176\177\178\179\180\181\182\183\184\185\186\187\188\189\190\191\192\193\194\195\196\197\198\199\200\201\202\203\204\205\206\207\208\209\210\211\212\213\214\215\216\217\218\219\220\221\222\223\224\225\226\227\228\229\230\231\232\233\234\235\236\237\238\239\240\241\242\243\244\245\246\247\248\249\250\251\252\253\254\255"};
int Fontstr_len = strlen(Fontstr);


///////////////////
// Load a font
int CFont::Load(const std::string& fontname, int _colour, int _width)
{
	LOAD_IMAGE(bmpFont,fontname);
	
	Colour = _colour;
	Width = _width;

	bmpWhite = gfxCreateSurface(bmpFont->w,bmpFont->h);
	bmpGreen = gfxCreateSurface(bmpFont->w,bmpFont->h);

	// Calculate the font width for each character
	CalculateWidth();

	PreCalculate(bmpWhite,tLX->clNormalLabel);
	PreCalculate(bmpGreen,tLX->clChatText);


	// Must do this after
	SDL_SetColorKey(bmpFont, SDL_SRCCOLORKEY, SDL_MapRGB(bmpFont->format,255,0,255));

		// Pre-calculate some colours
	f_pink = SDL_MapRGB(bmpFont->format,255,0,255);
	f_blue = tLX->clHeading;//SDL_MapRGB(bmpFont->format,0,0,255);
	f_white = tLX->clNormalLabel;//MakeColour(255,255,255);
	f_green = tLX->clChatText;//MakeColour(0,255,0);

	return true;
}


///////////////////
// Shutdown the font
void CFont::Shutdown(void)
{
	if(bmpWhite)
		SDL_FreeSurface(bmpWhite);
	if(bmpGreen)
		SDL_FreeSurface(bmpGreen);
}


///////////////////
// Calculate Widths
void CFont::CalculateWidth(void)
{
	unsigned int n;
	Uint32 pixel;
	int i,j;
	int a,b;

	// Lock the surface
	if(SDL_MUSTLOCK(bmpFont))
		SDL_LockSurface(bmpFont);

	Uint32 blue = SDL_MapRGB(bmpFont->format,0,0,255);
	
	for(n=0;n<Fontstr_len;n++) {
		a=n*Width;
		for(j=0;j<bmpFont->h;j++) {
			for(i=a,b=0;b<Width;i++,b++) {

				pixel = GetPixel(bmpFont,i,j);
				if(pixel == blue) {
					FontWidth[n] = b;
					break;
				}
			}
		}
	}


	// Unlock the surface
	if(SDL_MUSTLOCK(bmpFont))
		SDL_UnlockSurface(bmpFont);
}


///////////////////
// Precalculate a font's colour
void CFont::PreCalculate(SDL_Surface *bmpSurf, Uint32 colour)
{
	Uint32 pixel;
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

	SDL_SetColorKey(bmpSurf, SDL_SRCCOLORKEY, SDL_MapRGB(bmpSurf->format,255,0,255));
}


///////////////////
// Set to true, if this is an outline font
void CFont::SetOutline(int Outline)
{
	OutlineFont = Outline;
}

///////////////////
// Is this an outline font?
int CFont::IsOutline(void)
{
	return OutlineFont;
}

//////////////////
// Draw a text
void CFont::Draw(SDL_Surface *dst, int x, int y, int col, char *fmt,...)
{
	static char buf[512];
	va_list arg;

	va_start(arg, fmt);
	vsnprintf(buf, sizeof(buf),fmt, arg);
	fix_markend(buf);
	va_end(arg);

	DrawAdv(dst,x,y,99999,col,"%s",buf);
}

///////////////////
// Draw a font (advanced)
void CFont::DrawAdv(SDL_Surface *dst, int x, int y, int max_w, int col, char *fmt,...)
{
	static char buf[512];
	va_list arg;
	int pos=0;
	int n,l;
	Uint32 pixel;
	int i,j;
	int w;
	int a,b;
	int length = fix_strnlen(Fontstr);

	va_start(arg, fmt);
	vsnprintf(buf, sizeof(buf),fmt, arg);
	fix_markend(buf);
	va_end(arg);

	int txtlen = fix_strnlen(buf);


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

	
	Uint32 col2 = (Uint32)col;

	pos=0;
	for(n=0;n<txtlen;n++) {

		l = buf[n]-32;

		// Line break
		if (buf[n] == '\n')  {
			y += bmpFont->h+3;
			pos = 0;
			continue;
		}

		// Maximal width overflowed
		// TODO: doesn't support multiline texts, but it's faster...
		if (pos > max_w)
			break;

        // Ignore unkown characters
        if(l >= length-1 || l < 0 )
            continue;
		
		w=0;
		a=l*Width;

		if(!Colour) {
			SDL_SetColorKey(bmpFont, SDL_SRCCOLORKEY, SDL_MapRGB(bmpFont->format,255,0,255));
			DrawImageAdv(dst,bmpFont,a,0,x+pos,y,FontWidth[l],bmpFont->h);
			pos+=FontWidth[l];
			continue;
		}

		if (!OutlineFont)  {
			if (col2 == f_white)  {
				DrawImageAdv(dst,bmpWhite,a,0,x+pos,y,FontWidth[l],bmpFont->h);
				pos+=FontWidth[l];
				continue;
			} else if (!col2) {
				DrawImageAdv(dst,bmpFont,a,0,x+pos,y,FontWidth[l],bmpFont->h);
				pos+=FontWidth[l];
				continue;
			} else if( col2 == f_green) {
				DrawImageAdv(dst,bmpGreen,a,0,x+pos,y,FontWidth[l],bmpFont->h);
				pos+=FontWidth[l];
				continue;
			}
		}

		/*if(!Colour) {
			SDL_SetColorKey(bmpFont, SDL_SRCCOLORKEY, SDL_MapRGB(bmpFont->format,255,0,255));
			DrawImageAdv(dst,bmpFont,a,0,x+pos,y,FontWidth[l],bmpFont->h);
		}
		else */{
			Uint8 *src = (Uint8 *)bmpFont->pixels + a * bmpFont->format->BytesPerPixel;
			Uint8 *p;
			int bpp = bmpFont->format->BytesPerPixel;
			for(j=0;j<bmpFont->h;j++) {
				p = src;
				for(i=a,b=0;b<FontWidth[l];i++,b++,p+=bpp) {

					// Clipping
					if(x+pos+b < left)
						continue;
					if(y+j < top)
						break;

					if(y+j >= bottom)
						break;
					if(x+pos+b >= right)
						break;
						
					pixel = GetPixelFromAddr(p,bpp);

					if(pixel == f_pink)
						continue;

					if(pixel == 2 && OutlineFont)  {
							PutPixel(dst,x+pos+b,y+j,0);
							continue;
					}


					if(Colour)
						PutPixel(dst,x+pos+b,y+j,col);
				}
				src+= bmpFont->pitch;
			}
		}

		pos+=FontWidth[l];
	}


	// Unlock the surfaces
	if(SDL_MUSTLOCK(dst))
		SDL_UnlockSurface(dst);
	if(SDL_MUSTLOCK(bmpFont))
		SDL_UnlockSurface(bmpFont);

}


///////////////////
// Calculate the width of a string of text
int CFont::GetWidth(const std::string& buf)
{
	unsigned int n,l;
	int length = 0;

	// Calculate the length of the text
	for(n=0;n<buf.length();n++) {
        l = buf[n]-32;
        if( l >= Fontstr_len || l < 0 )
            continue;

		length += FontWidth[l];
	}
	
	return length;
}


///////////////////
// Draws the text in centre alignment
void CFont::DrawCentre(SDL_Surface *dst, int x, int y, int col, char *fmt, ...)
{
	static char buf[512];
	va_list arg;
	int pos;
	int length=0;
	unsigned int n,l;
	
	va_start(arg, fmt);
	vsnprintf(buf, sizeof(buf),fmt, arg);
	fix_markend(buf);
	va_end(arg);

	// Calculate the length of the text
	size_t buflen = fix_strnlen(buf);
	size_t fontstrlen = Fontstr_len;
	for(n=0;n<buflen;n++) {
		/*for(l=0;l<Fontstr_len;l++) {
			if(Fontstr[l] == buf[n])
				break;
		}*/

		l = buf[n]-32;
		if(l>=fontstrlen)
			continue;

		length+=FontWidth[l];
	}

	pos = x-length/2;

	Draw(dst,pos,y,col,"%s",buf);
}

///////////////////
// Draw's the text in centre alignment
void CFont::DrawCentreAdv(SDL_Surface *dst, int x, int y, int min_x, int max_w, int col, char *fmt, ...)
{
	static char buf[512];
	va_list arg;
	int pos;
	int length=0;
	unsigned int n,l;
	
	va_start(arg, fmt);
	vsnprintf(buf, sizeof(buf),fmt, arg);
	fix_markend(buf);
	va_end(arg);

	// Calculate the length of the text
	size_t buflen = fix_strnlen(buf);
	size_t fontstrlen = Fontstr_len;
	for(n=0;n<buflen;n++) {
		/*for(l=0;l<Fontstr_len;l++) {
			if(Fontstr[l] == buf[n])
				break;
		}*/

		l = buf[n]-32;
		if(l>=fontstrlen)
			continue;

		length+=FontWidth[l];
	}

	pos = MAX(min_x,x-length/2);

	DrawAdv(dst,pos,y,max_w,col,"%s",buf);
}
