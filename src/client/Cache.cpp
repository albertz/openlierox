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


#include "defs.h"
#include "LieroX.h"
#include "Sounds.h"
#include "Cache.h"
#include "GfxPrimitives.h"
#include "FindFile.h"

std::vector<CCache> Cache;



//////////////////////////////////////
//	      The cache item class
//////////////////////////////////////


///////////////////
// Load an image
SDL_Surface* _LoadImage(const std::string& filename) {
	std::string fname = GetFullFileName(filename);
	if(fname.size() == 0)
		return NULL;
		
	return IMG_Load(fname.c_str());
}



///////////////////
// Loads an image, and converts it to the same colour depth as the screen (speed)
SDL_Surface *CCache::LoadImgBPP(const std::string& _file, bool withalpha) {
	Type = CCH_IMAGE;
	Filename = _file;

	// Load the image
	SDL_Surface* img = _LoadImage(Filename);

	if(!img) {
//		printf("CCache::LoadImgBPP: Error loading file: %s\n", Filename.c_str());
		return NULL;
	}

	// Convert the image to the screen's colour depth
	if (withalpha)  {
		SDL_PixelFormat fmt = *(SDL_GetVideoSurface()->format);
		fmt.BitsPerPixel = 32;
		fmt.BytesPerPixel = 4;
		fmt.Rmask = ALPHASURFACE_RMASK;
		fmt.Gmask = ALPHASURFACE_GMASK;
		fmt.Bmask = ALPHASURFACE_BMASK;
		fmt.Amask = ALPHASURFACE_AMASK;
		int flags = iSurfaceFormat | SDL_SRCALPHA;
		Image = SDL_ConvertSurface(img,&fmt,flags);
	}
	else {
		// Remove the alpha flag, just for sure...
		// TODO: is this enough?
		//	if it has an Amask!=0, perhaps it ignores this flag
		//	perhaps better solution (if this isn't enough; needs more testing):
		//		like withalpha, copy videosurf->format
		//		TODO: what should we do with alpha-data? just set everything to Amask?
		img->flags &= ~SDL_SRCALPHA;

		Image = SDL_DisplayFormat(img);
	}

	SDL_FreeSurface(img);

	
	if(!Image) {
		printf("ERROR: LoadImgBPP: cannot create new surface\n");
		return NULL;
	}
	
	return Image;
}



///////////////////
// Load a sample
SoundSample* CCache::LoadSample(const std::string& _file, int maxplaying)
{
	Type = CCH_SOUND;
	Filename = _file;
	
	std::string fullfname = GetFullFileName(Filename);
	if(fullfname.size() == 0)
	{
//		SetError("Error loading sample %s: file not found", _file.c_str());
		return NULL;
	}
	
	// Load the sample
	Sample = LoadSoundSample(fullfname, maxplaying);
	
//	if(Sample)
//		Used = true;
//	else
//		SetError("Error loading sample: %s",_file.c_str());

	return Sample;
}


///////////////////
// Shutdown the cache item
void CCache::Shutdown(void)
{
	//if(!Used)
	//	return;

	switch(Type) {

		// Image
		case CCH_IMAGE:
			if(Image)
				SDL_FreeSurface(Image);
			Image = NULL;
			break;

		// Sample
		case CCH_SOUND:
			if(Sample)
				FreeSoundSample(Sample);
			Sample = NULL;
			break;
	}
}




//////////////////////////////////////
//	      The cache system
//////////////////////////////////////



///////////////////
// Initialize the cache
int InitializeCache(void)
{
	// Allocate the cache
	Cache.clear();

	return true;
}


///////////////////
// Shutdown the cache
void ShutdownCache(void)
{
	if(Cache.empty())
		return;


	for (std::vector<CCache>::iterator it=Cache.begin(); it != Cache.end(); it++)  {
		it->Shutdown();
	}
}


