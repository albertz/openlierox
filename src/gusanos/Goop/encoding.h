#ifndef GUSANOS_ENCODING_H
#define GUSANOS_ENCODING_H

#include <utility>
#include <cassert>
#include "omfggui.h" // For Rect
#include "zoidcom.h"
#include <iostream>
#include <stdexcept>

using std::cerr;
using std::endl;

namespace Encoding
{
	
inline unsigned int bitsOf(unsigned long n)
{
	unsigned int bits = 0;
	for(; n; n >>= 1)
		bits++;
		
	return bits;
}

/*
inline void encode(ZCom_BitStream& stream, int i, int count)
*/
inline void encode(ZCom_BitStream& stream, int i, int count)
{
	assert(count > 0);
	assert(i >= 0 && i < count);
	stream.addInt(i, bitsOf(count - 1));
}

inline int decode(ZCom_BitStream& stream, int count)
{
	assert(count > 0);
	return stream.getInt(bitsOf(count - 1));
}

inline unsigned int signedToUnsigned(int n)
{
	if(n < 0)
		return ((-n) << 1) | 1;
	else
		return n << 1;
}

inline int unsignedToSigned(unsigned int n)
{
	if(n & 1)
		return -(n >> 1);
	else
		return (n >> 1);
}

inline void encodeBit(ZCom_BitStream& stream, int bit)
{
	stream.addInt(bit, 1);
}

inline int decodeBit(ZCom_BitStream& stream)
{
	return stream.getInt(1);
}

inline void encodeEliasGamma(ZCom_BitStream& stream, unsigned int n)
{
	if(n < 1)
		throw std::runtime_error("encodeEliasGamma can't encode 0");
		
	int prefix = bitsOf(n);

	for(int i = 0; i < prefix - 1; ++i)
		encodeBit(stream, 0);

	encodeBit(stream, 1);
	stream.addInt(n, prefix - 1);
}

inline unsigned int decodeEliasGamma(ZCom_BitStream& stream)
{
	int prefix = 0;
	for(; decodeBit(stream) == 0; )
		++prefix;
		
	// prefix = number of prefixed zeroes
	
	return stream.getInt(prefix) | (1 << prefix);
}

inline void encodeEliasDelta(ZCom_BitStream& stream, unsigned int n)
{
	assert(n >= 1);
	int prefix = bitsOf(n);
	encodeEliasGamma(stream, prefix);
	stream.addInt(n, prefix - 1);
}

inline unsigned int decodeEliasDelta(ZCom_BitStream& stream)
{
	int prefix = decodeEliasGamma(stream) - 1;
	
	return stream.getInt(prefix) | (1 << prefix);
}

struct VectorEncoding
{
	VectorEncoding();
	
	VectorEncoding(Rect area_, int subPixelAcc_ = 1);
		
	template<class T>
	std::pair<long, long> quantize(T const& v)
	{
		long y = static_cast<long>((v.y - area.y1) * subPixelAcc + 0.5);
		if(y < 0)
			y = 0;
		else if(y > height)
			y = height - 1;
			
		long x = static_cast<long>((v.x - area.x1) * subPixelAcc + 0.5);
		if(x < 0)
			x = 0;
		else if(x > width)
			x = width - 1;
			
		return std::make_pair(x, y);
	}
	
	template<class T>
	void encode(ZCom_BitStream& stream, T const& v)
	{
		long y = static_cast<long>((v.y - area.y1) * subPixelAcc + 0.5);
		if(y < 0)
			y = 0;
		else if(y > height)
			y = height - 1;
			
		long x = static_cast<long>((v.x - area.x1) * subPixelAcc + 0.5);
		if(x < 0)
			x = 0;
		else if(x > width)
			x = width - 1;
		
		stream.addInt(x, bitsX);
		stream.addInt(y, bitsY);
	}
	
	template<class T>
	T decode(ZCom_BitStream& stream)
	{
		typedef typename T::manip_t manip_t;
		
		long x = stream.getInt(bitsX);
		long y = stream.getInt(bitsY);
		
		return T(manip_t(x) / subPixelAcc + area.x1, manip_t(y) / subPixelAcc + area.y1);
	}
	
	long totalBits()
	{
		return bitsX + bitsY;
	}
	
	Rect area;
	long total;
	long width;
	long height;
	long bitsX;
	long bitsY;
	
	int subPixelAcc;
	double isubPixelAcc;
};

struct DiffVectorEncoding
{
	DiffVectorEncoding(int subPixelAcc_ = 1);
	
	template<class T>
	std::pair<long, long> quantize(T const& v)
	{
		long y = static_cast<long>(v.y * subPixelAcc + 0.5);	
		long x = static_cast<long>(v.x * subPixelAcc + 0.5);

		return std::make_pair(x, y);
	}
	
	template<class T>
	void encode(ZCom_BitStream& stream, T const& v)
	{
		long y = static_cast<long>(v.y * subPixelAcc + 0.5);	
		long x = static_cast<long>(v.x * subPixelAcc + 0.5);
		
		encodeEliasDelta(stream, signedToUnsigned(x) + 1);
		encodeEliasDelta(stream, signedToUnsigned(y) + 1);
	}
	
	template<class T>
	T decode(ZCom_BitStream& stream)
	{
		typedef typename T::manip_t manip_t;
		
		long x = unsignedToSigned(decodeEliasDelta(stream) - 1);
		long y = unsignedToSigned(decodeEliasDelta(stream) - 1);
		
		return T(manip_t(x) / subPixelAcc, manip_t(y) / subPixelAcc);
	}

	int subPixelAcc;
};

}

#endif //GUSANOS_ENCODING_H
