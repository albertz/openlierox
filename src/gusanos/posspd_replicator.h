#ifndef POSSPD_REPLICATOR_H
#define POSSPD_REPLICATOR_H

#include "netstream.h"

#include "util/vec.h"
#include "encoding.h"
#include <utility>
#include <stdexcept>

#define COMPACT_FLOATS

class PosSpdReplicator : public Net_ReplicatorBasic
{
	private:
		
		static const int speedRepTime = 0;
		static const int speedPrec = 16;
		
		CVec*	m_posPtr;
		CVec*	m_spdPtr;
		
		int m_repCount;

#ifdef COMPACT_FLOATS
	std::pair<long, long> m_oldPos;
#else
	Vec	m_cmpPos;
#endif
	
	
	public:

		PosSpdReplicator(Net_ReplicatorSetup *_setup, CVec *pos, CVec *spd, Encoding::VectorEncoding& encoding_, Encoding::DiffVectorEncoding& diffEncoding_);
	
	// TODO: Implement this for safeness sake
		Net_Replicator* Duplicate(Net_Replicator *_dest)
		{
			if(_dest)
				*_dest = *this;
			else
				return new PosSpdReplicator(*this);
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
		Encoding::DiffVectorEncoding& diffEncoding;
};

#endif
