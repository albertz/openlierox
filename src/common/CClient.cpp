/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Client class
// Created 28/6/02
// Jason Boettcher


#include "LieroX.h"
#include "CClient.h"
#include "CBonus.h"
#include "Menu.h"
#include "console.h"
#include "FindFile.h"
#include "Graphics.h"
#include "CBar.h"
#include "CWorm.h"
#include "Error.h"
#include "Protocol.h"
#include "StringUtils.h"
#include "MathLib.h"
#include "EndianSwap.h"
#include "Version.h"


const float fDownloadRetryTimeout = 5.0;	// 5 seconds


///////////////////
// Clear the client details
void CClient::Clear(void)
{

#ifdef DEBUG
	if (cRemoteWorms || cProjectiles || cBonuses)  {
#ifdef _MSC_VER
		__asm int 3; // Breakpoint
#endif
		printf("FIXME: clearing a client that wasn't shut down! This will cause a memleak!");
	}
#endif

	iNumWorms = 0;
	int i;
	for(i=0;i<MAX_PLAYERS;i++)
		cLocalWorms[i] = NULL;
	cRemoteWorms = NULL;
	cProjectiles = NULL;
	cMap = NULL;
	bMapGrabbed = false;
	cNetChan.Clear();
	iNetStatus = NET_DISCONNECTED;
	bsUnreliable.Clear();
	iChat_Numlines = 0;
	fLoadingTime = 1;
	iScorePlayers = 0;
	cBonuses = NULL;
    nTopProjectile = 0;
	bUpdateScore = true;
	cChatList = NULL;
	bmpIngameScoreBg = NULL;
	bCurrentSettings = false;
	bForceWeaponsReady = false;

	tGameLog = NULL;
	iLastVictim = -1;
	iLastKiller = -1;

	InvalidateSocketState(tSocket);
	SetNetAddrValid( cServerAddr, false );

    cChatbox.setWidth(325);
	cChatbox.Clear();

	bLobbyReady = false;
	bGameReady = false;
	bReadySent = false;
	bGameOver = false;
	bGameMenu = false;
    bViewportMgr = false;
	tGameLobby.bSet = false;
	tGameLobby.nMaxWorms = MAX_PLAYERS;

	bBadConnection = false;
	bServerError = false;
    bClientError = false;
	bChat_Typing = false;
	fLastReceived = 99999;
	fSendWait = 0;
	fLastUpdateSent = -9999;


	bInServer = false;
	cIConnectedBuf = "";

	fMyPingRefreshed = 0;
	iMyPing = 0;
	fMyPingSent = 0;

	// Clear the message sizes
	memset(nMessageSizes, 0, sizeof(int)*RATE_NUMMSGS);

	// HINT: gamescript is shut down by the cache

	cShootList.Shutdown();
	cWeaponRestrictions.Shutdown();

    for(i=0; i<NUM_VIEWPORTS; i++) {
        cViewports[i].setUsed(false);
        cViewports[i].setID(i);
		cViewports[i].SetWorldX(0);
		cViewports[i].SetWorldY(0);
    }

	iHostOLXVer = 0;
	bHostAllowsMouse = false;

	bDownloadingMap = false;
	cHttpDownloader = NULL;
	sMapDownloadName = "";
	bMapDlError = false;
	sMapDlError = "";
	iMapDlProgress = 0;
	fLastDirtUpdate = fLastFileRequest = fLastFileRequestPacketReceived = tLX->fCurTime;
	iPartialDirtUpdateCount = 0;
	cPreviousDirtMap = "";
	getUdpFileDownloader()->reset();
	fSpectatorViewportMsgTimeout = tLX->fCurTime;
	sSpectatorViewportMsg = "";
	bSpectate = false;
}


