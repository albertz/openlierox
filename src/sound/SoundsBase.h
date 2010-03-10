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
#include "olx-types.h"
#include "sound/sound_sample.h"



// General sounds
typedef struct {
	SmartPointer<SoundSample> smpClick;
	SmartPointer<SoundSample> smpChat;
	SmartPointer<SoundSample> smpNotify;
} sfxgen_t;


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

void StartSound(SoundSample* smp, CVec pos);


// TODO: don't make them global
extern AbsTime fCurSongStart;
extern AbsTime fTimePaused;
extern bool	 bSongStopped;
extern byte iMusicVolume;
extern bool	 bSongFinished;


// Music

id3v1_t			GetMP3Info(const std::string& file);





// Globals
extern	sfxgen_t	sfxGeneral;





#endif  //  __SOUNDS_H__
