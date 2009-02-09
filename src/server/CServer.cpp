/////////////////////////////////////////
//
//			 OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Server class
// Created 28/6/02
// Jason Boettcher



#include <stdarg.h>
#include <vector>
#include <sstream>
#include <time.h>

#include "LieroX.h"
#include "Cache.h"
#include "CClient.h"
#include "CServer.h"
#include "console.h"
#include "CBanList.h"
#include "GfxPrimitives.h"
#include "FindFile.h"
#include "StringUtils.h"
#include "CWorm.h"
#include "Protocol.h"
#include "Error.h"
#include "MathLib.h"
#include "DedicatedControl.h"
#include "Physics.h"
#include "CServerNetEngine.h"
#include "CChannel.h"
#include "CServerConnection.h"
#include "Debug.h"
#include "CGameMode.h"

GameServer	*cServer = NULL;

// Bots' clients
CServerConnection *cBots = NULL;

// declare them only locally here as nobody really should use them explicitly
std::string OldLxCompatibleString(const std::string &Utf8String);

GameServer::GameServer() {
	Clear();
	CScriptableVars::RegisterVars("GameServer")
		( sWeaponRestFile, "WeaponRestrictionsFile" ) // Only for dedicated server
		;
}

GameServer::~GameServer()  {
	CScriptableVars::DeRegisterVars("GameServer");
}

///////////////////
// Clear the server
void GameServer::Clear(void)
{
	cClients = NULL;
	cMap = NULL;
	//cProjectiles = NULL;
	cWorms = NULL;
	iState = SVS_LOBBY;
	iServerFrame=0; lastClientSendData = 0;
	iNumPlayers = 0;
	bRandomMap = false;
	//iMaxWorms = MAX_PLAYERS;
	bGameOver = false;
	//iGameType = GMT_DEATHMATCH;
	fLastBonusTime = 0;
	InvalidateSocketState(tSocket);
	for(int i=0; i<MAX_CLIENTS; i++)
	{
		InvalidateSocketState(tNatTraverseSockets[i]);
		fNatTraverseSocketsLastAccessTime[i] = -9999;
	}
	bServerRegistered = false;
	fLastRegister = 0;
	nPort = LX_PORT;
	bLocalClientConnected = false;

	fLastUpdateSent = -9999;

	cBanList.loadList("cfg/ban.lst");
	cShootList.Clear();

	iSuicidesInPacket = 0;

	for(int i=0; i<MAX_CHALLENGES; i++) {
		SetNetAddrValid(tChallenges[i].Address, false);
		tChallenges[i].fTime = 0;
		tChallenges[i].iNum = 0;
	}

	tMasterServers.clear();
	tCurrentMasterServer = tMasterServers.begin();
	
	cGameMode = NULL;
}


///////////////////
// Start a server
int GameServer::StartServer()
{
	// Shutdown and clear any previous server settings
	Shutdown();
	Clear();

	if (!bDedicated && tLX->iGameType == GME_HOST)
		tLX->bHosted = true;

	// Notifications
	if (tLX->iGameType == GME_HOST)  {
		if (tLXOptions->bFirstHosting)
			notes << "Hosting for the first time" << endl;
		else if (tLXOptions->bFirstHostingThisVer)
			notes << "Hosting for the first time with this version of OpenLieroX" << endl;
	}
	if (bDedicated)
		notes << "Server max upload bandwidth is " << tLXOptions->iMaxUploadBandwidth << " bytes/s" << endl;

	// Is this the right place for this?
	sWeaponRestFile = "cfg/wpnrest.dat";
	bLocalClientConnected = false;

	// Disable SSH for non-dedicated servers as it is cheaty
	if (!bDedicated)
		tLXOptions->tGameInfo.bServerSideHealth = false;


	// Open the socket
	nPort = tLXOptions->iNetworkPort;
	tSocket = OpenUnreliableSocket(tLXOptions->iNetworkPort);
	if(!IsSocketStateValid(tSocket)) {
		hints << "Server: could not open socket on port " << tLXOptions->iNetworkPort << ", trying rebinding client socket" << endl;
		if( cClient->RebindSocket() ) {	// If client has taken that port, free it
			tSocket = OpenUnreliableSocket(tLXOptions->iNetworkPort);
		}

		if(!IsSocketStateValid(tSocket)) {
			hints << "Server: client rebinding didn't work, trying random port" << endl;
			tSocket = OpenUnreliableSocket(0);
		}
		
		if(!IsSocketStateValid(tSocket)) {
			hints << "Server: we cannot even open a random port!" << endl;
			SetError("Server Error: Could not open UDP socket");
			return false;
		}
		
		NetworkAddr a; GetLocalNetAddr(tSocket, a);
		nPort = GetNetAddrPort(a);
	}
	if(!ListenSocket(tSocket)) {
		SetError( "Server Error: cannot start listening" );
		return false;
	}

	if(tLX->iGameType == GME_HOST )
	{
		for( int f=0; f<MAX_CLIENTS; f++ )
		{
			tNatTraverseSockets[f] = OpenUnreliableSocket(0);
			if(!IsSocketStateValid(tNatTraverseSockets[f])) {
				continue;
			}
			if(!ListenSocket(tNatTraverseSockets[f])) {
				continue;
			}
		};
	};

	NetworkAddr addr;
	GetLocalNetAddr(tSocket, addr);
	NetAddrToString(addr, tLX->debug_string);
	printf("HINT: server started on %s\n", tLX->debug_string.c_str());

	// Initialize the clients
	cClients = new CServerConnection[MAX_CLIENTS];
	if(cClients==NULL) {
		SetError("Error: Out of memory!\nsv::Startserver() " + itoa(__LINE__));
		return false;
	}

	// Allocate the worms
	cWorms = new CWorm[MAX_WORMS];
	if(cWorms == NULL) {
		SetError("Error: Out of memory!\nsv::Startserver() " + itoa(__LINE__));
		return false;
	}

	// Initialize the bonuses
	int i;
	for(i=0;i<MAX_BONUSES;i++)
		cBonuses[i].setUsed(false);

	// Shooting list
	if( !cShootList.Initialize() ) {
		SetError("Error: Out of memory!\nsv::Startserver() " + itoa(__LINE__));
		return false;
	}

	// In the lobby
	iState = SVS_LOBBY;

	// Load the master server list
	FILE *fp = OpenGameFile("cfg/masterservers.txt","rt");
	if( fp )  {
		// Parse the lines
		while(!feof(fp)) {
			std::string buf = ReadUntil(fp);
			TrimSpaces(buf);
			if(buf.size() > 0) {
				tMasterServers.push_back(buf);
			}
		}

		fclose(fp);
	}

	tCurrentMasterServer = tMasterServers.begin();

	fp = OpenGameFile("cfg/udpmasterservers.txt","rt");
	if( fp )  {

		// Parse the lines
		while(!feof(fp)) {
			std::string buf = ReadUntil(fp);
			TrimSpaces(buf);
			if(buf.size() > 0) {
				tUdpMasterServers.push_back(buf);
			}
		}

		fclose(fp);
	}


	// Setup the register so it happens on the first frame
	bServerRegistered = true;
	fLastRegister = -99999;
	fLastRegisterUdp = tLX->fCurTime - 35; // 5 seconds from now - to give the local client enough time to join before registering the player count
	//if(bRegServer)
		//RegisterServer();

	// Initialize the clients
	for(i=0;i<MAX_CLIENTS;i++) {
		cClients[i].Clear();
		cClients[i].getUdpFileDownloader()->allowFileRequest(true);

		// Initialize the shooting list
		if( !cClients[i].getShootList()->Initialize() ) {
			SetError( "Server Error: cannot initialize the shooting list of some client" );
			return false;
		}
	}

	return true;
}


