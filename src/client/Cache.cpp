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
#include "StringUtils.h"

CCache cCache;
float getCurrentTime()
{
	if( tLX == NULL )	// Cache is used before tLX is initialized
		return 0;
	return tLX->fCurTime;
};

//////////////
// Save an image to the cache
CachedDataPointer<SDL_Surface> CCache::SaveImage(const std::string& file1, SDL_Surface *img)
{
	if( ! tLXOptions )
		return CachedDataPointer<SDL_Surface>(img);
	if( tLXOptions->iMaxCachedEntries == 0 )
		return CachedDataPointer<SDL_Surface>(img);	// Cache is disabled
	if (img == NULL)
		return CachedDataPointer<SDL_Surface>(img);

	std::string file = file1;
	stringlwr(file);
	if( ImageCache.find(file) != ImageCache.end() )	// Error - already in cache
	{
		printf("Error: image %s already in cache - memleak\n", file.c_str() );
		return CachedDataPointer<SDL_Surface>(img);
	};
	ImageCache[file] = img;

	ImageCache_ForRemoval[img] = ImageCache.find(file);
	TimeLastAccessed[ std::make_pair( CT_Image, img ) ] = getCurrentTime();
	ResourceRefCount[ std::make_pair( CT_Image, img ) ] = 0;
	ResourcesNotUsedCount ++;	// Decreased when we're creating CachedDataPointer
	return CachedDataPointer<SDL_Surface>(img);
}

//////////////
// Save a sound sample to the cache
CachedDataPointer<SoundSample> CCache::SaveSound(const std::string& file1, SoundSample *smp)
{
	if( ! tLXOptions )
		return CachedDataPointer<SoundSample>(smp);
	if( tLXOptions->iMaxCachedEntries == 0 )
		return CachedDataPointer<SoundSample>(smp);	// Cache is disabled
	if (smp == NULL)
		return CachedDataPointer<SoundSample>(smp);

	std::string file = file1;
	stringlwr(file);
	if( SoundCache.find(file) != SoundCache.end() )
	{
		printf("Error: sound %s already in cache - memleak\n", file.c_str() );
		return CachedDataPointer<SoundSample>(smp);
	};
	SoundCache[file] = smp;

	SoundCache_ForRemoval[smp] = SoundCache.find(file);
	TimeLastAccessed[ std::make_pair( CT_Sound, smp ) ] = getCurrentTime();
	ResourceRefCount[ std::make_pair( CT_Sound, smp ) ] = 0;
	ResourcesNotUsedCount ++;	// Decreased when we're creating CachedDataPointer
	return CachedDataPointer<SoundSample>(smp);
}

//////////////
// Save a map to the cache
void CCache::SaveMap(const std::string& file1, CMap *map)
{
	if( ! tLXOptions )
		return;
	if( tLXOptions->iMaxCachedEntries == 0 )
		return;	// Cache is disabled
	if (map == NULL)
		return;

	std::string file = file1;
	stringlwr(file);
	if( MapCache.find(file) != MapCache.end() )	// Error - already in cache
	{
		printf("Error: map %s already in cache - memleak\n", file.c_str() );
		return;
	};

	// Copy the map to the cache (not just the pointer because map changes during the game)
	CMap *cached_map = new CMap;
	if (cached_map == NULL)
		return;

	if (!cached_map->NewFrom(map))
		return;

	MapCache[file] = cached_map;

	MapCache_ForRemoval[cached_map] = MapCache.find(file);
	TimeLastAccessed[ std::make_pair( CT_Map, cached_map ) ] = getCurrentTime();
	ResourceRefCount[ std::make_pair( CT_Map, cached_map ) ] = 0;
	ResourcesNotUsedCount ++;	// Map in cache is not used initially
}

