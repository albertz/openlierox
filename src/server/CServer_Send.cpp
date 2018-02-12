/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Server class - Sending
// Created 1/7/02
// Jason Boettcher

#include <vector>


#include "LieroX.h"
#include "CServer.h"
#include "Debug.h"
#include "StringUtils.h"
#include "CServerConnection.h"
#include "CServerNetEngine.h"
#include "Protocol.h"
#include "CWorm.h"
#include "Timer.h"
#include "Consts.h"
#include "CChannel.h"
#include "CMap.h"
#ifdef DEBUG
#include "MathLib.h"
#endif
#include "CGameMode.h"

// declare them only locally here as nobody really should use them explicitly
std::string OldLxCompatibleString(const std::string &Utf8String);


///////////////////
// Send a client a packet
void CServerNetEngine::SendPacket(CBytestream *bs)
{
	if(cl->getStatus() == NET_DISCONNECTED || cl->getStatus() == NET_ZOMBIE)
		return;

	cl->getChannel()->AddReliablePacketToSend(*bs);
}

///////////////////
// Send all the clients a packet
void GameServer::SendGlobalPacket(CBytestream *bs)
{
	// Assume reliable
	CServerConnection *cl = cClients;
	for(int c = 0; c < MAX_CLIENTS; c++, cl++) {
		if(cl->getStatus() == NET_DISCONNECTED || cl->getStatus() == NET_ZOMBIE) continue;
		if(cl->getNetEngine() == NULL) continue;
		cl->getNetEngine()->SendPacket(bs);
	}
}

void GameServer::SendGlobalPacket(CBytestream *bs, const Version& minVersion)
{
	// Assume reliable
	CServerConnection *cl = cClients;
	for(int c = 0; c < MAX_CLIENTS; c++, cl++) {
		if(cl->getStatus() == NET_DISCONNECTED || cl->getStatus() == NET_ZOMBIE) continue;
		if(cl->getNetEngine() == NULL) continue;
		if(cl->getClientVersion() < minVersion) continue;
		cl->getNetEngine()->SendPacket(bs);
	}
}

// TODO: This function is designed wrong. this->cl should be the receiver
// and the parameter should be the ready client.
void CServerNetEngine::SendClientReady(CServerConnection* receiver) {
	// Let everyone know this client is ready to play
	
	//if(server->serverChoosesWeapons()) {
		// We don't necessarily have to send the weapons, we send them directly
		// (e.g. in cloneWeaponsToAllWorms() or in PrepareWorm()).
		
		// But the client doesn't set CWorm::bGameReady=true, so we still
		// have to send this.
	//}

	if ((receiver && receiver->getClientVersion() >= OLXBetaVersion(8)) || cl->getNumWorms() <= 2)  {
		CBytestream bytes;
		bytes.writeByte(S2C_CLREADY);
		bytes.writeByte(cl->getNumWorms());
		for (int i = 0;i < cl->getNumWorms();i++) {
			if(!cl->getWorm(i) || !cl->getWorm(i)->isUsed()) {
				errors << "SendClientReady: local worm nr " << i << " is wrong" << endl;
				goto SendReadySeperatly; // we cannot send them together 
			}

			// Send the weapon info here (also contains id)
			cl->getWorm(i)->writeWeapons(&bytes);
		}
		
		if(receiver)
			receiver->getNetEngine()->SendPacket(&bytes);
		else
			server->SendGlobalPacket(&bytes);
		
	} else { // old client && numworms > 2

		// Note: LX56 assumes that a client can have only 2 worms.
		
	SendReadySeperatly:
		
		for (int i = 0; i < cl->getNumWorms(); i++) {
			if(!cl->getWorm(i) || !cl->getWorm(i)->isUsed()) {
				errors << "SendClientReady: local worm nr " << i << " is wrong" << endl;
				continue;
			}
			
			CBytestream bytes;
			bytes.writeByte(S2C_CLREADY);
			bytes.writeByte(1);
			cl->getWorm(i)->writeWeapons(&bytes);
			if(receiver)
				receiver->getNetEngine()->SendPacket(&bytes);
			else
				server->SendGlobalPacket(&bytes);
		}
	}
}

void CServerNetEngine::WritePrepareGame(CBytestream *bs) 
{
	bs->writeByte(S2C_PREPAREGAME);
	// TODO: if that is always false, why do we have a variable for it?
	bs->writeBool(server->bRandomMap);	// Always false as of now
	if(!server->bRandomMap)
		bs->writeString("levels/" + tLXOptions->tGameInfo.sMapFile);
	
	// Game info
	bs->writeInt(server->getGameMode()->GeneralGameType(),1);
	bs->writeInt16((tLXOptions->tGameInfo.iLives < 0) ? WRM_UNLIM : tLXOptions->tGameInfo.iLives);
	bs->writeInt16(tLXOptions->tGameInfo.iKillLimit);
	bs->writeInt16((int)(server->getGameMode()->TimeLimit() / 60.0f));
	bs->writeInt16(tLXOptions->tGameInfo.iLoadingTime);
	bs->writeBool(tLXOptions->tGameInfo.bBonusesOn);
	bs->writeBool(tLXOptions->tGameInfo.bShowBonusName);
	if(server->getGameMode()->GeneralGameType() == GMT_TIME)
		bs->writeInt16(tLXOptions->tGameInfo.iTagLimit);
	bs->writeString(tLXOptions->tGameInfo.sModDir);
	
	server->cWeaponRestrictions.sendList(bs, server->cGameScript.get());
}

