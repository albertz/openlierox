/*
 *  Sounds.cpp
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 19.01.10.
 *  code under LGPL
 *
 */

#include "sound/SoundsBase.h"
#include "game/Sounds.h"

sfxgame_t	sfxGame;

void LoadSounds_Game() {
	sfxGame.smpNinja = LoadSample("data/sounds/throw.wav",4);
	sfxGame.smpPickup = LoadSample("data/sounds/pickup.wav",2);
	sfxGame.smpBump = LoadSample("data/sounds/bump.wav", 2);
	sfxGame.smpDeath[0] = LoadSample("data/sounds/death1.wav", 2);
	sfxGame.smpDeath[1] = LoadSample("data/sounds/death2.wav", 2);
	sfxGame.smpDeath[2] = LoadSample("data/sounds/death3.wav", 2);	
	
	sfxGame.smpTeamScore = LoadSample("data/sounds/teamscore.wav",2);
	if( sfxGame.smpTeamScore.get() == NULL ) {
		notes << "LoadSounds: cannot load teamscore.wav" << endl;
		sfxGame.smpTeamScore = sfxGeneral.smpNotify;
	}
	
}