//////////////
// Save a mod to the cache
CachedDataPointer<CGameScript> CCache::SaveMod(const std::string& file1, CGameScript *mod)
{
	if( ! tLXOptions )
		return CachedDataPointer<CGameScript>(mod);
	if( tLXOptions->iMaxCachedEntries == 0 )
		return CachedDataPointer<CGameScript>(mod);	// Cache is disabled
	if (mod == NULL)
		return CachedDataPointer<CGameScript>(mod);

	std::string file = file1;
	stringlwr(file);
	if( ModCache.find(file) != ModCache.end() )	// Error - already in cache
	{
		printf("Error: mod %s already in cache - memleak\n", file.c_str() );
		return CachedDataPointer<CGameScript>(mod);
	};

	ModCache[file] = mod;

	ModCache_ForRemoval[mod] = ModCache.find(file);
	TimeLastAccessed[ std::make_pair( CT_Mod, mod ) ] = getCurrentTime();
	ResourceRefCount[ std::make_pair( CT_Mod, mod ) ] = 0;
	ResourcesNotUsedCount ++;	// Decreased when we're creating CachedDataPointer
	return CachedDataPointer<CGameScript>(mod);
}

//////////////
// Get an image from the cache
CachedDataPointer<SDL_Surface> CCache::GetImage(const std::string& file1)
{
	std::string file = file1;
	stringlwr(file);
	std::map<std::string, SDL_Surface *>::iterator item = ImageCache.find(file);
	if(item != ImageCache.end())
	{
		TimeLastAccessed[ std::make_pair( CT_Image, item->second ) ] = getCurrentTime();
		return CachedDataPointer<SDL_Surface>(item->second);
	};
	return CachedDataPointer<SDL_Surface>(NULL);
}

//////////////
// Get a sound sample from the cache
CachedDataPointer<SoundSample> CCache::GetSound(const std::string& file1)
{
	std::string file = file1;
	stringlwr(file);
	std::map<std::string, SoundSample *>::iterator item = SoundCache.find(file);
	if(item != SoundCache.end())
	{
		TimeLastAccessed[ std::make_pair( CT_Sound, item->second ) ] = getCurrentTime();
		return CachedDataPointer<SoundSample>(item->second);
	};
	return CachedDataPointer<SoundSample>(NULL);
}

//////////////
// Get a map from the cache
CachedDataPointer<CMap> CCache::GetMap(const std::string& file1)
{
	std::string file = file1;
	stringlwr(file);
	std::map<std::string, CMap *>::iterator item = MapCache.find(file);
	if(item != MapCache.end())
	{
		TimeLastAccessed[ std::make_pair( CT_Map, item->second ) ] = getCurrentTime();
		return CachedDataPointer<CMap>(item->second);
	};
	return CachedDataPointer<CMap>(NULL);
}

//////////////
// Get a mod from the cache
CachedDataPointer<CGameScript> CCache::GetMod(const std::string& file1)
{
	std::string file = file1;
	stringlwr(file);
	std::map<std::string, CGameScript *>::iterator item = ModCache.find(file);
	if(item != ModCache.end())
	{
		TimeLastAccessed[ std::make_pair( CT_Mod, item->second ) ] = getCurrentTime();
		return CachedDataPointer<CGameScript>(item->second);
	};
	return CachedDataPointer<CGameScript>(NULL);
}

//////////////
// Free all allocated data
void CCache::Clear()
{
	RecursionFlag = true;

	// Free all the mods first, because they will use images and sounds which should be freed later
	for (std::map<std::string, CGameScript *>::iterator mod = ModCache.begin();
			mod != ModCache.end(); mod++)  {
		mod->second->Shutdown();
		delete mod->second;
	}
	ModCache.clear();

	// Free all the maps
	for (std::map<std::string, CMap *>::iterator map = MapCache.begin();
			map != MapCache.end(); map++)  {
		map->second->Shutdown();
		delete map->second;
	}
	MapCache.clear();

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
	
	
	TimeLastAccessed.clear();
	ResourceRefCount.clear();
	ImageCache_ForRemoval.clear();
	SoundCache_ForRemoval.clear();
	MapCache_ForRemoval.clear();
	ModCache_ForRemoval.clear();
	ResourcesNotUsedCount = 0;
	RecursionFlag = false;
}


