/*
 *  allegro.cpp
 *  Gusanos
 *
 *  Created by Albert Zeyer on 30.11.09.
 *  code under LGPL
 *
 */

#include <SDL.h>
#include <SDL_image.h>
#include <boost/static_assert.hpp>

#include "gusanos/allegro.h"
#include "FindFile.h"
#include "Debug.h"
#include "GfxPrimitives.h"
#include "StringUtils.h"
#include "InputEvents.h"
#include "EventQueue.h"
#include "LieroX.h"
#include "Options.h"



static int color_conversion = 0;
int get_color_conversion() { return color_conversion; }
void set_color_conversion(int mode) { color_conversion = mode; }

static int color_depth = 32;
int get_color_depth() { return color_depth; }
void set_color_depth(int depth) { color_depth = depth; }


static void sub_to_abs_coords_x(ALLEGRO_BITMAP* bmp, int& x) { x += bmp->sub_x; }
static void sub_to_abs_coords_y(ALLEGRO_BITMAP* bmp, int& y) { y += bmp->sub_y; }
static void sub_to_abs_coords(ALLEGRO_BITMAP* bmp, int& x, int& y) {
	sub_to_abs_coords_x(bmp, x);
	sub_to_abs_coords_y(bmp, y);
}

static ALLEGRO_BITMAP* create_bitmap_from_sdl(const SmartPointer<SDL_Surface>& surf, int subx, int suby, int subw, int subh) {
	ALLEGRO_BITMAP* bmp = new ALLEGRO_BITMAP();
	
	bmp->surf = surf;
	bmp->sub_x = subx;
	bmp->sub_y = suby;
	bmp->w = subw;
	bmp->h = subh;
	bmp->cl = 0;
	bmp->cr = subw;
	bmp->ct = 0;
	bmp->cb = subh;
	
	bmp->line = new unsigned char* [subh];
	for(int y = 0; y < subh; ++y)
		bmp->line[y] = (Uint8 *)surf->pixels + ((suby + y) * surf->pitch) + (subx * surf->format->BytesPerPixel);

	return bmp;
}

static ALLEGRO_BITMAP *create_bitmap_from_sdl(const SmartPointer<SDL_Surface>& surf) {	
	return create_bitmap_from_sdl(surf, 0, 0, surf->w, surf->h);
}

static void
graphics_dump_palette(SDL_Surface* p_bitmap)
{
    for(int l_i = 0; l_i < p_bitmap->format->palette->ncolors; l_i++) {
        printf("%d: %x %x %x\n", l_i,
			   p_bitmap->format->palette->colors[l_i].r,
			   p_bitmap->format->palette->colors[l_i].g,
			   p_bitmap->format->palette->colors[l_i].b);
    }
}

static void dumpUsedColors(SDL_Surface* surf);


ALLEGRO_BITMAP* screen = NULL;

static SDL_Surface* create_32bpp_sdlsurface(int w, int h) {
	int rmask = 0xff0000, gmask = 0xff00, bmask = 0xff, amask = 0xff000000;
	if(screen != NULL) {
		rmask = screen->surf->format->Rmask;
		gmask = screen->surf->format->Gmask;
		bmask = screen->surf->format->Bmask;
		amask = screen->surf->format->Amask;
	}
	else if(SDL_GetVideoSurface() && SDL_GetVideoSurface()->format->BitsPerPixel == 32) {
		rmask = SDL_GetVideoSurface()->format->Rmask;
		gmask = SDL_GetVideoSurface()->format->Gmask;
		bmask = SDL_GetVideoSurface()->format->Bmask;		
		amask = SDL_GetVideoSurface()->format->Amask;
	}
	if(amask == 0) amask = 0xff000000;
	
	return SDL_CreateRGBSurface(SDL_SWSURFACE /*| SDL_SRCALPHA*/, w, h, 32, rmask,gmask,bmask,0);
}

ALLEGRO_BITMAP *load_bitmap(const char *filename, RGB *pal) {
	std::string fullfilename = GetFullFileName(filename);	
	SDL_Surface* img = IMG_Load(Utf8ToSystemNative(fullfilename).c_str());
	if(!img) return NULL;
	
	if( img->format->BitsPerPixel == 8 )
		return create_bitmap_from_sdl(img);
	
	SDL_Surface* converted = create_32bpp_sdlsurface(img->w, img->h);
	CopySurface(converted, img, 0, 0, 0, 0, img->w, img->h);
	
	//SDL_Surface* converted = SDL_DisplayFormat(img);
	//SDL_FreeSurface(img);

	if(!converted) {
		errors << "Failed: Converting of bitmap " << filename << /*" to " << bpp <<*/ " bit" << endl;
		return NULL;
	}
	
	return create_bitmap_from_sdl(converted);
}

