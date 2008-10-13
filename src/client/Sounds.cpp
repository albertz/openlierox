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

// Disable this pseudo-warning
#ifdef _MSC_VER
#pragma warning(disable: 4786)
#pragma warning(disable: 4996)
#endif

#include <SDL.h>
#include <stdlib.h>

#include "LieroX.h"

#include "AuxLib.h"
#include "Cache.h"
#include "StringUtils.h"
#include "Sounds.h"
#include "MathLib.h"
#include "Timer.h"
#include "Options.h"
#include "FindFile.h"



///////////////////
// Load the sounds
int LoadSounds(void)
{
	sfxGame.smpNinja = LoadSample("data/sounds/throw.wav",4);
	sfxGame.smpPickup = LoadSample("data/sounds/pickup.wav",2);
	sfxGame.smpBump = LoadSample("data/sounds/bump.wav", 2);
	sfxGame.smpDeath[0] = LoadSample("data/sounds/death1.wav", 2);
	sfxGame.smpDeath[1] = LoadSample("data/sounds/death2.wav", 2);
	sfxGame.smpDeath[2] = LoadSample("data/sounds/death3.wav", 2);

	//sfxGeneral.smpChat = LoadSample("data/sounds/chat.wav",2);
	sfxGeneral.smpClick = LoadSample("data/sounds/click.wav",2);
	sfxGeneral.smpNotify = LoadSample("data/sounds/notify.wav",2);
	if( sfxGeneral.smpNotify.get() == NULL )
		sfxGeneral.smpNotify = LoadSample("data/sounds/dirt.wav",2);	// Very funny sound

	return true;
}


sfxgame_t	sfxGame;
sfxgen_t	sfxGeneral;



#ifdef DEDICATED_ONLY


///////////////////
// Load a sample
SmartPointer<SoundSample> LoadSample(const std::string& _filename, int maxplaying) { return NULL; }

bool InitSoundSystem(int rate, int channels, int buffers) { return false; }
bool StartSoundSystem() { return false; }
bool StopSoundSystem() { return false; }
bool SetSoundVolume(int vol) { return false; }
int GetSoundVolume(void) { return 0; }
bool QuitSoundSystem() { return false; }
SmartPointer<SoundSample> LoadSoundSample(const std::string& filename, int maxsimulplays) { return NULL; }
bool FreeSoundSample(SoundSample* sample) { return false; }
bool PlaySoundSample(SoundSample* sample) { return false; }
void StartSound(SoundSample* smp, CVec pos, int local, int volume, CWorm *me) {}

// TODO: remove these when they are not global anymore
float fCurSongStart = 0;
float fTimePaused = 0;
bool  bSongStopped = false;
byte  iMusicVolume = 50;
bool  bSongFinished;

bool IsSongLoading() { return false; }
void InitializeMusic(void) {}
void PlayMusicAsync(const std::string& file) {}
SoundMusic *LoadMusic(const std::string& file) { return NULL; }
void FreeMusic(SoundMusic *music) {}
void PlayMusic(SoundMusic *music, int number_of_repeats) {}
void StopMusic(void) {}
float GetCurrentMusicTime(void) { return 0; }
void SetMusicVolume(byte vol) {}
void MusicFinishedHook(void) {}
void ShutdownMusic() {}
void PauseMusic(void) {}
void ResumeMusic(void) {}
void RewindMusic(void) {}
void SetMusicPosition(double pos) {}
bool PlayingMusic(void) { return false; }
bool PausedMusic(void) { return false; }
int GetMusicType(SoundMusic *music) { return 0; }
bool GetSongStopped(void) { return false; }
bool GetSongFinished(void) { return false; }
byte GetMusicVolume(void) { return 0; }

#else //DEDICATED_ONLY

///////////////////
// Load a sample and cache it
SoundSample * LoadSoundSample(const std::string& filename, int maxsimulplays);

SmartPointer<SoundSample> LoadSample(const std::string& _filename, int maxplaying)
{
	// Try cache first
	SmartPointer<SoundSample> SampleCached = cCache.GetSound(_filename);
	if (SampleCached.get())
		return SampleCached;

	SmartPointer<SoundSample> Sample;

	std::string fullfname = GetFullFileName(_filename);
	if(fullfname.size() == 0)
		return NULL;

	// Load the sample
	Sample = LoadSoundSample(fullfname, maxplaying);
	if( Sample.get() == NULL )
		return NULL;

	// Save to cache
	cCache.SaveSound(_filename, Sample);
	return Sample;
}


