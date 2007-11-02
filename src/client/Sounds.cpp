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
#endif

#include "LieroX.h"
#include "Cache.h"
#include "StringUtils.h"
#include "Sounds.h"
#include "MathLib.h"


///////////////////
// Load a sample
SoundSample* LoadSample(const std::string& _filename, int maxplaying)
{
	std::string fname = _filename;

	// Has this been already loaded?
	std::map<std::string, CCache>::iterator item = Cache.find(fname);
	if(item != Cache.end())
		return item->second.GetSample();

	// Didn't find one already loaded? Load new one
	CCache tmp;
	SoundSample *result = tmp.LoadSample(fname, maxplaying);
	Cache[fname] = tmp;

	return result;
}




sfxgame_t	sfxGame;
sfxgen_t	sfxGeneral;

bool SoundSystemAvailable = false;

bool InitSoundSystem(int rate, int channels, int buffers) {
	SoundSystemAvailable = false;

	// HINT: other SDL stuff is already inited, we don't care here
	if( SDL_Init(SDL_INIT_AUDIO) != 0 ) {
		printf("InitSoundSystem: Unable to initialize SDL-sound: %s\n", SDL_GetError());
		return false;
	}

	if(Mix_OpenAudio(rate, AUDIO_S16, channels, buffers)) {
		printf("InitSoundSystem: Unable to open audio (SDL_mixer): %s\n", Mix_GetError());
    	return false;
	}

	Mix_AllocateChannels(1000); // TODO: enough?

	SoundSystemAvailable = true;
	return true;
}

bool SoundSystemStarted = false;
int SoundSystemVolume = 100;

bool StartSoundSystem() {
	if(!SoundSystemAvailable) return false;

	// TODO: this is only a workaround
	SoundSystemStarted = true;
	SetSoundVolume(SoundSystemVolume);
	return true;
}

bool StopSoundSystem() {
	if(!SoundSystemAvailable) return false;

	// TODO: this is only a workaround
	SoundSystemStarted = false;
	SetSoundVolume(0);
	return true;
}

bool SetSoundVolume(int vol) {
	if(!SoundSystemAvailable) return false;

	if(SoundSystemStarted) {
		SoundSystemVolume = vol;

		// The volume to use from 0 to MIX_MAX_VOLUME(128).
		//vol *= Round((float)MIX_MAX_VOLUME/100.0f);
		float tmp = (float)MIX_MAX_VOLUME*(float)vol/100.0f;
		Mix_Volume(-1, Round(tmp));

		return true;

	} else { // not SoundSystemStarted
		if(vol == 0) {
			Mix_Volume(-1, 0);
			return true;
		} else
			return false;

	}
}

int GetSoundVolume(void)  {
	if(!SoundSystemAvailable) return 0;

	return SoundSystemVolume;
}

bool QuitSoundSystem() {
	if(!SoundSystemAvailable) return false;

	Mix_CloseAudio();
	return true;
}

SoundSample* LoadSoundSample(const std::string& filename, int maxsimulplays) {
	if(!SoundSystemAvailable) return NULL;

	if(filename.size() > 0) {
		Mix_Chunk* sample = Mix_LoadWAV(filename.c_str());
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
	if(!SoundSystemAvailable) return false;

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
SDL_Thread *PlayMusThread = NULL;
bool		breakPlayThread = false;
bool		LoadingSong = false;
std::string SongName = "";
SoundMusic *LoadedMusic;

int PlayThreadMain(void *n)
{
	while (!breakPlayThread)  {
		if (!LoadingSong)
			SDL_Delay(10);
		else  {
			FreeMusic(LoadedMusic);  // Free any loaded music
			//SDL_Delay(10); // No hurry...
			LoadedMusic = LoadMusic(SongName);  // Load the new music
			//SDL_Delay(10);
			PlayMusic(LoadedMusic);  // Play the music
			LoadingSong = false;
		}
	}

	return 0;
}

bool IsSongLoading() { return LoadingSong; }

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

	new_music->sndMusic = Mix_LoadMUS(file.c_str());
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

void MusicFinishedHook(void)
{
	bSongFinished = !GetSongStopped();
}

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

void ShutdownMusic(void)
{
	breakPlayThread = true;
	Mix_HookMusicFinished(NULL);
	if (PlayMusThread)
		SDL_WaitThread(PlayMusThread, NULL);
	FreeMusic(LoadedMusic);
}