ALLEGRO_BITMAP *create_bitmap_ex(int color_depth, int width, int height) {	
	SDL_Surface* surf = NULL;
	if(color_depth == 8)
		surf = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 8, 0,0,0,0);
	else
		surf = create_32bpp_sdlsurface(width, height);
	if(!surf) {
		errors << "create_bitmap_ex: cannot create surface with " << width << "x" << height << "x" << color_depth << endl;
		return NULL;
	}
	FillSurface(surf, Color());
	
	if(surf->format->BitsPerPixel != color_depth)
		warnings << "create_bitmap_ex: couldn't create surface with " << color_depth << " bpp" << endl;
	
	return create_bitmap_from_sdl(surf);
}

ALLEGRO_BITMAP *create_bitmap(int width, int height) {
	return create_bitmap_ex(color_depth, width, height);
}

ALLEGRO_BITMAP *create_sub_bitmap(ALLEGRO_BITMAP *parent, int x, int y, int width, int height) {
	sub_to_abs_coords(parent, x, y);
	return create_bitmap_from_sdl(parent->surf, x, y, width, height);
}

ALLEGRO_BITMAP* create_copy_bitmap(ALLEGRO_BITMAP* other) {
	if(other)
		return create_bitmap_from_sdl(GetCopiedImage(other->surf), other->sub_x, other->sub_y, other->w, other->h);
	else
		return NULL;
}

void destroy_bitmap(ALLEGRO_BITMAP *bmp) {
	if(bmp == NULL) return;
	delete[] bmp->line;
	bmp->surf = NULL;
	delete bmp;
}



int set_gfx_mode(int card, int w, int h, int v_w, int v_h) { return 0; }
int SCREEN_W = 640, SCREEN_H = 480;

int allegro_error = 0;


int cpu_capabilities = 0;

// NOTE: This is only for testing right now, so people with gfx problems can test it.
// Later on, this is supposed to be removed. There is no reason why the user should
// be able to set this. If it is available and works, it should be used - otherwise not.
static bool cfgUseSSE = true, cfgUseMMX = true, cfgUseMMXExt = true;
static bool bRegisteredAllegroVars = CScriptableVars::RegisterVars("GameOptions")
( cfgUseSSE, "Video.UseSSE", true )
( cfgUseMMX, "Video.UseMMX", true )
( cfgUseMMXExt, "Video.UseMMXExt", true );


bool allegro_init() {
	cpu_capabilities = 0;
	notes << "Allegro: ";
	
	if(cfgUseSSE && SDL_HasSSE()) cpu_capabilities |= CPU_SSE;
	if(cfgUseMMX && SDL_HasMMX()) cpu_capabilities |= CPU_MMX;
	if(cfgUseMMXExt && SDL_HasMMXExt()) cpu_capabilities |= CPU_MMXPLUS;
	
	if(cpu_capabilities & CPU_SSE) notes << "SSE, "; else notes << "no SSE, ";
	if(cpu_capabilities & CPU_MMX) notes << "MMX, "; else notes << "no MMX, ";
	if(cpu_capabilities & CPU_MMXPLUS) notes << "MMXExt"; else notes << "no MMXExt";
	notes << endl;
	
	screen = create_bitmap_ex(32, SCREEN_W, SCREEN_H);
	notes << "Allegro screen format:" << endl;
	DumpPixelFormat(screen->surf->format);
	
	return true;
}

void allegro_exit() {
	destroy_bitmap(screen);
	screen = NULL;
}

void rest(int t) { SDL_Delay(t); }
void vsync() {  }


static void handle_sdlevents();

void acquire_screen() {}
void release_screen() {
	SDL_Flip(SDL_GetVideoSurface());
}





int set_display_switch_mode(int mode) { return 0; }



bool gusExists(const std::string& filename) {
	return IsFileAvailable(filename, false, false);
}

bool gusExistsFile(const std::string& filename) {
	return IsFileAvailable(filename, false, true);	
}

