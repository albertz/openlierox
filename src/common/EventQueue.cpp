/////////////////////////////////////////
//
//   OpenLieroX
//
//   event queue
//
//   based on the work of JasonB
//   enhanced by Dark Charlie and Albert Zeyer
//
//   code under LGPL
//
/////////////////////////////////////////


#include <list>
#include <cassert>
#include <SDL_thread.h>
#include <SDL_events.h>
#include <time.h>

#include "LieroX.h"
#include "EventQueue.h"
#include "ReadWriteLock.h"

static void InitQuitSignalHandler();

EventQueue* mainQueue = NULL;

struct EventQueueIntern {
	SDL_mutex* mutex;
	SDL_cond* cond;
	std::list<EventItem> queue;
	EventQueueIntern() : mutex(NULL), cond(NULL) {}
	void init() {
		mutex = SDL_CreateMutex();
		cond = SDL_CreateCond();
	}
	void uninit() { // WARNING: don't call this if any other thread could be using this queue
		if(queue.size() > 0) {
			warnings << "there are " << queue.size() << " pending events in the event queue" << endl;
		}
		for(std::list<EventItem>::iterator i = queue.begin(); i != queue.end(); ++i) {
			if(i->type == SDL_USEREVENT && i->user.code == UE_CustomEventHandler) {
				delete (CustomEventHandler*)i->user.data1;
			}
		}
		queue.clear();
		
		SDL_DestroyMutex(mutex);
		mutex = NULL;
		
		SDL_DestroyCond(cond);
		cond = NULL;		
	}
};

EventQueue::EventQueue() {
	data = new EventQueueIntern();
	data->init();
}

EventQueue::~EventQueue() {
	assert(data != NULL);
	data->uninit();
	delete data; data = NULL;
}


void InitEventQueue() {
	if(!mainQueue) mainQueue = new EventQueue();
	InitQuitSignalHandler();
}

void ShutdownEventQueue() {
	if(mainQueue) {
		delete mainQueue;
		mainQueue = NULL;
	}
}


/* Polls for currently pending events, and returns 1 if there are any pending
   events, or 0 if there are none available.  If 'event' is not NULL, the next
   event is removed from the queue and stored in that area.
 */
bool EventQueue::poll(EventItem& event) {
	ScopedLock lock(data->mutex);
	
	if( data->queue.empty() )
		return false;

	event = data->queue.front();	
	data->queue.pop_front();
	
	return true;
}

/* Waits indefinitely for the next available event, returning 1, or 0 if there
   was an error while waiting for events.  If 'event' is not NULL, the next
   event is removed from the queue and stored in that area.
 */
bool EventQueue::wait(EventItem& event) {
	ScopedLock lock(data->mutex);
	
	while( data->queue.empty() ) {
		SDL_CondWait( data->cond, data->mutex );
	}
	
	event = data->queue.front();
	
	data->queue.pop_front();
	
	return true;
}

/* Add an event to the event queue.
   This function returns true on success
   or there was some other error.
 */
bool EventQueue::push(const EventItem& event) {
	ScopedLock lock(data->mutex);
	
	// TODO: some limit if queue too full? it could happen that OLX eats up all mem if there is no limit
	data->queue.push_back(event);
	
	SDL_CondSignal(data->cond);
	
	return true;
}

static EventItem CustomEvent(CustomEventHandler* eh) {
	// TODO: this is a bit hacky because we still use the SDL_Event structure
	SDL_Event ev;
	ev.type = SDL_USEREVENT;
	ev.user.code = UE_CustomEventHandler;
	ev.user.data1 = eh; // TODO: we should use an own allocator here to improve performance
	ev.user.data2 = NULL;
	return ev;
}

bool EventQueue::push(CustomEventHandler* eh) {
	return push(CustomEvent(eh));
}

void EventQueue::copyCustomEvents(const _Event* oldOwner, _Event* newOwner) {
	ScopedLock lock(data->mutex);

	for(std::list<EventItem>::iterator i = data->queue.begin(); i != data->queue.end(); ++i) {
		if(i->type == SDL_USEREVENT && i->user.code == UE_CustomEventHandler && ((CustomEventHandler*)i->user.data1)->owner() == oldOwner) {
			data->queue.insert(i, CustomEvent(((CustomEventHandler*)i->user.data1)->copy(newOwner)));
		}
	}
}

void EventQueue::removeCustomEvents(const _Event* owner) {
	ScopedLock lock(data->mutex);
	
	for(std::list<EventItem>::iterator i = data->queue.begin(); i != data->queue.end(); ) {
		std::list<EventItem>::iterator last = i; ++i;
		const SDL_Event& ev = *last;
		if(ev.type == SDL_USEREVENT && ev.user.code == UE_CustomEventHandler && ((CustomEventHandler*)ev.user.data1)->owner() == owner) {
			delete (CustomEventHandler*)ev.user.data1;
			data->queue.erase(last);
		}
	}
}




#if defined(WIN32)

#include <windows.h>

 
static BOOL QuitSignalHandler( DWORD fdwCtrlType ) 
{ 
	EventItem ev;
	ev.type = SDL_QUIT;
	mainQueue->push(ev);
	tLX->bQuitCtrlC = true; // Set the special CTRL-C flag, so Dedicated Server won't try to close the non-existant pipe
	return TRUE;
}


static void InitQuitSignalHandler()
{
	SetConsoleCtrlHandler( (PHANDLER_ROUTINE) QuitSignalHandler, TRUE );
	SetConsoleMode(stdin, ENABLE_PROCESSED_INPUT);
}

#else // MacOSX, Linux, Unix

#include <signal.h>

static void QuitSignalHandler(int sig)
{
	SDL_Event event;
	event.type = SDL_QUIT;
	mainQueue->push(event);
}

static void InitQuitSignalHandler()
{
	signal(SIGINT, QuitSignalHandler);
	signal(SIGTERM, QuitSignalHandler);
}

#endif

