
#include <string>

#include "NewNetEngine.h"
#include "MathLib.h"
#include "Sounds.h"
#include "FindFile.h"
#include "CBytestream.h"

namespace NewNet
{

enum 
{ 
	TICK_TIME_DIV = 10	// One frame each 10 milliseconds
};

bool QuickDirtyCalculation;
bool ReCalculationNeeded;
unsigned ReCalculationTimeMs;
// Constants
unsigned PingTimeMs = 300;	// Send at least one packet in 10 ms - 10 packets per second, huge net load
unsigned DrawDelayMs = 100;	// Delay the drawing until all packets are received, otherwise it's very jumpy
unsigned ReCalculationMinimumTimeMs = 100;	// Re-calculate not faster than 10 times per second - eats CPU
const unsigned CalculateChecksumTime = 5000; // Calculate checksum once per 5 seconds - should be equal for all clients
// TODO: calculate DrawDelay from other client pings

int NumPlayers = -1;
int LocalPlayer = -1;

unsigned long OlxTimeDiffMs; // In milliseconds
unsigned long CurrentTimeMs; // In milliseconds
unsigned long BackupTime;	// In TICK_TIME chunks
unsigned long ClearEventsLastTime;

// Sorted by time and player - time in TICK_TIME chunks
typedef std::map< unsigned long, std::map< int, KeyState_t > > EventList_t;
EventList_t Events;
std::vector< KeyState_t > OldKeys;
std::vector< unsigned long > LastPacketTime; // Time in TICK_TIME chunks
unsigned Checksum;
unsigned long ChecksumTime; // Time in ms
unsigned long InitialRandomSeed; // Used for LoadState()/SaveState()
int player2netID[MAX_WORMS];
int net2playerID[MAX_WORMS];

void Activate( unsigned long localTime, unsigned long randomSeed )
{
			OlxTimeDiffMs = localTime;
			NumPlayers = 0;
			for( int i=0; i<MAX_WORMS; i++ )
			{
				player2netID[i] = -1;
				net2playerID[i] = -1;
			}
			for( int i=0; i<MAX_WORMS; i++ )
				if( cClient->getRemoteWorms()[i].isUsed() )
				{
					player2netID[NumPlayers] = cClient->getRemoteWorms()[i].getID();
					net2playerID[ cClient->getRemoteWorms()[i].getID() ] = NumPlayers;
					NumPlayers ++;
				};
			LocalPlayer = -1;
			if( cClient->getNumWorms() > 0 )
				LocalPlayer = net2playerID[cClient->getWorm(0)->getID()];
			InitialRandomSeed = randomSeed;

			CurrentTimeMs = 0;
			BackupTime = 0;
			ClearEventsLastTime = 0;
			Checksum = 0;
			ChecksumTime = 0;
			Events.clear();
			OldKeys.clear();
			OldKeys.resize(MAX_PLAYERS);
			LastPacketTime.clear();
			LastPacketTime.resize( MAX_PLAYERS, 0 );
			QuickDirtyCalculation = true;
			ReCalculationNeeded = false;
			ReCalculationTimeMs = 0;
			NetSyncedRandom_Seed( randomSeed );
			NetSyncedRandom_Save();
			Random_Seed( ~ randomSeed );
			SaveState();
};

void EndRound()
{
};

void ReCalculate()
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
	{
		NetSyncedRandom_Restore();
		(*list)[ActiveMod].restore();
	};

	std::map< int, Event_t > emptyEvent;
	while( BackupTime /* + DrawDelayMs / TICK_TIME_DIV */ + 1 < timeMin )
	{
		BackupTime++;
		CurrentTimeMs = BackupTime * TICK_TIME_DIV;
		bool calculateChecksum = CurrentTimeMs % CalculateChecksumTime == 0;
		unsigned checksum = (*list)[ActiveMod].calc(
			CurrentTimeMs * TICK_TIME_DIV / TickTime,
			Events.find(BackupTime) != Events.end() ? Events[BackupTime] : emptyEvent,
			false, calculateChecksum );	// Do the true thorough calculations
		if( calculateChecksum )
		{
			Checksum = checksum;
			ChecksumTime = CurrentTimeMs;
			//printf("OlxMod time %lu checksum 0x%X\n", ChecksumTime, Checksum );
		};
	};

	NetSyncedRandom_Save();
	(*list)[ActiveMod].save();
	CurrentTimeMs = BackupTime * TICK_TIME_DIV;

	// Clean up old events - do not clean them if we're the server, clients may ask for them.
	if( BackupTime - ClearEventsLastTime > 100 && LocalPlayer != 0 )
	{
		ClearEventsLastTime = BackupTime;
		Events.erase(Events.begin(), Events.lower_bound( BackupTime - 2 ));
	};
	QuickDirtyCalculation = true;
};