void CServerNetEngine::SendPrepareGame()
{
	CBytestream bs;	
	WritePrepareGame(&bs);
	SendPacket( &bs );
}

void CServerNetEngine::SendHideWorm(CWorm *worm, int forworm,  bool show, bool immediate)
{
	// For old clients we move the worm out of the map and kill it
	
	if(cl->getNumWorms() == 0 || cl->getWorm(0)->getID() != forworm)
		// ignore it
		return;
	
	CBytestream bs;

	// Hide the worm
	if (!show)  {
		// 
		// Update the position
		//

		if (cServer->getState() == SVS_PLAYING)  {
			bs.writeByte(S2C_UPDATEWORMS);
			bs.writeByte(1);  // Worm count
			bs.writeByte(worm->getID());
			bs.write2Int12(-20, -20);  // Position
			bs.writeInt(0, 1);  // Angle
			bs.writeByte(0);  // Flags
			bs.writeByte(0);  // Weapon

			// Velocity
			if(cl->getClientVersion() >= OLXBetaVersion(5)) {
				bs.writeInt16(0);
				bs.writeInt16(0);
			}

			// Send it reliably, this update is necessary
			SendPacket(&bs);

			//
			// Kill
			//
			SendWormDied(worm);
		}

	// Show the worm
	} else {
		SendSpawnWorm(worm, worm->getPos());
	}
}

void CServerNetEngineBeta7::WritePrepareGame(CBytestream *bs) 
{
	CServerNetEngine::WritePrepareGame(bs);

	bs->writeFloat( tLXOptions->tGameInfo.features[FT_GameSpeed] );

	// Set random weapons for spectating client, so it will skip weapon selection screen
	// Never do this for local client, local client must know correct state of serverChoosesWeapons!
	// TODO: it's hacky, don't have any ideas now how to make it nice
	bool spectate = cl->getNumWorms() > 0 && !cl->isLocalClient();
	if(spectate)
		for(int i = 0; i < cl->getNumWorms(); ++i)
			if(cl->getWorm(i) && !cl->getWorm(i)->isSpectating()) {
				spectate = false;
				break;
			}

	bs->writeBool( server->serverChoosesWeapons() || spectate );
	
	// We send random weapons from server in GameServer::StartGame()
	// TODO: Where are we doing that?
}

void CServerNetEngineBeta9::WritePrepareGame(CBytestream *bs) 
{
	CServerNetEngineBeta7::WritePrepareGame(bs);

	bs->writeFloat(server->getGameMode()->TimeLimit() / 60.0f);
	WriteFeatureSettings(bs);
	bs->writeString(server->getGameMode()->Name());
	
	// TODO: shouldn't this be somewhere in the clear function?
	cDamageReport.clear(); // In case something left from prev game
}


///////////////////
// Send all the clients a string of text
void GameServer::SendGlobalText(const std::string& text, int type) {
	if(!cClients) {
		errors << "GS:SendGlobalText: clients not initialised" << endl;
		return;
	}

	CServerConnection *cl = cClients;
	for(short c = 0; c < MAX_CLIENTS; c++, cl++) {
		if(cl->getStatus() == NET_DISCONNECTED || cl->getStatus() == NET_ZOMBIE)
			continue;

		cl->getNetEngine()->SendText(text, type);
	}
}


///////////////////
// Send a client a string of text
void CServerNetEngine::SendText(const std::string& text, int type) 
{
	CBytestream bs;

	std::string nohtml_text = StripHtmlTags(text);
		
	std::vector<std::string> split = splitstring(nohtml_text, 63, server->iState == SVS_LOBBY ? 600 : 300, tLX->cFont);
	
	for (std::vector<std::string>::const_iterator it = split.begin(); it != split.end(); it++)  {
		// Send it
		bs.writeByte(S2C_TEXT);
		bs.writeInt(type, 1);
		bs.writeString(OldLxCompatibleString(*it));
	}
	
	SendPacket(&bs);
}

void CServerNetEngineBeta3::SendText(const std::string& text, int type)
{
	// For beta 3 - beta 7 (no HTML support but unlimited length support)
	CBytestream bs;

	std::string nohtml_text = StripHtmlTags(text);

	bs.writeByte(S2C_TEXT);
	bs.writeInt(type, 1);
	bs.writeString(OldLxCompatibleString(nohtml_text));

	SendPacket(&bs);
}

void CServerNetEngineBeta3::SendHideWorm(CWorm *worm, int forworm, bool show, bool immediate)
{
	CServerNetEngine::SendHideWorm(worm, show, immediate);  // Just the same as for old LX
}

void CServerNetEngineBeta8::SendText(const std::string& text, int type)
{
	// For beta 8+ (HTML support)
	CBytestream bs;

	bs.writeByte(S2C_TEXT);
	bs.writeInt(type, 1);
	bs.writeString(OldLxCompatibleString(text));

	SendPacket(&bs);
}

void CServerNetEngineBeta7::SendChatCommandCompletionSolution(const std::string& startStr, const std::string& solution) {
	CBytestream bs;

	bs.writeByte(S2C_CHATCMDCOMPLSOL);
	bs.writeString(startStr);
	bs.writeString(solution);

	SendPacket(&bs);	
}

