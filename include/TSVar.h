/*
	OpenLieroX

	thread-safe variable,
	ensures that the value is always a correct one
	
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
		T ret = dat1;
		while(ret != dat2) { ret = dat1; }
		return ret;
	}
};

#endif
