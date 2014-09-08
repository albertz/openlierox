/*
	OpenLieroX

	Mutex wrapper implementation

	created 10-02-2009 by Karel Petranek
	code under LGPL
*/

#include <SDL_thread.h>

#include "Mutex.h"
#include "Condition.h"
#include "Debug.h"

#ifdef DEBUG

void Mutex::_lock_pre() {
	// Get the current thread ID, if the thread ID is the same as the one already set, we've called
	// lock() twice from the same thread
	if (m_lockedThread == SDL_ThreadID())  {
		errors << "Called mutex lock twice from the same thread, this will cause a deadlock" << endl;
		assert(false);
	}
}

void Mutex::_lock_post() {
	m_lockedThread = SDL_ThreadID();  // We hold the lock now
}

void Mutex::_unlock_pre() {
	// Make sure the mutex is released from the same thread it was locked in
	if (m_lockedThread != SDL_ThreadID())  {
		errors << "Releasing the mutex in other thread than locking it, this will cause a deadlock" << endl;
		assert(false);
	}
	m_lockedThread = INVALID_THREAD_ID;  // Lock released
}

void Mutex::_unlock_post() {
	// Nothing to do.
}

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
	_lock_pre();
	SDL_LockMutex(m_mutex);
	_lock_post();
}

void Mutex::unlock()	
{
	_unlock_pre();
	SDL_UnlockMutex(m_mutex);
	_unlock_post();
}

void Condition::wait(Mutex& mutex) {
	mutex._unlock_pre();
	mutex._unlock_post();
	SDL_CondWait(cond, mutex.m_mutex);
	mutex._lock_pre();
	mutex._lock_post();
}

bool Condition::wait(Mutex& mutex, Uint32 ms) {
	mutex._unlock_pre();
	mutex._unlock_post();
	bool ret = SDL_CondWaitTimeout(cond, mutex.m_mutex, ms) != SDL_MUTEX_TIMEDOUT;
	mutex._lock_pre();
	mutex._lock_post();
	return ret;
}

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