void CCache::IncreaseRefCount(SDL_Surface * p)
{
	ResourceRefCount_t :: iterator it = ResourceRefCount.find( std::make_pair( CT_Image, p ) );
	if( it != ResourceRefCount.end() )
	{
		if( it->second == 0 )
			ResourcesNotUsedCount --;
		it->second ++;
		//printf("CCache::IncreaseRefCount %i total %i: %s\n", it->second, ResourcesNotUsedCount, ImageCache_ForRemoval[p]->first.c_str() );
	};
};
void CCache::IncreaseRefCount(SoundSample * p)
{
	ResourceRefCount_t :: iterator it = ResourceRefCount.find( std::make_pair( CT_Sound, p ) );
	if( it != ResourceRefCount.end() )
	{
		if( it->second == 0 )
			ResourcesNotUsedCount --;
		it->second ++;
		//printf("CCache::IncreaseRefCount %i total %i: %s\n", it->second, ResourcesNotUsedCount, SoundCache_ForRemoval[p]->first.c_str() );
	};
};
void CCache::IncreaseRefCount(CMap * p)
{
	ResourceRefCount_t :: iterator it = ResourceRefCount.find( std::make_pair( CT_Map, p ) );
	if( it != ResourceRefCount.end() )
	{
		if( it->second == 0 )
			ResourcesNotUsedCount --;
		it->second ++;
		//printf("CCache::IncreaseRefCount %i total %i: %s\n", it->second, ResourcesNotUsedCount, MapCache_ForRemoval[p]->first.c_str() );
	};
};
void CCache::IncreaseRefCount(CGameScript * p)
{
	ResourceRefCount_t :: iterator it = ResourceRefCount.find( std::make_pair( CT_Mod, p ) );
	if( it != ResourceRefCount.end() )
	{
		if( it->second == 0 )
			ResourcesNotUsedCount --;
		it->second ++;
		//printf("CCache::IncreaseRefCount %i total %i: %s\n", it->second, ResourcesNotUsedCount, ModCache_ForRemoval[p]->first.c_str() );
	};
};

void CCache::DecreaseRefCount(SDL_Surface * p)
{
	ResourceRefCount_t :: iterator it = ResourceRefCount.find( std::make_pair( CT_Image, p ) );
	if( it != ResourceRefCount.end() )
	{
		it->second --;
		if( it->second == 0 )
			ResourcesNotUsedCount ++;
		//printf("CCache::DecreaseRefCount %i total %i: %s\n", it->second, ResourcesNotUsedCount, ImageCache_ForRemoval[p]->first.c_str() );
	};
	ClearExtraEntries();
};
void CCache::DecreaseRefCount(SoundSample * p)
{
	ResourceRefCount_t :: iterator it = ResourceRefCount.find( std::make_pair( CT_Sound, p ) );
	if( it != ResourceRefCount.end() )
	{
		it->second --;
		if( it->second == 0 )
			ResourcesNotUsedCount ++;
		//printf("CCache::DecreaseRefCount %i total %i: %s\n", it->second, ResourcesNotUsedCount, SoundCache_ForRemoval[p]->first.c_str() );
	};
	ClearExtraEntries();
};
void CCache::DecreaseRefCount(CMap * p)
{
	ResourceRefCount_t :: iterator it = ResourceRefCount.find( std::make_pair( CT_Map, p ) );
	if( it != ResourceRefCount.end() )
	{
		it->second --;
		if( it->second == 0 )
			ResourcesNotUsedCount ++;
		//printf("CCache::DecreaseRefCount %i total %i: %s\n", it->second, ResourcesNotUsedCount, MapCache_ForRemoval[p]->first.c_str() );
	};
	ClearExtraEntries();
};
void CCache::DecreaseRefCount(CGameScript * p)
{
	ResourceRefCount_t :: iterator it = ResourceRefCount.find( std::make_pair( CT_Mod, p ) );
	if( it != ResourceRefCount.end() )
	{
		it->second --;
		if( it->second == 0 )
			ResourcesNotUsedCount ++;
		//printf("CCache::DecreaseRefCount %i total %i: %s\n", it->second, ResourcesNotUsedCount, ModCache_ForRemoval[p]->first.c_str() );
	};
	ClearExtraEntries();
};

