/*
 *  RefCounter.h
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 01.06.09.
 *  code under LGPL
 *
 */

#ifndef __OLX__REFCOUNTER_H__
#define __OLX__REFCOUNTER_H__

#include <cstring> // for size_t

/*
 * This class is to be added as an additional baseclass and overload the onLastRefRemoved method.
 */

class RefCounter {
private:
	size_t* m_refCount;

	// asumes that already cleared
	void copy(const RefCounter& r) {
		m_refCount = r.m_refCount;
		(*m_refCount)++;
	}

protected:
	// call this manually if you aren't sure if your virtual function is already unset
	void uninit() {
		if(m_refCount) {
			(*m_refCount)--;
			size_t* ref = m_refCount;
			m_refCount = NULL;			
			if(*ref == 0) {
				delete ref;
				onLastRefRemoved();
			}
		}
	}
	
public:	
	RefCounter() : m_refCount(NULL) { m_refCount = new size_t(1); }
	RefCounter(const RefCounter& r) : m_refCount(NULL) { copy(r); }
	RefCounter& operator=(const RefCounter& r) { uninit(); copy(r); return *this; }
	virtual ~RefCounter() { uninit(); }
	size_t refCount() const { return *m_refCount; }
	
	virtual void onLastRefRemoved() = 0;
};

#endif

