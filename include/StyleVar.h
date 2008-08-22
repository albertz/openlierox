/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Style variable
// Created 12/8/08
// Karel Petranek

#ifndef __STYLEVAR_H__
#define __STYLEVAR_H__

#include <SDL.h> // for Uint32

// Stylable variable
// Behaves as a normal variable but takes care of the importance
#define DEFAULT_PRIORITY 0
#define CSS_PRIORITY 1
#define CSS_IMP_PRIORITY 2
#define TAG_ATTR_PRIORITY 3
#define TAG_CSS_PRIORITY 4
#define HIGHEST_PRIORITY 5

template<typename _T>
class StyleVar  {
private:
	_T		var;
	Uint32	priority;

public:
	StyleVar() : priority(DEFAULT_PRIORITY) {}
	//StyleVar(_T val) : var(val), priority(DEFAULT_PRIORITY) {}
	void set(_T val, Uint32 prio)	{ if (prio >= priority) { var = val; priority = prio; } }
	_T get() const	{ return var; }

	operator _T() const  { return var; }
	_T operator -> () const { return var; };
	bool operator == (const StyleVar& s2)	{ return *this == s2; }
	bool operator != (const StyleVar& s2)	{ return !(*this == s2); }

	StyleVar& operator=(const StyleVar& oth)  {
		if (&oth != this)  {
			var = oth.var;
			priority = oth.priority;
		}
		return *this;
	}
};


#endif // __STYLEVAR_H__
