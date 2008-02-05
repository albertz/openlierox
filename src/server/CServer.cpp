/////////////////////////////////////////
//
//             OpenLieroX
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
#include <iostream>

#include "LieroX.h"
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
#ifdef DEBUG
#include "MathLib.h"
#endif
#include "stun.h"

using namespace std;


GameServer	*cServer = NULL;


// Bots' clients
CClient *cBots = NULL;


///////////////////
// Clear the server
void GameServer::Clear(void)
{
	int i;

	cClients = NULL;
	cMap = NULL;
	//cProjectiles = NULL;
	cWorms = NULL;
	iState = SVS_LOBBY;
	iServerFrame=0;
	iNumPlayers = 0;
	bRandomMap = false;
	iMaxWorms = MAX_PLAYERS;
	bGameOver = false;
	iGameType = GMT_DEATHMATCH;
	fLastBonusTime = 0;
	InvalidateSocketState(tSocket);
	tGameLobby.bSet = false;
	bRegServer = false;
	bServerRegistered = false;
	fLastRegister = 0;
	nPort = LX_PORT;

	bTournament = false;

	fLastUpdateSent = -9999;

	cBanList.loadList("cfg/ban.lst");
	cShootList.Clear();

	bFirstBlood = true;

	for(i=0; i<MAX_CHALLENGES; i++) {
		SetNetAddrValid(tChallenges[i].Address, false);
		tChallenges[i].fTime = 0;
		tChallenges[i].iNum = 0;
	}
	ResetNetAddr( tSTUNAddress );

	tMasterServers.clear();
	tCurrentMasterServer = tMasterServers.begin();
}


