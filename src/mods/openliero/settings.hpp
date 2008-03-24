#ifndef LIERO_SETTINGS_HPP
#define LIERO_SETTINGS_HPP

#include "worm.hpp"
#include <string>
#include <cstring>
#include "OLXModInterface.h"
using namespace OlxMod;

struct Settings
{
	enum
	{
		GMKillEmAll,
		GMGameOfTag,
		GMCtF,
		GMSimpleCtF
	};
	
	static int const selectableWeapons = 5;
	
	static int const wormAnimTab[];
	
	Settings();
	
	bool load(std::string const& path);
	void save(std::string const& path);
	
	static void generateName(WormSettings& ws);
	
	unsigned char weapTable[40];
	int maxBonuses;
	int blood;
	int timeToLose;
	int flagsToWin;
	int gameMode;
	bool shadow;
	bool loadChange;
	bool namesOnBonuses;
	bool regenerateLevel;
	int lives;
	int loadingTime;
	bool randomLevel;
	std::string levelFile;
	bool map;
	bool screenSync;
	
	WormSettings wormSettings[OLXMOD_MAX_PLAYERS];
};

#endif // LIERO_SETTINGS_HPP
