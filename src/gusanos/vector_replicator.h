#ifndef VECTOR_REPLICATOR_H
#define VECTOR_REPLICATOR_H

#include "netstream.h"

#include "util/vec.h"
#include "encoding.h"
#include <utility>
#include <stdexcept>

#define COMPACT_FLOATS

class VectorReplicator : public Net_ReplicatorBasic
{
private:
	
	Vec*	m_ptr;
#ifdef COMPACT_FLOATS
	std::pair<long, long> m_old;
#else
	Vec	m_cmp;
#endif
	
public:

	VectorReplicator(Net_ReplicatorSetup *_setup, Vec *_data, Encoding::VectorEncoding& encoding_);
	
	// TODO: Implement this for safeness sake
	Net_Replicator* Duplicate(Net_Replicator *_dest)
	{
		if(_dest)
			*_dest = *this;
		else
			return new VectorReplicator(*this);
		return 0;
	}
	
	bool checkState();
	
	bool checkInitialState() { return true; }
	
	void packData(Net_BitStream *_stream);
	
	void unpackData(Net_BitStream *_stream, bool _store, Net_U32 _estimated_time_sent);
	
	void Process(eNet_NodeRole _localrole, Net_U32 _simulation_time_passed) {}
	
	void* peekData();
	
	void clearPeekData();
	
	//const Vec& getLastUpdate();
	
private:
	Encoding::VectorEncoding& encoding;
};

#endif
