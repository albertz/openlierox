/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Client class - Parsing
// Created 1/7/02
// Jason Boettcher



#include <assert.h>
#include <iostream>
#include <time.h>

#include "LieroX.h"
#include "Cache.h"
#include "CClient.h"
#include "CServer.h"
#include "DeprecatedGUI/Menu.h"
#include "console.h"
#include "GfxPrimitives.h"
#include "FindFile.h"
#include "StringUtils.h"
#include "Protocol.h"
#include "CWorm.h"
#include "Error.h"
#include "Entity.h"
#include "MathLib.h"
#include "EndianSwap.h"
#include "Physics.h"
#include "AuxLib.h"
#include "NotifyUser.h"
#include "Timer.h"
#include "XMLutils.h"
#include "CClientNetEngine.h"
#include "CChannel.h"
#include "DeprecatedGUI/CBrowser.h"
#include "ProfileSystem.h"


using namespace std;

#ifdef _MSC_VER
#undef min
#undef max
#endif

// declare them only locally here as nobody really should use them explicitly
std::string Utf8String(const std::string &OldLxString);


///////////////////
// Parse a connectionless packet
void CClientNetEngine::ParseConnectionlessPacket(CBytestream *bs)
{
	std::string cmd = bs->readString(128);

	if(cmd == "lx::challenge")
		ParseChallenge(bs);

	else if(cmd == "lx::goodconnection")
		ParseConnected(bs);

	else if(cmd == "lx::pong")
		ParsePong();

	else if(cmd == "lx::timeis")
		ParseTimeIs(bs);

	// A Bad Connection
	else if(cmd == "lx::badconnect") {
		// If we are already connected, ignore this
		if (client->iNetStatus == NET_CONNECTED || client->iNetStatus == NET_PLAYING)  {
			printf("CClientNetEngine::ParseConnectionlessPacket: already connected, ignoring\n");
		} else {
			client->iNetStatus = NET_DISCONNECTED;
			client->bBadConnection = true;
			client->strBadConnectMsg = "Server message: " + Utf8String(bs->readString());
		}
	}

	// Host has OpenLX Beta 3+
	else if(cmd.find("lx::openbeta") == 0)  {
		if (cmd.size() > 12)  {
			int betaver = MAX(0, atoi(cmd.substr(12)));
			Version version = OLXBetaVersion(betaver);
			if(client->cServerVersion < version) {
				client->cServerVersion = version;
				cout << "HINT: host is at least using OLX Beta3" << endl;
			}
		}
	}

	// this is only important for Beta4 as it's the main method there to inform about the version
	// we send the version string now together with the challenge packet
	else if(cmd == "lx::version")  {
		std::string versionStr = bs->readString();
		Version version(versionStr);
		if(version > client->cServerVersion) { // only update if this version is really newer than what we know already
			client->cServerVersion = version;
			cout << "HINT: Host is using " << versionStr << endl;
		}
	}

	else if (cmd == "lx:mouseAllowed")
		client->bHostAllowsMouse = true;

	else if (cmd == "lx:strafingAllowed")
		client->bHostAllowsStrafing = true;

	else if(cmd == "lx::traverse")
		ParseTraverse(bs);

	else if(cmd == "lx::connect_here")
		ParseConnectHere(bs);

	// Unknown
	else  {
		cout << "CClientNetEngine::ParseConnectionlessPacket: unknown command \"" << cmd << "\"" << endl;
		bs->SkipAll(); // Safety: ignore any data behind this unknown packet
	}
}


///////////////////
// Parse a challenge packet
void CClientNetEngine::ParseChallenge(CBytestream *bs)
{
	// If we are already connected, ignore this
	if (client->iNetStatus == NET_CONNECTED || client->iNetStatus == NET_PLAYING)  {
		printf("CClientNetEngine::ParseChallenge: already connected, ignoring\n");
		return;
	}

#ifdef DEBUG
	if (client->bConnectingBehindNat)
		printf("Got a challenge from the server.\n");
#endif

	CBytestream bytestr;
	bytestr.Clear();
	client->iChallenge = bs->readInt(4);
	if( ! bs->isPosAtEnd() ) {
		client->setServerVersion( bs->readString(128) );
		printf("CClient: connected to %s server\n", client->getServerVersion().asString().c_str());
	} else
		printf("CClient: connected to old (<= OLX beta3) server\n");

	// TODO: move this out here
	// Tell the server we are connecting, and give the server our details
	bytestr.writeInt(-1,4);
	bytestr.writeString("lx::connect");
	bytestr.writeInt(PROTOCOL_VERSION,1);
	bytestr.writeInt(client->iChallenge,4);
	bytestr.writeInt(client->iNetSpeed,1);
	bytestr.writeInt(client->iNumWorms, 1);

	// Send my worms info
    //
    // __MUST__ match the layout in CWorm::writeInfo() !!!
    //

	for(uint i=0;i<client->iNumWorms;i++) {
		// TODO: move this out here
		bytestr.writeString(RemoveSpecialChars(client->tProfiles[i]->sName));
		bytestr.writeInt(client->tProfiles[i]->iType,1);
		bytestr.writeInt(client->tProfiles[i]->iTeam,1);
		bytestr.writeString(client->tProfiles[i]->cSkin.getFileName());
		bytestr.writeInt(client->tProfiles[i]->R,1);
		bytestr.writeInt(client->tProfiles[i]->G,1);
		bytestr.writeInt(client->tProfiles[i]->B,1);
	}

	NetworkAddr addr;
	GetRemoteNetAddr(client->tSocket, addr);
	SetRemoteNetAddr(client->tSocket, addr);
	bytestr.Send(client->tSocket);

	client->setNetEngineFromServerVersion(); // *this may be deleted here! so it's the last command
}


///////////////////
// Parse a connected packet
void CClientNetEngine::ParseConnected(CBytestream *bs)
{
	NetworkAddr addr;

	// If already connected, ignore this
	if (client->iNetStatus == NET_CONNECTED)  {
		printf("CClientNetEngine::ParseConnected: already connected but server received our connect-package twice and we could have other worm-ids\n");
	}
	else
	if(client->iNetStatus == NET_PLAYING) {
		printf("CClientNetEngine::ParseConnected: currently playing; it's too risky to proceed a reconnection, so we ignore this\n");
		return;
	}

	// Setup the client
	client->iNetStatus = NET_CONNECTED;

	// small cleanup (needed for example for reconnecting)
	for(int i = 0; i < MAX_WORMS; i++) {
		client->cRemoteWorms[i].setUsed(false);
		client->cRemoteWorms[i].setAlive(false);
		client->cRemoteWorms[i].setKills(0);
		client->cRemoteWorms[i].setLives(WRM_OUT);
		client->cRemoteWorms[i].setProfile(NULL);
		client->cRemoteWorms[i].setType(PRF_HUMAN);
		client->cRemoteWorms[i].setLocal(false);
		client->cRemoteWorms[i].setTagIT(false);
		client->cRemoteWorms[i].setTagTime(0);
	}

	// Get the id's
	int id=0;
	for(ushort i=0;i<client->iNumWorms;i++) {
		id = bs->readInt(1);
		if (id < 0 || id >= MAX_WORMS)
			continue;
		client->cLocalWorms[i] = &client->cRemoteWorms[id];
		client->cLocalWorms[i]->setUsed(true);
		client->cLocalWorms[i]->setClient(NULL); // Local worms won't get CServerConnection owner
		client->cLocalWorms[i]->setGameScript(client->cGameScript.get()); // TODO: why was this commented out?
		//client->cLocalWorms[i]->setLoadingTime(client->fLoadingTime);  // TODO: why is this commented out?
		client->cLocalWorms[i]->setProfile(client->tProfiles[i]);
		client->cLocalWorms[i]->setTeam(client->tProfiles[i]->iTeam);
		client->cLocalWorms[i]->setLocal(true);
        client->cLocalWorms[i]->setType(client->tProfiles[i]->iType);
	}

	if(!bDedicated) {
		// Setup the viewports
		client->SetupViewports();

		// Setup the controls
		client->cLocalWorms[0]->SetupInputs( tLXOptions->sPlayerControls[0] );
		// TODO: setup also more viewports
		if(client->iNumWorms >= 2)
			client->cLocalWorms[1]->SetupInputs( tLXOptions->sPlayerControls[1] );
	}

	// Create my channel
	GetRemoteNetAddr(client->tSocket, addr);

	if( ! client->createChannel( std::min( client->getServerVersion(), GetGameVersion() ) ) )
	{
		client->bServerError = true;
		client->strServerErrorMsg = "Your client is incompatible to this server";
		return;
	};
	client->cNetChan->Create(&addr,client->tSocket);
	
	if( client->getServerVersion() == OLXBetaVersion(9) )
	{
		// There is no Beta9 release, everything that reports itself as Beta9 
		// is pre-release and have incompatible net protocol
		
		printf("Beta9 server detected - it is not supported anymore\n");
		// Put some spam in server chatbox
		if( client->getNumWorms() > 0 )
		{
			for( int count1 = 0; count1 < 2; count1++ )
			{
				for( int count2 = 0; count2 < 3; count2++ )
				{
					client->SendPackets();
					SDL_Delay(100);
					client->ReadPackets();
				}
				client->getNetEngine()->SendText("Your Beta9 support was dropped, please download 0.57 rc1 at http://openlierox.sourceforge.net/", client->getWorm(0)->getName() );
			}
			client->getNetEngine()->SendDisconnect();
		}
		
		client->bServerError = true;
		client->strServerErrorMsg = "This server uses Beta9 which is not supported anymore";
		
		return;
	}

	DeprecatedGUI::bJoin_Update = true;
	DeprecatedGUI::bHost_Update = true;

	client->bHostAllowsMouse = false;
	client->bHostAllowsStrafing = false;

}

