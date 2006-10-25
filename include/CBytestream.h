/////////////////////////////////////////
//
//   Auxiliary Software class library
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
	int			writeBool(int value);
	int			writeInt(int value, int numbytes);
	int			writeShort(short value);
	int			writeFloat(float value);
	int			writeString(char *fmt,...);


	// Reads
	uchar		readByte(void);
	int			readBool(void);
	int			readInt(int numbytes);
	short		readShort(void);
	float		readFloat(void);
	char		*readString(char *str);


	// Networking stuff
	void		Send(NLsocket sock)		{ nlWrite(sock,Data,Length); }
	int			Read(NLsocket sock)		{ Clear(); Length = nlRead(sock,Data,MAX_DATA); return Length; }
	



};







#endif  //  __CBITSTREAM_H__
