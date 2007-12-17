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


#include <assert.h>
#include <stdarg.h>
#include <iostream>
#include <iomanip>
#include "CBytestream.h"
#include "EndianSwap.h"
#include "StringUtils.h"
#ifdef DEBUG
#include "MathLib.h"
#endif

using namespace std;

void CBytestream::Test()
{
	std::cout << std::endl;
	std::cout << "Running a Bytestream debug test:" << std::endl;
	std::cout << "Tested function / Original / Write / Read / Warning" << std::endl;

	// Byte
	uchar b = 125;
	std::cout << "Byte: (" << b << ") ";
	writeByte(b);
	ResetPosToBegin();
	std::cout << "(" << Data << ") ";
	uchar b2 = readByte();
	std::cout << "(" << b2 << ") ";
	if (b2 != b)
		std::cout << "NOT SAME!";
	std::cout <<std::endl;
	Clear();

	// Bool
	bool boo = true;
	std::cout << "Bool: (" << boo << ") ";
	writeByte(boo);
	ResetPosToBegin();
	std::cout << "(" << Data << ") ";
	bool boo2 = readBool();
	std::cout << "(" << boo2 << ") ";
	if (boo2 != boo)
		std::cout << "NOT SAME!";
	std::cout << std::endl;
	Clear();

	// Integer
	int i = 125;
	std::cout << "Int: (" << i << ") ";
	writeInt(i, 4);
	ResetPosToBegin();
	std::cout << "(" << Data << ") ";
	int i2 = readInt(4);
	std::cout << "(" << itoa(i2) << ") ";
	if (i2 != i)
		std::cout << "NOT SAME!";
	std::cout <<std::endl;
	Clear();

	// Short
	short s = 125;
	std::cout << "Short: (" << s << ") ";
	writeInt16(s);
	ResetPosToBegin();
	std::cout << "(" << Data << ") ";
	short s2 = readInt16();
	std::cout << "(" << s2 << ") ";
	if (s2 != s)
		std::cout << "NOT SAME!";
	std::cout <<std::endl;
	Clear();

	// Float
	float f = 125.125f;
	std::cout << "Float: (" << f << ") ";
	writeFloat(f);
	ResetPosToBegin();
	std::cout << "(" << Data << ") ";
	float f2 = readFloat();
	std::cout << "(" << f2 << ") ";
	if (f2 != f)
		std::cout << "NOT SAME!";
	std::cout <<std::endl;
	Clear();

	// String
	std::string str = "Test";
	std::cout << "String: (" << str << ") ";
	writeString(str);
	ResetPosToBegin();
	std::cout << "(" << Data << ") ";
	std::string str2 = readString();
	std::cout << "(" << str2 << ") ";
	if (str2 != str)
		std::cout << "NOT SAME!";
	std::cout <<std::endl;
	Clear();

	// 2Int12
	short x = 125;
	short y = 521;
	std::cout << "2Int12: (" << x << "/" << y << ") ";
	write2Int12(x, y);
	ResetPosToBegin();
	std::cout << "(" << Data << ") ";
	short x2, y2;
	read2Int12(x2, y2);
	std::cout << "(" << x2 << "/" << y2 << ") ";
	if (x2 != x || y2 != y)
		std::cout << "NOT SAME!";
	std::cout <<std::endl;
	Clear();

	// 2Int4
	short u = 10;
	short v = 12;
	std::cout << "2Int4: (" << u << "/" << v << ") ";
	write2Int4(u, v);
	ResetPosToBegin();
	std::cout << "(" << Data << ") ";
	short u2, v2;
	read2Int4(u2, v2);
	std::cout << "(" << u2 << "/" << v2 << ") ";
	if (u2 != u || v2 != v)
		std::cout << "NOT SAME!";
	std::cout <<std::endl;
	Clear();
	
	// Bits
	writeBit(1); 
	writeBit(1); 
	writeBit(0); 
	writeBit(1);
	writeBit(1);
	writeBit(1);
	writeBit(0);
	writeBit(0);
	writeBit(0);
	writeBit(1);
	std::cout << "Bits: (" << (unsigned)Data[0] << ", " << (unsigned)Data[1] << ") ";
	ResetPosToBegin();
	if(	
		readBit() != 1 ||
		readBit() != 1 ||
		readBit() != 0 ||
		readBit() != 1 ||
		readBit() != 1 ||
		readBit() != 1 ||
		readBit() != 0 ||
		readBit() != 0 ||
		readBit() != 0 ||
		readBit() != 1
		)
		std::cout << "NOT SAME!";
	std::cout <<std::endl;
	Clear();

	std::string str1( "Lala\0_1\0!2345", 12 );
	std::cout << "Data: " << str1.size() << " ";
	writeData(str1);
	//ResetPosToBegin();
	readByte();
	if( readData(1) != "a" )
		std::cout << "NOT SAME!";
	str2 = readData();
	if( str2.size() <= 4 || str2[5] != 0 )
		std::cout << "\"" << str2 << "\" size " << str2.size() << " NOT SAME!";
	std::cout <<std::endl;
	Clear();


}