//////////////////
// Parse the server's ping reply
void CClientNetEngine::ParsePong()
{
	if (client->fMyPingSent > 0)  {
		int png = (int) ((tLX->fCurTime-client->fMyPingSent)*1000);

		// Make the ping slighter
		if (png - client->iMyPing > 5 && client->iMyPing && png)
			png = (png + client->iMyPing + client->iMyPing)/3;
		if (client->iMyPing - png > 5 && client->iMyPing && png)
			png = (png + png + client->iMyPing)/3;

		client->iMyPing = png;
	}
}

//////////////////
// Parse the server's servertime request reply
void CClientNetEngine::ParseTimeIs(CBytestream* bs)
{
	float time = bs->readFloat();
	if (time > client->fServertime)
		client->fServertime = time;

	// This is the response of the lx::time packet, which is sent instead of the lx::ping.
	// Therefore we should also handle this as a normal response to a ping.
	ParsePong();
}


//////////////////
// Parse a NAT traverse packet
void CClientNetEngine::ParseTraverse(CBytestream *bs)
{
	client->iNatTraverseState = NAT_SEND_CHALLENGE;
	client->iNatTryPort = 0;
	std::string addr = bs->readString();
	if( addr.find(":") == std::string::npos )
		return;
	StringToNetAddr(addr, client->cServerAddr); // HINT: this changes the address so the lx::challenge in CClientNetEngine::ConnectingBehindNat is sent to the real server
	int port = atoi( addr.substr( addr.find(":") + 1 ) );
	SetNetAddrPort(client->cServerAddr, port);
	NetAddrToString( client->cServerAddr, addr );

	// HINT: the connecting process now continues by sending a challenge in CClientNetEngine::ConnectingBehindNAT()

	printf("CClientNetEngine::ParseTraverse() %s port %i\n", addr.c_str(), port);
};

/////////////////////
// Parse a connect-here packet (when a public-ip client connects to a symmetric-nat server)
void CClientNetEngine::ParseConnectHere(CBytestream *bs)
{
	client->iNatTraverseState = NAT_SEND_CHALLENGE;
	client->iNatTryPort = 0;

	NetworkAddr addr;
	GetRemoteNetAddr(client->tSocket, addr);
	std::string a1, a2;
	NetAddrToString( client->cServerAddr, a1 );
	NetAddrToString( addr, a2 );
	printf("CClientNetEngine::ParseConnectHere(): addr %s to %s %s\n", a1.c_str(), a2.c_str(), a1 != a2 ? "- server behind symmetric NAT" : "" );

	GetRemoteNetAddr(client->tSocket, client->cServerAddr);
	CBytestream bs1;
	bs1.writeInt(-1,4);
	bs1.writeString("lx::ping");	// So NAT/firewall will understand we really want to connect there
	bs1.Send(client->tSocket);
	bs1.Send(client->tSocket);
	bs1.Send(client->tSocket);
};


/*
=======================================
		Connected Packets
=======================================
*/


///////////////////
// Parse a packet
void CClientNetEngine::ParsePacket(CBytestream *bs)
{
	uchar cmd;

	while(!bs->isPosAtEnd()) {
		cmd = bs->readInt(1);

		switch(cmd) {


			// Prepare the game
			case S2C_PREPAREGAME:
				if(!ParsePrepareGame(bs))
                    return;
				break;

			// Start the game
			case S2C_STARTGAME:
				ParseStartGame(bs);
				break;

			// Spawn a worm
			case S2C_SPAWNWORM:
				ParseSpawnWorm(bs);
				break;

			// Worm info
			case S2C_WORMINFO:
				ParseWormInfo(bs);
				break;

			// Worm weapon info
			case S2C_WORMWEAPONINFO:
				ParseWormWeaponInfo(bs);
				break;

			// Text
			case S2C_TEXT:
				ParseText(bs);
				break;

			// Chat command completion solution
			case S2C_CHATCMDCOMPLSOL:
				ParseChatCommandCompletionSolution(bs);
				break;

			// chat command completion list
			case S2C_CHATCMDCOMPLLST:
				ParseChatCommandCompletionList(bs);
				break;


			// AFK message
			case S2C_AFK:
				ParseAFK(bs);
				break;

			// Worm score
			case S2C_SCOREUPDATE:
				ParseScoreUpdate(bs);
				break;

			// Game over
			case S2C_GAMEOVER:
				ParseGameOver(bs);
				break;

			// Spawn bonus
			case S2C_SPAWNBONUS:
				ParseSpawnBonus(bs);
				break;

			// Tag update
			case S2C_TAGUPDATE:
				ParseTagUpdate(bs);
				break;

			// Some worms are ready to play (not lobby)
			case S2C_CLREADY:
				ParseCLReady(bs);
				break;

			// Update the lobby state of some worms
			case S2C_UPDATELOBBY:
				ParseUpdateLobby(bs);
				break;

			// Client has left
			case S2C_WORMSOUT:
				ParseWormsOut(bs);
				break;

			// Worm state update
			case S2C_UPDATEWORMS:
				ParseUpdateWorms(bs);
				break;

			// Game lobby update
			case S2C_UPDATELOBBYGAME:
				ParseUpdateLobbyGame(bs);
				break;

			// Worm down packet
			case S2C_WORMDOWN:
				ParseWormDown(bs);
				break;

			// Server has left
			case S2C_LEAVING:
				ParseServerLeaving(bs);
				break;

			// Single shot
			case S2C_SINGLESHOOT:
				ParseSingleShot(bs);
				break;

			// Multiple shots
			case S2C_MULTISHOOT:
				ParseMultiShot(bs);
				break;

			// Stats
			case S2C_UPDATESTATS:
				ParseUpdateStats(bs);
				break;

			// Destroy bonus
			case S2C_DESTROYBONUS:
				ParseDestroyBonus(bs);
				break;

			// Goto lobby
			case S2C_GOTOLOBBY:
				ParseGotoLobby(bs);
				break;

            // I have been dropped
            case S2C_DROPPED:
                ParseDropped(bs);
                break;

            case S2C_SENDFILE:
                ParseSendFile(bs);
                break;

			default:
#if !defined(FUZZY_ERROR_TESTING_S2C)
				printf("cl: Unknown packet\n");
#ifdef DEBUG
				printf("Bytestream dump:\n\n");
				bs->Dump();
				printf("\nDone dumping bytestream\n");
#endif //DEBUG

#endif //FUZZY_ERROR_TESTING
				return;

		}
	}
}


