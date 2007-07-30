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
#include "CClient.h"
#include "Protocol.h"
#include "CWorm.h"

using namespace std;

///////////////////
// Send all the clients a packet
void GameServer::SendGlobalPacket(CBytestream *bs)
{
	// Assume reliable
	CClient *cl = cClients;

	for(short c = 0; c < MAX_CLIENTS; c++, cl++) {
		if(cl->getStatus() == NET_DISCONNECTED || cl->getStatus() == NET_ZOMBIE)
			continue;

		cl->getChannel()->getMessageBS()->Append(bs);
	}
}


///////////////////
// Send all the clients a string of text
void GameServer::SendGlobalText(const std::string& text, int type) {
	static CBytestream bs;
	bs.Clear();

	bs.writeByte(S2C_TEXT);
	bs.writeInt(type,1);
	bs.writeString(text);

	CClient *cl = cClients;
	for(short c = 0; c < MAX_CLIENTS; c++, cl++) {
		if(cl->getStatus() == NET_DISCONNECTED || cl->getStatus() == NET_ZOMBIE)
			continue;

		cl->getChannel()->getMessageBS()->Append(&bs);
	}
}



///////////////////
// Update all the client about the playing worms
// Returns true if we sent an update
bool GameServer::SendUpdate(CClient *cl)
{
	short i;

	// Don't update so often
	if ((tLX->fCurTime - fLastUpdateSent) <= tLXOptions->fUpdatePeriod)
		if (tGameInfo.iGameType != GME_LOCAL)  {
			fLastUpdateSent = tLX->fCurTime;
			return false;
		}

	// Delays for different net speeds
	static const float	shootDelay[] = {0.025f, 0.010f, 0.005f, -1.0f};


	// Check if we have gone over the bandwidth rating for the client
	// If we have, just don't send a packet this frame
	if( !checkBandwidth(cl) ) {
		// We have gone over the bandwidth for the client, don't send a message this frame
		return false;
	}


    CBytestream *bs = cl->getUnreliable();
    bs->writeByte(S2C_UPDATEWORMS);
    
    vector<CWorm*> wormList;
    i = 0;
	for(CWorm *w = cWorms; i < MAX_WORMS; i++, w++) {
		if(!w->isUsed())
			continue;
		
		// Check if this client owns the worms
		if(cl->OwnsWorm(w))
			continue;
	
		wormList.push_back(w);
	}
	
    bs->writeByte(wormList.size());
    
    // Send all the _other_ worms details
    for(vector<CWorm*>::iterator w_it = wormList.begin(); w_it != wormList.end(); w_it++) {
        // Send out the update			
        bs->writeByte((*w_it)->getID());
        (*w_it)->writePacket(bs);
    }
    
    
    // Write out a stat packet
    bs->writeByte( S2C_UPDATESTATS );
    bs->writeByte( cl->getNumWorms() );
    for(i = 0; i < cl->getNumWorms(); i++)
        cl->getWorm(i)->writeStatUpdate(bs);
    
    // Send the shootlist (reliable)
    CShootList *sh = cl->getShootList();
    float delay = shootDelay[cl->getNetSpeed()];
    
    if(tLX->fCurTime - sh->getStartTime() > delay && sh->getNumShots() > 0) {
        
        // Send the shootlist
        if( sh->writePacket( cl->getChannel()->getMessageBS() ) )
            sh->Clear();
    }
	//}

	// Add the length of the client's unreliable packet to the frame's message size
	int *msgSize = cl->getMsgSize();
	msgSize[iServerFrame % RATE_NUMMSGS] = cl->getUnreliable()->GetLength();


	// All good
	return true;
}


///////////////////
// Check if we have gone over the clients bandwidth rating
// Returns true if we are under the bandwidth
bool GameServer::checkBandwidth(CClient *cl)
{
	// Don't bother checking if the client is on the same comp as the server
	if( tGameInfo.iGameType != GME_LOCAL )
		return true;
	if(cl->getNetSpeed() == 3)
		return true;

	
	// Modem, ISDN, LAN, local
	// (Bytes per second)
	int	Rates[4] = {2500, 7500, 10000, 50000};


	int total = 0;
	int i;

	// Add up the message sizes
	int *msgSizes = cl->getMsgSize();
	for(i=0; i<RATE_NUMMSGS; i++)
		total += msgSizes[i];

	// Are we over the clients bandwidth rate?
	if(total > Rates[cl->getNetSpeed()]) {

		// Clear this frame size
		msgSizes[iServerFrame % RATE_NUMMSGS] = 0;

		// Don't send the packet
		return false;
	}

	// All ok
	return true;
}


///////////////////
// Send an update of the game details in the lobby
void GameServer::UpdateGameLobby(void)
{
	static CBytestream bs;
	bs.Clear();

	game_lobby_t *gl = &tGameLobby;

	// Check if the details have been set yet
	if(!gl->nSet)
		return;

	bs.writeByte(S2C_UPDATELOBBYGAME);
	bs.writeByte(MAX(iMaxWorms,iNumPlayers));  // This fixes the player disappearing in lobby
	bs.writeString(gl->szMapName);
    bs.writeString(gl->szModName);
    bs.writeString(gl->szModDir);
	bs.writeByte(gl->nGameMode);
	bs.writeInt16(gl->nLives);
	bs.writeInt16(gl->nMaxKills);
	bs.writeInt16(gl->nLoadingTime);
    bs.writeByte(gl->nBonuses);

	SendGlobalPacket(&bs);
}


///////////////////
// Send updates for all the worm lobby states
void GameServer::SendWormLobbyUpdate(void)
{
    static CBytestream bytestr;
	bytestr.Clear();
    short c,i;

    CClient *cl = cClients;
	for(c=0;c<MAX_CLIENTS;c++,cl++) {
        if( cl->getStatus() == NET_DISCONNECTED || cl->getStatus() == NET_ZOMBIE )
            continue;
			
        // Set the client worms lobby ready state
        int ready = false;
	    for(i=0; i<cl->getNumWorms(); i++) {
		    lobbyworm_t *l = cl->getWorm(i)->getLobby();
		    if(l)
			    ready = l->iReady;
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
	static CBytestream bs;
	bs.Clear();

	CClient *cl = cClients;
	if (!cl)
		return;
	
	bs.writeByte(S2C_LEAVING);

	for(short c=0;c<MAX_CLIENTS;c++,cl++) {
		if(cl->getStatus() == NET_DISCONNECTED)
			continue;

		// Send it out-of-bounds 3 times to make sure all the clients received it
		for(int i=0;i<3;i++)
			cl->getChannel()->Transmit(&bs);
	}	
}


///////////////////
// Update the worm names, skins, colours etc
void GameServer::UpdateWorms(void)
{
	static CBytestream bytestr;
	bytestr.Clear();
	CWorm* w;
	w = cWorms;
	for(int i = 0; i < MAX_WORMS; i++, w++) {
		if(!w->isUsed())
			continue;
		bytestr.writeByte(S2C_WORMINFO);
		bytestr.writeInt(w->getID(), 1);
		w->writeInfo(&bytestr);
	}
	SendGlobalPacket(&bytestr);
}
