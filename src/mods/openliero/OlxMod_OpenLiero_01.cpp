
// Hooks in OpenLiero code for OLX modding system

#include "OLXModInterface.h"
using namespace OlxMod;
//#include "GfxPrimitives.h"

#include "gfx.hpp"
#include "sfx.hpp"
#include "game.hpp"
#include "weapsel.hpp"
#include "reader.hpp"
#include "viewport.hpp"
#include "worm.hpp"
#include "filesystem.hpp"
#include "gfx.hpp"
#include "sfx.hpp"
#include "weapsel.hpp"
#include "constants.hpp"

namespace OlxMod_OpenLiero_01	// To avoid name collision with other mods
{

int numPlayers = -1;
int localPlayer = -1;
SDL_Surface *OLXOutput = NULL;

// Saved variables
unsigned long currentTime = 0;
long currentTimeDiff = 0;

// For selecting weapons
bool bSelectingWeapons = true;
int curSel[OLXMOD_MAX_PLAYERS];
bool isReady[OLXMOD_MAX_PLAYERS];
Menu menus[OLXMOD_MAX_PLAYERS];
int fadeValue = 0;
int enabledWeaps = 0;

// For game loop
int fadeAmount = 180;
bool shutDown = false;

// Keyboard hooks
enum { MAX_KEYS = 7 };	// OpenLiero uses only 7 keys

bool keys [ MAX_KEYS * OLXMOD_MAX_PLAYERS ];
bool keysChanged [ MAX_KEYS * OLXMOD_MAX_PLAYERS ];

bool & getKey( int worm, int key )
{
	return keys[ worm * MAX_KEYS + key ];
};

bool & getKeyChanged( int worm, int key )
{
	return keysChanged[ worm * MAX_KEYS + key ];
};

void OlxMod_InitFunc( int _numPlayers, int _localPlayer, 
	std::map< std::string, CScriptableVars::ScriptVar_t > options,
	std::map< std::string, OlxMod_WeaponRestriction_t > weaponRestrictions,
	int ScreenX, int ScreenY, SDL_Surface *bmpDest )
{
	numPlayers = _numPlayers;
	localPlayer = _localPlayer;
	OLXOutput = bmpDest;

	for( int f = 0; f < MAX_KEYS * OLXMOD_MAX_PLAYERS; f++ )
		keys[f] = false;
	
	setLieroEXE("OpenLiero/LIERO.EXE");

		
	game.texts.loadFromEXE();
	initKeys();
	game.rand.seed(Uint32(std::time(0)));
	loadConstantsFromEXE();
	loadTablesFromEXE();

	gfx.font.loadFromEXE();
	gfx.loadPalette();
	gfx.loadMenus();
	gfx.loadGfx();
	game.loadMaterials();
	game.loadWeapons();
	game.loadTextures();
	game.loadOthers();

	sfx.init();
	
	sfx.loadFromSND();
	
	gfx.lastFrame = 0;
	gfx.screen = SDL_CreateRGBSurface(SDL_SWSURFACE, 320, 200, 8, 0, 0, 0, 0);
	gfx.back = gfx.screen;
	gfx.screenPixels = static_cast<unsigned char*>(gfx.screen->pixels);
	gfx.screenPitch = gfx.screen->pitch;
	
	game.settingsFile = "LIERO";
	
	if(!fileExists(lieroOPT)) // NOTE: Liero doesn't seem to use the contents of LIERO.OPT for anything useful
		game.settings = Settings();
	else 
		if(!game.loadSettings())
		{
			game.settingsFile = "LIERO";
			game.settings = Settings();
		};
	
	game.generateLevel();
	game.resetWorms();
	gfx.pal.clear();	

	game.initGame_OlxMod_01();
	game.cycles = 0;

	bSelectingWeapons = true;
	currentTime = 0;
	selectWeaponsInit_OlxMod_01();
	fadeAmount = 180;
	shutDown = false;
	
	game.settings.loadChange = true;	// Enable combo
};

struct savedState_t
{
	unsigned long currentTime;
	unsigned long currentTimeDiff;
	bool bSelectingWeapons;

