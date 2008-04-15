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
float getCurrentTime()
{
	if( tLX == NULL )	// Cache is used before tLX is initialized
		return 0;
	return tLX->fCurTime;
};

//////////////
// Save an image to the cache
void CCache::SaveImage(const std::string& file, SDL_Surface *img)
{
	if( tLXOptions->iMaxCachedEntries == 0 )
		return;	// Cache is disabled

	if (img == NULL)
		return;
	ImageCache[file] = img;
	ImageCache_ForRemoval[img] = ImageCache.find(file);

	TimeLastAccessed[ std::make_pair( CT_Image, img ) ] = getCurrentTime();
}

//////////////
// Save a sound sample to the cache
void CCache::SaveSound(const std::string& file, SoundSample *smp)
{
	if( tLXOptions->iMaxCachedEntries == 0 )
		return;	// Cache is disabled

	if (smp == NULL)
		return;
	SoundCache[file] = smp;
	SoundCache_ForRemoval[smp] = SoundCache.find(file);

	TimeLastAccessed[ std::make_pair( CT_Sound, smp ) ] = getCurrentTime();
}

//////////////
// Save a map to the cache
void CCache::SaveMap(const std::string& file, CMap *map)
{
	// TODO: the cache should only save up to ~5 maps, not more (saving mem is important)
	// HINT: this is not as easy as it seems. there are additional checks needed if the
	// map is currently used or not. and please don't add now isUsed vars everywhere, that
	// is no good solutions and only leads to mistakes. this should be done somehow
	// automatically. my current idea is to use smart pointers

	if( tLXOptions->iMaxCachedEntries == 0 )
		return;	// Cache is disabled

	if (map == NULL)
		return;

	// Copy the map to the cache (not just the pointer because map changes during the game)
	CMap *cached_map = new CMap;
	if (cached_map == NULL)
		return;

	if (!cached_map->NewFrom(map))
		return;

	MapCache[file] = cached_map;
	MapCache_ForRemoval[cached_map] = MapCache.find(file);

	TimeLastAccessed[ std::make_pair( CT_Map, cached_map ) ] = getCurrentTime();
}

//////////////
// Save a mod to the cache
void CCache::SaveMod(const std::string& file, CGameScript *mod)
{
	if( tLXOptions->iMaxCachedEntries == 0 )
		return;	// Cache is disabled
	
	if (mod == NULL)
		return;

	CGameScript *cached_mod = new CGameScript;
	if (!cached_mod)
		return;

	cached_mod->CopyFrom(mod);	

	ModCache[file] = cached_mod;
	ModCache_ForRemoval[cached_mod] = ModCache.find(file);

	TimeLastAccessed[ std::make_pair( CT_Mod, cached_mod ) ] = getCurrentTime();
}

//////////////
// Get an image from the cache
SDL_Surface *CCache::GetImage(const std::string& file)
{
	std::map<std::string, SDL_Surface *>::iterator item = ImageCache.find(file);
	if(item != ImageCache.end())
	{
		TimeLastAccessed[ std::make_pair( CT_Image, item->second ) ] = getCurrentTime();
		ResourcesNotUsed.erase( std::make_pair( CT_Image, item->second ) );
		return item->second;
	};
	return NULL;
}

//////////////
// Get a sound sample from the cache
SoundSample *CCache::GetSound(const std::string& file)
{
	std::map<std::string, SoundSample *>::iterator item = SoundCache.find(file);
	if(item != SoundCache.end())
	{
		TimeLastAccessed[ std::make_pair( CT_Sound, item->second ) ] = getCurrentTime();
		ResourcesNotUsed.erase( std::make_pair( CT_Sound, item->second ) );
		return item->second;
	};
	return NULL;
}

//////////////
// Get a map from the cache
CMap *CCache::GetMap(const std::string& file)
{
	std::map<std::string, CMap *>::iterator item = MapCache.find(file);
	if(item != MapCache.end())
	{
		TimeLastAccessed[ std::make_pair( CT_Map, item->second ) ] = getCurrentTime();
		ResourcesNotUsed.erase( std::make_pair( CT_Map, item->second ) );
		return item->second;
	};
	return NULL;
}

//////////////
// Get a mod from the cache
CGameScript *CCache::GetMod(const std::string& dir)
{
	std::map<std::string, CGameScript *>::iterator item = ModCache.find(dir);
	if(item != ModCache.end())
	{
		TimeLastAccessed[ std::make_pair( CT_Mod, item->second ) ] = getCurrentTime();
		ResourcesNotUsed.erase( std::make_pair( CT_Mod, item->second ) );
		return item->second;
	};
	return NULL;
}

