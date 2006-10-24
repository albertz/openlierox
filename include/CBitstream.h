/////////////////////////////////////////
//
//             Liero Xtreme
//
//     Copyright Auxiliary Software 2002
//
//
/////////////////////////////////////////


// Bitstream class
// Created 26/6/02
// Jason Boettcher


#ifndef __CBITSTREAM_H__
#define __CBITSTREAM_H__


#define		MAX_STREAMDATA		1024


class CBitstream {
public:
	// Constructor
	CBitstream() {
		Clear();
	}


private:
	// Attributes

	unsigned char	sData[MAX_STREAMDATA];
	int				iLength;
	int				iCurByte;
	int				iCurBit;


public:
	// Methods

	void			Clear(void);
	void			Reset(void);



	// --------------------------
	//   Data reading & writing
	// --------------------------

	// Int
	int				writeInt(int iValue, int iNumBits);
	int				readInt(int iNumBits);

	// Float
	int				writeFloat(float fValue);
	float			readFloat(void);

	// Vector
	int				writeVec(CVec v);
	CVec			readVec(void);

	// String
	int				writeString(char *sValue);
	char			*readString(char *sValue, int iMax);


	// --------------------------
	//   Sending & receiving
	// --------------------------

	int				Send( NLsocket sock );
	int				Recv( NLsocket sock );


	// --------------------------
	//   Other stuff
	// --------------------------

	char			*toString(char *sBuffer);
	int				getLength(void)				{ return iLength; }
	int				getCurByte(void)			{ return iCurByte; }
	int				getCurBit(void)				{ return iCurBit; }
	uchar			*getData(void)				{ return sData; }

	void			setPos(int p)				{ iCurByte=p; iCurBit=0; }

	void			Append(CBitstream *bs);
	void			AppendData(char *_data, int _length);



};



#endif  //  __CBITSTREAM_H__