bool gusIsDirectory(const std::string& filename) {
	return IsDirectory(filename);
}


bool gusOpenGameFileR(std::ifstream& f, const std::string& path, std::ios_base::openmode mode) {
	return OpenGameFileR(f, path, mode);
}

FILE* gusOpenGameFile(const std::string& path, const char *mode) {
	return OpenGameFile(path, mode);
}

Iterator<std::string>::Ref gusFileListIter(
										   const std::string& dir,
										   bool absolutePath,
										   const filemodes_t modefilter,
										   const std::string& namefilter) {
	return FileListIter(dir, absolutePath, modefilter, namefilter);
}

extern "C" {
	FILE* gusOpenGameFile(const char* path, const char *mode) {
		return gusOpenGameFile(std::string(path), mode);
	}
}



int _rgb_r_shift_15, _rgb_g_shift_15, _rgb_b_shift_15,
    _rgb_r_shift_16, _rgb_g_shift_16, _rgb_b_shift_16,
    _rgb_r_shift_24, _rgb_g_shift_24, _rgb_b_shift_24,
    _rgb_r_shift_32, _rgb_g_shift_32, _rgb_b_shift_32;

int _rgb_scale_5[32], _rgb_scale_6[64];

void rgb_to_hsv(int r, int g, int b, float *h, float *s, float *v) {	
    float maxc = (float)MAX(MAX(r, g), b);
    float minc = (float)MIN(MIN(r, g), b);
    *v = maxc;
    if(minc == maxc) {
		*h = 0;
		*s = 0;
		return;
	}

	*s = (maxc-minc) / maxc;
	float rc = (maxc-r) / (maxc-minc);
	float gc = (maxc-g) / (maxc-minc);
	float bc = (maxc-b) / (maxc-minc);
	if(r == maxc) *h = bc-gc;
	else if(g == maxc) *h = 2.0f+rc-bc;
	else *h = 4.0f+gc-rc;

	*h = *h/6.0f;
	FMOD(*h, 1.0f);
}


static bool abscoord_in_bmp(ALLEGRO_BITMAP* bmp, int x, int y) {
	return x >= bmp->sub_x && x < bmp->sub_x + bmp->w && y >= bmp->sub_y && y < bmp->sub_y + bmp->h;
}

static bool coord_in_bmp(ALLEGRO_BITMAP* bmp, int x, int y) {
	return x >= 0 && x < bmp->w && y >= 0 && y < bmp->h;
}


static int getpixel__nocheck(ALLEGRO_BITMAP *bmp, int x, int y) {
	unsigned long addr = (unsigned long) bmp->line[y] + x * bmp->surf->format->BytesPerPixel;
	switch(bmp->surf->format->BytesPerPixel) {
		case 1: return bmp_read8(addr);
		case 2: return bmp_read16(addr);
		case 3: return bmp_read24(addr);
		case 4: return bmp_read32(addr);
	}
	return 0;
}

static void putpixel__nocheck(ALLEGRO_BITMAP *bmp, int x, int y, int color) {
	unsigned long addr = (unsigned long) bmp->line[y] + x * bmp->surf->format->BytesPerPixel;
	switch(bmp->surf->format->BytesPerPixel) {
		case 1: bmp_write8(addr, color); break;
		case 2: bmp_write16(addr, color); break;
		case 3: bmp_write24(addr, color); break;
		case 4: bmp_write32(addr, color); break;
	}
}

int getpixel(ALLEGRO_BITMAP *bmp, int x, int y) {
	if(!coord_in_bmp(bmp, x, y)) return 0;
	return getpixel__nocheck(bmp, x, y);
}

void putpixel(ALLEGRO_BITMAP *bmp, int x, int y, int color) {
	if(!coord_in_bmp(bmp, x, y)) return;
	putpixel__nocheck(bmp, x, y, color);
}



static Color allegcol_to_Col(int col) {
	return Color(getr(col), getg(col), getb(col), SDL_ALPHA_OPAQUE);
}

void vline(ALLEGRO_BITMAP *bmp, int x, int y1, int y2, int color) {
	sub_to_abs_coords(bmp, x, y1);
	sub_to_abs_coords_y(bmp, y2);
	DrawVLine(bmp->surf.get(), y1, y2, x, allegcol_to_Col(color));
/*	for(int y = y1; y < y2; ++y)
		if(abscoord_in_bmp(bmp, x, y))
			putpixel(bmp, x, y, color);*/
}

