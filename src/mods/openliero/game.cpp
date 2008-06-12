#include "game.hpp"
#include "reader.hpp"
#include "viewport.hpp"
#include "worm.hpp"
#include "filesystem.hpp"
#include "gfx.hpp"
#include "sfx.hpp"
#include "weapsel.hpp"
#include "constants.hpp"
//#include "text.hpp" // TEMP

#include <iostream>

Game game;

const int stoneTab[3][4] =
{
	{98, 60, 61, 62},
	{63, 75, 85, 86},
	{89, 90, 97, 96}
};

void Texts::loadFromEXE()
{
	FILE* exe = openLieroEXE();
	
	
	random = readPascalStringAt(exe, 0xD6E3);
	random2 = readPascalStringAt(exe, 0xD413);
	regenLevel = readPascalStringAt(exe, 0xD41A);
	reloadLevel = readPascalStringAt(exe, 0xD42D);
	
	copyright1 = readPascalStringAt(exe, 0xFB60);
	copyright2 = readPascalStringAt(exe, 0xE693);
	saveoptions = readPascalStringAt(exe, 0xE6BB);
	loadoptions = readPascalStringAt(exe, 0xE6CC);
	curOptNoFile = readPascalStringAt(exe, 0xE6DD);
	curOpt = readPascalStringAt(exe, 0xE6FA);
	
	fseek(exe, 0x1B2BA, SEEK_SET);
	for(int i = 0; i < 4; ++i)
	{
		gameModes[i] = readPascalString(exe, 17);
	}
	
	gameModeSpec[0] = readPascalStringAt(exe, 0xD3EC);
	gameModeSpec[1] = readPascalStringAt(exe, 0xD3F2);
	gameModeSpec[2] = readPascalStringAt(exe, 0xD3FF);
	
	onoff[0] = readPascalStringAt(exe, 0x1AE84);
	onoff[1] = readPascalStringAt(exe, 0x1AE88);
	
	controllers[0] = readPascalStringAt(exe, 0x1B204);
	controllers[1] = readPascalStringAt(exe, 0x1B20A);
	
	fseek(exe, 0x1B2FE, SEEK_SET);
	for(int i = 0; i < 3; ++i)
	{
		weapStates[i] = readPascalString(exe, 13);
	}
		
	fseek(exe, 0x209A6, SEEK_SET);
	for(int i = 1; i < 177; ++i) // First key starts at 1
	{
		keyNames[i] = readPascalString(exe, 13);
	}
	
	selWeap = readPascalStringAt(exe, 0xA9C0);
	levelRandom = readPascalStringAt(exe, 0xA9D5);
	levelIs1 = readPascalStringAt(exe, 0xA9E3);
	levelIs2 = readPascalStringAt(exe, 0xA9EC);
	randomize = readPascalStringAt(exe, 0xA9F4);
	done = readPascalStringAt(exe, 0xA9EE);
	
	reloading = readPascalStringAt(exe, 0x7583);
	pressFire = readPascalStringAt(exe, 0x7590);
	
	kills = readPascalStringAt(exe, 0x75A4);
	lives = readPascalStringAt(exe, 0x75AC);
	
	selLevel = readPascalStringAt(exe, 0xD6F2);
	
	weapon = readPascalStringAt(exe, 0xD700);
	availability = readPascalStringAt(exe, 0xD707);
	noWeaps = readPascalStringAt(exe, 0xD714);
	
	fseek(exe, 0xFC5B, SEEK_SET);
	copyrightBarFormat = readUint8(exe);
}

Game::~Game()
{
}

void Game::initGame()
{
	clearWorms();
	
	bonuses.clear();
	wobjects.clear();
	sobjects.clear();
	bobjects.clear();
	nobjects.clear();
	
	/*
	Worm worm1(&game.settings.wormSettings[0], 0, 19);
	Worm worm2(&game.settings.wormSettings[1], 1, 20);
	
	addViewport(new Viewport(Rect(0, 0, 158, 158), worm1, 0, 504, 350));
	addViewport(new Viewport(Rect(160, 0, 158+160, 158), worm2, 218, 504, 350));
	
	addWorm(worm1);
	addWorm(worm2);
	*/
	
	// TODO: Move as much of this as possible into the Worm ctor
	for(std::size_t i = 0; i < worms.size(); ++i)
	{
		Worm& w = worms[i];
		w.makeSightGreen = false;
		w.lives = game.settings.lives;
		w.ready = true;
		w.movable = true;
		
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

		w.health = w.settings.health;
		w.visible = false;
		w.killedTimer = 150;
		
		w.currentWeapon = 1; // This is later changed to 0, why is it here?

/* Done in WormWeapon ctor
		for(int i = 0; i < game.settings.selectableWeapons; ++i)
		{
			w.weapons[i].available = true;
			w.weapons[i].delayLeft = 0;
			w.weapons[i].ammo = 0;
			
		}*/
	}
	
	gotChanged = false;
	lastKilled = -1;
}

