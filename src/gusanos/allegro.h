/*
 *  allegro.h
 *  Gusanos
 *
 *  Created by Albert Zeyer on 30.11.09.
 *  code under LGPL
 *
 */

/*
 * Wrapper functions for Allegro
 */

#ifndef __GUS_ALLEGRO_H__
#define __GUS_ALLEGRO_H__

#ifdef __cplusplus

#include <SDL.h>
#include <cstdio>
#include <fstream>
#include "SmartPointer.h"
#include "FindFile.h"
#include "CodeAttributes.h"

extern int allegro_error;
extern int cpu_capabilities;
enum {
	CPU_MMX = 1,
	CPU_SSE = 2,
	CPU_MMXPLUS = 4,
};

bool allegro_init();
void allegro_exit();
#define END_OF_MAIN extern void ___foo_allegro

#define END_OF_FUNCTION(x)
#define LOCK_VARIABLE(x)
#define LOCK_FUNCTION(x)

void rest(int t);
void vsync();



void install_mouse();
void remove_mouse();

#define MOUSE_FLAG_MOVE             1
#define MOUSE_FLAG_LEFT_DOWN        2
#define MOUSE_FLAG_LEFT_UP          4
#define MOUSE_FLAG_RIGHT_DOWN       8
#define MOUSE_FLAG_RIGHT_UP         16
#define MOUSE_FLAG_MIDDLE_DOWN      32
#define MOUSE_FLAG_MIDDLE_UP        64
#define MOUSE_FLAG_MOVE_Z           128

extern volatile int mouse_x;
extern volatile int mouse_y;
extern volatile int mouse_z;
extern volatile int mouse_b;
extern void (*mouse_callback)(int flags);

int poll_mouse();

struct ALLEGRO_BITMAP;
extern ALLEGRO_BITMAP* screen;


enum GfxType {
	GFX_AUTODETECT,
	GFX_AUTODETECT_WINDOWED,
	GFX_AUTODETECT_FULLSCREEN,
	GFX_DIRECTX,
	GFX_DIRECTX_WIN,
	GFX_XDGA,
	GFX_XDGA_FULLSCREEN,
	GFX_XDGA2,
	GFX_XWINDOWS,
	GFX_XWINDOWS_FULLSCREEN
};

extern int SCREEN_W, SCREEN_H;

enum {
	SWITCH_BACKAMNESIA,
	SWITCH_BACKGROUND
};

int set_display_switch_mode(int mode);

bool gusExists(const std::string& filename);
bool gusExistsFile(const std::string& filename);
bool gusIsDirectory(const std::string& filename);

bool gusOpenGameFileR(std::ifstream& f, const std::string& path, std::ios_base::openmode mode = std::ios_base::in);
FILE* gusOpenGameFile(const std::string& path, const char *mode);


Iterator<std::string>::Ref gusFileListIter(
										const std::string& dir,
										bool absolutePath = false,
										const filemodes_t modefilter = -1);


Uint32 makecol(int r, int g, int b);
Uint32 makecol_depth(int color_depth, int r, int g, int b);

#define TRACE printf
#define allegro_message(str) notes("Allegro: " str "\n")

extern int _rgb_r_shift_15, _rgb_g_shift_15, _rgb_b_shift_15,
           _rgb_r_shift_16, _rgb_g_shift_16, _rgb_b_shift_16,
           _rgb_r_shift_24, _rgb_g_shift_24, _rgb_b_shift_24,
           _rgb_r_shift_32, _rgb_g_shift_32, _rgb_b_shift_32;

extern int _rgb_scale_5[32], _rgb_scale_6[64];

#define __INLINE__  static INLINE

__INLINE__ int getr16(int c)
{
	return _rgb_scale_5[(c >> _rgb_r_shift_16) & 0x1F];
}


__INLINE__ int getg16(int c)
{
	return _rgb_scale_6[(c >> _rgb_g_shift_16) & 0x3F];
}


__INLINE__ int getb16(int c)
{
	return _rgb_scale_5[(c >> _rgb_b_shift_16) & 0x1F];
}


__INLINE__ int makecol16(int r, int g, int b)
{
   return (((r >> 3) << _rgb_r_shift_16) |
           ((g >> 2) << _rgb_g_shift_16) |
           ((b >> 3) << _rgb_b_shift_16));
}