void CBytestream::Clear() {
	Data = "";
	pos = 0;
	bitPos = 0;
}


///////////////////
// Append another bytestream onto this one
void CBytestream::Append(CBytestream *bs) {
	Data += bs->Data;
}


///////////////////
// Dump the data out
void CBytestream::Dump() {
	size_t i = 0;
	for(string::const_iterator it = Data.begin(); it != Data.end(); it++, i++) {
		if((uchar)*it <= 127 && (uchar)*it >= 32) {
			// Write out the byte and its ascii representation
			if(*it != '\\')
				cout << *it;
			else
				cout << "\\\\";
		} else
			cout << "\\" << hex << (uint)(uchar)*it << dec;
 
		// Linebreak after 16 dumped bytes
		if((i % 16) == 15)
			cout << endl;
	}
 
	cout << endl;
}



// Writes


///////////////////
// Writes a single byte
bool CBytestream::writeByte(uchar byte)
{
	Data += byte;
	return true;
}


///////////////////
// Writes a boolean value to the stream
bool CBytestream::writeBool(bool value)
{
	return writeByte((uchar)value);
}


///////////////////
// Writes an integer to the stream
bool CBytestream::writeInt(int value, uchar numbytes)
{
	// Numbytes cannot be more then 4
	assert(numbytes > 0 && numbytes < 5);

	// HINT: we send always in little endian
	Uint32 val = (Uint32)value;
	EndianSwap(val);

	for(short n = 0; n < numbytes; n++)
		writeByte( ((uchar *)&val)[n] );

	return true;
}


///////////////////
// Write a short to the stream
bool CBytestream::writeInt16(Sint16 value)
{
	// HINT: this time, the value is stored in big endian
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
	ByteSwap5(value);
#endif

	writeByte( ((uchar *)&value)[0]);
	writeByte( ((uchar *)&value)[1]);

	return true;
}


///////////////////
// Writes a float to the stream
bool CBytestream::writeFloat(float value)
{
	union {
		uchar bin[4];
		float val;
	} tmp;
	tmp.val = value;

	// HINT: original LX uses little endian floats over network
	EndianSwap(tmp.bin);

	for(short i = 0; i < 4; i++)
		writeByte(tmp.bin[i]);
			
	return true;
}


bool CBytestream::writeString(const std::string& value) {
	Data += value.c_str(); // convert it to a C-string because we don't want null-bytes in it
	Data += (char)'\0';
	
	return true;
}

// cast 2 int12 to 3 bytes
bool CBytestream::write2Int12(short x, short y) {
	writeByte((ushort)x & 0xff);
	writeByte((((ushort)x & 0xf00) >> 8) + (((ushort)y & 0xf) << 4));
	writeByte(((ushort)y & 0xff0) >> 4);
	return true;
}