bool GameServer::serverChoosesWeapons() {
	// HINT:
	// At the moment, the only cases where we need the bServerChoosesWeapons are:
	// - bForceRandomWeapons
	// - bSameWeaponsAsHostWorm
	// If we make this controllable via mods later on, there are other cases where we have to enable bServerChoosesWeapons.
	return
		tLXOptions->tGameInfo.bForceRandomWeapons ||
		(tLXOptions->tGameInfo.bSameWeaponsAsHostWorm && cClient->getNumWorms() > 0); // only makes sense if we have at least one worm	
}

///////////////////
// Start the game
int GameServer::StartGame()
{
	// remove from notifier; we don't want events anymore, we have a fixed FPS rate ingame
	RemoveSocketFromNotifierGroup( tSocket );
	for( int f=0; f<MAX_CLIENTS; f++ )
		if(IsSocketStateValid(tNatTraverseSockets[f]))
			RemoveSocketFromNotifierGroup(tNatTraverseSockets[f]);

	
	// Check that gamespeed != 0
	if (-0.05f <= (float)tLXOptions->tGameInfo.features[FT_GameSpeed] && (float)tLXOptions->tGameInfo.features[FT_GameSpeed] <= 0.05f) {
		warnings << "WARNING: gamespeed was set to " << tLXOptions->tGameInfo.features[FT_GameSpeed].toString() << "; resetting it to 1" << endl;
		tLXOptions->tGameInfo.features[FT_GameSpeed] = 1;
	}
	
		
	checkVersionCompatibilities(true);


	CBytestream bs;
	float timer;
	
	printf("GameServer::StartGame() mod %s\n", tLXOptions->tGameInfo.sModName.c_str());

	// Check
	if (!cWorms) { printf("ERROR: StartGame(): Worms not initialized\n"); return false; }
	
	CWorm *w = cWorms;
	for (int p = 0; p < MAX_WORMS; p++, w++) {
		if(w->isPrepared()) {
			warnings << "WARNING: StartGame(): worm " << p << " was already prepared! ";
			if(!w->isUsed()) warnings << "AND it is not even used!";
			warnings << endl;
			w->Unprepare();
		}
	}
	
	// TODO: why delete + create new map instead of simply shutdown/clear map?
	// WARNING: This can lead to segfaults if there are already prepared AI worms with running AI thread (therefore we unprepared them above)

	// Shutdown any previous map instances
	if(cMap) {
		cMap->Shutdown();
		delete cMap;
		cMap = NULL;
	}

	// Create the map
	cMap = new CMap;
	if(cMap == NULL) {
		SetError("Error: Out of memory!\nsv::Startgame() " + itoa(__LINE__));
		return false;
	}
	
	
	bRandomMap = false;
	/*
	if(stringcasecmp(tGameInfo.sMapFile,"_random_") == 0)
		bRandomMap = true;

	if(bRandomMap) {
		cMap->New(504,350,"dirt");
		cMap->ApplyRandomLayout( &tGameInfo.sMapRandom );

		// Free the random layout
		if( tGameInfo.sMapRandom.psObjects )
			delete[] tGameInfo.sMapRandom.psObjects;
		tGameInfo.sMapRandom.psObjects = NULL;
		tGameInfo.sMapRandom.bUsed = false;

	} else {
	*/
		timer = SDL_GetTicks()/1000.0f;
		std::string sMapFilename = "levels/" + tLXOptions->tGameInfo.sMapFile;
		if(!cMap->Load(sMapFilename)) {
			printf("Error: Could not load the '%s' level\n",sMapFilename.c_str());
			return false;
		}
		printf("Map loadtime: %f seconds\n",(float)(SDL_GetTicks()/1000.0f) - timer);

	// Load the game script
	timer = SDL_GetTicks()/1000.0f;

	cGameScript = cCache.GetMod( tLXOptions->tGameInfo.sModDir );
	if( cGameScript.get() == NULL )
	{
		cGameScript = new CGameScript();
		int result = cGameScript.get()->Load( tLXOptions->tGameInfo.sModDir );
		cCache.SaveMod( tLXOptions->tGameInfo.sModDir, cGameScript );

		if(result != GSE_OK) {
		printf("Error: Could not load the '%s' game script\n", tLXOptions->tGameInfo.sModDir.c_str());
		return false;
		};
	}
	printf("Mod loadtime: %f seconds\n",(float)(SDL_GetTicks()/1000.0f) - timer);

	// Load & update the weapon restrictions
	cWeaponRestrictions.loadList(sWeaponRestFile);
	cWeaponRestrictions.updateList(cGameScript.get());

	// Set some info on the worms
	for(int i=0;i<MAX_WORMS;i++) {
		if(cWorms[i].isUsed()) {
			cWorms[i].setLives(tLXOptions->tGameInfo.iLives);
			cWorms[i].setKills(0);
			cWorms[i].setDamage(0);
			cWorms[i].setGameScript(cGameScript.get());
			cWorms[i].setWpnRest(&cWeaponRestrictions);
			cWorms[i].setLoadingTime( (float)tLXOptions->tGameInfo.iLoadingTime / 100.0f );
			cWorms[i].setWeaponsReady(false);
		}
	}

	// Clear bonuses
	for(int i=0; i<MAX_BONUSES; i++)
		cBonuses[i].setUsed(false);

	// Clear the shooting list
	cShootList.Clear();

	fLastBonusTime = tLX->fCurTime;
	fWeaponSelectionTime = tLX->fCurTime;
	iWeaponSelectionTime_Warning = 0;

	// Set all the clients to 'not ready'
	for(int i=0;i<MAX_CLIENTS;i++) {
		cClients[i].getShootList()->Clear();
		cClients[i].setGameReady(false);
		cClients[i].getUdpFileDownloader()->allowFileRequest(false);
	}

	//TODO: Move into CTeamDeathMatch | CGameMode
	// If this is the host, and we have a team game: Send all the worm info back so the worms know what
	// teams they are on
	if( tLX->iGameType == GME_HOST ) {
		if( cGameMode->GameType() == GMT_TEAMS ) {

			CWorm *w = cWorms;
			CBytestream b;

			for(int i=0; i<MAX_WORMS; i++, w++ ) {
				if( !w->isUsed() )
					continue;

				// TODO: move that out here
				// Write out the info
				b.writeByte(S2C_WORMINFO);
				b.writeInt(w->getID(),1);
				w->writeInfo(&b);
			}

			SendGlobalPacket(&b);
		}
	}

	iState = SVS_GAME;		// In-game, waiting for players to load
	iServerFrame = 0;
	bGameOver = false;

	for( int i = 0; i < MAX_CLIENTS; i++ )
	{
		if( cClients[i].getStatus() != NET_CONNECTED )
			continue;
		cClients[i].getNetEngine()->SendPrepareGame();

		// Force random weapons for spectating clients
		if( cClients[i].getNumWorms() > 0 && cClients[i].getWorm(0)->isSpectating() )
		{
			for( int i = 0; i < cClients[i].getNumWorms(); i++ )
			{
				cClients[i].getWorm(i)->GetRandomWeapons();
				cClients[i].getWorm(i)->setWeaponsReady(true);
			}
			SendWeapons();	// TODO: we're sending multiple weapons packets, but clients handle them okay
		}
	}
	
	PhysicsEngine::Get()->initGame( cMap, cClient );

	if( DedicatedControl::Get() )
		DedicatedControl::Get()->WeaponSelections_Signal();

	// Re-register the server to reflect the state change
	if( tLXOptions->bRegServer && tLX->iGameType == GME_HOST )
		RegisterServerUdp();


	// initial server side weapon handling
	if(tLXOptions->tGameInfo.bSameWeaponsAsHostWorm && cClient->getNumWorms() > 0) {
		// we do the clone right after we selected the weapons for this worm
		// we cannot do anything here at this time
		// bForceRandomWeapons is handled from the client code
	}
	else if(tLXOptions->tGameInfo.bForceRandomWeapons) {
		for(int i=0;i<MAX_WORMS;i++) {
			if(!cWorms[i].isUsed())
				continue;
			cWorms[i].GetRandomWeapons();
			cWorms[i].setWeaponsReady(true);
		}
		
		// the other players will get the preparegame first and have therefore already called initWeaponSelection, therefore it is save to send this here
		SendWeapons();
	}
	
	// Prepare the gamemode
	cGameMode->PrepareGame();
	for(int i = 0; i < MAX_WORMS; i++) {
		if(!cWorms[i].isUsed())
			continue;
		cGameMode->PrepareWorm(&cWorms[i]);
	}
	
	return true;
}