void Game::processViewports()
{
	for(std::size_t i = 0; i < worms.size(); ++i)
	{
		worms[i].viewport.process();
	}
}

void Game::drawViewports()
{
	gfx.clear();
	for(std::size_t i = 0; i < worms.size(); ++i)
	{
		worms[i].viewport.draw();
	}
}

void Game::clearWorms()
{
	worms.clear();
}

void Game::resetWorms()
{
	for(std::size_t i = 0; i < worms.size(); ++i)
	{
		Worm& w = worms[i];
		w.health = w.settings.health;
		w.lives = settings.lives; // Not in the original!
		w.kills = 0;
		w.visible = false;
		w.killedTimer = 150;
		
		w.currentWeapon = 1;
	}
}

void Game::addWorm(const Worm & worm)
{
	worms.push_back(worm);
}

void Game::loadMaterials()
{
	FILE* exe = openLieroEXE();
	
	std::fseek(exe, 0x01C2E0, SEEK_SET);
	
	for(int i = 0; i < 256; ++i)
	{
		materials[i].flags = 0;
	}
	
	unsigned char bits[32];
	
	for(int i = 0; i < 5; ++i)
	{
		fread(bits, 1, 32, exe);
		
		for(int j = 0; j < 256; ++j)
		{
			int bit = ((bits[j >> 3] >> (j & 7)) & 1);
			materials[j].flags |= bit << i;
		}
	}
	
	std::fseek(exe, 0x01AEA8, SEEK_SET);
	
	fread(bits, 1, 32, exe);
	
	for(int j = 0; j < 256; ++j)
	{
		int bit = ((bits[j >> 3] >> (j & 7)) & 1);
		materials[j].flags |= bit << 5;
	}
}

struct Read32
{
	static inline int run(FILE* f)
	{
		return readSint32(f);
	}
};

struct Read16
{
	static inline int run(FILE* f)
	{
		return readSint16(f);
	}
};

struct Read8
{
	static inline int run(FILE* f)
	{
		return readUint8(f);
	}
};

struct ReadBool
{
	static inline bool run(FILE* f)
	{
		return readUint8(f) != 0;
	}
};

template<typename T>
struct Dec
{
	static inline int run(FILE* f)
	{
		return T::run(f) - 1;
	}
};

template<typename Reader, typename T, int N, typename U>
inline void readMembers(FILE* f, T(&arr)[N], U (T::*mem))
{
	for(int i = 0; i < N; ++i)
	{
		(arr[i].*mem) = Reader::run(f);
	}
}

