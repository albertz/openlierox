#include <string.h>
#include "allegro.h"
#include "allegro/internal/aintern.h"
#include "2xsai.h"

#define uint32 unsigned long
#define uint16 unsigned short
#define uint8 unsigned char


static uint32 colorMask = 0xF7DEF7DE;
static uint32 lowPixelMask = 0x08210821;
static uint32 qcolorMask = 0xE79CE79C;
static uint32 qlowpixelMask = 0x18631863;
static uint32 redblueMask = 0xF81F;
static uint32 greenMask = 0x7E0;
static int PixelsPerMask = 2;
static int xsai_depth = 0;



int Init_2xSaI(int d)
{

	int minr = 0, ming = 0, minb = 0;
	int i;
	
	if (d != 15 && d != 16 && d != 24 && d != 32)
		return -1;

	/* Get lowest color bit */	
	for (i = 0; i < 255; i++) {
		if (!minr)
			minr = makecol(i, 0, 0);
		if (!ming)
			ming = makecol(0, i, 0);
		if (!minb)
			minb = makecol(0, 0, i);
	}

	colorMask = (makecol_depth(d, 255, 0, 0) - minr) | (makecol_depth(d, 0, 255, 0) - ming) | (makecol_depth(d, 0, 0, 255) - minb);
	lowPixelMask = minr | ming | minb;
	qcolorMask = (makecol_depth(d, 255, 0, 0) - 3 * minr) | (makecol_depth(d, 0, 255, 0) - 3 * ming) | (makecol_depth(d, 0, 0, 255) - 3 * minb);
	qlowpixelMask = (minr * 3) | (ming * 3) | (minb * 3);
	redblueMask = makecol_depth(d, 255, 0, 255);
	greenMask = makecol_depth(d, 0, 255, 0);

	PixelsPerMask = (d <= 16) ? 2 : 1;
	
	if (PixelsPerMask == 2) {
		colorMask |= (colorMask << 16);
		qcolorMask |= (qcolorMask << 16);
		lowPixelMask |= (lowPixelMask << 16);
		qlowpixelMask |= (qlowpixelMask << 16);
	}

	TRACE("Color Mask:       0x%lX\n", colorMask);
	TRACE("Low Pixel Mask:   0x%lX\n", lowPixelMask);
	TRACE("QColor Mask:      0x%lX\n", qcolorMask);
	TRACE("QLow Pixel Mask:  0x%lX\n", qlowpixelMask);
	
	xsai_depth = d;

	return 0;
}


static int GetResult1(uint32 A, uint32 B, uint32 C, uint32 D)
{
	int x = 0;
	int y = 0;
	int r = 0;
	if (A == C)
		x += 1;
	else if (B == C)
		y += 1;
	if (A == D)
		x += 1;
	else if (B == D)
		y += 1;
	if (x <= 1)
		r += 1;
	if (y <= 1)
		r -= 1;
	return r;
}

static int GetResult2(uint32 A, uint32 B, uint32 C, uint32 D, uint32 E)
{
	int x = 0;
	int y = 0;
	int r = 0;
	if (A == C)
		x += 1;
	else if (B == C)
		y += 1;
	if (A == D)
		x += 1;
	else if (B == D)
		y += 1;
	if (x <= 1)
		r -= 1;
	if (y <= 1)
		r += 1;
	return r;
}


#define GET_RESULT(A, B, C, D) ((A != C || A != D) - (B != C || B != D))

#define INTERPOLATE(A, B) (((A & colorMask) >> 1) + ((B & colorMask) >> 1) + (A & B & lowPixelMask))

#define Q_INTERPOLATE(A, B, C, D) ((A & qcolorMask) >> 2) + ((B & qcolorMask) >> 2) + ((C & qcolorMask) >> 2) + ((D & qcolorMask) >> 2) \
	+ ((((A & qlowpixelMask) + (B & qlowpixelMask) + (C & qlowpixelMask) + (D & qlowpixelMask)) >> 2) & qlowpixelMask)