///////////////////
// Parse a prepare game packet
bool CClientNetEngine::ParsePrepareGame(CBytestream *bs)
{
	printf("Got ParsePrepareGame\n");

	if(Warning_QuitEngineFlagSet("CClientNetEngine::ParsePrepareGame: ")) {
		printf("HINT: some previous action tried to quit the GameLoop; we are ignoring this now\n");
		ResetQuitEngineFlag();
	}

	// We've already got this packet
	if (client->bGameReady)  {
		printf("CClientNetEngine::ParsePrepareGame: we already got this\n");
		return false;
	}

	// If we're playing, the game has to be ready
	if (client->iNetStatus == NET_PLAYING)  {
		printf("CClientNetEngine::ParsePrepareGame: playing, had to get this\n");
		client->bGameReady = true;
		return false;
	}

	NotifyUserOnEvent();


	// remove from notifier; we don't want events anymore, we have a fixed FPS rate ingame
	RemoveSocketFromNotifierGroup( client->tSocket );

	client->bGameReady = true;

	int random = bs->readInt(1);
	std::string sMapName;
	if(!random)
		sMapName = bs->readString();

	// Other game details
	client->iGameType = bs->readInt(1);
	client->iLives = bs->readInt16();
	client->iMaxKills = bs->readInt16();
	client->iTimeLimit = bs->readInt16();
	int l = bs->readInt16();
	client->fLoadingTime = (float)l/100.0f;
	client->bBonusesOn = bs->readBool();
	client->bShowBonusName = bs->readBool();

	if(client->iGameType == GMT_TAG)
		client->iTagLimit = bs->readInt16();

	// Load the gamescript
	client->sModName = bs->readString();

	// Bad packet
	if (client->sModName == "")  {
		printf("CClientNetEngine::ParsePrepareGame: invalid mod name (none)\n");
		client->bGameReady = false;
		return false;
	}

	// Clear any previous instances of the map
	if(tGameInfo.iGameType == GME_JOIN) {
		if(client->cMap) {
			client->cMap->Shutdown();
			delete client->cMap;
			client->cMap = NULL;
		}
	}

	// HINT: gamescript is shut down by the cache

    //bs->Dump();


	if(tGameInfo.iGameType == GME_JOIN) {
		client->cMap = new CMap;
		if(client->cMap == NULL) {

			// Disconnect
			client->Disconnect();

			DeprecatedGUI::Menu_MessageBox("Out of memory", "Out of memory when allocating the map.", DeprecatedGUI::LMB_OK);

			client->bGameReady = false;

			printf("CClientNetEngine::ParsePrepareGame: out of memory when allocating map\n");

			return false;
		}
	}


	if(random) {
		// Just create a random map

		// If we're remotely joining a server, we need to load the map
		// Note: This shouldn't happen, coz network games can't use random maps
		if(tGameInfo.iGameType == GME_JOIN) {
			if(!client->cMap->New(504,350,"dirt",client->tInterfaceSettings.MiniMapW,client->tInterfaceSettings.MiniMapH)) {
				client->Disconnect();
				client->bGameReady = false;
				printf("CClientNetEngine::ParsePrepareGame: could not create random map\n");
				return false;
			}
			client->cMap->ApplyRandom();
		} else {
			// Otherwise, grab the server's copy
			assert(cServer);

			client->cMap = cServer->getMap();
			if (!client->cMap)  {  // Bad packet
				client->bGameReady = false;
				return false;
			} else {
				client->cMap->SetMinimapDimensions(client->tInterfaceSettings.MiniMapW, client->tInterfaceSettings.MiniMapH);
				client->bMapGrabbed = true;
			}
		}

	} else {

		// Load the map from a file

		// Invalid packet
		if (sMapName == "")  {
			printf("CClientNetEngine::ParsePrepareGame: bad map name (none)\n");
			client->bGameReady = false;
			return false;
		}

		if(tGameInfo.iGameType == GME_JOIN) {

			// If we are downloading a map, wait until it finishes
			if (!client->bDownloadingMap)  {
				client->bWaitingForMap = false;

				client->cMap->SetMinimapDimensions(client->tInterfaceSettings.MiniMapW, client->tInterfaceSettings.MiniMapH);
				if(!client->cMap->Load(sMapName)) {
					// Show a cannot load level error message
					// If this is a host/local game, something is pretty wrong but if we display the message, things could
					// go even worse
					if (tGameInfo.iGameType == GME_JOIN)  {
						FillSurface(DeprecatedGUI::tMenu->bmpBuffer.get(), tLX->clBlack);

						DeprecatedGUI::Menu_MessageBox(
							"Loading Error",
							std::string("Could not load the level '") + sMapName + "'.\n" + LxGetLastError(),
							DeprecatedGUI::LMB_OK);
						client->bClientError = true;

						// Go back to the menu
						QuittoMenu();
					} else {
						printf("ERROR: load map error for a local game!\n");
					}

					client->bGameReady = false;

					printf("CClientNetEngine::ParsePrepareGame: could not load map "+sMapName+"\n");
					return false;
				}
			} else
				client->bWaitingForMap = true;
		} else {
			assert(cServer);

            // Grab the server's copy of the map
			client->cMap = cServer->getMap();
			if (!client->cMap)  {  // Bad packet
				client->bGameReady = false;
				return false;
			} else {
				client->cMap->SetMinimapDimensions(client->tInterfaceSettings.MiniMapW, client->tInterfaceSettings.MiniMapH);
				client->bMapGrabbed = true;
			}
		}

	}

	PhysicsEngine::Get()->initGame(client->cMap, client);

	client->cGameScript = cCache.GetMod( client->sModName );
	if( client->cGameScript.get() == NULL )
	{
		client->cGameScript = new CGameScript();

		if (client->bDownloadingMod)
			client->bWaitingForMod = true;
		else {
			client->bWaitingForMod = false;

			int result = client->cGameScript.get()->Load(client->sModName);
			cCache.SaveMod( client->sModName, client->cGameScript );
			if(result != GSE_OK) {

				// Show any error messages
				if (tGameInfo.iGameType == GME_JOIN)  {
					FillSurface(DeprecatedGUI::tMenu->bmpBuffer.get(), tLX->clBlack);
					std::string err("Error load game mod: ");
					err += client->sModName + "\r\nError code: " + itoa(result);
					DeprecatedGUI::Menu_MessageBox("Loading Error", err, DeprecatedGUI::LMB_OK);
					client->bClientError = true;

					// Go back to the menu
					GotoNetMenu();
				} else {
					printf("ERROR: load mod error for a local game!\n");
				}
				client->bGameReady = false;

				printf("CClientNetEngine::ParsePrepareGame: error loading mod "+client->sModName+"\n");
    			return false;
			}
		}
	}

    // Read the weapon restrictions
    client->cWeaponRestrictions.updateList(client->cGameScript.get());
    client->cWeaponRestrictions.readList(bs);

	tGameInfo.fGameSpeed = 1.0f;
	client->bServerChoosesWeapons = false;

	// TODO: Load any other stuff
	client->bGameReady = true;

	// Reset the scoreboard here so it doesn't show kills & lives when waiting for players
	client->InitializeIngameScore(true);

	// Copy the chat text from lobby to ingame chatbox
	switch (tGameInfo.iGameType)  {
	case GME_HOST:
		client->sChat_Text = DeprecatedGUI::Menu_Net_HostLobbyGetText();
		break;
	case GME_JOIN:
		client->sChat_Text = DeprecatedGUI::Menu_Net_JoinLobbyGetText();
		break;
	}

	if (!client->sChat_Text.empty())  {
		client->bChat_Typing = true;
		client->bChat_CursorVisible = true;
		client->iChat_Pos = client->sChat_Text.size();
		SendAFK( client->cLocalWorms[0]->getID(), AFK_TYPING_CHAT );
	}

	if(!bDedicated) {
		// TODO: move that out, that does not belong here
		// Load the chat
		DeprecatedGUI::CBrowser *lv = client->cChatList;
		if (lv)  {
			lv->setBorderSize(0);
			lv->InitializeChatBox();
			lines_iterator it = client->cChatbox.At((int)client->cChatbox.getNumLines()-256); // If there's more than 256 messages, we start not from beginning but from end()-256
			//int id = (lv->getLastItem() && lv->getItems()) ? lv->getLastItem()->iIndex + 1 : 0;

			for (; it != client->cChatbox.End(); it++)  {

				// Add only chat text (PM and Team PM messages too)
				if (it->iTextType == TXT_CHAT || it->iTextType == TXT_PRIVATE || it->iTextType == TXT_TEAMPM ) {
					lv->AddChatBoxLine(it->strLine, it->iColour, it->iTextType);
				}
			}
		}
	}


	// (If this is a local game?), we need to reload the worm graphics
	// We do this again because we've only just found out what type of game it is
    // Team games require changing worm colours to match the team colour
	// Inefficient, but i'm not going to redesign stuff for a simple gametype
	CWorm *w = client->cRemoteWorms;
	int num_worms = 0;
	ushort i;
	for(i=0;i<MAX_WORMS;i++,w++) {
		if(w->isUsed()) {
			w->ChangeGraphics(client->iGameType);

			// Also set some game details
			w->setLives(client->iLives);
			w->setKills(0);
			w->setHealth(100);
			w->setGameScript(client->cGameScript.get());
			w->setWpnRest(&client->cWeaponRestrictions);
			w->setLoadingTime(client->fLoadingTime);

			// Prepare for battle!
			w->Prepare(client->cMap);

			num_worms++;
		}
	}


	// Initialize the worms weapon selection menu & other stuff
	for(i=0;i<client->iNumWorms;i++) {
		// we already prepared all the worms (cRemoteWorms) above

		if (!client->bWaitingForMod)
			client->cLocalWorms[i]->InitWeaponSelection();
	}


	// Start the game logging
	client->StartLogging(num_worms);

	client->UpdateScoreboard();
	client->bShouldRepaintInfo = true;

	DeprecatedGUI::bJoin_Update = true;

    return true;
}

