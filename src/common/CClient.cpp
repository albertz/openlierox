/////////////////////////////////////////
//
//             Liero Xtreme
//
//     Copyright Auxiliary Software 2002
//
//
/////////////////////////////////////////


// Client class
// Created 28/6/02
// Jason Boettcher


#include "defs.h"
#include "LieroX.h"
#include "console.h"


///////////////////
// Clear the client details
void CClient::Clear(void)
{
	iNumWorms = 0;
	int i;
	for(i=0;i<MAX_PLAYERS;i++)
		cLocalWorms[i] = NULL;
	cRemoteWorms = NULL;
	cProjectiles = NULL;
	cMap = NULL;
	cNetChan.Clear();
	iNetStatus = NET_DISCONNECTED;
	bsUnreliable.Clear();
	iChat_Numlines = 0;
	fLoadingTime = 1;
	iScorePlayers = 0;
	cBonuses = NULL;
    nTopProjectile = 0;

	SetSocketStateValid(tSocket, false);

	pChatbox = &cChatbox;
    cChatbox.setWidth(500);
	pChatbox->Clear();

	iLobbyReady = false;
	iGameReady = false;
	iReadySent = false;
	iGameOver = false;
	iGameMenu = false;
    bViewportMgr = false;
	tGameLobby.nSet = false;
	tGameLobby.nMaxWorms = 8;

	iBadConnection = false;
	iServerError = false;
    iClientError = false;
	iChat_Typing = false;
	fLastReceived = 99999;
	iPing = 0;
	fSendWait = 0;

	iCanToggle = true;

	bInServer = false;
	cIConnectedBuf[0] = '\0'; 

	fMyPingRefreshed = 0;
	iMyPing = 0;
	fMyPingSent = 0;

	// Clear the message sizes
	memset(nMessageSizes, 0, sizeof(int)*RATE_NUMMSGS);

	cShootList.Shutdown();
	cGameScript.Shutdown();
	cWeaponRestrictions.Shutdown();

    for(i=0; i<NUM_VIEWPORTS; i++) {
        cViewports[i].setUsed(false);
        cViewports[i].setID(i);
		cViewports[i].SetWorldX(0);
		cViewports[i].SetWorldY(0);
    }    
}


///////////////////
// Clear the client for another game
void CClient::MinorClear(void)
{
	iNetStatus = NET_CONNECTED;
	iLobbyReady = false;
	iGameReady = false;
	iReadySent = false;
	iGameOver = false;
	iGameMenu = false;
    bViewportMgr = false;

	//fProjDrawTime = 0;
	//fProjSimulateTime = 0;

	iBadConnection = false;
	iServerError = false;
    iClientError = false;
	iChat_Typing = false;
	fLastReceived = 99999;

	fSendWait = 0;

	iChat_Numlines = 0;
	cChatbox.Clear();

	int i;
	for(i=0; i<MAX_WORMS; i++)  {
		cRemoteWorms[i].setGameReady(false);
		cRemoteWorms[i].setTagIT(false);
		cRemoteWorms[i].setTagTime(0);
	}

	for(i=0; i<MAX_PROJECTILES; i++)
		cProjectiles[i].setUsed(false);
    nTopProjectile = 0;

	for(i=0; i<MAX_BONUSES; i++)
		cBonuses[i].setUsed(false);

    for(i=0; i<NUM_VIEWPORTS; i++)  {
        cViewports[i].setUsed(false);
		cViewports[i].SetWorldX(0);
		cViewports[i].SetWorldY(0);
	}
}


