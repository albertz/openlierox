
#include <string>

#include "OLXModInterface.h"
#include "MathLib.h"
#include "Sounds.h"
#include "FindFile.h"
#include "CBytestream.h"

namespace OlxMod
{

enum { OLXMOD_TICK_TIME_DIV = OlxMod_GameSpeed_Fastest };

unsigned OlxMod_TickTime = 10;
bool OlxMod_QuickDirtyCalculation;
bool OlxMod_ReCalculationNeeded;
unsigned OlxMod_ReCalculationTimeMs;
// Constants
unsigned OlxMod_PingTimeMs = 300;	// Send at least one packet in 10 ms - 10 packets per second, huge net load
unsigned OlxMod_DrawDelayMs = 100;	// Delay the drawing until all packets are received, otherwise it's very jumpy
unsigned OlxMod_ReCalculationMinimumTimeMs = 100;	// Re-calculate not faster than 10 times per second - eats CPU
const unsigned OlxMod_CalculateChecksumTime = 5000; // Calculate checksum once per 5 seconds - should be equal for all clients
// TODO: calculate OlxMod_DrawDelay from other client pings

// If seeded with the some value, the multiple calls to NetSyncedRandom() will return the same sequence on different machines
void OlxMod_NetSyncedRandom_Seed( unsigned long );
void OlxMod_NetSyncedRandom_Save();
void OlxMod_NetSyncedRandom_Restore();
void OlxMod_StopSoundSystem();

void OlxMod_Random_Seed( unsigned long );

struct OlxMod_t
{
	std::string name;
	std::string dirname;
	OlxMod_InitFunc_t init;
	OlxMod_DeInitFunc_t deinit;
	OlxMod_SaveState_t save;
	OlxMod_RestoreState_t restore;
	OlxMod_CalculatePhysics_t calc;
	OlxMod_Draw_t draw;
	OlxMod_GetOptions_t options;
};

std::vector<OlxMod_t> * OlxMod_list = NULL;

int OlxMod_ActiveMod = -1;

bool OlxMod_RegisterMod( const char * name, const char * dirname,
							OlxMod_InitFunc_t init, OlxMod_DeInitFunc_t deinit,
							OlxMod_SaveState_t save, OlxMod_RestoreState_t restore,
							OlxMod_CalculatePhysics_t calc, OlxMod_Draw_t draw, OlxMod_GetOptions_t options )
{
	if( OlxMod_list == NULL )
		OlxMod_list = new std::vector<OlxMod_t>;
	OlxMod_t mod;
	mod.name = name;
	mod.dirname = dirname;
	mod.init = init;
	mod.deinit = deinit;
	mod.save = save;
	mod.restore = restore;
	mod.calc = calc;
	mod.draw = draw;
	mod.options = options;
	OlxMod_list->push_back(mod);
	return true;
};

std::vector<std::string> OlxMod_GetModList()
{
	std::vector<std::string> r;
	if( OlxMod_list == NULL )
		return r;
	std::string fname;
	for( unsigned f=0; f<OlxMod_list->size(); f++ )
		if( GetExactFileName( (*OlxMod_list)[f].dirname, fname ) ) // The mod directory exists
			r.push_back( (*OlxMod_list)[f].name );
	return r;
};

bool OlxMod_IsModInList( const std::string & s )
{
	//printf("OlxMod_IsModInList(%s)\n", s.c_str());
	if( OlxMod_list == NULL )
		return false;
	for( unsigned f=0; f<OlxMod_list->size(); f++ )
		if( (*OlxMod_list)[f].name == s )
		{
			std::string fname;
			if( GetExactFileName( (*OlxMod_list)[f].dirname, fname ) )
				return true;
		};
	return false;
};

void OlxMod_DeleteModList()
{
	if( OlxMod_list == NULL )
		return;
	delete OlxMod_list;
	OlxMod_list = NULL;
};

std::string OlxMod_GetCurrentMod()
{
	if( OlxMod_list == NULL )
		return "";
	if( OlxMod_ActiveMod == -1 )
		return "";
	return (*OlxMod_list)[OlxMod_ActiveMod].name;
};

bool OlxMod_ModSelected()
{
	if( OlxMod_list == NULL )
		return false;
	if( OlxMod_ActiveMod == -1 )
		return false;
	return true;
};

std::map< std::string, CScriptableVars::ScriptVarType_t > OlxMod_GetModOptions()
{
	std::map< std::string, CScriptableVars::ScriptVarType_t > opts;
	std::vector<std::string> weaps;
	if( OlxMod_list == NULL )
		return opts;
	if( OlxMod_ActiveMod == -1 )
		return opts;
	(*OlxMod_list)[OlxMod_ActiveMod].options( &opts, &weaps );
	return opts;
};

std::vector<std::string> OlxMod_GetModWeaponList()
{
	std::map< std::string, CScriptableVars::ScriptVarType_t > opts;
	std::vector<std::string> weaps;
	if( OlxMod_list == NULL )
		return weaps;
	if( OlxMod_ActiveMod == -1 )
		return weaps;
	(*OlxMod_list)[OlxMod_ActiveMod].options( &opts, &weaps );
	return weaps;
};

int OlxMod_NumPlayers = -1;
int OlxMod_LocalPlayer = -1;

unsigned long OlxMod_OlxTimeDiffMs; // In milliseconds
unsigned long OlxMod_CurrentTimeMs; // In milliseconds
unsigned long OlxMod_BackupTime;	// In OLXMOD_TICK_TIME chunks
unsigned long ClearEventsLastTime;

// Sorted by time and player - time in OLXMOD_TICK_TIME chunks
typedef std::map< unsigned long, std::map< int, OlxMod_Event_t > > OlxMod_EventList_t;
OlxMod_EventList_t OlxMod_Events;
std::vector< OlxMod_Event_t > OlxMod_OldKeys;
std::vector< unsigned long > OlxMod_LastPacketTime; // Time in OLXMOD_TICK_TIME chunks
unsigned OlxMod_Checksum;
unsigned long OlxMod_ChecksumTime; // Time in ms
unsigned long OlxMod_InitialRandomSeed; // Used for LoadState()/SaveState()
// Not used currently
std::map< std::string, CScriptableVars::ScriptVar_t > OlxMod_Options;
std::map< std::string, OlxMod_WeaponRestriction_t > OlxMod_WeaponRestrictions;
int OlxMod_ScreenX, OlxMod_ScreenY;
SDL_Surface * OlxMod_bmpDest;
bool OlxMod_SoundDisabled = true;

CInput *LoadGameStateKey = NULL;
CInput *SaveGameStateKey = NULL;

bool OlxMod_ActivateMod( const std::string & mod, OlxMod_GameSpeed_t speed,
	unsigned long localTime, int numPlayers,
	int localPlayer, unsigned long randomSeed,
	std::map< std::string, CScriptableVars::ScriptVar_t > options,
	std::map< std::string, OlxMod_WeaponRestriction_t > weaponRestrictions,
	int ScreenX, int ScreenY, SDL_Surface * bmpDest )
{
	if( OlxMod_list == NULL )
		return false;
	for( unsigned f=0; f<OlxMod_list->size(); f++ )
		if( (*OlxMod_list)[f].name == mod )
		{
			OlxMod_ActiveMod = f;
			OlxMod_TickTime = speed;
			OlxMod_OlxTimeDiffMs = localTime;
			OlxMod_NumPlayers = numPlayers;
			OlxMod_LocalPlayer = localPlayer;
			OlxMod_Options = options;
			OlxMod_WeaponRestrictions = weaponRestrictions;
			OlxMod_ScreenX = ScreenX;
			OlxMod_ScreenY = ScreenY;
			OlxMod_bmpDest = bmpDest;
			OlxMod_InitialRandomSeed = randomSeed;

			OlxMod_CurrentTimeMs = 0;
			OlxMod_BackupTime = 0;
			ClearEventsLastTime = 0;
			OlxMod_Checksum = 0;
			OlxMod_ChecksumTime = 0;
			OlxMod_Events.clear();
			OlxMod_OldKeys.clear();
			OlxMod_OldKeys.resize(OLXMOD_MAX_PLAYERS);
			OlxMod_LastPacketTime.clear();
			OlxMod_LastPacketTime.resize( OLXMOD_MAX_PLAYERS, 0 );
			OlxMod_QuickDirtyCalculation = true;
			OlxMod_ReCalculationNeeded = false;
			OlxMod_ReCalculationTimeMs = 0;
			OlxMod_NetSyncedRandom_Seed( randomSeed );
			OlxMod_NetSyncedRandom_Save();
			OlxMod_Random_Seed( ~ randomSeed );
			(*OlxMod_list)[OlxMod_ActiveMod].init( numPlayers, localPlayer, options, weaponRestrictions, ScreenX, ScreenY, bmpDest );
			(*OlxMod_list)[OlxMod_ActiveMod].save();

			#ifdef DEBUG
			LoadGameStateKey = new CInput();
			SaveGameStateKey = new CInput();
			SaveGameStateKey->Setup("F9");
			LoadGameStateKey->Setup("F10");
			#endif

			return true;
		};
	return false;
};

void OlxMod_EndRound()
{
	if( OlxMod_list == NULL )
		return;
	if( OlxMod_ActiveMod == -1 )
		return;
	(*OlxMod_list)[OlxMod_ActiveMod].deinit();
	OlxMod_ActiveMod = -1;
	OlxMod_StopSoundSystem();
	#ifdef DEBUG
	delete LoadGameStateKey;
	delete SaveGameStateKey;
	LoadGameStateKey = NULL;
	SaveGameStateKey = NULL;
	#endif
};

void OlxMod_ReCalculate()
{
	if( OlxMod_CurrentTimeMs - OlxMod_ReCalculationTimeMs < OlxMod_ReCalculationMinimumTimeMs || ! OlxMod_ReCalculationNeeded )
		return; // Limit recalculation - it is CPU-intensive
	//if( ! OlxMod_ReCalculationNeeded )
	//	return;

	OlxMod_ReCalculationTimeMs = OlxMod_CurrentTimeMs;
	OlxMod_ReCalculationNeeded = false;

	// Re-calculate physics if the packet received is from the most laggy client
	unsigned long timeMin = OlxMod_LastPacketTime[OlxMod_LocalPlayer];
	for( int f=1; f<OlxMod_NumPlayers; f++ )
		if( OlxMod_LastPacketTime[f] < timeMin )
			timeMin = OlxMod_LastPacketTime[f];

	//printf("OlxMod_ReCalculate(): OlxMod_BackupTime %lu timeMin %lu\n", OlxMod_BackupTime, timeMin);
	if( OlxMod_BackupTime /* + OlxMod_DrawDelayMs / OLXMOD_TICK_TIME_DIV */ + 1 >= timeMin )
		return;

	OlxMod_QuickDirtyCalculation = false;
	if( OlxMod_CurrentTimeMs != OlxMod_BackupTime * OLXMOD_TICK_TIME_DIV )	// Last recalc time
	{
		OlxMod_NetSyncedRandom_Restore();
		(*OlxMod_list)[OlxMod_ActiveMod].restore();
	};

	std::map< int, OlxMod_Event_t > emptyEvent;
	while( OlxMod_BackupTime /* + OlxMod_DrawDelayMs / OLXMOD_TICK_TIME_DIV */ + 1 < timeMin )
	{
		OlxMod_BackupTime++;
		OlxMod_CurrentTimeMs = OlxMod_BackupTime * OLXMOD_TICK_TIME_DIV;
		bool calculateChecksum = OlxMod_CurrentTimeMs % OlxMod_CalculateChecksumTime == 0;
		unsigned checksum = (*OlxMod_list)[OlxMod_ActiveMod].calc(
			OlxMod_CurrentTimeMs * OLXMOD_TICK_TIME_DIV / OlxMod_TickTime,
			OlxMod_Events.find(OlxMod_BackupTime) != OlxMod_Events.end() ? OlxMod_Events[OlxMod_BackupTime] : emptyEvent,
			false, calculateChecksum );	// Do the true thorough calculations
		if( calculateChecksum )
		{
			OlxMod_Checksum = checksum;
			OlxMod_ChecksumTime = OlxMod_CurrentTimeMs;
			//printf("OlxMod time %lu checksum 0x%X\n", OlxMod_ChecksumTime, OlxMod_Checksum );
		};
	};

	OlxMod_NetSyncedRandom_Save();
	(*OlxMod_list)[OlxMod_ActiveMod].save();
	OlxMod_CurrentTimeMs = OlxMod_BackupTime * OLXMOD_TICK_TIME_DIV;

	// Clean up old events - do not clean them if we're the server, clients may ask for them.
	if( OlxMod_BackupTime - ClearEventsLastTime > 100 && OlxMod_LocalPlayer != 0 )
	{
		ClearEventsLastTime = OlxMod_BackupTime;
		OlxMod_Events.erase(OlxMod_Events.begin(), OlxMod_Events.lower_bound( OlxMod_BackupTime - 2 ));
	};
	OlxMod_QuickDirtyCalculation = true;
};

// Should be called immediately after OlxMod_SendNetPacket() with the same time value
void OlxMod_Draw( unsigned long localTime, bool showScoreboard )
{
	localTime -= OlxMod_OlxTimeDiffMs;

	OlxMod_ReCalculate();

	//printf("OlxMod_Draw() time %lu oldtime %lu\n", localTime / OLXMOD_TICK_TIME , OlxMod_CurrentTimeMs / OLXMOD_TICK_TIME );

	std::map< int, OlxMod_Event_t > emptyEvent;

	while( OlxMod_CurrentTimeMs < localTime /*- OlxMod_DrawDelayMs*/ )
	{
		OlxMod_CurrentTimeMs += OLXMOD_TICK_TIME_DIV;
		(*OlxMod_list)[OlxMod_ActiveMod].calc( OlxMod_CurrentTimeMs * OLXMOD_TICK_TIME_DIV / OlxMod_TickTime ,
			OlxMod_Events.find(OlxMod_CurrentTimeMs / OLXMOD_TICK_TIME_DIV) != OlxMod_Events.end() ?
			OlxMod_Events[OlxMod_CurrentTimeMs / OLXMOD_TICK_TIME_DIV] : emptyEvent,
			true, false );	// Do fast inexact calculations
	};

	(*OlxMod_list)[OlxMod_ActiveMod].draw( showScoreboard );

};

int OlxMod_NetPacketSize()
{
	// Change here if you'll modify Receive()/Send()
	return 4+1;	// First 4 bytes is time in 10-ms chunks, second 2 bytes - keypress mask
};

void OlxMod_WriteKeys( const OlxMod_KeyState_t & keys, CBytestream * bs )
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

void OlxMod_ReadKeys( OlxMod_KeyState_t * keys, CBytestream * bs )
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

void OlxMod_CompareKeysChanged( const OlxMod_KeyState_t & keys1, const OlxMod_KeyState_t & keys2, OlxMod_KeyState_t * keysChanged )
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

void OlxMod_AddEmptyPacket( unsigned long localTime, CBytestream * bs )
{
	localTime -= OlxMod_OlxTimeDiffMs;
	localTime /= OLXMOD_TICK_TIME_DIV;
	bs->writeInt( localTime, 4 );
	OlxMod_KeyState_t keys;	// All keys are false by default
	OlxMod_WriteKeys( keys, bs );
};

unsigned OlxMod_EmptyPacketTime()
{
	return OlxMod_PingTimeMs;
};

// Returns true if data was re-calculated.
bool OlxMod_ReceiveNetPacket( CBytestream * bs, int player )
{
	unsigned long timeDiff = bs->readInt( 4 );	// TODO: 1-2 bytes are enough, I just screwed up with calculations
	/*
	// TODO: better time calculations?
	unsigned long fullTime = ( OlxMod_CurrentTimeMs / OLXMOD_TICK_TIME / 0x10000 ) * 0x10000 + timeDiff;
	if( fullTime > OlxMod_CurrentTimeMs / OLXMOD_TICK_TIME + 0x10000/2 )
		fullTime -= 0x10000;
	*/
	unsigned long fullTime = timeDiff;

	OlxMod_KeyState_t keys;
	OlxMod_ReadKeys( &keys, bs );

	OlxMod_KeyState_t changedKeys = OlxMod_OldKeys[ player ].keys;
	OlxMod_CompareKeysChanged( keys, changedKeys, &changedKeys );

	OlxMod_OldKeys[ player ] = keys;

	OlxMod_Events[ fullTime ] [ player ] = OlxMod_Event_t( keys, changedKeys );
	OlxMod_LastPacketTime[ player ] = fullTime;

	OlxMod_ReCalculationNeeded = true;
	OlxMod_ReCalculate();

	// debug - remove it
	/*
	// If sums here are different that means the CChannel missed some packets
	unsigned checksum = 0;
	unsigned checksumTime = 1;
	//printf("====================== OlxMod_Events\n");
	for( OlxMod_EventList_t::iterator it = OlxMod_Events.begin(); it != OlxMod_Events.lower_bound(OlxMod_BackupTime); it++ )
	{
		//printf("Time %lu", it->first );
		checksum += it->first * 10000;
		for( std::map< int, OlxMod_Event_t > :: iterator it1 = it->second.begin(); it1 != it->second.end(); it1++ )
			checksum += (it1->first*0x120+1) * (it1->second.keys.up*1 + it1->second.keys.down*2 +
				it1->second.keys.left*4 + it1->second.keys.right*8 + it1->second.keys.shoot*16 +
				it1->second.keys.selweap*32 + it1->second.keys.jump*64 + it1->second.keys.rope*128 );
			//printf(" player %i keys %i%i%i%i%i%i%i%i", it1->first, it1->second.keys.up, it1->second.keys.down,
			//	it1->second.keys.left, it1->second.keys.right, it1->second.keys.shoot, it1->second.keys.selweap, it1->second.keys.jump, it1->second.keys.rope );
		if( it->first / 500 > checksumTime )
		{
			printf("OlxMod_Events Keys checksum for time %lu = 0x%X\n", it->first, checksum );
			checksumTime = 1 + it->first / 500;
		};
	};
	*/

	return true;
};

// Should be called for every gameloop frame with current key state, returns true if there's something to send
// Draw() should be called after this func
bool OlxMod_SendNetPacket( unsigned long localTime, OlxMod_KeyState_t keys, CBytestream * bs )
{
	//printf("OlxMod_SendNetPacket() time %lu\n", localTime);
	localTime -= OlxMod_OlxTimeDiffMs;
	localTime /= OLXMOD_TICK_TIME_DIV;

	// Ignore too frequent keypresses, or we'll get net desync
	// The keypress will be processed on the next frame if user holds the key long enough (e.x. for 20 ms)
	if( localTime == OlxMod_CurrentTimeMs / OLXMOD_TICK_TIME_DIV )
		return false;

	if( keys == OlxMod_OldKeys[ OlxMod_LocalPlayer ].keys &&
		localTime - OlxMod_LastPacketTime[ OlxMod_LocalPlayer ] < OlxMod_PingTimeMs / OLXMOD_TICK_TIME_DIV ) // Do not flood the net
		return false;

	OlxMod_KeyState_t changedKeys = OlxMod_OldKeys[ OlxMod_LocalPlayer ].keys;
	OlxMod_CompareKeysChanged( keys, changedKeys, &changedKeys );

	OlxMod_Events[ localTime ] [ OlxMod_LocalPlayer ] = OlxMod_Event_t( keys, changedKeys );
	//printf("OlxMod_SendNetPacket() put keys in time %lu\n", localTime);
	bs->writeInt( localTime, 4 );	// TODO: 1-2 bytes are enough, I just screwed up with calculations
	OlxMod_WriteKeys( keys, bs );

	OlxMod_OldKeys[ OlxMod_LocalPlayer ] = keys;

	OlxMod_LastPacketTime[ OlxMod_LocalPlayer ] = localTime;

	if( OlxMod_NumPlayers == 1 )
		OlxMod_ReCalculationNeeded = true;

	#ifdef DEBUG
	if( SaveGameStateKey->isDownOnce() )
	{
		std::string data;
		OlxMod_SaveGameState( &data, OlxMod_BackupTime );
		FILE * ff = OpenGameFile("SavedGameState.sav", "wb");
		fwrite( data.c_str(), 1, data.size(), ff );
		fclose(ff);
	}
	else
	if( LoadGameStateKey->isDownOnce() )
	{
		std::string data;
		FILE * ff = OpenGameFile("SavedGameState.sav", "rb");
		char t[1024];
		while( ! feof(ff) )
		{
			int readed = fread( t, 1, 1024, ff );
			data.append( t, readed );
		};
		fclose(ff);
		OlxMod_LoadGameState( data );
	};
	#endif

	return true;
};

bool OlxMod_Frame( unsigned long localTime, OlxMod_KeyState_t keys, CBytestream * bs, bool showScoreboard )
{
	bool ret = OlxMod_SendNetPacket( localTime, keys, bs );
	OlxMod_Draw( localTime, showScoreboard );
	return ret;
};

unsigned OlxMod_GetChecksum( unsigned long * time )
{
	if( time )
		*time = OlxMod_ChecksumTime;
	return OlxMod_Checksum;
};

void OlxMod_SaveGameState( std::string * state, unsigned long time )
{
	if(state == NULL)
		return;
	CBytestream bs;
	bs.writeInt( time, 4 );
	for( OlxMod_EventList_t::const_iterator it = OlxMod_Events.begin(),
		it1 = OlxMod_Events.upper_bound( time ); it != it1; ++it )
	{
		if( it->second.size() == 0 )
			continue;
		bs.writeInt( it->first, 4 );
		bs.writeByte( it->second.size() );
		for( std::map< int, OlxMod_Event_t >::const_iterator it2 = it->second.begin();
				it2 != it->second.end(); ++it2 )
		{
			bs.writeByte( it2->first );
			OlxMod_WriteKeys( it2->second.keys, &bs );
		};
	};
	bs.ResetPosToBegin();
	*state = bs.readData();
	printf("OlxMod_SaveGameState() size %i\n", (int)state->size());
};

void OlxMod_LoadGameState( const std::string & state )
{
	printf("OlxMod_LoadGameState() size %i\n", (int)state.size());

	CBytestream bs;
	bs.writeData(state);
	bs.ResetPosToBegin();
	int OlxMod_BackupTime_new = bs.readInt(4);

	int ActiveMod_old = OlxMod_ActiveMod;
	OlxMod_Events.erase(OlxMod_Events.begin(), OlxMod_Events.upper_bound( OlxMod_BackupTime_new ));
	OlxMod_EventList_t events = OlxMod_Events;
	OlxMod_EndRound();
	//std::vector< OlxMod_Event_t > OlxMod_OldKeys_old = OlxMod_OldKeys;
	OlxMod_OldKeys.clear();
	OlxMod_OldKeys.resize( OLXMOD_MAX_PLAYERS );

	while( !bs.isPosAtEnd() )
	{
		std::map< int, OlxMod_Event_t > events1;
		unsigned long time1 = bs.readInt(4);
		unsigned count = bs.readByte();
		for( unsigned f=0; f<count; f++ )
		{
			int player = bs.readByte();
			OlxMod_KeyState_t keys;
			OlxMod_ReadKeys( &keys, &bs );
			OlxMod_KeyState_t changedKeys = OlxMod_OldKeys[ player ].keys;
			OlxMod_CompareKeysChanged( keys, changedKeys, &changedKeys );
			OlxMod_OldKeys[ player ] = keys;
			events1.insert( std::make_pair( player, OlxMod_Event_t( keys, changedKeys ) ) );
		};
		events.insert( std::make_pair( time1, events1 ) );
	};
	std::vector< OlxMod_Event_t > OlxMod_OldKeys_new = OlxMod_OldKeys;
	OlxMod_ActivateMod( (*OlxMod_list)[ActiveMod_old].name, (OlxMod_GameSpeed_t)OlxMod_TickTime,
						OlxMod_OlxTimeDiffMs, OlxMod_NumPlayers,
						OlxMod_LocalPlayer, OlxMod_InitialRandomSeed,
						OlxMod_Options, OlxMod_WeaponRestrictions,
						OlxMod_ScreenX, OlxMod_ScreenY, OlxMod_bmpDest );
	bool SoundDisabled_old = OlxMod_SoundDisabled;
	OlxMod_SoundDisabled = true;
	OlxMod_ReCalculationNeeded = true;
	OlxMod_Events = events;
	for( int f=0; f<OlxMod_NumPlayers; f++ )
		OlxMod_LastPacketTime[f] = OlxMod_BackupTime_new;
	OlxMod_OldKeys = OlxMod_OldKeys_new; //OlxMod_OldKeys = OlxMod_OldKeys_old;
	OlxMod_CurrentTimeMs = OlxMod_BackupTime_new * OLXMOD_TICK_TIME_DIV;
	OlxMod_ReCalculate();
	OlxMod_SoundDisabled = SoundDisabled_old;
};


OlxMod_taus113_state_t OlxMod_NetSyncedRandom_state, OlxMod_NetSyncedRandom_state_saved, OlxMod_Random_state;

#define LCG(n) ((69069UL * n) & 0xffffffffUL)
#define MASK 0xffffffffUL

void OlxMod___Random_Seed__(unsigned long s, OlxMod_taus113_state_t & NetSyncedRandom_state)
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
  OlxMod_NetSyncedRandom();
  OlxMod_NetSyncedRandom();
  OlxMod_NetSyncedRandom();
  OlxMod_NetSyncedRandom();
  OlxMod_NetSyncedRandom();
  OlxMod_NetSyncedRandom();
  OlxMod_NetSyncedRandom();
  OlxMod_NetSyncedRandom();
  OlxMod_NetSyncedRandom();
  OlxMod_NetSyncedRandom();

