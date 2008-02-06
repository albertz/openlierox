/*
	OpenLieroX

	very efficient thread-safe variable
	
	Ensures that the value is always a correct one,
	that means, if a=0 and you make a:=1, then at any
	time a=0 || a=1. This is an advantage as normally
	the value of a is undefined in the time of the
	assignment.
	
	IMPORTANT HINT: This code doesn't use any synchronisation
	and there are cases where it doesn't work, though very
	unprobable ones and also ones which can be avoided if you
	take care. This is more or less the price you pay for the
	efficiency. If you want to have a more ensured version,
	don't use this class!
	
	One case where it could break:
	- thread1 calls a=1; a=2; (multiple assignments directly after each other)
	- thread2 reads a in the meantime
	
	sample of usage:
global:
	TSVar<int> a;
thread1:
	a = 1;
	sleep(100);
	a = 2;
thread2:
	while(true) assert((int)a == 1 || (int)a == 2);
info:
	The assert in thread2 would be false sometimes without TSVar,
	that shows the sense of TSVar.

	code under LGPL
	created 06-07-2007
	by Albert Zeyer and Dark Charlie
*/

#ifndef __TSVAR_H__
#define __TSVAR_H__

// TODO: perhaps rename this class?
template<typename T>
class TSVar {
private:
	T dat1, dat2;
public:
	TSVar& operator=(const T& v) {
		dat1 = v;
		dat2 = v;
		return *this;
	}
	operator T() {
		T ret = dat2;
		while(ret != dat1) { ret = dat2; }
		return ret;
	}
};

#endif
