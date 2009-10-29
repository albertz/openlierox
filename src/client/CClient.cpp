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
#include "ProfileSystem.h"
#include "CClient.h"
#include "CBonus.h"
#include "DeprecatedGUI/Menu.h"
#include "console.h"
#include "FindFile.h"
#include "DeprecatedGUI/Graphics.h"
#include "DeprecatedGUI/CBar.h"
#include "CWorm.h"
#include "Error.h"
#include "Protocol.h"
#include "StringUtils.h"
#include "MathLib.h"
#include "EndianSwap.h"
#include "Version.h"
#include "CServer.h"
#include "AuxLib.h"
#include "Networking.h"
#include "Timer.h"
#include "XMLutils.h"
#include "CClientNetEngine.h"
#include "DeprecatedGUI/CBrowser.h"
#include "CChannel.h"
#include "IRC.h"
#include "NotifyUser.h"
#include "CWormHuman.h"
#include "Debug.h"
#include "ConversationLogger.h"
#include "CServerConnection.h"
#include "CServerNetEngine.h"
#include "FlagInfo.h"
#include "CMap.h"
#include "DedicatedControl.h"
#include "Command.h"

#include <zip.h> // For unzipping downloaded mod





///////////////////
// Clear the client details
void CClient::Clear()
{

#ifdef DEBUG
	if (cRemoteWorms || cBonuses)  {
#ifdef _MSC_VER
		__asm int 3; // Breakpoint
#endif
		warnings << "clearing a client that wasn't shut down! This will cause a memleak!" << endl;
	}
#endif

	tGameInfo = tLXOptions->tGameInfo; // TODO: is it ok to copy the serverside features? shouldn't we perhaps set some initial GameInfo?
	tGameInfo.fTimeLimit = -100;
	otherGameInfo.clear();
	iNumWorms = 0;
	for(int i=0;i<MAX_PLAYERS;i++)
	{
		cLocalWorms[i] = NULL;
		tProfiles[i] = NULL;
	}
	cRemoteWorms = NULL;
	cProjectiles.clear();
	projPosMap.clear();
	cMap = NULL;
	bMapGrabbed = false;
	if( cNetChan )
		delete cNetChan;
		cNetChan = NULL;
		iNetStatus 	 = NET_DISCONNECTED;	
	reconnectingAmount = 0;
	bsUnreliable.Clear();
	iChat_Numlines = 0;
	iScorePlayers = 0;
	cBonuses = NULL;
	bUpdateScore = true;
	fLastScoreUpdate = AbsTime();
	cChatList = NULL;
	bmpIngameScoreBg = NULL;
	bCurrentSettings = false;
	bLocalClient = false;
	fServertime = TimeDiff();
	
	tGameLog = NULL;
	iLastVictim = -1;
	iLastKiller = -1;

	tSocket = new NetworkSocket();
	SetNetAddrValid( cServerAddr, false );

	cChatbox.Clear();

	bGameReady = false;
	bGameRunning = false;
	bReadySent = false;
	bGameOver = false;
	bGameMenu = false;
    bViewportMgr = false;
	//tGameLobby.bSet = false;
	//tGameLobby.nMaxWorms = MAX_PLAYERS;

	bBadConnection = false;
	bServerError = false;
    bClientError = false;
	bChat_Typing = false;
	fLastReceived = AbsTime::Max();
	fSendWait = 0;
	fLastUpdateSent = AbsTime();


	fMyPingRefreshed = 0;
	iMyPing = 0;
	fMyPingSent = 0;


	// HINT: gamescript is shut down by the cache

	cShootList.Shutdown();
	cWeaponRestrictions.Shutdown();

    for(int i=0; i<NUM_VIEWPORTS; i++) {
        cViewports[i].setUsed(false);
		cViewports[i].setTarget(NULL);
        cViewports[i].setID(i);
		cViewports[i].SetWorldX(0);
		cViewports[i].SetWorldY(0);
    }

	cServerVersion.reset();
	bHostAllowsStrafing = false;

	bDownloadingMap = false;
	bDownloadingMod = false;
	cHttpDownloader = NULL;
	sMapDownloadName = "";
	bDlError = false;
	sDlError = "";
	iDlProgress = 0;
	fLastFileRequest = fLastFileRequestPacketReceived = tLX->currentTime;
	getUdpFileDownloader()->reset();
	fSpectatorViewportMsgTimeout = tLX->currentTime;
	sSpectatorViewportMsg = "";
	bWaitingForMap = false;
	bWaitingForMod = false;
	bHaveMap = false;
	bHaveMod = false;
}


///////////////////
// Clear the client for another game
void CClient::MinorClear()
{
	iNetStatus = NET_CONNECTED;
	bGameReady = false;
	bGameRunning = false;
	bReadySent = false;
	bGameOver = false;
	bGameMenu = false;
    bViewportMgr = false;
	bUpdateScore = true;
	fLastScoreUpdate = AbsTime();
	bCurrentSettings = false;
	bWaitingForMap = false;
	bWaitingForMod = false;

	//fProjDrawTime = 0;
	//fProjSimulateTime = 0;

	iLastVictim = -1;
	iLastKiller = -1;

	bBadConnection = false;
	bServerError = false;
    bClientError = false;
	bChat_Typing = false;
	fLastReceived = AbsTime::Max();
	fServertime = TimeDiff();

	fSendWait = 0;

	iChat_Numlines = 0;
	if(!bDedicated)
		cChatList->InitializeChatBox();

	int i;
	for(i=0; i<MAX_WORMS; i++)  {
		cRemoteWorms[i].resetAngleAndDir();
		cRemoteWorms[i].Unprepare();
	}

	cProjectiles.clear();
	projPosMap.clear();

	for(i=0; i<MAX_BONUSES; i++)
		cBonuses[i].setUsed(false);

    for(i=0; i<NUM_VIEWPORTS; i++)  {
        cViewports[i].setUsed(false);
		cViewports[i].setTarget(NULL);
		cViewports[i].SetWorldX(0);
		cViewports[i].SetWorldY(0);
	}
	fLastFileRequest = fLastFileRequestPacketReceived = tLX->currentTime;
	getUdpFileDownloader()->reset();
	fSpectatorViewportMsgTimeout = tLX->currentTime;
	sSpectatorViewportMsg = "";
	
	if(m_flagInfo)
		m_flagInfo->reset();
	
	for(int i = 0; i < 4; ++i)
		iTeamScores[i] = 0;
}

/*
void CClient::ReinitLocalWorms() {
	// Initialize the local worms
	iNumWorms = tGameInfo.iNumPlayers;

	for(uint i=0;i<iNumWorms;i++) {
		cLocalWorms[i] = NULL;
		tProfiles[i] = tGameInfo.cPlayers[i];
	}
}
*/
CClient::CClient() {
	// TODO: merge this with Clear()
	//notes << "cl:Constructor" << endl;
	cRemoteWorms = NULL;
	cMap = NULL;
	m_flagInfo = NULL;
	cBonuses = NULL;
	bmpBoxBuffer = NULL;
	bmpBoxLeft = NULL;
	bmpBoxRight = NULL;
	bmpIngameScoreBg = NULL;
	cHealthBar1 = NULL;
	cHealthBar2 = NULL;
	cWeaponBar1 = NULL;
	cWeaponBar2 = NULL;
	cDownloadBar = NULL;
	bGameReady = false;
	bMapGrabbed = false;
	cChatList = NULL;
	bUpdateScore = true;
	fLastScoreUpdate = AbsTime();
	bShouldRepaintInfo = true;
	bCurrentSettings = false;
	tMapDlCallback = NULL;
	tModDlCallback = NULL;
	
	tGameLog = NULL;
	iLastVictim = -1;
	iLastKiller = -1;
	
	szServerName="";
	
	cNetEngine = new CClientNetEngine(this);
	cNetChan = NULL;
	iNetStatus = NET_DISCONNECTED;
	bsUnreliable.Clear();
	bBadConnection = false;
	bServerError = false;
	bChat_Typing = false;
	bTeamChat = false;
	fChat_BlinkTime = 0;
	bChat_CursorVisible = true;
	bClientError = false;
	iNetSpeed = 3;
	fLastUpdateSent = AbsTime();
	tSocket = new NetworkSocket();
	SetNetAddrValid( cServerAddr, false );
	bLocalClient = false;
	
	iMyPing = 0;
	fMyPingRefreshed = 0;
	fMyPingSent = 0;
	
	//fProjDrawTime = 0;
	//fProjSimulateTime = 0;
	
	fSendWait = 0;
	
	bMuted = false;
	bRepaintChatbox = true;
		
	fLastFileRequest = tLX->currentTime;
	
	bDownloadingMap = false;
	cHttpDownloader = NULL;
	sMapDownloadName = "";
	bDlError = false;
	sDlError = "";
	iDlProgress = 0;
	
	Clear();
}

CClient::~CClient() {
	Shutdown();
	Clear();
	if(cNetEngine) 
		delete cNetEngine;
}

int	CClient::getPing() { return cNetChan->getPing(); }
void CClient::setPing(int _p) { cNetChan->setPing(_p); }

