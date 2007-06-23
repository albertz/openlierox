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

#include <SDL/SDL.h>
#include <vector>
#include <string>

// Cached item types
#define		CCH_IMAGE		0
#define		CCH_SOUND		1			// MP3's not included, coz they stream
#define		CCH_TEXTURE		2

// Incorperate Textures? Animations?

// this forward-declaration is needed here
// it will be declared in Sounds.h
struct SoundSample;

// The item class (can be surface, sample)
class CCache {
public:
	// Constructor
	CCache() {
		Type = CCH_IMAGE;
		
		Image = NULL;
		Sample = NULL;
	}


private:
	// Attributes

	int		Type;

	std::string	Filename;

	// Image
	SDL_Surface*	Image;

	// Sample
	SoundSample*	Sample;


public:
	// Methods

	
	// Loading
	SDL_Surface*	LoadImgBPP(const std::string& _file, bool withalpha);
	SoundSample*	LoadSample(const std::string& _file, int maxplaying);


	// Shutdowning
	void			Shutdown(void);


	// Variables
	int				getType(void)			{ return Type; }
	std::string		getFilename(void)		{ return Filename; }

	SDL_Surface		*GetImage(void)			{ return Image; }
	SoundSample*	GetSample(void)			{ return Sample; }
};

extern std::vector<CCache> Cache;



//////////////////////////////////////
//			The cache system
//////////////////////////////////////


int		InitializeCache(void);
void	ShutdownCache(void);






#endif  //  __CACHE_H__
