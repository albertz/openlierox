/////////////////////////////////////////
//
//   OpenLieroX
//
//   Wrapper functions for SDL event system
//
//   based on the work of JasonB
//   enhanced by Dark Charlie and Albert Zeyer
//
//   code under LGPL
//
/////////////////////////////////////////


// This functions are required for dedicated server with no video
// If we have no video initialized SDL_PushEvent() / SDL_PollEvent() / SDL_WaitEvent() won't work
// so we'll use our internal implementation of these three functions

#include <list>
#include <SDL_thread.h>
#include <SDL_events.h>
#include <time.h>

#include "LieroX.h"
#include "EventQueue.h"
#include "ReadWriteLock.h"

static void SDLwrap_InitializeQuitHandler();


static SDL_mutex *SDLwrap_lock = NULL;
static SDL_cond *SDLwrap_cond = NULL;
static std::list< SDL_Event > SDLwrap_events;

void SDLwrap_Initialize()
{
	if( ! bDedicated )
		return;
	SDLwrap_lock = SDL_CreateMutex();
	SDLwrap_cond = SDL_CreateCond();
	SDLwrap_InitializeQuitHandler();
}

void SDLwrap_Shutdown()
{
	if( ! bDedicated )
		return;
	
  SDL_DestroyMutex(SDLwrap_lock);
  SDLwrap_lock = NULL;

  SDL_DestroyCond(SDLwrap_cond);
  SDLwrap_cond = NULL;
}


/* Polls for currently pending events, and returns 1 if there are any pending
   events, or 0 if there are none available.  If 'event' is not NULL, the next
   event is removed from the queue and stored in that area.
 */
int SDLwrap_PollEvent(SDL_Event *event)
{
	if( ! bDedicated )
		return SDL_PollEvent(event);

	ScopedLock lock(SDLwrap_lock);
	
	if( SDLwrap_events.empty() )
		return 0;
	else if( event == NULL )
		return 1;
	
	*event = SDLwrap_events.front();
	
	SDLwrap_events.pop_front();
	
	return 1;
}

/* Waits indefinitely for the next available event, returning 1, or 0 if there
   was an error while waiting for events.  If 'event' is not NULL, the next
   event is removed from the queue and stored in that area.
 */
int SDLwrap_WaitEvent(SDL_Event *event)
{
	if( ! bDedicated )
		return SDL_WaitEvent(event);

	ScopedLock lock(SDLwrap_lock);
	
	while( SDLwrap_events.empty() )
	{
		SDL_CondWait( SDLwrap_cond, SDLwrap_lock );
	}
	
	*event = SDLwrap_events.front();
	
	SDLwrap_events.pop_front();
	
	return 1;
}

/* Add an event to the event queue.
   This function returns 0 on success, or -1 if the event queue was full
   or there was some other error.
 */
int SDLwrap_PushEvent(SDL_Event *event)
{
	if( ! bDedicated )
		return SDL_PushEvent(event);

	ScopedLock lock(SDLwrap_lock);
	
	SDLwrap_events.push_back(*event);
	
	SDL_CondSignal(SDLwrap_cond);
	
	return 0;
}


#if defined(WIN32) // MacOSX, Linux, Unix

// TODO: check if this compiles!

#include <windows.h>

 
static BOOL SDLwrap_QuitHandler( DWORD fdwCtrlType ) 
{ 
	SDL_Event event;
	event.type = SDL_QUIT;
	SDLwrap_PushEvent(&event);
	return TRUE;
}


void SDLwrap_InitializeQuitHandler()
{
	SetConsoleCtrlHandler( (PHANDLER_ROUTINE) SDLwrap_QuitHandler, TRUE );
}

#else

#include <signal.h>

static void SDLwrap_QuitHandler(int sig)
{
	SDL_Event event;
	event.type = SDL_QUIT;
	SDLwrap_PushEvent(&event);
}

void SDLwrap_InitializeQuitHandler()
{
	signal(SIGINT, SDLwrap_QuitHandler);
	signal(SIGTERM, SDLwrap_QuitHandler);
}

#endif

