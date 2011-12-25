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

#include <sys/types.h>
#include <sys/stat.h>
// Perror
#include <cstdio>

#include <g15daemon_client.h>
//#include <libg15.h>
#include <libg15render.h>
#include "CodeAttributes.h"

const float G15FRAMETIME = 0.25f; // 4 frames/second, we shouldn't need more
const float G15SPLASHTIME = 5.0f;
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
	g15canvas canvas;
	OLXG15_weapon_t Weapons[5];
	int rows[5];

	int curWeapon;

	timeval startTime;
	bool showingSplash;
	float lastFrame;
	float timeShown; // For splash screen, perhaps for some kill message too?
	int oldGameState;

public:
	OLXG15_t();
	~OLXG15_t();

	bool init();

	void menuFrame();
	void gameFrame();


	void updateWeapon(const int slotID);
	void drawBounds(const int size);

	void drawWpnName(const std::string& name, const int row, const int size);
	void clearRow(const int row);
	void clearReload(const int row, const int size);
	void sayHi();
	void testWeaponScreen(const int size);
	void renderWeapon(const int wepNum, const int size);
	void showSplashScreen();

	int wpnSpace(const int size) {
		switch (size) {
			case G15_TEXT_MED: return 131;
			case G15_TEXT_LARGE: return 114;
			default: /*G15_TEXT_SMALL*/ return 135;
		}
	}

	// Small can take up to 40 characters. 3 pix wide 1 pix spacing 6 pix high (4x6)
	// Medium can take up to 32 characters. 4 pix wide 1 pix spacing 7 pix high (5x7)
	// Large can take up to 20 characters. 7 pix wide 1 pix spacing 8 pix high (8x8)
	INLINE int getCharWidth(const int size)
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
	INLINE int getCharHeight(const int size)
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


	// Returns what X pixel to start on to get the specified effect
	INLINE int centerAlign(const std::string& txt, const int size)
	{
		return (G15_LCD_WIDTH/2) - (txt.length()*getCharWidth(size))/2;
	}
	// Last pixel is actually #159 (starts on 0), but this removes the spacing (1pix).
	// For some unknown reason it's 161 instead of 160 to remove spacing?? (new: most likely front spacing)
	// Returns what X pixel to start on to get the specified effect
	INLINE int rightAlign(const std::string& txt, const int size)
	{
		return (G15_LCD_WIDTH+1) - (txt.length()*getCharWidth(size));
	}
	// Returns what Y pixel to start on to center text at pixel y, will place them all on the same line
	// if bottom, returns pixel for getting the bottom of the text at pixel y;
	// This description sucks, just try it out, ok?
	INLINE int yAlign(const int y,const int size, const bool bottom = false)
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
	INLINE void drawXBM(g15canvas* canvas, unsigned char* data, const int width, const int height ,const int pos_x, const int pos_y)
	{
		int y = 0;
		int z = 0;
		unsigned char byte;
		int bytes_per_row = (int) ceil((double) width / 8);

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
