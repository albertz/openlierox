#include "gfx.hpp"
#include "reader.hpp"
#include "game.hpp"
#include "sfx.hpp"
#include "text.hpp"
#include "keys.hpp"
#include "filesystem.hpp"
#include <cstring>
#include <cassert>
#include <cctype>
#include <SDL/SDL.h>
#include <iostream>

/*
ds:0000 is 0x 1AE80
*/

Gfx gfx;

const int Gfx::fireConeOffset[2][7][2] =
{
	{{-3, 1}, {-4, 0}, {-4, -2}, {-4, -4}, {-3, -5}, {-2, -6}, {0, -6}},
	{{3, 1}, {4, 0}, {4, -2}, {4, -4}, {3, -5}, {2, -6}, {0, -6}},
};

void Palette::activate()
{
	SDL_Color realPal[256];
	
	for(int i = 0; i < 256; ++i)
	{
		realPal[i].r = entries[i].r << 2;
		realPal[i].g = entries[i].g << 2;
		realPal[i].b = entries[i].b << 2;
	}
	
	SDL_SetColors(gfx.back, realPal, 0, 256);
}

int fadeValue(int v, int amount)
{
	assert(v < 64);
	v = (v * amount) >> 5;
	if(v < 0) v = 0;
	return v;
}

int lightUpValue(int v, int amount)
{
	v = (v * (32 - amount) + amount*63) >> 5;
	if(v > 63) v = 63;
	return v;
}

void Palette::fade(int amount)
{
	for(int i = 0; i < 256; ++i)
	{
		entries[i].r = fadeValue(entries[i].r, amount);
		entries[i].g = fadeValue(entries[i].g, amount);
		entries[i].b = fadeValue(entries[i].b, amount);
	}
}

void Palette::lightUp(int amount)
{
	for(int i = 0; i < 256; ++i)
	{
		entries[i].r = lightUpValue(entries[i].r, amount);
		entries[i].g = lightUpValue(entries[i].g, amount);
		entries[i].b = lightUpValue(entries[i].b, amount);
	}
}

void Palette::rotate(int from, int to)
{
	SDL_Color tocol = entries[to];
	for(int i = to; i > from; --i)
	{
		entries[i] = entries[i - 1];
	}
	entries[from] = tocol;
}

void Palette::clear()
{
	std::memset(entries, 0, sizeof(entries));
}

void Palette::read(FILE* f)
{
	for(int i = 0; i < 256; ++i)
	{
		unsigned char rgb[3];
		fread(rgb, 1, 3, f);
		
		entries[i].r = rgb[0] & 63;
		entries[i].g = rgb[1] & 63;
		entries[i].b = rgb[2] & 63;
	}
}


void SpriteSet::read(FILE* f, int width, int height, int count)
{
	assert(width == height); // We only support rectangular sprites right now
	
	this->width = width;
	this->height = height;
	this->spriteSize = width * height;
	this->count = count;
	
	int amount = spriteSize * count;
	data.resize(amount);
	
	std::vector<PalIdx> temp(amount);
	
	fread(&temp[0], 1, amount, f);
	
	PalIdx* dest = &data[0];
	PalIdx* src = &temp[0];
	
	for(int i = 0; i < count; i++)
	{
		for(int x = 0; x < width; ++x)
		{
			for(int y = 0; y < height; ++y)
			{
				dest[x + y*width] = src[y];
			}
			
			src += height;
		}
		
		dest += spriteSize;
	}
}

void SpriteSet::allocate(int width, int height, int count)
{
	this->width = width;
	this->height = height;
	this->spriteSize = width * height;
	this->count = count;
	
	int amount = spriteSize * count;
	data.resize(amount);
}

Gfx::Gfx()
: firstMenuItem(1)
, settingsMenuValues(true)
, playerMenuValues(true)
, screen(0)
, back(0)
, frozenScreen(320 * 200)
, screenFlash(0)
, running(true)
, fullscreen(false)
, doubleRes(false)
, menuCyclic(0)
{
	clearKeys();
}

void Gfx::init()
{
	setVideoMode();
	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
	SDL_EnableUNICODE(1);
	SDL_WM_SetCaption("Liero", 0);
	SDL_ShowCursor(SDL_DISABLE);
	lastFrame = SDL_GetTicks();
}

void Gfx::setVideoMode()
{
	int flags = SDL_SWSURFACE;
	if(fullscreen)
		flags |= SDL_FULLSCREEN;
		
	if(screen != back)
	{
		SDL_FreeSurface(screen);
		screen = 0;
	}

	if(!SDL_VideoModeOK(320, 200, 8, flags)
	|| gfx.doubleRes)
	{
		screen = SDL_CreateRGBSurface(SDL_SWSURFACE, 320, 200, 8, 0, 0, 0, 0);
		back = SDL_SetVideoMode(640, 480, 8, flags);
	}
	else
	{
		back = screen = SDL_SetVideoMode(320, 200, 8, flags);
	}
	screenPixels = static_cast<unsigned char*>(screen->pixels);
	screenPitch = screen->pitch;
}

void Gfx::loadPalette()
{
	FILE* exe = openLieroEXE();
	
	std::fseek(exe, 132774, SEEK_SET);
	
	exepal.read(exe);
	origpal = exepal;
	pal = origpal;
	
	std::fseek(exe, 0x1AF0C, SEEK_SET);
	for(int i = 0; i < 4; ++i)
	{
		colourAnim[i].from = readUint8(exe);
		colourAnim[i].to = readUint8(exe);
	}
}

