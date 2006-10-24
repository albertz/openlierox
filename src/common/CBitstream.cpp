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


#include "defs.h"


///////////////////
// Clear the bitstream
void CBitstream::Clear(void)
{
	for(int i=0;i<MAX_STREAMDATA;i++)
		sData[i]=0;

	iLength=0;
	iCurByte = iCurBit = 0;
}


///////////////////
// Reset the bitstream
void CBitstream::Reset(void)
{
	iCurByte = iCurBit = 0;
}


///////////////////
// Write an int to the stream
// Returns the number of bits written
int CBitstream::writeInt(int iValue, int iNumBits)
{
	if(!iLength) {
		iLength=1;
		iCurByte=0;
	}

	for(int i=0;i<iNumBits;i++) {
		if(iCurBit > 7) {
			iCurBit=0;
			iCurByte++;
			iLength++;
		}

		int bit = (iValue>>i) & 0x01;
		sData[iCurByte] |= (bit << iCurBit);

		iCurBit++;
	}

	return iNumBits;
}


///////////////////
// Read an in from the stream
int CBitstream::readInt(int iNumBits)
{
	int value = 0;

	for(int i=0; i<iNumBits; i++) {
		
		// Over a byte?
		if(iCurBit > 7) {
			iCurBit=0;
			iCurByte++;
		}

		int bit = (sData[iCurByte] & (1<<iCurBit)) ? 1 : 0;
		value |= (bit<<i);

		iCurBit++;
	}

	return value;
}


///////////////////
// Write a float to the stream
// Returns the number of bits written
int CBitstream::writeFloat(float fValue)
{
	// Use a 4 byte union to convert a float to a 32bit int
	union
	{
		float   f;
		int     l;
	} dat;

	dat.f = fValue;
	return writeInt(dat.l,32);
}


///////////////////
// Read a float from the stream
float CBitstream::readFloat(void)
{
	// Use a 4 byte union to convert 4 8bit ints into a float
	union
	{
		unsigned char b[4];
		float	f;
		int		l;
	} dat;

	dat.b[0] = readInt(8);
	dat.b[1] = readInt(8);
	dat.b[2] = readInt(8);
	dat.b[3] = readInt(8);

	return dat.f;
}



///////////////////
// Write a vector to the stream
// Returns the number of bits written
int CBitstream::writeVec(CVec v)
{
	int value =  writeFloat(v.GetX());
		value += writeFloat(v.GetY());

	return value;
}


///////////////////
// Read a vector from the stream
CVec CBitstream::readVec(void)
{
	return CVec( readFloat(), readFloat() );
}


///////////////////
// Write a string to the stream (including null terminator)
// Returns the number of bits written
int CBitstream::writeString(char *sValue)
{
	int value=0;

	for(unsigned int b=0;b<=strlen(sValue);b++)
		value += writeInt(sValue[b],8);

	return value;
}


///////////////////
// Read a string from the stream (including null terminator)
char *CBitstream::readString(char *sValue, int iMax)
{
	int v;
	strcpy(sValue,"");

	for(int b=0;b<iMax;b++) {
		v = readInt(8);
		sValue[b] = v;
		if(v==0)
			break;
	}

	// End it in case the last character is still text
	sValue[iMax-1] = 0;

	return sValue;
}


///////////////////
// Convert the data to a string
char *CBitstream::toString(char *sBuffer)
{
	char buf[32];
	strcpy(sBuffer,"");

	// Go through each byte
	for(int b=0;b<iLength;b++) {
		int numbits = iLength-1 == b ? iCurBit : 8;

		sprintf(buf,"Byte=%d ", sData[b]);
		strcat(sBuffer,buf);
		

		// Go through each bit
		for(int i=0;i<numbits;i++)
			strcat(sBuffer, sData[b] & (1<<i) ? "1" : "0");
		
		strcat(sBuffer,"\n");
	}


	return sBuffer;
}


///////////////////
// Send the bitstream through a network connection
int CBitstream::Send( NLsocket sock )
{
	return nlWrite( sock, sData, iLength );
}


///////////////////
// Recieve data from a network connection
int CBitstream::Recv( NLsocket sock )
{
	iLength = nlRead( sock, sData, MAX_STREAMDATA );
	iCurByte = iCurBit = 0;
	return iLength;
}


///////////////////
// Append another bit stream onto this
void CBitstream::Append( CBitstream *bs )
{
	int bits = 8;
	int old = bs->getCurBit();

	// Check for overflow
	if(iLength + bs->getLength() >= MAX_STREAMDATA)
		return;

	bs->Reset();

	for(int i=0;i<bs->getLength(); i++) {
		if(i==bs->getLength()-1)
			bits = old;

		writeInt( bs->readInt(bits), bits);
	}
}


///////////////////
// Append raw data onto this bitstream
void CBitstream::AppendData(char *_data, int _length)
{
	// Check for overflow
	if(iLength + _length >= MAX_STREAMDATA)
		return;

	for(int i=0;i<_length;i++)
		writeInt( _data[i], 8 );
}