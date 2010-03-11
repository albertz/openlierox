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
#include <cstdlib>

#include "sound/SoundsBase.h"
#include "LieroX.h"
#include "Debug.h"
#include "AuxLib.h"
#include "Cache.h"
#include "StringUtils.h"
#include "MathLib.h"
#include "Timer.h"
#include "Options.h"
#include "FindFile.h"
#include "game/Sounds.h"

#include "sound/sfx.h"
#include "sound/sfxdriver.h"


///////////////////
// Load the sounds
bool LoadSounds()
{
	if(bDedicated) return false;
	
	//sfxGeneral.smpChat = LoadSample("data/sounds/chat.wav",2);
	sfxGeneral.smpClick = LoadSample("data/sounds/click.wav",2);
	sfxGeneral.smpNotify = LoadSample("data/sounds/notify.wav",2);
	if( sfxGeneral.smpNotify.get() == NULL ) {
		notes << "LoadSounds: cannot load notify.wav" << endl;
		sfxGeneral.smpNotify = LoadSample("data/sounds/dirt.wav",2);	// Very funny sound
	}
	
	LoadSounds_Game();
	
	return true;
}

void ShutdownSounds() {
	ShutdownSounds_Game();

	sfxGeneral = sfxgen_t();
}

sfxgen_t	sfxGeneral;



#ifdef DEDICATED_ONLY


///////////////////
// Load a sample
SmartPointer<SoundSample> LoadSample(const std::string& _filename, int maxplaying) { return NULL; }

bool InitSoundSystem(int rate, int channels, int buffers) { return false; }
bool StartSoundSystem() { return false; }
bool StopSoundSystem() { return false; }
bool SetSoundVolume(int vol) { return false; }
int GetSoundVolume() { return 0; }
bool QuitSoundSystem() { return false; }
SmartPointer<SoundSample> LoadSoundSample(const std::string& filename, int maxsimulplays) { return NULL; }
bool FreeSoundSample(SoundSample* sample) { return false; }
bool PlaySoundSample(SoundSample* sample) { return false; }
void StartSound(SoundSample* smp, CVec pos, int local, int volume, CWorm *me) {}
void StartSound(SoundSample* smp, CVec pos) {}

#else // not DEDICATED_ONLY

static bool SoundSystemAvailable = false;

///////////////////
// Load a sample and cache it

SmartPointer<SoundSample> LoadSample(const std::string& _filename, int maxplaying)
{
	if(bDedicated) return NULL;
	if(!SoundSystemAvailable) return NULL;
	
	if(_filename.size() == 0) return NULL;
	std::string filenameWithoutExt = GetFilenameWithoutExt(_filename);
	
	// Try cache first
	SmartPointer<SoundSample> SampleCached = cCache.GetSound(filenameWithoutExt);
	if (SampleCached.get())
		return SampleCached;

	SmartPointer<SoundSample> Sample = sfx.getDriver()->load(_filename);
	
	// in case we failed and it was not an ogg file, try again the same filename with ogg instead
	if((!Sample.get() || !Sample->avail()) && !stringcaseequal( GetFileExtensionWithDot(_filename), ".ogg" ))
		Sample = sfx.getDriver()->load(filenameWithoutExt + ".ogg");
	
	if(Sample.get() && Sample->avail()) {
		Sample->maxSimultaniousPlays = maxplaying;
		cCache.SaveSound(filenameWithoutExt, Sample); // Save to cache
		return Sample;
	}

	// dont print additional warnings here, we will print all warnings inside of load
	return NULL;
}


bool InitSoundSystem(int rate, int channels, int buffers) {
	if(SoundSystemAvailable) return true;
	SoundSystemAvailable = false;

	if(bDedicated) return false;

	if(!sfx.init()) {
		errors << "InitSoundSystem failed" << endl;
		return false;
	}
	
	SoundSystemAvailable = true;
	notes << "SoundSystem initialised" << endl;
	return true;
}

static bool SoundSystemStarted = false;

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
	if(SoundSystemAvailable)
		sfx.getDriver()->setVolume(0);
	return true;
}

bool SetSoundVolume(int vol) {
	tLXOptions->iSoundVolume = vol;
	if(!SoundSystemAvailable) return false;

	if(SoundSystemStarted) {
		sfx.getDriver()->setVolume(float(vol) / 100.0f);
		return true;
	}

	return false;
}

int GetSoundVolume()  {
	if(!SoundSystemAvailable) return 0;

	return Round(sfx.getDriver()->volume() * 100);
}

bool QuitSoundSystem() {	
	if(!SoundSystemAvailable) return false;
	SoundSystemAvailable = false;

	cCache.ClearSounds();
	sfx.shutDown();
	return true;
}

bool FreeSoundSample(SoundSample* sample) {
	// HINT: this gets called in the SmartPointer<> destructor, at the very end of the program
	// It is safe to call Mix_FreeChunk here though

	// no sample, so we are ready
	if(!sample) return true;

	// the destructor will free the internal sfxdriver dependend buffer
	delete sample;
	return true;
}

bool PlaySoundSample(SoundSample* sample) {
	if(!SoundSystemAvailable || !SoundSystemStarted) return false;

	if(sample == NULL)
		return false;
	
	sfx.playSimpleGlobal(sample);
	return true;
}




///////////////////
// Play a sound in the viewport
void StartSound(SoundSample* smp, CVec pos, int local, int volume, CWorm *me)
{
	if(!SoundSystemAvailable || !SoundSystemStarted) return;

	if(smp == NULL)
		return;
		
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
	
	sfx.playSimple2D(smp, pos);
	// this was the old call (using BASS_SamplePlayEx):
	//PlayExSampleSoundEx(smp,0,-1,volume,pan,-1);
}

void StartSound(SoundSample* smp, CVec pos) {
	sfx.playSimple2D(smp, pos);
}




#endif


id3v1_t GetMP3Info(const std::string& file)
{
	id3v1_t info;
	// Clear the info
	memset(&info,0,sizeof(id3v1_t));

	if (file == "")
		return info;

	FILE *fp = fopen(Utf8ToSystemNative(file).c_str(),"rb");
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