void CServerNetEngineBeta7::SendChatCommandCompletionList(const std::string& startStr, const std::list<std::string>& solutions) {
	// the solutions are for the last parameter of the command (or the command itself if no param is given)!
	CBytestream bs;

	bs.writeByte(S2C_CHATCMDCOMPLLST);
	bs.writeString(startStr);
	bs.writeInt((uint)solutions.size(), 4);
	for(std::list<std::string>::const_iterator it = solutions.begin(); it != solutions.end(); ++it)
		bs.writeString(*it);

	SendPacket(&bs);
}

// send S2C_WORMSOUT
void CServerNetEngine::SendWormsOut(const std::list<byte>& ids) {
	if(ids.size() == 0) return; // ignore
	
	CBytestream bs;
	bs.writeByte(S2C_WORMSOUT);
	bs.writeByte(ids.size());
	
	for(std::list<byte>::const_iterator it = ids.begin(); it != ids.end(); ++it)
		bs.writeByte(*it);
	
	SendPacket(&bs);
}

// WARNING: When using this, be sure that we also drop the specific client. This is
// needed because the local worm amount of the client is different from ours in
// the meanwhile and it would screw up the network.
void GameServer::SendWormsOut(const std::list<byte>& ids) 
{
	for(int c = 0; c < MAX_CLIENTS; c++) {
		CServerConnection* cl = &cClients[c];
		if (cl->getStatus() == NET_DISCONNECTED || cl->getStatus() == NET_ZOMBIE)
			continue;
		
		cl->getNetEngine()->SendWormsOut(ids);
	}
}

///////////////////
// Update all the client about the playing worms
// Returns true if we sent an update
bool GameServer::SendUpdate()
{
	// Delays for different net speeds
	static const float	shootDelay[] = {0.010f, 0.005f, 0.0f, 0.0f};

	//
	// Get the update packets for each worm that needs it and save them
	//
	std::list<CWorm *> worms_to_update;
	CWorm *w = cWorms;
	{
		int i, j;
		for (i = j = 0; j < iNumPlayers && i < MAX_WORMS; i++, w++)  {
			if (!w->isUsed())
				continue;

			// HINT: this can happen when a new client joins during game and has not selected weapons yet
			if (w->getClient())
				if (!w->getClient()->getGameReady())
					continue;

			++j;

			// w is an own server-side copy of the worm-structure,
			// therefore we don't get problems by using the same checkPacketNeeded as client is also using
			if (w->checkPacketNeeded())  {
				worms_to_update.push_back(w);
			}
		}
	}

	size_t uploadAmount = 0;

	{
		int last = lastClientSendData;
		for (int i = 0; i < MAX_CLIENTS; i++)  {
			CServerConnection* cl = &cClients[ (i + lastClientSendData + 1) % MAX_CLIENTS ]; // fairly distribute the packets over the clients

			if (cl->getStatus() == NET_DISCONNECTED || cl->getStatus() == NET_ZOMBIE)
				continue;

			// HINT: happens when clients join during game and haven't selected their weapons yet
			// HINT 2: this should be valid for beta 9+ though so the user can see others playing while selecting weapons
			if (cl->getClientVersion() < OLXBetaVersion(0,58,1))
				if (!cl->getGameReady())
					continue;

			// Check if we have gone over the bandwidth rating for the client
			// If we have, just don't send a packet this frame
			if( !checkBandwidth(cl) ) {
				// We have gone over the bandwidth for the client, don't send a message this frame
				//hints << "Over bandwidth for client " << i << endl;
				continue;
			}

			if(!cl->isLocalClient()) {
				// check our server bandwidth
				static Rate<100,5000> blockRate; // only for debug output
				static Rate<100,5000> blockRateAbs; // only for debug output
				blockRateAbs.addData(tLX->currentTime, 1);
				if(!checkUploadBandwidth(GetUpload() /* + uploadAmount */)) {
					// we have gone over our own bandwidth for non-local clients				
					blockRate.addData(tLX->currentTime, 1);
					static AbsTime lastMessageTime = tLX->currentTime;
					if(tLX->currentTime - lastMessageTime > 30.0) {
						notes << "we got over the max upload bandwidth" << endl;
						notes << "   current upload is " << GetUpload() << " bytes/sec (last 2 secs)" << endl;
						notes << "   current short upload is " << GetUpload(0.1f) << " bytes/sec (last 0.1 sec)" << endl;
						notes << "   upload amount of this frame is " << uploadAmount << " bytes" << endl;
						if(blockRateAbs.getRate() > 0)
							notes << "   current block/update rate is " << float(100.0f * blockRate.getRate() / blockRateAbs.getRate()) << " % (last 5 secs)" << endl;
						lastMessageTime = tLX->currentTime;
					}
					continue;
				}
			}

			CBytestream update_packets;  // Contains all the update packets except the one from this client

			byte num_worms = 0;

			// Send all the _other_ worms details
			{
				std::list<CWorm*>::const_iterator w_it = worms_to_update.begin();
				for(; w_it != worms_to_update.end(); w_it++) {
					CWorm* w = *w_it;

					// Check if this client owns the worm
					if(cl->OwnsWorm(w->getID()))
						continue;
						
					// Give the game mode a chance to override sending a packet (might reduce data sent)
					if(!getGameMode()->NeedUpdate(cl, w))
						continue;

					++num_worms;

					CBytestream bytes;
					bytes.writeByte(w->getID());
					w->writePacket(&bytes, true, cl);

					// Send out the update
					update_packets.Append(&bytes);
				}
			}

			CBytestream *bs = cl->getUnreliable();
			size_t oldBsPos = bs->GetPos();

			// Write the packets to the unreliable bytestream
			bs->writeByte(S2C_UPDATEWORMS);
			bs->writeByte(num_worms);
			bs->Append(&update_packets);
			
			// Write out a stat packet
			{
				bool need_send = false;
				{
					for (short j=0; j < cl->getNumWorms(); j++)
						if (cl->getWorm(j)->checkStatePacketNeeded())  {
							cl->getWorm(j)->updateStatCheckVariables();
							need_send = true;
							break;
						}
				}

				// Only if necessary
				if (need_send)  {
					bs->writeByte( S2C_UPDATESTATS );
					bs->writeByte( cl->getNumWorms() );
					for(short j = 0; j < cl->getNumWorms(); j++)
						cl->getWorm(j)->writeStatUpdate(bs);
				}
			}

			if(!cl->isLocalClient())
				uploadAmount += (bs->GetPos() - oldBsPos);
			
			// Send the shootlist (reliable)
			CShootList *sh = cl->getShootList();
			float delay = shootDelay[cl->getNetSpeed()];

			if(tLX->currentTime - sh->getStartTime() > delay && sh->getNumShots() > 0) {
				CBytestream shootBs;

				// Send the shootlist
				if( sh->writePacket(&shootBs, cl->getClientVersion()) )
					sh->Clear();

				if(!cl->isLocalClient())
					uploadAmount += shootBs.GetLength();
					
				cl->getChannel()->AddReliablePacketToSend(shootBs);
			}
			
			// TODO: that doesn't update uploadAmount
			cl->getNetEngine()->SendReportDamage();

			if(!cl->isLocalClient())
				last = i;
		}
		
		lastClientSendData = last;
	}

	// All good
	return true;
}

