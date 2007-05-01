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

#define DEFINE_INTERNDATA_CLASS(_classname) \
	class _classname { \
	public: \
		_classname(); \
		_classname(const _classname& b); \
		void operator=(const _classname& b); \
		~_classname(); \
		void* intern_data; \
	private: \
		void init(); \
		void reset(); \
		unsigned int* refcount; \
	};
	
#define	DECLARE_INTERNDATA_CLASS(_classname, _datatype) \
	_classname::_classname() { init(); } \
	void _classname::init() { \
		intern_data = new _datatype; \
		refcount = new unsigned int(1); \
	} \
	_classname::~_classname() { reset(); } \
	void _classname::reset() { \
		if(!intern_data || !refcount) return; \
		(*refcount)--; \
		if(*refcount == 0) { \
			delete (_datatype*)intern_data; \
			delete refcount; \
		} \
		intern_data = NULL; \
		refcount = NULL; \
	} \
	_classname::_classname(const _classname& b) { \
		intern_data = b.intern_data; \
		refcount = b.refcount; \
		(*refcount)++; \
	} \
	void _classname::operator=(const _classname& b) { \
		if(&b == this) return; \
		reset(); \
		intern_data = b.intern_data; \
		refcount = b.refcount; \
		(*refcount)++; \
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



#endif
