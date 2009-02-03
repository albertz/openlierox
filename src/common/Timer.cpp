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


#include <list>
#include <SDL_thread.h>
#include <time.h>
#include <cassert>
#include "Timer.h"
#include "Debug.h"
#include "InputEvents.h"

int		Frames = 0;
float	OldFPSTime = 0;
int		Fps = 0;


///////////////////
// Get the frames per second count
// Should be called once per frame
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

int		Frames_MinFPS = 0;
float	OldFPSTime_MinFPS = 0;
float	PrevFrameTime_MinFPS = 0;
float	MaxFrameTime_MinFPS = 0;
int		Fps_MinFPS = 0;

///////////////////
// Get the minimal frames per second count ( the slowest frame )
// Should be called once per frame
int GetMinFPS(void)
{
	Frames_MinFPS++;
	float ms = GetMilliSeconds();
	if( ms - OldFPSTime_MinFPS >= 1.0f ) 
	{
		OldFPSTime_MinFPS = ms;

		if( MaxFrameTime_MinFPS > 0.00000001 )
			Fps_MinFPS = (int)( 1.0f / MaxFrameTime_MinFPS );
		if( Fps_MinFPS > Fps )
			Fps_MinFPS = Fps;

		MaxFrameTime_MinFPS = ms - PrevFrameTime_MinFPS;
		Frames_MinFPS = 0;
	}

	if( MaxFrameTime_MinFPS < ms - PrevFrameTime_MinFPS )
		MaxFrameTime_MinFPS = ms - PrevFrameTime_MinFPS;

	PrevFrameTime_MinFPS = ms;

	return Fps_MinFPS;
}

///////////////////
// Get the actual time
std::string GetTime()
{
	char cTime[64];
	cTime[0]= '\0';
	time_t t;
	time(&t);
	struct tm* tp;
	tp = localtime(&t);
	if (tp)
		strftime(cTime, 26, "%Y-%m-%d-%a-%H-%M-%S", tp);
	cTime[63] = '\0';
	return cTime;
}

// -----------------------------------
// Timer class
// HINT: the timer is not exact, do not use it for any exact timing, like ingame simulation

// Timer data, contains almost the same info as the timer class
struct TimerData {
	Timer*				timer;
	Ref<Event<Timer::EventData>::Handler> onTimerHandler;
	void*				userData;
	Uint32				interval;
	bool				once;
	bool				quitSignal;
	SDL_TimerID			timerID;
};

// Global list that holds info about headless timers
// Used to make sure there are no memory leaks
std::list<TimerData *> timers;

//////////////////
// Initialize working with timers
void InitializeTimers()
{

}

///////////////////////
// Shut down and free all running timers
void ShutdownTimers()
{
	assert(!EventSystemInited()); // Make sure the event system is shut down (to avoid double freed memory)

	// Stop and free all the running timers
	for (std::list<TimerData *>::iterator it = timers.begin(); it != timers.end(); it++)  {
		if ((*it)->timerID)
			SDL_RemoveTimer((*it)->timerID);

		// Call the user function, because it might free some data
		Event<Timer::EventData>::Handler& handler = (*it)->timer ? (*it)->timer->onTimer.handler().get() : (*it)->onTimerHandler.get();
		bool cont = true;
		(*it)->quitSignal = true;
		if (&handler) // Make sure it exists
			handler(Timer::EventData(NULL, (*it)->userData, cont));

		delete (*it);
	}

	timers.clear();
}

/////////////////////////
// Removes the timer from the global list (called when the timer is stopped/destroyed)
static void RemoveTimerFromGlobalList(TimerData *data)
{
	for (std::list<TimerData *>::iterator it = timers.begin(); it != timers.end(); it++)  {
		if (*it == data)  {
			timers.erase(it);
			break;
		}
	}
}


struct InternTimerEventData {
	TimerData* data; // for intern struct TimerData
	bool lastEvent;
	InternTimerEventData(TimerData* d, bool le) : data(d), lastEvent(le) {}
};
Event<InternTimerEventData> onInternTimerSignal;
static bool timerSystem_inited = false;


static void Timer_handleEvent(InternTimerEventData data);

static void InitTimerSystem() {
	if(timerSystem_inited) return;
	onInternTimerSignal.handler() = getEventHandler(&Timer_handleEvent);
	timerSystem_inited = true;
}

////////////////
// Constructors
Timer::Timer() : 
	userData(NULL), interval(1000),
	once(false), m_running(false), m_lastData(NULL) {
	InitTimerSystem();
}

