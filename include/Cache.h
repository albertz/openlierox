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

private:
	// TODO: use intern-data
	std::map<std::string, SDL_Surface *> ImageCache;
	std::map<std::string, SoundSample *> SoundCache;
	std::map<std::string, CMap *> MapCache;
	std::map<std::string, CGameScript *> ModCache;

public:
	SDL_Surface		*GetImage(const std::string& file);
	SoundSample		*GetSound(const std::string& file);
	CMap			*GetMap(const std::string& file);
	CGameScript		*GetMod(const std::string& dir);

	void			SaveImage(const std::string& file, SDL_Surface *img);
	void			SaveSound(const std::string& file, SoundSample *smp);
	void			SaveMap(const std::string& file, CMap *map);
	void			SaveMod(const std::string& dir, CGameScript *mod);
};

extern CCache cCache;


#endif  //  __CACHE_H__
