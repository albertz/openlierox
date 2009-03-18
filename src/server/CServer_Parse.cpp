/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie, Albert Zeyer and Martin Griffin
//
//
/////////////////////////////////////////


// Server class - Parsing
// Created 1/7/02
// Jason Boettcher



#include "LieroX.h"
#include "FindFile.h"
#include "CServer.h"
#include "ProfileSystem.h"
#include "DeprecatedGUI/Menu.h"
#include "CServerConnection.h"
#include "CServerNetEngine.h"
#include "CChannel.h"
#include "StringUtils.h"
#include "CWorm.h"
#include "Protocol.h"
#include "ConfigHandler.h"
#include "ChatCommand.h"
#include "DedicatedControl.h"
#include "AuxLib.h"
#include "Version.h"
#include "Timer.h"
#include "NotifyUser.h"
#include "XMLutils.h"
#include "CClientNetEngine.h"
#include "IpToCountryDB.h"
#include "Debug.h"
#include "CGameMode.h"

#ifdef _MSC_VER
#undef min
#undef max
#endif


// TODO: move this out here after we have moved all the Send/Parse stuff
// declare them only locally here as nobody really should use them explicitly
std::string OldLxCompatibleString(const std::string &Utf8String);
std::string Utf8String(const std::string &OldLxString);


/*
=======================================
		Connected Packets
=======================================
*/

///////////////////
// Parse a packet
void CServerNetEngine::ParsePacket(CBytestream *bs) {

	// TODO: That's hack, all processing should be done in CChannel_056b
	// Maybe server will work without that code at all, should check against 0.56b
	CChannel *chan = cl->getChannel();
	if( chan != NULL )
	{
		chan->recheckSeqs();
	}

	cl->setLastReceived(tLX->currentTime);


	// Do not process empty packets
	if (bs->isPosAtEnd())
		return;

	uchar cmd;

	while (!bs->isPosAtEnd()) {
		cmd = bs->readInt(1);

		switch (cmd) {

			// Client is ready
		case C2S_IMREADY:
			ParseImReady(bs);
			break;

			// Update packet
		case C2S_UPDATE:
			ParseUpdate(bs);
			break;

			// Death
		case C2S_DEATH:
			ParseDeathPacket(bs);
			break;

			// Chat text
		case C2S_CHATTEXT:
			ParseChatText(bs);
			break;

		case C2S_CHATCMDCOMPLREQ:
			ParseChatCommandCompletionRequest(bs);	
			break;

		case C2S_AFK:
			ParseAFK(bs);
			break;

			// Update lobby
		case C2S_UPDATELOBBY:
			ParseUpdateLobby(bs);
			break;

			// Disconnect
		case C2S_DISCONNECT:
			ParseDisconnect();
			break;

			// Bonus grabbed
		case C2S_GRABBONUS:
			ParseGrabBonus(bs);
			break;

		case C2S_SENDFILE:
			ParseSendFile(bs);
			break;

		case C2S_REPORTDAMAGE:
			ParseReportDamage(bs);
			break;
			
		case C2S_NEWNET_KEYS:
			ParseNewNetKeys(bs);
			break;

		default:
			// HACK, HACK: old olx/lxp clients send the ping twice, once normally once per channel
			// which leads to warnings here - we simply parse it here and avoid warnings

			/*
			It's a bug in LieroX Pro that sent the ping
			packet twice: once per CChannel - it looked like this:
			(8 bytes of CChannel's sequence)(Connectionless header)(Packet itself)
			and then the same packet normally (via CBytestream::Send)
			(Connectionless header) (Packet itself)

			The hack solved the first case which generated warning of unknown packet.
			*/

			// Avoid "reading from stream behind end" warning if this is really a bad packet
			// and print the bad command instead
			if (cmd == 0xff && bs->GetRestLen() > 3)
				if (bs->readInt(3) == 0xffffff) {
					std::string address;
					NetAddrToString(cl->getChannel()->getAddress(), address);
					server->ParseConnectionlessPacket(cl->getChannel()->getSocket(), bs, address);
					break;
				}

			// Really a bad packet
#if !defined(FUZZY_ERROR_TESTING_C2S)
			warnings("sv: Bad command in packet (" + itoa(cmd) + ")\n");
			if(cl->isLocalClient()) {
				notes << "Bad package from local client, dumping bytestream:" << endl;
				bs->Dump();
			}
#endif
		}
	}
}


///////////////////
// Parse a 'im ready' packet
void CServerNetEngine::ParseImReady(CBytestream *bs) {
	if ( (server->iState != SVS_GAME && !server->serverAllowsConnectDuringGame()) || server->iState == SVS_LOBBY )
	{
		notes("GameServer::ParseImReady: Not waiting for ready, packet is being ignored.\n");

		// Skip to get the correct position in the stream
		int num = bs->readByte();
		for (int i = 0;i < num;i++)  {
			bs->Skip(1);
			CWorm::skipWeapons(bs);
		}

		return;
	}

	int i, j;
	// Note: This isn't a lobby ready

	// Read the worms weapons
	int num = bs->readByte();
	for (i = 0; i < num; i++) {
		int id = bs->readByte();
		if (id >= 0 && id < MAX_WORMS)  {
			if(!server->cWorms[id].isUsed()) {
				warnings << "ParseImReady: got unused worm-ID!" << endl;
				CWorm::skipWeapons(bs);
				continue;
			}
			server->cWorms[id].readWeapons(bs);
			for (j = 0; j < 5; j++)
				server->cWorms[id].getWeapon(j)->Enabled =
					server->cWeaponRestrictions.isEnabled(server->cWorms[id].getWeapon(j)->Weapon->Name) ||
					server->cWeaponRestrictions.isBonus(server->cWorms[id].getWeapon(j)->Weapon->Name);
		
		} else { // wrong id -> Skip to get the right position
			CWorm::skipWeapons(bs);
		}
	}


	// Set this client to 'ready'
	cl->setGameReady(true);

	SendClientReady(NULL);
	
	if(server->iState == SVS_PLAYING && server->serverAllowsConnectDuringGame())
		server->BeginMatch(cl);
	else
		// Check if all the clients are ready
		server->CheckReadyClient();
}


///////////////////
// Parse an update packet
void CServerNetEngine::ParseUpdate(CBytestream *bs) {
	for (short i = 0; i < cl->getNumWorms(); i++) {
		CWorm *w = cl->getWorm(i);

		w->readPacket(bs, server->cWorms);

		// If the worm is shooting, handle it
		if (w->getWormState()->bShoot && w->getAlive() && server->iState == SVS_PLAYING)
			server->WormShoot(w, server); // handle shot and add to shootlist to send it later to the clients
	}
}


///////////////////
// Parse a death packet
void CServerNetEngine::ParseDeathPacket(CBytestream *bs) {
	// No kills in lobby
	if (server->iState != SVS_PLAYING)  {
		notes << "GameServer::ParseDeathPacket: Not playing, ignoring the packet." << endl;

		// Skip to get the correct position in the stream
		bs->Skip(2);

		return;
	}

	int victim = bs->readInt(1);
	int killer = bs->readInt(1);

	// Bad packet
	if (bs->GetPos() > bs->GetLength())  {
		warnings << "GameServer::ParseDeathPacket: Reading beyond the end of stream." << endl;
		return;
	}

	// If the game is already over, ignore this
	if (server->bGameOver)  {
		notes("GameServer::killWorm: Game is over, ignoring.\n");
		return;
	}
	// Safety check
	if (victim < 0 || victim >= MAX_WORMS)  {
		warnings("GameServer::killWorm: victim ID out of bounds.\n");
		return;
	}
	if (killer < 0 || killer >= MAX_WORMS)  {
		warnings("GameServer::killWorm: killer ID out of bounds.\n");
		return;
	}

	if (tLXOptions->tGameInfo.bServerSideHealth)  {
		// Cheat prevention check (God Mode etc), make sure killer is the host or the packet is sent by the client owning the worm
		if (!cl->isLocalClient())  {
			if (cl->OwnsWorm(victim))  {  // He wants to die, let's fulfill his dream ;)
				CWorm *w = cClient->getRemoteWorms() + victim;
				if (!w->getAlreadyKilled())  // Prevents killing the worm twice (once by server and once by the client itself)
					cClient->getNetEngine()->SendDeath(victim, killer);
			} else {
				warnings << "GameServer::ParseDeathPacket: victim " << victim << " is not one of the client's worms." << endl;
			}

			// The client on this machine will send the death again, then we'll parse it
			return;
		}
	} else {
		// Cheat prevention check: make sure the victim is one of the client's worms
		if (!cl->OwnsWorm(victim))  {
			std::string clientWorms;
			for(int i=cl->getNumWorms();i > 0 && i <= 2;i++) {
				CWorm* w = cl->getWorm(i);
				if (w) {
					if (i & 1)
						clientWorms += " ";
					clientWorms += itoa(w->getID()) + "," + w->getName() + ".";
				}
			}
			printf("GameServer::ParseDeathPacket: victim (%i) is not one of the client's worms (%s).\n",
						victim, clientWorms.c_str());
			return;
		}
	}

	// A small hack: multiple-suiciding can lag down the game, check if
	// the packet contains another death (with same killer and victim), if so, just
	// increase the suicide variable and proceed
	if (killer == victim)  {
		server->iSuicidesInPacket++;
		if (!bs->isPosAtEnd())  {
			if (bs->peekByte() == C2S_DEATH)  {
				std::string s = bs->peekData(3);
				if (s.size() == 3)  {
					s.erase(s.begin()); // The C2S_DEATH byte
					if (((uchar)s[0] == (uchar)victim) && ((uchar)s[1] == (uchar)killer))
						return;
				}
			}
		}
	}

	server->killWorm(victim, killer, server->iSuicidesInPacket);
	
	server->iSuicidesInPacket = 0; // Reset counter, so next /suicide command will work correctly
}


