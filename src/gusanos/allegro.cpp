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
#include "gusanos/blitters/blitters.h"


static int color_conversion = 0;
int get_color_conversion() { return color_conversion; }

LocalSetColorConversion::LocalSetColorConversion(int f) : old(color_conversion) {
	color_conversion = f;
}

LocalSetColorConversion::~LocalSetColorConversion() {
	color_conversion = old;
}

static int color_depth = 32;
int get_color_depth() { return color_depth; }

LocalSetColorDepth::LocalSetColorDepth(int d) : old(color_depth) {
	color_depth = d;
}

LocalSetColorDepth::~LocalSetColorDepth() {
	color_depth = old;
}


static void sub_to_abs_coords_x(ALLEGRO_BITMAP* bmp, int& x) { x += bmp->sub_x; }
static void sub_to_abs_coords_y(ALLEGRO_BITMAP* bmp, int& y) { y += bmp->sub_y; }
static void sub_to_abs_coords(ALLEGRO_BITMAP* bmp, int& x, int& y) {
	sub_to_abs_coords_x(bmp, x);
	sub_to_abs_coords_y(bmp, y);
}

static ALLEGRO_BITMAP* create_bitmap_from_sdl(const SmartPointer<SDL_Surface>& surf, int subx, int suby, int subw, int subh) {
	if(surf.get() == NULL) return NULL;

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

ALLEGRO_BITMAP *create_bitmap_from_sdl(const SmartPointer<SDL_Surface>& surf) {
	if(!surf.get()) return NULL;
	return create_bitmap_from_sdl(surf, 0, 0, surf->w, surf->h);
}

void
graphics_dump_palette(SDL_Surface* p_bitmap)
{
    for(int l_i = 0; l_i < p_bitmap->format->palette->ncolors; l_i++) {
        printf("%d: %x %x %x\n", l_i,
			   p_bitmap->format->palette->colors[l_i].r,
			   p_bitmap->format->palette->colors[l_i].g,
			   p_bitmap->format->palette->colors[l_i].b);
    }
}

void dumpUsedColors(SDL_Surface* surf);


ALLEGRO_BITMAP* screen = NULL;

SmartPointer<SDL_Surface> create_32bpp_sdlsurface__allegroformat(int w, int h) {
	// some Gusanos functions, esp. the blitters code, assume that we use only the 0xffffff bits.
	// So, to keep things simple, just use this fixed format.
	const static int
	rmask = 0xff0000,
	gmask = 0xff00,
	bmask = 0xff,
	amask = 0 /*0xff000000*/;
	
	return SDL_CreateRGBSurface(0, w, h, 32, rmask,gmask,bmask,amask);
}

SmartPointer<SDL_Surface> load_bitmap__allegroformat(const std::string& filename, bool stretch2) {
	std::string fullfilename = GetFullFileName(filename);	
	SmartPointer<SDL_Surface> img = LoadGameImage_unaltered(filename, false, true);
	if(img.get() == NULL)
		return NULL;
	if(img->format->BitsPerPixel == 8) {
		if(stretch2) return GetCopiedStretched2Image(img);
		return img;
	}

	SmartPointer<SDL_Surface> converted = create_32bpp_sdlsurface__allegroformat(stretch2 ? (img->w*2) : img->w, stretch2 ? (img->h*2) : img->h);
	CopySurface(converted.get(), img.get(), 0, 0, 0, 0, img->w, img->h, stretch2);
	
	if(!converted.get()) {
		errors << "Failed: Converting of bitmap " << filename << /*" to " << bpp <<*/ " bit" << endl;
		return NULL;
	}

	return converted;
}

ALLEGRO_BITMAP *load_bitmap(const std::string& filename, bool stretch2) {
	return create_bitmap_from_sdl(load_bitmap__allegroformat(filename, stretch2));
}

ALLEGRO_BITMAP *create_bitmap_ex(int color_depth, int width, int height) {	
	SmartPointer<SDL_Surface> surf;
	if(color_depth == 8)
		surf = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 8, 0,0,0,0);
	else
		surf = create_32bpp_sdlsurface__allegroformat(width, height);
	if(!surf.get()) {
		errors << "create_bitmap_ex: cannot create surface with " << width << "x" << height << "x" << color_depth << endl;
		return NULL;
	}
	FillSurface(surf.get(), Color());
	
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
	//if(cfgUseMMXExt && SDL_HasMMXExt()) cpu_capabilities |= CPU_MMXPLUS; // TODO...?
	
	if(cpu_capabilities & CPU_SSE) notes << "SSE, "; else notes << "no SSE, ";
	if(cpu_capabilities & CPU_MMX) notes << "MMX, "; else notes << "no MMX, ";
	if(cpu_capabilities & CPU_MMXPLUS) notes << "MMXExt"; else notes << "no MMXExt";
	notes << endl;
	
	screen = create_bitmap_ex(32, SCREEN_W, SCREEN_H);
	
	return true;
}

