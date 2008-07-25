////////////////////////////
// Standard includes
////////////////////////////

#include <iostream>
#include <SDL.h>
#include <string>

///////////////////////
// Global defines
///////////////////////
#define RMASK  0x00ff0000
#define GMASK  0x0000ff00
#define BMASK  0x000000ff
#define AMASK  0xff000000

// For 16bit surface
/*#define RMASK  0x0f00
#define GMASK  0x00f0
#define BMASK  0x000f
#define AMASK  0x0000*/

#define FIRST_CHARACTER 32  // Space
#define LAST_CHARACTER 1000 // TODO: some normal range
#define DEF_FONTSIZE 15

///////////////////////
// Types
///////////////////////
typedef unsigned int color_t;
typedef unsigned char byte;
typedef unsigned char uchar;
struct arguments_t {
	std::string InputFile;
	std::string OutputFile;
	bool Outline;
	bool Bold;
	bool Italic;
	bool Underline;
	bool Antialiased;
	size_t Size;
	size_t LastChar;
};

///////////////////////
// Global variables
///////////////////////

extern SDL_Surface* Screen;
extern std::ostream& Output;

//////////////////////
// Functions
//////////////////////

arguments_t ParseArguments(int argc, char *argv[]);
void DisplayHelp(const std::string& ExeName);
bool FileExists(const std::string& str);
size_t FindLastPathSep(const std::string& path);
void DrawVLine(SDL_Surface *bmpDest, int x, color_t color);
SDL_Surface *RenderText(TTF_Font *Font, Uint16 *Text, size_t TextLen, bool Outline, bool Antialiased);
void ApplyOutline(SDL_Surface *BitmapText);
void BlitAlpha(SDL_Surface *dst,SDL_Surface *src,int x, int y);
bool SavePNG(SDL_Surface *image, const std::string& file, bool alpha);
void Quit();
