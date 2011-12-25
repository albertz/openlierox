/*
 *  Bitstream.h
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 15.02.10.
 *  code under GPL
 *
 */

#ifndef __OLX_BITSTREAM_H__
#define __OLX_BITSTREAM_H__

#include <vector>
#include <string>
#include "CodeAttributes.h"

class BitStream {
private:
	std::vector<bool> m_data;
	size_t m_readPos;  // in bits
	
	void growIfNeeded(size_t addBits);
	void writeBits(const std::vector<bool>& bits);
	std::vector<bool> readBits(size_t bitCount);
	void reset();
	
	bool testBool();
	bool testInt();
	bool testFloat();
	bool testStream();
	bool testString();
	bool testSafety();
public:
	BitStream() : m_readPos(0) {}
	BitStream(const std::string& rawdata);
	
	void addBool(bool);
	void addInt(int n, int bits);
	void addSignedInt(int n, int bits);
	void addFloat(float f, int bits);
	void addBitStream(const BitStream& str);
	void addString(const std::string&);
	
	bool getBool();
	int getInt(int bits);
	int getSignedInt(int bits);
	float getFloat(int bits);
	std::string getString();
	
	BitStream* Duplicate();
	bool runTests();
	
	const std::vector<bool>& data() const { return m_data; }
	void resetPos() { m_readPos = 0; }
	void setBitPos(size_t p) { m_readPos = p; }
	void skipBits(size_t b) { m_readPos += b; }
	size_t bitPos() const { return m_readPos; }
	size_t bitSize() const { return m_data.size(); }
	size_t restBitSize() const { return m_data.size() - m_readPos; }
};

static INLINE char getCharFromBits(BitStream& bs) {
	return (char) (unsigned char) bs.getInt(8);
}


#endif
