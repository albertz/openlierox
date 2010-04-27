/*
 *  VecTimeRecorder.h
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 16.04.10.
 *  code under LGPL
 *
 */

#ifndef __OLX_VECTIMERECORDER_H__
#define __OLX_VECTIMERECORDER_H__

#include "CVec.h"
#include "PhysicsLX56.h"

/*
 A class which records some data (like a vector) over time.
 It is supposed to make a record for each physics frame (or for any predefined frame rate) over the last X frames.
 */

template<typename _Data, size_t _FramesCount>
class DataTimeRecorder {
public:
	typedef _Data Data;
	static const size_t FramesCount = _FramesCount;

private:
	Data m_data[FramesCount];
	size_t m_curIndex;
	size_t m_entryCount;
	
public:
	DataTimeRecorder() : m_curIndex(0), m_entryCount(0) {}
	
	void clear() { m_entryCount = 0; }
	void clear(const Data& d) { clear(); push_back(d); }
	void push_back(const Data& d) {
		m_data[m_curIndex] = d;
		m_curIndex++; m_curIndex %= FramesCount;
		if(m_entryCount < FramesCount) m_entryCount++;
	}
	
	bool have(size_t frameBack) const { return frameBack < m_entryCount; }
	size_t entryCount() const { return m_entryCount; }
	
	const _Data& get(size_t framesBack /* Note that it counts backwards! */) const {
		return m_data[ (m_curIndex + FramesCount - (framesBack % FramesCount)) % FramesCount ];
	}
	
	static size_t framesBackCountForTime(size_t millisecsPerFrame, size_t millisecsBack) {
		return millisecsBack / millisecsPerFrame;
	}
	
	const _Data& getBest(size_t millisecsPerFrame, size_t millisecsBack) {
		size_t framesBack = framesBackCountForTime(millisecsPerFrame, millisecsBack);
		if(have(framesBack)) return get(framesBack);
		if(m_entryCount > 0) return get(m_entryCount - 1);
		return m_data[(m_curIndex+FramesCount-1)%FramesCount]; // wrong but this should anyway not happen
	}
};

typedef DataTimeRecorder<CVec, 100> VecTimeRecorder;

#endif
