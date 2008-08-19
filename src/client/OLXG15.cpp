//
// C++ Implementation: OLXG15
//
// Description: OLX G15 interface for displaying things like weapon info on the LCD.
//
//
// Author:  Daniel Sjoholm <steelside@gmail.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifdef WITH_G15
#include <cmath>
#include <sys/time.h>
#include "StringUtils.h"
#include "Options.h"
#include "LieroX.h"
#include "CWorm.h"
#include "CClient.h"

#include "OLXG15.h"
#include "OLX_g15logo_ver4.xbm"

OLXG15_t *OLXG15 = NULL;
OLXG15_t::OLXG15_t()
{
	screenfd = -1;
	curScreen = -1;
	screenVer = 0;
	showingSplash = false;
	lastFrame = 0.0f;
	oldGameState = NET_DISCONNECTED;
}
OLXG15_t::~OLXG15_t()
{
	if (screenfd != -1)
		g15_close_screen(screenfd);
}
bool OLXG15_t::init()
{
	gettimeofday(&startTime,NULL);
	screenfd = new_g15_screen(G15_G15RBUF);
	if(screenfd < 0)
	{
		std::cout << "Unable to connect to G15daemon! No G15 Support will be availbile." << std::endl;
		return false;
	}

	g15r_initCanvas(&canvas);
	//sayHi();


	showSplashScreen();
	g15_send(screenfd,(char *)canvas.buffer,G15_BUFFER_LEN);
	/*
	for (int i = 0; i < 3; ++i)
	{
		g15r_clearScreen (&canvas, G15_COLOR_WHITE);
		testWeaponScreen(i);
		//drawBounds(i);
		g15_send(screenfd,(char *)canvas.buffer,G15_BUFFER_LEN);
	}
	*/

	/*
	unsigned int keystate = 0;
	nanosleep(&sleepTime,NULL);
	for (int i = 0; i <= 3;++i)
	{
		g15_recv(screenfd,(char*)&keystate,4);

		if(keystate & G15_KEY_L1)
		{
			std::cout << "L1 pressed.. bye!" << std::endl;
		}
		else if(keystate & G15_KEY_L2)
		{
			std::cout << "L2 pressed" << std::endl;
		}
		else if(keystate & G15_KEY_L3)
		{
			std::cout << "L3 pressed" << std::endl;
		}
		else if(keystate & G15_KEY_L4)
		{
			std::cout << "L4 pressed" << std::endl;
		}
		else if(keystate & G15_KEY_L5)
		{
			std::cout << "L5 pressed" << std::endl;
		}
	}
	*/

	/*
	g15r_drawLine (&canvas, 0, 0, 130, 0, G15_COLOR_BLACK);

	for (int x = 130; x <= 170;++x)
	{
		g15r_setPixel (&canvas, x, 0,G15_COLOR_BLACK);
		g15r_renderString(&canvas, (unsigned char*)itoa(x).c_str(),2,G15_TEXT_LARGE,80,0);
		g15_send(screenfd,(char *)canvas.buffer,G15_BUFFER_LEN);
		nanosleep(&sleepTime,NULL);
	}
	*/
	std::cout << "OLXG15 ready" << std::endl;
	return true;
}

