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

#define SAFEALLOC_MAXALLOCS (1024 * 1024)
#define SAFEALLOC_POOLSIZE (50 * 1024 * 1024)

struct SafeAlloc {
	char* p;
	size_t s;
	SafeAlloc *left, *right;
	
	SafeAlloc() : p(NULL), s(0), left(NULL), right(NULL) {}
	bool isUsed() const { return p != NULL; }
	void reset() { p = NULL; s = 0; left = NULL; right = NULL; }
	void initPoolMem() { *(SafeAlloc**)p = this; }
	void* userPt() const { return p + extraSpacePerAlloc; }
	size_t userSize() const { return s - SafeAlloc::extraSpacePerAlloc; }

	static const size_t extraSpacePerAlloc = sizeof(SafeAlloc*);
};

static SafeAlloc* safeAllocs = NULL; // sorted list
static char safeAllocs_Mem[SAFEALLOC_MAXALLOCS * sizeof(SafeAlloc)];
static SafeAlloc *safeAlloc_first = NULL;
static SafeAlloc *safeAlloc_lastAlloc = NULL;
static char safeAllocPool[SAFEALLOC_POOLSIZE];
static Mutex* safeAlloc_mutex = NULL;
static char safeAlloc_mutex_Mem[sizeof(Mutex)];

void safeAlloc_init() {
	safeAlloc_mutex = (Mutex*) &safeAlloc_mutex_Mem[0];
	new (safeAlloc_mutex) Mutex;

	safeAllocs = (SafeAlloc*) &safeAllocs_Mem[0];
	for(size_t i = 0; i < SAFEALLOC_MAXALLOCS; ++i)
		new (&safeAllocs[i]) SafeAlloc;
}

void safeAlloc_uninit() {
	for(size_t i = 0; i < SAFEALLOC_MAXALLOCS; ++i)
		safeAllocs[i] . ~SafeAlloc();
	safeAllocs = NULL;
	safeAlloc_mutex -> ~Mutex();
	safeAlloc_mutex = NULL;
}

bool safeAlloc_listEmpty() { return safeAlloc_first == NULL; }

int safeAlloc_allocNum(SafeAlloc* a) { return int(a - &safeAllocs[0]); }

void safeAlloc_dbg(const char* txt, SafeAlloc* a) {
	printf("%s alloc %i @%i (#%i) for %i bytes\n",
		   txt,
		   safeAlloc_allocNum(a),
		   int(a->userPt()),
		   int(a->p - &safeAllocPool[0]),
		   int(a->userSize()));
}

size_t safeAlloc_sanityCheck(SafeAlloc* a) {
	if(a < &safeAllocs[0]) {
		printf("wrong: alloc @%i is left outside array\n", int(a));
		return 0;
	}

	if(a >= &safeAllocs[SAFEALLOC_MAXALLOCS]) {
		printf("wrong: alloc @%i is right outside array\n", int(a));
		return 0;
	}
	
	if(a->isUsed()) {
		if(a->s <= SafeAlloc::extraSpacePerAlloc) printf("wrong: alloc %i has invalid size %i\n", safeAlloc_allocNum(a), a->s);
		if(a->left >= a) printf("wrong: alloc %i has invalid left link\n", safeAlloc_allocNum(a));
		if(a->left == NULL && safeAlloc_first != a) printf("wrong: alloc %i has no left link\n", safeAlloc_allocNum(a));
		if(a->right != NULL && a->right <= a) printf("wrong: alloc %i has invalid right link\n", safeAlloc_allocNum(a));

		// all right followers MUST be used
		size_t allocNum = 1;
		for(SafeAlloc* b = a; ; ) {
			if(!b->isUsed()) {
				printf("wrong: alloc %i has unused right follower %i\n", safeAlloc_allocNum(a), safeAlloc_allocNum(b));
				break;
			}
			if(b->right == NULL) {
				// all next items in array must be unused
				for(SafeAlloc* c = b + 1; c < &safeAllocs[SAFEALLOC_MAXALLOCS]; ++c) {
					if(c->isUsed()) {
						printf("wrong: alloc %i has used past-end %i right follower %i\n", safeAlloc_allocNum(a), safeAlloc_allocNum(b), safeAlloc_allocNum(c));
						break;
					}
				}
				break;
			}
			b = b->right;
			allocNum++;
		}
		return allocNum;
	}
	else {
		if(a->s != 0) printf("wrong: unused alloc %i has size = %i\n", safeAlloc_allocNum(a), a->s);
		if(a->left != NULL) {
			printf("wrong: unused alloc %i has left link\n", safeAlloc_allocNum(a));
			if(a->left >= a) printf("wrong: unused alloc %i has invalid left link\n", safeAlloc_allocNum(a));
		}
		if(a->right != NULL) {
			printf("wrong: unused alloc %i has right link\n", safeAlloc_allocNum(a));
			if(a->right <= a) printf("wrong: unused alloc %i has invalid right link\n", safeAlloc_allocNum(a));
		}
	}

	return 0;
}

