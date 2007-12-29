/*
	OpenLieroX

	thread-safe variable,
	ensures that the value is always a correct one
	
	sample:
global:
	TSVar<int> a;
thread1:
	a = 1;
	sleep(100);
	a = 2;
thread2:
	while(true) assert((int)a == 1 || (int)a == 2);
info:
	The assert in thread2 would be false sometimes without TSVar.

	code under LGPL
	created 06-07-2007
	by Albert Zeyer and Dark Charlie
*/

#ifndef __TSVAR_H__
#define __TSVAR_H__

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