void Gfx::loadMenus()
{
	FILE* exe = openLieroEXE();
	
	fseek(exe, 0x1B08A, SEEK_SET);
	mainMenu.readItems(exe, 14, 4, true);
	
	fseek(exe, 0x1B0C2, SEEK_SET);
	settingsMenu.readItems(exe, 21, 15, false, 48, 7);
	
	settingsMenuValues.items.assign(12, MenuItem(48, 7, ""));
	
	fseek(exe, 0x1B210, SEEK_SET);
	playerMenu.readItems(exe, 13, 13, false, 48, 7);
	
	playerMenuValues.items.assign(13, MenuItem(48, 7, ""));
}

void Gfx::loadGfx()
{
	FILE* exe = openLieroEXE();
	
	fseek(exe, 0x1C1DE, SEEK_SET);
	bonusFrames[0] = readUint8(exe);
	bonusFrames[1] = readUint8(exe);
	
	FILE* gfx = openLieroCHR();
	
	fseek(gfx, 10, SEEK_SET); // Skip some header
	
	largeSprites.read(gfx, 16, 16, 110);
	fseek(gfx, 4, SEEK_CUR); // Extra stuff
	
	smallSprites.read(gfx, 7, 7, 130);
	fseek(gfx, 4, SEEK_CUR); // Extra stuff
	
	textSprites.read(gfx, 4, 4, 26);
	
	for(int y = 0; y < 16; ++y)
	for(int x = 0; x < 16; ++x)
	{
		int idx = y * 16 + x;
		largeSprites.spritePtr(73)[idx] = game.rand(4) + 160;
		largeSprites.spritePtr(74)[idx] = game.rand(4) + 160;
		
		largeSprites.spritePtr(87)[idx] = game.rand(4) + 12;
		largeSprites.spritePtr(88)[idx] = game.rand(4) + 12;
		
		largeSprites.spritePtr(82)[idx] = game.rand(4) + 94;
		largeSprites.spritePtr(83)[idx] = game.rand(4) + 94;
	}
	
	wormSprites.allocate(16, 16, 2 * 2 * 21);
	
	for(int i = 0; i < 21; ++i)
	{
		for(int y = 0; y < 16; ++y)
		for(int x = 0; x < 16; ++x)
		{
			PalIdx pix = (largeSprites.spritePtr(16 + i) + y*16)[x];
			
			(wormSprite(i, 1, 0) + y*16)[x] = pix;
			if(x == 15)
				(wormSprite(i, 0, 0) + y*16)[15] = 0;
			else
				(wormSprite(i, 0, 0) + y*16)[14 - x] = pix;
			
			if(pix >= 30 && pix <= 34)
				pix += 9; // Change worm colour
				
			(wormSprite(i, 1, 1) + y*16)[x] = pix;
			
			if(x == 15)
				(wormSprite(i, 0, 1) + y*16)[15] = 0; // A bit haxy, but works
			else
				(wormSprite(i, 0, 1) + y*16)[14 - x] = pix;
		}
	}
	
	fireConeSprites.allocate(16, 16, 2 * 7);
	
	for(int i = 0; i < 7; ++i)
	{
		for(int y = 0; y < 16; ++y)
		for(int x = 0; x < 16; ++x)
		{
			PalIdx pix = (largeSprites.spritePtr(9 + i) + y*16)[x];
			
			(fireConeSprite(i, 1) + y*16)[x] = pix;
			
			if(x == 15)
				(fireConeSprite(i, 0) + y*16)[15] = 0;
			else
				(fireConeSprite(i, 0) + y*16)[14 - x] = pix;
			
		}
	}
}


void Gfx::updateSettingsMenu()
{
	settingsMenuValues.items[0].string = game.texts.gameModes[game.settings.gameMode];
	
	switch(game.settings.gameMode)
	{
		case Settings::GMKillEmAll:
			settingsMenuValues.items[1].string = toString(game.settings.lives);
			settingsMenu.items[1].string = game.texts.gameModeSpec[0];
		break;
		
		case Settings::GMGameOfTag:		
			settingsMenuValues.items[1].string = timeToString(game.settings.timeToLose);
			settingsMenu.items[1].string = game.texts.gameModeSpec[1];
		break;
		
		case Settings::GMCtF:
		case Settings::GMSimpleCtF:
			settingsMenuValues.items[1].string = toString(game.settings.flagsToWin);
			settingsMenu.items[1].string = game.texts.gameModeSpec[2];
		break;
	}
	
	settingsMenuValues.items[2].string = toString(game.settings.loadingTime) + '%';
	settingsMenuValues.items[3].string = toString(game.settings.maxBonuses);
	
	settingsMenuValues.items[4].string = game.texts.onoff[game.settings.namesOnBonuses];
	settingsMenuValues.items[5].string = game.texts.onoff[game.settings.map];
	
	settingsMenuValues.items[6].string = toString(game.settings.blood) + '%';
	
	std::string levelPath = joinPath(lieroEXERoot, game.settings.levelFile + ".lev");
	if(!game.settings.randomLevel && fileExists(levelPath))
	{
		settingsMenuValues.items[7].string = '"' + game.settings.levelFile + '"';
		settingsMenuValues.items[8].string = game.texts.reloadLevel;
	}
	else
	{
		settingsMenuValues.items[7].string = game.texts.random2;
		settingsMenuValues.items[8].string = game.texts.regenLevel;
	}
	
	
/* TODO
 char buffer[260];
 sprintf(buffer, "%s.lev", settings.levelfile);
 if(!settings.randomlevel && FileExists(buffer))
 {
  sprintf(mnuSettingsItems[7].szString, "\"%s\"", settings.levelfile);
  mnuSettingsMenuItems[8].szString = (char*)txt_reloadlevel;
 } else
 {
  strcpy(mnuSettingsItems[7].szString, "Random");
  mnuSettingsMenuItems[8].szString = (char*)txt_regenlevel;
 } // D663
*/
	settingsMenuValues.items[8].string = game.texts.onoff[game.settings.regenerateLevel];
	settingsMenuValues.items[9].string = game.texts.onoff[game.settings.shadow];
	settingsMenuValues.items[10].string = game.texts.onoff[game.settings.screenSync];
	settingsMenuValues.items[11].string = game.texts.onoff[game.settings.loadChange];
	
}

