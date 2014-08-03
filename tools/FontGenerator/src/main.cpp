/*
	OpenLieroX - FontGenerator
	
	by Dark Charlie, Albert Zeyer
	Code under LGPL
	( 15-05-2007 )
*/

#include <iostream>
#include <SDL_ttf.h>
#include <gd.h>

#include "StringUtils.h"
#include "Utils.h"

#include "FontGenerator.h"

#ifndef MAX
#define MAX(x,y) ( (x) > (y) ? (x) : (y) )
#endif

SDL_Surface* Screen = NULL;

using namespace std;

ostream& Output = cout;

// Main entry point
int main(int argc, char *argv[])
{
	//
	// Initialization
	//

	Output << "Welcome to Font Generator!" << endl;
	
	// Initialize SDL
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_NOPARACHUTE) == -1) {
		Output << "Could not initialize the SDL library. Quitting." << endl;
		return -1;
	}

	// Initialize SDL_ttf
	if (TTF_Init() == -1)  {
		SDL_Quit();
		Output << "Could not initialize SDL_ttf library. Quitting." << endl;
		return -1;
	}

	//
	// Parse the arguments
	//

	if (argc < 2)  { // Not enough of parameters
		DisplayHelp(argv[0]);
		SDL_Delay(1000);
		Quit();
		return -1;
	}

	arguments_t Arguments = ParseArguments(argc,argv);

	//
	// Create the font
	//
	TTF_Font *Font = TTF_OpenFont(Arguments.InputFile.c_str(),Arguments.Size);
	if (!Font)  {
		Output << "Could not open the font!" << endl;
		Quit();
		return -1;
	}

	Output << "Font " << Arguments.InputFile << " successfully loaded!" << endl;

	if (Arguments.Bold)
		TTF_SetFontStyle(Font,TTF_STYLE_BOLD);
	if (Arguments.Italic)
		TTF_SetFontStyle(Font,TTF_STYLE_ITALIC);
	// TODO: remove underline at all, it makes no sense
	//if (Arguments.Underline)
	//	TTF_SetFontStyle(Font,TTF_STYLE_UNDERLINE);

	//
	// Prepare for rendering
	//
	Output << "Preparing for compilation" << endl;
	if( Arguments.LastChar > 65535 )
		Arguments.LastChar = 65535;
		
	Uint16 * Characters = new Uint16[(Arguments.LastChar-FIRST_CHARACTER)+1];

	for (size_t c=0;c<(Arguments.LastChar-FIRST_CHARACTER);c++)  {
		Characters[c] = c+FIRST_CHARACTER;
	}
	Characters[(Arguments.LastChar-FIRST_CHARACTER)] = 0; // Terminating


	//
	//  Render the font
	//

	Output << "Rendering the font." << endl;

	SDL_Surface *OutBmp = RenderText(Font,Characters,(Arguments.LastChar-FIRST_CHARACTER),Arguments.Outline,Arguments.Antialiased);
	if (!OutBmp)  {
		TTF_CloseFont(Font);
		Quit();
		return -1;
	}

	//
	// Save the font
	//

	Output << "Saving in " << Arguments.OutputFile << endl;
	if (!SavePNG(OutBmp,Arguments.OutputFile,Arguments.Antialiased))  
		Output << "Could not save the resulting bitmap." << endl;
	//SDL_SaveBMP(OutBmp,(Arguments.OutputFile+".bmp").c_str());

	//
	// Quit
	//
	delete [] Characters;
	Output << "Successfully finished! :)" << endl;
	SDL_FreeSurface(OutBmp);
	TTF_CloseFont(Font);
	Quit();

	return 0;
}

