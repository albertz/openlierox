/*
 *  Bitstream.cpp
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 15.02.10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "Bitstream.h"
#include "Debug.h"
#include "EndianSwap.h"


Net_BitStream::Net_BitStream(const std::string& raw) {
	m_readPos = 0;
	m_data.reserve( raw.size() * 8 );
	for(size_t i = 0; i < raw.size(); ++i)
		addInt((unsigned char) raw[i], 8);
}

// Grows the bit stream if the number of bits that are going to be added exceeds the buffer size
void Net_BitStream::growIfNeeded(size_t addBits)
{
	m_data.reserve(m_data.size() + addBits);
}

static const unsigned char bitMasks[] = { 1, 2, 4, 8, 16, 32, 64, 128 };

void Net_BitStream::writeBits(const std::vector<bool>& bits)
{
	growIfNeeded(bits.size());
	for(std::vector<bool>::const_iterator i = bits.begin(); i != bits.end(); ++i)
		m_data.push_back(*i);
}

std::vector<bool> Net_BitStream::readBits(size_t bitCount)
{
	std::vector<bool> ret;
	ret.reserve(bitCount);
	for(size_t i = 0; i < bitCount && m_readPos < m_data.size(); ++i, ++m_readPos)
		ret.push_back(m_data[m_readPos]);
	if(ret.size() < bitCount)
		errors << "Net_BitStream::readBits: reading from behind end" << endl;
	return ret;
}

void Net_BitStream::addBool(bool b) {
	m_data.push_back(b);
}

void Net_BitStream::addInt(int n, int bits) {
	growIfNeeded(bits);
	for(int i = 0; i < bits; ++i)
		m_data.push_back( (((unsigned long)n >> i) & 1) != 0 );
}

void Net_BitStream::addSignedInt(int n, int bits) {
	growIfNeeded(bits);
	addBool( n < 0 );
	if( n >= 0)
		addInt(n, bits - 1);
	else
		addInt((1 << (bits - 1)) - n, bits - 1);
}

void Net_BitStream::addFloat(float f, int bits) {
	// TODO: check bits
	union  {
		char bytes[4];
		float f;
	} data;
	data.f = f;
	BEndianSwap(data.f);
	addBitStream( Net_BitStream( std::string(data.bytes, 4) ) );
}

void Net_BitStream::addBitStream(const Net_BitStream& str) {
	writeBits(str.m_data);
}

void Net_BitStream::addString(const std::string& str) {
	std::string::const_iterator end = str.end();
	for(std::string::const_iterator i = str.begin(); i != end; ++i)
		if(*i == 0) {
			warnings << "Net_BitStream::addString: strings contains NULL-char: " << str << endl;
			end = i;
		}
	
	std::string raw(str.begin(), end);
	raw += '\0';
	addBitStream(Net_BitStream(raw));
}

bool Net_BitStream::getBool() {
	if(m_readPos < m_data.size()) {
		bool ret = m_data[m_readPos];
		m_readPos++;
		return ret;
	}
	
	errors << "Net_BitStream::getBool: reading from behind end" << endl;
	return false;
}

int Net_BitStream::getInt(int bits) {
	unsigned long ret = 0;
	for(int i = 0; i < bits; ++i)
		if(getBool()) ret |= 1 << i;
	return ret;
}

int Net_BitStream::getSignedInt(int bits) {
	bool sign = getBool();
	
	if( !sign /*n >= 0*/ )
		return getInt(bits - 1);
	else
		return (1 << (bits - 1)) - getInt(bits - 1);
}

float Net_BitStream::getFloat(int bits) {
	// TODO: see addFloat, check bits
	union  {
		char bytes[4];
		float f;
	} data;
	
	for(int i = 0; i < 4; ++i)
		data.bytes[i] = getCharFromBits(*this);
	BEndianSwap(data.f);
	
	return data.f;
}

std::string Net_BitStream::getString() {
	std::string ret;
	while(char c = getCharFromBits(*this))
		ret += c;
	return ret;
}

Net_BitStream* Net_BitStream::Duplicate() { 
	return new Net_BitStream(*this);
}

void Net_BitStream::reset()
{
	m_readPos = 0;
	m_data.clear();
}

//
// Net_BitStream Tests
//
bool Net_BitStream::testBool()
{
	reset();
	addBool(true);
	addBool(false);
	bool r1 = getBool();
	bool r2 = getBool();
	return r1 && !r2;
}

bool Net_BitStream::testInt()
{
	reset();
	for (int i = 0; i < 32; i++)
		addInt(1 << i, i + 1);
	for (int i = 0; i < 32; i++)
		if (getInt(i + 1) != (1 << i))
			return false;
	return true;
}

bool Net_BitStream::testFloat()
{
	reset();
	union T  {
		int n;
		float f;
	};
	
	for (int i = 0; i < 32; i++)  {
		T t; t.n = 1 << i;
		addFloat(t.f, i + 1);
	}
	for (int i = 0; i < 32; i++)  {
		T t; t.f = getFloat(i + 1);
		if (t.n != (1 << i))
			return false;
	}
	return true;
}

bool Net_BitStream::testStream()
{
	reset();
	Net_BitStream s2;
	s2.addBool(true);
	s2.addBool(false);
	s2.addInt(50, 32);
	s2.addInt(60, 16);
	s2.addInt(30, 5);
	addInt(14, 4);
	addBool(true);
	addBitStream(s2);
	if (getInt(4) != 14)
		return false;
	if (getBool() != true)
		return false;
	if (getBool() != true)
		return false;
	if (getBool() != false)
		return false;
	if (getInt(32) != 50)
		return false;
	if (getInt(16) != 60)
		return false;
	if (getInt(5) != 30)
		return false;
	return true;
}

bool Net_BitStream::testSafety()
{
	try {
		reset();
		getBool();
		addInt(5, 5);
		getInt(6);
		addFloat(0.0f, 32);
		getFloat(31);
		getInt(5);
	} catch (...)  {
		return false;
	}
	return true;
}

bool Net_BitStream::testString()
{
	addBool(true);
	addString("some short text to test bitstream");
	if (getBool() != true)
		return false;
	if (std::string(getString()) != "some short text to test bitstream")
		return false;
	return true;
}

bool Net_BitStream::runTests()
{
	bool res = true;
	if (!testBool())  {
		printf("Boolean test failed!\n");
		res = false;
	}
	if (!testInt())  {
		printf("Integer test failed\n");
		res = false;
	}
	if (!testFloat())  {
		printf("Float test failed\n");
		res = false;
	}
	if (!testStream())  {
		printf("Stream test failed\n");
		res = false;
	}
	if (!testString())  {
		printf("String test failed\n");
		res = false;
	}
	if (!testSafety())  {
		printf("Safety test failed\n");
		res = false;
	}
	return res;
}