void Gfx::updatePlayerMenu(int player)
{
	WormSettings const& ws = game.worms[player].settings;
	
	playerMenuValues.items[0].string = game.worms[player].settings.name;
	playerMenuValues.items[1].string = toString(game.worms[player].settings.health) + '%';
	playerMenuValues.items[2].string = toString(game.worms[player].settings.rgb[0]);
	playerMenuValues.items[3].string = toString(game.worms[player].settings.rgb[1]);
	playerMenuValues.items[4].string = toString(game.worms[player].settings.rgb[2]);
	
	for(int i = 0; i < 7; ++i)
	{
		playerMenuValues.items[i + 5].string = game.texts.keyNames[ws.controls[i]];
	}

	playerMenuValues.items[12].string = game.texts.controllers[game.worms[player].settings.controller];
}


void Gfx::setWormColours()
{
	int const b[2] = {0x58, 0x78}; // TODO: Read from EXE?

	for(unsigned i = 0; i < game.worms.size() && i < 2; ++i) // std::min() screwed up somewhere
	{
		int idx = game.worms[i].settings.colour;
		
		origpal.setWormColours(idx, game.worms[i].settings.rgb);
		
		for(int j = 0; j < 6; ++j)
		{
			origpal.entries[b[i] + j] = origpal.entries[idx + (j % 3) - 1];
		}
		
		for(int j = 0; j < 3; ++j)
		{
			origpal.entries[129 + i * 4 + j] = origpal.entries[idx + j];
		}
	}
}

void processEvent(SDL_Event& ev)
{
	switch(ev.type)
	{
		case SDL_KEYDOWN:
		{
		
			SDLKey s = ev.key.keysym.sym;
			/*
			gfx.keys[s] = true;
			*/
			Uint32 dosScan = SDLToDOSKey(ev.key.keysym);
			if(dosScan)
				gfx.dosKeys[dosScan] = true;
				
#if 0
			std::cout << "v " << s << ", " << std::hex << ev.key.keysym.mod << ", " << std::dec << int(ev.key.keysym.scancode) << std::endl;
#endif
			
			if(((ev.key.keysym.mod & KMOD_ALT) && s == SDLK_RETURN)
			|| s == SDLK_F5)
			{
				gfx.fullscreen = !gfx.fullscreen;
				gfx.setVideoMode();
			}
			else if(s == SDLK_F6)
			{
				gfx.doubleRes = !gfx.doubleRes;
				gfx.setVideoMode();
			}
		}
		break;
		
		case SDL_KEYUP:
		{/*
			gfx.keys[ev.key.keysym.sym] = false;
			*/
			SDLKey s = ev.key.keysym.sym;
			
			Uint32 dosScan = SDLToDOSKey(s);
			if(dosScan)
				gfx.dosKeys[dosScan] = false;
				
#if 0
			std::cout << "^ " << s << ", " << std::hex << ev.key.keysym.mod << ", " << std::dec << int(ev.key.keysym.scancode) << std::endl;
#endif
		}
		break;
		
		case SDL_QUIT:
		{
			gfx.running = false;
		}
		break;
	}
}

void Gfx::process()
{
	SDL_Event ev;
	while(SDL_PollEvent(&ev))
	{
		processEvent(ev);
	}
	
	processReader();
}

SDL_keysym Gfx::waitForKey()
{
	SDL_Event ev;
	while(SDL_WaitEvent(&ev))
	{
		processEvent(ev);
		if(ev.type == SDL_KEYDOWN)
		{
			return ev.key.keysym;
		}
	}
	
	return SDL_keysym(); // Dummy
}

void Gfx::clearKeys()
{
	//std::memset(keys, 0, sizeof(keys));
	std::memset(dosKeys, 0, sizeof(dosKeys));
}