void CServerNetEngine::SendWeapons()
{
	CBytestream bs;
	
	CWorm* w = server->cWorms;
	for(int i = 0; i < MAX_WORMS; i++, w++) {
		if(!w->isUsed())
			continue;
		if(!w->getWeaponsReady())
			continue;
		bs.writeByte(S2C_WORMWEAPONINFO);
		w->writeWeapons(&bs);
	}
	
	SendPacket(&bs);
}

///////////////////
// Send weapons to the client, or, if client is NULL, send to all clients
void GameServer::SendWeapons(CServerConnection* cl)
{
	if(cl)
		cl->getNetEngine()->SendWeapons();
	else
		for(int c = 0; c < MAX_CLIENTS; c++)
			cClients[c].getNetEngine()->SendWeapons();
}

///////////////////
// Tells all clients that the worm is now tagged
void GameServer::SendWormTagged(CWorm *w)
{
	// Check
	if (!w)  {
		errors << "A NULL worm passed to GameServer::SendWormTagged" << endl;
		return;
	}

	// Build the packet
	CBytestream bs;
	bs.writeByte(S2C_TAGUPDATE);
	bs.writeInt(w->getID(), 1);
	bs.writeFloat((float)w->getTagTime().seconds());

	// Send
	SendGlobalPacket(&bs);
}

///////////////////
// Check if we have gone over the clients bandwidth rating
// Returns true if we are under the bandwidth
bool GameServer::checkBandwidth(CServerConnection *cl)
{
	// Don't bother checking if the client is on the same comp as the server
	if( tLX->iGameType == GME_LOCAL )
		return true;
	if(cl->getNetSpeed() == 3) // local
		return true;


	// Modem, ISDN, LAN, local
	// (Bytes per second)
	const float	Rates[4] = {2500, 7500, 10000, 50000};

	// Are we over the clients bandwidth rate?
	if(cl->getChannel()->getOutgoingRate() > Rates[cl->getNetSpeed()]) {

		// Don't send the packet
		return false;
	}

	// All ok
	return true;
}

// true means we can send further data
bool GameServer::checkUploadBandwidth(float fCurUploadRate) {
	if( tLX->iGameType == GME_LOCAL )
		return true;

	float fMaxRate = getMaxUploadBandwidth();
	
	{
		static bool didShowMessageAlready = false;
		if(!didShowMessageAlready)
			notes << "using max upload rate " << (fMaxRate / 1024.0f) << " kb/sec" << endl;
		didShowMessageAlready = true;
	}
	
	return fCurUploadRate < fMaxRate;
}

void CServerNetEngineBeta9::WriteFeatureSettings(CBytestream* bs) {
	int ftC = featureArrayLen();
	assert(ftC < 256*256);
	CBytestream bs1;
	int sendCount = 0;
	foreach( Feature*, f, Array(featureArray,ftC) )
	{
		if( f->get()->group < GIG_GameModeSpecific_Start ||
			f->get()->group == cServer->getGameMode()->getGameInfoGroupInOptions() )
		{
			if( tLXOptions->tGameInfo.features.hostGet(f->get()) == f->get()->unsetValue ) // Do not send a feature if it has default value = LX56 behavior
				continue;
			sendCount ++;
			bs1.writeString( f->get()->name );
			bs1.writeString( f->get()->humanReadableName );
			bs1.writeVar( tLXOptions->tGameInfo.features.hostGet(f->get()) );
			bs1.writeBool( tLXOptions->tGameInfo.features.olderClientsSupportSetting(f->get()) );
		}
	}
	bs->writeInt(sendCount, 2);
	bs->Append(&bs1);
}