bool CClientNetEngineBeta7::ParsePrepareGame(CBytestream *bs)
{
	if( ! CClientNetEngine::ParsePrepareGame(bs) )
		return false;

	// >=Beta7 is sending this
	tGameInfo.fGameSpeed = bs->readFloat();
	client->bServerChoosesWeapons = bs->readBool();

    return true;
};


///////////////////
// Parse a start game packet
void CClientNetEngine::ParseStartGame(CBytestream *bs)
{
	// Already got this
	if (client->iNetStatus == NET_PLAYING)  {
		printf("CClientNetEngine::ParseStartGame: already playing - ignoring\n");
		return;
	}

	// Check that the game is ready
	if (!client->bGameReady)  {
		printf("CClientNetEngine::ParseStartGame: cannot start the game because the game is not ready\n");
		return;
	}

	printf("Client: get start game signal\n");
	client->fLastSimulationTime = tLX->fCurTime;
	client->iNetStatus = NET_PLAYING;
	client->fServertime = 0;

	// Set the local players to dead so we wait until the server spawns us
	for(uint i=0;i<client->iNumWorms;i++)
		client->cLocalWorms[i]->setAlive(false);

	// Re-initialize the ingame scoreboard
	client->InitializeIngameScore(false);
	client->bUpdateScore = true;


	// let our worms know that the game starts know
	for(uint i=0;i<client->iNumWorms;i++) {
		client->cLocalWorms[i]->StartGame();
	}

	NotifyUserOnEvent();
}


///////////////////
// Parse a spawn worm packet
void CClientNetEngine::ParseSpawnWorm(CBytestream *bs)
{
	int id = bs->readByte();
	int x = bs->readInt(2);
	int y = bs->readInt(2);

	// Check
	if (client->iNetStatus != NET_PLAYING)  {
		printf("CClientNetEngine::ParseSpawnWorm: Cannot spawn when not playing (packet ignored)\n");
		return;
	}

	if (!client->cMap) {
		printf("CClientNetEngine::ParseSpawnWorm: cMap not set (packet ignored)\n");
		return;
	}

	// Is the spawnpoint in the map?
	if (x > (int)client->cMap->GetWidth() || x < 0)  {
		printf("CClientNetEngine::ParseSpawnWorm: X-coordinate not in map ("+itoa(x)+")\n");
		return;
	}
	if (y > (int)client->cMap->GetHeight() || y < 0)  {
		printf("CClientNetEngine::ParseSpawnWorm: Y-coordinate not in map ("+itoa(y)+")\n");
		return;
	}

	CVec p = CVec( (float)x, (float)y );

	if (id < 0 || id >= MAX_PLAYERS)  {
		printf("CClientNetEngine::ParseSpawnWorm: invalid ID ("+itoa(id)+")\n");
		return;
	}

	client->cRemoteWorms[id].setAlive(true);
	client->cRemoteWorms[id].Spawn(p);

	client->cMap->CarveHole(SPAWN_HOLESIZE,p);

	// Show a spawn entity
	SpawnEntity(ENT_SPAWN,0,p,CVec(0,0),0,NULL);

	client->UpdateScoreboard();
	if (client->cRemoteWorms[id].getLocal())
		client->bShouldRepaintInfo = true;

	if( client->bSpectate
		&& client->iNumWorms > 0
		&& client->cLocalWorms[0] == &client->cRemoteWorms[id]
		&& client->cLocalWorms[0]->getType() == PRF_HUMAN
		&& client->cRemoteWorms[id].getLives() != WRM_UNLIM)
	{
		// Suicide myself as long as I spawned
		// we do this to get my own worm out of the game because we want only spectate the game
		SendDeath( id, id );
	}
	else
	{
		// Lock viewport back on local worm, if it was screwed when spectating after death
		if( client->iNumWorms > 0 )
			if( client->cLocalWorms[0] == &client->cRemoteWorms[id] && client->cLocalWorms[0]->getType() == PRF_HUMAN )
				client->SetupViewports(client->cLocalWorms[0], NULL, VW_FOLLOW, VW_FOLLOW);
		if( client->iNumWorms >= 2 )
			if (client->cLocalWorms[1]->getType() == PRF_HUMAN)
				client->SetupViewports(client->cLocalWorms[0], client->cLocalWorms[1], VW_FOLLOW, VW_FOLLOW);
		client->sSpectatorViewportMsg = "";
	}
}


///////////////////
// Parse a worm info packet
void CClientNetEngine::ParseWormInfo(CBytestream *bs)
{
	int id = bs->readInt(1);

	// Validate the id
	if (id < 0 || id >= MAX_WORMS)  {
		printf("CClientNetEngine::ParseWormInfo: invalid ID ("+itoa(id)+")\n");
		CWorm::skipInfo(bs); // Skip not to break other packets
		return;
	}

	// A new worm?
	if (!client->cRemoteWorms[id].isUsed())  {
		client->cRemoteWorms[id].Clear();
		client->cRemoteWorms[id].setUsed(true);
		client->cRemoteWorms[id].setClient(NULL); // Client-sided worms won't have CServerConnection
		client->cRemoteWorms[id].setLocal(false);
		client->cRemoteWorms[id].setGameScript(client->cGameScript.get());
		if (client->iNetStatus == NET_PLAYING || client->bGameReady)  {
			client->cRemoteWorms[id].Prepare(client->cMap);
		}
		client->cRemoteWorms[id].setID(id);
	}

	client->cRemoteWorms[id].readInfo(bs);

	// Safety
	if (client->iNetStatus == NET_PLAYING || client->bGameReady)
		client->cRemoteWorms[id].setMap(client->cMap);

	// Load the worm graphics
	if(!client->cRemoteWorms[id].ChangeGraphics(client->iGameType)) {
        printf("CClientNetEngine::ParseWormInfo(): ChangeGraphics() failed\n");
	}

	client->UpdateScoreboard();
	if (client->cRemoteWorms[id].getLocal())
		client->bShouldRepaintInfo = true;

	DeprecatedGUI::bJoin_Update = true;
	DeprecatedGUI::bHost_Update = true;
}

///////////////////
// Parse a worm info packet
void CClientNetEngine::ParseWormWeaponInfo(CBytestream *bs)
{
	int id = bs->readInt(1);

	// Validate the id
	if (id < 0 || id >= MAX_WORMS)  {
		printf("CClientNetEngine::ParseWormInfo: invalid ID ("+itoa(id)+")\n");
		CWorm::skipWeapons(bs); // Skip not to break other packets
		return;
	}

	client->cRemoteWorms[id].readWeapons(bs);

	client->UpdateScoreboard();
	if (client->cRemoteWorms[id].getLocal())
		client->bShouldRepaintInfo = true;
}