void Gfx::flip()
{
	pal.activate();
	if(screen != back)
	{
		PalIdx* dest = reinterpret_cast<PalIdx*>(back->pixels);
		PalIdx* src = screenPixels;
		
		std::size_t destPitch = back->pitch;
		std::size_t srcPitch = screenPitch;
		
		for(int y = 0; y < 200; ++y)
		for(int x = 0; x < 320; ++x)
		{
			int dx = (x << 1);
			int dy = (y << 1) + 40;
			PalIdx pix = src[y*srcPitch + x];
			dest[dy*destPitch + dx] = pix;
			dest[dy*destPitch + (dx+1)] = pix;
			dest[(dy+1)*destPitch + dx] = pix;
			dest[(dy+1)*destPitch + (dx+1)] = pix;
		}
	}
	
	SDL_Flip(back);
	
	if(game.settings.screenSync)
	{
		SDL_Delay(0);
		/*
		static unsigned int const delay = 14u;
		
		while((SDL_GetTicks() - lastFrame) < delay)
		{
			SDL_Delay(0);
		}
		
		while((SDL_GetTicks() - lastFrame) >= delay)
			lastFrame += delay;
		*/
	}
	else
		SDL_Delay(0);
}


void Gfx::clear()
{
	SDL_FillRect(screen, 0, 0);
}

void fillRect(int x, int y, int w, int h, int colour)
{
	SDL_Rect rect;
	rect.x = x;
	rect.y = y;
	rect.w = w;
	rect.h = h;
	SDL_FillRect(gfx.screen, &rect, colour);
}

void playChangeSound(int change)
{
	if(change > 0)
	{
		sfx.play(25, -1);
	}
	else
	{
		sfx.play(26, -1);
	}
}

void resetLeftRight()
{
/*
	gfx.keys[SDLK_LEFT] = false;
	gfx.keys[SDLK_RIGHT] = false;*/
	
	gfx.releaseSDLKey(SDLK_LEFT);
	gfx.releaseSDLKey(SDLK_RIGHT);
}

template<typename T>
void changeVariable(T& var, T change, T min, T max, T scale)
{
	if(change < 0 && var > min)
	{
		var += change * scale;
	}
	if(change > 0 && var < max)
	{
		var += change * scale;
	}
}

