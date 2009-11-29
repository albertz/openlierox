#ifndef DEDSERV

#include "blitters.h"
#include "colors.h"
#include "mmx.h"
#include "macros.h"

namespace Blitters
{

void rectfill_add_32_mmx(BITMAP* where, int x1, int y1, int x2, int y2, Pixel colour, int fact)
{
	typedef Pixel32 pixel_t_1;
	typedef Pixel32 pixel_t_2; // Doesn't really matter what type this is
	
	if(fact <= 0)
		return;

	CLIP_RECT();
	
	Pixel col = scaleColor_32(colour, fact);
	
	movd_rm(mm7, col);       //  -  col
	punpckldq_rr(mm7, mm7);  // col col
	
	// w00t, 14 pixels at a time

	RECT_Y_LOOP(
		//RECT_X_LOOP_ALIGN(14, 8,
		RECT_X_LOOP_NOALIGN(14,
			*p = addColorsCrude_32(*p, col)
		,
			movq_rm(mm0, p[0]);
			movq_rm(mm1, p[2]);
			movq_rm(mm2, p[4]);
			movq_rm(mm3, p[6]);
			movq_rm(mm4, p[8]);
			movq_rm(mm5, p[10]);
			movq_rm(mm6, p[12]);
			paddusb_rr(mm0, mm7);
			paddusb_rr(mm1, mm7);
			paddusb_rr(mm2, mm7);
			paddusb_rr(mm3, mm7);
			paddusb_rr(mm4, mm7);
			paddusb_rr(mm5, mm7);
			paddusb_rr(mm6, mm7);
			movq_mr(p[0], mm0);
			movq_mr(p[2], mm1);
			movq_mr(p[4], mm2);
			movq_mr(p[6], mm3);
			movq_mr(p[8], mm4);
			movq_mr(p[10], mm5);
			movq_mr(p[12], mm6);
		)
	)
	
	
	emms();
}

void hline_add_32_mmx(BITMAP* where, int x1, int y1, int x2, Pixel colour, int fact)
{
	typedef Pixel32 pixel_t_1;
	typedef Pixel32 pixel_t_2; // Doesn't really matter what type this is
	
	if(fact <= 0)
		return;
	Pixel col = scaleColor_32(colour, fact);
	
	movd_rm(mm7, col);       //  -  col
	punpckldq_rr(mm7, mm7);  // col col
	
	RECT_X_LOOP_NOALIGN(14,
		*p = addColorsCrude_32(*p, col)
	,
		movq_rm(mm0, p[0]);
		movq_rm(mm1, p[2]);
		movq_rm(mm2, p[4]);
		movq_rm(mm3, p[6]);
		movq_rm(mm4, p[8]);
		movq_rm(mm5, p[10]);
		movq_rm(mm6, p[12]);
		paddusb_rr(mm0, mm7);
		paddusb_rr(mm1, mm7);
		paddusb_rr(mm2, mm7);
		paddusb_rr(mm3, mm7);
		paddusb_rr(mm4, mm7);
		paddusb_rr(mm5, mm7);
		paddusb_rr(mm6, mm7);
		movq_mr(p[0], mm0);
		movq_mr(p[2], mm1);
		movq_mr(p[4], mm2);
		movq_mr(p[6], mm3);
		movq_mr(p[8], mm4);
		movq_mr(p[10], mm5);
		movq_mr(p[12], mm6);
	)

	emms();
}

void drawSprite_add_32_mmx_sse(BITMAP* where, BITMAP* from, int x, int y, int cutl, int cutt, int cutr, int cutb, int fact)
{
	typedef Pixel32 pixel_t_1;
	typedef Pixel32 pixel_t_2; // Doesn't really matter what type this is
	
	if(fact <= 0)
		return;
	
	CLIP_SPRITE_REGION();

	static unsigned long long rb_mask32 = 0x00FF00FF00FF00FFull;
	//static unsigned long long g_mask32  = 0x0000FF000000FF00ull;
	
	movq_rm(mm3, rb_mask32);

	if(fact >= 255)
	{
		SPRITE_Y_LOOP(
			SPRITE_X_LOOP_NOALIGN(8,
				Pixel s = *src;
				if(s != maskcolor_32)
					*dest = addColorsCrude_32(*dest, s)
			,
				//prefetchnta 32(%esi);
				//prefetcht0  %es:32(%edi);
				
#ifdef NDEBUG // GCC 4 has problems with these instructions with -O0 somewhy
				prefetchnta(src[8]);
				prefetcht0(dest[8]);
#endif
				movq_rm(mm4, src[0]);    // mm0 = src1 | src2
				movq_rm(mm5, src[2]);    // mm4 = src3 | src4
				movq_rm(mm6, src[4]);    // mm4 = src3 | src4
				movq_rm(mm7, src[6]);    // mm4 = src3 | src4
				
				movq_rr(mm0, mm4);
				movq_rr(mm1, mm5);
				movq_rr(mm2, mm6);

				pcmpeqd_rr(mm4, mm3);
				pcmpeqd_rr(mm5, mm3);
				pcmpeqd_rr(mm6, mm3);
				
				pandn_rr(mm4, mm0);
				pandn_rr(mm5, mm1);
				pandn_rr(mm6, mm2);
				
				// mask last
				movq_rr(mm0, mm7);
				pcmpeqd_rr(mm7, mm3);
				pandn_rr(mm7, mm0);
				
				movq_rm(mm0, dest[0]);
				movq_rm(mm1, dest[2]);
				movq_rm(mm2, dest[4]);
				movq_rm(mm3, dest[6]);
				
				paddusb_rr(mm4, mm0);
				paddusb_rr(mm5, mm1);
				paddusb_rr(mm6, mm2);
				paddusb_rr(mm7, mm3);
				
				movq_mr(dest[0], mm4);
				movq_mr(dest[2], mm5);
				movq_mr(dest[4], mm6);
				movq_mr(dest[6], mm7);
			)
		)
	}
	else
	{
		int fact2 = fact * 256;
		movd_rm(mm7, fact2);
		punpcklwd_rr(mm7, mm7); // 00000000ff00ff00
		punpcklwd_rr(mm7, mm7); // ff00ff00ff00ff00
		pxor_rr(mm6, mm6);
		
		SPRITE_Y_LOOP(
			SPRITE_X_LOOP_NOALIGN(4,
				Pixel s = *src;
				if(s != maskcolor_32)
					*dest = addColorsCrude_32(*dest, scaleColor_32(s, fact))
			,
				// Optimized from 39 to 26 instructions ;o
				
				movq_rm(mm0, src[0]);    // mm0 = src1 | src2
				movq_rm(mm4, src[2]);    // mm4 = src3 | src4
				
#ifdef NDEBUG // GCC 4 has problems with these instructions with -O0 somewhy
				prefetchnta(src[8]);
				prefetcht0(dest[8]);
#endif
				
				movq_rr(mm1, mm0);
				movq_rr(mm5, mm4);

				pcmpeqd_rr(mm0, mm3);
				pcmpeqd_rr(mm4, mm3);
				pandn_rr(mm0, mm1);
				pandn_rr(mm4, mm5);
				
				// mm0 and mm4 are masked
				
				// mm6 is 0
				movq_rr(mm1, mm0);
				movq_rr(mm5, mm4);
				punpckhbw_rr(mm0, mm6); // mm0 = src1 = 00rr00gg00bb
				punpckhbw_rr(mm4, mm6); // mm4 = src3 = 00rr00gg00bb
				punpcklbw_rr(mm1, mm6); // mm1 = src2 = 00rr00gg00bb
				punpcklbw_rr(mm5, mm6); // mm5 = src4 = 00rr00gg00bb
				
				pmulhuw_rr(mm0, mm7); // mm0 = src1 = 00rs00gs00bs
				pmulhuw_rr(mm4, mm7); // mm4 = src3 = 00rs00gs00bs
				pmulhuw_rr(mm1, mm7); // mm1 = src2 = 00rs00gs00bs
				pmulhuw_rr(mm5, mm7); // mm5 = src4 = 00rs00gs00bs
				
				packuswb_rr(mm1, mm0); // mm5 = src1 | src2
				packuswb_rr(mm5, mm4); // mm6 = src3 | src4

				movq_rm(mm0, dest[0]);
				movq_rm(mm4, dest[2]);
				
				paddusb_rr(mm1, mm0);
				paddusb_rr(mm5, mm4);
				
				movq_mr(dest[0], mm1);
				movq_mr(dest[2], mm5);
			)
		)
	}
	
	emms();
}

void drawSprite_add_16_mmx_sse(BITMAP* where, BITMAP* from, int x, int y, int cutl, int cutt, int cutr, int cutb, int fact)
{
	typedef Pixel16 pixel_t_1;
	typedef Pixel16 pixel_t_2; // Doesn't really matter what type this is
	
	if(fact < 4)
		return;
	
	CLIP_SPRITE_REGION();

	static unsigned long long RED = 0xF800F800F800F800ull;
	static unsigned long long GREEN = 0x07E007E007E007E0ull;
	static unsigned long long BLUE = 0x001F001F001F001Full;
	static unsigned long long MASK = 0xF81FF81FF81FF81Full;

	int scaledFact = (fact + 4) / 8;
	
	fact = scaledFact * 8;
	
	movd_rm(mm6, fact);
	punpcklwd_rr(mm6, mm6); // 00000000ff00ff00
	punpcklwd_rr(mm6, mm6); // ff00ff00ff00ff00
	
	movq_rm(mm7, MASK);

	SPRITE_Y_LOOP(
		SPRITE_X_LOOP_NOALIGN(4,
			Pixel s = *src;
			if(s != maskcolor_16)
				*dest = addColors_16_2(*dest, scaleColor_16(s, scaledFact))
		,
			//TODO: Rearrange
#ifdef NDEBUG // GCC 4 has problems with these instructions with -O0 somewhy
			prefetchnta(src[8]);
			prefetcht0(dest[8]);
#endif

			movq_rm(mm0, dest[0]);    // mm1 = dest1 | dest2 | dest3 | dest4
			movq_rm(mm3, src[0]);     // mm0 = src1 | src2 | src3 | src4
			
			movq_rr(mm1, mm3);
			pcmpeqw_rr(mm3, mm7);
			pandn_rr(mm3, mm1);
			
			movq_rr(mm1, mm0);
			movq_rr(mm2, mm0);
			pand_rm(mm0, RED);
			pand_rm(mm1, GREEN);
			pand_rm(mm2, BLUE);
			
			movq_rr(mm4, mm3);
			movq_rr(mm5, mm3);
			
			pand_rm(mm3, RED);
			pand_rm(mm4, GREEN);
			pand_rm(mm5, BLUE);
		
			psrlw_ri(mm3, 8);
			psrlw_ri(mm4, 3);
			psllw_ri(mm5, 3);
			pmullw_rr(mm3, mm6);
			pmullw_rr(mm4, mm6);
			pmullw_rr(mm5, mm6);
		
			psllw_ri(mm1, 5);
			psllw_ri(mm2, 11);
		
			paddusw_rr(mm0, mm3);
			paddusw_rr(mm1, mm4);
			paddusw_rr(mm2, mm5);
			psrlw_ri(mm1, 5);
			psrlw_ri(mm2, 11);
			
			pand_rm(mm0, RED);
			pand_rm(mm1, GREEN);
			pand_rm(mm2, BLUE);
		
			por_rr(mm0, mm1);
			por_rr(mm0, mm2);
			
			movq_mr(dest[0], mm0);
		)
	)

	emms();
}

void drawSpriteLine_add_8_mmx_sse(BITMAP* where, BITMAP* from, int x, int y, int x1, int y1, int x2, int fact)
{
	typedef Pixel8 pixel_t_1;
	typedef Pixel32 pixel_t_2; // Doesn't really matter what type this is
	
	if(bitmap_color_depth(from) != 8)
		return;
		
	CLIP_HLINE();

	if(fact >= 255)
	{
		SPRITE_X_LOOP_NOALIGN(16,
			*dest = addColorsCrude_8_4(*dest, *src)
		,
			//prefetchnta(src[8]);
			//prefetcht0(dest[8]);
			
			movq_rm(mm0, src[0]);
			movq_rm(mm1, src[2]);
			movq_rm(mm2, dest[0]);
			movq_rm(mm3, dest[2]);
			paddusb_rr(mm0, mm2);
			paddusb_rr(mm1, mm3);
			movq_mr(dest[0], mm0);
			movq_mr(dest[2], mm1);
		)
	}
	else
	{
		int fact2 = fact * 256;
		movd_rm(mm7, fact2);
		punpcklwd_rr(mm7, mm7); // 00000000ff00ff00
		punpcklwd_rr(mm7, mm7); // ff00ff00ff00ff00
		pxor_rr(mm6, mm6);

		SPRITE_X_LOOP_NOALIGN(16,
			*dest = addColorsCrude_8_4(*dest, scaleColor_8_4(*src, fact))
		,
			movq_rm(mm0, src[0]);    // mm0 = src1 | src2
			movq_rm(mm4, src[2]);    // mm4 = src3 | src4
			
			//prefetchnta(src[8]);
			//prefetcht0(dest[8]);
			
			movq_rr(mm1, mm0);
			movq_rr(mm5, mm4);
			punpckhbw_rr(mm0, mm6); // mm0 = src1 = 00rr00gg00bb
			punpckhbw_rr(mm4, mm6); // mm4 = src3 = 00rr00gg00bb
			punpcklbw_rr(mm1, mm6); // mm1 = src2 = 00rr00gg00bb
			punpcklbw_rr(mm5, mm6); // mm5 = src4 = 00rr00gg00bb
			
			pmulhuw_rr(mm0, mm7); // mm0 = src1 = 00rs00gs00bs
			pmulhuw_rr(mm4, mm7); // mm4 = src3 = 00rs00gs00bs
			pmulhuw_rr(mm1, mm7); // mm1 = src2 = 00rs00gs00bs
			pmulhuw_rr(mm5, mm7); // mm5 = src4 = 00rs00gs00bs
			
			packuswb_rr(mm1, mm0); // mm5 = src1 | src2
			packuswb_rr(mm5, mm4); // mm6 = src3 | src4
	
			movq_rm(mm0, dest[0]);
			movq_rm(mm4, dest[2]);
			
			paddusb_rr(mm1, mm0);
			paddusb_rr(mm5, mm4);
			
			movq_mr(dest[0], mm1);
			movq_mr(dest[2], mm5);
		)
	}

	emms();
}

}

#endif //DEDSERV
