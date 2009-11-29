#ifndef POSSPD_REPLICATOR_H
#define POSSPD_REPLICATOR_H

#include <zoidcom.h>

#include "util/vec.h"
#include "encoding.h"
#include <utility>
#include <stdexcept>

#define COMPACT_FLOATS

class PosSpdReplicator : public ZCom_ReplicatorBasic
{
	private:
		
		static const int speedRepTime = 0;
		static const int speedPrec = 16;
		
		Vec*	m_posPtr;
		Vec*	m_spdPtr;
		
		int m_repCount;

#ifdef COMPACT_FLOATS
	std::pair<long, long> m_oldPos;
#else
	Vec	m_cmpPos;
#endif
	
	
	public:

		PosSpdReplicator(ZCom_ReplicatorSetup *_setup, Vec *pos, Vec *spd, Encoding::VectorEncoding& encoding_, Encoding::DiffVectorEncoding& diffEncoding_);
	
	// TODO: Implement this for safeness sake
		ZCom_Replicator* Duplicate(ZCom_Replicator *_dest)
		{
			if(_dest)
				*_dest = *this;
			else
				return new PosSpdReplicator(*this);
			return 0;
		}
	
		bool checkState();
	
		bool checkInitialState() { return true; }
	
		void packData(ZCom_BitStream *_stream);
	
		void unpackData(ZCom_BitStream *_stream, bool _store, zU32 _estimated_time_sent);
	
		void Process(eZCom_NodeRole _localrole, zU32 _simulation_time_passed) {}
	
		void* peekData();
	
		void clearPeekData();
	
	//const Vec& getLastUpdate();
	
	private:
		Encoding::VectorEncoding& encoding;
		Encoding::DiffVectorEncoding& diffEncoding;
};

#endif
