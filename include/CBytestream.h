/////////////////////////////////////////
//
//   OpenLieroX
//
//   Auxiliary Software class library
//
//   based on the work of JasonB
//   enhanced by Dark Charlie and Albert Zeyer
//
//   code under LGPL
//
/////////////////////////////////////////


// Byte stream class
// Created 13/10/01
// Jason Boettcher


#ifndef __CBYTESTREAM_H__
#define __CBYTESTREAM_H__

#include <SDL.h> // for SInt16
#include <string>
#include "types.h"
#include "Networking.h"


class CBytestream {
public:
	CBytestream()  {
		pos = 0;
		bitPos = 0;
		Data = "";
	}

	CBytestream(const CBytestream& bs) {
		operator=(bs);
	}
	
	CBytestream& operator=(const CBytestream& bs) {
		pos = bs.pos;
		Data = bs.Data;
		bitPos = bs.bitPos;
		return *this;
	}
	
private:
	// Attributes
	size_t pos;
	size_t bitPos;
	std::string Data;

public:
	// Methods

	// Debug
	void		Test();


	// Generic data
	void		ResetBitPos()		{ bitPos = 0; }
	void		ResetPosToBegin()	{ pos = 0; bitPos = 0; }
	size_t		GetLength()			{ return Data.size(); }
	size_t		GetPos() 			{ return pos; }
	size_t		GetRestLen()		{ return isPosAtEnd() ? 0 : (Data.size() - pos); }
	
	void		Clear();
	void		Append(CBytestream *bs);
    void        Dump();
	

	// Writes
	bool		writeByte(uchar byte);
	bool		writeBool(bool value);
	bool		writeInt(int value, uchar numbytes);
	bool		writeInt16(Sint16 value);
	bool		writeFloat(float value);
	bool		writeString(const std::string& value);
	bool		write2Int12(short x, short y);
	bool		write2Int4(short x, short y);
	bool		writeBit(bool bit);
	bool		writeData(const std::string& value);	// Do not append '\0' at the end
	
	// Reads
	uchar		readByte(void);
	bool		readBool(void);
	int			readInt(uchar numbytes);
	Sint16		readInt16(void);
	float		readFloat(void);
	std::string readString();
	std::string readString(size_t maxlen);
	void		read2Int12(short& x, short& y);
	void		read2Int4(short& x, short& y);
	bool		readBit();
	std::string	readData( size_t size = (size_t)(-1) );

	// Peeks
	uchar		peekByte();
	std::string	peekData(size_t len);

	// Skips
	// Folowing functions return true if we're at the end of stream after the skip
	bool		Skip(size_t num);
	inline bool SkipInt()		{ return Skip(4); }
	inline bool SkipFloat()		{ return Skip(4); }
	inline bool SkipShort()		{ return Skip(2); }
	bool		SkipString();
	void		SkipAll()		{ pos = Data.size(); }
	
	bool		isPosAtEnd() 	{ return GetPos() >= GetLength(); }
	
	// Networking stuff
	bool	Send(NetworkSocket sock);
	size_t	Read(NetworkSocket sock);

};

// Inline class to operate with bits for dirt updates, hopefully optimized by compiler
// No bound-checking is made, be sure your data is large enough
class CBytestreamBitIterator
{
	char * data;
	size_t pos;
	Uint8 bitMask;

	public:

	CBytestreamBitIterator(char * Data): data(Data) 
	{ 
		resetPos();
	};

	size_t getPos() const
	{
		return pos;
	};
	void resetPos()
	{
		pos = 0;
		bitMask = 1;
	};

	void operator ++()
	{
		if( bitMask >= 128 )
		{
			bitMask = 1;
			pos ++;
		}
		else
			bitMask *= 2;
	};

	bool getBit() const
	{
		return ( data[pos] & bitMask ) != 0;
	};
	void setBit()
	{
		data[pos] |= bitMask;
	};
	void clearBit()
	{
		data[pos] &= ~ bitMask;
	};
	void xorBit()
	{
		data[pos] ^= bitMask;
	};

	// Slower functions
	bool readBit()
	{
		bool ret = getBit();
		++(*this);
		return ret;
	};
	void writeBit( bool bit )
	{
		if( bit )
			setBit();
		else
			clearBit();
		++(*this);
	};
	
	static size_t getSizeInBytes( size_t bits )
	{
		size_t bytes = bits / 8;
		if( bits % 8 )
			bytes++;
		return bytes;
	};
};

#endif  //  __CBITSTREAM_H__
