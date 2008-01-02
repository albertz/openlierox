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
#include "Timer.h"

int		Frames = 0;
float	OldFPSTime = 0;
int		Fps = 0;


///////////////////
// Get the frames per second count
int GetFPS(void)
{
	Frames++;

	if(GetMilliSeconds() - OldFPSTime >= 1.0f) {
		OldFPSTime = GetMilliSeconds();
		Fps = Frames;
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

struct TimerThreadData {
	Timer* timer;
	Timer::OnTimerProc onTimer;
	void* userData;
	Uint32 interval;
	bool once;
	bool quit_signal;
};

static int TimerThread(void* data) {
	TimerThreadData* timer = (TimerThreadData*)data;
	
	SDL_Event ev;
	ev.type = SDL_USEREVENT_TIMER;
	ev.user.code = 0;
	ev.user.data1 = timer;
	ev.user.data2 = (void*)false; // signal if event is last
	// HINT: ev.user.data2 is a void* by definition, but we use it as a bool here
	
	bool quit;
	do {
		SDL_Delay( timer->interval );
		quit = timer->once || timer->quit_signal;
		if(quit) ev.user.data2 = (void*)true;
		SDL_PushEvent( &ev );
	} while( !quit );

	return 0;
}

Timer::Timer() : onTimer(NULL), userData(NULL), interval(1000), once(false), m_running(false) {}
Timer::Timer(OnTimerProc ontim, void* dat, Uint32 t, bool o) :
	onTimer(ontim), userData(dat), interval(t), once(o), m_running(false) {}
Timer::~Timer() { stop(); }

bool Timer::running() { return m_running; }
	
bool Timer::start() {
	if(m_running) stop();	
	
	TimerThreadData* data = new TimerThreadData;
	data->timer = this;
	data->onTimer = onTimer;
	data->userData = userData;
	data->interval = interval;
	data->once = once;
	data->quit_signal = false;
	last_thread_data = data;
	
	SDL_Thread* thread = SDL_CreateThread( &TimerThread, data );
	if(thread != NULL) {
		m_running = true;
		return true;
	} else
		return false;
}

bool Timer::startHeadless() {
	TimerThreadData* data = new TimerThreadData;
	data->timer = NULL;
	data->onTimer = onTimer;
	data->userData = userData;
	data->interval = interval;
	data->once = once;
	data->quit_signal = false;
	
	SDL_Thread* thread = SDL_CreateThread( &TimerThread, data );
	return thread != NULL;
}

void Timer::stop() {
	if(!m_running) return;
	m_running = false;
	((TimerThreadData*)last_thread_data)->quit_signal = true;
}

void Timer::handleEvent(SDL_Event& ev) {
	TimerThreadData* timer = (TimerThreadData*)ev.user.data1;
	if(timer->quit_signal) return; // it could happen that we get at the end still one more event
	
	if(!timer->onTimer(timer->timer, timer->userData)) {
		timer->quit_signal = true;
	}

	if( ev.user.data2 ) // last-event signal
		delete timer;
}
