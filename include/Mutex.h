/*
	OpenLieroX

	Mutex wrapper

	created 10-02-2009 by Karel Petranek
	code under LGPL
*/

#ifndef __MUTEX_H__
#define __MUTEX_H__

#include <SDL_mutex.h>
#include "CodeAttributes.h"

#define INVALID_THREAD_ID (Uint32)-1

class Condition;

// Mutex wrapper class with some extra debugging checks
class Mutex : DontCopyTag {
	friend class Condition;
private:
	SDL_mutex *m_mutex;

#ifdef DEBUG
	volatile unsigned long m_lockedThread;  // Thread that keeps the lock
#endif

public:
#ifdef DEBUG
	Mutex();
	~Mutex();
	void lock();
	void unlock();

	static void test();
#else
	Mutex()			{ m_mutex = SDL_CreateMutex(); }
	~Mutex()		{ if(m_mutex) SDL_DestroyMutex(m_mutex); }
	void lock()		{ SDL_LockMutex(m_mutex); }
	void unlock()	{ SDL_UnlockMutex(m_mutex); }
#endif
	
	struct ScopedLock : DontCopyTag {
		Mutex& mutex;
		ScopedLock(Mutex& m) : mutex(m) { mutex.lock(); }
		~ScopedLock() { mutex.unlock(); }
	};

	struct ScopedUnlock : DontCopyTag {
		Mutex& mutex;
		ScopedUnlock(Mutex& m) : mutex(m) { mutex.unlock(); }
		~ScopedUnlock() { mutex.lock(); }
	};
};

#endif // __MUTEX_H__