///////////////////
// Initialize the client
int CClient::Initialize()
{
	uint i;

	// Shutdown & clear any old client data
	Shutdown();
	Clear();

	iNetSpeed = tLXOptions->iNetworkSpeed;

	// Local/host games use instant speed
	if(tLX->iGameType != GME_JOIN)
		iNetSpeed = NST_LOCAL;


	//ReinitLocalWorms();
	
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
		cRemoteWorms[i].setTagTime(TimeDiff(0));
		cRemoteWorms[i].setTeam(0);
		cRemoteWorms[i].setUsed(false);
		cRemoteWorms[i].setClient(NULL); // Local worms won't get server connection owner
	}

	// Set our version to the current game version
	// HINT: this function is called only for the local client, not for server's client
	setClientVersion(GetGameVersion());

	// Initialize the bonuses
	cBonuses = new CBonus[MAX_BONUSES];
	if(cBonuses == NULL) {
		SetError("Error: Out of memory!\ncl::Initialize() " + itoa(__LINE__));
		return false;
	}


	// Open a new socket
	if( tLX->iGameType == GME_JOIN ) {
		tSocket->OpenUnreliable( tLXOptions->iNetworkPort );	// Open socket on port from options in hopes that user forwarded that port on router
	}
	if(!tSocket->isOpen()) {	// If socket won't open that's not error - open another one on random port
		tSocket->OpenUnreliable(0);
	}
	if(!tSocket->isOpen()) {
		SetError("Error: Could not open UDP socket!");
		return false;
	}

	// Initialize the drawing
	if(!InitializeDrawing())
		return false;

	if(bDedicated)
		cChatList = NULL;
	else {
		// Initialize chat box (must be after drawing because of interface settings)
		cChatList = new DeprecatedGUI::CBrowser;
		if (!cChatList)
			return false;
		cChatList->InitializeChatBox();
		cChatList->Setup(0,	tInterfaceSettings.ChatBoxX,
							tInterfaceSettings.ChatBoxY,
							tInterfaceSettings.ChatBoxW, // A little hack because of replacing CListview with CBrowser to put the scrollbar on the correct place
							tInterfaceSettings.ChatBoxH);
	}
	// Clear the network channel
	//cNetChan.Clear();

	// Initialize the shooting list
	cShootList.Initialize();

	this->SetupGameInputs();
	
    // Initialize the weather
    //cWeather.Initialize(wth_snow);

	m_flagInfo = new FlagInfo();
	
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
		errors("Out of memory while allocating log\n");
		return;
	}
	tGameLog->tWorms = NULL;
	tGameLog->fGameStart = tLX->currentTime;
	tGameLog->iNumWorms = num_players;
	tGameLog->iWinner = -1;
	tGameLog->sGameStart = GetDateTime();
	tGameLog->sServerName = szServerName;
	NetAddrToString(cNetChan->getAddress(), tGameLog->sServerIP);

	// Allocate log worms
	int i;
	tGameLog->tWorms = new log_worm_t[num_players];
	if (!tGameLog->tWorms)
		return;

	// Initialize the log worms
	int j = 0;
	for (i=0; i < MAX_WORMS; i++)  {
		if (cRemoteWorms[i].isUsed())  {
			tGameLog->tWorms[j].bLeft = false;
			tGameLog->tWorms[j].iID = cRemoteWorms[i].getID();
			tGameLog->tWorms[j].sName = cRemoteWorms[i].getName();
			tGameLog->tWorms[j].sSkin = cRemoteWorms[i].getSkin().getFileName();
			tGameLog->tWorms[j].iKills = 0;
			tGameLog->tWorms[j].iLives = tGameInfo.iLives;
			tGameLog->tWorms[j].iSuicides = 0;
			tGameLog->tWorms[j].iTeamKills = 0;
			tGameLog->tWorms[j].iTeamDeaths = 0;
			tGameLog->tWorms[j].bTagIT = false;
			if (tGameInfo.iGeneralGameType == GMT_TEAMS)
				tGameLog->tWorms[j].iTeam = cRemoteWorms[i].getTeam();
			else
				tGameLog->tWorms[j].iTeam = -1;
			tGameLog->tWorms[j].fTagTime = 0.0f;
			tGameLog->tWorms[j].fTimeLeft = 0.0f;
			tGameLog->tWorms[j].iType = cRemoteWorms[i].getType()->toInt();
			j++;

			if (j >= num_players)
				break;
		}
	}
}


///////////////////////
// Called when an IRC message arrives
void CClient::IRC_OnNewMessage(const std::string &msg, int type)
{
	//DeprecatedGUI::CBrowser *brw = cClient ? cClient->cChatList : NULL;
	if (!cClient || !GetGlobalIRC())
		return;

	// Ignore any messages that do not contain user's nick (we don't want spammy junk ingame)
	if (stringtolower(msg).find(stringtolower(GetGlobalIRC()->getNick())) == std::string::npos &&
		type != IRCClient::IRC_TEXT_PRIVATE)
		return;

	// Add the message
	switch (type)  {
	case IRCClient::IRC_TEXT_CHAT:
		cClient->getChatbox()->AddText("<b>IRC:</b> " + msg, tLX->clChatText, TXT_CHAT, tLX->currentTime);
		break;
	case IRCClient::IRC_TEXT_NOTICE:
		cClient->getChatbox()->AddText("<b>IRC:</b> " + msg, tLX->clNotice, TXT_NOTICE, tLX->currentTime);
		break;
	case IRCClient::IRC_TEXT_ACTION:
		cClient->getChatbox()->AddText("<b>IRC:</b> " + msg, tLX->clNetworkText, TXT_CHAT, tLX->currentTime);
		break;
	case IRCClient::IRC_TEXT_PRIVATE:
		cClient->getChatbox()->AddText("<b>IRC:</b> " + msg, tLX->clNetworkText, TXT_PRIVATE, tLX->currentTime);
		break;
	default:
		cClient->getChatbox()->AddText("<b>IRC:</b> " + msg, tLX->clChatText, TXT_CHAT, tLX->currentTime);
	}


	// Notify the user if the message contains his nick
	NotifyUserOnEvent();
}

/////////////////////////
// Called when the IRC is disconnected
void CClient::IRC_OnDisconnect()
{
	// TODO: notify the user
}

/////////////////
// Initialize map downloading
void CClient::InitializeDownloads()
{
	if (!cHttpDownloader)
		cHttpDownloader = new CHttpDownloadManager();
	bDownloadingMap = false;
	bDownloadingMod = false;
	bDlError = false;
	iDlProgress = 0;
	sDlError = "";
	sMapDownloadName = "";
	sModDownloadName = "";
}

/////////////////
// Shutdown map downloading
void CClient::ShutdownDownloads()
{
	if (cHttpDownloader)
		delete cHttpDownloader;
	cHttpDownloader = NULL;
	bDownloadingMap = false;
	bDownloadingMod = false;
	bDlError = false;
	iDlProgress = 0;
	sDlError = "";
	sMapDownloadName = "";
	sModDownloadName = "";
}

/////////////////
// Download a map
void CClient::DownloadMap(const std::string& mapname)
{
	notes << "CClient::DownloadMap() " << mapname << endl;

	// Check
	if (!cHttpDownloader)  {
		sDlError = "Could not initialize the downloader";
		bDlError = true;
		return;
	}

	cHttpDownloader->StartFileDownload(mapname, "levels");
	sMapDownloadName = mapname;
	bDownloadingMap = true;
	iDownloadMethod = DL_HTTP; // We start with HTTP

	setLastFileRequest( AbsTime() ); // Re-enable file requests, if previous request failed
}

///////////////////
// Download a mod
void CClient::DownloadMod(const std::string &modname)
{
	if (modname.size() == 0)
		return;

	// Check
	if (!cHttpDownloader)  {
		sDlError = "Could not initialize the downloader";
		bDlError = true;
		return;
	}

	sModDownloadName = modname;
	cHttpDownloader->StartFileDownload(sModDownloadName + ".zip", "./");
	bDownloadingMod = true;
	iDownloadMethod = DL_HTTP; // We start with HTTP

	iModDownloadingSize = 0;
	setLastFileRequest( AbsTime() ); // Re-enable file requests, if previous request failed
}

///////////////////
// Abort any map downloading
void CClient::AbortDownloads()
{
	if (bDownloadingMap)  {
		bDownloadingMap = false;
		if (cHttpDownloader)
			cHttpDownloader->CancelFileDownload(sMapDownloadName);
		getUdpFileDownloader()->removeFileFromRequest("levels/" + sMapDownloadName);
	}

	if (bDownloadingMod)  {
		bDownloadingMod = false;
		if (cHttpDownloader)
			cHttpDownloader->CancelFileDownload(sModDownloadName + ".zip");
	}

	// Cancel any UDP downloads
	if (getUdpFileDownloader()->getFilesPendingAmount() > 0 || getUdpFileDownloader()->isReceiving())  {
		getUdpFileDownloader()->abortDownload();

		cNetEngine->SendFileData();
	}

	if (tModDlCallback)
		tModDlCallback();
	if (tMapDlCallback)
		tMapDlCallback();

	// Disable file download for current session, if server sent ABORT do not try to re-download
	setLastFileRequest( tLX->currentTime + 10000.0f ); 

	InitializeDownloads();
}

////////////////
// Finish downloading of the map
void CClient::FinishMapDownloads()
{
	// Check that the file exists and is ok
	std::string levelname = CMap::GetLevelName(sMapDownloadName);
	if (levelname != "")  {
		if (tGameInfo.sMapFile == sMapDownloadName)  {
			bHaveMap = true;
			tGameInfo.sMapName = levelname;
			if (tMapDlCallback)
				tMapDlCallback();
		}

		// If downloaded via HTTP, don't try UDP
		if (iDownloadMethod == DL_HTTP)
			getUdpFileDownloader()->removeFileFromRequest("levels/" + sMapDownloadName);

		if (cHttpDownloader)
			cHttpDownloader->RemoveFileDownload(sMapDownloadName);

		// If playing, load the map
		if (iNetStatus == NET_PLAYING || (iNetStatus == NET_CONNECTED && bWaitingForMap))  {
			if (cMap && cMap->getCreated())  {
				hints << "Finished map downloading but another map is already loaded." << endl;
				return;
			}

			if (!cMap)  {
				warnings << "In game and cMap is not allocated." << endl;
				cMap = new CMap;
			}

			if (!cMap->Load("levels/" + sMapDownloadName))  {  // Load the map
				// Weird
				errors << "Could not load the downloaded map!" << endl;
				Disconnect();
				GotoNetMenu();
			}
		}
	}
}