void OLXG15_t::frame()
{
	lastFrame += tLX->fRealDeltaTime;
	if (lastFrame < G15FRAMETIME)
		return;
	lastFrame = 0.0f;

	// TODO: Sucks if we add more of these things.
	if (showingSplash)
	{
		// To show as close to our time-limit as possible, even if loading took longer
		timeval curTime;
		gettimeofday(&curTime,NULL);

		timeShown += tLX->fRealDeltaTime;
		if (timeShown >= G15SPLASHTIME || startTime.tv_sec + (int) G15SPLASHTIME < curTime.tv_sec)
		{
			// TODO: ATM it looks better to show the logo until a game starts.
			// Perhaps have a message here when in lobby and selecting weapons? (incase minimized?)
			//g15r_clearScreen (&canvas, G15_COLOR_WHITE);
			//showingSplash = false;
			//timeShown = 0.0f;
			// Temporary incase someone compiles it with G15
			/*
			std::string tmp = "This is all it can do so far :(";
			g15r_renderString (&canvas, (unsigned char*)tmp.c_str(), 0, G15_TEXT_MED, centerAlign(tmp,G15_TEXT_MED), yAlign(G15_HEIGHT/2,G15_TEXT_MED));
			g15_send(screenfd,(char *)canvas.buffer,G15_BUFFER_LEN);
			*/
			//return;
		}
	}


	switch (cClient->getStatus())
	{
		// TODO: Lobby in net play
		// Weapon selections in local
		case NET_CONNECTED:
			if (oldGameState == NET_CONNECTED)
				break;
			std::cout << "Status: " << itoa(cClient->getStatus()) << "/" << itoa(oldGameState) << std::endl;
			g15r_clearScreen (&canvas, G15_COLOR_WHITE);
			showingSplash = false;
			timeShown = 0.0f;
		case NET_PLAYING:
			if (oldGameState == NET_PLAYING)
				break;
			break;
	}


	// This is the only function that's going to get called from OLX regulary (except info-setting)
	// Handles everything

	// Areas of the screen:
	// Loadage:
	// 159 - getCharWidth(size)*4 to 159 = loadage. S:145 - 159 M:142 - 159 L:130 - 159
	// Charge indicators:
	// (loadage - getCharWidth(size)*2) to loadage. S:137 - 143 M:133 - 140 L:116 - 128
	// Weapon name:
	// 0 to Charge indicators. S:0 - 135 M:0 - 131 L:0 - 114
	for (int i=0; i < 5;++i)
	{
		updateWeapon(i);
		if (!Weapons[i].changed)
			continue; // It hasn't changed, no need to update
		renderWeapon(Weapons[i],G15_TEXT_MED,i);
		Weapons[i].changed = false;
	}
	g15_send(screenfd,(char *)canvas.buffer,G15_BUFFER_LEN);

	oldGameState = cClient->getStatus();

}
void OLXG15_t::updateWeapon(const int& slotID)
{
	//int			getCurrentWeapon(void)
	// TODO: Make it customizable to show worm 0/1?
	CWorm *worm = cClient->getWorm(0);
	if (!worm)
		return;
	wpnslot_t *wpn = worm->getWeapon(slotID);

	// TODO: can this get nulled after game (most likely?)
	// If so, perhaps make it so the weapon has been changed.
	// No weapon yet
	if (!wpn->Weapon)
		return;
	if (Weapons[slotID].name != wpn->Weapon->Name)
	{
		Weapons[slotID].name = wpn->Weapon->Name;
		Weapons[slotID].changed = true;
	}

	if (Weapons[slotID].charge != (int)(wpn->Charge * 100))
	{
		Weapons[slotID].charge = (int)(wpn->Charge * 100);
		Weapons[slotID].changed = true;
	}

	if (Weapons[slotID].reloading != wpn->Reloading)
	{
		std::cout << "Reloading" << std::endl;
		Weapons[slotID].reloading = wpn->Reloading;
		Weapons[slotID].changed = true;
	}

	if (Weapons[slotID].reloading)
	{
		Weapons[slotID].changed = true;
		if (Weapons[slotID].chargeIndicator == 3)
			Weapons[slotID].chargeIndicator = 0;

		++Weapons[slotID].chargeIndicator;
	}
	else
		Weapons[slotID].chargeIndicator = 0; // To clear for next reload

	return;

}
void OLXG15_t::drawBounds(const int& size)
{
	switch (size)
	{
		case G15_TEXT_SMALL:
			//g15r_drawLine(&canvas, 145, 5, 159, 5, G15_COLOR_BLACK);
			// 14 pixels (16) -2 cause of spacing in ends
			g15r_drawLine(&canvas, 136, 0, 136, 43, G15_COLOR_BLACK);
			g15r_drawLine(&canvas, 144, 0, 144, 43, G15_COLOR_BLACK);

			g15r_drawLine(&canvas, 0, 30, 134, 30, G15_COLOR_BLACK);
			break;
		case G15_TEXT_MED:
			//g15r_drawLine(&canvas, 142, 6, 159, 6, G15_COLOR_BLACK);
			// 17 pixels (20) -3 cause of spacing in ends + 1 pix from the 1er being small
			g15r_drawLine(&canvas, 132, 0, 132, 43, G15_COLOR_BLACK);
			g15r_drawLine(&canvas, 141, 0, 141, 43, G15_COLOR_BLACK);

			g15r_drawLine(&canvas, 0, 27, 130, 27, G15_COLOR_BLACK);
			break;
		case G15_TEXT_LARGE:
			//g15r_drawLine(&canvas, 130, 7, 159, 7, G15_COLOR_BLACK);
			// 29 pixels (32) -3 cause of spacing in ends + 1 pix from the 1er being small
			g15r_drawLine(&canvas, 115, 0, 115, 43, G15_COLOR_BLACK);
			g15r_drawLine(&canvas, 129, 0, 129, 43, G15_COLOR_BLACK);

			g15r_drawLine(&canvas, 0, 32, 113, 32, G15_COLOR_BLACK);
			break;
	}
	//g15r_drawLine(&canvas, 1, 41, 158, 41, G15_COLOR_BLACK);
	//g15r_drawLine(&canvas, 0, 42, 159, 42, G15_COLOR_BLACK);
	//g15r_drawLine(&canvas, 0, 40, 160, 40, G15_COLOR_BLACK);
}

