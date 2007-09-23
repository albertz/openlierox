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
#ifdef DEBUG
#include "MathLib.h"
#endif


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

	InvalidateSocketState(tSocket);

    cChatbox.setWidth(325);
	cChatbox.Clear();

	iLobbyReady = false;
	iGameReady = false;
	iReadySent = false;
	iGameOver = false;
	iGameMenu = false;
    bViewportMgr = false;
	tGameLobby.nSet = false;
	tGameLobby.nMaxWorms = MAX_PLAYERS;

	iBadConnection = false;
	iServerError = false;
    iClientError = false;
	iChat_Typing = false;
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

	cShootList.Shutdown();
	cGameScript.Shutdown();
	cWeaponRestrictions.Shutdown();

    for(i=0; i<NUM_VIEWPORTS; i++) {
        cViewports[i].setUsed(false);
        cViewports[i].setID(i);
		cViewports[i].SetWorldX(0);
		cViewports[i].SetWorldY(0);
    }    

	bHostOLXb4 = false;

	bDownloadingMap = false;
	cFileDownloader = NULL;
	sMapDownloadName = "";
	bMapDlError = false;
	sMapDlError = "";
	iMapDlProgress = 0;
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
	bUpdateScore = true;

	//fProjDrawTime = 0;
	//fProjSimulateTime = 0;

	iBadConnection = false;
	iServerError = false;
    iClientError = false;
	iChat_Typing = false;
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
		SetError("Error: Out of memory!\ncl::Initialize() %d",__LINE__);
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
    cShowScore.Setup(tLXOptions->sGeneralControls[SIN_SCORE]);
	cShowHealth.Setup(tLXOptions->sGeneralControls[SIN_HEALTH]);
	cShowSettings.Setup(tLXOptions->sGeneralControls[SIN_SETTINGS]);
	cViewportMgr.Setup(tLXOptions->sGeneralControls[SIN_VIEWPORTS]);
	cToggleTopBar.Setup(tLXOptions->sGeneralControls[SIN_TOGGLETOPBAR]);

	// Clear the frames


    // Initialize the weather
    //cWeather.Initialize(wth_snow);

	
	return true;
}

/////////////////
// Initialize map downloading
void CClient::InitializeDownloads()
{
	if (!cFileDownloader)
		cFileDownloader = new CFileDownloader();
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
	if (cFileDownloader)
		delete cFileDownloader;
	cFileDownloader = NULL;
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
	if (!cFileDownloader)  {
		sMapDlError = "Could not initialize the downloader";
		bMapDlError = true;
		return;
	}

	cFileDownloader->StartFileDownload(mapname, "levels");
	sMapDownloadName = mapname;
	bDownloadingMap = true;
}

/////////////////
// Process map downloading
void CClient::ProcessMapDownloads()
{
	// Nothing to process
	if (!bDownloadingMap || cFileDownloader == NULL)
		return;

	// Download finished
	if (cFileDownloader->IsFileDownloaded(sMapDownloadName))  {
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
					printf("WARNING: Finished map downloading but another map is already loaded. Deleting it.\n");
					delete cMap;
				}

				cMap = new CMap;
				if (cMap)  {
					if (!cMap->Load("levels/" + sMapDownloadName))  {  // Load the map
						// Weird
						printf("Could not load the downloaded map!\n");
						Disconnect();
						QuittoMenu();
					}
				}
			}
		} else {  // Inaccessible
			printf("Cannot access the downloaded map!\n");
			if (iNetStatus == NET_PLAYING)  {
				Disconnect();
				QuittoMenu();
			}
		}

		sMapDownloadName = "";

		return;
	}

	// Error check
	DownloadError error = cFileDownloader->FileDownloadError(sMapDownloadName);
	if (error.iError != FILEDL_ERROR_NO_ERROR)  {
		bDownloadingMap = false;
		sMapDlError = sMapDownloadName + " downloading error: " + error.sErrorMsg;
		sMapDownloadName = "";
		bMapDlError = true;
	}

	// Get the progress
	iMapDlProgress = cFileDownloader->GetFileProgress(sMapDownloadName);
}

///////////////////
// Main frame
void CClient::Frame(void)
{
	if(iNetStatus == NET_PLAYING) {
		iServerFrame++;
		fServerTime += tLX->fDeltaTime;
	}

    //static float oldtime =0;

	/*if(tLX->fCurTime - oldtime < 1.0/72.0)
		return;		// So we don't flood packets out to the clients
	else
		oldtime = tLX->fCurTime;*/

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

		// Check for connectionless packets (four leading 0xff's)
		if(bs.readInt(4) == -1) {
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
		iServerError = true;
		strServerErrorMsg = "Connection with server timed out";
		
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
	strServerAddr = address;
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
	if(GetNetAddrPort(&addr) == 0)  {
		if (tGameInfo.iGameType == GME_JOIN) // Remote joining
			SetNetAddrPort(&addr, LX_PORT);  // Try the default port if no port specified
		else // Host or local
			SetNetAddrPort(&addr, tLXOptions->iNetworkPort);  // Use the port specified in options
	}

	fConnectTime = tLX->fCurTime;
	iNumConnects++;

	// Request a challenge id
	CBytestream bs;
	bs.writeInt(-1,4);
	bs.writeString("lx::getchallenge");
	
	SetRemoteNetAddr(tSocket,&addr);
	bs.Send(tSocket);

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
	}

	// Two wormsize
	else  {
        cViewports[0].Setup(0, top, 318, h, type1);
        cViewports[0].setTarget(w1);
		cViewports[0].setUsed(true);

		cViewports[1].Setup(322, top, 318, h, type2);
        cViewports[1].setTarget(w2);
		cViewports[1].setUsed(true);
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
	if(iNetStatus == NET_CONNECTED && iGameReady)  {
		uint i;
		
		// Go through and draw the first two worms select menus
		for(i=0;i<iNumWorms;i++) {
			// Select weapons
			cLocalWorms[i]->setWeaponsReady(true);
			cLocalWorms[i]->setCurrentWeapon(0);
		}

		// If we're ready, let the server know
		if(!iReadySent) {
			iReadySent = true;
			CBytestream *bytes = cNetChan.getMessageBS();
			bytes->writeByte(C2S_IMREADY);
			bytes->writeByte(iNumWorms);

			// Send my worm's weapon details
			for(i=0;i<iNumWorms;i++)
				cLocalWorms[i]->writeWeapons(bytes);
		}
	}
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
	if (bmpBoxBuffer)  {
		SDL_FreeSurface(bmpBoxBuffer);
		bmpBoxBuffer = NULL;
	}

	// Ingame score background
	if (bmpIngameScoreBg)  {
		SDL_FreeSurface(bmpIngameScoreBg);
		bmpIngameScoreBg = NULL;
	}

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

	// Gamescript
	cGameScript.Shutdown();

	// Weapon restrictions
	cWeaponRestrictions.Shutdown();
	
	// Close the socket
	if(IsSocketStateValid(tSocket))
		CloseSocket(tSocket);
	InvalidateSocketState(tSocket);

	// Shutdown map downloads
	ShutdownDownloads();

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