///////////////////
// Begin the match
void GameServer::BeginMatch(CServerConnection* receiver)
{
	hints << "Server: BeginMatch";
	if(receiver) hints << " for " << receiver->debugName();
	hints << endl;

	bool firstStart = false;
	
	if( iState != SVS_PLAYING) {
		// game has started for noone yet and we get the first start signal
		firstStart = true;
		iState = SVS_PLAYING;
		if( DedicatedControl::Get() )
			DedicatedControl::Get()->GameStarted_Signal();
		
		// Initialize some server settings
		fServertime = 0;
		iServerFrame = 0;
		bGameOver = false;
		fGameOverTime = -9999;
		cShootList.Clear();
	}
	

	// Send the connected clients a startgame message
	CBytestream bs;
	bs.writeInt(S2C_STARTGAME,1);
	if(receiver)
		receiver->getNetEngine()->SendPacket(&bs);
	else
		SendGlobalPacket(&bs);
	

	if(receiver) {		
		// inform new client about other ready clients
		CServerConnection *cl = cClients;
		for(int c = 0; c < MAX_CLIENTS; c++, cl++) {
			// Client not connected or no worms
			if( cl == receiver || cl->getStatus() == NET_DISCONNECTED || cl->getStatus() == NET_ZOMBIE )
				continue;
			
			if(cl->getGameReady()) {				
				// spawn all worms for the new client
				for(int i = 0; i < cl->getNumWorms(); i++) {
					if(!cl->getWorm(i)) continue;
					
					receiver->getNetEngine()->SendWormScore( cl->getWorm(i) );
					
					if(cl->getWorm(i)->getAlive()) {
						receiver->getNetEngine()->SendSpawnWorm( cl->getWorm(i), cl->getWorm(i)->getPos() );
					}
				}
			}
		}
		// Spawn receiver's worms
		cl = receiver;
		for(int i = 0; i < receiver->getNumWorms(); i++) {
			if(!cl->getWorm(i)) continue;
			for(int ii = 0; ii < MAX_CLIENTS; ii++)
				cClients[ii].getNetEngine()->SendWormScore( cl->getWorm(i) );
					
			if(cl->getWorm(i)->getAlive()) {
				SpawnWorm( cl->getWorm(i) );
				if( tLXOptions->tGameInfo.bEmptyWeaponsOnRespawn )
					SendEmptyWeaponsOnRespawn( cl->getWorm(i) );
			}
		}
	}
	
	if(firstStart) {
		for(int i=0;i<MAX_WORMS;i++) {
			if(cWorms[i].isUsed())
				cWorms[i].setAlive(false);
		}
		for(int i=0;i<MAX_WORMS;i++) {
			if( cWorms[i].isUsed() && cWorms[i].getLives() != WRM_OUT )
				SpawnWorm( & cWorms[i] );
				if( tLXOptions->tGameInfo.bEmptyWeaponsOnRespawn )
					SendEmptyWeaponsOnRespawn( & cWorms[i] );
		}
	}

	if(firstStart)
		iLastVictim = -1;
	
	// For spectators: set their lives to out and tell clients about it
	for (int i = 0; i < MAX_WORMS; i++)  {
		if (cWorms[i].isUsed() && cWorms[i].isSpectating() && cWorms[i].getLives() != WRM_OUT)  {
			cWorms[i].setLives(WRM_OUT);
			cWorms[i].setKills(0);
			cWorms[i].setDamage(0);
			if(receiver)
				receiver->getNetEngine()->SendWormScore( & cWorms[i] );
			else
				for(int ii = 0; ii < MAX_CLIENTS; ii++)
					cClients[ii].getNetEngine()->SendWormScore( & cWorms[i] );
		}
	}
	
	// perhaps the state is already bad
	RecheckGame();

	if(firstStart) {
		// Re-register the server to reflect the state change in the serverlist
		if( tLXOptions->bRegServer && tLX->iGameType == GME_HOST )
			RegisterServerUdp();
	}
}


////////////////
// End the game
void GameServer::GameOver(int winner)
{
	// The game is already over
	if (bGameOver)
		return;

	hints << "gameover, worm " << winner << " has won the match" << endl;
	bGameOver = true;
	fGameOverTime = tLX->fCurTime;

	// Let everyone know that the game is over
	CBytestream bs;
	bs.writeByte(S2C_GAMEOVER);
	bs.writeInt(winner, 1);
	SendGlobalPacket(&bs);

	// Reset the state of all the worms so they don't keep shooting/moving after the game is over
	// HINT: next frame will send the update to all worms
	CWorm *w = cWorms;
	int i;
	for ( i=0; i < MAX_WORMS; i++, w++ )  {
		if (!w->isUsed())
			continue;

		w->clearInput();
		
		if( cGameMode->GameType() != GMT_TEAMS )
		{
			if( w->getID() == winner )
				w->addTotalWins();
			else
				w->addTotalLosses();
		}
		else	// winner == team id
		{
			if( w->getTeam() == winner )
				w->addTotalWins();
			else
				w->addTotalLosses();
		}
	}
}

///////////////////
// Main server frame
void GameServer::Frame(void)
{
	// Playing frame
	if(iState == SVS_PLAYING) {
		fServertime += tLX->fRealDeltaTime;
		iServerFrame++;
	}

	// Process any http requests (register, deregister)
	if( tLXOptions->bRegServer && !bServerRegistered )
		ProcessRegister();


	ReadPackets();

	SimulateGame();

	CheckTimeouts();

	CheckRegister();

	SendFiles();

	SendPackets();
}


