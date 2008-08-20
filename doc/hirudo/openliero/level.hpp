#ifndef LIERO_LEVEL_HPP
#define LIERO_LEVEL_HPP

#include <vector>
#include <string>
#include <cstdio>
#include "rect.hpp"
#include <SDL/SDL.h>

struct Level
{
	bool load(std::string const& path);
	
	void generateDirtPattern();
	void generateRandom();
	void makeShadow();
	
	unsigned char& pixel(int x, int y)
	{
		return data[x + y*width];
	}
	
	unsigned char checkedPixelWrap(int x, int y)
	{
		unsigned int idx = static_cast<unsigned int>(x + y*width);
		if(idx < data.size())
			return data[idx];
		return 0;
	}
	
	bool inside(int x, int y)
	{
		return static_cast<unsigned int>(x) < static_cast<unsigned int>(width)
		    && static_cast<unsigned int>(y) < static_cast<unsigned int>(height);
	}
	
	
	Rect rect()
	{
		return Rect(0, 0, width, height);
	}
	
	std::vector<unsigned char> data;
	//SDL_Surface* surf;
	
	int width;
	int height;
};

#endif // LIERO_LEVEL_HPP
