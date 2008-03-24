#ifndef LIERO_GFX_HPP
#define LIERO_GFX_HPP

#include <SDL/SDL.h>
#include <cstdio>
#include <cassert>
#include "font.hpp"
#include "menu.hpp"
#include "colour.hpp"
#include "rect.hpp"
#include "rand.hpp"
#include "keys.hpp"

struct Palette
{
	SDL_Color entries[256];
	
	void activate();
	void fade(int amount);
	void lightUp(int amount);
	void rotate(int from, int to);
	void read(FILE* f);
	
	void scaleAdd(int dest, int c[3], int scale, int add)
	{
		entries[dest].r = (add + c[0] * scale) / 64;
		entries[dest].g = (add + c[1] * scale) / 64;
		entries[dest].b = (add + c[2] * scale) / 64;
		
		assert(entries[dest].r < 64);
		assert(entries[dest].g < 64);
		assert(entries[dest].b < 64);
	}
		
	void setWormColours(int base, int c[3])
	{
		scaleAdd(base - 2, c, 38, 0);
		scaleAdd(base - 1, c, 50, 0);
		scaleAdd(base    , c, 64, 0);
		scaleAdd(base + 1, c, 47, 1008);
		scaleAdd(base + 2, c, 28, 2205);
	}
	
	void clear();
};

struct SpriteSet
{
	std::vector<PalIdx> data;
	int width;
	int height;
	int spriteSize;
	int count;
	
	void read(FILE* f, int width, int height, int count);
	
	PalIdx* spritePtr(int frame)
	{
		return &data[frame*spriteSize];
	}
	
	void allocate(int width, int height, int count);
};

struct Key
{
	Key(int sym, char ch)
	: sym(sym), ch(ch)
	{
	}
	
	int sym;
	char ch;
};

struct ColourAnim
{
	int from;
	int to;
};

struct Gfx
{
	Gfx();
	
	static int fireConeOffset[2][7][2];
		
	void init();
	void setVideoMode();
	void loadPalette();
	void loadMenus();
	void loadGfx();
	void process();
	void flip();

	void flip_OlxMod_01();
	
	void clear();
	void clearKeys();
	
	unsigned char& getScreenPixel(int x, int y)
	{
		return (static_cast<unsigned char*>(screenPixels) + y*screenPitch)[x];
	}
	
	PalIdx* wormSprite(int f, int dir, int w)
	{
		// ----- Changed when importing to OLX -----
		w %= 2;	// We have only 2 worm sprites by now
		// ----- Changed when importing to OLX -----
		return wormSprites.spritePtr(f + dir*7*3 + w*2*7*3);
	}
	
	PalIdx* fireConeSprite(int f, int dir)
	{
		return fireConeSprites.spritePtr(f + dir*7);
	}
	
	/*
	bool testKeyOnce(int key)
	{
		bool state = keys[key];
		keys[key] = false;
		return state;
	}
	
	bool testKey(int key)
	{
		return keys[key];
	}
	
	void releaseKey(int key)
	{
		keys[key] = false;
	}*/
	
	// ----- Changed when importing to OLX -----
	// Hook the keys to OLX input

	bool testKeyOnce(Uint32 key);
	bool testKey(Uint32 key);
	void releaseKey(Uint32 key);
	void pressKey(Uint32 key);
	void setKey(Uint32 key, bool state);
	void toggleKey(Uint32 key);
	bool testSDLKeyOnce(SDLKey key);
	bool testSDLKey(SDLKey key);
	void releaseSDLKey(SDLKey key);
	
	// ----- Changed when importing to OLX -----

	void resetPalette(Palette& newPal)
	{
		origpal = newPal;
		setWormColours();
	}
	
	SDL_keysym waitForKey();
	
	void settingEnter(int item);
	void settingLeftRight(int change, int item);
	void updateSettingsMenu();
	void updatePlayerMenu(int player);
	void setWormColours();
	int menuLoop();
	void mainLoop();
	void drawBasicMenu(int curSel);
	void playerSettings(int player);
	//void inputString(std::string& dest, std::size_t maxLen, int x, int y, bool onlyDigits = false);
	bool inputString(std::string& dest, std::size_t maxLen, int x, int y, int (*filter)(int) = 0, std::string const& prefix = "", bool centered = true);
	void inputInteger(int& dest, int min, int max, std::size_t maxLen, int x, int y);
	
	void drawTextSmall(char const* str, int x, int y);
		
	int firstMenuItem; // The first visible item in the mainMenu
	Menu mainMenu;
	Menu settingsMenu;
	Menu settingsMenuValues;
	Menu playerMenu;
	Menu playerMenuValues;
	int curMenu;
	
	SpriteSet smallSprites;
	SpriteSet largeSprites;
	SpriteSet textSprites;
	SpriteSet wormSprites;
	SpriteSet fireConeSprites;
	
	Palette pal;
	Palette origpal;
	Palette exepal;
	ColourAnim colourAnim[4];
	int bonusFrames[2];
	//bool keys[SDLK_LAST];
	bool dosKeys[177];
	SDL_Surface* screen;
	SDL_Surface* back;
	std::vector<PalIdx> frozenScreen;
	unsigned char* screenPixels;
	unsigned int screenPitch;
	int screenFlash;
	Font font;
	bool running;
	bool fullscreen;
	bool doubleRes;
	Uint32 lastFrame;
	int menuCyclic;
	Rand rand; // PRNG for things that don't affect the game
};

void fillRect(int x, int y, int w, int h, int colour);
void drawBar(int x, int y, int width, int colour);
void drawRoundedBox(int x, int y, int colour, int height, int width);
void blitImageNoKeyColour(SDL_Surface* scr, PalIdx* mem, int x, int y, int width, int height, int pitch);
void blitImage(SDL_Surface* scr, PalIdx* mem, int x, int y, int width, int height);
void blitImageR(SDL_Surface* scr, PalIdx* mem, int x, int y, int width, int height);
void blitShadowImage(SDL_Surface* scr, PalIdx* mem, int x, int y, int width, int height);
void blitStone(bool p1, PalIdx* mem, int x, int y);
void blitFireCone(SDL_Surface* scr, int fc, PalIdx* mem, int x, int y);
void drawDirtEffect(int dirtEffect, int x, int y);
void blitImageOnMap(PalIdx* mem, int x, int y, int width, int height);
void correctShadow(Rect rect);

void drawNinjarope(int fromX, int fromY, int toX, int toY);
void drawLaserSight(int fromX, int fromY, int toX, int toY);
void drawShadowLine(int fromX, int fromY, int toX, int toY);
void drawLine(int fromX, int fromY, int toX, int toY, int colour);
bool isInside(SDL_Rect const& rect, int x, int y);

inline void blitImageNoKeyColour(SDL_Surface* scr, PalIdx* mem, int x, int y, int width, int height)
{
	blitImageNoKeyColour(scr, mem, x, y, width, height, width);
}

extern Gfx gfx;

#endif // LIERO_GFX_HPP
