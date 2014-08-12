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
#include "Debug.h"
#include "sound/SoundsBase.h"
#include "Cache.h"
#include "GfxPrimitives.h"
#include "FindFile.h"
#include "CGameScript.h"
#include "game/CMap.h"
#include "StringUtils.h"
#include "Timer.h"
#include "Options.h"
#include "AuxLib.h"


#ifdef DEBUG
Timer *timer = NULL;

void CacheTimer(Timer::EventData data)  {
	static size_t last_size = 0;

	std::string unit = "B";
	size_t size = cCache.GetCacheSize();
	if (size != last_size)  {
		last_size = size;

		if (size >= 1024 * 1024 * 1024)  {
			size /= 1024 * 1024 * 1024;
			unit = "GB";
		} else if (size >= 1024 * 1024)  {
			size /= 1024 * 1024;
			unit = "MB";
		} else if (size >= 1024)  {
			size /= 1024;
			unit = "KB";
		}

		notes << "Current cache size: " << size << " " << unit << endl;
	}
}

void InitCacheDebug()
{
	timer = new Timer("cache", getEventHandler(&CacheTimer), NULL, 10000);
	timer->start();
}

void ShutdownCacheDebug()
{
	timer->stop();
	delete timer;
}
#endif

CCache cCache;
static AbsTime getCurrentTime()
{
	if( tLX == NULL )	// Cache is used before tLX is initialized
		return GetTime();
	return tLX->currentTime;
}

//////////////
// Save an image to the cache
void CCache::SaveImage__unsafe(const std::string& file1, const SmartPointer<SDL_Surface> & img)
{
	if (img.get() == NULL)
		return;

	std::string file = file1;
	stringlwr(file);
	if( ImageCache.find(file) != ImageCache.end() )	// Error - already in cache
	{
		errors << "Error: image already in cache - memleak: " << file << endl;
		return;
	}
	//notes << "CCache::SaveImage(): " << img << " " << file << endl;
	ImageCache[file] = ImageItem_t(img, getCurrentTime(), 0);
}

//////////////
// Save a sound sample to the cache
void CCache::SaveSound(const std::string& file1, const SmartPointer<SoundSample> & smp)
{
	ScopedLock lock(mutex);
	if (smp.get() == NULL)
		return;

	std::string file = file1;
	stringlwr(file);
	if( SoundCache.find(file) != SoundCache.end() )
	{
		errors << "Error: sound already in cache - memleak: " << file << endl;
		return;
	};
	SoundCache[file] = SoundItem_t( smp, getCurrentTime(), 0 );
}

//////////////
// Save a map to the cache
void CCache::SaveMap(const std::string& file1, CMap *map)
{
	{
		ScopedLock lock(mutex);
		if (map == NULL)
			return;

		std::string file = file1;
		stringlwr(file);
		if( MapCache.find(file) != MapCache.end() )	// Error - already in cache
		{
			errors << "Error: map already in cache: " << file << endl;
			return;
		}

		// Copy the map to the cache (not just the pointer because map changes during the game)
		SmartPointer<CMap> cached_map = new CMap;
		if (cached_map.get() == NULL)
			return;

		if (!cached_map->NewFrom(map))
			return;

		struct stat st;
		StatFile(file1, &st);
		MapCache[file] = MapItem_t( cached_map, getCurrentTime(), st.st_mtime );
	}
	ClearExtraEntries(); // Cache can get very big when browsing through levels - clear it here
}

//////////////
// Save a mod to the cache
void CCache::SaveMod(const std::string& file1, const SmartPointer<CGameScript> & mod)
{
	if(mod.get() == NULL) {
		errors << "SaveMod: tried to safe NULL gamescript" << endl;
		return;
	}
	
	if(!mod->isLoaded()) {
		errors << "SaveMod: tried to safe non-loaded gamescript" << endl;
		return;
	}
	
	// dont save gus mods
	if(mod->gusEngineUsed()) return;
	
	std::string file = file1;
	stringlwr(file);

	ScopedLock lock(mutex);
	if( ModCache.find(file) != ModCache.end() )	// Error - already in cache
	{
		errors << "Error: mod already in cache - memleak: " << file << endl;
		return;
	}

	struct stat st;
	StatFile(file1, &st);
	ModCache[file] = ModItem_t( mod, getCurrentTime(), st.st_mtime );
}