//////////////
// Free all allocated data
void CCache::Clear()
{
	// Free all the images
	for (std::map<std::string, SDL_Surface *>::iterator img = ImageCache.begin();
			img != ImageCache.end(); img++)
		gfxFreeSurface(img->second);
	ImageCache.clear();
	
	// Free all the samples
	for (std::map<std::string, SoundSample *>::iterator snd = SoundCache.begin();
			snd != SoundCache.end(); snd++)
		FreeSoundSample(snd->second);
	SoundCache.clear();
	
	// Free all the maps
	for (std::map<std::string, CMap *>::iterator map = MapCache.begin();
			map != MapCache.end(); map++)  {
		map->second->Shutdown();
		delete map->second;
	}
	MapCache.clear();
	
	// Free all the mods
	for (std::map<std::string, CGameScript *>::iterator mod = ModCache.begin();
			mod != ModCache.end(); mod++)  {
		mod->second->Shutdown();
		delete mod->second;
	}
	ModCache.clear();

	TimeLastAccessed.clear();
	ResourcesNotUsed.clear();
	ImageCache_ForRemoval.clear();
	SoundCache_ForRemoval.clear();
	MapCache_ForRemoval.clear();
	ModCache_ForRemoval.clear();
}


bool CCache::ResourceNotUsed(SDL_Surface * p)
{
	bool inCache = (TimeLastAccessed.find( std::make_pair( CT_Image, p ) ) != TimeLastAccessed.end() );
	if(inCache) ResourcesNotUsed.insert( std::make_pair( CT_Image, p ) );
	ClearExtraEntries();
	return inCache;
};

bool CCache::ResourceNotUsed(SoundSample * p)
{
	bool inCache = ( TimeLastAccessed.find( std::make_pair( CT_Sound, p ) ) != TimeLastAccessed.end() );
	if(inCache) ResourcesNotUsed.insert( std::make_pair( CT_Sound, p ) );
	ClearExtraEntries();
	return inCache;
};

bool CCache::ResourceNotUsed(CMap * p)
{
	bool inCache = ( TimeLastAccessed.find( std::make_pair( CT_Map, p ) ) != TimeLastAccessed.end() );
	printf("CCache::ResourceNotUsed(CMap * p) %i\n", inCache);
	if(inCache) ResourcesNotUsed.insert( std::make_pair( CT_Map, p ) );
	ClearExtraEntries();
	return inCache;
};

bool CCache::ResourceNotUsed(CGameScript * p)
{
	bool inCache = ( TimeLastAccessed.find( std::make_pair( CT_Mod, p ) ) != TimeLastAccessed.end() );
	if(inCache) ResourcesNotUsed.insert( std::make_pair( CT_Mod, p ) );
	ClearExtraEntries();
	return inCache;
};

void CCache::ClearExtraEntries()
{
	if( (int)ResourcesNotUsed.size() < tLXOptions->iMaxCachedEntries ) 
		return;
	printf("CCache::ClearExtraEntries()\n");
	// Clean up half of the cache at once - cleaning up is quite slow
	// TODO: faster algorithm, and shorter templates
	int clearCount = ResourcesNotUsed.size() - tLXOptions->iMaxCachedEntries / 2;
	// Sorted by last-access time, we can use iterators here - they are not invalidated in a map
	// when one element is erased from it
	std::map< float, std::pair< CachedType_t, void * > > TimeSorted; 
	for( ResourcesNotUsed_t :: iterator it = ResourcesNotUsed.begin();
			it != ResourcesNotUsed.end(); it++  )
	{
		TimeSorted.insert( std::make_pair( TimeLastAccessed[*it], *it ) );
	};
	for( std::map< float, std::pair< CachedType_t, void * > > :: iterator it1 = TimeSorted.begin();
			it1 != TimeSorted.end() && clearCount > 0 ; it1++, clearCount-- )
	{
		// Uhh I just love such non-obvious constructs :P
		if( it1->second.first == CT_Image )
		{
			SDL_Surface * surf = (SDL_Surface *)it1->second.second;
			gfxFreeSurface(surf);
			ImageCache.erase( ImageCache_ForRemoval[(SDL_Surface *)it1->second.second] );
			ImageCache_ForRemoval.erase((SDL_Surface *)it1->second.second);
		}
		else if( it1->second.first == CT_Sound )
		{
			FreeSoundSample((SoundSample *)it1->second.second);
			SoundCache.erase( SoundCache_ForRemoval[(SoundSample *)it1->second.second] );
			SoundCache_ForRemoval.erase((SoundSample *)it1->second.second);
		}
		else if( it1->second.first == CT_Map )
		{
			((CMap *)it1->second.second)->Shutdown();
			delete ((CMap *)it1->second.second);
			MapCache.erase( MapCache_ForRemoval[(CMap *)it1->second.second] );
			MapCache_ForRemoval.erase((CMap *)it1->second.second);
		}
		else if( it1->second.first == CT_Mod )
		{
			((CGameScript *)it1->second.second)->Shutdown();
			delete ((CGameScript *)it1->second.second);
			ModCache.erase( ModCache_ForRemoval[(CGameScript *)it1->second.second] );
			ModCache_ForRemoval.erase((CGameScript *)it1->second.second);
		};
		TimeLastAccessed.erase( it1->second );
		ResourcesNotUsed.erase( it1->second );
	};
};
