#include "level.hpp"

#include "game.hpp"
#include "gfx.hpp"
#include "colour.hpp"
#include "filesystem.hpp"
#include <cstring>

void Level::generateDirtPattern()
{
	width = 504;
	height = 350;
	data.resize(width * height);
	
	pixel(0, 0) = game.rand(7) + 12;
	
	for(int y = 1; y < height; ++y)
		pixel(0, y) = ((game.rand(7) + 12) + pixel(0, y - 1)) >> 1;
		
	for(int x = 1; x < width; ++x)
		pixel(x, 0) = ((game.rand(7) + 12) + pixel(x - 1, 0)) >> 1;
		
	for(int y = 1; y < height; ++y)
	for(int x = 1; x < width; ++x)
	{
		pixel(x, y) = (pixel(x - 1, y) + pixel(x, y - 1) + game.rand(8) + 12) / 3;
	}
	
	// TODO: Optimize the following
	
	int count = game.rand(100);
	
	for(int i = 0; i < count; ++i)
	{
		int x = game.rand(width) - 8;
		int y = game.rand(height) - 8;
		
		int temp = game.rand(4) + 69;
		
		PalIdx* image = gfx.largeSprites.spritePtr(temp);
		
		for(int cy = 0; cy < 16; ++cy)
		{
			int my = cy + y;
			if(my >= height)
				break;
				
			if(my < 0)
				continue;
			
			for(int cx = 0; cx < 16; ++cx)
			{
				int mx = cx + x;
				if(mx >= width)
					break;
					
				if(mx < 0)
					continue;
					
				PalIdx srcPix = image[(cy << 4) + cx];
				if(srcPix > 0)
				{
					PalIdx& pix = pixel(mx, my);
					if(pix > 176 && pix < 180)
						pix = (srcPix + pix) / 2;
					else
						pix = srcPix;
				}
			}
		}
	}
	
	count = game.rand(15);
	
	for(int i = 0; i < count; ++i)
	{
		int x = game.rand(width) - 8;
		int y = game.rand(height) - 8;
		
		int which = game.rand(4) + 56;
		
		blitStone(false, gfx.largeSprites.spritePtr(which), x, y);
	}
}

bool isNoRock(int size, int x, int y)
{
	Rect rect(x, y, x + size + 1, y + size + 1);
	
	rect.intersect(Rect(0, 0, game.level.width, game.level.height));
	
	for(int y = rect.y1; y < rect.y2; ++y)
	for(int x = rect.x1; x < rect.x2; ++x)
	{
		if(game.materials[game.level.pixel(x, y)].rock())
			return false;
	}
	
	return true;
}

void Level::generateRandom()
{
	game.settings.levelFile.clear();
	game.settings.randomLevel = true;
	gfx.resetPalette(gfx.exepal);
	
	generateDirtPattern();
	
	int count = game.rand(50) + 5;
	
	for(int i = 0; i < count; ++i)
	{
		int cx = game.rand(width) - 8;
		int cy = game.rand(height) - 8;
		
		int dx = game.rand(11) - 5;
		int dy = game.rand(5) - 2;
		
		int count2 = game.rand(12);
		
		for(int j = 0; j < count2; ++j)
		{
			int count3 = game.rand(5);
		
			for(int k = 0; k < count3; ++k)
			{
				cx += dx;
				cy += dy;
				drawDirtEffect(1, cx, cy); // TODO: Check if it really should be dirt effect 1
			}
			
			cx -= (count3 + 1) * dx; // TODO: Check if it really should be (count3 + 1)
			cy -= (count3 + 1) * dy; // TODO: Check if it really should be (count3 + 1)
			
			cx += game.rand(7) - 3;
			cy += game.rand(15) - 7;
		}
	}
	
	count = game.rand(15) + 5;
	for(int i = 0; i < count; ++i)
	{
		int cx, cy;
		do
		{
			cx = game.rand(game.level.width) - 16;
			
			if(game.rand(4) == 0)
				cy = game.level.height - 1 - game.rand(20);
			else
				cy = game.rand(game.level.height) - 16;
		}
		while(!isNoRock(32, cx, cy));
		
		int rock = game.rand(3);
		
		blitStone(false, gfx.largeSprites.spritePtr(stoneTab[rock][0]), cx, cy);
		blitStone(false, gfx.largeSprites.spritePtr(stoneTab[rock][1]), cx + 16, cy);
		blitStone(false, gfx.largeSprites.spritePtr(stoneTab[rock][2]), cx, cy + 16);
		blitStone(false, gfx.largeSprites.spritePtr(stoneTab[rock][3]), cx + 16, cy + 16);
	}
	
	count = game.rand(25) + 5;
	
	for(int i = 0; i < count; ++i)
	{
		int cx, cy;
		do
		{
			cx = game.rand(game.level.width) - 8;
			
			if(game.rand(5) == 0)
				cy = game.level.height - 1 - game.rand(13);
			else
				cy = game.rand(game.level.height) - 8;
		}
		while(!isNoRock(15, cx, cy));
		
		blitStone(0, gfx.largeSprites.spritePtr(game.rand(6) + 3), cx, cy);
	}
}

void Level::makeShadow()
{
	for(int x = 0; x < width - 3; ++x)
	for(int y = 3; y < height; ++y)
	{
		if(game.materials[pixel(x, y)].seeShadow()
		&& game.materials[pixel(x + 3, y - 3)].dirtRock())
		{
			pixel(x, y) += 4;
		}
		
		if(pixel(x, y) >= 12
		&& pixel(x, y) <= 18
		&& game.materials[pixel(x + 3, y - 3)].rock())
		{
			pixel(x, y) -= 2;
			if(pixel(x, y) < 12)
				pixel(x, y) = 12;
		}
	}
	
	for(int x = 0; x < width; ++x)
	{
		if(game.materials[pixel(x, height - 1)].background())
		{
			pixel(x, height - 1) = 13;
		}
	}
}

bool Level::load(std::string const& path)
{
	width = 504;
	height = 350;
	data.resize(width * height);
	
	ScopedFile f(tolerantFOpen(path.c_str(), "rb"));
	if(!f)
		return false;
		
	std::size_t len = fileLength(f);
	
	if(len > 504*350)
	{
		std::fseek(f, 504*350, SEEK_SET);
		char buf[10];
		
		std::fread(buf, 1, 10, f);
		
		if(!std::memcmp("POWERLEVEL", buf, 10))
		{
			Palette pal;
			pal.read(f);
			gfx.resetPalette(pal);
			
			std::fseek(f, 0, SEEK_SET);
			std::fread(&data[0], 1, width * height, f);
			return true;
		}
	}
	
	std::fseek(f, 0, SEEK_SET);
	std::fread(&data[0], 1, width * height, f);
	gfx.resetPalette(gfx.exepal);
	
	return true;
}
