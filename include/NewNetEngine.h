/////////////////////////////////////////
//
//   OpenLieroX
//
//   Header file for inclusion of whole games (like OpenLiero or Gusanos) into OLX as modules, with minimal changes to them
//
//   based on the work of JasonB
//   enhanced by Dark Charlie and Albert Zeyer
//
//   code under LGPL
//
/////////////////////////////////////////


#ifndef __NEWNETENGINE_H__
#define __NEWNETENGINE_H__

#include <string>
#include "CBytestream.h"
#include "Consts.h"
#include "CodeAttributes.h"

namespace NewNet {

// ------ Structs and functions to be used from OLX ------

enum 
{ 
	TICK_TIME = 10	// Calculate one frame each 10 milliseconds - should be equal for all clients and should be never changed
};

enum Keys_t
{
	K_UP,
	K_DOWN,
	K_LEFT,
	K_RIGHT,
	K_SHOOT,
	K_JUMP,
	K_SELWEAP,
	K_ROPE,
	K_STRAFE,
	
	K_MAX	
};

struct KeyState_t
{
	bool keys[K_MAX];
	
	KeyState_t();	// All keys are false initially
	
	bool operator == ( const KeyState_t & k ) const;
	KeyState_t operator & ( const KeyState_t & k ) const;	// and
	KeyState_t operator | ( const KeyState_t & k ) const;	// or
	KeyState_t operator ^ ( const KeyState_t & k ) const;	// xor
	KeyState_t operator ~ () const;	// not
	int getFirstPressedKey() const; // Returns idx of first pressed key, or -1
	int getBitmask() const;
};

// Random number implementation
struct __taus113_state_t
{
  unsigned z1, z2, z3, z4;
};
static INLINE unsigned ___Random__( __taus113_state_t & NetSyncedRandom_state );
static INLINE double ___RandomDouble__( __taus113_state_t & NetSyncedRandom_state );
static INLINE float ___RandomFloat__( __taus113_state_t & NetSyncedRandom_state );
void ___Random_Seed__(unsigned s, __taus113_state_t & NetSyncedRandom_state);

class NetSyncedRandom
{
	public:
	NetSyncedRandom() { seed(); };
	
	unsigned get() { return ___Random__(state); };
	unsigned getInt( unsigned Max ) { return ___Random__(state) % (Max+1); };
	double getDoublePositive() { return ___RandomDouble__(state); };	// In range [0.0:1.0]
	float getFloatPositive() { return ___RandomFloat__(state); };	// In range [0.0:1.0]
	float getFloat() { return ___RandomFloat__(state) * 2.0f - 1.0f; };	// In range [-1.0:1.0]
	
	void seed( unsigned s = getSeed() ){ ___Random_Seed__(s, state); };
	static unsigned getSeed();
	
	unsigned getChecksum() const { return state.z1 + state.z2 + state.z3 + state.z4; };
	
