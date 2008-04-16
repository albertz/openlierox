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

// these forward-declaration are needed here
// they will be declared in Sounds.h, CMap.h and CGameScript.h
struct SoundSample;
class CMap;
class CGameScript;

// Ref-counting pointer for CCache - not thread safe, we're doing all cache operations from the main thread.
// It's safe to use it as a plain pointer even if the data is not in cache, like assigning NULL and so on.
// If you want thread safety don't add anything here, protect CCache instance with a mutex.
template< class T >
class CachedDataPointer
{
	public:
	CachedDataPointer( T * _ptr = NULL );
	CachedDataPointer( const CachedDataPointer & _ptr );
	const CachedDataPointer & operator = ( const CachedDataPointer & _ptr );
	const CachedDataPointer & operator = ( T * _ptr );
	~CachedDataPointer();
	T * get() const { return ptr; };
	operator T * () const { return ptr; };
	T * operator -> () const { return ptr; };

	private:
	T * ptr;
};

class CCache  {
public:
	CCache() { ResourcesNotUsedCount = 0; RecursionFlag = false; }
	void Clear();

private:
	std::map<std::string, SDL_Surface *> ImageCache;
	std::map<std::string, SoundSample *> SoundCache;
	std::map<std::string, CMap *> MapCache;
	std::map<std::string, CGameScript *> ModCache;


	enum CachedType_t { CT_Image, CT_Sound, CT_Map, CT_Mod };
	typedef std::map< std::pair< CachedType_t, void * >, float > TimeLastAccessed_t;
	TimeLastAccessed_t TimeLastAccessed;
	typedef std::map< std::pair< CachedType_t, void * >, int > ResourceRefCount_t;
	ResourceRefCount_t ResourceRefCount;
	int ResourcesNotUsedCount;

	// For ClearExtraEntries() funcs - used to convert pointer to the entry in cache
	// TODO: remove them somehow, maybe make another algorithm
	std::map<SDL_Surface *, std::map<std::string, SDL_Surface *> :: iterator> ImageCache_ForRemoval;
	std::map<SoundSample *, std::map<std::string, SoundSample *> :: iterator> SoundCache_ForRemoval;
	std::map<CMap *, std::map<std::string, CMap *> :: iterator> MapCache_ForRemoval;
	std::map<CGameScript *, std::map<std::string, CGameScript *> :: iterator> ModCache_ForRemoval;
	
	bool RecursionFlag;	// To avoid recursion when calling ClearExtraEntries()
	
public:
	CachedDataPointer<SDL_Surface>	GetImage(const std::string& file);
	CachedDataPointer<SoundSample>	GetSound(const std::string& file);
	CachedDataPointer<CMap>			GetMap(const std::string& file);
	CachedDataPointer<CGameScript>	GetMod(const std::string& dir);

	// Images and sounds and mods are stored in cache by reference, 
	// no copying is done, so it is required to use CachedDataPointer returned,
	// if you don't want cache system to delete your image right after you've loaded it.
	// Oh, don't ever call gfxFreeSurface() or FreeSoundSample() - cache will do that for you.
	CachedDataPointer<SDL_Surface>	SaveImage(const std::string& file, SDL_Surface *img);
	CachedDataPointer<SoundSample>	SaveSound(const std::string& file, SoundSample *smp);
	CachedDataPointer<CGameScript>	SaveMod(const std::string& dir, CGameScript *mod);
	// Map is copied to cache, 'cause it will be modified during game - you should free your data yourself.
	void							SaveMap(const std::string& file, CMap *map);
	
private:

	template< class T >
	friend class CachedDataPointer;

	void			ClearExtraEntries();
	
	// Used by CachedDataPointer class
	void			IncreaseRefCount(SDL_Surface *);
	void			IncreaseRefCount(SoundSample *);
	void			IncreaseRefCount(CMap *);
	void			IncreaseRefCount(CGameScript *);

	void			DecreaseRefCount(SDL_Surface *);
	void			DecreaseRefCount(SoundSample *);
	void			DecreaseRefCount(CMap *);
	void			DecreaseRefCount(CGameScript *);
};

extern CCache cCache;

template< class T >
CachedDataPointer<T> :: CachedDataPointer( T * _ptr )
{
	ptr = _ptr;
	cCache.IncreaseRefCount( ptr );
};

template< class T >
CachedDataPointer<T> :: ~CachedDataPointer()
{
	cCache.DecreaseRefCount( ptr );
};

template< class T >
CachedDataPointer<T> :: CachedDataPointer( const CachedDataPointer & _ptr )
{
	ptr = _ptr.ptr;
	cCache.IncreaseRefCount( ptr );
};

template< class T >
const CachedDataPointer<T> & CachedDataPointer<T> :: operator = ( const CachedDataPointer & _ptr )
{
	cCache.DecreaseRefCount( ptr );
	ptr = _ptr.ptr;
	cCache.IncreaseRefCount( ptr );
	return *this;
};

template< class T >
const CachedDataPointer<T> & CachedDataPointer<T> :: operator = ( T * _ptr )
{
	cCache.DecreaseRefCount( ptr );
	ptr = _ptr;
	cCache.IncreaseRefCount( ptr );
	return *this;
};

#endif  //  __CACHE_H__
