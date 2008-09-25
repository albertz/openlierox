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

// TODO: remove this here! i wonder that we didn't get trouble because of this
typedef unsigned char byte;

//#include "CViewport.h"
//#include "CMap.h"
//#include "Timer.h"

#include "CVec.h"
#include "SmartPointer.h"


#ifndef DEDICATED_ONLY
// we are using SDL_mixer at the moment
#include <SDL_mixer.h>

#	if !defined(SDL_MIXER_MAJOR_VERSION) || (SDL_MIXER_MAJOR_VERSION < 1) || !defined(SDL_MIXER_MINOR_VERSION) || (SDL_MIXER_MINOR_VERSION < 2) || !defined(SDL_MIXER_PATCHLEVEL) || (SDL_MIXER_PATCHLEVEL < 8)
// <=SDL_mixer-1.2.7 does not restart correctly 
#		define SDLMIXER_WORKAROUND_RESTART	1
#	else
#		define SDLMIXER_WORKAROUND_RESTART	0
#	endif
#endif //DEDICATED_ONLY

#define MUSIC_REPEAT_INFINITE -1

#define SND_CMD MUS_CMD
#define SND_WAV MUS_WAV
#define SND_MOD MUS_MOD
#define SND_MID MUS_MID
#define SND_OGG MUS_OGG
#define SND_MP3 MUS_MP3

// TODO: use DECLARE_INTERN_DATA here to avoid SDL_mixer.h in this file
// this typedef can be replaced if another sound system is wanted
// also, all *Sound* functions need to be recoded then
// for using this, handle with pointers of it
struct SoundSample {
#ifndef DEDICATED_ONLY	
	Mix_Chunk* sample;
#endif	//DEDICATED_ONLY
	int maxsimulplays;
// TODO: and other stuff
};

// TODO: use DECLARE_INTERN_DATA here to avoid SDL_mixer.h in this file
// Music
struct SoundMusic {
#ifndef DEDICATED_ONLY	
	Mix_Music *sndMusic;
#endif //DEDICATED_ONLY
};

// General sounds
typedef struct {
	SmartPointer<SoundSample> smpClick;
	SmartPointer<SoundSample> smpChat;
	SmartPointer<SoundSample> smpNotify;
} sfxgen_t;


// Game sounds
typedef struct {
	SmartPointer<SoundSample> smpNinja;
	SmartPointer<SoundSample> smpPickup;
	SmartPointer<SoundSample> smpBump;
	SmartPointer<SoundSample> smpDeath[3];
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


SmartPointer<SoundSample> LoadSample(const std::string& _filename, int maxplaying);

// Routines
bool	InitSoundSystem(int rate, int channels, int buffers);
bool	StartSoundSystem();
bool	StopSoundSystem();
bool	SetSoundVolume(int vol);
int		GetSoundVolume(void);
bool	QuitSoundSystem();
//SoundSample * LoadSoundSample(const std::string& filename, int maxsimulplays); // Not cached - used internally only
//bool	FreeSoundSample(SoundSample* sample);	// Should be avoided with cache system
bool	PlaySoundSample(SoundSample* sample);
inline bool	PlaySoundSample(const SmartPointer<SoundSample> & sample) {
	return PlaySoundSample(sample.get());
};


class CWorm;

int		LoadSounds(void);
void	ShutdownSounds(void);
void	StartSound(SoundSample* smp, CVec pos, int local, int volume, CWorm *me);
inline void StartSound(const SmartPointer<SoundSample> & smp, CVec pos, int local, int volume, CWorm *me) {
	StartSound(smp.get(), pos, local, volume, me);
};

// TODO: don't make them global
extern float fCurSongStart;
extern float fTimePaused;
extern bool	 bSongStopped;
extern byte iMusicVolume;
extern bool	 bSongFinished;


// Music
SoundMusic		*LoadMusic(const std::string& file);
void			PlayMusicAsync(const std::string& file);
bool			IsSongLoading();
void			FreeMusic(SoundMusic *music);
void			PlayMusic(SoundMusic *music, int number_of_repeats=1);
void			PauseMusic(void);
void			ResumeMusic(void);
void			RewindMusic(void);
void			SetMusicPosition(double pos);
void			StopMusic(void);
bool			PlayingMusic(void);
bool			PausedMusic(void);
int				GetMusicType(SoundMusic *music = NULL);
float			GetCurrentMusicTime(void);
bool			GetSongStopped(void);
bool			GetSongFinished(void);
id3v1_t			GetMP3Info(const std::string& file);

void			SetMusicVolume(byte vol);
byte			GetMusicVolume(void);

void			InitializeMusic(void);
void			ShutdownMusic(void);





// Globals
extern	sfxgame_t	sfxGame;
extern	sfxgen_t	sfxGeneral;





#endif  //  __SOUNDS_H__