///////////////////
// Clear the client for another game
void CClient::MinorClear(void)
{
	iNetStatus = NET_CONNECTED;
	bLobbyReady = false;
	bGameReady = false;
	bReadySent = false;
	bGameOver = false;
	bGameMenu = false;
    bViewportMgr = false;
	bUpdateScore = true;
	bCurrentSettings = false;
	bForceWeaponsReady = false;

	//fProjDrawTime = 0;
	//fProjSimulateTime = 0;

	iLastVictim = -1;
	iLastKiller = -1;

	bBadConnection = false;
	bServerError = false;
    bClientError = false;
	bChat_Typing = false;
	fLastReceived = 99999;

	fSendWait = 0;

	iChat_Numlines = 0;
	((CListview *)cChatList)->Clear();

	int i;
	for(i=0; i<MAX_WORMS; i++)  {
		cRemoteWorms[i].setGameReady(false);
		cRemoteWorms[i].setTagIT(false);
		cRemoteWorms[i].setTagTime(0);
	}

	for(i=0; i<MAX_PROJECTILES; i++)
		cProjectiles[i].setUnused();
    nTopProjectile = 0;

	for(i=0; i<MAX_BONUSES; i++)
		cBonuses[i].setUsed(false);

    for(i=0; i<NUM_VIEWPORTS; i++)  {
        cViewports[i].setUsed(false);
		cViewports[i].SetWorldX(0);
		cViewports[i].SetWorldY(0);
	}
	fLastDirtUpdate = fLastFileRequest = fLastFileRequestPacketReceived = tLX->fCurTime;
	iPartialDirtUpdateCount = 0;
	cPreviousDirtMap = "";
	getUdpFileDownloader()->reset();
	fSpectatorViewportMsgTimeout = tLX->fCurTime;
	sSpectatorViewportMsg = "";
}


///////////////////
// Initialize the client
int CClient::Initialize(void)
{
	uint i;

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
		SetError("Error: Out of memory!\ncl::Initialize() " + itoa(__LINE__));
		return false;
	}

	// Set id's
	for(i=0;i<MAX_WORMS;i++) {
		cRemoteWorms[i].Init();
		cRemoteWorms[i].setID(i);
		cRemoteWorms[i].setTagIT(false);
		cRemoteWorms[i].setTagTime(0);
		cRemoteWorms[i].setTeam(0);
		cRemoteWorms[i].setFlag(false);
		cRemoteWorms[i].setUsed(false);
	}


	// Initialize the projectiles
	cProjectiles = new CProjectile[MAX_PROJECTILES];
	if(cProjectiles == NULL) {
		SetError("Error: Out of memory!\ncl::Initialize() " + itoa(__LINE__));
		return false;
	}
    for(i=0; i<MAX_PROJECTILES;i++)
        cProjectiles[i].setID(i);
    nTopProjectile = 0;


	// Initialize the bonuses
	cBonuses = new CBonus[MAX_BONUSES];
	if(cBonuses == NULL) {
		SetError("Error: Out of memory!\ncl::Initialize() " + itoa(__LINE__));
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

	// Initialize chat box (must be after drawing because of interface settings)
	cChatList = (void *)(new CListview);
	if (!cChatList)
		return false;
	((CListview *)cChatList)->Clear();
	((CListview *)cChatList)->setShowSelect(false);
	((CListview *)cChatList)->setRedrawMenu(false);
	((CListview *)cChatList)->setDrawBorder(false);
	((CListview *)cChatList)->Setup(0,
									tInterfaceSettings.ChatBoxX,
									tInterfaceSettings.ChatBoxY,
									tInterfaceSettings.ChatBoxW,
									tInterfaceSettings.ChatBoxH);
	((CListview *)cChatList)->SetupScrollbar(tInterfaceSettings.ChatboxScrollbarX,
											 tInterfaceSettings.ChatboxScrollbarY,
											 tInterfaceSettings.ChatboxScrollbarH,
											 tInterfaceSettings.ChatboxScrollbarAlwaysVisible);



	// Clear the network channel
	cNetChan.Clear();

	// Initialize the shooting list
	cShootList.Initialize();

	// General key shortcuts
	cChat_Input.Setup(tLXOptions->sGeneralControls[SIN_CHAT]);
	cTeamChat_Input.Setup(tLXOptions->sGeneralControls[SIN_TEAMCHAT]);
    cShowScore.Setup(tLXOptions->sGeneralControls[SIN_SCORE]);
	cShowHealth.Setup(tLXOptions->sGeneralControls[SIN_HEALTH]);
	cShowSettings.Setup(tLXOptions->sGeneralControls[SIN_SETTINGS]);
	cViewportMgr.Setup(tLXOptions->sGeneralControls[SIN_VIEWPORTS]);
	cToggleTopBar.Setup(tLXOptions->sGeneralControls[SIN_TOGGLETOPBAR]);

	// Clear the frames


    // Initialize the weather
    //cWeather.Initialize(wth_snow);

	InitializeSpectatorViewportKeys();

	return true;
}

