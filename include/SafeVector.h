//
// C++ Interface: SafeVector
//
// Description: vector with extra checks
//
//
// Author: Albert Zeyer <ich@az2000.de>, (C) 2009
//
// code under LGPL
//
//


#ifndef __OLX__SAFEVECTOR_H__
#define __OLX__SAFEVECTOR_H__

#include <cstring> // for size_t

template<typename T>
class SafeVector {
private:	
	T* m_data;
	size_t m_size;
	
public:
	SafeVector() : m_data(NULL), m_size(0) {}
	SafeVector(size_t s) : m_data(NULL), m_size(0) { resize(s); }
	SafeVector(const SafeVector& v) : m_data(NULL), m_size(0) { operator=(v); }
	~SafeVector() { clear(); }
	
	void clear() {
		if(m_data) delete[] m_data;
		m_data = NULL; m_size = 0;
	}
	
	size_t size() { return m_size; }
	
	SafeVector& operator=(const SafeVector& v) {
		clear();
		if(v.size()) {
			m_size = v.m_size;
			m_data = new T[m_size];
			for(size_t i = 0; i < m_size; ++i) m_data[i] = v.m_data[i];
		}
	}
	
	void resize(size_t s) {
		clear();
		if(s) {
			m_data = new T[s];
			if(m_data) m_size = s;
		}
	}
	
	T* operator[](long i) {
		if(i < 0) return NULL;
		if((size_t)i >= m_size) return NULL;
		return &m_data[i];
	}
};

#endif
