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


// Cached item types
#define		CCH_IMAGE		0
#define		CCH_SOUND		1			// MP3's not included, coz they stream
#define		CCH_TEXTURE		2

// Incorperate Textures? Animations?

#define		MAX_CACHE		1024

// this forward-declaration is needed here
// it will be declared in Sounds.h
struct SoundSample;

// The item class (can be surface, sample)
class CCache {
public:
	// Constructor
	CCache() {
		Used = false;
		Type = CCH_IMAGE;
		
		Image = NULL;
	}


private:
	// Attributes

	int		Used;
	int		Type;

	char	Filename[256];

	// Image
	SDL_Surface		*Image;

	// Sample
	SoundSample*	Sample;


public:
	// Methods

	
	// Loading
	SDL_Surface		*LoadImg(char *_file);
	SDL_Surface		*LoadImgBPP(char *_file, int bpp);
	SoundSample*			LoadSample(char *_file, int maxplaying);


	// Shutdowning
	void			Shutdown(void);


	// Variables
	int				isUsed(void)			{ return Used; }
	int				getType(void)			{ return Type; }
	char			*getFilename(void)		{ return Filename; }

	SDL_Surface		*GetImage(void)			{ return Image; }
	SoundSample*			GetSample(void)			{ return Sample; }
};

extern CCache* Cache;



//////////////////////////////////////
//			The cache system
//////////////////////////////////////


int		InitializeCache(void);
void	ShutdownCache(void);

SDL_Surface     *_LoadImage(char *filename);

SDL_Surface		*LoadImage(char *_filename, int correctbpp);
SoundSample*			LoadSample(char *_filename, int maxplaying);






#endif  //  __CACHE_H__