///////////////////
// Initialize the logging
void CClient::StartLogging(int num_players)
{
	// Shutdown any previous logs
	ShutdownLog();

	// Allocate the log
	tGameLog = new game_log_t;
	if (!tGameLog)  {
		printf("Out of memory while allocating log\n");
		return;
	}
	tGameLog->tWorms = NULL;
	tGameLog->fGameStart = tLX->fCurTime;
	tGameLog->iNumWorms = num_players;
	tGameLog->iWinner = -1;
	tGameLog->sGameStart = GetTime();
	tGameLog->sServerName = szServerName;
	NetAddrToString(cNetChan.getAddress(), tGameLog->sServerIP);

	// Allocate log worms
	int i;
	tGameLog->tWorms = new log_worm_t[num_players];
	if (!tGameLog->tWorms)
		return;

	// Initialize the log worms
	for (i=0; i < num_players; i++)  {
		tGameLog->tWorms[i].bLeft = false;
		tGameLog->tWorms[i].iID = cRemoteWorms[i].getID();
		tGameLog->tWorms[i].sName = cRemoteWorms[i].getName();
		tGameLog->tWorms[i].sSkin = cRemoteWorms[i].getSkin();
		tGameLog->tWorms[i].iKills = 0;
		tGameLog->tWorms[i].iLives = tGameInfo.iLives;
		tGameLog->tWorms[i].iSuicides = 0;
		tGameLog->tWorms[i].iTeamKills = 0;
		tGameLog->tWorms[i].iTeamDeaths = 0;
		tGameLog->tWorms[i].bTagIT = false;
		if (tGameInfo.iGameMode == GMT_TEAMDEATH || tGameInfo.iGameMode == GMT_VIP)
			tGameLog->tWorms[i].iTeam = cRemoteWorms[i].getTeam();
		else
			tGameLog->tWorms[i].iTeam = -1;
		tGameLog->tWorms[i].fTagTime = 0.0f;
		tGameLog->tWorms[i].fTimeLeft = 0.0f;
		tGameLog->tWorms[i].iType = cRemoteWorms[i].getType();
	}
}

/////////////////
// Initialize map downloading
void CClient::InitializeDownloads()
{
	if (!cHttpDownloader)
		cHttpDownloader = new CHttpDownloadManager();
	bDownloadingMap = false;
	bMapDlError = false;
	iMapDlProgress = 0;
	sMapDlError = "";
	sMapDownloadName = "";
}

/////////////////
// Shutdown map downloading
void CClient::ShutdownDownloads()
{
	if (cHttpDownloader)
		delete cHttpDownloader;
	cHttpDownloader = NULL;
	bDownloadingMap = false;
	bMapDlError = false;
	iMapDlProgress = 0;
	sMapDlError = "";
	sMapDownloadName = "";
}

/////////////////
// Download a map
void CClient::DownloadMap(const std::string& mapname)
{
	// Initialize if not initialized
	InitializeDownloads();

	// Check
	if (!cHttpDownloader)  {
		sMapDlError = "Could not initialize the downloader";
		bMapDlError = true;
		return;
	}

	cHttpDownloader->StartFileDownload(mapname, "levels");
	sMapDownloadName = mapname;
	bDownloadingMap = true;
	iDownloadMethod = DL_HTTP; // We start with HTTP
}