void Game::loadWeapons()
{
	FILE* exe = openLieroEXE();
	
	fseek(exe, 112806, SEEK_SET);
	
	readMembers<Read8>(exe, weapons, &Weapon::detectDistance);
	readMembers<ReadBool>(exe, weapons, &Weapon::affectByWorm);
	readMembers<Read8>(exe, weapons, &Weapon::blowAway);
	
	for(int i = 0; i < 40; ++i)
	{
		weapOrder[i + 1] = readUint8(exe) - 1;
	}
	
	readMembers<Read16>(exe, weapons, &Weapon::gravity);
	readMembers<ReadBool>(exe, weapons, &Weapon::shadow);
	readMembers<ReadBool>(exe, weapons, &Weapon::laserSight);
	readMembers<Dec<Read8> >(exe, weapons, &Weapon::launchSound);
	readMembers<ReadBool>(exe, weapons, &Weapon::loopSound);
	readMembers<Dec<Read8> >(exe, weapons, &Weapon::exploSound);
	readMembers<Read16>(exe, weapons, &Weapon::speed);
	readMembers<Read16>(exe, weapons, &Weapon::addSpeed);
	readMembers<Read16>(exe, weapons, &Weapon::distribution);
	readMembers<Read8>(exe, weapons, &Weapon::parts);
	readMembers<Read8>(exe, weapons, &Weapon::recoil);
	readMembers<Read16>(exe, weapons, &Weapon::multSpeed);
	readMembers<Read16>(exe, weapons, &Weapon::delay);
	readMembers<Read16>(exe, weapons, &Weapon::loadingTime);
	readMembers<Read8>(exe, weapons, &Weapon::ammo);
	readMembers<Dec<Read8> >(exe, weapons, &Weapon::createOnExp);
	readMembers<Dec<Read8> >(exe, weapons, &Weapon::dirtEffect);
	readMembers<Read8>(exe, weapons, &Weapon::leaveShells);
	readMembers<Read8>(exe, weapons, &Weapon::leaveShellDelay);
	readMembers<ReadBool>(exe, weapons, &Weapon::playReloadSound);
	readMembers<ReadBool>(exe, weapons, &Weapon::wormExplode);
	readMembers<ReadBool>(exe, weapons, &Weapon::explGround);
	readMembers<ReadBool>(exe, weapons, &Weapon::wormCollide);
	readMembers<Read8>(exe, weapons, &Weapon::fireCone);
	readMembers<ReadBool>(exe, weapons, &Weapon::collideWithObjects);
	readMembers<ReadBool>(exe, weapons, &Weapon::affectByExplosions);
	readMembers<Read8>(exe, weapons, &Weapon::bounce);
	readMembers<Read16>(exe, weapons, &Weapon::timeToExplo);
	readMembers<Read16>(exe, weapons, &Weapon::timeToExploV);
	readMembers<Read8>(exe, weapons, &Weapon::hitDamage);
	readMembers<Read8>(exe, weapons, &Weapon::bloodOnHit);
	readMembers<Read16>(exe, weapons, &Weapon::startFrame);
	readMembers<Read8>(exe, weapons, &Weapon::numFrames);
	readMembers<ReadBool>(exe, weapons, &Weapon::loopAnim);
	readMembers<Read8>(exe, weapons, &Weapon::shotType);
	readMembers<Read8>(exe, weapons, &Weapon::colourBullets);
	readMembers<Read8>(exe, weapons, &Weapon::splinterAmount);
	readMembers<Read8>(exe, weapons, &Weapon::splinterColour);
	readMembers<Dec<Read8> >(exe, weapons, &Weapon::splinterType);
	readMembers<Read8>(exe, weapons, &Weapon::splinterScatter);
	readMembers<Dec<Read8> >(exe, weapons, &Weapon::objTrailType);
	readMembers<Read8>(exe, weapons, &Weapon::objTrailDelay);
	readMembers<Read8>(exe, weapons, &Weapon::partTrailType);
	readMembers<Dec<Read8> >(exe, weapons, &Weapon::partTrailObj);
	readMembers<Read8>(exe, weapons, &Weapon::partTrailDelay);
	
	fseek(exe, 0x1B676, SEEK_SET);
	for(int i = 0; i < 40; ++i)
	{
		weapons[i].name = readPascalString(exe, 14);
		weapons[i].id = i;
	}
	
	// Special objects
	fseek(exe, 115218, SEEK_SET);
	readMembers<Dec<Read8> >(exe, sobjectTypes, &SObjectType::startSound);
	//fseek(exe, 115232, SEEK_SET);
	readMembers<Read8>(exe, sobjectTypes, &SObjectType::numSounds);
	//fseek(exe, 115246, SEEK_SET);
	readMembers<Read8>(exe, sobjectTypes, &SObjectType::animDelay);
	//fseek(exe, 115260, SEEK_SET);
	readMembers<Read8>(exe, sobjectTypes, &SObjectType::startFrame);
	//fseek(exe, 115274, SEEK_SET);
	readMembers<Read8>(exe, sobjectTypes, &SObjectType::numFrames);
	//fseek(exe, 115288, SEEK_SET);
	readMembers<Read8>(exe, sobjectTypes, &SObjectType::detectRange);
	//fseek(exe, 115302, SEEK_SET);
	readMembers<Read8>(exe, sobjectTypes, &SObjectType::damage);
	//fseek(exe, 0x1C274, SEEK_SET);
	readMembers<Read32>(exe, sobjectTypes, &SObjectType::blowAway); // blowAway has 13 slots, not 14. The last value will overlap with shadow.
	fseek(exe, 115368, SEEK_SET);
	readMembers<ReadBool>(exe, sobjectTypes, &SObjectType::shadow);
	//fseek(exe, 115382, SEEK_SET);
	readMembers<Read8>(exe, sobjectTypes, &SObjectType::shake);
	//fseek(exe, 115396, SEEK_SET);
	readMembers<Read8>(exe, sobjectTypes, &SObjectType::flash);
	//fseek(exe, 115410, SEEK_SET); // Was 115409
	readMembers<Dec<Read8> >(exe, sobjectTypes, &SObjectType::dirtEffect);
	
	for(int i = 0; i < 14; ++i) // TODO: Unhardcode
	{
		sobjectTypes[i].id = i;
	}
	
	fseek(exe, 111430, SEEK_SET);
	
	readMembers<Read8>(exe, nobjectTypes, &NObjectType::detectDistance);
	readMembers<Read16>(exe, nobjectTypes, &NObjectType::gravity);
	readMembers<Read16>(exe, nobjectTypes, &NObjectType::speed);
	readMembers<Read16>(exe, nobjectTypes, &NObjectType::speedV);
	readMembers<Read16>(exe, nobjectTypes, &NObjectType::distribution);
	readMembers<Read8>(exe, nobjectTypes, &NObjectType::blowAway);
	readMembers<Read8>(exe, nobjectTypes, &NObjectType::bounce);
	readMembers<Read8>(exe, nobjectTypes, &NObjectType::hitDamage);
	readMembers<ReadBool>(exe, nobjectTypes, &NObjectType::wormExplode);
	readMembers<ReadBool>(exe, nobjectTypes, &NObjectType::explGround);
	readMembers<ReadBool>(exe, nobjectTypes, &NObjectType::wormDestroy);
	readMembers<Read8>(exe, nobjectTypes, &NObjectType::bloodOnHit);
	readMembers<Read8>(exe, nobjectTypes, &NObjectType::startFrame);
	readMembers<Read8>(exe, nobjectTypes, &NObjectType::numFrames);
	readMembers<ReadBool>(exe, nobjectTypes, &NObjectType::drawOnMap);
	readMembers<Read8>(exe, nobjectTypes, &NObjectType::colourBullets);
	readMembers<Dec<Read8> >(exe, nobjectTypes, &NObjectType::createOnExp);
	readMembers<ReadBool>(exe, nobjectTypes, &NObjectType::affectByExplosions);
	readMembers<Dec<Read8> >(exe, nobjectTypes, &NObjectType::dirtEffect);
	readMembers<Read8>(exe, nobjectTypes, &NObjectType::splinterAmount);
	readMembers<Read8>(exe, nobjectTypes, &NObjectType::splinterColour);
	readMembers<Dec<Read8> >(exe, nobjectTypes, &NObjectType::splinterType);
	readMembers<ReadBool>(exe, nobjectTypes, &NObjectType::bloodTrail);
	readMembers<Read8>(exe, nobjectTypes, &NObjectType::bloodTrailDelay);
	readMembers<Dec<Read8> >(exe, nobjectTypes, &NObjectType::leaveObj);
	readMembers<Read8>(exe, nobjectTypes, &NObjectType::leaveObjDelay);
	readMembers<Read16>(exe, nobjectTypes, &NObjectType::timeToExplo);
	readMembers<Read16>(exe, nobjectTypes, &NObjectType::timeToExploV);
	
	for(int i = 0; i < 24; ++i) // TODO: Unhardcode
	{
		nobjectTypes[i].id = i;
	}
}