///////////////////
// Read packets
void GameServer::ReadPackets(void)
{
	CBytestream bs;
	NetworkAddr adrFrom;
	int c;

	NetworkSocket pSock = tSocket;
	for( int sockNum=-1; sockNum < MAX_CLIENTS; sockNum++ )
	{
		if(sockNum != -1 )
			break;

		if( sockNum >= 0 )
			pSock = tNatTraverseSockets[sockNum];

		if (!IsSocketStateValid(pSock))
			continue;
		
		while(bs.Read(pSock)) {
			// Set out address to addr from where last packet was sent, used for NAT traverse
			GetRemoteNetAddr(pSock, adrFrom);
			SetRemoteNetAddr(pSock, adrFrom);

			// Check for connectionless packets (four leading 0xff's)
			if(bs.readInt(4) == -1) {
				std::string address;
				NetAddrToString(adrFrom, address);
				bs.ResetPosToBegin();
				// parse all connectionless packets
				// For example lx::openbeta* was sent in a way that 2 packages were sent at once.
				// <rev1457 (incl. Beta3) versions only will parse one package at a time.
				// I fixed that now since >rev1457 that it parses multiple packages here
				// (but only for new net-commands).
				// Same thing in CClient.cpp in ReadPackets
				while(!bs.isPosAtEnd() && bs.readInt(4) == -1)
					ParseConnectionlessPacket(pSock, &bs, address);
				continue;
			}
			bs.ResetPosToBegin();

			// Read packets
			CServerConnection *cl = cClients;
			for(c=0;c<MAX_CLIENTS;c++,cl++) {

				// Reset the suicide packet count
				iSuicidesInPacket = 0;

				// Player not connected
				if(cl->getStatus() == NET_DISCONNECTED)
					continue;

				// Check if the packet is from this player
				if(!AreNetAddrEqual(adrFrom, cl->getChannel()->getAddress()))
					continue;

				// Check the port
				if (GetNetAddrPort(adrFrom) != GetNetAddrPort(cl->getChannel()->getAddress()))
					continue;

				// Parse the packet - process continuously in case we've received multiple logical packets on new CChannel
				while( cl->getChannel()->Process(&bs) )
				{
					// Only process the actual packet for playing clients
					if( cl->getStatus() != NET_ZOMBIE )
						cl->getNetEngine()->ParsePacket(&bs);
					bs.Clear();
				};
			}
		}
	}
}


///////////////////
// Send packets
void GameServer::SendPackets(void)
{
	int c;
	CServerConnection *cl = cClients;

	// If we are playing, send update to the clients
	if (iState == SVS_PLAYING)
		SendUpdate();

	// Randomly send a random packet :)
#if defined(FUZZY_ERROR_TESTING) && defined(FUZZY_ERROR_TESTING_S2C)
	if (GetRandomInt(50) > 24)
		SendRandomPacket();
#endif


	// Go through each client and send them a message
	for(c=0;c<MAX_CLIENTS;c++,cl++) {
		if(cl->getStatus() == NET_DISCONNECTED)
			continue;

		// Send out the packets if we haven't gone over the clients bandwidth
		cl->getChannel()->Transmit(cl->getUnreliable());

		// Clear the unreliable bytestream
		cl->getUnreliable()->Clear();
	}
}


///////////////////
// Register the server
void GameServer::RegisterServer(void)
{
	if (tMasterServers.size() == 0)
		return;

	// Create the url
	std::string addr_name;

	// We don't know the external IP, just use the local one
	// Doesn't matter what IP we use because the masterserver finds it out by itself anyways
	NetworkAddr addr;
	GetLocalNetAddr(tSocket, addr);
	NetAddrToString(addr, addr_name);

	// Remove port from IP
	size_t pos = addr_name.rfind(':');
	if (pos != std::string::npos)
		addr_name.erase(pos, std::string::npos);

	sCurrentUrl = std::string(LX_SVRREG) + "?port=" + itoa(nPort) + "&addr=" + addr_name;

	bServerRegistered = false;

	// Start with the first server
	printf("Registering server at " + *tCurrentMasterServer + "\n");
	tCurrentMasterServer = tMasterServers.begin();
	tHttp.RequestData(*tCurrentMasterServer + sCurrentUrl, tLXOptions->sHttpProxy);
}


///////////////////
// Process the registering of the server
void GameServer::ProcessRegister(void)
{
	if(!tLXOptions->bRegServer || bServerRegistered || tMasterServers.size() == 0 || tLX->iGameType != GME_HOST)
		return;

	int result = tHttp.ProcessRequest();

	switch(result)  {
	// Normal, keep going
	case HTTP_PROC_PROCESSING:
		return; // Processing, no more work for us
	break;

	// Failed
	case HTTP_PROC_ERROR:
		notifyLog("Could not register with master server: " + tHttp.GetError().sErrorMsg);
	break;

	// Completed ok
	case HTTP_PROC_FINISHED:
		fLastRegister = tLX->fCurTime;
	break;
	}

	// Server failed or finished, anyway, go on
	tCurrentMasterServer++;
	if (tCurrentMasterServer != tMasterServers.end())  {
		printf("Registering server at " + *tCurrentMasterServer + "\n");
		tHttp.RequestData(*tCurrentMasterServer + sCurrentUrl, tLXOptions->sHttpProxy);
	} else {
		// All servers are processed
		bServerRegistered = true;
		tCurrentMasterServer = tMasterServers.begin();
	}

}

void GameServer::RegisterServerUdp(void)
{
	// Don't register a local play
	if (tLX->iGameType == GME_LOCAL)
		return;

	for( uint f=0; f<tUdpMasterServers.size(); f++ )
	{
		printf("Registering on UDP masterserver %s\n", tUdpMasterServers[f].c_str());
		NetworkAddr addr;
		if( tUdpMasterServers[f].find(":") == std::string::npos )
			continue;
		std::string domain = tUdpMasterServers[f].substr( 0, tUdpMasterServers[f].find(":") );
		int port = atoi(tUdpMasterServers[f].substr( tUdpMasterServers[f].find(":") + 1 ));
		if( !GetFromDnsCache(domain, addr) )
		{
			GetNetAddrFromNameAsync(domain, addr);
			fLastRegisterUdp = tLX->fCurTime - 35;
			continue;
		};
		SetNetAddrPort( addr, port );
		SetRemoteNetAddr( tSocket, addr );

		CBytestream bs;

		bs.writeInt(-1,4);
		bs.writeString("lx::dummypacket");	// So NAT/firewall will understand we really want to connect there
		bs.Send(tSocket);
		bs.Send(tSocket);
		bs.Send(tSocket);

		bs.Clear();
		bs.writeInt(-1, 4);
		bs.writeString("lx::register");
		bs.writeString(OldLxCompatibleString(tLXOptions->sServerName));
		bs.writeByte(iNumPlayers);
		bs.writeByte(tLXOptions->tGameInfo.iMaxPlayers);
		bs.writeByte(iState);
		// Beta8+
		bs.writeString(GetGameVersion().asString());
		bs.writeByte(serverAllowsConnectDuringGame());
		

		bs.Send(tSocket);
		return;	// Only one UDP masterserver is supported
	};
}