////////////////////
// Finish downloading of a mod
void CClient::FinishModDownloads()
{
	if ( !IsFileAvailable(sModDownloadName + "/script.lgs", false) && 
			IsFileAvailable(sModDownloadName + ".zip", false) )
	{
		// Unzip mod file
		std::string fname;
		GetExactFileName( sModDownloadName + ".zip", fname );
		fname = GetFullFileName( fname );

		zip * zipfile = zip_open( Utf8ToSystemNative(fname).c_str(), 0, NULL );
		if( zipfile == NULL ) {
			warnings("Cannot access the downloaded mod!\n");
			if (iNetStatus == NET_PLAYING || (iNetStatus == NET_CONNECTED && bWaitingForMod))  {
				Disconnect();
				GotoNetMenu();
			}
			return;
		}

		if( zip_name_locate(zipfile, (sModDownloadName + "/script.lgs").c_str(), ZIP_FL_NOCASE) == -1 )
		{
			warnings("Cannot access the downloaded mod!\n");
			if (iNetStatus == NET_PLAYING || (iNetStatus == NET_CONNECTED && bWaitingForMod))  {
				Disconnect();
				GotoNetMenu();
			}
			return;
		}
		
		for( int f = 0; f < zip_get_num_files(zipfile); f++ )
		{
			const char * fname = zip_get_name(zipfile, f, 0);
			// Check if file is valid and is inside mod dir and not already exist
			if( fname == NULL || std::string(fname).find("..") != std::string::npos ||
				stringtolower( fname ).find( stringtolower(sModDownloadName) ) != 0 ||
				IsFileAvailable(fname, false) )
				continue;
			zip_file * fileInZip = zip_fopen_index(zipfile, f, 0);
			FILE * fileWrite = OpenGameFile(fname, "wb");
			if( fileInZip == NULL || fileWrite == NULL )
				continue;
			char buf[4096];
			int readed;
			while( ( readed = zip_fread(fileInZip, buf, sizeof(buf)) ) > 0 )
				fwrite( buf, 1, readed, fileWrite );
			fclose(fileWrite);
			zip_fclose(fileInZip);
		};
		
		zip_close(zipfile);
	}

	// Check that the script.lgs file is available
	if (!IsFileAvailable(sModDownloadName + "/script.lgs", false))  {
		warnings("Cannot access the downloaded mod!\n");

		if (iNetStatus == NET_PLAYING || (iNetStatus == NET_CONNECTED && bWaitingForMod))  {
			Disconnect();
			GotoNetMenu();
		}

		return;
	}

	// Update the lobby
	if (tGameInfo.sModDir == sModDownloadName)  {
		bDownloadingMod = false;
		bHaveMod = true;
		if (tModDlCallback)
			tModDlCallback();
	}

	// Load the mod if playing
	if (iNetStatus == NET_PLAYING || (iNetStatus == NET_CONNECTED && bWaitingForMod))  {
		bWaitingForMod = false;

		if (cGameScript->GetNumWeapons() > 0)  {
			hints("Finished downloading the mod but another mod is already loaded.\n");
			return;
		}

		if (stringcaseequal(tGameInfo.sModName, sModDownloadName))  {
			if (cGameScript->Load(sModDownloadName) != GSE_OK)  {
				errors("Could not load the downloaded mod.\n");
				Disconnect();
				GotoNetMenu();
				return;
			}
			bWaitingForMod = false;
			
			// Initialize the weapon selection
			for (size_t i = 0; i < iNumWorms; i++)
				cLocalWorms[i]->initWeaponSelection();
		} else {
			warnings("The downloaded mod is not the one we are waiting for.\n");
			Disconnect();
			GotoNetMenu();
			return;
		}
	}
}

const float fDownloadRetryTimeout = 4.0;	// Server should calculate CRC for large amount of files, like mod dir

void CClient::ProcessUdpUploads()
{
	// Server requested some file (CRC check on our map) or we're sending file request
	if( getUdpFileDownloader()->isSending() ) 
	{
		cNetEngine->SendFileData();
		
		fLastFileRequestPacketReceived = tLX->currentTime;
		fLastFileRequest = tLX->currentTime + fDownloadRetryTimeout/10.0f;
		// Do not spam server with STAT packets, it may take long to scan all files in mod dir
		if( getUdpFileDownloader()->getFilename() == "STAT:" || getUdpFileDownloader()->getFilename() == "GET:" )
			fLastFileRequest = tLX->currentTime + fDownloadRetryTimeout;
	}
}

/////////////////
// Process map downloading
void CClient::ProcessMapDownloads()
{
	if( bDownloadingMap && iDownloadMethod == DL_HTTP && cHttpDownloader )  {

		// Download finished
		if (cHttpDownloader->IsFileDownloaded(sMapDownloadName))  {
			bDownloadingMap = false;
			bDlError = false;
			sDlError = "";
			iDlProgress = 100;

			FinishMapDownloads();;

			sMapDownloadName = "";
			bWaitingForMap = false;

			return;
		}

		// Error check
		DownloadError error = cHttpDownloader->FileDownloadError(sMapDownloadName);
		if (error.iError != FILEDL_ERROR_NO_ERROR)  {
			sDlError = sMapDownloadName + " downloading error: " + error.sErrorMsg;

			if (bWaitingForMap)  {
				Disconnect();
				GotoNetMenu();
				return;
			}

			// HTTP failed, let's try UDP
			if( getServerVersion() > OLXBetaVersion(4) )
			{
				iDownloadMethod = DL_UDP;
				bDownloadingMap = true;
				iDlProgress = 0;
				cHttpDownloader->RemoveFileDownload(sMapDownloadName);

				// Request file downloading with UDP - it won't start until HTTP won't finish
				getUdpFileDownloader()->clearAborted();
				getUdpFileDownloader()->requestFile("levels/" + sMapDownloadName, true);
				getUdpFileDownloader()->requestFileInfo("levels/" + sMapDownloadName, true); // To get valid progressbar

			} else {
				bDownloadingMap = false;
				getUdpFileDownloader()->removeFileFromRequest("levels/" + sMapDownloadName);
				sMapDownloadName = "";
				if (tMapDlCallback)
					tMapDlCallback();
				return;
			}
		} else {  // No error

			// Get the progress
			iDlProgress = cHttpDownloader->GetFileProgress(sMapDownloadName);
			return; // Skip UDP downloading if HTTP downloading active
		}
	}

  	// UDP file download used for maps and mods - we can download map via HTTP and mod via UDP from host
	if( getServerVersion() < OLXBetaVersion(4) || iNetStatus == NET_DISCONNECTED )  {
		if (tMapDlCallback && bDownloadingMap)  {
			bDownloadingMap = false;
			tMapDlCallback();
		}
		bDownloadingMap = false;
		sMapDownloadName = "";
		return;
	}

	// If not downloading the map, quit
	if (!bDownloadingMap)
		return;

	if( getUdpFileDownloader()->isReceiving() )	 {
		if( fLastFileRequestPacketReceived + fDownloadRetryTimeout < tLX->currentTime ) { // Server stopped sending file in the middle
			fLastFileRequestPacketReceived = tLX->currentTime;
			if( ! getUdpFileDownloader()->requestFilesPending() )  { // More files to receive
				bDlError = true;
				sDlError = sMapDownloadName + " downloading error: UDP timeout";
				bDownloadingMap = false;

				if (tMapDlCallback)
					tMapDlCallback();

				// If waiting for the map ingame, just quit
				if (bWaitingForMap)  {
					Disconnect();
					GotoNetMenu();
					return;
				}
			}
		}
		
		if( getUdpFileDownloader()->getFilename() == "levels/" + getGameLobby()->sMapFile ) {
			bDownloadingMap = true;
			iDownloadMethod = DL_UDP;
		}

		iDlProgress = (int)(getUdpFileDownloader()->getFileDownloadingProgress() * 100.0f);

		return;
	}

	// Download finished
	if( getUdpFileDownloader()->wasError() && bDownloadingMap )  {
		getUdpFileDownloader()->clearError();
		bDlError = true;
		sDlError = sMapDownloadName + " downloading error: checksum failed";
		bDownloadingMap = false;
		if (tMapDlCallback)
			tMapDlCallback();
	}

	if( getUdpFileDownloader()->wasAborted() && bDownloadingMap )  {
		getUdpFileDownloader()->clearAborted();
		bDlError = true;
		sDlError = sMapDownloadName + " downloading error: server aborted download";
		bDownloadingMap = false;
		if (tMapDlCallback)
			tMapDlCallback();
	}

	// HINT: gets finished in CClient::ParseSendFile

	if( fLastFileRequest > tLX->currentTime )
		return;

	fLastFileRequestPacketReceived = tLX->currentTime;
	fLastFileRequest = tLX->currentTime + fDownloadRetryTimeout/10.0f; // Request another file from server after little timeout

	getUdpFileDownloader()->requestFilesPending(); // More files to receive?
}

