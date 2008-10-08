/*
	OpenLieroX

	various utilities

	code under LGPL
	created 01-05-2007
	by Albert Zeyer and Dark Charlie
*/

#ifndef __UTILS_H__
#define __UTILS_H__

#include <stddef.h> // for size_t


template<typename _RandomAccessType, typename _ValueType, typename _PosType = size_t>
class iterator {
protected:
	_RandomAccessType& base;
	_PosType pos;
public:
	iterator(_RandomAccessType& b, _PosType p) : base(b), pos(p) {}
	static iterator end(_RandomAccessType& b) {
		return iterator<_RandomAccessType, _ValueType>(b, -1); }

	template<typename _ValueType2, typename _PosType2>
	void operator==(const iterator<_RandomAccessType, _ValueType2, _PosType2>& it) {
		return base == it->base && (pos == it->pos || MIN(pos,it->pos) >= base.size()); }

	iterator& operator++() { pos++; return *this; }
	iterator& operator--() { pos--; return *this; }
	_ValueType operator*() { return base[pos]; }
};

/*
// WARNING: name conflicts with some implementations of STL (eg STLport)
template<typename _RandomAccessType, typename _ValueType, typename _PosType = size_t>
class reverse_iterator : public iterator<_RandomAccessType, _ValueType, _PosType> {
public:
	reverse_iterator(_RandomAccessType& b, _PosType p) : iterator<_RandomAccessType, _ValueType, _PosType>(b, p) {}
	void operator++() { this->pos--; }
	void operator--() { this->pos++; }
};
*/

/*
	helpers for declaration/definition of classes
*/

#define INTERNDATA_CLASS_BEGIN(_classname) \
	class _classname { \
	public: \
		_classname(); \
		_classname(const _classname& b); \
		void operator=(const _classname& b); \
		~_classname(); \
		void* intern_data; \
	private:

#define INTERNDATA_CLASS_END \
	private: \
		void INTERNDATA__init(); \
		void INTERNDATA__reset(); \
	};

#define DEFINE_INTERNDATA_CLASS(_classname) \
	INTERNDATA_CLASS_BEGIN(_classname) \
	INTERNDATA_CLASS_END

#define	DECLARE_INTERNDATA_CLASS(_classname, _datatype) \
	_classname::_classname() { INTERNDATA__init(); } \
	void _classname::INTERNDATA__init() { \
		intern_data = new _datatype; \
	} \
	_classname::~_classname() { INTERNDATA__reset(); } \
	void _classname::INTERNDATA__reset() { \
		if(!intern_data) return; \
		delete (_datatype*)intern_data; \
		intern_data = NULL; \
	} \
	_classname::_classname(const _classname& b) { \
		INTERNDATA__init(); \
		if (intern_data) \
			*(_datatype*)intern_data = *(const _datatype*)b.intern_data; \
	} \
	void _classname::operator=(const _classname& b) { \
		if(&b == this || !intern_data) return; \
		*(_datatype*)intern_data = *(const _datatype*)b.intern_data; \
	} \
	_datatype* _classname##Data(_classname* obj) { \
		if(obj) return (_datatype*)obj->intern_data; \
		else return NULL; \
	} \
	const _datatype* _classname##Data(const _classname* obj) { \
		if(obj) return (_datatype*)obj->intern_data; \
		else return NULL; \
	}


/*
	intended to hold an object of an abstract class (interface)

	Ref< Iterator<int> > is a typical example, because you normally
	don't want to know about the specific implementation of Iterator
*/
template < typename _Obj >
class Ref {
private:
	_Obj* m_obj;
	void clear() { if(m_obj) delete m_obj; m_obj = NULL; }

public:
	Ref(_Obj* obj = NULL) : m_obj(obj) {}
	Ref(const Ref& ref) { m_obj = ref->copy(); }
	~Ref() { clear(); }

	Ref& operator=(_Obj* obj) { clear(); m_obj = obj; return *this; }
	Ref& operator=(const Ref& ref) { clear(); m_obj = ref->copy(); return *this; }

	_Obj* operator->() { return m_obj; }
	const _Obj* operator->() const { return m_obj; }
	_Obj& get() { return *m_obj; }
	const _Obj& get() const { return *m_obj; }	
	_Obj* overtake() { _Obj* r = m_obj; m_obj = NULL; return r; } // this resets the Ref and returns the old pointer without deleting it
};


template <typename _dst, typename _src>
bool isSameType(const _src& obj1, const _dst& obj2) {
	if(sizeof(_dst) < sizeof(_src)) return isSameType(obj2, obj1);
	return dynamic_cast<const _dst*>(&obj1) != NULL;
}


/*
	some very basic math functions
*/


template <class Iter> void SafeAdvance(Iter& it, size_t count, const Iter& end)  {
	for (size_t i=0; i < count && it != end; i++, it++)  {}
}

// some basic functors
// TODO: perhaps move them elsewhere?

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

// Assertion and breakpoint in the code
#ifdef DEBUG
#	ifdef _MSC_VER
#		define	DEBUGASSERT() { printf("Assertion: %s in %s:%i\n", (__FUNCSIG__), (__FILE__), (__LINE__)); __asm  { int 3 }; }
#	else
#		ifdef __GNUC__
#			define	DEBUGASSERT() { printf("Assertion: %s in %s:%i\n", __FUNCTION__, __FILE__, __LINE__); assert(false); }
#		else
#			define DEBUGASSERT() { printf("Assertion: in %s:%i\n", __FILE__, __LINE__); assert(false); }
#		endif
#	endif
#else
#	define DEBUGASSERT() { }
#endif

#endif

