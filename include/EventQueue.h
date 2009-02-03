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

#ifndef __SDLEVENTSWRAPPERS_H__
#define __SDLEVENTSWRAPPERS_H__

#include <SDL_events.h>

// bDedicated should be set before calling this function

void SDLwrap_Initialize();

void SDLwrap_Shutdown();


/* Polls for currently pending events, and returns 1 if there are any pending
   events, or 0 if there are none available.  If 'event' is not NULL, the next
   event is removed from the queue and stored in that area.
 */
int SDLwrap_PollEvent(SDL_Event *event);

/* Waits indefinitely for the next available event, returning 1, or 0 if there
   was an error while waiting for events.  If 'event' is not NULL, the next
   event is removed from the queue and stored in that area.
 */
int SDLwrap_WaitEvent(SDL_Event *event);

/* Add an event to the event queue.
   This function returns 0 on success, or -1 if the event queue was full
   or there was some other error.
 */
int SDLwrap_PushEvent(SDL_Event *event);


#endif  //  __SDLEVENTSWRAPPERS_H__