// Should be called immediately after SendNetPacket() with the same time value
void Draw( unsigned long localTime, bool showScoreboard )
{
	localTime -= OlxTimeDiffMs;

	ReCalculate();

	//printf("Draw() time %lu oldtime %lu\n", localTime / TICK_TIME , CurrentTimeMs / TICK_TIME );

	std::map< int, Event_t > emptyEvent;

	while( CurrentTimeMs < localTime /*- DrawDelayMs*/ )
	{
		CurrentTimeMs += TICK_TIME_DIV;
		(*list)[ActiveMod].calc( CurrentTimeMs * TICK_TIME_DIV / TickTime ,
			Events.find(CurrentTimeMs / TICK_TIME_DIV) != Events.end() ?
			Events[CurrentTimeMs / TICK_TIME_DIV] : emptyEvent,
			true, false );	// Do fast inexact calculations
	};

	(*list)[ActiveMod].draw( showScoreboard );

};

int NetPacketSize()
{
	// Change here if you'll modify Receive()/Send()
	return 4+1;	// First 4 bytes is time in 10-ms chunks, second 2 bytes - keypress mask
};

void WriteKeys( const KeyState_t & keys, CBytestream * bs )
{
	// TODO: send not key bitmask but changed key index - will limit traffic to 1 byte
	bs->ResetBitPos();
	bs->writeBit( keys.up );
	bs->writeBit( keys.down );
	bs->writeBit( keys.left );
	bs->writeBit( keys.right );
	bs->writeBit( keys.shoot );
	bs->writeBit( keys.jump );
	bs->writeBit( keys.selweap );
	bs->writeBit( keys.rope );
	//bs->writeBit( keys.strafe );	// TODO: adds second byte with just 1 bit used
	bs->ResetBitPos();
};

void ReadKeys( KeyState_t * keys, CBytestream * bs )
{
	bs->ResetBitPos();
	keys->up = bs->readBit();
	keys->down = bs->readBit();
	keys->left = bs->readBit();
	keys->right = bs->readBit();
	keys->shoot = bs->readBit();
	keys->jump = bs->readBit();
	keys->selweap = bs->readBit();
	keys->rope = bs->readBit();
	//keys->strafe = bs->readBit();
	bs->ResetBitPos();
};

void CompareKeysChanged( const KeyState_t & keys1, const KeyState_t & keys2, KeyState_t * keysChanged )
{
	keysChanged->up = keys1.up != keys2.up;
	keysChanged->down = keys1.down != keys2.down;
	keysChanged->left = keys1.left != keys2.left;
	keysChanged->right = keys1.right != keys2.right;
	keysChanged->shoot = keys1.shoot != keys2.shoot;
	keysChanged->jump = keys1.jump != keys2.jump;
	keysChanged->selweap = keys1.selweap != keys2.selweap;
	keysChanged->rope = keys1.rope != keys2.rope;
	keysChanged->strafe = keys1.strafe != keys2.strafe;
};

void AddEmptyPacket( unsigned long localTime, CBytestream * bs )
{
	localTime -= OlxTimeDiffMs;
	localTime /= TICK_TIME_DIV;
	bs->writeInt( localTime, 4 );
	KeyState_t keys;	// All keys are false by default
	WriteKeys( keys, bs );
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
	/*
	// TODO: better time calculations?
	unsigned long fullTime = ( CurrentTimeMs / TICK_TIME / 0x10000 ) * 0x10000 + timeDiff;
	if( fullTime > CurrentTimeMs / TICK_TIME + 0x10000/2 )
		fullTime -= 0x10000;
	*/
	unsigned long fullTime = timeDiff;

	KeyState_t keys;
	ReadKeys( &keys, bs );

	KeyState_t changedKeys = OldKeys[ player ].keys;
	CompareKeysChanged( keys, changedKeys, &changedKeys );

	OldKeys[ player ] = keys;

	Events[ fullTime ] [ player ] = Event_t( keys, changedKeys );
	LastPacketTime[ player ] = fullTime;

	ReCalculationNeeded = true;
	ReCalculate();

	// debug - remove it
	/*
	// If sums here are different that means the CChannel missed some packets
	unsigned checksum = 0;
	unsigned checksumTime = 1;
	//printf("====================== Events\n");
	for( EventList_t::iterator it = Events.begin(); it != Events.lower_bound(BackupTime); it++ )
	{
		//printf("Time %lu", it->first );
		checksum += it->first * 10000;
		for( std::map< int, Event_t > :: iterator it1 = it->second.begin(); it1 != it->second.end(); it1++ )
			checksum += (it1->first*0x120+1) * (it1->second.keys.up*1 + it1->second.keys.down*2 +
				it1->second.keys.left*4 + it1->second.keys.right*8 + it1->second.keys.shoot*16 +
				it1->second.keys.selweap*32 + it1->second.keys.jump*64 + it1->second.keys.rope*128 );
			//printf(" player %i keys %i%i%i%i%i%i%i%i", it1->first, it1->second.keys.up, it1->second.keys.down,
			//	it1->second.keys.left, it1->second.keys.right, it1->second.keys.shoot, it1->second.keys.selweap, it1->second.keys.jump, it1->second.keys.rope );
		if( it->first / 500 > checksumTime )
		{
			printf("Events Keys checksum for time %lu = 0x%X\n", it->first, checksum );
			checksumTime = 1 + it->first / 500;
		};
	};
	*/

	return true;
};

