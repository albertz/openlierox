/////////////////////////////////////////
//
//   Auxiliary Software class library
//
/////////////////////////////////////////


// Byte stream class
// Created 13/10/01
// Jason Boettcher


#include "defs.h"


// TODO: Little endian support


///////////////////
// Append another bytestream onto this one
void CBytestream::Append(CBytestream *bs)
{
	// Check to make sure we don't overflow the buffer
	if(CurByte + bs->GetLength() >= MAX_DATA) {
		d_printf("byte stream is too big to append any extra data\n");
		return;
	}

	memcpy(Data+CurByte,bs->GetData(),bs->GetLength());
	CurByte+=bs->GetLength();
	Length+=bs->GetLength();
}


///////////////////
// Dump the data out
void CBytestream::Dump(void)
{
    char buf[MAX_DATA+1];

    memcpy(buf, Data, Length);
    buf[Length] = '\0';

    fwrite(buf,1,Length,stdout);

}


///////////////////
// Append some data onto this bytestream
void CBytestream::AppendData(char *_data, int _length)
{
	// Check to make sure we don't overflow the buffer
	if(CurByte + _length >= MAX_DATA)
		return;

	memcpy(Data+CurByte,_data,_length);
	CurByte += _length;
	Length += _length;
}



// Writes


///////////////////
// Writes a single byte
int CBytestream::writeByte(uchar byte)
{
	if(CurByte >= MAX_DATA)
		return false;

	Data[CurByte] = byte;
	CurByte++;
	Length++;
	return true;
}


///////////////////
// Writes a boolean value to the stream
int CBytestream::writeBool(int value)
{
	return writeByte((uchar) value);
}


///////////////////
// Writes a signed short
/*void CBytestream::writeSShort(short value)
{


}*/


///////////////////
// Writes an integer to the stream
int CBytestream::writeInt(int value, int numbytes)
{
	uchar bytes[4];
	int n;

	// Numbytes cannot be more then 4
	if(numbytes <= 0 || numbytes >= 5)
		return false;

	// Copy the interger into individual bytes
	bytes[0] = value & 0xff;
	bytes[1] = (value>>8) & 0xff;
	bytes[2] = (value>>16) & 0xff;
	bytes[3] = value>>24;

	for(n=0;n<numbytes;n++)
		if(!writeByte(bytes[n]))
			return false;

	return true;
}


///////////////////
// Write a short to the stream
int CBytestream::writeShort(short value)
{
	uchar data[4];
	int a=0;
	nl_writeShort(data,a,value);

	writeByte(data[0]);
	writeByte(data[1]);

	return true;
}


///////////////////
// Writes a float to the stream
int CBytestream::writeFloat(float value)
{
	// Create a 4 byte union
	union
	{
		float   f;
		int     l;
	} dat;

	dat.f = value;
	return writeInt(dat.l,4);
}


///////////////////
// Write a string to the stream
int CBytestream::writeString(char *fmt,...)
{
	char buf[1024];
	va_list	va;

	va_start(va,fmt);
	vsprintf(buf,fmt,va);
	va_end(va);

	int len = strlen(buf);

	if(len+CurByte >= MAX_DATA)
		return false;

	strcpy((char *)Data+CurByte,buf);
	CurByte+=len+1;
	Length+=len+1;

	return true;
}





// Reads




///////////////////
// Reads a single byte
uchar CBytestream::readByte(void)
{
	if(CurByte >= MAX_DATA)
		return 0;

	return Data[CurByte++];
}


///////////////////
// Reads a boolean value from the stream
int CBytestream::readBool(void)
{
	return readByte();
}


///////////////////
// Reads an interger value from the stream
int CBytestream::readInt(int numbytes)
{
	uchar bytes[4];
	int n;
	int value;

	// Numbytes cannot be more than 4
	if(numbytes <= 0 || numbytes >= 5)
		return false;

	for(n=0;n<numbytes;n++)
		bytes[n] = readByte();

	if(numbytes>0)
		value = (int)bytes[0];
	if(numbytes>1)
		value+= (int)bytes[1]<<8;
	if(numbytes>2)
		value+= (int)bytes[2]<<16;
	if(numbytes>3)
		value+= (int)bytes[3]<<24;
		
	return value;
}


///////////////////
// Read a short from the stream
short CBytestream::readShort(void)
{
	uchar data[4];
	short value = 0;

	data[0] = readByte();
	data[1] = readByte();
	
	int a=0;
	nl_readShort(data,a,value);

	return value;
}



///////////////////
// Read a float value from the stream
float CBytestream::readFloat(void)
{
	// Create a 4 byte union
	union
	{
		uchar	b[4];
		float	f;
		int		l;
	} dat;

	dat.b[0] = readByte();
	dat.b[1] = readByte();
	dat.b[2] = readByte();
	dat.b[3] = readByte();

	return dat.f;   
}


///////////////////
// Read a string from the stream
char *CBytestream::readString(char *str)
{
	bool valid = false;
	for (int i=CurByte; i<GetLength(); i++)
		if(Data[i] == '\0')
			valid = true;

	if (!valid)
		return "";

	strcpy(str,(char *)Data+CurByte);
	CurByte += strlen(str)+1;

	return str;
}