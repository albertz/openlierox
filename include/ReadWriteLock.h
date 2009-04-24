/*
 *	simple ReadWriteLock, implemented by using mutex's
 *
 *	by Albert Zeyer,  code under LGPL
*/

#ifndef __READWRITELOCK_H__
#define __READWRITELOCK_H__

#include <SDL.h>
#include "ThreadPool.h"
#include "Debug.h"

class ReadWriteLock {
private:
	SDL_mutex* mutex;
	unsigned int readCounter;
	unsigned short writerWaitingFlag;

public:
	ReadWriteLock() {
		readCounter = 0;
		writerWaitingFlag = 0;
		mutex = SDL_CreateMutex();
	}

	~ReadWriteLock() {
		if(readCounter)
			warnings("destroying ReadWriteLock with positive readCounter!\n");
		SDL_DestroyMutex(mutex);
	}

	void startReadAccess() {
		SDL_mutexP(mutex);

		// wait for any writer in the queue
		while(writerWaitingFlag) {
		    SDL_mutexV(mutex);
            SDL_Delay(1);
		    SDL_mutexP(mutex);
		}

		readCounter++;
		SDL_mutexV(mutex);
	}

	void endReadAccess() {
		SDL_mutexP(mutex);
		readCounter--;
		SDL_mutexV(mutex);
	}

	void startWriteAccess() {
		SDL_mutexP(mutex);
		// wait for other writers
		while(writerWaitingFlag) {
		    SDL_mutexV(mutex);
            SDL_Delay(1);
		    SDL_mutexP(mutex);
		}
        writerWaitingFlag = 1;

        // wait for other readers
        while(readCounter) {
            SDL_mutexV(mutex);
            SDL_Delay(1);
            SDL_mutexP(mutex);
        }
	}

	void endWriteAccess() {
        writerWaitingFlag = 0;
		SDL_mutexV(mutex);
	}

};

// General scoped lock for SDL_Mutex
class ScopedLock {
private:
	SDL_mutex* data_mutex;
	// Non-copyable
	ScopedLock( const ScopedLock & ) : data_mutex(NULL) { assert(false); };
	ScopedLock & operator= ( const ScopedLock & ) { assert(false); return *this; };

public:
	ScopedLock( SDL_mutex* mutex ): data_mutex(mutex) {
		SDL_mutexP(data_mutex);
		// It is safe to call SDL_mutexP()/SDL_mutexV() on a mutex several times
		// HINT to the comment: it's not only safe, it's the meaning of it; in the case it is called twice,
		// it locks until there is a SDL_mutexV. But *always* call SDL_mutexV from the same thread which has
		// called SDL_mutexP before (else you get serious trouble). Also never call SDL_mutexV when there
		// was no SDL_mutexP before.
	}

	~ScopedLock() {
		SDL_mutexV(data_mutex);
	}

	SDL_mutex* getMutex() { return data_mutex; };	// For usage with SDL_CondWait(), DON'T call SDL_mutexP( lock.getMutex() );
};

class ScopedReadLock {
private:
	ReadWriteLock& lock;
	
	// Non-copyable
	ScopedReadLock( const ScopedReadLock& l ) : lock(l.lock) { assert(false); };
	ScopedReadLock & operator= ( const ScopedReadLock & ) { assert(false); return *this; };
	
public:
	ScopedReadLock( ReadWriteLock& l ): lock(l) { l.startReadAccess(); }
	~ScopedReadLock() { lock.endReadAccess(); }	
};


// TODO: add ReadScopedLock and WriteScopedLock classes for ReadWriteLock

#endif // __READWRITELOCK_H__
