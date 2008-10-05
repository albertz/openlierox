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
#include "StringUtils.h"
#include "CServer.h"
#include "CServerConnection.h"
#include "Protocol.h"
#include "CWorm.h"
#include "Timer.h"
#include "Consts.h"
#ifdef DEBUG
#include "MathLib.h"
#endif

using namespace std;

///////////////////
// Send a client a packet
void GameServer::SendPacket(CBytestream *bs, CServerConnection *cl)
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

	for(short c = 0; c < MAX_CLIENTS; c++, cl++) {
		if(cl->getStatus() == NET_DISCONNECTED || cl->getStatus() == NET_ZOMBIE)
			continue;

		cl->getChannel()->AddReliablePacketToSend(*bs);
	}
}


///////////////////
// Send all the clients a string of text
void GameServer::SendGlobalText(const std::string& text, int type) {
	CBytestream bs, oldbs, nohtmlbs;

	std::string nohtml_text = StripHtmlTags(text);

	// HINT: if the message is longer than 64 characters, we split it in more messages
	// (else we could exploit old clients... :( )
	const std::vector<std::string>& split = splitstring(nohtml_text, 63, iState == SVS_LOBBY ? 600 : 300, tLX->cFont);

	for (std::vector<std::string>::const_iterator it = split.begin(); it != split.end(); it++)  {
		// Send it
		oldbs.writeByte(S2C_TEXT);
		oldbs.writeInt(type, 1);
		oldbs.writeString(*it);
	}

	// For beta 3 - beta 7 (no HTML support but unlimited length support)
	nohtmlbs.writeByte(S2C_TEXT);
	nohtmlbs.writeInt(type, 1);
	nohtmlbs.writeString(nohtml_text);

	// For beta 8+ (HTML support)
	bs.writeByte(S2C_TEXT);
	bs.writeInt(type, 1);
	bs.writeString(text);

	CServerConnection *cl = cClients;
	for(short c = 0; c < MAX_CLIENTS; c++, cl++) {
		if(cl->getStatus() == NET_DISCONNECTED || cl->getStatus() == NET_ZOMBIE)
			continue;

		if (cl->getClientVersion() < OLXBetaVersion(3))
			cl->getChannel()->AddReliablePacketToSend(oldbs);
		else if (cl->getClientVersion() < OLXBetaVersion(8))
			cl->getChannel()->AddReliablePacketToSend(nohtmlbs);
		else
			cl->getChannel()->AddReliablePacketToSend(bs);
	}
}


///////////////////
// Send a client a string of text
void GameServer::SendText(CServerConnection *cl, const std::string& text, int type) {
	CBytestream bs;

	bs.writeByte(S2C_TEXT);
	bs.writeInt(type,1);
	bs.writeString(text);

	cl->getChannel()->AddReliablePacketToSend(bs);
}

void GameServer::SendChatCommandCompletionSolution(CServerConnection* cl, const std::string& startStr, const std::string& solution) {
	CBytestream bs;

	bs.writeByte(S2C_CHATCMDCOMPLSOL);
	bs.writeString(startStr);
	bs.writeString(solution);

	cl->getChannel()->AddReliablePacketToSend(bs);	
}

void GameServer::SendChatCommandCompletionList(CServerConnection* cl, const std::string& startStr, const std::list<std::string>& solutions) {
	CBytestream bs;

	bs.writeByte(S2C_CHATCMDCOMPLLST);
	bs.writeString(startStr);
	bs.writeInt((uint)solutions.size(), 4);
	for(std::list<std::string>::const_iterator it = solutions.begin(); it != solutions.end(); ++it)
		bs.writeString(*it);

	cl->getChannel()->AddReliablePacketToSend(bs);
}



// send S2C_WORMSOUT
void GameServer::SendWormsOut(const std::list<byte>& ids) {
	CBytestream bs;
	bs.writeByte(S2C_WORMSOUT);
	bs.writeByte(ids.size());
	
	for(std::list<byte>::const_iterator it = ids.begin(); it != ids.end(); ++it)
		bs.writeByte(*it);
	
	SendGlobalPacket(&bs);
}



