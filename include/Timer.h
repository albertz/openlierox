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
#include "Event.h"

///////////////////
// Get the number of milliseconds since SDL started the timer
// TODO: Wrong function name - it returns seconds actually.
inline float	GetMilliSeconds(void) { return (float)SDL_GetTicks() * 0.001f; }


int				GetFPS(void);
int				GetMinFPS(void);
std::string		GetTime();



/*
	---------------------------------
	event-driven Timer class
*/

/*
	Timer class

	After start(), there is a thread which will push frequently events to the
	SDL event queue. It will use the settings at the time of starting the timer.
	All later changes are ignored. If you hit start again, the current thread
	will stop and a new one with the new settings will be started. stop() will
	stop the timer.
	
	After stop() returns, no more events belonging to this thread
	will be handled (this is guaranteed). Though it's possible that there
	is one last event handled exactly at the time of calling stop() when
	calling it from another thread than the main thread. If you call stop()
	from the main-thread, there will be no more timer-event for sure.
	
	The events itself will be handled in the main thread
	(in the thread that calls ProcessEvents() or WaitForNextEvent()).

	If the callback-functions returns false, the thread will also stop.

	You can also use startHeadless() which will run independently from the
	object. That means that stop() has no effect on the thread. The only
	possibility to break the timer is to return false from within the callback.
	
	userData can be used to point to some additional data. It's just a pointer,
	the timer itself just forwards it but ignores the content behind. So you
	have to care yourself about memory management.
	
	WARNING: don't set a too short interval because else the SDL event queue get flooded

	TODO: a CleanupCallback could also be saved to be called at the very last time
	after the last Timer-event to cleanup the userData for example.
	if somebody needs this, feel free to implement
*/
struct TimerData;

class Timer {
public:
	struct EventData {
		Timer* sender;
		void* userData;
		bool& shouldContinue;
		EventData(Timer* s, void* d, bool& c) : sender(s), userData(d), shouldContinue(c) {}
	};

	Timer();
	Timer(const std::string& nam, Null n, void* dat = NULL, Uint32 t = 1000, bool o = false);
	Timer(const std::string& nam, void (*fct)(EventData dat), void* dat = NULL, Uint32 t = 1000, bool o = false);
	Timer(const std::string& nam, Event<EventData>::Handler* hndl, void* dat = NULL, Uint32 t = 1000, bool o = false);
	Timer(const std::string& nam, const Timer& t) { (*this) = t; }
	Timer& operator=(const Timer& t) { name = t.name; onTimer = t.onTimer; userData = t.userData; interval = t.interval; once = t.once; return *this; }
	~Timer();

	std::string name;
	Event<EventData> onTimer;
	void* userData; // forwarded to the event-function
	Uint32 interval; // how often an event is pushed in the SDL queue
	bool once; // the event is only pushed once and then it quits
	bool running(); // this timer is currently running
	
	bool start();
	bool startHeadless(); // start independent timer
	void stop();
	
private:
	bool m_running;
	TimerData* m_lastData;	// it's TimerData* intern
};

void InitializeTimers();
void ShutdownTimers();




#endif  //  __TIMER_H__