///////////////////
// Parse a chat text packet
void CServerNetEngine::ParseChatText(CBytestream *bs) {
	std::string buf = Utf8String(bs->readString());

	if(cl->getNumWorms() == 0) {
		warnings << cl->debugName() << " with no worms sends message '" << buf << "'" << endl;
		return;
	}

	if (buf.empty()) { // Ignore empty messages
		warnings << cl->debugName() << " sends empty message" << endl;
		return;
	}

	// TODO: is it correct that we check here only for worm 0 ?
	// TODO: should we perhaps also check, if the beginning of buf is really the correct name?

	std::string command_buf = buf;
	if (cl->getWorm(0)) {
		std::string startStr = cl->getWorm(0)->getName() + ": ";
		if( buf.size() > startStr.size() && buf.substr(0,startStr.size()) == startStr )
			command_buf = buf.substr(startStr.size());  // Special buffer used for parsing special commands (begin with /)
	}
	notes << "CHAT: " << buf << endl;

	// Check for special commands
	if (command_buf.size() > 2)
		if (command_buf[0] == '/' && command_buf[1] != '/')  {  // When two slashes at the beginning, parse as a normal message
			ParseChatCommand(command_buf);
			return;
		}

	// Check for Clx (a cheating version of lx)
	if(buf[0] == 0x04) {
		server->SendGlobalText(cl->debugName() + " seems to have CLX or some other hack", TXT_NORMAL);
	}

	// Don't send text from muted players
	if (cl->getMuted()) {
		notes << "ignored message from muted " << cl->debugName() << endl;
		return;
	}

	if( !cl->isLocalClient() ) {
		// Check if player tries to fake other player
		bool nameMatch = false;
		for( int i=0; i<cl->getNumWorms(); i++ )
			if( buf.find(cl->getWorm(i)->getName() + ": ") == 0 )
				nameMatch = true;
		if( !nameMatch )
		{
			notes << "Client " << cl->debugName() << " probably tries to fake other player, or an old client uses /me cmd" << endl;
			return;
		}
	}
	
	server->SendGlobalText(buf, TXT_CHAT);

	if( DedicatedControl::Get() && buf.size() > cl->getWorm(0)->getName().size() + 2 )
		DedicatedControl::Get()->ChatMessage_Signal(cl->getWorm(0),buf.substr(cl->getWorm(0)->getName().size() + 2));
}

void CServerNetEngineBeta7::ParseChatCommandCompletionRequest(CBytestream *bs) {
	std::list<std::string> possibilities;
	
	std::string startStr = bs->readString();
	TrimSpaces(startStr);
	std::vector<std::string> cmdStart = ParseCommandMessage("/" + startStr, false);
		
	if(cmdStart.size() > 1) {
		ChatCommand* cmd = GetCommand(cmdStart[0]);
		if(!cmd) {
			SendText("Chat auto completion: unknown command", TXT_NETWORK);
			return;
		}
		
		// TODO: Move it out here and make it more general (add it to ChatCommand structure).
		if(cmd->tProcFunc == &ProcessSetVar && cmdStart.size() == 2) {
			// TODO: make faster with lower_bound(), upper_bound() and correct sorting of m_vars
			for(CScriptableVars::const_iterator it = CScriptableVars::Vars().begin(); it != CScriptableVars::Vars().end(); ++it) {
				// ignore callbacks
				if(it->second.type == SVT_CALLBACK) continue;
				
				if( subStrCaseEqual(cmdStart[1], it->first, cmdStart[1].size()) ) {
					std::string nextComplete = cmdStart[1];
					for(size_t f = cmdStart[1].size();; ++f) {
						if(f >= it->first.size()) { nextComplete += ' '; break; }
						nextComplete += it->first[f];
						if(it->first[f] == '.') break;
					}
					
					if(possibilities.size() == 0 || *possibilities.rbegin() != nextComplete) {
						possibilities.push_back(nextComplete);
					}
				}
			}

			if(possibilities.size() == 0) {
				SendText("Chat auto completion: unknown variable", TXT_NETWORK);
				return;
			}

			if(possibilities.size() == 1) {
				cl->getNetEngine()->SendChatCommandCompletionSolution(startStr, cmd->sName + " " + possibilities.front());
				return;
			}

			size_t l = maxStartingEqualStr(possibilities);
			if(l > cmdStart[1].size()) {
				// we can complete to some longer sequence
				cl->getNetEngine()->SendChatCommandCompletionSolution(startStr, cmd->sName + " " + possibilities.front().substr(0, l));
				return;
			}
			
			// send list of all possibilities
			cl->getNetEngine()->SendChatCommandCompletionList(startStr, possibilities);
			return;
		}
		
		return;
	}
	stringlwr(cmdStart[0]);
	
	for (uint i=0; tKnownCommands[i].tProcFunc != NULL; ++i) {
		if(subStrEqual(cmdStart[0], tKnownCommands[i].sName, cmdStart[0].size()))
			possibilities.push_back(tKnownCommands[i].sName);
		else if(subStrEqual(cmdStart[0], tKnownCommands[i].sAlias, cmdStart[0].size()))
			possibilities.push_back(tKnownCommands[i].sAlias);
	}
	
	if(possibilities.size() == 0) {
		SendText("Chat auto completion: unknown command", TXT_NETWORK);
		return;
	}
	
	if(possibilities.size() == 1) {
		// we have exactly one solution
		ChatCommand* cmd = GetCommand(startStr);
		if(cmd && startStr.size() <= possibilities.front().size()) {
			SendText("Chat auto completion: " + cmd->sName + ": " +
					 itoa(cmd->iMinParamCount) + "-" + itoa(cmd->iMaxParamCount) + " params",
					 TXT_NETWORK);		   
		}
		else
			cl->getNetEngine()->SendChatCommandCompletionSolution(startStr, possibilities.front() + " ");
		return;
	}
	
	size_t l = maxStartingEqualStr(possibilities);
	if(l > startStr.size()) {
		// we can complete to some longer sequence
		cl->getNetEngine()->SendChatCommandCompletionSolution(startStr, possibilities.front().substr(0, l));
		return;
	}
	
	// send list of all possibilities
	cl->getNetEngine()->SendChatCommandCompletionList(startStr, possibilities);
}

void CServerNetEngineBeta7::ParseAFK(CBytestream *bs) {

	int wormid = bs->readByte();
	int afkType = bs->readByte();
	std::string message = bs->readString(128);
	if (wormid < 0 || wormid >= MAX_WORMS)
		return;
	
	CWorm *w = &server->cWorms[wormid];
	if( ! w->isUsed() || w->getClient() != cl )
		return;
	
	w->setAFK( (AFK_TYPE)afkType, message );

	CBytestream bs1;
	bs1.writeByte( S2C_AFK );
	bs1.writeByte( wormid );
	bs1.writeByte( afkType );
	bs1.writeString( message );
	
	CServerConnection *cl1;
	int i;
	for( i=0, cl1=server->cClients; i < MAX_CLIENTS; i++, cl1++ )
		if( cl1->getStatus() == NET_CONNECTED && cl1->getClientVersion() >= OLXBetaVersion(7) )
			cl1->getNetEngine()->SendPacket( &bs1 );
		
}