/////////////////////
// Process mod downloading
void CClient::ProcessModDownloads()
{
	if( bDownloadingMod && iDownloadMethod == DL_HTTP && cHttpDownloader )  {

		// Download finished
		if (cHttpDownloader->IsFileDownloaded(sModDownloadName + ".zip"))  {
			bDownloadingMod = false;
			bDlError = false;
			sDlError = "";
			iDlProgress = 100;

			FinishModDownloads();;

			sModDownloadName = "";
			bWaitingForMod = false;

			return;
		}

		// Error check
		DownloadError error = cHttpDownloader->FileDownloadError(sModDownloadName + ".zip");
		if (error.iError != FILEDL_ERROR_NO_ERROR)  {
			sDlError = sModDownloadName + ".zip" + " downloading error: " + error.sErrorMsg;

			if (bWaitingForMod)  {
				Disconnect();
				GotoNetMenu();
				return;
			}

			// HTTP failed, let's try UDP
			if( getServerVersion() > OLXBetaVersion(4) )
			{
				iDownloadMethod = DL_UDP;
				bDownloadingMod = true;
				iDlProgress = 0;
				cHttpDownloader->RemoveFileDownload(sModDownloadName + ".zip");
				
				// Request file downloading with UDP - it won't start until HTTP won't finish
				getUdpFileDownloader()->requestFileInfo(sModDownloadName, true); // To get valid progressbar

			} else {
				bDownloadingMod = false;
				sModDownloadName = "";
				if (tModDlCallback)
					tModDlCallback();
				return;
			}
		} else {  // No error

			// Get the progress
			iDlProgress = cHttpDownloader->GetFileProgress(sMapDownloadName);
			return; // Skip UDP downloading if HTTP downloading active
		}
	}

	if (!bDownloadingMod)
		return;

  	// Can we download anything at all?
	if( getServerVersion() < OLXBetaVersion(4) || iNetStatus == NET_DISCONNECTED )  {
		if (tModDlCallback)  {
			bDownloadingMod = false;
			tModDlCallback();
		}
		bDownloadingMod = false;
		sModDownloadName = "";
		return;
	}

	// HINT: the downloading gets finished in CClient::ParseSendFile

	// Aborted?
	if (cUdpFileDownloader.wasAborted())  {
		hints << "Mod downloading aborted." << endl;
		bDownloadingMod = false;
		sModDownloadName = "";

		if (tModDlCallback)
			tModDlCallback();

		if (bWaitingForMod)  {
			Disconnect();
			GotoNetMenu();
			return;
		}
	}

	// Update download progress
	if( iModDownloadingSize == 0 )
		iDlProgress = byte(cUdpFileDownloader.getFileDownloadingProgress() * 100);
	bool downloadStarted = ( cUdpFileDownloader.getFilename() != "" && cUdpFileDownloader.getFilename() != "STAT:" );
	iDlProgress = byte( ( downloadStarted ? 5.0f : 0.0f ) + 95.0f - 95.0f / iModDownloadingSize * 
					(cUdpFileDownloader.getFilesPendingSize() - cUdpFileDownloader.getFileDownloadingProgressBytes()) );
	
	// Receiving
	if(cUdpFileDownloader.isReceiving())	 {
		if( fLastFileRequestPacketReceived + fDownloadRetryTimeout < tLX->currentTime ) { // Server stopped sending file in the middle
			fLastFileRequestPacketReceived = tLX->currentTime;
			if(!cUdpFileDownloader.requestFilesPending())  { // More files to receive
				bDownloadingMod = false;
				errors << "Mod download error: connection timeout" << endl;

				if (tModDlCallback)
					tModDlCallback();

				// If waiting for the map ingame, just quit
				if (bWaitingForMod)  {
					Disconnect();
					GotoNetMenu();
					return;
				}
			}
		}

		return;
	}

	// Request another file from server after little timeout
	if( fLastFileRequest <= tLX->currentTime )  {

		// Re-request the file if the request failed
		if (cUdpFileDownloader.getFilesPendingAmount() == 0)  {
			cUdpFileDownloader.requestFileInfo(sModDownloadName, true);
		}

		fLastFileRequestPacketReceived = tLX->currentTime;
		fLastFileRequest = tLX->currentTime + fDownloadRetryTimeout/10.0f;

		getUdpFileDownloader()->requestFilesPending(); // More files to receive?
	}
}

///////////////////
// Main frame
void CClient::Frame()
{
	if(bGameRunning) {
		fServertime += tLX->fRealDeltaTime;
	}

	ReadPackets();

	ProcessMapDownloads();
	ProcessModDownloads();
	ProcessUdpUploads();

	SimulateHud();

	// could be that some console command wants to quit
	if(!tLX || tLX->bQuitEngine || tLX->bQuitGame)
		return;
	
	if((bGameRunning || iNetStatus == NET_PLAYING) && !bWaitingForMap && !bWaitingForMod && cMap && cGameScript.get())
	{
		if( NewNet::Active() )
			NewNet_Frame();
		else
			Simulation();
	}

	SendPackets();

	// Connecting process
	if (bConnectingBehindNat)
		ConnectingBehindNAT();
	else
		Connecting();
}

void CClient::NewNet_Frame()
{
	CBytestream out, packet;
	if( getNumWorms() <= 0 )
		return;
	out.writeByte( C2S_NEWNET_KEYS );
	out.writeByte( getWorm(0)->getID() );
	while( NewNet::Frame(&out) )
	{
		packet.Append(&out);
		out.Clear();
		out.writeByte( C2S_NEWNET_KEYS );
		out.writeByte( getWorm(0)->getID() );
	}
	
	if( NewNet::ChecksumRecalculated() )
		getNetEngine()->SendNewNetChecksum();

	cNetChan->AddReliablePacketToSend(packet);
}

///////////////////
// Read the packets
bool CClient::ReadPackets()
{	
	CBytestream		bs;
	bool anythingNew = false;
	
	while(bs.Read(tSocket)) {
		anythingNew = true;
		
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
				cNetEngine->ParseConnectionlessPacket(&bs);
			continue;
		}
		bs.ResetPosToBegin();

		if(iNetStatus == NET_DISCONNECTED || iNetStatus == NET_CONNECTING)
			continue;

		if(cNetChan == NULL) {
			errors << "Client::ReadPackets: cNetChan is unset!" << endl;
			continue;
		}		
		
		// Parse the packet - process continuously in case we've received multiple logical packets on new CChannel
		while( cNetChan->Process(&bs) )
		{
			while( cNetEngine->ParsePacket(&bs) ) { }
			bs.Clear();
		}
	}

	// Check if our connection with the server timed out
	if(iNetStatus == NET_PLAYING && cNetChan->getLastReceived() + TimeDiff((float)LX_CLTIMEOUT) < tLX->currentTime && tLX->iGameType == GME_JOIN) {
		// AbsTime out
		bServerError = true;
		strServerErrorMsg = "Connection with server timed out";

		// Stop any file downloads
		if (bDownloadingMap && cHttpDownloader)
			cHttpDownloader->CancelFileDownload(sMapDownloadName);
		getUdpFileDownloader()->reset();

		if( NewNet::Active() )
			NewNet::EndRound();

		// The next frame will pickup the server error flag set & handle the msgbox, disconnecting & quiting
	}
	
	return anythingNew;
}


///////////////////
// Send the packets
void CClient::SendPackets(bool sendPendingOnly)
{
	if(!sendPendingOnly) {
		// Playing packets
		if(iNetStatus == NET_PLAYING || bGameReady)
		{
			cNetEngine->SendWormDetails();
			cNetEngine->SendReportDamage();	// It sends only if someting is queued
		}


		// Send every second
		// TODO: move this somewhere else
		if ((iNetStatus == NET_PLAYING || (iNetStatus == NET_CONNECTED && bGameReady))  && (tLX->currentTime - fMyPingRefreshed).seconds() > 1) {
			CBytestream ping;

			// TODO: move this out here
			ping.Clear();
			ping.writeInt(-1,4);
			if(cServerVersion >= OLXBetaVersion(8))
				ping.writeString("lx::time"); // request for servertime
			else
				ping.writeString("lx::ping");

			ping.Send(this->getChannel()->getSocket());

			fMyPingSent = tLX->currentTime;
			fMyPingRefreshed = tLX->currentTime;
		}
		
		
		// Randomly send a random packet
#if defined(FUZZY_ERROR_TESTING) && defined(FUZZY_ERROR_TESTING_C2S)
		if (GetRandomInt(50) > 24 && iNetStatus == NET_CONNECTED)
			cNetEngine->SendRandomPacket();
#endif
	}
	
	if(iNetStatus == NET_PLAYING || iNetStatus == NET_CONNECTED)
		cNetChan->Transmit(&bsUnreliable);


	if(!sendPendingOnly) {
		if (iNetStatus == NET_CONNECTED && bGameReady && bReadySent)
		{
			bool serverThinksWeAreNotReadyWhenWeAre = false;
			for(unsigned int i=0;i<iNumWorms;i++)
			{
				// getGameReady = what server thinks. getWeaponsReady = what we know.
				serverThinksWeAreNotReadyWhenWeAre |= !cLocalWorms[i]->getGameReady() && cLocalWorms[i]->getWeaponsReady();
			}


			// ready as in localclient == ready, server thinks we are not.
			if (serverThinksWeAreNotReadyWhenWeAre)
			{
				fSendWait += tLX->fDeltaTime;
				if (fSendWait.seconds() > 1.0) {
					notes << "CClient::SendPackets: Server thinks that ";
					for(unsigned int i=0;i<iNumWorms;i++) {
						if(!cLocalWorms[i]->getGameReady() && cLocalWorms[i]->getWeaponsReady())
							notes << "; " << i << ":" << cLocalWorms[i]->getName();
					}
					notes << " is not ready" << endl;
					
					fSendWait = 0.0f;
					cNetEngine->SendGameReady();
				}
			}
		}
	}
	
	bsUnreliable.Clear();
}

bool JoinServer(const std::string& addr, const std::string& name, const std::string& player) {
	hints << "JoinServer " << addr << " (" << name << ") with player '" << player << "'" << endl;
	//tGameInfo.iNumPlayers = 1;
		
	tLX->iGameType = GME_JOIN;
	if(!cClient->Initialize()) {
		warnings << "JoinServer: Could not initialize client" << endl;
		return false;
	}
	
	// Initialize has cleaned up all worms, so this is not necessarily needed
	cClient->setNumWorms(0);
	// Add the player to the list
	profile_t *ply = FindProfile(player);
	if(ply) {
		if(bDedicated && ply->iType == PRF_HUMAN->toInt())
			warnings << "JoinServer: player " << player << " is a human - a human cannot be used in dedicated mode" << endl;
		else {
			cClient->getLocalWormProfiles()[0] = ply;
			cClient->setNumWorms(1);
		}
	}
	
	cClient->setServerName(name);
	
	tLX->iGameType = GME_JOIN;
	cClient->Connect(addr);
	
	if(DedicatedControl::Get())
		DedicatedControl::Get()->Connecting_Signal(addr);

	return true;
}