void rgb_to_hsv(int r, int g, int b, float *h, float *s, float *v);


struct RGB
{
	unsigned char r, g, b;
	unsigned char filler;
};


struct ALLEGRO_BITMAP            /* a bitmap structure */
	{
		SmartPointer<SDL_Surface> surf;
		
		// ALLEGRO_BITMAP itself
		int sub_x, sub_y;		// subpart (if you use create_sub_bitmap)
		int w, h;                     /* width and height in pixels */
		//int clip;                     /* flag if clipping is turned on */
		int cl, cr, ct, cb;           /* clip left, right, top and bottom values */
		

		//void (*write_bank)();         /* write bank selector, see bank.s */
		//void (*read_bank)();          /* read bank selector, see bank.s */
		//void *dat;                    /* the memory we allocated for the bitmap */
		//int bitmap_id;                /* for identifying sub-bitmaps */
		//void *extra;                  /* points to a structure with more info */
		//int line_ofs;                 /* line offset (for screen sub-bitmaps) */
		//int seg;                      /* bitmap segment */
		unsigned char **line;       /* pointers to the start of each line */
	};


SmartPointer<SDL_Surface> create_32bpp_sdlsurface__allegroformat(int w, int h);
SmartPointer<SDL_Surface> load_bitmap__allegroformat(const std::string& filename);
ALLEGRO_BITMAP *load_bitmap(const std::string& filename);
ALLEGRO_BITMAP *create_bitmap_from_sdl(const SmartPointer<SDL_Surface>& surf);
ALLEGRO_BITMAP *create_bitmap(int width, int height);
ALLEGRO_BITMAP *create_bitmap_ex(int color_depth, int width, int height);
ALLEGRO_BITMAP *create_sub_bitmap(ALLEGRO_BITMAP *parent, int x, int y, int width, int height);
ALLEGRO_BITMAP *create_copy_bitmap(ALLEGRO_BITMAP* other);
void destroy_bitmap(ALLEGRO_BITMAP *bitmap);


#define BYTES_PER_PIXEL(bpp)     (((int)(bpp) + 7) / 8)

__INLINE__ int bitmap_color_depth(ALLEGRO_BITMAP *bmp)
{
	return bmp->surf->format->BitsPerPixel;
}


#define BMP_TYPE_LINEAR    1     /* memory bitmaps, mode 13h, SVGA */
#define BMP_TYPE_PLANAR    2     /* mode-X bitmaps */

__INLINE__ int is_planar_bitmap(ALLEGRO_BITMAP *bmp)
{
	return true; // TODO ?
}

__INLINE__ int is_video_bitmap(ALLEGRO_BITMAP *bmp)
{
	return false; // TODO ?
}



int getpixel(ALLEGRO_BITMAP *bmp, int x, int y);
void putpixel(ALLEGRO_BITMAP *bmp, int x, int y, Uint32 color);
void vline(ALLEGRO_BITMAP *bmp, int x, int y1, int y2, Uint32 color);
void hline(ALLEGRO_BITMAP *bmp, int x1, int y, int x2, Uint32 color);
void line(ALLEGRO_BITMAP *bmp, int x1, int y1, int x2, int y2, Uint32 color);
void rectfill(ALLEGRO_BITMAP *bmp, int x1, int y1, int x2, int y2, Uint32 color);
void circle(ALLEGRO_BITMAP *bmp, int x, int y, int radius, Uint32 color);
void clear_to_color(struct ALLEGRO_BITMAP *bitmap, Uint32 color);
void draw_sprite(ALLEGRO_BITMAP *bmp, ALLEGRO_BITMAP *sprite, int x, int y);
void draw_sprite_h_flip(struct ALLEGRO_BITMAP *bmp, struct ALLEGRO_BITMAP *sprite, int x, int y);

void blit(ALLEGRO_BITMAP *source, ALLEGRO_BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height);
void stretch_blit(ALLEGRO_BITMAP *s, ALLEGRO_BITMAP *d, int s_x, int s_y, int s_w, int s_h, int d_x, int d_y, int d_w, int d_h);
void masked_blit(ALLEGRO_BITMAP *source, ALLEGRO_BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height);

void clear_bitmap(ALLEGRO_BITMAP*);