//////////////
// Get an image from the cache
SmartPointer<SDL_Surface> CCache::GetImage__unsafe(const std::string& file1)
{
	std::string file = file1;
	stringlwr(file);
	ImageCache_t::iterator item = ImageCache.find(file);
	if(item != ImageCache.end())
	{
		item->second.fSaveTime = getCurrentTime();
		return item->second.bmpSurf;
	}
	return NULL;
}

//////////////
// Get a sound sample from the cache
SmartPointer<SoundSample> CCache::GetSound(const std::string& file1)
{
	ScopedLock lock(mutex);
	std::string file = file1;
	stringlwr(file);
	SoundCache_t::iterator item = SoundCache.find(file);
	if(item != SoundCache.end())
	{
		item->second.fSaveTime = getCurrentTime();
		return item->second.sndSample;
	};
	return NULL;
}

//////////////
// Get a map from the cache
SmartPointer<CMap> CCache::GetMap(const std::string& file1)
{
	ScopedLock lock(mutex);
	std::string file = file1;
	stringlwr(file);
	MapCache_t::iterator item = MapCache.find(file);
	if(item != MapCache.end())
	{
		// If the file has changed, don't consider it as found and erase it from the cache
		struct stat st;
		StatFile(item->first, &st);
		if (item->second.iFileTimeStamp != (Uint64)st.st_mtime)  {
			item->second.tMap = NULL;
			MapCache.erase(item);
			return NULL;
		}

		item->second.fSaveTime = getCurrentTime();
		return item->second.tMap;
	}
	return NULL;
}

//////////////
// Get a mod from the cache
SmartPointer<CGameScript> CCache::GetMod(const std::string& file1)
{
	ScopedLock lock(mutex);
	std::string file = file1;
	stringlwr(file);
	ModCache_t::iterator item = ModCache.find(file);
	if(item != ModCache.end())
	{
		// If the file has changed, don't consider it as found and erase it from the cache
		struct stat st;
		StatFile(item->first, &st);
		if (item->second.iFileTimeStamp != (Uint64)st.st_mtime)  {
			item->second.tMod = NULL;
			ModCache.erase(item);
			return NULL;
		}

		item->second.fSaveTime = getCurrentTime();
		return item->second.tMod;
	};
	return NULL;
}

//////////////
// Free all allocated data
void CCache::Clear()
{
	ScopedLock lock(mutex);
	ModCache.clear();
	MapCache.clear();
	ImageCache.clear();
	SoundCache.clear();
}

void CCache::ClearSounds() {
	ScopedLock lock(mutex);
	SoundCache.clear();
}


///////////////////////
// Get the number of memory occupied (in bytes)
size_t CCache::GetCacheSize()
{
	ScopedLock lock(mutex);

	size_t res = sizeof(CCache);
	for (ImageCache_t::iterator it = ImageCache.begin(); it != ImageCache.end(); it++)
		res += GetSurfaceMemorySize(it->second.bmpSurf.get()) + it->first.size();

	for (ModCache_t::iterator it = ModCache.begin(); it != ModCache.end(); it++)
		res += it->second.tMod.get()->GetMemorySize() + it->first.size();

	for (MapCache_t::iterator it = MapCache.begin(); it != MapCache.end(); it++)
		res += it->second.tMap.get()->GetMemorySize() + it->first.size();

	for (SoundCache_t::iterator it = SoundCache.begin(); it != SoundCache.end(); it++)
		res += it->first.size() + sizeof(SoundSample) + it->second.sndSample->GetMemorySize();

	return res;
}

size_t CCache::GetEntryCount() {
	ScopedLock lock(mutex);
	
	size_t res = 0;
	res += ImageCache.size();
	res += ModCache.size();
	res += MapCache.size();
	res += SoundCache.size();
	
	return res;
}

