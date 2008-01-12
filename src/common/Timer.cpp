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


#include <SDL_thread.h>
#include <time.h>
#include <assert.h>
#include "Timer.h"

int		Frames = 0;
float	OldFPSTime = 0;
int		Fps = 0;


///////////////////
// Get the frames per second count
int GetFPS(void)
{
	Frames++;

	float dt = GetMilliSeconds() - OldFPSTime;
	if(dt >= 1.0f) {
		OldFPSTime = GetMilliSeconds();
		Fps = (int)( (float)Frames / dt );
		Frames = 0;
	}

	return Fps;
}

///////////////////
// Get the actual time
std::string GetTime()
{
	static char cTime[100];
	time_t t;
	time(&t);
	struct tm* tp;
	tp = localtime(&t);
	strftime(cTime, 26, "%Y-%m-%d-%a-%H-%M-%S", tp);
	return cTime;
}




// -----------------------------------
// Timer class
// HINT: the timer is not exact, do not use it for any exact timing, like ingame simulation

// Timer data, contains almost the same info as the timer class
struct TimerData {
	Timer*				timer;
	Timer::OnTimerProc	onTimer;
	void*				userData;
	Uint32				interval;
	bool				once;
	bool				quitSignal;
	SDL_TimerID			timerID;
};

////////////////
// Constructors
Timer::Timer() : 
	onTimer(NULL), userData(NULL), interval(1000),
	once(false), m_running(false), m_lastData(NULL) {}

Timer::Timer(OnTimerProc ontim, void* dat, Uint32 t, bool o) :
	onTimer(ontim), userData(dat), interval(t),
	once(o), m_running(false), m_lastData(NULL) {}

////////////////
// Destructor
Timer::~Timer()
{
	// TODO: because of this, headless timers will cause memleaks if still running when quitting
	// (because they can't get any quit signal and won't be freed here). In handleEvent 
	// there should be check for tLX->bQuitGame && timer_data->timer == NULL (headless timer) and the timer should
	// be stopped if that condition is met (it is a bit dirty solution, if you think of a better one, implement it
	// and remove this)
	
	// Stop the timer if running and not headless
	stop();
}

///////////////////
// Handle the timer callback, called from SDL
static Uint32 Timer_handleCallback(Uint32 interval, void *param)
{
	TimerData* timer_data = (TimerData *)param;
	
	bool lastEvent = timer_data->once || timer_data->quitSignal;
	
	SDL_Event ev;
	ev.type = SDL_USEREVENT_TIMER;
	ev.user.code = 0;
	ev.user.data1 = (void *) timer_data;
	ev.user.data2 = (void *) lastEvent; // signal if event is last one
	// HINT: ev.user.data2 is a void* by definition, but we use it as a bool here

	SDL_PushEvent(&ev);

	if(lastEvent) {
		// we have to call it here to ensure that this callback is never called again
		// we also have to ensure that there is only *one* event with lastEvent=true
		// (and this event has to be of course the last event for this timer in the queue)
		if(!SDL_RemoveTimer(timer_data->timerID)) {
			// should never happen!
			printf("WARNING: could not remove timer\n");
		}
	}
	
	// If run once, return almost infinite interval and wait for handleEvent to destroy us
	// If run periodically, return the same interval as passed in (SDL will keep the timer alive with the same interval)
	return lastEvent ? (Uint32)-1 : timer_data->interval;
}


/////////////////
// Returns true if this timer is running
bool Timer::running() { return m_running; }

////////////////
// Starts the timer
bool Timer::start() 
{
	// Stop if running
	if (m_running)
		stop();
	
	// Copy the info to timer data structure and run the timer
	TimerData* data = new TimerData;
	m_lastData = (void *)data;
	
	data->timer = this;
	data->onTimer = onTimer;
	data->userData = userData;
	data->interval = interval;
	data->once = once;
	data->quitSignal = false;
	data->timerID = SDL_AddTimer(interval, &Timer_handleCallback, (void *) data);
	
	if(data->timerID == NULL) {
		printf("WARNING: failed to start timer\n");
		delete data;
		return false;
	} else {
		m_running = true;
		return true;
	}
}

//////////////////
// Starts the timer without pointing at "this" object (it means the timer object can be a temporary local variable)
bool Timer::startHeadless()
{
	// Copy the info to timer data structure and run the timer
	TimerData* data = new TimerData;
	data->timer = NULL;
	data->onTimer = onTimer;
	data->userData = userData;
	data->interval = interval;
	data->once = once;
	data->quitSignal = false;
	data->timerID = SDL_AddTimer(interval, &Timer_handleCallback, (void *) data);
	
	if(data->timerID == NULL) {
		printf("WARNING: failed to start headless timer\n");
		delete data;
		return false;
	} else
		return true;
}

////////////////
// Stops the timer
void Timer::stop()
{
	// Already stopped
	if(!m_running) return;
	
	((TimerData*)m_lastData)->quitSignal = true; // it will be removed in the last event
	((TimerData*)m_lastData)->timer = NULL; // remove the reference to avoid any calls to this object again (perhaps we delete this timer-object directly after)
	m_lastData = NULL;
	m_running = false;
}


///////////////
// Handle the timer event, called from HandleNextEvent
void Timer::handleEvent(SDL_Event& ev)
{
	TimerData* timer_data = (TimerData*)ev.user.data1;
	assert(timer_data != NULL);
	
	// Run the client function (if no quitSignal) and quit the timer if it returns false
	// Also quit if we got last event signal
	if( !timer_data->quitSignal )
	if( !timer_data->onTimer(timer_data->timer, timer_data->userData) ) { // false returned => stop
		if(timer_data->timer) // No headless timer => call stop() to handle intern state correctly
			timer_data->timer->stop();
		else
			timer_data->quitSignal = true;
	}
	
	if(ev.user.data2)  { // last-event-signal
		if(timer_data->timer) // No headless timer => call stop() to handle intern state correctly
			timer_data->timer->stop();
				
		// we can delete here as we have ensured that this is realy the last event
		delete timer_data;
	}
}
