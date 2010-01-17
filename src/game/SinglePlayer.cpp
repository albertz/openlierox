/*
 *  SinglePlayer.cpp
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 15.01.10.
 *  code under LGPL
 *
 */

#include "SinglePlayer.h"
#include "Options.h"
#include "ConfigHandler.h"
#include "game/Mod.h"
#include "game/Level.h"
#include "GfxPrimitives.h"
#include "DeprecatedGUI/Menu.h"
#include "CClient.h"
#include "CServer.h"
#include "ProfileSystem.h"
#include "CWormHuman.h"

SinglePlayerGame singlePlayerGame;

int SinglePlayerGame::maxAllowedLevelForCurrentGame() {
	int level = 1;
	std::map<std::string,int>::iterator f = tLXOptions->localplayLevels.find(currentGame);
	if(f != tLXOptions->localplayLevels.end())
		level = MAX(f->second, 1);
	return level;
}

void SinglePlayerGame::setGame(const std::string& game) {
	if(!stringcaseequal(currentGame, game)) {
		currentGame = game;		
		setLevel(maxAllowedLevelForCurrentGame());
	}
}

static bool gameLevelExists(const std::string& game, int levelNr) {
	std::string level;
	if(!ReadString("games/games.cfg", game, "Level" + itoa(levelNr), level, "") || Trimmed(level) == "")
		return false;
	else
		return true;
}

static bool gameExists(const std::string& game) {
	return gameLevelExists(game, 1);
}

bool SinglePlayerGame::setLevel(int levelNr) {
	currentGameValid = false;
	image = NULL;
	levelInfo = LevelInfo();
	modInfo = ModInfo();
	description = "";
	
	if(!gameExists(currentGame)) {
		description = "<font color=red>Game <b>" + currentGame + "</b> is invalid</font>";
		errors << "SinglePlayerGame::setLevel: game " << currentGame << " unknown" << endl;
		return false;
	}
	
	currentLevel = levelNr;
	if(currentLevel <= 0) currentLevel = 1;
	
	if(!gameLevelExists(currentGame, currentLevel)) {
		warnings << "SinglePlayerGame::setLevel: game " << currentGame << " doesnt have level " << currentLevel << endl;
		do { currentLevel--; }
		while(currentLevel > 0 && !gameLevelExists(currentGame, currentLevel));
		if(currentLevel <= 0) {
			errors << "SinglePlayerGame::setLevel: woot" << endl;
			return false;
		}
	}
	
	std::string level, mod, img;
	ReadString("games/games.cfg", currentGame, "Level" + itoa(currentLevel), level, ""); TrimSpaces(level);
	ReadString("games/games.cfg", currentGame, "Mod" + itoa(currentLevel), mod, "Classic"); TrimSpaces(mod);
	ReadString("games/games.cfg", currentGame, "Image" + itoa(currentLevel), img, ""); TrimSpaces(img);
	ReadString("games/games.cfg", currentGame, "Desc" + itoa(currentLevel), description, ""); TrimSpaces(description);
	
	level = "../games/" + currentGame + "/" + level; // it's relative to levels
	levelInfo = infoForLevel(level);
	if(!levelInfo.valid) {
		description = "<font color=red>Problem while loading level " + level + "</font>";
		errors << "SinglePlayerGame::setLevel: cannot find map " << level << " for game " << currentGame << ":" << currentLevel << endl;
		return false;
	}
	levelInfo.path = level; // to force this uncommon filename
	
	modInfo = infoForMod(mod);
	if(!modInfo.valid) {
		description = "<font color=red>Problem while loading mod " + mod + "</font>";
		errors << "SinglePlayerGame::setLevel: cannot find mod " << mod << " for game " << currentGame << ":" << currentLevel << endl;
		return false;
	}
	
	if(description == "")
		description = "Undescribed level <b>" + itoa(currentLevel) + "</b> of game <b>" + currentGame + "</b>";
	
	image = LoadGameImage("games/" + currentGame + "/" + img);
	if(image.get() == NULL) {
		warnings << "SinglePlayerGame::setLevel: cannot load preview image '" << img << "' for game " << currentGame << ":" << currentLevel << endl;
		image = gfxCreateSurfaceAlpha(200, 50);
		DrawCross(image.get(), 0, 0, 200, 50, Color(255,0,0));
	}

	currentGameValid = true;
	return true;
}