void CServerNetEngine::SendUpdateLobbyGame()
{
	CBytestream bs;
	WriteUpdateLobbyGame(&bs);
	SendPacket(&bs);
}

void CServerNetEngine::WriteUpdateLobbyGame(CBytestream *bs)
{
	bs->writeByte(S2C_UPDATELOBBYGAME);
	bs->writeByte(MAX(tLXOptions->tGameInfo.iMaxPlayers,server->getNumPlayers()));  // This fixes the player disappearing in lobby
	bs->writeString(tLXOptions->tGameInfo.sMapFile);
	bs->writeString(tLXOptions->tGameInfo.sModName);
	bs->writeString(tLXOptions->tGameInfo.sModDir);
	bs->writeByte(server->getGameMode()->GeneralGameType());
	bs->writeInt16((tLXOptions->tGameInfo.iLives < 0) ? WRM_UNLIM : tLXOptions->tGameInfo.iLives);
	bs->writeInt16(tLXOptions->tGameInfo.iKillLimit);
	bs->writeInt16(tLXOptions->tGameInfo.iLoadingTime);
	bs->writeByte(tLXOptions->tGameInfo.bBonusesOn);
}

void CServerNetEngineBeta7::WriteUpdateLobbyGame(CBytestream *bs)
{
	CServerNetEngine::WriteUpdateLobbyGame(bs);
	bs->writeFloat(tLXOptions->tGameInfo.features[FT_GameSpeed]);
	bs->writeBool(tLXOptions->tGameInfo.bForceRandomWeapons);
	bs->writeBool(tLXOptions->tGameInfo.bSameWeaponsAsHostWorm);
}

void CServerNetEngineBeta9::WriteUpdateLobbyGame(CBytestream *bs)
{
	CServerNetEngineBeta7::WriteUpdateLobbyGame(bs);
	bs->writeFloat(server->getGameMode()->TimeLimit() / 60.0f);
	CServerNetEngineBeta9::WriteFeatureSettings(bs);
	bs->writeString(server->getGameMode()->Name());
}


///////////////////
// Send an update of the game details in the lobby
void GameServer::UpdateGameLobby()
{
	if(getGameMode() == NULL) {
		errors << "Trying to play a non-existant gamemode" << endl;
		tLXOptions->tGameInfo.gameMode = GameMode(GM_DEATHMATCH);
	}
	
	// Read map/mod name from map/mod file
	tLXOptions->tGameInfo.sMapName = CMap::GetLevelName(tLXOptions->tGameInfo.sMapFile);
	CGameScript::CheckFile(tLXOptions->tGameInfo.sModDir, tLXOptions->tGameInfo.sModName);
	
	m_clientsNeedLobbyUpdate = true;
	m_clientsNeedLobbyUpdateTime = tLX->currentTime;
}

void CServerNetEngine::SendUpdateLobby(CServerConnection *target)
{
    CBytestream bytestr;

    CServerConnection *cl = server->cClients;
	for(short c=0; c<MAX_CLIENTS; c++,cl++) 
	{
	    if( target )
	    {
    		cl = target;
			if( c != 0 )
				break;
    	}
			
        if( cl->getStatus() == NET_DISCONNECTED || cl->getStatus() == NET_ZOMBIE )
            continue;

        // Set the client worms lobby ready state
        bool ready = false;
	    for(short i=0; i < cl->getNumWorms(); i++) {
		    ready = cl->getWorm(i)->getLobbyReady();
	    }

	    // Let all the worms know about the new lobby state
		if (cl->getNumWorms() <= 2)  { // Have to do this way because of bug in LX 0.56
			bytestr.writeByte(S2C_UPDATELOBBY);
			bytestr.writeByte(cl->getNumWorms());
			bytestr.writeByte(ready);
			for(short i=0; i<cl->getNumWorms(); i++) {
				bytestr.writeByte(cl->getWorm(i)->getID());
				bytestr.writeByte(cl->getWorm(i)->getTeam());
			}
		} else {
			int written = 0;
			while (written < cl->getNumWorms())  {
				bytestr.writeByte(S2C_UPDATELOBBY);
				bytestr.writeByte(1);
				bytestr.writeByte(ready);
				bytestr.writeByte(cl->getWorm(written)->getID());
				bytestr.writeByte(cl->getWorm(written)->getTeam());
				written++;
			}
		}
    }
	SendPacket(&bytestr);
}

////////////////////////
// Hide a worm at receiver's screen
void CServerNetEngineBeta9::SendHideWorm(CWorm *worm, int forworm, bool show, bool immediate)
{
	if (!worm)  {
		errors << "Invalid worm or receiver in SendHideWorm" << endl;
		return;
	}

	if(forworm < 0 || forworm >= MAX_WORMS) {
		errors << "Invalid forworm in SendHideWorm" << endl;
		return;
	}
	
	CBytestream bs;
	bs.writeByte(S2C_HIDEWORM);
	bs.writeByte(worm->getID());
	bs.writeByte(forworm);
	bs.writeBool(!show);  // True - hide, false - show
	bs.writeBool(immediate);  // True - immediate (no animation), false - animation
	SendPacket(&bs);
}