	// Weapon selection menu
	int curSel[OLXMOD_MAX_PLAYERS];
	bool isReady[OLXMOD_MAX_PLAYERS];
	Menu menus[OLXMOD_MAX_PLAYERS];

	int fadeValue;
	int enabledWeaps;

	int fadeAmount;
	bool shutDown;
	
	// Game
	Game game;
	
	Worm worms[OLXMOD_MAX_PLAYERS];
	Viewport viewports[OLXMOD_MAX_PLAYERS];
	
	bool keys [ MAX_KEYS * OLXMOD_MAX_PLAYERS ];
	bool keysChanged [ MAX_KEYS * OLXMOD_MAX_PLAYERS ];
};
savedState_t savedState;

void OlxMod_DeInitFunc()
{
	printf("OlxMod_DeInitFunc()\n");
	// TODO: doesn't free all allocated memory, check with some mem debug tool
	savedState.game.viewports.clear();
	savedState.game.worms.clear();

	game.clearViewports();
	game.clearWorms();
	closeAllCachedFiles();
	SDL_FreeSurface(gfx.screen);
	gfx.screen = NULL;
	OLXOutput = NULL;
};

void OlxMod_SaveState()
{
	savedState.currentTime = currentTime;
	savedState.currentTimeDiff = currentTimeDiff;
	savedState.bSelectingWeapons = bSelectingWeapons;
	savedState.fadeValue = fadeValue;
	savedState.enabledWeaps = enabledWeaps;
	savedState.fadeAmount = fadeAmount;
	savedState.shutDown = shutDown;
	savedState.game = game;	// Big big bulky copy - TODO: optimize that
	for( int f=0; f<numPlayers; f++ )
	{
		savedState.worms[f] = *game.worms[f];
		savedState.viewports[f] = *game.viewports[f];
		for( int f1=0; f1<MAX_KEYS; f1++ )
		{
			savedState.keys[f1+f*MAX_KEYS] = keys[f1+f*MAX_KEYS];
			savedState.keysChanged[f1+f*MAX_KEYS] = keysChanged[f1+f*MAX_KEYS];
		};
		savedState.curSel[f] = curSel[f];
		savedState.isReady[f] = isReady[f];
		savedState.menus[f] = menus[f];
	};
	// Hopefully I didn't miss something
};

void OlxMod_RestoreState()
{
	currentTime = savedState.currentTime;
	currentTimeDiff = savedState.currentTimeDiff;
	bSelectingWeapons = savedState.bSelectingWeapons;
	fadeValue = savedState.fadeValue;
	enabledWeaps = savedState.enabledWeaps;
	fadeAmount = savedState.fadeAmount;
	shutDown = savedState.shutDown;
	game = savedState.game;
	for( int f=0; f<numPlayers; f++ )
	{
		*game.worms[f] = savedState.worms[f];
		*game.viewports[f] = savedState.viewports[f];
		for( int f1=0; f1<MAX_KEYS; f1++ )
		{
			keys[f1+f*MAX_KEYS] = savedState.keys[f1+f*MAX_KEYS];
			keysChanged[f1+f*MAX_KEYS] = savedState.keysChanged[f1+f*MAX_KEYS];
		};
		curSel[f] = savedState.curSel[f];
		isReady[f] = savedState.isReady[f];
		menus[f] = savedState.menus[f];
	};
};

void OlxMod_CalculatePhysics( unsigned gameTime, const std::map< int, OlxMod_Event_t > &keys, bool fastCalculation )
{
	currentTimeDiff += gameTime - currentTime;
	currentTime = gameTime;
	//printf("OlxMod_CalculatePhysics() currentTime %lu currentTimeDiff %li\n", currentTime, currentTimeDiff );
	for( std::map< int, OlxMod_Event_t > :: const_iterator it = keys.begin(); it != keys.end(); it++ )
	{
		//printf("OlxMod_CalculatePhysics: player %i keys %i%i%i%i%i%i%i%i changed %i%i%i%i%i%i%i%i\n", it->first, 
		//it->second.keys.up, it->second.keys.down, it->second.keys.left, it->second.keys.right, it->second.keys.shoot, it->second.keys.selweap, it->second.keys.jump, it->second.keys.rope,
		//it->second.keysChanged.up, it->second.keysChanged.down, it->second.keysChanged.left, it->second.keysChanged.right, it->second.keysChanged.shoot, it->second.keysChanged.selweap, it->second.keysChanged.jump, it->second.keysChanged.rope );
		getKeyChanged( it->first, 0 ) = getKey( it->first, 0 ) != it->second.keys.up;
		getKeyChanged( it->first, 1 ) = getKey( it->first, 1 ) != it->second.keys.down;
		getKeyChanged( it->first, 2 ) = getKey( it->first, 2 ) != it->second.keys.left;
		getKeyChanged( it->first, 3 ) = getKey( it->first, 3 ) != it->second.keys.right;
		getKeyChanged( it->first, 4 ) = getKey( it->first, 4 ) != it->second.keys.shoot;
		getKeyChanged( it->first, 5 ) = getKey( it->first, 5 ) != it->second.keys.selweap;
		getKeyChanged( it->first, 6 ) = getKey( it->first, 6 ) != it->second.keys.jump;
		getKey( it->first, 0 ) = it->second.keys.up;
		getKey( it->first, 1 ) = it->second.keys.down;
		getKey( it->first, 2 ) = it->second.keys.left;
		getKey( it->first, 3 ) = it->second.keys.right;
		getKey( it->first, 4 ) = it->second.keys.shoot;
		getKey( it->first, 5 ) = it->second.keys.selweap;
		getKey( it->first, 6 ) = it->second.keys.jump;
	};
	while( currentTimeDiff > 0 )
	{
		game.gameLoop_OlxMod_01();
		currentTimeDiff -= OlxMod_GameSpeed_Fastest; // Assume 10 ms frame, may be changed later
	};
	//printf("OlxMod_CalculatePhysics() exit\n");
	// Some debug
	if( ! fastCalculation && gameTime % 5000 == 0 )
	{
		unsigned f;
		unsigned level=0;
		for( f=0; f < game.level.data.size(); f++ )
			level += unsigned(game.level.data[f]) * unsigned(f+1);

		unsigned worms=0;
		for( f=0; f < game.worms.size(); f++ )
		{
			worms += game.worms[f]->x;
			worms += game.worms[f]->y*100;
			worms += game.worms[f]->velX*10000;
			worms += game.worms[f]->velY*1000000;
			worms += game.worms[f]->aimingAngle*100000000;
		}

		unsigned viewports=0;
		for( f=0; f < game.viewports.size(); f++ )
		{
			viewports += game.viewports[f]->x;
			viewports += game.viewports[f]->y*100;
			viewports += game.viewports[f]->centerX*10000;
			viewports += game.viewports[f]->centerY*1000000;
		}
		unsigned random = game.rand();
		unsigned total = level + worms + viewports + random;
		printf( "OpenLiero checksums for time %i sec: level 0x%X, worms 0x%X viewports 0x%X random 0x%X total 0x%X\n", gameTime / 1000, level, worms, viewports, random, total );
	};
};

void OlxMod_Draw( bool showScoreboard )
{
	//game.drawViewports();
	
	// Clean the part of the screen with healthbar
	for(int y = 160; y < 200; ++y)
		for(int x = 0; x < 120; ++x)
			gfx.screenPixels[ y * 320 + x ] = 0;
	if( ! bSelectingWeapons )
		game.viewports[localPlayer]->draw();
	gfx.flip_OlxMod_01();
};

void OlxMod_GetOptions( std::map< std::string, CScriptableVars::ScriptVarType_t > * options, std::vector<std::string> * WeaponList )
{
};


bool OlxMod_registered = OlxMod_RegisterMod( "Liero Orthodox v0.1", &OlxMod_InitFunc, &OlxMod_DeInitFunc,
							&OlxMod_SaveState, &OlxMod_RestoreState,
							&OlxMod_CalculatePhysics, &OlxMod_Draw, &OlxMod_GetOptions );

}; // namespace

