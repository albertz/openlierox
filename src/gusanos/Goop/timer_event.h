#ifndef TIMER_EVENT_H
#define TIMER_EVENT_H

#include "events.h"
#include "util/math_func.h"

struct TimerEvent : public Event
{
	
	struct State
	{
		State(TimerEvent* event_, int count_) 
		: count(count_), triggerCount(0), event(event_)
		{}
		
		bool tick()
		{
			if ( event->triggerTimes <= 0 || triggerCount < event->triggerTimes )
			{
				--count;
				if ( count < 0 )
				{
					reset();
					++triggerCount;
					return true;
				}
			}
			return false;
		}
	
		void reset()
		{
			count = event->delay + rndInt(event->delayVariation+1);
		}
		
		void completeReset()
		{
			if ( event->startDelay < 0 )
			{
				count = event->delay + rndInt(event->delayVariation+1);
			}else
			{
				count = event->startDelay;
			}
			triggerCount = 0;
		}
		
		int count;
		int triggerCount;
		TimerEvent* event;
	};

	State createState()
	{
		int tmpCount;
		if ( startDelay < 0 )
		{
			tmpCount = delay + rndInt(delayVariation+1);
		}else
		{
			tmpCount = startDelay;
		}
		return State( this, tmpCount );
	}
	
	TimerEvent(int _delay, int _delayVariation, int _triggerTimes, int _startDelay = -1)
	: delay(_delay), delayVariation(_delayVariation), triggerTimes(_triggerTimes), startDelay(_startDelay)
	{}
	
	TimerEvent(std::vector<BaseAction*>& actions, int _delay, int _delayVariation, int _triggerTimes, int _startDelay = -1)
	: Event(actions)
	, delay(_delay), delayVariation(_delayVariation), triggerTimes(_triggerTimes), startDelay(_startDelay)
	{}
	
	~TimerEvent() {};
	
	int delay;
	int delayVariation;
	int triggerTimes;
	int startDelay;
};

#endif