void Gfx::settingEnter(int item)
{
	int selectY = item * 8 + 20;
	switch(item)
	{
		case 0: //GAME MODE
			game.settings.gameMode = (game.settings.gameMode + 1) % 4;
		break;
		
		case 1:  //LIVES / TIME TO LOSE / FLAGS TO WIN //D772
			switch(game.settings.gameMode)
			{
				case Settings::GMKillEmAll:
					inputInteger(game.settings.lives, 0, 999, 3, 280, selectY);
				break;
				
				case Settings::GMCtF: // D7AF
				case Settings::GMSimpleCtF:
					inputInteger(game.settings.flagsToWin, 0, 999, 3, 280, selectY);
				break;
			} // D7E7
		break;
			
		case 2: // LOADING TIMES // D7EA
			inputInteger(game.settings.loadingTime, 0, 9999, 4, 280, selectY);
		break;
		
		case 3: // MAX BONUSES // D82A
			inputInteger(game.settings.maxBonuses, 0, 99, 2, 280, selectY);
   		break;
   		
		case 4: // NAMES ON BONUSES // D86B
			game.settings.namesOnBonuses ^= 1; //Toggles first bit
		break;
		
		case 5: // MAP //D87F
			game.settings.map ^= 1;
		break;
		
		case 7: // LEVEL //D893
		{
			std::vector<std::string> list;
			
			list.push_back(game.texts.random);
			
			DirectoryIterator di(joinPath(lieroEXERoot, ".")); // TODO: Fix lieroEXERoot to be "." instead of ""
			
			for(; di; ++di)
			{
				std::string str = *di;
				
				if(ciCompare(getExtension(str), "LEV"))
					list.push_back(getBasename(str));
			}
			
			std::size_t curSel = 0;
			std::size_t topItem = 0;
			
			if(!game.settings.levelFile.empty())
			{
				for(std::size_t i = 1; i < list.size(); ++i)
				{
					if(ciCompare(list[i], game.settings.levelFile))
					{
						curSel = i;
						break;
					}
				}
			}
			
			if(list.size() > curSel + 14) // We subtract 14 instead of 13, since list.size() seems to be 1 too much compared to 'listitems' in the original
				topItem = curSel;
			else if(list.size() >= 14)
				topItem = list.size() - 14;
			else
				topItem = 0;
				
			std::vector<PalIdx> tempScreen(320 * 200);
			
			// Reset the right part of the screen
			blitImageNoKeyColour(screen, &frozenScreen[160], 160, 0, 160, 200, 320);
			
			drawRoundedBox(178, 20, 0, 7, gfx.font.getWidth(game.texts.selLevel));
			gfx.font.drawText(game.texts.selLevel, 180, 21, 50);
			
			std::memcpy(&tempScreen[0], gfx.screenPixels, tempScreen.size());
			
			do
			{
				std::memcpy(gfx.screenPixels, &tempScreen[0], tempScreen.size());
								
				std::size_t max = topItem + 14;
				
				for(std::size_t i = topItem; i < max; ++i)
				{
					if(i < list.size())
					{
						int y = int(i - topItem) * 8 + 29;
						std::string item = list[i];
						toUpperCase(item); // TODO: Maybe optimize this
						
						if(i == curSel)
						{
							drawRoundedBox(178, int(curSel - topItem) * 8 + 28, 0, 7, gfx.font.getWidth(item));
						}
						
						gfx.font.drawText(item, 181, y + 1, 0);
						gfx.font.drawText(item, 180, y, (i != curSel) ? 7 : 168);
					}
				}
				
				if(list.size() > 14)
				{
					gfx.font.drawChar(22, 172, 30, 0);
					gfx.font.drawChar(22, 171, 29, 50);
					gfx.font.drawChar(23, 172, 134, 0);
					gfx.font.drawChar(23, 171, 133, 50);
					
					int height = int(14*96 / list.size());
					int y = int(topItem * 96 / list.size());
					
					fillRect(171, y + 37, 7, height, 0);
					fillRect(170, y + 36, 7, height, 7);
				}
				
				if(testSDLKeyOnce(SDLK_UP))
				{
					sfx.play(26, -1);
					
					if(curSel > 0)
						--curSel;
					if(topItem > curSel)
						topItem = curSel;
				}
				
				if(testSDLKeyOnce(SDLK_DOWN))
				{
					sfx.play(25, -1);
					
					if(curSel < list.size() - 1)
						++curSel;
					if(topItem + 13 < curSel)
						topItem = curSel - 13;
				}
				
				if(testSDLKeyOnce(SDLK_RETURN)
				|| testSDLKeyOnce(SDLK_KP_ENTER))
				{
					sfx.play(27, -1);
					
					if(curSel == 0)
					{
						game.settings.randomLevel = true;
						game.settings.levelFile.clear();
					}
					else
					{
						game.settings.randomLevel = false;
						game.settings.levelFile = list[curSel];
					}
					
					break;
				}
				
				origpal.rotate(168, 174);
				pal = origpal;
				
				flip();
				process();
			}
			while(!testSDLKeyOnce(SDLK_ESCAPE));
		}
		break;
		
		case 8: // REGENERATE LEVEL // DD92
			game.settings.regenerateLevel ^= 1;
		break;
		case 9: // SHADOWS // DDA6
			game.settings.shadow ^= 1;
		break;
		case 10: // SCREEN SYNC // DDBA
			game.settings.screenSync ^= 1;
		break;
		case 11: // LOAD+CHANGE // DDCE
			game.settings.loadChange ^= 1;
		break;
		
		case 12: // PLAYER 1 OPTIONS // DDE2
			playerSettings(0);
		break;
		case 13: // PLAYER 2 OPTIONS // DDF2
			playerSettings(1);
		break;
		case 14: // WEAPON OPTIONS // DE02
		{
			std::size_t curSel = 1;
			std::size_t topItem = 1;
			std::size_t listItems = 40;
			
			std::vector<PalIdx> tempScreen(320 * 200);
			
			// Reset the right part of the screen
			blitImageNoKeyColour(screen, &frozenScreen[160], 160, 0, 160, 200, 320);
			
			drawRoundedBox(178, 20, 0, 7, gfx.font.getWidth(game.texts.weapon));
			drawRoundedBox(248, 20, 0, 7, gfx.font.getWidth(game.texts.availability));
			
			gfx.font.drawText(game.texts.weapon, 180, 21, 50);
			gfx.font.drawText(game.texts.availability, 250, 21, 50);
			
			std::memcpy(&tempScreen[0], gfx.screenPixels, tempScreen.size());
			
			while(true)
			{
				std::memcpy(gfx.screenPixels, &tempScreen[0], tempScreen.size());
				
				std::size_t max = topItem + 14;
				for(std::size_t i = topItem; i < max; ++i)
				{
					if(i <= listItems)
					{
						int index = game.weapOrder[i];
						int state = game.settings.weapTable[index];
						std::string const& stateStr = game.texts.weapStates[state];
						std::string const& weapName = game.weapons[index].name;
						int nameWidth = gfx.font.getWidth(weapName);
						int stateWidth = gfx.font.getWidth(stateStr);
						
						int y = int(i - topItem) * 8 + 29;
						
						if(i == curSel)
						{
							drawRoundedBox(178, y - 1, 0, 7, nameWidth);
							drawRoundedBox(268 - stateWidth/2, y - 1, 0, 7, stateWidth);
						}
						
						gfx.font.drawText(weapName, 181, y + 1, 0); // TODO: A single function drawing text with shadow
						gfx.font.drawText(weapName, 180, y, (i != curSel) ? 7 : 168);
						gfx.font.drawText(stateStr, 271 - stateWidth/2, y + 1, 0);
						gfx.font.drawText(stateStr, 270 - stateWidth/2, y, (i != curSel) ? 7 : 168);
						
					}
				}
				
				
				gfx.font.drawChar(22, 172, 30, 0);
				gfx.font.drawChar(22, 171, 29, 50);
				gfx.font.drawChar(23, 172, 134, 0);
				gfx.font.drawChar(23, 171, 133, 50);
				
				int height = 33;
				int y = int((topItem * 96 + 96) / 40);
				
				fillRect(171, y + 34, 7, height, 0);
				fillRect(170, y + 33, 7, height, 7);
				
				if(testSDLKeyOnce(SDLK_UP))
				{
					sfx.play(26, -1);
					
					if(curSel > 1)
						--curSel;
					if(topItem > curSel)
						topItem = curSel;
				}
				
				if(testSDLKeyOnce(SDLK_DOWN))
				{
					sfx.play(25, -1);
					
					if(curSel < 40)
						++curSel;
					if(topItem + 13 < curSel)
						++topItem;
				}
				
				if(testSDLKeyOnce(SDLK_LEFT))
				{
					sfx.play(25, -1);
					
					unsigned char& v = game.settings.weapTable[game.weapOrder[curSel]];
					
					v = (v - 1 + 3) % 3;
				}
				
				if(testSDLKeyOnce(SDLK_RIGHT))
				{
					sfx.play(26, -1);
					
					unsigned char& v = game.settings.weapTable[game.weapOrder[curSel]];
					
					v = (v + 1 + 3) % 3;
				}
								
				origpal.rotate(168, 174);
				pal = origpal;
				
				flip();
				process();
				
				if(testSDLKeyOnce(SDLK_ESCAPE))
				{
					int count = 0;
					
					for(int i = 0; i < 40; ++i)
					{
						if(game.settings.weapTable[i] == 0)
							++count;
					}
						
					if(count > 0)
						break; // Enough weapons available
						
					drawRoundedBox(178, 58, 0, 17, 98);
					gfx.font.drawText(game.texts.noWeaps, 180, 60, 6);
					
					flip();
					process();
					
					gfx.waitForKey();
				}
			}
			
			sfx.play(27, -1);
		}
		break;
	}
}

