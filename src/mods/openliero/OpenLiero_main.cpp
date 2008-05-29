#ifndef LIERO_MAIN_HPP
#define LIERO_MAIN_HPP

#include <SDL/SDL.h>
#include <SDL/SDL_getenv.h>

#include "gfx.hpp"
#include "sfx.hpp"
#include "sys.hpp"
#include "game.hpp"
#include "viewport.hpp"
#include "worm.hpp"
#include "reader.hpp"
#include "filesystem.hpp"
#include "text.hpp"
#include "keys.hpp"
#include "constants.hpp"
#include "math.hpp"
#include "console.hpp"
#include "platform.hpp"

#include <iostream>
#include <ctime>
#include <exception>

//#undef main

int gameEntry(int argc, char* argv[])
try
{
	// TODO: Better PRNG seeding
	Console::init();
	//gfx.rand.seed(Uint32(std::time(0)));
	
	bool exeSet = false;
	
	for(int i = 1; i < argc; ++i)
	{
		if(argv[i][0] == '-')
		{
			switch(argv[i][1])
			{
			case 'v':
				// SDL_putenv seems to take char* in linux, STOOPID
				SDL_putenv(const_cast<char*>((std::string("SDL_VIDEODRIVER=") + &argv[i][2]).c_str()));
			break;
			}
		}
		else
		{
			setLieroEXE(argv[i]);
			exeSet = true;
		}
	}
	
	if(!exeSet)
		setLieroEXE("LIERO.EXE");

	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
	
/*
	char buf[256];
	std::cout << SDL_VideoDriverName(buf, 256) << std::endl;
*/
	
		
	game.texts.loadFromEXE();
	initKeys();
	game.rand.seed(Uint32(std::time(0)));
	loadConstantsFromEXE();
	loadTablesFromEXE();

	Console::clear();
	Console::writeTextBar(game.texts.copyright1, game.texts.copyrightBarFormat);
	Console::setAttributes(0x07);
	Console::writeLine("");
	
	Console::write(S[LoadingAndThinking]);
	gfx.font.loadFromEXE();
	gfx.loadPalette();
	gfx.loadMenus();
	gfx.loadGfx();
	game.loadMaterials();
	game.loadWeapons();
	game.loadTextures();
	game.loadOthers();
	Console::writeLine(S[OK]);
	
	Console::writeLine(S[InitSound]);
	sfx.init();
	
	Console::write(S[Init_BaseIO]);
	Console::write("0220");
	Console::write(S[Init_IRQ]);
	Console::write("7");
	Console::write(S[Init_DMA8]);
	Console::write("1");
	Console::write(S[Init_DMA16]);
	Console::writeLine("5");
	
	Console::write(S[Init_DSPVersion]);

	//SDL_version const* mixerVer = Mix_Linked_Version();
	//Console::write(toString(mixerVer->major) + "." + toString(mixerVer->minor));
	Console::write(S[Init_Colon]);
	Console::write(S[Init_16bit]);
	Console::writeLine(S[Init_Autoinit]);
	
	Console::writeLine(S[Init_XMSSucc]);
	
	Console::write(S[Init_FreeXMS]);
#ifdef LIERO_WIN32
	Console::write(toString(Win32::getFreeMemory()));
#else
	Console::write("ONE MILLION ");
#endif
	Console::write(S[Init_k]);
	
	Console::write(S[LoadingSounds]);
	sfx.loadFromSND();
	Console::writeLine(S[OK2]);
	
	Console::writeLine("");
	Console::write(S[PressAnyKey]);
	Console::waitForAnyKey();
	Console::clear();
	
	gfx.init();
	
	game.settingsFile = "LIERO";
	
	if(!fileExists(lieroOPT)) // NOTE: Liero doesn't seem to use the contents of LIERO.OPT for anything useful
	{
		game.settings = Settings();
		game.saveSettings();
	}
	else
	{
	/*
		FILE* f = fopen(lieroOPT.c_str(), "rb");
		std::size_t len = fileLength(f);
		if(len > 255) len = 255;
		char buf[256];
		fread(buf, 1, len, f);
		game.settingsFile.assign(buf, len);
		
		rtrim(game.settingsFile);
		*/
		if(!game.loadSettings())
		{
			game.settingsFile = "LIERO";
			game.settings = Settings();
			game.saveSettings();
		}

		//fclose(f);
	}
	
	game.initGame();
	gfx.mainLoop();
	
	game.settingsFile = "LIERO";
	game.settings.save(joinPath(lieroEXERoot, "LIERO.DAT"));
	
	FILE* f = fopen(lieroOPT.c_str(), "wb");
	fwrite(game.settingsFile.data(), 1, game.settingsFile.size(), f);
	fputc('\r', f);
	fputc('\n', f);
	fclose(f);
	
	closeAllCachedFiles();
	
	SDL_Quit();
	
	return 0;
}
catch(std::exception& ex)
{
	SDL_Quit();
	Console::setAttributes(0x2f);
	Console::writeLine(std::string("EXCEPTION: ") + ex.what());
	Console::waitForAnyKey();
	return 1;
}

#endif // LIERO_MAIN_HPP

