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

#include <SDL.h>
#include <cstdio>

extern int allegro_error;

void allegro_init();
void allegro_exit();
#define END_OF_MAIN extern void ___foo_allegro

#define END_OF_FUNCTION(x)
#define LOCK_VARIABLE(x)
#define LOCK_FUNCTION(x)

void rest(int t);
void vsync();

void install_timer();


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

struct BITMAP;
extern BITMAP* screen;


void acquire_screen();
void release_screen();

enum {
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

int set_gfx_mode(int card, int w, int h, int v_w, int v_h);
extern int SCREEN_W, SCREEN_H;

enum {
	SWITCH_BACKAMNESIA,
	SWITCH_BACKGROUND
};

int set_display_switch_mode(int mode);

bool exists(const char* filename);


#define PACKFILE FILE
#define pack_fread(buf, size, f) fread(buf, size, 1, f)

void register_datafile_object(int id, void *(*load)(PACKFILE *f, long size), void (*destroy)(void *data));

#define DAT_ID(a,b,c,d)    ((a<<24) | (b<<16) | (c<<8) | d)




int makecol(int r, int g, int b);
//int makecol8(int r, int g, int b);
int makecol_depth(int color_depth, int r, int g, int b);

#define TRACE printf
#define allegro_message printf

extern int _rgb_r_shift_15, _rgb_g_shift_15, _rgb_b_shift_15,
           _rgb_r_shift_16, _rgb_g_shift_16, _rgb_b_shift_16,
           _rgb_r_shift_24, _rgb_g_shift_24, _rgb_b_shift_24,
           _rgb_r_shift_32, _rgb_g_shift_32, _rgb_b_shift_32;

extern int _rgb_scale_5[32], _rgb_scale_6[64];

#define __INLINE__  static inline

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

struct BITMAP;

struct GFX_VTABLE        /* functions for drawing onto bitmaps */
	{
		int bitmap_type;
		int color_depth;
		int mask_color;
		
