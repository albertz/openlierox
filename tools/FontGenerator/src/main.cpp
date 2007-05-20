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

	// Set the video mode
	/*if ((Screen = SDL_SetVideoMode(1024,50, 0,SDL_SWSURFACE)) == NULL)  {
		Quit();
		Output("Could not set the videomode.");
		return -1;
	}*/

	//
	// Parse the arguments
	//

	if (argc < 2)  {
		// TODO: own function for this output
		// TODO: fill this with text
		Output
			<< "Too less parameters" << endl
			<< "usage: " << endl
			<< "  " << argv[0] << " <input_file>" << endl;
		Quit();
		return -1;
	}

	// File name
	/*std::string InputFile = argv[1];  // Note: argv[0] is path to this program
	if (InputFile.rfind('.') == std::string::npos)
		InputFile += ".ttf";

	if (!FileExists(InputFile))  {
		Output("The input file \""+InputFile+"\" does not exist!");
		Quit();

		return -1;
	}

	// Outline, size and output file
	std::string OutputFile = "./"+InputFile.substr(FindLastPathSep(InputFile),InputFile.rfind("."))+".png";
	bool Outline = false;
	size_t Size = 15;

	int i;
	for (i=2;i<argc;i++)  {
		if (!strcasecmp(argv[i],"-outline"))
			Outline = true;
		else if (Size = atoi(argv[i])) 
			continue;
		else
			OutputFile = argv[i];
	}

	if (!Size) Size = 15; // Check*/
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

	if (Arguments.Bold)
		TTF_SetFontStyle(Font,TTF_STYLE_BOLD);
	if (Arguments.Italic)
		TTF_SetFontStyle(Font,TTF_STYLE_ITALIC);
	if (Arguments.Underline)
		TTF_SetFontStyle(Font,TTF_STYLE_UNDERLINE);

	//
	// Count the surface size
	//
	int SurfaceWidth = 0;
	int SurfaceHeight = 0;
	Uint16 Characters[(LAST_CHARACTER-FIRST_CHARACTER)*2+1];  // There's a space between each two characters to make some space for
															  // the dividing line. There's no possibility to increase spacing and
															  // TTF_RenderGlyph & TTF_GlyphMetrics don't give enough flexibility
															  // to render the text (someone forgot on glyph y-offset)

	register Uint16 c;
	for (c=0;c<(LAST_CHARACTER-FIRST_CHARACTER)*2;c++)  {
		Characters[c] = (c/2)+FIRST_CHARACTER;
		Characters[++c] = ' ';
	}
	Characters[(LAST_CHARACTER-FIRST_CHARACTER)*2] = 0; // Terminating
	TTF_SizeUNICODE(Font,Characters,&SurfaceWidth,&SurfaceHeight);


	//
	// Create the surface
	//
	SDL_Surface *OutBmp = SDL_CreateRGBSurface(SDL_SWSURFACE,SurfaceWidth,SurfaceHeight,32,RMASK,GMASK,BMASK,AMASK);
	if (!OutBmp)  {
		Output << "Out of memory while creating the bitmap surface." << endl;
		TTF_CloseFont(Font);
		Quit();
		return -1;
	}

	color_t Pink = SDL_MapRGB(OutBmp->format,255,0,255);
	SDL_FillRect(OutBmp,NULL,Pink);  // Fill with pink

	const color_t Blue = SDL_MapRGB(OutBmp->format,0,0,255);  // Separator color

	//
	//  Render the font
	//

	// First, render the font
	//SDL_Color fntPink = {255,0,255};

	// Replace all the not black/not pink colors with black/pink
	// It looks better than TTF_RenderUNICODE_Solid then
	/*color_t *px,*py;
	unsigned short x,y;
	color_t CurCol;
	for (py= (color_t *)OutBmp->pixels,y=0;y<OutBmp->h;py+=OutBmp->pitch/4,y++)
		for (px = py,x=0; x<OutBmp->w; px++,x++)  {
			CurCol = *px;
			if (abs(CurCol-Pink) < CurCol) *px=Pink;
			else *px = 0;
		}*/

	/*color_t *px,*py;
	unsigned short x,y;
	Uint8 r,g,b,a;
	for (py= (color_t *)Text->pixels,y=0;y<Text->h;py+=Text->pitch/4,y++)
		for (px = py,x=0; x<Text->w; px++,x++)  {
			SDL_GetRGBA(*px,Text->format,&r,&g,&b,&a);
			if (a < 150) 
				*px=Pink;
			else 
				*px = 0xff000000;
		}*/

	SDL_Color fntBlack = {0,0,0};
	SDL_Surface *Text = TTF_RenderUNICODE_Solid(Font,Characters,fntBlack);
	SDL_BlitSurface(Text,NULL,OutBmp,NULL);
	SDL_FreeSurface(Text);

	//
	// Add the dividing lines
	//
	int cur_x = 0;
	int space_width=0;
	TTF_GlyphMetrics(Font,' ',NULL,NULL,NULL,NULL,&space_width);
	int advance;

	for (c=0;c<LAST_CHARACTER-FIRST_CHARACTER;c++)  {
		if (TTF_GlyphMetrics(Font,Characters[c*2],NULL,NULL,NULL,NULL,&advance) != -1)  // c*2 because of the spaces
			cur_x += advance;
		else
			continue;  // The character is not in the font

		DrawVLine(OutBmp,cur_x,Blue);
		cur_x += space_width;
	}

	//
	// Save the font
	//

	if (!SavePNG(OutBmp,Arguments.OutputFile))  
		Output << "Could not save the resulting bitmap." << endl;

	//
	// Quit
	//
	SDL_FreeSurface(OutBmp);
	Quit();

	return 0;
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
  SDL_Delay(1000);
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