void safeAlloc_sanityCheckAll() {
	size_t allocNum = 0;
	for(size_t i = 0; i < SAFEALLOC_MAXALLOCS; ++i) {
		safeAlloc_sanityCheck(&safeAllocs[i]);
		if(safeAllocs[i].isUsed()) allocNum++;
	}

	if(!safeAlloc_listEmpty()) {
		if(!safeAlloc_first->isUsed()) printf("wrong: first alloc %i is unused\n", safeAlloc_allocNum(safeAlloc_first));

		size_t allocNum2 = safeAlloc_sanityCheck(safeAlloc_first);
		if(allocNum != allocNum2) printf("wrong: absolute allocs (%i) is different than linked list allocs (%i)\n", allocNum, allocNum2);
	}
}

typedef std::pair<SafeAlloc*,SafeAlloc*> LRpair;

LRpair safeAlloc_findFreeAt(SafeAlloc* start, size_t n) {
	for(SafeAlloc* a = start; a; a = a->right) {
		SafeAlloc* b = a->right;

		// Note: obsolete security check
		if(!a->isUsed()) {
			printf("SafeAlloc: something is REALLY messed up!\n");
			printf("sanity check of %i:\n", safeAlloc_allocNum(a));
			safeAlloc_sanityCheck(a);
			printf("sanity check everything:\n");
			safeAlloc_sanityCheckAll();
			printf("sanity check first (%i):\n", safeAlloc_allocNum(safeAlloc_first));
			safeAlloc_sanityCheck(safeAlloc_first);
			safeAlloc_dbg("mess a", a);
			return LRpair(a, b); // cannot really recover, so who cares...
		}

		// Note: obsolete security check
		if(b && !b->isUsed()) {
			printf("SafeAlloc: something is rightly messed up!\n");
			safeAlloc_sanityCheck(b);
			safeAlloc_dbg("mess b", b);
			return LRpair(a, b); // cannot really recover, so who cares...
		}
		
		if(a + 1 == b || a + 1 == &safeAllocs[SAFEALLOC_MAXALLOCS])
			// we cannot make a list entry here, so continue searching
			continue;
		
		const char* rightEnd = b ? b->p : &safeAllocPool[SAFEALLOC_POOLSIZE];
		char* leftStart = a->p + a->s;

		// Note: obsolete security check
		if(rightEnd < leftStart) {
			printf("SafeAlloc: something is messed up!\n");
			continue; // doesn't really make sense, but we cannot do anything else
		}

		// Note: obsolete security check
		if(b && b->left != a) {
			printf("SafeAlloc: linked list is messed up!\n");
		}
		
		const size_t neededSize = n + SafeAlloc::extraSpacePerAlloc;
		if(size_t(rightEnd - leftStart) >= neededSize)
			// we have enough place here, return
			return LRpair(a, b);
	}
	
	// nothing free, return invalid pair
	return LRpair(&safeAllocs[SAFEALLOC_MAXALLOCS], NULL);
}

LRpair safeAlloc_findFree(size_t n) {
	if(safeAlloc_listEmpty())
		return LRpair(NULL, NULL);

	LRpair ret = LRpair(&safeAllocs[SAFEALLOC_MAXALLOCS], NULL);
	if(safeAlloc_lastAlloc) {
		ret = safeAlloc_findFreeAt(safeAlloc_lastAlloc, n);
		if(ret.first < &safeAllocs[SAFEALLOC_MAXALLOCS]) return ret;
	}
	
	if(safeAlloc_first > &safeAllocs[0]) {
		const char* rightEnd = safeAlloc_first->p;
		char* leftStart = &safeAllocPool[0];
		
		const size_t neededSize = n + SafeAlloc::extraSpacePerAlloc;
		if(size_t(rightEnd - leftStart) >= neededSize)
			// we have enough place here, return
			return LRpair(NULL, safeAlloc_first);
	}

	return safeAlloc_findFreeAt(safeAlloc_first, n);
}