void Game::loadTextures()
{
	FILE* exe = openLieroEXE();
	
	fseek(exe, 0x1C208, SEEK_SET);
	readMembers<ReadBool>(exe, textures, &Texture::nDrawBack);
	fseek(exe, 0x1C1EA, SEEK_SET);
	readMembers<Read8>(exe, textures, &Texture::mFrame);
	fseek(exe, 0x1C1F4, SEEK_SET);
	readMembers<Read8>(exe, textures, &Texture::sFrame);
	fseek(exe, 0x1C1FE, SEEK_SET);
	readMembers<Read8>(exe, textures, &Texture::rFrame);
}

void Game::loadOthers()
{
	FILE* exe = openLieroEXE();
	
	fseek(exe, 0x1C1E2, SEEK_SET);
	
	for(int i = 0; i < 2; ++i)
	for(int j = 0; j < 2; ++j)
		bonusRandTimer[j][i] = readUint16(exe);
		
	fseek(exe, 0x1AEEE + 2, SEEK_SET);
	
	for(int i = 0; i < 2; ++i)
	for(int j = 0; j < 7; ++j)
		aiParams.k[i][j] = readUint16(exe);
		
	fseek(exe, 0x1C1E0, SEEK_SET);
	
	for(int i = 0; i < 2; ++i)
		bonusSObjects[i] = readUint8(exe) - 1;
}

