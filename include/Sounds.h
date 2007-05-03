/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Sounds header file
// Created 29/7/02
// Jason Boettcher


#ifndef __SOUNDS_H__
#define __SOUNDS_H__

typedef unsigned char byte;

#include "CViewport.h"
#include "CMap.h"
#include "Timer.h"

// we are using SDL_mixer at the moment
#include <SDL/SDL_mixer.h>

#define MUSIC_REPEAT_INFINITE -1

#define SND_CMD MUS_CMD
#define SND_WAV MUS_WAV
#define SND_MOD MUS_MOD
#define SND_MID MUS_MID
#define SND_OGG MUS_OGG
#define SND_MP3 MUS_MP3

// this typedef can be replaced if another sound system is wanted
// also, all *Sound* functions need to be recoded then
// for using this, handle with pointers of it
struct SoundSample {
	Mix_Chunk* sample;
	int maxsimulplays;
// TODO: and other stuff
};

// Music
struct SoundMusic {
	Mix_Music *sndMusic;
};

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

// ID3 tag format
// HINT: this are fixed widths, which are directly read out of the file
// TODO: ID3v2 support
typedef struct id3v1_s {
	char		name[30];
	char		interpreter[30];
	char		album[30];
	char		year[4];
	char		comment[30];
	byte		genre;
} id3v1_t;


SoundSample*	LoadSample(const UCString& _filename, int maxplaying);

// Routines
bool	InitSoundSystem(int rate, int channels, int buffers);
bool	StartSoundSystem();
bool	StopSoundSystem();
bool	SetSoundVolume(int vol);
int		GetSoundVolume(void);
bool	QuitSoundSystem();
SoundSample* LoadSoundSample(const UCString& filename, int maxsimulplays);
bool	FreeSoundSample(SoundSample* sample);
bool	PlaySoundSample(SoundSample* sample);

int		LoadSounds(void);
void	ShutdownSounds(void);
void	StartSound(SoundSample* smp, CVec pos, int local, int volume, CWorm *me);

extern float fCurSongStart;
extern float fTimePaused;
extern bool	 bSongStopped;
extern byte iMusicVolume;
extern bool	 bSongFinished;


// Music
void			MusicFinishedHook(void);
SoundMusic		*LoadMusic(const UCString& file);
void			PlayMusicAsync(const UCString& file);
bool			IsSongLoading();
void			FreeMusic(SoundMusic *music);
void			PlayMusic(SoundMusic *music, int number_of_repeats=1);
inline void		PauseMusic(void) {Mix_PauseMusic(); fTimePaused = GetMilliSeconds(); bSongStopped = false;}
inline void		ResumeMusic(void) {Mix_ResumeMusic();fCurSongStart += GetMilliSeconds()-fTimePaused; fTimePaused = 0; bSongStopped = false;}
inline void		RewindMusic(void) {Mix_RewindMusic();fCurSongStart = GetMilliSeconds();fTimePaused = 0;}
inline void		SetMusicPosition(double pos)  {Mix_RewindMusic(); Mix_SetMusicPosition(pos); }
void			StopMusic(void);
inline bool		PlayingMusic(void) {return Mix_PlayingMusic() != 0; }
inline bool		PausedMusic(void) {return Mix_PausedMusic() != 0; }
inline int		GetMusicType(SoundMusic *music = NULL) {if (music) {return Mix_GetMusicType(music->sndMusic);} else {return Mix_GetMusicType(NULL);} }
float			GetCurrentMusicTime(void);
inline bool		GetSongStopped(void) {return bSongStopped; }
inline bool		GetSongFinished(void) { bool tmp = bSongFinished; bSongFinished = false; return tmp; }
id3v1_t			GetMP3Info(const UCString& file);

void			SetMusicVolume(byte vol);
inline byte		GetMusicVolume(void) { return iMusicVolume; }

void			InitializeMusic(void);
void			ShutdownMusic(void);





// Globals
extern	sfxgame_t	sfxGame;
extern	sfxgen_t	sfxGeneral;





#endif  //  __SOUNDS_H__
