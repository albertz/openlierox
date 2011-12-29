#ifndef PLAYER_OPTIONS_H
#define PLAYER_OPTIONS_H

#include <SDL.h>
#include <string>
#include <list>
#include "util/angle.h"
#include "gusanos/allegro.h"

// TODO: Move these to blitters/<somewhere>
inline Uint32 universalColor(Uint8 r, Uint8 g, Uint8 b)
{
	return ((r & 0xFF) << 16) | ((g & 0xFF) << 8) | (b & 0xFF) | 0xff000000;
}

inline Uint32 universalToLocalColor(Uint32 c)
{
	return makecol(
		(c >> 16) & 0xFF,
		(c >> 8) & 0xFF,
		(c) & 0xFF);
}

struct PlayerOptions
{
	PlayerOptions();	
	float ropeAdjustSpeed;
};

#endif  // _PLAYER_OPTIONS_H_