///////////////////
// Parse a 'update lobby' packet
void CServerNetEngine::ParseUpdateLobby(CBytestream *bs) {
	// Must be in lobby
	if ( server->iState != SVS_LOBBY )  {
		notes << "GameServer::ParseUpdateLobby: Not in lobby." << endl;

		// Skip to get the right position
		bs->Skip(1);

		return;
	}

	bool ready = bs->readBool();
	int i;

	// Set the client worms lobby ready state
	for (i = 0; i < cl->getNumWorms(); i++) {
		lobbyworm_t *l = cl->getWorm(i)->getLobby();
		if (l)
			l->bReady = ready;
	}
	
	// Let all the worms know about the new lobby state
	for( int i=0; i<MAX_CLIENTS; i++ )
		server->cClients[i].getNetEngine()->SendUpdateLobby();
}


///////////////////
// Parse a disconnect packet
void CServerNetEngine::ParseDisconnect() {
	// Check if the client hasn't already left
	if (cl->getStatus() == NET_DISCONNECTED)  {
		notes << "GameServer::ParseDisconnect: Client has already disconnected." << endl;
		return;
	}

	// Host cannot leave...
	if (cl->isLocalClient())  {
		/* There are several cases (e.g. in ParsePrepareGame) where we would
		 * just disconnect, even as the local client. It's hard to catch all
		 * these possible cases and in most cases, we have a screwed up
		 * network stream with the client.
		 * For that reason, we just try to reconnect it now.
		 */
		warnings << "host-client disconnected, reconnecting now ..." << endl;
		cServer->bLocalClientConnected = false;
		cClient->Connect("127.0.0.1:" + itoa(cServer->getPort()));
		
		return;
	}

	server->DropClient(cl, CLL_QUIT);
}



///////////////////
// Parse a 'grab bonus' packet
void CServerNetEngine::ParseGrabBonus(CBytestream *bs) {
	int id = bs->readByte();
	int wormid = bs->readByte();
	int curwpn = bs->readByte();

	// Check
	if (server->iState != SVS_PLAYING)  {
		notes << "GameServer::ParseGrabBonus: Not playing." << endl;
		return;
	}


	// Worm ID ok?
	if (wormid >= 0 && wormid < MAX_WORMS) {
		CWorm *w = &server->cWorms[wormid];

		// Bonus id ok?
		if (id >= 0 && id < MAX_BONUSES) {
			CBonus *b = &server->cBonuses[id];

			if (b->getUsed()) {

				// If it's a weapon, change the worm's current weapon
				if (b->getType() == BNS_WEAPON) {

					if (curwpn >= 0 && curwpn < 5) {

						wpnslot_t *wpn = w->getWeapon(curwpn);
						wpn->Weapon = server->cGameScript.get()->GetWeapons() + b->getWeapon();
						wpn->Charge = 1;
						wpn->Reloading = false;
					}
				}

				// Tell all the players that the bonus is now gone
				CBytestream bs;
				bs.writeByte(S2C_DESTROYBONUS);
				bs.writeByte(id);
				server->SendGlobalPacket(&bs);
				
				if( b->getType() == BNS_HEALTH && server->getClient(w->getID())->getClientVersion() < OLXBetaVersion(9) )
					for( int i=0; i < MAX_CLIENTS; i++ )
						if( server->cClients[i].getStatus() == NET_CONNECTED )
							server->cClients[i].getNetEngine()->QueueReportDamage( w->getID(), -30, w->getID() ); // It's random between 10-50 actually, we're doing approximation here
			} else {
				notes << "GameServer::ParseGrabBonus: Bonus already destroyed." << endl;
			}
		} else {
			notes << "GameServer::ParseGrabBonus: Invalid bonus ID" << endl;
		}
	} else {
		notes << "GameServer::ParseGrabBonus: invalid worm ID" << endl;
	}
}

void CServerNetEngine::ParseSendFile(CBytestream *bs)
{
	cl->setLastFileRequestPacketReceived( AbsTime() ); // Set time in the past to force sending next packet
	if( cl->getUdpFileDownloader()->receive(bs) )
	{
		if( cl->getUdpFileDownloader()->isFinished() &&
			( cl->getUdpFileDownloader()->getFilename() == "GET:" || cl->getUdpFileDownloader()->getFilename() == "STAT:" ) )
		{
			cl->getUdpFileDownloader()->abortDownload();	// We can't provide that file or statistics on it
		}
	}
}

/////////////////
// Parse a command from chat
bool CServerNetEngine::ParseChatCommand(const std::string& message)
{
	// Parse the message
	const std::vector<std::string>& parsed = ParseCommandMessage(message, false);

	// Invalid
	if (parsed.size() == 0)
		return false;

	// Get the command
	ChatCommand *cmd = GetCommand(parsed[0]);
	if (!cmd)  {
		SendText("The command is not supported.", TXT_NETWORK);
		return false;
	}

	// Check the params
	size_t num_params = parsed.size() - 1;
	if (num_params < cmd->iMinParamCount || num_params > cmd->iMaxParamCount)  {
		SendText("Invalid parameter count.", TXT_NETWORK);
		return false;
	}

	// Get the parameters
	std::vector<std::string> parameters = std::vector<std::string>(parsed.begin() + 1, parsed.end());

	// Process the command
	std::string error = cmd->tProcFunc(parameters, (cl->getNumWorms() > 0) ? cl->getWorm(0)->getID() : 0); // TODO: is this handling for worm0 correct? fix if not
	if (error.size() != 0)  {
		SendText(error, TXT_NETWORK);
		return false;
	}

	return true;
}

void CServerNetEngineBeta9::ParseReportDamage(CBytestream *bs)
{
	int id = bs->readByte();
	int damage = bs->readByte();
	if( damage > SCHAR_MAX )		// Healing = negative damage
		damage -= UCHAR_MAX + 1;	// Wrap it around
	int offenderId = bs->readByte();

	if( server->iState != SVS_PLAYING )
		return;

	if( id < 0 || id >= MAX_WORMS || offenderId < 0 || offenderId >= MAX_WORMS )
		return;

	CWorm *w = & server->getWorms()[id];
	CWorm *offender = & server->getWorms()[offenderId];
	
	if( ! w->isUsed() || ! offender->isUsed() )
		return;
	
	if( ! cl->OwnsWorm(id) && ! cl->isLocalClient() )	// Allow local client to send damage for pre-Beta9 clients
		return;
	
	offender->addDamage( damage, w, tLXOptions->tGameInfo );

	//printf("CServerNetEngineBeta9::ParseReportDamage() offender %i dmg %i victim %i\n", offender->getID(), damage, id);
	// Re-send the packet to all clients, except the sender
	for( int i=0; i < MAX_CLIENTS; i++ )
		if( server->cClients[i].getStatus() == NET_CONNECTED && (&server->cClients[i]) != cl )
			server->cClients[i].getNetEngine()->QueueReportDamage( w->getID(), damage, offender->getID() );
}


void CServerNetEngineBeta9::ParseNewNetKeys(CBytestream *bs)
{
	int id = bs->readByte();
	if( id < 0 || id >= MAX_WORMS || !cl->OwnsWorm(id) )
	{
		warnings << "CServerNetEngineBeta9::ParseNewNetKeys(): worm id " << id << " client doesn't own worm" << endl;
		bs->Skip( NewNet::NetPacketSize() );
		return;
	}

	CBytestream send;
	send.writeByte( S2C_NEWNET_KEYS );
	send.writeByte( id );
	send.writeData( bs->readData( NewNet::NetPacketSize() ) );
	
	// Re-send the packet to all clients, except the sender
	for( int i=0; i < MAX_CLIENTS; i++ )
		if( server->cClients[i].getStatus() == NET_CONNECTED && (&server->cClients[i]) != cl )
		{
			send.ResetPosToBegin();
			server->cClients[i].getNetEngine()->SendPacket(&send);
		}
}

/*
===========================

  Connectionless packets

===========================
*/