// Render the text
SDL_Surface *RenderText(TTF_Font *Font, Uint16 *Text, size_t TextLen, bool Outline, bool Antialiased)
{
	//
	// Variables
	//

	const SDL_Color Black = {0,0,0};
	const SDL_Color White = {255,255,255};
	SDL_Color Color = Outline ? White : Black;
	
	SDL_Surface *Result = NULL;
	int SurfaceWidth,SurfaceHeight;
	SDL_Surface *Glyph = NULL;
	SDL_Rect Rect; Rect.x = 0; Rect.y=0;
	int Descent = 0;
	int YOffset = TTF_FontAscent(Font);  // Actually a baseline

	// Glyph metrics
	int minx,maxy,advance;

	//
	// Count the font descent
	//

	// NOTE: sometimes we need to count the descent value by ourself, because there's some bug in SDL_ttf
	// which returns font descent 0 for pcf fonts
	if (!(Descent = TTF_FontDescent(Font)))  {
		Glyph = TTF_RenderGlyph_Solid(Font,'g',Color); // Render the g character, which has a descent
		if (!Glyph)  { // Weird...
			Output << "Cannot render the glyph for descent counting." << endl;
			return NULL;
		}

		// Get the metrics of the g character
		if (TTF_GlyphMetrics(Font,'g',NULL,NULL,NULL,&maxy,NULL) == -1)  {  // Even more weird...
			Output << "Cannot get the glyph metrics for descent counting" << endl;
			SDL_FreeSurface(Glyph);
			return NULL;
		}

		Descent = TTF_FontAscent(Font)-maxy+Glyph->h-TTF_FontHeight(Font);  // Count the descent
		SDL_FreeSurface(Glyph);  // Free the g glyph
	}

	//
	// Prepare the final surface
	//

	// Get the surface dimensions
	/*if (TTF_SizeUNICODE(Font,Text,&SurfaceWidth,&SurfaceHeight) == -1)  {
		Output << "Cannot get the font size!" << endl;
		return NULL;
	}*/
	SurfaceWidth = 0;
	for (size_t i=0;i<TextLen;i++)  {
		if (TTF_GlyphMetrics(Font,Text[i],&minx,NULL,NULL,&maxy,&advance) == -1)
			continue;
		SurfaceWidth += advance + 2;  // Space for separators and "what if" space
		if (minx < 0)
			SurfaceWidth -= minx;
	}

	SurfaceHeight = TTF_FontHeight(Font); // Height

	if (!TTF_FontDescent(Font))  {  // SDL_ttf bug, see the comment above
		// This means the SurfaceHeight is also wrong, so we fix it here
		SurfaceHeight = Descent+TTF_FontAscent(Font);
	}

	// Make space for outline if needed
	if (Outline)  {
		SurfaceWidth += 2*TextLen;  // Outline left and right
		SurfaceHeight += 2; // Outline top and bottom
		YOffset++; // First pixel line reserved
	}

	// Create the surface
	// NOTE: MUST be 32 bit, SDLSurface2GDSurface requires it!!!
	Result = SDL_CreateRGBSurface(0,SurfaceWidth,SurfaceHeight,32,RMASK,GMASK,BMASK,AMASK);
	if (!Result)  {
		Output << "Out of memory while creating the bitmap surface." << endl;
		return NULL;
	}

	if (Antialiased)   {
		SDL_FillRect(Result,NULL,SDL_MapRGBA(Result->format,255,0,255,SDL_ALPHA_TRANSPARENT));  // Set the whole surface to transparent
	}
	else
		SDL_FillRect(Result,NULL,SDL_MapRGB(Result->format,255,0,255));  // Fill with pink (=transparent)


	//
	// Render the text, glyph by glyph
	//
	const color_t Blue = SDL_MapRGBA(Result->format, 0, 0, 255, SDL_ALPHA_OPAQUE);

	for(size_t i = 0; i < TextLen; i++)  {
		// Get the glyph metrics
		if (TTF_GlyphMetrics(Font,Text[i],&minx,NULL,NULL,&maxy,&advance) == -1)
			continue;

		// Blit the glyph
		if (Antialiased)
			Glyph = TTF_RenderGlyph_Blended(Font,Text[i],Color);
		else
			Glyph = TTF_RenderGlyph_Solid(Font,Text[i],Color);
		if (!Glyph)
			continue;
		Rect.y = YOffset-maxy;
		if (Antialiased)
			BlitAlpha(Result,Glyph,Rect.x,Rect.y);
		else
			SDL_BlitSurface(Glyph,NULL,Result,&Rect);
		Rect.x += advance+Outline+1;  // Outline is either 0 (disabled) or 1 (enabled), +1 is "what if" space
		if (minx < 0)
			Rect.x -= minx;
		SDL_FreeSurface(Glyph);

		// Dividing line
		DrawVLine(Result,Rect.x,Blue);
		Rect.x++;
		Rect.x += Outline;
	}

	// Apply the outline if needed
	if (Outline)
		ApplyOutline(Result);

	return Result;
}

