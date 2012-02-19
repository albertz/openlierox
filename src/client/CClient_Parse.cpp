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
#include "game/CWorm.h"
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
#include "game/CMap.h"
#include "Utils.h"
#include "gusanos/network.h"
#include "game/Game.h"
#include "game/Mod.h"
#include "game/SinglePlayer.h"
#include "sound/SoundsBase.h"
#include "gusanos/gusgame.h"
#include "ThreadVar.h"
#include "CGameScript.h"
#include "ClientConnectionRequestInfo.h"
#include "util/macros.h"


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


CClientNetEngine::~CClientNetEngine() {}

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

	if(client->connectInfo.get() == NULL) {
		errors << "CClientNetEngine::ParseChallenge: connectInfo is NULL" << endl;
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
	bytestr.writeInt(client->connectInfo->worms.size(), 1);

	// Send my worms info
    //
    // __MUST__ match the layout in CWorm::writeInfo() !!!
    //

	foreach(w, client->connectInfo->worms) {
		// TODO: move this out here
		bytestr.writeString(RemoveSpecialChars((*w)->sName));
		bytestr.writeInt((*w)->iType, 1);
		bytestr.writeInt((*w)->iTeam,1);
		bytestr.writeString((*w)->cSkin.getFileName());
		bytestr.writeInt((*w)->R, 1);
		bytestr.writeInt((*w)->G, 1);
		bytestr.writeInt((*w)->B, 1);
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
	
	if(client->connectInfo.get() == NULL) {
		errors << "ParseConnected: connectInfo is NULL" << endl;
		bs->SkipAll();
		return;
	}
	
	bool isReconnect = client->reconnectingAmount > 0;
	if(isReconnect) client->reconnectingAmount--;
	
	NetworkAddr addr;

	if(!isReconnect) {
		if(game.isClient()) {
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
	
	
	// Get the id's
	for(ushort i=0; i < client->connectInfo->worms.size(); i++) {
		int id = bs->readInt(1);
		if(game.isServer()) continue; // we already initialized them server-side
		if (id < 0 || id >= MAX_WORMS) {
			warnings << "ParseConnected: parsed invalid id " << id << endl;
			notes << "Something is screwed up -> reconnecting" << endl;
			client->Reconnect();
			return;
		}
		if(CWorm* w = game.wormById(id, false)) {
			warnings << "ParseConnected: worm ID " << id << " is already taken by worm " << w->getName() << endl;
			notes << "Something is screwed up -> reconnecting" << endl;
			client->Reconnect();
			return;
		}
		game.createNewWorm(id, true, client->connectInfo->worms[i], client->getClientVersion());
	}

	if(!isReconnect && !bDedicated) {
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
		// We always allow it on servers < 0.57 beta5 because 0.57b5 is the first version
		// which has a setting for allowing/disallowing it.
		client->bHostAllowsStrafing = client->getServerVersion() < OLXBetaVersion(0,57,5);
	}
	
	if(game.isClient())
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
		if (time > game.serverTime())
			game.serverFrame = time.milliseconds() / Game::FixedFrameTime;
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

			case S2C_GUSANOSUPDATE:
				network.olxParseUpdate(NetConnID_server(), *bs);
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



// Cannot think of a better function name. :P
// Before 0.59 beta6, there were many settings which were not in the feature array.
static bool wasNotAFeatureSettingBefore(FeatureIndex i) {
	if(i == FT_Lives ||
	   i == FT_KillLimit ||
	   i == FT_TagLimit ||
	   i == FT_LoadingTime ||
	   i == FT_Bonuses ||
	   i == FT_ShowBonusName ||
	   i == FT_TimeLimit ||
	   i == FT_Map ||
	   i == FT_GameMode ||
	   i == FT_Mod ||
	   i == FT_SettingsPreset ||
	   i == FT_WormGroundSpeed ||
	   i == FT_WormAirSpeed ||
	   i == FT_WormAirFriction ||
	   i == FT_WormGravity ||
	   i == FT_WormJumpForce ||
	   i == FT_RopeMaxLength ||
	   i == FT_RopeRestLength ||
	   i == FT_RopeStrength)
		return true;
	return false;
}


///////////////////
// Parse a prepare game packet
bool CClientNetEngine::ParsePrepareGame(CBytestream *bs)
{
	// TODO: remove that as soon as we do the map loading in a seperate thread
	ScopedBackgroundLoadingAni backgroundLoadingAni(320, 280, 50, 50, Color(128,128,128), Color(64,64,64));
	
	// Note: This is probably the most hacky way of doing this but it should work fine.
	// In the future, it probably will be exactly the other way around:
	// Map/mod loading will be in an extra thread and the main game thread will keep us alive in the meanwhile.
	// WARNING: This code assumes that we don't access cl->cNetChan in the meanwhile. Even when doing this
	// in a clean way, we would need that. In case we need this at some time, the only solution is to make CChannel
	// threadsafe or to add another way to keep us alive (maybe a connection-less ping package with the same effect or so).
	struct TimeoutAvoider {
		struct TimeoutAvoiderAction : Action {
			CChannel* chan;
			SmartPointer< ThreadVar<bool> > scope;
			
			Result handle() {
				int c = 0;
				while(true) {
					{
						ThreadVar<bool>::Reader s(*scope.get());
						if(!s.get()) break;
						
						if(c == 1) {
							// This will kind of keep us alive.
							CBytestream emptyUnreliable;
							chan->Transmit(&emptyUnreliable);
						}
					}
					c++; c %= 10;
					SDL_Delay(100);
				}
				return true;
			}
		};
		
		SmartPointer< ThreadVar<bool> > scope;
		TimeoutAvoider(CChannel* c) {
			scope = new ThreadVar<bool>(true);
			TimeoutAvoiderAction* a = new TimeoutAvoiderAction();
			a->chan = c;
			a->scope = scope;
			threadPool->start(a, "ParsePrepareGame timeout avoider keep-me-alive", true);
		}
		~TimeoutAvoider() { scope->write() = false; }
	};
	TimeoutAvoider timeoutAvoider(client->cNetChan);
	
	
	notes << "Client: Got ParsePrepareGame" << endl;

	bool isReconnect = false;
	
	if(Warning_QuitEngineFlagSet("CClientNetEngine::ParsePrepareGame: ")) {
		warnings << "some previous action tried to quit the GameLoop; we are ignoring this now" << endl;
		ResetQuitEngineFlag();
	}

	// We've already got this packet
	if (game.state >= Game::S_Preparing && !game.gameOver)  {
		((game.isClient()) ? warnings : notes)
			<< "CClientNetEngine::ParsePrepareGame: we already got this" << endl;
		
		// HINT: we ignore it here for the safety because S2C_PREPAREGAME is 0 and it is
		// a very common value in corrupted streams
		// TODO: skip to the right position, the packet could be valid
		if( game.isClient() )
			return false;
		isReconnect = true;
	}

	// If we're playing, the game has to be ready
	if (client->iNetStatus == NET_PLAYING && !game.gameOver)  {
		((game.isClient()) ? warnings : notes)
			<< "CClientNetEngine::ParsePrepareGame: playing, already had to get this" << endl;
		game.state = std::max((int32_t)Game::S_Preparing, (int32_t)game.state);

		// The same comment here as above.
		// TODO: skip to the right position, the packet could be valid
		if( game.isClient() )
			return false;
		isReconnect = true;
	}

	if(!isReconnect)
		NotifyUserOnEvent();

	if(!isReconnect) {
		game.state = Game::S_Preparing; // wait for ParseStartGame
		game.gameOver = false;
	}
	
	// remove from notifier; we don't want events anymore, we have a fixed FPS rate ingame
	if(!isReconnect)
		client->tSocket->setWithEvents(false);

	game.state = Game::S_Preparing;
	client->flagInfo()->reset();
	for(int i = 0; i < MAX_TEAMS; ++i) {
		client->iTeamScores[i] = 0;
	}
	
	int random = bs->readInt(1);
	std::string sMapFilename;
	if(!random) {
		sMapFilename = bs->readString();
		if(game.isClient())
			client->getGameLobby().overwrite[FT_Map] = infoForLevel(GetBaseFilename(sMapFilename));
	}
	
	// Other game details
	if(game.isClient()) {
		client->getGameLobby().overwrite[FT_GameMode] = GameModeInfo::fromNetworkModeInt(bs->readInt(1));
		client->getGameLobby().overwrite[FT_Lives] = bs->readInt16();
		client->getGameLobby().overwrite[FT_KillLimit] = bs->readInt16();
		client->getGameLobby().overwrite[FT_TimeLimit] = (float)bs->readInt16();
		client->getGameLobby().overwrite[FT_LoadingTime] = bs->readInt16();
		client->getGameLobby().overwrite[FT_Bonuses] = bs->readBool();
		client->getGameLobby().overwrite[FT_ShowBonusName] = bs->readBool();
	}
	else
		bs->Skip(11);
	game.serverFrame = 0;
	
	// in server mode, server would reset this
	if(game.isClient())
		client->permanentText = "";
	
	if(client->getGeneralGameType() == GMT_TIME) {
		if(game.isClient())
			client->getGameLobby().overwrite[FT_TagLimit] = (float)bs->readInt16();
		else
			bs->Skip(2);
	}

	// Set the gamescript
	if(game.isClient())
		client->getGameLobby().overwrite[FT_Mod] = infoForMod(bs->readString());
	else
		bs->SkipString();
	
	// Bad packet
	if (client->getGameLobby()[FT_Mod].as<ModInfo>()->name == "")  {
		hints << "CClientNetEngine::ParsePrepareGame: invalid mod name (none)" << endl;
		game.state = Game::S_Lobby;
		return false;
	}

	client->m_flagInfo->reset();
	
	// HINT: gamescript is shut down by the cache

    //bs->Dump();
	
	/*if(!isReconnect)
		PhysicsEngine::Get()->initGame();*/
	
	if(!isReconnect) {
		if(game.isClient()) {
			if (client->bDownloadingMod)
				client->bWaitingForMod = true;
			else {
				client->bWaitingForMod = false;
				
				Result result = game.loadMod();
				if(!result) {
					
					// Show any error messages
					FillSurface(DeprecatedGUI::tMenu->bmpBuffer.get(), tLX->clBlack);
					std::string err = "Error load game mod ";
					err += client->getGameLobby()[FT_Mod].as<ModInfo>()->name;
					err += "\r\n" + result.humanErrorMsg;
					DeprecatedGUI::Menu_MessageBox("Loading Error", err, DeprecatedGUI::LMB_OK);
					client->bClientError = true;
					
					// Go back to the menu
					GotoNetMenu();

					game.state = Game::S_Lobby;
					
					errors << "CClientNetEngine::ParsePrepareGame: error loading mod " << client->getGameLobby()[FT_Mod].as<ModInfo>()->name << endl;
					return false;
				}
			}
		}
	}
	
	if(random) 
	{
		// TODO: why don't we support that anymore? and since when?
		// Since I've moved all dynamically allocated datas to smartpointers, don't remember why I've removed that
		hints << "CClientNetEngine::ParsePrepareGame: random map requested, and we do not support these anymore" << endl;
		game.state = Game::S_Lobby;
		return false;
	} else
	{
		// Load the map from a file

		// Invalid packet
		if (sMapFilename == "")  {
			hints << "CClientNetEngine::ParsePrepareGame: bad map name (none)" << endl;
			game.state = Game::S_Lobby;
			return false;
		}

		if(game.isClient()) {

			// check if we have level
			if(CMap::GetLevelName(GetBaseFilename(sMapFilename)) == "") {
				client->DownloadMap(GetBaseFilename(sMapFilename));  // Download the map
				// we have bDownloadingMap = true when this was successfull
			}
			
			// If we are downloading a map, wait until it finishes
			if (!client->bDownloadingMap)  {
				client->bWaitingForMap = false;

				if(NegResult r = game.loadMap()) {
					notes << "CClientNetEngine::ParsePrepareGame: could not load map " << sMapFilename << endl;

					// Show a cannot load level error message
					// If this is a host/local game, something is pretty wrong but if we display the message, things could
					// go even worse
					FillSurface(DeprecatedGUI::tMenu->bmpBuffer.get(), tLX->clBlack);

					DeprecatedGUI::Menu_MessageBox(
						"Loading Error",
						std::string("Could not load the level '") + sMapFilename + "'.\n" + LxGetLastError() + "\n" + r.res.humanErrorMsg,
						DeprecatedGUI::LMB_OK);
					client->bClientError = true;

					// Go back to the menu
					QuittoMenu();

					game.state = Game::S_Lobby;

					return false;
				}
				
				game.gameMap()->SetMinimapDimensions(client->tInterfaceSettings.MiniMapW, client->tInterfaceSettings.MiniMapH);
			} else {
				client->bWaitingForMap = true;
				notes << "Client: we got PrepareGame but we have to wait first until the download of the map finishes" << endl;
			}
		} else { // GME_HOST
			
			game.gameMap()->SetMinimapDimensions(client->tInterfaceSettings.MiniMapW, client->tInterfaceSettings.MiniMapH);
			client->bMapGrabbed = true;
		
			if(game.gameMap()->getFilename() != sMapFilename) {
				errors << "Client (in host mode): we got PrepareGame for map " << sMapFilename << " but server has loaded map " << game.gameMap()->getFilename() << endl;
				game.state = Game::S_Lobby;
				return false;
			}
		}

	}

	// NOTE: This was moved from Game:prepareGameLoop to here because it is needed *before* we prepare the worms
	
	// always also load Gusanos engine
	// even with LX-stuff-only, we may access/need it (for network stuff and later probably more)
	if( !game.gameMap()->gusIsLoaded() && (game.isServer() || client->getServerVersion() >= OLXBetaVersion(0,59,1) ) ) {
		// WARNING: This may be temporary
		// Right now, we load the gus mod in the map loader (gusGame.changeLevel).
		// Thus, when we don't load a gus level, we must load the mod manually.
		
		if(!game.gameScript()->gusEngineUsed())
			gusGame.setMod(gusGame.getDefaultPath());
		gusGame.loadModWithoutMap();
	}
	
	if(gusGame.isEngineNeeded()) {
		// Unprepare all worms first.
		// Some init scripts (e.g. Promode) have trouble when there are players
		// but their bindings.playerInit was not called.
		// As the bindings.playerInit is firstly set here by the init scripts,
		// there is no other way.
		// The worms will get prepared later on again.
		// TODO: do the preparation client/server independently somewhere else
		// TODO: as well as the gus loading/init stuff
		for_each_iterator(CWorm*, w, game.worms())
			w->get()->Unprepare();

		gusGame.runInitScripts();
	}	
	
    // Read the weapon restrictions
	if(game.isClient())
		game.loadWeaponRestrictions();
    game.weaponRestrictions()->readList(bs);
	
	client->projPosMap.clear();
	client->projPosMap.resize(CClient::MapPosIndex( VectorD2<int>(game.gameMap()->GetWidth(), game.gameMap()->GetHeight())).index(game.gameMap()) );
	client->cProjectiles.clear();

	if(game.isClient()) client->getGameLobby().overwrite[FT_GameSpeed] = 1.0f;
	client->bServerChoosesWeapons = false;

	// TODO: Load any other stuff
	game.state = Game::S_Preparing;

	if(!isReconnect) {
		// Reset the scoreboard here so it doesn't show kills & lives when waiting for players
		client->InitializeIngameScore(true);

		// Copy the chat text from lobby to ingame chatbox
		if( game.isServer() && !game.isLocalGame() )
			client->sChat_Text = DeprecatedGUI::Menu_Net_HostLobbyGetText();
		else if( game.isClient() )
			client->sChat_Text = DeprecatedGUI::Menu_Net_JoinLobbyGetText();

		if (!client->sChat_Text.empty())  {
			client->bChat_Typing = true;
			client->bChat_CursorVisible = true;
			client->iChat_Pos = client->sChat_Text.size();
			if(game.firstLocalHumanWorm())
				SendAFK( game.firstLocalHumanWorm()->getID(), AFK_TYPING_CHAT );
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

	for_each_iterator(CWorm*, w_, game.worms()) {
		CWorm* w = w_->get();
		
		// (If this is a local game?), we need to reload the worm graphics
		// We do this again because we've only just found out what type of game it is
		// Team games require changing worm colours to match the team colour
		// Inefficient, but i'm not going to redesign stuff for a simple gametype
		w->ChangeGraphics(client->getGeneralGameType());

		notes << "Client: preparing worm " << w->getID() << ":" << w->getName() << " for battle" << endl;

		// Also set some game details
		w->setLives(client->getGameLobby()[FT_Lives]);
		w->setKills(0);
		w->setDeaths(0);
		w->setTeamkills(0);
		w->setDamage(0);
		w->setHealth(100);
		w->bWeaponsReady = false;

		// Prepare for battle!
		w->Prepare();
	}

	// The worms are first prepared here in this function and thus the input handlers where not set before.
	// We have to set the control keys now.
	client->SetupGameInputs();


	// Initialize the worms weapon selection menu & other stuff
	if (!client->bWaitingForMod)
		for_each_iterator(CWorm*, w, game.localWorms()) {
			// we already prepared all the worms (cRemoteWorms) above
			if(!w->get()->bWeaponsReady)
				w->get()->initWeaponSelection();
		}
	
	// Start the game logging
	if(!isReconnect)
		client->StartLogging(game.worms()->size());
	
	if(!isReconnect && !bDedicated)
	{
		client->SetupViewports();
		
		if( ! ( game.gameScript() && game.gameScript()->gusEngineUsed() ) )
		{
			client->cChatList->Setup(0,	client->tInterfaceSettings.ChatBoxX,
										client->tInterfaceSettings.ChatBoxY,
										client->tInterfaceSettings.ChatBoxW,
										client->tInterfaceSettings.ChatBoxH);
			client->cChatList->showScrollbar(true);
		}
		else // Expand chatbox for Gus, looks better
		{
			client->cChatList->Setup(0,	5,
										client->tInterfaceSettings.ChatBoxY,
										client->tInterfaceSettings.ChatBoxW + client->tInterfaceSettings.ChatBoxX - 5,
										client->tInterfaceSettings.ChatBoxH);
			client->cChatList->showScrollbar(false);
		}
	}
	
	client->UpdateScoreboard();
	client->bShouldRepaintInfo = true;

	DeprecatedGUI::bJoin_Update = true;

	if(!isReconnect) {
		if( GetGlobalIRC() )
			GetGlobalIRC()->setAwayMessage("Playing: " + client->getServerName());
	}
	
	if(game.isClient())
	for(size_t i = 0; i < FeatureArrayLen; ++i) {
		if(client->getServerVersion() < OLXBetaVersion(0,59,6)) {
			// Before 0.59b6, many settings have been outside of the FT array.
			// So, on those old servers, we handle them seperately and we don't want to reset them here!
			if(wasNotAFeatureSettingBefore((FeatureIndex)i))
				continue;
		}
		
		// certain settings were updated anyway, so don't reset them
		if(i == FT_Map ||
		   i == FT_Mod ||
		   i == FT_GameMode ||
		   i == FT_Lives ||
		   i == FT_KillLimit ||
		   i == FT_TimeLimit ||
		   i == FT_LoadingTime ||
		   i == FT_Bonuses ||
		   i == FT_ShowBonusName)
			continue;
		
		client->getGameLobby().overwrite[(FeatureIndex)i] = featureArray[i].unsetValue;  // Clean it up
	}
	client->otherGameInfo.clear();
	
    return true;
}

bool CClientNetEngineBeta7::ParsePrepareGame(CBytestream *bs)
{
	if( ! CClientNetEngine::ParsePrepareGame(bs) )
		return false;

	if(game.isClient()) {
		// >=Beta7 is sending this
		client->getGameLobby().overwrite[FT_GameSpeed] = bs->readFloat();
		client->bServerChoosesWeapons = bs->readBool();
	}
	else
		bs->Skip(5);
	
    return true;
}


void CClientNetEngineBeta9::ParseFeatureSettings(CBytestream* bs) {
	// FeatureSettings() constructor initializes with default values, and we want here an unset values
	if(game.isClient())
	for(size_t i = 0; i < FeatureArrayLen; ++i) {
		if(client->getServerVersion() < OLXBetaVersion(0,59,6)) {
			// Before 0.59b6, many settings have been outside of the FT array.
			// So, on those old servers, we handle them seperately and we don't want to reset them here!
			if(wasNotAFeatureSettingBefore((FeatureIndex)i))
				continue;
		}
	
		client->getGameLobby().overwrite[(FeatureIndex)i] = featureArray[i].unsetValue;  // Clean it up
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
		Feature* f = featureByName(name);
		std::string humanName = bs->readString();
		
		ScriptVar_t value;
		if(!bs->readVar(value, (f && f->valueType == SVT_CUSTOM) ? &f->unsetValue.ptrCustom().get() : NULL))
			errors << "ParseFeatureSettings: error while reading var " << name << " (" << humanName << ")" << endl;
		bool olderClientsSupported = bs->readBool(); // to be understand as: if feature is unknown to us, it's save to ignore
		
		if(game.isServer()) continue; // ignore

		// f != NULL -> we know about the feature -> we support it
		if(f) {
			// we support the feature
			if(value.type == f->valueType) {
				client->getGameLobby().overwrite[f] = value;
			} else {
				client->getGameLobby().overwrite[f] = f->unsetValue; // fallback, the game is anyway somehow screwed
				if( !olderClientsSupported && !f->serverSideOnly ) {
					errors << "server setting for feature " << name << " has wrong type " << value.type << endl;
				} else {
					warnings << "server setting for feature " << name << " has wrong type " << value.type << " but it's safe to ignore" << endl;
				}
			}
		} else if(olderClientsSupported) {
			// unknown for us but we support it
			client->otherGameInfo.set(name, humanName, value, FeatureCompatibleSettingList::Feature::FCSL_JUSTUNKNOWN);
		} else {
			// server setting is incompatible with our client
			client->otherGameInfo.set(name, humanName, value, FeatureCompatibleSettingList::Feature::FCSL_INCOMPATIBLE);
		}
	}
}

bool CClientNetEngineBeta9::ParsePrepareGame(CBytestream *bs)
{
	bool isReconnect = game.state >= Game::S_Preparing || client->iNetStatus == NET_PLAYING;
	
	if( ! CClientNetEngineBeta7::ParsePrepareGame(bs) )
		return false;

	if(game.isClient()) {
		client->tGameInfo.overwrite[FT_TimeLimit] = bs->readFloat();
		if((float)client->tGameInfo[FT_TimeLimit] < 0) client->tGameInfo.overwrite[FT_TimeLimit] = -1.0f;
	}
	else
		bs->Skip(4);
	
	ParseFeatureSettings(bs);

	if(game.isClient())
		client->getGameLobby().overwrite[FT_GameMode] = client->getGameLobby()[FT_GameMode].as<GameModeInfo>()->withNewName(bs->readString());
	else
		bs->SkipString();
	
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
	if (game.state < Game::S_Preparing)  {
		warnings << "CClientNetEngine::ParseStartGame: cannot start the game because the game is not ready" << endl;
		return;
	}

	if (client->iNetStatus == NET_PLAYING && game.state < Game::S_Playing)  {
		warnings << "Client: got start signal in runnign game with unset gamerunning flag" << endl;
		client->iNetStatus = NET_CONNECTED;
	}
	
	notes << "Client: get BeginMatch signal";
	
	if(game.state == Game::S_Playing) {
		notes << ", back to game" << endl;
		client->iNetStatus = NET_PLAYING;
		for_each_iterator(CWorm*, w, game.localWorms()) {
			if(w->get()->bWeaponsReady)
				w->get()->StartGame();
		}
		return;
	}
	notes << endl;
	game.state = Game::S_Playing;
	
	// Already got this
	if (client->iNetStatus == NET_PLAYING)  {
		notes << "CClientNetEngine::ParseStartGame: already playing - ignoring" << endl;
		return;
	}
	
	client->fLastSimulationTime = tLX->currentTime;
	client->iNetStatus = NET_PLAYING;
	game.serverFrame = 0;

	// Re-initialize the ingame scoreboard
	client->InitializeIngameScore(false);
	client->bUpdateScore = true;


	// let our worms know that the game starts know
	for_each_iterator(CWorm*, w, game.localWorms())
		w->get()->StartGame();

	NotifyUserOnEvent();
	
	client->bShouldRepaintInfo = true;
}

void CClientNetEngineBeta9::ParseStartGame(CBytestream *bs)
{
	CClientNetEngine::ParseStartGame(bs);
	if( (bool)client->getGameLobby()[FT_NewNetEngine] )
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
	if(game.state < Game::S_Preparing)  {
		warnings << "CClientNetEngine::ParseSpawnWorm: Cannot spawn worm when not playing" << endl;
		return;
	}

	if (!game.gameMap()) {
		warnings << "CClientNetEngine::ParseSpawnWorm: game.gameMap() not set (packet ignored)" << endl;
		return;
	}

	// Is the spawnpoint in the map?
	if (x > (int)game.gameMap()->GetWidth() || x < 0)  {
		warnings << "CClientNetEngine::ParseSpawnWorm: X-coordinate not in map (" << x << ")" << endl;
		return;
	}
	if (y > (int)game.gameMap()->GetHeight() || y < 0)  {
		warnings << "CClientNetEngine::ParseSpawnWorm: Y-coordinate not in map (" << y << ")" << endl;
		return;
	}

	if(x == 0 || y == 0) {
		hints << "spawned in strange pos (" << x << "," << y << "), could be a bug" << endl;
	}
	
	CVec p = CVec( (float)x, (float)y );
	CWorm* w = game.wormById(id, false);
	
	if (!w)  {
		warnings << "CClientNetEngine::ParseSpawnWorm: invalid ID (" << id << ")" << endl;
		return;
	}

	w->Spawn(p);

	game.gameMap()->CarveHole(SPAWN_HOLESIZE,p,cClient->getGameLobby()[FT_InfiniteMap]);

	if(client->isWormVisibleOnAnyViewport(id)) {
		// Show a spawn entity but only if worm is not hidden on any of our local viewports
		SpawnEntity(ENT_SPAWN,0,p,CVec(0,0),Color(),NULL);
	}
	
	client->UpdateScoreboard();
	//if (client->cRemoteWorms[id].getLocal()) // We may spectate and watch other worm, so redraw always
	client->bShouldRepaintInfo = true;

	for(int i = 0; i < NUM_VIEWPORTS; ++i) {
		CViewport* v = &client->cViewports[i];
		if(!v->getUsed()) continue;
		if(v->getOrigTarget() != w) continue;
		
		// Lock viewport back on worm, if it was screwed when spectating after death
		client->sSpectatorViewportMsg = "";
		v->setType(VW_FOLLOW);
		v->setTarget(w);
		v->setSmooth(!client->OwnsWorm(id));
		break;
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
	CWorm* w = game.wormById(id, false);
	if (w == NULL)  {
		w = game.createNewWorm(id, false, new profile_t(), Version());
		newWorm = true;
	}
	
	WormJoinInfo wormInfo;
	wormInfo.readInfo(bs);
	wormInfo.applyTo(w);

	if(newWorm) {
		w->setLives(((int32_t)client->getGameLobby()[FT_Lives] < 0) ? (int32_t)WRM_UNLIM : (int32_t)client->getGameLobby()[FT_Lives]);
		w->setClient(NULL); // Client-sided worms won't have CServerConnection
		if (client->iNetStatus == NET_PLAYING || game.state >= Game::S_Preparing) {
			if(game.isClient())
				w->Prepare();
		}
		if( client->getServerVersion() < OLXBetaVersion(0,58,1) &&
			! w->getLocal() )	// Pre-Beta9 servers won't send us info on other clients version
			w->setClientVersion(Version());	// LX56 version
	}

	// Load the worm graphics
	if(!w->ChangeGraphics(client->getGeneralGameType())) {
        warnings << "CClientNetEngine::ParseWormInfo(): ChangeGraphics() failed" << endl;
	}

	client->UpdateScoreboard();
	if (w->getLocal())
		client->bShouldRepaintInfo = true;

	DeprecatedGUI::bJoin_Update = true;
	DeprecatedGUI::bHost_Update = true;
	return id;
}

int CClientNetEngineBeta9::ParseWormInfo(CBytestream *bs)
{
	int id = CClientNetEngine::ParseWormInfo(bs);
	Version ver(bs->readString());
	if(CWorm* w = game.wormById(id, false))
		w->setClientVersion(ver);
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
		
	CWorm* w = game.wormById(id, false);
	if(!w) {
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
	int	t = game.firstLocalHumanWorm() ? game.firstLocalHumanWorm()->getTeam() : 0;
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
	if(game.isLocalGame()) {
		if(type == TXT_NETWORK)
			return;
		if(type != TXT_CHAT)
			col = tLX->clNormalText;
    }

	buf = Utf8String(buf);  // Convert any possible pseudo-UTF8 (old LX compatible) to normal UTF8 string

	// Htmlentity nicks in the message
	for_each_iterator(CWorm*, w, game.worms()) {
		replace(buf, w->get()->getName(), xmlEntities(w->get()->getName()), buf);
	}

	client->cChatbox.AddText(buf, col, (TXT_TYPE)type, tLX->currentTime);


	// Log the conversation
	if (tLXOptions->bLogConvos && convoLogger)
		convoLogger->logMessage(buf, (TXT_TYPE)type);
}


static std::string getChatText(CClient* client) {
	if(client->getStatus() == NET_PLAYING || game.state >= Game::S_Preparing)
		return client->chatterText();
	else if(game.isServer() && !game.isLocalGame())
		return DeprecatedGUI::Menu_Net_HostLobbyGetText();
	else if(game.isClient())
		return DeprecatedGUI::Menu_Net_JoinLobbyGetText();

	warnings << "WARNING: getChatText(): cannot find chat source" << endl;
	return "";
}

static void setChatText(CClient* client, const std::string& txt) {
	if(client->getStatus() == NET_PLAYING || game.state >= Game::S_Preparing) {
		client->chatterText() = txt;
		client->setChatPos( Utf8StringSize(txt) );
	} else if(game.isServer() && !game.isLocalGame()) {
		DeprecatedGUI::Menu_Net_HostLobbySetText( txt );
	} else if(game.isClient()) {
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
	if(client->getGameLobby()[FT_GameMode].as<GameModeInfo>()->generalGameType != GMT_TEAMS)
		warnings << "ParseTeamScoreUpdate: it's not a teamgame" << endl;
	
	bool someTeamScored = false;
	int teamCount = bs->readByte();
	for(int i = 0; i < teamCount; ++i) {
		if(bs->isPosAtEnd()) {
			warnings << "ParseTeamScoreUpdate: network is screwed up" << endl;
			break;
		}
		
		if(i == MAX_TEAMS) warnings << "ParseTeamScoreUpdate: cannot handle teamscores for other than the first " << (int)MAX_TEAMS << " teams" << endl;
		int score = bs->readInt16();
		if(i < MAX_TEAMS) {
			if(score > client->iTeamScores[i]) someTeamScored = true;
			client->iTeamScores[i] = score;
		}
	}
	
	if(someTeamScored && client->getGameLobby()[FT_GameMode].as<GameModeInfo>()->mode == GameMode(GM_CTF))
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
		if (client->getGameLobby()[FT_GameMode].as<GameModeInfo>()->generalGameType == GMT_TEAMS)  {

			if ((int)client->getGameLobby()[FT_KillLimit] != -1)  {
				CWorm* w = game.wormById(client->iMatchWinner, false);
				if(w)
					client->iMatchWinnerTeam = w->getTeam();
			} else if ((int)client->getGameLobby()[FT_Lives] != -2)  {
				for_each_iterator(CWorm*, w, game.worms()) {
					if (w->get()->getLives() >= 0)  {
						client->iMatchWinnerTeam = w->get()->getTeam();
						break;
					}
				}
			}
		}

		// Older servers send wrong info about tag winner, better if we count it ourself
		if (client->getGameLobby()[FT_GameMode].as<GameModeInfo>()->generalGameType == GMT_TIME)  {
			TimeDiff max = TimeDiff(0);

			for_each_iterator(CWorm*, w, game.worms()) {
				if (w->get()->getTagTime() > max)  {
					max = w->get()->getTagTime();
					client->iMatchWinner = w->get()->getID();
				}
			}
		}
	}
	else { // server >=beta9
		client->iMatchWinner = bs->readByte();

		if(client->getGameLobby()[FT_GameMode].as<GameModeInfo>()->generalGameType == GMT_TEAMS)  {
			client->iMatchWinnerTeam = bs->readByte();
			int teamCount = bs->readByte();
			for(int i = 0; i < teamCount; ++i) {
				if(bs->isPosAtEnd()) {
					warnings << "ParseGameOver: network is screwed up" << endl;
					break;
				}
				
				if(i == MAX_TEAMS) warnings << "ParseGameOver: cannot handle teamscores for other than the first " << (int)MAX_TEAMS << " teams" << endl;
				int score = bs->readInt16();
				if(i < MAX_TEAMS) client->iTeamScores[i] = score;
			}
		} else
			client->iMatchWinnerTeam = -1;
	}
		
	// Game over
	hints << "Client: the game is over";
	if(client->iMatchWinner >= 0 && client->iMatchWinner < MAX_WORMS) {
		hints << ", the winner is worm " << game.wormName(client->iMatchWinner);
	}
	if(client->iMatchWinnerTeam >= 0) {
		hints << ", the winning team is team " << client->iMatchWinnerTeam;
	}
	hints << endl;
	game.gameOver = true;
	game.gameOverFrame = game.serverFrame;

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
	if (game.state < Game::S_Preparing)  {
		warnings << "CClientNetEngine::ParseSpawnBonus: Cannot spawn bonus when not playing (packet ignored)" << endl;
		return;
	}

	if (id < 0 || id >= MAX_BONUSES)  {
		warnings << "CClientNetEngine::ParseSpawnBonus: invalid bonus ID (" << id << ")" << endl;
		return;
	}

	if (!game.gameMap()) { // Weird
		warnings << "CClientNetEngine::ParseSpawnBonus: game.gameMap() not set" << endl;
		return;
	}

	if (x > (int)game.gameMap()->GetWidth() || x < 0)  {
		warnings << "CClientNetEngine::ParseSpawnBonus: X-coordinate not in map (" << x << ")" << endl;
		return;
	}

	if (y > (int)game.gameMap()->GetHeight() || y < 0)  {
		warnings << "CClientNetEngine::ParseSpawnBonus: Y-coordinate not in map (" << y << ")" << endl;
		return;
	}

	CVec p = CVec( (float)x, (float)y );

	client->cBonuses[id].Spawn(p, type, wpn, game.gameScript());
	game.gameMap()->CarveHole(SPAWN_HOLESIZE,p,cClient->getGameLobby()[FT_InfiniteMap]);

	SpawnEntity(ENT_SPAWN,0,p,CVec(0,0),Color(),NULL);
}


// TODO: Rename this to ParseTimeUpdate (?)
///////////////////
// Parse a tag update packet
void CClientNetEngine::ParseTagUpdate(CBytestream *bs)
{
	if (game.state < Game::S_Preparing || game.gameOver)  {
		warnings << "CClientNetEngine::ParseTagUpdate: not playing - ignoring" << endl;
		return;
	}

	CWorm* target = getWorm(client, bs, "ParseTagUpdate", Skip<sizeof(float)>);
	if(!target) return;
	TimeDiff time = TimeDiff(bs->readFloat());

	if (client->getGameLobby()[FT_GameMode].as<GameModeInfo>()->generalGameType != GMT_TIME)  {
		warnings << "CClientNetEngine::ParseTagUpdate: game mode is not tag - ignoring" << endl;
		return;
	}

	// Set all the worms 'tag' property to false
	for_each_iterator(CWorm*, w, game.worms())
		w->get()->setTagIT(false);

	// Tag the worm
	target->setTagIT(true);
	target->setTagTime(time);

	// Log it
	log_worm_t *l = client->GetLogWorm(target->getID());
	if (l)  {
		foreach(otherw, client->tGameLog->tWorms)
			otherw->second.bTagIT = false;

		l->fTagTime = time;
		l->bTagIT = true;
	}
}


///////////////////
// Parse client-ready packet
void CClientNetEngine::ParseCLReady(CBytestream *bs)
{
	int numworms = bs->readByte();

	if((numworms < 0 || numworms > MAX_PLAYERS) && !game.isLocalGame()) {
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
		
		int id = bs->readByte();
		
		CWorm* w = game.wormById(id, false);
		if(!w) {
			warnings << "Client: got CLReady with bad worm " << id << endl;
			// Skip the info and if end of packet, just end
			if (CWorm::skipWeapons(bs))	break;
			continue;			
		}
		
		if(!w->getLocal())
			w->bWeaponsReady = true;

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
		int id = bs->readByte();
        int team = MAX(0,MIN(3,(int)bs->readByte()));

		CWorm* w = game.wormById(id, false);
        if(!w) {
			warnings << "CClientNetEngine::ParseUpdateLobby: invalid worm ID (" << id << ")" << endl;
			continue;			
		}
			
		w->setLobbyReady(ready);
		w->setTeam(team);
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
		int id = bs->readByte();

		CWorm *w = game.wormById(id, false);
		if(!w) {
			warnings << "ParseWormsOut: worm " << id << " invalid" << endl;
			continue;
		}
		
		if(w->getLocal()) {
			// Server kicks local worms using S2C_DROPPED, this packet cannot be used for it
			hints << "Warning: server says we (" << game.wormName(id) << ") have left but that is not true" << endl;
			continue;
		}

		// Log this
		if (client->tGameLog)  {
			log_worm_t *l = client->GetLogWorm(id);
			if (l)  {
				l->bLeft = true;
				l->fTimeLeft = game.serverTime();
			}
		}
		
		if( NewNet::Active() )
			NewNet::PlayerLeft(id);

		game.removeWorm(w);
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
	
	if(game.state < Game::S_Preparing || !game.gameMap() || !game.gameMap()->isLoaded()) {
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
		int id = bs->readByte();

		CWorm* w = game.wormById(id, false);
		if (!w)  {
			hints << "CClientNetEngine::ParseUpdateWorms: invalid worm " << id << endl;
			if (CWorm::skipPacketState(bs)) break; // Skip not to lose the right position
			continue;
		}

		w->readPacketState(bs);
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


template<typename T>
static bool onlyUpdateInLobby(CClient* client, FeatureIndex i, const T& update) {
	if(client->getStatus() != NET_CONNECTED) {
		if(client->getGameLobby()[i] != ScriptVar_t(update))
			notes << "CClientNetEngine::ParseUpdateLobbyGame: not in lobby - ignoring update " << featureArray[i].name << " '" << client->getGameLobby()[i].toString() << "' -> '" << ScriptVar_t(update).toString() << "'" << endl;
		return false;
	}

	client->getGameLobby().overwrite[i] = update;
	return true;	
}

///////////////////
// Parse an 'update game lobby' packet
void CClientNetEngine::ParseUpdateLobbyGame(CBytestream *bs)
{
	/*client->getGameLobby()->iMaxPlayers =*/ bs->readByte();
	const bool recheckMap = onlyUpdateInLobby(client, FT_Map, infoForLevel(bs->readString()));
	ModInfo modInfo;
	modInfo.name = bs->readString();
	modInfo.path = bs->readString();
	const bool recheckMod = onlyUpdateInLobby(client, FT_Mod, modInfo);
	client->getGameLobby().overwrite[FT_GameMode] = GameModeInfo::fromNetworkModeInt(bs->readByte());
	client->getGameLobby().overwrite[FT_Lives] = bs->readInt16();
	client->getGameLobby().overwrite[FT_KillLimit] = bs->readInt16();
	client->getGameLobby().overwrite[FT_TimeLimit] = -100.0f;
	client->getGameLobby().overwrite[FT_LoadingTime] = bs->readInt16();
    client->getGameLobby().overwrite[FT_Bonuses] = bs->readBool();

	client->getGameLobby().overwrite[FT_GameSpeed] = 1.0f;
	client->getGameLobby().overwrite[FT_ForceRandomWeapons] = false;
	client->getGameLobby().overwrite[FT_SameWeaponsAsHostWorm] = false;
	
	if(recheckMap)
		client->bHaveMap = infoForLevel(client->getGameLobby()[FT_Map].as<LevelInfo>()->path).valid;
	
	if(recheckMod)
		client->bHaveMod = infoForMod(client->getGameLobby()[FT_Mod].as<ModInfo>()->path).valid;

	for(size_t i = 0; i < FeatureArrayLen; ++i) {
		if(client->getServerVersion() < OLXBetaVersion(0,59,6)) {
			// Before 0.59b6, many settings have been outside of the FT array.
			// So, on those old servers, we handle them seperately and we don't want to reset them here!
			if(wasNotAFeatureSettingBefore((FeatureIndex)i))
				continue;
		}
		
		// certain settings were updated anyway, so don't reset them
		if(i == FT_Map ||
		   i == FT_Mod ||
		   i == FT_GameMode ||
		   i == FT_Lives ||
		   i == FT_KillLimit ||
		   i == FT_TimeLimit ||
		   i == FT_LoadingTime ||
		   i == FT_Bonuses ||
		   i == FT_ShowBonusName)
			continue;
		
		client->getGameLobby().overwrite[(FeatureIndex)i] = featureArray[i].unsetValue;  // Clean it up
	}
	client->otherGameInfo.clear();
	
	DeprecatedGUI::bJoin_Update = true;
	DeprecatedGUI::bHost_Update = true;
}

void CClientNetEngineBeta7::ParseUpdateLobbyGame(CBytestream *bs)
{
	CClientNetEngine::ParseUpdateLobbyGame(bs);

	client->getGameLobby().overwrite[FT_GameSpeed] = bs->readFloat();
	client->getGameLobby().overwrite[FT_ForceRandomWeapons] = bs->readBool();
	client->getGameLobby().overwrite[FT_SameWeaponsAsHostWorm] = bs->readBool();
}

void CClientNetEngineBeta9::ParseUpdateLobbyGame(CBytestream *bs)
{
	CClientNetEngineBeta7::ParseUpdateLobbyGame(bs);

	client->tGameInfo.overwrite[FT_TimeLimit] = bs->readFloat();
	if((float)client->tGameInfo[FT_TimeLimit] < 0) client->tGameInfo.overwrite[FT_TimeLimit] = -1.0f;

	ParseFeatureSettings(bs);
	
	client->getGameLobby().overwrite[FT_GameMode] = client->getGameLobby()[FT_GameMode].as<GameModeInfo>()->withNewName(bs->readString());
}


///////////////////
// Parse a 'worm down' packet (Worm died)
void CClientNetEngine::ParseWormDown(CBytestream *bs)
{
	// Don't allow anyone to kill us in lobby
	if (game.state < Game::S_Preparing)  {
		notes << "CClientNetEngine::ParseWormDown: not playing - ignoring" << endl;
		bs->Skip(1);  // ID
		return;
	}

	int id = bs->readByte();
	CWorm* w = game.wormById(id, false);
	
	if(!w)
		warnings << "CClientNetEngine::ParseWormDown: invalid worm ID (" << id << ")" << endl;
	else {
		// If the respawn time is 0, the worm can be spawned even before the simulation is done
		// Therefore the check for isAlive in the simulation does not work in all cases
		// Because of that, we unattach the rope here, just to be sure
		for_each_iterator(CWorm*, w2, game.worms()) {
			if(w2->get()->getNinjaRope()->getAttachedPlayer() == w)
				w2->get()->getNinjaRope()->UnAttachPlayer();
		}

		w->Kill(false);
		if (w->getLocal() && w->getType() == PRF_HUMAN)
			w->clearInput();

		// Make a death sound
		int s = GetRandomInt(2);
		StartSound( sfxGame.smpDeath[s], w->getPos(), w->getLocal(), -1 );

		// Spawn some giblets
		for(short n=0;n<7;n++)
			SpawnEntity(ENT_GIB,0,w->getPos(),GetRandomVec()*80,Color(),w->getGibimg());

		// Blood
		float amount = CLAMP(w->getHealth(), 50.f, 100.f) * ((float)tLXOptions->iBloodAmount / 100.0f);
		for(int i=0;i<amount;i++) {
			float sp = GetRandomNum()*100+50;
			SpawnEntity(ENT_BLOODDROPPER,0,w->getPos(),GetRandomVec()*sp,Color(128,0,0),NULL);
			SpawnEntity(ENT_BLOOD,0,w->getPos(),GetRandomVec()*sp,Color(200,0,0),NULL);
			SpawnEntity(ENT_BLOOD,0,w->getPos(),GetRandomVec()*sp,Color(128,0,0),NULL);
		}
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
			CWorm* killerW = game.wormById(client->iLastKiller, false);
			CWorm* victimW = game.wormById(client->iLastVictim, false);
			
			// Suicide
			if (l_kill == l_vict)  {
				l_vict->iSuicides++;
			}

			// Teamkill
			else if (killerW && victimW && killerW->getTeam() == victimW->getTeam())  {
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

	if (game.isServer())  {
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
		CShootList::skipSingle(bs, client->getServerVersion()); // Skip to get to the correct position
		return;
	}

	client->cShootList.readSingle(bs, client->getServerVersion(), game.gameScript()->GetNumWeapons() - 1);

	// Process the shots
	client->ProcessServerShotList();

}


///////////////////
// Parse a 'multi shot' packet
void CClientNetEngine::ParseMultiShot(CBytestream *bs)
{
	if(!client->canSimulate())  {
		CShootList::skipMulti(bs, client->getServerVersion()); // Skip to get to the correct position
		return;
	}

	client->cShootList.readMulti(bs, client->getServerVersion(), game.gameScript()->GetNumWeapons() - 1);

	// Process the shots
	client->ProcessServerShotList();
}


///////////////////
// Update the worms stats
void CClientNetEngine::ParseUpdateStats(CBytestream *bs)
{
	client->bShouldRepaintInfo = true;

	byte num = bs->readByte();
	if (num > MAX_PLAYERS)
		warnings << "CClientNetEngine::ParseUpdateStats: invalid worm count (" << num << ") - clamping" << endl;

	short c = 0;
	for_each_iterator(CWorm*, w, game.localWorms()) {
		c++;
		if(c > num) break;
		w->get()->readStatUpdate(bs);
	}
	if(c != num)
		warnings << "CClientNetEngine::ParseUpdateStats: local worms num = " << game.localWorms()->size() << ", but we got " << num << " updates" << endl;

	// Skip if there were some clamped worms
	for (short i=0;i<num-c;i++)
		if (CWorm::skipStatUpdate(bs))
			break;
}


///////////////////
// Parse a 'destroy bonus' packet
void CClientNetEngine::ParseDestroyBonus(CBytestream *bs)
{
	byte id = bs->readByte();

	if (game.state < Game::S_Preparing)  {
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
	if (game.isServer())  {
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
	game.state = Game::S_Lobby;

	// Hide the console
	Con_Hide();

	DeprecatedGUI::Menu_FloatingOptionsShutdown();


	if(game.isClient()) {

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
	if (game.isServer())  {
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
					IsFileAvailable( "levels/" + client->getGameLobby()[FT_Map].as<LevelInfo>()->path ) )
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
				client->getUdpFileDownloader()->getFilename().find( client->getGameLobby()[FT_Mod].as<ModInfo>()->path ) == 0 &&
				IsFileAvailable(client->getGameLobby()[FT_Mod].as<ModInfo>()->path + "/script.lgs", false) )
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
				if( client->getUdpFileDownloader()->getFileInfo()[f].filename.find( client->getGameLobby()[FT_Mod].as<ModInfo>()->path ) == 0 &&
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
				if( client->getUdpFileDownloader()->getFileInfo()[f].filename.find( client->getGameLobby()[FT_Mod].as<ModInfo>()->path ) == 0 &&
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

	if( game.state < Game::S_Preparing )
		return;
	CWorm *w = game.wormById(id, false);
	CWorm *offender = game.wormById(offenderId, false);	
	if( !w || !offender )
		return;
	
	w->getDamageReport()[offender->getID()].damage += damage;
	w->getDamageReport()[offender->getID()].lastTime = tLX->currentTime;
	w->injure(damage);	// Calculate correct healthbar
	// Update worm damage count (it gets updated in UPDATESCORE packet, we do local calculations here, but they are wrong if we connected during game)
	//notes << "CClientNetEngineBeta9::ParseReportDamage() offender " << offender->getID() << " dmg " << damage << " victim " << id << endl;
	offender->addDamage( damage, w, false );
}

void CClientNetEngineBeta9::ParseScoreUpdate(CBytestream *bs)
{
	short id = bs->readInt(1);
	CWorm* w = game.wormById(id, false);
	
	if(!w)
		// do this to get the right position in net stream
		bs->Skip(6);
	else {
		log_worm_t *l = client->GetLogWorm(id);

		w->setLives( MAX<int>((int)bs->readInt16(), WRM_UNLIM) );
	
		w->setKills( bs->readInt(4) );
		float damage = bs->readFloat();
		if( w->getDamage() != damage )
		{
			// Occurs pretty often, don't spam console, still it should be the same on client and server
			//warnings << "CClientNetEngineBeta9::ParseScoreUpdate(): damage for worm " << client->cRemoteWorms[id].getName() << " is " << client->cRemoteWorms[id].getDamage() << " server sent us " << damage << endl;
		}
		w->setDamage( damage );

		
		if (w->getLocal())
			client->bShouldRepaintInfo = true;

		// Logging
		if (l)  {
			// Check if the stats changed
			bool stats_changed = false;
			if (l->iLives != w->getLives())  {
				l->iLives = w->getLives();
				client->iLastVictim = id;
				stats_changed = true;
			}

			if (l->iKills != w->getScore())  {
				l->iKills = w->getScore();
				client->iLastKiller = id;
				stats_changed = true;
			}

			// If the update was sent but no changes made -> this is a killer that made a teamkill
			// See CServer::ParseDeathPacket for more info
			if (!stats_changed)
				client->iLastKiller = id;
		}
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
	if (game.wormById(forworm, false) == NULL)  {
		errors << "ParseHideWorm: invalid forworm ID " << forworm << endl;
		return;
	}

	// Get the worm
	CWorm *w = game.wormById(id, false);
	if (!w)  {
		errors << "ParseHideWorm: the worm " << id << " does not exist" << endl;
		return;
	}

	// old clients were so stupid to mix up functionality of hideworm/spawning
	if(client->getServerVersion() <= OLXBetaVersion(0,59,7)) {
		w->Spawn(w->getPos());	// We won't get SpawnWorm packet from H&S server
		if (!hide && !immediate)	// Show sparkles only when worm is discovered, or else we'll know where it has been respawned
			SpawnEntity(ENT_SPAWN,0,w->getPos(),CVec(0,0),Color(),NULL); // Spawn some sparkles, looks good
	}
	
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
	
	w->bWeaponsReady = false;
	if(client->OwnsWorm(w->getID())) {
		notes << "server sends us SelectWeapons for worm " << w->getID() << endl;
		
		if(game.gameScript()->gusEngineUsed()) {
			if(CWormHumanInputHandler* player = dynamic_cast<CWormHumanInputHandler*> (w->inputHandler()))
				// this will restart the weapon selection in most mods
				game.onNewHumanPlayer_Lua(player);
			else
				//notes << "SelectWeapons in Gusanos for bots not supported yet" << endl;
				game.onNewPlayer_Lua(w->inputHandler());
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