void OLXG15_t::sayHi()
{
	// Graphics test
	// Draw "Hi there" in different sizes

	g15r_renderString (&canvas, (unsigned char*)"Hi there small", 0, G15_TEXT_SMALL, 0, 0);
	g15r_renderString (&canvas, (unsigned char*)"Hi there medium", 1, G15_TEXT_MED, 0, 0);
	g15r_renderString (&canvas, (unsigned char*)"Hi there large", 2, G15_TEXT_LARGE, 0, 0);

	// Positioning
	g15r_renderString (&canvas, (unsigned char*)"Lefty", 0, G15_TEXT_MED, 0, yAlign(43,G15_TEXT_MED,true));

	std::string tmp = "Centery";
	g15r_renderString (&canvas, (unsigned char*)tmp.c_str(), 0, G15_TEXT_LARGE, centerAlign(tmp,G15_TEXT_LARGE), yAlign(43,G15_TEXT_LARGE,true));

	tmp = "Righty";
	g15r_renderString (&canvas, (unsigned char*)tmp.c_str(), 0, G15_TEXT_SMALL, rightAlign(tmp,G15_TEXT_SMALL), yAlign(43,G15_TEXT_SMALL,true));


	// Drawing functions
	g15r_drawLine (&canvas, 0, 43, 160, 0, G15_COLOR_BLACK);
	g15r_drawLine (&canvas, 0, 34, 160, 34, G15_COLOR_BLACK);
	/*
	g15r_setPixel (&canvas, 15, 24,G15_COLOR_BLACK);
	g15r_setPixel (&canvas, 15, 26,G15_COLOR_BLACK);
	g15r_setPixel (&canvas, 15, 28,G15_COLOR_BLACK);
	g15r_setPixel (&canvas, 15, 30,G15_COLOR_BLACK);
	*/
	//g15r_renderString (&canvas, (unsigned char*)"abcdefghijklmnopqrstuvwxyz1234567890,._-", 3, G15_TEXT_SMALL, 0, 0);
}