///////////////////
// Initialize the client
int CClient::Initialize(void)
{
	int i;

	// Shutdown & clear any old client data
	Shutdown();
	Clear();

	fLoadingTime = (float)tGameInfo.iLoadingTimes/100.0f;
	iNetSpeed = tLXOptions->iNetworkSpeed;

	// Local/host games use instant speed
	if(tGameInfo.iGameType != GME_JOIN)
		iNetSpeed = NST_LOCAL;


	// Initialize the local worms
	iNumWorms = tGameInfo.iNumPlayers;

	for(i=0;i<iNumWorms;i++) { 
		cLocalWorms[i] = NULL;
		tProfiles[i] = tGameInfo.cPlayers[i];
	}

	// Initialize the remote worms
	cRemoteWorms = new CWorm[MAX_WORMS];
	if(cRemoteWorms == NULL) {
		SetError("Error: Out of memory!\ncl::Initialize() %d",__LINE__);
		return false;
	}

	// Set id's
	for(i=0;i<MAX_WORMS;i++) {
		cRemoteWorms[i].Init();
		cRemoteWorms[i].setID(i);
		cRemoteWorms[i].setTagIT(false);
		cRemoteWorms[i].setTagTime(0);
	}
	

	// Initialize the projectiles
	cProjectiles = new CProjectile[MAX_PROJECTILES];
	if(cProjectiles == NULL) {
		SetError("Error: Out of memory!\ncl::Initialize() %d",__LINE__);
		return false;
	}
    for(i=0; i<MAX_PROJECTILES;i++)
        cProjectiles[i].setID(i);
    nTopProjectile = 0;


	// Initialize the bonuses
	cBonuses = new CBonus[MAX_BONUSES];
	if(cBonuses == NULL) {
		SetError("Error: Out of memory!\ncl::Initialize() %d",__LINE__);
		return false;
	}


	// Open a new socket
	tSocket = OpenUnreliableSocket(0);
	if(!IsSocketStateValid(tSocket)) {
		SetError("Error: Could not open UDP socket!");
		return false;
	}


	// Initialize the drawing
	if(!InitializeDrawing())
		return false;
	

	// Clear the network channel
	cNetChan.Clear();

	// Initialize the shooting list
	cShootList.Initialize();

	cChat_Input.Setup(tLXOptions->sGeneralControls[SIN_CHAT]);
    cShowScore.Setup(tLXOptions->sGeneralControls[SIN_SCORE]);
	cShowHealth.Setup(tLXOptions->sGeneralControls[SIN_HEALTH]);
	cShowSettings.Setup(tLXOptions->sGeneralControls[SIN_SETTINGS]);
	cViewportMgr.Setup(tLXOptions->sGeneralControls[SIN_VIEWPORTS]);

	// Clear the frames


    // Initialize the weather
    //cWeather.Initialize(wth_snow);

	
	return true;
}


///////////////////
// Main frame
void CClient::Frame(void)
{
	if(iNetStatus == NET_PLAYING) {
		iServerFrame++;
		fServerTime += tLX->fDeltaTime;
	}

    static float oldtime =0;

	/*if(tLX->fCurTime - oldtime < 1.0/72.0)
		return;		// So we don't flood packets out to the clients
	else
		oldtime = tLX->fCurTime;*/

	
	ReadPackets();

    SimulateHud();

    if(iNetStatus == NET_PLAYING)
		Simulation();

	SendPackets();

	// Connecting process
	Connecting();
}


///////////////////
// Read the packets
void CClient::ReadPackets(void)
{
	CBytestream		bs;

	while(bs.Read(tSocket)) {

		// Check for connectionless packets (four leading 0xff's)
		if(*(int *)bs.GetData() == -1) {
			bs.SetPos(4);
			ParseConnectionlessPacket(&bs);
			continue;
		}

		if(iNetStatus == NET_DISCONNECTED || iNetStatus == NET_CONNECTING)
			continue;

		// Parse the packet
		if(cNetChan.Process(&bs))
			ParsePacket(&bs);
	}


	// Check if our connection with the server timed out
	if(iNetStatus == NET_PLAYING && cNetChan.getLastReceived() < tLX->fCurTime - LX_CLTIMEOUT && tGameInfo.iGameType == GME_JOIN) {
		// Time out
		iServerError = true;
		strcpy(strServerErrorMsg, "Connection with server timed out");
		
		// The next frame will pickup the server error flag set & handle the msgbox, disconnecting & quiting
	}
}

 
///////////////////
// Send the packets
void CClient::SendPackets(void)
{

	// So we don't flood packets out to server
	/*fSendWait += tLX->fDeltaTime;
	if(fSendWait < 0.5)  {
		fSendWait = 0;
		return;
	}*/

	// Playing packets
	if(iNetStatus == NET_PLAYING)
		SendWormDetails();

	if(iNetStatus == NET_PLAYING || iNetStatus == NET_CONNECTED)
		cNetChan.Transmit(&bsUnreliable);
	
	bsUnreliable.Clear();
}


///////////////////
// Start a connection with the server
void CClient::Connect(char *address)
{
	iNetStatus = NET_CONNECTING;
	strcpy(strServerAddr,address);
	iNumConnects=0;
	iBadConnection = false;

	fConnectTime = -99999;		// This will force the connecting process to run straight away
}


///////////////////
// Connecting process
void CClient::Connecting(void)
{
	NetworkAddr addr;

	// Check if we are connecting
	if(iNetStatus != NET_CONNECTING)
		return;

	// Try every 3 seconds
	if(tLX->fCurTime - fConnectTime < 3)
		return;


	// If we have tried 10 times (10*3 = 30secs) just quit trying
	if(iNumConnects >= 10) {
		iNetStatus = NET_DISCONNECTED;
		return;
	}

	StringToNetAddr(strServerAddr,&addr);
	if(GetNetAddrPort(&addr) == 0)
		SetNetAddrPort(&addr,tLXOptions->iNetworkPort);			// TODO: Potential error; What if we are 
																// connecting remotely?

	fConnectTime = tLX->fCurTime;
	iNumConnects++;

	// Request a challenge id
	CBytestream bs;
	bs.writeInt(-1,4);
	bs.writeString("%s","lx::getchallenge");
	
	SetRemoteNetAddr(tSocket,&addr);
	bs.Send(tSocket);

	printf("HINT: sending challenge request to %s\n", strServerAddr);
}


