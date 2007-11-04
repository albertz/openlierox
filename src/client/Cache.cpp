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


#include "LieroX.h"
#include "Sounds.h"
#include "Cache.h"
#include "GfxPrimitives.h"
#include "FindFile.h"
#include "CGameScript.h"
#include "CMap.h"

CCache cCache;

//////////////
// Save an image to the cache
void CCache::SaveImage(const std::string& file, SDL_Surface *img)
{
	if (img != NULL)
		ImageCache[file] = img;
}

//////////////
// Save a sound sample to the cache
void CCache::SaveSound(const std::string& file, SoundSample *smp)
{
	if (smp != NULL)
		SoundCache[file] = smp;
}

//////////////
// Save a map to the cache
void CCache::SaveMap(const std::string& file, CMap *map)
{
	if (map == NULL)
		return;

	// Copy the map to the cache (not just the pointer because map changes during the game)
	CMap *cached_map = new CMap;
	if (cached_map == NULL)
		return;

	if (!cached_map->NewFrom(map))
		return;

	MapCache[file] = cached_map;
}

//////////////
// Save a mod to the cache
void CCache::SaveMod(const std::string& file, CGameScript *mod)
{
	if (mod == NULL)
		return;

	CGameScript *cached_mod = new CGameScript;
	if (!cached_mod)
		return;

	*cached_mod = *mod;

	ModCache[file] = cached_mod;

}

//////////////
// Get an image from the cache
SDL_Surface *CCache::GetImage(const std::string& file)
{
	std::map<std::string, SDL_Surface *>::iterator item = ImageCache.find(file);
	if(item != ImageCache.end())
		return item->second;
	else
		return NULL;
}

//////////////
// Get a sound sample from the cache
SoundSample *CCache::GetSound(const std::string& file)
{
	std::map<std::string, SoundSample *>::iterator item = SoundCache.find(file);
	if(item != SoundCache.end())
		return item->second;
	else
		return NULL;
}

//////////////
// Get a map from the cache
CMap *CCache::GetMap(const std::string& file)
{
	std::map<std::string, CMap *>::iterator item = MapCache.find(file);
	if(item != MapCache.end())
		return item->second;
	else
		return NULL;
}

//////////////
// Get a mod from the cache
CGameScript *CCache::GetMod(const std::string& dir)
{
	std::map<std::string, CGameScript *>::iterator item = ModCache.find(dir);
	if(item != ModCache.end())
		return item->second;
	else
		return NULL;
}

//////////////
// Free all allocated data
CCache::~CCache()
{
	// Free all the images
	std::map<std::string, SDL_Surface *>::iterator img = ImageCache.begin();
	for (; img != ImageCache.end(); img++)
		SDL_FreeSurface(img->second);

	// Free all the samples
	std::map<std::string, SoundSample *>::iterator snd = SoundCache.begin();
	for (; snd != SoundCache.end(); snd++)
		FreeSoundSample(snd->second);

	// Free all the maps
	std::map<std::string, CMap *>::iterator map = MapCache.begin();
	for (; map != MapCache.end(); map++)  {
		map->second->Shutdown();
		delete map->second;
	}

	// Free all the mods
	std::map<std::string, CGameScript *>::iterator mod = ModCache.begin();
	for (; mod != ModCache.end(); mod++)  {
		mod->second->Shutdown();
		delete mod->second;
	}

}


