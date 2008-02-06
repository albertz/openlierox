/*
 *	simple ReadWriteLock, implemented by using mutex's
 *
 *	by Albert Zeyer,  code under LGPL
*/

#ifndef __READWRITELOCK_H__
#define __READWRITELOCK_H__

#include <SDL.h>
#include <SDL_thread.h>

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
			printf("WARNING: destroying ReadWriteLock with positive readCounter!\n");
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

#endif // __READWRITELOCK_H__
