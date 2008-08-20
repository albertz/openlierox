#include "font.hpp"
#include "reader.hpp"
#include "gfx.hpp"
#include "colour.hpp"
#include <iostream>

void Font::loadFromEXE()
{
	chars.resize(250);
	
	std::size_t const FontSize = 250 * 8 * 8 + 1;
	std::vector<unsigned char> temp(FontSize);
	
	FILE* exe = openLieroEXE();
	
	fseek(exe, 0x1C825, SEEK_SET);
	
	fread(&temp[0], 1, FontSize, exe);
	
	for(int i = 0; i < 250; ++i)
	{
		unsigned char* ptr = &temp[i*64 + 1];
		
		for(int y = 0; y < 8; ++y)
		{
			for(int x = 0; x < 7; ++x)
			{
				chars[i].data[y*7 + x] = ptr[y*8 + x];
			}
		}
		
		chars[i].width = ptr[63];
	}
}

void Font::drawChar(unsigned char c, int x, int y, int colour)
{
	if(c >= 2 && c < 252
	&& x >= 0 && x < gfx.screen->w-7)
	{
		PalIdx* scr = &gfx.getScreenPixel(x, y);
		unsigned char* fnt = chars[c].data;
		
		for(int cy = 8; cy > 0; --cy)
		{
			for(int cx = 7; cx > 0; --cx)
			{
				if(*fnt) *scr = colour;
				++scr; ++fnt;
			}
			
			scr += gfx.screen->pitch - 7;
		}
	}
}

void Font::drawText(char const* str, std::size_t len, int x, int y, int colour)
{
	if(y >= 0 && y < gfx.screen->h-8)
	{
		int orgX = x;
		
		for(std::size_t i = 0; i < len; ++str, ++i)
		{
			unsigned char c = static_cast<unsigned char>(*str);
			
			if(!c)
			{
				x = orgX;
				y += 8;
			}
			else if(c >= 2 && c < 252)
			{
				c -= 2;
				
				if(x >= 0 && x < gfx.screen->w-7)
				{
					PalIdx* scr = &gfx.getScreenPixel(x, y);
					unsigned char* fnt = chars[c].data;
					
					for(int cy = 8; cy > 0; --cy)
					{
						for(int cx = 7; cx > 0; --cx)
						{
							if(*fnt) *scr = colour;
							++scr; ++fnt;
						}
						
						scr += gfx.screen->pitch - 7;
					}
				}
				
				x += chars[c].width;
			}
		}
	}
}

int Font::getWidth(char const* str, std::size_t len)
{
	int width = 0;
	
	for(std::size_t i = 0; i < len; ++str, ++i)
	{
		unsigned char c = static_cast<unsigned char>(*str);
		if(c >= 2 && c < 252)
			width += chars[c - 2].width;
	}
	
	return width;
}
