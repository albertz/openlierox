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


#ifndef __CBITSTREAM_H__
#define __CBITSTREAM_H__

#include <SDL/SDL.h> // for SInt16
#include <string>
#include <sstream>
#include "types.h"
#include "Networking.h"


class CBytestream {
public:
	// Constructor
	CBytestream()	{ Clear(); }
	CBytestream(const CBytestream& bs)	{ (*this) = bs; }
	CBytestream& operator=(const CBytestream& bs) {
		Data.str(bs.Data.str());
		return *this;
	}
	
private:
	// Attributes
	std::stringstream Data;

public:
	// Methods

	// Debug
	void		Test();


	// Generic data
	void		ResetPosToBegin()		{ Data.seekg(0, std::ios::beg); }
	size_t		GetLength(void)	const	{ return Data.str().size(); }
	size_t		GetPos(void) 			{ return Data.tellg(); }

	void		Clear(void)				{ Data.str(""); }

	void		Append(CBytestream *bs);

    void        Dump(void);
	

	// Writes
	bool		writeByte(uchar byte);
	bool		writeBool(bool value);
	bool		writeInt(int value, uchar numbytes);
	bool		writeInt16(Sint16 value);
	bool		writeFloat(float value);
	bool		writeString(const std::string& value);
	bool		write2Int12(short x, short y);
	bool		write2Int4(short x, short y);
	
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

	// Skips
	// Folowing functions return true if we're at the end of stream after the skip
	bool		Skip(size_t num);
	inline bool SkipInt()		{ return Skip(4); }
	inline bool SkipFloat()		{ return Skip(4); }
	inline bool SkipShort()		{ return Skip(2); }
	bool		SkipString();
	void		SkipAll()		{ Data.seekg(0, std::ios::end); }
	
	bool		isPosAtEnd() 	{ return Data.tellg() >= GetLength(); }
	
	// Networking stuff
	void	Send(NetworkSocket sock);
	size_t	Read(NetworkSocket sock);

};







#endif  //  __CBITSTREAM_H__