/////////////////
// Process map downloading
void CClient::ProcessMapDownloads()
{

	if( bDownloadingMap && iDownloadMethod == DL_HTTP && cHttpDownloader )  {

		// Process
		cHttpDownloader->ProcessDownloads();

		// Download finished
		if (cHttpDownloader->IsFileDownloaded(sMapDownloadName))  {
			bDownloadingMap = false;
			bMapDlError = false;
			sMapDlError = "";
			iMapDlProgress = 100;

			// Check that the file exists (it should!)
			if (IsFileAvailable("levels/" + sMapDownloadName, false))  {
				tGameLobby.bHaveMap = true;
				tGameLobby.szDecodedMapName = Menu_GetLevelName(sMapDownloadName);

				// If playing, load the map
				if (iNetStatus == NET_PLAYING)  {
					if (cMap)  {
						printf("WARNING: Finished map downloading but another map is already loaded.\n");
						return;
					}

					cMap = new CMap;
					if (cMap)  {
						if (!cMap->Load("levels/" + sMapDownloadName))  {  // Load the map
							// Weird
							printf("Could not load the downloaded map!\n");
							Disconnect();
							GotoNetMenu();
						}
					}
				}
			} else {  // Inaccessible
				printf("Cannot access the downloaded map!\n");
				if (iNetStatus == NET_PLAYING)  {
					Disconnect();
					GotoNetMenu();
				}
			}

			sMapDownloadName = "";

			return;
		}

		// Error check
		DownloadError error = cHttpDownloader->FileDownloadError(sMapDownloadName);
		if (error.iError != FILEDL_ERROR_NO_ERROR)  {
			sMapDlError = sMapDownloadName + " downloading error: " + error.sErrorMsg;

			// HTTP failed, let's try UDP
			if( getHostVer() > 4 )
			{
				printf("Could not download the map via HTTP, trying UDP...\n");
				getUdpFileDownloader()->requestFile("levels/" + sMapDownloadName, true);
				getUdpFileDownloader()->requestFileInfo("levels/" + sMapDownloadName, true); // To get valid progressbar
				iDownloadMethod = DL_UDP;
			};
		}

		// Get the progress
		iMapDlProgress = cHttpDownloader->GetFileProgress(sMapDownloadName);
	};



  	// UDP file download used for maps, mods and worm skins - we can download map via HTTP and mod via UDP from host
	if( getHostVer() < 4 || iNetStatus == NET_DISCONNECTED )
		return;

	if( getUdpFileDownloader()->isReceiving() )	 {
		if( fLastFileRequestPacketReceived + fDownloadRetryTimeout < tLX->fCurTime ) { // Server stopped sending file in the middle
			fLastFileRequestPacketReceived = tLX->fCurTime;
			if( ! getUdpFileDownloader()->requestFilesPending() )  { // More files to receive
				bMapDlError = true;
				bDownloadingMap = false;
			}
		}
		if( getUdpFileDownloader()->getFileDownloading() == "levels/" + cClient->getGameLobby()->szMapName ) {
			bDownloadingMap = true;
			iDownloadMethod = DL_UDP;
		};
		if( bDownloadingMap && iDownloadMethod == DL_UDP )
			iMapDlProgress = (int)(getUdpFileDownloader()->getFileDownloadingProgress() * 100.0f);
		return;
	}

	// Server requested some file (wormskin) or we're sending file request
	if( getUdpFileDownloader()->isSending() )	 {
		CBytestream bs;
		bs.writeByte(C2S_SENDFILE);
		getUdpFileDownloader()->send(&bs);
		cNetChan.AddReliablePacketToSend(bs);
		fLastFileRequestPacketReceived = tLX->fCurTime;
		fLastFileRequest = tLX->fCurTime + fDownloadRetryTimeout/10.0f;
		return;
	}

	// Download finished
	if( getUdpFileDownloader()->wasError() )  {
		getUdpFileDownloader()->clearError();
		bMapDlError = true;
		bDownloadingMap = false;
	}

	if( bDownloadingMap && iDownloadMethod == DL_UDP )
		bDownloadingMap = false;

	if( fLastFileRequest > tLX->fCurTime )
		return;

	fLastFileRequestPacketReceived = tLX->fCurTime;
	fLastFileRequest = tLX->fCurTime + fDownloadRetryTimeout/10.0f; // Request another file from server after little timeout

	getUdpFileDownloader()->requestFilesPending(); // More files to receive?
}