  return;
};

void OlxMod_NetSyncedRandom_Save()
{
	OlxMod_NetSyncedRandom_state_saved = OlxMod_NetSyncedRandom_state;
};
void OlxMod_NetSyncedRandom_Restore()
{
	OlxMod_NetSyncedRandom_state = OlxMod_NetSyncedRandom_state_saved;
};

void OlxMod_NetSyncedRandom_Seed(unsigned long s)
{
	OlxMod___Random_Seed__(s, OlxMod_NetSyncedRandom_state);
}

void OlxMod_Random_Seed(unsigned long s)
{
	OlxMod___Random_Seed__(s, OlxMod_Random_state);
}


#undef LCG
#undef MASK


FILE* OlxMod_OpenGameFile(const char *path, const char *mode)
{
	return OpenGameFile( path, mode );
};

// ----- Sound system -----
#ifdef DEDICATED_ONLY

void OlxMod_InitSoundSystem(int rate, unsigned sample_format, int channels) { };
int OlxMod_PlaySoundSample( void * data, unsigned len, int loops ) { return -1; };
void OlxMod_StopSoundSample( int channel ) { };
bool OlxMod_IsSoundSamplePlaying( int channel ) { return false; };
void OlxMod_StopSoundSystem() { };

#else // DEDICATED_ONLY