///////////////////
// Start a server
int GameServer::StartServer(const std::string& name, int port, int maxplayers, bool regserver)
{
	// Shutdown and clear any previous server settings
	Shutdown();
	Clear();

	sName = name;
	iMaxWorms = maxplayers;
	bRegServer = regserver;
	nPort = port;



	// Open the socket
	tSocket = OpenUnreliableSocket(port);
	if(!IsSocketStateValid(tSocket)) {
		SystemError("Error: Could not open UDP socket");
		return false;
	}
	if(!ListenSocket(tSocket)) {
		SystemError( "Error: cannot start listening" );
		return false;
	}

	NetworkAddr addr;
	GetLocalNetAddr(tSocket, addr);
	NetAddrToString(addr, tLX->debug_string);
	printf("HINT: server started on %s\n", tLX->debug_string.c_str());

	// TODO: put this in a separate function
	ResetNetAddr( tSTUNAddress );
	if( tLXOptions->sSTUNServer != "" && tGameInfo.iGameType != GME_LOCAL /* && regserver */ )
	{
		try	// STUN server is not critical, so if it failed just proceed further
		{
			StunAtrString username;
			StunAtrString password;
			username.sizeValue = 0;
			password.sizeValue = 0;
			StunMessage req;
			memset(&req, 0, sizeof(StunMessage));
			stunBuildReqSimple( &req, username, false, false, 1 ); 
			char buf[STUN_MAX_MESSAGE_SIZE];
			int len = STUN_MAX_MESSAGE_SIZE;
			len = stunEncodeMessage( req, buf, len, password, false );
			ResetNetAddr( addr );
			std::string STUNServer = tLXOptions->sSTUNServer;
			int STUNPort = STUN_PORT;
			if( STUNServer.find(":") != std::string::npos )	{
				STUNPort = atoi( STUNServer.substr( STUNServer.find(":")+1 ).c_str() );
				STUNServer = STUNServer.substr( 0, STUNServer.find(":") );
			}
			
			// resolving hostname
			int count = 100;	// 2 secs
			if( !GetNetAddrFromNameAsync( STUNServer, addr ) )
				throw std::string("Error resolving hostname ") + tLXOptions->sSTUNServer;

			while( !IsNetAddrValid(addr) && --count > 0 )  {
				SDL_Delay(20);
				// TODO: handle events here to response
			}

			if( count <= 0 )
				throw std::string("Cannot resolve hostname ") + tLXOptions->sSTUNServer;
			SetNetAddrPort( addr, STUNPort );
			SetRemoteNetAddr( tSocket, addr );
			
			// sendinq request
			WriteSocket( tSocket, buf, len );
			count = 100;	// 2 secs
			while( ! isDataAvailable(tSocket) && --count > 0 )  {
				SDL_Delay(20);
				// TODO: handle events here to response
				WriteSocket( tSocket, buf, len );
			}

			if( count <= 0 )
				throw std::string("No response from server");
			len = ReadSocket( tSocket, buf, sizeof(buf) );
			StunMessage resp;
			memset(&resp, 0, sizeof(StunMessage));
			if( ! stunParseMessage( buf, len, resp, false ) )
				throw std::string("Wrong response from server");

			if( resp.hasMappedAddress )	{
				std::ostringstream os;
				for(short i = 3; i >= 0; i--) {
					os << (255 & (resp.mappedAddress.ipv4.addr >> i*8));
					if(i != 0) os << ".";
				}
				os << ":" << resp.mappedAddress.ipv4.port;
				StringToNetAddr( os.str(), tSTUNAddress );
				printf("HINT: STUN returned address: " + os.str() + "\n");
			} else {
				printf("HINT: STUN returned the same address, using default port number " + itoa(port) + "\n");
				ResetNetAddr( tSTUNAddress );
			}
		}
		catch( const std::string & s )
		{
			printf("HINT: STUN server failed: %s, using default port number %i\n", s.c_str(), port);
			ResetNetAddr( tSTUNAddress );
		}
	}

	// Initialize the clients
	cClients = new CClient[MAX_CLIENTS];
	if(cClients==NULL) {
		SetError("Error: Out of memory!\nsv::Startserver() %d",__LINE__);
		return false;
	}

	// Allocate the worms
	cWorms = new CWorm[MAX_WORMS];
	if(cWorms == NULL) {
		SetError("Error: Out of memory!\nsv::Startserver() %d",__LINE__);
		return false;
	}

	// Initialize the bonuses
	int i;
	for(i=0;i<MAX_BONUSES;i++)
		cBonuses[i].setUsed(false);

	// Shooting list
	if( !cShootList.Initialize() ) {
		SetError("Error: Out of memory!\nsv::Startserver() %d",__LINE__);
		return false;
	}

	// In the lobby
	iState = SVS_LOBBY;

	// Load the master server list
    FILE *fp = OpenGameFile("cfg/masterservers.txt","rt");
    if( !fp )
        return false;

    // Parse the lines
    while(!feof(fp)) {
		std::string buf = ReadUntil(fp);
        TrimSpaces(buf);
        if(buf.size() > 0) {
			tMasterServers.push_back(buf);
        }
    }
	tCurrentMasterServer = tMasterServers.begin();

	fclose(fp);


	// Setup the register so it happens on the first frame
	bServerRegistered = true;
    fLastRegister = -99999;
	//if(bRegServer)
		//RegisterServer();

	// Initialize the clients
	for(i=0;i<MAX_CLIENTS;i++) {
		cClients[i].Clear();
		cClients[i].getFileDownloaderInGame()->allowFileRequest(tLXOptions->bAllowFileDownload);

		// Initialize the shooting list
		if( !cClients[i].getShootList()->Initialize() ) {
			SystemError( "cannot initialize the shooting list of some client" );
			return false;
		}
	}


	bFirstBlood = true;


	return true;
}