// cast 2 int4 to 1 byte
bool CBytestream::write2Int4(short x, short y) {
	return writeByte(((ushort)x & 0xf) + (((ushort)y & 0xf) << 4));
}


bool CBytestream::writeBit(bool bit)
{
	if( bitPos == 0 )
		writeByte( 0 );
	uchar byte = Data[ Data.size() - 1 ];
	byte = byte | ( ( bit ? 1 : 0 ) << bitPos );
	Data[ Data.size() - 1 ] = byte;
	bitPos ++;
	if( bitPos >= 8 )
		bitPos = 0;
	return true;
}

bool CBytestream::writeData(const std::string& value)
{
	Data.append( value );
	return true;
};



// Reads




///////////////////
// Reads a single byte
uchar CBytestream::readByte(void) {
	if(!isPosAtEnd())
		return Data[pos++];
	else {
		printf("WARNING: reading from stream behind end\n");
		return 0;
	}
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

	Uint32 ret = 0;
	for(short n=0; n<numbytes; n++)
		ret += (Uint32)readByte() << (n * 8);
	
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
	value = (Uint16)dat[0];
	value += (Uint16)dat[1] << 8;

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


std::string CBytestream::readString() {
	static std::string result; result = "";
	uchar b;
	while((b = readByte()) != 0) result += (char)b;
	return result;
}

std::string CBytestream::readString(size_t maxlen) {
	static std::string result; result = "";
	size_t i = 0;
	uchar b = 0;
	while(i < maxlen && (b = readByte()) != 0) {
		result += b;
		++i;
	}
	if(b != 0)
		printf("WARNING: CBytestream: stop reading string at no real ending\n");
	return result;
}

// cast 3 bytes to 2 int12
void CBytestream::read2Int12(short& x, short& y) {
	static ushort dat[3];
	dat[0] = readByte();
	dat[1] = readByte();
	dat[2] = readByte();

	x = dat[0] + ((dat[1] & 0xf) << 8);
	y = (short)(((dat[1] & 0xf0) >> 4) + (dat[2] << 4));
}

// cast 1 byte to 2 int4
void CBytestream::read2Int4(short& x, short& y) {
	uchar tmp = readByte();
	x = tmp & 0xf;
	y = (short)((tmp & 0xf0) >> 4);
}


bool CBytestream::readBit()
{
	if( isPosAtEnd() )
	{
		printf("WARNING: reading from stream behind end\n");
		return false;
	}
	bool ret = Data[pos] & ( 1 << bitPos );
	bitPos ++;
	if( bitPos >= 8 )
	{
		bitPos = 0;
		pos ++;
	};
	return ret;
};

std::string CBytestream::readData( uint size )
{
	size = MIN( size, GetLength() - pos );
	uint oldpos = pos;
	pos += size;
	return Data.substr( oldpos, size );
};

// Skips a string, including the terminating character
// Returns true if we're at the end of the stream after the skip
bool CBytestream::SkipString() {
	readString();
	return isPosAtEnd();
}

bool CBytestream::Skip(size_t num) {
	pos += num;
	return isPosAtEnd();
}

////////////////
// Read from network
// WARNING: overrides any previous data
size_t CBytestream::Read(NetworkSocket sock) {
	Clear();
	static char buf[4096];
	size_t len = 0;
	int res; // MUST be signed, else an overflow can occur (ReadScoket can return -1!)
	while(true) {
		res = ReadSocket(sock, buf, sizeof(buf));
		if(res <= 0) break;
		Data.append(buf, res);
		len += res;
		if((size_t)res < sizeof(buf)) break;
	}

#ifdef DEBUG
	// DEBUG: randomly drop packets to test network stability
/*	if (GetRandomInt(128) > 110)  {
		printf("DEBUG: packet ignored\n");
		Dump();
		printf("\n");
		Clear();
		return 0;
	}*/
#endif

	return len;
}

bool CBytestream::Send(NetworkSocket sock) {
	return (size_t)WriteSocket(sock, Data.data(), Data.size()) == Data.size();
}

