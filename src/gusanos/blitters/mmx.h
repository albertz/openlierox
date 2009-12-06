#ifndef MMX_H
#define MMX_H

#ifdef __GNUC__

union mmx_reg
{
	mmx_reg()
	{
	}
	
	mmx_reg(unsigned long long v)
	: uq(v)
	{
	}
	
	long long               q;
	unsigned long long      uq;
	int                     d[2];
	unsigned int            ud[2];
	short                   w[4];
	unsigned short          uw[4];
	char                    b[8];
	unsigned char           ub[8];
	float                   s[2];
};

#define mmx_ri(op,reg,imm) \
	__asm__ __volatile__ (#op " %0, %%" #reg \
	: /* nothing */ \
	: "i" (imm) )

#define mmx_rm(op,reg,mem) \
	__asm__ __volatile__ (#op " %0, %%" #reg \
	: /* nothing */ \
	: "m" (mem))

#define mmx_mr(op,mem,reg) \
	__asm__ __volatile__ (#op " %%" #reg ", %0" \
	: "=m" (mem) \
	: /* nothing */ )

#define mmx_rr(op,regd,regs) \
	__asm__ __volatile__ (#op " %" #regs ", %" #regd)
	
#define mmx_fetch(mem,hint) \
	__asm__ __volatile__ ("prefetch" #hint " %0" \
	: /* nothing */ \
	: "X" (mem))

#define movd_mr(A, B) mmx_mr(movd, A, B)
#define movd_rm(A, B) mmx_rm(movd, A, B)
#define movd_rr(A, B) mmx_rr(movd, A, B)
#define movq_mr(A, B) mmx_mr(movq, A, B)
#define movq_rm(A, B) mmx_rm(movq, A, B)
#define movq_rr(A, B) mmx_rr(movq, A, B)
#define paddb_rr(A, B) mmx_rr(paddb, A, B)
#define paddw_rr(A, B) mmx_rr(paddw, A, B)
#define paddusb_rr(A, B) mmx_rr(paddusb, A, B)
#define paddusw_rr(A, B) mmx_rr(paddusw, A, B)
#define psubw_rr(A, B) mmx_rr(psubw, A, B)
#define psrlq_ri(A, B) mmx_ri (psrlq, A, B)
#define psrlw_ri(A, B) mmx_ri (psrlw, A, B)
#define psraw_ri(A, B) mmx_ri (psraw, A, B)
#define psllq_ri(A, B) mmx_ri (psllq, A, B)
#define psllw_ri(A, B) mmx_ri (psllw, A, B)
#define pand_rr(A, B) mmx_rr (pand, A, B)
#define pand_rm(A, B) mmx_rm (pand, A, B)
#define pandn_rr(A, B) mmx_rr (pandn, A, B)
#define por_rr(A, B) mmx_rr (por, A, B)
#define pxor_rr(A, B) mmx_rr (pxor, A, B)
#define punpckldq_rr(A, B) mmx_rr(punpckldq, A, B)
#define punpcklwd_rr(A, B) mmx_rr(punpcklwd, A, B)
#define punpcklbw_rr(A, B) mmx_rr(punpcklbw, A, B)
#define punpckhbw_rr(A, B) mmx_rr(punpckhbw, A, B)
#define packuswb_rr(A, B) mmx_rr(packuswb, A, B)
#define pcmpeqd_rr(A, B) mmx_rr(pcmpeqd, A, B)
#define pcmpeqw_rr(A, B) mmx_rr(pcmpeqw, A, B)
#define pcmpeqd_rm(A, B) mmx_rm(pcmpeqd, A, B)

#define pmullw_rr(A, B) mmx_rr(pmullw, A, B)
#define pmulhw_rr(A, B) mmx_rr(pmulhw, A, B)
#define pmulhw_rm(A, B) mmx_rm(pmulhw, A, B)
#define pmullw_rm(A, B) mmx_rm(pmullw, A, B)
#define pmulhuw_rr(A, B) mmx_rr(pmulhuw, A, B)


#define mmx_rri(op, regd, regs, imm) \
	__asm__ __volatile__ (#op " %0, %%" #regs ", %%" #regd \
	: /* nothing */ \
	: "X" (imm) )



//SSE / AMD MMX ext
#define pmulhuw_rr(A, B) mmx_rr(pmulhuw, A, B)
#define pavgb_rr(A, B) mmx_rr(pavgb, A, B)
#define pshufw_rri(A, B, C) mmx_rri(pshufw, A, B, C)

#define movntq_mr(A, B)  mmx_mr(movntq, A, B)

#define prefetcht0(A)  mmx_fetch(A, t0)
#define prefetcht1(A)  mmx_fetch(A, t1)
#define prefetcht2(A)  mmx_fetch(A, t2)
#define prefetchnta(A) mmx_fetch(A, nta)


#define emms() __asm__ __volatile__ ("emms")

#else
#error "MMX capability only available on GCC!"
#endif

#ifndef always_inline
#if defined(__GNUC__) && (__GNUC__ > 3 || __GNUC__ == 3 && __GNUC_MINOR__ > 0)
#    define always_inline __attribute__((always_inline)) inline
#else
#    define always_inline inline
#endif
#endif

#endif //MMX_H