bool Gfx::inputString(std::string& dest, std::size_t maxLen, int x, int y, int (*filter)(int), std::string const& prefix, bool centered)
{
	std::string buffer = dest;
	
	while(true)
	{
		std::string str = prefix + buffer + '_';
		
		int width = font.getWidth(str);
		
		int adjust = centered ? width/2 : 0;
		
		int clrX = x - 10 - adjust;
		
		int offset = clrX + y*320; // TODO: Unhardcode 320
		
		blitImageNoKeyColour(screen, &frozenScreen[offset], clrX, y, clrX + 10 + width, 8, 320);
		
		drawRoundedBox(x - 2 - adjust, y, 0, 7, width);
		
		font.drawText(str, x - adjust, y + 1, 50);
		flip();
		SDL_keysym key(waitForKey());
		
		switch(key.sym)
		{
		case SDLK_BACKSPACE:
			if(!buffer.empty())
			{
				buffer.erase(buffer.size() - 1);
			}
		break;
		
		case SDLK_RETURN:
		case SDLK_KP_ENTER:
			dest = buffer;
			sfx.play(27, -1);
			clearKeys();
			return true;
			
		case SDLK_ESCAPE:
			clearKeys();
			return false;
			
		default:
			int k = unicodeToDOS(key.unicode);
			if(k
			&& buffer.size() < maxLen
			&& (
			    !filter
			 || (k = filter(k))))
			{
				buffer += char(k);
			}
		}
	}
}

int filterDigits(int k)
{
	return std::isdigit(k) ? k : 0;
}

void Gfx::inputInteger(int& dest, int min, int max, std::size_t maxLen, int x, int y)
{
	std::string str(toString(dest));
	
	if(inputString(str, maxLen, x, y, filterDigits)
	&& !str.empty())
	{
		dest = std::atoi(str.c_str());
		if(dest < min)
			dest = min;
		else if(dest > max)
			dest = max;
	}
}


void Gfx::playerSettings(int player)
{
	int curSel = 0;
	int menuCyclic = 0;
	
	WormSettings& ws = game.worms[player].settings;
	
	updatePlayerMenu(player);
	
	do
	{
		int selectY = (curSel << 3) + 20;
		
		drawBasicMenu(0);

		playerMenu.draw(178, 20, false, curSel);
		playerMenuValues.draw(273, 20, false, curSel);
		
		for(int o = 0; o < 12; o++)
		{
			int ypos = (o<<3);

			if(o >= 2 && o <= 4) //Color settings
			{
				int rgbcol = o - 2;

				if(o == curSel)
				{
					drawRoundedBox(202, ypos + 20, 168, 7, game.worms[player].settings.rgb[rgbcol] - 1);
				}
				else // CE98
				{
					drawRoundedBox(202, ypos + 20, 0, 7, game.worms[player].settings.rgb[rgbcol] - 1);
				}
				
				fillRect(203, ypos + 21, game.worms[player].settings.rgb[rgbcol], 5, game.worms[player].settings.colour);
			} // CED9
			
			
		} // CF22
		
		drawRoundedBox(163, 19, 0, 12, 11);

		blitImage(gfx.screen, wormSprite(2, 1, player), 163, 20, 16, 16);
		

		// l_CF9E:

		if(testSDLKeyOnce(SDLK_UP))
		{
			sfx.play(26, -1);
			--curSel;
			if(curSel < 0)
				curSel = 12;
		} // CFD0

		if(testSDLKeyOnce(SDLK_DOWN))
		{
			sfx.play(25, -1);
		
			++curSel;
			if(curSel > 12)
				curSel = 0;
		} // D002
		
		if(menuCyclic == 0)
		{
		
			switch(curSel)
			{
			case 1:
				
				if(testSDLKey(SDLK_LEFT))
				{
					if(ws.health > 1)
						--ws.health;
					updatePlayerMenu(player);
				}
				if(testSDLKey(SDLK_RIGHT))
				{
					if(ws.health < 10000)
						++ws.health;
					updatePlayerMenu(player);
				}
			
			break;
			
			case 2:
			case 3:
			case 4:
				
				if(testSDLKey(SDLK_LEFT))
				{
					--ws.rgb[curSel - 2];
					if(ws.rgb[curSel - 2] < 0)
						ws.rgb[curSel - 2] = 0;
					updatePlayerMenu(player);
				}
				if(testSDLKey(SDLK_RIGHT))
				{
					++ws.rgb[curSel - 2];
					if(ws.rgb[curSel - 2] > 63)
						ws.rgb[curSel - 2] = 63;
					updatePlayerMenu(player);
				}
				
			break;
			
			case 12:
				
				if(testSDLKeyOnce(SDLK_LEFT))
				{
					sfx.play(25, -1); // Should it be 26?
					ws.controller = (ws.controller - 1 + 2) % 2;
					updatePlayerMenu(player);
				}
				if(testSDLKeyOnce(SDLK_RIGHT))
				{
					sfx.play(26, -1); // Should it be 25?
					ws.controller = (ws.controller + 1) % 2;
					updatePlayerMenu(player);
				}
			break;
			
			}
		}
		
		if(testSDLKeyOnce(SDLK_RETURN)
		|| testSDLKeyOnce(SDLK_KP_ENTER))
		{
			sfx.play(27, -1);
			
			switch(curSel)
			{
			case 0:
				ws.randomName = false;
				inputString(ws.name, 20, 275, 20);
				
				if(ws.name.empty())
				{
					Settings::generateName(ws);
				}
				//updatePlayerMenu(player);
			break;
			
			case 1:
				inputInteger(ws.health, 0, 10000, 5, 275, 28);
			break;
			
			case 2:
			case 3:
			case 4:
				inputInteger(ws.rgb[curSel-2], 0, 63, 2, 275, selectY);
			break;
			
			case 5: // D2AB
			case 6:
			case 7:
			case 8:
			case 9:
			case 10:
			case 11:
			{
				SDL_keysym key(waitForKey());
				
				if(key.sym != SDLK_ESCAPE)
				{
					Uint32 k = SDLToDOSKey(key.sym);
					if(k)
						ws.controls[curSel - 5] = k;
				/*
					int lieroKey = SDLToLieroKeys[key.sym];
					
					if(lieroKey)
					{
						ws.controls[curSel - 5] = lieroKey;
					}
				*/
				}
				
				clearKeys();
			}
			break;
			
			}
			
			updatePlayerMenu(player);
		}
		
		setWormColours();
		origpal.rotate(168, 174);
		pal = origpal;
 
		flip();
		process();
		
		menuCyclic = (menuCyclic + 1) & 3;
	}
	while(!testSDLKeyOnce(SDLK_ESCAPE));
	
}