static bool SoundSystemAvailable = false;

bool InitSoundSystem(int rate, int channels, int buffers) {
	if(SoundSystemAvailable) return true;
	SoundSystemAvailable = false;

	if(getenv("SDL_AUDIODRIVER"))
		printf("SDL_AUDIODRIVER=%s\n", getenv("SDL_AUDIODRIVER"));
#if defined(__linux__)
	if(!getenv("SDL_AUDIODRIVER")) {
		printf("SDL_AUDIODRIVER not set, setting to ALSA\n");
		putenv((char*)"SDL_AUDIODRIVER=alsa");
	}
#endif

initSoundSystem:

	// HINT: other SDL stuff is already inited, we don't care here
	if( SDL_InitSubSystem(SDL_INIT_AUDIO) != 0 ) {
		printf("InitSoundSystem: Unable to initialize SDL-sound: %s\n", SDL_GetError());
		if(getenv("SDL_AUDIODRIVER")) {
			printf("trying again with SDL_AUDIODRIVER unset\n");
			unsetenv("SDL_AUDIODRIVER");
			goto initSoundSystem;
		} else
			return false;
	}

	if(Mix_OpenAudio(rate, AUDIO_S16, channels, buffers)) {
		printf("InitSoundSystem: Unable to open audio (SDL_mixer): %s\n", Mix_GetError());
		if(getenv("SDL_AUDIODRIVER")) {
			printf("trying again with SDL_AUDIODRIVER unset\n");
			unsetenv("SDL_AUDIODRIVER");
			goto initSoundSystem;
		} else
			return false;
	}

	int allocChanNum = Mix_AllocateChannels(1000); // TODO: enough?

	SoundSystemAvailable = true;
	printf("SoundSystem initialised, %i channels allocated\n", allocChanNum);
	return true;
}

static bool SoundSystemStarted = false;
static int SoundSystemVolume = 100;

bool StartSoundSystem() {
	if(!SoundSystemAvailable) return false;

	// TODO: this is only a workaround
	SoundSystemStarted = true;
	SetSoundVolume( tLXOptions->iSoundVolume );
	return true;
}

bool StopSoundSystem() {
	if(!SoundSystemAvailable) return false;

	// TODO: this is only a workaround
	SoundSystemStarted = false;
	Mix_Volume(-1, 0);
	return true;
}

bool SetSoundVolume(int vol) {
	tLXOptions->iSoundVolume = vol;
	if(!SoundSystemAvailable) return false;

	if(SoundSystemStarted) {
		SoundSystemVolume = vol;

		// The volume to use from 0 to MIX_MAX_VOLUME(128).
		float tmp = (float)MIX_MAX_VOLUME*(float)vol/100.0f;
		Mix_Volume(-1, Round(tmp));

		return true;
	}

	return false;
}

int GetSoundVolume(void)  {
	if(!SoundSystemAvailable) return 0;

	return SoundSystemVolume;
}

bool QuitSoundSystem() {
#if SDLMIXER_WORKAROUND_RESTART == 1
	if(bRestartGameAfterQuit) {
		printf("WARNING: You are using an old version of SDL_mixer. You should at least use 1.2.8.\n");
		printf("There is a known bug in 1.2.7, therefore we cannot restart the sound-system and we will leave it running at this point.\n");
		// HINT: in ShutdownAuxLib, SDL_Quit will exclude the quitting of audio
		return false;
	}
#endif

	if(!SoundSystemAvailable) return false;
	SoundSystemAvailable = false;

	Mix_CloseAudio();
	return true;
}

SoundSample * LoadSoundSample(const std::string& filename, int maxsimulplays) {
	if(!SoundSystemAvailable) return NULL;

	if(filename.size() > 0) {
		Mix_Chunk* sample = Mix_LoadWAV(Utf8ToSystemNative(filename).c_str());
		if(!sample) {
//			printf("LoadSoundSample: Error while loading %s: %s\n", filename.c_str(), Mix_GetError());
			return NULL;
		}

		SoundSample* ret = new SoundSample;
		ret->sample = sample;
		ret->maxsimulplays = maxsimulplays;
		return ret;

	} else
		return NULL;
}

