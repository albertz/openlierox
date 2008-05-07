/////////////////////////////////////////
//
//   OpenLieroX
//
//   Auxiliary Software class library
//
//   based on the work of JasonB
//   enhanced by Dark Charlie and Albert Zeyer
//
//   code under LGPL
//
/////////////////////////////////////////


// Cache system
// Created 7/11/01
// By Jason Boettcher


#ifndef __CACHE_H__
#define __CACHE_H__

#include <SDL.h>
#include <vector>
#include <map>
#include <set>
#include <list>
#include "SmartPointer.h"

// these forward-declaration are needed here
// they will be declared in CMap.h and CGameScript.h
struct SoundSample;
class CMap;
class CGameScript;
 
class CCache  {
public:
	CCache() { mutex = SDL_CreateMutex(); };
	~CCache() { SDL_DestroyMutex(mutex); };
	void Clear();
	void ClearExtraEntries(); // Clears cache partially - should be called from time to time

	SmartPointer<SDL_Surface>	GetImage(const std::string& file);
	SmartPointer<SoundSample>	GetSound(const std::string& file);
	SmartPointer<CMap>			GetMap(const std::string& file);
	SmartPointer<CGameScript>	GetMod(const std::string& dir);

	// Images and sounds and mods are stored in cache by reference, 
	// no copying is done, so it is required to use SmartPointer returned,
	// if you don't want cache system to delete your image right after you've loaded it.
	// Oh, don't ever call gfxFreeSurface() or FreeSoundSample() - cache will do that for you.
	void	SaveImage(const std::string& file, const SmartPointer<SDL_Surface> & img);
	void	SaveSound(const std::string& file, const SmartPointer<SoundSample> & smp);
	void	SaveMod(const std::string& dir, const SmartPointer<CGameScript> & mod);
	// Map is copied to cache, 'cause it will be modified during game - you should free your data yourself.
	void	SaveMap(const std::string& file, CMap *map);

private:
	typedef std::map<std::string, std::pair< SmartPointer<SDL_Surface>, float > > ImageCache_t;
	ImageCache_t ImageCache;
	typedef std::map<std::string, std::pair< SmartPointer<SoundSample>, float > > SoundCache_t;
	SoundCache_t SoundCache;
	typedef std::map<std::string, std::pair< SmartPointer<CMap>, float > > MapCache_t;
	MapCache_t MapCache;
	typedef std::map<std::string, std::pair< SmartPointer<CGameScript>, float > > ModCache_t;
	ModCache_t ModCache;
	
	SDL_mutex* mutex;
};

extern CCache cCache;

#endif  //  __CACHE_H__