///////////////////
// Start a connection with the server
void CClient::Connect(const std::string& address)
{
	notes << "Client connect to " << address << endl;
	iNetStatus = NET_CONNECTING;
	reconnectingAmount = 0;
	strServerAddr_HumanReadable = strServerAddr = address;
	iNumConnects = 0;
	bBadConnection = false;
	cServerVersion.reset();
	fConnectTime = tLX->currentTime;
	bConnectingBehindNat = false;
	iNatTraverseState = NAT_RESOLVING_DNS;
	fLastTraverseSent = AbsTime();
	fLastChallengeSent = AbsTime();

	// TODO: use the easier and better event system
	// Register the IRC callbacks
	if (GetGlobalIRC())  {
		GetGlobalIRC()->setNewMessageCallback(&IRC_OnNewMessage);
		GetGlobalIRC()->setDisconnectCallback(&IRC_OnDisconnect);
	}

	InitializeDownloads();

	if(!StringToNetAddr(address, cServerAddr)) {

		strServerAddr_HumanReadable = strServerAddr + " (...)";
		Timer("client connect DNS timeout", null, NULL, DNS_TIMEOUT * 1000 + 50, true).startHeadless();

		if(!GetNetAddrFromNameAsync(address, cServerAddr)) {
			iNetStatus = NET_DISCONNECTED;
			bBadConnection = true;
			strBadConnectMsg = "Unknown error while resolving address";
		}
	}

	// Connecting to a server behind a NAT?
	if(DeprecatedGUI::Menu_SvrList_GetUdpMasterserverForServer(strServerAddr) != "")  {
		bConnectingBehindNat = true;
		sUdpMasterserverAddress = DeprecatedGUI::Menu_SvrList_GetUdpMasterserverForServer(strServerAddr);

		// Start UDP NAT traversal immediately - we know for sure that
		// the host is registered on UDP masterserver and won't respond on ping


		iNatTraverseState = NAT_RESOLVING_DNS;

		ConnectingBehindNAT();

	} else {  // Normal server

		// send a challenge packet immediatly (if possible)
		Connecting(true);
	}
}

void CClient::Reconnect() {
	(tLX->iGameType == GME_JOIN ? notes : warnings)
		<< "Reconnecting local client" << endl;
	
	// HINT: Don't disconnect because we don't want to lose the connection
	// and we also want to keep the client struct on the server.

	// TODO: move this out here
	// Tell the server we are connecting, and give the server our details
	CBytestream bytestr;
	bytestr.writeInt(-1,4);
	bytestr.writeString("lx::connect");
	bytestr.writeInt(PROTOCOL_VERSION,1);
	bytestr.writeInt(this->iChallenge,4);
	bytestr.writeInt(this->iNetSpeed,1);
	bytestr.writeInt(this->iNumWorms, 1);
	
	// Send my worms info
    //
    // __MUST__ match the layout in CWorm::writeInfo() !!!
    //
	
	for(uint i=0;i<this->iNumWorms;i++) {
		// TODO: move this out here
		bytestr.writeString(RemoveSpecialChars(this->tProfiles[i]->sName));
		bytestr.writeInt(this->tProfiles[i]->iType,1);
		bytestr.writeInt(this->tProfiles[i]->iTeam,1);
		bytestr.writeString(this->tProfiles[i]->cSkin.getFileName());
		bytestr.writeInt(this->tProfiles[i]->R,1);
		bytestr.writeInt(this->tProfiles[i]->G,1);
		bytestr.writeInt(this->tProfiles[i]->B,1);
	}
	
	tSocket->reapplyRemoteAddress();
	bytestr.Send(tSocket);
	
	iNetStatus = NET_CONNECTING;
	reconnectingAmount++;
}


/////////////////////
// Connecting to a server behind a NAT
void CClient::ConnectingBehindNAT()
{
#define TRAVERSE_TIMEOUT 3
#define CHALLENGE_TIMEOUT 1.5f
#define TRY_PORT_COUNT 7 // Number of ports around the given one we try

	// Check if we are connecting
	if(iNetStatus != NET_CONNECTING || !bConnectingBehindNat)
		return;

	switch (iNatTraverseState)  {
	case NAT_RESOLVING_DNS:  {

		// Resolve the UDP master server address
		SetNetAddrValid(cServerAddr, false);
		if(!GetNetAddrFromNameAsync(sUdpMasterserverAddress, cServerAddr)) {
			iNetStatus = NET_DISCONNECTED;
			bBadConnection = true;
			strBadConnectMsg = "Unknown error while resolving address";
			return;
		}

		// Check for a DNS timeout
		if (!IsNetAddrValid(cServerAddr))  {
			if ((tLX->currentTime - fConnectTime).seconds() >= DNS_TIMEOUT)  {
				iNetStatus = NET_DISCONNECTED;
				bBadConnection = true;
				strBadConnectMsg = "Could not find the master server.";
				return;
			}

			// To make sure we get called again
			Timer("client ConnectingBehindNAT DNS timeout", null, NULL, 40, true).startHeadless();

			return; // Wait for DNS resolution

		// The address is valid, send the traverse
		} else {
			CBytestream bs;
			tSocket->setRemoteAddress(cServerAddr); // HINT: this is the address of UDP master server

			bs.writeInt(-1,4);
			bs.writeString("lx::dummypacket");
			bs.Send(tSocket);	// So NAT will open port
			bs.Send(tSocket);
			bs.Send(tSocket);
			bs.Clear();

			// The traverse packet
			bs.writeInt(-1,4);
			bs.writeString("lx::traverse");
			bs.writeString(strServerAddr);	// Old address specified in connect()
			bs.Send(tSocket);

			fLastTraverseSent = tLX->currentTime;
			iNatTraverseState = NAT_WAIT_TRAVERSE_REPLY;

			// To make sure we get called again
			Timer("client ConnectingBehindNAT traverse timeout", null, NULL, (Uint32)(TRAVERSE_TIMEOUT * 1000), true).startHeadless();
		}
	} break;

	case NAT_WAIT_TRAVERSE_REPLY:
		// Check for timeouts
		if ((tLX->currentTime - fLastTraverseSent).seconds() >= TRAVERSE_TIMEOUT)  {
			iNetStatus = NET_DISCONNECTED;
			bBadConnection = true;
			strBadConnectMsg = "No reply from the server."; // Previous message always made me think that masterserver is down, when there's just no reply
			hints << "The UDP master server did not reply to the traverse packet - target server inaccessible." << endl;
			return;
		}
	break;

	case NAT_SEND_CHALLENGE:  {
		// Build the packet
		CBytestream chall;
		chall.writeInt(-1,4);
		chall.writeString("lx::getchallenge");
		chall.writeString(GetFullGameName());

		// For buggy NATs we send a ping before the challenge, in case the first packet gets ignored
		CBytestream ping;
		ping.writeInt(-1,4);
		ping.writeString("lx::ping");

		// Send the packet to few ports around the given one to increase the probability
		static const int p[] = {0, 2, 1, 3, 4, -1, -2}; // Sorted by the probability to speed up the joining process
		int port = GetNetAddrPort(cServerAddr);

		SetNetAddrPort(cServerAddr, (ushort)(port + p[iNatTryPort]));
		tSocket->setRemoteAddress(cServerAddr); // HINT: this is the address of the server behind NAT, not the UDP masterserver  (it got changed in ParseTraverse)

		// As we use this tSocket both for sending and receiving,
		// it's safer to reset the address here.
		ping.Send(tSocket);
		chall.Send(tSocket);

		// Print it to the console
		std::string str;
		NetAddrToString(cServerAddr, str);
		notes << "sending challenge to " << str << endl;

		SetNetAddrPort(cServerAddr, (ushort)port); // Put back the original port

		// To make sure we get called again
		Timer("client ConnectingBehindNAT challenge timeout", null, NULL, (Uint32)(CHALLENGE_TIMEOUT * 1000), true).startHeadless();

		fLastChallengeSent = tLX->currentTime;
		iNatTraverseState = NAT_WAIT_CHALLENGE_REPLY;
		iNatTryPort++;
	} break;

	case NAT_WAIT_CHALLENGE_REPLY:  {
		// Check for timeouts
		if ((tLX->currentTime - fLastChallengeSent).seconds() >= CHALLENGE_TIMEOUT)  {

			// If trying ports around the given one
			if (iNatTryPort >= TRY_PORT_COUNT)  {
				notes << "The server behind a NAT did not reply to our challenge." << endl;
				iNatTryPort = 0;
			} else {
				iNatTraverseState = NAT_SEND_CHALLENGE;
				
				// To make sure we get called again
				Timer("client ConnectingBehindNAT challenge waiter", null, NULL, 10, true).startHeadless();
				return;
			}


			// If we've tried 3 times, give up
			if (iNumConnects >= 3)  {
				iNetStatus = NET_DISCONNECTED;
				bBadConnection = true;
				strBadConnectMsg = "Cannot connect to the server.";
			}
			iNumConnects++;

			// Start from scratch
			iNatTraverseState = NAT_RESOLVING_DNS;
			fLastTraverseSent = AbsTime();
			fLastChallengeSent = AbsTime();

			// To make sure we get called again
			Timer("client ConnectingBehindNAT restarter", null, NULL, 10, true).startHeadless();

			return;
		}
	} break;
	}
}