///////////////////
// Parse a text packet
void CClientNetEngine::ParseText(CBytestream *bs)
{
	int type = bs->readInt(1);
	if( type < TXT_CHAT )
		type = TXT_CHAT;
	if( type > TXT_TEAMPM )
		type = TXT_TEAMPM;

	Uint32 col = tLX->clWhite;
	int	t = bDedicated ? 0 : client->cLocalWorms[0]->getTeam();
	switch(type) {
		// Chat
		case TXT_CHAT:		col = tLX->clChatText;		break;
		// Normal
		case TXT_NORMAL:	col = tLX->clNormalText;	break;
		// Notice
		case TXT_NOTICE:	col = tLX->clNotice;		break;
		// Network
		case TXT_NETWORK:	col = tLX->clNetworkText;	break;
		// Private
		case TXT_PRIVATE:	col = tLX->clPrivateText;	break;
		// Team Private Chat
		case TXT_TEAMPM:	col = tLX->clTeamColors[t];	break;
	}

	std::string buf = bs->readString();

	// If we are playing a local game, discard network messages
	if(tGameInfo.iGameType == GME_LOCAL) {
		if(type == TXT_NETWORK)
			return;
		if(type != TXT_CHAT)
			col = tLX->clNormalText;
    }

    FILE *f;

	buf = Utf8String(buf);  // Convert any possible pseudo-UTF8 (old LX compatible) to normal UTF8 string

	// Htmlentity nicks in the message
	CWorm *w = client->getRemoteWorms();
	if (w)  {
		for (int i = 0; i < MAX_WORMS; i++, w++)  {
			if (w->isUsed())
				replace(buf, w->getName(), xmlEntities(w->getName()), buf);
		}
	}

	client->cChatbox.AddText(buf, col, (TXT_TYPE)type, tLX->fCurTime);


	// Log the conversation
	if (tLXOptions->bLogConvos)  {
		if(!client->bInServer)  {
			client->cIConnectedBuf = buf;
			return;
		}

		f = OpenGameFile("Conversations.log","a");
		if (!f)
			return;
		fputs("    <message type=\"",f);

		switch(type) {
			// Chat
			case TXT_CHAT:		fputs("CHAT",f);	break;
			// Normal
			case TXT_NORMAL:	fputs("NORMAL",f);	break;
			// Notice
			case TXT_NOTICE:	fputs("NOTICE",f);	break;
			// Network
			case TXT_NETWORK:	fputs("NETWORK",f);	break;
			// Private
			case TXT_PRIVATE:	fputs("PRIVATE",f);	break;
			// Team Private Chat
			case TXT_TEAMPM:	fputs("TEAMPM",f);	break;
		}

		fputs("\" text=\"",f);
		fputs(buf.c_str(),f);
		fputs("\" />\r\n",f);
		fclose(f);
	}
}


static std::string getChatText(CClient* client) {
	if(client->getStatus() == NET_PLAYING)
		return client->chatterText();
	else if(tGameInfo.iGameType == GME_HOST)
		return DeprecatedGUI::Menu_Net_HostLobbyGetText();
	else if(tGameInfo.iGameType == GME_JOIN)
		return DeprecatedGUI::Menu_Net_JoinLobbyGetText();

	cout << "WARNING: getChatText(): cannot find chat source" << endl;
	return "";
}

static void setChatText(CClient* client, const std::string& txt) {
	if(client->getStatus() == NET_PLAYING) {
		client->chatterText() = txt;
		client->setChatPos( Utf8StringSize(txt) );
	} else if(tGameInfo.iGameType == GME_HOST) {
		DeprecatedGUI::Menu_Net_HostLobbySetText( txt );
	} else if(tGameInfo.iGameType == GME_JOIN) {
		DeprecatedGUI::Menu_Net_JoinLobbySetText( txt );
	} else
		cout << "WARNING: setChatText(): cannot find chat source" << endl;
}


void CClientNetEngineBeta7::ParseChatCommandCompletionSolution(CBytestream* bs) {
	std::string startStr = bs->readString();
	std::string solution = bs->readString();

	std::string chatCmd = getChatText(client);

	if(strSeemsLikeChatCommand(chatCmd))
		chatCmd = chatCmd.substr(1);
	else
		return;

	if(stringcaseequal(startStr, chatCmd))
		setChatText(client, "/" + solution);
}

void CClientNetEngineBeta7::ParseChatCommandCompletionList(CBytestream* bs) {
	std::string startStr = bs->readString();

	std::list<std::string> possibilities;
	uint n = bs->readInt(4);
	if (n > 32)
		printf("WARNING: ParseChatCompletionList got a too big number of suggestions (%i)\n", n);

	for(uint i = 0; i < n && !bs->isPosAtEnd(); i++)
		possibilities.push_back(bs->readString());

	std::string chatCmd = getChatText(client);

	if(strSeemsLikeChatCommand(chatCmd))
		chatCmd = chatCmd.substr(1);
	else
		return;

	if(!stringcaseequal(startStr, chatCmd))
		return;

	std::string posStr;
	for(std::list<std::string>::iterator it = possibilities.begin(); it != possibilities.end(); ++it) {
		if(it != possibilities.begin()) posStr += " ";
		posStr += *it;
	}

	client->cChatbox.AddText(posStr, tLX->clNotice, TXT_NOTICE, tLX->fCurTime);
}

///////////////////
// Parse AFK packet
void CClientNetEngineBeta7::ParseAFK(CBytestream *bs)
{
	int id = bs->readByte();
	AFK_TYPE afkType = (AFK_TYPE)bs->readByte();
	std::string message = bs->readString(128);

	// Validate the id
	if (id < 0 || id >= MAX_WORMS)  {
		printf("CClientNetEngine::ParseAFK: invalid ID ("+itoa(id)+")\n");
		return;
	}

	if( ! client->cRemoteWorms[id].isUsed() )
		return;

	client->cRemoteWorms[id].setAFK(afkType, message);

}


///////////////////
// Parse a score update packet
void CClientNetEngine::ParseScoreUpdate(CBytestream *bs)
{
	short id = bs->readInt(1);

	if(id >= 0 && id < MAX_WORMS)  {
		log_worm_t *l = client->GetLogWorm(id);

		// Update the score
		client->cRemoteWorms[id].readScore(bs);
		if (client->cRemoteWorms[id].getLocal())
			client->bShouldRepaintInfo = true;

		// Logging
		if (l)  {
			// Check if the stats changed
			bool stats_changed = false;
			if (l->iLives != client->cRemoteWorms[id].getLives())  {
				l->iLives = client->cRemoteWorms[id].getLives();
				client->iLastVictim = id;
				stats_changed = true;
			}

			if (l->iKills != client->cRemoteWorms[id].getKills())  {
				l->iKills = client->cRemoteWorms[id].getKills();
				client->iLastKiller = id;
				stats_changed = true;
			}

			// If the update was sent but no changes made -> this is a killer that made a teamkill
			// See CServer::ParseDeathPacket for more info
			if (!stats_changed)
				client->iLastKiller = id;
		}
	}
	else
		// do this to get the right position in net stream
		CWorm::skipScore(bs);

	client->UpdateScoreboard();

	DeprecatedGUI::bJoin_Update = true;
	DeprecatedGUI::bHost_Update = true;
}


///////////////////
// Parse a game over packet
void CClientNetEngine::ParseGameOver(CBytestream *bs)
{
	// Check
	if (client->bGameOver)  {
		printf("CClientNetEngine::ParseGameOver: the game is already over, ignoring");
		bs->Skip(1);
		return;
	}

	client->iMatchWinner = CLAMP(bs->readInt(1), 0, MAX_PLAYERS - 1);

	// Get the winner team if TDM (old servers send wrong info here, better when we find it out)
	if (tGameInfo.iGameMode == GMT_TEAMDEATH)  {

		if (tGameInfo.iKillLimit != -1)  {
			client->iMatchWinner = client->cRemoteWorms[client->iMatchWinner].getTeam();
		} else if (tGameInfo.iLives != -2)  {
			for (int i=0; i < MAX_WORMS; i++)  {
				if (client->cRemoteWorms[i].getLives() >= 0)  {
					client->iMatchWinner = client->cRemoteWorms[i].getTeam();
					break;
				}
			}
		}
	}

	// Older servers send wrong info about tag winner, better if we count it ourself
	if (tGameInfo.iGameMode == GMT_TAG)  {
		float max = 0;

		for (int i=0; i < MAX_WORMS; i++)  {
			if (client->cRemoteWorms[i].isUsed() && client->cRemoteWorms[i].getTagTime() > max)  {
				max = client->cRemoteWorms[i].getTagTime();
				client->iMatchWinner = i;
			}
		}
	}

	// Game over
	cout << "the game is over" << endl;
	client->bGameOver = true;
	client->fGameOverTime = tLX->fCurTime;

	if (client->tGameLog)
		client->tGameLog->iWinner = client->iMatchWinner;

    // Clear the projectiles
    client->cProjectiles.clear();

	client->UpdateScoreboard();
	client->bShouldRepaintInfo = true;

	// if we are away (perhaps waiting because we were out), notify us
	NotifyUserOnEvent();
}


