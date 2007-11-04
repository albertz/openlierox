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
	iRandomMap = false;
	iMaxWorms = MAX_PLAYERS;
	iGameOver = false;
	iGameType = GMT_DEATHMATCH;
	fLastBonusTime = 0;
	InvalidateSocketState(tSocket);
	tGameLobby.nSet = false;
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
		SetNetAddrValid(&tChallenges[i].Address, false);
		tChallenges[i].fTime = 0;
		tChallenges[i].iNum = 0;
	}
	ResetNetAddr( &tSTUNAddress );

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
	GetLocalNetAddr(tSocket,&addr);
	NetAddrToString(&addr, tLX->debug_string);
	printf("HINT: server started on %s\n", tLX->debug_string.c_str());

	ResetNetAddr( &tSTUNAddress );
	if( tLXOptions->sSTUNServer != "" /* && regserver */ )
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
			ResetNetAddr( &addr );
			std::string STUNServer = tLXOptions->sSTUNServer;
			int STUNPort = STUN_PORT;
			if( STUNServer.find(":") != std::string::npos )
			{
				STUNPort = atoi( STUNServer.substr( STUNServer.find(":")+1 ).c_str() );
				STUNServer = STUNServer.substr( 0, STUNServer.find(":") );
			};
			if( !GetNetAddrFromName( STUNServer, &addr ) )	
			{
				throw std::string("Cannot resolve hostname ") + tLXOptions->sSTUNServer;
			};
			SetNetAddrPort( &addr, STUNPort );
			SetRemoteNetAddr( tSocket, &addr );
			WriteSocket( tSocket, buf, len );
			int count = 20;	// 2 secs
			while( ! isDataAvailable(tSocket) && --count > 0 )
			{
				SDL_Delay(100);
				WriteSocket( tSocket, buf, len );
			};
			if( count <= 0 ) throw std::string("No responce from server");
			len = ReadSocket( tSocket, buf, sizeof(buf) );
			StunMessage resp;
			memset(&resp, 0, sizeof(StunMessage));
			if( ! stunParseMessage( buf, len, resp, false ) ) throw std::string("Wrong responce from server");
			std::ostringstream os;
			for(short i = 3; i >= 0; i--) {
				os << (255 & (resp.mappedAddress.ipv4.addr >> i*8));
				if(i != 0) os << ".";
			}
			os << ":" << resp.mappedAddress.ipv4.port;
			StringToNetAddr( os.str(), &tSTUNAddress );
			std::string s;
			NetAddrToString( &tSTUNAddress, s );
			printf("HINT: STUN returned address: %s\n", s.c_str());
		}
		catch( const std::string & s )
		{
			printf("HINT: STUN server failed: %s\n", s.c_str());
		};
	};

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
int GameServer::StartGame(void)
{
	CBytestream bs;

	iLives =		 tGameInfo.iLives;
	iGameType =		 tGameInfo.iGameMode;
	iMaxKills =		 tGameInfo.iKillLimit;
	iTimeLimit =	 tGameInfo.iTimeLimit;
	iTagLimit =		 tGameInfo.iTagLimit;
	sModName = tGameInfo.sModName;
	iLoadingTimes =	 tGameInfo.iLoadingTimes;
	iBonusesOn =	 tGameInfo.iBonusesOn;
	iShowBonusName = tGameInfo.iShowBonusName;

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

	iRandomMap = false;
	if(stringcasecmp(tGameInfo.sMapFile,"_random_") == 0)
		iRandomMap = true;

	if(iRandomMap) {
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
	cGameScript.Shutdown();
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

	for(int i=0;i<MAX_WORMS;i++) {
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
	}
	SendGlobalPacket(&bytestr);

	// Set some info on the worms
	for(int i=0;i<MAX_WORMS;i++) {
		if(cWorms[i].isUsed()) {
			cWorms[i].setLives(iLives);
            cWorms[i].setKills(0);
			cWorms[i].setGameScript(&cGameScript);
            cWorms[i].setWpnRest(&cWeaponRestrictions);
			cWorms[i].setLoadingTime( (float)iLoadingTimes / 100.0f );
			cWorms[i].setKillsInRow(0);
			cWorms[i].setDeathsInRow(0);
		}
	}

	// Clear bonuses
	for(int i=0; i<MAX_BONUSES; i++)
		cBonuses[i].setUsed(false);

	// Clear the shooting list
	cShootList.Clear();



	fLastBonusTime = tLX->fCurTime;

	// Set all the clients to 'not ready'
	for(int i=0;i<MAX_CLIENTS;i++) {
		cClients[i].getShootList()->Clear();
		cClients[i].setGameReady(false);
	}


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
    iGameOver = false;

	bs.Clear();
	bs.writeByte(S2C_PREPAREGAME);
	bs.writeInt(iRandomMap,1);
	if(!iRandomMap)
		bs.writeString(sMapFilename);

	// Game info
	bs.writeInt(iGameType,1);
	bs.writeInt16(iLives);
	bs.writeInt16(iMaxKills);
	bs.writeInt16(iTimeLimit);
	bs.writeInt16(iLoadingTimes);
	bs.writeInt(iBonusesOn, 1);
	bs.writeInt(iShowBonusName, 1);
	if(iGameType == GMT_TAG)
		bs.writeInt16(iTagLimit);
	bs.writeString(tGameInfo.sModDir);

    cWeaponRestrictions.sendList(&bs);

	SendGlobalPacket(&bs);


	return true;
}


///////////////////
// Begin the match
void GameServer::BeginMatch(void)
{
	cMap->SetModifiedFlag();
	
	int i;

	iState = SVS_PLAYING;


	// Initialize some server settings
	fServertime = 0;
	iServerFrame = 0;
    iGameOver = false;
	fGameOverTime = -9999;
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
			SpawnWorm(&cWorms[i]);
	}

	iLastVictim = -1;

	for(i=0;i<MAX_WORMS;i++) 
		iFlagHolders[i] = -1;

	// Setup the flag worms
	for(i=0;i<MAX_WORMS;i++) {
		if(!cWorms[i].getFlag())
			continue;
		(cClient->getRemoteWorms() + cWorms[i].getID())->setFlag(true);
	}
}


