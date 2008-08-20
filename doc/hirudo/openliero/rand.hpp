#ifndef LIERO_RAND_HPP
#define LIERO_RAND_HPP

#include <SDL/SDL.h>
#include "OLXModInterface.h"
using namespace OlxMod;

// ----- Changed when importing to OLX -----
// This is now net-synced :)

struct NetSyncedRand
{
	void seed(Uint32 newSeed) { }
	
	Uint32 next()
	{
		return OlxMod_NetSyncedRandom();
	}
	
	Uint32 operator()()
	{
		return next();
	}
	
	Uint32 operator()(Uint32 max)
	{
		Uint64 v = next();
		v *= max;
		return Uint32(v >> 32);
	}
	
	Uint32 operator()(Uint32 min, Uint32 max)
	{
		return (*this)(max - min) + min;
	}
};

struct Rand
{
	void seed(Uint32 newSeed) { }
	
	Uint32 next()
	{
		return OlxMod_Random();
	}
	
	Uint32 operator()()
	{
		return next();
	}
	
	Uint32 operator()(Uint32 max)
	{
		Uint64 v = next();
		v *= max;
		return Uint32(v >> 32);
	}
	
	Uint32 operator()(Uint32 min, Uint32 max)
	{
		return (*this)(max - min) + min;
	}
};

// ----- Changed when importing to OLX -----

#endif // LIERO_RAND_HPP
