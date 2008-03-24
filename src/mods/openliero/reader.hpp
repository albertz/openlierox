#ifndef LIERO_READER_HPP
#define LIERO_READER_HPP

#include <cstdio>
#include <string>
#include <SDL/SDL.h>

extern std::string lieroEXERoot;
extern std::string lieroOPT;

// Return an opened file
FILE* openFile(std::string const& name);

FILE* openLieroEXE();
FILE* openLieroSND();
FILE* openLieroCHR();

inline std::string readPascalString(FILE* f)
{
	unsigned char length;
	fread(&length, 1, 1, f);
	char txt[256];
	fread(txt, 1, length, f);
	return std::string(txt, length);
}

inline std::string readPascalString(FILE* f, unsigned char fieldLen)
{
	char txt[256];
	fread(txt, 1, fieldLen, f);
	unsigned char length = static_cast<unsigned char>(txt[0]);
	return std::string(txt + 1, length);
}

inline void writePascalString(FILE* f, std::string const& str, unsigned char fieldLen)
{
	int len = int(str.size() < fieldLen ? str.size() : fieldLen - 1);
	std::size_t zeroes = fieldLen - 1 - len;
	fputc(len, f);
	fwrite(str.data(), 1, len, f);
	for(std::size_t i = 0; i < zeroes; ++i)
		fputc(0, f);
}

inline std::string readPascalStringAt(FILE* f, int location)
{
	unsigned char length;
	fseek(f, location, SEEK_SET);
	fread(&length, 1, 1, f);
	char txt[256];
	fread(txt, 1, length, f);
	return std::string(txt, length);
}

inline Uint32 readUint8(FILE* f)
{
	unsigned char temp[1];
	fread(temp, 1, 1, f);
	return temp[0];
}

inline void writeUint8(FILE* f, Uint32 v)
{
	fputc(v & 0xff, f);
}

inline Sint32 readSint8(FILE* f)
{
	char temp[1];
	fread(temp, 1, 1, f);
	return temp[0];
}

inline Uint32 readUint16(FILE* f)
{
	unsigned char temp[2];
	fread(temp, 1, 2, f);
	return temp[0] + (temp[1] << 8);
}

inline void writeUint16(FILE* f, Uint32 v)
{
	fputc(v & 0xff, f);
	fputc((v >> 8) & 0xff, f);
}

inline Sint32 readSint16(FILE* f)
{
	unsigned char temp[2];
	fread(temp, 1, 2, f);
	return temp[0] + (static_cast<char>(temp[1]) << 8);
}

inline Uint32 readUint32(FILE* f)
{
	unsigned char temp[4];
	fread(temp, 1, 4, f);
	return temp[0] + (temp[1] << 8) + (temp[2] << 16) + (temp[3] << 24);
}

inline Sint32 readSint32(FILE* f)
{
	unsigned char temp[4];
	fread(temp, 1, 4, f);
	return temp[0] + (temp[1] << 8) + (temp[2] << 16) + (static_cast<char>(temp[3]) << 24);
}

void setLieroEXE(std::string const& path);

// Close old files
void processReader();
void closeAllCachedFiles();

#endif // LIERO_READER_HPP
