/*
	OpenLieroX

	fast fixed vector class

	created 27-05-2008 by Albert Zeyer
	code under LGPL
*/

#ifndef __FASTVECTOR_H__
#define __FASTVECTOR_H__

#include "Iter.h"
#include "Event.h"
#include "MathLib.h"


/*
	_Obj has to provide these functions:
		bool isUsed() const;
		void setUnused();
		Event<> onInvalidation;

	also, FastVector only works correct if all new objects are requested from it, 
	so you should not call operator delete on getNewObj() return value or some other stupid thing
*/
template < typename _Obj, int SIZE >
class FastVector {
protected:
	_Obj m_objects[SIZE];
	int m_firstUnused;
	int m_lastUsed;

	void findNewFirstUnused() {
		while(true) {
			m_firstUnused++;
			if(m_firstUnused > m_lastUsed) break;
			if(m_firstUnused >= SIZE) break;
			if(!isUsed(m_firstUnused)) break;
		}
	}

	void findNewLastUsed() {
		while(true) {
			m_lastUsed--;
			if(m_lastUsed < 0) break;
			if(m_lastUsed < m_firstUnused) break;
			if(isUsed(m_lastUsed)) break;
		}
	}

	void init() {
		for(int i = 0; i < SIZE; i++) {
			m_objects[i].setUnused();
			m_objects[i].onInvalidation.handler() = getEventHandler(this, &FastVector::onObjectInvalidation);
		}
	}

	void onObjectInvalidation(EventData data) {
		int i = (int)((_Obj*)data.owner - &m_objects[0]);
		refreshObj(i);
	}

public:
	FastVector() : m_firstUnused(0), m_lastUsed(-1) { init(); }

	int firstUnused() { return m_firstUnused; }
	int lastUsed() { return m_lastUsed; }
	bool isUsed(int index) const { return m_objects[index].isUsed(); }
	_Obj& operator[](int index) { return m_objects[index]; }

	void clear() {
		for(int i = 0; i <= m_lastUsed; i++)
			m_objects[i].setUnused();
		m_firstUnused = 0;
		m_lastUsed = -1;
	}

	size_t size() {
		size_t c = 0;
		for(int i = 0; i <= m_lastUsed; i++)
			if(m_objects[i].isUsed()) c++;
		return c;
	}
	
	// this function assumes, that the returned object is used after
	_Obj* getNewObj() {
		if(m_firstUnused >= SIZE) return NULL;

		int newObj = m_firstUnused;
		if(newObj > m_lastUsed) m_lastUsed = newObj;
		findNewFirstUnused();
		return &m_objects[newObj];
	}

	// call this when you manually have setUnused() an obj
	void refreshObj(int index) {
		if(!isUsed(index)) {
			if(index < m_firstUnused) m_firstUnused = index;
			if(index >= m_lastUsed) findNewLastUsed();
		}
	}

	class Iterator : public ::Iterator<_Obj*> {
	private:
		int m_index;
		FastVector& m_parent;
	public:
		Iterator(FastVector& parent, int index = 0) : m_parent(parent) {
			m_index = CLAMP(index, 0, SIZE) - 1;
			next();
		}
		Iterator* copy() const { return new Iterator(m_parent, m_index); }

		virtual void reset() { m_index = -1; next(); }
		void next() {
			++m_index;
			while (m_index < SIZE) {
				if(m_parent.isUsed(m_index))
					break;

				++m_index;

				if(m_index > m_parent.m_lastUsed)
					m_index = SIZE;
			}
		}
		bool operator==(const ::Iterator<_Obj*>& other) const { return m_index == ((Iterator*)&other)->m_index; }

		bool isValid() { return m_index < SIZE; }
		_Obj* get() { return &m_parent[m_index]; }
	};

	typename ::Iterator<_Obj*>::Ref begin() { return new Iterator(*this); }

	FastVector & operator = ( const FastVector & v ) {
		// Fast copy routine ( _Obj should provide sane operator= )
		for(int i = 0; i < SIZE; i++) 
		{
			if( v.m_objects[i].isUsed() )
			{
				m_objects[i] = v.m_objects[i];
				m_objects[i].onInvalidation.handler() = getEventHandler(this, &FastVector::onObjectInvalidation);
			}
			else
				m_objects[i].setUnused();
		}
		m_firstUnused = v.m_firstUnused;
		m_lastUsed = v.m_lastUsed;
		return *this;
	}
};

#endif
