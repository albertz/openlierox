/*
	OpenLieroX

	various functors

	code under LGPL
	created 01-05-2007
	by Albert Zeyer and Dark Charlie
*/

#ifndef __FUNCTORS_H__
#define __FUNCTORS_H__


// joins 2 functors
template<typename _F1, typename _F2>
class JoinedFunctors : _F1, _F2 {
private:
	_F1& f1; _F2& f2;
public:
	JoinedFunctors(_F1& f1_, _F2& f2_) : f1(f1_), f2(f2_) {}

	template<typename Targ1>
	bool operator()(Targ1 arg1) {
		return f1(arg1) && f2(arg1);
	}

	template<typename Targ1, typename Targ2>
	bool operator()(Targ1 arg1, Targ2 arg2) {
		return f1(arg1, arg2) && f2(arg1, arg2);
	}

};

template <typename _ParamType>
class NopFunctor {
public:
	void operator()(_ParamType param) {}
};


#endif