void Gfx::settingLeftRight(int change, int item)
{
	switch(item)
	{
		case 4: //NAMES ON BONUSES
		case 5: //MAP
		case 8: //REGENERATE LEVEL
		case 9: //SHADOWS
		case 10: //SCREEN SYNC
		case 11: //LOAD+CHANGE
			playChangeSound(change);

			resetLeftRight();

			settingEnter(item);
		break;
			
		case 0: //GAME MODE // E4CF
			playChangeSound(change);

			resetLeftRight();

			game.settings.gameMode = (game.settings.gameMode + change + 4) % 4;
			
			
		break;
		
		case 1: //LIVES / TIME TO LOSE / FLAGS TO WIN //E530
			if(menuCyclic == 0)
			{
				switch(game.settings.gameMode)
				{
				case Settings::GMKillEmAll:
					changeVariable(game.settings.lives, change, 1, 999, 1);
					
				break;
				
				case Settings::GMGameOfTag: // E579
					changeVariable(game.settings.timeToLose, change, 60, 3600, 10);
				break;
				
				case Settings::GMCtF:
				case Settings::GMSimpleCtF: //E5B0
					changeVariable(game.settings.flagsToWin, change, 1, 999, 1);
				break;
				} // E5E3
			}
			break;
		case 2: //LOADING TIMES // E5E6
			if(menuCyclic == 0)
			{
				changeVariable(game.settings.loadingTime, change, 0, 9999, 1);
			}
			break;
		case 3: //MAX BONUSES // E622
			if(menuCyclic == 0)
			{
				changeVariable(game.settings.maxBonuses, change, 0, 99, 1);
			}
			break;
		case 6: //AMOUNT OF BLOOD // E65B
			if(menuCyclic == 0)
			{
				changeVariable(game.settings.blood, change, 0, 500, 25);
				
			}
			break;
	} // E68F
}

void Gfx::mainLoop()
{
	game.generateLevel();
	game.resetWorms();
	
	while(true)
	{
		int selection = menuLoop();
		
		if(selection == 1 || selection == 0)
		{
			game.startGame(selection == 1);
		}
		else if(selection == 3) // QUIT TO OS
		{
			break;
		}
	}
}

void Gfx::drawBasicMenu(int curSel)
{
	std::memcpy(screen->pixels, &frozenScreen[0], frozenScreen.size());
	
	font.drawText(game.texts.saveoptions, 36, 54+20, 0);
	font.drawText(game.texts.loadoptions, 36, 61+20, 0);
	
	font.drawText(game.texts.saveoptions, 36, 53+20, 10);
	font.drawText(game.texts.loadoptions, 36, 60+20, 10);
	
	if(game.settingsFile.empty())
	{
		gfx.font.drawText(game.texts.curOptNoFile, 36, 46+20, 0);
		gfx.font.drawText(game.texts.curOptNoFile, 35, 45+20, 147);
	}
	else
	{
		gfx.font.drawText(game.texts.curOpt + game.settingsFile, 36, 46+20, 0);
		gfx.font.drawText(game.texts.curOpt + game.settingsFile, 35, 45+20, 147);
	}
	
	
/* TODO
	if(!settingsfile[0])
	{
		DrawTextMW(txt_curoptnofile, 36, 46+20, 0);
		DrawTextMW(txt_curoptnofile, 35, 45+20, 147);
	} else // E8A6
	{
char buffer[256];
		sprintf(buffer, txt_curopt, settingsfile);
		DrawTextMW(buffer, 36, 46+20, 0);
		DrawTextMW(buffer, 35, 45+20, 147);
	} // E90E
	*/
	
	if(curMenu == 0)
		mainMenu.draw(53, 20, false, curSel, firstMenuItem);
	else
		mainMenu.draw(53, 20, true, -1, firstMenuItem);
}

