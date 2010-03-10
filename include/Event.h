/*
	OpenLieroX

	event class

	created on 27-05-2008 by Albert Zeyer
	code under LGPL
*/

#ifndef __EVENT_H__
#define __EVENT_H__

#include <list>
#include <SDL.h>
#include "olx-types.h"
#include "Ref.h"
#include "SmartPointer.h"
#include "EventQueue.h"
#include "Utils.h" // for isSameType

struct EventData {
	EventData(void* own = NULL) : owner(own) {}

	void* owner;
};


class _Event {};

/*
	This is an Event class, which represents a possible event.
	It handles all the event handlers.
	
	_Data should provide at least an owner field like EventData
*/
template< typename _Data = EventData >
class Event : public _Event {
public:
	class Handler {
	public:
		typedef _Data Data;
		virtual ~Handler() {}
		virtual void operator()(_Data data) = 0;
		virtual bool operator==(const Handler& hndl) = 0;
		virtual Handler* copy() const = 0;
	};

	typedef std::list< Ref<Handler> > HandlerList;
	
protected:
	class HandlerAccessor {
	private:
		Event* base;
	public:
		HandlerAccessor(Event* b) : base(b) {}
		HandlerAccessor& operator=(const Ref<Handler>& h) { base->m_handlers.clear(); if(h.isSet()) base->m_handlers.push_back(h); return *this; }
		HandlerAccessor& operator=(Null) { base->m_handlers.clear(); return *this; }
		HandlerAccessor& operator+=(const Ref<Handler>& h) { if(h.isSet()) base->m_handlers.push_back(h); return *this; }
		HandlerAccessor& operator-=(const Ref<Handler>& h) {
			for(typename Event::HandlerList::iterator i = base->m_handlers.begin(); i != base->m_handlers.end(); ++i)
				if(i->get() == h.get()) {
					base->m_handlers.erase(i);
					break;
				}
			return *this;
		}

		const typename Event::HandlerList& get() { return base->m_handlers; }
	};
	
private:
	friend class HandlerAccessor;
	HandlerList m_handlers;
	
public:
	Event() { handler() = null; }
	~Event() { if (mainQueue) mainQueue->removeCustomEvents(this); }
	Event(const Event& e) { (*this) = e; }
	Event& operator=(const Event& e) { m_handlers = e.m_handlers; return *this; }
	HandlerAccessor handler() { return HandlerAccessor(this); }

	void pushToMainQueue(_Data data) { if(mainQueue) mainQueue->push(new EventThrower<_Data>(this, data)); }

	void occurred(_Data data) {
		callHandlers(m_handlers, data);
	}
	static void callHandlers(HandlerList& handlers, _Data data) {
		for(typename HandlerList::iterator i = handlers.begin(); i != handlers.end(); ++i)
			i->get()(data);
	}
};


template< typename _Base, typename _Data = EventData >
class MemberFunction : public Event<_Data>::Handler {
public:
	typedef void (_Base::*Function) (_Data data);
private:
	_Base* m_obj;
	Function m_fct;
public:
	MemberFunction(_Base* obj, Function fct) : m_obj(obj), m_fct(fct) {}
	MemberFunction(const MemberFunction& other) : m_obj(other.m_obj), m_fct(other.m_fct) {}

	virtual void operator()(_Data data) { (*m_obj.*m_fct)(data); }
	virtual bool operator==(const typename Event<_Data>::Handler& hndl) {
		const MemberFunction* hPtr = dynamic_cast<const MemberFunction*>(&hndl);
		if(hPtr == NULL) return false;
		return hPtr->m_obj == m_obj && hPtr->m_fct == m_fct;
	}
	virtual typename Event<_Data>::Handler* copy() const { return new MemberFunction(m_obj, m_fct); }
};


template< typename _Data = EventData >
class StaticFunction : public Event<_Data>::Handler {
public:
	typedef void (*Function) (_Data data);
private:
	Function m_fct;
public:
	StaticFunction(Function fct) : m_fct(fct) {}
	StaticFunction(const StaticFunction& other) : m_fct(other.m_fct) {}

	virtual void operator()(_Data data) { (m_fct)(data); }
	virtual bool operator==(const typename Event<_Data>::Handler& hndl) {
		const StaticFunction* hPtr = dynamic_cast<const StaticFunction*>(&hndl);
		if(hPtr == NULL) return false;
		return hPtr->m_fct == m_fct;
	}
	virtual typename Event<_Data>::Handler* copy() const { return new StaticFunction(m_fct); }
};


template< typename _Base, typename _Data >
Ref<class Event<_Data>::Handler> getEventHandler( _Base* obj, void (_Base::*fct) (_Data data) ) {
	return new MemberFunction<_Base,_Data>( obj, fct );
}

template< typename _Data >
Ref<class Event<_Data>::Handler> getEventHandler( void (*fct) (_Data data) ) {
	return new StaticFunction<_Data>( fct );
}

#endif
