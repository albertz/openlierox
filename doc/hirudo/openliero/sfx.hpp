#ifndef LIERO_SFX_HPP
#define LIERO_SFX_HPP

#include <SDL.h>

#ifndef DEDICATED_ONLY
#include <SDL_mixer.h>
#endif

#include <vector>
#include <map>
#include "OLXModInterface.h"
using namespace OlxMod;

// ----- Changed when importing to OLX -----

struct Sfx
{
	struct ChannelInfo
	{
		ChannelInfo()
		: id(0)
		{
		}

		int id; // ID of the sound playing on this channel
	};

	~Sfx();

	void init();
	void loadFromSND();

	void play(int sound, int id = -1, int loops = 0);
	bool isPlaying(int id);
	void stop(int id);

#ifndef DEDICATED_ONLY
	std::vector<Mix_Chunk> sounds;
	std::map< int, int > soundsPlaying;
#endif
};

// ----- Changed when importing to OLX -----

extern Sfx sfx;

#endif // LIERO_SFX_HPP