///////////////////
// Start the game
int GameServer::StartGame()
{
	// remove from notifier; we don't want events anymore, we have a fixed FPS rate ingame
	RemoveSocketFromNotifierGroup( tSocket );

	CBytestream bs;

	iLives =		 tGameInfo.iLives;
	iGameType =		 tGameInfo.iGameMode;
	iMaxKills =		 tGameInfo.iKillLimit;
	fTimeLimit =	 tGameInfo.fTimeLimit;
	iTagLimit =		 tGameInfo.iTagLimit;
	sModName =		 tGameInfo.sModName;
	iLoadingTimes =	 tGameInfo.iLoadingTimes;
	bBonusesOn =	 tGameInfo.bBonusesOn;
	bShowBonusName = tGameInfo.bShowBonusName;

	// Check
	if (!cWorms)
		return false;


	// Reset the first blood
	bFirstBlood = true;


	// Shutdown any previous map instances
	if(cMap) {
		cMap->Shutdown();
		delete cMap;
		cMap = NULL;
	}

	// Create the map
	cMap = new CMap;
	if(cMap == NULL) {
		SetError("Error: Out of memory!\nsv::Startgame() %d",__LINE__);
		return false;
	}

	bRandomMap = false;
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

		sMapFilename = "levels/" + tGameInfo.sMapFile;
		if(!cMap->Load(sMapFilename)) {
			printf("Error: Could not load the '%s' level\n",sMapFilename.c_str());
			return false;
		}
	}

	// Load the game script
	if(!cGameScript.Load(tGameInfo.sModDir)) {
		printf("Error: Could not load the '%s' game script\n",sModName.c_str());
		return false;
	}

    // Load & update the weapon restrictions
    cWeaponRestrictions.loadList("cfg/wpnrest.dat");
    cWeaponRestrictions.updateList(&cGameScript);

	// Setup the flags
	int flags = (iGameType == GMT_CTF) + (iGameType == GMT_TEAMCTF)*2;
	CBytestream bytestr;
	bytestr.Clear();

	{for(int i=0;i<MAX_WORMS;i++) {
		if(cWorms[i].isUsed())
			continue;
		if(flags) {
			cWorms[i].setID(i);
			cWorms[i].setUsed(true);
			cWorms[i].setupLobby();
			cWorms[i].setFlag(true);
			cWorms[i].setName("Flag "+itoa(flags));
			cWorms[i].setSkin("flag.png");
			cWorms[i].setColour(255, 255, 255);
			cWorms[i].setTeam(flags-1);
			bytestr.writeByte(S2C_WORMINFO);
			bytestr.writeInt(i, 1);
			cWorms[i].writeInfo(&bytestr);
			iNumPlayers++;
			flags--;
		}
	}}
	SendGlobalPacket(&bytestr);

	// Set some info on the worms
	{for(int i=0;i<MAX_WORMS;i++) {
		if(cWorms[i].isUsed()) {
			cWorms[i].setLives(iLives);
            cWorms[i].setKills(0);
			cWorms[i].setGameScript(&cGameScript);
            cWorms[i].setWpnRest(&cWeaponRestrictions);
			cWorms[i].setLoadingTime( (float)iLoadingTimes / 100.0f );
			cWorms[i].setKillsInRow(0);
			cWorms[i].setDeathsInRow(0);
		}
	}}

	// Clear bonuses
	for(int i=0; i<MAX_BONUSES; i++)
		cBonuses[i].setUsed(false);

	// Clear the shooting list
	cShootList.Clear();



	fLastBonusTime = tLX->fCurTime;
	fWeaponSelectionTime = tLX->fCurTime;
	iWeaponSelectionTime_Warning = 0;

	// Set all the clients to 'not ready'
	{for(int i=0;i<MAX_CLIENTS;i++) {
		cClients[i].getShootList()->Clear();
		cClients[i].setGameReady(false);
		cClients[i].getFileDownloaderInGame()->allowFileRequest(false);
	}}


    // If this is the host, and we have a team game: Send all the worm info back so the worms know what
    // teams they are on
    if( tGameInfo.iGameType == GME_HOST ) {
        if( iGameType == GMT_TEAMDEATH || iGameType == GMT_VIP ) {

            CWorm *w = cWorms;
            CBytestream b;

            for(int i=0; i<MAX_WORMS; i++, w++ ) {
                if( !w->isUsed() )
                    continue;

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

	bs.Clear();
	bs.writeByte(S2C_PREPAREGAME);
	bs.writeBool(bRandomMap);
	if(!bRandomMap)
		bs.writeString(sMapFilename);

	// Game info
	bs.writeInt(iGameType,1);
	bs.writeInt16(iLives);
	bs.writeInt16(iMaxKills);
	bs.writeInt16((int)fTimeLimit);
	bs.writeInt16(iLoadingTimes);
	bs.writeBool(bBonusesOn);
	bs.writeBool(bShowBonusName);
	if(iGameType == GMT_TAG)
		bs.writeInt16(iTagLimit);
	bs.writeString(tGameInfo.sModDir);

    cWeaponRestrictions.sendList(&bs);

	SendGlobalPacket(&bs);
	// Cannot send anything after S2C_PREPAREGAME because of bug in old clients

	return true;
}


///////////////////
// Begin the match
void GameServer::BeginMatch(void)
{
	cout << "BeginMatch" << endl;
	int i;

	iState = SVS_PLAYING;
		
	// Initialize some server settings
	fServertime = 0;
	iServerFrame = 0;
    bGameOver = false;
	fGameOverTime = -9999;
	fLastRespawnWaveTime = 0;
	cShootList.Clear();

	// Send the connected clients a startgame message
	CBytestream bs;
	bs.writeInt(S2C_STARTGAME,1);

	SendGlobalPacket(&bs);

	// If this is a game of tag, find a random worm to make 'it'
	if(iGameType == GMT_TAG)
		TagRandomWorm();

	// Spawn the worms
	for(i=0;i<MAX_WORMS;i++) {
		if(cWorms[i].isUsed())
			cWorms[i].setAlive(false);
	}
	SpawnWave();	// Group teams

	iLastVictim = -1;

	for(i=0;i<MAX_WORMS;i++) 
		iFlagHolders[i] = -1;

	// Setup the flag worms
	for(i=0;i<MAX_WORMS;i++) {
		if(!cWorms[i].getFlag())
			continue;
		cClient->getRemoteWorms()[cWorms[i].getID()].setFlag(true);
	}

	// No need to kill local worms for dedicated server - they will suicide by themselves
	
	// perhaps the state is already bad
	RecheckGame();
}


////////////////
// End the game
void GameServer::GameOver(int winner)
{
	// The game is already over
	if (bGameOver)
		return;

	cout << "gameover, worm " << winner << " has won the match" << endl;
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
	}
	
	for( i=0; i<MAX_CLIENTS; i++ )
		cClients[i].getFileDownloaderInGame()->allowFileRequest(tLXOptions->bAllowFileDownload);
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
	if(bRegServer && !bServerRegistered )
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

	while(bs.Read(tSocket)) {
		// TODO: comment this; is this still needed?
		GetRemoteNetAddr(tSocket, adrFrom);
		SetRemoteNetAddr(tSocket, adrFrom);

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
				ParseConnectionlessPacket(&bs, address);
			continue;
		}
		bs.ResetPosToBegin();

		// Read packets
		CClient *cl = cClients;
		for(c=0;c<MAX_CLIENTS;c++,cl++) {

			// Player not connected
			if(cl->getStatus() == NET_DISCONNECTED)
				continue;

			// Check if the packet is from this player
			if(!AreNetAddrEqual(adrFrom, cl->getChannel()->getAddress()))
				continue;

			// Check the port
			if (GetNetAddrPort(adrFrom) != GetNetAddrPort(cl->getChannel()->getAddress()))
				continue;

			// Process the net channel
            if(cl->getChannel()->Process(&bs)) {

                // Only process the actual packet for playing clients
                if( cl->getStatus() != NET_ZOMBIE )
				    ParseClientPacket(cl, &bs);
            }
		}
	}
}