// TODO: make use of Sounds.h, don't use SDL mixer directly
// Add necessary functions to Sounds.h for that

int OlxMod_SampleRateConvert;

// We assume that mod pre-caches all sound files and won't toss the memory pointers around
std::map< void *, Mix_Chunk > OlxMod_SoundCache;
std::map< unsigned long, std::map< void *, int > > OlxMod_SoundsPlaying;

// Supports only 22050 and 44100 samplerates currently
void OlxMod_InitSoundSystem(int rate, unsigned sample_format, int channels)
{
	OlxMod_SoundDisabled = false;
	if( ! tLXOptions->bSoundOn )
	{
		OlxMod_SoundDisabled = true;
		return;
	};
	if( sample_format != AUDIO_S16 && sample_format != AUDIO_S16SYS )
	{
		OlxMod_SoundDisabled = true;
		return;
	};
	if( channels != 1 )
	{
		OlxMod_SoundDisabled = true;
		return;
	};
	if( rate == 44100 )
		OlxMod_SampleRateConvert = 0;
	if( rate == 22050 )
		OlxMod_SampleRateConvert = 1;
	else
	{
		OlxMod_SoundDisabled = true;
		return;
	};
};

// Returns channel where sound is played
int OlxMod_PlaySoundSample( void * data, unsigned len, int loops )
{
	//printf("OlxMod_PlaySoundSample() %X len %i play %i\n", (unsigned)data, len, !OlxMod_QuickDirtyCalculation);
	if( OlxMod_SoundDisabled )
		return -1;
	if( OlxMod_QuickDirtyCalculation )
	{
		if( OlxMod_SoundsPlaying[ OlxMod_CurrentTimeMs / OLXMOD_TICK_TIME_DIV ].count( data ) != 0 )
			return -1;
	}
	else
	{
		if( OlxMod_SoundsPlaying[ OlxMod_CurrentTimeMs / OLXMOD_TICK_TIME_DIV ].count( data ) != 0 )
			return OlxMod_SoundsPlaying[ OlxMod_CurrentTimeMs / OLXMOD_TICK_TIME_DIV ][ data ];
	}
	int channel = -1;
	if( OlxMod_SoundCache.find( data ) != OlxMod_SoundCache.end() )
	{
		channel = Mix_PlayChannel( -1, &OlxMod_SoundCache[data], loops );
	}
	else if( OlxMod_SampleRateConvert == 0 )
	{
		Mix_Chunk c;
		c.allocated = 0;
		c.alen = len;
		c.abuf = (Uint8 *) data;
		c.volume = MIX_MAX_VOLUME;
		OlxMod_SoundCache[data] = c;
		channel = Mix_PlayChannel( -1, &OlxMod_SoundCache[data], loops );
	}
	else if( OlxMod_SampleRateConvert == 1 )
	{
		Mix_Chunk c;
		c.allocated = 0;
		c.alen = len * 2;
		c.abuf = new Uint8[len * 2];
		c.volume = MIX_MAX_VOLUME;
		for( unsigned f=0; f<len/2; f++ )
		{
			((Sint16 *)c.abuf)[f*2] = ((Sint16 *)data)[f];
			((Sint16 *)c.abuf)[f*2+1] = ((Sint16 *)data)[f];
		};
		OlxMod_SoundCache[data] = c;
		channel = Mix_PlayChannel( -1, &OlxMod_SoundCache[data], loops );
	};
	OlxMod_SoundsPlaying[ OlxMod_CurrentTimeMs / OLXMOD_TICK_TIME_DIV ][ data ] = channel;
	return channel;
};