///////////////////
// Update all the client about the playing worms
// Returns true if we sent an update
bool GameServer::SendUpdate()
{
	// Delays for different net speeds
	static const float	shootDelay[] = {0.025f, 0.010f, 0.005f, -1.0f};

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

			++j;

			// w is an own server-side copy of the worm-structure,
			// therefore we don't get problems by using the same checkPacketNeeded as client is also using
			//if (w->checkPacketNeeded())
				worms_to_update.push_back(w);
		}
	}


	{
		for (int i = 0; i < MAX_CLIENTS; i++)  {
			CServerConnection* cl = &cClients[ (i + iServerFrame) % MAX_CLIENTS ]; // fairly distribute the packets over the clients

			if (cl->getStatus() == NET_DISCONNECTED || cl->getStatus() == NET_ZOMBIE)
				continue;

			// Check if we have gone over the bandwidth rating for the client
			// If we have, just don't send a packet this frame
			if( !checkBandwidth(cl) ) {
				// We have gone over the bandwidth for the client, don't send a message this frame
				//printf("over bandwidth for client %i\n", i);
				continue;
			}

			// check our server bandwidth
			if( cl->getNetSpeed() < 3 // <3 is non-local
			&& !checkUploadBandwidth(GetUpload()) ) {
				// we have gone over our own bandwidth for non-local clients
				//printf("over upload bandwidth\n");
				continue;
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

					++num_worms;

					CBytestream bytes;
					bytes.writeByte(w->getID());
					w->writePacket(&bytes, true, cl);

					// Send out the update
					update_packets.Append(&bytes);
				}
			}

			CBytestream *bs = cl->getUnreliable();

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

    		{
				// Send the shootlist (reliable)
				CShootList *sh = cl->getShootList();
				float delay = shootDelay[cl->getNetSpeed()];

				if(tLX->fCurTime - sh->getStartTime() > delay && sh->getNumShots() > 0) {
					CBytestream shootBs;

					// Send the shootlist
					if( sh->writePacket(&shootBs) )
						sh->Clear();

					cl->getChannel()->AddReliablePacketToSend(shootBs);
				}
			}
		}
	}

	// All good
	return true;
}

void GameServer::sendWeapons() {
	CBytestream bs;
	
	CWorm* w = cWorms;
	for(int i = 0; i < MAX_WORMS; i++, w++) {
		if(!w->isUsed())
			continue;
		bs.writeByte(S2C_WORMWEAPONINFO);
		w->writeWeapons(&bs);
	}
	
	SendGlobalPacket(&bs);
}


///////////////////
// Check if we have gone over the clients bandwidth rating
// Returns true if we are under the bandwidth
bool GameServer::checkBandwidth(CServerConnection *cl)
{
	// Don't bother checking if the client is on the same comp as the server
	if( tGameInfo.iGameType != GME_LOCAL )
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
	if( tGameInfo.iGameType != GME_LOCAL )
		return true;

	// Modem, ISDN, LAN, local
	// (Bytes per second)
	const float	Rates[4] = {2500, 7500, 10000, 50000};

	float fMaxRate = Rates[tLXOptions->iNetworkSpeed];
	if(tLXOptions->iNetworkSpeed >= 2) {
		// only use Network.MaxServerUploadBandwidth option if we set Network.Speed to LAN (or higher)
		fMaxRate = MAX(fMaxRate, (float)tLXOptions->iMaxUploadBandwidth);
	}

	{
		static bool didShowMessageAlready = false;
		if(!didShowMessageAlready)
			printf("using max upload rate %f kb/sec\n", fMaxRate / 1024.0f);
		didShowMessageAlready = true;
	}

	return fCurUploadRate < fMaxRate;
}

