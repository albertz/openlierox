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



#include <cassert>
#include <time.h>

#include "LieroX.h"
#include "Cache.h"
#include "CClient.h"
#include "CServer.h"
#include "OLXConsole.h"
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
#include "DeprecatedGUI/Menu.h"
#include "DeprecatedGUI/CBrowser.h"
#include "ProfileSystem.h"
#include "IRC.h"
#include "CWormHuman.h"
#include "CWormBot.h"
#include "Debug.h"
#include "CGameMode.h"
#include "ConversationLogger.h"
#include "FlagInfo.h"
#include "CMap.h"
#include "Utils.h"
#include "gusanos/network.h"
#include "game/Game.h"
#include "game/Mod.h"
#include "game/SinglePlayer.h"
#include "sound/SoundsBase.h"


#ifdef _MSC_VER
#undef min
#undef max
#endif

// declare them only locally here as nobody really should use them explicitly
std::string Utf8String(const std::string &OldLxString);

static Logger DebugNetLogger(0,2,1000, "DbgNet: ");
static bool Debug_Net_ClPrintFullStream = false;
static bool Debug_Net_ClConnLess = false;
static bool Debug_Net_ClConn = false;

static bool bRegisteredNetDebugVars = CScriptableVars::RegisterVars("Debug.Net")
( DebugNetLogger.minCoutVerb, "LoggerCoutVerb" )
( DebugNetLogger.minIngameConVerb, "LoggerIngameVerb" )
( Debug_Net_ClPrintFullStream, "ClPrintFullStream" )
( Debug_Net_ClConnLess, "ClConnLess" )
( Debug_Net_ClConn, "ClConn" );


///////////////////
// Parse a connectionless packet
void CClientNetEngine::ParseConnectionlessPacket(CBytestream *bs)
{
	size_t s1 = bs->GetPos();
	std::string cmd = bs->readString(128);
	size_t s2 = bs->GetPos();
	
	if(Debug_Net_ClConnLess)
		DebugNetLogger << "ConnectionLessPacket: { '" << cmd << "', restlen: " << bs->GetRestLen() << endl;
	
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
			notes << "CClientNetEngine::ParseConnectionlessPacket: already connected, ignoring" << endl;
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
				notes << "host is at least using OLX Beta3" << endl;
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
			notes << "Host is using " << versionStr << endl;
		}
	}

	else if (cmd == "lx:strafingAllowed")
		client->bHostAllowsStrafing = true;

	else if(cmd == "lx::traverse")
		ParseTraverse(bs);

	else if(cmd == "lx::connect_here")
		ParseConnectHere(bs);
	
	// ignore this (we get it very often if we have hosted once - it's a server package)
	else if(cmd == "lx::ping") {}
	// ignore this also, it's sent by old servers
	else if(cmd == "lx:mouseAllowed") {}
	
	// Unknown
	else  {
		warnings << "CClientNetEngine::ParseConnectionlessPacket: unknown command \"" << cmd << "\"" << endl;
		bs->SkipAll(); // Safety: ignore any data behind this unknown packet
	}
	
	if(Debug_Net_ClConnLess) {
		if(Debug_Net_ClPrintFullStream)
			bs->Dump(PrintOnLogger(DebugNetLogger), Set(s1, s2, bs->GetPos()));
		else
			bs->Dump(PrintOnLogger(DebugNetLogger), Set(s2 - s1), s1, bs->GetPos() - s1);
		DebugNetLogger << "}" << endl;
	}
}


///////////////////
// Parse a challenge packet
void CClientNetEngine::ParseChallenge(CBytestream *bs)
{
	// If we are already connected, ignore this
	if (client->iNetStatus == NET_CONNECTED || client->iNetStatus == NET_PLAYING)  {
		notes << "CClientNetEngine::ParseChallenge: already connected, ignoring" << endl;
		return;
	}

#ifdef DEBUG
	if (client->bConnectingBehindNat)
		notes << "Got a challenge from the server." << endl;
#endif

	CBytestream bytestr;
	bytestr.Clear();
	client->iChallenge = bs->readInt(4);
	if( ! bs->isPosAtEnd() ) {
		client->setServerVersion( bs->readString(128) );
		notes << "CClient: got challenge response from " << client->getServerVersion().asString() << " server" << endl;
	} else
		notes << "CClient: got challenge response from old (<= OLX beta3) server" << endl;

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

	client->tSocket->reapplyRemoteAddress();
	bytestr.Send(client->tSocket.get());

	client->setNetEngineFromServerVersion(); // *this may be deleted here! so it's the last command
}


///////////////////
// Parse a connected packet
void CClientNetEngine::ParseConnected(CBytestream *bs)
{
	if(client->reconnectingAmount >= 2) {
		// This is needed because only the last connect-package will have the correct worm-amount
		client->reconnectingAmount--;
		notes << "ParseConnected: We are waiting for " << client->reconnectingAmount << " other reconnect, ignoring this one now" << endl;
		bs->SkipAll(); // the next connect should be in a seperate UDP package
		return;
	}
	
	bool isReconnect = client->reconnectingAmount > 0;
	if(isReconnect) client->reconnectingAmount--;
	
	NetworkAddr addr;

	if(!isReconnect) {
		if(tLX->iGameType == GME_JOIN) {
			// If already connected, ignore this
			if (client->iNetStatus == NET_CONNECTED)  {
				notes << "CClientNetEngine::ParseConnected: already connected but server received our connect-package twice and we could have other worm-ids" << endl;
			}
			else if(client->iNetStatus == NET_PLAYING) {
				warnings << "CClientNetEngine::ParseConnected: currently playing; ";
				warnings << "it's too risky to proceed a reconnection, so we ignore this" << endl;
				return;
			}
		}
		else { // hosting and no official reconnect
			if (client->iNetStatus != NET_CONNECTING)  {
				warnings << "ParseConnected: local client is not supposed to reconnect right now";
				warnings << " (state: " << NetStateString((ClientNetState)client->iNetStatus) << ")" << endl;
				bs->Dump();
				return;
			}
		}
	} else
		notes << "ParseConnected: reconnected" << endl;

	// Setup the client
	client->iNetStatus = NET_CONNECTED;
	
	// small cleanup (needed for example for reconnecting)
	/*for(int i = 0; i < MAX_WORMS; i++) {
		client->cRemoteWorms[i].setUsed(false);
		client->cRemoteWorms[i].setAlive(false);
		client->cRemoteWorms[i].setKills(0);
		client->cRemoteWorms[i].setDeaths(0);
		client->cRemoteWorms[i].setTeamkills(0);
		client->cRemoteWorms[i].setLives(WRM_OUT);
		client->cRemoteWorms[i].setProfile(NULL);
		client->cRemoteWorms[i].setType(PRF_HUMAN);
		client->cRemoteWorms[i].setLocal(false);
		client->cRemoteWorms[i].setTagIT(false);
		client->cRemoteWorms[i].setTagTime(TimeDiff(0));
		client->cRemoteWorms[i].setClientVersion(Version());
	}*/

	if( client->iNumWorms < 0 ) {
		errors << "client->iNumWorms = " << client->iNumWorms << " is less than zero" << endl;
		client->Disconnect();
		return;
	}
	
	
	
	// Get the id's
	int id=0;
	for(ushort i=0;i<client->iNumWorms;i++) {
		id = bs->readInt(1);
		if (id < 0 || id >= MAX_WORMS) {
			warnings << "ParseConnected: parsed invalid id " << id << endl;
			notes << "Something is screwed up -> reconnecting" << endl;
			client->Reconnect();
			return;
		}
		for(int j = 0; j < i; j++) {
			if(client->cLocalWorms[j]->getID() == id) {
				warnings << "ParseConnected: local worm nr " << j << " already has the id " << id;
				warnings << ", cannot assign it twice to local worm nr " << i << endl;
				notes << "Something is screwed up -> reconnecting" << endl;
				client->Reconnect();
				return;
			}
		}
		if(client->cRemoteWorms[id].isUsed() && !client->cRemoteWorms[id].getLocal()) {
			warnings << "ParseConnected: worm " << id << " is a remote worm";
			warnings << ", cannot be used as our local worm " << i << endl;
			notes << "Something is screwed up -> reconnecting" << endl;
			client->Reconnect();
			return;
		}
		client->cLocalWorms[i] = &client->cRemoteWorms[id];
		if(!client->cLocalWorms[i]->isUsed()) {
			client->cLocalWorms[i]->Clear();
			client->cLocalWorms[i]->setID(id);
			client->cLocalWorms[i]->setUsed(true);
			client->cLocalWorms[i]->setClient(NULL); // Local worms won't get CServerConnection owner
			client->cLocalWorms[i]->setProfile(client->tProfiles[i]);
			if(client->tProfiles[i]) {
				client->cLocalWorms[i]->setTeam(client->tProfiles[i]->iTeam);
				client->cLocalWorms[i]->setType(WormType::fromInt(client->tProfiles[i]->iType));
			} else
				warnings << "ParseConnected: profile " << i << " for worm " << id << " is not set" << endl;
			client->cLocalWorms[i]->setLocal(true);
			client->cLocalWorms[i]->setClientVersion(client->getClientVersion());
		}
		if(!client->cLocalWorms[i]->getLocal()) {
			warnings << "ParseConnected: already used local worm " << id << " was not set to local" << endl;
			client->cLocalWorms[i]->setLocal(true);
		}
	}

	// TODO: why do we setup the viewports only if we have at least one worm?
	if(!isReconnect && !bDedicated && client->iNumWorms > 0) {
		// Setup the viewports
		client->SetupViewports();

		client->SetupGameInputs();
	}

	// Create my channel
	addr = client->tSocket->remoteAddress();

	if( isReconnect && !client->cNetChan )
		warnings << "we are reconnecting but we don't have a working communication channel yet" << endl;
	if( !isReconnect && client->cNetChan ) {
		warnings << "ParseConnected: we still have an old channel" << endl;
		delete client->cNetChan;
		client->cNetChan = NULL;
	}
	
	if( client->cNetChan == NULL ) {
		if( ! client->createChannel( std::min( client->getServerVersion(), GetGameVersion() ) ) )
		{
			client->bServerError = true;
			client->strServerErrorMsg = "Your client is incompatible to this server";
			return;
		}
		client->cNetChan->Create(addr, client->tSocket);
	}
	
	if( client->getServerVersion().isBanned() )
	{
		// There is no Beta9 release, everything that reports itself as Beta9 
		// is pre-release and have incompatible net protocol
		
		notes << "Banned server version " << client->getServerVersion().asString() << " detected - it is not supported anymore" << endl;
		
		client->bServerError = true;
		client->strServerErrorMsg = "This server uses " + client->getServerVersion().asString() + " which is not supported anymore";
		
		return;
	}

	DeprecatedGUI::bJoin_Update = true;
	DeprecatedGUI::bHost_Update = true;

	if(!isReconnect) {
		client->bHostAllowsStrafing = false;
	}
	
	if(tLX->iGameType == GME_JOIN)
		// in CServer::StartServer, we call olxHost
		// we must do the same here now in case of client
		network.olxConnect();	
	
	// Log the connecting
	if (!isReconnect && tLXOptions->bLogConvos && convoLogger)
		convoLogger->enterServer(client->getServerName());
	
	if( !isReconnect && GetGlobalIRC() )
		GetGlobalIRC()->setAwayMessage("Server: " + client->getServerName());
}