void CCache::ClearExtraEntries()
{
	if( ! tLXOptions || RecursionFlag )
		return;
	if( ResourcesNotUsedCount < tLXOptions->iMaxCachedEntries || tLXOptions->iMaxCachedEntries == 0 )
		return;
	RecursionFlag = true;
	// Clean up half of the cache at once - cleaning up is quite slow
	// TODO: faster algorithm, and shorter templates
	int clearCount = ResourcesNotUsedCount - tLXOptions->iMaxCachedEntries / 2;
	// Sorted by last-access time, we can use iterators here - they are not invalidated in a map
	// when one element is erased from it
	std::multimap< float, std::pair< CachedType_t, void * > > TimeSorted; 
	for( ResourceRefCount_t :: iterator it = ResourceRefCount.begin();
			it != ResourceRefCount.end(); it++  )
	{
		if( it->second <= 0 )
			TimeSorted.insert( std::make_pair( TimeLastAccessed[it->first], it->first ) );
	};
	printf("CCache::ClearExtraEntries removing %i items, %i total removable\n", clearCount, TimeSorted.size() );
	for( std::multimap< float, std::pair< CachedType_t, void * > > :: iterator it1 = TimeSorted.begin();
			it1 != TimeSorted.end() && clearCount > 0 ; it1++, clearCount-- )
	{
		// Uhh I just love such non-obvious constructs :P
		if( it1->second.first == CT_Image )
		{
			//printf("CCache::ClearExtraEntries removing: %s\n", ImageCache_ForRemoval[(SDL_Surface *)it1->second.second]->first.c_str() );
			gfxFreeSurface((SDL_Surface *)it1->second.second);
			ImageCache.erase( ImageCache_ForRemoval[(SDL_Surface *)it1->second.second] );
			ImageCache_ForRemoval.erase((SDL_Surface *)it1->second.second);
		}
		else if( it1->second.first == CT_Sound )
		{
			//printf("CCache::ClearExtraEntries removing: %s\n", SoundCache_ForRemoval[(SoundSample *)it1->second.second]->first.c_str() );
			FreeSoundSample((SoundSample *)it1->second.second);
			SoundCache.erase( SoundCache_ForRemoval[(SoundSample *)it1->second.second] );
			SoundCache_ForRemoval.erase((SoundSample *)it1->second.second);
		}
		else if( it1->second.first == CT_Map )
		{
			//printf("CCache::ClearExtraEntries removing: %s\n", MapCache_ForRemoval[(CMap *)it1->second.second]->first.c_str() );
			((CMap *)it1->second.second)->Shutdown();
			delete ((CMap *)it1->second.second);
			MapCache.erase( MapCache_ForRemoval[(CMap *)it1->second.second] );
			MapCache_ForRemoval.erase((CMap *)it1->second.second);
		}
		else if( it1->second.first == CT_Mod )
		{
			//printf("CCache::ClearExtraEntries removing: %s\n", ModCache_ForRemoval[(CGameScript *)it1->second.second]->first.c_str() );
			((CGameScript *)it1->second.second)->Shutdown();
			delete ((CGameScript *)it1->second.second);
			ModCache.erase( ModCache_ForRemoval[(CGameScript *)it1->second.second] );
			ModCache_ForRemoval.erase((CGameScript *)it1->second.second);
		}
		else
		{
			printf("CCache::ClearExtraEntries() - corrupted cache, invalid resource type\n");
		};
		TimeLastAccessed.erase( it1->second );
		ResourceRefCount.erase( it1->second );
		ResourcesNotUsedCount --;
	};
	if( clearCount > 0 )
	{
			printf("CCache::ClearExtraEntries() - corrupted cache, removed not all entries\n");
	};
	RecursionFlag = false;
};