///////////////////
// Parses connectionless packets
void GameServer::ParseConnectionlessPacket(NetworkSocket tSocket, CBytestream *bs, const std::string& ip) {
	std::string cmd = bs->readString(128);

	if (cmd == "lx::getchallenge")
		ParseGetChallenge(tSocket, bs);
	else if (cmd == "lx::connect")
		ParseConnect(tSocket, bs);
	else if (cmd == "lx::ping")
		ParsePing(tSocket);
	else if (cmd == "lx::time") // request for cServer->fServertime
		ParseTime(tSocket);
	else if (cmd == "lx::query")
		ParseQuery(tSocket, bs, ip);
	else if (cmd == "lx::getinfo")
		ParseGetInfo(tSocket);
	else if (cmd == "lx::wantsjoin")
		ParseWantsJoin(tSocket, bs, ip);
	else if (cmd == "lx::traverse")
		ParseTraverse(tSocket, bs, ip);
	else if (cmd == "lx::registered")
		ParseServerRegistered(tSocket);
	else  {
		warnings << "GameServer::ParseConnectionlessPacket: unknown packet \"" << cmd << "\"" << endl;
		bs->SkipAll(); // Safety: ignore any data behind this unknown packet
	}
}


///////////////////
// Handle a "getchallenge" msg
void GameServer::ParseGetChallenge(NetworkSocket tSocket, CBytestream *bs_in) {
	int			i;
	NetworkAddr	adrFrom;
	AbsTime		OldestTime = AbsTime::MAX();
	int			ChallengeToSet = -1;
	CBytestream	bs;

	//printf("Got GetChallenge packet\n");

	GetRemoteNetAddr(tSocket, adrFrom);

	// If were in the game, deny challenges
	if ( iState != SVS_LOBBY && !serverAllowsConnectDuringGame() ) {
		bs.Clear();
		// TODO: move this out here
		bs.writeInt(-1, 4);
		bs.writeString("lx::badconnect");
		bs.writeString(OldLxCompatibleString(networkTexts->sGameInProgress));
		bs.Send(tSocket);
		printf("GameServer::ParseGetChallenge: Cannot join, the game is in progress.\n");
		return;
	}


	// see if we already have a challenge for this ip
	for (i = 0;i < MAX_CHALLENGES;i++) {

		if (IsNetAddrValid(tChallenges[i].Address)) {
			if (AreNetAddrEqual(adrFrom, tChallenges[i].Address))
				continue;
			if (ChallengeToSet < 0 || tChallenges[i].fTime < OldestTime) {
				OldestTime = tChallenges[i].fTime;
				ChallengeToSet = i;
			}
		} else {
			ChallengeToSet = i;
			break;
		}
	}

	std::string client_version;
	if( ! bs_in->isPosAtEnd() )
		client_version = bs_in->readString(128);

	if (ChallengeToSet >= 0) {

		// overwrite the oldest
		tChallenges[ChallengeToSet].iNum = (rand() << 16) ^ rand();
		tChallenges[ChallengeToSet].Address = adrFrom;
		tChallenges[ChallengeToSet].fTime = tLX->currentTime;
		tChallenges[ChallengeToSet].sClientVersion = client_version;

		i = ChallengeToSet;
	}

	// Send the challenge details back to the client
	SetRemoteNetAddr(tSocket, adrFrom);


	// TODO: move this out here
	bs.writeInt(-1, 4);
	bs.writeString("lx::challenge");
	bs.writeInt(tChallenges[i].iNum, 4);
	if( client_version != "" )
		bs.writeString(GetFullGameName());
	bs.Send(tSocket);
}


