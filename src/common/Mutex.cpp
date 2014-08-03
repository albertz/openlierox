/*
	OpenLieroX

	Mutex wrapper implementation

	created 10-02-2009 by Karel Petranek
	code under LGPL
*/

#include <SDL_thread.h>

#include "Mutex.h"
#include "Debug.h"

#ifdef DEBUG

// TODO: This code is threadsafe and thus not very usable.

/*
Mutex::Mutex()
{
	m_mutex = SDL_CreateMutex(); 
	m_lockedThread = INVALID_THREAD_ID;
}

Mutex::~Mutex()	
{ 
	SDL_DestroyMutex(m_mutex);  

	// If there's a thread set, we're destroying a locked mutex
	if (m_lockedThread != INVALID_THREAD_ID)  {
		errors << "Destroyed a locked mutex" << endl;
		assert(false);
	}
}

void Mutex::lock()		
{
	// Get the current thread ID, if the thread ID is the same as the one already set, we've called
	// lock() twice from the same thread
	Uint32 id = SDL_ThreadID();
	if (m_lockedThread == id)  {
		errors << "Called mutex lock twice from the same thread, this will cause a deadlock" << endl;
		assert(false);
	}

	SDL_LockMutex(m_mutex); 
	m_lockedThread = id;  // We hold the lock now
}

void Mutex::unlock()	
{
	// Make sure the mutex is released from the same thread it was locked in
	if (m_lockedThread != SDL_ThreadID())  {
		errors << "Releasing the mutex in other thread than locking it, this will cause a deadlock" << endl;
		assert(false);
	}
	m_lockedThread = INVALID_THREAD_ID;  // Lock released

	SDL_UnlockMutex(m_mutex); 
}
*/

// Testing stuff
/*
int thread_release_main(void *m)
{
	Mutex *mutex = (Mutex *)m;
	mutex->lock();
	return 0;
}


void Mutex::test()
{
	Mutex mtx;

	// Lock in one thread and unlock in another one
	SDL_Thread *rel = SDL_CreateThread(thread_release_main, "Mutex test", &mtx);
	SDL_WaitThread(rel, NULL);
	mtx.unlock();

	// Deadlock
	mtx.lock();
	mtx.lock();

	//mtx.lock();  // Lock and then destroy
}
*/

#endif