///////////////////
// Parse a spawn bonus packet
void CClientNetEngine::ParseSpawnBonus(CBytestream *bs)
{
	int wpn = 0;
	int type = MAX(0,MIN((int)bs->readByte(),2));

	if(type == BNS_WEAPON)
		wpn = bs->readInt(1);

	int id = bs->readByte();
	int x = bs->readInt(2);
	int y = bs->readInt(2);

	// Check
	if (client->iNetStatus != NET_PLAYING)  {
		printf("CClientNetEngine::ParseSpawnBonus: Cannot spawn bonus when not playing (packet ignored)\n");
		return;
	}

	if (id < 0 || id >= MAX_BONUSES)  {
		printf("CClientNetEngine::ParseSpawnBonus: invalid bonus ID ("+itoa(id)+")\n");
		return;
	}

	if (!client->cMap) { // Weird
		printf("WARNING: CClientNetEngine::ParseSpawnBonus: cMap not set\n");
		return;
	}

	if (x > (int)client->cMap->GetWidth() || x < 0)  {
		printf("CClientNetEngine::ParseSpawnBonus: X-coordinate not in map ("+itoa(x)+")\n");
		return;
	}

	if (y > (int)client->cMap->GetHeight() || y < 0)  {
		printf("CClientNetEngine::ParseSpawnBonus: Y-coordinate not in map ("+itoa(y)+")\n");
		return;
	}

	CVec p = CVec( (float)x, (float)y );

	client->cBonuses[id].Spawn(p, type, wpn, client->cGameScript.get());
	client->cMap->CarveHole(SPAWN_HOLESIZE,p);

	SpawnEntity(ENT_SPAWN,0,p,CVec(0,0),0,NULL);
}


///////////////////
// Parse a tag update packet
void CClientNetEngine::ParseTagUpdate(CBytestream *bs)
{
	if (client->iNetStatus != NET_PLAYING || client->bGameOver)  {
		printf("CClientNetEngine::ParseTagUpdate: not playing - ignoring\n");
		return;
	}

	int id = bs->readInt(1);
	float time = bs->readFloat();

	// Safety check
	if(id <0 || id >= MAX_WORMS)  {
		printf("CClientNetEngine::ParseTagUpdate: invalid worm ID ("+itoa(id)+")\n");
		return;
	}

	if (tGameInfo.iGameMode != GMT_TAG)  {
		printf("CClientNetEngine::ParseTagUpdate: game mode is not tag - ignoring\n");
		return;
	}

	// Set all the worms 'tag' property to false
	CWorm *w = client->cRemoteWorms;
	for(int i=0;i<MAX_WORMS;i++,w++) {
		if(w->isUsed())
			w->setTagIT(false);
	}

	// Tag the worm
	client->cRemoteWorms[id].setTagIT(true);
	client->cRemoteWorms[id].setTagTime(time);

	// Log it
	log_worm_t *l = client->GetLogWorm(id);
	if (l)  {
		for (int i=0; i < client->tGameLog->iNumWorms; i++)
			client->tGameLog->tWorms[i].bTagIT = false;

		l->fTagTime = time;
		l->bTagIT = true;
	}
}


///////////////////
// Parse client-ready packet
void CClientNetEngine::ParseCLReady(CBytestream *bs)
{
	int numworms = bs->readByte();

	if((numworms < 0 || numworms > MAX_PLAYERS) && tGameInfo.iGameType != GME_LOCAL) {
		// bad packet
		printf("CClientNetEngine::ParseCLReady: invalid numworms ("+itoa(numworms)+")\n");
		// Skip to get the right position
		for (short i=0;i<numworms;i++)  {
			bs->Skip(1); // id
			CWorm::skipWeapons(bs);
		}
		return;
	}


	byte id;
	CWorm *w;
	for(short i=0;i<numworms;i++) {
		id = bs->readByte();

		if( id >= MAX_WORMS) {
			printf("CClientNetEngine::ParseCLReady: bad worm ID ("+itoa(id)+")\n");
			continue;
		}

		w = &client->cRemoteWorms[id];
		if(w) {
			w->setGameReady(true);

			// Read the weapon info
			w->readWeapons(bs);
		} else {
			// Skip the info and if end of packet, just end
			if (CWorm::skipWeapons(bs))
				break;
		}

	}

	client->bUpdateScore = true; // Change the ingame scoreboard
}


///////////////////
// Parse an update-lobby packet
void CClientNetEngine::ParseUpdateLobby(CBytestream *bs)
{
	int numworms = bs->readByte();
	bool ready = bs->readBool();

	if (client->iNetStatus != NET_CONNECTED || numworms < 0 || numworms > MAX_WORMS)  {
		if (client->iNetStatus != NET_CONNECTED)
			printf("CClientNetEngine::ParseUpdateLobby: not in lobby - ignoring\n");
		else
			printf("CClientNetEngine::ParseUpdateLobby: invalid strange numworms value ("+itoa(numworms)+")\n");

		// Skip to get the right position in stream
		bs->Skip(numworms);
		return;
	}
	/*if(numworms == 0)
		printf("CClientNetEngine::ParseUpdateLobby: warning: numworms == 0\n");*/

	std::string HostName;

	byte id;
	CWorm *w;
	for(short i=0;i<numworms;i++) {
		id = bs->readByte();
        int team = MAX(0,MIN(3,(int)bs->readByte()));

		if( id >= MAX_WORMS) {
			printf("CClientNetEngine::ParseUpdateLobby: invalid worm ID ("+itoa(id)+")\n");
			continue;
		}


		w = &client->cRemoteWorms[id];
        if(w) {
			w->getLobby()->bReady = ready;
            w->getLobby()->iTeam = team;
			w->setTeam(team);
			if(i==0)
				HostName = w->getName();
        }
	}

	// Update lobby
	DeprecatedGUI::bJoin_Update = true;
	DeprecatedGUI::bHost_Update = true;

	// Log the conversation
	if (tLXOptions->bLogConvos)  {
		if(client->bInServer)
			return;

		client->bInServer = true;

		FILE *f;

		f = OpenGameFile("Conversations.log","a");
		if (!f)
			return;
		fputs("  <server hostname=\"",f);
		fputs(HostName.c_str(),f);
		std::string cTime = GetTime();
		fprintf(f,"\" jointime=\"%s\">\r\n",cTime.c_str());
		if(client->cIConnectedBuf != "")  {
			fputs("    <message type=\"NETWORK\" text=\"",f);
			fputs(client->cIConnectedBuf.c_str(),f);
			fputs("\" />\r\n",f);
			client->cIConnectedBuf = "";
		}
		fclose(f);
	}

}


///////////////////
// Parse a worms-out (named 'client-left' before) packet
void CClientNetEngine::ParseWormsOut(CBytestream *bs)
{
	byte numworms = bs->readByte();

	if(numworms < 1 || numworms > MAX_PLAYERS) {
		// bad packet
		printf("CClientNetEngine::ParseWormsOut: bad numworms count ("+itoa(numworms)+")\n");

		// Skip to the right position
		bs->Skip(numworms);

		return;
	}


	byte id;
	CWorm *w;
	for(byte i=0;i<numworms;i++) {
		id = bs->readByte();

		if( id >= MAX_WORMS) {
			printf("CClientNetEngine::ParseWormsOut: invalid worm ID ("+itoa(id)+")\n");
			continue;
		}

		w = &client->cRemoteWorms[id];
		if(!w->getLocal()) { // Server kicks local worms using S2C_DROPPED, this packet cannot be used for it
			w->setUsed(false);
			w->setAlive(false);
			w->getLobby()->iType = LBY_OPEN;

			// Log this
			if (client->tGameLog)  {
				log_worm_t *l = client->GetLogWorm(id);
				if (l)  {
					l->bLeft = true;
					l->fTimeLeft = tLX->fCurTime;
				}
			}
		} else {
			printf("Warning: server says we've left but that is not true\n");
		}
	}

	DeprecatedGUI::bJoin_Update = true;
	DeprecatedGUI::bHost_Update = true;

	client->UpdateScoreboard();
}


///////////////////
// Parse an 'update-worms' packet
void CClientNetEngine::ParseUpdateWorms(CBytestream *bs)
{
	byte count = bs->readByte();
	if (count >= MAX_WORMS || client->iNetStatus != NET_PLAYING)  {
		if (client->iNetStatus != NET_PLAYING)
			printf("CClientNetEngine::ParseUpdateWorms: not playing, ignored\n");
		else
			printf("CClientNetEngine::ParseUpdateWorms: invalid worm count ("+itoa(count)+")\n");

		// Skip to the right position
		for (byte i=0;i<count;i++)  {
			bs->Skip(1);
			CWorm::skipPacketState(bs);
		}

		return;
	}

	byte id;

	for(byte i=0;i<count;i++) {
		id = bs->readByte();

		if (id >= MAX_WORMS)  {
			printf("CClientNetEngine::ParseUpdateWorms: invalid worm ID ("+itoa(id)+")\n");
			if (CWorm::skipPacketState(bs))  {  // Skip not to lose the right position
				break;
			}
			continue;
		}

		/*if (!cRemoteWorms[id].isUsed())  {
			i--;
			continue;
		}*/

		client->cRemoteWorms[id].readPacketState(bs,client->cRemoteWorms);

	}

	DeprecatedGUI::bJoin_Update = true;
	DeprecatedGUI::bHost_Update = true;
}