using namespace OlxMod_OpenLiero_01;

void selectWeaponsInit_OlxMod_01()
{
	enabledWeaps = 0;
	for(int i = 0; i < 40; ++i)
	{
		if(game.settings.weapTable[i] == 0)
			++enabledWeaps;
	}
	
	for(int i = 0; i < numPlayers; ++i)
	{
		bool weapUsed[256] = {};
		
		Worm& worm = *game.worms[i];
		WormSettings& ws = *worm.settings;
		
		menus[i].items.clear();
		menus[i].items.push_back(MenuItem(1, 1, game.texts.randomize));
		
		for(int j = 0; j < game.settings.selectableWeapons; ++j)
		{
			if(ws.weapons[j] == 0)
			{
				ws.weapons[j] = game.rand(1, 41);
			}
			
			bool enoughWeapons = (enabledWeaps >= game.settings.selectableWeapons);
			
			while(true)
			{
				int w = game.weapOrder[ws.weapons[j]];
				
				if((!enoughWeapons || !weapUsed[w])
				&& game.settings.weapTable[w] <= 0)
					break;
					
				ws.weapons[j] = game.rand(1, 41);
			}
			
			int w = game.weapOrder[ws.weapons[j]];
			
			weapUsed[w] = true;
			
			WormWeapon& ww = worm.weapons[j];
			
			ww.ammo = 0;
			ww.id = w;
			
			menus[i].items.push_back(MenuItem(48, 48, game.weapons[w].name));
		}
		
		menus[i].items.push_back(MenuItem(10, 10, game.texts.done));
		
		worm.currentWeapon = 0;
		
		curSel[i] = 0;
		isReady[i] = false;
	}

	game.processViewports();
	game.drawViewports();
	
	drawRoundedBox(114, 2, 0, 7, gfx.font.getWidth(game.texts.selWeap));
	
	gfx.font.drawText(game.texts.selWeap, 116, 3, 50);
	
	WormSettings& ws = game.settings.wormSettings[0];
	int selWeapX = ws.selWeapX;
	int width = gfx.font.getWidth(ws.name);
	drawRoundedBox(selWeapX + 29 - width/2, 18, 0, 7, width);
	gfx.font.drawText(ws.name, selWeapX + 31 - width/2, 19, ws.colour + 1);
		
	if(game.settings.levelFile.empty())
	{
		gfx.font.drawText(game.texts.levelRandom, 0, 162, 50);
	}
	else
	{
		gfx.font.drawText((game.texts.levelIs1 + game.settings.levelFile + game.texts.levelIs2), 0, 162, 50);
	}
	
	std::memcpy(&gfx.frozenScreen[0], gfx.screen->pixels, gfx.frozenScreen.size());
	
	fadeValue = 0;
	
	/*
	// Skip weapon selection screen
	{
		bSelectingWeapons = false;
	
		for(std::size_t i = 0; i < game.worms.size(); ++i)
		{
			Worm& worm = *game.worms[i];
		
			worm.currentWeapon = 0; // It was 1 in OpenLiero A1
		
			for(int j = 0; j < game.settings.selectableWeapons; ++j)
			{
				worm.weapons[j].ammo = game.weapons[worm.weapons[j].id].ammo;
			}
			
			worm.ready = true;
		}

		sfx.play(22, 22);

		for(int w = 0; w < 40; ++w)
		{
			game.weapons[w].computedLoadingTime = (game.settings.loadingTime * game.weapons[w].loadingTime) / 100;
			if(game.weapons[w].computedLoadingTime == 0)
				game.weapons[w].computedLoadingTime = 1;
		}
	
		fadeAmount = 180;
		shutDown = false;

	};
	*/

};