		int  (*getpixel)(struct BITMAP *bmp, int x, int y);
		void (*putpixel)(struct BITMAP *bmp, int x, int y, int color);
		void (*vline)(struct BITMAP *bmp, int x, int y1, int y2, int color);
		void (*hline)(struct BITMAP *bmp, int x1, int y, int x2, int color);
		void (*line)(struct BITMAP *bmp, int x1, int y1, int x2, int y2, int color);
		void (*rectfill)(struct BITMAP *bmp, int x1, int y1, int x2, int y2, int color);
		void (*draw_sprite)(struct BITMAP *bmp, struct BITMAP *sprite, int x, int y);
		void (*draw_256_sprite)(struct BITMAP *bmp, struct BITMAP *sprite, int x, int y);
		void (*draw_sprite_v_flip)(struct BITMAP *bmp, struct BITMAP *sprite, int x, int y);
		void (*draw_sprite_h_flip)(struct BITMAP *bmp, struct BITMAP *sprite, int x, int y);
		void (*draw_sprite_vh_flip)(struct BITMAP *bmp, struct BITMAP *sprite, int x, int y);
		void (*draw_trans_sprite)(struct BITMAP *bmp, struct BITMAP *sprite, int x, int y);
		void (*draw_lit_sprite)(struct BITMAP *bmp, struct BITMAP *sprite, int x, int y, int color);
		void (*draw_rle_sprite)(struct BITMAP *bmp, struct RLE_SPRITE *sprite, int x, int y);
		void (*draw_trans_rle_sprite)(struct BITMAP *bmp, struct RLE_SPRITE *sprite, int x, int y);
		void (*draw_lit_rle_sprite)(struct BITMAP *bmp, struct RLE_SPRITE *sprite, int x, int y, int color);
		void (*draw_character)(struct BITMAP *bmp, struct BITMAP *sprite, int x, int y, int color);
		void (*textout_fixed)(struct BITMAP *bmp, void *f, int h, unsigned char *str, int x, int y, int color);
		void (*blit_from_memory)(struct BITMAP *source, struct BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height);
		void (*blit_to_memory)(struct BITMAP *source, struct BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height);
		void (*blit_to_self)(struct BITMAP *source, struct BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height);
		void (*blit_to_self_forward)(struct BITMAP *source, struct BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height);
		void (*blit_to_self_backward)(struct BITMAP *source, struct BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height);
		void (*masked_blit)(struct BITMAP *source, struct BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height);
		void (*clear_to_color)(struct BITMAP *bitmap, int color);
		void (*draw_sprite_end)(void);
		void (*blit_end)(void);
	};

struct BITMAP            /* a bitmap structure */
	{
		int w, h;                     /* width and height in pixels */
		int clip;                     /* flag if clipping is turned on */
		int cl, cr, ct, cb;           /* clip left, right, top and bottom values */
		GFX_VTABLE *vtable;				/* drawing functions */
		void (*write_bank)();         /* write bank selector, see bank.s */
		void (*read_bank)();          /* read bank selector, see bank.s */
		void *dat;                    /* the memory we allocated for the bitmap */
		int bitmap_id;                /* for identifying sub-bitmaps */
		void *extra;                  /* points to a structure with more info */
		int line_ofs;                 /* line offset (for screen sub-bitmaps) */
		int seg;                      /* bitmap segment */
		unsigned char *line[0];       /* pointers to the start of each line */
	};


BITMAP *load_bitmap(const char *filename, RGB *pal);
BITMAP *create_bitmap(int width, int height);
BITMAP *create_bitmap_ex(int color_depth, int width, int height);
BITMAP *create_sub_bitmap(BITMAP *parent, int x, int y, int width, int height);
void destroy_bitmap(BITMAP *bitmap);

void register_bitmap_file_type(char *ext, BITMAP *(*load)(const char *filename, RGB *pal), int (*save)(const char *filename, BITMAP *bmp, const RGB *pal));


#define BYTES_PER_PIXEL(bpp)     (((int)(bpp) + 7) / 8)

__INLINE__ int bitmap_color_depth(BITMAP *bmp)
{
	return bmp->vtable->color_depth;
}


#define BMP_TYPE_LINEAR    1     /* memory bitmaps, mode 13h, SVGA */
#define BMP_TYPE_PLANAR    2     /* mode-X bitmaps */

__INLINE__ int is_planar_bitmap(BITMAP *bmp)
{
	return (bmp->vtable->bitmap_type == BMP_TYPE_PLANAR);
}

__INLINE__ int is_video_bitmap(BITMAP *bmp)
{
	return false; // TODO ?
}


void circle(BITMAP *bmp, int x, int y, int radius, int color);
void clear_to_color(struct BITMAP *bitmap, int color);
void draw_sprite_h_flip(struct BITMAP *bmp, struct BITMAP *sprite, int x, int y);

void blit(BITMAP *source, BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height);
void stretch_blit(BITMAP *s, BITMAP *d, int s_x, int s_y, int s_w, int s_h, int d_x, int d_y, int d_w, int d_h);
void masked_blit(BITMAP *source, BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height);

void clear_bitmap(BITMAP*);


#ifndef AL_INLINE
   #define AL_INLINE(type, name, args, code)       type name args;
#endif


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
#define bmp_write8(addr, c)         (*((unsigned char  *)(addr)) = (c))
#define bmp_write15(addr, c)        (*((unsigned short *)(addr)) = (c))
#define bmp_write16(addr, c)        (*((unsigned short *)(addr)) = (c))
#define bmp_write32(addr, c)        (*((unsigned long  *)(addr)) = (c))

#define bmp_read8(addr)             (*((unsigned char  *)(addr)))
#define bmp_read15(addr)            (*((unsigned short *)(addr)))
#define bmp_read16(addr)            (*((unsigned short *)(addr)))
#define bmp_read32(addr)            (*((unsigned long  *)(addr)))

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

unsigned long bmp_write_line(BITMAP *bmp, int line);
void bmp_unwrite_line(BITMAP* bmp);



#define DRAW_MODE_SOLID             0        /* flags for drawing_mode() */
#define DRAW_MODE_XOR               1
#define DRAW_MODE_COPY_PATTERN      2
#define DRAW_MODE_SOLID_PATTERN     3
#define DRAW_MODE_MASKED_PATTERN    4
#define DRAW_MODE_TRANS             5

void drawing_mode(int mode, BITMAP *pattern, int x_anchor, int y_anchor);
void set_trans_blender(int r, int g, int b, int a);
void set_add_blender (int r, int g, int b, int a);
void solid_mode();

int getr(int c);
int getg(int c);
int getb(int c);

int get_color_conversion();
void set_color_conversion(int mode);

int get_color_depth();
void set_color_depth(int depth);


void set_clip_rect(BITMAP *bitmap, int x1, int y_1, int x2, int y2);
void get_clip_rect(BITMAP *bitmap, int *x1, int *y_1, int *x2, int *y2);


__INLINE__ void vline(BITMAP *bmp, int x, int y1, int y2, int color)
{
	bmp->vtable->vline(bmp, x, y1, y2, color);
}

__INLINE__ void hline(BITMAP *bmp, int x1, int y, int x2, int color)
{
	bmp->vtable->hline(bmp, x1, y, x2, color);
}

__INLINE__ void line(BITMAP *bmp, int x1, int y1, int x2, int y2, int color)
{
	bmp->vtable->line(bmp, x1, y1, x2, y2, color);
}

__INLINE__ void rectfill(BITMAP *bmp, int x1, int y1, int x2, int y2, int color)
{
	bmp->vtable->rectfill(bmp, x1, y1, x2, y2, color);
}

__INLINE__ void draw_sprite(BITMAP *bmp, BITMAP *sprite, int x, int y)
{
	if (sprite->vtable->color_depth == 8)
		bmp->vtable->draw_256_sprite(bmp, sprite, x, y);
	else
		bmp->vtable->draw_sprite(bmp, sprite, x, y);
}

__INLINE__ int getpixel(BITMAP *bmp, int x, int y)
{
	return bmp->vtable->getpixel(bmp, x, y);
}

__INLINE__ void putpixel(BITMAP *bmp, int x, int y, int color)
{
	bmp->vtable->putpixel(bmp, x, y, color);
}









void clear_keybuf();

enum {
	__allegro_KEY_A            = 1,
	__allegro_KEY_B            = 2,
	__allegro_KEY_C            = 3,
	__allegro_KEY_D            = 4,
	__allegro_KEY_E            = 5,
	__allegro_KEY_F            = 6,
	__allegro_KEY_G            = 7,
	__allegro_KEY_H            = 8,
	__allegro_KEY_I            = 9,
	__allegro_KEY_J            = 10,
	__allegro_KEY_K            = 11,
	__allegro_KEY_L            = 12,
	__allegro_KEY_M            = 13,
	__allegro_KEY_N            = 14,
	__allegro_KEY_O            = 15,
	__allegro_KEY_P            = 16,
	__allegro_KEY_Q            = 17,
	__allegro_KEY_R            = 18,
	__allegro_KEY_S            = 19,
	__allegro_KEY_T            = 20,
	__allegro_KEY_U            = 21,
	__allegro_KEY_V            = 22,
	__allegro_KEY_W            = 23,
	__allegro_KEY_X            = 24,
	__allegro_KEY_Y            = 25,
	__allegro_KEY_Z            = 26,
	__allegro_KEY_0            = 27,
	__allegro_KEY_1            = 28,
	__allegro_KEY_2            = 29,
	__allegro_KEY_3            = 30,
	__allegro_KEY_4            = 31,
	__allegro_KEY_5            = 32,
	__allegro_KEY_6            = 33,
	__allegro_KEY_7            = 34,
	__allegro_KEY_8            = 35,
	__allegro_KEY_9            = 36,
	__allegro_KEY_0_PAD        = 37,
	__allegro_KEY_1_PAD        = 38,
	__allegro_KEY_2_PAD        = 39,
	__allegro_KEY_3_PAD        = 40,
	__allegro_KEY_4_PAD        = 41,
	__allegro_KEY_5_PAD        = 42,
	__allegro_KEY_6_PAD        = 43,
	__allegro_KEY_7_PAD        = 44,
	__allegro_KEY_8_PAD        = 45,
	__allegro_KEY_9_PAD        = 46,
	__allegro_KEY_F1           = 47,
	__allegro_KEY_F2           = 48,
	__allegro_KEY_F3           = 49,
	__allegro_KEY_F4           = 50,
	__allegro_KEY_F5           = 51,
	__allegro_KEY_F6           = 52,
	__allegro_KEY_F7           = 53,
	__allegro_KEY_F8           = 54,
	__allegro_KEY_F9           = 55,
	__allegro_KEY_F10          = 56,
	__allegro_KEY_F11          = 57,
	__allegro_KEY_F12          = 58,
	__allegro_KEY_ESC          = 59,
	__allegro_KEY_TILDE        = 60,
	__allegro_KEY_MINUS        = 61,
	__allegro_KEY_EQUALS       = 62,
	__allegro_KEY_BACKSPACE    = 63,
	__allegro_KEY_TAB          = 64,
	__allegro_KEY_OPENBRACE    = 65,
	__allegro_KEY_CLOSEBRACE   = 66,
	__allegro_KEY_ENTER        = 67,
	__allegro_KEY_COLON        = 68,
	__allegro_KEY_QUOTE        = 69,
	__allegro_KEY_BACKSLASH    = 70,
	__allegro_KEY_BACKSLASH2   = 71,
	__allegro_KEY_COMMA        = 72,
	__allegro_KEY_STOP         = 73,
	__allegro_KEY_SLASH        = 74,
	__allegro_KEY_SPACE        = 75,
	__allegro_KEY_INSERT       = 76,
	__allegro_KEY_DEL          = 77,
	__allegro_KEY_HOME         = 78,
	__allegro_KEY_END          = 79,
	__allegro_KEY_PGUP         = 80,
	__allegro_KEY_PGDN         = 81,
	__allegro_KEY_LEFT         = 82,
	__allegro_KEY_RIGHT        = 83,
	__allegro_KEY_UP           = 84,
	__allegro_KEY_DOWN         = 85,
	__allegro_KEY_SLASH_PAD    = 86,
	__allegro_KEY_ASTERISK     = 87,
	__allegro_KEY_MINUS_PAD    = 88,
	__allegro_KEY_PLUS_PAD     = 89,
	__allegro_KEY_DEL_PAD      = 90,
	__allegro_KEY_ENTER_PAD    = 91,
	__allegro_KEY_PRTSCR       = 92,
	__allegro_KEY_PAUSE        = 93,
	__allegro_KEY_ABNT_C1      = 94,
	__allegro_KEY_YEN          = 95,
	__allegro_KEY_KANA         = 96,
	__allegro_KEY_CONVERT      = 97,
	__allegro_KEY_NOCONVERT    = 98,
	__allegro_KEY_AT           = 99,
	__allegro_KEY_CIRCUMFLEX   = 100,
	__allegro_KEY_COLON2       = 101,
	__allegro_KEY_KANJI        = 102,
	__allegro_KEY_EQUALS_PAD   = 103,  /* MacOS X */
	__allegro_KEY_BACKQUOTE    = 104,  /* MacOS X */
	__allegro_KEY_SEMICOLON    = 105,  /* MacOS X */
	__allegro_KEY_COMMAND      = 106,  /* MacOS X */
	__allegro_KEY_UNKNOWN1     = 107,
	__allegro_KEY_UNKNOWN2     = 108,
	__allegro_KEY_UNKNOWN3     = 109,
	__allegro_KEY_UNKNOWN4     = 110,
	__allegro_KEY_UNKNOWN5     = 111,
	__allegro_KEY_UNKNOWN6     = 112,
	__allegro_KEY_UNKNOWN7     = 113,
	__allegro_KEY_UNKNOWN8     = 114,
	
