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
	ev.user.data2 = NULL;
	
	do {
		SDL_Delay( timer->interval );
		SDL_PushEvent( &ev );
	} while( !timer->once && !timer->quit_signal );

	delete timer;
	return 0;
}

Timer::Timer() : interval(1000), once(false), m_running(false) {}
Timer::~Timer() { stop(); }

bool Timer::running() { return m_running; }
	
bool Timer::start() {
	if(m_running) stop();	
	
	TimerThreadData* data = new TimerThreadData;
	data->timer = this;
	data->onTimer = onTimer;
	data->interval = interval;
	data->once = once;
	data->quit_signal = false;
	last_thread_data = data;
	
	SDL_Thread* thread = SDL_CreateThread( &TimerThread, last_thread_data );
	if(thread != NULL) {
		m_running = true;
		return true;
	} else
		return false;
}

void Timer::stop() {
	if(!m_running) return;
	((TimerThreadData*)last_thread_data)->quit_signal = true;
}

void Timer::handleEvent(SDL_Event& ev) {
	TimerThreadData* timer = (TimerThreadData*)ev.user.data1;
	if(timer->quit_signal) return; // it could happen that we get at the end still one more event
	
	timer->onTimer(*timer->timer);
}