int upperCaseOnly(int k)
{
	k = std::toupper(k);
	
	if((k >= 'A' && k <= 'Z')
	|| (k == 0x8f || k == 0x8e || k == 0x99) // � �and �
	|| (k >= '0' && k <= '9'))
		return k;
		
	return 0;
}

int Gfx::menuLoop()
{
	std::memset(pal.entries, 0, sizeof(pal.entries));
	game.processViewports();
	game.drawViewports();
	flip();
	process();
	
	fillRect(0, 151, 160, 7, 0);
	font.drawText(game.texts.copyright2, 2, 152, 19);
	
	int curSel = firstMenuItem;
	int curSel2 = 0;
	int fadeValue = 0;
	curMenu = 0;

	std::memcpy(&frozenScreen[0], screen->pixels, frozenScreen.size());

	updateSettingsMenu();
	
	bool menuRunning = true;
	menuCyclic = 0;
		
	do
	{
		menuCyclic = (menuCyclic + 1) % 5;
		
		drawBasicMenu(curSel);
		
		if(curMenu == 1)
		{
			settingsMenu.draw(178, 20, false, curSel2);
			settingsMenuValues.draw(278, 20, false, curSel2);
		}
		else
		{
			settingsMenu.draw(178, 20, true);
			settingsMenuValues.draw(278, 20, true);
		}
		
		if(testSDLKeyOnce(SDLK_ESCAPE))
		{
			if(curMenu == 1)
				curMenu = 0;
			else
				curSel = 3;
		}
		
		if(testSDLKeyOnce(SDLK_UP))
		{
			sfx.play(26, -1);
			
			if(curMenu == 0)
			{
				--curSel;
				if(curSel < firstMenuItem)
				{
					curSel = int(mainMenu.items.size() - 1);
				}
			}
			else if(curMenu == 1)
			{
				--curSel2;
				if(curSel2 < 0)
				{
					curSel2 = int(settingsMenu.items.size() - 1);
				}
			}
		}
		
		if(testSDLKeyOnce(SDLK_DOWN))
		{
			sfx.play(25, -1);
			
			
			if(curMenu == 0)
			{
				++curSel;
				if(curSel >= int(mainMenu.items.size()))
				{
					curSel = firstMenuItem;
				}
			}
			else if(curMenu == 1)
			{
				++curSel2;
				
				if(curSel2 >= int(settingsMenu.items.size()))
				{
					curSel2 = 0;
				}
			}
		}

		if(testSDLKeyOnce(SDLK_RETURN)
		|| testSDLKeyOnce(SDLK_KP_ENTER))
		{
			sfx.play(27, -1);
			
			if(curMenu == 0)
			{
				if(curSel == 2)
				{
					curMenu = 1; // Go into settings menu
				}
				else // ED71
				{
					curMenu = 0;
					menuRunning = false;
				} // ED75
			}
			else if(curMenu == 1)
			{
				settingEnter(curSel2);
				updateSettingsMenu();
			}
		}
		
		if(testSDLKeyOnce(SDLK_s)) // TODO: Check for the real 's' here?
		{
			if(inputString(game.settingsFile, 8, 35, 65, upperCaseOnly, "Filename: ", false))
			{
				game.saveSettings();
			}
		}
		
		if(testSDLKeyOnce(SDLK_l)) // TODO: Check if inputString should make a sound even when loading fails
		{
			while(inputString(game.settingsFile, 8, 35, 65, upperCaseOnly, "Filename: ", false))
			{
				if(game.loadSettings())
				{
					updateSettingsMenu();
					break;
				}
			}
		}

		if(curMenu == 1)
		{
			if(testSDLKey(SDLK_LEFT))
			{
				settingLeftRight(-1, curSel2);
				updateSettingsMenu();
			} // EDAE
			if(testSDLKey(SDLK_RIGHT))
			{
				settingLeftRight(1, curSel2);
				updateSettingsMenu();
			} // EDBF
		}

		setWormColours();
		origpal.rotate(168, 174);
		pal = origpal;

		if(fadeValue < 32)
		{
			fadeValue += 1;
			pal.fade(fadeValue);
		} // EDE3
		
		flip();
		process();
	}
	while(menuRunning);

	for(int w = 32; w > 0; --w)
	{
		pal = origpal;
		pal.fade(w);
		flip(); // TODO: We should just screen sync and set the palette here
	} // EE36
	
	return curSel;
}


void Gfx::drawTextSmall(char const* str, int x, int y)
{
	for(; *str; ++str)
	{
		unsigned char c = *str - 'A';
		
		if(c < 26)
		{
			blitImage(screen, textSprites.spritePtr(c), x, y, 4, 4);
		}
		
		x += 4;
	}
}