///////////////////
// Send packets
void GameServer::SendPackets(void)
{
	int c;
	CClient *cl = cClients;
	static float oldtime =0;

	if(tLX->fCurTime - oldtime < 1.0/72.0)
		return;		// So we don't flood packets out to the clients
	else
		oldtime = tLX->fCurTime;


	// If we are playing, send update to the clients
	if (iState == SVS_PLAYING)
		SendUpdate();

	// Randomly send a random packet :)
#ifdef DEBUG
	/*if (GetRandomInt(50) > 24 && iState == SVS_PLAYING)
		SendRandomPacket();*/
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

	if (IsNetAddrValid( tSTUNAddress ))  {
		NetAddrToString(tSTUNAddress, addr_name);
	} else  {
		// We don't know the external IP, just use the local one
		// Doesn't matter what IP we use because the masterserver finds it out by itself anyways
		NetworkAddr addr;
		GetLocalNetAddr(tSocket, addr);
		NetAddrToString(addr, addr_name);
	}

	// Remove port from IP
	size_t pos = addr_name.rfind(':');
	if (pos != std::string::npos)
		addr_name.erase(pos, std::string::npos);

	// HINT: we don't use the port returned by STUN because it works only for people with cone NAT
	// It's a rare case and then even people that enabled the LX port couldn't host
	
	sCurrentUrl = std::string(LX_SVRREG) + "?port=" + itoa(nPort) + "&addr=" + addr_name;

    bServerRegistered = false;

	// Start with the first server
	printf("Registering server at " + *tCurrentMasterServer + "\n");
	tCurrentMasterServer = tMasterServers.begin();
	tHttp.RequestData(*tCurrentMasterServer + sCurrentUrl);
}