void hline(ALLEGRO_BITMAP *bmp, int x1, int y, int x2, int color) {
	sub_to_abs_coords(bmp, x1, y);
	sub_to_abs_coords_x(bmp, x2);
	DrawHLine(bmp->surf.get(), x1, x2, y, allegcol_to_Col(color));
/*	for(int x = x1; x < x2; ++x)
		if(abscoord_in_bmp(bmp, x, y))
			putpixel(bmp, x, y, color);*/
}

void line(ALLEGRO_BITMAP *bmp, int x1, int y1, int x2, int y2, int color) {
	sub_to_abs_coords(bmp, x1, y1);
	sub_to_abs_coords(bmp, x2, y2);
	DrawLine(bmp->surf.get(), x1, y1, x2, y2, allegcol_to_Col(color));
}

void rectfill(ALLEGRO_BITMAP *bmp, int x1, int y1, int x2, int y2, int color) {
	sub_to_abs_coords(bmp, x1, y1);
	sub_to_abs_coords(bmp, x2, y2);
	SDL_Rect rect = { x1, y1, x2 - x1, y2 - y1 };
	SDL_FillRect(bmp->surf.get(), &rect, color);
}

void circle(ALLEGRO_BITMAP *bmp, int x, int y, int radius, int color) {
	sub_to_abs_coords(bmp, x, y);
	DrawCircleFilled(bmp->surf.get(), x, y, radius, radius, allegcol_to_Col(color));
}


void clear_to_color(ALLEGRO_BITMAP *bmp, int color) {
	rectfill(bmp, 0,0, bmp->w, bmp->h, color);
}


static void blit_8to8__abscoord(ALLEGRO_BITMAP *source, ALLEGRO_BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height) {
	int& sy = source_y;
	int& dy = dest_y;
	for(int Cy = height; Cy >= 0; --Cy, ++sy, ++dy) {
		int sx = source_x;
		int dx = dest_x;
		for(int Cx = width; Cx >= 0; --Cx, ++sx, ++dx) {
			if(abscoord_in_bmp(dest, dx, dy) && abscoord_in_bmp(source, sx, sy))
				putpixel__nocheck(dest, dx, dy, getpixel__nocheck(source, sx, sy));
		}
	}
}

static int getpixel__nocheck(SDL_Surface *surf, int x, int y) {
	unsigned long addr = (unsigned long) surf->pixels + y * surf->pitch + x * surf->format->BytesPerPixel;
	switch(surf->format->BytesPerPixel) {
		case 1: return bmp_read8(addr);
		case 2: return bmp_read16(addr);
		case 3: return bmp_read24(addr);
		case 4: return bmp_read32(addr);
	}
	return 0;
}

static void dumpUsedColors(SDL_Surface* surf) {
	std::set<Uint32> cols;
	for(int y = surf->h - 1; y >= 0; --y) {
		for(int x = surf->w - 1; x >= 0; --x) {
			cols.insert(getpixel__nocheck(surf, x, y));
		}
	}
	for(std::set<Uint32>::iterator i = cols.begin(); i != cols.end(); ++i)
		notes << "  : " << *i << endl;
}

void blit(ALLEGRO_BITMAP *source, ALLEGRO_BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height) {
	sub_to_abs_coords(source, source_x, source_y);
	sub_to_abs_coords(dest, dest_x, dest_y);
/*	if(source->surf->format->BitsPerPixel == 8 && dest->surf->format->BitsPerPixel == 8) {
		blit_8to8__abscoord(source, dest, source_x, source_y, dest_x, dest_y, width, height);
	}
	else {*/
		SDL_Rect srcrect = { source_x, source_y, width, height };
		SDL_Rect dstrect = { dest_x, dest_y, width, height };
		SDL_BlitSurface(source->surf.get(), &srcrect, dest->surf.get(), &dstrect);
//	}
}

void stretch_blit(ALLEGRO_BITMAP *s, ALLEGRO_BITMAP *d, int s_x, int s_y, int s_w, int s_h, int d_x, int d_y, int d_w, int d_h) {
	sub_to_abs_coords(s, s_x, s_y);
	sub_to_abs_coords(d, d_x, d_y);
	DrawImageResizedAdv(d->surf.get(), s->surf.get(), s_x, s_y, d_x, d_y, s_w, s_h, d_w, d_h);
}

