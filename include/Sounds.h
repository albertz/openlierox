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


// General sounds
typedef struct {
	HSAMPLE		smpClick;
	HSAMPLE		smpChat;
} sfxgen_t;


// Game sounds
typedef struct {
	HSAMPLE		smpNinja;
	HSAMPLE		smpPickup;
	HSAMPLE		smpBump;
	HSAMPLE		smpDeath[3];
} sfxgame_t;


// Routines
int		LoadSounds(void);
void	ShutdownSounds(void);
void	StartSound(HSAMPLE smp, CVec pos, int local, int volume, CWorm *me);


// Globals
extern	sfxgame_t	sfxGame;
extern	sfxgen_t	sfxGeneral;





#endif  //  __SOUNDS_H__