///////////////////
// Disconnect
void CClient::Disconnect(void)
{
	CBytestream bs;

	bs.writeByte(C2S_DISCONNECT);

	// Send the pack a few times to make sure the server gets the packet
	for(int i=0;i<3;i++)
		cNetChan.Transmit(&bs);

	iNetStatus = NET_DISCONNECTED;


	if (tLXOptions->iLogConvos)  {
		FILE *f;


		if(!bInServer)
			return;

		f = OpenGameFile("Conversations.log","a");
		if (!f) 
			return;
		fputs("  </server>\r\n",f);
		bInServer = false;
		fclose(f);
	}
}


///////////////////
// Setup the viewports for the local players
void CClient::SetupViewports(void)
{
    for( int i=0; i<3; i++ )
        cViewports[i].setUsed(false);

    // Setup inputs
    cViewports[0].setupInputs( tLXOptions->sPlayer1Controls );
    cViewports[1].setupInputs( tLXOptions->sPlayer2Controls );


	// If there is only 1 local player, setup 1 main viewport
	if(iNumWorms == 1) {
        // HACK HACK: FOR AI TESTING
        //cViewports[0].Setup(0,0,640,382,VW_FREELOOK);

        cViewports[0].Setup(0,0,640,382,VW_FOLLOW);
        cViewports[0].setTarget(cLocalWorms[0]);
		cViewports[0].setUsed(true);
	}

	// If there are more then 2 local players, only use 2 viewports if the first two are humans
	if(iNumWorms >= 2) {
        bool both = false;
        if( cLocalWorms[1]->getType() == PRF_HUMAN )
            both = true;

        if( !both ) {
            cViewports[0].Setup(0,0,640,382,VW_FOLLOW);
            cViewports[0].setTarget(cLocalWorms[0]);
		    cViewports[0].setUsed(true);
        }

        if( both ) {
            cViewports[0].Setup(0,0,318,382,VW_FOLLOW);
            cViewports[0].setTarget(cLocalWorms[0]);
		    cViewports[0].setUsed(true);

		    cViewports[1].Setup(322,0,318,382,VW_FOLLOW);
            cViewports[1].setTarget(cLocalWorms[1]);
		    cViewports[1].setUsed(true);
        }
	}
}


///////////////////
// Return true if we own the worm
int CClient::OwnsWorm(CWorm *w)
{
	for(int i=0;i<iNumWorms;i++) {
		if(w->getID() == cLocalWorms[i]->getID())
			return true;
	}

	return false;
}


///////////////////
// Setup the worms (server func)
void CClient::SetupWorms(int numworms, CWorm *worms)
{
	/*iNumWorms = numworms;

	cLocalWorms = new CWorm[iNumWorms];
	if(cLocalWorms == NULL)
		return;

	for(int i=0;i<iNumWorms;i++)
		cLocalWorms[i] = worms[i];*/
}


///////////////////
// Shutdown the client
void CClient::Shutdown(void)
{
	int i;

	// Remote worms
	if(cRemoteWorms) {
		for(i=0;i<MAX_WORMS;i++)
			cRemoteWorms[i].Shutdown();
		delete[] cRemoteWorms;
		cRemoteWorms = NULL;
	}

	// Projectiles
	if(cProjectiles) {
		delete[] cProjectiles;
		cProjectiles = NULL;
	}

	// Map
	if(tGameInfo.iGameType == GME_JOIN) {
		if(cMap) {
			int bCreated = cMap->getCreated() > 0;
			cMap->Shutdown();
			if (bCreated)
				delete cMap;
			cMap = NULL;
		}
	}

	// Shooting list
	cShootList.Shutdown();


	// Bonuses
	if(cBonuses) {
		delete[] cBonuses;
		cBonuses = NULL;
	}

	// Gamescript
	cGameScript.Shutdown();

	// Weapon restrictions
	cWeaponRestrictions.Shutdown();
	
	// Close the socket
	if(IsSocketStateValid(tSocket))
		CloseSocket(tSocket);
	SetSocketStateValid(tSocket, false);

	// Log this
	if (tLXOptions->iLogConvos)  {
		FILE *f;

		if(!bInServer)
			return;

		f = OpenGameFile("Conversations.log","a");
		if (!f) 
			return;
		fputs("  </server>\r\n",f);
		bInServer = false;
		fclose(f);
	}
}
