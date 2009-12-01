#ifndef TIMER_EVENT_H
#define TIMER_EVENT_H

#include "events.h"
#include "util/math_func.h"

struct TimerEvent : public GameEvent
{
	int delay;
	int delayVariation;
	int triggerTimes;
	int startDelay;

	struct State
	{
		int count;
		int triggerCount;
		TimerEvent* event;

		State(TimerEvent* event_, int count_);
		bool tick();
		void reset();
		void completeReset();
	};

	TimerEvent(int _delay, int _delayVariation, int _triggerTimes, int _startDelay = -1);
	TimerEvent(std::vector<BaseAction*>& actions, int _delay, int _delayVariation, int _triggerTimes, int _startDelay = -1);
	//~TimerEvent() {};
	State createState();
};

#endif