void GameServer::DeRegisterServerUdp(void)
{
	for( uint f=0; f<tUdpMasterServers.size(); f++ )
	{
		NetworkAddr addr;
		if( tUdpMasterServers[f].find(":") == std::string::npos )
			continue;
		std::string domain = tUdpMasterServers[f].substr( 0, tUdpMasterServers[f].find(":") );
		int port = atoi(tUdpMasterServers[f].substr( tUdpMasterServers[f].find(":") + 1 ));
		if( !GetFromDnsCache(domain, addr) )
		{
			GetNetAddrFromNameAsync(domain, addr);
			continue;
		};
		SetNetAddrPort( addr, port );
		SetRemoteNetAddr( tSocket, addr );

		CBytestream bs;

		bs.writeInt(-1,4);
		bs.writeString("lx::dummypacket");	// So NAT/firewall will understand we really want to connect there
		bs.Send(tSocket);
		bs.Send(tSocket);
		bs.Send(tSocket);

		bs.Clear();
		bs.writeInt(-1, 4);
		bs.writeString("lx::deregister");

		bs.Send(tSocket);
		return;	// Only one UDP masterserver is supported
	};
}


///////////////////
// This checks the registering of a server
void GameServer::CheckRegister(void)
{
	// If we don't want to register, just leave
	if(!tLXOptions->bRegServer || tLX->iGameType != GME_HOST)
		return;

	// If we registered over n seconds ago, register again
	// The master server will not add duplicates, instead it will update the last ping time
	// so we will have another 5 minutes before our server is cleared
	if( tLX->fCurTime - fLastRegister > 4*60 ) {
		bServerRegistered = false;
		fLastRegister = tLX->fCurTime;
		RegisterServer();
	}
	// UDP masterserver will remove our registry in 2 minutes
	if( tLX->fCurTime - fLastRegisterUdp > 40 ) {
		fLastRegisterUdp = tLX->fCurTime;
		RegisterServerUdp();
	}
}


///////////////////
// De-register the server
bool GameServer::DeRegisterServer(void)
{
	// If we aren't registered, or we didn't try to register, just leave
	if( !tLXOptions->bRegServer || !bServerRegistered || tMasterServers.size() == 0 || tLX->iGameType != GME_HOST)
		return false;

	// Create the url
	std::string addr_name;
	NetworkAddr addr;

	GetLocalNetAddr(tSocket, addr);
	NetAddrToString(addr, addr_name);

	sCurrentUrl = std::string(LX_SVRDEREG) + "?port=" + itoa(nPort) + "&addr=" + addr_name;

	// Initialize the request
	bServerRegistered = false;

	// Start with the first server
	printf("De-registering server at " + *tCurrentMasterServer + "\n");
	tCurrentMasterServer = tMasterServers.begin();
	tHttp.RequestData(*tCurrentMasterServer + sCurrentUrl, tLXOptions->sHttpProxy);

	DeRegisterServerUdp();

	return true;
}


///////////////////
// Process the de-registering of the server
bool GameServer::ProcessDeRegister(void)
{
	if (tHttp.ProcessRequest() != HTTP_PROC_PROCESSING)  {

		// Process the next server (if any)
		tCurrentMasterServer++;
		if (tCurrentMasterServer != tMasterServers.end())  {
			printf("De-registering server at " + *tCurrentMasterServer + "\n");
			tHttp.RequestData(*tCurrentMasterServer + sCurrentUrl, tLXOptions->sHttpProxy);
			return false;
		} else {
			tCurrentMasterServer = tMasterServers.begin();
			return true;  // No more servers, we finished
		}
	}

	return false;
}


///////////////////
// Check if any clients haved timed out or are out of zombie state
void GameServer::CheckTimeouts(void)
{
	int c;

	float dropvalue = tLX->fCurTime - LX_SVTIMEOUT;

	// Check
	if (!cClients)
		return;

	// Cycle through clients
	CServerConnection *cl = cClients;
	for(c = 0; c < MAX_CLIENTS; c++, cl++) {
		// Client not connected or no worms
		if(cl->getStatus() == NET_DISCONNECTED)
			continue;

		// Don't disconnect the local client
		if (cl->isLocalClient())
			continue;

		// Check for a drop
		if( cl->getLastReceived() < dropvalue && ( cl->getStatus() != NET_ZOMBIE ) ) {
			DropClient(cl, CLL_TIMEOUT);
		}

		// Is the client out of zombie state?
		if(cl->getStatus() == NET_ZOMBIE && tLX->fCurTime > cl->getZombieTime() ) {
			cl->setStatus(NET_DISCONNECTED);
		}
	}
	CheckWeaponSelectionTime();	// This is kinda timeout too
}

void GameServer::CheckWeaponSelectionTime()
{
	if( iState != SVS_GAME || tLX->iGameType != GME_HOST )
		return;

	// Issue some sort of warning to clients
	if( tLXOptions->tGameInfo.iWeaponSelectionMaxTime - ( tLX->fCurTime - fWeaponSelectionTime ) < 5.2 &&
		iWeaponSelectionTime_Warning < 2 )
	{
		iWeaponSelectionTime_Warning = 2;
		SendGlobalText("You have 5 seconds to select your weapons, hurry or you'll be kicked.", TXT_NOTICE);
	};
	if( tLXOptions->tGameInfo.iWeaponSelectionMaxTime - ( tLX->fCurTime - fWeaponSelectionTime ) < 10.2 &&
		iWeaponSelectionTime_Warning == 0 )
	{
		iWeaponSelectionTime_Warning = 1;
		SendGlobalText("You have 10 seconds to select your weapons.", TXT_NOTICE);
	};
	//printf("GameServer::CheckWeaponSelectionTime() %f > %i\n", tLX->fCurTime - fWeaponSelectionTime, tLXOptions->iWeaponSelectionMaxTime);
	if( tLX->fCurTime - fWeaponSelectionTime > tLXOptions->tGameInfo.iWeaponSelectionMaxTime )
	{
		// Kick retards who still mess with their weapons, we'll start on next frame
		CServerConnection *cl = cClients;
		for(int c = 0; c < MAX_CLIENTS; c++, cl++)
		{
			if( cl->getStatus() == NET_DISCONNECTED || cl->getStatus() == NET_ZOMBIE )
				continue;
			if( cl->getGameReady() )
				continue;
			if( cl->isLocalClient() ) {
				for(int i = 0; i < cl->getNumWorms(); i++) {
					if(!cl->getWorm(i)->getWeaponsReady()) {
						warnings << "WARNING: own worm " << cl->getWorm(i)->getName() << " is selecting weapons too long, forcing random weapons" << endl;
						cl->getWorm(i)->GetRandomWeapons();
						cl->getWorm(i)->setWeaponsReady(true);
					}
				}
				continue;
			}
			DropClient( cl, CLL_KICK, "selected weapons too long" );
		};
	};
};

