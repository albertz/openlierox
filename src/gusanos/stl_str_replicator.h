#ifndef STL_STR_REPLICATOR_H
#define STL_STR_REPLICATOR_H

#include "netstream.h"
#include <string>
#include <stdexcept>

class STLStringReplicator : public Net_ReplicatorBasic
{
	private:
	
		std::string*	m_ptr;
		std::string	m_cmp;
	
	public:

		STLStringReplicator(Net_ReplicatorSetup *_setup, std::string *_data);
	
	// TODO: Implement this for safeness sake
		Net_Replicator* Duplicate(Net_Replicator *_dest)
		{
			if(_dest)
				*_dest = *this;
			else
				return new STLStringReplicator(*this);
			return 0;
		}
	
		bool checkState();
		
		void packData(BitStream *_stream);
	
		void unpackData(BitStream *_stream, bool _store);
		
		void* peekData();
	
		void clearPeekData();
};

#endif