// Applies an outline to the text
// Surface must have following parameters:
// * Pink is a transparent color
// * White is a font color
// * 32 bit color depth
// * Must not be NULL
void ApplyOutline(SDL_Surface *BitmapText)
{
	//const color_t Transparent = SDL_MapRGB(BitmapText->format,255,0,255);
	// TODO: not used
//	const color_t FontColor = SDL_MapRGB(BitmapText->format,255,255,255);
	const color_t OutlineColor = SDL_MapRGB(BitmapText->format,0,0,0);

	unsigned short pitchstep = BitmapText->pitch/sizeof(color_t);
	Uint32 *pxrow = (Uint32 *)BitmapText->pixels+pitchstep;
	Uint32 *px;
	Uint32 *TempPx;
	Uint8 r,g,b;
	int x,y;

	for (y=1;y<BitmapText->h-1;y++,pxrow+=pitchstep)  {
		px = pxrow+1;
		for (x=1;x<BitmapText->w-1;x++,px++)  {
			SDL_GetRGB(*px,BitmapText->format,&r,&g,&b);
			if (!(byte)(~r+~g+~b))  {  // White
				// Left pixel
				TempPx = px-1; SDL_GetRGB(*TempPx,BitmapText->format,&r,&g,&b);
				if (!(byte)(~r+g+~b)) *TempPx = OutlineColor;  // ~r+g+b~ = 0 is valid only for pink

				// Top pixel
				TempPx = px-pitchstep; SDL_GetRGB(*TempPx,BitmapText->format,&r,&g,&b);
				if (!(byte)(~r+g+~b)) *TempPx = OutlineColor;

				// Right pixel
				TempPx = px+1; SDL_GetRGB(*TempPx,BitmapText->format,&r,&g,&b);
				if (!(byte)(~r+g+~b)) *TempPx = OutlineColor;

				// Bottom pixel
				TempPx = px+pitchstep; SDL_GetRGB(*TempPx,BitmapText->format,&r,&g,&b);
				if (!(byte)(~r+g+~b)) *TempPx = OutlineColor;
			}
		}
	}
}

// Displays the help
void DisplayHelp(const std::string& ExeName)
{
	Output << "Usage:" << endl
	<< ExeName << " " << "<input file> [<output file>] [-bold] [-italic] [-underline] [-outline] [-antialiasing] [-size:number] [-amount:number]" << endl;

#ifdef WIN32
	Output << endl << endl;
	system("pause");
#endif
}

// Parse the program arguments
arguments_t ParseArguments(int argc,char *argv[])
{
	arguments_t Result;
	Result.InputFile = "";
	Result.OutputFile = "";
	Result.Size = DEF_FONTSIZE;
	Result.LastChar = LAST_CHARACTER;
	Result.Bold = false;
	Result.Italic = false;
	Result.Underline = false;
	Result.Outline = false;
	Result.Antialiased = false;

	int i;
	for (i=1;i<argc;i++)  {
		// Input and output
		if (argv[i][0] != '-')  {
			if (Result.InputFile == "")
				Result.InputFile = argv[i];
			else if (Result.OutputFile == "")
				Result.OutputFile = argv[i];
			else
				Output << "Unknown argument: " << argv[i] << endl;
			continue;
		}

		// Bold
		if (!strcasecmp(argv[i],"-bold"))  {
			Result.Bold = true;
		// Italic
		} else if (!strcasecmp(argv[i],"-italic"))  {
			Result.Italic = true;
		// Underline
		} else if (!strcasecmp(argv[i],"-underline"))  {
			Result.Underline = true;
		// Outline
		} else if (!strcasecmp(argv[i],"-outline"))  {
			Result.Outline = true;
		// Size
		} else if (strstr(strlwr(argv[i]),"-size:"))  {
			Result.Size = atoi(argv[i]+strlen("size:"));
			if (Result.Size == 0)
				Result.Size = DEF_FONTSIZE;
		} else if (strstr(strlwr(argv[i]),"-amount:"))  {
			Result.LastChar = atoi(argv[i]+strlen("-amount:"));
			if (Result.LastChar == 0)
				Result.LastChar = LAST_CHARACTER;
		// Antialiasing
		} else if (strstr(strlwr(argv[i]),"-antialiasing"))  {
			Result.Antialiased = true;
		}
		// Unknown
		else  {
			Output << "Unknown argument: " << argv[i] <<endl;
		}
	}

	if (Result.InputFile.rfind('.') == std::string::npos)
		Result.InputFile += ".ttf";

	if (!FileExists(Result.InputFile))
		return Result;

	if (Result.OutputFile == "")
		if (Result.Outline)
			Result.OutputFile = "./"+Result.InputFile.substr(FindLastPathSep(Result.InputFile),Result.InputFile.rfind("."))+"_out"+".png";
		else
			Result.OutputFile = "./"+Result.InputFile.substr(FindLastPathSep(Result.InputFile),Result.InputFile.rfind("."))+".png";

	return Result;
}