///////////////////
// Connecting process
void CClient::Connecting(bool force)
{
	// Check if we are connecting
	if(iNetStatus != NET_CONNECTING)
		return;

	// Try every three seconds
	if(!force && ((tLX->currentTime - fConnectTime).seconds() < 3.0f))
		return;

	// For local play/hosting: don't send the challenge more times
	// On slower machines the loading can be pretty slow and take more than 3 seconds
	// That doesn't mean that the packet is not delivered
	if (tLX->iGameType != GME_JOIN && iNumConnects > 0)
		return;


	// If we have tried 30 times (30 secs) revert to UDP server
	if(iNumConnects >= 30) {
		iNetStatus = NET_DISCONNECTED;
		bBadConnection = true;
		strBadConnectMsg = "Server timeout after 30 tries";
		return;
	}

	// Check for DNS timeout
	if(!IsNetAddrValid(cServerAddr)) {
		if((tLX->currentTime - fConnectTime).seconds() >= DNS_TIMEOUT) { // timeout
			iNetStatus = NET_DISCONNECTED;
			bBadConnection = true;
			strBadConnectMsg = "Domain could not be resolved after " + itoa(DNS_TIMEOUT) + " seconds";
		}
		return;
	}

	// Check that we have a port
	if(GetNetAddrPort(cServerAddr) == 0)  {
		if (tLX->iGameType == GME_JOIN) // Remote joining
			SetNetAddrPort(cServerAddr, LX_PORT);  // Try the default port if no port specified
		else // Host or local
			SetNetAddrPort(cServerAddr, tLXOptions->iNetworkPort);  // Use the port specified in options
	}

	// Update the server address
	std::string rawServerAddr;
	NetAddrToString( cServerAddr, rawServerAddr );
	//if( rawServerAddr != strServerAddr )
	//	strServerAddr_HumanReadable = strServerAddr + " (" + rawServerAddr + ")";

	fConnectTime = tLX->currentTime;
	iNumConnects++;

	// Request a challenge id
	CBytestream bs;
	tSocket->setRemoteAddress(cServerAddr);

	// Send the challengle packet
	bs.writeInt(-1,4);
	bs.writeString("lx::getchallenge");
	bs.writeString(GetFullGameName());
	// As we use this tSocket both for sending and receiving,
	// it's safer to reset the address here.
	bs.Send(tSocket);


	Timer("client Connecting timeout", null, NULL, 1000, true).startHeadless();

	notes << "sending challenge request to " << rawServerAddr << endl;
}


///////////////////
// Disconnect
void CClient::Disconnect()
{
	cNetEngine->SendDisconnect();

	iNetStatus = NET_DISCONNECTED;

	if (bDownloadingMap)
		cHttpDownloader->CancelFileDownload(sMapDownloadName);
	getUdpFileDownloader()->reset();


	// Log leaving the server
	if (tLXOptions->bLogConvos && convoLogger)
		convoLogger->leaveServer();
	
	if( GetGlobalIRC() )
		GetGlobalIRC()->setAwayMessage("");
}


///////////////////
// Setup the viewports for the local players
void CClient::SetupViewports() {
	if(bDedicated) return;
	
	std::vector<CWorm*> humanWorms; humanWorms.reserve(2);
	for(uint i = 0; i < iNumWorms; ++i) {
		if(cLocalWorms[i] && cLocalWorms[i]->getType() == PRF_HUMAN)
			humanWorms.push_back(cLocalWorms[i]);
	}

	SetupViewports((humanWorms.size() > 0) ? humanWorms[0] : NULL, (humanWorms.size() > 1) ? humanWorms[1] : NULL, VW_FOLLOW, VW_FOLLOW);
}


///////////////////
// Setup the viewports
void CClient::SetupViewports(CWorm *w1, CWorm *w2, int type1, int type2)
{
	if (w1 == NULL)  {
		warnings << "CClient::SetupViewports: Worm1 is NULL, quitting!" << endl;
		return;
	}

	// Reset
	for( int i=0; i<NUM_VIEWPORTS; i++ )  {
        cViewports[i].setUsed(false);
		cViewports[i].setTarget(NULL);
	}

    // Setup inputs
    cViewports[0].setupInputs( tLXOptions->sPlayerControls[0] );
    cViewports[1].setupInputs( tLXOptions->sPlayerControls[1] );


	// Setup according to top and bottom interface bars
	SmartPointer<SDL_Surface> topbar = NULL;
	SmartPointer<SDL_Surface> bottombar = NULL;
	if (tLX->iGameType == GME_LOCAL)  {
		bottombar = DeprecatedGUI::gfxGame.bmpGameLocalBackground;
		topbar = DeprecatedGUI::gfxGame.bmpGameLocalTopBar;
	} else {
		bottombar = DeprecatedGUI::gfxGame.bmpGameNetBackground;
		topbar = DeprecatedGUI::gfxGame.bmpGameNetTopBar;
	}


	int top = topbar.get() ? (topbar.get()->h) : (tLX->cFont.GetHeight() + 3); // Top bound of the viewports
	if (!tLXOptions->bTopBarVisible)
		top = 0;

	int h = bottombar.get() ? (480 - bottombar.get()->h - top) : (382 - top); // Height of the viewports

	// One worm
	if(w2 == NULL) {
        // HACK HACK: FOR AI TESTING
        //cViewports[0].Setup(0,0,640,382,VW_FREELOOK);

		cViewports[0].Setup(0, top, 640, h, type1);
		cViewports[0].setSmooth( !OwnsWorm(w1->getID()) );
		cViewports[0].setTarget(w1);
		cViewports[0].setUsed(true);
	}

	// Two wormsize
	else  {
		cViewports[0].Setup(0, top, 318, h, type1);
		cViewports[0].setSmooth( !OwnsWorm(w1->getID()) );
		cViewports[0].setTarget(w1);
		cViewports[0].setUsed(true);

		cViewports[1].Setup(322, top, 318, h, type2);
		cViewports[1].setSmooth( !OwnsWorm(w2->getID()) );
		cViewports[1].setTarget(w2);
		cViewports[1].setUsed(true);
	}
	
	bShouldRepaintInfo = true;
}


///////////////////
// Return true if we own the worm
int CClient::OwnsWorm(int id)
{
	for(uint i=0;i<iNumWorms;i++) {
		if(cLocalWorms[i] && id == cLocalWorms[i]->getID())
			return true;
	}

	return false;
}

int CClient::getTeamWormCount(int t) const {
	int c = 0;
	for(int i = 0; i < MAX_WORMS; ++i) {
		CWorm* w = &cRemoteWorms[i];
		if(w->isUsed() && w->getTeam() == t)
			c++;
	}
	return c;
}


static bool setWormAdd(profile_t* p) {
	if(p == NULL) {
		errors << "addWorm(): you have to specify the profile" << endl;
		return false;
	}

	if(cClient->getNumWorms() + 1 >= MIN((int)MAX_WORMS, cClient->getGameLobby()->iMaxPlayers)) {
		warnings << "addWorm(): too many worms" << endl;
		return false;
	}
	
	cClient->getLocalWormProfiles()[cClient->getNumWorms()] = p;
	cClient->setNumWorms(cClient->getNumWorms() + 1);
	return true;
}

