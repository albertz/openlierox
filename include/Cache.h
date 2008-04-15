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

// Cached item types
#define		CCH_IMAGE		0
#define		CCH_SOUND		1			// MP3's not included, coz they stream
#define		CCH_TEXTURE		2
#define		CCH_MAP			3
#define		CCH_MOD			4

// Incorperate Textures? Animations?

// these forward-declaration are needed here
// they will be declared in Sounds.h, CMap.h and CGameScript.h
struct SoundSample;
class CMap;
class CGameScript;


class CCache  {
public:
	CCache() {}
	void Clear();
	void ClearExtraEntries();

private:
	// TODO: use intern-data
	std::map<std::string, SDL_Surface *> ImageCache;
	std::map<std::string, SoundSample *> SoundCache;
	std::map<std::string, CMap *> MapCache;
	std::map<std::string, CGameScript *> ModCache;


	// For ClearExtraEntries() funcs - used to convert pointer to resource to the entry in cache
	std::map<SDL_Surface *, std::map<std::string, SDL_Surface *> :: iterator> ImageCache_ForRemoval;
	std::map<SoundSample *, std::map<std::string, SoundSample *> :: iterator> SoundCache_ForRemoval;
	std::map<CMap *, std::map<std::string, CMap *> :: iterator> MapCache_ForRemoval;
	std::map<CGameScript *, std::map<std::string, CGameScript *> :: iterator> ModCache_ForRemoval;
	
	enum CachedType_t { CT_Image, CT_Sound, CT_Map, CT_Mod };
	typedef std::map< std::pair< CachedType_t, void * >, float > TimeLastAccessed_t;
	TimeLastAccessed_t TimeLastAccessed;
	typedef std::set< std::pair< CachedType_t, void * > > ResourcesNotUsed_t;
	ResourcesNotUsed_t ResourcesNotUsed;

public:
	SDL_Surface		*GetImage(const std::string& file);
	SoundSample		*GetSound(const std::string& file);
	CMap			*GetMap(const std::string& file);
	CGameScript		*GetMod(const std::string& dir);

	void			SaveImage(const std::string& file, SDL_Surface *img);
	void			SaveSound(const std::string& file, SoundSample *smp);
	void			SaveMap(const std::string& file, CMap *map);
	void			SaveMod(const std::string& dir, CGameScript *mod);
	
	// Resources that are not used may be deleted from cache at any time (may be called with invalid pointer)
	// Returns true if resource was in cache
	bool			ResourceNotUsed(SDL_Surface *);
	bool			ResourceNotUsed(SoundSample *);
	bool			ResourceNotUsed(CMap *);
	bool			ResourceNotUsed(CGameScript *);
};

extern CCache cCache;


#endif  //  __CACHE_H__
