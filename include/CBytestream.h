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



#define		MAX_DATA		4096


class CBytestream {
public:
	// Constructor
	CBytestream() {
		Clear();
	}

private:
	// Attributes

	int			CurByte;

	int			Length;
	uchar		Data[MAX_DATA];


public:
	// Methods


	// Generic data
	uchar		*GetData(void)			{ return Data; }
	int			GetLength(void)			{ return Length; }
	void		SetLength(int l)		{ Length = l; }
	int			GetPos(void)			{ return CurByte; }
	void		SetPos(int _p)			{ CurByte = _p; }


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
	
	// Networking stuff
	inline void		Send(NetworkSocket sock)		{ WriteSocket(sock,Data,Length); }
	inline int		Read(NetworkSocket sock)		{ Clear(); Length = ReadSocket(sock,Data,MAX_DATA); return (Length>0)?Length:0; }
	



};







#endif  //  __CBITSTREAM_H__