void masked_blit(ALLEGRO_BITMAP *source, ALLEGRO_BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height) {
	SetColorKey(source->surf.get());
	blit(source, dest, source_x, source_y, dest_x, dest_y, width, height);
}

void draw_sprite(ALLEGRO_BITMAP *bmp, ALLEGRO_BITMAP *sprite, int x, int y) {
	masked_blit(sprite, bmp, 0, 0, x, y, sprite->w, sprite->h);
}

void draw_sprite_h_flip(struct ALLEGRO_BITMAP *bmp, struct ALLEGRO_BITMAP *sprite, int x, int y) {
	DrawImageAdv_Mirror(bmp->surf.get(), sprite->surf.get(), 0, 0, x, y, sprite->w, sprite->h);
}


void clear_bitmap(ALLEGRO_BITMAP* bmp) { clear_to_color(bmp, 0); }



unsigned long bmp_write_line(ALLEGRO_BITMAP *bmp, int line) {
	return (unsigned long) bmp->line[line];
}

void bmp_unwrite_line(ALLEGRO_BITMAP* bmp) {}




void drawing_mode(int mode, ALLEGRO_BITMAP *pattern, int x_anchor, int y_anchor) {}
void set_trans_blender(int r, int g, int b, int a) {}
void set_add_blender (int r, int g, int b, int a) {}
void solid_mode() {}

/*
int getr(int c) { Uint8 r,g,b; SDL_GetRGB(c, mainPixelFormat, &r, &g, &b); return r; }
int getg(int c) { Uint8 r,g,b; SDL_GetRGB(c, mainPixelFormat, &r, &g, &b); return g; }
int getb(int c) { Uint8 r,g,b; SDL_GetRGB(c, mainPixelFormat, &r, &g, &b); return b; }
*/
int getr(int c) { return Uint8(Uint32(c) >> screen->surf->format->Rshift); }
int getg(int c) { return Uint8(Uint32(c) >> screen->surf->format->Gshift); }
int getb(int c) { return Uint8(Uint32(c) >> screen->surf->format->Bshift); }


static Uint32 makecol_intern(Uint32 r, Uint32 g, Uint32 b) {
	return ((r & 0xff) << screen->surf->format->Rshift) | ((g & 0xff) << screen->surf->format->Gshift) | ((b & 0xff) << screen->surf->format->Bshift);
}

int makecol(int r, int g, int b) { return makecol_intern(r,g,b); }
int makecol_depth(int color_depth, int r, int g, int b) {
	return makecol(r,g,b);
	//return SDL_MapRGB(&pixelformat[color_depth/8],r,g,b);
}



void set_clip_rect(ALLEGRO_BITMAP *bitmap, int x1, int y_1, int x2, int y2) {}
void get_clip_rect(ALLEGRO_BITMAP *bitmap, int *x1, int *y_1, int *x2, int *y2) {}






// index is allegro key, value is sdl keysym
static const Uint32 sdlkeymap[KEY_MAX+1] =
{
SDLK_UNKNOWN,
'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z',
'0','1','2','3','4','5','6','7','8','9',
'0','1','2','3','4','5','6','7','8','9', // numpads (no difference in SDL)
SDLK_F1,SDLK_F2,SDLK_F3,SDLK_F4,SDLK_F5,SDLK_F6,SDLK_F7,SDLK_F8,SDLK_F9,SDLK_F10,SDLK_F11,SDLK_F12,
SDLK_ESCAPE,
'~',
'-',
'=',
SDLK_BACKSPACE,
SDLK_TAB,
'{','}',
SDLK_RETURN,
SDLK_COLON,
SDLK_QUOTE,
SDLK_BACKSLASH,
SDLK_BACKSLASH, // second backslash?
',',
SDLK_UNKNOWN, // STOP ?
SDLK_SLASH,
SDLK_SPACE,
SDLK_INSERT,SDLK_DELETE,
SDLK_HOME,SDLK_END,
SDLK_PAGEUP,SDLK_PAGEDOWN,
SDLK_LEFT,SDLK_RIGHT,SDLK_UP,SDLK_DOWN,
SDLK_SLASH, // numpad
SDLK_ASTERISK,
SDLK_MINUS,SDLK_PLUS,SDLK_DELETE,SDLK_RETURN, // numpad
SDLK_PRINT,
SDLK_PAUSE,
SDLK_UNKNOWN, // ABNT_C1
SDLK_UNKNOWN, // YEN
SDLK_UNKNOWN, // __allegro_KEY_KANA         = 96,
SDLK_UNKNOWN, // __allegro_KEY_CONVERT      = 97,
SDLK_UNKNOWN, // __allegro_KEY_NOCONVERT    = 98,
SDLK_AT,
SDLK_UNKNOWN, // __allegro_KEY_CIRCUMFLEX   = 100,
SDLK_UNKNOWN, // __allegro_KEY_COLON2       = 101,
SDLK_UNKNOWN, // __allegro_KEY_KANJI        = 102,
SDLK_EQUALS, // numpad
SDLK_BACKQUOTE,
SDLK_SEMICOLON,
SDLK_LSUPER, // command (MacOSX)
0,0,0,0,0,0,0,0, // allegro unknown1-8
// modifiers are starting here
SDLK_LSHIFT,SDLK_RSHIFT,
SDLK_LCTRL,SDLK_RCTRL,
SDLK_LALT, // alt
SDLK_RALT, // alt gr
SDLK_LSUPER, // lwin
SDLK_RSUPER, // rwin
SDLK_LMETA, // menu
SDLK_SCROLLOCK,
SDLK_NUMLOCK,
SDLK_CAPSLOCK,
SDLK_UNKNOWN // key max
};


