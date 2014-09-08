/*
	Condition wrapper
	
	OpenLieroX
	
	code under LGPL
	created 11-05-2009
*/

#ifndef __OLX__CONDITION_H__
#define __OLX__CONDITION_H__

#include <SDL_mutex.h>
#include "Mutex.h"

class Condition {
private:
	SDL_cond* cond;
public:
	Condition() { cond = SDL_CreateCond(); }
	~Condition() { SDL_DestroyCond(cond); cond = NULL; }
	
	void signal() { SDL_CondSignal(cond); }
	void broadcast() { SDL_CondBroadcast(cond); }
#ifdef DEBUG
	void wait(Mutex& mutex);
	bool wait(Mutex& mutex, Uint32 ms);
#else
	void wait(Mutex& mutex) { SDL_CondWait(cond, mutex.m_mutex); }
	bool wait(Mutex& mutex, Uint32 ms) { return SDL_CondWaitTimeout(cond, mutex.m_mutex, ms) != SDL_MUTEX_TIMEDOUT; }
#endif
};

#endif