void allegro_exit() {
	destroy_bitmap(screen);
	screen = NULL;
}

void rest(int t) { SDL_Delay(t); }
void vsync() {  }





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
										   const filemodes_t modefilter) {
	return FileListIter(dir, absolutePath, modefilter);
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


// We only support DRAW_MODE_TRANS which is translucent color blending.
// (This was the only mode (for Allgro drawing_mode) used by Gus code.)

// linear interpolator blender mode for combining translucent or lit truecolor pixels
void set_trans_blender(int r, int g, int b, int a) {}

// additive blender mode for combining translucent or lit truecolor pixels
void set_add_blender (int r, int g, int b, int a) {}

// sets mode to DRAW_MODE_SOLID. i.e. simply copying pixels without op
void solid_mode() {}


static bool coord_in_bmp(ALLEGRO_BITMAP* bmp, int x, int y) {
	return x >= 0 && x < bmp->w && y >= 0 && y < bmp->h;
}


static Uint32 getpixel__nocheck(ALLEGRO_BITMAP *bmp, int x, int y) {
	unsigned char* addr = bmp->line[y] + x * bmp->surf->format->BytesPerPixel;
	switch(bmp->surf->format->BytesPerPixel) {
		case 1: return bmp_read8(addr);
		case 2: return bmp_read16(addr);
		case 3: return bmp_read24(addr);
		case 4: return bmp_read32(addr);
	}
	return 0;
}

static void putpixel__nocheck(ALLEGRO_BITMAP *bmp, int x, int y, int color) {
	unsigned char* addr = bmp->line[y] + x * bmp->surf->format->BytesPerPixel;
	switch(bmp->surf->format->BytesPerPixel) {
		case 1: bmp_write8(addr, color); break;
		case 2: bmp_write16(addr, color); break;
		case 3: bmp_write24(addr, color); break;
		case 4: bmp_write32(addr, color); break;
	}
}

Uint32 getpixel(ALLEGRO_BITMAP *bmp, int x, int y) {
	if(!coord_in_bmp(bmp, x, y)) return 0;
	return getpixel__nocheck(bmp, x, y);
}

void putpixel(ALLEGRO_BITMAP *bmp, int x, int y, Uint32 color) {
	if(!coord_in_bmp(bmp, x, y)) return;
	putpixel__nocheck(bmp, x, y, color);
}

void putpixel2x2(ALLEGRO_BITMAP *bmp, int x, int y, Uint32 color) {
	for(short dy = 0; dy < 2; ++dy)
	for(short dx = 0; dx < 2; ++dx)
	putpixel(bmp, x+dx, y+dy, color);
}

void copypixel_solid2x2(ALLEGRO_BITMAP* dst, ALLEGRO_BITMAP* src, int x, int y) {
	for(short dy = 0; dy < 2; ++dy)
	for(short dx = 0; dx < 2; ++dx)
	putpixel_solid(dst, x+dx, y+dy, getpixel(src, x+dx, y+dy));
}


static Color allegcol_to_Col(Uint32 col) {
	return Color(getr(col), getg(col), getb(col), SDL_ALPHA_OPAQUE);
}

void vline(ALLEGRO_BITMAP *bmp, int x, int y1, int y2, Uint32 color) {
	sub_to_abs_coords(bmp, x, y1);
	sub_to_abs_coords_y(bmp, y2);
	DrawVLine(bmp->surf.get(), y1, y2, x, allegcol_to_Col(color));
}

void hline(ALLEGRO_BITMAP *bmp, int x1, int y, int x2, Uint32 color) {
	sub_to_abs_coords(bmp, x1, y);
	sub_to_abs_coords_x(bmp, x2);
	DrawHLine(bmp->surf.get(), x1, x2, y, allegcol_to_Col(color));
}

void line(ALLEGRO_BITMAP *bmp, int x1, int y1, int x2, int y2, Uint32 color) {
	sub_to_abs_coords(bmp, x1, y1);
	sub_to_abs_coords(bmp, x2, y2);
	DrawLine(bmp->surf.get(), x1, y1, x2, y2, allegcol_to_Col(color));
}

void rectfill(ALLEGRO_BITMAP *bmp, int x1, int y1, int x2, int y2, Uint32 color) {
	sub_to_abs_coords(bmp, x1, y1);
	sub_to_abs_coords(bmp, x2, y2);
	if(bmp->surf->format->BitsPerPixel == 8) {
		// currently, DrawRectFill cannot handle 8bit and/or allegcol_to_Col is wrong
		SDL_Rect r = { (Sint16)x1, (Sint16)y1, (Uint16)(x2-x1), (Uint16)(y2-y1) };
		SDL_FillRect(bmp->surf.get(), &r, color);
	}
	else
		DrawRectFill(bmp->surf.get(), x1, y1, x2, y2, allegcol_to_Col(color));
}

void circle(ALLEGRO_BITMAP *bmp, int x, int y, int radius, Uint32 color) {
	sub_to_abs_coords(bmp, x, y);
	DrawCircleFilled(bmp->surf.get(), x, y, radius, radius, allegcol_to_Col(color));
}


void clear_to_color(ALLEGRO_BITMAP *bmp, Uint32 color) {
	rectfill(bmp, 0,0, bmp->w, bmp->h, color);
}


static int getpixel__nocheck(SDL_Surface *surf, int x, int y) {
	unsigned char* addr = (unsigned char*)surf->pixels + y * surf->pitch + x * surf->format->BytesPerPixel;
	switch(surf->format->BytesPerPixel) {
		case 1: return bmp_read8(addr);
		case 2: return bmp_read16(addr);
		case 3: return bmp_read24(addr);
		case 4: return bmp_read32(addr);
	}
	return 0;
}

void dumpUsedColors(SDL_Surface* surf) {
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
	SDL_Rect srcrect = { (Sint16)source_x, (Sint16)source_y, (Uint16)width, (Uint16)height };
	SDL_Rect dstrect = { (Sint16)dest_x, (Sint16)dest_y, (Uint16)width, (Uint16)height };
	DrawImageAdv(dest->surf.get(), source->surf.get(), dstrect, srcrect);
	// This doesn' work. We need colorkey handling:
	// CopySurface(dest->surf.get(), source->surf.get(), source_x, source_y, dest_x, dest_y, width, height);
}

void blit(SDL_Surface* source, ALLEGRO_BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height) {
	sub_to_abs_coords(dest, dest_x, dest_y);
	SDL_Rect srcrect = { (Sint16)source_x, (Sint16)source_y, (Uint16)width, (Uint16)height };
	SDL_Rect dstrect = { (Sint16)dest_x, (Sint16)dest_y, (Uint16)width, (Uint16)height };
	DrawImageAdv(dest->surf.get(), source, dstrect, srcrect);
}

void blit_stretch2(ALLEGRO_BITMAP *source, ALLEGRO_BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int swidth, int sheight) {
	sub_to_abs_coords(source, source_x, source_y);
	sub_to_abs_coords(dest, dest_x, dest_y);
	DrawImageStretch2(dest->surf.get(), source->surf, source_x, source_y, dest_x, dest_y, swidth, sheight);
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

void masked_blit_stretch2(ALLEGRO_BITMAP *source, ALLEGRO_BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height) {
	SetColorKey(source->surf.get());
	blit_stretch2(source, dest, source_x, source_y, dest_x, dest_y, width, height);
}

void draw_sprite(ALLEGRO_BITMAP *bmp, ALLEGRO_BITMAP *sprite, int x, int y) {
	masked_blit(sprite, bmp, 0, 0, x, y, sprite->w, sprite->h);
}

void draw_sprite_h_flip(struct ALLEGRO_BITMAP *bmp, struct ALLEGRO_BITMAP *sprite, int x, int y) {
	DrawImageAdv_Mirror(bmp->surf.get(), sprite->surf.get(), 0, 0, x, y, sprite->w, sprite->h);
}


void clear_bitmap(ALLEGRO_BITMAP* bmp) { clear_to_color(bmp, 0); }



uint8_t* bmp_write_line(ALLEGRO_BITMAP *bmp, int line) {
	return bmp->line[line];
}

void bmp_unwrite_line(ALLEGRO_BITMAP* bmp) {}





/*
int getr(int c) { Uint8 r,g,b; SDL_GetRGB(c, mainPixelFormat, &r, &g, &b); return r; }
int getg(int c) { Uint8 r,g,b; SDL_GetRGB(c, mainPixelFormat, &r, &g, &b); return g; }
int getb(int c) { Uint8 r,g,b; SDL_GetRGB(c, mainPixelFormat, &r, &g, &b); return b; }
*/
int getr(Uint32 c) { return Uint8(c >> screen->surf->format->Rshift); }
int getg(Uint32 c) { return Uint8(c >> screen->surf->format->Gshift); }
int getb(Uint32 c) { return Uint8(c >> screen->surf->format->Bshift); }


static Uint32 makecol_intern(Uint32 r, Uint32 g, Uint32 b) {
	// we currently dont have alpha at all in gusanos
	return
		((r & 0xff) << screen->surf->format->Rshift) |
		((g & 0xff) << screen->surf->format->Gshift) |
		((b & 0xff) << screen->surf->format->Bshift)
		;
}

Uint32 makecol(int r, int g, int b) { return makecol_intern(r,g,b); }
Uint32 makecol_depth(int color_depth, int r, int g, int b) {
	return makecol(r,g,b);
	//return SDL_MapRGB(&pixelformat[color_depth/8],r,g,b);
}



void set_clip_rect(ALLEGRO_BITMAP *bitmap, int x1, int y_1, int x2, int y2) {}
void get_clip_rect(ALLEGRO_BITMAP *bitmap, int *x1, int *y_1, int *x2, int *y2) {}