void* safeAlloc_alloc(size_t n) {
	Mutex::ScopedLock lock(*safeAlloc_mutex);
	LRpair pair = safeAlloc_findFree(n);
	
	if(pair.first == NULL) {
		// everything free or before first entry
		
		if(n + SafeAlloc::extraSpacePerAlloc > SAFEALLOC_POOLSIZE) {
			// no chance
			printf("SafeAlloc alloc: cannot allocate %i bytes, it's more than our whole pool supports\n", n);
			return NULL;
		}
		
		// init list
		safeAllocs[0].reset();
		safeAllocs[0].right = safeAlloc_first; if(safeAlloc_first) safeAlloc_first->left = &safeAllocs[0];
		safeAllocs[0].p = &safeAllocPool[0];
		safeAllocs[0].s = n + SafeAlloc::extraSpacePerAlloc;
		safeAllocs[0].initPoolMem();
		safeAlloc_first = &safeAllocs[0];
		safeAlloc_lastAlloc = safeAlloc_first;
		//safeAlloc_dbg("init first", &safeAllocs[0]);
		return safeAllocs[0].userPt();
	}

	SafeAlloc* a = pair.first + 1;
	if(a < &safeAllocs[SAFEALLOC_MAXALLOCS]) {
		// we fit after pair.first
		
		a->reset();
		a->left = pair.first; pair.first->right = a;
		a->right = pair.second; if(pair.second) pair.second->left = a;
		a->p = a->left->p + a->left->s; // directly next to the left alloc
		a->s = n + SafeAlloc::extraSpacePerAlloc;
		a->initPoolMem();
		safeAlloc_lastAlloc = a;
		//safeAlloc_dbg("init", a);
		return a->userPt();
	}

	// nothing free
	printf("SafeAlloc alloc: cannot allocate %i bytes, nothing free anymore\n", n);
	return NULL;
}

void safeAlloc_free(void* p) {
	Mutex::ScopedLock lock(*safeAlloc_mutex);
	
	if((size_t)p < SafeAlloc::extraSpacePerAlloc) {
		printf("SafeAlloc free: pointer %i is invalid!\n", int(p));
		return;
	}
	
	SafeAlloc** const a = (SafeAlloc**)p - 1;
	if((char*)a < &safeAllocPool[0] || (char*)a + sizeof(SafeAlloc*) - 1 >= &safeAllocPool[SAFEALLOC_POOLSIZE]) {
		printf("SafeAlloc free: pointer %i is not inside pool and thus invalid!\n", int(p));
		return;
	}

	SafeAlloc* const al = *a;
	
	if(al < &safeAllocs[0] || al >= &safeAllocs[SAFEALLOC_MAXALLOCS]) {
		printf("SafeAlloc free: pointer %i is messed up!\n", int(p));
		return;
	}

	if(((size_t)al - (size_t)&safeAllocs[0]) % sizeof(SafeAlloc) != 0) {
		printf("SafeAlloc free: pointer %i is fucked up!\n", int(p));
		return;
	}

	if(!al->isUsed()) {
		printf("SafeAlloc free: pointer %i is strange, perhaps already freed?!\n", int(p));
		return;
	}

	if(safeAlloc_listEmpty()) {
		printf("SafeAlloc free: pointer %i must be wrong, there aren't any allocations!\n", int(p));
		return;
	}
	
	if(al->userPt() != p) {
		printf("SafeAlloc free: pointer %i doesn't behave well!\n", int(p));
		return;
	}

	if(al->left == NULL && safeAlloc_first != al) {
		printf("SafeAlloc free: pointer %i is old!\n", int(p));
		return;
	}

	if(al < safeAlloc_first) {
		printf("SafeAlloc free: pointer %i is wrong, it must point to an old alloc!\n", int(p));
		return;
	}

	if(al == safeAlloc_first && al->left != NULL) {
		printf("SafeAlloc free: pointer %i has wrong linked list info!\n", int(p));
		return;
	}
	
	// everything seems sane
	// clean up now
	//safeAlloc_dbg("free", al);
	if(safeAlloc_lastAlloc == al)
		safeAlloc_lastAlloc = al->left;
	if(al->left == NULL) {
		safeAlloc_first = al->right;
		if(al->right) al->right->left = NULL;
	} else {
		al->left->right = al->right;
		if(al->right) al->right->left = al->left;
	}
	al->reset();
}


  template<typename _Tp>
    class safe_allocator
    {
    public:
      typedef size_t     size_type;
      typedef ptrdiff_t  difference_type;
      typedef _Tp*       pointer;
      typedef const _Tp* const_pointer;
      typedef _Tp&       reference;
      typedef const _Tp& const_reference;
      typedef _Tp        value_type;

      template<typename _Tp1>
        struct rebind
	  { typedef safe_allocator<_Tp1> other; };

	  safe_allocator() throw() { }

	  safe_allocator(const safe_allocator&) throw() { }

      template<typename _Tp1>
        safe_allocator(const safe_allocator<_Tp1>&) throw() { }

      ~safe_allocator() throw() { }

      pointer
      address(reference __x) const { return &__x; }

      const_pointer
      address(const_reference __x) const { return &__x; }

      // NB: __n is permitted to be 0.  The C++ standard says nothing
      // about what the return value is when __n == 0.
      pointer
      allocate(size_type __n, const void* = 0)
      {
        pointer __ret = static_cast<_Tp*>(safeAlloc_alloc(__n * sizeof(_Tp)));
        if (!__ret)
          throw std::bad_alloc();
        return __ret;
      }

      // __p is not permitted to be a null pointer.
      void
      deallocate(pointer __p, size_type)
      { safeAlloc_free(static_cast<void*>(__p)); }

      size_type
      max_size() const throw()
	  { return SAFEALLOC_POOLSIZE - SafeAlloc::extraSpacePerAlloc; }

      // _GLIBCXX_RESOLVE_LIB_DEFECTS
      // 402. wrong new expression in [some_] allocator::construct
      void
      construct(pointer __p, const _Tp& __val)
      { ::new(__p) value_type(__val); }

      void
      destroy(pointer __p) { __p->~_Tp(); }
    };

  template<typename _Tp>
    INLINE bool
    operator==(const safe_allocator<_Tp>&, const safe_allocator<_Tp>&)
    { return true; }

  template<typename _Tp>
    INLINE bool
    operator!=(const safe_allocator<_Tp>&, const safe_allocator<_Tp>&)
    { return false; }