///////////////////
// Send an update of the game details in the lobby
void GameServer::UpdateGameLobby(void)
{
	game_lobby_t *gl = &tGameLobby;

	// TODO: Temporary hack, we should move game_t and game_lobby_t into GameOptions.
	gl->nGameMode = tLXOptions->tGameinfo.iGameMode = tGameInfo.iGameMode;
	gl->nLives = tLXOptions->tGameinfo.iLives = tGameInfo.iLives;
	gl->fGameSpeed = tLXOptions->tGameinfo.fGameSpeed = tGameInfo.fGameSpeed;
	gl->nMaxWorms = tLXOptions->tGameinfo.iMaxPlayers;
	gl->nMaxKills = tLXOptions->tGameinfo.iKillLimit = tGameInfo.iKillLimit;
	gl->nLoadingTime = tLXOptions->tGameinfo.iLoadingTime = tGameInfo.iLoadingTimes;
	gl->bBonuses = tLXOptions->tGameinfo.bBonusesOn = tGameInfo.bBonusesOn;
	gl->szMapFile = tLXOptions->tGameinfo.sMapFilename = tGameInfo.sMapFile;
	gl->szModName = tGameInfo.sModName;
	gl->szModDir = tLXOptions->tGameinfo.szModDir = tGameInfo.sModDir;
	gl->bForceRandomWeapons = tLXOptions->tGameinfo.bForceRandomWeapons;
	gl->bSameWeaponsAsHostWorm = tLXOptions->tGameinfo.bSameWeaponsAsHostWorm;
	
	// TODO: move this code out here

	CServerConnection *cl = cClients;

	for(int i = 0; i < MAX_CLIENTS; i++, cl++) {
		if(cl->getStatus() != NET_CONNECTED)
			continue;

		CBytestream bs;
		bs.writeByte(S2C_UPDATELOBBYGAME);
		bs.writeByte(MAX(iMaxWorms,iNumPlayers));  // This fixes the player disappearing in lobby
		bs.writeString(gl->szMapFile);
	    bs.writeString(gl->szModName);
	    bs.writeString(gl->szModDir);
		// HACK: The VIP and CTF gametypes need to be disguised as Deathmatch or Team Deathmatches
		if(gl->nGameMode == GMT_VIP || gl->nGameMode == GMT_TEAMCTF)
			bs.writeByte(GMT_TEAMDEATH);
		else if(gl->nGameMode == GMT_CTF)
			bs.writeByte(GMT_DEATHMATCH);
		else
			bs.writeByte(gl->nGameMode);
		bs.writeInt16(gl->nLives);
		bs.writeInt16(gl->nMaxKills);
		bs.writeInt16(gl->nLoadingTime);
	    bs.writeByte(gl->bBonuses);
		
		// since Beta7
		if( cl->getClientVersion() >= OLXBetaVersion(7) )
		{
			bs.writeFloat(gl->fGameSpeed);
			bs.writeBool(gl->bForceRandomWeapons);
			bs.writeBool(gl->bSameWeaponsAsHostWorm);
		}

		SendPacket(&bs, cl);
	}

}


///////////////////
// Send updates for all the worm lobby states
void GameServer::SendWormLobbyUpdate(void)
{
    CBytestream bytestr;
    short c,i;

    CServerConnection *cl = cClients;
	for(c=0;c<MAX_CLIENTS;c++,cl++) {
        if( cl->getStatus() == NET_DISCONNECTED || cl->getStatus() == NET_ZOMBIE )
            continue;

        // Set the client worms lobby ready state
        bool ready = false;
	    for(i=0; i < cl->getNumWorms(); i++) {
		    lobbyworm_t *l = cl->getWorm(i)->getLobby();
		    if(l)
			    ready = l->bReady;
	    }

	    // Let all the worms know about the new lobby state
		if (cl->getNumWorms() <= 2)  {
			bytestr.writeByte(S2C_UPDATELOBBY);
			bytestr.writeByte(cl->getNumWorms());
			bytestr.writeByte(ready);
			for(i=0; i<cl->getNumWorms(); i++) {
				bytestr.writeByte(cl->getWorm(i)->getID());
				bytestr.writeByte(cl->getWorm(i)->getLobby()->iTeam);
			}
		} else {
			int written = 0;
			while (written < cl->getNumWorms())  {
				bytestr.writeByte(S2C_UPDATELOBBY);
				bytestr.writeByte(1);
				bytestr.writeByte(ready);
				bytestr.writeByte(cl->getWorm(written)->getID());
				bytestr.writeByte(cl->getWorm(written)->getLobby()->iTeam);
				written++;
			}
		}
    }

	SendGlobalPacket(&bytestr);
}


