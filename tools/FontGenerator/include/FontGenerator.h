////////////////////////////
// Standard includes
////////////////////////////

#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <string>
#include <GD/gd.h>

///////////////////////
// Global defines
///////////////////////
#define RMASK  0x00ff0000
#define GMASK  0x0000ff00
#define BMASK  0x000000ff
#define AMASK  0x00000000

// For 16bit surface
/*#define RMASK  0x0f00
#define GMASK  0x00f0
#define BMASK  0x000f
#define AMASK  0x0000*/

#define FIRST_CHARACTER 32  // Space
#define LAST_CHARACTER 256 // TODO: some normal range

///////////////////////
// Types
///////////////////////
typedef unsigned int color_t;
typedef unsigned char byte;
typedef byte uchar;

///////////////////////
// Global variables
///////////////////////

SDL_Surface *Screen = NULL;

//////////////////////
// Functions
//////////////////////

#ifdef _MSC_VER
inline int strcasecmp(const char *a, const char *b) {return _stricmp(a,b); }  // To provide the standards
#endif

template <typename T> inline T MIN(T a, T b) { return a<b?a:b; }
template <typename T> inline T MAX(T a, T b) { return a>b?a:b; }

void Output(const std::string& str);
bool FileExists(const std::string& str);
size_t FindLastPathSep(const std::string& path);
void DrawVLine(SDL_Surface *bmpDest, int x, color_t color);
bool SavePNG(SDL_Surface *image, const std::string& file);
void Quit();