///////////////////
// Main frame
void CClient::Frame(void)
{
	if(iNetStatus == NET_PLAYING) {
		iServerFrame++;
		fServerTime += tLX->fRealDeltaTime;
	}

	ProcessMapDownloads();

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
		// each bs.Read reads the next UDP packet and resets the bs
		// UDP is packet-based that means, we will only get single packages, no stream

		// Check for connectionless packets (four leading 0xff's)
		if(bs.readInt(4) == -1) {
			bs.ResetPosToBegin();
			// parse all connectionless packets (there can be multiple packages in one package)
			// For example lx::openbeta* was sent in a way that 2 packages were sent at once.
			// <rev1457 (incl. Beta3) versions only will parse one package at a time.
			// I fixed that now since >rev1457 that it parses multiple packages here
			// (but only for new net-commands).
			// Same thing in CServer.cpp in ReadPackets
			while(!bs.isPosAtEnd() && bs.readInt(4) == -1)
				ParseConnectionlessPacket(&bs);
			continue;
		}
		bs.ResetPosToBegin();

		if(iNetStatus == NET_DISCONNECTED || iNetStatus == NET_CONNECTING)
			continue;

		// Parse the packet
		if(cNetChan.Process(&bs))
			ParsePacket(&bs);
	}


	// Check if our connection with the server timed out
	if(iNetStatus == NET_PLAYING && cNetChan.getLastReceived() < tLX->fCurTime - LX_CLTIMEOUT && tGameInfo.iGameType == GME_JOIN) {
		// Time out
		bServerError = true;
		strServerErrorMsg = "Connection with server timed out";

		// Stop any file downloads
		if (bDownloadingMap && cHttpDownloader)
			cHttpDownloader->CancelFileDownload(sMapDownloadName);
		getUdpFileDownloader()->reset();

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


	// Randomly send a random packet
#ifdef DEBUG
	/*if (GetRandomInt(50) > 24)
		SendRandomPacket();*/
#endif

	if(iNetStatus == NET_PLAYING || iNetStatus == NET_CONNECTED)
		cNetChan.Transmit(&bsUnreliable);

	bsUnreliable.Clear();
}


///////////////////
// Start a connection with the server
void CClient::Connect(const std::string& address)
{
	iNetStatus = NET_CONNECTING;
	strServerAddr_HumanReadable = strServerAddr = address;
	iNumConnects = 0;
	bBadConnection = false;
	iHostOLXVer = 0;
	fConnectTime = tLX->fCurTime;

	if(!StringToNetAddr(address, cServerAddr)) {

		strServerAddr_HumanReadable = strServerAddr + " (...)";
		Timer(&Timer::DummyHandler, NULL, DNS_TIMEOUT * 1000, true).startHeadless();

		if(!GetNetAddrFromNameAsync(address, cServerAddr)) {
			iNetStatus = NET_DISCONNECTED;
			bBadConnection = true;
			strBadConnectMsg = "Unknown error while resolving address";
		}
	}

	// send a challenge packet immediatly (if possible)
	Connecting(true);
}


