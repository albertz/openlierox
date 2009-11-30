/*
 *  netstream.h
 *  Gusanos
 *
 *  Created by Albert Zeyer on 30.11.09.
 *  code under LGPL
 *
 */

// This is going to be a replacement for ZoidCom

#ifndef __GUS_NETSTREAM_H__
#define __GUS_NETSTREAM_H__

#include <stdint.h>

typedef uint8_t Net_U8;
typedef uint32_t Net_U32;
typedef uint32_t Net_ClassID;
typedef uint32_t Net_ConnID;
typedef uint32_t Net_NodeID;


struct Net_BitStream {
	void addInt(void* p, int bits);
	int getInt(int bits);
};


struct Net_Node {};


class Net_Replicator {};

class Net_ReplicatorSetup {};

struct Net_ReplicatorBasic {
	uint8_t m_flags;
	Net_BitStream* getPeekStream();
	
	void* peekDataRetrieve();
};

struct Net_NodeReplicationInterceptor {};

enum eNet_NodeRole {
	eNet_RoleUndefined
};
enum eNet_SendMode {};

enum {
		Net_REPLICATOR_INITIALIZED
};

enum {
	Net_REPRULE_AUTH_2_ALL	
};

#endif