// Blits one surface to another, keeping the alpha
void BlitAlpha(SDL_Surface *dst,SDL_Surface *src,int x, int y)
{
	// Clipping
	int clip_w = src->w;
	int clip_h = src->h;
	int off_x = 0;
	int off_y = 0;

	if (x+src->w > dst->w) { clip_w = dst->w-x; }
	if (x < 0)  { off_x = -x; }
	if (y+src->h > dst->h) { clip_h = dst->h-y; }
	if (y < 0) { off_y = -y; }

	Uint8 r,g,b,a;

	uchar *src_pxr = (uchar *)src->pixels + src->pitch*off_y + src->format->BytesPerPixel*off_x;
	uchar *src_px = NULL;
	uchar *dst_pxr = (uchar *)dst->pixels + dst->pitch*(y+off_y) + dst->format->BytesPerPixel*(x+off_x);
	uchar *dst_px = NULL;
	int i,j;
	for (j=off_y;j<clip_h;j++,src_pxr+=src->pitch,dst_pxr+=dst->pitch)  {
		src_px = src_pxr;
		dst_px = dst_pxr;
		for (i=0;i<src->w;i++,src_px+=src->format->BytesPerPixel,dst_px+=dst->format->BytesPerPixel)  {
			// HINT: folowing line requires a 32 bit surface
			SDL_GetRGBA(*(Uint32 *)src_px,src->format,&r,&g,&b,&a);
			if (a != SDL_ALPHA_TRANSPARENT)  { // ignore completely transparent pixels
				memcpy(dst_px,src_px,src->format->BytesPerPixel);
			}
		}
	}
}

// Closes the libraries
void Quit()
{
  TTF_Quit();
  SDL_Quit();
}

// Checks whether the file exists
bool FileExists(const std::string& str)
{
	FILE *fp = fopen(str.c_str(),"r");
	if (!fp)
		return false;
	fclose(fp);
	return true;
}

// Draw vertical line
void DrawVLine(SDL_Surface *bmpDest, int x, color_t color) {
	if (x >= bmpDest->w)
		x = bmpDest->w-1;

	// Y is 0, Y2 is bmpDest->h

	register unsigned short pitch = (unsigned short)bmpDest->pitch;
	register byte bpp = (byte)bmpDest->format->BytesPerPixel;
	register uchar *px2 = (uchar *)bmpDest->pixels+pitch*(bmpDest->h-1)+bpp*x;

	SDL_LockSurface(bmpDest);
	for (register uchar *px= (uchar *)bmpDest->pixels+bpp*x;px <= px2;px+=pitch)
		memcpy(px,&color,bpp);

	SDL_UnlockSurface(bmpDest);
}

// Finds the last path separator
size_t FindLastPathSep(const std::string& path)
{
	size_t pos1 = path.rfind('/');
	if (pos1 == std::string::npos) pos1 = 0;
	size_t pos2 = path.rfind('\\');
	if (pos2 == std::string::npos) pos2 = 0;
	return MAX(pos1,pos2);
}

// Converts the SDL_surface to gdImagePtr
gdImagePtr SDLSurface2GDImage(SDL_Surface* src, bool alpha) {
	// WARNING: src has to be a 32bpp surface!

	gdImagePtr gd_image = gdImageCreateTrueColor(src->w,src->h);
	if(!gd_image)
		return NULL;

	if (alpha)  {
		uchar *pxr = (uchar *)src->pixels;
		Uint32 *px = NULL;
		Uint8 R,G,B,A;
		for (int y=0;y<src->h;y++,pxr+=src->pitch)  {
			px=(Uint32 *)pxr;
			for (int x=0;x<src->w;x++,px++)  {
				SDL_GetRGBA(*px,src->format,&R,&G,&B,&A);
				gd_image->tpixels[y][x] = gdTrueColorAlpha(R, G, B, ~(A/2));
			}
		}
	} else {
		for(int y = 0; y < src->h; y++) {
			memcpy(gd_image->tpixels[y], (uchar*)src->pixels + y*src->pitch, src->pitch);
		}
	}
	
	return gd_image;
}

// Saves the surface in PNG format
bool SavePNG(SDL_Surface *image, const std::string& file, bool alpha)  {
	gdImagePtr gd_image = NULL;

	gd_image = SDLSurface2GDImage ( image, alpha );
	if ( !gd_image )
		return false;
	gd_image->saveAlphaFlag = alpha;

	// Save the image
	FILE *out;
	int s;
	char *data = NULL;
	out = fopen ( file.c_str(), "wb" );
	if ( !out )
		return false;

	data = ( char * ) gdImagePngPtr ( gd_image, &s );

	size_t size = s>0?s:-s;
	if ( !data )
	{
		return false;
	}
	if ( fwrite ( data, 1, size, out ) != size )
	{
		return false;
	}

	if ( fclose ( out ) != 0 )
	{
		return false;
	}

	// Free everything
	gdFree ( data );

	gdImageDestroy ( gd_image );

	return true;
}
