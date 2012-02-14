#ifndef OLX_ANGLEREPLICATOR_H
#define OLX_ANGLEREPLICATOR_H

#include <stdexcept>
#include "util/angle.h"
#include "gusanos/netstream.h"
#include "util/Bitstream.h"

template<class T>
class BasicAngleReplicator : public Net_ReplicatorBasic
{
	private:
		typedef BasicAngle<T> Type;
		Type* m_ptr;
		Type  m_old;

	public:

		BasicAngleReplicator(Net_ReplicatorSetup* setup, Type* data)
				: Net_ReplicatorBasic(setup),
				m_ptr(data)
		{
			m_flags |= Net_REPLICATOR_INITIALIZED;
		}

		// TODO: Implement this for safeness sake
		Net_Replicator* Duplicate(Net_Replicator *_dest)
		{
			if(_dest)
				*_dest = *this;
			else
				return new BasicAngleReplicator(*this);
			return 0;
		}

		bool checkState()
		{
			bool res = m_old != *m_ptr;
			m_old = *m_ptr;
			return res;
		}

		void packData(BitStream *stream)
		{
			stream->addInt(*m_ptr, Type::prec);
		}

		void unpackData(BitStream* stream, bool store)
		{
			Type angle(T(stream->getInt(Type::prec)));
			if(store)
				*m_ptr = angle;
		}

		void* peekData()
		{
			BitStream* stream = getPeekStream();
			assert(stream);

			Type* ret = new Type(T(stream->getInt(Type::prec)));

			peekDataStore(ret);

			return (void *)ret;
		}

		void clearPeekData()
		{
			Type* buf = (Type *)peekDataRetrieve();
			delete buf;
		}
};

typedef BasicAngleReplicator<int> AngleReplicator;
typedef BasicAngleReplicator<int> AngleDiffReplicator;

#endif // OLX_ANGLEREPLICATOR_H