bool FreeSoundSample(SoundSample* sample) {
	// HINT: this gets called in the SmartPointer<> destructor, at the very end of the program
	// It is safe to call Mix_FreeChunk here though

	// no sample, so we are ready
	if(!sample) return true;

	if(sample->sample) {
		Mix_FreeChunk(sample->sample);
		sample->sample = NULL;
	}
	delete sample;
	return true;
}

bool PlaySoundSample(SoundSample* sample) {
	if(!SoundSystemAvailable) return false;

	if(sample == NULL || sample->sample == NULL)
		return false;

	if(Mix_PlayChannel(-1, sample->sample, 0) != 0) {
		//printf("PlaySoundSample: Error playing %s\n", Mix_GetError());
		return false;
	}

	return true;
}




///////////////////
// Play a sound in the viewport
void StartSound(SoundSample* smp, CVec pos, int local, int volume, CWorm *me)
{
    // TODO: not used
//	int pan = 0;
//	int maxhearing = 750;	// Maximum distance for hearing

	// If this wasn't a sound by me, setup the volume & pan based on position
	if(!local) {
		/*float side = pos.x - me->getPos().x;
		float distance = CalculateDistance(pos,me->getPos());

		// To far
		if(distance > maxhearing)
			return;

		volume = (int)(100.0f*(1.0f-distance/maxhearing));
		pan = (int)(100*(side/maxhearing));*/


		// Check if it's in the viewport
		/*CViewport *v = me->getViewport();
		int wx = v->GetWorldX();
		int wy = v->GetWorldY();
		int l = v->GetLeft();
		int t = v->GetTop();

		// Are we inside the viewport?
		int x = (int)pos.x - wx;
		int y = (int)pos.y - wy;
		x*=2;
		y*=2;

		if(x+l+10 < l || x-10 > v->GetVirtW())
			return;
		if(y+t+10 < t || y-10 > v->GetVirtH())
			return;*/
	}

	// TODO: implement a PlayExSoundSample for this
	// this was the old call (using BASS_SamplePlayEx):
	//PlayExSampleSoundEx(smp,0,-1,volume,pan,-1);
	// we are using a workaround here
	// TODO: let it like that, in LX 0.6x this has been made and no one liked it much
	PlaySoundSample(smp);
}

//
// Music part
//
float fCurSongStart = 0;
float fTimePaused = 0;
bool  bSongStopped = false;
byte  iMusicVolume = 50;
bool  bSongFinished;

// Loading thread
static SDL_Thread *PlayMusThread = NULL;
static bool		breakPlayThread = false;
static bool		LoadingSong = false;
static std::string SongName = "";
static SoundMusic *LoadedMusic;

static int PlayThreadMain(void *n)
{
	while (!breakPlayThread)  {
		if (!LoadingSong)
			SDL_Delay(10);
		else  {
			FreeMusic(LoadedMusic);  // Free any loaded music
			//SDL_Delay(10); // No hurry...
			LoadedMusic = LoadMusic(SongName);  // Load the new music
			//SDL_Delay(10);
			if (LoadedMusic)
				PlayMusic(LoadedMusic);  // Play the music
			LoadingSong = false;
		}
	}

	return 0;
}

bool IsSongLoading() { return LoadingSong; }


static void MusicFinishedHook(void)
{
	bSongFinished = !GetSongStopped();
}


void InitializeMusic(void)
{
	Mix_HookMusicFinished(&MusicFinishedHook);
	SetMusicVolume(tLXOptions->iMusicVolume);
	PlayMusThread = SDL_CreateThread(PlayThreadMain,NULL);
}

void PlayMusicAsync(const std::string& file)
{
	while (LoadingSong) SDL_Delay(5);  // If we're currently loading another song, wait

	if (file == SongName && LoadedMusic)  {
		if (!PlayingMusic())
			PlayMusic(LoadedMusic, 1); // If stopped, play
		return;  // Already loading this
	}

	SongName = file;
	LoadingSong = true;
}

