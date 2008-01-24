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

	CBytestream& operator=(const CBytestream& bs) {
		pos = bs.pos;
		Data = bs.Data;
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
	std::string	readData( uint size = (uint)(-1) );

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







#endif  //  __CBITSTREAM_H__
