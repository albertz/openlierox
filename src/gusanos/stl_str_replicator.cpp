#include "stl_str_replicator.h"

#include "netstream.h"
#include <string>
#include <cassert>

using namespace std;

STLStringReplicator::STLStringReplicator(Net_ReplicatorSetup *_setup, string *_data) :
		Net_ReplicatorBasic(_setup),
		m_ptr(_data)
{
	m_flags |= Net_REPLICATOR_INITIALIZED;
}

bool STLStringReplicator::checkState()
{
	bool s = ( m_cmp != *m_ptr );
	m_cmp = *m_ptr;
	return s;
}

void STLStringReplicator::packData(Net_BitStream *_stream)
{
	_stream->addString( m_ptr->c_str() );
}

void STLStringReplicator::unpackData(Net_BitStream *_stream, bool _store)
{
	if (_store) {
		*m_ptr = _stream->getStringStatic();
	} else {
		_stream->getStringStatic();
	}
}

void* STLStringReplicator::peekData()
{
	assert(getPeekStream());
	string* retString = new string;
	*retString = getPeekStream()->getStringStatic();

	peekDataStore(retString);

	return (void*) retString;
}

void STLStringReplicator::clearPeekData()
{
	string *buf = (string*) peekDataRetrieve();
	if (buf)
		delete buf;
};

