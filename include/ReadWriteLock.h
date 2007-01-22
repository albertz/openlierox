#ifndef __READWRITELOCK_H__
#define __READWRITELOCK_H__

/*
 *	simple ReadWriteLock, implemented by using mutex's
 *
 *	by Albert Zeyer, code under LGPL
*/

#include <SDL/SDL_thread.h>

class ReadWriteLock {
private:
	SDL_mutex* writeMutex;
	SDL_mutex* readCounterMutex;
	unsigned int readCounter;
		
public:
	ReadWriteLock() {
		readCounter = 0;
        writeMutex = SDL_CreateMutex();
		readCounterMutex = SDL_CreateMutex();
	}
	
	~ReadWriteLock() {		
		if(readCounter)
			printf("WARNING: destroying ReadWriteLock with positive readCounter!\n");
		SDL_DestroyMutex(readCounterMutex);
		SDL_DestroyMutex(writeMutex);	
	}
	
	inline void startReadAccess() {
		if(SDL_mutexP(readCounterMutex)<0) assert(false);
		if(readCounter == 0) if(SDL_mutexP(writeMutex)<0) assert(false);
		readCounter++;		
		SDL_mutexV(readCounterMutex);
	}
	
	inline void endReadAccess() {
		if(SDL_mutexP(readCounterMutex)<0) assert(false);
		readCounter--;
		if(readCounter == 0) SDL_mutexV(writeMutex);
		SDL_mutexV(readCounterMutex);
	}

	inline void startWriteAccess() {
		if(SDL_mutexP(writeMutex)<0) assert(false);
	}

	inline void endWriteAccess() {
		SDL_mutexV(writeMutex);	
	}
	
};

#endif // __READWRITELOCK_H__