	__allegro_KEY_MODIFIERS    = 115,
	
	__allegro_KEY_LSHIFT       = 115,
	__allegro_KEY_RSHIFT       = 116,
	__allegro_KEY_LCONTROL     = 117,
	__allegro_KEY_RCONTROL     = 118,
	__allegro_KEY_ALT          = 119,
	__allegro_KEY_ALTGR        = 120,
	__allegro_KEY_LWIN         = 121,
	__allegro_KEY_RWIN         = 122,
	__allegro_KEY_MENU         = 123,
	__allegro_KEY_SCRLOCK      = 124,
	__allegro_KEY_NUMLOCK      = 125,
	__allegro_KEY_CAPSLOCK     = 126,
	
	__allegro_KEY_MAX          = 127
};

#define KB_SHIFT_FLAG         __allegro_KB_SHIFT_FLAG
#define KB_CTRL_FLAG          __allegro_KB_CTRL_FLAG
#define KB_ALT_FLAG           __allegro_KB_ALT_FLAG
#define KB_LWIN_FLAG          __allegro_KB_LWIN_FLAG
#define KB_RWIN_FLAG          __allegro_KB_RWIN_FLAG
#define KB_MENU_FLAG          __allegro_KB_MENU_FLAG
#define KB_COMMAND_FLAG       __allegro_KB_COMMAND_FLAG
#define KB_SCROLOCK_FLAG      __allegro_KB_SCROLOCK_FLAG
#define KB_NUMLOCK_FLAG       __allegro_KB_NUMLOCK_FLAG
#define KB_CAPSLOCK_FLAG      __allegro_KB_CAPSLOCK_FLAG
#define KB_INALTSEQ_FLAG      __allegro_KB_INALTSEQ_FLAG
#define KB_ACCENT1_FLAG       __allegro_KB_ACCENT1_FLAG
#define KB_ACCENT2_FLAG       __allegro_KB_ACCENT2_FLAG
#define KB_ACCENT3_FLAG       __allegro_KB_ACCENT3_FLAG
#define KB_ACCENT4_FLAG       __allegro_KB_ACCENT4_FLAG

#define KEY_A                 __allegro_KEY_A
#define KEY_B                 __allegro_KEY_B
#define KEY_C                 __allegro_KEY_C
#define KEY_D                 __allegro_KEY_D
#define KEY_E                 __allegro_KEY_E
#define KEY_F                 __allegro_KEY_F
#define KEY_G                 __allegro_KEY_G
#define KEY_H                 __allegro_KEY_H
#define KEY_I                 __allegro_KEY_I
#define KEY_J                 __allegro_KEY_J
#define KEY_K                 __allegro_KEY_K
#define KEY_L                 __allegro_KEY_L
#define KEY_M                 __allegro_KEY_M
#define KEY_N                 __allegro_KEY_N
#define KEY_O                 __allegro_KEY_O
#define KEY_P                 __allegro_KEY_P
#define KEY_Q                 __allegro_KEY_Q
#define KEY_R                 __allegro_KEY_R
#define KEY_S                 __allegro_KEY_S
#define KEY_T                 __allegro_KEY_T
#define KEY_U                 __allegro_KEY_U
#define KEY_V                 __allegro_KEY_V
#define KEY_W                 __allegro_KEY_W
#define KEY_X                 __allegro_KEY_X
#define KEY_Y                 __allegro_KEY_Y
#define KEY_Z                 __allegro_KEY_Z
#define KEY_0                 __allegro_KEY_0
#define KEY_1                 __allegro_KEY_1
#define KEY_2                 __allegro_KEY_2
#define KEY_3                 __allegro_KEY_3
#define KEY_4                 __allegro_KEY_4
#define KEY_5                 __allegro_KEY_5
#define KEY_6                 __allegro_KEY_6
#define KEY_7                 __allegro_KEY_7
#define KEY_8                 __allegro_KEY_8
#define KEY_9                 __allegro_KEY_9
#define KEY_0_PAD             __allegro_KEY_0_PAD
#define KEY_1_PAD             __allegro_KEY_1_PAD
#define KEY_2_PAD             __allegro_KEY_2_PAD
#define KEY_3_PAD             __allegro_KEY_3_PAD
#define KEY_4_PAD             __allegro_KEY_4_PAD
#define KEY_5_PAD             __allegro_KEY_5_PAD
#define KEY_6_PAD             __allegro_KEY_6_PAD
#define KEY_7_PAD             __allegro_KEY_7_PAD
#define KEY_8_PAD             __allegro_KEY_8_PAD
#define KEY_9_PAD             __allegro_KEY_9_PAD
#define KEY_F1                __allegro_KEY_F1
#define KEY_F2                __allegro_KEY_F2
#define KEY_F3                __allegro_KEY_F3
#define KEY_F4                __allegro_KEY_F4
#define KEY_F5                __allegro_KEY_F5
#define KEY_F6                __allegro_KEY_F6
#define KEY_F7                __allegro_KEY_F7
#define KEY_F8                __allegro_KEY_F8
#define KEY_F9                __allegro_KEY_F9
#define KEY_F10               __allegro_KEY_F10
#define KEY_F11               __allegro_KEY_F11
#define KEY_F12               __allegro_KEY_F12
#define KEY_ESC               __allegro_KEY_ESC
#define KEY_TILDE             __allegro_KEY_TILDE
#define KEY_MINUS             __allegro_KEY_MINUS
#define KEY_EQUALS            __allegro_KEY_EQUALS
#define KEY_BACKSPACE         __allegro_KEY_BACKSPACE
#define KEY_TAB               __allegro_KEY_TAB
#define KEY_OPENBRACE         __allegro_KEY_OPENBRACE
#define KEY_CLOSEBRACE        __allegro_KEY_CLOSEBRACE
#define KEY_ENTER             __allegro_KEY_ENTER
#define KEY_COLON             __allegro_KEY_COLON
#define KEY_QUOTE             __allegro_KEY_QUOTE
#define KEY_BACKSLASH         __allegro_KEY_BACKSLASH
#define KEY_BACKSLASH2        __allegro_KEY_BACKSLASH2
#define KEY_COMMA             __allegro_KEY_COMMA
#define KEY_STOP              __allegro_KEY_STOP
#define KEY_SLASH             __allegro_KEY_SLASH
#define KEY_SPACE             __allegro_KEY_SPACE
#define KEY_INSERT            __allegro_KEY_INSERT
#define KEY_DEL               __allegro_KEY_DEL
#define KEY_HOME              __allegro_KEY_HOME
#define KEY_END               __allegro_KEY_END
#define KEY_PGUP              __allegro_KEY_PGUP
#define KEY_PGDN              __allegro_KEY_PGDN
#define KEY_LEFT              __allegro_KEY_LEFT
#define KEY_RIGHT             __allegro_KEY_RIGHT
#define KEY_UP                __allegro_KEY_UP
#define KEY_DOWN              __allegro_KEY_DOWN
#define KEY_SLASH_PAD         __allegro_KEY_SLASH_PAD
#define KEY_ASTERISK          __allegro_KEY_ASTERISK
#define KEY_MINUS_PAD         __allegro_KEY_MINUS_PAD
#define KEY_PLUS_PAD          __allegro_KEY_PLUS_PAD
#define KEY_DEL_PAD           __allegro_KEY_DEL_PAD
#define KEY_ENTER_PAD         __allegro_KEY_ENTER_PAD
#define KEY_PRTSCR            __allegro_KEY_PRTSCR
#define KEY_PAUSE             __allegro_KEY_PAUSE
#define KEY_ABNT_C1           __allegro_KEY_ABNT_C1
#define KEY_YEN               __allegro_KEY_YEN
#define KEY_KANA              __allegro_KEY_KANA
#define KEY_CONVERT           __allegro_KEY_CONVERT
#define KEY_NOCONVERT         __allegro_KEY_NOCONVERT
#define KEY_AT                __allegro_KEY_AT
#define KEY_CIRCUMFLEX        __allegro_KEY_CIRCUMFLEX
#define KEY_COLON2            __allegro_KEY_COLON2
#define KEY_KANJI             __allegro_KEY_KANJI
#define KEY_EQUALS_PAD        __allegro_KEY_EQUALS_PAD
#define KEY_BACKQUOTE         __allegro_KEY_BACKQUOTE
#define KEY_SEMICOLON         __allegro_KEY_SEMICOLON
#define KEY_COMMAND           __allegro_KEY_COMMAND
#define KEY_UNKNOWN1          __allegro_KEY_UNKNOWN1
#define KEY_UNKNOWN2          __allegro_KEY_UNKNOWN2
#define KEY_UNKNOWN3          __allegro_KEY_UNKNOWN3
#define KEY_UNKNOWN4          __allegro_KEY_UNKNOWN4
#define KEY_UNKNOWN5          __allegro_KEY_UNKNOWN5
#define KEY_UNKNOWN6          __allegro_KEY_UNKNOWN6
#define KEY_UNKNOWN7          __allegro_KEY_UNKNOWN7
#define KEY_UNKNOWN8          __allegro_KEY_UNKNOWN8

#define KEY_MODIFIERS         __allegro_KEY_MODIFIERS

#define KEY_LSHIFT            __allegro_KEY_LSHIFT
#define KEY_RSHIFT            __allegro_KEY_RSHIFT
#define KEY_LCONTROL          __allegro_KEY_LCONTROL
#define KEY_RCONTROL          __allegro_KEY_RCONTROL
#define KEY_ALT               __allegro_KEY_ALT
#define KEY_ALTGR             __allegro_KEY_ALTGR
#define KEY_LWIN              __allegro_KEY_LWIN
#define KEY_RWIN              __allegro_KEY_RWIN
#define KEY_MENU              __allegro_KEY_MENU
#define KEY_SCRLOCK           __allegro_KEY_SCRLOCK
#define KEY_NUMLOCK           __allegro_KEY_NUMLOCK
#define KEY_CAPSLOCK          __allegro_KEY_CAPSLOCK

#define KEY_MAX               __allegro_KEY_MAX


void install_keyboard();
void remove_keyboard();
bool keypressed();
int readkey();
extern int key[KEY_MAX];



#endif
