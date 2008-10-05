/*
	OpenLieroX

	event class

	created on 27-05-2008 by Albert Zeyer
	code under LGPL
*/

#ifndef __EVENT_H__
#define __EVENT_H__

#include "Utils.h"
#include "types.h"


struct EventData {
	EventData(void* own = NULL) : owner(own) {}

	void* owner;
};

/*
	This is an Event class, which represents a possible event.
	It handles all the event handlers.
	
	_Data should provide at least an owner field like EventData
*/
template< typename _Data = EventData >
class Event {
public:
	class Handler {
	public:
		typedef _Data Data;
		virtual ~Handler() {}
		virtual void operator()(_Data data) = 0;
		virtual bool operator==(const Handler& hndl) = 0;
		virtual Handler* copy() const = 0;
	};

protected:
	class Handler_NoOp : public Handler {
	public:
		virtual void operator()(_Data) {}
		virtual bool operator==(const Handler& hndl) { return isSameType(hndl, *this); }
		virtual Handler* copy() const { return new Handler_NoOp(); }
	};
	
	class Handler_Joined : public Handler {
	public:
		Ref<Handler> m_handler1, m_handler2;

		Handler_Joined(Handler* h1 = NULL, Handler* h2 = NULL) : m_handler1(h1), m_handler2(h2) {}
		virtual void operator()(_Data data) { m_handler1.get()(data); m_handler2.get()(data); }
		virtual bool operator==(const Handler& hndl) {
			const Handler_Joined* hPtr = dynamic_cast<const Handler_Joined*>(&hndl);
			if(hPtr == NULL) return false;
			return m_handler1.get() == hPtr->m_handler1.get() && m_handler2.get() == hPtr->m_handler2.get();
		}
		virtual Handler* copy() const { return new Handler_Joined(m_handler1->copy(), m_handler2->copy()); }
	};
	
	class HandlerAccessor : protected Event<_Data> {
	public:
		// WARNING: Don't assign NULL, use always null!
		// WARNING: The Event-class overtakes the pointer, so don't assign any static stuff. Use getEventHandler() for example.
		HandlerAccessor& operator=(Handler* h) { m_handler = h; return *this; }
		HandlerAccessor& operator=(Null) { m_handler = new Handler_NoOp(); return *this; }
		HandlerAccessor& operator+=(Handler* h) { m_handler = new Handler_Joined(m_handler.overtake(), h); return *this; }
		HandlerAccessor& operator-=(Handler* h) {			
			Ref<Handler>* base = &m_handler;
			Handler_Joined* joined;
			while((joined = dynamic_cast<Handler_Joined*>(&base->get())) != NULL) {
				if(joined->m_handler2.get() == *h) {
					*base = joined->m_handler1.overtake();
					delete h;
					return *this;
				}
				// go one step up
				base = &joined->m_handler1;
			}
			
			// base is not a Handler_Joined anymore
			if(base->get() == *h) {
				*base = new Handler_NoOp();
				delete h;
				return *this;
			}
			
			// handler not found
			// do nothing, just ignore this
			delete h;
			return *this;
		}
		
		Handler& get() { return m_handler.get(); } 
	};
	
private:
	Ref<Handler> m_handler; // WARNING: don't assign NULL here!

public:
	Event() { handler() = null; }
	Event(const Event& e) { (*this) = e; }
	Event& operator=(const Event& e) { m_handler = e.m_handler->copy(); return *this; }
	HandlerAccessor& handler() { return (HandlerAccessor&)(*this); }

	void occurred(_Data data) { m_handler.get()(data); }
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
MemberFunction<_Base,_Data>* getEventHandler( _Base* obj, void (_Base::*fct) (_Data data) ) {
	return new MemberFunction<_Base,_Data>( obj, fct );
}

template< typename _Data >
StaticFunction<_Data>* getEventHandler( void (*fct) (_Data data) ) {
	return new StaticFunction<_Data>( fct );
}

#endif
