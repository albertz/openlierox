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
#include <set>
#include "olx-types.h"
#include "Networking.h"
#include "SmartPointer.h"
#include "CodeAttributes.h"

class ScriptVar_t;
struct Logger;
struct PrintOutFct;
struct CustomVar;

class CBytestream {
public:
	CBytestream() : pos(0), bitPos(0), Data("") {}
	CBytestream(const std::string& rawData) : pos(0), bitPos(0), Data(rawData) {}
	
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
	size_t		GetLength()	const 	{ return Data.size(); }
	size_t		GetPos() const 		{ return pos; }
	size_t		GetRestLen() const 	{ return isPosAtEnd() ? 0 : (Data.size() - pos); }
	bool		isPosAtEnd() const { return GetPos() >= GetLength(); }
	void		revertByte()		{ assert(pos > 0); pos--; }
	void		flushOld()			{ Data.erase(0, pos); pos = 0; }
	std::string	getRawData(size_t start, size_t end) { assert(start <= end); return Data.substr(start, end - start + 1); } 
	
	void		Clear();
	void		Append(CBytestream *bs);
	
	// Note: marks positions are relative to start; start=0 means from the very beginning of the stream (not from pos)
    void        Dump(const PrintOutFct& printer, const std::set<size_t>& marks = std::set<size_t>(), size_t start = 0, size_t count = (size_t)-1);
	void		Dump();

	// Writes
	bool		writeByte(uchar byte);
	bool		writeBool(bool value);
	bool		writeInt(int value, uchar numbytes);
	bool		writeInt16(Sint16 value);
	bool		writeUInt64(Uint64 value);
	bool		writeFloat(float value);
	bool		writeString(const std::string& value);
	bool		write2Int12(short x, short y);
	bool		write2Int4(short x, short y);
	bool		writeBit(bool bit);
	bool		writeData(const std::string& value);	// Do not append '\0' at the end, writes just the raw data
	bool		writeVar(const ScriptVar_t& var);
	
	// Reads
	uchar		readByte();
	bool		readBool();
	int			readInt(uchar numbytes); // readInt(2) will return 0:65535 range, not -32768:32767, so save it into Sint16
	Sint16		readInt16();
	Uint64		readUInt64();
	float		readFloat();
	std::string readString();
	std::string readString(size_t maxlen);
	void		read2Int12(short& x, short& y);
	void		read2Int4(short& x, short& y);
	bool		readBit();
	std::string	readData( size_t size = (size_t)(-1) );
	bool		readVar(ScriptVar_t& var, const CustomVar* customType = NULL);

	// Peeks
	uchar		peekByte() const;
	std::string	peekData(size_t len) const;

	const std::string& data() const { return Data; }
	
	// Skips
	// Folowing functions return true if we're at the end of stream after the skip
	bool		Skip(size_t num);
	bool SkipInt()		{ return Skip(4); }
	bool SkipFloat()		{ return Skip(4); }
	bool SkipShort()		{ return Skip(2); }
	bool		SkipString();
	void		SkipAll()		{ pos = Data.size(); }
	bool	SkipRestBits() { if(isPosAtEnd()) return true; ResetBitPos(); pos++; return isPosAtEnd(); }
	bool SkipVar();

	// Networking stuff
	bool	Send(NetworkSocket* sock);
	size_t	Read(NetworkSocket* sock);
	bool Send(const SmartPointer<NetworkSocket>& sock) { return Send(sock.get()); }
	size_t Read(const SmartPointer<NetworkSocket>& sock) { return Read(sock.get()); }
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
	}

	size_t getPos() const
	{
		return pos;
	}
	void resetPos()
	{
		pos = 0;
		bitMask = 1;
	}

	CBytestreamBitIterator& operator ++()
	{
		if( bitMask >= 128 )
		{
			bitMask = 1;
			pos ++;
		}
		else
			bitMask *= 2;
		return *this;
	}

	bool getBit() const
	{
		return ( data[pos] & bitMask ) != 0;
	}
	void setBit()
	{
		data[pos] |= bitMask;
	}
	void clearBit()
	{
		data[pos] &= ~ bitMask;
	}
	void xorBit()
	{
		data[pos] ^= bitMask;
	}

	// Slower functions
	bool readBit()
	{
		bool ret = getBit();
		++(*this);
		return ret;
	}
	void writeBit( bool bit )
	{
		if( bit )
			setBit();
		else
			clearBit();
		++(*this);
	}
	
	static size_t getSizeInBytes( size_t bits )
	{
		size_t bytes = bits / 8;
		if( bits % 8 )
			bytes++;
		return bytes;
	}
};


template< bool (*fct1) (CBytestream*), bool (*fct2) (CBytestream*) >
INLINE bool SkipMult(CBytestream* bs) { return (*fct1)(bs) && (*fct2)(bs); }

template< size_t NUM >
INLINE bool Skip(CBytestream* bs) { return bs->Skip(NUM); }

INLINE bool SkipString(CBytestream* bs) { return bs->SkipString(); }

#endif  //  __CBITSTREAM_H__
