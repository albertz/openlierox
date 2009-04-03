/*
	OpenLieroX
	
	makros for endianess / swapping / conversions
	
	Code under LGPL
	by Albert Zeyer, Dark Charlie, 23-06-2007
*/

#ifndef __ENDIANSWAP_H__
#define __ENDIANSWAP_H__

#include <SDL.h>
#include <cstdio>
#include "StaticAssert.h"

#ifdef _MSC_VER
// there is no way for MSVC to cast int->bool without a warning
#pragma warning(disable: 4800)
#endif

#define ByteSwap5(x) ByteSwap((unsigned char *) &x,sizeof(x))

inline static void ByteSwap(unsigned char * b, int n) {
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
T GetByteSwapped(T b) {
	ByteSwap(&b, sizeof(T));
	return b;
}

#if !defined(SDL_BYTEORDER)
#	error SDL_BYTEORDER not defined
#endif
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
#	define EndianSwap(x)		;
#	define BEndianSwap(x)		ByteSwap5(x);
#	define GetEndianSwapped(x)	(x)
#elif SDL_BYTEORDER == SDL_BIG_ENDIAN
#	define EndianSwap(x)		ByteSwap5(x);
#	define BEndianSwap(x)		;
#	define GetEndianSwapped(x)	(GetByteSwapped(x))
#else
#	error unknown ENDIAN type
#endif

template <int size, int nmemb, typename T>
static size_t fwrite_endian_compat_wrapper(FILE* stream, T d) {
	static_assert(nmemb == 1, nmemb__equals1);
	static_assert(size == sizeof(T), size__mismatch);
	EndianSwap(d);
	return fwrite(&d, sizeof(T), 1, stream);	
}

#define fwrite_endian_compat(d, size, nmemb, stream) \
	fwrite_endian_compat_wrapper<(size),(nmemb)>((stream), (d))


template <int size, int nmemb, typename T>
static size_t fread_compat_wrapper(FILE* stream, T& d) {
	static_assert(nmemb == 1, nmemb__equals1);
	static_assert(size == sizeof(T), size__mismatch);
	return fread(&d, sizeof(T), 1, stream);	
}

#define fread_compat(d, size, nmemb, stream) \
	fread_compat_wrapper<(size),(nmemb)>((stream), (d))


template <typename T, typename _D>
static size_t fwrite_endian(FILE* stream, _D d) {
	T data = d;
	EndianSwap(data);
	return fwrite(&data, sizeof(T), 1, stream);
}

template <typename T, typename _D>
static size_t fread_endian(FILE* stream, _D& d) {
	T data;
	size_t ret = fread(&data, sizeof(T), 1, stream);
	EndianSwap(data);
	if(ret > 0) d = (_D)data;
	return ret;
}

#endif