///////////////////
// Drop a client
void GameServer::DropClient(CServerConnection *cl, int reason, const std::string& sReason)
{
	// Never ever drop a local client
	if (cl->isLocalClient())  {
		printf("An attempt to drop a local client was ignored\n");
		return;
	}

	// send out messages
	std::string cl_msg;
	std::string buf;
	int i;
	for(i=0; i<cl->getNumWorms(); i++) {
		switch(reason) {

			// Quit
			case CLL_QUIT:
				replacemax(networkTexts->sHasLeft,"<player>", cl->getWorm(i)->getName(), buf, 1);
				cl_msg = sReason.size() ? sReason : networkTexts->sYouQuit;
				break;

			// Timeout
			case CLL_TIMEOUT:
				replacemax(networkTexts->sHasTimedOut,"<player>", cl->getWorm(i)->getName(), buf, 1);
				cl_msg = sReason.size() ? sReason : networkTexts->sYouTimed;
				break;

			// Kicked
			case CLL_KICK:
				if (sReason.size() == 0)  { // No reason
					replacemax(networkTexts->sHasBeenKicked,"<player>", cl->getWorm(i)->getName(), buf, 1);
					cl_msg = networkTexts->sKickedYou;
				} else {
					replacemax(networkTexts->sHasBeenKickedReason,"<player>", cl->getWorm(i)->getName(), buf, 1);
					replacemax(buf,"<reason>", sReason, buf, 5);
					replacemax(buf,"your", "their", buf, 5); // TODO: dirty...
					replacemax(buf,"you", "they", buf, 5);
					replacemax(networkTexts->sKickedYouReason,"<reason>",sReason, cl_msg, 1);
				}
				break;

			// Banned
			case CLL_BAN:
				if (sReason.size() == 0)  { // No reason
					replacemax(networkTexts->sHasBeenBanned,"<player>", cl->getWorm(i)->getName(), buf, 1);
					cl_msg = networkTexts->sBannedYou;
				} else {
					replacemax(networkTexts->sHasBeenBannedReason,"<player>", cl->getWorm(i)->getName(), buf, 1);
					replacemax(buf,"<reason>", sReason, buf, 5);
					replacemax(buf,"your", "their", buf, 5); // TODO: dirty...
					replacemax(buf,"you", "they", buf, 5);
					replacemax(networkTexts->sBannedYouReason,"<reason>",sReason, cl_msg, 1);
				}
				break;
		}

		// Send only if the text isn't <none>
		if(buf != "<none>")
			SendGlobalText((buf),TXT_NETWORK);
		
		// Notify the game mode that the worm has been dropped
		cGameMode->Drop(cl->getWorm(i));
	}
	
	// remove the client and drop worms
	RemoveClient(cl);
	
	// Go into a zombie state for a while so the reliable channel can still get the
	// reliable data to the client
	cl->setStatus(NET_ZOMBIE);
	cl->setZombieTime(tLX->fCurTime + 3);

	// Send the client directly a dropped packet
	// TODO: move this out here
	CBytestream bs;
	bs.writeByte(S2C_DROPPED);
	bs.writeString(OldLxCompatibleString(cl_msg));
	cl->getChannel()->AddReliablePacketToSend(bs);

}

void GameServer::RemoveClientWorms(CServerConnection* cl) {
	std::list<byte> wormsOutList;

	int i;
	for(i=0; i<cl->getNumWorms(); i++) {
		if(!cl->getWorm(i)) {
			warnings << "WARNING: worm " << i << " of " << cl->debugName() << " is not set" << endl;
			continue;
		}
		if(!cl->getWorm(i)->isUsed()) {
			warnings << "WARNING: worm " << i << " of " << cl->debugName() << " is not used" << endl;
			cl->setWorm(i, NULL);
			continue;
		}
		
		if( DedicatedControl::Get() )
			DedicatedControl::Get()->WormLeft_Signal( cl->getWorm(i) );

		wormsOutList.push_back(cl->getWorm(i)->getID());
		
		// Reset variables
		cl->setMuted(false);
		cl->getWorm(i)->setUsed(false);
		cl->getWorm(i)->setAlive(false);
		cl->getWorm(i)->setSpectating(false);
		cl->setWorm(i, NULL);
	}
	cl->setNumWorms(0); // No worms are present anymore	

	// Tell everyone that the client's worms have left both through the net & text
	SendWormsOut(wormsOutList);

	// Re-Calculate number of players
	iNumPlayers=0;
	CWorm *w = cWorms;
	for(i=0;i<MAX_WORMS;i++,w++) {
		if(w->isUsed())
			iNumPlayers++;
	}

	// Now that a player has left, re-check the game status
	RecheckGame();
	
	// If we're waiting for players to be ready, check again
	if(iState == SVS_GAME)
		CheckReadyClient();
}

void GameServer::RemoveClient(CServerConnection* cl) {
	// Never ever drop a local client
	if (cl->isLocalClient())  {
		printf("An attempt to remove a local client was ignored\n");
		return;
	}
	
	RemoveClientWorms(cl);

	cl->setStatus(NET_DISCONNECTED);
}


bool GameServer::serverAllowsConnectDuringGame() {
	return tLXOptions->tGameInfo.bAllowConnectDuringGame;
}

void GameServer::checkVersionCompatibilities(bool dropOut) {
	// Cycle through clients
	CServerConnection *cl = cClients;
	for(int c = 0; c < MAX_CLIENTS; c++, cl++) {
		// Client not connected or no worms
		if(cl->getStatus() == NET_DISCONNECTED || cl->getStatus() == NET_ZOMBIE)
			continue;

		// HINT: It doesn't really make sense to check local clients, though we can just do it to check for strange errors.
		//if (cl->isLocalClient())
		//	continue;
		
		checkVersionCompatibility(cl, dropOut);
	}
}

bool GameServer::checkVersionCompatibility(CServerConnection* cl, bool dropOut, bool makeMsg) {
	if(serverChoosesWeapons()) {
		if(!forceMinVersion(cl, OLXBetaVersion(7), "server chooses the weapons", dropOut, makeMsg))
			return false;	
	}
	
	if(serverAllowsConnectDuringGame()) {
		if(!forceMinVersion(cl, OLXBetaVersion(8), "connecting during game is allowed", dropOut, makeMsg))
			return false;
	}
	
	foreach( Feature*, f, Array(featureArray,featureArrayLen()) ) {
		if(!tLXOptions->tGameInfo.features.olderClientsSupportSetting(f->get())) {
			if(!forceMinVersion(cl, f->get()->minVersion, f->get()->humanReadableName + " is set to " + tLXOptions->tGameInfo.features.hostGet(f->get()).toString(), dropOut, makeMsg))
				return false;
		}
	}
	
	return true;
}

bool GameServer::forceMinVersion(CServerConnection* cl, const Version& ver, const std::string& reason, bool dropOut, bool makeMsg) {
	if(cl->getClientVersion() < ver) {
		std::string kickReason = cl->getClientVersion().asString() + " is too old: " + reason;
		std::string playerName = (cl->getNumWorms() > 0) ? cl->getWorm(0)->getName() : cl->debugName();
		if(dropOut)
			DropClient(cl, CLL_KICK, kickReason);
		if(makeMsg)
			SendGlobalText((playerName + " is too old: " + reason), TXT_NOTICE);
		return false;
	}
	return true;
}

bool GameServer::clientsConnected_less(const Version& ver) {
	CServerConnection *cl = cClients;
	for(int c = 0; c < MAX_CLIENTS; c++, cl++)
		if( cl->getStatus() == NET_CONNECTED && cl->getClientVersion() < ver )
			return true;
	return false;
}