///////////////////
// Parse an 'update game lobby' packet
void CClientNetEngine::ParseUpdateLobbyGame(CBytestream *bs)
{
	if (client->iNetStatus != NET_CONNECTED)  {
		printf("CClientNetEngine::ParseUpdateLobbyGame: not in lobby - ignoring\n");

		// Skip to get the right position
		bs->Skip(1);
		bs->SkipString();
		bs->SkipString();
		bs->SkipString();
		bs->Skip(8); // All other info

		return;
	}

	game_lobby_t    *gl = &client->tGameLobby;
    FILE            *fp = NULL;

	if (!gl)  {
		//TODO: uniform message system
		//MessageBox(0,"Could not find lobby","Error",MB_OK);
		printf("CClientNetEngine::ParseUpdateLobbyGame: Could not find lobby\n");
		return;
	}

	gl->bSet = true;
	gl->nMaxWorms = bs->readByte();
	gl->szMapFile = bs->readString();
    gl->szModName = bs->readString();
    gl->szModDir = bs->readString();
	gl->nGameMode = bs->readByte();
	gl->nLives = bs->readInt16();
	gl->nMaxKills = bs->readInt16();
	gl->nLoadingTime = bs->readInt16();
    gl->bBonuses = bs->readBool();

	gl->fGameSpeed = 1.0f;
	gl->bForceRandomWeapons = false;
	gl->bSameWeaponsAsHostWorm = false;

    // Check if we have the level & mod
    gl->bHaveMap = true;
    gl->bHaveMod = true;

    // Does the level file exist
    fp = OpenGameFile("levels/" + gl->szMapFile,"rb");
    if(!fp)
        gl->bHaveMap = false;
    else
        fclose(fp);

	// Convert the map filename to map name
	if (gl->bHaveMap)  {
		std::string MapName = DeprecatedGUI::Menu_GetLevelName(gl->szMapFile);
		gl->szDecodedMapName = (MapName != "") ? MapName : gl->szMapFile;
	}

    // Does the 'script.lgs' file exist in the mod dir?
    fp = OpenGameFile(gl->szModDir + "/script.lgs", "rb");
    if(!fp)
        gl->bHaveMod = false;
    else
        fclose(fp);

	tGameInfo.sMapFile = gl->szMapFile;
	tGameInfo.sMapName = gl->szDecodedMapName;
	tGameInfo.sModName = gl->szModName;
	tGameInfo.sModDir = gl->szModDir;
	tGameInfo.bBonusesOn = gl->bBonuses;
	tGameInfo.iGameMode = gl->nGameMode;
	tGameInfo.iKillLimit = gl->nMaxKills;
	tGameInfo.iLives = gl->nLives;
	tGameInfo.iLoadingTimes = gl->nLoadingTime;

	DeprecatedGUI::bJoin_Update = true;
	DeprecatedGUI::bHost_Update = true;
}

void CClientNetEngineBeta7::ParseUpdateLobbyGame(CBytestream *bs)
{
	CClientNetEngine::ParseUpdateLobbyGame(bs);

	game_lobby_t    *gl = &client->tGameLobby;

	gl->fGameSpeed = bs->readFloat();
	gl->bForceRandomWeapons = bs->readBool();
	gl->bSameWeaponsAsHostWorm = bs->readBool();

	tGameInfo.fGameSpeed = gl->fGameSpeed;
};

///////////////////
// Parse a 'worm down' packet
// TODO: what exactly is this?
void CClientNetEngine::ParseWormDown(CBytestream *bs)
{
	// Don't allow anyone to kill us in lobby
	if (client->iNetStatus != NET_PLAYING)  {
		printf("CClientNetEngine::ParseWormDown: not playing - ignoring\n");
		bs->Skip(1);  // ID
		return;
	}

	byte id = bs->readByte();
	byte n;
	CWorm *w;
	float amount;
	short i;

	if(id < MAX_WORMS) {
		client->cRemoteWorms[id].setAlive(false);
		if (client->cRemoteWorms[id].getLocal() && client->cRemoteWorms[id].getType() == PRF_HUMAN)
			client->cRemoteWorms[id].clearInput();

		// Make a death sound
		int s = GetRandomInt(2);
		StartSound( sfxGame.smpDeath[s], client->cRemoteWorms[id].getPos(), client->cRemoteWorms[id].getLocal(), -1, client->cLocalWorms[0]);

		// Spawn some giblets
		w = &client->cRemoteWorms[id];

		for(n=0;n<7;n++)
			SpawnEntity(ENT_GIB,0,w->getPos(),CVec(GetRandomNum()*80,GetRandomNum()*80),0,w->getGibimg().get());

		// Blood
		amount = 50.0f * ((float)tLXOptions->iBloodAmount / 100.0f);
		for(i=0;i<amount;i++) {
			float sp = GetRandomNum()*100+50;
			SpawnEntity(ENT_BLOODDROPPER,0,w->getPos(),CVec(GetRandomNum()*sp,GetRandomNum()*sp),MakeColour(128,0,0),NULL);
			SpawnEntity(ENT_BLOOD,0,w->getPos(),CVec(GetRandomNum()*sp,GetRandomNum()*sp),MakeColour(200,0,0),NULL);
			SpawnEntity(ENT_BLOOD,0,w->getPos(),CVec(GetRandomNum()*sp,GetRandomNum()*sp),MakeColour(128,0,0),NULL);
		}
	} else {
		printf("CClientNetEngine::ParseWormDown: invalid worm ID ("+itoa(id)+")\n");
	}

	// Someone has been killed, log it
	if (client->iLastVictim != -1)  {
		log_worm_t *l_vict = client->GetLogWorm(client->iLastVictim);
		log_worm_t *l_kill = l_vict;

		// If we haven't received killer's update score, it has been a suicide
		if (client->iLastKiller != -1)
			l_kill = client->GetLogWorm(client->iLastKiller);

		if (l_kill && l_vict)  {
			// HINT: lives and kills are updated in ParseScoreUpdate

			// Suicide
			if (l_kill == l_vict)  {
				l_vict->iSuicides++;
			}

			// Teamkill
			else if (client->cRemoteWorms[client->iLastKiller].getTeam() ==
						client->cRemoteWorms[client->iLastVictim].getTeam())  {
				l_kill->iTeamKills++;
				l_vict->iTeamDeaths++;
			}
		}
	}

	// Reset
	client->iLastVictim = client->iLastKiller = -1;
}


///////////////////
// Parse a 'server left' packet
void CClientNetEngine::ParseServerLeaving(CBytestream *bs)
{
	// Set the server error details

	if (tGameInfo.iGameType != GME_JOIN)  {
		printf("WARNING: got local server leaving packet, ignoring...\n");
		return;
	}
	// Not so much an error, but rather a disconnection of communication between us & server
	client->bServerError = true;
	client->strServerErrorMsg = "Server has quit";

	if (tLXOptions->bLogConvos)  {
		if(!client->bInServer)
			return;

		FILE *f;

		f = OpenGameFile("Conversations.log","a");
		if (!f)
			return;
		fputs("  </server>\r\n",f);
		client->bInServer = false;
		fclose(f);
	}

	NotifyUserOnEvent();

}


///////////////////
// Parse a 'single shot' packet
void CClientNetEngine::ParseSingleShot(CBytestream *bs)
{
	if(client->iNetStatus != NET_PLAYING || client->bGameOver)  {
		printf("CClientNetEngine::ParseSingleShot: not playing - ignoring\n");
		CShootList::skipSingle(bs); // Skip to get to the correct position
		return;
	}

	client->cShootList.readSingle(bs, client->cGameScript.get()->GetNumWeapons() - 1);

	// Process the shots
	client->ProcessServerShotList();

}


