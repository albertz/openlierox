#include "menu.hpp"

#include "gfx.hpp"
#include "reader.hpp"

/*
MenuItem mnuMainMenuItems[] =
{
	{10, 10, "RESUME GAME"},
	{10, 10, "NEW GAME"},
	{48, 48, "OPTIONS"},
	{6, 6,   "QUIT TO OS"}
};*/

void MenuItem::draw(int x, int y, bool selected, bool disabled, bool centered)
{
	int wid = gfx.font.getWidth(string);
	if(centered)
		x -= (wid >> 1);
	
	if(selected)
	{
		drawRoundedBox(x, y, 0, 7, wid);
	}
	else
	{
		gfx.font.drawText(string, x + 3, y + 2, 0);
	}
	
	PalIdx c;
	
	if(disabled)
		c = disColour;
	else if(selected)
		c = 168;
	else
		c = colour;
		
	gfx.font.drawText(string, x + 2, y + 1, c);
}

void Menu::draw(int x, int y, bool disabled, int selection, int firstItem, int lastItem)
{
	if(lastItem == -1)
		lastItem = int(items.size()) - 1;
		
	for(int c = firstItem; c <= lastItem; ++c)
	{
		if(disabled)
			items[c].draw(x, y, false, true, centered);
		else
		{
			if(c != selection)
			{
				items[c].draw(x, y, false, false, centered);
			}
			else
			{
				items[c].draw(x, y, true, false, centered);
			}
		}
		
		y += itemHeight;
	}
}

void Menu::readItems(FILE* f, int length, int count, bool colourPrefix, PalIdx colour, PalIdx disColour)
{
	char temp[256];
	for(int i = 0; i < count; ++i)
	{
		fread(&temp[0], 1, length, f);
		int offset = 1;
		int length = static_cast<unsigned char>(temp[0]);
		if(colourPrefix)
		{
			colour = disColour = temp[2];
			length -= 2;
			offset += 2;
		}
		items.push_back(MenuItem(colour, disColour, std::string(&temp[offset], length)));
	}
}

void Menu::readItem(FILE* f, int offset, PalIdx colour, PalIdx disColour)
{
	items.push_back(MenuItem(colour, disColour, readPascalStringAt(f, offset)));
}