///////////////////
// Handle a 'connect' message
void GameServer::ParseConnect(NetworkSocket tSocket, CBytestream *bs) {
	CBytestream		bytestr;
	NetworkAddr		adrFrom;
	int				p, player = -1;
	int				numplayers;
	CServerConnection		/*	*cl,*/ *newcl;

	//printf("Got Connect packet\n");

	// Connection details
	int		ProtocolVersion;
	//int		Port = LX_PORT;
	int		ChallId;
	int		iNetSpeed = 0;


	// Ignore if we are playing (the challenge should have denied the client with a msg)
	if ( !serverAllowsConnectDuringGame() && iState != SVS_LOBBY )  {
//	if (iState == SVS_PLAYING) {
		notes << "GameServer::ParseConnect: In game, ignoring." << endl;
		return;
	}
	
	// User Info to get
	GetRemoteNetAddr(tSocket, adrFrom);
	std::string addrFromStr;
	NetAddrToString(adrFrom, addrFromStr);

	CServerConnection* reconnectFrom = NULL;
	p = 0;
	for(CServerConnection* cl = cClients;p<MAX_CLIENTS;p++,cl++) {
		
		if(cl->getStatus() == NET_DISCONNECTED)
			continue;
		
		if(AreNetAddrEqual(adrFrom, cl->getChannel()->getAddress())) {
			reconnectFrom = cl;
			break;
		}
	}			
	
	
	// Read packet
	ProtocolVersion = bs->readInt(1);
	if (ProtocolVersion != PROTOCOL_VERSION) {
		printf("Wrong protocol version, server protocol version is %d\n", PROTOCOL_VERSION);

		// Get the string to send
		std::string buf;
		if (networkTexts->sWrongProtocol != "<none>")  {
			replacemax(networkTexts->sWrongProtocol, "<version>", itoa(PROTOCOL_VERSION), buf, 1);
		} else
			buf = " ";

		// Wrong protocol version, don't connect client
		bytestr.Clear();
		bytestr.writeInt(-1, 4);
		bytestr.writeString("lx::badconnect");
		bytestr.writeString(OldLxCompatibleString(buf));
		bytestr.Send(tSocket);
		printf("GameServer::ParseConnect: Wrong protocol version");
		return;
	}

	std::string szAddress;
	NetAddrToString(adrFrom, szAddress);

	// Is this IP banned?
	if (getBanList()->isBanned(szAddress))  {
		printf("Banned client %s was trying to connect\n", szAddress.c_str());
		bytestr.Clear();
		bytestr.writeInt(-1, 4);
		bytestr.writeString("lx::badconnect");
		bytestr.writeString(OldLxCompatibleString(networkTexts->sYouAreBanned));
		bytestr.Send(tSocket);
		return;
	}

	//Port = pack->ReadShort();
	ChallId = bs->readInt(4);
	iNetSpeed = bs->readInt(1);

	// Make sure the net speed is within bounds, because it's used for indexing
	iNetSpeed = CLAMP(iNetSpeed, 0, 3);

	// Get user info
	int numworms = bs->readInt(1);
	numworms = CLAMP(numworms, 0, (int)MAX_PLAYERS);
	
	Version clientVersion;
	
	// If we ignored this challenge verification, there could be double connections

	// See if the challenge is valid
	bool valid_challenge = false;
	if(reconnectFrom) {
		// ignore the challenge, the challenge was already verified earlier
		valid_challenge = true;
		clientVersion = reconnectFrom->getClientVersion();
	}
	else {
		int i;
		for (i = 0; i < MAX_CHALLENGES; i++) {
			if (IsNetAddrValid(tChallenges[i].Address) && AreNetAddrEqual(adrFrom, tChallenges[i].Address)) {

				if (ChallId == tChallenges[i].iNum)  { // good
					SetNetAddrValid(tChallenges[i].Address, false); // Invalidate it here to avoid duplicate connections
					tChallenges[i].iNum = 0;
					valid_challenge = true;
					break;
				} else { // bad
					valid_challenge = false;

					// HINT: we could receive another connect packet which will contain this challenge
					// and therefore get the worm connected twice. To avoid it, we clear the challenge here
					SetNetAddrValid(tChallenges[i].Address, false);
					tChallenges[i].iNum = 0;
					hints << "HINT: deleting a doubled challenge" << endl;

					// There can be more challanges from one client, if this one doesn't match,
					// perhaps some other does
				}
			}
		}

		if (!valid_challenge)  {
			notes << "Bad connection verification of client" << endl;
			bytestr.Clear();
			bytestr.writeInt(-1, 4);
			bytestr.writeString("lx::badconnect");
			bytestr.writeString(OldLxCompatibleString(networkTexts->sBadVerification));
			bytestr.Send(tSocket);
			return;
		}
		
		// Ran out of challenges
		if ( i == MAX_CHALLENGES ) {
			printf("No connection verification for client found\n");
			bytestr.Clear();
			bytestr.writeInt(-1, 4);
			bytestr.writeString("lx::badconnect");
			bytestr.writeString(OldLxCompatibleString(networkTexts->sNoIpVerification));
			bytestr.Send(tSocket);
			return;
		}
		
		const std::string & clientVersionStr = tChallenges[i].sClientVersion;
		clientVersion = clientVersionStr;
	}


	// Check if this ip isn't already connected
	// HINT: this works in every case, even if there are two people behind one router
	// because the address ports are checked as well (router assigns a different port for each person)
	newcl = NULL;
	if(reconnectFrom) {
		// HINT: just let the client connect again
		newcl = reconnectFrom;
		notes << "Reconnecting ";
		if(reconnectFrom->isLocalClient()) {
			bLocalClientConnected = false; // because we are reconnecting the client
			notes << "local ";
		}
		notes << "client " << reconnectFrom->debugName() << endl;
		
		if(reconnectFrom->isLocalClient() && addrFromStr.find("127.0.0.1") != 0) {
			errors << "client cannot be the local client because it has address " << addrFromStr << endl;
			bLocalClientConnected = true; // we just assume that
			reconnectFrom->setLocalClient(false);
		}
				
		/*
		 // Must not have got the connection good packet
		 if(cl->getStatus() == NET_CONNECTED) {
		 printf("Duplicate connection\n");
		 
		 // Resend the 'good connection' packet
		 bytestr.Clear();
		 bytestr.writeInt(-1,4);
		 bytestr.writeString("lx::goodconnection");
		 // Send the worm ids
		 for( int i=0; i<cl->getNumWorms(); i++ )
		 bytestr.writeInt(cl->getWorm(i)->getID(), 1);
		 bytestr.Send(tSocket);
		 return;
		 }
		 
		 // TODO: does this make sense? the already connected client tries to connect again while playing?
		 // Trying to connect while playing? Drop the client
		 if(cl->getStatus() == NET_PLAYING) {
		 //conprintf("Client tried to reconnect\n");
		 //DropClient(&players[p]);
		 return;
		 }
		 */		
	}

	// Find a spot for the client
	player = -1;
	p=0;
	if(newcl == NULL)
		for (CServerConnection* cl = cClients; p < MAX_CLIENTS; p++, cl++) {
			if (cl->getStatus() == NET_DISCONNECTED) {
				newcl = cl;
				break;
			}
		}

	
	// Calculate number of current players on this server
	numplayers = 0;
	CWorm *w = cWorms;
	for (p = 0;p < MAX_WORMS;p++, w++) {
		if (w->isUsed())
			numplayers++;
	}
	if(numplayers != iNumPlayers)
		warnings << "WARNING: stored player count " << iNumPlayers << " is different from recalculated player count " << numplayers << endl;

	// Ran out of slots
	if (!newcl) {
		printf("I have no more open slots for the new client\n");
		printf("%s - Server Error report",GetDateTime().c_str());
		notes << "currentTime is " << (tLX->currentTime - AbsTime()).seconds() << " Numplayers is " << numplayers << endl;
		std::string msg;

		bytestr.Clear();
		bytestr.writeInt(-1, 4);
		bytestr.writeString("lx::badconnect");
		bytestr.writeString(OldLxCompatibleString(networkTexts->sNoEmptySlots));
		bytestr.Send(tSocket);
		return;
	}

	// Server full (maxed already, or the number of extra worms wanting to join will go over the max)
	int max_players = (tLX->iGameType == GME_HOST ? tLXOptions->tGameInfo.iMaxPlayers : MAX_WORMS); // No limits (almost) for local play
	if (numplayers >= max_players || numplayers + numworms > max_players) {
		printf("I am full, so the new client cannot join\n");
		bytestr.Clear();
		bytestr.writeInt(-1, 4);
		bytestr.writeString("lx::badconnect");
		bytestr.writeString(OldLxCompatibleString(networkTexts->sServerFull));
		bytestr.Send(tSocket);
		return;
	}


	// Connect
	if (!newcl)
		return;

	// TODO: this is a bad hack, fix it
	// If this is the first client connected, it is our local client
	if (!bLocalClientConnected)  {
		if (addrFromStr.find("127.0.0.1") == 0)  { // Safety: check the IP
			newcl->setLocalClient(true);
			bLocalClientConnected = true;
			if(!reconnectFrom) // don't spam too much
				notes << "GameServer: our local client has connected" << endl;
		}
	} else {
		newcl->setLocalClient(false);
	}

	if( reconnectFrom && !newcl->getChannel() )
		warnings << "ParseConnect: reconnecting client doesn't has channel yet" << endl;
	if( !reconnectFrom && newcl->getChannel() ) {
		// TODO: It seems that this happens very often. Why?
		//warnings << "ParseConnect: new client has old channel set" << endl;
		newcl->resetChannel();
	}

	if( newcl->getChannel() == NULL) { 
		if(! newcl->createChannel( std::min(clientVersion, GetGameVersion() ) ) )
		{	// This should not happen - just in case
			errors << "Cannot create CChannel for client - invalid client version " << clientVersion.asString() << endl;
			bytestr.Clear();
			bytestr.writeInt(-1, 4);
			bytestr.writeString("lx::badconnect");
			bytestr.writeString(OldLxCompatibleString("Your client is incompatible to this server"));
			bytestr.Send(tSocket);
			return;
		}

		newcl->getChannel()->Create(&adrFrom, tSocket);
	}
	
	newcl->setLastReceived(tLX->currentTime);
	newcl->setNetSpeed(iNetSpeed);

	newcl->setClientVersion( clientVersion );

	newcl->setStatus(NET_CONNECTED);

	if(reconnectFrom && !newcl->getNetEngine())
		warnings << "ParseConnect: net engine is not set for reconnecting client" << endl;
	if(!reconnectFrom && newcl->getNetEngine()) {
		// TODO: It seems that this happens very often. Why?
		//warnings << "ParseConnect: old net engine was still set" << endl;
		newcl->resetNetEngine();
	}
	if(!newcl->getNetEngine())
		newcl->setNetEngineFromClientVersion(); // Deletes CServerNetEngine instance
	// WARNING/HINT/TODO: If we'll ever move ParseConnect() to CServerNetEngine this func should be called last, 'cause *this is deleted

	if(!reconnectFrom)
		newcl->getRights()->Nothing();  // Reset the rights here
	
	// check version compatibility already earlier to not add/kick bots if not needed
	if( iState != SVS_LOBBY ) {
		if(!checkVersionCompatibility(newcl, true, false))
			return; // client is not compatible, so it was dropped out already
	}
	

	
	
	// prepare Welcome message
	std::string strWelcomeMessage = tLXOptions->sWelcomeMessage;
	if(strWelcomeMessage == "<none>") strWelcomeMessage = "";
	if(strWelcomeMessage != "")  {
		
		// Server name3
		replacemax(strWelcomeMessage, "<server>", tLXOptions->sServerName, strWelcomeMessage, 1);
		
		// Host name
		replacemax(strWelcomeMessage, "<me>", cWorms[0].getName(), strWelcomeMessage, 1);
		
		// Version
		replacemax(strWelcomeMessage, "<version>", clientVersion.asHumanString(), strWelcomeMessage, 1);
		
		// Country
		if (strWelcomeMessage.find("<country>") != std::string::npos)  {
			IpInfo info;
			std::string str_addr;
			NetAddrToString(newcl->getChannel()->getAddress(), str_addr);
			if (str_addr != "")  {
				info = tIpToCountryDB->GetInfoAboutIP(str_addr);
				replacemax(strWelcomeMessage, "<country>", info.Country, strWelcomeMessage, 1);
			}
		}
		
		// Continent
		if (strWelcomeMessage.find("<continent>") != std::string::npos)  {
			IpInfo info;
			std::string str_addr;
			NetAddrToString(newcl->getChannel()->getAddress(), str_addr);
			if (str_addr != "")  {
				info = tIpToCountryDB->GetInfoAboutIP(str_addr);
				replacemax(strWelcomeMessage, "<continent>", info.Continent, strWelcomeMessage, 1);
			}
		}
		
		
		// Address
		std::string str_addr;
		NetAddrToString(newcl->getChannel()->getAddress(), str_addr);
		// Remove port
		size_t pos = str_addr.rfind(':');
		if (pos != std::string::npos)
			str_addr.erase(pos);
		replacemax(strWelcomeMessage, "<ip>", str_addr, strWelcomeMessage, 1);
	}
	
	
	
	// Find spots in our list for the worms
	int ids[MAX_PLAYERS];
	for(int i = 0; i < MAX_PLAYERS; ++i) ids[i] = -1;
	
	std::vector<WormJoinInfo> newWorms;
	newWorms.resize(numworms);
	for (int i = 0; i < numworms; i++) {
		newWorms[i].readInfo(bs);

		// If bots aren't allowed, disconnect the client
		if (newWorms[i].m_type == PRF_COMPUTER && !tLXOptions->bAllowRemoteBots && !strincludes(szAddress, "127.0.0.1"))  {
			hints << "Bot was trying to connect from " << newcl->debugName() << endl;
			bytestr.Clear();
			bytestr.writeInt(-1, 4);
			bytestr.writeString("lx::badconnect");
			bytestr.writeString(OldLxCompatibleString(networkTexts->sBotsNotAllowed));
			bytestr.Send(tSocket);
			
			RemoveClient(newcl);
			return;
		}		
	}

	std::set<CWorm*> removeWormList;
	
	if(reconnectFrom) {
		for(int i = 0; i < reconnectFrom->getNumWorms(); ++i) {
			CWorm* w = reconnectFrom->getWorm(i);
			if(!w) {
				warnings << "ParseConnect with reconnecting client: worm nr " << i << " from client is unset" << endl;
				continue;
			}
			if(!w->isUsed()) {
				warnings << "ParseConnect with reconnecting client: worm nr " << i << " from client is not used" << endl;
				continue;
			}
				
			bool found = false;
			for(int j = 0; j < numworms; ++j) {
				// compare by name, we have no possibility to do it more exact but it's also not that important
				if(ids[j] < 0 && newWorms[j].sName == w->getName()) {
					// found one
					found = true;
					ids[j] = w->getID();
					// HINT: Don't apply the other information from WormJoinInfo,
					// the worm should be up-to-date and we could screw it up (e.g. skin or team).
					break;
				}
			}
			
			if(!found) {
				removeWormList.insert(w);
			}
		}
	}
	
	std::set<CWorm*> newJoinedWorms;
	
	// search slots for new worms
	for (int i = 0; i < numworms; ++i) {
		if(ids[i] >= 0) continue; // this worm is already associated
		
		CWorm* w = cWorms;
		for (int j  = 0; j < MAX_WORMS; j++, w++) {
			if (w->isUsed())
				continue;
			
			newWorms[i].applyTo(w);
			w->setID(j);
			w->setClient(newcl);
			w->setUsed(true);
			w->setupLobby();
			w->setDamage(0);
			if( tLX->iGameType == GME_HOST ) // in local play, we use the team-nr from the WormJoinInfo
				w->setTeam(0);
			newcl->setWorm(i, w);
			ids[i] = j;
			iNumPlayers++;

			newJoinedWorms.insert(w);
			
			if( DedicatedControl::Get() )
				DedicatedControl::Get()->NewWorm_Signal(w);
			
			notes << "Worm joined: " << w->getName();
			notes << " (id " << w->getID() << ",";
			notes << " from " << newcl->debugName(false) << ")" << endl;
			
			if(tLXOptions->iRandomTeamForNewWorm > 0 && getGameMode()->GameTeams() > 1) {
				w->setTeam(-1); // set it invalid to have correct firstEmpty
				
				int firstEmpty = getFirstEmptyTeam();
				//notes << "random(" << tLXOptions->iRandomTeamForNewWorm << "): firstempty=" << firstEmpty << endl;
				if(firstEmpty >= 0 && firstEmpty <= tLXOptions->iRandomTeamForNewWorm)
					w->setTeam(firstEmpty);
				else {
					int team = GetRandomInt(MIN(tLXOptions->iRandomTeamForNewWorm, getGameMode()->GameTeams() - 1));
					//notes << "   randomteam=" << team << endl;
					w->setTeam(team);
				}
				// we will send a WormLobbyUpdate later anyway
			}

			// "Has connected" message
			if (networkTexts->sHasConnected != "<none>" && networkTexts->sHasConnected != "")  {
				SendGlobalText(replacemax(networkTexts->sHasConnected, "<player>", w->getName(), 1), TXT_NETWORK);
			}
			
			// Send the welcome message
			if(strWelcomeMessage != "")
				SendGlobalText(replacemax(strWelcomeMessage, "<player>", w->getName(), 1), TXT_NETWORK);
			
			break;
		}
	}

	if(removeWormList.size() > 0)
		RemoveClientWorms(newcl, removeWormList);
	
	// Set the worm info
	newcl->setNumWorms(numworms);
	for (int i = 0;i < numworms;i++) {
		if(ids[i] < 0) {
			// Very strange, we should have catched this case earlier. This means that there were not enough open slots.
			errors << "Server::ParseConnect: We didn't found a slot for " << newWorms[i].sName << " for " << newcl->debugName(false) << endl;
			newcl->setWorm(i, NULL);
			continue;
		}
		newcl->setWorm(i, &cWorms[ids[i]]);
	}
		
	// remove bots if not wanted anymore
	bool sendOutGoodConnection = true;
	if(!reconnectFrom || !reconnectFrom->isLocalClient()) {
		int wormCount = cClient->getNumWorms();
		CheckForFillWithBots();
		if(wormCount != cClient->getNumWorms() && newcl->isLocalClient()) {
			// We are reconnecting the local client and we have a different numworms
			// on the client already, so if we would send a goodconnection now
			// with the outdated (and wrong amount of) worm-ids, we would
			// not parse the goodconnection package correct (because of different numworms).
			sendOutGoodConnection = false;
			notes << "Server: local client has already changed its worms amount, we are waiting for reconnect" << endl;
		}
	}
	
	if(sendOutGoodConnection) {
		// Let em know they connected good
		bytestr.Clear();
		bytestr.writeInt(-1, 4);
		bytestr.writeString("lx::goodconnection");

		// Tell the client the id's of the worms
		for (int i = 0;i < numworms; i++)
			bytestr.writeInt(ids[i], 1);

		bytestr.Send(tSocket);

		// Let them know our version
		bytestr.Clear();
		bytestr.writeInt(-1, 4);
		// sadly we have to send this because it was not thought about any forward-compatibility when it was implemented in Beta3
		// Server version is also added to challenge packet so client will receive it for sure (or won't connect at all).
		bytestr.writeString("lx::openbeta3");
		// sadly we have to send this for Beta4
		// we are sending the version string already in the challenge
		bytestr.writeInt(-1, 4);
		bytestr.writeString("lx::version");
		bytestr.writeString(GetFullGameName());
		bytestr.Send(tSocket);

		//if (tLXOptions->tGameInfo.bAllowMouseAiming)
		{
			bytestr.Clear();
			bytestr.writeInt(-1, 4);
			bytestr.writeString("lx:mouseAllowed");
			bytestr.Send(tSocket);
		}

		if (tLXOptions->tGameInfo.bAllowStrafing)
		{
			bytestr.Clear();
			bytestr.writeInt(-1, 4);
			bytestr.writeString("lx:strafingAllowed");
			bytestr.Send(tSocket);
		}
	}
	
	NotifyUserOnEvent(); // new player connected; if user is away, notify him


	// Tell all the connected clients the info about these worm(s)
	bytestr.Clear();

	//
	// Resend data about all worms to everybody
	// This is so the new client knows about all the worms
	// And the current client knows about the new worms
	//
	w = cWorms;
	for (int i = 0;i < MAX_WORMS;i++, w++) {

		if (!w->isUsed())
			continue;

		bytestr.writeByte(S2C_WORMINFO);
		bytestr.writeInt(w->getID(), 1);
		w->writeInfo(&bytestr);
	}

	SendGlobalPacket(&bytestr);

	
	// just inform everybody in case the client is not compatible
	checkVersionCompatibility(newcl, false);

	if( iState == SVS_LOBBY )
		UpdateGameLobby(); // tell everybody about game lobby details
	else
		// TODO: is that needed? in theory, we should send all important things also in prepare game
		newcl->getNetEngine()->SendUpdateLobbyGame(); // send him the game lobby details

	if (tLX->iGameType != GME_LOCAL) {
		if( iState == SVS_LOBBY )
			SendWormLobbyUpdate(); // to everbody
		else
		{
			for( int i=0; i<MAX_CLIENTS; i++ )
				if( & cClients[i] != newcl )
					SendWormLobbyUpdate( & cClients[i], newcl); // send only data about new client
				else
					SendWormLobbyUpdate(newcl); // send only to new client
		}
	}
	
	// Update players listbox
	DeprecatedGUI::bHost_Update = true;

	// Make host authorised
	if(newcl->isLocalClient())
		newcl->getRights()->Everything();
		
	newcl->setConnectTime(tLX->currentTime);
	
	
	// handling for connect during game
	if( iState != SVS_LOBBY ) {
		// we already check for compatibility earlier
		
		newcl->getShootList()->Clear();
		newcl->setGameReady(false);
		newcl->getUdpFileDownloader()->allowFileRequest(false);
				
		// TODO: why is that needed? some lines above, we already have sent exactly that
		// If this is the host, and we have a team game: Send all the worm info back so the worms know what
		// teams they are on
		if( getGameMode()->GameTeams() > 1 ) {
			
			CWorm *w = cWorms;
			CBytestream b;
			
			for(int i=0; i<MAX_WORMS; i++, w++ ) {
				if( !w->isUsed() )
					continue;
				
				// TODO: move that out here
				// Write out the info
				b.writeByte(S2C_WORMINFO);
				b.writeInt(w->getID(),1);
				w->writeInfo(&b);
			}
			
			newcl->getNetEngine()->SendPacket(&b);
		}

		// Set some info on the worms
		for(std::set<CWorm*>::iterator w = newJoinedWorms.begin(); w != newJoinedWorms.end(); ++w) {
			if((*w)->isPrepared()) {
				warnings << "WARNING: connectduringgame: worm " << (*w)->getID() << " was already prepared! ";
				if(!(*w)->isUsed()) warnings << "AND it is even not used!";
				warnings << endl;
				(*w)->Unprepare();
			}
			
			// If the game has limited lives all new worms are spectators
			if( tLXOptions->tGameInfo.iLives == WRM_UNLIM || iState != SVS_PLAYING || allWormsHaveFullLives() ) // Do not set WRM_OUT if we're in weapon selection screen
				(*w)->setLives(tLXOptions->tGameInfo.iLives);
			else {
				(*w)->setLives(WRM_OUT);
			}
			(*w)->setKills(0);
			(*w)->setGameScript(cGameScript.get());
			(*w)->setWpnRest(&cWeaponRestrictions);
			(*w)->setLoadingTime( (float)tLXOptions->tGameInfo.iLoadingTime / 100.0f );
			(*w)->setWeaponsReady(false);
			
			for(int ii = 0; ii < MAX_CLIENTS; ii++)
				cClients[ii].getNetEngine()->SendWormScore( (*w) );
		}
		
		newcl->getNetEngine()->SendPrepareGame();
		
		// TODO: what is the information of this hint? and does it apply here anyway?
		// Cannot send anything after S2C_PREPAREGAME because of bug in old 0.56 clients - Beta5+ does not have this bug
		
		// inform new client about other ready clients
		CServerConnection *cl = cClients;
		for(int c = 0; c < MAX_CLIENTS; c++, cl++) {
			// Client not connected or no worms
			if(cl->getStatus() == NET_DISCONNECTED || cl->getStatus() == NET_ZOMBIE)
				continue;

			if(cl->getGameReady()) {
				cl->getNetEngine()->SendClientReady(newcl);
			}
		}
		
		// initial server side weapon handling
		if(tLXOptions->tGameInfo.bSameWeaponsAsHostWorm && cClient->getNumWorms() > 0) {
			if(cClient->getWorm(0)->getWeaponsReady()) {
				for(std::set<CWorm*>::iterator w = newJoinedWorms.begin(); w != newJoinedWorms.end(); ++w) {
					(*w)->CloneWeaponsFrom(cClient->getWorm(0));
					(*w)->setWeaponsReady(true);
				}
			}
			// if we are not ready with weapon selection, we will send the new client worms weapons later to everybody
		}
		// If new client is spectating skip weapon selection screen
		// HINT: remove this if we'll get new clients joining and playing with limited lives games
		else if(tLXOptions->tGameInfo.bForceRandomWeapons || 
			( tLXOptions->tGameInfo.iLives != WRM_UNLIM && iState == SVS_PLAYING ) ) {
			for(std::set<CWorm*>::iterator w = newJoinedWorms.begin(); w != newJoinedWorms.end(); ++w) {
				(*w)->GetRandomWeapons();
				(*w)->setWeaponsReady(true);
			}
		}

		// send the client all already selected weapons of the other worms
		SendWeapons(newcl);
	}
}