///////////////////
// Send updates for all the worm lobby states
void GameServer::SendWormLobbyUpdate(CServerConnection* receiver, CServerConnection *target)
{
	if(receiver)
		receiver->getNetEngine()->SendUpdateLobby(target);
	else
		for( int i = 0; i < MAX_CLIENTS; i++ ) {
			if(!cClients[i].isUsed()) continue;
			cClients[i].getNetEngine()->SendUpdateLobby(target);
		}
}


///////////////////
// Tell all the clients that we're disconnecting
void GameServer::SendDisconnect()
{
	CServerConnection *cl = cClients;
	if (!cl) // means we already shut down
		return;

	CBytestream bs;
	bs.writeByte(S2C_LEAVING);

	for(short c=0; c<MAX_CLIENTS; c++,cl++) {
		if(cl->getStatus() == NET_DISCONNECTED)
			continue;

		// Send it out-of-bounds 3 times to make sure all the clients received it
		for(short i=0; i<3; i++)
			cl->getChannel()->Transmit(&bs);
	}
}


///////////////////
// Update the worm name, skin, colour etc
void CServerNetEngine::SendUpdateWorm( CWorm* w )
{
	CBytestream bytestr;
	bytestr.writeByte(S2C_WORMINFO);
	bytestr.writeInt(w->getID(), 1);
	w->writeInfo(&bytestr);
	SendPacket(&bytestr);
}

// Version required to show question mark on damage popup number for older clients
void CServerNetEngineBeta9::SendUpdateWorm( CWorm* w )
{
	CBytestream bytestr;
	bytestr.writeByte(S2C_WORMINFO);
	bytestr.writeInt(w->getID(), 1);
	w->writeInfo(&bytestr);
	bytestr.writeString(w->getClient()->getClientVersion().asString());
	SendPacket(&bytestr);
}

void GameServer::UpdateWorm(CWorm* w)
{
	for(int i = 0; i < MAX_CLIENTS; i++)
		cClients[i].getNetEngine()->SendUpdateWorm(w);
}

///////////////////
// Update the worm names, skins, colours etc
void GameServer::UpdateWorms()
{
	CWorm* w = cWorms;
	for(int i = 0; i < MAX_WORMS; i++, w++) {
		if(!w->isUsed())
			continue;
		UpdateWorm(w);
	}
}

#ifdef FUZZY_ERROR_TESTING
///////////////
// Used for testing network stability
void GameServer::SendRandomPacket()
{
	CBytestream bs;
	int random_length = GetRandomInt(50);
	for (int i=0; i < random_length; i++)
		bs.writeByte((uchar)GetRandomInt(255));

	CServerConnection* cl = cClients;
	for(short c=0; c<MAX_CLIENTS; c++,cl++) {
		if(cl->getStatus() == NET_DISCONNECTED || cl->getStatus() == NET_ZOMBIE)
			continue;

		// don't send these random clients to the local client
		if(cl->isLocalClient())
			continue;
		
		cl->SendPacket(&bs);
	}
}
#endif

static const float pingCoeff = 1.5f;	// Send another packet in minPing/pingCoeff milliseconds
static const int minPingDefault = 200;

int CServerNetEngineBeta5::SendFiles()
{
	if(cl->getStatus() == NET_DISCONNECTED || cl->getStatus() == NET_ZOMBIE)
		return 0;
	int ping = 0;
	// That's a bit floody algorithm, it can be optimized I think
	if( cl->getUdpFileDownloader()->isSending() &&
		( cl->getChannel()->getBufferEmpty() ||
			( ! cl->getChannel()->getBufferFull() &&
			cl->getChannel()->getPing() != 0 &&
			tLX->currentTime - cl->getLastFileRequestPacketReceived() <= cl->getChannel()->getPing()/1000.0f / pingCoeff )) )
	{
		cl->setLastFileRequestPacketReceived( tLX->currentTime );
		CBytestream bs;
		bs.writeByte(S2C_SENDFILE);
		cl->getUdpFileDownloader()->send(&bs);
		cl->getNetEngine()->SendPacket( &bs );
		ping = minPingDefault; // Default assumed ping
		if( cl->getChannel()->getPing() != 0 )
			ping = cl->getChannel()->getPing();
	}
	return ping;
}

void GameServer::SendFiles()
{
	// To keep sending packets if no acknowledge received from client -
	// process will pause for a long time otherwise, 'cause we're in GUI part
	bool startTimer = false;
	int minPing = minPingDefault;

	for(int c = 0; c < MAX_CLIENTS; c++)
	{
		if(!cClients[c].getNetEngine()) continue;
		int ping = cClients[c].getNetEngine()->SendFiles();
		if( ping > 0 )
		{
			startTimer = true;
			if( minPing > ping )
				minPing = ping;
		}
	}

	if( startTimer )
		Timer( "GS::SendFiles", null, NULL, (Uint32)(minPing / pingCoeff), true ).startHeadless();
}

