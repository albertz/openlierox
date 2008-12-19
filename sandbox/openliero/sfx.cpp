#include "sfx.hpp"
#include "reader.hpp"
#include "console.hpp"
#include <vector>
#include <cassert>
#include <SDL/SDL.h>
#include <iostream>

// ----- Changed when importing to OLX -----

Sfx sfx;

#ifdef DEDICATED_ONLY

void Sfx::init() {}
void Sfx::loadFromSND() {}
void Sfx::play(int sound, int id, int loops) {}
void Sfx::stop(int id) {}
bool Sfx::isPlaying(int id) { return false; }
Sfx::~Sfx() {}


#else // DEDICATED_ONLY

#include <SDL_mixer.h>

void Sfx::init()
{
	OlxMod_InitSoundSystem(22050, AUDIO_S16SYS, 1);
}

void Sfx::loadFromSND()
{
	FILE* snd = openLieroSND();

	int count = readUint16(snd);

	sounds.resize(count);

	long oldPos = ftell(snd);

	for(int i = 0; i < count; ++i)
	{
		fseek(snd, oldPos + 8, SEEK_SET); // Skip name

		int offset = readUint32(snd);
		int length = readUint32(snd);

		oldPos = ftell(snd);

		int byteLength = length * 2;
		Uint8* buf = new Uint8[byteLength];

		sounds[i].allocated = 0;
		sounds[i].abuf = buf;
		sounds[i].alen = byteLength;
		sounds[i].volume = 128;

		Sint16* ptr = reinterpret_cast<Sint16*>(buf);

		std::vector<Sint8> temp(length);

		if(length > 0)
		{
			fseek(snd, offset, SEEK_SET);
			fread(&temp[0], 1, length, snd);
		}

		for(int j = 0; j < length; ++j)
		{
			ptr[j] = int(temp[j]) * 30;
		}
	}
	//fclose( snd );	// This file is cached internally by OpenLiero
}

void Sfx::play(int sound, int id, int loops)
{
	int channel = OlxMod_PlaySoundSample( sounds[sound].abuf, sounds[sound].alen, loops );
	if( id != -1 )
	{
		if( isPlaying( id ) )
			stop( id );
		soundsPlaying[id] = channel;
	};
}

void Sfx::stop(int id)
{
	if( soundsPlaying.find(id) == soundsPlaying.end() )
		return;
	OlxMod_StopSoundSample( soundsPlaying[id] );
	soundsPlaying.erase(id);
}

bool Sfx::isPlaying(int id)
{
	// That one will desync net, 'cause sounds are played in different speed on different machines
	// and the NetSyncedRandom() will be called some extra times.
	return false;
	if( soundsPlaying.find(id) == soundsPlaying.end() )
		return false;
	bool playing = OlxMod_IsSoundSamplePlaying( soundsPlaying[id] );
	if( ! playing )
		soundsPlaying.erase(id);
	return playing;
}

Sfx::~Sfx()
{
	for(std::size_t i = 0; i < sounds.size(); ++i)
	{
		delete [] sounds[i].abuf;
		sounds[i].abuf = 0;
	}
	sounds.clear();
	for( std::map< int, int > :: iterator it = soundsPlaying.begin(); it != soundsPlaying.end(); it++ )
	{
		OlxMod_StopSoundSample( it->second );
	};
	soundsPlaying.clear();
}

#endif

// ----- Changed when importing to OLX -----

