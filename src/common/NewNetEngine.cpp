
#include <string>
#include <limits.h>
#include <time.h>

#include "NewNetEngine.h"
#include "MathLib.h"
#include "FindFile.h"
#include "CBytestream.h"
#include "CClient.h"
#include "CMap.h"
#include "CWorm.h"
#include "ProfileSystem.h"
#include "CWormHuman.h"
#include "Entity.h"
#include "CServer.h"

namespace NewNet
{

// -------- The stuff that interacts with OLX: save/restore game state and calculate physics ---------

CWorm * SavedWormState = NULL;
NetSyncedRandom netRandom, netRandom_Saved;
AbsTime cClientLastSimulationTime;

void SaveState()
{
	netRandom_Saved = netRandom;
	cClientLastSimulationTime = cClient->fLastSimulationTime;
	cClient->getMap()->NewNet_SaveToMemory();

	cClient->NewNet_SaveProjectiles();
	NewNet_SaveEntities();

	for( int i = 0; i < MAX_WORMS; i++ )
		SavedWormState[i].NewNet_CopyWormState( cClient->getRemoteWorms()[i] );
};

void RestoreState()
{
	netRandom = netRandom_Saved;
	cClient->fLastSimulationTime = cClientLastSimulationTime;
	cClient->getMap()->NewNet_RestoreFromMemory();

	cClient->NewNet_LoadProjectiles();
	NewNet_LoadEntities();

	for( int i = 0; i < MAX_WORMS; i++ )
		cClient->getRemoteWorms()[i].NewNet_CopyWormState( SavedWormState[i] );
};

// TODO: make respawning server-sided and remove this function
static CVec NewNet_FindSpot(CWorm *Worm) // Avoid name conflict with CServer::FindSpot
{
	// This should use net synced Worm->NewNet_random for now, later I'll use respawn routines from CServer

	float w = cClient->getMap()->GetWidth();
	float h = cClient->getMap()->GetHeight();
	CMap::PixelFlagAccess flags(cClient->getMap());
	
	// Find a random cell to start in - retry if failed
	size_t tries = 1000;
	while(tries-- > 0) {
		CVec pos;
		pos.x = Worm->NewNet_random.getFloatPositive() * w;
		pos.y = Worm->NewNet_random.getFloatPositive() * h;

		if(!cClient->getMap()->IsGoodSpawnPoint(flags, pos))
			continue;
		
		return pos;
	}
	
	errors << "NewNet_FindSpot: cannot find spot" << endl;
	return CVec();
}


void DisableAdvancedFeatures()
{
	 // Disables bonuses and connect-during-game for now, 
	 // I can add bonuses but connect-during-game is complicated
	 tLXOptions->tGameInfo.bBonusesOn = false;
	 tLXOptions->tGameInfo.bAllowConnectDuringGame = false;
	 tLXOptions->tGameInfo.bAllowConnectDuringGame = false;
	 tLXOptions->tGameInfo.features[FT_ImmediateStart] = false;
	 tLXOptions->tGameInfo.features[FT_AllowWeaponsChange] = false;
	 //tLXOptions->tGameInfo.fRespawnTime = 2.5f; // We just ignore it now
	 //tLXOptions->tGameInfo.bRespawnGroupTeams = false;
	 //tLXOptions->tGameInfo.bEmptyWeaponsOnRespawn = false;
	 //*cClient->getGameLobby() = tLXOptions->tGameInfo;
};

void CalculateCurrentState( AbsTime localTime );
bool SendNetPacket( AbsTime localTime, KeyState_t keys, CBytestream * bs );

// --------- Net sending-receiving functions and internal stuff independent of OLX ---------

bool NewNetActive = false;
bool QuickDirtyCalculation = false;
bool ReCalculationNeeded = true;
AbsTime ReCalculationTimeMs;
// Constants
TimeDiff PingTimeMs = TimeDiff(250);	// Send at least one packet in 10 ms - 10 packets per second, huge net load
// TODO: calculate DrawDelayMs from other client pings
// TimeDiff DrawDelayMs = TimeDiff(100);	// Not used currently // Delay the drawing until all packets are received, otherwise worms will teleport
TimeDiff ReCalculationMinimumTimeMs = TimeDiff(150);	// Re-calculate not faster than 7.5 times per second - eats CPU
TimeDiff CalculateChecksumTime = TimeDiff(10000); // Calculate checksum once per 10 seconds - should be equal for all clients

int NumPlayers = -1;
int LocalPlayer = -1;

// TODO: why is it named diff but used absolute?
AbsTime OlxTimeDiffMs; // In milliseconds
AbsTime CurrentTimeMs; // In milliseconds
AbsTime BackupTime; // In milliseconds
AbsTime ClearEventsLastTime;

struct KeysEvent_t
{
	KeyState_t keys;
	KeyState_t keysChanged;
};

// Sorted by player and time - time in milliseconds
typedef std::map< AbsTime, KeysEvent_t > EventList_t;
EventList_t Events [MAX_WORMS];
KeyState_t OldKeys[MAX_WORMS];
AbsTime LastPacketTime[MAX_WORMS];
AbsTime LastSoundPlayedTime[MAX_WORMS];
unsigned Checksum;
AbsTime ChecksumTime; 
AbsTime OldChecksumTime;
//int InitialRandomSeed; // Used for LoadState()/SaveState()
bool playersLeft[MAX_WORMS];


// GameTime is started from 0, for calculating exact physics the Physics() is called consecutively in chunks of 10 Ms 
// The exception for this is when we are called from CalculateCurrentState() - 
// it will call Physics() with fastCalculation flag set to true and arbitrary gameTime -
// we are allowed to skip some collision checks and to revert to faster routines in that case, 
// as long as the game image on the screen will look smooth - we will discard that results anyway.
// Keys is the state of keys for given player.
// If calculateChecksum set to true the Physics() should return checksum of game state (at least current net synced random number).

unsigned CalculatePhysics( AbsTime gameTime, KeyState_t keys[MAX_WORMS], KeyState_t keysChanged[MAX_WORMS], bool fastCalculation, bool calculateChecksum )
{
	unsigned checksum = 0;
	for( int i = 0; i < MAX_WORMS; i++ )
	{
		CWorm * w = & cClient->getRemoteWorms()[i];
		if( playersLeft[i] && LastPacketTime[i] <= gameTime + TimeDiff(TICK_TIME) )
		{
			w->setUsed(false);
			if( CanUpdateGameState() )
				playersLeft[i] = false;
		};
		if( w->isUsed() )
		{
			// Respawn dead worms
			// TODO: make this server-sided
			if( !w->getAlive() && w->getLives() != WRM_OUT )
				if( gameTime > w->getTimeofDeath() + 2.5f )
				{
					CVec spot = NewNet_FindSpot(w);
					cClient->getMap()->CarveHole(SPAWN_HOLESIZE, spot, (bool)cClient->getGameLobby()->features[FT_InfiniteMap]);
					w->Spawn( spot );
					// Show a spawn entity
					SpawnEntity(ENT_SPAWN,0,spot,CVec(0,0),Color(),NULL);
				}

			w->NewNet_SimulateWorm( keys[i], keysChanged[i] );
				
			if( calculateChecksum )
				checksum += ( w->getID() % 4 + 1 ) * 
					( (int)w->getPos().x + (int)w->getPos().y * 0x100 + (int)w->getHealth() * 0x100000 + 
					w->NewNet_random.getChecksum() );
		}
	}

	cClient->NewNet_Simulation();

	if( calculateChecksum )
	{
		for( int i=0; i<100; i++ ) // First 100 projectiles are enough to tell if we synced or not I think
		{
			if( cClient->getProjectiles()[i].isUsed() )
			{
				checksum += ( i % 8 + 1 ) * (
					(int)cClient->getProjectiles()[i].getPos().x + 
					(int)cClient->getProjectiles()[i].getPos().y * 0x100 );
			}
		}
	}
	return checksum;
};

#ifdef DEBUG
static unsigned getMapChecksum()
{
	unsigned checksum = 0;
	cClient->getMap()->lockFlags();
	for(size_t y = 0; y < cClient->getMap()->GetHeight(); ++y)
		for(size_t x = 0; x < cClient->getMap()->GetWidth(); ++x)
			checksum += ( ( cClient->getMap()->GetPixelFlag(x,y) & PX_EMPTY ) + ( cClient->getMap()->GetPixelFlag(x,y) & PX_DIRT ) * 2 ) * ( (y*cClient->getMap()->GetWidth()+x) % 0x1000000 + 1 );
	cClient->getMap()->unlockFlags();
	return checksum;
}
#endif

void PlayerLeft(int id)
{
	playersLeft[id] = true;
};


void getKeysForTime( AbsTime t, KeyState_t keys[MAX_WORMS], KeyState_t keysChanged[MAX_WORMS] )
{
	for( int i=0; i<MAX_WORMS; i++ )
	{
		EventList_t :: const_iterator it = Events[i].upper_bound(t);
		if( it != Events[i].begin() )
		{
			--it;
			keys[i] = it->second.keys;
			if( it->first == t )
				keysChanged[i] = it->second.keysChanged;
			else
				keysChanged[i] = KeyState_t();
		}
		else
		{
			keys[i] = KeyState_t();
			keysChanged[i] = KeyState_t();
		}
	}
};

void StartRound( unsigned randomSeed )
{
			OlxTimeDiffMs = tLX->currentTime;
				
			cClient->fLastSimulationTime = 0;
			
			//InitialRandomSeed = randomSeed;

			CurrentTimeMs = 0;
			BackupTime = 0;
			ClearEventsLastTime = 0;
			Checksum = 0;
			ChecksumTime = 0;
			OldChecksumTime = 0;
			QuickDirtyCalculation = true;
			ReCalculationNeeded = false;
			ReCalculationTimeMs = 0;
			NewNetActive = true;
			if( ! SavedWormState )
				SavedWormState = new CWorm[MAX_WORMS];

			NumPlayers = 0;
			netRandom.seed(randomSeed);
			for( int i=0; i<MAX_WORMS; i++ )
			{
				playersLeft[i] = false;
				Events[i].clear();
				OldKeys[i] = KeyState_t();
				LastPacketTime[i] = AbsTime();
				LastSoundPlayedTime[i] = AbsTime();
				if( cClient->getRemoteWorms()[i].isUsed() )
				{
					NumPlayers ++;
					cClient->getRemoteWorms()[i].NewNet_InitWormState(randomSeed + i);
					CVec spot = NewNet_FindSpot( &cClient->getRemoteWorms()[i] );
					cClient->getMap()->CarveHole(SPAWN_HOLESIZE, spot, (bool)cClient->getGameLobby()->features[FT_InfiniteMap]);
					cClient->getRemoteWorms()[i].Spawn( spot );
				};
			}
			LocalPlayer = -1;
			if( cClient->getNumWorms() > 0 )
				LocalPlayer = cClient->getWorm(0)->getID();

			SaveState();
};

void EndRound()
{
	RestoreState();
	cClient->getMap()->NewNet_Deinit();
	delete [] SavedWormState;
	SavedWormState = NULL;
	NewNetActive = false;
};

bool Frame( CBytestream * bs )
{
	AbsTime localTime = tLX->currentTime;
	localTime.time -= OlxTimeDiffMs.time;
	localTime.time -= localTime.time % TICK_TIME;

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
		keys.keys[K_STRAFE] = hnd->getInputStrafe().isDown();
	};
	