//////////////////
// Parse the server's ping reply
void CClientNetEngine::ParsePong()
{
	if (client->fMyPingSent.seconds() > 0)  {
		int png = (int) ((tLX->currentTime-client->fMyPingSent).milliseconds());

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
	float serverTime = bs->readFloat();
	if( serverTime > 0.0 )
	{
		TimeDiff time = TimeDiff(serverTime);
		if (time > client->fServertime)
			client->fServertime = time;
	}

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

	notes << "Client got traverse from " << addr << " port " << port << endl;
}

/////////////////////
// Parse a connect-here packet (when a public-ip client connects to a symmetric-nat server)
void CClientNetEngine::ParseConnectHere(CBytestream *bs)
{
	client->iNatTraverseState = NAT_SEND_CHALLENGE;
	client->iNatTryPort = 0;

	NetworkAddr addr = client->tSocket->remoteAddress();
	std::string a1, a2;
	NetAddrToString( client->cServerAddr, a1 );
	NetAddrToString( addr, a2 );
	notes << "Client got connect_here from " << a1 << " to " << a2 << (a1 != a2 ? "- server behind symmetric NAT" : "") << endl;

	client->cServerAddr = client->tSocket->remoteAddress();
	CBytestream bs1;
	bs1.writeInt(-1,4);
	bs1.writeString("lx::ping");	// So NAT/firewall will understand we really want to connect there
	bs1.Send(client->tSocket.get());
	bs1.Send(client->tSocket.get());
	bs1.Send(client->tSocket.get());
}


/*
=======================================
		Connected Packets
=======================================
*/


///////////////////
// Parse a packet
bool CClientNetEngine::ParsePacket(CBytestream *bs)
{
	if(bs->isPosAtEnd())
		return false;

	size_t s = bs->GetPos();
	uchar cmd = bs->readInt(1);
	
	if(Debug_Net_ClConn)
		DebugNetLogger << "Packet: { '" << (int)cmd << "', restlen: " << bs->GetRestLen() << endl;
	
	bool ret = true;
	
		switch(cmd) {

			// Prepare the game
			case S2C_PREPAREGAME:
				ret &= ParsePrepareGame(bs);
				// HINT: Don't disconnect, we often get here after a corrupted packet because S2C_PREPAREGAME=0 which is a very common value
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

            case S2C_REPORTDAMAGE:
                ParseReportDamage(bs);
                break;

			case S2C_HIDEWORM:
				ParseHideWorm(bs);
				break;
				
			case S2C_NEWNET_KEYS:
				ParseNewNetKeys(bs);
				break;

			case S2C_TEAMSCOREUPDATE:
				ParseTeamScoreUpdate(bs);
				break;
				
			case S2C_FLAGINFO:
				ParseFlagInfo(bs);
				break;
				
			case S2C_SETWORMPROPS:
				ParseWormProps(bs);
				break;
			
			case S2C_SELECTWEAPONS:
				ParseSelectWeapons(bs);
				break;
				
			case S2C_GUSANOS:
				network.olxParse(NetConnID_server(), *bs);
				break;
				
			case S2C_PLAYSOUND:
				ParsePlaySound(bs);
				break;
				
			default:
#if !defined(FUZZY_ERROR_TESTING_S2C)
				warnings << "cl: Unknown packet " << (unsigned)cmd << endl;
#ifdef DEBUG
				// only if debugging sys is disabled because we would print it anyway then
				if(!Debug_Net_ClConn) {
					notes << "Bytestream dump:" << endl;
					// Note: independent from net debugging system because I want to see this in users log even if he didn't enabled the debugging system
					bs->Dump();
					notes << "Done dumping bytestream" << endl;
				}
#endif //DEBUG

#endif //FUZZY_ERROR_TESTING
				ret = false;
		}

	if(Debug_Net_ClConn) {
		if(Debug_Net_ClPrintFullStream)
			bs->Dump(PrintOnLogger(DebugNetLogger), Set(s, bs->GetPos()));
		else
			bs->Dump(PrintOnLogger(DebugNetLogger), std::set<size_t>(), s, bs->GetPos() - s);
		DebugNetLogger << "}" << endl;
	}
	
	return ret;
}


///////////////////
// Parse a prepare game packet
bool CClientNetEngine::ParsePrepareGame(CBytestream *bs)
{
	notes << "Client: Got ParsePrepareGame" << endl;

	bool isReconnect = false;
	
	if(Warning_QuitEngineFlagSet("CClientNetEngine::ParsePrepareGame: ")) {
		warnings << "some previous action tried to quit the GameLoop; we are ignoring this now" << endl;
		ResetQuitEngineFlag();
	}

	// We've already got this packet
	if (client->bGameReady && !client->bGameOver)  {
		((tLX->iGameType == GME_JOIN) ? warnings : notes)
			<< "CClientNetEngine::ParsePrepareGame: we already got this" << endl;
		
		// HINT: we ignore it here for the safety because S2C_PREPAREGAME is 0 and it is
		// a very common value in corrupted streams
		// TODO: skip to the right position, the packet could be valid
		if( tLX->iGameType == GME_JOIN )
			return false;
		isReconnect = true;
	}

	// If we're playing, the game has to be ready
	if (client->iNetStatus == NET_PLAYING && !client->bGameOver)  {
		((tLX->iGameType == GME_JOIN) ? warnings : notes)
			<< "CClientNetEngine::ParsePrepareGame: playing, already had to get this" << endl;
		client->bGameReady = true;

		// The same comment here as above.
		// TODO: skip to the right position, the packet could be valid
		if( tLX->iGameType == GME_JOIN )
			return false;
		isReconnect = true;
	}

	if(!isReconnect)
		NotifyUserOnEvent();

	if(!isReconnect) {
		client->bGameRunning = false; // wait for ParseStartGame
		client->bGameOver = false;
	}
	
	// remove from notifier; we don't want events anymore, we have a fixed FPS rate ingame
	if(!isReconnect)
		client->tSocket->setWithEvents(false);

	client->bGameReady = true;
	client->flagInfo()->reset();
	for(int i = 0; i < 4; ++i) {
		client->iTeamScores[i] = 0;
	}
	
	int random = bs->readInt(1);
	std::string sMapFilename;
	if(!random)
		sMapFilename = bs->readString();
	
	// Other game details
	client->tGameInfo.iGeneralGameType = bs->readInt(1);
	if( client->tGameInfo.iGeneralGameType > GMT_MAX || client->tGameInfo.iGeneralGameType < 0 )
		client->tGameInfo.iGeneralGameType = GMT_NORMAL;
	client->tGameInfo.gameMode = NULL;
	client->tGameInfo.sGameMode = guessGeneralGameTypeName(client->tGameInfo.iGeneralGameType);

	client->tGameInfo.iLives = bs->readInt16();
	client->tGameInfo.iKillLimit = bs->readInt16();
	client->tGameInfo.fTimeLimit = (float)bs->readInt16();
	client->tGameInfo.iLoadingTime = bs->readInt16();
	client->tGameInfo.bBonusesOn = bs->readBool();
	client->tGameInfo.bShowBonusName = bs->readBool();
	client->fServertime = 0;
	
	// in server mode, server would reset this
	if(game.isClient())
		client->permanentText = "";
	
	if(client->getGeneralGameType() == GMT_TIME)
		client->tGameInfo.iTagLimit = bs->readInt16();

	// Load the gamescript
	client->tGameInfo.sModName = bs->readString();

	// Bad packet
	if (client->tGameInfo.sModName == "")  {
		hints << "CClientNetEngine::ParsePrepareGame: invalid mod name (none)" << endl;
		client->bGameReady = false;
		return false;
	}

	// Clear any previous instances of the map
	if(tLX->iGameType == GME_JOIN) {
		if(client->cMap) {
			client->cMap->Shutdown();
			delete client->cMap;
			client->cMap = NULL;
			cServer->resetMap();
		}
	}

	client->m_flagInfo->reset();
	
	// HINT: gamescript is shut down by the cache

    //bs->Dump();
	
	/*if(!isReconnect)
		PhysicsEngine::Get()->initGame();*/
	
	if(!isReconnect) {
		if(tLX->iGameType == GME_JOIN) {
			client->cGameScript = cCache.GetMod( client->tGameInfo.sModName );
			if( client->cGameScript.get() == NULL )
			{
				client->cGameScript = new CGameScript();
				
				if (client->bDownloadingMod)
					client->bWaitingForMod = true;
				else {
					client->bWaitingForMod = false;
					
					int result = client->cGameScript.get()->Load(client->tGameInfo.sModName);
					cCache.SaveMod( client->tGameInfo.sModName, client->cGameScript );
					if(result != GSE_OK) {
						
						// Show any error messages
						if (tLX->iGameType == GME_JOIN)  {
							FillSurface(DeprecatedGUI::tMenu->bmpBuffer.get(), tLX->clBlack);
							std::string err("Error load game mod: ");
							err += client->tGameInfo.sModName + "\r\nError code: " + itoa(result);
							DeprecatedGUI::Menu_MessageBox("Loading Error", err, DeprecatedGUI::LMB_OK);
							client->bClientError = true;
							
							// Go back to the menu
							GotoNetMenu();
						} else {
							errors << "ParsePrepareGame: load mod error for a local game!" << endl;
						}
						client->bGameReady = false;
						
						errors << "CClientNetEngine::ParsePrepareGame: error loading mod " << client->tGameInfo.sModName << endl;
						return false;
					}
				}
			}
		}
		else { // hosting
			client->cGameScript = cServer->getGameScript();
			if(client->cGameScript.get() == NULL) {
				errors << "ParsePrepareGame: server has mod unset" << endl;
				client->bGameReady = false;
				
				errors << "CClientNetEngine::ParsePrepareGame: error loading mod " << client->tGameInfo.sModName << endl;
				return false;
			}
		}
	}
	

	if(tLX->iGameType == GME_JOIN) {
		client->cMap = new CMap;
		if(client->cMap == NULL) {

			// Disconnect
			client->Disconnect();

			DeprecatedGUI::Menu_MessageBox("Out of memory", "Out of memory when allocating the map.", DeprecatedGUI::LMB_OK);

			client->bGameReady = false;

			errors << "CClientNetEngine::ParsePrepareGame: out of memory when allocating map" << endl;

			return false;
		}
	}

	if(random) 
	{
		// TODO: why don't we support that anymore? and since when?
		// Since I've moved all dynamically allocated datas to smartpointers, don't remember why I've removed that
		hints << "CClientNetEngine::ParsePrepareGame: random map requested, and we do not support these anymore" << endl;
		client->bGameReady = false;
		return false;
	} else
	{
		// Load the map from a file

		// Invalid packet
		if (sMapFilename == "")  {
			hints << "CClientNetEngine::ParsePrepareGame: bad map name (none)" << endl;
			client->bGameReady = false;
			return false;
		}

		if(tLX->iGameType == GME_JOIN) {

			// check if we have level
			if(CMap::GetLevelName(GetBaseFilename(sMapFilename)) == "") {
				client->DownloadMap(GetBaseFilename(sMapFilename));  // Download the map
				// we have bDownloadingMap = true when this was successfull
			}
			
			// If we are downloading a map, wait until it finishes
			if (!client->bDownloadingMap)  {
				client->bWaitingForMap = false;

				client->cMap->SetMinimapDimensions(client->tInterfaceSettings.MiniMapW, client->tInterfaceSettings.MiniMapH);
				if(!client->cMap->Load(sMapFilename)) {
					notes << "CClientNetEngine::ParsePrepareGame: could not load map " << sMapFilename << endl;

					// Show a cannot load level error message
					// If this is a host/local game, something is pretty wrong but if we display the message, things could
					// go even worse
					FillSurface(DeprecatedGUI::tMenu->bmpBuffer.get(), tLX->clBlack);

					DeprecatedGUI::Menu_MessageBox(
						"Loading Error",
						std::string("Could not load the level '") + sMapFilename + "'.\n" + LxGetLastError(),
						DeprecatedGUI::LMB_OK);
					client->bClientError = true;

					// Go back to the menu
					QuittoMenu();

					client->bGameReady = false;

					return false;
				}
			} else {
				client->bWaitingForMap = true;
				notes << "Client: we got PrepareGame but we have to wait first until the download of the map finishes" << endl;
			}
		} else { // GME_HOST
			assert(cServer);

            // Grab the server's copy of the map
			client->cMap = cServer->getMap();
			if (!client->cMap)  {  // Bad packet
				errors << "our server has map unset" << endl;
				client->bGameReady = false;
				return false;
			}
			
			client->cMap->SetMinimapDimensions(client->tInterfaceSettings.MiniMapW, client->tInterfaceSettings.MiniMapH);
			client->bMapGrabbed = true;
		
			if(client->cMap->getFilename() != sMapFilename) {
				errors << "Client (in host mode): we got PrepareGame for map " << sMapFilename << " but server has loaded map " << client->cMap->getFilename() << endl;
				client->bGameReady = false;
				return false;
			}
		}

	}
	
    // Read the weapon restrictions
    client->cWeaponRestrictions.updateList(client->cGameScript.get()->GetWeaponList());
    client->cWeaponRestrictions.readList(bs);
	
	client->projPosMap.clear();
	client->projPosMap.resize(CClient::MapPosIndex( VectorD2<int>(client->cMap->GetWidth(), client->cMap->GetHeight())).index(client->cMap) );
	client->cProjectiles.clear();

	client->tGameInfo.features[FT_GameSpeed] = 1.0f;
	client->bServerChoosesWeapons = false;

	// TODO: Load any other stuff
	client->bGameReady = true;

	if(!isReconnect) {
		// Reset the scoreboard here so it doesn't show kills & lives when waiting for players
		client->InitializeIngameScore(true);

		// Copy the chat text from lobby to ingame chatbox
		if( tLX->iGameType == GME_HOST )
			client->sChat_Text = DeprecatedGUI::Menu_Net_HostLobbyGetText();
		else if( tLX->iGameType == GME_JOIN )
			client->sChat_Text = DeprecatedGUI::Menu_Net_JoinLobbyGetText();

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
	}

	CWorm *w = client->cRemoteWorms;
	int num_worms = 0;
	ushort i;
	for(i=0;i<MAX_WORMS;i++,w++) {
		if(w->isUsed()) {
			num_worms++;

			// (If this is a local game?), we need to reload the worm graphics
			// We do this again because we've only just found out what type of game it is
			// Team games require changing worm colours to match the team colour
			// Inefficient, but i'm not going to redesign stuff for a simple gametype
			w->ChangeGraphics(client->getGeneralGameType());

			if(isReconnect && w->isPrepared())
				continue;
			
			notes << "Client: preparing worm " << i << ":" << w->getName() << " for battle" << endl;

			// Also set some game details
			w->setLives(client->tGameInfo.iLives);
			w->setKills(0);
			w->setDeaths(0);
			w->setTeamkills(0);
			w->setDamage(0);
			w->setHealth(100);
			w->setWeaponsReady(false);

			// Prepare for battle!
			w->Prepare(false);
		}
	}

	// The worms are first prepared here in this function and thus the input handlers where not set before.
	// We have to set the control keys now.
	client->SetupGameInputs();


	// Initialize the worms weapon selection menu & other stuff
	if (!client->bWaitingForMod)
		for(i=0;i<client->iNumWorms;i++) {
			// we already prepared all the worms (cRemoteWorms) above
			if(!client->cLocalWorms[i]->getWeaponsReady())
				client->cLocalWorms[i]->initWeaponSelection();
		}
	
	// Start the game logging
	if(!isReconnect)
		client->StartLogging(num_worms);
	
	if(!isReconnect)
	{
		client->SetupViewports();
		// Init viewports once if we're playing with bot
		if(client->cLocalWorms[0] && client->cLocalWorms[0]->getType() == PRF_COMPUTER)
			client->SetupViewports(client->cLocalWorms[0], NULL, VW_FOLLOW, VW_FOLLOW);
		
		if( ! ( client->cGameScript.get() && client->cGameScript->gusEngineUsed() ) )
			client->cChatList->Setup(0,	client->tInterfaceSettings.ChatBoxX,
										client->tInterfaceSettings.ChatBoxY,
										client->tInterfaceSettings.ChatBoxW,
										client->tInterfaceSettings.ChatBoxH);
		else // Expand chatbox for Gus, looks better
			client->cChatList->Setup(0,	5,
										client->tInterfaceSettings.ChatBoxY,
										client->tInterfaceSettings.ChatBoxW + client->tInterfaceSettings.ChatBoxX - 5,
										client->tInterfaceSettings.ChatBoxH);
	}
	
	client->UpdateScoreboard();
	client->bShouldRepaintInfo = true;

	DeprecatedGUI::bJoin_Update = true;

	if(!isReconnect) {
		if( GetGlobalIRC() )
			GetGlobalIRC()->setAwayMessage("Playing: " + client->getServerName());
	}
	
	for_each_iterator( Feature*, f, Array(featureArray,featureArrayLen()) ) {
		client->tGameInfo.features[f->get()] = f->get()->unsetValue;
	}

    return true;
}

bool CClientNetEngineBeta7::ParsePrepareGame(CBytestream *bs)
{
	if( ! CClientNetEngine::ParsePrepareGame(bs) )
		return false;

	// >=Beta7 is sending this
	client->tGameInfo.features[FT_GameSpeed] = bs->readFloat();
	client->bServerChoosesWeapons = bs->readBool();

    return true;
}

void CClientNetEngineBeta9::ParseFeatureSettings(CBytestream* bs) {
	// FeatureSettings() constructor initializes with default values, and we want here an unset values
	for_each_iterator( Feature*, f, Array(featureArray,featureArrayLen()) ) {
		client->tGameInfo.features[f->get()] = f->get()->unsetValue;  // Clean it up
	}
	client->otherGameInfo.clear();
	int ftC = bs->readInt(2);
	for(int i = 0; i < ftC; ++i) {
		std::string name = bs->readString();
		if(name == "") {
			warnings << "Server gives bad features" << endl;
			bs->SkipAll();
			break;
		}
		std::string humanName = bs->readString();
		ScriptVar_t value; bs->readVar(value);
		bool olderClientsSupported = bs->readBool(); // to be understand as: if feature is unknown to us, it's save to ignore
		Feature* f = featureByName(name);
		// f != NULL -> we know about the feature -> we support it
		if(f && !f->serverSideOnly) {
			// we support the feature
			if(value.type == f->valueType) {
				client->tGameInfo.features[f] = value;
			} else {
				client->tGameInfo.features[f] = f->unsetValue; // fallback, the game is anyway somehow screwed
				if( !olderClientsSupported ) {
					errors << "server setting for feature " << name << " has wrong type " << value.type << endl;
				} else {
					warnings << "server setting for feature " << name << " has wrong type " << value.type << " but it's safe to ignore" << endl;
				}
			}
		} else if(f && f->serverSideOnly) {
			// we support it && serversideonly -> just store it for convenience
			client->otherGameInfo.set(name, humanName, value, FeatureCompatibleSettingList::Feature::FCSL_SUPPORTED);
		} else if(olderClientsSupported) {
			// unknown for us but we support it
			client->otherGameInfo.set(name, humanName, value, FeatureCompatibleSettingList::Feature::FCSL_JUSTUNKNOWN);
		} else {
			// server setting is incompatible with our client
			client->otherGameInfo.set(name, humanName, value, FeatureCompatibleSettingList::Feature::FCSL_INCOMPATIBLE);
		}
	}
}

static void setClientGameMode(CGameMode*& mode, const std::string& modeName) {
	if(game.isServer()) {
		// grab from server
		mode = tLXOptions->tGameInfo.gameMode;
		
		// overwrite in case of single player mode
		// we do this so in case we have a standard mode, the bots can act normal
		if(mode == &singlePlayerGame) {
			mode = singlePlayerGame.standardGameMode;
			if(mode)
				notes << "Playing " << tLXOptions->tGameInfo.gameMode->Name() << " with gamemode " << mode->Name() << endl;
		}
	}
	else
		mode = GameMode( modeName );	
	
	// NOTE: NULL is valid here
}

bool CClientNetEngineBeta9::ParsePrepareGame(CBytestream *bs)
{
	bool isReconnect = client->bGameReady || client->iNetStatus == NET_PLAYING;
	
	if( ! CClientNetEngineBeta7::ParsePrepareGame(bs) )
		return false;

	client->tGameInfo.fTimeLimit = bs->readFloat();
	if(client->tGameInfo.fTimeLimit < 0) client->tGameInfo.fTimeLimit = -1;
	
	ParseFeatureSettings(bs);

	client->tGameInfo.sGameMode = bs->readString();
	setClientGameMode(client->tGameInfo.gameMode, client->tGameInfo.sGameMode);
	
	// TODO: shouldn't this be somewhere in the clear function?
	if(!isReconnect)
		cDamageReport.clear(); // In case something left from prev game

	return true;
}


///////////////////
// Parse a start game packet (means BeginMatch actually)
void CClientNetEngine::ParseStartGame(CBytestream *bs)
{
	// Check that the game is ready
	if (!client->bGameReady)  {
		warnings << "CClientNetEngine::ParseStartGame: cannot start the game because the game is not ready" << endl;
		return;
	}

	if (client->iNetStatus == NET_PLAYING && !client->bGameRunning)  {
		warnings << "Client: got start signal in runnign game with unset gamerunning flag" << endl;
		client->iNetStatus = NET_CONNECTED;
	}
	
	notes << "Client: get BeginMatch signal";
	
	if(client->bGameRunning) {
		notes << ", back to game" << endl;
		client->iNetStatus = NET_PLAYING;
		for(uint i=0;i<client->iNumWorms;i++) {
			if(client->cLocalWorms[i]->getWeaponsReady())
				client->cLocalWorms[i]->StartGame();
		}
		return;
	}
	notes << endl;
	client->bGameRunning = true;
	
	// Already got this
	if (client->iNetStatus == NET_PLAYING)  {
		notes << "CClientNetEngine::ParseStartGame: already playing - ignoring" << endl;
		return;
	}
	
	client->fLastSimulationTime = tLX->currentTime;
	client->iNetStatus = NET_PLAYING;
	client->fServertime = 0;

	// Re-initialize the ingame scoreboard
	client->InitializeIngameScore(false);
	client->bUpdateScore = true;


	// let our worms know that the game starts know
	for(uint i=0;i<client->iNumWorms;i++) {
		client->cLocalWorms[i]->StartGame();
	}

	NotifyUserOnEvent();
	
	client->bShouldRepaintInfo = true;
}

void CClientNetEngineBeta9::ParseStartGame(CBytestream *bs)
{
	CClientNetEngine::ParseStartGame(bs);
	if( client->tGameInfo.features[FT_NewNetEngine] )
	{
		NewNet::StartRound( bs->readInt(4) );
		CClient * cl = client;
		delete cl->cNetEngine; // Warning: deletes *this, so "client" var is inaccessible
		cl->cNetEngine = new CClientNetEngineBeta9NewNet(cl);
	}
}

void CClientNetEngineBeta9NewNet::ParseGotoLobby(CBytestream *bs)
{
	NewNet::EndRound();
	CClientNetEngine::ParseGotoLobby(bs);
	CClient * cl = client;
	delete cl->cNetEngine; // Warning: deletes *this, so "client" var is inaccessible
	cl->cNetEngine = new CClientNetEngineBeta9(cl);
}

void CClientNetEngineBeta9NewNet::ParseNewNetKeys(CBytestream *bs)
{
	int worm = bs->readByte();
	if( worm < 0 || worm >= MAX_WORMS )
	{
		warnings << "CClientNetEngineBeta9NewNet::ParseNewNetKeys(): invalid worm id " << worm << endl;
		bs->Skip( NewNet::NetPacketSize() );
		return;
	}
	NewNet::ReceiveNetPacket( bs, worm );
}

///////////////////
// Parse a spawn worm packet
void CClientNetEngine::ParseSpawnWorm(CBytestream *bs)
{
	int id = bs->readByte();
	int x = bs->readInt(2);
	int y = bs->readInt(2);

	// Check
	if (!client->bGameReady)  {
		warnings << "CClientNetEngine::ParseSpawnWorm: Cannot spawn worm when not playing" << endl;
		return;
	}

	if (!client->cMap) {
		warnings << "CClientNetEngine::ParseSpawnWorm: cMap not set (packet ignored)" << endl;
		return;
	}

	// Is the spawnpoint in the map?
	if (x > (int)client->cMap->GetWidth() || x < 0)  {
		warnings << "CClientNetEngine::ParseSpawnWorm: X-coordinate not in map (" << x << ")" << endl;
		return;
	}
	if (y > (int)client->cMap->GetHeight() || y < 0)  {
		warnings << "CClientNetEngine::ParseSpawnWorm: Y-coordinate not in map (" << y << ")" << endl;
		return;
	}

	if(x == 0 || y == 0) {
		hints << "spawned in strange pos (" << x << "," << y << "), could be a bug" << endl;
	}
	
	CVec p = CVec( (float)x, (float)y );

	if (id < 0 || id >= MAX_PLAYERS)  {
		warnings << "CClientNetEngine::ParseSpawnWorm: invalid ID (" << id << ")" << endl;
		return;
	}

	client->cRemoteWorms[id].Spawn(p);

	client->cMap->CarveHole(SPAWN_HOLESIZE,p,cClient->getGameLobby()->features[FT_InfiniteMap]);

	// Show a spawn entity
	SpawnEntity(ENT_SPAWN,0,p,CVec(0,0),Color(),NULL);

	client->UpdateScoreboard();
	//if (client->cRemoteWorms[id].getLocal()) // We may spectate and watch other worm, so redraw always
	client->bShouldRepaintInfo = true;


	bool both = client->cViewports[1].getUsed();

	// Lock viewport back on local worm, if it was screwed when spectating after death
	if( client->iNumWorms > 0 && !both ) {
		if( client->cLocalWorms[0] == &client->cRemoteWorms[id] && client->cLocalWorms[0]->getType() == PRF_HUMAN )
		{
			client->SetupViewports(client->cLocalWorms[0], NULL, VW_FOLLOW, VW_FOLLOW);
			client->sSpectatorViewportMsg = "";
		}
	}
	if( both )  {
		if (client->cLocalWorms[1] && client->cLocalWorms[1]->getType() == PRF_HUMAN) {
			client->SetupViewports(client->cLocalWorms[0], client->cLocalWorms[1], VW_FOLLOW, VW_FOLLOW);
			client->sSpectatorViewportMsg = "";
		}
		else if (client->cLocalWorms[0]->getType() == PRF_HUMAN) {
			client->SetupViewports(client->cLocalWorms[0], client->cViewports[1].getTarget(), VW_FOLLOW, 
			client->cViewports[1].getType());
			client->sSpectatorViewportMsg = "";
		}
	}
}

void CClientNetEngineBeta9NewNet::ParseSpawnWorm(CBytestream *bs)
{
	/* int id = */ bs->readByte();
	/* int x = */ bs->readInt(2);
	/* int y = */ bs->readInt(2);
	// Skip this info for now, we'll use it later
}

void CClientNetEngineBeta9NewNet::ParseWormDown(CBytestream *bs)
{
	/* byte id = */ bs->readByte();
	// Skip this info for now, we'll use it later
}



///////////////////
// Parse a worm info packet
int CClientNetEngine::ParseWormInfo(CBytestream *bs)
{
	if(bs->GetRestLen() == 0) {
		warnings << "CClientNetEngine::ParseWormInfo: data screwed up" << endl;
		return -1;
	}
	
	int id = bs->readInt(1);

	// Validate the id
	if (id < 0 || id >= MAX_WORMS)  {
		warnings << "CClientNetEngine::ParseWormInfo: invalid ID (" << id << ")" << endl;
		bs->SkipAll(); // data is screwed up
		return -1;
	}

	// A new worm?
	bool newWorm = false;
	if (!client->cRemoteWorms[id].isUsed())  {
		client->cRemoteWorms[id].Clear();
		client->cRemoteWorms[id].setID(id);
		client->cRemoteWorms[id].setUsed(true);
		newWorm = true;
	}
	
	WormJoinInfo wormInfo;
	wormInfo.readInfo(bs);
	wormInfo.applyTo(&client->cRemoteWorms[id]);

	if(newWorm) {
		client->cRemoteWorms[id].setLives((client->tGameInfo.iLives < 0) ? WRM_UNLIM : client->tGameInfo.iLives);
		client->cRemoteWorms[id].setClient(NULL); // Client-sided worms won't have CServerConnection
		client->cRemoteWorms[id].setLocal(false);
		if (client->iNetStatus == NET_PLAYING || client->bGameReady)  {
			client->cRemoteWorms[id].Prepare(false);
		}
		if( client->getServerVersion() < OLXBetaVersion(0,58,1) &&
			! client->cRemoteWorms[id].getLocal() )	// Pre-Beta9 servers won't send us info on other clients version
			client->cRemoteWorms[id].setClientVersion(Version());	// LX56 version
	}

	// Load the worm graphics
	if(!client->cRemoteWorms[id].ChangeGraphics(client->getGeneralGameType())) {
        warnings << "CClientNetEngine::ParseWormInfo(): ChangeGraphics() failed" << endl;
	}

	client->UpdateScoreboard();
	if (client->cRemoteWorms[id].getLocal())
		client->bShouldRepaintInfo = true;

	DeprecatedGUI::bJoin_Update = true;
	DeprecatedGUI::bHost_Update = true;
	return id;
}

int CClientNetEngineBeta9::ParseWormInfo(CBytestream *bs)
{
	int id = CClientNetEngine::ParseWormInfo(bs);
	if( id >= 0 && id < MAX_WORMS )
	{
		Version ver(bs->readString());
		client->cRemoteWorms[id].setClientVersion(ver);
	}
	return id;
}

static CWorm* getWorm(CClient* cl, CBytestream* bs, const std::string& fct, bool (*skipFct)(CBytestream*bs) = NULL) {
	if(bs->GetRestLen() == 0) {
		warnings << fct << ": data screwed up at worm ID" << endl;
		return NULL;
	}
	
	int id = bs->readByte();
	if(id < 0 || id >= MAX_WORMS) {
		warnings << fct << ": worm ID " << id << " is invalid" << endl;
		bs->SkipAll();
		return NULL;
	}
	
	if(cl->getRemoteWorms() == NULL) {
		warnings << fct << ": worms are not initialised" << endl;
		if(skipFct) (*skipFct)(bs);
		return NULL;		
	}
	
	CWorm* w = &cl->getRemoteWorms()[id];
	if(!w->isUsed()) {
		warnings << fct << ": worm ID " << id << " is unused" << endl;
		if(skipFct) (*skipFct)(bs);
		return NULL;
	}
	
	return w;
}

///////////////////
// Parse a worm info packet
void CClientNetEngine::ParseWormWeaponInfo(CBytestream *bs)
{
	CWorm* w = getWorm(client, bs, "CClientNetEngine::ParseWormWeaponInfo", CWorm::skipWeapons);
	if(!w) return;

	//notes << "Client:ParseWormWeaponInfo: ";
	w->readWeapons(bs);

	client->UpdateScoreboard();
	if (w->getLocal())
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

	Color col = tLX->clWhite;
	int	t = client->getNumWorms() == 0 ? 0 : client->cLocalWorms[0]->getTeam();
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
	if(tLX->iGameType == GME_LOCAL) {
		if(type == TXT_NETWORK)
			return;
		if(type != TXT_CHAT)
			col = tLX->clNormalText;
    }

	buf = Utf8String(buf);  // Convert any possible pseudo-UTF8 (old LX compatible) to normal UTF8 string

	// Htmlentity nicks in the message
	CWorm *w = client->getRemoteWorms();
	if (w)  {
		for (int i = 0; i < MAX_WORMS; i++, w++)  {
			if (w->isUsed())
				replace(buf, w->getName(), xmlEntities(w->getName()), buf);
		}
	}

	client->cChatbox.AddText(buf, col, (TXT_TYPE)type, tLX->currentTime);


	// Log the conversation
	if (tLXOptions->bLogConvos && convoLogger)
		convoLogger->logMessage(buf, (TXT_TYPE)type);
}


static std::string getChatText(CClient* client) {
	if(client->getStatus() == NET_PLAYING || client->getGameReady())
		return client->chatterText();
	else if(tLX->iGameType == GME_HOST)
		return DeprecatedGUI::Menu_Net_HostLobbyGetText();
	else if(tLX->iGameType == GME_JOIN)
		return DeprecatedGUI::Menu_Net_JoinLobbyGetText();

	warnings << "WARNING: getChatText(): cannot find chat source" << endl;
	return "";
}

static void setChatText(CClient* client, const std::string& txt) {
	if(client->getStatus() == NET_PLAYING || client->getGameReady()) {
		client->chatterText() = txt;
		client->setChatPos( Utf8StringSize(txt) );
	} else if(tLX->iGameType == GME_HOST) {
		DeprecatedGUI::Menu_Net_HostLobbySetText( txt );
	} else if(tLX->iGameType == GME_JOIN) {
		DeprecatedGUI::Menu_Net_JoinLobbySetText( txt );
	} else
		warnings << "WARNING: setChatText(): cannot find chat source" << endl;
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
		warnings << "ParseChatCompletionList got a too big number of suggestions (" << n << ")" << endl;

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

	client->cChatbox.AddText(posStr, tLX->clNotice, TXT_NOTICE, tLX->currentTime);
}

///////////////////
// Parse AFK packet
void CClientNetEngineBeta7::ParseAFK(CBytestream *bs)
{
	CWorm* w = getWorm(client, bs, "CClientNetEngine::ParseAFK", SkipMult<Skip<2>, SkipString>);
	if(!w) return;
	
	AFK_TYPE afkType = (AFK_TYPE)bs->readByte();
	std::string message = bs->readString(128);
	
	w->setAFK(afkType, message);
}


///////////////////
// Parse a score update packet
void CClientNetEngine::ParseScoreUpdate(CBytestream *bs)
{
	CWorm* w = getWorm(client, bs, "ParseScoreUpdate", Skip<3>);
	if(!w) return;

	log_worm_t *l = client->GetLogWorm(w->getID());

	w->setLives( MAX<int>((int)bs->readInt16(), WRM_UNLIM) );
	w->setKills( MAX(bs->readInt(1), 0) );

	
	if (w->getLocal())
		client->bShouldRepaintInfo = true;

	// Logging
	if (l)  {
		// Check if the stats changed
		bool stats_changed = false;
		if (l->iLives != w->getLives())  {
			l->iLives = w->getLives();
			client->iLastVictim = w->getID();
			stats_changed = true;
		}

		if (l->iKills != w->getScore())  {
			l->iKills = w->getScore();
			client->iLastKiller = w->getID();
			stats_changed = true;
		}

		// If the update was sent but no changes made -> this is a killer that made a teamkill
		// See CServer::ParseDeathPacket for more info
		if (!stats_changed)
			client->iLastKiller = w->getID();
	}

	client->UpdateScoreboard();

	DeprecatedGUI::bJoin_Update = true;
	DeprecatedGUI::bHost_Update = true;
}

void CClientNetEngine::ParseTeamScoreUpdate(CBytestream *bs) {
	warnings << "ParseTeamScoreUpdate: got update from too old " << client->getServerVersion().asString() << " server" << endl;
	bs->SkipAll(); // screwed up
}

void CClientNetEngineBeta9::ParseTeamScoreUpdate(CBytestream *bs) {
	if(client->tGameInfo.iGeneralGameType != GMT_TEAMS)
		warnings << "ParseTeamScoreUpdate: it's not a teamgame" << endl;
	
	bool someTeamScored = false;
	int teamCount = bs->readByte();
	for(int i = 0; i < teamCount; ++i) {
		if(bs->isPosAtEnd()) {
			warnings << "ParseTeamScoreUpdate: network is screwed up" << endl;
			break;
		}
		
		if(i == 4) warnings << "ParseTeamScoreUpdate: cannot handle teamscores for other than the first 4 teams" << endl;
		int score = bs->readInt16();
		if(i < 4) {
			if(score > client->iTeamScores[i]) someTeamScored = true;
			client->iTeamScores[i] = score;
		}
	}
	
	if(someTeamScored && client->tGameInfo.gameMode == GameMode(GM_CTF))
		PlaySoundSample(sfxGame.smpTeamScore.get());
	
	// reorder the list
	client->UpdateScoreboard();
}


///////////////////
// Parse a game over packet
void CClientNetEngine::ParseGameOver(CBytestream *bs)
{
	if(client->getServerVersion() < OLXBetaVersion(0,58,1)) {
		client->iMatchWinner = CLAMP(bs->readInt(1), 0, MAX_PLAYERS - 1);
			
		client->iMatchWinnerTeam = -1;

		// Get the winner team if TDM (old servers send wrong info here, better when we find it out)
		if (client->tGameInfo.iGeneralGameType == GMT_TEAMS)  {

			if (client->tGameInfo.iKillLimit != -1)  {
				client->iMatchWinnerTeam = client->cRemoteWorms[client->iMatchWinner].getTeam();
			} else if (client->tGameInfo.iLives != -2)  {
				for (int i=0; i < MAX_WORMS; i++)  {
					if (client->cRemoteWorms[i].getLives() >= 0)  {
						client->iMatchWinnerTeam = client->cRemoteWorms[i].getTeam();
						break;
					}
				}
			}
		}

		// Older servers send wrong info about tag winner, better if we count it ourself
		if (client->tGameInfo.iGeneralGameType == GMT_TIME)  {
			TimeDiff max = TimeDiff(0);

			for (int i=0; i < MAX_WORMS; i++)  {
				if (client->cRemoteWorms[i].isUsed() && client->cRemoteWorms[i].getTagTime() > max)  {
					max = client->cRemoteWorms[i].getTagTime();
					client->iMatchWinner = i;
				}
			}
		}
	}
	else { // server >=beta9
		client->iMatchWinner = bs->readByte();

		if(client->tGameInfo.iGeneralGameType == GMT_TEAMS)  {
			client->iMatchWinnerTeam = bs->readByte();
			int teamCount = bs->readByte();
			for(int i = 0; i < teamCount; ++i) {
				if(bs->isPosAtEnd()) {
					warnings << "ParseGameOver: network is screwed up" << endl;
					break;
				}
				
				if(i == 4) warnings << "ParseGameOver: cannot handle teamscores for other than the first 4 teams" << endl;
				int score = bs->readInt16();
				if(i < 4) client->iTeamScores[i] = score;
			}
		} else
			client->iMatchWinnerTeam = -1;
	}
	
	// Check
	if (client->bGameOver)  {
		notes << "CClientNetEngine::ParseGameOver: the game is already over, ignoring" << endl;
		return;
	}
	
	
	// Game over
	hints << "Client: the game is over";
	if(client->iMatchWinner >= 0 && client->iMatchWinner < MAX_WORMS) {
		hints << ", the winner is worm " << client->iMatchWinner << ":" << client->cRemoteWorms[client->iMatchWinner].getName();
	}
	if(client->iMatchWinnerTeam >= 0) {
		hints << ", the winning team is team " << client->iMatchWinnerTeam;
	}
	hints << endl;
	client->bGameOver = true;
	client->fGameOverTime = tLX->currentTime;

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
	if (!client->bGameReady)  {
		warnings << "CClientNetEngine::ParseSpawnBonus: Cannot spawn bonus when not playing (packet ignored)" << endl;
		return;
	}

	if (id < 0 || id >= MAX_BONUSES)  {
		warnings << "CClientNetEngine::ParseSpawnBonus: invalid bonus ID (" << id << ")" << endl;
		return;
	}

	if (!client->cMap) { // Weird
		warnings << "CClientNetEngine::ParseSpawnBonus: cMap not set" << endl;
		return;
	}

	if (x > (int)client->cMap->GetWidth() || x < 0)  {
		warnings << "CClientNetEngine::ParseSpawnBonus: X-coordinate not in map (" << x << ")" << endl;
		return;
	}

	if (y > (int)client->cMap->GetHeight() || y < 0)  {
		warnings << "CClientNetEngine::ParseSpawnBonus: Y-coordinate not in map (" << y << ")" << endl;
		return;
	}

	CVec p = CVec( (float)x, (float)y );

	client->cBonuses[id].Spawn(p, type, wpn, client->cGameScript.get());
	client->cMap->CarveHole(SPAWN_HOLESIZE,p,cClient->getGameLobby()->features[FT_InfiniteMap]);

	SpawnEntity(ENT_SPAWN,0,p,CVec(0,0),Color(),NULL);
}


// TODO: Rename this to ParseTimeUpdate (?)
///////////////////
// Parse a tag update packet
void CClientNetEngine::ParseTagUpdate(CBytestream *bs)
{
	if (!client->bGameReady || client->bGameOver)  {
		warnings << "CClientNetEngine::ParseTagUpdate: not playing - ignoring" << endl;
		return;
	}

	CWorm* target = getWorm(client, bs, "ParseTagUpdate", Skip<sizeof(float)>);
	if(!target) return;
	TimeDiff time = TimeDiff(bs->readFloat());

	if (client->tGameInfo.iGeneralGameType != GMT_TIME)  {
		warnings << "CClientNetEngine::ParseTagUpdate: game mode is not tag - ignoring" << endl;
		return;
	}

	// Set all the worms 'tag' property to false
	CWorm *w = client->cRemoteWorms;
	for(int i=0;i<MAX_WORMS;i++,w++) {
		if(w->isUsed())
			w->setTagIT(false);
	}

	// Tag the worm
	target->setTagIT(true);
	target->setTagTime(time);

	// Log it
	log_worm_t *l = client->GetLogWorm(target->getID());
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

	if((numworms < 0 || numworms > MAX_PLAYERS) && tLX->iGameType != GME_LOCAL) {
		// bad packet
		hints << "CClientNetEngine::ParseCLReady: invalid numworms (" << numworms << ")" << endl;
		bs->SkipAll();
		return;
	}


	for(short i=0;i<numworms;i++) {
		if(bs->isPosAtEnd()) {
			hints << "CClientNetEngine::ParseCLReady: package messed up" << endl;
			return;
		}
		
		byte id = bs->readByte();

		if(id >= MAX_WORMS) {
			hints << "CClientNetEngine::ParseCLReady: bad worm ID (" << int(id) << ")" << endl;
			bs->SkipAll();
			return;
		}

		if(!client->cRemoteWorms) {
			warnings << "Client: got CLReady with uninit worms" << endl;
			// Skip the info and if end of packet, just end
			if (CWorm::skipWeapons(bs))	break;
			continue;
		}
		
		CWorm* w = &client->cRemoteWorms[id];

		w->setGameReady(true);

		// Read the weapon info
		//notes << "Client:ParseCLReady: ";
		w->readWeapons(bs);

	}

	client->bUpdateScore = true; // Change the ingame scoreboard
}


///////////////////
// Parse an update-lobby packet, when worms got ready/notready
void CClientNetEngine::ParseUpdateLobby(CBytestream *bs)
{
	int numworms = bs->readByte();
	bool ready = bs->readBool();

	if (numworms < 0 || numworms > MAX_WORMS)  {
		warnings << "CClientNetEngine::ParseUpdateLobby: invalid strange numworms value (" << numworms << ")" << endl;

		// Skip to get the right position in stream
		bs->Skip(numworms);
		bs->Skip(numworms);
		return;
	}

	for(short i=0;i<numworms;i++) {
		byte id = bs->readByte();
        int team = MAX(0,MIN(3,(int)bs->readByte()));

		if( id >= MAX_WORMS) {
			warnings << "CClientNetEngine::ParseUpdateLobby: invalid worm ID (" << id << ")" << endl;
			continue;
		}

		CWorm* w = &client->cRemoteWorms[id];
        if(w) {
			w->setLobbyReady(ready);
			w->setTeam(team);
        }
	}

	// Update lobby
	DeprecatedGUI::bJoin_Update = true;
	DeprecatedGUI::bHost_Update = true;
}

///////////////////
// Parse a worms-out (named 'client-left' before) packet
void CClientNetEngine::ParseWormsOut(CBytestream *bs)
{
	byte numworms = bs->readByte();

	if(numworms < 1 || numworms > MAX_PLAYERS) {
		// bad packet
		hints << "CClientNetEngine::ParseWormsOut: bad numworms count (" << int(numworms) << ")" << endl;

		// Skip to the right position
		bs->Skip(numworms);

		return;
	}


	for(int i=0;i<numworms;i++) {
		byte id = bs->readByte();

		if( id >= MAX_WORMS) {
			hints << "CClientNetEngine::ParseWormsOut: invalid worm ID (" << int(id) << ")" << endl;
			continue;
		}

		CWorm *w = &client->cRemoteWorms[id];
		if(!w->isUsed()) {
			warnings << "ParseWormsOut: worm " << int(id) << " is not used anymore" << endl;
			continue;
		}
		
		if(!w->getLocal()) { // Server kicks local worms using S2C_DROPPED, this packet cannot be used for it

			// Log this
			if (client->tGameLog)  {
				log_worm_t *l = client->GetLogWorm(id);
				if (l)  {
					l->bLeft = true;
					l->fTimeLeft = client->serverTime();
				}
			}
			
			if( NewNet::Active() )
				NewNet::PlayerLeft(id);

			client->RemoveWorm(id);
			
		} else {
			hints << "Warning: server says we've left but that is not true" << endl;
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
	if (count > MAX_WORMS)  {
		hints << "CClientNetEngine::ParseUpdateWorms: invalid worm count (" << count << ")" << endl;

		// Skip to the right position
		for (byte i=0;i<count;i++)  {
			bs->Skip(1);
			CWorm::skipPacketState(bs);
		}

		return;
	}
	
	if(!client->bGameReady || !client->cMap || !client->cMap->isLoaded()) {
		// We could receive an update if we didn't got the preparegame package yet.
		// This is because all the data about the preparegame could be sent in multiple packages
		// and each reliable package can contain a worm update.
		
		// Skip to the right position
		for (byte i=0;i<count;i++)  {
			bs->Skip(1);
			CWorm::skipPacketState(bs);
		}

		return;
	}

	for(byte i=0;i<count;i++) {
		byte id = bs->readByte();

		if (id >= MAX_WORMS)  {
			hints << "CClientNetEngine::ParseUpdateWorms: invalid worm ID (" << id << ")" << endl;
			if (CWorm::skipPacketState(bs))  {  // Skip not to lose the right position
				break;
			}
			continue;
		}

		// TODO: what is with that check? remove if outdated
		/*if (!cRemoteWorms[id].isUsed())  {
			i--;
			continue;
		}*/

		client->cRemoteWorms[id].readPacketState(bs,client->cRemoteWorms);

	}

	DeprecatedGUI::bJoin_Update = true;
	DeprecatedGUI::bHost_Update = true;
}

void CClientNetEngineBeta9NewNet::ParseUpdateWorms(CBytestream *bs)
{
	byte count = bs->readByte();
	// Skip to the right position
	for (byte i=0;i<count;i++)  
	{
		bs->Skip(1);
		CWorm::skipPacketState(bs);
	}
}


///////////////////
// Parse an 'update game lobby' packet
void CClientNetEngine::ParseUpdateLobbyGame(CBytestream *bs)
{
	if (client->iNetStatus != NET_CONNECTED)  {
		notes << "CClientNetEngine::ParseUpdateLobbyGame: not in lobby - ignoring" << endl;

		// Skip to get the right position
		bs->Skip(1);
		bs->SkipString();
		bs->SkipString();
		bs->SkipString();
		bs->Skip(8); // All other info

		return;
	}

	client->tGameInfo.iMaxPlayers = bs->readByte();
	client->tGameInfo.sMapFile = bs->readString();
    client->tGameInfo.sModName = bs->readString();
    client->tGameInfo.sModDir = bs->readString();
	client->tGameInfo.iGeneralGameType = bs->readByte();
	if( client->tGameInfo.iGeneralGameType > GMT_MAX || client->tGameInfo.iGeneralGameType < 0 )
		client->tGameInfo.iGeneralGameType = GMT_NORMAL;
	client->tGameInfo.sGameMode = "";
	client->tGameInfo.gameMode = NULL;
	client->tGameInfo.iLives = bs->readInt16();
	client->tGameInfo.iKillLimit = bs->readInt16();
	client->tGameInfo.fTimeLimit = -100;
	client->tGameInfo.iLoadingTime = bs->readInt16();
    client->tGameInfo.bBonusesOn = bs->readBool();

	client->tGameInfo.features[FT_GameSpeed] = 1.0f;
	client->tGameInfo.bForceRandomWeapons = false;
	client->tGameInfo.bSameWeaponsAsHostWorm = false;

    // Check if we have the level & mod
    client->bHaveMod = true;

    // Does the level file exist
	std::string MapName = CMap::GetLevelName(client->tGameInfo.sMapFile);
    client->bHaveMap = MapName != "";
	
	// Convert the map filename to map name
	if (client->bHaveMap)  {
		client->tGameInfo.sMapName = MapName;
	}

    // Does the 'script.lgs' file exist in the mod dir?
	if(!infoForMod(client->tGameInfo.sModDir).valid)
        client->bHaveMod = false;

	for_each_iterator( Feature*, f, Array(featureArray,featureArrayLen()) ) {
		client->tGameInfo.features[f->get()] = f->get()->unsetValue;
	}
	
	DeprecatedGUI::bJoin_Update = true;
	DeprecatedGUI::bHost_Update = true;
}

void CClientNetEngineBeta7::ParseUpdateLobbyGame(CBytestream *bs)
{
	CClientNetEngine::ParseUpdateLobbyGame(bs);

	client->tGameInfo.features[FT_GameSpeed] = bs->readFloat();
	client->tGameInfo.bForceRandomWeapons = bs->readBool();
	client->tGameInfo.bSameWeaponsAsHostWorm = bs->readBool();
}

void CClientNetEngineBeta9::ParseUpdateLobbyGame(CBytestream *bs)
{
	CClientNetEngineBeta7::ParseUpdateLobbyGame(bs);

	client->tGameInfo.fTimeLimit = bs->readFloat();
	if(client->tGameInfo.fTimeLimit < 0) client->tGameInfo.fTimeLimit = -1;

	ParseFeatureSettings(bs);
	
	client->tGameInfo.sGameMode = bs->readString();
	setClientGameMode(client->tGameInfo.gameMode, client->tGameInfo.sGameMode);
}


///////////////////
// Parse a 'worm down' packet (Worm died)
void CClientNetEngine::ParseWormDown(CBytestream *bs)
{
	// Don't allow anyone to kill us in lobby
	if (!client->bGameReady)  {
		notes << "CClientNetEngine::ParseWormDown: not playing - ignoring" << endl;
		bs->Skip(1);  // ID
		return;
	}

	byte id = bs->readByte();

	if(id < MAX_WORMS) {
		// If the respawn time is 0, the worm can be spawned even before the simulation is done
		// Therefore the check for isAlive in the simulation does not work in all cases
		// Because of that, we unattach the rope here, just to be sure
		if (client->cRemoteWorms[id].getHookedWorm())
			client->cRemoteWorms[id].getHookedWorm()->getNinjaRope()->UnAttachPlayer();  // HINT: hookedWorm is reset here (set to NULL)

		
		client->cRemoteWorms[id].Kill();
		if (client->cRemoteWorms[id].getLocal() && client->cRemoteWorms[id].getType() == PRF_HUMAN)
			client->cRemoteWorms[id].clearInput();

		// Make a death sound
		int s = GetRandomInt(2);
		StartSound( sfxGame.smpDeath[s], client->cRemoteWorms[id].getPos(), client->cRemoteWorms[id].getLocal(), -1, client->cLocalWorms[0]);

		// Spawn some giblets
		CWorm* w = &client->cRemoteWorms[id];

		for(short n=0;n<7;n++)
			SpawnEntity(ENT_GIB,0,w->getPos(),CVec(GetRandomNum()*80,GetRandomNum()*80),Color(),w->getGibimg());

		// Blood
		float amount = 50.0f * ((float)tLXOptions->iBloodAmount / 100.0f);
		for(int i=0;i<amount;i++) {
			float sp = GetRandomNum()*100+50;
			SpawnEntity(ENT_BLOODDROPPER,0,w->getPos(),CVec(GetRandomNum()*sp,GetRandomNum()*sp),Color(128,0,0),NULL);
			SpawnEntity(ENT_BLOOD,0,w->getPos(),CVec(GetRandomNum()*sp,GetRandomNum()*sp),Color(200,0,0),NULL);
			SpawnEntity(ENT_BLOOD,0,w->getPos(),CVec(GetRandomNum()*sp,GetRandomNum()*sp),Color(128,0,0),NULL);
		}
	} else {
		warnings << "CClientNetEngine::ParseWormDown: invalid worm ID (" << id << ")" << endl;
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

	if (tLX->iGameType != GME_JOIN)  {
		warnings << "Got local server leaving packet, ignoring..." << endl;
		return;
	}
	// Not so much an error, but rather a disconnection of communication between us & server
	client->bServerError = true;
	client->strServerErrorMsg = "Server has quit";

	// Log
	if (tLXOptions->bLogConvos && convoLogger)
		convoLogger->leaveServer();

	NotifyUserOnEvent();
	
	if( NewNet::Active() )
		NewNet::EndRound();
}


///////////////////
// Parse a 'single shot' packet
void CClientNetEngine::ParseSingleShot(CBytestream *bs)
{
	if(!client->canSimulate()) {
		if(client->bGameReady)
			notes << "CClientNetEngine::ParseSingleShot: game over - ignoring" << endl;
		CShootList::skipSingle(bs, client->getServerVersion()); // Skip to get to the correct position
		return;
	}

	client->cShootList.readSingle(bs, client->getServerVersion(), client->cGameScript.get()->GetNumWeapons() - 1);

	// Process the shots
	client->ProcessServerShotList();

}


///////////////////
// Parse a 'multi shot' packet
void CClientNetEngine::ParseMultiShot(CBytestream *bs)
{
	if(!client->canSimulate())  {
		if(client->bGameReady)
			notes << "CClientNetEngine::ParseMultiShot: game over - ignoring" << endl;
		CShootList::skipMulti(bs, client->getServerVersion()); // Skip to get to the correct position
		return;
	}

	client->cShootList.readMulti(bs, client->getServerVersion(), client->cGameScript.get()->GetNumWeapons() - 1);

	// Process the shots
	client->ProcessServerShotList();
}


///////////////////
// Update the worms stats
void CClientNetEngine::ParseUpdateStats(CBytestream *bs)
{
	byte num = bs->readByte();
	if (num > MAX_PLAYERS)
		warnings << "CClientNetEngine::ParseUpdateStats: invalid worm count (" << num << ") - clamping" << endl;

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

	if (!client->bGameReady)  {
		warnings << "CClientNetEngine::ParseDestroyBonus: Ignoring, the game is not running." << endl;
		return;
	}

	if( id < MAX_BONUSES )
		client->cBonuses[id].setUsed(false);
	else
		warnings << "CClientNetEngine::ParseDestroyBonus: invalid bonus ID (" << id << ")" << endl;
}


///////////////////
// Parse a 'goto lobby' packet
void CClientNetEngine::ParseGotoLobby(CBytestream *)
{
	notes << "Client: received gotoLobby signal" << endl;

	// TODO: Why did we have that code? In hosting mode, we should always trust the server.
	// Even worse, the check is not fully correct. client->bGameOver means that the game is over.
	/*
	if (tLX->iGameType != GME_JOIN)  {
		if (!tLX->bQuitEngine)  {
			warnings << "we should go to lobby but should not quit the game, ignoring game over signal" << endl;
			return;
		}
	}
	 */
	
	// in lobby we need the events again
	client->tSocket->setWithEvents(true);

	// Do a minor clean up
	client->MinorClear();

	// Hide the console
	Con_Hide();

	DeprecatedGUI::Menu_FloatingOptionsShutdown();


	if(tLX->iGameType == GME_JOIN) {

		// Tell server my worms aren't ready
		CBytestream bs;
		bs.Clear();
		bs.writeByte(C2S_UPDATELOBBY);
		bs.writeByte(0);
		client->cNetChan->AddReliablePacketToSend(bs);

		// Goto the join lobby
		GotoJoinLobby();
	}

	client->ShutdownLog();

	if( GetGlobalIRC() )
		GetGlobalIRC()->setAwayMessage("Server: " + client->getServerName());

}


///////////////////
// Parse a 'dropped' packet
void CClientNetEngine::ParseDropped(CBytestream *bs)
{
    // Set the server error details

	// Ignore if we are hosting/local, it's a nonsense
	if (tLX->iGameType != GME_JOIN)  {
		warnings << "Got dropped from local server (" << bs->readString() << "), ignoring" << endl;
		return;
	}

	// Not so much an error, but a message why we were dropped
	client->bServerError = true;
	client->strServerErrorMsg = Utf8String(bs->readString());

	// Log
	if (tLXOptions->bLogConvos && convoLogger)
		convoLogger->logMessage(client->strServerErrorMsg, TXT_NETWORK);
	
	if( NewNet::Active() )
		NewNet::EndRound();
}

// Server sent us some file
void CClientNetEngine::ParseSendFile(CBytestream *bs)
{

	client->fLastFileRequestPacketReceived = tLX->currentTime;
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
				errors << "CClientNetEngine::ParseSendFile(): cannot write file " << client->getUdpFileDownloader()->getFilename() << endl;
				return;
			};
			fwrite( client->getUdpFileDownloader()->getData().data(), 1, client->getUdpFileDownloader()->getData().size(), ff );
			fclose(ff);

			if( client->getUdpFileDownloader()->getFilename().find("levels/") == 0 &&
					IsFileAvailable( "levels/" + client->tGameInfo.sMapFile ) )
			{
				client->bDownloadingMap = false;
				client->bWaitingForMap = false;
				client->FinishMapDownloads();
				client->sMapDownloadName = "";

				DeprecatedGUI::bJoin_Update = true;
				DeprecatedGUI::bHost_Update = true;
			}
			if( client->getUdpFileDownloader()->getFilename().find("skins/") == 0 )
			{
				// Loads skin from disk automatically on next frame
				DeprecatedGUI::bJoin_Update = true;
				DeprecatedGUI::bHost_Update = true;
			}
			if( ! client->bHaveMod &&
				client->getUdpFileDownloader()->getFilename().find( client->tGameInfo.sModDir ) == 0 &&
				IsFileAvailable(client->tGameInfo.sModDir + "/script.lgs", false) )
			{
				client->bDownloadingMod = false;
				client->bWaitingForMod = false;
				client->FinishModDownloads();
				client->sModDownloadName = "";

				DeprecatedGUI::bJoin_Update = true;
				DeprecatedGUI::bHost_Update = true;
			}

			client->getUdpFileDownloader()->requestFilesPending(); // Immediately request another file
			client->fLastFileRequest = tLX->currentTime;

		}
		else
		if( client->getUdpFileDownloader()->getFilename() == "STAT_ACK:" &&
			client->getUdpFileDownloader()->getFileInfo().size() > 0 &&
			! client->bHaveMod &&
			client->getUdpFileDownloader()->isFinished() )
		{
			// Got filenames list of mod dir - push "script.lgs" to the end of list to download all other data before
			uint f;
			for( f=0; f<client->getUdpFileDownloader()->getFileInfo().size(); f++ )
			{
				if( client->getUdpFileDownloader()->getFileInfo()[f].filename.find( client->tGameInfo.sModDir ) == 0 &&
					! IsFileAvailable( client->getUdpFileDownloader()->getFileInfo()[f].filename ) &&
					stringcaserfind( client->getUdpFileDownloader()->getFileInfo()[f].filename, "/script.lgs" ) != std::string::npos )
				{
					client->getUdpFileDownloader()->requestFile( client->getUdpFileDownloader()->getFileInfo()[f].filename, true );
					client->fLastFileRequest = tLX->currentTime + 1.5f;	// Small delay so server will be able to send all the info
					client->iModDownloadingSize = client->getUdpFileDownloader()->getFilesPendingSize();
				}
			}
			for( f=0; f<client->getUdpFileDownloader()->getFileInfo().size(); f++ )
			{
				if( client->getUdpFileDownloader()->getFileInfo()[f].filename.find( client->tGameInfo.sModDir ) == 0 &&
					! IsFileAvailable( client->getUdpFileDownloader()->getFileInfo()[f].filename ) &&
					stringcaserfind( client->getUdpFileDownloader()->getFileInfo()[f].filename, "/script.lgs" ) == std::string::npos )
				{
					client->getUdpFileDownloader()->requestFile( client->getUdpFileDownloader()->getFileInfo()[f].filename, true );
					client->fLastFileRequest = tLX->currentTime + 1.5f;	// Small delay so server will be able to send all the info
					client->iModDownloadingSize = client->getUdpFileDownloader()->getFilesPendingSize();
				}
			}
		}
	}
	if( client->getUdpFileDownloader()->isReceiving() )
	{
		// Speed up download - server will send next packet when receives ping, or once in 0.5 seconds
		CBytestream bs;
		bs.writeByte(C2S_SENDFILE);
		client->getUdpFileDownloader()->sendPing( &bs );
		client->cNetChan->AddReliablePacketToSend(bs);
	}
}

void CClientNetEngineBeta9::ParseReportDamage(CBytestream *bs)
{
	int id = bs->readByte();
	float damage = bs->readFloat();
	int offenderId = bs->readByte();

	if( !client->bGameReady )
		return;
	if( id < 0 || id >= MAX_WORMS || offenderId < 0 || offenderId >= MAX_WORMS )
		return;
	CWorm *w = & client->getRemoteWorms()[id];
	CWorm *offender = & client->getRemoteWorms()[offenderId];
	
	if( ! w->isUsed() || ! offender->isUsed() )
		return;
	
	w->getDamageReport()[offender->getID()].damage += damage;
	w->getDamageReport()[offender->getID()].lastTime = tLX->currentTime;
	w->injure(damage);	// Calculate correct healthbar
	// Update worm damage count (it gets updated in UPDATESCORE packet, we do local calculations here, but they are wrong if we connected during game)
	//notes << "CClientNetEngineBeta9::ParseReportDamage() offender " << offender->getID() << " dmg " << damage << " victim " << id << endl;
	offender->addDamage( damage, w, client->tGameInfo );
}

void CClientNetEngineBeta9::ParseScoreUpdate(CBytestream *bs)
{
	short id = bs->readInt(1);

	if(id >= 0 && id < MAX_WORMS)  {
		log_worm_t *l = client->GetLogWorm(id);

		client->cRemoteWorms[id].setLives( MAX<int>((int)bs->readInt16(), WRM_UNLIM) );
	
		client->cRemoteWorms[id].setKills( bs->readInt(4) );
		float damage = bs->readFloat();
		if( client->cRemoteWorms[id].getDamage() != damage )
		{
			// Occurs pretty often, don't spam console, still it should be the same on client and server
			//warnings << "CClientNetEngineBeta9::ParseScoreUpdate(): damage for worm " << client->cRemoteWorms[id].getName() << " is " << client->cRemoteWorms[id].getDamage() << " server sent us " << damage << endl;
		}
		client->cRemoteWorms[id].setDamage( damage );

		
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

			if (l->iKills != client->cRemoteWorms[id].getScore())  {
				l->iKills = client->cRemoteWorms[id].getScore();
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
	{
		// do this to get the right position in net stream
		bs->Skip(6);
	}

	client->UpdateScoreboard();

	DeprecatedGUI::bJoin_Update = true;
	DeprecatedGUI::bHost_Update = true;
};

void CClientNetEngineBeta9::ParseHideWorm(CBytestream *bs)
{
	int id = bs->readByte();
	int forworm = bs->readByte();
	bool hide = bs->readBool();
	bool immediate = bs->readBool();  // Immediate hiding (no animation)

	// Check
	if (id < 0 || id >= MAX_WORMS)  {
		errors << "ParseHideWorm: invalid worm ID " << id << endl;
		return;
	}

	// Check
	if (forworm < 0 || forworm >= MAX_WORMS)  {
		errors << "ParseHideWorm: invalid forworm ID " << forworm << endl;
		return;
	}

	// Get the worm
	CWorm *w = client->getRemoteWorms() + id;
	if (!client->getRemoteWorms() || !w->isUsed())  {
		errors << "ParseHideWorm: the worm " << id << " does not exist" << endl;
		return;
	}

	w->Spawn(w->getPos());	// We won't get SpawnWorm packet from H&S server
	if (!hide && !immediate)	// Show sparkles only when worm is discovered, or else we'll know where it has been respawned
		SpawnEntity(ENT_SPAWN,0,w->getPos(),CVec(0,0),Color(),NULL); // Spawn some sparkles, looks good

	// Hide or show the worm
	if (hide)
		w->Hide(forworm, immediate);
	else
		w->Show(forworm, immediate);
}

void CClientNetEngine::ParseFlagInfo(CBytestream* bs) {
	warnings << "Client: got flaginfo from too old " << client->cServerVersion.asString() << " server" << endl;
	bs->SkipAll(); // screwed up
}

void CClientNetEngineBeta9::ParseFlagInfo(CBytestream* bs) {
	if(client->m_flagInfo == NULL) {
		warnings << "Client: Got flaginfo with flaginfo unset" << endl;
		FlagInfo::skipUpdate(bs);
		return;
	}
	
	client->m_flagInfo->readUpdate(bs);
}

void CClientNetEngine::ParseWormProps(CBytestream* bs) {
	warnings << "Client: got worm properties from too old " << client->cServerVersion.asString() << " server" << endl;
	bs->SkipAll(); // screwed up
}

void CClientNetEngineBeta9::ParseWormProps(CBytestream* bs) {
	CWorm* w = getWorm(client, bs, "ParseWormProps", Skip<2*sizeof(float)+1>);
	if(!w) return;

	bs->ResetBitPos();
	bool canUseNinja = bs->readBit();
	bool canAirJump = bs->readBit();
	bs->SkipRestBits(); // WARNING: remove this when we read 8 bits
	float speedFactor = bs->readFloat();
	float damageFactor = bs->readFloat();
	float shieldFactor = bs->readFloat();
		
	w->setSpeedFactor(speedFactor);
	w->setDamageFactor(damageFactor);
	w->setShieldFactor(shieldFactor);
	w->setCanUseNinja(canUseNinja);
	w->setCanAirJump(canAirJump);
}

void CClientNetEngine::ParseSelectWeapons(CBytestream* bs) {
	warnings << "Client: got worm select weapons from too old " << client->cServerVersion.asString() << " server" << endl;
	bs->SkipAll(); // screwed up
}

void CClientNetEngineBeta9::ParseSelectWeapons(CBytestream* bs) {
	CWorm* w = getWorm(client, bs, "ParseSelectWeapons");
	if(!w) return;
	
	w->setWeaponsReady(false);
	if(client->OwnsWorm(w->getID())) {
		notes << "server sends us SelectWeapons for worm " << w->getID() << endl;
		
		if(game.gameScript()->gusEngineUsed()) {
			if(CWormHumanInputHandler* player = dynamic_cast<CWormHumanInputHandler*> (w->inputHandler()))
				// this will restart the weapon selection in most mods
				game.onNewHumanPlayer_Lua(player);
			else
				//notes << "SelectWeapons in Gusanos for bots not supported yet" << endl;
				game.onNewPlayer_Lua(player);
		} else {	
			client->setStatus(NET_CONNECTED); // well, that means that we are in weapon selection...
			client->bReadySent = false;
			w->initWeaponSelection();
		}
	}
}

void CClientNetEngineBeta9::ParsePlaySound(CBytestream* bs) {
	std::string fn = bs->readString();
	// security check
	if(fn.find("..") != std::string::npos) {
		warnings << "ParsePlaySound: filename " << fn << " seems corrupt" << endl;
		return;
	}
	
	PlayGameSound(fn);
}

