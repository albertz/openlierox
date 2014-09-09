#include "liero.h"
#include "../gfx.h"
#include "FindFile.h"
#include <string>
#include <vector>
#include <cstring>
#include <iostream>
using std::cerr;
using std::endl;

#ifndef DEDICATED_ONLY
LieroFontLoader LieroFontLoader::instance;

bool LieroFontLoader::canLoad(std::string const& path, std::string& name)
{
	if(GetFileExtensionWithDot(path) == ".lft")
	{
		name = GetBaseFilenameWithoutExt(path);
		return true;
	}
	return false;
}
	
bool LieroFontLoader::load(Font* font, std::string const& path)
{
	font->free();
	
	std::ifstream f;
	OpenGameFileR(f, path, std::ios::binary);
	if(!f)
		return false;
		
	long bitmapWidth = 7, bitmapHeight = 250 * 8;

	ALLEGRO_BITMAP* fontBitmap = create_bitmap_ex(8, (int)bitmapWidth, (int)bitmapHeight);
	if(!fontBitmap)
		return false;
		
	font->m_supportColoring = true;
		
	std::vector<char> buffer(/*16000*/ 249 * 8 * 8 + 64 + 1 );
	
	f.ignore(8); //First 8 bytes are useless
	f.read(&buffer[0], buffer.size());
	
	if(f.gcount() < (int)buffer.size())
		return false;

	font->m_chars.assign(2, Font::CharInfo(Rect(0, 0, 2, 2), 0)); // Two empty slots

	int y = 0;
	for(int i = 0; i < 250; ++i)
	{
		int width = buffer[i * 8 * 8 + 64];
		if(width < 2)
			width = 2;
			
		int beginy = y;
		int endy = y;

		for(int y2 = 0; y2 < 8; ++y2, ++y)
		{
			for(int x = 0; x < 7; ++x)
			{
				char v = buffer[y*8 + x + 1];
				
				if(v)
					endy = y;
				
				int c = v ? 255 : 0;

				putpixel(fontBitmap, x, y, c);
			}
		}
		
		font->m_chars.push_back(Font::CharInfo(Rect(0, beginy, width, endy + 1)*2, 0));
	}

	SmartPointer<SDL_Surface> doubleResFont = GetCopiedStretched2Image(fontBitmap->surf);
	destroy_bitmap(fontBitmap); fontBitmap = NULL;
	font->m_bitmap = create_bitmap_from_sdl(doubleResFont);

	font->buildSubBitmaps();
	
	return true;
}

const char* LieroFontLoader::getName()
{
	return "Liero font loader";
}

std::string LieroFontLoader::format() { return "Liero font"; }
std::string LieroFontLoader::formatShort() { return "Liero"; }


#endif
