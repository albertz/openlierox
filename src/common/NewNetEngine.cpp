
#include <string>
#include <limits.h>
#include <time.h>

#include "NewNetEngine.h"
#include "MathLib.h"
#include "Sounds.h"
#include "FindFile.h"
#include "CBytestream.h"
#include "CClient.h"
#include "CMap.h"
#include "CWorm.h"
#include "ProfileSystem.h"
#include "CWormHuman.h"

namespace NewNet
{

// -------- The stuff that interacts with OLX: save/restore game state and calculate physics ---------

CWorm * SavedWormState = NULL;
NetSyncedRandom netRandom, netRandom_Saved;

void SaveState()
{
	netRandom_Saved = netRandom;
	cClient->getMap()->NewNet_SaveToMemory();

	cClient->NewNet_SaveProjectiles();

	for( int i = 0; i < MAX_WORMS; i++ )
		cClient->getRemoteWorms()[i].NewNet_SaveWormState( &SavedWormState[i] );
};

void RestoreState()
{
	netRandom = netRandom_Saved;
	cClient->getMap()->NewNet_RestoreFromMemory();

	cClient->NewNet_LoadProjectiles();

	for( int i = 0; i < MAX_WORMS; i++ )
		cClient->getRemoteWorms()[i].NewNet_RestoreWormState( &SavedWormState[i] );
};

unsigned CalculatePhysics( unsigned gameTime, KeyState_t keys[MAX_WORMS], KeyState_t keysChanged[MAX_WORMS], bool fastCalculation, bool calculateChecksum )
{
	//cClient->NewNet_Simulation();
	return 0;
};

void DisableAdvancedFeatures()
{
	 // Disables bonuses and connect-during-game for now, 
	 // I can add bonuses but connect-during-game is complicated
	 tLXOptions->tGameInfo.bBonusesOn = false;
	 //tLXOptions->tGameInfo.fRespawnTime = 2.5f; // Should be the same for all clients
	 //tLXOptions->tGameInfo.bRespawnGroupTeams = false;
	 //tLXOptions->tGameInfo.bEmptyWeaponsOnRespawn = false;
	 tLXOptions->tGameInfo.bAllowConnectDuringGame = false;
	 tLXOptions->tGameInfo.bAllowStrafing = false;
	 //*cClient->getGameLobby() = tLXOptions->tGameInfo;
};

void CalculateCurrentState( unsigned long localTime );
bool SendNetPacket( unsigned long localTime, KeyState_t keys, CBytestream * bs );

bool Frame( CBytestream * bs )
{
	unsigned long localTime = (unsigned long) (tLX->fCurTime * 1000);
	KeyState_t keys;
	if( cClient->getNumWorms() > 0 && cClient->getWorm(0)->getType() == PRF_HUMAN )
	{
		CWormHumanInputHandler * hnd = (CWormHumanInputHandler *) cClient->getWorm(0)->inputHandler();
		keys.keys[K_UP] = hnd->getInputUp().isDown();
		keys.keys[K_DOWN] = hnd->getInputDown().isDown();
		keys.keys[K_LEFT] = hnd->getInputLeft().isDown();
		keys.keys[K_RIGHT] = hnd->getInputRight().isDown();
		keys.keys[K_SHOOT] = hnd->getInputShoot().isDown();
		keys.keys[K_JUMP] = hnd->getInputJump().isDown();
		keys.keys[K_SELWEAP] = hnd->getInputWeapon().isDown();
		keys.keys[K_ROPE] = hnd->getInputRope().isDown();
		if( tLXOptions->bOldSkoolRope )
			keys.keys[K_ROPE] = ( hnd->getInputJump().isDown() && hnd->getInputWeapon().isDown() );
	};
	bool ret = SendNetPacket( localTime, keys, bs );
	CalculateCurrentState( localTime );
	return ret;
};

// --------- Net sending-receiving functions and internal stuff independent of OLX ---------
enum 
{ 
	TICK_TIME_DIV = 10	// One frame each 10 milliseconds - should be equal for all clients and should be never changed
};

bool QuickDirtyCalculation;
bool ReCalculationNeeded;
unsigned ReCalculationTimeMs;
// Constants
unsigned PingTimeMs = 300;	// Send at least one packet in 10 ms - 10 packets per second, huge net load
// TODO: calculate DrawDelayMs from other client pings
unsigned DrawDelayMs = 100;	// Not used currently // Delay the drawing until all packets are received, otherwise worms will teleport
unsigned ReCalculationMinimumTimeMs = 100;	// Re-calculate not faster than 10 times per second - eats CPU
const unsigned CalculateChecksumTime = 5000; // Calculate checksum once per 5 seconds - should be equal for all clients

int NumPlayers = -1;
int LocalPlayer = -1;

unsigned long OlxTimeDiffMs; // In milliseconds
unsigned long CurrentTimeMs; // In milliseconds
unsigned long BackupTime;	// In TICK_TIME chunks
unsigned long ClearEventsLastTime;

struct KeysEvent_t
{
	KeyState_t keys;
	KeyState_t keysChanged;
};

// Sorted by player and time - time in TICK_TIME_DIV chunks
typedef std::map< int, KeysEvent_t > EventList_t;
EventList_t Events [MAX_WORMS];
std::vector< KeyState_t > OldKeys;
std::vector< unsigned long > LastPacketTime; // Time in TICK_TIME_DIV chunks
unsigned Checksum;
unsigned long ChecksumTime; // Time in ms
unsigned long InitialRandomSeed; // Used for LoadState()/SaveState()
int player2netID[MAX_WORMS];
int net2playerID[MAX_WORMS];

void getKeysForTime( unsigned long t, KeyState_t keys[MAX_WORMS], KeyState_t keysChanged[MAX_WORMS] )
{
	for( int i=0; i<MAX_WORMS; i++ )
	{
		EventList_t :: const_iterator it = Events[i].upper_bound(t);
		if( it != Events[i].begin() )
		{
			--it;
			keys[i] = it->second.keys;
			keysChanged[i] = it->second.keysChanged;
		}
		else
		{
			keys[i] = KeyState_t();
			keysChanged[i] = KeyState_t();
		}
	}
};

void Activate( unsigned long localTime, unsigned long randomSeed )
{
			OlxTimeDiffMs = localTime;
			NumPlayers = 0;
			netRandom.seed(randomSeed);
			for( int i=0; i<MAX_WORMS; i++ )
			{
				player2netID[i] = -1;
				net2playerID[i] = -1;
			}
			for( int i=0; i<MAX_WORMS; i++ )
			{
				Events[i].clear();
				if( cClient->getRemoteWorms()[i].isUsed() )
				{
					player2netID[NumPlayers] = cClient->getRemoteWorms()[i].getID();
					net2playerID[ cClient->getRemoteWorms()[i].getID() ] = NumPlayers;
					NumPlayers ++;
					cClient->getRemoteWorms()[i].NewNet_random.seed(randomSeed + i);
				};
			}
			LocalPlayer = -1;
			if( cClient->getNumWorms() > 0 )
				LocalPlayer = net2playerID[cClient->getWorm(0)->getID()];
			InitialRandomSeed = randomSeed;

			CurrentTimeMs = 0;
			BackupTime = 0;
			ClearEventsLastTime = 0;
			Checksum = 0;
			ChecksumTime = 0;
			OldKeys.clear();
			OldKeys.resize(MAX_PLAYERS);
			LastPacketTime.clear();
			LastPacketTime.resize( MAX_PLAYERS, 0 );
			QuickDirtyCalculation = true;
			ReCalculationNeeded = false;
			ReCalculationTimeMs = 0;
			if( ! SavedWormState )
				SavedWormState = new CWorm[MAX_WORMS];
			SaveState();
};

void EndRound()
{
	delete [] SavedWormState;
	SavedWormState = NULL;
};

void ReCalculateSavedState()
{
	if( CurrentTimeMs - ReCalculationTimeMs < ReCalculationMinimumTimeMs || ! ReCalculationNeeded )
		return; // Limit recalculation - it is CPU-intensive
	//if( ! ReCalculationNeeded )
	//	return;

	ReCalculationTimeMs = CurrentTimeMs;
	ReCalculationNeeded = false;

	// Re-calculate physics if the packet received is from the most laggy client
	unsigned long timeMin = LastPacketTime[LocalPlayer];
	for( int f=1; f<NumPlayers; f++ )
		if( LastPacketTime[f] < timeMin )
			timeMin = LastPacketTime[f];

	//printf("ReCalculate(): BackupTime %lu timeMin %lu\n", BackupTime, timeMin);
	if( BackupTime /* + DrawDelayMs / TICK_TIME_DIV */ + 1 >= timeMin )
		return;

	QuickDirtyCalculation = false;
	if( CurrentTimeMs != BackupTime * TICK_TIME_DIV )	// Last recalc time
		RestoreState();

	while( BackupTime /* + DrawDelayMs / TICK_TIME_DIV */ + 1 < timeMin )
	{
		BackupTime++;
		CurrentTimeMs = BackupTime * TICK_TIME_DIV;
		bool calculateChecksum = CurrentTimeMs % CalculateChecksumTime == 0;

		KeyState_t keys[MAX_WORMS];
		KeyState_t keysChanged[MAX_WORMS];
		getKeysForTime( BackupTime, keys, keysChanged );

		unsigned checksum = CalculatePhysics( CurrentTimeMs, keys, keysChanged, false, calculateChecksum );
		if( calculateChecksum )
		{
			Checksum = checksum;
			ChecksumTime = CurrentTimeMs;
			//printf("OlxMod time %lu checksum 0x%X\n", ChecksumTime, Checksum );
		};
	};

	SaveState();
	CurrentTimeMs = BackupTime * TICK_TIME_DIV;

	// Clean up old events - do not clean them if we're the server, clients may ask for them.
	/*
	// TODO: ensure every worm has at least one event left in the array, that's why commented this code out
	if( BackupTime - ClearEventsLastTime > 100 && LocalPlayer != 0 )
	{
		ClearEventsLastTime = BackupTime;
		Events.erase(Events.begin(), Events.lower_bound( BackupTime - 2 ));
	};
	*/
	QuickDirtyCalculation = true;
};

// Should be called immediately after SendNetPacket() with the same time value
void CalculateCurrentState( unsigned long localTime )
{
	localTime -= OlxTimeDiffMs;

	ReCalculateSavedState();

	//printf("Draw() time %lu oldtime %lu\n", localTime / TICK_TIME , CurrentTimeMs / TICK_TIME );

	while( CurrentTimeMs < localTime /*- DrawDelayMs*/ )
	{
		CurrentTimeMs += TICK_TIME_DIV;

		KeyState_t keys[MAX_WORMS];
		KeyState_t keysChanged[MAX_WORMS];
		getKeysForTime( CurrentTimeMs / TICK_TIME_DIV, keys, keysChanged );

		CalculatePhysics( CurrentTimeMs, keys, keysChanged, true, false );
	};
};

int NetPacketSize()
{
	// Change here if you'll modify Receive()/Send()
	return 4+1;	// First 4 bytes is time in 10-ms chunks, second byte - keypress idx
};

void AddEmptyPacket( unsigned long localTime, CBytestream * bs )
{
	localTime -= OlxTimeDiffMs;
	localTime /= TICK_TIME_DIV;
	bs->writeInt( localTime, 4 );
	bs->writeByte( UCHAR_MAX );
};

unsigned EmptyPacketTime()
{
	return PingTimeMs;
};

// Returns true if data was re-calculated.
bool ReceiveNetPacket( CBytestream * bs, int player )
{
	player = net2playerID[player];
	unsigned long timeDiff = bs->readInt( 4 );	// TODO: 1-2 bytes are enough, I just screwed up with calculations

	unsigned long fullTime = timeDiff;

	KeyState_t keys = OldKeys[ player ];
	int keyIdx = bs->readByte();
	if( keyIdx != UCHAR_MAX )
		keys.keys[keyIdx] = ! keys.keys[keyIdx];

	OldKeys[ player ] = keys;
	Events[ player ] [ fullTime ] .keys = keys;
	if( keyIdx != UCHAR_MAX )
		Events[ player ] [ fullTime ] .keysChanged.keys[keyIdx] = ! Events[ player ] [ fullTime ] .keysChanged.keys[keyIdx];
	LastPacketTime[ player ] = fullTime;

	ReCalculationNeeded = true;
	// We don't want to calculate with just 1 of 2 keys pressed - it will desync
	// Net engine will send them in single packet anyway, so they are coupled together
	//ReCalculateSavedState(); // Called from Frame() anyway, 

	return true;
};

// Should be called for every gameloop frame with current key state, returns true if there's something to send
// Draw() should be called after this func
bool SendNetPacket( unsigned long localTime, KeyState_t keys, CBytestream * bs )
{
	//printf("SendNetPacket() time %lu\n", localTime);
	localTime -= OlxTimeDiffMs;
	localTime /= TICK_TIME_DIV;

	if( keys == OldKeys[ LocalPlayer ] &&
		localTime - LastPacketTime[ LocalPlayer ] < PingTimeMs / TICK_TIME_DIV ) // Do not flood the net with non-changed keys
		return false;

	KeyState_t changedKeys = OldKeys[ LocalPlayer ] ^ keys;

	//printf("SendNetPacket() put keys in time %lu\n", localTime);
	bs->writeInt( localTime, 4 );	// TODO: 1-2 bytes are enough, I just screwed up with calculations
	int changedKeyIdx = changedKeys.getFirstPressedKey();
	if( changedKeyIdx == -1 )
		bs->writeByte( UCHAR_MAX );
	else
	{
		// If we pressed 2 keys SendNetPacket() will be called two times
		bs->writeByte( changedKeyIdx );
		OldKeys[ LocalPlayer ].keys[ changedKeyIdx ] = ! OldKeys[ LocalPlayer ].keys[ changedKeyIdx ];
	}
	Events[ LocalPlayer ] [ localTime ] .keys = OldKeys[ LocalPlayer ];
	if( changedKeyIdx != -1 )
		Events[ LocalPlayer ] [ localTime ] .keysChanged.keys[changedKeyIdx] = ! Events[ LocalPlayer ] [ localTime ] .keysChanged.keys[changedKeyIdx];

	LastPacketTime[ LocalPlayer ] = localTime;

	if( NumPlayers == 1 )
		ReCalculationNeeded = true;

	return true;
};

unsigned GetChecksum( unsigned long * time )
{
	if( time )
		*time = ChecksumTime;
	return Checksum;
};


// -------- Misc funcs, boring implementation of randomizer and keys bit funcs -------------

#define LCG(n) ((69069UL * n) & 0xffffffffUL)
#define MASK 0xffffffffUL

void ___Random_Seed__(unsigned long s, __taus113_state_t & NetSyncedRandom_state)
{
  if (!s)
    s = 1UL;                    /* default seed is 1 */

  NetSyncedRandom_state.z1 = LCG (s);
  if (NetSyncedRandom_state.z1 < 2UL)
    NetSyncedRandom_state.z1 += 2UL;
  NetSyncedRandom_state.z2 = LCG (NetSyncedRandom_state.z1);
  if (NetSyncedRandom_state.z2 < 8UL)
    NetSyncedRandom_state.z2 += 8UL;
  NetSyncedRandom_state.z3 = LCG (NetSyncedRandom_state.z2);
  if (NetSyncedRandom_state.z3 < 16UL)
    NetSyncedRandom_state.z3 += 16UL;
  NetSyncedRandom_state.z4 = LCG (NetSyncedRandom_state.z3);
  if (NetSyncedRandom_state.z4 < 128UL)
    NetSyncedRandom_state.z4 += 128UL;

  /* Calling RNG ten times to satify recurrence condition */
  ___Random__(NetSyncedRandom_state);
  ___Random__(NetSyncedRandom_state);
  ___Random__(NetSyncedRandom_state);
  ___Random__(NetSyncedRandom_state);
  ___Random__(NetSyncedRandom_state);
  ___Random__(NetSyncedRandom_state);
  ___Random__(NetSyncedRandom_state);
  ___Random__(NetSyncedRandom_state);
  ___Random__(NetSyncedRandom_state);
  ___Random__(NetSyncedRandom_state);

  return;
};

#undef LCG
#undef MASK

