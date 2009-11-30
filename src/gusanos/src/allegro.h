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

int makecol(int r, int g, int b);
//int makecol8(int r, int g, int b);
int makecol_depth(int color_depth, int r, int g, int b);

#define TRACE printf

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


BITMAP *create_bitmap(int width, int height);
BITMAP *create_sub_bitmap(BITMAP *parent, int x, int y, int width, int height);
void destroy_bitmap(BITMAP *bitmap);


#define BYTES_PER_PIXEL(bpp)     (((int)(bpp) + 7) / 8)

static inline int bitmap_color_depth(BITMAP *bmp)
{
	return bmp->vtable->color_depth;
}


#define BMP_TYPE_LINEAR    1     /* memory bitmaps, mode 13h, SVGA */
#define BMP_TYPE_PLANAR    2     /* mode-X bitmaps */

static inline int is_planar_bitmap(BITMAP *bmp)
{
	return (bmp->vtable->bitmap_type == BMP_TYPE_PLANAR);
}

static inline int is_video_bitmap(BITMAP *bmp)
{
	return false; // TODO ?
}

void stretch_blit(BITMAP *s, BITMAP *d, int s_x, int s_y, int s_w, int s_h, int d_x, int d_y, int d_w, int d_h);
void masked_blit(BITMAP *source, BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height);


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



static inline void hline(BITMAP *bmp, int x1, int y, int x2, int color)
{
	bmp->vtable->hline(bmp, x1, y, x2, color);
}

static inline void line(BITMAP *bmp, int x1, int y1, int x2, int y2, int color)
{
	bmp->vtable->line(bmp, x1, y1, x2, y2, color);
}

static inline void rectfill(BITMAP *bmp, int x1, int y1, int x2, int y2, int color)
{
	bmp->vtable->rectfill(bmp, x1, y1, x2, y2, color);
}

static inline void draw_sprite(BITMAP *bmp, BITMAP *sprite, int x, int y)
{
	if (sprite->vtable->color_depth == 8)
		bmp->vtable->draw_256_sprite(bmp, sprite, x, y);
	else
		bmp->vtable->draw_sprite(bmp, sprite, x, y);
}

#endif