void GameServer::SendEmptyWeaponsOnRespawn( CWorm * Worm )
{
	CBytestream bs;
	CServerConnection * cl = Worm->getClient();
	if(cl == NULL) {
		errors << "GS::SendEmptyWeaponsOnRespawn: client of worm " << Worm->getID() << ":" << Worm->getName() << " is unset" << endl;
		DumpConnections();
		return;
	}
	int i, j, curWeapon = Worm->getCurrentWeapon();
	for( i = 0; i < 5; i++ )
	{
		Worm->getWeapon(i)->Charge=0;
		Worm->getWeapon(i)->Reloading=1;
	}
	for( i=0; i<5; i++ )
	{
		if( i != curWeapon )
		{
			Worm->setCurrentWeapon(i);
			bs.writeByte( S2C_UPDATESTATS );
			bs.writeByte( cl->getNumWorms() );
			for( j = 0; j < cl->getNumWorms(); j++ )
				cl->getWorm(j)->writeStatUpdate(&bs);
		}
	}
	Worm->setCurrentWeapon(curWeapon);
	bs.writeByte( S2C_UPDATESTATS );
	bs.writeByte( cl->getNumWorms() );
	for( j = 0; j < cl->getNumWorms(); j++ )
		cl->getWorm(j)->writeStatUpdate(&bs);
	cl->getNetEngine()->SendPacket(&bs);
}

void CServerNetEngine::SendSpawnWorm(CWorm *Worm, CVec pos)
{
	server->getWorms()[Worm->getID()].setSpawnedOnce();
	
	CBytestream bs;
	bs.writeByte(S2C_SPAWNWORM);
	bs.writeInt(Worm->getID(), 1);
	bs.writeInt( (int)pos.x, 2);
	bs.writeInt( (int)pos.y, 2);

	SendPacket(&bs);
}

void CServerNetEngine::SendWormDied(CWorm *Worm)
{
	CBytestream bs;
	bs.writeByte(S2C_WORMDOWN);
	bs.writeInt(Worm->getID(), 1);

	SendPacket(&bs);
}

void CServerNetEngine::SendWormScore(CWorm *Worm)
{
	CBytestream bs;
	bs.writeByte(S2C_SCOREUPDATE);
	bs.writeInt(Worm->getID(), 1);
	bs.writeInt16(Worm->getLives());

	// Overflow hack
	if (Worm->getScore() > 255)
		bs.writeInt(255, 1);
	else
		bs.writeInt(Worm->getScore() > 0 ? Worm->getScore() : 0, 1);

	SendPacket(&bs);
}

void CServerNetEngineBeta9::SendWormScore(CWorm *Worm)
{
	// If we have some damage reports in buffer send them first so clients won't sum up updated damage score and reported damage packet sent later
	SendReportDamage(true);

	CBytestream bs;
	bs.writeByte(S2C_SCOREUPDATE);
	bs.writeInt(Worm->getID(), 1);
	bs.writeInt16(Worm->getLives());	// Still int16 to allow WRM_OUT parsing (maybe I'm wrong though)
	bs.writeInt(Worm->getScore(), 4); // Negative kills are allowed
	bs.writeFloat(Worm->getDamage());

	SendPacket(&bs);
}

void CServerNetEngineBeta9::QueueReportDamage(int victim, float damage, int offender)
{
	// Buffer up all damage and send it once per 0.1 second for LAN nettype, or once per 0.3 seconds for modem
	std::pair< int, int > dmgPair = std::make_pair( victim, offender );
	if( cDamageReport.count( dmgPair ) == 0 )
		cDamageReport[ dmgPair ] = 0;
	cDamageReport[ dmgPair ] += damage;
	
	SendReportDamage();
}

void CServerNetEngineBeta9::SendReportDamage(bool flush)
{
	if( ! flush && tLX->currentTime - fLastDamageReportSent < 0.1f * ( NST_LOCAL - cl->getNetSpeed() ) )
		return;

	CBytestream bs;

	for( std::map< std::pair< int, int >, float > :: iterator it = cDamageReport.begin(); 
			it != cDamageReport.end(); it++ )
	{
		int victim = it->first.first;
		int offender = it->first.second;
		float damage = it->second;
		bs.writeByte(S2C_REPORTDAMAGE);
		bs.writeByte(victim);
		bs.writeFloat(damage);
		bs.writeByte(offender);
	}

	SendPacket(&bs);

	cDamageReport.clear();
	fLastDamageReportSent = tLX->currentTime;
}

void CServerNetEngineBeta9::SendTeamScoreUpdate() {
	// only do this in a team game
	if(server->getGameMode()->GeneralGameType() != GMT_TEAMS) return;

	CBytestream bs;
	bs.writeByte(S2C_TEAMSCOREUPDATE);
	bs.writeByte(server->getGameMode()->GameTeams());
	for(int i = 0; i < server->getGameMode()->GameTeams(); ++i) {
		bs.writeInt16(server->getGameMode()->TeamScores(i));
	}	
	SendPacket(&bs);
}

void GameServer::SendTeamScoreUpdate() {
	for(int c = 0; c < MAX_CLIENTS; c++) {
		if(cClients[c].getStatus() == NET_CONNECTED)
			cClients[c].getNetEngine()->SendTeamScoreUpdate();
	}
}

void CServerNetEngine::SendWormProperties(bool onlyIfNotDef) {
	CWorm* w = server->getWorms();
	for(int i = 0; i < MAX_WORMS; ++i, ++w) {
		if(!w->isUsed()) continue;
		if(onlyIfNotDef && isWormPropertyDefault(w)) continue;
		
		SendWormProperties(w);
	}
}