///////////////////
// Tell all the clients that we're disconnecting
void GameServer::SendDisconnect(void)
{
	CServerConnection *cl = cClients;
	if (!cl)
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
// Update the worm names, skins, colours etc
void GameServer::UpdateWorms(void)
{
	CBytestream bytestr;
	CWorm* w = cWorms;
	for(int i = 0; i < MAX_WORMS; i++, w++) {
		if(!w->isUsed())
			continue;
		bytestr.writeByte(S2C_WORMINFO);
		bytestr.writeInt(w->getID(), 1);
		w->writeInfo(&bytestr);
	}
	SendGlobalPacket(&bytestr);
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

	SendGlobalPacket(&bs);
}
#endif

void GameServer::SendFiles()
{

	if(iState != SVS_LOBBY)
		return;

	// To keep sending packets if no acknowledge received from client -
	// process will pause for a long time otherwise, 'cause we're in GUI part
	bool startTimer = false;
	int minPing = 200;
	float pingCoeff = 1.5f;	// Send another packet in minPing/pingCoeff milliseconds

	CServerConnection *cl = cClients;

	for(int c = 0; c < MAX_CLIENTS; c++, cl++)
	{
		if(cl->getStatus() == NET_DISCONNECTED || cl->getStatus() == NET_ZOMBIE)
			continue;
		// That's a bit floody algorithm, it can be optimized I think
		if( cl->getUdpFileDownloader()->isSending() &&
			( cl->getChannel()->getBufferEmpty() ||
				! cl->getChannel()->getBufferFull() &&
				cl->getChannel()->getPing() != 0 &&
				tLX->fCurTime - cl->getLastFileRequestPacketReceived() <= cl->getChannel()->getPing()/1000.0f / pingCoeff ) )
		{
			startTimer = true;
			cl->setLastFileRequestPacketReceived( tLX->fCurTime );
			CBytestream bs;
			bs.writeByte(S2C_SENDFILE);
			cl->getUdpFileDownloader()->send(&bs);
			SendPacket( &bs, cl );
			if( cl->getChannel()->getPing() < minPing && cl->getChannel()->getPing() != 0 )
				minPing = cl->getChannel()->getPing();
		};
	};

	if( startTimer )
		Timer( null, NULL, (Uint32)(minPing / pingCoeff), true ).startHeadless();
};

void GameServer::SendEmptyWeaponsOnRespawn( CWorm * Worm )
{
	CBytestream bs;
	CServerConnection * cl = Worm->getClient();
	int i, j, curWeapon = Worm->getCurrentWeapon();
	for( i = 0; i < 5; i++ )
	{
		Worm->getWeapon(i)->Charge=0;
		Worm->getWeapon(i)->Reloading=1;
	};
	for( i=0; i<5; i++ )
	{
		if( i != curWeapon )
		{
			Worm->setCurrentWeapon(i);
			bs.writeByte( S2C_UPDATESTATS );
			bs.writeByte( cl->getNumWorms() );
			for( j = 0; j < cl->getNumWorms(); j++ )
				cl->getWorm(j)->writeStatUpdate(&bs);
		};
	};
	Worm->setCurrentWeapon(curWeapon);
	bs.writeByte( S2C_UPDATESTATS );
	bs.writeByte( cl->getNumWorms() );
	for( j = 0; j < cl->getNumWorms(); j++ )
		cl->getWorm(j)->writeStatUpdate(&bs);
	SendPacket(&bs, cl);
};