static std::list<int> updateAddedWorms(bool outOfGame) {
	std::list<int> addedWorms;
	
	if(tLX->iGameType == GME_JOIN) {
		if(outOfGame) warnings << "updateAddedWorms: we cannot avoid worm spawning" << endl;
		cClient->Reconnect(); // we have to reconnect to inform the server about the new worm
		return addedWorms;
	}
	
	// we can do it direct as host (similar to kickWorm)

	CServerConnection* localConn = NULL;
	for( int i=0; i<MAX_CLIENTS; i++ )
		if(cServer->getClients()[i].isLocalClient()) {
			localConn = &cServer->getClients()[i];
			break;
		}
	if(localConn == NULL) {
		errors << "updateAddedWorms: localClient not found" << endl;
		return addedWorms;
	}
	
	for(int i = 0; i < cClient->getNumWorms(); ++i) {
		if(localConn->getWorm(i) == NULL) { // this is a new worm
			// first add the worm on the server
			
			WormJoinInfo info;
			info.loadFromProfile(cClient->getLocalWormProfiles()[i]);
			CWorm* serverWorm = cServer->AddWorm(info);
			if(serverWorm == NULL) {
				warnings << "updateAddedWorms: cannot add worm " << info.sName << endl;
				cClient->setNumWorms(i);
				break;
			}
			addedWorms.push_back(serverWorm->getID());
			
			serverWorm->setClient(localConn);
			localConn->setNumWorms(i + 1);
			localConn->setWorm(i, serverWorm);

			hints << "Worm added: " << serverWorm->getName();
			hints << " (id " << serverWorm->getID() << ", team " << serverWorm->getTeam() << ")" << endl;

			serverWorm->setAlive(false);
			if(outOfGame) serverWorm->setLives(WRM_OUT);
			if(cServer->getState() != SVS_LOBBY) {
				serverWorm->Prepare(true); // prepare serverside
				cServer->PrepareWorm(serverWorm);
			}

			// inform everybody else about new worm
			for(int ii = 0; ii < MAX_CLIENTS; ii++) {
				if(cServer->getClients()[ii].isLocalClient()) continue;
				if(cServer->getClients()[ii].getStatus() != NET_CONNECTED) continue;
				if(cServer->getClients()[ii].getNetEngine() == NULL) continue;
				cServer->getClients()[ii].getNetEngine()->SendUpdateWorm(serverWorm);
				
			}
			
			// handling for connect during game
			if( cServer->getState() != SVS_LOBBY ) {
				for(int ii = 0; ii < MAX_CLIENTS; ii++) {
					if(cServer->getClients()[ii].getStatus() != NET_CONNECTED) continue;
					if(cServer->getClients()[ii].getNetEngine() == NULL) continue;
					cServer->getClients()[ii].getNetEngine()->SendWormScore( serverWorm );
				}

				if(!CServerNetEngine::isWormPropertyDefault(serverWorm)) {					
					for( int j = 0; j < MAX_CLIENTS; j++ ) {
						if(cServer->getClients()[j].getStatus() != NET_CONNECTED) continue;
						if(cServer->getClients()[j].getNetEngine() == NULL) continue;
						cServer->getClients()[j].getNetEngine()->SendWormProperties(serverWorm); // if we have changed them in prepare or so
					}
				}
			}
			
			// ----- now add the worm on the client ----
			
			// (code from CClientNetEngine::ParseConnected) 
			cClient->setWorm(i, &cClient->getRemoteWorms()[serverWorm->getID()]);
			if(cClient->getWorm(i)->isUsed()) {
				warnings << "updateAddedWorms: local worm " << i << " was already used, it is ";
				warnings << cClient->getWorm(i)->getID() << ":" << cClient->getWorm(i)->getName() << endl;
			}
			
			cClient->getWorm(i)->setUsed(true);
			cClient->getWorm(i)->setAlive(false);
			info.applyTo(cClient->getWorm(i)); // sets skin
			cClient->getWorm(i)->setClient(NULL); // Local worms won't get CServerConnection owner
			cClient->getWorm(i)->setLocal(true);
			cClient->getWorm(i)->setName(serverWorm->getName());
			cClient->getWorm(i)->setID(serverWorm->getID());
			cClient->getWorm(i)->setTeam(serverWorm->getTeam());
			cClient->getWorm(i)->setGameScript(cClient->getGameScript().get()); // TODO: why was this commented out?
			//cClient->getWorm(i)->setLoadingTime(cClient->fLoadingTime);  // TODO: why is this commented out?
			cClient->getWorm(i)->setProfile(cClient->getLocalWormProfiles()[i]);
			if(cClient->getLocalWormProfiles()[i]) {
				cClient->getWorm(i)->setType(WormType::fromInt(cClient->getLocalWormProfiles()[i]->iType));
			} else
				warnings << "updateAddedWorms: profile " << i << " for worm " << serverWorm->getID() << " is not set" << endl;
			cClient->getWorm(i)->setClientVersion(cClient->getClientVersion());
			if(!cClient->getWorm(i)->ChangeGraphics(cClient->getGeneralGameType()))
				warnings << "updateAddedWorms: changegraphics for worm " << serverWorm->getID() << " failed" << endl;
			
			
			// gameready means that we had a preparegame package
			// status==NET_PLAYING means that we are already playing
			if( cClient->getGameReady() ) {
									
				// Also set some game details
				cClient->getWorm(i)->setLives(serverWorm->getLives());
				cClient->getWorm(i)->setKills(serverWorm->getKills());
				cClient->getWorm(i)->setDeaths(serverWorm->getDeaths());
				cClient->getWorm(i)->setTeamkills(serverWorm->getTeamkills());
				cClient->getWorm(i)->setDamage(serverWorm->getDamage());
				cClient->getWorm(i)->setHealth(serverWorm->getHealth());
				cClient->getWorm(i)->setGameScript(cClient->getGameScript().get());
				cClient->getWorm(i)->setWpnRest(cClient->getWeaponRestrictions());
				cClient->getWorm(i)->setLoadingTime(cClient->getGameLobby()->iLoadingTime/100.0f);
				cClient->getWorm(i)->setWeaponsReady(serverWorm->getWeaponsReady());
				cClient->getWorm(i)->CloneWeaponsFrom(serverWorm); // if we had serverside weapons, this will clone them
				
				// Prepare for battle!
				cClient->getWorm(i)->Prepare(false);
				cClient->getWorm(i)->setCanUseNinja(serverWorm->canUseNinja());
				cClient->getWorm(i)->setSpeedFactor(serverWorm->speedFactor());
				cClient->getWorm(i)->setDamageFactor(serverWorm->damageFactor());
				cClient->getWorm(i)->setShieldFactor(serverWorm->shieldFactor());
				cClient->getWorm(i)->setCanAirJump(serverWorm->canAirJump());
				
				if(!cClient->getWorm(i)->getWeaponsReady())
					cClient->getWorm(i)->initWeaponSelection();
				
				if(!cClient->getWorm(i)->getWeaponsReady()) {
					// Note for bots: In the normal case (for bots), they already should have selected their weapons in initWeaponSelection().
					// In case of forcerandomwpns, we have set the wpns in GameServer::PrepareWorm, so they also should be ready.
					// In case of samewpnsashostwrm, it could be that we are waiting for the host worm.
					// Also note that the outOfGame-parameter is ignored here.
					notes << "updateAddedWorms: we have to wait for the weapon selection of the new worm" << endl;
					cClient->setStatus(NET_CONNECTED); // this means that we are not ready with weapon selection
					cClient->setReadySent(false); // to force resent
					// we will recheck that in clients frame
				}
				else { // weapons are already ready
					// copy weapons to server
					serverWorm->CloneWeaponsFrom(cClient->getWorm(i));
					serverWorm->setWeaponsReady(true);
					
					if(cClient->getStatus() == NET_PLAYING) { // that means that we were already ready before
						// send weapon list to other clients
						for(int ii = 0; ii < MAX_CLIENTS; ii++) {
							if(!cServer->getClients()[ii].isLocalClient()) {
								// TODO: move that out here
								CBytestream bs;
								bs.writeByte(S2C_CLREADY);
								bs.writeByte(1);
								cClient->getWorm(i)->writeWeapons(&bs);
								cServer->getClients()[ii].getNetEngine()->SendPacket(&bs);
							}
						}
					}						

					if(cServer->getState() == SVS_PLAYING && !outOfGame) {
						// spawn worm
						cServer->SpawnWorm( serverWorm );
					}						
				}
				
				if(!bDedicated && cClient->getWorm(i)->getType() == PRF_HUMAN) {
					// we must resetup the inputs
					cClient->SetupGameInputs();
					// also resetup viewports
					cClient->SetupViewports();
				}

				cClient->UpdateScoreboard();
			}
		}
	}

	// grabbed some code from GameServer::ParseConnect
	
	if( cServer->getState() == SVS_LOBBY )
		cServer->UpdateGameLobby(); // tell everybody about game lobby details
	
	if (tLX->iGameType != GME_LOCAL) {
		if( cServer->getState() == SVS_LOBBY )
			cServer->SendWormLobbyUpdate(); // to everbody
		else {
			for( int i=0; i<MAX_CLIENTS; i++ )
				cServer->SendWormLobbyUpdate( &cServer->getClients()[i], localConn); // send only data about our client
		}
	}
	
	DeprecatedGUI::bHost_Update = true;
	
	// Game state has changed (in many possible ways), just recheck
	cServer->RecheckGame();
	
	return addedWorms;
}

static void prepareWormAdd() {
	if(tLX->iGameType != GME_JOIN) {
		// We are changing the amounts of worms, thus we have to sync the network now.
		// This is needed because the network protocol depends on the amount of worms
		// and we cannot parse old packets in the network stream correct anymore.
		
		SyncServerAndClient();
	}
}

std::list<int> CClient::AddRandomBots(int amount, bool outOfGame) {
	{
		std::string reason;
		if(!canAddWorm(&reason)) {
			hints << "CClient::AddRandomBots: " << reason << endl;
			return std::list<int>();
		}
	}

	// too many worms are handled in the loop below
	if(amount < 1) {
		errors << "AddRandomBot: " << amount << " is an invalid amount" << endl;
		return std::list<int>();
	}

	std::vector<profile_t*> bots;
	for(profile_t* p = GetProfiles(); p != NULL; p = p->tNext) {
		if(p->iType == PRF_COMPUTER->toInt())
			bots.push_back(p);
	}
	
	if(bots.size() == 0) {
		// TODO: add a bot to profiles in that case
		errors << "Can't find ANY bot profile!" << endl;
		return std::list<int>();
	}
	
	prepareWormAdd();
	for(int i = 0; i < amount; ++i)
		if(!setWormAdd(randomChoiceFrom(bots))) break;
	return updateAddedWorms(outOfGame);
}

int CClient::AddWorm(profile_t* p, bool outOfGame) {
	{
		std::string reason;
		if(!canAddWorm(&reason)) {
			hints << "CClient::AddWorm: " << reason << endl;
			return -1;
		}
	}
	
	prepareWormAdd();
	if(!setWormAdd(p)) return -1;
	std::list<int> worms = updateAddedWorms(outOfGame);
	if(worms.size() == 1)
		return worms.front();
	else if(worms.size() == 0)
		return -1;

	errors << "CClient::AddWorm has added multiple worms..." << endl;
	return worms.front();
}


//////////////////
// Remove the worm
void CClient::RemoveWorm(int id)
{
	if(id < 0 || id >= MAX_WORMS) {
		warnings << "CClient::RemoveWorm: worm id " << id << " is invalid" << endl;
		return;
	}
	
	bool found = false;
	for (int i=0;i<MAX_PLAYERS;i++)  {
		if (cLocalWorms[i])  {
			if(i >= getNumWorms()) {
				warnings << "Client:RemoveWorm: own worm " << cLocalWorms[i]->getID() << ":" << cLocalWorms[i]->getName() << " should not exist" << endl;
			}
			if (cLocalWorms[i]->getID() == id)  {
				found = true;
				iNumWorms--;
				cLocalWorms[i] = NULL;
				tProfiles[i] = NULL;
				for (int j=i;j<MAX_PLAYERS-1;j++)  {
					cLocalWorms[j] = cLocalWorms[j+1];
					tProfiles[j] = tProfiles[j+1];
				}
				cLocalWorms[MAX_PLAYERS-1] = NULL;
				tProfiles[MAX_PLAYERS-1] = NULL;				
				break;
			}
		}
	}
	
	if (cRemoteWorms)  {
		if(cRemoteWorms[id].getLocal() && !found) {
			warnings << "CClient::RemoveWorm: didn't found local worm with id " << id << endl;
		}

		cRemoteWorms[id].Unprepare();
		// TODO: why not a Clear() here?
		cRemoteWorms[id].setUsed(false);
		cRemoteWorms[id].setAlive(false);
		cRemoteWorms[id].setKills(0);
		cRemoteWorms[id].setDeaths(0);
		cRemoteWorms[id].setTeamkills(0);
		cRemoteWorms[id].setLives(WRM_OUT);
		cRemoteWorms[id].setProfile(NULL);
		cRemoteWorms[id].setType(PRF_HUMAN);
		cRemoteWorms[id].setLocal(false);
		cRemoteWorms[id].setTagIT(false);
		cRemoteWorms[id].setTagTime(TimeDiff(0));
	}
	else
		errors << "CClient::RemoveWorm: cRemoteWorms not set" << endl;
}