	bool ret = SendNetPacket( localTime, keys, bs );
	if( !ret )
	{
		if( cClient->getNumWorms() > 0 && cClient->getWorm(0)->getType() == PRF_HUMAN )
			cClient->getWorm(0)->inputHandler()->clearInput();
		CalculateCurrentState( localTime );
	}
	return ret;
};

void ReCalculateSavedState()
{
	if( CurrentTimeMs < ReCalculationTimeMs + ReCalculationMinimumTimeMs || ! ReCalculationNeeded )
		return; // Limit recalculation - it is CPU-intensive

	ReCalculationTimeMs = CurrentTimeMs;
	ReCalculationNeeded = false;

	// Re-calculate physics if the packet received is from the most laggy client
	AbsTime timeMin = LastPacketTime[LocalPlayer];
	for( int f=0; f<MAX_WORMS; f++ )
		if( LastPacketTime[f] < timeMin && cClient->getRemoteWorms()[f].isUsed() )
			timeMin = LastPacketTime[f];

	if( BackupTime + TimeDiff(TICK_TIME) + TimeDiff(TICK_TIME) >= timeMin ) // Safety hack
		return;

	QuickDirtyCalculation = false;
	RestoreState();

	while( BackupTime + TimeDiff(TICK_TIME) + TimeDiff(TICK_TIME) < timeMin ) // Safety hack
	{
		BackupTime += TimeDiff(TICK_TIME);
		CurrentTimeMs = BackupTime;
		bool calculateChecksum = CurrentTimeMs.time % CalculateChecksumTime.timeDiff == 0 || CurrentTimeMs.time == TICK_TIME;

		KeyState_t keys[MAX_WORMS];
		KeyState_t keysChanged[MAX_WORMS];
		getKeysForTime( BackupTime, keys, keysChanged );

		unsigned checksum = CalculatePhysics( CurrentTimeMs, keys, keysChanged, false, calculateChecksum );
		if( calculateChecksum )
		{
			Checksum = checksum;
			ChecksumTime = CurrentTimeMs;
			int KeysCheck = 0;
			for( int f=0; f < MAX_WORMS; f++ )
			{
				for( EventList_t::iterator it = Events[f].begin(); it != Events[f].end(); it++ )
					if( it->first <= ChecksumTime )
						KeysCheck += (it->second.keys.getBitmask() + it->second.keysChanged.getBitmask() * 0x1000) * ( f % 4 + 1 );
			}
			#ifdef DEBUG
			hints << "ReCalculateSavedState() time " << ChecksumTime.time << " checksum " << Checksum << 
					" Keys check " << KeysCheck << " map " << getMapChecksum() << endl;
			#endif
		};
	};

	SaveState();
	CurrentTimeMs = BackupTime;

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
void CalculateCurrentState( AbsTime localTime )
{
	ReCalculateSavedState();

	while( CurrentTimeMs < localTime /*- DrawDelayMs*/ )
	{
		CurrentTimeMs += TimeDiff(TICK_TIME);

		KeyState_t keys[MAX_WORMS];
		KeyState_t keysChanged[MAX_WORMS];
		getKeysForTime( CurrentTimeMs, keys, keysChanged );

		CalculatePhysics( CurrentTimeMs, keys, keysChanged, true, false );
	};
};

int NetPacketSize()
{
	// Change here if you'll modify Receive()/Send()
	return 4+1;	// First 4 bytes is time, second byte - keypress idx
}

// Returns true if data was re-calculated.
void ReceiveNetPacket( CBytestream * bs, int player )
{
	int timeDiff = bs->readInt( 4 );	// TODO: 1-2 bytes are enough, I just screwed up with calculations

	AbsTime fullTime(timeDiff);

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
};

// Should be called for every gameloop frame with current key state, returns true if there's something to send
// Draw() should be called after this func
bool SendNetPacket( AbsTime localTime, KeyState_t keys, CBytestream * bs )
{
	if( keys == OldKeys[ LocalPlayer ] &&
		localTime < LastPacketTime[ LocalPlayer ] + PingTimeMs ) // Do not flood the net with non-changed keys
		return false;

	KeyState_t changedKeys = OldKeys[ LocalPlayer ] ^ keys;

	bs->writeInt( (int)localTime.time, 4 );	// TODO: 1-2 bytes are enough, I just screwed up with calculations
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
		Events[ LocalPlayer ] [ localTime ] .keysChanged.keys[changedKeyIdx] = true;

	LastPacketTime[ LocalPlayer ] = localTime;

	if( NumPlayers == 1 )
		ReCalculationNeeded = true;

	return true;
};

unsigned GetChecksum( AbsTime * time )
{
	if( time )
		*time = ChecksumTime;
	return Checksum;
};

bool ChecksumRecalculated()
{
	if( OldChecksumTime != ChecksumTime && ChecksumTime + PingTimeMs < BackupTime )
	{
		OldChecksumTime = ChecksumTime;
		return true;
	}
	return false;
}

AbsTime GetCurTime()
{
	return CurrentTimeMs;
}

bool CanUpdateGameState()
{
	return !QuickDirtyCalculation || !NewNetActive;
};

bool CanPlaySound(int wormID)
{
	if( !NewNetActive )
		return true;
	if( LastSoundPlayedTime[wormID] < CurrentTimeMs )
	{
		LastSoundPlayedTime[wormID] = CurrentTimeMs;
		return true;
	}
	return false;
};

bool Active()
{
	return NewNetActive;
}


// -------- Misc funcs, boring implementation of randomizer and keys bit funcs -------------

#define LCG(n) ((69069UL * n) & 0xffffffffUL)

void ___Random_Seed__(unsigned s, __taus113_state_t & NetSyncedRandom_state)
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

	int KeyState_t::getBitmask() const
	{
		int b = 0;
		for( int i=0; i<K_MAX; i++ )
			if( keys[i] )
				b |= 1 << i;
		return b;
	}
	
	unsigned NetSyncedRandom::getSeed()
	{
		return (~(unsigned)time(NULL)) + SDL_GetTicks();
	};
};