///////////////////
// Process the registering of the server
void GameServer::ProcessRegister(void)
{
    static std::string szError;

	if(!bRegServer || bServerRegistered || tMasterServers.size() == 0)
		return;

	int result = tHttp.ProcessRequest();
	
	switch(result)  {
	// Normal, keep going
	case HTTP_PROC_PROCESSING:
		return;

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
		tHttp.RequestData(*tCurrentMasterServer + sCurrentUrl);
	} else {
		// All servers are processed
		bServerRegistered = true;
		tCurrentMasterServer = tMasterServers.begin();
	}

}


///////////////////
// This checks the registering of a server
void GameServer::CheckRegister(void)
{
	// If we don't want to register, just leave
	if(!bRegServer)
		return;

	// If we registered over n seconds ago, register again
	// The master server will not add duplicates, instead it will update the last ping time
	// so we will have another 5 minutes before our server is cleared
	if( tLX->fCurTime - fLastRegister > 4*60 ) {
		bServerRegistered = false;
		fLastRegister = tLX->fCurTime;
		RegisterServer();
	}
}


///////////////////
// De-register the server
bool GameServer::DeRegisterServer(void)
{
	// If we aren't registered, or we didn't try to register, just leave
	if( !bRegServer || !bServerRegistered || tMasterServers.size() == 0)
		return false;

	// Create the url
	std::string addr_name;
	NetworkAddr addr;

	GetLocalNetAddr(tSocket, addr);
	NetAddrToString(addr, addr_name);

	// Stun server
	sCurrentUrl = std::string(LX_SVRDEREG) + "?port=" + itoa(nPort) + "&addr=" + addr_name;
	if( IsNetAddrValid( tSTUNAddress ) )
	{
		NetAddrToString(tSTUNAddress, addr_name);
		sCurrentUrl = std::string(LX_SVRDEREG) + "?port=" + itoa(GetNetAddrPort( tSTUNAddress )) + "&addr=" + addr_name;
	}

	// Initialize the request
	bServerRegistered = false;

	// Start with the first server
	printf("De-registering server at " + *tCurrentMasterServer + "\n");
	tCurrentMasterServer = tMasterServers.begin();
	tHttp.RequestData(*tCurrentMasterServer + sCurrentUrl);

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
			tHttp.RequestData(*tCurrentMasterServer + sCurrentUrl);
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

	// Cycle through clients
	CClient *cl = cClients;
	for(c = 0; c < MAX_CLIENTS; c++, cl++) {
		// Client not connected or no worms
		if(cl->getStatus() == NET_DISCONNECTED || cl->getNumWorms() == 0)
			continue;

        // Check for a drop
		if( cl->getLastReceived() < dropvalue && cl->getWorm(0)->getID() != 0 && ( cl->getStatus() != NET_ZOMBIE ) ) {
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
	if( iState != SVS_GAME || tGameInfo.iGameType != GME_HOST )
		return;
	// Issue some sort of warning to clients
	if( tLXOptions->iWeaponSelectionMaxTime - ( tLX->fCurTime - fWeaponSelectionTime ) < 5.2 && 
		iWeaponSelectionTime_Warning < 2 )
	{
		iWeaponSelectionTime_Warning = 2;
		SendGlobalText("You have 5 seconds to select your weapons, hurry or you'll be kicked.", TXT_NOTICE);
	};
	if( tLXOptions->iWeaponSelectionMaxTime - ( tLX->fCurTime - fWeaponSelectionTime ) < 10.2 && 
		iWeaponSelectionTime_Warning == 0 )
	{
		iWeaponSelectionTime_Warning = 1;
		SendGlobalText("You have 10 seconds to select your weapons.", TXT_NOTICE);
	};
	//printf("GameServer::CheckWeaponSelectionTime() %f > %i\n", tLX->fCurTime - fWeaponSelectionTime, tLXOptions->iWeaponSelectionMaxTime);
	if( tLX->fCurTime - fWeaponSelectionTime > tLXOptions->iWeaponSelectionMaxTime )
	{
		// Kick retards who still mess with their weapons, we'll start on next frame
		CClient *cl = cClients;
		for(int c = 0; c < MAX_CLIENTS; c++, cl++)
		{
			if( cl->getStatus() == NET_DISCONNECTED || cl->getStatus() == NET_ZOMBIE )
				continue;
			if( cl->getGameReady() )
				continue;
			if( cl->getWorm(0) )
				if( cl->getWorm(0)->getID() == 0)  {
					cClient->setForceWeaponsReady(true); // Instead of kicking, force the host to make weapons ready
					continue;
				}
			DropClient( cl, CLL_KICK, "selected weapons too long" );
		};
	};
};

///////////////////
// Drop a client
void GameServer::DropClient(CClient *cl, int reason, const std::string& sReason)
{
    std::string cl_msg;

	// Tell everyone that the client's worms have left both through the net & text
	CBytestream bs;
	bs.writeByte(S2C_CLLEFT);
	bs.writeByte(cl->getNumWorms());

	static std::string buf;
	int i;
	for(i=0; i<cl->getNumWorms(); i++) {
		bs.writeByte(cl->getWorm(i)->getID());

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
					replacemax(buf,"your", "their", buf, 5);
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
					replacemax(buf,"your", "their", buf, 5);
					replacemax(buf,"you", "they", buf, 5);
					replacemax(networkTexts->sBannedYouReason,"<reason>",sReason, cl_msg, 1);
				}
				break;
        }

		// Send only if the text isn't <none>
		if(buf != "<none>")
			SendGlobalText(OldLxCompatibleString(buf),TXT_NETWORK);

		// Reset variables
		cl->setMuted(false);
		cl->getWorm(i)->setUsed(false);
		cl->getWorm(i)->setAlive(false);
	}


    // Go into a zombie state for a while so the reliable channel can still get the
    // reliable data to the client
    cl->setStatus(NET_ZOMBIE);
    cl->setZombieTime(tLX->fCurTime + 3);

	SendGlobalPacket(&bs);

    // Send the client directly a dropped packet
	// TODO: move this out here
    bs.Clear();
    bs.writeByte(S2C_DROPPED);
    bs.writeString(OldLxCompatibleString(cl_msg));
    cl->getChannel()->getMessageBS()->Append(&bs);


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


///////////////////
// Kick a worm out of the server
void GameServer::kickWorm(int wormID, const std::string& sReason)
{
    if( wormID < 0 || wormID >= MAX_PLAYERS )  {
		if (Con_IsUsed())
			Con_Printf(CNC_NOTIFY, "Could not find worm with ID '" + itoa(wormID) + "'");
		return;
	}

	if (!wormID)  {
		Con_Printf(CNC_NOTIFY, "You can't kick yourself!");
		return;  // Don't kick ourself
	}

    // Get the worm
    CWorm *w = cWorms + wormID;
    if( !w->isUsed() )  {
		if (Con_IsUsed())
			Con_Printf(CNC_NOTIFY, "Could not find worm with ID '" + itoa(wormID) + "'");
        return;
	}

    // Get the client
    CClient *cl = w->getClient();
    if( !cl )
        return;

	// Local worms are handled another way
	if (cClient)  {
		if (cClient->OwnsWorm(w))  {

			// Delete the worm from client and server
			cClient->RemoveWorm(w->getID());
			w->setAlive(false);
			w->setKills(0);
			w->setLives(WRM_OUT);
			w->setUsed(false);

			// Update the number of players on server/client
			iNumPlayers--;
			tGameInfo.iNumPlayers--;
			cl->RemoveWorm(w->getID());

			// Tell everyone that the client's worms have left both through the net & text
			CBytestream bs;
			bs.writeByte(S2C_CLLEFT);
			bs.writeByte(1);
			bs.writeByte(wormID);
			SendGlobalPacket(&bs);

			// Send the message
			if (sReason.size() == 0)
				SendGlobalText(OldLxCompatibleString(replacemax(networkTexts->sHasBeenKicked,
				"<player>", w->getName(), 1)),	TXT_NETWORK);
			else
				SendGlobalText(OldLxCompatibleString(replacemax(replacemax(networkTexts->sHasBeenKickedReason,
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


    // Drop the client
    DropClient(cl, CLL_KICK, sReason);
}


///////////////////
// Kick a worm out of the server (by name)
void GameServer::kickWorm(const std::string& szWormName, const std::string& sReason)
{
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
    Con_Printf(CNC_NOTIFY, "Could not find worm '" + szWormName + "'");
}


///////////////////
// Ban and kick the worm out of the server
void GameServer::banWorm(int wormID, const std::string& sReason)
{
    if( wormID < 0 || wormID >= MAX_PLAYERS )  {
		if (Con_IsUsed())
			Con_Printf(CNC_NOTIFY, "Could not find worm with ID '" + itoa(wormID) + "'");
        return;
	}

	if (!wormID)  {
		Con_Printf(CNC_NOTIFY, "You can't ban yourself!");
		return;  // Don't ban ourself
	}

    // Get the worm
    CWorm *w = cWorms + wormID;
	if (!w)
		return;

    if( !w->isUsed() )  {
		if (Con_IsUsed())
			Con_Printf(CNC_NOTIFY, "Could not find worm with ID '" + itoa(wormID) + "'");
        return;
	}

    // Get the client
    CClient *cl = w->getClient();
    if( !cl )
        return;

	// Local worms are handled another way
	// We just kick the worm, banning makes no sense
	if (cClient)  {
		if (cClient->OwnsWorm(w))  {

			// Delete the worm from client and server
			cClient->RemoveWorm(w->getID());
			w->setAlive(false);
			w->setKills(0);
			w->setLives(WRM_OUT);
			w->setUsed(false);

			// Update the number of players on server/client
			iNumPlayers--;
			tGameInfo.iNumPlayers--;
			cl->RemoveWorm(w->getID());

			// Tell everyone that the client's worms have left both through the net & text
			CBytestream bs;
			bs.writeByte(S2C_CLLEFT);
			bs.writeByte(1);
			bs.writeByte(wormID);
			SendGlobalPacket(&bs);

			// Send the message
			if (sReason.size() == 0)
				SendGlobalText(OldLxCompatibleString(replacemax(networkTexts->sHasBeenBanned,
				"<player>", w->getName(), 1)),	TXT_NETWORK);
			else
				SendGlobalText(OldLxCompatibleString(replacemax(replacemax(networkTexts->sHasBeenBannedReason,
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

	static std::string szAddress;
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
    Con_Printf(CNC_NOTIFY, "Could not find worm '" + szWormName + "'");
}

///////////////////
// Mute the worm, so no messages will be delivered from him
// Actually, mutes a client
void GameServer::muteWorm(int wormID)
{
    if( wormID < 0 || wormID >= MAX_PLAYERS )  {
		if (Con_IsUsed())
			Con_Printf(CNC_NOTIFY, "Could not find worm with ID '" + itoa(wormID) + "'");
        return;
	}

    // Get the worm
    CWorm *w = cWorms + wormID;
	if (!w)
		return;

    if( !w->isUsed() )  {
		if (Con_IsUsed())
			Con_Printf(CNC_NOTIFY,"Could not find worm with ID '" + itoa(wormID) + "'");
        return;
	}

    // Get the client
    CClient *cl = w->getClient();
    if( !cl )
        return;

	// Local worms are handled in an other way
	// We just say, the worm is muted, but do not do anything actually
	if (cClient)  {
		if (cClient->OwnsWorm(w))  {
			// Send the message
			SendGlobalText(OldLxCompatibleString(replacemax(networkTexts->sHasBeenMuted,"<player>", w->getName(), 1)),
							TXT_NETWORK);

			// End here
			return;
		}
	}

	// Mute
	cl->setMuted(true);

	// Send the text
	if (networkTexts->sHasBeenMuted!="<none>")  {
		SendGlobalText(OldLxCompatibleString(replacemax(networkTexts->sHasBeenMuted,"<player>",w->getName(),1)),
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
    Con_Printf(CNC_NOTIFY, "Could not find worm '" + szWormName + "'");
}

///////////////////
// Unmute the worm, so the messages will be delivered from him
// Actually, unmutes a client
void GameServer::unmuteWorm(int wormID)
{
    if( wormID < 0 || wormID >= MAX_PLAYERS )  {
		if (Con_IsUsed())
			Con_Printf(CNC_NOTIFY, "Could not find worm with ID '" + itoa(wormID) + "'");
        return;
	}

    // Get the worm
    CWorm *w = cWorms + wormID;
	if (!w)
		return;

    if( !w->isUsed() )  {
		if (Con_IsUsed())
			Con_Printf(CNC_NOTIFY, "Could not find worm with ID '" + itoa(wormID) + "'");
        return;
	}

    // Get the client
    CClient *cl = w->getClient();
    if( !cl )
        return;

	// Unmute
	cl->setMuted(false);

	// Send the message
	if (networkTexts->sHasBeenUnmuted!="<none>")  {
		SendGlobalText(OldLxCompatibleString(replacemax(networkTexts->sHasBeenUnmuted,"<player>",w->getName(),1)),
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
    Con_Printf(CNC_NOTIFY, "Could not find worm '" + szWormName + "'");
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
            c->AddText(msg, tLX->clNetworkText, tLX->fCurTime);
    }

}

//////////////////
// Get the client owning this worm
CClient *GameServer::getClient(int iWormID)
{
	if (iWormID < 0 || iWormID > MAX_WORMS)
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
// Get the download rate in bytes/s
float GameServer::GetDownload()
{
	float result = 0;
	CClient *cl = cClients;

	// Sum downloads from all clients
	for (int i=0; i < MAX_CLIENTS; i++, cl++)  {
		if (cl->getStatus() != NET_DISCONNECTED)
			result += cl->getChannel()->getIncomingRate();
	}

	return result;
}

///////////////////
// Get the upload rate in bytes/s
float GameServer::GetUpload()
{
	float result = 0;
	CClient *cl = cClients;

	// Sum downloads from all clients
	for (int i=0; i < MAX_CLIENTS; i++, cl++)  {
		if (cl->getStatus() != NET_DISCONNECTED)
			result += cl->getChannel()->getOutgoingRate();
	}

	return result;
}

///////////////////
// Shutdown the server
void GameServer::Shutdown(void)
{
	uint i;

	if(IsSocketStateValid(tSocket))
	{
		CloseSocket(tSocket);
	};
	InvalidateSocketState(tSocket);

	if(cClients) {
		for(i=0;i<MAX_CLIENTS;i++)
			cClients[i].Shutdown();
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

	/*
	if(cProjectiles) {
		delete[] cProjectiles;
		cProjectiles = NULL;
	}*/

	cBanList.Shutdown();

	// HINT: the gamescript is shut down by the cache
}