///////////////////
// Parse a ping packet
void GameServer::ParsePing(NetworkSocket tSocket) 
{
	// Ignore pings in local
	// HINT: this can happen when you quit your server and go play local immediatelly - some
	// people have not updated their serverlist yet and try to ping and query the server
	if (tLX->iGameType != GME_HOST)
		return;

	NetworkAddr		adrFrom;
	GetRemoteNetAddr(tSocket, adrFrom);

	// Send the challenge details back to the client
	SetRemoteNetAddr(tSocket, adrFrom);

	CBytestream bs;

	bs.Clear();
	bs.writeInt(-1, 4);
	bs.writeString("lx::pong");

	bs.Send(tSocket);
}


///////////////////
// Parse a fservertime request packet
void GameServer::ParseTime(NetworkSocket tSocket)
{
	// Ignore pings in local
	// HINT: this can happen when you quit your server and go play local immediatelly - some
	// people have not updated their serverlist yet and try to ping and query the server
	if (tLX->iGameType != GME_HOST)
		return;

	NetworkAddr		adrFrom;
	GetRemoteNetAddr(tSocket, adrFrom);

	// Send the challenge details back to the client
	SetRemoteNetAddr(tSocket, adrFrom);

	CBytestream bs;

	bs.Clear();
	bs.writeInt(-1, 4);
	bs.writeString("lx::timeis");
	bs.writeFloat( (float)fServertime.seconds() );

	bs.Send(tSocket);
}