ScriptVar_t GameServer::isNonDamProjGoesThroughNeeded(const ScriptVar_t& preset) {
	if(!(bool)preset) return ScriptVar_t(false);
	if(!tLXOptions->tGameInfo.features[FT_TeamInjure] || !tLXOptions->tGameInfo.features[FT_SelfInjure])
		return preset;
	else
		return ScriptVar_t(false);
}



///////////////////
// Kick a worm out of the server
void GameServer::kickWorm(int wormID, const std::string& sReason)
{
	if (!cWorms)
		return;

	if( wormID < 0 || wormID >= MAX_PLAYERS )  {
		Con_AddText(CNC_NOTIFY, "Could not find worm with ID '" + itoa(wormID) + "'");
		return;
	}

	if (!wormID && !bDedicated)  {
		Con_AddText(CNC_NOTIFY, "You can't kick yourself!");
		return;  // Don't kick ourself
	}

	// Get the worm
	CWorm *w = cWorms + wormID;
	if( !w->isUsed() )  {
		Con_AddText(CNC_NOTIFY, "Could not find worm with ID '" + itoa(wormID) + "'");
		return;
	}

	// Local worms are handled another way
	if (cClient)  {
		if (cClient->OwnsWorm(w->getID()))  {
			// Delete the worm from client and server
			cClient->RemoveWorm(w->getID());
			w->setAlive(false);
			w->setKills(0);
			w->setLives(WRM_OUT);
			w->setUsed(false);

			// Update the number of players on server/client
			iNumPlayers--;
			if (w->getClient())
				w->getClient()->RemoveWorm(w->getID());

			// Tell everyone that the client's worms have left both through the net & text
			CBytestream bs;
			bs.writeByte(S2C_WORMSOUT);
			bs.writeByte(1);
			bs.writeByte(wormID);
			SendGlobalPacket(&bs);

			// Send the message
			if (sReason.size() == 0)
				SendGlobalText((replacemax(networkTexts->sHasBeenKicked,
				"<player>", w->getName(), 1)),	TXT_NETWORK);
			else
				SendGlobalText((replacemax(replacemax(networkTexts->sHasBeenKickedReason,
				"<player>", w->getName(), 1), "<reason>", sReason, 1)),	TXT_NETWORK);

			// Now that a player has left, re-check the game status
			RecheckGame();

			// If we're waiting for players to be ready, check again
			if(iState == SVS_GAME)
				CheckReadyClient();

			// End here
			return;
		}
	}

	// Get the client
	CServerConnection *cl = w->getClient();
	if( !cl ) {
		Con_AddText(CNC_ERROR, "This worm cannot be kicked, the client is unknown");
		return;
	}

	// Drop the whole client
	// TODO: only kick this worm, not the whole client
	DropClient(cl, CLL_KICK, sReason);
}


///////////////////
// Kick a worm out of the server (by name)
void GameServer::kickWorm(const std::string& szWormName, const std::string& sReason)
{
	if (!cWorms)
		return;

	// Find the worm name
	CWorm *w = cWorms;
	for(int i=0; i < MAX_WORMS; i++, w++) {
		if(!w->isUsed())
			continue;

		if(stringcasecmp(w->getName(), szWormName) == 0) {
			kickWorm(i, sReason);
			return;
		}
	}

	// Didn't find the worm
	Con_AddText(CNC_NOTIFY, "Could not find worm '" + szWormName + "'");
}


///////////////////
// Ban and kick the worm out of the server
void GameServer::banWorm(int wormID, const std::string& sReason)
{
	if (!cWorms)
		return;

	if( wormID < 0 || wormID >= MAX_PLAYERS )  {
		if (Con_IsVisible())
			Con_AddText(CNC_NOTIFY, "Could not find worm with ID '" + itoa(wormID) + "'");
		return;
	}

	if (!wormID && !bDedicated)  {
		Con_AddText(CNC_NOTIFY, "You can't ban yourself!");
		return;  // Don't ban ourself
	}

	// Get the worm
	CWorm *w = cWorms + wormID;
	if (!w)
		return;

	if( !w->isUsed() )  {
		if (Con_IsVisible())
			Con_AddText(CNC_NOTIFY, "Could not find worm with ID '" + itoa(wormID) + "'");
		return;
	}

	// Get the client
	CServerConnection *cl = w->getClient();
	if( !cl )
		return;

	// Local worms are handled another way
	// We just kick the worm, banning makes no sense
	if (cClient)  {
		if (cClient->OwnsWorm(w->getID()))  {

			// Delete the worm from client and server
			cClient->RemoveWorm(w->getID());
			w->setAlive(false);
			w->setKills(0);
			w->setLives(WRM_OUT);
			w->setUsed(false);

			// Update the number of players on server/client
			iNumPlayers--;
			//tGameInfo.iNumPlayers--;
			cl->RemoveWorm(w->getID());

			// Tell everyone that the client's worms have left both through the net & text
			CBytestream bs;
			bs.writeByte(S2C_WORMSOUT);
			bs.writeByte(1);
			bs.writeByte(wormID);
			SendGlobalPacket(&bs);

			// Send the message
			if (sReason.size() == 0)
				SendGlobalText((replacemax(networkTexts->sHasBeenBanned,
				"<player>", w->getName(), 1)),	TXT_NETWORK);
			else
				SendGlobalText((replacemax(replacemax(networkTexts->sHasBeenBannedReason,
				"<player>", w->getName(), 1), "<reason>", sReason, 1)),	TXT_NETWORK);

			// Now that a player has left, re-check the game status
			RecheckGame();

			// If we're waiting for players to be ready, check again
			if(iState == SVS_GAME)
				CheckReadyClient();

			// End here
			return;
		}
	}

	std::string szAddress;
	NetAddrToString(cl->getChannel()->getAddress(),szAddress);

	getBanList()->addBanned(szAddress,w->getName());

	// Drop the client
	DropClient(cl, CLL_BAN, sReason);
}


void GameServer::banWorm(const std::string& szWormName, const std::string& sReason)
{
	// Find the worm name
	CWorm *w = cWorms;
	if (!w)
		return;

	for(int i=0; i<MAX_WORMS; i++, w++) {
		if(!w->isUsed())
			continue;

		if(stringcasecmp(w->getName(), szWormName) == 0) {
			banWorm(i, sReason);
			return;
		}
	}

	// Didn't find the worm
	Con_AddText(CNC_NOTIFY, "Could not find worm '" + szWormName + "'");
}

///////////////////
// Mute the worm, so no messages will be delivered from him
// Actually, mutes a client
void GameServer::muteWorm(int wormID)
{
	if( wormID < 0 || wormID >= MAX_PLAYERS )  {
		if (Con_IsVisible())
			Con_AddText(CNC_NOTIFY, "Could not find worm with ID '" + itoa(wormID) + "'");
		return;
	}

	// Get the worm
	CWorm *w = cWorms + wormID;
	if (!cWorms)
		return;

	if( !w->isUsed() )  {
		if (Con_IsVisible())
			Con_AddText(CNC_NOTIFY,"Could not find worm with ID '" + itoa(wormID) + "'");
		return;
	}

	// Get the client
	CServerConnection *cl = w->getClient();
	if( !cl )
		return;

	// Local worms are handled in an other way
	// We just say, the worm is muted, but do not do anything actually
	if (cClient)  {
		if (cClient->OwnsWorm(w->getID()))  {
			// Send the message
			SendGlobalText((replacemax(networkTexts->sHasBeenMuted,"<player>", w->getName(), 1)),
							TXT_NETWORK);

			// End here
			return;
		}
	}

	// Mute
	cl->setMuted(true);

	// Send the text
	if (networkTexts->sHasBeenMuted!="<none>")  {
		SendGlobalText((replacemax(networkTexts->sHasBeenMuted,"<player>",w->getName(),1)),
						TXT_NETWORK);
	}
}


