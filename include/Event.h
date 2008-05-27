/*
	OpenLieroX

	event class

	created on 27-05-2008 by Albert Zeyer
	code under LGPL
*/

#ifndef __EVENT_H__
#define __EVENT_H__

#include "Utils.h"


struct EventData {
	EventData(void* own = NULL) : owner(own) {}

	void* owner;
};

/*
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
	};

private:
	Ref<Handler> m_handler;

public:
	Event() : m_handler(NULL) {}
	Ref<Handler>& handler() { return m_handler; }

	void occurred(_Data data) { if(&m_handler.get()) m_handler.get()(data); }

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
};

template< typename _Base, typename _Data >
MemberFunction<_Base,_Data>* getEventHandler( _Base* obj, void (_Base::*fct) (_Data data) ) {
	return new MemberFunction<_Base,_Data>( obj, fct );
}

#endif
