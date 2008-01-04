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
class ScopedLock;

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
		//printf("SmartPointer::init    (%10p %10p %10p %10p %3i) newObj %10p\n", this, obj, refCount, mutex, refCount?*refCount:-99, newObj);
		if(!mutex) {
			mutex = SDL_CreateMutex();
			obj = newObj;
			refCount = new int;
			*refCount = 1;
		}
	}
	
	void reset() {
		//printf("SmartPointer::reset   (%10p %10p %10p %10p %3i)\n", this, obj, refCount, mutex, refCount?*refCount:-99);
		if(mutex) {
			lock();
			(*refCount)--;
			if(*refCount == 0) {
				delete obj; delete refCount; // Yay, works!
				obj = NULL;
				refCount = NULL;
				unlock();
				SDL_DestroyMutex(mutex);
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
	
	void lock() { 
		//printf("SmartPointer::lock    (%10p %10p %10p %10p %3i)\n", this, obj, refCount, mutex, refCount?*refCount:-99);
		SDL_mutexP(mutex); 
	}
	void unlock() { 
		//printf("SmartPointer::unlock  (%10p %10p %10p %10p %3i)\n", this, obj, refCount, mutex, refCount?*refCount:-99);
		SDL_mutexV(mutex); 
	}

public:
	SmartPointer() : obj(NULL), refCount(NULL), mutex(NULL) {
		//printf("SmartPointer::construc(%10p %10p %10p %10p %3i)\n", this, obj, refCount, mutex, refCount?*refCount:-99);
		_SpecificInitFunctor()(this);
	}
	~SmartPointer() { 
		//printf("SmartPointer::destruct(%10p %10p %10p %10p %3i)\n", this, obj, refCount, mutex, refCount?*refCount:-99);
		reset(); 
	}
	
	// Default copy constructor and operator=
	// If you specify any template<> params here these funcs will be silently ignored by compiler
	SmartPointer(const SmartPointer& pt) : obj(NULL), refCount(NULL), mutex(NULL) { operator=(pt); }
	SmartPointer& operator=(const SmartPointer& pt) {
		//printf("SmartPointer::op=Ptr  (%10p %10p %10p %10p %3i)\n", this, obj, refCount, mutex, refCount?*refCount:-99);
		if(mutex == pt.mutex) return *this; // ignore this case
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
	
	// WARNING: Be carefull, don't assing a pointer to different SmartPointer objects,
	// else they will get freed twice in the end. Always copy the SmartPointer itself.
	SmartPointer(_Type* pt): obj(NULL), refCount(NULL), mutex(NULL) { operator=(pt); }
	SmartPointer& operator=(_Type* pt) {
		//printf("SmartPointer::op=Type (%10p %10p %10p %3i)\n", obj, refCount, mutex, refCount?*refCount:-99);
		assert(pt != NULL);
		if(obj == pt) return *this; // ignore this case
		reset();
		init(pt);
		return *this;
	}
	
	_Type* get() { return obj; }
	const _Type* get() const { return obj; }

	friend class ScopedLock;
};

/*
	Locks the mutex inside SmartPointer, allowing only one thread to access the data.
	Usage: 
	void func( SmartPointer< SomeData > SomeSmartPointer )
	{
		DoSomething(); 
		ScopedLock lock( SomeSmartPointer ); 
		if( ! SomeSmartPointer.get()->data )
			return; // Unlocks here
		DoSomethingElse( SomeSmartPointer.get()->data )
	} // Auto-unlocks after closing brace
	
	HINT: DON'T USE this, it's wrong. it just locks new copies of the SmartPointer,
	but not the usage of the object itself somewhere else from another already made
	copy of this SmartPointer.
	
	for example:
	global:
		SmartPointer<int> p1(new int(42));
		SmartPointer<int> p2(p1);	
	thread1:
		{
			ScopedLock lock(p1);
			// ACTION1: do something with p1->get()
		}
	thread2:
		{
			ScopedLock lock(p2);
			// ACTION2: do something with p2->get()
		}

	Both ACTION1 and ACTION2 will be run at the same time without any locking.
	
	Sadly, what you tried to implement here (generally a good idea) isn't that easy.
	I think it's better if the object itself which is used is threadsafe if
	someone needs this later and not to implement this on this base. Of course
	it's possible to implemnt it directly into the SmartPointer but the SmartPointer
	is intented to be very fast with very small overhead over a normal void*.
	If you want to have something general, you could implement something like
	a threadsafe access wrapper object. Like:
	
	template<typename _Type>
	class ThreadsafeObject { // constructor locks accesser.mutex, destructor unlocks
	public: _Type* get();
	}
	
	template<typename _Type>
	class ThreadsafeAccesser {
	private: mutex
	public: ThreadsafeObject<_Type> get();
	}
	
	And then if you want to use it together with SmartPointer:
	
	SmartPointer< ThreadsafeAccesser<_Type> > pointer;
	
*/
class ScopedLock
{
	private:
	SDL_mutex* data_mutex;
	// Non-copyable
	ScopedLock( const ScopedLock & ) { assert(false); };
	ScopedLock & operator= ( const ScopedLock & ) { assert(false); return *this; };

	public:
	ScopedLock( SDL_mutex* mutex ): data_mutex(mutex) {
		SDL_mutexP(data_mutex);
		// It is safe to call SDL_mutexP()/SDL_mutexV() on a mutex several times
		// HINT to the comment: it's not only safe, it's the meaning of it; in the case it is called twice,
		// it locks until there is a SDL_mutexV. But *always* call SDL_mutexV from the same thread which has
		// called SDL_mutexP before (else you get serious trouble). Also never call SDL_mutexV when there
		// was no SDL_mutexP before.
	};
	
	~ScopedLock() {
		SDL_mutexV(data_mutex); 
	};
	
	template < typename _Type, typename _SpecificInitFunctor >
	ScopedLock( SmartPointer< _Type, _SpecificInitFunctor > & pt )
		: data_mutex( pt.mutex ) // TODO: Don't use pt.mutex; it's the mutex for the reference counter, nothing else.
								 // Copies from the SmartPointer can be allowed, there is no problem. Only the
								 // access itself to ->get() should be synchronised.
	{
		SDL_mutexP(data_mutex); 
	};
	
	SDL_mutex* getMutex() { return data_mutex; };	// For usage with SDL_CondWait()
};

#endif