////////////////
// End the game
void GameServer::GameOver(int winner)
{
	// The game is already over
	if (iGameOver)
		return;

	iGameOver = true;
	fGameOverTime = tLX->fCurTime;

	// Let everyone know that the game is over
	CBytestream bs;
	bs.writeByte(S2C_GAMEOVER);
	bs.writeInt(winner, 1);
	SendGlobalPacket(&bs);

	// Reset the state of all the worms so they don't keep shooting/moving after the game is over
	// HINT: next frame will send the update to all worms
	CWorm *w = cWorms;
	for (int i=0; i < MAX_WORMS; i++, w++)  {
		if (!w->isUsed())
			continue;

		w->clearInput();
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
	if(bRegServer && !bServerRegistered )
		ProcessRegister();


	ReadPackets();

	SimulateGame();

	CheckTimeouts();

	CheckRegister();

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

		GetRemoteNetAddr(tSocket,&adrFrom);
		SetRemoteNetAddr(tSocket,&adrFrom);

		// Check for connectionless packets (four leading 0xff's)
		if(bs.readInt(4) == -1) {
			std::string address;
			NetAddrToString(&adrFrom, address);
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
			if(!AreNetAddrEqual(&adrFrom,cl->getChannel()->getAddress()))
				continue;

			// Check the port
			if (GetNetAddrPort(&adrFrom) != GetNetAddrPort(cl->getChannel()->getAddress()))
				continue;

			// Process the net channel
            if(cl->getChannel()->Process(&bs)) {

                // Only process the actual packet for playing clients
                if( cl->getStatus() != NET_ZOMBIE )
				    ParseClientPacket(cl,&bs);
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
	NetworkAddr addr;

	GetLocalNetAddr(tSocket,&addr);
	NetAddrToString(&addr, addr_name);

	sCurrentUrl = std::string(LX_SVRREG) + "?port=" + itoa(nPort) + "&addr=" + addr_name;
	if (IsNetAddrValid( &tSTUNAddress ))  {
		NetAddrToString(&tSTUNAddress, addr_name);
		sCurrentUrl = std::string(LX_SVRREG) + "?port=" + itoa(GetNetAddrPort( &tSTUNAddress )) + "&addr=" + addr_name;
	}

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
		notifyLog("Could not register with master server: %s", tHttp.GetError().sErrorMsg.c_str());
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

	GetLocalNetAddr(tSocket,&addr);
	NetAddrToString(&addr, addr_name);

	// Stun server
	sCurrentUrl = std::string(LX_SVRDEREG) + "?port=" + itoa(nPort) + "&addr=" + addr_name;
	if( IsNetAddrValid( &tSTUNAddress ) )
	{
		NetAddrToString(&tSTUNAddress, addr_name);
		sCurrentUrl = std::string(LX_SVRDEREG) + "?port=" + itoa(GetNetAddrPort( &tSTUNAddress )) + "&addr=" + addr_name;
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
		// Client not connected
		if(cl->getStatus() == NET_DISCONNECTED)
			continue;

        // Check for a drop
		if(cl->getLastReceived() < dropvalue && cl->getStatus() != NET_ZOMBIE && cl->getWorm(0)->getID() != 0) {
			DropClient(cl, CLL_TIMEOUT);
		}

        // Is the client out of zombie state?
        if(cl->getStatus() == NET_ZOMBIE && tLX->fCurTime > cl->getZombieTime() ) {
            cl->setStatus(NET_DISCONNECTED);
        }
	}
}


///////////////////
// Drop a client
void GameServer::DropClient(CClient *cl, int reason)
{
    static std::string cl_msg;
    cl_msg = "";

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
                cl_msg = networkTexts->sYouQuit;
                break;

            // Timeout
            case CLL_TIMEOUT:
				replacemax(networkTexts->sHasTimedOut,"<player>", cl->getWorm(i)->getName(), buf, 1);
                cl_msg = networkTexts->sYouTimed;
                break;

            // Kicked
            case CLL_KICK:
				replacemax(networkTexts->sHasBeenKicked,"<player>", cl->getWorm(i)->getName(), buf, 1);
                cl_msg = networkTexts->sKickedYou;
                break;

			// Banned
			case CLL_BAN:
				replacemax(networkTexts->sHasBeenBanned,"<player>", cl->getWorm(i)->getName(), buf, 1);
				cl_msg = networkTexts->sBannedYou;
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
// Drop a client
void GameServer::DropClient(CClient *cl, int reason, std::string sReason)
{
    static std::string cl_msg;
    cl_msg = "";

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
                cl_msg = sReason;
                break;

            // Timeout
            case CLL_TIMEOUT:
				replacemax(networkTexts->sHasTimedOut,"<player>", cl->getWorm(i)->getName(), buf, 1);
                cl_msg = sReason;
                break;

            // Kicked
            case CLL_KICK:
				replacemax(networkTexts->sHasBeenKickedReason,"<player>", cl->getWorm(i)->getName(), buf, 1);
				replacemax(buf,"<reason>", sReason, buf, 5);
				replacemax(buf,"your", "their", buf, 5);
				replacemax(buf,"you", "they", buf, 5);
                replacemax(networkTexts->sKickedYouReason,"<reason>",sReason, cl_msg, 1);
                break;

			// Banned
			case CLL_BAN:
				replacemax(networkTexts->sHasBeenBanned,"<player>", cl->getWorm(i)->getName(), buf, 1);
				cl_msg = sReason;
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
void GameServer::kickWorm(int wormID)
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
			SendGlobalText(OldLxCompatibleString(replacemax(networkTexts->sHasBeenKicked,"<player>", w->getName(), 1)),
						TXT_NETWORK);

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
    DropClient(cl, CLL_KICK);
}


///////////////////
// Kick a worm out of the server with a custom message
void GameServer::kickWorm(int wormID, std::string sReason)
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
			SendGlobalText(OldLxCompatibleString(replacemax(networkTexts->sHasBeenKicked,"<player>", w->getName(), 1)),
						TXT_NETWORK);

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
void GameServer::kickWorm(const std::string& szWormName)
{
    // Find the worm name
    CWorm *w = cWorms;
    for(int i=0; i<MAX_WORMS; i++, w++) {
        if(!w->isUsed())
            continue;

        if(stringcasecmp(w->getName(), szWormName) == 0) {
            kickWorm(i);
            return;
        }
    }

    // Didn't find the worm
    Con_Printf(CNC_NOTIFY, "Could not find worm '" + szWormName + "'");
}

///////////////////
// Kick a worm out of the server (by name) with a custom kick message
void GameServer::kickWorm(const std::string& szWormName, std::string sReason)
{
    // Find the worm name
    CWorm *w = cWorms;
    for(int i=0; i<MAX_WORMS; i++, w++) {
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
void GameServer::banWorm(int wormID)
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
	// We just kick the worm, banning has no sense
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
			SendGlobalText(OldLxCompatibleString(replacemax(networkTexts->sHasBeenBanned,"<player>", w->getName(), 1)),
							TXT_NETWORK);

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
    DropClient(cl, CLL_BAN);
}


void GameServer::banWorm(const std::string& szWormName)
{
    // Find the worm name
    CWorm *w = cWorms;
	if (!w)
		return;

    for(int i=0; i<MAX_WORMS; i++, w++) {
        if(!w->isUsed())
            continue;

        if(stringcasecmp(w->getName(), szWormName) == 0) {
            banWorm(i);
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

	/*if (!wormID)  {
		Con_Printf(CNC_NOTIFY,"You can't mute yourself.");
		return;  // Don't mute ourself
	}*/

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

	/*if (!wormID)  {
		Con_Printf(CNC_NOTIFY,"You can't unmute yourself.");
		return;  // Don't ban ourself
	}*/

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
void GameServer::notifyLog(char *fmt, ...)
{
    static char buf[512];
	va_list	va;

	va_start(va,fmt);
	vsnprintf(buf,sizeof(buf),fmt,va);
	fix_markend(buf);
	va_end(va);

    // Local hosting?
    // Add it to the clients chatbox
    if(cClient) {
        CChatBox *c = cClient->getChatbox();
        if(c)
            c->AddText(buf, MakeColour(200,200,200), tLX->fCurTime);
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
		CloseSocket(tSocket);
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

	cGameScript.Shutdown();
}
