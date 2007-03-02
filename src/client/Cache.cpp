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
#include "GfxPrimitives.h"


CCache		*Cache = NULL;



//////////////////////////////////////
//	      The cache item class
//////////////////////////////////////


///////////////////
// Load an image
SDL_Surface *_LoadImage(const std::string& filename)
{
    /*SDL_Surface *psSurf = NULL;
    Uint32 Rmask, Gmask, Bmask, Amask;
    Rmask = Gmask = Bmask = Amask = 0;

    if ( SDL_BYTEORDER == SDL_LIL_ENDIAN ) {
	    Rmask = 0x000000FF;
		Gmask = 0x0000FF00;
		Bmask = 0x00FF0000;
		Amask = 0xFF000000;
	} else {	    
		Rmask = 0xFF000000;
		Gmask = 0x00FF0000;
		Bmask = 0x0000FF00;
		Amask = 0x000000FF;
	}

    // Load the image (32bpp)
	corona::Image* image = corona::OpenImage(filename,corona::FF_AUTODETECT,corona::PF_R8G8B8A8);

	if(!image)
		return NULL;

    
    psSurf = SDL_CreateRGBSurface(iSurfaceFormat, image->getWidth(), image->getHeight(), 32, Rmask, Gmask, Bmask, Amask);
    if( !psSurf )
        return NULL;

    // Copy the data over
    memcpy( psSurf->pixels, image->getPixels(), psSurf->w*psSurf->h*4);

    delete image;

    return psSurf;*/

	std::string fname = GetFullFileName(filename);
	if(fname.size() == 0)
		return NULL;
		
#ifndef WIN32
	struct stat s;
	if(stat(fname.c_str(), &s) == 0 && S_ISREG(s.st_mode)) {
//		printf("_LoadImage(%s): %0.1f kBytes\n", fname, s.st_size / 1024.0f);
		return IMG_Load(fname.c_str());
	}
	else {
//		printf("_LoadImage(%s): ERROR: cannot stat the file\n", fname);
		return NULL;
	}
#else // WIN32
	return IMG_Load(fname.c_str());
#endif
}



///////////////////
// Loads an image, and converts it to the same colour depth as the screen (speed)
SDL_Surface *CCache::LoadImgBPP(const std::string& _file, bool withalpha) {
	SDL_Surface *img;

	Type = CCH_IMAGE;
	Filename = _file;

	// Load the image
	img = _LoadImage(Filename);

	if(!img) {
//		printf("CCache::LoadImgBPP: Error loading file: %s\n", Filename.c_str());
		return NULL;
	}

	// Convert the image to the screen's colour depth
	
	if(withalpha)
		Image = gfxCreateSurfaceAlpha(img->w, img->h);
	else
		Image = gfxCreateSurface(img->w, img->h);
	
	if(!Image) {
		printf("ERROR: LoadImgBPP: cannot create new surface\n");
		SDL_FreeSurface(img);
		return NULL;
	}
	
	// Blit it onto the new surface
	SDL_BlitSurface(img,NULL,Image,NULL);
	SDL_FreeSurface(img);

	Used = true;

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
	
	if(Sample)
		Used = true;
//	else
//		SetError("Error loading sample: %s",_file.c_str());

	return Sample;
}


///////////////////
// Shutdown the cache item
void CCache::Shutdown(void)
{
	if(!Used)
		return;

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
	Cache = new CCache[MAX_CACHE];
	if(Cache == NULL) {
		SystemError("Error: InitializeCache(): Out of memory");
		return false;
	}

	return true;
}


///////////////////
// Shutdown the cache
void ShutdownCache(void)
{
	int n;

	if(Cache == NULL)
		return;


	// Go through the cache and shutdown every item
	for(n=0;n<MAX_CACHE;n++) {
		if(!Cache[n].isUsed())
			continue;

		Cache[n].Shutdown();
	}

	// Free the cache class
	delete[] Cache;
	Cache = NULL;
}



///////////////////
// Load an image
SDL_Surface *LoadImage(const std::string& _filename, bool withalpha)
{
	int n;
	CCache *cach;

	// Go through and see if we can find the same image already loaded
	cach = Cache;
	for(n=0;n<MAX_CACHE;n++,cach++) {
		if(cach->isUsed() && cach->getType() == CCH_IMAGE) {
			if(stringcasecmp(cach->getFilename(),_filename) == 0)
				return cach->GetImage();
		}
	}

	// Didn't find one already loaded? Find a free spot and load one
	cach = Cache;
	for(n=0;n<MAX_CACHE;n++,cach++) {
		if(cach->isUsed())
			continue;
		break;
	}

	// Not enough free spaces
	if(n == MAX_CACHE-1) {
		printf("Error: The cache ran out of free spaces\n");
		return NULL;
	}


	// Load the image
	return cach->LoadImgBPP(_filename, withalpha);
}


///////////////////
// Load a sample
SoundSample* LoadSample(const std::string& _filename, int maxplaying)
{
	int n;
	CCache *cach;

	// Go through and see if we can find the same sound already loaded
	cach = Cache;
	for(n=0;n<MAX_CACHE;n++,cach++) {
		if(cach->isUsed() && cach->getType() == CCH_SOUND) {
			if(stringcasecmp(cach->getFilename(),_filename) == 0)
				return cach->GetSample();
		}
	}

	// Didn't find one already loaded? Find a free spot and load one
	cach = Cache;
	for(n=0;n<MAX_CACHE;n++,cach++) {
		if(cach->isUsed())
			continue;
		break;
	}

	// Not enough free spaces
	if(n == MAX_CACHE-1) {
		printf("Error: The cache ran out of free spaces\n");
		return 0;
	}

	return cach->LoadSample(_filename,maxplaying);
}
