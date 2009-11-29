#ifndef DEDSERV

#include "blitters.h"
#include "colors.h"
#include "mmx.h"
#include "macros.h"

namespace Blitters
{
	
void drawSprite_blendalpha_32_to_32_mmx_sse(BITMAP* where, BITMAP* from, int x, int y, int cutl, int cutt, int cutr, int cutb, int fact)
{
	typedef Pixel32 pixel_t_1;
	typedef Pixel32 pixel_t_2; // Doesn't really matter what type this is
	
	if(fact <= 0)
		return;
	
	CLIP_SPRITE_REGION();
	
	int fact2 = fact * 256;
	movd_rm(mm6, fact2);
	punpcklwd_rr(mm6, mm6); // 00000000ff00ff00
	punpcklwd_rr(mm6, mm6); // ff00ff00ff00ff00
	
	static unsigned long long fact32;
	
	movq_mr(fact32, mm6);
	
	SPRITE_Y_LOOP(
		SPRITE_X_LOOP_NOALIGN(4,
			Pixel s = *src;
			*dest = blendColorsFact_32(*dest, s, (((s >> 24) * fact) >> 8))
		,
#ifdef NDEBUG // GCC 4 has problems with these instructions with -O0 somewhy
			prefetchnta(src[8]);
			prefetcht0(dest[8]);
#endif
			
			movq_rm(mm0, src[0]);    // mm0 = src1 | src2
			movq_rm(mm1, dest[0]);   // mm1 = dest1 | dest2
			    		
			// Do src1/2 - dest1/2
			
			pxor_rr(mm7, mm7);

			movq_rr(mm2, mm0);
		
			punpcklbw_rr(mm0, mm7); // mm0 = src2 = 00aa00rr00gg00bb
			punpckhbw_rr(mm2, mm7); // mm2 = src1 = 00aa00rr00gg00bb
			
			movq_rr(mm3, mm1);
			movq_rr(mm5, mm1);
		
			punpcklbw_rr(mm1, mm7); // mm1 = dest2 = 00rr00gg00bb
			punpckhbw_rr(mm3, mm7); // mm3 = dest1 = 00rr00gg00bb

			pshufw_rri(mm4, mm0, 0xFF); // mm4 = src2 = 00aa00aa00aa00aa
			pshufw_rri(mm7, mm2, 0xFF); // mm7 = src1 = 00aa00aa00aa00aa

			psubw_rr(mm0, mm1);
			psubw_rr(mm2, mm3);

			pmulhuw_rr(mm4, mm6); // mm4 = src2 = 00as00as00as00as
			pmulhuw_rr(mm7, mm6); // mm7 = src1 = 00as00as00as00as
			
			pmullw_rr(mm0, mm4);
			pmullw_rr(mm2, mm7);
			psrlw_ri(mm0, 8);
			psrlw_ri(mm2, 8);
			
			packuswb_rr(mm0, mm2);  // mm0 = scaled1 | scaled2
			
			movq_rm(mm2, src[2]);    // mm1 = src3 | src4
			movq_rm(mm3, dest[2]);   // mm3 = dest3 | dest4
			
			paddb_rr(mm0, mm5);
			
			// Do src3/4 - dest3/4
			
			pxor_rr(mm7, mm7);
			
			movq_mr(dest[0], mm0);

			movq_rr(mm0, mm2);
			
			punpcklbw_rr(mm2, mm7); // mm2 = src4 = 00aa00rr00gg00bb
			punpckhbw_rr(mm0, mm7); // mm0 = src3 = 00aa00rr00gg00bb
			
			movq_rr(mm1, mm3);
			movq_rr(mm5, mm3);
			
			punpcklbw_rr(mm3, mm7); // mm3 = dest4 = 00rr00gg00bb
			punpckhbw_rr(mm1, mm7); // mm1 = dest3 = 00rr00gg00bb

			pshufw_rri(mm4, mm2, 0xFF); // mm4 = src4 = 00aa00aa00aa00aa
			pshufw_rri(mm7, mm0, 0xFF); // mm7 = src3 = 00aa00aa00aa00aa

			psubw_rr(mm2, mm3);
			psubw_rr(mm0, mm1);

			pmulhuw_rr(mm4, mm6); // mm4 = src4 = 00as00as00as00as
			pmulhuw_rr(mm7, mm6); // mm7 = src3 = 00as00as00as00as

			pmullw_rr(mm2, mm4);
			pmullw_rr(mm0, mm7);
			psrlw_ri(mm2, 8);
			psrlw_ri(mm0, 8);

			packuswb_rr(mm2, mm0);  // mm1 = scaled3 | scaled4
			
			paddb_rr(mm2, mm5);

			movq_mr(dest[2], mm2);
		)
	)

	emms();
}

void drawSprite_blend_32_mmx_sse(BITMAP* where, BITMAP* from, int x, int y, int cutl, int cutt, int cutr, int cutb, int fact)
{
	typedef Pixel32 pixel_t_1;
	typedef Pixel32 pixel_t_2; // Doesn't really matter what type this is
	
	if(fact <= 0)
		return;
	
	CLIP_SPRITE_REGION();

	static unsigned long long rb_mask32 = 0x00FF00FF00FF00FFull;
	//static unsigned long long g_mask32  = 0x0000FF000000FF00ull;
	
	//movq_rm(mm3, rb_mask32);

	if(fact >= 127 && fact <= 128)
	{
		SPRITE_Y_LOOP(
			SPRITE_X_LOOP_NOALIGN(4,
				Pixel s = *src;
				if(s != maskcolor_32)
					*dest = blendColorsHalfCrude_32(*dest, s)
			,
#ifdef NDEBUG // GCC 4 has problems with these instructions with -O0 somewhy
				prefetchnta(src[8]);
				prefetcht0(dest[8]);
#endif
				
				movq_rm(mm0, src[0]);    // mm0 = src1 | src2
				movq_rm(mm1, src[2]);    // mm1 = src3 | src4
				
				movq_rm(mm2, dest[0]);   // mm2 = dest1 | dest2
				movq_rm(mm3, dest[2]);   // mm3 = dest3 | dest4
				
				movq_rr(mm4, mm0);
				movq_rr(mm5, mm1);
				
				pcmpeqd_rm(mm0, rb_mask32);
				pcmpeqd_rm(mm1, rb_mask32);
				movq_rr(mm6, mm2);
				movq_rr(mm7, mm3);
				pand_rr(mm6, mm0);
				pand_rr(mm7, mm1);
				pandn_rr(mm0, mm4);
				pandn_rr(mm1, mm5);
				por_rr(mm0, mm6);
	    		por_rr(mm1, mm7);
				
				
				pavgb_rr(mm0, mm2);
				pavgb_rr(mm1, mm3);
				
				movq_mr(dest[0], mm0);
				movq_mr(dest[2], mm1);
			)
		)
	}
	else
	{
		movd_rm(mm5, fact);
		punpcklwd_rr(mm5, mm5); // 0000000000ff00ff
		punpcklwd_rr(mm5, mm5); // 00ff00ff00ff00ff
		
		static unsigned long long fact32;
		
		movq_mr(fact32, mm5);

		SPRITE_Y_LOOP(
			SPRITE_X_LOOP_NOALIGN(4,
				Pixel s = *src;
				if(s != maskcolor_32)
					*dest = blendColorsFact_32(*dest, s, fact)
			,
#ifdef NDEBUG // GCC 4 has problems with these instructions with -O0 somewhy
				prefetchnta(src[8]);
				prefetcht0(dest[8]);
#endif
				
				movq_rm(mm0, src[0]);    // mm0 = src1 | src2
				movq_rm(mm1, src[2]);    // mm1 = src3 | src4
				
				movq_rm(mm2, dest[0]);   // mm2 = dest1 | dest2
				movq_rm(mm3, dest[2]);   // mm3 = dest3 | dest4
				
				movq_rr(mm4, mm0);
				movq_rr(mm5, mm1);

				// Can this be optimized?
				pcmpeqd_rm(mm0, rb_mask32);
				pcmpeqd_rm(mm1, rb_mask32);
				movq_rr(mm6, mm2);
				movq_rr(mm7, mm3);
				pand_rr(mm6, mm0);
				pand_rr(mm7, mm1);
				pandn_rr(mm0, mm4);
				pandn_rr(mm1, mm5);
				por_rr(mm0, mm6);
	    		por_rr(mm1, mm7);
	    		
				pxor_rr(mm7, mm7);

				// Do src1/2 - dest1/2
				
				movq_rr(mm4, mm0);
			
				punpcklbw_rr(mm0, mm7); // mm0 = src2 = 00rr00gg00bb
				punpckhbw_rr(mm4, mm7); // mm4 = src1 = 00rr00gg00bb
				
				movq_rr(mm6, mm2);
				movq_rr(mm5, mm2);
			
				punpcklbw_rr(mm2, mm7); // mm2 = dest2 = 00rr00gg00bb
				punpckhbw_rr(mm6, mm7); // mm6 = dest1 = 00rr00gg00bb
				
				psubw_rr(mm0, mm2);
				psubw_rr(mm4, mm6);
				
				movq_rm(mm6, fact32);
				
				pmullw_rr(mm0, mm6);
				pmullw_rr(mm4, mm6);
				psrlw_ri(mm0, 8);
				psrlw_ri(mm4, 8);
				
				packuswb_rr(mm0, mm4);  // mm0 = scaled1 | scaled2
				
				paddb_rr(mm0, mm5);
				
				// Do src3/4 - dest3/4
				
				movq_rr(mm4, mm1);
				
				punpcklbw_rr(mm1, mm7); // mm1 = src4 = 00rr00gg00bb
				punpckhbw_rr(mm4, mm7); // mm4 = src3 = 00rr00gg00bb
				
				movq_rr(mm2, mm3);
				movq_rr(mm5, mm3);
				
				punpcklbw_rr(mm3, mm7); // mm3 = dest4 = 00rr00gg00bb
				punpckhbw_rr(mm2, mm7); // mm6 = dest3 = 00rr00gg00bb
				
				psubw_rr(mm1, mm3);
				psubw_rr(mm4, mm2);
				
				pmullw_rr(mm1, mm6);
				pmullw_rr(mm4, mm6);
				psrlw_ri(mm1, 8);
				psrlw_ri(mm4, 8);

				packuswb_rr(mm1, mm4);  // mm1 = scaled3 | scaled4
				
				paddb_rr(mm1, mm5);

				movq_mr(dest[0], mm0);
				movq_mr(dest[2], mm1);
				
			)
		)
	}
	
	emms();
}

/*
void drawSprite_blendtint_8_to_32_sse_amd(BITMAP* where, BITMAP* from, int x, int y, int cutl, int cutt, int cutr, int cutb, int fact, int color)
{
	typedef Pixel32 pixel_t_1;
	typedef Pixel32 pixel_t_2; // Doesn't really matter what type this is
	
	if(fact <= 0)
		return;
	
	CLIP_SPRITE_REGION();

	static unsigned long long rb_mask32 = 0x00FF00FF00FF00FFull;
	//static unsigned long long g_mask32  = 0x0000FF000000FF00ull;
	
	movq_rm(mm6, fact);
	punpcklwd_rr(mm6, mm5); // 0000000000ff00ff
	punpcklwd_rr(mm6, mm5); // 00ff00ff00ff00ff
	
	
	
	#define SHUF_MASK(a, b, c, d) ((a << 6) | (b << 4) | (c << 2) | d)

	if(fact >= 255)
	{
		pxor_rr(mm7, mm7);
		movq_rm(mm6, color);
		movq_rr(mm5, mm6);
		punpckldq_rr(mm5, mm5);
		punpcklbw_rr(mm6, mm7); // mm6 = 00rr00gg00bb
		SPRITE_Y_LOOP(
			SPRITE_X_LOOP_NOALIGN(4,
				Pixel s = *src;
				if(s != maskcolor_32)
					*dest = blendColorsHalfCrude_32(*dest, s)
			,
				prefetchnta(src[8]);
				prefetcht0(dest[8]);
				movd_rm(mm0, src[0]);
				movq_rm(mm1, dest[0]);
				punpcklbw_rr(mm0, mm7); // mm0 = src0-3 = 00pa00pb00pc00pd
				movq_rr(mm2, mm1);
				punpcklbw_rr(mm1, mm7); // mm1 = dest2 = 00rr00gg00bb
				punpckhbw_rr(mm2, mm7); // mm2 = dest1 = 00rr00gg00bb
				
				pshufw_rri(mm3, mm0, SHUF_MASK(3, 3, 3, 3)); // mm1 = src2 = 00pa00pa00pa00pa
				pshufw_rri(mm4, mm0, SHUF_MASK(2, 2, 2, 2)); // mm1 = src2 = 00pa00pa00pa00pa
				
				psubw_rr(mm1, mm6);
				psubw_rr(mm2, mm6);
				pmullw_rr(mm1, mm3);
				pmullw_rr(mm2, mm4);
				psrlw_ri(mm1, 8);
				
				
				
				movq_rm(mm2, dest[2]);
				
				movq_rm(mm0, src[0]);    // mm0 = src1 | src2
				movq_rm(mm1, src[2]);    // mm1 = src3 | src4
				
				movq_rm(mm2, dest[0]);   // mm2 = dest1 | dest2
				movq_rm(mm3, dest[2]);   // mm3 = dest3 | dest4
				
				movq_rr(mm4, mm0);
				movq_rr(mm5, mm1);
				
				pcmpeqd_rm(mm0, rb_mask32);
				pcmpeqd_rm(mm1, rb_mask32);
				movq_rr(mm6, mm2);
				movq_rr(mm7, mm3);
				pand_rr(mm6, mm0);
				pand_rr(mm7, mm1);
				pandn_rr(mm0, mm4);
				pandn_rr(mm1, mm5);
				por_rr(mm0, mm6);
	    		por_rr(mm1, mm7);
				
				
				pavgb_rr(mm0, mm2);
				pavgb_rr(mm1, mm3);
				
				movq_mr(dest[0], mm0);
				movq_mr(dest[2], mm1);
			)
		)
	}
	else
	{
		movd_rm(mm5, fact);
		punpcklwd_rr(mm5, mm5); // 0000000000ff00ff
		punpcklwd_rr(mm5, mm5); // 00ff00ff00ff00ff
		
		static unsigned long long fact32;
		
		movq_mr(fact32, mm5);

		SPRITE_Y_LOOP(
			SPRITE_X_LOOP_NOALIGN(4,
				Pixel s = *src;
				if(s != maskcolor_32)
					*dest = blendColorsFact_32(*dest, s, fact)
			,
				prefetchnta(src[8]);
				prefetcht0(dest[8]);
				
				movq_rm(mm0, src[0]);    // mm0 = src1 | src2
				movq_rm(mm1, src[2]);    // mm1 = src3 | src4
				
				movq_rm(mm2, dest[0]);   // mm2 = dest1 | dest2
				movq_rm(mm3, dest[2]);   // mm3 = dest3 | dest4
				
				movq_rr(mm4, mm0);
				movq_rr(mm5, mm1);

				// Can this be optimized?
				pcmpeqd_rm(mm0, rb_mask32);
				pcmpeqd_rm(mm1, rb_mask32);
				movq_rr(mm6, mm2);
				movq_rr(mm7, mm3);
				pand_rr(mm6, mm0);
				pand_rr(mm7, mm1);
				pandn_rr(mm0, mm4);
				pandn_rr(mm1, mm5);
				por_rr(mm0, mm6);
	    		por_rr(mm1, mm7);
	    		
				pxor_rr(mm7, mm7);

				// Do src1/2 - dest1/2
				
				movq_rr(mm4, mm0);
			
				punpcklbw_rr(mm0, mm7); // mm0 = src2 = 00rr00gg00bb
				punpckhbw_rr(mm4, mm7); // mm4 = src1 = 00rr00gg00bb
				
				movq_rr(mm6, mm2);
				movq_rr(mm5, mm2);
			
				punpcklbw_rr(mm2, mm7); // mm2 = dest2 = 00rr00gg00bb
				punpckhbw_rr(mm6, mm7); // mm6 = dest1 = 00rr00gg00bb
				
				psubw_rr(mm0, mm2);
				psubw_rr(mm4, mm6);
				
				movq_rm(mm6, fact32);
				
				pmullw_rr(mm0, mm6);
				pmullw_rr(mm4, mm6);
				psrlw_ri(mm0, 8);
				psrlw_ri(mm4, 8);
				
				packuswb_rr(mm0, mm4);  // mm0 = scaled1 | scaled2
				
				paddb_rr(mm0, mm5);
				
				// Do src3/4 - dest3/4
				
				movq_rr(mm4, mm1);
				
				punpcklbw_rr(mm1, mm7); // mm1 = src4 = 00rr00gg00bb
				punpckhbw_rr(mm4, mm7); // mm4 = src3 = 00rr00gg00bb
				
				movq_rr(mm2, mm3);
				movq_rr(mm5, mm3);
				
				punpcklbw_rr(mm3, mm7); // mm3 = dest4 = 00rr00gg00bb
				punpckhbw_rr(mm2, mm7); // mm6 = dest3 = 00rr00gg00bb
				
				psubw_rr(mm1, mm3);
				psubw_rr(mm4, mm2);
				
				pmullw_rr(mm1, mm6);
				pmullw_rr(mm4, mm6);
				psrlw_ri(mm1, 8);
				psrlw_ri(mm4, 8);

				packuswb_rr(mm1, mm4);  // mm1 = scaled3 | scaled4
				
				paddb_rr(mm1, mm5);

				movq_mr(dest[0], mm0);
				movq_mr(dest[2], mm1);
				
			)
		)
	}
	
	emms();
}
*/

#define FBLEND_BLEND_16_4_MMX(source1, dest1, fact, r_mask, g_mask, b_mask, scratch1, scratch2, scratch3, scratch4) \
	movq_rr(scratch1, source1);                                          \
	movq_rr(scratch2, dest1);                                          \
	movq_rm(scratch3, r_mask);                                          \
	                                                                 \
	pand_rr(source1, scratch3);    /* Mask Red */                        \
	pand_rr(dest1, scratch3);                                            \
	                                                                 \
	psrlw_ri(source1, 5);         /* Multiply by the factor */          \
	psrlw_ri(dest1, 5);                                                 \
	psubw_rr(source1, dest1);                                            \
	pmullw_rr(source1, fact);                                            \
	psllw_ri(dest1, 5);                                                 \
	                                                                 \
	paddw_rr(source1, dest1);      /* Offset and clean up Red */         \
	                                                                 \
	movq_rr(scratch4, scratch1);                                         \
	pand_rr(source1, scratch3);                                          \
	movq_rm(scratch3, g_mask);                                           \
	movq_rr(dest1, scratch2);                                            \
	                                                                 \
	pand_rr(scratch1, scratch3);   /* Mask Green */                      \
	pand_rr(dest1, scratch3);                                            \
	                                                                 \
	psubw_rr(scratch1, dest1);     /* Multiply by the factor */          \
	pmullw_rr(scratch1, fact);                                           \
	psrlw_ri(scratch1, 5);                                              \
	                                                                 \
	paddw_rr(scratch1, dest1);     /* Offset and clean up Green */       \
	pand_rr(scratch1, scratch3);                                         \
	                                                                 \
	movq_rm(scratch3, b_mask);                                           \
	por_rr(source1, scratch1);    /* Combine Red and Green */           \
	                                                                 \
	pand_rr(scratch4, scratch3);   /* Mask Blue */                       \
	pand_rr(scratch2, scratch3);                                         \
	                                                                 \
	psubw_rr(scratch4, scratch2);  /* Multiply by the factor */          \
	pmullw_rr(scratch4, fact);                                           \
	psrlw_ri(scratch4, 5);                                              \
	                                                                 \
	paddw_rr(scratch4, scratch2);  /* Offset and clean up Blue */        \
	pand_rr(scratch4, scratch3);                                         \
	                                                                 \
	por_rr(source1, scratch4)     /* Combine RGB */
	

void drawSprite_blend_16_mmx_sse(BITMAP* where, BITMAP* from, int x, int y, int cutl, int cutt, int cutr, int cutb, int fact)
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
	