/* Clipping Macro, stolen from Allegro, modified to work with 2xSaI */
#define BLIT_CLIP2(src, dest, s_x, s_y, d_x, d_y, w, h, xscale, yscale)      \
   /* check for ridiculous cases */                                          \
   if ((s_x >= src->cr) || (s_y >= src->cb) ||                               \
       (d_x >= dest->cr) || (d_y >= dest->cb))                               \
      return;                                                                \
                                                                             \
   if ((s_x + w < src->cl) || (s_y + h < src->ct) ||                         \
       (d_x + w * xscale < dest->cl) || (d_y + h * yscale < dest->ct))       \
      return;                                                                \
                                                                             \
   if (xscale < 1 || yscale < 1)                                             \
      return;                                                                \
                                                                             \
   /* clip src left */                                                       \
   if (s_x < src->cl) {                                                      \
      w += s_x;                                                              \
      d_x -= s_x * xscale;                                                   \
      s_x = src->cl;                                                         \
   }                                                                         \
                                                                             \
   /* clip src top */                                                        \
   if (s_y < src->ct) {                                                      \
      h += s_y;                                                              \
      d_y -= s_y * yscale;                                                   \
      s_y = src->ct;                                                         \
   }                                                                         \
                                                                             \
   /* clip src right */                                                      \
   if (s_x + w > src->cr)                                                    \
      w = src->cr - s_x;                                                     \
                                                                             \
   /* clip src bottom */                                                     \
   if (s_y + h > src->cb)                                                    \
      h = src->cb - s_y;                                                     \
                                                                             \
   /* clip dest left */                                                      \
   if (d_x < dest->cl) {                                                     \
      d_x -= dest->cl;                                                       \
      w += d_x / xscale;                                                     \
      s_x -= d_x / xscale;                                                   \
      d_x = dest->cl;                                                        \
   }                                                                         \
                                                                             \
   /* clip dest top */                                                       \
   if (d_y < dest->ct) {                                                     \
      d_y -= dest->ct;                                                       \
      h += d_y / yscale;                                                     \
      s_y -= d_y / yscale;                                                   \
      d_y = dest->ct;                                                        \
   }                                                                         \
                                                                             \
   /* clip dest right */                                                     \
   if (d_x + w * xscale > dest->cr)                                          \
      w = (dest->cr - d_x) / xscale;                                         \
                                                                             \
   /* clip dest bottom */                                                    \
   if (d_y + h * yscale > dest->cb)                                          \
      h = (dest->cb - d_y) / yscale;                                         \
                                                                             \
   /* bottle out if zero size */                                             \
   if ((w <= 0) || (h <= 0))                                                 \
      return;


//static unsigned char *src_line[4];
//static unsigned char *dst_line[2];


void Super2xSaI(BITMAP * src, BITMAP * dest, int s_x, int s_y, int d_x, int d_y, int w, int h)
{
	int sbpp, dbpp;

	BITMAP *dst2 = NULL;

	if (!src || !dest)
		return;

	sbpp = bitmap_color_depth(src);
	dbpp = bitmap_color_depth(dest);

	if ((sbpp != xsai_depth) || (sbpp != dbpp))	/* Must be same color depth */
		return;

	BLIT_CLIP2(src, dest, s_x, s_y, d_x, d_y, w, h, 2, 2);	
		
	if (w < 4 || h < 4) {  /* Image is too small to be 2xSaI'ed. */
		stretch_blit(src, dest, s_x, s_y, w, h, d_x, d_y, w * 2, h * 2);
		return;
	}	
	
	sbpp = BYTES_PER_PIXEL(sbpp);
	if (d_x || d_y)
		dst2 = create_sub_bitmap(dest, d_x, d_y, w * 2, h * 2);
	
	Super2xSaI_ex(src->line[s_y] + s_x * sbpp, (unsigned int)(src->line[1] - src->line[0]), NULL, dst2 ? dst2 : dest, w, h);
	
	if (dst2)
		destroy_bitmap(dst2);
	
	return;
}

