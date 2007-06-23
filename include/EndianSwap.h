/*
	OpenLieroX
	
	makros for endianess / swapping / conversions
	
	Code under LGPL
	by Albert Zeyer, Dark Charlie, 23-06-2007
*/

#ifndef __ENDIANSWAP_H__
#define __ENDIANSWAP_H__

#include <SDL/SDL.h>

#define ByteSwap5(x) ByteSwap((unsigned char *) &x,sizeof(x))

inline void ByteSwap(unsigned char * b, int n) {
	register int i = 0;
	register int j = n - 1;
	register unsigned char tmp;
	while(i < j) {
		tmp = b[i]; b[i] = b[j]; b[j] = tmp;
		i++, j--;
	}
}

extern unsigned char byteswap_buffer[16];

template <typename T>
inline T* GetByteSwapped(const T b) {
	*((T*)byteswap_buffer) = b;
	ByteSwap(byteswap_buffer, sizeof(T));
	return (T*)byteswap_buffer;
}

#if !defined(SDL_BYTEORDER)
#	error SDL_BYTEORDER not defined
#endif
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
#	define EndianSwap(x)		;
#	define BEndianSwap(x)		ByteSwap5(x);
#	define GetEndianSwapped(x)	(&x)
#elif SDL_BYTEORDER == SDL_BIG_ENDIAN
#	define EndianSwap(x)		ByteSwap5(x);
#	define BEndianSwap(x)		;
#	define GetEndianSwapped(x)	(GetByteSwapped(x))
#else
#	error unknown ENDIAN type
#endif


#endif