///////////////////
// Parse a "wants to join" packet
void GameServer::ParseWantsJoin(NetworkSocket tSocket, CBytestream *bs, const std::string& ip) {

	std::string Nick = bs->readString();
	xmlEntityText(Nick);

	// Ignore wants to join in local
	// HINT: this can happen when you quit your server and go play local immediatelly - some
	// people have not updated their serverlist yet and try to ping and query the server
	if (tLX->iGameType != GME_HOST)
		return;

	// Allowed?
	if (!tLXOptions->bAllowWantsJoinMsg)
		return;

	// Accept these messages from banned clients?
	if (!tLXOptions->bWantsJoinBanned && cBanList.isBanned(ip))
		return;

	// Notify about the wants to join
	if (networkTexts->sWantsJoin != "<none>")  {
		std::string buf;
		replacemax(networkTexts->sWantsJoin, "<player>", Nick, buf, 1);
		SendGlobalText(buf, TXT_NORMAL);
	}
}


///////////////////
// Parse a query packet
void GameServer::ParseQuery(NetworkSocket tSocket, CBytestream *bs, const std::string& ip) 
{
	CBytestream bytestr;

	int num = bs->readByte();

	// Ignore queries in local
	// HINT: this can happen when you quit your server and go play local immediatelly - some
	// people have not updated their serverlist yet and try to ping and query the server
	if (tLX->iGameType != GME_HOST)
		return;

	bytestr.writeInt(-1, 4);
	bytestr.writeString("lx::queryreturn");

	// Get Port
	size_t pos = ip.rfind(':');
	if (pos != std::string::npos)
		ip.substr(pos);
	
	//if(ip == "23401")
	//	bytestr.writeString(OldLxCompatibleString(sName+" (private)")); // Not used anyway
	//else
	bytestr.writeString(OldLxCompatibleString(tLXOptions->sServerName));
	bytestr.writeByte(iNumPlayers);
	bytestr.writeByte(tLXOptions->tGameInfo.iMaxPlayers);
	bytestr.writeByte(iState);
	bytestr.writeByte(num);
	// Beta8+ info - old clients will just skip it
	bytestr.writeString( GetGameVersion().asString() );
	bytestr.writeByte( serverAllowsConnectDuringGame() );

	bytestr.Send(tSocket);
}


