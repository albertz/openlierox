/*
	OpenLieroX
	code under LGPL
	27-12-2007 Albert Zeyer
*/

#ifndef __SMARTPOINTER_H__
#define __SMARTPOINTER_H__

#include <limits.h>
#include <assert.h>
#include <SDL_thread.h>
#include "Utils.h"

template < typename _Type, typename _SpecificInitFunctor >
class SmartPointer;

/*
	standard smartpointer based on simple refcounting
	
	The refcounting is multithreading safe in this class,
	you can have copies of this object in different threads.
	Though it's not designed to operate with the same
	object in different threads. Also there is absolutly no
	thread safty on the pointer itself, you have to care
	about this yourself.
*/
template < typename _Type, typename _SpecificInitFunctor = NopFunctor<void*> >
class SmartPointer {
private:
	_Type* obj;
	int* refCount;
	SDL_mutex* mutex;

	void init(_Type* newObj) {
		if(!mutex) {
			mutex = SDL_CreateMutex();
			lock();
			obj = newObj;
			refCount = new int;
			*refCount = 1;
			unlock();
		}
	}
	
	void uninitObject() {
		delete obj; delete refCount;
 		SDL_DestroyMutex(mutex);
	}

	void reset() {
		if(mutex) {
			lock();
			(*refCount)--;
			if(*refCount == 0) {
				unlock();
				uninitObject();
		 	} else
		 		unlock();
		}
		obj = NULL;
		refCount = NULL;
		mutex = NULL;
	}
	
	void incCounter() {
		assert(*refCount > 0 && *refCount < INT_MAX);
		(*refCount)++;
	}
	
	void lock() { SDL_mutexP(mutex); }
	void unlock() { SDL_mutexV(mutex); }

public:
	SmartPointer() : obj(NULL), refCount(NULL), mutex(NULL) { _SpecificInitFunctor()(this); }
	~SmartPointer() { reset(); }
	
	template <typename _OtherSmartPointer>
	SmartPointer(const _OtherSmartPointer& pt) { operator=(pt); }
	template <typename _OtherSmartPointer>
	SmartPointer& operator=(const _OtherSmartPointer& pt) {
		if(mutex == pt.mutex) return; // ignore this case
		reset();
		mutex = pt.mutex;
		if(mutex) {
			lock();
			obj = pt.obj; refCount = pt.refCount;
			incCounter();
			unlock();
		} else { obj = NULL; refCount = NULL; }
		return *this;
	}
	
	SmartPointer(_Type* pt) { operator=(pt); }
	SmartPointer& operator=(_Type* pt) {
		assert(pt != NULL);
		if(obj == pt) return *this; // ignore this case
		reset();
		init(pt);
		return *this;
	}

	_Type* get() { return obj; }
	const _Type* get() const { return obj; }
};

#endif
