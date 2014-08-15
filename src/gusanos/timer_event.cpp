#include "timer_event.h"

TimerEvent::State::State(TimerEvent* event_, int count_)
		: count(count_), triggerCount(0), event(event_)
{}

bool TimerEvent::State::tick()
{
	if ( event->triggerTimes <= 0 || triggerCount < event->triggerTimes ) {
		--count;
		if ( count < 0 ) {
			reset();
			++triggerCount;
			return true;
		}
	}
	return false;
}

void TimerEvent::State::reset()
{
	count = event->delay + (int)rndInt(event->delayVariation+1);
}

void TimerEvent::State::completeReset()
{
	if ( event->startDelay < 0 ) {
		count = event->delay + (int)rndInt(event->delayVariation+1);
	} else {
		count = event->startDelay;
	}
	triggerCount = 0;
}

TimerEvent::State TimerEvent::createState()
{
	int tmpCount;
	if ( startDelay < 0 ) {
		tmpCount = delay + (int)rndInt(delayVariation+1);
	} else {
		tmpCount = startDelay;
	}
	return State( this, tmpCount );
}

TimerEvent::TimerEvent(int _delay, int _delayVariation, int _triggerTimes, int _startDelay)
		: delay(_delay), delayVariation(_delayVariation), triggerTimes(_triggerTimes), startDelay(_startDelay)
{}

TimerEvent::TimerEvent(GameEvent::Actions& actions, int _delay, int _delayVariation, int _triggerTimes, int _startDelay)
		: GameEvent(actions)
		, delay(_delay), delayVariation(_delayVariation), triggerTimes(_triggerTimes), startDelay(_startDelay)
{}