	private:
	__taus113_state_t state;
};

extern NetSyncedRandom netRandom;

// ---------------------------------------------	!!
// TODO: recode all the following and make it OOP   !!
// ---------------------------------------------	!!
	
void StartRound( unsigned randomSeed );

void EndRound();

void ReceiveNetPacket( CBytestream * bs, int player );

// Calculates and draws the mod, returns true if there's something to send -
// should be called in a cycle, may send several packets in one frame, we should send them all as one packet over net
bool Frame( CBytestream * bs );

// Returns packet size without player ID.
int NetPacketSize();

// If player left during game
void PlayerLeft(int id);

// Returns mod checksum, and sets the time var to the time when that checksum was calculated
unsigned GetChecksum( AbsTime * time = NULL );

bool ChecksumRecalculated();

// Returns current simulation time in milliseconds inside new net engine, or tLX->currentTime if new net engine not active
AbsTime GetCurTime();

// If we can do things not handled by new net engine, like update score table (or respawn worm when I'll move that to server)
// Returns true if new net engine not active
bool CanUpdateGameState();
// If we received packet from specific worm we can play weapon sound - CanUpdateGameState() is true only when packets from all worms arrived
// Also it won't allow sound to be played twice due to rollback/recalculate
// Returns true if new net engine not active
bool CanPlaySound(int wormID);

// Returns if new net engine is active currently (time between StartRound() and EndRound())
bool Active();


// ------ Internal functions - do not use them from OLX ------

// SaveState() should save all current game physics data,
// such as positions and velocities of worms and projectiles, and pattern of destroyed dirt, and all in-game timers.
// RestoreState() should do the opposite.
// These functions will be called quite often, so please optimize your dirt-save and restore routines, 
// so they won't use plain memcpy() on 1Mb memory array, but something like saving small parts that are changed.
void SaveState();
void RestoreState();


void ReCalculateSavedState();

void CalculateCurrentState( AbsTime localTime );


void NetSyncedRandom_Seed(unsigned s);
void Random_Seed(unsigned s);

// These functions called from SaveState() and RestoreState(), and should not be called directly
void NetSyncedRandom_Save();
void NetSyncedRandom_Restore();


// ----- Inline NetSyncedRandom implementation ------
// Ripped from Gnu Math library
// Lightweight and fast random number generator that will give enough randomness for us
// I don't want to use Mersenne Twister 'cuz it has a state of 624 ints which should be copied with Save()/Restore()

/* rng/taus113.c
 * Copyright (C) 2002 Atakan Gurkan
 * Based on the file taus.c which has the notice
 * Copyright (C) 1996, 1997, 1998, 1999, 2000 James Theiler, Brian Gough
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

/* This is a maximally equidistributed combined, collision free 
   Tausworthe generator, with a period ~2^{113}. The sequence is,

   x_n = (z1_n ^ z2_n ^ z3_n ^ z4_n)  

   b = (((z1_n <<  6) ^ z1_n) >> 13)
   z1_{n+1} = (((z1_n & 4294967294) << 18) ^ b)
   b = (((z2_n <<  2) ^ z2_n) >> 27)
   z2_{n+1} = (((z2_n & 4294967288) <<  2) ^ b)
   b = (((z3_n << 13) ^ z3_n) >> 21)
   z3_{n+1} = (((z3_n & 4294967280) <<  7) ^ b)
   b = (((z4_n <<  3)  ^ z4_n) >> 12)
   z4_{n+1} = (((z4_n & 4294967168) << 13) ^ b)

   computed modulo 2^32. In the formulas above '^' means exclusive-or 
   (C-notation), not exponentiation. 
   The algorithm is for 32-bit integers, hence a bitmask is used to clear 
   all but least significant 32 bits, after left shifts, to make the code 
   work on architectures where integers are 64-bit.

   The generator is initialized with 
   zi = (69069 * z{i+1}) MOD 2^32 where z0 is the seed provided
   During initialization a check is done to make sure that the initial seeds 
   have a required number of their most significant bits set.
   After this, the state is passed through the RNG 10 times to ensure the
   state satisfies a recurrence relation.

   References:
   P. L'Ecuyer, "Tables of Maximally-Equidistributed Combined LFSR Generators",
   Mathematics of Computation, 68, 225 (1999), 261--269.
     http://www.iro.umontreal.ca/~lecuyer/myftp/papers/tausme2.ps
   P. L'Ecuyer, "Maximally Equidistributed Combined Tausworthe Generators", 
   Mathematics of Computation, 65, 213 (1996), 203--213.
     http://www.iro.umontreal.ca/~lecuyer/myftp/papers/tausme.ps
   the online version of the latter contains corrections to the print version.
*/

#define LCG(n) ((69069UL * n) & 0xffffffffUL)
#define NSRMASK 0xffffffffUL

static INLINE unsigned ___Random__( __taus113_state_t & NetSyncedRandom_state )
{
  unsigned b1, b2, b3, b4;

  b1 = ((((NetSyncedRandom_state.z1 << 6UL) & NSRMASK) ^ NetSyncedRandom_state.z1) >> 13UL);
  NetSyncedRandom_state.z1 = ((((NetSyncedRandom_state.z1 & 4294967294UL) << 18UL) & NSRMASK) ^ b1);

  b2 = ((((NetSyncedRandom_state.z2 << 2UL) & NSRMASK) ^ NetSyncedRandom_state.z2) >> 27UL);
  NetSyncedRandom_state.z2 = ((((NetSyncedRandom_state.z2 & 4294967288UL) << 2UL) & NSRMASK) ^ b2);

  b3 = ((((NetSyncedRandom_state.z3 << 13UL) & NSRMASK) ^ NetSyncedRandom_state.z3) >> 21UL);
  NetSyncedRandom_state.z3 = ((((NetSyncedRandom_state.z3 & 4294967280UL) << 7UL) & NSRMASK) ^ b3);

  b4 = ((((NetSyncedRandom_state.z4 << 3UL) & NSRMASK) ^ NetSyncedRandom_state.z4) >> 12UL);
  NetSyncedRandom_state.z4 = ((((NetSyncedRandom_state.z4 & 4294967168UL) << 13UL) & NSRMASK) ^ b4);

  return (NetSyncedRandom_state.z1 ^ NetSyncedRandom_state.z2 ^ NetSyncedRandom_state.z3 ^ NetSyncedRandom_state.z4);

};
static INLINE double ___RandomDouble__( __taus113_state_t & NetSyncedRandom_state )
{
  return ___Random__( NetSyncedRandom_state ) / 4294967295.0;
};
static INLINE float ___RandomFloat__( __taus113_state_t & NetSyncedRandom_state )
{
  return ___Random__( NetSyncedRandom_state ) / 4294967295.0f;
};

#undef LCG
#undef NSRMASK

} // namespace

#endif