void OlxMod_StopSoundSample( int channel )
{
	if( OlxMod_SoundDisabled || OlxMod_QuickDirtyCalculation )
		return;
	Mix_HaltChannel( channel );

	// Clean up old events
	if( OlxMod_BackupTime > OlxMod_DrawDelayMs / OLXMOD_TICK_TIME_DIV )
		for( std::map< unsigned long, std::map< void *, int > > ::iterator it = OlxMod_SoundsPlaying.begin(),
			it1 = OlxMod_SoundsPlaying.lower_bound( OlxMod_BackupTime - OlxMod_DrawDelayMs / OLXMOD_TICK_TIME_DIV ), it2; it != it1; )
	{
		it2 = it;
		++it;
		OlxMod_SoundsPlaying.erase(it2);
	};
};

bool OlxMod_IsSoundSamplePlaying( int channel )
{
	if( OlxMod_SoundDisabled || OlxMod_QuickDirtyCalculation )
		return false;
	return Mix_Playing( channel ) != 0;
};

void OlxMod_StopSoundSystem()
{
	if( OlxMod_SoundDisabled )
		return;
	for( std::map< unsigned long, std::map< void *, int > > ::iterator it = OlxMod_SoundsPlaying.begin();
			it != OlxMod_SoundsPlaying.end(); it++ )
	{
		for( std::map< void *, int > ::iterator it1 = it->second.begin();
				it1 != it->second.end(); it1++ )
		{
			Mix_HaltChannel( it1->second );
		}
	};
	OlxMod_SoundsPlaying.clear();
	for( std::map< void *, Mix_Chunk > :: iterator it = OlxMod_SoundCache.begin(); it != OlxMod_SoundCache.end(); it++ )
	{
		delete [] ( Uint8 * ) ( it->second.abuf );
	};
	OlxMod_SoundCache.clear();
};

#endif // DEDICATED_ONLY

};
