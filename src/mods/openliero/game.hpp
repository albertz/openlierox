#ifndef LIERO_GAME_HPP
#define LIERO_GAME_HPP

#include <vector>
#include "level.hpp"
#include "settings.hpp"
#include "weapon.hpp"
#include "sobject.hpp"
#include "nobject.hpp"
#include "bobject.hpp"
#include "rand.hpp"
#include "bonus.hpp"
#include <string>

struct Viewport;
struct Worm;

extern int stoneTab[3][4];

struct Material
{
	enum
	{
		Dirt = 1<<0,
		Dirt2 = 1<<1,
		Rock = 1<<2,
		Background = 1<<3,
		SeeShadow = 1<<4,
		WormM = 1<<5
	};
	
	bool dirt() { return (flags & Dirt) != 0; }
	bool dirt2() { return (flags & Dirt2) != 0; }
	bool rock() { return (flags & Rock) != 0; }
	bool background() { return (flags & Background) != 0; }
	bool seeShadow() { return (flags & SeeShadow) != 0; }
		
	// Constructed
	bool dirtRock() { return (flags & (Dirt | Dirt2 | Rock)) != 0; }
	bool anyDirt() { return (flags & (Dirt | Dirt2)) != 0; }
	bool dirtBack() { return (flags & (Dirt | Dirt2 | Background)) != 0; }
	bool worm() { return (flags & WormM) != 0; }
	
	int flags;
};

struct Texture
{
	bool nDrawBack; // 1C208
	int  mFrame; // 1C1EA
	int  sFrame; // 1C1F4
	int  rFrame; // 1C1FE
};

struct Texts
{
	void loadFromEXE();

	std::string copyright1;	
	std::string copyright2;
	std::string loadoptions;
	std::string saveoptions;
	std::string curOptNoFile;
	std::string curOpt;
	
	std::string gameModes[4];
	std::string gameModeSpec[3];
	std::string onoff[2];
	std::string controllers[2];
	
	std::string keyNames[177];
	
	std::string random;
	std::string random2;
	std::string reloadLevel;
	std::string regenLevel;
	std::string selWeap;
	std::string levelRandom;
	std::string levelIs1;
	std::string levelIs2;
	std::string randomize;
	std::string done;
	
	std::string kills;
	std::string lives;
	
	std::string suicide;
	std::string reloading;
	std::string pressFire;
	std::string selLevel;
	
	std::string noWeaps;
	std::string weapon;
	std::string availability;
	std::string weapStates[3];
	
	
	int copyrightBarFormat;
	
};

struct AIParams
{
	int k[2][7]; // 0x1AEEE, contiguous words
};

struct Game
{
	~Game();
	
	void processViewports();
	void drawViewports();
	void clearWorms();
	void addWorm(const Worm &);
	void resetWorms();
	void initGame();
	void draw();
	void startGame(bool isStartingGame);
	bool isGameOver();
	
	void initGame_OlxMod_01();
	void gameLoop_OlxMod_01();
	
	void loadMaterials();
	void loadWeapons();
	void loadTextures();
	void loadOthers();
	void generateLevel();
	
	void saveSettings();
	bool loadSettings();
	
	Material pixelMat(int x, int y)
	{
		return materials[level.pixel(x, y)];
	}
	
	Material materials[256];
	Level level;
	Settings settings;
	Texts texts;
	Texture textures[9];
	Weapon weapons[40];
	SObjectType sobjectTypes[14];
	NObjectType nobjectTypes[24];
	int weapOrder[41]; // 1-based!
	int bonusRandTimer[2][2];
	int bonusSObjects[2];
	AIParams aiParams;
	
	bool oldRandomLevel;
	std::string oldLevelFile;
	
	int cycles;
	int lastKilled; // Last killed worm
	bool gotChanged;
	// ----- Changed when importing to OLX -----
	NetSyncedRand rand;
	// ----- Changed when importing to OLX -----

	std::string settingsFile; // Currently loaded settings file
	
	std::vector<Viewport> viewports;
	std::vector<Worm> worms;
	
	typedef ObjectList<Bonus, 99> BonusList;
	typedef ObjectList<WObject, 600> WObjectList;
	typedef ObjectList<SObject, 700> SObjectList;
	typedef ObjectList<NObject, 600> NObjectList;
	typedef ObjectList<BObject, 700> BObjectList;
	BonusList bonuses;
	WObjectList wobjects;
	SObjectList sobjects;
	NObjectList nobjects;
	BObjectList bobjects;
};

bool checkRespawnPosition(int x2, int y2, int oldX, int oldY, int x, int y);

extern void createBonus();

extern Game game;

#endif // LIERO_GAME_HPP

