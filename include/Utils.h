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
	
	void operator++() { pos++; }
	void operator--() { pos--; }
	_ValueType operator*() { return base[pos]; }
};

template<typename _RandomAccessType, typename _ValueType, typename _PosType = size_t>
class reverse_iterator : public iterator<_RandomAccessType, _ValueType, _PosType> {
public:
	reverse_iterator(_RandomAccessType& b, _PosType p) : iterator<_RandomAccessType, _ValueType, _PosType>(b, p) {}
	void operator++() { this->pos--; }
	void operator--() { this->pos++; }
};


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
		void init(); \
		void reset(); \
	};

#define DEFINE_INTERNDATA_CLASS(_classname) \
	INTERNDATA_CLASS_BEGIN(_classname) \
	INTERNDATA_CLASS_END
	
#define	DECLARE_INTERNDATA_CLASS(_classname, _datatype) \
	_classname::_classname() { init(); } \
	void _classname::init() { \
		intern_data = new _datatype; \
	} \
	_classname::~_classname() { reset(); } \
	void _classname::reset() { \
		if(!intern_data) return; \
		delete (_datatype*)intern_data; \
		intern_data = NULL; \
	} \
	_classname::_classname(const _classname& b) { \
		init(); \
		if (intern_data) \
			memcpy(intern_data,b.intern_data,sizeof(_datatype)); \
	} \
	void _classname::operator=(const _classname& b) { \
		if(&b == this || !intern_data) return; \
		memcpy(intern_data,b.intern_data,sizeof(_datatype)); \
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
	some very basic math functions
*/

template <typename T> inline T MIN(T a, T b) { return a<b?a:b; }
template <typename T> inline T MAX(T a, T b) { return a>b?a:b; }
inline unsigned long MIN(unsigned long a, unsigned int b) { return a<b?a:b; }



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

#endif