Timer::Timer(Null, void* dat, Uint32 t, bool o) :
	userData(dat), interval(t),
	once(o), m_running(false), m_lastData(NULL) {
	InitTimerSystem();
}

Timer::Timer(void (*fct)(EventData dat), void* dat, Uint32 t, bool o) :
	userData(dat), interval(t),
	once(o), m_running(false), m_lastData(NULL) {
	onTimer.handler() = getEventHandler(fct);
	InitTimerSystem();
}

Timer::Timer(Event<EventData>::Handler* hndl, void* dat, Uint32 t, bool o) :
	userData(dat), interval(t),
	once(o), m_running(false), m_lastData(NULL) {
	onTimer.handler() = hndl;
	InitTimerSystem();
}


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
	// Check
	if (!param)  {
		// Very weird, should not happen, but the crash reports say something else...
		assert(false);
		return 0;
	}

	TimerData* timer_data = (TimerData *)param;
	
	bool lastEvent = timer_data->once || timer_data->quitSignal;
	
	onInternTimerSignal.pushToMainQueue(InternTimerEventData(timer_data, lastEvent));


	if(lastEvent) {
		// we have to call it here to ensure that this callback is never called again
		// we also have to ensure that there is only *one* event with lastEvent=true
		// (and this event has to be of course the last event for this timer in the queue)
		if(!SDL_RemoveTimer(timer_data->timerID)) {
			// should never happen!
			warnings << "could not remove timer" << endl;
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
	data->onTimerHandler = NULL;
	data->userData = userData;
	data->interval = interval;
	data->once = once;
	data->quitSignal = false;
	data->timerID = SDL_AddTimer(interval, &Timer_handleCallback, (void *) data);
	
	if(data->timerID == NULL) {
		warnings << "failed to start timer" << endl;
		delete data;
		return false;
	} else {
		timers.push_back(data); // Add it to the global timer array

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
	data->onTimerHandler = onTimer.handler().get().copy();
	data->userData = userData;
	data->interval = interval;
	data->once = once;
	data->quitSignal = false;
	data->timerID = SDL_AddTimer(interval, &Timer_handleCallback, (void *) data);
	
	if(data->timerID == NULL) {
		warnings << "failed to start headless timer" << endl;
		delete data;
		return false;
	} else  {
		timers.push_back(data); // Add it to the global timer array
		return true;
	}
}

////////////////
// Stops the timer
void Timer::stop()
{
	// Already stopped
	if(!m_running) return;
	
	((TimerData*)m_lastData)->quitSignal = true; // it will be removed in the last event; TODO: if the interval is big and the game is shut down in the meanwhile, there will be a memleak
	TimerData *d = ((TimerData*)m_lastData);
	if (d->timer && !EventSystemInited())  {  // If the event system is not initialized, it is sure that the handler won't get called and we must free the date to avoid leaks
		RemoveTimerFromGlobalList(d); // To avoid double freed memory
		delete d;
		d = NULL;
	}  else
		d->timer = NULL; // remove the reference to avoid any calls to this object again (perhaps we delete this timer-object directly after)
	m_lastData = NULL;
	m_running = false;
}



///////////////
// Handle the timer event, called from HandleNextEvent
static void Timer_handleEvent(InternTimerEventData data)
{
	TimerData* timer_data = data.data;
	assert(timer_data != NULL);
	
	// Run the client function (if no quitSignal) and quit the timer if it returns false
	// Also quit if we got last event signal
	if( !timer_data->quitSignal ) {
		Event<Timer::EventData>::Handler& handler = timer_data->timer ? timer_data->timer->onTimer.handler().get() : timer_data->onTimerHandler.get();
		bool shouldContinue = true;
		handler( Timer::EventData(timer_data->timer, timer_data->userData, shouldContinue) );
		if( !shouldContinue ) {
			if(timer_data->timer) // No headless timer => call stop() to handle intern state correctly
				timer_data->timer->stop();
			else
				timer_data->quitSignal = true;
		}
	}
	
	if(data.lastEvent)  { // last-event-signal
		if(timer_data->timer) // No headless timer => call stop() to handle intern state correctly
			timer_data->timer->stop();
		else  {
			// Headless timer is being stopped, remove it from active headless timers list
			RemoveTimerFromGlobalList(timer_data);
		}
				
		// we can delete here as we have ensured that this is realy the last event
		delete timer_data;
	}
}

