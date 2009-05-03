#include "vector_replicator.h"

#include <zoidcom.h>
#include <iostream>
#include "util/vec.h"
using std::cerr;
using std::endl;

VectorReplicator::VectorReplicator(ZCom_ReplicatorSetup *_setup, Vec *_data, Encoding::VectorEncoding& encoding_) : 
	ZCom_ReplicatorBasic(_setup),
	m_ptr(_data), encoding(encoding_)
{
	m_flags |= ZCOM_REPLICATOR_INITIALIZED;
}

bool VectorReplicator::checkState()
{
#ifdef COMPACT_FLOATS
	std::pair<long, long> v = encoding.quantize(*m_ptr);
	bool s = (m_old.first != v.first || m_old.second != v.second);
	m_old = v;
	return s;
#else
	bool s = (( m_cmp.x != m_ptr->x ) || ( m_cmp.y != m_ptr->y));
	m_cmp = *m_ptr;
	return s;
#endif
}

void VectorReplicator::packData(ZCom_BitStream *_stream)
{
#ifdef COMPACT_FLOATS
/*
	dynamic_bitset<> n = game.level.vectorEncoding.encode(*m_ptr);
	Encoding::writeBitset(*_stream, n);
*/
	encoding.encode(*_stream, *m_ptr);
	
#else
	_stream->addFloat(m_ptr->x,32);
	_stream->addFloat(m_ptr->y,32);
#endif
}

void VectorReplicator::unpackData(ZCom_BitStream *_stream, bool _store, zU32 _estimated_time_sent)
{
	if (_store)
	{
#ifdef COMPACT_FLOATS
/*
		dynamic_bitset<> n = Encoding::readBitset(*_stream, game.level.vectorEncoding.bits);
		*m_ptr = game.level.vectorEncoding.decode<Vec>(n);*/
		*m_ptr = encoding.decode<Vec>(*_stream);
#else
		m_ptr->x = _stream->getFloat(32);
		m_ptr->y = _stream->getFloat(32);
#endif
	}
	else 
	{
#ifdef COMPACT_FLOATS
		//Encoding::readBitset(*_stream, game.level.vectorEncoding.bits);
		encoding.decode<Vec>(*_stream);
#else
		_stream->getFloat(32);
		_stream->getFloat(32);
#endif
	}
}

void* VectorReplicator::peekData()
{
	assert(getPeekStream());
	Vec *retVec = new Vec;
#ifdef COMPACT_FLOATS
/*
	dynamic_bitset<> n = Encoding::readBitset(*getPeekStream(), game.level.vectorEncoding.bits);
	*retVec = game.level.vectorEncoding.decode<Vec>(n);*/
	*retVec = encoding.decode<Vec>(*getPeekStream());
#else
	retVec->x = getPeekStream()->getFloat(32);
	retVec->y = getPeekStream()->getFloat(32);
#endif
	peekDataStore(retVec);
	
	return (void*) retVec;
}

void VectorReplicator::clearPeekData()
{
	Vec *buf = (Vec*) peekDataRetrieve();
	delete buf;
};

/*Vec VectorReplicator::getLastUpdate()
{
	return m_cmp;
}*/
