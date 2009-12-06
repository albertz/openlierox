#ifndef DEDSERV

#include "blitters.h"
#include "colors.h"
#include "mmx.h"
#include "macros.h"

namespace Blitters
{
	
void drawSprite_mult_8_to_32_mmx_sse(BITMAP* where, BITMAP* from, int x, int y, int cutl, int cutt, int cutr, int cutb)
{
	typedef Pixel32 pixel_t_dest_1;
	typedef Pixel32 pixel_t_dest_2; // Doesn't matter what this is
	typedef Pixel8 pixel_t_src_1;
	typedef Pixel8 pixel_t_src_2; // Doesn't matter what this is
	
	if(bitmap_color_depth(where) != 32
	|| bitmap_color_depth(from) != 8)
		return;

	CLIP_SPRITE_REGION();
	
	#define SHUF_MASK(a, b, c, d) ((a << 6) | (b << 4) | (c << 2) | d)

	pxor_rr(mm6, mm6);
	
	SPRITE_Y_LOOP(
		SPRITE_X_LOOP_NOALIGN_T(4,
			*dest = scaleColor_32(*dest, *src)
		,
#ifdef NDEBUG // GCC 4 has problems with these instructions with -O0 somewhy
			prefetchnta(src[8]);
			prefetcht0(dest[8]);
#endif
			
			pxor_rr(mm0, mm0);
			
			movd_rm(mm7, src[0]);
			movq_rm(mm1, dest[0]);
			punpcklbw_rr(mm0, mm7); // mm0 = src1-4 = pa00pb00pc00pd00
			movq_rr(mm2, mm1);
			
			punpcklbw_rr(mm2, mm6); // mm2 = dest2 = 00rr00gg00bb
			punpckhbw_rr(mm1, mm6); // mm1 = dest1 = 00rr00gg00bb
			
			pshufw_rri(mm4, mm0, SHUF_MASK(0, 0, 0, 0)); // mm4 = src2 = pb00pb00pb00pb00
			pshufw_rri(mm3, mm0, SHUF_MASK(1, 1, 1, 1)); // mm3 = src1 = pa00pa00pa00pa00
			
			pmulhuw_rr(mm2, mm4);
			pmulhuw_rr(mm1, mm3);
			
			packuswb_rr(mm2, mm1);
			
			movq_rm(mm1, dest[2]);
			
			pshufw_rri(mm4, mm0, SHUF_MASK(2, 2, 2, 2)); // mm4 = src4 = pd00pd00pd00pd00
			pshufw_rri(mm3, mm0, SHUF_MASK(3, 3, 3, 3)); // mm3 = src3 = pc00pc00pc00pc00
			
			movq_rr(mm5, mm1);
			
			punpckhbw_rr(mm1, mm6); // mm1 = dest3 = 00rr00gg00bb
			
			punpcklbw_rr(mm5, mm6); // mm5 = dest4 = 00rr00gg00bb
			
			pmulhuw_rr(mm1, mm3);
			pmulhuw_rr(mm5, mm4);
			
			movq_mr(dest[0], mm2);
			
			packuswb_rr(mm5, mm1);
			
			movq_mr(dest[2], mm5);
		)
	)
	
	emms();
}

} //namespace Blitters

#endif