static bool addPlayerToClient() {
	// Initialize has cleaned up all worms, so this is not necessarily needed
	cClient->setNumWorms(0);

	std::string player = tLXOptions->sLastSelectedPlayer;
	profile_t *ply = (player == "") ? FindProfile(player) : NULL;
	if(ply == NULL) {
		for(ply = GetProfiles(); ply; ply = ply->tNext) {
			if(ply->iType == PRF_HUMAN->toInt())
				// ok
				break;
		}
	}
	if(ply == NULL) {
		// there is no single human player
		AddDefaultPlayers();
		// try again
		for(ply = GetProfiles(); ply; ply = ply->tNext) {
			if(ply->iType == PRF_HUMAN->toInt())
				// ok
				break;
		}		
	}
	if(ply == NULL) {
		errors << "addPlayerToClient: this really should never happen, something very messed up happend" << endl;
		return false;
	}

	// this is the current (ugly) way to tell CClient to create a local worm later on with that profile
	// that is done in CClientNetEngine::ParseConnected or updateAddedWorms
	cClient->getLocalWormProfiles()[0] = ply;
	cClient->setNumWorms(1);
	
	return true;
}

static GameOptions::GameInfo oldSettings;

bool SinglePlayerGame::startGame() {
	if(!currentGameValid) {
		errors << "SinglePlayerGame::startGame: cannot start game: current game/level is invalid" << endl;
		return false;
	}
	
	if(bDedicated) {
		errors << "SinglePlayerGame::startGame: cannot do that in dedicated mode" << endl;
		// actually, we could but I really dont see a reason why we would want
		return false;
	}
	
	oldSettings = tLXOptions->tGameInfo;
	tLXOptions->tGameInfo.sMapFile = levelInfo.path;
	tLXOptions->tGameInfo.sMapName = levelInfo.name;
	
	tLX->iGameType = GME_LOCAL;
	
	if(! cClient->Initialize() )
	{
		errors << "Could not initialize client" << endl;
		return false;
	}

	if(!addPlayerToClient()) {
		errors << "SinglePlayerGame::startGame: didn't found human worm" << endl;
		return false;
	}
	
	if(!cServer->StartServer()) {
		errors << "Could not start server" << endl;
		return false;
	}
	
	tLXOptions->tGameInfo.features[FT_NewNetEngine] = false;
	tLXOptions->tGameInfo.sModDir = modInfo.path;
	tLXOptions->tGameInfo.sModName = modInfo.name;
	
	tLXOptions->tGameInfo.gameMode = this;
	
	levelSucceeded = false;
	return true;
}

void SinglePlayerGame::setLevelSucceeded() {
	levelSucceeded = true;

	if(gameLevelExists(currentGame, currentLevel + 1)) {
		setLevel(currentLevel + 1);
		tLXOptions->localplayLevels[currentGame] = MAX((int)tLXOptions->localplayLevels[currentGame], (int)currentLevel);
	}
}

void SinglePlayerGame::Simulate() {
	if(levelSucceeded)
		cServer->RecheckGame();
}

bool SinglePlayerGame::CheckGameOver() {
	return levelSucceeded;
}

int SinglePlayerGame::Winner() {
	for(int i = 0; i < cClient->getNumWorms(); ++i)
		if(dynamic_cast<CWormHumanInputHandler*>( cClient->getWorm(i)->getOwner() ) != NULL)
			return cClient->getWorm(i)->getID();
	return -1;
}

void SinglePlayerGame::GameOver() {
	tLXOptions->tGameInfo = oldSettings;	
}