bool CClient::isWormVisibleOnAnyViewport(int worm) const {
	for(int i = 0; i < NUM_VIEWPORTS; ++i) {
		if(!cViewports[i].getUsed()) continue;
		if(cRemoteWorms[worm].isVisible(&cViewports[i])) return true;
	}
	return false;
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
	mod = cGameScript.get()->GetHeader()->ModName;
	xmlEntityText(levelfile);
	xmlEntityText(modfile);
	xmlEntityText(level);
	xmlEntityText(mod);

	// Save the game info
	data =	"<game datetime=\"" + tGameLog->sGameStart + "\" " +
			"length=\"" + ftoa((fGameOverTime - tGameLog->fGameStart).seconds()) + "\" " +
			"loading=\"" + itoa(tGameInfo.iLoadingTime) + "\" " +
			"gamespeed=\"" + ftoa(tGameInfo.features[FT_GameSpeed]) + "\" " +
			"lives=\"" + itoa(tGameInfo.iLives) + "\" " +
			"maxkills=\"" + itoa(tGameInfo.iKillLimit) + "\" " +
			"bonuses=\"" + (tGameInfo.bBonusesOn ? "1" : "0") + "\" " +
			"bonusnames=\"" + (tGameInfo.bShowBonusName ? "1" : "0") + "\" " +
			"levelfile=\"" + levelfile + "\" " +
			"modfile=\"" + modfile + "\" " +
			"level=\"" + level + "\" " +
			"mod=\"" + mod + "\" " +
			"winner=\"" + itoa(tGameLog->iWinner) + "\" " +
			"gamemode=\"" + tGameInfo.sGameMode + "\">";

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
		xmlEntityText(player);

		// Replace the entities
		skin = tGameLog->tWorms[i].sSkin;
		xmlEntityText(skin);

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
				"tagtime=\"" + ftoa(tGameLog->tWorms[i].fTagTime.seconds()) + "\" " +
				"left=\"" + (tGameLog->tWorms[i].bLeft ? "1" : "0") + "\" " +
				"timeleft=\"" + ftoa(tGameLog->tWorms[i].fTimeLeft.seconds()) + "\" " +
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


////////////////////////
// Select weapons for client handling local bots in net play
// Normally, this is done in CClient::Draw
void CClient::BotSelectWeapons()
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
			cNetEngine->SendGameReady();
		}
	}
}


bool CClient::canAddWorm(std::string* noReason) {
	if(!shouldDoProjectileSimulation()) {
		if(noReason) *noReason = "we have projectile simulation disabled (Misc.DoProjectileSimulationInDedicated)";
		return false;
	}
	
	return true;
}


///////////////////
// Shutdown the log structure
void CClient::ShutdownLog()
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
void CClient::Shutdown() {
	// Remote worms
	if(cRemoteWorms) {
		for(int i=0;i<MAX_WORMS;i++)
			cRemoteWorms[i].Shutdown();
		delete[] cRemoteWorms;
		cRemoteWorms = NULL;
	}

	for(int i = 0; i < MAX_WORMS; ++i)
		cLocalWorms[i] = NULL;
	for(int i = 0; i < MAX_WORMS; ++i)
		tProfiles[i] = NULL;
	
	
	// Projectiles
	cProjectiles.clear();
	projPosMap.clear();

	// Map
	if(tLX->iGameType == GME_JOIN) {
		if(cMap && !bMapGrabbed) {
			cMap->Shutdown();
			delete cMap;
		}
		bMapGrabbed = false;
		cMap = NULL;
	}

	// Box buffer
	bmpBoxBuffer = NULL;

	// Ingame score background
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
	if (cDownloadBar)
		delete cDownloadBar;

	cHealthBar1 = cHealthBar2 = cWeaponBar1 = cWeaponBar2 = cDownloadBar = NULL;

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
		cChatList->InitializeChatBox(); // TODO; why is that needed to delete the chatbox? this seems like a hack which has to be fixed
		delete cChatList;
		cChatList = NULL;
	}

	if(m_flagInfo) {
		delete m_flagInfo;
		m_flagInfo = NULL;
	}
	
	// HINT: GameScript is shut down by the cache

	// Weapon restrictions
	cWeaponRestrictions.Shutdown();

	// Close the socket
	tSocket->Clear();

	// Shutdown map downloads
	ShutdownDownloads();

	// Shutdown logging
	ShutdownLog();

	// If the game options are running, shut them down as well
	if (DeprecatedGUI::bShowFloatingOptions)
		DeprecatedGUI::Menu_FloatingOptionsShutdown();

	// Log this
	if (tLXOptions->bLogConvos && convoLogger)
		convoLogger->leaveServer();
}

void CClient::setClientVersion(const Version& v)
{
	cClientVersion = v;
	//printf(this->debugName() + " is using " + cClientVersion.asString() + "\n");
}

void CClient::setServerVersion(const std::string & _s)
{
	cServerVersion.setByString(_s);
	notes("Server is using " + cServerVersion.asString() + "\n");
}

bool CClient::RebindSocket()
{
	if(!tSocket->isOpen())
		return false;
	NetworkAddr addr = tSocket->localAddress();
	tSocket->Close();
	if(!tSocket->OpenUnreliable(0)) {
		SetError("Error: Could not open UDP socket!");
		return false;
	}
	return true;
}

CChannel * CClient::createChannel(const Version& v)
{
	if( cNetChan )
		delete cNetChan;
	if( v >= OLXBetaVersion(0,58,1) )
		cNetChan = new CChannel3();
	else if( v >= OLXBetaVersion(6) )
		cNetChan = new CChannel2();
	else
		cNetChan = new CChannel_056b();
	return cNetChan;
}

void CClient::setNetEngineFromServerVersion()
{
	if(cNetEngine) delete cNetEngine; cNetEngine = NULL;
	if( getServerVersion() >= OLXBetaVersion(0,58,1) )
		cNetEngine = new CClientNetEngineBeta9(this);
	else if( getServerVersion() >= OLXBetaVersion(7) )
		cNetEngine = new CClientNetEngineBeta7(this);
	else
		cNetEngine = new CClientNetEngine(this);
};

std::string CClient::debugName() {
	std::string adr = "?.?.?.?";

	if(isLocalClient())
		adr = "local";
	else if(!getChannel())  {
		warnings("CServerConnection::debugName(): getChannel() == NULL\n");
	} else if(!NetAddrToString(getChannel()->getAddress(), adr))  {
		warnings("CServerConnection::debugName(): NetAddrToString failed\n");
	}

	std::string worms = "no worms";
	if(getNumWorms() > 0) {
		worms = "";
		for(int i = 0; i < getNumWorms(); ++i) {
			if(i > 0) worms += ", ";
			if(getWorm(i)) {
				worms += itoa(getWorm(i)->getID());
				worms += " '";
				worms += getWorm(i)->getName();
				worms += "'";
			} else {
				worms += "BAD";
			}
		}
	}

	return "CClient(" + adr +") with " + worms;
}


void CClient::SetupGameInputs()
{
	if(bDedicated) return;
	
	// Setup the controls
	int humanWormNum = 0;
	for(int i = 0; i < getNumWorms(); i++) {
		if(!getWorm(i)) break; // rare cases, for example if we call it while adding local worms
		CWormHumanInputHandler* handler = dynamic_cast<CWormHumanInputHandler*> (getWorm(i)->inputHandler());
		if(handler) {
			// TODO: Later, let the handler save a rev to his sPlayerControls. This would give
			// more flexibility to the player and he can have multiple player control sets.
			// Then, we would call a reloadInputs() here.
			if(humanWormNum <= 1) {
				handler->setupInputs( tLXOptions->sPlayerControls[humanWormNum] );
				humanWormNum++;
			}
			else
				warnings << "SetupGameInputs: we currently don't support more than 2 local human worms" << endl;
		}
	}

	// TODO: allow more viewports here
	cViewports[0].setupInputs( tLXOptions->sPlayerControls[0] );
	cViewports[1].setupInputs( tLXOptions->sPlayerControls[1] );
	

	// General key shortcuts
	cChat_Input.Setup(tLXOptions->sGeneralControls[SIN_CHAT]);
	cTeamChat_Input.Setup(tLXOptions->sGeneralControls[SIN_TEAMCHAT]);
    cShowScore.Setup(tLXOptions->sGeneralControls[SIN_SCORE]);
	cShowHealth.Setup(tLXOptions->sGeneralControls[SIN_HEALTH]);
	cShowSettings.Setup(tLXOptions->sGeneralControls[SIN_SETTINGS]);
	cViewportMgr.Setup(tLXOptions->sGeneralControls[SIN_VIEWPORTS]);
	cToggleTopBar.Setup(tLXOptions->sGeneralControls[SIN_TOGGLETOPBAR]);

	InitializeSpectatorViewportKeys();

}

int CClient::getNumRemoteWorms()
{
	int ret = 0;
	if( !cRemoteWorms )
		return 0;
	for(int i = 0; i < MAX_WORMS; i++ )
		if( cRemoteWorms[i].isUsed() )
			ret++;
	return ret;
}

void CClient::NewNet_SaveProjectiles()
{
	NewNet_SavedProjectiles = cProjectiles;
}

void CClient::NewNet_LoadProjectiles()
{
	cProjectiles = NewNet_SavedProjectiles;
}

long CClient::MapPosIndex::index(const CMap* m) const {
	if(x < 0 || y < 0) return -1;
	const int w = m->GetWidth() / GRIDW + 1;
	const int h = m->GetHeight() / GRIDH + 1;
	if(x >= w || y >= h) return -1;
	return y * w + x;
}



void CClient::DumpGameState(CmdLineIntf* caller) {
	caller->writeMsg(std::string("Client state: ") + NetStateString((ClientNetState)getStatus()));
	if(getStatus() == NET_DISCONNECTED) return;
	caller->writeMsg("Server info: version " + getServerVersion().asString() + ", addr '" + getServerAddr_HumanReadable() + "'");
	if(cRemoteWorms) {
		for(int i = 0; i < MAX_WORMS; ++i) {
			CWorm* w = &cRemoteWorms[i];
			if(!w->isUsed()) continue;
			std::ostringstream msg;
			msg << " * worm " << i << ":" << w->getName();
			if(w->getLocal()) msg << "(local)";
			caller->writeMsg(msg.str());
		}
	}
	else
		caller->writeMsg("Worms not initialised");
}