void selectWeaponsLoop_OlxMod_01()
{
	printf("selectWeaponsLoop_OlxMod_01()\n");

	{
		std::memcpy(gfx.screen->pixels, &gfx.frozenScreen[0], gfx.frozenScreen.size());

		for(int i = 0; i < numPlayers; ++i)
		{
			int weapID = curSel[i] - 1;
			
			Worm& worm = *game.worms[i];
			WormSettings& ws = *worm.settings;
			
			if(!isReady[i])
			{
				if( i == localPlayer )
					menus[i].draw(ws.selWeapX - 2, 28, false, curSel[i]);
				
				if(weapID >= 0 && weapID < game.settings.selectableWeapons)
				{
					if( gfx.testKeyOnce(worm.keyLeft()) )
					{
						sfx.play(25, -1);
						
						do
						{
							--ws.weapons[weapID];
							if(ws.weapons[weapID] < 1)
								ws.weapons[weapID] = 40; // TODO: Unhardcode
						}
						while(game.settings.weapTable[game.weapOrder[ws.weapons[weapID]]] != 0);
						
						int w = game.weapOrder[ws.weapons[weapID]];
						worm.weapons[weapID].id = w;
						menus[i].items[curSel[i]].string = game.weapons[w].name;
					}
					
					if( gfx.testKeyOnce(worm.keyRight()) )
					{
						sfx.play(26, -1);
						
						do
						{
							++ws.weapons[weapID];
							if(ws.weapons[weapID] > 40)
								ws.weapons[weapID] = 1; // TODO: Unhardcode
						}
						while(game.settings.weapTable[game.weapOrder[ws.weapons[weapID]]] != 0);
						
						int w = game.weapOrder[ws.weapons[weapID]];
						worm.weapons[weapID].id = w;
						menus[i].items[curSel[i]].string = game.weapons[w].name;
					}
				}
				
				if( gfx.testKeyOnce(worm.keyUp()) )
				{
					sfx.play(26, -1);
					int s = int(menus[i].items.size());
					curSel[i] = (curSel[i] - 1 + s) % s;
				}
				
				if( gfx.testKeyOnce(worm.keyDown()) )
				{
					sfx.play(25, -1);
					int s = int(menus[i].items.size());
					curSel[i] = (curSel[i] + 1 + s) % s;
				}
				
				if( gfx.testKeyOnce(worm.keyFire()) )
				{
					if(curSel[i] == 0)
					{
						bool weapUsed[256] = {};
						
						bool enoughWeapons = (enabledWeaps >= game.settings.selectableWeapons);
						
						for(int j = 0; j < game.settings.selectableWeapons; ++j)
						{
							while(true)
							{
								ws.weapons[j] = game.rand(1, 41);
								
								int w = game.weapOrder[ws.weapons[j]];
								
								if((!enoughWeapons || !weapUsed[w])
								&& game.settings.weapTable[w] <= 0)
									break;
							}
							
							int w = game.weapOrder[ws.weapons[j]];
							
							weapUsed[w] = true;
							
							WormWeapon& ww = worm.weapons[j];
							
							ww.ammo = 0;
							ww.id = w;
							
							menus[i].items[j + 1].string = game.weapons[w].name;
						}
					}
					else if(curSel[i] == 6) // TODO: Unhardcode
					{
						sfx.play(27, -1);
						isReady[i] = true;
					}
				}
			}
		}
			
		gfx.origpal.rotate(168, 174);
		gfx.pal = gfx.origpal;
		
		if(fadeValue <= 32)
		{
			gfx.pal.fade(fadeValue);
			fadeValue += 1;
		}
		
		//gfx.flip_OlxMod_01();
		//gfx.process();
	}
	
	bool ready = true;
	for(int i = 0; i < numPlayers; ++i)
		if(!isReady[i])
			ready = false;
			
	if( ready && bSelectingWeapons )
	{
		bSelectingWeapons = false;
	
		for(std::size_t i = 0; i < game.worms.size(); ++i)
		{
			Worm& worm = *game.worms[i];
		
			worm.currentWeapon = 0; // It was 1 in OpenLiero A1
		
			for(int j = 0; j < game.settings.selectableWeapons; ++j)
			{
				worm.weapons[j].ammo = game.weapons[worm.weapons[j].id].ammo;
			}
		}
	};
};


