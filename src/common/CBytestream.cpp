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

#include <stdarg.h>

#include "defs.h"
#include "CBytestream.h"
#include "StringUtils.h"


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
    static char buf[MAX_DATA+1];

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
int CBytestream::writeBool(bool value)
{
	return writeByte((uchar)value);
}


///////////////////
// Writes a signed short
/*void CBytestream::writeSShort(short value)
{


}*/


///////////////////
// Writes an integer to the stream
int CBytestream::writeInt(int value, uchar numbytes)
{
	// Numbytes cannot be more then 4
	if(numbytes <= 0 || numbytes >= 5)
		return false;

	// Copy the interger into individual bytes
	// HINT: this is endian independent code; it uses little endian
	uchar bytes[4];
	bytes[0] = (uint)value & 0xff;
	bytes[1] = ((uint)value & 0xff00) / 0x100;
	bytes[2] = ((uint)value & 0xff0000) / 0x10000;
	bytes[3] = ((uint)value & 0xff000000) / 0x1000000;

	for(short n=0;n<numbytes;n++)
		if(!writeByte(bytes[n]))
			return false;

	return true;
}


///////////////////
// Write a short to the stream
int CBytestream::writeInt16(Sint16 value)
{
	// HINT: this time, the value is stored in big endian
	uchar dat[2];
	dat[1] = (Uint16)value & 0xff;
	dat[0] = ((Uint16)value & 0xff00) / 0x100;

	if (!writeByte(dat[0]))
		return false;

	if (!writeByte(dat[1]))
		return false;

	return true;
}


///////////////////
// Writes a float to the stream
int CBytestream::writeFloat(float value)
{
	union {
		uchar bin[4];
		float val;
	} tmp;
	tmp.val = value;

	// HINT: original LX uses little endian floats over network
	EndianSwap(tmp.bin);

	for(short i = 0; i < 4; i++)
		if(!writeByte(tmp.bin[i]))
			return false;
			
	return true;
}


///////////////////
// Write a string to the stream
int CBytestream::writeString(char *fmt,...)
{
	static char buf[1024];
	va_list	va;

	va_start(va,fmt);
	vsnprintf(buf,sizeof(buf),fmt,va);	fix_markend(buf);
	va_end(va);

	int len = fix_strnlen(buf);

	if(len+CurByte >= MAX_DATA)
		return false;

	memcpy((char *)Data+CurByte,buf,len);
	Data[CurByte+len] = '\0';
	CurByte+=len+1;
	Length+=len+1;

	return true;
}

int	CBytestream::writeString(const std::string& value) {
	size_t len = value.length();
	
	if(len + CurByte >= MAX_DATA)
		return false;
	
	memcpy((char*)Data+CurByte, value.c_str(), len);
	Data[CurByte + len] = '\0';
	CurByte += len + 1;
	Length += len + 1;
	
	return true;
}

// cast 2 int12 to 3 bytes
int	CBytestream::write2Int12(short x, short y) {
	if(!writeByte(x & 0xff)) return false;
	if(!writeByte(((x & 0xf00) / 0x100) + (y & 0xf) * 0x10)) return false;
	if(!writeByte((y & 0xff0) / 0x10)) return false;
	return true;
}

// cast 2 int4 to 1 byte
int CBytestream::write2Int4(short x, short y) {
	return writeByte((x & 0xf) + (y & 0xf) * 0x10);
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
bool CBytestream::readBool(void)
{
	return readByte() != 0;
}


///////////////////
// Reads an interger value from the stream
int CBytestream::readInt(uchar numbytes)
{
	// Numbytes cannot be more than 4
	if(numbytes <= 0 || numbytes >= 5)
		return 0;

	uchar bytes[4];
	for(short n=0;n<numbytes;n++)
		bytes[n] = readByte();

	// HINT: this is endian independent; value is stored in little endian
	uint ret = 0;	
	if(numbytes>0)
		ret = (uint)bytes[0];
	if(numbytes>1)
		ret += (uint)bytes[1] * 0x100;
	if(numbytes>2)
		ret += (uint)bytes[2] * 0x10000;
	if(numbytes>3)
		ret += (uint)bytes[3] * 0x1000000;
	
	return (int)ret;
}


///////////////////
// Read a short from the stream
Sint16 CBytestream::readInt16(void)
{
	// HINT: this time, the value is stored in big endian
	uchar dat[2];
	dat[1] = readByte();
	dat[0] = readByte();

	Uint16 value;
	value = dat[0];
	value += dat[1] * 0x100;

	return (Sint16)value;
}



///////////////////
// Read a float value from the stream
float CBytestream::readFloat(void)
{
	union {
		uchar bin[4];
		float val;
	} tmp;

	tmp.val = 0;
	
	for(short i = 0; i < 4; i++)
		tmp.bin[i] = readByte();

	// HINT: original LX uses little endian floats over network
	EndianSwap(tmp.bin);

	return tmp.val;
}


///////////////////
// Read a string from the stream
char *CBytestream::readString(char *str, size_t maxlen)
{
	if (!str)
		return false;

	// Validate that there is some terminating character
	bool valid = false;
	size_t len = 0;
	for (len=CurByte; len<(size_t)GetLength(); len++)
		if(Data[len] == '\0')
		{
			valid = true;
			len -= CurByte;
			break;	
		}

	// Invalid
	if (!valid || len >= maxlen)  {
		str[0] = '\0';
		return str;
	}

	memcpy(str,(char *)Data+CurByte, MIN(len+1, maxlen-1));
	str[maxlen-1] = '\0';
	CurByte += len+1;

	return str;
}

std::string CBytestream::readString() {
	static std::string result;
	size_t i;
	size_t len = (size_t)GetLength();
	for(i=CurByte; i<len; i++)
		if(Data[i] == '\0') {
			result = std::string((char*)(&Data[CurByte]), i-CurByte);
			CurByte = i+1;
			return result;		
		}

	return "";
}

std::string CBytestream::readString(size_t maxlen) {
	static std::string result;
	size_t i;
	size_t len = MIN((size_t)GetLength(), CurByte+maxlen+1);
	for(i=CurByte; i<len; i++)
		if(Data[i] == '\0') {
			result = std::string((char *)(&Data[CurByte]), i-CurByte);
			CurByte = i+1;
			return result;		
		}

	return "";
}

// cast 3 bytes to 2 int12
void CBytestream::read2Int12(short& x, short& y) {
	short dat[3];
	dat[0] = readByte();
	dat[1] = readByte();
	dat[2] = readByte();

	x = dat[0] + ((dat[1] & 0xf) * 0x100);
	y = (short)(((dat[1] & 0xf0) / 0x10) + dat[2] * 0x10);
}

// cast 1 byte to 2 int4
void CBytestream::read2Int4(short& x, short& y) {
	uchar tmp = readByte();
	x = tmp & 0xf;
	y = (short)((tmp & 0xf0) / 0x10);
}

// Skips a string, including the terminating character
// Returns true if we're at the end of the stream after the skip
bool CBytestream::SkipString()  {
	for (;CurByte<Length;CurByte++) {
		if (!Data[CurByte])  {  // Zero byte terminates the string
			CurByte++; // Skip the zero byte as well
			break;
		}
	}
	if (CurByte >= Length-1)  {
		CurByte = Length-1;
		return true;
	}
	return false;  // Stream not yet ending
}