void Game::generateLevel()
{
	if(settings.randomLevel)
	{
		level.generateRandom();
	}
	else
	{
		// TODO: Check .LEV as well as .lev
		if(!level.load(joinPath(lieroEXERoot, settings.levelFile + ".lev")))
			level.generateRandom();

	}
	
	oldRandomLevel = settings.randomLevel;
	oldLevelFile = settings.levelFile;
	
	if(settings.shadow)
	{
		level.makeShadow();
	}
}

void Game::saveSettings()
{
	settings.save(joinPath(lieroEXERoot, settingsFile + ".DAT"));
}

bool Game::loadSettings()
{
	return settings.load(joinPath(lieroEXERoot, settingsFile + ".DAT"));
}

void Game::draw()
{
	drawViewports();
}

bool checkBonusSpawnPosition(int x, int y)
{
	Rect rect(x - 2, y - 2, x + 3, y + 3);
	
	rect.intersect(game.level.rect());
	
	for(int cx = rect.x1; cx < rect.x2; ++cx)
	for(int cy = rect.y1; cy < rect.y2; ++cy)
	{
		if(game.materials[game.level.pixel(cx, cy)].dirtRock())
			return false;
	}
	
	return true;
}

void createBonus()
{
	if(int(game.bonuses.size()) >= game.settings.maxBonuses)
		return;
		
	Bonus* bonus = game.bonuses.newObject();
	if(!bonus)
		return;
	
	for(std::size_t i = 0; i < 50000; ++i)
	{
		int ix = game.rand(C[BonusSpawnRectW]);
		int iy = game.rand(C[BonusSpawnRectH]);
		
		if(H[HBonusSpawnRect])
		{
			ix += C[BonusSpawnRectX];
			iy += C[BonusSpawnRectY];
		}
		
		if(checkBonusSpawnPosition(ix, iy))
		{
			int frame;
			
			if(H[HBonusOnlyHealth])
				frame = 1;
			else if(H[HBonusOnlyWeapon])
				frame = 0;
			else
				frame = game.rand(2);
			
			bonus->x = itof(ix);
			bonus->y = itof(iy);
			bonus->velY = 0;
			bonus->frame = frame;
			bonus->timer = game.rand(game.bonusRandTimer[frame][1]) + game.bonusRandTimer[frame][0];
			
			if(frame == 0)
			{
				do
				{
					bonus->weapon = game.rand(40); // TODO: Unhardcode
				}
				while(game.settings.weapTable[bonus->weapon] == 2);
			}
			
			game.sobjectTypes[7].create(ix, iy, 0);
			return;
		}
	} // 234F
	
	game.bonuses.free(bonus);
}

