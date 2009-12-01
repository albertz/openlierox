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

#include "CVec.h"
#include "SmartPointer.h"
#include "types.h"


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
// TODO ...
#endif	//DEDICATED_ONLY
	int maxsimulplays;
// TODO: and other stuff
};

// TODO: use DECLARE_INTERN_DATA here to avoid SDL_mixer.h in this file
// Music
struct SoundMusic {
#ifndef DEDICATED_ONLY	
	// TODO ...
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
	SmartPointer<SoundSample> smpTeamScore;	
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
int		GetSoundVolume();
bool	QuitSoundSystem();
//SoundSample * LoadSoundSample(const std::string& filename, int maxsimulplays); // Not cached - used internally only
//bool	FreeSoundSample(SoundSample* sample);	// Should be avoided with cache system
bool	PlaySoundSample(SoundSample* sample);
inline bool	PlaySoundSample(const SmartPointer<SoundSample> & sample) {
	return PlaySoundSample(sample.get());
}


class CWorm;

bool	LoadSounds();
void	ShutdownSounds();
void	StartSound(SoundSample* smp, CVec pos, int local, int volume, CWorm *me);
inline void StartSound(const SmartPointer<SoundSample> & smp, CVec pos, int local, int volume, CWorm *me) {
	StartSound(smp.get(), pos, local, volume, me);
}

// TODO: don't make them global
extern AbsTime fCurSongStart;
extern AbsTime fTimePaused;
extern bool	 bSongStopped;
extern byte iMusicVolume;
extern bool	 bSongFinished;


// Music

typedef void (*MusicFinishedCB)();

id3v1_t			GetMP3Info(const std::string& file);
void			SetMusicFinishedHandler(MusicFinishedCB cb);





// Globals
extern	sfxgame_t	sfxGame;
extern	sfxgen_t	sfxGeneral;





#endif  //  __SOUNDS_H__