void Super2xSaI_ex(uint8 *src, uint32 src_pitch, uint8 *unused, BITMAP *dest, uint32 width, uint32 height) {

	int j, v;
	unsigned int x, y;
	int sbpp = BYTES_PER_PIXEL(bitmap_color_depth(dest));
	unsigned long color[16];
	unsigned char **src_line = new unsigned char*[4];
	unsigned char **dst_line = new unsigned char*[2];

	/* Point to the first 3 lines. */
	src_line[0] = src;
	src_line[1] = src;
	src_line[2] = src + src_pitch;
	src_line[3] = src + src_pitch * 2;
	
	/* Can we write the results directly? */
	if (is_video_bitmap(dest) || is_planar_bitmap(dest)) {
		//dst_line[0] = malloc(sizeof(char) * sbpp * width);
		//dst_line[1] = malloc(sizeof(char) * sbpp * width);
		dst_line[0] = new unsigned char[sbpp*width*2];
		dst_line[1] = new unsigned char[sbpp*width*2];
		v = 1;
	}
	else {
		dst_line[0] = dest->line[0];
		dst_line[1] = dest->line[1];
		v = 0;
	}
	
	/* Set destination */
	bmp_select(dest);

	x = 0, y = 0;
	
	if (PixelsPerMask == 2) {
		unsigned short *sbp;
		sbp = (unsigned short*)src_line[0];
		color[0] = *sbp;       color[1] = color[0];   color[2] = color[0];    color[3] = color[0];
		color[4] = color[0];   color[5] = color[0];   color[6] = *(sbp + 1);  color[7] = *(sbp + 2);
		sbp = (unsigned short*)src_line[2];
		color[8] = *sbp;     color[9] = color[8];     color[10] = *(sbp + 1); color[11] = *(sbp + 2);
		sbp = (unsigned short*)src_line[3];
		color[12] = *sbp;    color[13] = color[12];   color[14] = *(sbp + 1); color[15] = *(sbp + 2);
	}
	else {
		unsigned long *lbp;
		lbp = (unsigned long*)src_line[0];
		color[0] = *lbp;       color[1] = color[0];   color[2] = color[0];    color[3] = color[0];
		color[4] = color[0];   color[5] = color[0];   color[6] = *(lbp + 1);  color[7] = *(lbp + 2);
		lbp = (unsigned long*)src_line[2];
		color[8] = *lbp;     color[9] = color[8];     color[10] = *(lbp + 1); color[11] = *(lbp + 2);
		lbp = (unsigned long*)src_line[3];
		color[12] = *lbp;    color[13] = color[12];   color[14] = *(lbp + 1); color[15] = *(lbp + 2);
	}

	for (y = 0; y < height; y++) {
	
		/* Todo: x = width - 2, x = width - 1 */
		
		for (x = 0; x < width; x++) {
			unsigned long product1a, product1b, product2a, product2b;

//---------------------------------------  B0 B1 B2 B3    0  1  2  3
//                                         4  5* 6  S2 -> 4  5* 6  7
//                                         1  2  3  S1    8  9 10 11
//                                         A0 A1 A2 A3   12 13 14 15
//--------------------------------------
			if (color[9] == color[6] && color[5] != color[10]) {
				product2b = color[9];
				product1b = product2b;
			}
			else if (color[5] == color[10] && color[9] != color[6]) {
				product2b = color[5];
				product1b = product2b;
			}
			else if (color[5] == color[10] && color[9] == color[6]) {
				int r = 0;

				r += GET_RESULT(color[6], color[5], color[8], color[13]);
				r += GET_RESULT(color[6], color[5], color[4], color[1]);
				r += GET_RESULT(color[6], color[5], color[14], color[11]);
				r += GET_RESULT(color[6], color[5], color[2], color[7]);

				if (r > 0)
					product1b = color[6];
				else if (r < 0)
					product1b = color[5];
				else
					product1b = INTERPOLATE(color[5], color[6]);
					
				product2b = product1b;

			}
			else {
				if (color[6] == color[10] && color[10] == color[13] && color[9] != color[14] && color[10] != color[12])
					product2b = Q_INTERPOLATE(color[10], color[10], color[10], color[9]);
				else if (color[5] == color[9] && color[9] == color[14] && color[13] != color[10] && color[9] != color[15])
					product2b = Q_INTERPOLATE(color[9], color[9], color[9], color[10]);
				else
					product2b = INTERPOLATE(color[9], color[10]);

				if (color[6] == color[10] && color[6] == color[1] && color[5] != color[2] && color[6] != color[0])
					product1b = Q_INTERPOLATE(color[6], color[6], color[6], color[5]);
				else if (color[5] == color[9] && color[5] == color[2] && color[1] != color[6] && color[5] != color[3])
					product1b = Q_INTERPOLATE(color[6], color[5], color[5], color[5]);
				else
					product1b = INTERPOLATE(color[5], color[6]);
			}

			if (color[5] == color[10] && color[9] != color[6] && color[4] == color[5] && color[5] != color[14])
				product2a = INTERPOLATE(color[9], color[5]);
			else if (color[5] == color[8] && color[6] == color[5] && color[4] != color[9] && color[5] != color[12])
				product2a = INTERPOLATE(color[9], color[5]);
			else
				product2a = color[9];

			if (color[9] == color[6] && color[5] != color[10] && color[8] == color[9] && color[9] != color[2])
				product1a = INTERPOLATE(color[9], color[5]);
			else if (color[4] == color[9] && color[10] == color[9] && color[8] != color[5] && color[9] != color[0])
				product1a = INTERPOLATE(color[9], color[5]);
			else
				product1a = color[5];
	
			if (PixelsPerMask == 2) {
				*((unsigned long *) (&dst_line[0][x * 4])) = product1a | (product1b << 16);
				*((unsigned long *) (&dst_line[1][x * 4])) = product2a | (product2b << 16);
			}
			else {
				*((unsigned long *) (&dst_line[0][x * 8])) = product1a;
				*((unsigned long *) (&dst_line[0][x * 8 + 4])) = product1b;
				*((unsigned long *) (&dst_line[1][x * 8])) = product2a;
				*((unsigned long *) (&dst_line[1][x * 8 + 4])) = product2b;
			}
			
			/* Move color matrix forward */
			color[0] = color[1]; color[4] = color[5]; color[8] = color[9];   color[12] = color[13];
			color[1] = color[2]; color[5] = color[6]; color[9] = color[10];  color[13] = color[14];
			color[2] = color[3]; color[6] = color[7]; color[10] = color[11]; color[14] = color[15];
			
			if (x < width - 3) {
				x += 3;
				if (PixelsPerMask == 2) {
					color[3] = *(((unsigned short*)src_line[0]) + x);					
					color[7] = *(((unsigned short*)src_line[1]) + x);
					color[11] = *(((unsigned short*)src_line[2]) + x);
					color[15] = *(((unsigned short*)src_line[3]) + x);
				}
				else {
					color[3] = *(((unsigned long*)src_line[0]) + x);
					color[7] = *(((unsigned long*)src_line[1]) + x);
					color[11] = *(((unsigned long*)src_line[2]) + x);
					color[15] = *(((unsigned long*)src_line[3]) + x);
				}
				x -= 3;
			}
		}

		/* We're done with one line, so we shift the source lines up */
		src_line[0] = src_line[1];
		src_line[1] = src_line[2];
		src_line[2] = src_line[3];		

		/* Read next line */
		if (y + 3 >= height)
			src_line[3] = src_line[2];
		else
			src_line[3] = src_line[2] + src_pitch;
			
		/* Then shift the color matrix up */
		if (PixelsPerMask == 2) {
			unsigned short *sbp;
			sbp = (unsigned short*)src_line[0];
			color[0] = *sbp;     color[1] = color[0];    color[2] = *(sbp + 1);  color[3] = *(sbp + 2);
			sbp = (unsigned short*)src_line[1];
			color[4] = *sbp;     color[5] = color[4];    color[6] = *(sbp + 1);  color[7] = *(sbp + 2);
			sbp = (unsigned short*)src_line[2];
			color[8] = *sbp;     color[9] = color[9];    color[10] = *(sbp + 1); color[11] = *(sbp + 2);
			sbp = (unsigned short*)src_line[3];
			color[12] = *sbp;    color[13] = color[12];  color[14] = *(sbp + 1); color[15] = *(sbp + 2);
		}
		else {
			unsigned long *lbp;
			lbp = (unsigned long*)src_line[0];
			color[0] = *lbp;     color[1] = color[0];    color[2] = *(lbp + 1);  color[3] = *(lbp + 2);
			lbp = (unsigned long*)src_line[1];
			color[4] = *lbp;     color[5] = color[4];    color[6] = *(lbp + 1);  color[7] = *(lbp + 2);
			lbp = (unsigned long*)src_line[2];
			color[8] = *lbp;     color[9] = color[9];    color[10] = *(lbp + 1); color[11] = *(lbp + 2);
			lbp = (unsigned long*)src_line[3];
			color[12] = *lbp;    color[13] = color[12];  color[14] = *(lbp + 1); color[15] = *(lbp + 2);
		}
		
		
		/* Write the 2 lines, if not already done so */
		if (v) {
			unsigned long dst_addr;
		
			dst_addr = bmp_write_line(dest, y * 2);
			for (j = 0; j < dest->w * sbpp; (j += sizeof(long)), (dst_addr += sizeof(long)))
				bmp_write32(dst_addr, *((unsigned long *) (dst_line[0] + j)));
			
			
			dst_addr = bmp_write_line(dest, y * 2 + 1);
			for (j = 0; j < dest->w * sbpp; (j += sizeof(long)), (dst_addr += sizeof(long)))
				bmp_write32(dst_addr, *((unsigned long *) (dst_line[1] + j)));
		}
		else {
			if (y < height - 1) {
				dst_line[0] = dest->line[y * 2 + 2];
				dst_line[1] = dest->line[y * 2 + 3];
			}
		}
	}
	bmp_unwrite_line(dest);

	if (v) {
		delete[] dst_line[0];
		delete[] dst_line[1];
	}

	delete[] src_line;
	delete[] dst_line;

}



