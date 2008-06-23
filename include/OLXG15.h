//
// C++ Interface: OLXG15
//
// Description:see .cpp
//
//
// Author:  <>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifdef WITH_G15
#ifndef __OLXG15_H__
#define __OLXG15_H__
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
// Perror
#include <cstdio>

#include <g15daemon_client.h>
//#include <libg15.h>
#include <libg15render.h>

class OLXG15_weapon_t
{
// To let it access chargeIndicator
friend class OLXG15_t;
private:
	int chargeIndicator;
public:
	std::string name;
	int charge;
	bool reloading;
	bool changed;

	OLXG15_weapon_t()
	{
		name = "";
		charge = 0;
		reloading = false;
		chargeIndicator = 0;
		changed = false;
	}
};
class OLXG15_t
{
private:
	int screenfd;
	int curScreen;
	int screenVer;
	g15canvas canvas;
	OLXG15_weapon_t Weapons[5];

public:
	OLXG15_t();
	~OLXG15_t();

	bool init();
	void frame();

	void drawBounds(const int& size);

	void drawWpnName(const std::string& name, const int& row, const int& size);
	void clearReload(const int& row, const int& size);
	void sayHi();
	void testWeaponScreen(const int& size);
	void renderWeapon(OLXG15_weapon_t& weapon, const int& size, const int& row);
	void showSplashScreen();

	int wpnSpace(const int& size) {
		switch (size) {
			case G15_TEXT_MED: return 131;
			case G15_TEXT_LARGE: return 114;
			default: /*G15_TEXT_SMALL*/ return 135;
		}
	}

	// Variables
	OLXG15_weapon_t* getWeapon(const int& num)		{ return &Weapons[num]; }

	// Small can take up to 40 characters. 3 pix wide 1 pix spacing 6 pix high (4x6)
	// Medium can take up to 32 characters. 4 pix wide 1 pix spacing 7 pix high (5x7)
	// Large can take up to 20 characters. 7 pix wide 1 pix spacing 8 pix high (8x8)
	int getCharWidth(const int& size)
	{
		switch (size)
		{
			case G15_TEXT_MED:
				return 5;
			case G15_TEXT_LARGE:
				return 8;
				default: //G15_TEXT_SMALL + incase we get an invalid size
					return 4;
		}
	}
	int getCharHeight(const int& size)
	{
		switch (size)
		{
			case G15_TEXT_MED:
				return 7;
			case G15_TEXT_LARGE:
				return 8;
				default: //G15_TEXT_SMALL + incase we get an invalid size
					return 6;
		}
	}

	// TODO: Ripped from my personal G15 header file, perhaps put in it's own headerfile here too?
	// Returns what X pixel to start on to get the specified effect
	int centerAlign(const std::string& txt, const int& size)
	{
		return (G15_LCD_WIDTH/2) - (txt.length()*getCharWidth(size))/2;
	}
	// Last pixel is actually #159 (starts on 0), but this removes the spacing (1pix).
	// For some unknown reason it's 161 instead of 160 to remove spacing?? (new: most likely front spacing)
	// Returns what X pixel to start on to get the specified effect
	int rightAlign(const std::string& txt, const int& size)
	{
		return (G15_LCD_WIDTH+1) - (txt.length()*getCharWidth(size));
	}
	// Returns what Y pixel to start on to center text at pixel y, will place them all on the same line
	// if bottom, returns pixel for getting the bottom of the text at pixel y;
	// This description sucks, just try it out, ok?
	int yAlign(const int& y,const int& size, const bool& bottom = false)
	{
		int height = getCharHeight(size);
		switch (size)
		{
			case G15_TEXT_SMALL:
				return bottom?(y - height):(y - height/3); //2
				default: // MED & LARGE has got the same formula
					return bottom?(y - height):(y - height/2); //4
		}
	}
	void drawXBM(g15canvas* canvas, unsigned char* data, const int& width, const int& height ,const int& pos_x, const int& pos_y)
	{
		int y = 0;
		int z = 0;
		unsigned char byte;
		int bytes_per_row = ceil((double) width / 8);

		int bits_left = width;
		int current_bit = 0;

		for(y = 0; y < height; y ++)
		{
			bits_left = width;
			for(z=0;z < bytes_per_row; z++)
			{
				byte = data[(y * bytes_per_row) + z];
				current_bit = 0;
				while(current_bit < 8)
				{
					if(bits_left > 0)
					{
						if((byte >> current_bit) & 1) g15r_setPixel(canvas, (current_bit + (z*8) + pos_x),y + pos_y,G15_COLOR_BLACK);
						bits_left--;
					}
					current_bit++;
				}
			}
		}
	}
};

extern OLXG15_t *OLXG15;

#endif //__OLXG15_H__
#endif //WITH_G15