void OLXG15_t::testWeaponScreen(const int& size)
{
	Weapons[0].name = "Super Shotgun";
	Weapons[0].charge = 100;

	Weapons[1].name = "Napalm";
	Weapons[1].charge = 0;
	Weapons[1].reloading = true;
	Weapons[1].chargeIndicator = 1;

	Weapons[2].name = "Handgun with a really long name";
	Weapons[2].charge = 45;

	Weapons[3].name = "Cannon";
	Weapons[3].charge = 33;

	Weapons[4].name = "Doomsday";
	Weapons[4].charge = 6;

	for (int i =0; i < 5;++i)
	{
		renderWeapon(Weapons[i],size,i);
	}
}
// Renders 1 weapon row
void OLXG15_t::renderWeapon(OLXG15_weapon_t& weapon, const int& size, const int& row)
{
	clearRow(row,size);
	// Name
	drawWpnName(weapon.name,row,size);
	//g15r_renderString (&canvas, (unsigned char*)weapon.name.c_str(), row, size, 0, 0);

	// % loaded
	std::string chargeMsg = itoa(weapon.charge) + "%";
	g15r_renderString (&canvas, (unsigned char*)chargeMsg.c_str(), row, size, rightAlign(chargeMsg,size), 0);



	// Reloading? If so, blink >>/>
	// Re-using chargeMsg
	if (weapon.reloading)
	{
		std::cout << "Rendering reload!" << std::endl;
		if (weapon.chargeIndicator == 1)
			chargeMsg = ">>";
		else
			chargeMsg = "> ";

		// (100% = 4 letters - keep out of normal reading zone) -4*getCharWidth moves 4 letters left.
		g15r_renderString (&canvas, (unsigned char*)chargeMsg.c_str(), row, size,
							rightAlign(chargeMsg,size)-( (4*getCharWidth(size))-size ), 0);
	}
	else
		clearReload(row,size);

	return;
}
void OLXG15_t::drawWpnName(const std::string& name, const int& row, const int& size)
{
	// Areas of the screen:
	// Weapon name:
	// 0 to Charge indicators. S:0 - 135 M:0 - 131 L:0 - 114
	int width = (name.length() * getCharWidth(size));// -2, row removes front spacing, we remove back. (IGNORE - WANT 3PX SPACING)

	if (width > wpnSpace(size))
	{

		//std::cout << "(Row "<< row << ") Length is " << (name.length()) << " we need max " << wpnSpace(size)/getCharWidth(size) << std::endl;

		// Remove enough for 3 dots = 3 characters.
		std::string cutoff = name.substr(0,wpnSpace(size)/getCharWidth(size)-3);
		cutoff += "...";
		g15r_renderString (&canvas, (unsigned char*)cutoff.c_str(), row, size, 0, 0);
		return;
	}
	g15r_renderString (&canvas, (unsigned char*)name.c_str(), row, size, 0, 0);
}
void OLXG15_t::clearRow(const int& row, const int& size)
{
	int y1 = row * getCharHeight(size);
	int y2 = y1 + getCharHeight(size)-1; // Magical number, else it clears too much.
	g15r_pixelBox(&canvas,0,y1,G15_LCD_WIDTH,y2,G15_COLOR_WHITE,1,G15_PIXEL_FILL);
}
void OLXG15_t::clearReload(const int& row, const int& size)
{
	// Areas of the screen:
	// Charge indicators:
	// (loadage - getCharWidth(size)*2) to loadage. S:137 - 143 M:133 - 140 L:116 - 128
	// void g15r_pixelBox (g15canvas *canvas, int x1, int y1, int x2, int y2,int color, int thick, int fill)
	int y1 = row*getCharHeight(size);
	int y2 = y1 + getCharHeight(size)-2;
	//std::cout << "y1 = " << y1 << " for row " << row << ". charHeight is " << getCharHeight(size) << std::endl;

	// -2 is to remove the usual padding issued by it, we don't need to clear that, it won't be written in.
	switch (size)
	{
		case G15_TEXT_MED:
			g15r_pixelBox(&canvas,133,y1,140,y2,G15_COLOR_WHITE,1,G15_PIXEL_FILL);
			return;
		case G15_TEXT_LARGE:
			g15r_pixelBox(&canvas,116,y1,128,y2,G15_COLOR_WHITE,1,G15_PIXEL_FILL);
			return;
		default: // + G15_TEXT_SMALL
			g15r_pixelBox(&canvas,137,y1,143,y2,G15_COLOR_WHITE,1,G15_PIXEL_FILL);
			return;
	}
}

void OLXG15_t::showSplashScreen()
{
	drawXBM(&canvas,OLX_g15logo_ver4_bits,OLX_g15logo_ver4_width,OLX_g15logo_ver4_height,0,0);
	showingSplash = true;
}
#endif //WITH_G15