// index is sdl keysym, value is allegro key
static int allegrokeymap[SDLK_LAST];

static int findAllegroKey(Uint32 sdlkeysym) {
	for(int i = 0; i < KEY_MAX; ++i) {
		if(sdlkeymap[i] == sdlkeysym) return i;
	}
	return KEY_UNKNOWN1;
}

void install_keyboard() {
	for(Uint32 i = 0; i < SDLK_LAST; ++i)
		allegrokeymap[i] = findAllegroKey(i);
	
	// just some checks to be sure i didn't made a mistake
	assert(sdlkeymap[KEY_ESC] == SDLK_ESCAPE);
	assert(sdlkeymap[__allegro_KEY_DEL] == SDLK_DELETE);
	assert(sdlkeymap[__allegro_KEY_DOWN] == SDLK_DOWN);
	assert(sdlkeymap[__allegro_KEY_AT] == SDLK_AT);
	assert(sdlkeymap[KEY_SEMICOLON] == SDLK_SEMICOLON);
	assert(sdlkeymap[KEY_LSHIFT] == SDLK_LSHIFT);
	assert(sdlkeymap[KEY_SCRLOCK] == SDLK_SCROLLOCK);
	assert(sdlkeymap[KEY_MAX] == SDLK_UNKNOWN);
	assert(allegrokeymap[SDLK_ESCAPE] == KEY_ESC);
}

void remove_keyboard() {}


static std::list<UnicodeChar> keyqueue;

bool keypressed() {
	return keyqueue.size() > 0;
}

int readkey() {
	int key = keyqueue.front();
	keyqueue.pop_front();
	return key;
}

int key[KEY_MAX];

void clear_keybuf() {
	keyqueue.clear();
	// don't clear key[], this is only about readkey()
	// for(int i = 0; i < KEY_MAX; ++i) key[i] = 0;
}

static void handle_sdlevents_keyb() {
	for(int i = 0; i < KEY_MAX; ++i)
		key[i] = GetKeyboard()->KeyDown[sdlkeymap[i]] ? 1 : 0;
	
	for(int i = 0; i < GetKeyboard()->queueLength; ++i) {
		KeyboardEvent& ev = GetKeyboard()->keyQueue[i];
		if(ev.down) keyqueue.push_back( ev.sym );
	}
}







void install_mouse() {}
void remove_mouse() {}

volatile int mouse_x = 0;
volatile int mouse_y = 0;
volatile int mouse_z = 0;
volatile int mouse_b = 0;
void (*mouse_callback)(int flags) = NULL;


int poll_mouse() { return 0; }


static void handle_sdlevents_mouse() {
	mouse_t* m = GetMouse();
	mouse_x = m->X;
	mouse_y = m->Y;
	mouse_b = m->Button;
}

static void handle_sdlevents() {
	handle_sdlevents_keyb();
	handle_sdlevents_mouse();	
}


void pushAllegroEvents() {
	//handle_sdlevents();
}

