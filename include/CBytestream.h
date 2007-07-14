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
#include <sstream>
#include "types.h"
#include "Networking.h"


#define		MAX_DATA		4096


class CBytestream {
public:
	// Constructor
	CBytestream() {
		Clear();
	}

private:
	// Attributes

	size_t		CurByte;

	size_t			Length;
	unsigned char	Data[MAX_DATA]; // TODO: use std::sstream


public:
	// Methods


	// Generic data
	uchar		*GetData(void)			{ return Data; }
	size_t		GetLength(void)			{ return Length; }
	void		SetLength(size_t l)		{ Length = l; }
	size_t		GetPos(void)			{ return CurByte; }
	void		SetPos(size_t _p)			{ CurByte = _p; }


	void		Reset(void)				{ CurByte = 0; }
	void		Clear(void)				{ CurByte = 0; Length = 0; memset(Data,0,MAX_DATA); }

	void		Append(CBytestream *bs);
	void		AppendData(char *_data, int _length);

    void        Dump(void);
	

	// Writes
	int			writeByte(uchar byte);
	int			writeBool(bool value);
	int			writeInt(int value, uchar numbytes);
	int			writeInt16(Sint16 value);
	int			writeFloat(float value);
	int			writeString(char *fmt,...);
	int			writeString(const std::string& value);
	int			write2Int12(short x, short y);
	int			write2Int4(short x, short y);
	
	// Reads
	uchar		readByte(void);
	bool		readBool(void);
	int			readInt(uchar numbytes);
	Sint16		readInt16(void);
	float		readFloat(void);
	char		*readString(char *str, size_t maxlen);
	std::string readString();
	std::string readString(size_t maxlen);
	void		read2Int12(short& x, short& y);
	void		read2Int4(short& x, short& y);

	// Skips
	// Folowing functions return true if we're at the end of stream after the skip
	inline bool	Skip(size_t num)  { CurByte += num; if (CurByte >= Length)  {CurByte = Length-1; return true; } else return false;}
	inline bool SkipInt() { return Skip(4); }
	inline bool SkipFloat() { return Skip(4); }
	inline bool SkipShort()  { return Skip(2); }
	bool		SkipString();
	
	// Networking stuff
	inline void		Send(NetworkSocket sock)		{ WriteSocket(sock,Data,(int)Length); }
	inline size_t	Read(NetworkSocket sock)		{ Clear(); Length = ReadSocket(sock,Data,MAX_DATA); return (Length>0)?Length:0; }
	



};







#endif  //  __CBITSTREAM_H__
