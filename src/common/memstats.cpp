/*
 *  memstats.cpp
 *  OpenLieroX
 *
 *  memstats implementation for memstats.h
 *  
 *  Created by Albert Zeyer on 03.09.09.
 *  code under LGPL
 *
 */

#ifdef MEMSTATS

#include <new>
#include <iostream>
#include <map>
#include <string>
#include "Debug.h"
#include "Mutex.h"

#undef new
#undef delete

typedef std::basic_string<char, std::char_traits<char>, __gnu_cxx::malloc_allocator<char> > String;

static void dbgError(const String& err) {
	printf("memstats error: %s\n", err.c_str());
	RaiseDebugger();
	DumpCallstackPrintf();
	fflush(0);
}

static void dbgMsg(const String& msg) {
	printf("memstats: %s\n", msg.c_str());
	fflush(0);
}

template<typename T>
String IntToStr(T num, int base = 10) {
	String buf;
	if(num < 0) { buf = "-"; num = -num; }
	
	do {	
		buf = "0123456789abcdefghijklmnopqrstuvwxyz"[num % base] + buf;
		num /= base;
	} while(num);
	
	return buf;	
}


size_t findLastPathSep(const String& path) {
	size_t slash = path.rfind('\\');
	size_t slash2 = path.rfind('/');
	if(slash == String::npos)
		slash = slash2;
	else if(slash2 != String::npos)
		slash = std::max(slash, slash2);
	return slash;
}

String GetBaseFilename(const String& filename) {
	size_t p = findLastPathSep(filename);
	if(p == String::npos) return filename;
	return filename.substr(p+1);
}


typedef String FileName;
typedef int Line;
typedef std::pair<FileName,Line> ObjType;
typedef std::pair<ObjType,size_t> AllocInfo;

String ObjTypeAsStr(const ObjType& obj) {
	return obj.first + ":" + IntToStr(obj.second);
}

String AllocInfoAsStr(const AllocInfo& al) {
	return ObjTypeAsStr(al.first) + "#" + IntToStr(al.second);
}

typedef std::map<ObjType, size_t, std::less<ObjType>, __gnu_cxx::malloc_allocator< std::pair<const ObjType, size_t> > > Allocations;
typedef std::map<void*, AllocInfo, std::less<void*>, __gnu_cxx::malloc_allocator< std::pair<void* const, AllocInfo> > > AllocInfoMap;

struct MemStats {
	Mutex mutex;
	Allocations allocSums;
	AllocInfoMap allocInfos;
	
	MemStats() {
		printf("--- MemStats initialisation ---\n");		
	}
};
static MemStats* stats = NULL;
static bool finalCleanup = false;

static bool initMemStats() {
	if(finalCleanup) return false;
	if(stats == NULL) {
		stats = (MemStats*) malloc(sizeof(MemStats));
		new(stats) MemStats;
	}
	return true;
}

struct MemStats_FinalCleanup {
	~MemStats_FinalCleanup() {
		if(stats != NULL) {
			stats -> ~MemStats();
			free(stats);
			stats = NULL;
			finalCleanup = true;
		}
	}
} memStats_finalCleanup;


void * operator new (size_t size, dmalloc_t, const char* file, int line) {
	void* p = malloc(size);
	
	ObjType obj( GetBaseFilename(String(file)), line );
	if(initMemStats()) {
		Mutex::ScopedLock lock(stats->mutex);
		stats->allocSums[obj] += size;
		stats->allocInfos[p] = AllocInfo(obj, size);
	}
	
	return p;
}

void * operator new [] (size_t size, dmalloc_t, const char* file, int line) {
	return :: operator new (size, dmalloc_t(), file, line);
}

void * operator new (size_t size) throw (std::bad_alloc) {
	return :: operator new (size, dmalloc_t(), "??", 0); 
}

void * operator new [] (size_t size) throw (std::bad_alloc) {
	return :: operator new (size, dmalloc_t(), "??", 0); 
}

void* operator new(std::size_t size, const std::nothrow_t&) throw() {
	return :: operator new (size, dmalloc_t(), "??", 0); 	
}

void* operator new[](std::size_t size, const std::nothrow_t&) throw() {
	return :: operator new (size, dmalloc_t(), "??", 0); 	
}

void operator delete (void * p) throw() {
	if(!initMemStats()) {
		free(p);
		return;
	}
	
	Mutex::ScopedLock lock(stats->mutex);

	AllocInfoMap::iterator i = stats->allocInfos.find(p);
	if(i == stats->allocInfos.end()) {
		dbgError("deleting invalid pointer");
		dbgMsg("* pointer: " + IntToStr((int)p));
		if(stats->allocInfos.size() == 0) {
			dbgMsg("* none allocations are made right now");
			return;
		}
		i = stats->allocInfos.lower_bound(p);
		if(i == stats->allocInfos.begin()) {
			dbgMsg("* pointer is before first allocation");
			dbgMsg("* first allocation @" + IntToStr((int)i->first));
			return;
		}
		--i; // go before pointer
		dbgMsg("* below alloc is " + AllocInfoAsStr(i->second) + " @" + IntToStr((int)i->first));
		if((size_t)i->first + i->second.second > (size_t)p)
			dbgMsg("* pointer IS inside that range");
		else
			dbgMsg("* pointer is NOT inside that range");
		return;
	}
	
	const AllocInfo allocInfo = i->second;
	stats->allocInfos.erase(i);
	size_t& allocSum = stats->allocSums[allocInfo.first];
	if(allocSum >= allocInfo.second)
		allocSum -= allocInfo.second;
	else {
		dbgError("delete: allocation info messed up, allocSum too small");
		dbgMsg("* pointer: " + IntToStr((int)p));
		dbgMsg("* allocSum: " + IntToStr(allocSum));
		dbgMsg("* alloc size: " + IntToStr(allocInfo.second));
		allocSum = 0;
	}

	free(p);
}

void operator delete [] (void * p) throw() {
	:: operator delete (p);
}

void operator delete(void* p, const std::nothrow_t&) throw() {
	:: operator delete (p);	
}

void operator delete[](void* p , const std::nothrow_t&) throw() {
	:: operator delete (p);	
}

void printMemStats() {
	if(stats) {
		Mutex::ScopedLock lock(stats->mutex);
		dbgMsg("-- MemStats --");
		dbgMsg("allocs: " + IntToStr(stats->allocInfos.size()));
		dbgMsg("num of different alloc types: " + IntToStr(stats->allocSums.size()));
		// TODO ...
	}
	else
		dbgMsg("MemStats not initialised");
}


#endif