void GameServer::muteWorm(const std::string& szWormName)
{
	// Find the worm name
	CWorm *w = cWorms;
	if (!w)
		return;

	for(int i=0; i<MAX_WORMS; i++, w++) {
		if(!w->isUsed())
			continue;

		if(stringcasecmp(w->getName(), szWormName) == 0) {
			muteWorm(i);
			return;
		}
	}

	// Didn't find the worm
	Con_AddText(CNC_NOTIFY, "Could not find worm '" + szWormName + "'");
}

///////////////////
// Unmute the worm, so the messages will be delivered from him
// Actually, unmutes a client
void GameServer::unmuteWorm(int wormID)
{
	if( wormID < 0 || wormID >= MAX_PLAYERS )  {
		if (Con_IsVisible())
			Con_AddText(CNC_NOTIFY, "Could not find worm with ID '" + itoa(wormID) + "'");
		return;
	}

	// Get the worm
	CWorm *w = cWorms + wormID;
	if (!cWorms)
		return;

	if( !w->isUsed() )  {
		if (Con_IsVisible())
			Con_AddText(CNC_NOTIFY, "Could not find worm with ID '" + itoa(wormID) + "'");
		return;
	}

	// Get the client
	CServerConnection *cl = w->getClient();
	if( !cl )
		return;

	// Unmute
	cl->setMuted(false);

	// Send the message
	if (networkTexts->sHasBeenUnmuted!="<none>")  {
		SendGlobalText((replacemax(networkTexts->sHasBeenUnmuted,"<player>",w->getName(),1)),
						TXT_NETWORK);
	}
}


void GameServer::unmuteWorm(const std::string& szWormName)
{
	// Find the worm name
	CWorm *w = cWorms;
	if (!w)
		return;

	for(int i=0; i<MAX_WORMS; i++, w++) {
		if(!w->isUsed())
			continue;

		if(stringcasecmp(w->getName(), szWormName) == 0) {
			unmuteWorm(i);
			return;
		}
	}

	// Didn't find the worm
	Con_AddText(CNC_NOTIFY, "Could not find worm '" + szWormName + "'");
}

void GameServer::authorizeWorm(int wormID)
{
	if( wormID < 0 || wormID >= MAX_PLAYERS )  {
		if (Con_IsVisible())
			Con_AddText(CNC_NOTIFY, "Could not find worm with ID '" + itoa(wormID) + "'");
		return;
	}

	// Get the worm
	CWorm *w = cWorms + wormID;
	if (!cWorms)
		return;

	if( !w->isUsed() )  {
		if (Con_IsVisible())
			Con_AddText(CNC_NOTIFY, "Could not find worm with ID '" + itoa(wormID) + "'");
		return;
	}

	// Get the client
	CServerConnection *cl = getClient(wormID);
	if( !cl )
		return;

	cl->getRights()->Everything();
	cServer->SendGlobalText((getWorms() + wormID)->getName() + " has been authorised", TXT_NORMAL);
}


void GameServer::cloneWeaponsToAllWorms(CWorm* worm) {
	if (!cWorms)  {
		errors << "cloneWeaponsToAllWorms called when the server is not running" << endl;
		return;
	}

	CWorm *w = cWorms;
	for (int i = 0; i < MAX_WORMS; i++, w++) {
		if(w->isUsed()) {
			w->CloneWeaponsFrom(worm);
			w->setWeaponsReady(true);
		}
	}

	SendWeapons();
}




///////////////////
// Notify the host about stuff
void GameServer::notifyLog(const std::string& msg)
{
	// Local hosting?
	// Add it to the clients chatbox
	if(cClient) {
		CChatBox *c = cClient->getChatbox();
		if(c)
			c->AddText(msg, tLX->clNetworkText, TXT_NETWORK, tLX->fCurTime);
	}

}

//////////////////
// Get the client owning this worm
CServerConnection *GameServer::getClient(int iWormID)
{
	if (iWormID < 0 || iWormID > MAX_WORMS || !cWorms)
		return NULL;

	CWorm *w = cWorms;

	for(int p=0;p<MAX_WORMS;p++,w++) {
		if(w->isUsed())
			if (w->getID() == iWormID)
				return w->getClient();
	}

	return NULL;
}


///////////////////
// Get the download rate in bytes/s for all non-local clients
float GameServer::GetDownload()
{
	if(!cClients) return 0;
	float result = 0;
	CServerConnection *cl = cClients;

	// Sum downloads from all clients
	for (int i=0; i < MAX_CLIENTS; i++, cl++)  {
		if (cl->getStatus() != NET_DISCONNECTED && cl->getStatus() != NET_ZOMBIE && !cl->isLocalClient() && cl->getChannel() != NULL)
			result += cl->getChannel()->getIncomingRate();
	}

	return result;
}

///////////////////
// Get the upload rate in bytes/s for all non-local clients
float GameServer::GetUpload(float timeRange)
{
	if(!cClients) return 0;
	float result = 0;
	CServerConnection *cl = cClients;

	// Sum downloads from all clients
	for (int i=0; i < MAX_CLIENTS; i++, cl++)  {
		if (cl->getStatus() != NET_DISCONNECTED && cl->getStatus() != NET_ZOMBIE && !cl->isLocalClient() && cl->getChannel() != NULL)
			result += cl->getChannel()->getOutgoingRate(timeRange);
	}

	return result;
}

///////////////////
// Shutdown the server
void GameServer::Shutdown(void)
{
	uint i;

	// If we've hosted this session, set the FirstHost option to false
	if (tLX->bHosted)  {
		tLXOptions->bFirstHosting = false;
		tLXOptions->bFirstHostingThisVer = false;
	}

	// Kick clients if they still connected (sends just one packet which may be lost, but whatever, we're shutting down)
	if(cClients && tLX->iGameType == GME_HOST)
	{
		SendDisconnect();
	}

	if(IsSocketStateValid(tSocket))
	{
		CloseSocket(tSocket);
	};
	InvalidateSocketState(tSocket);
	for(i=0; i<MAX_CLIENTS; i++)
	{
		if(IsSocketStateValid(tNatTraverseSockets[i]))
			CloseSocket(tNatTraverseSockets[i]);
		InvalidateSocketState(tNatTraverseSockets[i]);
	};

	if(cClients) {
		delete[] cClients;
		cClients = NULL;
	}

	if(cWorms) {
		delete[] cWorms;
		cWorms = NULL;
	}

	if(cMap) {
		cMap->Shutdown();
		delete cMap;
		cMap = NULL;
	}

	cShootList.Shutdown();

	cWeaponRestrictions.Shutdown();


	cBanList.Shutdown();

	// HINT: the gamescript is shut down by the cache
}