void CCache::ClearExtraEntries()
{
	static const int estimatedCacheEntrySize = 1024*300;
	size_t availableMem = GetFreeSysMemory() + GetCacheSize();
	int estimatedMaxPossibleEntries = int(availableMem / estimatedCacheEntrySize);
	if(estimatedMaxPossibleEntries < tLXOptions->iMaxCachedEntries) {
		warnings << "The estimated possible max entry count in cache (" << estimatedMaxPossibleEntries << ") ";
		warnings << "is lower than the current set maximum (" << tLXOptions->iMaxCachedEntries << ")" << endl;
		hints << "Free system memory: " << (GetFreeSysMemory() / 1024) << " KB" << endl; 
		notes << "Lowering cache limit now" << endl;
		tLXOptions->iMaxCachedEntries = estimatedMaxPossibleEntries;
	}

	ScopedLock lock(mutex);
	
	if( (int)MapCache.size() >= tLXOptions->iMaxCachedEntries / 50 )
	{	// Sorted by last-access time, iterators are not invalidated in a map when element is erased
		typedef std::multimap< AbsTime, MapCache_t :: iterator > TimeSorted_t;
		TimeSorted_t TimeSorted;
		int count = 0;
		for( MapCache_t :: iterator it = MapCache.begin(); it != MapCache.end(); it++  )
		{
			TimeSorted.insert( std::make_pair( it->second.fSaveTime, it ) );
			if( it->second.tMap.getRefCount() <= 1 )
				count ++;
		}
		{
			int clearCount = (int)MIN( (size_t)count, MapCache.size() - tLXOptions->iMaxCachedEntries / 50 );
			for( TimeSorted_t :: iterator it1 = TimeSorted.begin();
					it1 != TimeSorted.end() && clearCount > 0; it1++, clearCount-- )
			{
				if( it1->second->second.tMap.tryDeleteData() )
				{
					clearCount --;
					MapCache.erase( it1->second );
				}
			}
		}
	}
	
	// Delete mods first, and images/sounds after, 'cause they are used by mods mainly
	if( (int)ModCache.size() >= tLXOptions->iMaxCachedEntries / 50 )
	{	// Sorted by last-access time, iterators are not invalidated in a map when element is erased
		typedef std::multimap< AbsTime, ModCache_t :: iterator > TimeSorted_t;
		TimeSorted_t TimeSorted;
		int count = 0;
		for( ModCache_t :: iterator it = ModCache.begin(); it != ModCache.end(); it++  )
		{
			TimeSorted.insert( std::make_pair( it->second.fSaveTime, it ) );
			if( it->second.tMod.getRefCount() <= 1 )
				count ++;
		}
		{
			int clearCount = (int)MIN( (size_t)count, ModCache.size() - tLXOptions->iMaxCachedEntries / 50 );
			for( TimeSorted_t :: iterator it1 = TimeSorted.begin();
					it1 != TimeSorted.end() && clearCount > 0; it1++, clearCount-- )
			{
				if( it1->second->second.tMod.tryDeleteData() )
				{
					clearCount --;
					ModCache.erase( it1->second );
				}
			}
		}
	}

	if( (int)ImageCache.size() >= tLXOptions->iMaxCachedEntries )
	{	// Sorted by last-access time, iterators are not invalidated in a map when element is erased
		typedef std::multimap< AbsTime, ImageCache_t :: iterator > TimeSorted_t;
		TimeSorted_t TimeSorted;
		int count = 0;
		for( ImageCache_t :: iterator it = ImageCache.begin(); it != ImageCache.end(); it++  )
		{
			TimeSorted.insert( std::make_pair( it->second.fSaveTime, it ) );
			if( it->second.bmpSurf.getRefCount() <= 1 )
				count ++;
		}
		{
			int clearCount = (int)MIN( (size_t)count, ImageCache.size() - tLXOptions->iMaxCachedEntries );
			for( TimeSorted_t :: iterator it1 = TimeSorted.begin();
					it1 != TimeSorted.end() && clearCount > 0; it1++ )
			{
				if( it1->second->second.bmpSurf.tryDeleteData() )
				{
					clearCount --;
					ImageCache.erase( it1->second );
				};
			};
		};
	};

	if( (int)SoundCache.size() >= tLXOptions->iMaxCachedEntries )
	{	// Sorted by last-access time, iterators are not invalidated in a map when element is erased
		typedef std::multimap< AbsTime, SoundCache_t :: iterator > TimeSorted_t;
		TimeSorted_t TimeSorted;
		int count = 0;
		for( SoundCache_t :: iterator it = SoundCache.begin(); it != SoundCache.end(); it++  )
		{
			TimeSorted.insert( std::make_pair( it->second.fSaveTime, it ) );
			if( it->second.sndSample.getRefCount() <= 1 )
				count ++;
		}
		{
			int clearCount = (int)MIN( (size_t)count, SoundCache.size() - tLXOptions->iMaxCachedEntries );
			for( TimeSorted_t :: iterator it1 = TimeSorted.begin();
					it1 != TimeSorted.end() && clearCount > 0; it1++, clearCount-- )
			{
				if( it1->second->second.sndSample.tryDeleteData() )
				{
					clearCount --;
					SoundCache.erase( it1->second );
				}
			}
		}
	}
}