void Game::gameLoop_OlxMod_01()
{
	//printf("Game::gameLoop_OlxMod_01()\n");
	
	if(bSelectingWeapons)
	{
		selectWeaponsLoop_OlxMod_01();
		
		if( bSelectingWeapons )
			return;
		
		sfx.play(22, 22);

		cycles = 0;
		
		for(int w = 0; w < 40; ++w)
		{
			weapons[w].computedLoadingTime = (settings.loadingTime * weapons[w].loadingTime) / 100;
			if(weapons[w].computedLoadingTime == 0)
				weapons[w].computedLoadingTime = 1;
		}
	
		fadeAmount = 180;
		shutDown = false;
	}
	
		++cycles;
		
		if(!H[HBonusDisable]
		&& settings.maxBonuses > 0
		&& rand(C[BonusDropChance]) == 0)
		{
			createBonus();
		}
			
		for(std::size_t i = 0; i < worms.size(); ++i)
		{
			worms[i]->process();
		}
		
		for(std::size_t i = 0; i < worms.size(); ++i)
		{
			worms[i]->ninjarope.process(*worms[i]);
		}
		
		switch(game.settings.gameMode)
		{
		case Settings::GMGameOfTag:
		{
			bool someInvisible = false;
			for(std::size_t i = 0; i < worms.size(); ++i)
			{
				if(!worms[i]->visible)
				{
					someInvisible = true;
					break;
				}
			}
			
			if(!someInvisible
			&& lastKilled
			&& (cycles % 70) == 0
			&& lastKilled->timer < settings.timeToLose)
			{
				++lastKilled->timer;
			}
		}
		break;
		}
		
		processViewports();
		//drawViewports();
				
		for(BonusList::iterator i = bonuses.begin(); i != bonuses.end(); ++i)
		{
			i->process();
		}
		
		if((cycles & 1) == 0)
		{
			for(std::size_t i = 0; i < viewports.size(); ++i)
			{
				Viewport& v = *viewports[i];
				
				bool down = false;
				
				if(v.worm->killedTimer > 16)
					down = true;
					
				if(down)
				{
					if(v.bannerY < 2)
						++v.bannerY;
				}
				else
				{
					if(v.bannerY > -8)
						--v.bannerY;
				}
			}
		}
		
		for(SObjectList::iterator i = game.sobjects.begin(); i != game.sobjects.end(); ++i)
		{
			i->process();
		}
		
		// TODO: Check processing order of bonuses, wobjects etc.
		
		for(WObjectList::iterator i = wobjects.begin(); i != wobjects.end(); ++i)
		{
			i->process();
		}
		
		for(NObjectList::iterator i = nobjects.begin(); i != nobjects.end(); ++i)
		{
			i->process();
		}
		
		for(BObjectList::iterator i = bobjects.begin(); i != bobjects.end(); ++i)
		{
			i->process();
		}
		
		if((cycles & 3) == 0)
		{
			for(int w = 0; w < 4; ++w)
			{
				gfx.origpal.rotate(gfx.colourAnim[w].from, gfx.colourAnim[w].to); // ADD gfx.colourAnim
			}
		}
		
		gfx.pal = gfx.origpal;
		
		if(fadeAmount <= 32)
			gfx.pal.fade(fadeAmount);

		if(gfx.screenFlash > 0)
		{
			gfx.pal.lightUp(gfx.screenFlash);
		}
		
		//gfx.flip_OlxMod_01();
		//gfx.process();
		
		if(gfx.screenFlash > 0)
			--gfx.screenFlash;
		
		if(isGameOver())
		{
			gfx.firstMenuItem = 1;
			shutDown = true;
		}
		
		for(std::size_t i = 0; i < viewports.size(); ++i)
		{
			if(viewports[i]->shake > 0)
				viewports[i]->shake -= 4000; // TODO: Read 4000 from exe?
		}
		
		/*
		if(gfx.testSDLKeyOnce(SDLK_ESCAPE)
		&& !shutDown)
		{
			gfx.firstMenuItem = 0;
			fadeAmount = 31;
			shutDown = true;
		}
		*/
		
		if(shutDown)
		{
			fadeAmount -= 1;
		}
		else
		{
			if(fadeAmount < 33)
			{
				fadeAmount += 1;
				if(fadeAmount >= 33)
					fadeAmount = 180;
			}
		}

	if( fadeAmount <= 0 || shutDown )
	{
		//OlxMod_EndRound();
	};
	
	//gfx.clearKeys();
	//printf("Game::gameLoop_OlxMod_01() exit\n");

}