#if !defined(SDL_BYTEORDER)
#	error SDL_BYTEORDER not defined
#endif
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
#	define ALLEGRO_LITTLE_ENDIAN 1
#elif SDL_BYTEORDER == SDL_BIG_ENDIAN
#	define ALLEGRO_BIG_ENDIAN 1
#else
#	error unknown ENDIAN type
#endif


/* endian-independent 3-byte accessor macros */
#ifdef ALLEGRO_LITTLE_ENDIAN

   #define READ3BYTES(p)  ((*(unsigned char *)(p))               \
                           | (*((unsigned char *)(p) + 1) << 8)  \
                           | (*((unsigned char *)(p) + 2) << 16))

   #define WRITE3BYTES(p,c)  ((*(unsigned char *)(p) = (c)),             \
                              (*((unsigned char *)(p) + 1) = (c) >> 8),  \
                              (*((unsigned char *)(p) + 2) = (c) >> 16))

#elif defined ALLEGRO_BIG_ENDIAN

   #define READ3BYTES(p)  ((*(unsigned char *)(p) << 16)         \
                           | (*((unsigned char *)(p) + 1) << 8)  \
                           | (*((unsigned char *)(p) + 2)))

   #define WRITE3BYTES(p,c)  ((*(unsigned char *)(p) = (c) >> 16),       \
                              (*((unsigned char *)(p) + 1) = (c) >> 8),  \
                              (*((unsigned char *)(p) + 2) = (c)))

#else
   #error endianess not defined
#endif

/* generic versions of the video memory access helpers */
#ifndef bmp_select
#define bmp_select(bmp)
#endif

#ifndef bmp_write8
#define bmp_write8(addr, c)         (*((Uint8  *)(addr)) = (Uint8)(c))
#define bmp_write15(addr, c)        (*((Uint16 *)(addr)) = (Uint16)(c))
#define bmp_write16(addr, c)        (*((Uint16 *)(addr)) = (Uint16)(c))
#define bmp_write32(addr, c)        (*((Uint32 *)(addr)) = (c))

#define bmp_read8(addr)             (*((Uint8  *)(addr)))
#define bmp_read15(addr)            (*((Uint16 *)(addr)))
#define bmp_read16(addr)            (*((Uint16 *)(addr)))
#define bmp_read32(addr)            (*((Uint32 *)(addr)))

#ifndef AL_INLINE
//#define AL_INLINE(type, name, args, code)       type name args;
#define AL_INLINE(type, name, args, code)       static INLINE type name args code
#endif


AL_INLINE(int, bmp_read24, (unsigned long addr),
{
	unsigned char *p = (unsigned char *)addr;
	int c;
	
	c = READ3BYTES(p);
	
	return c;
})

AL_INLINE(void, bmp_write24, (unsigned long addr, int c),
{
	unsigned char *p = (unsigned char *)addr;
	
	WRITE3BYTES(p, c);
})

#endif

unsigned long bmp_write_line(ALLEGRO_BITMAP *bmp, int line);
void bmp_unwrite_line(ALLEGRO_BITMAP* bmp);



#define DRAW_MODE_SOLID             0        /* flags for drawing_mode() */
#define DRAW_MODE_XOR               1
#define DRAW_MODE_COPY_PATTERN      2
#define DRAW_MODE_SOLID_PATTERN     3
#define DRAW_MODE_MASKED_PATTERN    4
#define DRAW_MODE_TRANS             5

void drawing_mode(int mode, ALLEGRO_BITMAP *pattern, int x_anchor, int y_anchor);
void set_trans_blender(int r, int g, int b, int a);
void set_add_blender (int r, int g, int b, int a);
void solid_mode();

int getr(Uint32 c);
int getg(Uint32 c);
int getb(Uint32 c);

int get_color_conversion();
int get_color_depth();

struct LocalSetColorConversion
{
	LocalSetColorConversion(int flags);
	~LocalSetColorConversion();
private:
	int old;
};

struct LocalSetColorDepth
{
	LocalSetColorDepth(int depth);
	~LocalSetColorDepth();
private:
	int old;
};


void set_clip_rect(ALLEGRO_BITMAP *bitmap, int x1, int y_1, int x2, int y2);
void get_clip_rect(ALLEGRO_BITMAP *bitmap, int *x1, int *y_1, int *x2, int *y2);



extern "C" {
#endif // C++ only stuff

#include <stdio.h>
FILE* gusOpenGameFile(const char* path, const char *mode);

#ifdef __cplusplus
}
#endif

#endif