SoundMusic *LoadMusic(const std::string& file)
{
	if (file == "")
		return NULL;

	SoundMusic *new_music = new SoundMusic;
	if (!new_music)
		return NULL;

	new_music->sndMusic = Mix_LoadMUS(Utf8ToSystemNative(file).c_str());

	if (!new_music->sndMusic)  {
		delete new_music;
		return NULL;
	}

	return new_music;
}

void FreeMusic(SoundMusic *music)
{
	if (music) {
		Mix_FreeMusic(music->sndMusic);
		delete music;
	}
}

void PlayMusic(SoundMusic *music, int number_of_repeats)
{
	if (!music)
		return;
	//Mix_PlayMusic(music->sndMusic,number_of_repeats);
	Mix_FadeInMusic(music->sndMusic,number_of_repeats,500);
	fCurSongStart = GetMilliSeconds();
	fTimePaused = 0;
	bSongStopped = false;
}

void StopMusic(void)
{
	byte oldvolume = GetMusicVolume();
	SetMusicVolume(0);
	Mix_HaltMusic();
	if (Mix_PausedMusic())
		Mix_ResumeMusic();
	fCurSongStart = 0;
	fTimePaused = 0;
	bSongStopped = true;
	SetMusicVolume(oldvolume);
}

float GetCurrentMusicTime(void)
{
	// No song playing
	if (!fCurSongStart)
		return 0;

	// Paused
	if (fTimePaused)
		return fTimePaused-fCurSongStart;
	// Not paused
	else
		return GetMilliSeconds()-fCurSongStart;
}

void SetMusicVolume(byte vol)
{
	vol = (byte)MIN(vol,100);
	iMusicVolume = vol;
	tLXOptions->iMusicVolume = vol;

	// The volume to use from 0 to MIX_MAX_VOLUME(128).
	float tmp = (float)MIX_MAX_VOLUME*(float)vol/100.0f;
	Mix_VolumeMusic(Round(tmp));
}

void ShutdownMusic(void)
{
	breakPlayThread = true;
	Mix_HookMusicFinished(NULL);
	if (PlayMusThread)
		SDL_WaitThread(PlayMusThread, NULL);
	FreeMusic(LoadedMusic);
}



void		PauseMusic(void) {Mix_PauseMusic(); fTimePaused = GetMilliSeconds(); bSongStopped = false;}
void		ResumeMusic(void) {Mix_ResumeMusic();fCurSongStart += GetMilliSeconds()-fTimePaused; fTimePaused = 0; bSongStopped = false;}
void		RewindMusic(void) {Mix_RewindMusic();fCurSongStart = GetMilliSeconds();fTimePaused = 0;}
void		SetMusicPosition(double pos)  {Mix_RewindMusic(); Mix_SetMusicPosition(pos); }
bool		PlayingMusic(void) {return Mix_PlayingMusic() != 0; }
bool		PausedMusic(void) {return Mix_PausedMusic() != 0; }
int			GetMusicType(SoundMusic *music) {if (music) {return Mix_GetMusicType(music->sndMusic);} else {return Mix_GetMusicType(NULL);} }
bool		GetSongStopped(void) {return bSongStopped; }
bool		GetSongFinished(void) { bool tmp = bSongFinished; bSongFinished = false; return tmp; }
byte		GetMusicVolume(void) { return iMusicVolume; }



#endif


id3v1_t GetMP3Info(const std::string& file)
{
	id3v1_t info;
	// Clear the info
	memset(&info,0,sizeof(id3v1_t));

	if (file == "")
		return info;

	FILE *fp = fopen(file.c_str(),"rb");
	if (!fp)
		return info;

	// The tag begins 128 bytes before the end of the MP3 file
	if (fseek(fp,-128,SEEK_END))  {
		fclose(fp);
		return info;
	}

	// Check the header
	char header[4];
	if (!fread(&header[0],3,1,fp))  {
		fclose(fp);
		return info;
	}
	header[3] = '\0';
	if (strcmp(header,"TAG"))  {
		fclose(fp);
		return info;
	}

	// Read the ID3 tag
	if (!fread(&info,sizeof(id3v1_t),1,fp))  {
		memset(&info,0,sizeof(id3v1_t));
		fclose(fp);
		return info;
	}

	fclose(fp);

	return info;
}

template <> void SmartPointer_ObjectDeinit<SoundSample> ( SoundSample * obj )
{
	FreeSoundSample(obj);
};
