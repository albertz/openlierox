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
#include <algorithm>
#include "StaticAssert.h"
#include "CVec.h"

#ifdef _MSC_VER
// there is no way for MSVC to cast int->bool without a warning
#pragma warning(disable: 4800)
#endif

template <int n>
void ByteSwap(unsigned char * b) {
	static_assert(n == 1 || n % 2 == 0, n_must_be_equal);
	for(int i = 0; i < n/2; ++i) {
		std::swap(b[i], b[n - i - 1]);
	}
}

template <typename T>
T GetByteSwapped(T b) {
	ByteSwap<sizeof(T)>(&b);
	return b;
}

template <typename T>
void ByteSwap5(T& x) {
	ByteSwap<sizeof(T)>((unsigned char*) &x);
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

template <typename T, typename PtrT>
static T pread_endian(PtrT& p, PtrT end) {
	static_assert(sizeof(*(PtrT())) == 1, PtrT_of_8bit);
	T data = T();
	if(p + sizeof(T) <= end) {
		data = *(T*)p;
		EndianSwap(data);
	}
	p += sizeof(T);
	return data;
}


template<typename T1, typename T2>
static size_t fread_endian_M(FILE* stream, MatrixD2<T2>& d) {
	if(fread_endian<T1>(stream, d.v1.x) == 0) return 0;
	if(fread_endian<T1>(stream, d.v1.y) == 0) return 0;
	if(fread_endian<T1>(stream, d.v2.x) == 0) return 0;
	if(fread_endian<T1>(stream, d.v2.y) == 0) return 0;
	return 1;
}

template<typename T1, typename T2>
static size_t fwrite_endian_M(FILE* stream, MatrixD2<T2>& d) {
	if(fwrite_endian<T1>(stream, d.v1.x) == 0) return 0;
	if(fwrite_endian<T1>(stream, d.v1.y) == 0) return 0;
	if(fwrite_endian<T1>(stream, d.v2.x) == 0) return 0;
	if(fwrite_endian<T1>(stream, d.v2.y) == 0) return 0;
	return 1;
}

template<typename T1, typename T2>
static size_t fread_endian_V(FILE* stream, VectorD2<T2>& d) {
	if(fread_endian<T1>(stream, d.x) == 0) return 0;
	if(fread_endian<T1>(stream, d.y) == 0) return 0;
	return 1;
}

template<typename T1, typename T2>
static size_t fwrite_endian_V(FILE* stream, VectorD2<T2>& d) {
	if(fwrite_endian<T1>(stream, d.x) == 0) return 0;
	if(fwrite_endian<T1>(stream, d.y) == 0) return 0;
	return 1;
}


#endif