void SuperEagle(BITMAP * src, BITMAP * dest, int s_x, int s_y, int d_x, int d_y, int w, int h)
{
	int sbpp, dbpp;

	BITMAP *dst2 = NULL;

	if (!src || !dest)
		return;

	sbpp = bitmap_color_depth(src);
	dbpp = bitmap_color_depth(dest);

	if ((sbpp != xsai_depth) || (sbpp != dbpp))	/* Must be same color depth */
		return;

	BLIT_CLIP2(src, dest, s_x, s_y, d_x, d_y, w, h, 2, 2);	
		
	if (w < 4 || h < 4) {  /* Image is too small to be 2xSaI'ed. */
		stretch_blit(src, dest, s_x, s_y, w, h, d_x, d_y, w * 2, h * 2);
		return;
	}	
	
	sbpp = BYTES_PER_PIXEL(sbpp);
	if (d_x || d_y)
		dst2 = create_sub_bitmap(dest, d_x, d_y, w * 2, h * 2);
	
	SuperEagle_ex(src->line[s_y] + s_x * sbpp, (unsigned int)(src->line[1] - src->line[0]), NULL, dst2 ? dst2 : dest, w, h);
	
	if (dst2)
		destroy_bitmap(dst2);
	
	return;
}

void SuperEagle_ex(uint8 *src, uint32 src_pitch, uint8 *unused, BITMAP *dest, uint32 width, uint32 height) {

	int j, v;
	unsigned int x, y;
	int sbpp = BYTES_PER_PIXEL(bitmap_color_depth(dest));
	unsigned long color[12];
	unsigned char **src_line = new unsigned char*[4];
	unsigned char **dst_line = new unsigned char*[2];

	/* Point to the first 3 lines. */
	src_line[0] = src;
	src_line[1] = src;
	src_line[2] = src + src_pitch;
	src_line[3] = src + src_pitch * 2;
	
	/* Can we write the results directly? */
	if (is_video_bitmap(dest) || is_planar_bitmap(dest)) {
		//dst_line[0] = malloc(sizeof(char) * sbpp * width);
		//dst_line[1] = malloc(sizeof(char) * sbpp * width);
		dst_line[0] = new unsigned char[sbpp*width];
		dst_line[1] = new unsigned char[sbpp*width];
		v = 1;
	}
	else {
		dst_line[0] = dest->line[0];
		dst_line[1] = dest->line[1];
		v = 0;
	}
	
	/* Set destination */
	bmp_select(dest);

	x = 0, y = 0;
	
	if (PixelsPerMask == 2) {
		unsigned short *sbp;
		sbp = (unsigned short*)src_line[0];
		color[0] = *sbp;       color[1] = color[0];   color[2] = color[0];    color[3] = color[0];
		color[4] = *(sbp + 1); color[5] = *(sbp + 2);
		sbp = (unsigned short*)src_line[2];
		color[6] = *sbp;     color[7] = color[6];     color[8] = *(sbp + 1); color[9] = *(sbp + 2);
		sbp = (unsigned short*)src_line[3];
		color[10] = *sbp;    color[11] = *(sbp + 1); 
	}
	else {
		unsigned long *lbp;
		lbp = (unsigned long*)src_line[0];
		color[0] = *lbp;       color[1] = color[0];   color[2] = color[0];    color[3] = color[0];
		color[4] = *(lbp + 1); color[5] = *(lbp + 2);
		lbp = (unsigned long*)src_line[2];
		color[6] = *lbp;     color[7] = color[6];     color[8] = *(lbp + 1); color[9] = *(lbp + 2);
		lbp = (unsigned long*)src_line[3];
		color[10] = *lbp;    color[11] = *(lbp + 1);
	}

	for (y = 0; y < height; y++) {
	
		/* Todo: x = width - 2, x = width - 1 */
		
		for (x = 0; x < width; x++) {
			unsigned long product1a, product1b, product2a, product2b;

//---------------------------------------     B1 B2           0  1
//                                         4  5  6  S2 ->  2  3  4  5
//                                         1  2  3  S1     6  7  8  9
//                                            A1 A2          10 11

			if (color[7] == color[4] && color[3] != color[8]) {
				product1b = product2a = color[7];

				if ((color[6] == color[7]) || (color[4] == color[1]))
					product1a = INTERPOLATE(color[7], INTERPOLATE(color[7], color[3]));
				else
					product1a = INTERPOLATE(color[3], color[4]);

				if ((color[4] == color[5]) || (color[7] == color[10]))
					product2b = INTERPOLATE(color[7], INTERPOLATE(color[7], color[8]));
				else
					product2b = INTERPOLATE(color[7], color[8]);
			}
			else if (color[3] == color[8] && color[7] != color[4]) {
				product2b = product1a = color[3];

				if ((color[0] == color[3]) || (color[5] == color[9]))
					product1b = INTERPOLATE(color[3], INTERPOLATE(color[3], color[4]));
				else
					product1b = INTERPOLATE(color[3], color[1]);

				if ((color[8] == color[11]) || (color[2] == color[3]))
					product2a = INTERPOLATE(color[3], INTERPOLATE(color[3], color[2]));
				else
					product2a = INTERPOLATE(color[7], color[8]);

			}
			else if (color[3] == color[8] && color[7] == color[4]) {
				register int r = 0;

				r += GET_RESULT(color[4], color[3], color[6], color[10]);
				r += GET_RESULT(color[4], color[3], color[2], color[0]);
				r += GET_RESULT(color[4], color[3], color[11], color[9]);
				r += GET_RESULT(color[4], color[3], color[1], color[5]);

				if (r > 0) {
					product1b = product2a = color[7];
					product1a = product2b = INTERPOLATE(color[3], color[4]);
				}
				else if (r < 0) {
					product2b = product1a = color[3];
					product1b = product2a = INTERPOLATE(color[3], color[4]);
				}
				else {
					product2b = product1a = color[3];
					product1b = product2a = color[7];
				}
			}
			else {
				product2b = product1a = INTERPOLATE(color[7], color[4]);
				product2b = Q_INTERPOLATE(color[8], color[8], color[8], product2b);
				product1a = Q_INTERPOLATE(color[3], color[3], color[3], product1a);

				product2a = product1b = INTERPOLATE(color[3], color[8]);
				product2a = Q_INTERPOLATE(color[7], color[7], color[7], product2a);
				product1b = Q_INTERPOLATE(color[4], color[4], color[4], product1b);
			}

			if (PixelsPerMask == 2) {
				*((unsigned long *) (&dst_line[0][x * 4])) = product1a | (product1b << 16);
				*((unsigned long *) (&dst_line[1][x * 4])) = product2a | (product2b << 16);
			}
			else {
				*((unsigned long *) (&dst_line[0][x * 8])) = product1a;
				*((unsigned long *) (&dst_line[0][x * 8 + 4])) = product1b;
				*((unsigned long *) (&dst_line[1][x * 8])) = product2a;
				*((unsigned long *) (&dst_line[1][x * 8 + 4])) = product2b;
			}
			
			/* Move color matrix forward */
			color[0] = color[1]; 
			color[2] = color[3]; color[3] = color[4]; color[4] = color[5];
			color[6] = color[7]; color[7] = color[8]; color[8] = color[9]; 
			color[10] = color[11];
			
			if (x < width - 2) {
				x += 2;
				if (PixelsPerMask == 2) {
					color[1] = *(((unsigned short*)src_line[0]) + x);
					if (x < width) {
						color[5] = *(((unsigned short*)src_line[1]) + x + 1);
						color[9] = *(((unsigned short*)src_line[2]) + x + 1);
					}
					color[11] = *(((unsigned short*)src_line[3]) + x);
				}
				else {
					color[1] = *(((unsigned long*)src_line[0]) + x);
					if (x < width) {
						color[5] = *(((unsigned long*)src_line[1]) + x + 1);
						color[9] = *(((unsigned long*)src_line[2]) + x + 1);
					}
					color[11] = *(((unsigned long*)src_line[3]) + x);
				}
				x -= 2;
			}
		}

		/* We're done with one line, so we shift the source lines up */
		src_line[0] = src_line[1];
		src_line[1] = src_line[2];
		src_line[2] = src_line[3];		

		/* Read next line */
		if (y + 3 >= height)
			src_line[3] = src_line[2];
		else
			src_line[3] = src_line[2] + src_pitch;
			
		/* Then shift the color matrix up */
		if (PixelsPerMask == 2) {
			unsigned short *sbp;
			sbp = (unsigned short*)src_line[0];
			color[0] = *sbp;     color[1] = *(sbp + 1);
			sbp = (unsigned short*)src_line[1];
			color[2] = *sbp;     color[3] = color[2];    color[4] = *(sbp + 1);  color[5] = *(sbp + 2);
			sbp = (unsigned short*)src_line[2];
			color[6] = *sbp;     color[7] = color[6];    color[8] = *(sbp + 1);  color[9] = *(sbp + 2);
			sbp = (unsigned short*)src_line[3];
			color[10] = *sbp;    color[11] = *(sbp + 1);
		}
		else {
			unsigned long *lbp;
			lbp = (unsigned long*)src_line[0];
			color[0] = *lbp;     color[1] = *(lbp + 1);
			lbp = (unsigned long*)src_line[1];
			color[2] = *lbp;     color[3] = color[2];    color[4] = *(lbp + 1);  color[5] = *(lbp + 2);
			lbp = (unsigned long*)src_line[2];
			color[6] = *lbp;     color[7] = color[6];    color[8] = *(lbp + 1);  color[9] = *(lbp + 2);
			lbp = (unsigned long*)src_line[3];
			color[10] = *lbp;    color[11] = *(lbp + 1);
		}


		/* Write the 2 lines, if not already done so */
		if (v) {
			unsigned long dst_addr;
		
			dst_addr = bmp_write_line(dest, y * 2);
			for (j = 0; j < dest->w * sbpp; j += sizeof(long))
				bmp_write32(dst_addr + j, *((unsigned long *) (dst_line[0] + j)));
				
			dst_addr = bmp_write_line(dest, y * 2 + 1);
			for (j = 0; j < dest->w * sbpp; j += sizeof(long))
				bmp_write32(dst_addr + j, *((unsigned long *) (dst_line[1] + j)));
		}
		else {
			if (y < height - 1) {
				dst_line[0] = dest->line[y * 2 + 2];
				dst_line[1] = dest->line[y * 2 + 3];
			}
		}
	}
	bmp_unwrite_line(dest);
	
	if (v) {
		delete dst_line[0];
		delete dst_line[1];
	}
	delete[] src_line;
	delete[] dst_line;

}
