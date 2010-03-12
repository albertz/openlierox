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
#include <cassert>
#include "SmartPointer.h"
#include "olx-types.h"

// these forward-declaration are needed here
// they will be declared in CMap.h and CGameScript.h
class SoundSample;
class CMap;
class CGameScript;
 
class CCache  {
public:
	CCache() { mutex = SDL_CreateMutex(); };
	~CCache() { SDL_DestroyMutex(mutex); };
	CCache(const CCache&) { assert(false); }
	CCache& operator=(const CCache&) { assert(false); return *this; }
	void Clear();
	void ClearSounds();
	void ClearExtraEntries(); // Clears cache partially - should be called from time to time

	SmartPointer<SDL_Surface>	GetImage__unsafe(const std::string& file);
	SmartPointer<SoundSample>	GetSound(const std::string& file);
	SmartPointer<CMap>			GetMap(const std::string& file);
	SmartPointer<CGameScript>	GetMod(const std::string& dir);

	// Images and sounds and mods are stored in cache by reference, 
	// no copying is done, so it is required to use SmartPointer returned,
	// if you don't want cache system to delete your image right after you've loaded it.
	// Oh, don't ever call gfxFreeSurface() or FreeSoundSample() - cache will do that for you.
	void	SaveImage__unsafe(const std::string& file, const SmartPointer<SDL_Surface> & img);
	void	SaveSound(const std::string& file, const SmartPointer<SoundSample> & smp);
	void	SaveMod(const std::string& dir, const SmartPointer<CGameScript> & mod);
	// Map is copied to cache, 'cause it will be modified during game - you should free your data yourself.
	void	SaveMap(const std::string& file, CMap *map);
	size_t	GetCacheSize();
	size_t	GetEntryCount();

	SDL_mutex* mutex;

private:
	class CacheItem_t { public:
		//virtual ~CacheItem_t() {} // TODO: do we need this when there is no other virtual function? 
		CacheItem_t() : fSaveTime(0), iFileTimeStamp(0) {}
		CacheItem_t(const CacheItem_t& oth)  { operator=(oth); }
		CacheItem_t(const AbsTime& savetime, Uint64 timestamp) : fSaveTime(savetime), iFileTimeStamp(timestamp) {}
		// TODO: this op= was virtual; was there any reason for this? (no other class had overloaded it)
		CacheItem_t& operator=(const CacheItem_t& oth)  { if (&oth != this) { fSaveTime = oth.fSaveTime; iFileTimeStamp = oth.iFileTimeStamp; } return *this; }
		AbsTime	fSaveTime;
		Uint64	iFileTimeStamp;
	};

	class ImageItem_t : public CacheItem_t { public:
		ImageItem_t() : CacheItem_t() { bmpSurf = NULL; }
		ImageItem_t(const ImageItem_t& oth) { operator=(oth); }
		ImageItem_t(SmartPointer<SDL_Surface> img, const AbsTime& savetime, Uint64 timestamp) : CacheItem_t(savetime, timestamp) { bmpSurf = img; }
		ImageItem_t& operator=(const ImageItem_t& oth) { CacheItem_t::operator =(oth); if (&oth != this) { bmpSurf = oth.bmpSurf; } return *this; }
		SmartPointer<SDL_Surface> bmpSurf;
	};

	class SoundItem_t : public CacheItem_t { public:
		SoundItem_t() : CacheItem_t() { sndSample = NULL; }
		SoundItem_t(const SoundItem_t& oth) { operator=(oth); }
		SoundItem_t(SmartPointer<SoundSample> snd, const AbsTime& savetime, Uint64 timestamp) : CacheItem_t(savetime, timestamp) { sndSample = snd; }
		SoundItem_t& operator=(const SoundItem_t& oth) { CacheItem_t::operator =(oth); if (&oth != this) { sndSample = oth.sndSample; } return *this; }
		SmartPointer<SoundSample> sndSample;
	};

	class MapItem_t : public CacheItem_t { public:
		MapItem_t() : CacheItem_t() { tMap = NULL; }
		MapItem_t(const MapItem_t& oth) { operator=(oth); }
		MapItem_t(SmartPointer<CMap> map, const AbsTime& savetime, Uint64 timestamp) : CacheItem_t(savetime, timestamp) { tMap = map; }
		MapItem_t& operator=(const MapItem_t& oth) { CacheItem_t::operator =(oth); if (&oth != this) { tMap = oth.tMap; } return *this; }
		SmartPointer<CMap> tMap;
	};

	class ModItem_t : public CacheItem_t { public:
		ModItem_t() : CacheItem_t() { tMod = NULL; }
		ModItem_t(const ModItem_t& oth) { operator=(oth); }
		ModItem_t(SmartPointer<CGameScript> mod, const AbsTime& savetime, Uint64 timestamp) : CacheItem_t(savetime, timestamp) { tMod = mod; }
		ModItem_t& operator=(const ModItem_t& oth) { CacheItem_t::operator =(oth); if (&oth != this) { tMod = oth.tMod; } return *this; }
		SmartPointer<CGameScript> tMod;
	};

	typedef std::map<std::string, ImageItem_t > ImageCache_t;
	ImageCache_t ImageCache;
	typedef std::map<std::string, SoundItem_t > SoundCache_t;
	SoundCache_t SoundCache;
	typedef std::map<std::string, MapItem_t > MapCache_t;
	MapCache_t MapCache;
	typedef std::map<std::string, ModItem_t > ModCache_t;
	ModCache_t ModCache;
};

extern CCache cCache;

// Debug functions
#ifdef DEBUG
void InitCacheDebug();
void ShutdownCacheDebug();
#endif

#endif  //  __CACHE_H__