///////////////////
// Parse a 'multi shot' packet
void CClientNetEngine::ParseMultiShot(CBytestream *bs)
{
	if(client->iNetStatus != NET_PLAYING || client->bGameOver)  {
		printf("CClientNetEngine::ParseMultiShot: not playing - ignoring\n");
		CShootList::skipMulti(bs); // Skip to get to the correct position
		return;
	}

	client->cShootList.readMulti(bs, client->cGameScript.get()->GetNumWeapons() - 1);

	// Process the shots
	client->ProcessServerShotList();
}


///////////////////
// Update the worms stats
void CClientNetEngine::ParseUpdateStats(CBytestream *bs)
{
	byte num = bs->readByte();
	if (num > MAX_PLAYERS)
		printf("CClientNetEngine::ParseUpdateStats: invalid worm count ("+itoa(num)+") - clamping\n");

	short oldnum = num;
	num = (byte)MIN(num,MAX_PLAYERS);

	short i;
	for(i=0; i<num; i++)
		if (client->getWorm(i))  {
			if (client->getWorm(i)->getLocal())
				client->bShouldRepaintInfo = true;

			client->getWorm(i)->readStatUpdate(bs);
		}

	// Skip if there were some clamped worms
	for (i=0;i<oldnum-num;i++)
		if (CWorm::skipStatUpdate(bs))
			break;
}


///////////////////
// Parse a 'destroy bonus' packet
void CClientNetEngine::ParseDestroyBonus(CBytestream *bs)
{
	byte id = bs->readByte();

	if (client->iNetStatus != NET_PLAYING)  {
		printf("CClientNetEngine::ParseDestroyBonus: Ignoring, the game is not running.\n");
		return;
	}

	if( id < MAX_BONUSES )
		client->cBonuses[id].setUsed(false);
	else
		printf("CClientNetEngine::ParseDestroyBonus: invalid bonus ID ("+itoa(id)+")\n");
}


///////////////////
// Parse a 'goto lobby' packet
void CClientNetEngine::ParseGotoLobby(CBytestream *)
{
	printf("Client: received gotoLobby signal\n");

	if (tGameInfo.iGameType != GME_JOIN)  {
		if (!tLX->bQuitEngine)  {
			printf("WARNING: we should go to lobby but should not quit the game, ignoring game over signal\n");
			return;
		}
	}

	// in lobby we need the events again
	AddSocketToNotifierGroup( client->tSocket );

	// Do a minor clean up
	client->MinorClear();

	// Hide the console
	Con_Hide();

	DeprecatedGUI::Menu_FloatingOptionsShutdown();


	if(tGameInfo.iGameType == GME_JOIN) {

		// Tell server my worms aren't ready
		CBytestream bs;
		bs.Clear();
		bs.writeByte(C2S_UPDATELOBBY);
		bs.writeByte(0);
		client->cNetChan->AddReliablePacketToSend(bs);


		// Goto the join lobby
		DeprecatedGUI::Menu_Net_GotoJoinLobby();
	}

	client->ShutdownLog();

}


///////////////////
// Parse a 'dropped' packet
void CClientNetEngine::ParseDropped(CBytestream *bs)
{
    // Set the server error details

	// Ignore if we are hosting/local, it's a nonsense
	if (tGameInfo.iGameType != GME_JOIN)  {
		printf("WARNING: got dropped from local server, ignoring\n");
		return;
	}

	// Not so much an error, but a message why we were dropped
	client->bServerError = true;
	client->strServerErrorMsg = Utf8String(bs->readString());

	if (tLXOptions->bLogConvos)  {
		if(!client->bInServer)
			return;

		FILE *f;

		f = OpenGameFile("Conversations.log","a");
		if (!f)
			return;
		fputs("    <message type=\"NETWORK\" text=\"",f);
		fputs(client->strServerErrorMsg.c_str(),f);
		fputs("\" />",f);
		fputs("  </server>\r\n",f);
		client->bInServer = false;
		fclose(f);
	}

}

// Server sent us some file
void CClientNetEngine::ParseSendFile(CBytestream *bs)
{

	client->fLastFileRequestPacketReceived = tLX->fCurTime;
	if( client->getUdpFileDownloader()->receive(bs) )
	{
		if( CUdpFileDownloader::isPathValid( client->getUdpFileDownloader()->getFilename() ) &&
			! IsFileAvailable( client->getUdpFileDownloader()->getFilename() ) &&
			client->getUdpFileDownloader()->isFinished() )
		{
			// Server sent us some file we don't have - okay, save it
			FILE * ff=OpenGameFile( client->getUdpFileDownloader()->getFilename(), "wb" );
			if( ff == NULL )
			{
				printf("CClientNetEngine::ParseSendFile(): cannot write file %s\n", client->getUdpFileDownloader()->getFilename().c_str() );
				return;
			};
			fwrite( client->getUdpFileDownloader()->getData().c_str(), 1, client->getUdpFileDownloader()->getData().size(), ff );
			fclose(ff);

			if( client->getUdpFileDownloader()->getFilename().find("levels/") == 0 &&
					IsFileAvailable( "levels/" + client->tGameLobby.szMapFile ) )
			{
				client->bDownloadingMap = false;
				client->bWaitingForMap = false;
				client->FinishMapDownloads();
				client->sMapDownloadName = "";

				DeprecatedGUI::bJoin_Update = true;
				DeprecatedGUI::bHost_Update = true;
			};
			if( client->getUdpFileDownloader()->getFilename().find("skins/") == 0 )
			{
				// Loads skin from disk automatically on next frame
				DeprecatedGUI::bJoin_Update = true;
				DeprecatedGUI::bHost_Update = true;
			};
			if( ! client->tGameLobby.bHaveMod &&
				client->getUdpFileDownloader()->getFilename().find( client->tGameLobby.szModDir ) == 0 &&
				IsFileAvailable(client->tGameLobby.szModDir + "/script.lgs", false) )
			{
				client->bDownloadingMod = false;
				client->bWaitingForMod = false;
				client->FinishModDownloads();
				client->sModDownloadName = "";

				DeprecatedGUI::bJoin_Update = true;
				DeprecatedGUI::bHost_Update = true;
			};

			client->getUdpFileDownloader()->requestFilesPending(); // Immediately request another file
			client->fLastFileRequest = tLX->fCurTime;

		}
		else
		if( client->getUdpFileDownloader()->getFilename() == "STAT_ACK:" &&
			client->getUdpFileDownloader()->getFileInfo().size() > 0 &&
			! client->tGameLobby.bHaveMod &&
			client->getUdpFileDownloader()->isFinished() )
		{
			// Got filenames list of mod dir - push "script.lgs" to the end of list to download all other data before
			uint f;
			for( f=0; f<client->getUdpFileDownloader()->getFileInfo().size(); f++ )
			{
				if( client->getUdpFileDownloader()->getFileInfo()[f].filename.find( client->tGameLobby.szModDir ) == 0 &&
					! IsFileAvailable( client->getUdpFileDownloader()->getFileInfo()[f].filename ) &&
					stringcaserfind( client->getUdpFileDownloader()->getFileInfo()[f].filename, "/script.lgs" ) != std::string::npos )
				{
					client->getUdpFileDownloader()->requestFile( client->getUdpFileDownloader()->getFileInfo()[f].filename, true );
					client->fLastFileRequest = tLX->fCurTime + 1.5f;	// Small delay so server will be able to send all the info
					client->iModDownloadingSize = client->getUdpFileDownloader()->getFilesPendingSize();
				}
			};
			for( f=0; f<client->getUdpFileDownloader()->getFileInfo().size(); f++ )
			{
				if( client->getUdpFileDownloader()->getFileInfo()[f].filename.find( client->tGameLobby.szModDir ) == 0 &&
					! IsFileAvailable( client->getUdpFileDownloader()->getFileInfo()[f].filename ) &&
					stringcaserfind( client->getUdpFileDownloader()->getFileInfo()[f].filename, "/script.lgs" ) == std::string::npos )
				{
					client->getUdpFileDownloader()->requestFile( client->getUdpFileDownloader()->getFileInfo()[f].filename, true );
					client->fLastFileRequest = tLX->fCurTime + 1.5f;	// Small delay so server will be able to send all the info
					client->iModDownloadingSize = client->getUdpFileDownloader()->getFilesPendingSize();
				}
			};
		};
	};
	if( client->getUdpFileDownloader()->isReceiving() )
	{
		// TODO: move this out here
		// Speed up download - server will send next packet when receives ping, or once in 0.5 seconds
		CBytestream bs;
		bs.writeByte(C2S_SENDFILE);
		client->getUdpFileDownloader()->sendPing( &bs );
		client->cNetChan->AddReliablePacketToSend(bs);
	};
};