// Should be called for every gameloop frame with current key state, returns true if there's something to send
// Draw() should be called after this func
bool SendNetPacket( unsigned long localTime, KeyState_t keys, CBytestream * bs )
{
	//printf("SendNetPacket() time %lu\n", localTime);
	localTime -= OlxTimeDiffMs;
	localTime /= TICK_TIME_DIV;

	// Ignore too frequent keypresses, or we'll get net desync
	// The keypress will be processed on the next frame if user holds the key long enough (e.x. for 20 ms)
	if( localTime == CurrentTimeMs / TICK_TIME_DIV )
		return false;

	if( keys == OldKeys[ LocalPlayer ].keys &&
		localTime - LastPacketTime[ LocalPlayer ] < PingTimeMs / TICK_TIME_DIV ) // Do not flood the net
		return false;

	KeyState_t changedKeys = OldKeys[ LocalPlayer ].keys;
	CompareKeysChanged( keys, changedKeys, &changedKeys );

	Events[ localTime ] [ LocalPlayer ] = Event_t( keys, changedKeys );
	//printf("SendNetPacket() put keys in time %lu\n", localTime);
	bs->writeInt( localTime, 4 );	// TODO: 1-2 bytes are enough, I just screwed up with calculations
	WriteKeys( keys, bs );

	OldKeys[ LocalPlayer ] = keys;

	LastPacketTime[ LocalPlayer ] = localTime;

	if( NumPlayers == 1 )
		ReCalculationNeeded = true;

	return true;
};

bool Frame( unsigned long localTime, KeyState_t keys, CBytestream * bs, bool showScoreboard )
{
	bool ret = SendNetPacket( localTime, keys, bs );
	Draw( localTime, showScoreboard );
	return ret;
};

unsigned GetChecksum( unsigned long * time )
{
	if( time )
		*time = ChecksumTime;
	return Checksum;
};

void SaveState()
{
};
void RestoreState()
{
};

unsigned CalculatePhysics( unsigned gameTime, const std::map< int, KeyState_t > &keys, bool fastCalculation, bool calculateChecksum )
{
};


taus113_state_t NetSyncedRandom_state, NetSyncedRandom_state_saved, Random_state;

#define LCG(n) ((69069UL * n) & 0xffffffffUL)
#define MASK 0xffffffffUL

void __Random_Seed__(unsigned long s, taus113_state_t & NetSyncedRandom_state)
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
  NetSyncedRandom();
  NetSyncedRandom();
  NetSyncedRandom();
  NetSyncedRandom();
  NetSyncedRandom();
  NetSyncedRandom();
  NetSyncedRandom();
  NetSyncedRandom();
  NetSyncedRandom();
  NetSyncedRandom();

  return;
};

void NetSyncedRandom_Save()
{
	NetSyncedRandom_state_saved = NetSyncedRandom_state;
};
void NetSyncedRandom_Restore()
{
	NetSyncedRandom_state = NetSyncedRandom_state_saved;
};

void NetSyncedRandom_Seed(unsigned long s)
{
	__Random_Seed__(s, NetSyncedRandom_state);
}

void Random_Seed(unsigned long s)
{
	__Random_Seed__(s, Random_state);
}


#undef LCG
#undef MASK

	KeyState_t::KeyState_t()
	{
		for( int i=0; i<K_MAX; i++ )
			keys[i] = false;
	};

	bool KeyState_t::operator == ( const KeyState_t & k )
	{
		for( int i=0; i<K_MAX; i++ )
			if( keys[i] != k.keys[i] )
				return false;
		return true;
	};

	KeyState_t KeyState_t::operator & ( const KeyState_t & k )	// and
	{
		KeyState_t res;
		for( int i=0; i<K_MAX; i++ )
			if( keys[i] && k.keys[i] )
				res.keys[i] = true;
		return res;
	}

	KeyState_t KeyState_t::operator | ( const KeyState_t & k )	// or
	{
		KeyState_t res;
		for( int i=0; i<K_MAX; i++ )
			if( keys[i] || k.keys[i] )
				res.keys[i] = true;
		return res;
	}
	
	KeyState_t KeyState_t::operator ^ ( const KeyState_t & k )	// xor
	{
		KeyState_t res;
		for( int i=0; i<K_MAX; i++ )
			if( keys[i] != k.keys[i] )
				res.keys[i] = true;
		return res;
	}

	KeyState_t KeyState_t::operator ~ ()	// not
	{
		KeyState_t res;
		for( int i=0; i<K_MAX; i++ )
			if( ! keys[i] )
				res.keys[i] = true;
		return res;
	}


};