	KeyState_t::KeyState_t()
	{
		for( int i=0; i<K_MAX; i++ )
			keys[i] = false;
	};

	bool KeyState_t::operator == ( const KeyState_t & k ) const
	{
		for( int i=0; i<K_MAX; i++ )
			if( keys[i] != k.keys[i] )
				return false;
		return true;
	};

	KeyState_t KeyState_t::operator & ( const KeyState_t & k ) const	// and
	{
		KeyState_t res;
		for( int i=0; i<K_MAX; i++ )
			if( keys[i] && k.keys[i] )
				res.keys[i] = true;
		return res;
	}

	KeyState_t KeyState_t::operator | ( const KeyState_t & k ) const	// or
	{
		KeyState_t res;
		for( int i=0; i<K_MAX; i++ )
			if( keys[i] || k.keys[i] )
				res.keys[i] = true;
		return res;
	}
	
	KeyState_t KeyState_t::operator ^ ( const KeyState_t & k ) const	// xor
	{
		KeyState_t res;
		for( int i=0; i<K_MAX; i++ )
			if( keys[i] != k.keys[i] )
				res.keys[i] = true;
		return res;
	}

	KeyState_t KeyState_t::operator ~ () const	// not
	{
		KeyState_t res;
		for( int i=0; i<K_MAX; i++ )
			if( ! keys[i] )
				res.keys[i] = true;
		return res;
	}

	int KeyState_t::getFirstPressedKey() const // Returns idx of first pressed key, or -1
	{
		for( int i=0; i<K_MAX; i++ )
			if( keys[i] )
				return i;
		return -1;
	}

	unsigned long NetSyncedRandom::getSeed()
	{
		return (~(unsigned long)time(NULL)) + SDL_GetTicks();
	};
};