void CServerNetEngine::SendWormProperties(CWorm* worm) {
	if(!worm->isUsed()) {
		warnings << "SendWormProperties called for unused worm" << endl;
		return;
	}
	
	if(isWormPropertyDefault(worm)) return; // ok, don't give a warning in that case
	
	warnings << "SendWormProperties cannot be used for clients with <Beta9 (" << cl->debugName() << ")" << endl;
}

void CServerNetEngineBeta9::SendWormProperties(CWorm* worm) {
	if(!worm->isUsed()) {
		warnings << "SendWormProperties called for unused worm" << endl;
		return;
	}

	CBytestream bs;
	bs.writeByte(S2C_SETWORMPROPS);
	bs.writeByte(worm->getID());
	bs.writeBit(worm->canUseNinja());
	bs.writeBit(worm->canAirJump());
	bs.writeFloat(worm->speedFactor());
	bs.writeFloat(worm->damageFactor());
	bs.writeFloat(worm->shieldFactor());
	SendPacket(&bs);
}

bool CServerNetEngine::isWormPropertyDefault(CWorm* worm) {
	// defaults are set in CWorm::Prepare
	return
		worm->speedFactor() == 1.0f &&
		worm->damageFactor() == 1.0f &&
		worm->shieldFactor() == 1.0f &&
		worm->canUseNinja() &&
		!worm->canAirJump();
}

void CServerNetEngine::SendSelectWeapons(CWorm* worm) {
	warnings << "SendSelectWeapons not supported for " << cl->debugName() << endl;
}

void CServerNetEngineBeta9::SendSelectWeapons(CWorm* worm) {
	if(!worm->isUsed()) {
		warnings << "SendSelectWeapons called for unused worm" << endl;
		return;
	}

	CBytestream bs;
	bs.writeByte(S2C_SELECTWEAPONS);
	bs.writeByte(worm->getID());
	SendPacket(&bs);
}

void GameServer::SetWormSpeedFactor(int wormID, float f) {
	if(wormID < 0 || wormID >= MAX_WORMS || !cWorms[wormID].isUsed()) {
		warnings << "SetWormSpeedFactor: worm " << wormID << " is invalid" << endl;
		return;
	}
	
	if(cWorms[wormID].speedFactor() == f) return; // nothing need to be changed
	
	cWorms[wormID].setSpeedFactor(f);
	
	for(int c = 0; c < MAX_CLIENTS; c++) {
		if(cClients[c].getStatus() == NET_CONNECTED)
			cClients[c].getNetEngine()->SendWormProperties(&cWorms[wormID]);
	}	
}

void GameServer::SetWormCanUseNinja(int wormID, bool b) {
	if(wormID < 0 || wormID >= MAX_WORMS || !cWorms[wormID].isUsed()) {
		warnings << "SetWormCanUseNinja: worm " << wormID << " is invalid" << endl;
		return;
	}
	
	if(cWorms[wormID].canUseNinja() == b) return; // nothing need to be changed
	
	cWorms[wormID].setCanUseNinja(b);
	
	for(int c = 0; c < MAX_CLIENTS; c++) {
		if(cClients[c].getStatus() == NET_CONNECTED)
			cClients[c].getNetEngine()->SendWormProperties(&cWorms[wormID]);
	}		
}

void GameServer::SetWormDamageFactor(int wormID, float f) {
	if(wormID < 0 || wormID >= MAX_WORMS || !cWorms[wormID].isUsed()) {
		warnings << "SetWormDamageFactor: worm " << wormID << " is invalid" << endl;
		return;
	}
	
	if(cWorms[wormID].damageFactor() == f) return; // nothing need to be changed
	
	cWorms[wormID].setDamageFactor(f);
	
	for(int c = 0; c < MAX_CLIENTS; c++) {
		if(cClients[c].getStatus() == NET_CONNECTED)
			cClients[c].getNetEngine()->SendWormProperties(&cWorms[wormID]);
	}
}

void GameServer::SetWormShieldFactor(int wormID, float f) {
	if(wormID < 0 || wormID >= MAX_WORMS || !cWorms[wormID].isUsed()) {
		warnings << "SetWormDamageFactor: worm " << wormID << " is invalid" << endl;
		return;
	}
	
	if(cWorms[wormID].shieldFactor() == f) return; // nothing need to be changed
	
	cWorms[wormID].setShieldFactor(f);
	
	for(int c = 0; c < MAX_CLIENTS; c++) {
		if(cClients[c].getStatus() == NET_CONNECTED)
			cClients[c].getNetEngine()->SendWormProperties(&cWorms[wormID]);
	}
}

void GameServer::SetWormCanAirJump(int wormID, bool b) {
	if(wormID < 0 || wormID >= MAX_WORMS || !cWorms[wormID].isUsed()) {
		warnings << "SetWormCanAirJump: worm " << wormID << " is invalid" << endl;
		return;
	}
	
	if(cWorms[wormID].canAirJump() == b) return; // nothing need to be changed
	
	cWorms[wormID].setCanAirJump(b);
	
	for(int c = 0; c < MAX_CLIENTS; c++) {
		if(cClients[c].getStatus() == NET_CONNECTED)
			cClients[c].getNetEngine()->SendWormProperties(&cWorms[wormID]);
	}		
}
