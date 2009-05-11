/*
	Autocompletion support

	OpenLieroX

	code under LGPL
	created on 12-05-2009 by Albert Zeyer
*/

#ifndef __OLX__AUTOCOMPLETION_H__
#define __OLX__AUTOCOMPLETION_H__

#include <string>
#include "Mutex.h"
#include "PreInitVar.h"
#include "Condition.h"

class AutocompletionInfo {
public:
	struct InputState {
		InputState() : pos(0) {}
		InputState(const std::string& t, size_t p) : text(t), pos(p) {}
		std::string text;
		size_t pos;
	};
	
private:
	Mutex mutex;
	Condition cond;
	PIVar(bool,false) isSet;
	PIVar(bool,false) fail;
	InputState old;
	InputState replace;
	
public:
	void pushReplace(const InputState& old, const InputState& replace) {
		Mutex::ScopedLock lock(mutex);
		isSet = true;
		fail = false;
		this->old = old;
		this->replace = replace;
		cond.signal();
	}

	void pushFail(const InputState& old) {
		Mutex::ScopedLock lock(mutex);
		isSet = true;
		fail = true;
		cond.signal();
	}
	
	bool waitForReplace(InputState& replace) {
		Mutex::ScopedLock lock(mutex);
		while(!isSet) cond.wait(mutex);
		replace = this->replace;
		return !fail;
	}
};


#endif
