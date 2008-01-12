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
	SDL_TimerID			timerID;
	bool				stoppingFailed;
};

////////////////
// Constructors
Timer::Timer() : 
	onTimer(NULL), userData(NULL), interval(1000),
	once(false), m_running(false), timerID(NULL), m_lastData(NULL) {}

Timer::Timer(OnTimerProc ontim, void* dat, Uint32 t, bool o) :
	onTimer(ontim), userData(dat), interval(t),
	once(o), m_running(false), timerID(NULL), m_lastData(NULL) {}

////////////////
// Destructor
Timer::~Timer()
{
	// HINT: this can be called even when we want the timer to be running (headless timer, the local variable is being destroyed)

	// TODO: because of this, headless timers will cause memleaks if still running when quitting
	// (because they can't get any quit signal and won't be freed here). In handleEvent 
	// there should be check for tLX->bQuitGame && timer_data->timer == NULL (headless timer) and the timer should
	// be stopped if that condition is met (it is a bit dirty solution, if you think of a better one, implement it
	// and remove this)
	
	// Stop the timer if running and not headless and report an error if not successful

	// Nothing to stop (either already stopped or the timer is headless)
	if (!m_running || !timerID)  {
		// Safety - not to cause memleaks, should not happen though
		if (m_lastData)  {
			printf("WARNING: m_lastData not NULL even though the timer is stopped or headless!\n");
			delete m_lastData;
		}
		return;
	}

	// Stop the timer
	if (!SDL_RemoveTimer(timerID))  {
		printf("WARNING: Timer::~Timer could not stop the timer!\n");

		// TODO: this will cause that m_lastData won't get freed
		// However, if we freed it here, we could get a memory corruption/segfault later
		// because there will be more events coming and in the event handlers the
		// m_lastData is used
		return;
	}

	// Free the timer data if some
	if (m_lastData)
		delete m_lastData;
}

/////////////////
// Returns true if the timer is running
bool Timer::running() { return m_running; }

////////////////
// Starts the timer
bool Timer::start() 
{
	// Stop if running
	if (m_running)  {
		stop();

		// Failed to stop
		if (m_running)  {
			printf("Timer::start(): failed to stop previously set timer!\n");
			return false;
		}
	}
	
	// Copy the info to timer data structure and run the timer
	TimerData* data = new TimerData;
	data->timer = this;
	data->onTimer = onTimer;
	data->userData = userData;
	data->interval = interval;
	data->once = once;
	data->stoppingFailed = false;
	data->timerID = SDL_AddTimer(interval, &handleCallback, (void *) data);
	
	timerID = data->timerID;
	m_lastData = (void *)data;
	m_running = (timerID != NULL);

	return m_running;
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
	data->stoppingFailed = false;
	data->timerID = SDL_AddTimer(interval, &handleCallback, (void *) data);
	
	return timerID != NULL;
}

////////////////
// Stops the timer
void Timer::stop()
{

	// Already stopped
	if(!m_running || !timerID)  {
		m_running = false; // Safety
		timerID = NULL; // Safety
		return;
	}

	// Stop the timer
	m_running = SDL_RemoveTimer(timerID) != 0;

	// Stopping failed
	if (m_running)  {
		assert(m_lastData != NULL); // Safety, should always exist here
		((TimerData *)m_lastData)->stoppingFailed = true; // When the next event occurs we'll try to top the timer
	}

	// Stopping sucessful
	else  {
		timerID = NULL;
		if (m_lastData)
			delete m_lastData;
		m_lastData = NULL;
	}
}

///////////////
// Handle the timer event, called from HandleNextEvent
void Timer::handleEvent(SDL_Event& ev)
{
	TimerData* timer_data = (TimerData*)ev.user.data1;
	assert(timer_data != NULL);

	// We failed stopping the timmer last time, we'll try to do it now
	if (timer_data->stoppingFailed)  {
		if (SDL_RemoveTimer(timer_data->timerID))  {
			// Success!!
			if (timer_data->timer)  { // Not headless, tell the object we were successful
				timer_data->timer->m_running = false;
				assert(timer_data == timer_data->timer->m_lastData);  // Safety, should be always true
				timer_data->timer->m_lastData = NULL;
			}

			delete timer_data;
		}

		return; // Even if we failed to stop it again, we won't continue beucase the user
				// does not expect any further calls. Perhaps we will be successful next time
	}
	
	// Run the client function and quit the timer if it returns false
	// Also quit if we got last event signal
	if(!timer_data->onTimer(timer_data->timer, timer_data->userData) || ev.user.data2) {
		if(timer_data->timer) // No headless timer => call stop() to handle intern state correctly
			timer_data->timer->stop();
		else  {  // Headless timer, the original object is probably destroyed so we just remove everything we can and quit
			if (SDL_RemoveTimer(timer_data->timerID))
				delete timer_data;
			else  // Failed to remove the timer
				timer_data->stoppingFailed = true;
		}
	}
}

///////////////////
// Handle the timer callback, called from SDL
Uint32 Timer::handleCallback(Uint32 interval, void *param)
{
	TimerData* timer_data = (TimerData *)param;
	
	SDL_Event ev;
	ev.type = SDL_USEREVENT_TIMER;
	ev.user.code = 0;
	ev.user.data1 = (void *) timer_data;
	ev.user.data2 = (void *) timer_data->once; // signal if event is last
	// HINT: perhaps we will have some more stopping reasons later
	// HINT: ev.user.data2 is a void* by definition, but we use it as a bool here

	SDL_PushEvent(&ev);

	// If run once, return almost infinite interval and wait for handleEvent to destroy us
	// If run periodically, return the same interval as passed in (SDL will keep the timer alive with the same interval)
	// TODO: allow interval changes while running? If yes, check if this is headless timer and then
	// return timer_data->timer->interval instead of the interval passed by SDL
	return timer_data->once ? (Uint32)-1 : interval;
}
