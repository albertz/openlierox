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


#ifndef __OLXMODINTERFACE_H__
#define __OLXMODINTERFACE_H__

// Should depend on minimal number of headres
#include <SDL.h>
#include <string>
#include <vector>
#include <map>
#include "CScriptableVars.h"
#include "CBytestream.h"
#include "SmartPointer.h"

// This is pre-release of OlxMod API, so subject to changes

namespace OlxMod
{
// TODO: remove OlxMod_ prefix as long as ve have OlxMod:: namespace

enum { OLXMOD_MAX_PLAYERS = 32 };	// Should be equal to MAX_PLAYERS from Client.h

// Only one mod at a time will run, so mod static data shouldn't be done non-static
// Mods should use SDL for graphics, and be platform-independent (any SDL app should be platform-independent).
// Support for games that use Allegro library is planned at some distant future.
// Mods need not to know anything about network - OLX will call Physics() mod callback
// with parameters like current game time and keys pressed by each player, so the physics
// calculation on each of clients will be exactly the same, provided you use net synced random number generator.
// The mod should provide SaveState() and RestoreState() functions so OLX can pre-render the approximate
// game screen before the actual net data arrived, if mod won't provide them the game will feel very laggy.

struct OlxMod_KeyState_t
{
	bool up, down, left, right, shoot, jump, selweap, rope, strafe;
	OlxMod_KeyState_t(): up(false), down(false), left(false), right(false), shoot(false), jump(false), selweap(false), rope(false), strafe(false) {};
	bool operator == ( const OlxMod_KeyState_t & k )
	{
		return up == k.up && down == k.down && left == k.left && right == k.right && shoot == k.shoot && jump == k.jump && selweap == k.selweap && rope == k.rope && strafe == k.strafe ;
	};
};

struct OlxMod_Event_t
{
	OlxMod_Event_t( OlxMod_KeyState_t k = OlxMod_KeyState_t(), OlxMod_KeyState_t kC = OlxMod_KeyState_t() ):
		keys(k), keysChanged(kC) {};
	OlxMod_KeyState_t keys;
	OlxMod_KeyState_t keysChanged;
};

enum OlxMod_WeaponRestriction_t
{
	OlxMod_WREnabled,
	OlxMod_WRBonus,
	OlxMod_WRBanned
};

// The functions that mod should export to OLX
// These are self-explanatory

// TODO: options and weaponRestrictions are empty now
// Screen currently is 640x480 only, scaling is planned.
typedef void (*OlxMod_InitFunc_t)( int numPlayers, int localPlayer, 
	std::map< std::string, CScriptableVars::ScriptVar_t > options,
	std::map< std::string, OlxMod_WeaponRestriction_t > weaponRestrictions,
	int ScreenX, int ScreenY, SDL_Surface * bmpDest );
typedef void (*OlxMod_DeInitFunc_t)();

// The mod should provide means to rollback mechanism - SaveState() should save all current game physics data,
// such as positions and velocities of worms and projectiles, and pattern of destroyed dirt, and all in-game timers.
// RestoreState() should do the opposite.
// Note that you should not call NetSyncedRandom_Save() and NetSyncedRandom_Restore() from mod - that's done automatically.
// These functions will be called quite often, so please optimize your dirt-save and restore routines, 
// so they won't use plain memcpy() on 1Mb memory array, but something like saving small parts that are changed.
typedef void (*OlxMod_SaveState_t)();
typedef void (*OlxMod_RestoreState_t)();

// GameTime is started from 0, for calculating exact physics the Physics() is called consecutively in chunks of 10 Ms 
// (though that time may change in next versions of OLX).
// Theoretically the mod should provide such physics that calling Physics(100) will produce the same results as calling 
// Physics(10) 10 times, but OLX will free the mod from such complexities - it will always call Physics(10) 10 times.
// The exception for this is when OLX going to call Draw() - it will call Physics() with fastCalculation flag set to true
// and arbitrary gameTime, and then will call Draw() - mod is free to skip some collision checks and to revert to faster
// routines in that case, as long as the game image on the screen will look smooth - OLX will discard that results anyway after Draw().
// Keys is the state of keys (up/down and changed state) for given player.
// OlxMod_PlaySoundSample should be called from Physics(), so intelligent rollback mechanism will stop all sounds caused by lag.
typedef void (*OlxMod_CalculatePhysics_t)( unsigned gameTime, const std::map< int, OlxMod_Event_t > &keys, bool fastCalculation );

// Draws the game screen (currently it's 640x480 only, scaling is planned)
// showScoreboard is true when user presses scoreboard key (typically Tab) - mod may draw some info or just ignore it
// The mod can actually do all drawing in Physics(), yet it's better to move at least part of it here
typedef void (*OlxMod_Draw_t)( bool showScoreboard );

// Called by OLX to get configurable mod options - made a callback for it so OLX and mod can init themselves, e.x. init search paths and read some files from disk.
typedef void (*OlxMod_GetOptions_t)( std::map< std::string, CScriptableVars::ScriptVarType_t > * options, std::vector<std::string> * WeaponList );

// Register your mod (should be called once).
bool OlxMod_RegisterMod( const char * name, OlxMod_InitFunc_t init, OlxMod_DeInitFunc_t deinit, 
							OlxMod_SaveState_t save, OlxMod_RestoreState_t restore,
							OlxMod_CalculatePhysics_t calc, OlxMod_Draw_t draw, OlxMod_GetOptions_t options);

// Tells OLX that current round has ended - returns to lobby and calls DeInit() callback - may be called by OLX itself if server quit or crashed.
void OlxMod_EndRound();

// Mod should use only these funcs instead of any random() funcs it uses now for physics calculation - they already seeded.
static inline unsigned long OlxMod_NetSyncedRandom();
static inline double OlxMod_NetSyncedRandomDouble();	// In range 0.0-1.0, 1.0 not included.

// Mod may use these funcs instead of any random() funcs it uses now for graphics output - they already seededm but not synced.
static inline unsigned long OlxMod_Random();
static inline double OlxMod_RandomDouble();	// In range 0.0-1.0, 1.0 not included.

// Pipe mod sounds through OLX sound system - mod should use these funcs instead of their own sound output.
// The only values supported by now are 22050 or 44100 sample rate, 1 channel and AUDIO_S16 sample format from SDL headers
void OlxMod_InitSoundSystem(int rate, unsigned sample_format, int channels);
int OlxMod_PlaySoundSample( void * data, unsigned len, int loops=0 );	// Returns channel where sound is played
void OlxMod_StopSoundSample( int channel );
bool OlxMod_IsSoundSamplePlaying( int channel );

// Enable smart OLX searchpaths for mods - use instead of fopen() everywhere
FILE* OlxMod_OpenGameFile(const char *path, const char *mode); // just calls OpenGameFile

// ----- End of OLX mod API -----

// ----- Following functions are used internally by OLX, mods should not use them -----
// TODO: move to another header

std::vector<std::string> OlxMod_GetModList();

bool OlxMod_IsModInList( const std::string & s );

void OlxMod_DeleteModList();

std::string OlxMod_GetCurrentMod();

bool OlxMod_ModSelected();

// TODO: returns empty list now
std::map< std::string, CScriptableVars::ScriptVarType_t > OlxMod_GetModOptions();

// TODO: returns empty list now
std::vector<std::string> OlxMod_GetModWeaponList();

enum OlxMod_GameSpeed_t { 
	OlxMod_GameSpeed_Fastest = 10, 	// 100 fps
	OlxMod_GameSpeed_Fast = 15, 	// 75 fps
	OlxMod_GameSpeed_Normal = 20,	// 50 fps
	OlxMod_GameSpeed_Slow = 25,		// 40 fps
	OlxMod_GameSpeed_Slowest = 30	// 33 fps
};

bool OlxMod_ActivateMod( const std::string & mod, OlxMod_GameSpeed_t speed, 
	unsigned long localTime, int numPlayers, 
	int localPlayer, unsigned long randomSeed,
	std::map< std::string, CScriptableVars::ScriptVar_t > options,
	std::map< std::string, OlxMod_WeaponRestriction_t > weaponRestrictions,
	int ScreenX, int ScreenY, SDL_Surface * bmpDest );

int OlxMod_NetPacketSize();
// In case player disconnects the engine should emulate that player is present and won't press any buttons.
void OlxMod_AddEmptyPacket( unsigned long localTime, CBytestream * bs );
// How often to send empty packets
unsigned OlxMod_EmptyPacketTime();

// Returns true if data was re-calculated.
bool OlxMod_ReceiveNetPacket( CBytestream * bs, int player );

// Calculates and draws the mod, returns true if there's something to send. showScoreboard is passed to mod drawing func - the mod may ignore it.
bool OlxMod_Frame( unsigned long localTime, OlxMod_KeyState_t keys, CBytestream * bs, bool showScoreboard );


// ----- End of OLX internal API -----

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
#define MASK 0xffffffffUL

typedef struct
{
  unsigned long int z1, z2, z3, z4;
}
OlxMod_taus113_state_t;

extern OlxMod_taus113_state_t OlxMod_NetSyncedRandom_state, OlxMod_Random_state;

static inline unsigned long OlxMod___Random__( OlxMod_taus113_state_t & NetSyncedRandom_state )
{
  unsigned long b1, b2, b3, b4;

  b1 = ((((NetSyncedRandom_state.z1 << 6UL) & MASK) ^ NetSyncedRandom_state.z1) >> 13UL);
  NetSyncedRandom_state.z1 = ((((NetSyncedRandom_state.z1 & 4294967294UL) << 18UL) & MASK) ^ b1);

  b2 = ((((NetSyncedRandom_state.z2 << 2UL) & MASK) ^ NetSyncedRandom_state.z2) >> 27UL);
  NetSyncedRandom_state.z2 = ((((NetSyncedRandom_state.z2 & 4294967288UL) << 2UL) & MASK) ^ b2);

  b3 = ((((NetSyncedRandom_state.z3 << 13UL) & MASK) ^ NetSyncedRandom_state.z3) >> 21UL);
  NetSyncedRandom_state.z3 = ((((NetSyncedRandom_state.z3 & 4294967280UL) << 7UL) & MASK) ^ b3);

  b4 = ((((NetSyncedRandom_state.z4 << 3UL) & MASK) ^ NetSyncedRandom_state.z4) >> 12UL);
  NetSyncedRandom_state.z4 = ((((NetSyncedRandom_state.z4 & 4294967168UL) << 13UL) & MASK) ^ b4);

  return (NetSyncedRandom_state.z1 ^ NetSyncedRandom_state.z2 ^ NetSyncedRandom_state.z3 ^ NetSyncedRandom_state.z4);

}
static inline double OlxMod___RandomDouble__( OlxMod_taus113_state_t & NetSyncedRandom_state )
{
  return OlxMod___Random__( NetSyncedRandom_state ) / 4294967296.0;
}

static inline unsigned long OlxMod_NetSyncedRandom()
{
	return OlxMod___Random__( OlxMod_NetSyncedRandom_state );
};
static inline double OlxMod_NetSyncedRandomDouble()
{
	return OlxMod___RandomDouble__( OlxMod_NetSyncedRandom_state );
};

static inline unsigned long OlxMod_Random()
{
	return OlxMod___Random__( OlxMod_Random_state );
};
static inline double OlxMod_RandomDouble()
{
	return OlxMod___RandomDouble__( OlxMod_Random_state );
};

#undef LCG
#undef MASK

} // namespace

#endif

