/////////////////////////////////////////
//
//             Liero Xtreme
//
//     Copyright Auxiliary Software 2002
//
//
/////////////////////////////////////////


// Sounds header file
// Created 29/7/02
// Jason Boettcher


#ifndef __SOUNDS_H__
#define __SOUNDS_H__

#include "CViewport.h"
#include "CMap.h"

// we are using SDL_mixer at the moment
#include <SDL/SDL_mixer.h>

// this typedef can be replaced if another sound system is wanted
// also, all *Sound* functions need to be recoded then
// for using this, handle with pointers of it
struct SoundSample {
	Mix_Chunk* sample;
	int maxsimulplays;
// TODO: and other stuff
};

// TODO: this has to be implemented later
// typedef Mix_Music SoundMusic;


// General sounds
typedef struct {
	SoundSample*		smpClick;
	SoundSample*		smpChat;
} sfxgen_t;


// Game sounds
typedef struct {
	SoundSample*		smpNinja;
	SoundSample*		smpPickup;
	SoundSample*		smpBump;
	SoundSample*		smpDeath[3];
} sfxgame_t;


// Routines
int		InitSoundSystem(int rate, int channels, int buffers);
int		StartSoundSystem();
int		StopSoundSystem();
int		SetSoundVolume(int vol);
int		QuitSoundSystem();
SoundSample* LoadSoundSample(char* filename, int maxsimulplays);
int		FreeSoundSample(SoundSample* sample);
int		PlaySoundSample(SoundSample* sample);
// TODO: the music part, for example:
// int	LoadSoundMusic();
// int	PlaySoundMusic();
// etc.

int		LoadSounds(void);
void	ShutdownSounds(void);
void	StartSound(SoundSample* smp, CVec pos, int local, int volume, CWorm *me);


// Globals
extern	sfxgame_t	sfxGame;
extern	sfxgen_t	sfxGeneral;





#endif  //  __SOUNDS_H__