///////////////////
// Parse a get_info packet
void GameServer::ParseGetInfo(NetworkSocket tSocket) 
{
	// Ignore queries in local
	// HINT: this can happen when you quit your server and go play local immediatelly - some
	// people have not updated their serverlist yet and try to ping and query the server
	if (tLX->iGameType != GME_HOST)
		return;

	CBytestream     bs;

	bs.Clear();
	bs.writeInt(-1, 4);
	bs.writeString("lx::serverinfo");

	bs.writeString(OldLxCompatibleString(tLXOptions->sServerName));
	bs.writeByte(tLXOptions->tGameInfo.iMaxPlayers);
	bs.writeByte(iState);

	// TODO: check if we should append "levels/" string here, it was like this in old code
	bs.writeString( iState == SVS_PLAYING ? "levels/" + tLXOptions->tGameInfo.sMapFile : tLXOptions->tGameInfo.sMapFile );
	bs.writeString(tLXOptions->tGameInfo.sModName);
	bs.writeByte(getGameMode()->GeneralGameType());
	bs.writeInt16(tLXOptions->tGameInfo.iLives);
	bs.writeInt16(tLXOptions->tGameInfo.iKillLimit);
	bs.writeInt16(tLXOptions->tGameInfo.iLoadingTime);
	bs.writeBool(tLXOptions->tGameInfo.bBonusesOn);


	// Players
	int     numplayers = 0;
	int     p;
	CWorm *w = cWorms;
	for (p = 0;p < MAX_WORMS;p++, w++) {
		if (w->isUsed())
			numplayers++;
	}

	bs.writeByte(numplayers);
	w = cWorms;
	for (p = 0;p < MAX_WORMS;p++, w++) {
		if (w->isUsed()) {
			bs.writeString(RemoveSpecialChars(w->getName()));
			bs.writeInt(w->getKills(), 2);
		}
	}

	w = cWorms;
	// Write out lives
	for (p = 0;p < MAX_WORMS;p++, w++) {
		if (w->isUsed())
			bs.writeInt(w->getLives(), 2);
	}

	// Write out IPs
	w = cWorms;
	for (p = 0; p < MAX_WORMS; p++, w++)  {
		if (w->isUsed())  {
			std::string addr;
			if (NetAddrToString(w->getClient()->getChannel()->getAddress(), addr))  {
				size_t pos = addr.find(':');
				if (pos != std::string::npos)
					addr.erase(pos, std::string::npos);
			} else {
				printf("ERROR: Cannot convert address for worm " + w->getName() + "\n");
			}

			if (addr.size() == 0)
				addr = "0.0.0.0";
			bs.writeString(addr);
		}
	}

	// Write out my version (we do this since Beta5)
	bs.writeString(GetFullGameName());

	// since Beta7
	bs.writeFloat(tLXOptions->tGameInfo.features[FT_GameSpeed]);
	
	// since Beta9
	CServerNetEngineBeta9::WriteFeatureSettings(&bs);

	// Game mode name
	bs.writeString(getGameMode()->Name());

	bs.Send(tSocket);
}

struct SendConnectHereAfterTimeout_Data
{
	SendConnectHereAfterTimeout_Data( int _socknum, NetworkAddr _addr )
		{ socknum = _socknum; addr = _addr; };
	int socknum; // Socket idx in CServer - don't know why, probably this doesn't matter
	NetworkAddr addr;
};

void GameServer::SendConnectHereAfterTimeout (Timer::EventData ev)
{
	SendConnectHereAfterTimeout_Data * data = (SendConnectHereAfterTimeout_Data *) ev.userData;

	// This can happen if the user quit the server in the meantime
	if (cServer == NULL || cServer->getClients() == NULL)  {
		delete data;
		ev.shouldContinue = false;
		return;
	}

	NetworkAddr addr;
	ResetNetAddr(addr);
	GetRemoteNetAddr( cServer->tNatTraverseSockets[data->socknum], addr );
	std::string s;
	NetAddrToString( data->addr, s );
	printf("SendConnectHereAfterTimeout() %s:%i\n", s.c_str(), GetNetAddrPort(data->addr) );

	for( int f = 0; f < MAX_CLIENTS; f++ )
	{
		if( cServer->getClients()[f].getStatus() != NET_DISCONNECTED &&
			AreNetAddrEqual( addr, cServer->getClients()[f].getChannel()->getAddress() ) )
		{
			NetAddrToString( cServer->getClients()[f].getChannel()->getAddress(), s );
			printf("SendConnectHereAfterTimeout() socket is used by client %i: %s:%i\n", 
					f, s.c_str(), GetNetAddrPort(cServer->getClients()[f].getChannel()->getAddress()) );
			return;
		}
	}
	
	CBytestream bs;
	bs.writeInt(-1, 4);
	bs.writeString("lx::connect_here");// In case server behind symmetric NAT and client has IP-restricted NAT or above

	int oldPort = GetNetAddrPort(data->addr);
	SetNetAddrPort(data->addr, LX_PORT); // Many people have this port enabled, perhaps we are lucky
	SetRemoteNetAddr( cServer->tNatTraverseSockets[data->socknum], data->addr);
	bs.Send(cServer->tNatTraverseSockets[data->socknum]);
	bs.Send(cServer->tNatTraverseSockets[data->socknum]);
	bs.Send(cServer->tNatTraverseSockets[data->socknum]);
	SetNetAddrPort(data->addr, oldPort);
	SetRemoteNetAddr( cServer->tNatTraverseSockets[data->socknum], data->addr);
	bs.Send(cServer->tNatTraverseSockets[data->socknum]);
	bs.Send(cServer->tNatTraverseSockets[data->socknum]);
	bs.Send(cServer->tNatTraverseSockets[data->socknum]);

	delete data;
	ev.shouldContinue = false;
}

// Parse NAT traverse packet - can be received only with CServer::tSocket, send responce to one of tNatTraverseSockets[]
void GameServer::ParseTraverse(NetworkSocket tSocket, CBytestream *bs, const std::string& ip)
{
	NetworkAddr		adrFrom, adrClient;
	GetRemoteNetAddr(tSocket, adrFrom);
	std::string adrClientStr = bs->readString();
	StringToNetAddr( adrClientStr, adrClient );
	printf("GameServer::ParseTraverse() %s\n", adrClientStr.c_str());

	// Find unused socket
	int socknum=-1;
	for( int f=0; f<MAX_CLIENTS; f++ )
	{
		int f1=0;
		for( ; f1<MAX_CLIENTS; f1++ )
		{
			NetworkAddr addr1, addr2;
			if( cClients[f1].getStatus() == NET_DISCONNECTED || cClients[f1].getChannel() == NULL )
				continue;
			GetLocalNetAddr( cClients[f1].getChannel()->getSocket(), addr1 );
			GetLocalNetAddr( tNatTraverseSockets[f], addr2 );
			if( GetNetAddrPort(addr1) == GetNetAddrPort(addr2) )
				break;
		};
		if( f1 >= MAX_CLIENTS )
		{
			if( socknum == -1 )
				socknum = f;
			else if( fNatTraverseSocketsLastAccessTime[socknum] < fNatTraverseSocketsLastAccessTime[f] )
				socknum = f;
		};
	};
	if( socknum >= MAX_CLIENTS || socknum < 0 )
		return;

	fNatTraverseSocketsLastAccessTime[socknum] = tLX->currentTime;


	//printf("Sending lx:::traverse back, socknum %i\n", socknum);
	// Send lx::traverse to udp server and lx::pong to client
	CBytestream bs1;

	bs1.Clear();
	bs1.writeInt(-1, 4);
	bs1.writeString("lx::traverse");
	bs1.writeString(adrClientStr);

	// Send traverse to server
	SetRemoteNetAddr(tNatTraverseSockets[socknum], adrFrom);
	bs1.Send(tNatTraverseSockets[socknum]);

	// Send ping to client to open NAT port
	bs1.Clear();
	bs1.writeInt(-1, 4);
	bs1.writeString("lx::pong");

	//SetNetAddrPort(adrClient, (ushort)(port + i));
	SetRemoteNetAddr(tNatTraverseSockets[socknum], adrClient);

	// Send 3 times - first packet may be ignored by remote NAT
	bs1.Send(tNatTraverseSockets[socknum]);
	bs1.Send(tNatTraverseSockets[socknum]);
	bs1.Send(tNatTraverseSockets[socknum]);

	bs1.Clear();
	bs1.writeInt(-1, 4);
	bs1.writeString("lx::connect_here");
	bs1.Send(tNatTraverseSockets[socknum]);

	// Send "lx::connect_here" after some time if we're behind symmetric NAT and client has restricted cone NAT or global IP
	Timer( "GS::ParseTraverse SendConnectHereAfterTimeout", &SendConnectHereAfterTimeout,
			new SendConnectHereAfterTimeout_Data(socknum, adrClient), 3000, true ).startHeadless();
}

// Server sent us "lx::registered", that means it's alive - record that
void GameServer::ParseServerRegistered(NetworkSocket tSocket)
{
	// TODO: add code here
	notes << "GameServer::ParseServerRegistered()" << endl;
}