void Game::initGame_OlxMod_01()
{
	clearWorms();
	clearViewports();
	
	bonuses.clear();
	wobjects.clear();
	sobjects.clear();
	bobjects.clear();
	nobjects.clear();
	
	
	for( int player = 0; player < numPlayers; player++ )
	{
		game.settings.wormSettings[player].controller = 0;	// Human
		Settings::generateName(game.settings.wormSettings[player]);
		game.settings.wormSettings[player].randomName = false;
		
		// TODO: find out more palette colours for all 32 players
		game.settings.wormSettings[player].colour = player % 2 ? 41 : 32;

		int f;
		for( f=0; f<3; f++ )
			game.settings.wormSettings[player].rgb[f] = game.rand() & 63;
		for( f=0; f<5; f++ )
			game.settings.wormSettings[player].weapons[f] = game.rand(1, 41);
		
		Worm* worm = new Worm(&game.settings.wormSettings[player], player, player % 2 ? 19 : 20 );
		//Worm* worm2 = new Worm(&game.settings.wormSettings[1], 1, 20);
		addWorm(worm);
	
		Worm& w = *worms[player];
		w.makeSightGreen = false;
		w.lives = game.settings.lives;
		w.ready = false;
		//w.visible = false;
		//w.movable = true;
		
		if(game.rand(2) > 0)
		{
			w.aimingAngle = itof(32);
			w.direction = 0;
		}
		else
		{
			w.aimingAngle = itof(96);
			w.direction = 1;
		}

		w.health = w.settings->health;
		w.visible = false;
		w.killedTimer = 150;
		
		w.currentWeapon = 1; // This is later changed to 0, why is it here?

		//::viewportsCache[player] = ;
		addViewport(new Viewport(Rect(0, 0, 158+160, 158), &w, 0, 504, 350));
		
		for( f=0; f<MAX_KEYS; f++ )
			w.settings->controls[f] = player*MAX_KEYS + f;
	};
	
	
	gotChanged = false;
	lastKilled = 0;

}

