/////////////////////////////////////////
//
//   OpenLieroX
//
//   Auxiliary Software class library
//
//   based on the work of JasonB
//   enhanced by Dark Charlie and Albert Zeyer
//
//   code under LGPL
//
/////////////////////////////////////////


// Timer
// Created 12/11/01
// Jason Boettcher


#ifndef __TIMER_H__
#define __TIMER_H__

#include <string>
#include <SDL.h>

///////////////////
// Get the number of milliseconds since SDL started the timer
inline float	GetMilliSeconds(void) { return (float)SDL_GetTicks() * 0.001f; }


int				GetFPS(void);
std::string		GetTime();



/*
	---------------------------------
	event-driven Timer class
*/

// SDL-event number for timer events
enum	{ SDL_USEREVENT_TIMER = SDL_USEREVENT + 2 };

/*
	Timer class

	After start(), there is a thread which will push frequently events to the
	SDL event queue. It will use the settings at the time of starting the timer.
	All later changes are ignored. If you hit start again, the current thread
	will stop and a new one with the new settings will be started. stop() will
	stop the timer. After stop() returns, no more events belonging to this thread
	will be handled. The events itself will be handled in the main thread
	(in the thread that calls ProcessEvents() or WaitForNextEvent()).

	If the callback-functions returns false, the thread will also stop.

	You can also use startHeadless() which will run independently from the
	object. That means that stop() has no effect on the thread. The only
	possibility to break the timer is to return false from within the callback.
*/
class Timer {
public:
	typedef bool (*OnTimerProc) (Timer* sender, void* userData);

	Timer();
	Timer(OnTimerProc ontim, void* dat = NULL, Uint32 t = 1000, bool o = false);
	~Timer();	

	OnTimerProc onTimer;
	void* userData;
	Uint32 interval;
	bool once;
	bool running();
	
	bool start();
	bool startHeadless();
	void stop();

	static void handleEvent(SDL_Event& ev);
	
private:
	bool m_running;
	void* last_thread_data;	
};




#endif  //  __TIMER_H__