void Game::startGame(bool isStartingGame)
{
	gfx.pal.clear();
	
	if(isStartingGame)
	{
		if(settings.regenerateLevel
		|| settings.randomLevel != oldRandomLevel
		|| settings.levelFile != oldLevelFile)
		{
			generateLevel();
		}
	
		
		initGame();
		
		
		
		for(std::size_t i = 0; i < viewports.size(); ++i)
		{
			worms[i].viewport.x = 0;
			worms[i].viewport.y = 0;
		}

		selectWeapons();
		
		sfx.play(22, 22);

		cycles = 0;
		
		for(int w = 0; w < 40; ++w)
		{
			weapons[w].computedLoadingTime = (settings.loadingTime * weapons[w].loadingTime) / 100;
			if(weapons[w].computedLoadingTime == 0)
				weapons[w].computedLoadingTime = 1;
		}
	}
	
	int fadeAmount = isStartingGame ? 180 : 0;
	bool shutDown = false;
	
	do
	{
		++cycles;
		
		if(!H[HBonusDisable]
		&& settings.maxBonuses > 0
		&& rand(C[BonusDropChance]) == 0)
		{
			createBonus();
		}
			
		for(std::size_t i = 0; i < worms.size(); ++i)
		{
			worms[i].process();
		}
		
		for(std::size_t i = 0; i < worms.size(); ++i)
		{
			worms[i].ninjarope.process(worms[i]);
		}
		
		switch(game.settings.gameMode)
		{
		case Settings::GMGameOfTag:
		{
			bool someInvisible = false;
			for(std::size_t i = 0; i < worms.size(); ++i)
			{
				if(!worms[i].visible)
				{
					someInvisible = true;
					break;
				}
			}
			
			if(!someInvisible
			&& lastKilled
			&& (cycles % 70) == 0
			&& worms[lastKilled].timer < settings.timeToLose)
			{
				++worms[lastKilled].timer;
			}
		}
		break;
		}
		
		processViewports();
		drawViewports();
				
		for(BonusList::iterator i = bonuses.begin(); i != bonuses.end(); ++i)
		{
			i->process();
		}
		
		if((cycles & 1) == 0)
		{
			for(std::size_t i = 0; i < worms.size(); ++i)
			{
				Viewport& v = worms[i].viewport;
				
				bool down = false;
				
				if(worms[i].killedTimer > 16)
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
		
		gfx.flip();
		gfx.process();
		
		if(gfx.screenFlash > 0)
			--gfx.screenFlash;
		
		if(isGameOver())
		{
			gfx.firstMenuItem = 1;
			shutDown = true;
		}
		
		for(std::size_t i = 0; i < worms.size(); ++i)
		{
			if(worms[i].viewport.shake > 0)
				worms[i].viewport.shake -= 4000; // TODO: Read 4000 from exe?
		}
		
		if(gfx.testSDLKeyOnce(SDLK_ESCAPE)
		&& !shutDown)
		{
			gfx.firstMenuItem = 0;
			fadeAmount = 31;
			shutDown = true;
		}

		if(shutDown)
		{
			fadeAmount -= 1;
		}
		else if(!isStartingGame)
		{
			if(fadeAmount < 33)
			{
				fadeAmount += 1;
				if(fadeAmount >= 33)
					fadeAmount = 180;
			}
		}
	}
	while(fadeAmount > 0);
	
	gfx.clearKeys();
}

bool Game::isGameOver()
{
	if(settings.gameMode == Settings::GMKillEmAll)
	{
		for(std::size_t i = 0; i < worms.size(); ++i)
		{
			if(worms[i].lives <= 0)
				return true;
		}
	}
	else if(settings.gameMode == Settings::GMGameOfTag)
	{
		for(std::size_t i = 0; i < worms.size(); ++i)
		{
			if(worms[i].timer >= game.settings.timeToLose)
				return true;
		}
	}
	else if(settings.gameMode == Settings::GMCtF
	|| settings.gameMode == Settings::GMSimpleCtF)
	{
		for(std::size_t i = 0; i < worms.size(); ++i)
		{
			if(worms[i].flags >= game.settings.flagsToWin)
				return true;
		}
	}
	
	return false;
}

bool checkRespawnPosition(int x2, int y2, int oldX, int oldY, int x, int y)
{
	int deltaX = oldX;
	int deltaY = oldY - y;
	int enemyDX = x2 - x;
	int enemyDY = y2 - y;
	
	if((std::abs(deltaX) <= C[WormMinSpawnDistLast] && std::abs(deltaY) <= C[WormMinSpawnDistLast])
	|| (std::abs(enemyDX) <= C[WormMinSpawnDistEnemy] && std::abs(enemyDY) <= C[WormMinSpawnDistEnemy])) // TODO: Read the 160s from the exe
		return false;
		
	int maxX = x + 3;
	int maxY = y + 4;
	int minX = x - 3;
	int minY = y - 4;
	
	if(maxX >= game.level.width) maxX = game.level.width - 1;
	if(maxY >= game.level.height) maxY = game.level.height - 1;
	if(minX < 0) minX = 0;
	if(minY < 0) minY = 0;
	
	for(int i = minX; i != maxX; ++i)
	for(int j = minY; j != maxY; ++j)
	{
		if(game.materials[game.level.pixel(i, j)].rock()) // TODO: The special rock respawn bug is here, consider an option to turn it off
			return false;
	}
	
	return true;
}
