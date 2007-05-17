/*
	OpenLieroX - FontGenerator
	
	by Dark Charlie, Albert Zeyer
	Code under LGPL
	( 15-05-2007 )
*/

#include <iostream>
#include <SDL/SDL_ttf.h>
#include <gd/gd.h>

#include "FontGenerator.h"

SDL_Surface* Screen = NULL;

using namespace std;

// Main entry point
int main(int argc, char *argv[])
{
	//
	// Initialization
	//

	Output("Welcome to Font Generator!");
	
	// Initialize SDL
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_NOPARACHUTE) == -1) {
		Output("Could not initialize the SDL library. Quitting.");
		return -1;
	}

	// Initialize SDL_ttf
	if (TTF_Init() == -1)  {
		SDL_Quit();
		Output("Could not initialize SDL_ttf library. Quitting.");
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
		Output("Not enough of parameters");
		Quit();
		return -1;
	}

	// File name
	std::string InputFile = argv[1];  // Note: argv[0] is path to this program
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
	size_t Size = 12;

	int i;
	for (i=2;i<argc;i++)  {
		if (!strcasecmp(argv[i],"-outline"))
			Outline = true;
		else if (Size = atoi(argv[i])) 
			continue;
		else
			OutputFile = argv[i];
	}

	if (!Size) Size = 12; // Check

	//
	// Create the font
	//
	TTF_Font *Font = TTF_OpenFont(InputFile.c_str(),Size);
	if (!Font)  {
		Output("Could not open the font!");
		Quit();
		return -1;
	}

	//
	// Count the surface size
	//
	int SurfaceWidth = 0;
	int SurfaceHeight = 0;

	int sw = 0;
	Uint16 buf[2];
	buf[1] = 0; // Terminating

	for (register Uint16 c=FIRST_CHARACTER;c<=LAST_CHARACTER;c++)  {
		buf[1] = c;
		TTF_SizeUNICODE(Font,buf,&sw,&SurfaceHeight);
		SurfaceWidth+=sw+1;  // +1 is the blue dividing line
	}

	//
	// Create the surface
	//
	SDL_Surface *OutBmp = SDL_CreateRGBSurface(SDL_SWSURFACE,SurfaceWidth,SurfaceHeight,32,RMASK,GMASK,BMASK,AMASK);
	if (!OutBmp)  {
		Output("Out of memory while creating the bitmap surface.");
		TTF_CloseFont(Font);
		Quit();
		return -1;
	}

	SDL_FillRect(OutBmp,NULL,SDL_MapRGB(OutBmp->format,255,0,255));  // Fill with pink

	const color_t Blue = SDL_MapRGB(OutBmp->format,0,0,255);  // Separator color

	//
	//  Render the font
	//
	SDL_Rect Rect; Rect.y = 0; Rect.x = 0;
	SDL_Surface *character = NULL;
	SDL_Color Black = {0,0,0};
	for (c=FIRST_CHARACTER;c<=LAST_CHARACTER;c++)  {
		buf[0] = c;
		character = TTF_RenderUNICODE_Solid(Font,&buf[0],Black);  // Render the black character
		if (character)  {
			Rect.w = character->w;
			Rect.h = character->h;
			SDL_BlitSurface(character,NULL,OutBmp,&Rect);
			Rect.x += Rect.w;
			DrawVLine(OutBmp,Rect.x,Blue);
			Rect.x++;
			SDL_FreeSurface(character);
		}
	}

	//
	// Save the font
	//

	if (!SavePNG(OutBmp,OutputFile))  
		Output("Could not save the resulting bitmap.");


	//
	// Quit
	//
	SDL_FreeSurface(OutBmp);
	Quit();

	return 0;
}

// Print out an output
void Output(const std::string& str)
{
	if (str != "")
		cout << str << endl;
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
