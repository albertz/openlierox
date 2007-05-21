/*
	OpenLieroX - FontGenerator
	
	by Dark Charlie, Albert Zeyer
	Code under LGPL
	( 15-05-2007 )
*/

#include <iostream>
#include <SDL/SDL_ttf.h>
#include <gd.h>

#include "StringUtils.h"
#include "Utils.h"

#include "FontGenerator.h"

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
	if (Arguments.Underline)
		TTF_SetFontStyle(Font,TTF_STYLE_UNDERLINE);

	//
	// Prepare for rendering
	//
	Output << "Preparing for compilation" << endl;

	Uint16 Characters[(LAST_CHARACTER-FIRST_CHARACTER)+1];

	register Uint16 c;
	for (c=0;c<(LAST_CHARACTER-FIRST_CHARACTER);c++)  {
		Characters[c] = c+FIRST_CHARACTER;
	}
	Characters[(LAST_CHARACTER-FIRST_CHARACTER)] = 0; // Terminating


	//
	//  Render the font
	//

	Output << "Rendering the font." << endl;

	SDL_Surface *OutBmp = RenderText(Font,Characters,(LAST_CHARACTER-FIRST_CHARACTER),Arguments.Outline);
	if (!OutBmp)  {
		TTF_CloseFont(Font);
		Quit();
		return -1;
	}

	//
	// Save the font
	//

	Output << "Saving in " << Arguments.OutputFile << endl;
	if (!SavePNG(OutBmp,Arguments.OutputFile))  
		Output << "Could not save the resulting bitmap." << endl;

	//
	// Quit
	//
	Output << "Successfully finished! :)" << endl;
	SDL_FreeSurface(OutBmp);
	TTF_CloseFont(Font);
	Quit();

	return 0;
}

// Render the text
SDL_Surface *RenderText(TTF_Font *Font, Uint16 *Text, size_t TextLen, bool Outline)
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
	int maxy,advance;

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
	if (TTF_SizeUNICODE(Font,Text,&SurfaceWidth,&SurfaceHeight) == -1)  {
		Output << "Cannot get the font size!" << endl;
		return NULL;
	}
	SurfaceWidth += TextLen; // Space for separators

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
	Result = SDL_CreateRGBSurface(SDL_SWSURFACE,SurfaceWidth,SurfaceHeight,32,RMASK,GMASK,BMASK,AMASK);
	if (!Result)  {
		Output << "Out of memory while creating the bitmap surface." << endl;
		return NULL;
	}

	SDL_FillRect(Result,NULL,SDL_MapRGB(Result->format,255,0,255));  // Fill with pink (transparent)


	//
	// Render the text, glyph by glyph
	//
	const color_t Blue = SDL_MapRGB(Result->format,0,0,255);

	for (size_t i=0;i<TextLen;i++)  {
		// Get the glyph metrics
		if (TTF_GlyphMetrics(Font,Text[i],NULL,NULL,NULL,&maxy,&advance) == -1)
			continue;

		// Blit the glyph
		Glyph = TTF_RenderGlyph_Solid(Font,Text[i],Color);
		if (!Glyph)
			continue;
		Rect.y = YOffset-maxy;
		SDL_BlitSurface(Glyph,NULL,Result,&Rect);
		Rect.x += advance+Outline;  // Outline is either 0 (disabled) or 1 (enabled)
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
	const color_t Transparent = SDL_MapRGB(BitmapText->format,255,0,255);
	const color_t FontColor = SDL_MapRGB(BitmapText->format,255,255,255);
	const color_t OutlineColor = 0;

	unsigned short pitchstep = BitmapText->pitch/sizeof(color_t);
	Uint32 *pxrow = (Uint32 *)BitmapText->pixels+pitchstep;
	Uint32 *px;
	Uint32 *TempPx;
	int x,y;

	for (y=1;y<BitmapText->h-1;y++,pxrow+=pitchstep)  {
		px = pxrow+1;
		for (x=1;x<BitmapText->w-1;x++,px++)  {
			if (*px == FontColor)  {
				// Left pixel
				TempPx = px-1;
				if (*TempPx == Transparent) *TempPx = OutlineColor;

				// Top pixel
				TempPx = px-pitchstep;
				if (*TempPx == Transparent) *TempPx = OutlineColor;

				// Right pixel
				TempPx = px+1;
				if (*TempPx == Transparent) *TempPx = OutlineColor;

				// Bottom pixel
				TempPx = px+pitchstep;
				if (*TempPx == Transparent) *TempPx = OutlineColor;
			}
		}
	}
}

// Displays the help
void DisplayHelp(const std::string& ExeName)
{
	Output << "Usage:" << endl
	<< ExeName << " " << "<input file> [<output file>] [-bold] [-italic] [-underline] [-outline]" << endl;

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
	Result.Bold = false;
	Result.Italic = false;
	Result.Underline = false;
	Result.Outline = false;

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
			Result.Size = atoi(argv[i]+6); // 6 == strlen("size:")
			if (Result.Size == 0)
				Result.Size = DEF_FONTSIZE;
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
		Result.OutputFile = "./"+Result.InputFile.substr(FindLastPathSep(Result.InputFile),Result.InputFile.rfind("."))+".png";

	return Result;
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
gdImagePtr SDLSurface2GDImage(SDL_Surface* src) {
	// WARNING: src has to be a 32bpp surface!

	gdImagePtr gd_image = gdImageCreateTrueColor(src->w,src->h);
	if(!gd_image)
		return NULL;
	
	for(int y = 0; y < src->h; y++) {
		memcpy(gd_image->tpixels[y], (uchar*)src->pixels + y*src->pitch, src->pitch);	
	}
	
	return gd_image;
}

// Saves the surface in PNG format
bool SavePNG(SDL_Surface *image, const std::string& file)  {
	gdImagePtr gd_image = NULL;

	gd_image = SDLSurface2GDImage ( image );
	if ( !gd_image )
		return false;

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
