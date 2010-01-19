/*
 *  Sounds.h
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 19.01.10.
 *  code under LGPL
 *
 */


#ifndef __OLX_GAME_SOUNDS_H__
#define __OLX_GAME_SOUNDS_H__

#include "SmartPointer.h"

struct SoundSample;

// Game sounds
struct sfxgame_t {
	SmartPointer<SoundSample> smpNinja;
	SmartPointer<SoundSample> smpPickup;
	SmartPointer<SoundSample> smpBump;
	SmartPointer<SoundSample> smpDeath[3];
	SmartPointer<SoundSample> smpTeamScore;
	
	std::map< std::string, SmartPointer<SoundSample> > gameSounds;
};

extern	sfxgame_t	sfxGame;

void LoadSounds_Game();
void ShutdownSounds_Game();

void PlayGameSound(const std::string& name);


#endif