	fact = (fact + 4) / 8;

	movd_rm(mm6, fact);
	punpcklwd_rr(mm6, mm6); // 00000000ff00ff00
	punpcklwd_rr(mm6, mm6); // ff00ff00ff00ff00
	
	movq_rm(mm7, MASK);

	SPRITE_Y_LOOP(
		SPRITE_X_LOOP_NOALIGN(4,
			Pixel s = *src;
			if(s != maskcolor_16)
				*dest = blendColorsFact_16_2(*dest, s, fact)
		,
#ifdef NDEBUG // GCC 4 has problems with these instructions with -O0 somewhy
			prefetchnta(src[8]);
			prefetcht0(dest[8]);
#endif
			
			movq_rm(mm0, dest[0]);    // mm1 = dest1 | dest2 | dest3 | dest4
			movq_rm(mm3, src[0]);     // mm0 = src1 | src2 | src3 | src4
			
			movq_rr(mm1, mm3);
			movq_rr(mm2, mm0);
			pcmpeqw_rr(mm3, mm7);
			pand_rr(mm2, mm3);
			pandn_rr(mm3, mm1);
			por_rr(mm3, mm2);

			FBLEND_BLEND_16_4_MMX(mm3, mm0, mm6, RED, GREEN, BLUE, mm1, mm2, mm4, mm5);
			
			movq_mr(dest[0], mm3);
		)
	)

	emms();
}

}

#endif