///////////////////
// Connecting process
void CClient::Connecting(bool force)
{
	// Check if we are connecting
	if(iNetStatus != NET_CONNECTING)
		return;

	// Try every 3 seconds
	if(!force && (tLX->fCurTime - fConnectTime < 3))
		return;

	// For local play/hosting: don't send the challenge more times
	// On slower machines the loading can be pretty slow and take more than 3 seconds
	// That doesn't mean that the packet is not delivered
	if (tGameInfo.iGameType != GME_JOIN && iNumConnects > 0)
		return;


	// If we have tried 10 times (10*3 = 30secs) just quit trying
	if(iNumConnects >= 10) {
		iNetStatus = NET_DISCONNECTED;
		bBadConnection = true;
		strBadConnectMsg = "Server timeout after 10 tries";
		return;
	}

	if(!IsNetAddrValid(cServerAddr)) {
		if(tLX->fCurTime - fConnectTime >= DNS_TIMEOUT) { // timeout
			iNetStatus = NET_DISCONNECTED;
			bBadConnection = true;
			strBadConnectMsg = "Domain could not be resolved after " + itoa(DNS_TIMEOUT) + " seconds";
		}
		return;
	}

	if(GetNetAddrPort(cServerAddr) == 0)  {
		if (tGameInfo.iGameType == GME_JOIN) // Remote joining
			SetNetAddrPort(cServerAddr, LX_PORT);  // Try the default port if no port specified
		else // Host or local
			SetNetAddrPort(cServerAddr, tLXOptions->iNetworkPort);  // Use the port specified in options
	}

	{
		std::string rawServerAddr;
		NetAddrToString( cServerAddr, rawServerAddr );
		if( rawServerAddr != strServerAddr )
			strServerAddr_HumanReadable = strServerAddr + " (" + rawServerAddr + ")";
	}

	fConnectTime = tLX->fCurTime;
	iNumConnects++;

	// Request a challenge id
	CBytestream bs;
	bs.writeInt(-1,4);
	bs.writeString("lx::getchallenge");
	bs.writeString(GetFullGameName());

	// As we use this tSocket both for sending and receiving,
	// it's saver to reset the address here.
	SetRemoteNetAddr(tSocket, cServerAddr);
	bs.Send(tSocket);

	Timer(&Timer::DummyHandler, NULL, 3000, true).startHeadless();

	printf("HINT: sending challenge request to %s\n", strServerAddr.c_str());
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

	if (bDownloadingMap)
		cHttpDownloader->CancelFileDownload(sMapDownloadName);
	getUdpFileDownloader()->reset();


	if (tLXOptions->bLogConvos)  {
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
	// Two players
	if (iNumWorms >= 2)
		if (cLocalWorms[1])
			if (cLocalWorms[1]->getType() == PRF_HUMAN)  {
				SetupViewports(cLocalWorms[0], cLocalWorms[1], VW_FOLLOW, VW_FOLLOW);
				return;
			}

	// Only one player
	SetupViewports(cLocalWorms[0], NULL, VW_FOLLOW, VW_FOLLOW);
}


///////////////////
// Setup the viewports
void CClient::SetupViewports(CWorm *w1, CWorm *w2, int type1, int type2)
{
	if (w1 == NULL)  {
		printf("CClient::SetupViewports: Worm1 is NULL, quitting!");
		return;
	}

	// Reset
    for( int i=0; i<NUM_VIEWPORTS; i++ )
        cViewports[i].setUsed(false);

    // Setup inputs
    cViewports[0].setupInputs( tLXOptions->sPlayerControls[0] );
    cViewports[1].setupInputs( tLXOptions->sPlayerControls[1] );


	// Setup according to top and bottom interface bars
	SDL_Surface *topbar = NULL;
	SDL_Surface *bottombar = NULL;
	if (tGameInfo.iGameType == GME_LOCAL)  {
		bottombar = gfxGame.bmpGameLocalBackground;
		topbar = gfxGame.bmpGameLocalTopBar;
	} else {
		bottombar = gfxGame.bmpGameNetBackground;
		topbar = gfxGame.bmpGameNetTopBar;
	}


	int top = topbar ? (topbar->h) : (tLX->cFont.GetHeight() + 3); // Top bound of the viewports
	if (!tLXOptions->tGameinfo.bTopBarVisible)
		top = 0;

	int h = bottombar ? (480 - bottombar->h - top) : (382 - top); // Height of the viewports

	// One worm
	if(w2 == NULL) {
        // HACK HACK: FOR AI TESTING
        //cViewports[0].Setup(0,0,640,382,VW_FREELOOK);

        cViewports[0].Setup(0, top, 640, h, type1);
        cViewports[0].setTarget(w1);
		cViewports[0].setUsed(true);
		cViewports[0].setSmooth( !OwnsWorm(w1) );
	}

	// Two wormsize
	else  {
        cViewports[0].Setup(0, top, 318, h, type1);
        cViewports[0].setTarget(w1);
		cViewports[0].setUsed(true);
		cViewports[0].setSmooth( !OwnsWorm(w1) );

		cViewports[1].Setup(322, top, 318, h, type2);
        cViewports[1].setTarget(w2);
		cViewports[1].setUsed(true);
		cViewports[1].setSmooth( !OwnsWorm(w2) );
	}
}


///////////////////
// Return true if we own the worm
int CClient::OwnsWorm(CWorm *w)
{
	for(uint i=0;i<iNumWorms;i++) {
		if(w->getID() == cLocalWorms[i]->getID())
			return true;
	}

	return false;
}

//////////////////
// Remove the worm
void CClient::RemoveWorm(int id)
{
	iNumWorms--;

	int i,j;
	for (i=0;i<MAX_PLAYERS;i++)  {
		if (cLocalWorms[i])  {
			if (cLocalWorms[i]->getID() == id)  {
				cLocalWorms[i] = NULL;
				for (j=i;j<MAX_PLAYERS-2;j++)  {
					cLocalWorms[j] = cLocalWorms[j+1];
				}
			}
		}
	}

	if (cRemoteWorms)  {
		for (i=0;i<MAX_WORMS;i++) {
			if (cRemoteWorms[i].getID() == id)  {
				cRemoteWorms[i].setUsed(false);
				cRemoteWorms[i].setAlive(false);
				cRemoteWorms[i].setKills(0);
				cRemoteWorms[i].setLives(WRM_OUT);
				cRemoteWorms[i].setProfile(NULL);
				if (cRemoteWorms[i].getType() == PRF_COMPUTER)  {
					cRemoteWorms[i].AI_Shutdown();
					cRemoteWorms[i].setType(PRF_HUMAN);
				}
				cRemoteWorms[i].setLocal(false);
				cRemoteWorms[i].setTagIT(false);
				cRemoteWorms[i].setTagTime(0);
			}
		}
	}

}

/////////////////
// Writes the log into the specified file
void CClient::GetLogData(std::string& data)
{
	// Clear
	data = "";

	// Checks
	if (!tGameLog)
		return;

	if (!tGameLog->tWorms)
		return;

	std::string levelfile, modfile, level, mod, player, skin;

	// Fill in the details
	levelfile = tGameInfo.sMapFile;
	modfile = tGameInfo.sModDir;
	level = cMap->getName();
	mod = cGameScript.GetHeader()->ModName;
	xmlEntities(levelfile);
	xmlEntities(modfile);
	xmlEntities(level);
	xmlEntities(mod);

	// Save the game info
	data =	"<game datetime=\"" + tGameLog->sGameStart + "\" " +
			"length=\"" + ftoa(fGameOverTime - tGameLog->fGameStart) + "\" " +
			"loading=\"" + itoa(tGameInfo.iLoadingTimes) + "\" " +
			"lives=\"" + itoa(tGameInfo.iLives) + "\" " +
			"maxkills=\"" + itoa(tGameInfo.iKillLimit) + "\" " +
			"bonuses=\"" + (tGameInfo.bBonusesOn ? "1" : "0") + "\" " +
			"bonusnames=\"" + (tGameInfo.bShowBonusName ? "1" : "0") + "\" " +
			"levelfile=\"" + levelfile + "\" " +
			"modfile=\"" + modfile + "\" " +
			"level=\"" + level + "\" " +
			"mod=\"" + mod + "\" " +
			"winner=\"" + itoa(tGameLog->iWinner) + "\" " +
			"gamemode=\"" + itoa(tGameInfo.iGameMode) + "\">";

	// Count the number of players
	int num_players = 0;
	{for (short i=0; i < MAX_WORMS; i++)
		num_players += cRemoteWorms[i].isUsed() ? 1 : 0;
	}

	// Save the general players info
	data += "<players startcount=\"" + itoa(tGameLog->iNumWorms) + "\" endcount=\"" + itoa(num_players) + "\">";

	// Info for each player
	for (short i=0; i < tGameLog->iNumWorms; i++)  {

		// Replace the entities
		player = tGameLog->tWorms[i].sName;
		xmlEntities(player);

		// Replace the entities
		skin = tGameLog->tWorms[i].sSkin;
		xmlEntities(skin);

		// Write the info
		data += "<player name=\"" + player + "\" " +
				"skin=\"" + skin + "\" " +
				"id=\"" + itoa(tGameLog->tWorms[i].iID) + "\" "
				"kills=\"" + itoa(tGameLog->tWorms[i].iKills) + "\" " +
				"lives=\"" + itoa(tGameLog->tWorms[i].iLives) + "\" " +
				"suicides=\"" + itoa(tGameLog->tWorms[i].iSuicides) + "\" " +
				"teamkills=\"" + itoa(tGameLog->tWorms[i].iTeamKills) + "\" " +
				"teamdeaths=\"" + itoa(tGameLog->tWorms[i].iTeamDeaths) + "\" " +
				"team=\"" + itoa(tGameLog->tWorms[i].iTeam) + "\" " +
				"tag=\"" + (tGameLog->tWorms[i].bTagIT ? "1" : "0") + "\" " +
				"tagtime=\"" + ftoa(tGameLog->tWorms[i].fTagTime) + "\" " +
				"left=\"" + (tGameLog->tWorms[i].bLeft ? "1" : "0") + "\" " +
				"timeleft=\"" + ftoa(MAX(0.0f, tGameLog->tWorms[i].fTimeLeft - tGameLog->fGameStart)) + "\" " +
				"type=\"" + itoa(tGameLog->tWorms[i].iType) + "\"/>";
	}

	// End tags
	data += "</players>";
	data += "</game>";
}


//////////////////
// Returns the log worm with the specified id
log_worm_t *CClient::GetLogWorm(int id)
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

////////////////////////
// Select weapons for client handling local bots in net play
// Normally, this is done in CClient::Draw
void CClient::BotSelectWeapons(void)
{
	if(iNetStatus == NET_CONNECTED && bGameReady)  {
		uint i;

		// Go through and draw the first two worms select menus
		for(i=0;i<iNumWorms;i++) {
			// Select weapons
			cLocalWorms[i]->setWeaponsReady(true);
			cLocalWorms[i]->setCurrentWeapon(0);
		}

		// If we're ready, let the server know
		if(!bReadySent) {
			bReadySent = true;
			// TODO: move this out here
			CBytestream bytes;
			bytes.writeByte(C2S_IMREADY);
			bytes.writeByte(iNumWorms);

			// Send my worm's weapon details
			for(i=0;i<iNumWorms;i++)
				cLocalWorms[i]->writeWeapons(&bytes);

			cNetChan.AddReliablePacketToSend(bytes);
		}
	}
}

///////////////////
// Shutdown the log structure
void CClient::ShutdownLog(void)
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
		if(cMap && !bMapGrabbed) {
			cMap->Shutdown();
			delete cMap;
		}
		bMapGrabbed = false;
		cMap = NULL;
	}

	// Box buffer
	gfxFreeSurface(bmpBoxBuffer);
	bmpBoxBuffer = NULL;

	// Ingame score background
	gfxFreeSurface(bmpIngameScoreBg);
	bmpIngameScoreBg = NULL;

	// Bars
	if (cHealthBar1)
		delete cHealthBar1;
	if (cHealthBar2)
		delete cHealthBar2;
	if (cWeaponBar1)
		delete cWeaponBar1;
	if (cWeaponBar2)
		delete cWeaponBar2;

	cHealthBar1 = cHealthBar2 = cWeaponBar1 = cWeaponBar2 = NULL;

	// Shooting list
	cShootList.Shutdown();

	// Game menu
	cGameMenuLayout.Shutdown();

	// Scoreboard
	cScoreLayout.Shutdown();


	// Bonuses
	if(cBonuses) {
		delete[] cBonuses;
		cBonuses = NULL;
	}

	// Chatlist
	if (cChatList)  {
		((CListview *)cChatList)->Clear();
		((CListview *)cChatList)->Destroy();
		delete (CListview *)cChatList;
		cChatList = NULL;
	}

	// HINT: GameScript is shut down by the cache

	// Weapon restrictions
	cWeaponRestrictions.Shutdown();

	// Close the socket
	if(IsSocketStateValid(tSocket))
	{
		CloseSocket(tSocket);
	}
	InvalidateSocketState(tSocket);

	// Shutdown map downloads
	ShutdownDownloads();

	// Shutdown logging
	ShutdownLog();

	// Log this
	if (tLXOptions->bLogConvos)  {
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

void CClient::setClientVersion(const std::string & _s)
{
	//printf("CClient::setClientVersion(): %s\n", _s.c_str() );
	sClientVersion = _s;
	iHostOLXVer = 4;
	iClientOLXVer = 4;
}

void CClient::setServerVersion(const std::string & _s)
{
	//printf("CClient::setServerVersion(): %s\n", _s.c_str() );
	sServerVersion = _s;
	iHostOLXVer = 4;
	iClientOLXVer = 4;
}