typedef std::basic_string<char, std::char_traits<char>, safe_allocator<char> > String;

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

String SizeAsStr(size_t sum) {
	if(sum < 2*1024)
		return IntToStr(sum) + " B";
	else if(sum < 2*1024*1024)
		return IntToStr(sum / 1024) + " KB";
	else
		return IntToStr(sum / (1024*1024)) + " MB";
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

typedef std::map<ObjType, size_t, std::less<ObjType>, safe_allocator< std::pair<const ObjType, size_t> > > Allocations;
typedef std::map<void*, AllocInfo, std::less<void*>, safe_allocator< std::pair<void* const, AllocInfo> > > AllocInfoMap;

struct MemStats {
	Mutex mutex;
	Allocations allocSums;
	AllocInfoMap allocInfos;
};
static MemStats* stats = NULL;
static bool finalCleanup = false;

static bool initMemStats() {
	if(finalCleanup) return false;
	if(stats == NULL) {
		printf("--- MemStats initialisation ---\n");
		safeAlloc_init();
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
			safeAlloc_uninit();
			finalCleanup = true;
		}
	}
} memStats_finalCleanup;


void * operator new (size_t size, dmalloc_t, const char* file, int line) {
	void* p = malloc(size);
	
	if(initMemStats()) {
		ObjType obj( GetBaseFilename(String(file)), line );

		if(p == NULL) {
			dbgError("out-of-memory @" + ObjTypeAsStr(obj));
			printMemStats();
			throw std::bad_alloc();
		}
		
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
	try {
		return :: operator new (size, dmalloc_t(), "??", 0);
	}
	catch(std::bad_alloc) {
		return NULL;
	}
}

void* operator new[](std::size_t size, const std::nothrow_t&) throw() {
	try {
		return :: operator new (size, dmalloc_t(), "??", 0);
	}
	catch(std::bad_alloc) {
		return NULL;
	}
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
		dbgMsg("-- MemStats --");

		typedef std::multimap<size_t, ObjType, std::less<size_t>, safe_allocator< std::pair<const size_t,ObjType> > > Allocs;
		Allocs allocs;
		size_t sum = 0;
		{
			Mutex::ScopedLock lock(stats->mutex);
			dbgMsg("allocs: " + IntToStr(stats->allocInfos.size()));
			dbgMsg("num of different alloc types: " + IntToStr(stats->allocSums.size()));
			
			// sort all alloc types by alloc sum
			for(Allocations::iterator i = stats->allocSums.begin(); i != stats->allocSums.end(); ++i) {
				allocs.insert( Allocs::value_type(i->second, i->first) );
				sum += i->second;
			}
		}
		
		dbgMsg("allocated mem: " + SizeAsStr(sum));
		int count = 30;
		for(Allocs::reverse_iterator i = allocs.rbegin(); i != allocs.rend(); ++i) {
			dbgMsg(". " + ObjTypeAsStr(i->second) + " - " + SizeAsStr(i->first));
			count--;
			if(count <= 0) break;
		}
		dbgMsg(".");
	}
	else
		// NOTE: we cannot use dbgMsg because this needs the safeAlloc system which is init together with memstats
		printf("* MemStats not initialised *\n");
}


#endif

