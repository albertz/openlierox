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
	iMaxWorms = 8;
	iGameOver = false;
	iGameType = GMT_DEATHMATCH;
	fLastBonusTime = 0;
	SetSocketStateValid(tSocket, false);
	tGameLobby.nSet = false;
	bRegServer = false;
	bServerRegistered = false;
	fLastRegister = 0;
	nPort = LX_PORT;

	tGameLog = NULL;
	bTakeScreenshot = false;
	bScreenshotToken = false;
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

	// In zee lobby
	iState = SVS_LOBBY;

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
	bTournament =	 tGameInfo.bTournament;

	//
	// Start the logging
	//
	tGameLog = new game_log_t;
	if (!tGameLog)  {
		printf("Out of memory while allocating log\n");
		return false;
	}
	tGameLog->tWorms = NULL;
	tGameLog->fGameStart = tLX->fCurTime;
	tGameLog->iNumWorms = iNumPlayers;
	tGameLog->sGameStart = GetTime();

	bTakeScreenshot = false;
	bScreenshotToken = false;

	// Check
	if (!cWorms)
		return false;

	// Allocate the log worms
	int i;
	tGameLog->tWorms = new log_worm_t[iNumPlayers];
	if (!tGameLog->tWorms)
		return false;

	// Initialize the log worms
	for (i=0;i<iNumPlayers;i++)  {
		tGameLog->tWorms[i].bLeft = false;
		tGameLog->tWorms[i].iID = cWorms[i].getID();
		tGameLog->tWorms[i].sName = cWorms[i].getName();
		tGameLog->tWorms[i].sSkin = cWorms[i].getSkin();
		tGameLog->tWorms[i].iKills = 0;
		tGameLog->tWorms[i].iLives = tGameInfo.iLives;
		tGameLog->tWorms[i].iLeavingReason = -1;
		tGameLog->tWorms[i].iSuicides = 0;
		tGameLog->tWorms[i].bTagIT = false;
		if (tGameInfo.iGameMode == GMT_TEAMDEATH)
			tGameLog->tWorms[i].iTeam = cWorms[i].getTeam();
		else
			tGameLog->tWorms[i].iTeam = -1;
		tGameLog->tWorms[i].fTagTime = 0.0f;
		tGameLog->tWorms[i].fTimeLeft = 0.0f;
		tGameLog->tWorms[i].iType = cWorms[i].getType();
		NetworkAddr *a = cWorms[i].getClient()->getChannel()->getAddress();
		NetAddrToString(a,tGameLog->tWorms[i].sIP);
	}



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
	if(stringcasecmp(tGameInfo.sMapname,"_random_") == 0)
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

		sMapFilename = "levels/" + tGameInfo.sMapname;
		if(!cMap->Load(sMapFilename)) {
			printf("Error: Could not load the '%s' level\n",sMapFilename.c_str());
			return false;
		}
	}

	// Load the game script
	cGameScript.Shutdown();
	if(!cGameScript.Load(sModName)) {
		printf("Error: Could not load the '%s' game script\n",sModName.c_str());
		return false;
	}


    // Load & update the weapon restrictions
    cWeaponRestrictions.loadList("cfg/wpnrest.dat");
    cWeaponRestrictions.updateList(&cGameScript);


	// Set some info on the worms
	for(i=0;i<MAX_WORMS;i++) {
		if(cWorms[i].isUsed()) {
			cWorms[i].setLives(iLives);
            cWorms[i].setKills(0);
			cWorms[i].setGameScript(&cGameScript);
            cWorms[i].setWpnRest(&cWeaponRestrictions);
			cWorms[i].setLoadingTime( (float)iLoadingTimes / 100.0f );
			cWorms[i].setKillsInRow(0);
		}
	}

	// Clear bonuses
	for(i=0; i<MAX_BONUSES; i++)
		cBonuses[i].setUsed(false);

	// Clear the shooting list
	cShootList.Clear();



	fLastBonusTime = tLX->fCurTime;

	// Set all the clients to 'not ready'
	for(i=0;i<MAX_CLIENTS;i++) {
		cClients[i].getShootList()->Clear();
		cClients[i].setGameReady(false);
	}


    // If this is the host, and we have a team game: Send all the worm info back so the worms know what
    // teams they are on
    if( tGameInfo.iGameType == GME_HOST ) {
        if( iGameType == GMT_TEAMDEATH ) {

            CWorm *w = cWorms;
            CBytestream b;

            for( i=0; i<MAX_WORMS; i++, w++ ) {
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
	bs.writeString(sModName);

    cWeaponRestrictions.sendList(&bs);

	SendGlobalPacket(&bs);


	return true;
}


///////////////////
// Begin the match
void GameServer::BeginMatch(void)
{
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
			ParseConnectionlessPacket(&bs);
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


			// TODO: Check ports

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
	CBytestream *bs;
	static float oldtime =0;

	if(tLX->fCurTime - oldtime < 1.0/72.0)
		return;		// So we don't flood packets out to the clients
	else
		oldtime = tLX->fCurTime;



	// Go through each client and send them a message
	for(c=0;c<MAX_CLIENTS;c++,cl++) {
		if(cl->getStatus() == NET_DISCONNECTED)
			continue;


		if(iState == SVS_PLAYING && cl->getStatus() != NET_ZOMBIE)
			SendUpdate(cl);

		// Send out the packets if we havn't gone over the clients bandwidth
		bs = cl->getUnreliable();
		cl->getChannel()->Transmit(bs);

		// Clear the unreliable bytestream
		cl->getUnreliable()->Clear();
	}
}


///////////////////
// Register the server
void GameServer::RegisterServer(void)
{
	// Create the url
	static std::string url;
	static std::string buf;

	NetworkAddr addr;

	GetLocalNetAddr(tSocket,&addr);
	NetAddrToString(&addr, buf);

	url = std::string(LX_SVRREG) + "?port=" + itoa(nPort) + "&addr=" + buf;

    bServerRegistered = false;

    // Get the first server from the master server list
    FILE *fp = OpenGameFile("cfg/masterservers.txt","rt");
    if( !fp ) {
        bRegServer = false;
        notifyLog("Could not register with master server: 'Could not open master server file'");
        return;
    }

    // Find the first line
    while( !feof(fp) ) {
        buf = ReadUntil(fp);
        TrimSpaces(buf);
        if( buf != "" ) {
            if( !http_InitializeRequest(buf, url) ) {
                bRegServer = false;
                notifyLog("Could not register with master server: 'Could not open TCP socket'");
            }
            break;
        }
    }

    fclose(fp);
}


///////////////////
// Process the registering of the server
void GameServer::ProcessRegister(void)
{
    static std::string szError;

	if(!bRegServer || bServerRegistered)
		return;

	int result = http_ProcessRequest(&szError);

	// Normal, keep going
	if(result == 0)
		return;
	// Failed
    if(result == -1) {
		bRegServer = false;
        notifyLog("Could not register with master server: %s", szError.c_str());
    }
	// Completed ok
	if(result == 1) {
		bServerRegistered = true;
		fLastRegister = tLX->fCurTime;
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
	if( !bRegServer || !bServerRegistered )
		return false;

	// Create the url
	static std::string url;
	static std::string buf;

	NetworkAddr addr;

	GetLocalNetAddr(tSocket,&addr);
	NetAddrToString(&addr, buf);

	url = std::string(LX_SVRDEREG) + "?port=" + itoa(nPort) + "&addr=" + buf;

	// Initialize the request
	bServerRegistered = false;

	// Get the first server from the master server list
    FILE *fp = OpenGameFile("cfg/masterservers.txt","rt");
    if( !fp )
        return false;

    // Find the first line
    while( !feof(fp) ) {
    	buf = ReadUntil(fp);
        TrimSpaces(buf);
        if( buf != "" ) {
            if( !http_InitializeRequest(buf, url) ) {
                fclose(fp);
                return false;
            }
            break;
        }
    }

    fclose(fp);
    return true;
}


///////////////////
// Process the de-registering of the server
bool GameServer::ProcessDeRegister(void)
{
	return http_ProcessRequest(NULL) != 0;
}


///////////////////
// Check if any clients haved timed out or are out of zombie state
void GameServer::CheckTimeouts(void)
{
	int c;

	float dropvalue = tLX->fCurTime - LX_SVTIMEOUT;

	// Cycle through clients
	CClient *cl = cClients;
	for(c=0;c<MAX_CLIENTS;c++,cl++) {
		// Client not connected
		if(cl->getStatus() == NET_DISCONNECTED)
			continue;

        // Check for a drop
		if(cl->getLastReceived() < dropvalue && cl->getStatus() != NET_ZOMBIE) {
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

		// Log worm leaving
		log_worm_t *logworm = GetLogWorm(cl->getWorm(i)->getID());
		if (logworm)  {
			logworm->bLeft = true;
			logworm->iLeavingReason = reason;
			logworm->fTimeLeft = tLX->fCurTime;
		}

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


/////////////////
// Writes the log into the specified file
bool GameServer::WriteLogToFile(FILE *f)
{
	printf("WriteLogToFile intitializated\n");

	if (!f || !tGameLog)
		return false;

	if (!tGameLog->tWorms)
		return false;

	static std::string levelfile,modfile,level,mod,player,skin;

	printf("Filling in the game details... ");

	// Fill in the details
	levelfile = sMapFilename;
	modfile = sModName;
	level = cMap->getName();
	mod = cGameScript.GetHeader()->ModName;
	xmlEntities(levelfile);
	xmlEntities(modfile);
	xmlEntities(level);
	xmlEntities(mod);

	printf("DONE\n");

	// Save the game info
	fprintf(f,"<game datetime=\"%s\" length=\"%f\" loading=\"%i\" lives=\"%i\" maxkills=\"%i\" bonuses=\"%i\" bonusnames=\"%i\" levelfile=\"%s\" modfile=\"%s\" level=\"%s\" mod=\"%s\" gamemode=\"%i\">",
				tGameLog->sGameStart.c_str(),fGameOverTime-tGameLog->fGameStart,iLoadingTimes,iLives,iMaxKills,iBonusesOn,iShowBonusName,levelfile.c_str(),modfile.c_str(),level.c_str(),mod.c_str(),iGameType);

	printf("Game info saved\n");

	// Save the general players info
	fprintf(f,"<players startcount=\"%i\" endcount=\"%i\">",tGameLog->iNumWorms,iNumPlayers);

	printf("Players info saved\n");

	fflush(f);

	// Info for each player
	short i;
	for (i=0;i<tGameLog->iNumWorms;i++)  {
		printf("Writing player %i... ",i);

		// Replace the entities
		player = tGameLog->tWorms[i].sName;
		xmlEntities(player);

		// Replace the entities
		skin = tGameLog->tWorms[i].sSkin;
		xmlEntities(skin);

		// Write the info
		fprintf(f,"<player name=\"%s\" skin=\"%s\" id=\"%i\" kills=\"%i\" lives=\"%i\" suicides=\"%i\" team=\"%i\" tag=\"%i\" tagtime=\"%f\" left=\"%i\" leavingreason=\"%i\" timeleft=\"%f\" type=\"%i\" ip=\"%s\"/>",
		player.c_str(),skin.c_str(),tGameLog->tWorms[i].iID,tGameLog->tWorms[i].iKills,tGameLog->tWorms[i].iLives,tGameLog->tWorms[i].iSuicides,tGameLog->tWorms[i].iTeam,tGameLog->tWorms[i].bTagIT,tGameLog->tWorms[i].fTagTime,tGameLog->tWorms[i].bLeft,tGameLog->tWorms[i].iLeavingReason,MAX(0.0f,tGameLog->tWorms[i].fTimeLeft-tGameLog->fGameStart),tGameLog->tWorms[i].iType,tGameLog->tWorms[i].sIP.c_str());

		/*fprintf(f,"<player name=\"%s\" id=\"%i\" kills=\"%i\" lives=\"%i\" suicides=\"%i\" team=\"%i\" tag=\"%i\" tagtime=\"%f\" left=\"%i\" leavingreason=\"%i\" timeleft=\"%f\"/>",
				player,tGameLog->tWorms[i].iID,tGameLog->tWorms[i].iKills,tGameLog->tWorms[i].iLives,tGameLog->tWorms[i].iSuicides,tGameLog->tWorms[i].iTeam,tGameLog->tWorms[i].bTagIT,tGameLog->tWorms[i].fTagTime,tGameLog->tWorms[i].bLeft,tGameLog->tWorms[i].iLeavingReason,MAX(0.0f,tGameLog->tWorms[i].fTimeLeft-tGameLog->fGameStart));*/

		fflush(f);

		printf("DONE\n");
	}

	printf("Writing end tags\n");

	// End tags
	fprintf(f,"</players>");
	fprintf(f,"</game>");

	return true;
}

//////////////////
// Returns the log worm with the specified id
log_worm_t *GameServer::GetLogWorm(int id)
{
	// Check
	if (!tGameLog)
		return NULL;
	if (!tGameLog->tWorms)
		return NULL;

	// Go through the worms, checking the IDs
	log_worm_t *w = tGameLog->tWorms;
	int i;
	for(i=0;i<tGameLog->iNumWorms;i++,w++)
		if (w->iID == id)
			return w;

	// Not found
	return NULL;
}

///////////////////
// Shutdown the log structure
void GameServer::ShutdownLog(void)
{
	if (!tGameLog)
		return;

	// Free the worms
	if (tGameLog->tWorms)
		delete[] tGameLog->tWorms;

	// Free the log structure
	delete tGameLog;

	tGameLog = NULL;
}


///////////////////
// Shutdown the server
void GameServer::Shutdown(void)
{
	uint i;

	if(IsSocketStateValid(tSocket))
		CloseSocket(tSocket);
	SetSocketStateValid(tSocket, false);

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

	ShutdownLog();

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
