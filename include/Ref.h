/*
	OpenLieroX

	Ref class. various utilities

	code under LGPL
	created 01-05-2007
	by Albert Zeyer and Dark Charlie
*/

#ifndef __OLX__REF_H__
#define __OLX__REF_H__

#include <assert.h>

/*
	Intended to hold an object of an abstract class (interface).
	The object is given and must live on the heap.
	The `Ref` instance owns that `_Obj` instance.
	When `Ref` gets deleted, it also deletes the `_Obj` instance.
	
	The `_Obj` type must support `_Obj* _Obj::copy()`, and when we
	copy `Ref` instances, the object will also be copied.
	(In that sense, the name "Ref" might be a bit missleading.)

	`Ref< Iterator<int> >` is a typical example, because you normally
	don't want to know about the specific implementation of Iterator.
*/
template < typename _Obj >
class Ref {
private:
	_Obj* m_obj;
	void clear() { if(m_obj) delete m_obj; m_obj = NULL; }

public:
	Ref(_Obj* obj) : m_obj(obj) { assert(obj); }
	Ref(const Ref& ref) : m_obj(NULL) { *this = ref; }
	Ref(Ref&& ref) : m_obj(NULL) { *this = std::forward<Ref>(ref); }
	~Ref() { clear(); }

	Ref& operator=(_Obj* obj) {
		if(obj != m_obj) {
			clear();
			m_obj = obj;
		}
		return *this;
	}
	Ref& operator=(const Ref& ref) {
		if(ref.m_obj != m_obj) {
			clear();
			assert(ref); // a copy is only valid if the source is set
			m_obj = ref->copy();
		}
		return *this;
	}
	Ref& operator=(Ref&& ref) {
		if(ref.m_obj != m_obj) {
			clear();
			std::swap(m_obj, ref.m_obj);
		}
		return *this;
	}

	operator bool() const { return isSet(); }
	_Obj* operator->() { return m_obj; }
	const _Obj* operator->() const { return m_obj; }
	_Obj& get() { return *m_obj; }
	const _Obj& get() const { return *m_obj; }
	bool isSet() const { return m_obj != NULL; }
	_Obj* overtake() { _Obj* r = m_obj; m_obj = NULL; return r; } // this resets the Ref and returns the old pointer without deleting it
};

#endif

