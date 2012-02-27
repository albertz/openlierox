// OpenLieroX
// code under LGPL
// created 2012-02-27, by Albert Zeyer

#ifndef OLX_PODFORCLASS_H
#define OLX_PODFORCLASS_H

#include <boost/type_traits/alignment_of.hpp>

// workaround to warning: dereferencing type-punned pointer will break strict-aliasing rules
// also get the correctly aligned ptr
template<typename T>
T* pointer_cast_and_align(const char* p) {
	size_t p2 = (size_t)p + boost::alignment_of<T>::value;
	p2 -= p2 % boost::alignment_of<T>::value;
	return (T*)p2;
}

// Plain-old-data struct for non-POD classes/struct
// You must call init/uninit here yourself!
template<typename T>
struct PODForClass {
	char data[sizeof(T) + boost::alignment_of<T>::value];
	T& get() { return *pointer_cast_and_align<T>(data); }
	operator T&() { return get(); }
	const T& get() const { return *pointer_cast_and_align<T>(data); }
	operator const T&() const { return get(); }
	void init() { new (&get()) T; }
	void init(const T& v) { new (&get()) T(v); }
	void uninit() { get().~T(); }
	bool operator==(const T& o) const { return get() == o; }
	bool operator<(const T& o) const { return get() < o; }
};


#endif // PODFORCLASS_H