void Gfx::flip_OlxMod_01()
{
	//printf("Gfx::flip_OlxMod_01()\n");
	
	SDL_LockSurface(OLXOutput);
	std::size_t destPitch = OLXOutput->pitch;
	std::size_t destBpp = OLXOutput->format->BytesPerPixel;
	std::size_t srcPitch = screenPitch;
	SDL_PixelFormat * destFmt = OLXOutput->format;
	Uint32 realPal[256];
	for( int f=0; f<256; f++ )
		realPal[f] = SDL_MapRGB(destFmt, pal.entries[f].r<<2, pal.entries[f].g<<2, pal.entries[f].b<<2);
	for(int y = 0; y < 200; ++y)
	{
		PalIdx* src = screenPixels + y*srcPitch;
		Uint8* dest = (Uint8*)OLXOutput->pixels + y*destPitch*2;
		Uint8* dest1 = (Uint8*)OLXOutput->pixels + y*destPitch*2+destPitch;
		for(int x = 0; x < 320; ++x)
		{
			Uint32 color = realPal[src[x]];
			memcpy( dest + x*destBpp*2, &color, destBpp );
			memcpy( dest + x*destBpp*2+destBpp, &color, destBpp );
			memcpy( dest1 + x*destBpp*2, &color, destBpp );
			memcpy( dest1 + x*destBpp*2+destBpp, &color, destBpp );
		}
	};
	SDL_UnlockSurface(OLXOutput);
	
	//pal.activate();
	//SDL_BlitSurface( screen, NULL, OLXOutput, NULL );	// Does not work! but should be faster
}

	bool Gfx::testKeyOnce(Uint32 key)
	{
		//bool state = dosKeys[key];
		//dosKeys[key] = false;
		//return state;
		bool state = getKey( key / MAX_KEYS, key % MAX_KEYS ) && getKeyChanged( key / MAX_KEYS, key % MAX_KEYS );
		if( state )	
		{
			getKeyChanged( key / MAX_KEYS, key % MAX_KEYS ) = false;
			//printf("Gfx::testKeyOnce() worm %u key %u state %i\n", key/MAX_KEYS, key%MAX_KEYS, state );
		}
		return state;
	}
	
	bool Gfx::testKey(Uint32 key)
	{
		//return dosKeys[key];
		bool state = getKey( key / MAX_KEYS, key % MAX_KEYS );
		//if( state )	
		//	printf("Gfx::testKey() worm %u key %u state %i\n", key/MAX_KEYS, key%MAX_KEYS, state );
		return state;
	}
	
	void Gfx::releaseKey(Uint32 key)
	{
		//dosKeys[key] = false;
		getKeyChanged( key / MAX_KEYS, key % MAX_KEYS ) = false;
	}
	
	void Gfx::pressKey(Uint32 key)
	{
		// Called only from bot AI
		//dosKeys[key] = true;
		//getKey( key / MAX_KEYS, key % MAX_KEYS ) = true;
	}
	
	void Gfx::setKey(Uint32 key, bool state)
	{
		// Called only from bot AI
		//dosKeys[key] = state;
		//getKey( key / MAX_KEYS, key % MAX_KEYS ) = state;
	}
	
	void Gfx::toggleKey(Uint32 key)
	{
		// Called only from bot AI
		//dosKeys[key] = !dosKeys[key];
		//getKey( key / MAX_KEYS, key % MAX_KEYS ) = ! getKey( key / MAX_KEYS, key % MAX_KEYS );
	}
	
	bool Gfx::testSDLKeyOnce(SDLKey key)
	{
		// Called only from menus
		//Uint32 k = SDLToDOSKey(key);
		//return k ? testKeyOnce(k) : false;
		int keynum = -1;
		if( key == SDLK_UP ) keynum = 0;
		if( key == SDLK_DOWN ) keynum = 1;
		if( key == SDLK_LEFT ) keynum = 2;
		if( key == SDLK_RIGHT ) keynum = 3;
		if( key == SDLK_RETURN ) keynum = 4;
		if( keynum != -1 )
		{
			bool state = getKey( 0, keynum );	// Only first player may change menu items
			getKey( 0, keynum ) = false;
			return state;
		}
		return false;
	}
	
	bool Gfx::testSDLKey(SDLKey key)
	{
		// Called only from menus
		//Uint32 k = SDLToDOSKey(key);
		//return k ? testKey(k) : false;
		int keynum = -1;
		if( key == SDLK_UP ) keynum = 0;
		if( key == SDLK_DOWN ) keynum = 1;
		if( key == SDLK_LEFT ) keynum = 2;
		if( key == SDLK_RIGHT ) keynum = 3;
		if( key == SDLK_RETURN ) keynum = 4;
		if( keynum != -1 )
			return getKey( 0, keynum );	// Only first player may change menu items
		return false;
	}
	
	void Gfx::releaseSDLKey(SDLKey key)
	{
		// Called only from menus
		//Uint32 k = SDLToDOSKey(key);
		//if(k)
		//	dosKeys[k] = false;
		int keynum = -1;
		if( key == SDLK_UP ) keynum = 0;
		if( key == SDLK_DOWN ) keynum = 1;
		if( key == SDLK_LEFT ) keynum = 2;
		if( key == SDLK_RIGHT ) keynum = 3;
		if( key == SDLK_RETURN ) keynum = 4;
		if( keynum != -1 )
			getKey( 0, keynum ) = false;	// Only first player may change menu items
	}
