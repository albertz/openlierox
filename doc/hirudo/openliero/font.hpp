#ifndef LIERO_FONT_HPP
#define LIERO_FONT_HPP

#include <vector>
#include <string>

struct Font
{
	struct Char
	{
		unsigned char data[8*7];
		int width;
	};
	
	Font()
	: chars(250)
	{
	}
	
	void loadFromEXE();
	void drawText(char const* str, std::size_t len, int x, int y, int colour);
	int getWidth(char const* str, std::size_t len);
	void drawChar(unsigned char ch, int x, int y, int colour);
	
	void drawText(std::string const& str, int x, int y, int colour)
	{
		drawText(str.data(), str.size(), x, y, colour);
	}
	
	int getWidth(std::string const& str)
	{
		return getWidth(str.data(), str.size());
	}
	
	std::vector<Char> chars;
};

#endif // LIERO_FONT_HPP
