#ifndef DETECT_EVENT_H
#define DETECT_EVENT_H

#include "events.h"

class Event;
class BaseObject;

struct DetectEvent : public Event
{
	DetectEvent( float range, bool detectOwner, int detectFilter );
	DetectEvent(std::vector<BaseAction*>&, float range, bool detectOwner, int detectFilter);
	~DetectEvent();
	
	void check( BaseObject* ownerObject );
			
	//Event* m_event;
	float m_range;
	bool m_detectOwner;
	unsigned int m_detectFilter; // detect filters ored together, 1 is worms filter, 2^n for the custom filters with n > 0
};

#endif // DETECT_EVENT_H
