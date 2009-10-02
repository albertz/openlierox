/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Client class - Sending routines
// Created 22/7/02
// Jason Boettcher

#include "LieroX.h"

#include "CClientNetEngine.h"
#include "StringUtils.h"
#include "CClient.h"
#include "Protocol.h"
#include "CWorm.h"
#include "MathLib.h"
#include "ChatCommand.h"
#include "Command.h"
#include "EndianSwap.h"
#include "CServer.h"
#include "AuxLib.h"
#include "CChannel.h"



// declare them only locally here as nobody really should use them explicitly
std::string OldLxCompatibleString(const std::string &Utf8String);


///////////////////
// Send the worm details
void CClientNetEngine::SendWormDetails(void)
{
	// Don't flood packets so often
	// we are checking in w->checkPacketNeeded() if we need to send an update
	// we are checking with bandwidth if we should add an update
	/*if ((tLX->fCurTime - fLastUpdateSent) <= tLXOptions->fUpdatePeriod)
		if (tGameInfo.iGameType != GME_LOCAL)
			return; */

	CBytestream bs;
	uint i;

	// If all my worms are dead, don't send
	bool Alive = false;
	for(i=0;i<client->iNumWorms;i++) {
		if(client->cLocalWorms[i]->getAlive()) {
			Alive = true;
			break;
		}
	}

	if(!Alive)
		return;


	// Check if we need to write the state update
	bool update = false;
	for(i = 0; i < client->iNumWorms; i++)
		if (client->cLocalWorms[i]->checkPacketNeeded())  {
			update = true;
			break;
		}

	// No update, just quit
	if (!update)
		return;

	// TODO: Have we always used the limitcheck from GameServer here?
	// We should perhaps move it out from GameServer. Looks a bit strange here to use something from GameServer.
	if(	tGameInfo.iGameType == GME_JOIN // we are a client in a netgame
	&& !GameServer::checkUploadBandwidth(client->getChannel()->getOutgoingRate()) )
		return;

	client->fLastUpdateSent = tLX->fCurTime;

	// Write the update
	bs.writeByte(C2S_UPDATE);

	for(i = 0; i < client->iNumWorms; i++)
		client->cLocalWorms[i]->writePacket(&bs, false, NULL);

	client->bsUnreliable.Append(&bs);
}


void CClientNetEngine::SendGameReady()
{
	CBytestream bs;
	bs.writeByte(C2S_IMREADY);
	bs.writeByte(client->iNumWorms);

	// Send my worm's weapon details
	for(unsigned int i=0;i<client->iNumWorms;i++)
		client->cLocalWorms[i]->writeWeapons( &bs );

	client->cNetChan->AddReliablePacketToSend(bs);
}


///////////////////
// Send a death
void CClientNetEngine::SendDeath(int victim, int killer)
{
	CBytestream bs;

	bs.writeByte(C2S_DEATH);
	bs.writeInt(victim,1);
	bs.writeInt(killer,1);

	client->cNetChan->AddReliablePacketToSend(bs);
}


///////////////////
// Send a string of text
void CClientNetEngine::SendText(const std::string& sText, std::string sWormName)
{
	if(HandleDebugCommand(sText)) return;
	
	bool chat_command = sText.size() >= 2 && sText[0] == '/' && sText[1] != '/';

	// We can safely send long messages to servers >= beta8
	if (client->getServerVersion() >= OLXBetaVersion(8))  {

		// HTML support since beta 8
		SendTextInternal(OldLxCompatibleString(sText), sWormName);


	// <=beta7 server
	} else {

		if (chat_command &&
			client->getServerVersion() < OLXBetaVersion(3) && // <beta3 clients don't have chat command support
			sText.find("/me ") > 0) // Ignores "/me" special command
		{
			// Try if we can execute the same command in console (mainly for "/suicide" command to work on all servers)
			if( ! Cmd_ParseLine(sText.substr(1)) ) {
				client->cChatbox.AddText("HINT: server cannot execute commands, only OLX beta3+ can", tLX->clNotice, TXT_NOTICE, tLX->fCurTime);
			}
			return;

		} else if (chat_command &&
				   client->getServerVersion() >= OLXBetaVersion(3)) {
			// we don't have to split chat commands
			// "/me ..." is also save, because server uses SendGlobalText for sending and it splits the text there
			SendTextInternal(OldLxCompatibleString(sText), sWormName);

		} else { // we can savely split the message (and we have to)
			
			// If the text is too long, split it in smaller pieces and then send (backward comaptibility)
			int name_w = tLX->cFont.GetWidth(sWormName + ": ");
			const std::vector<std::string>& split = splitstring(StripHtmlTags(sText), 63,
				client->iNetStatus == NET_CONNECTED ? 600 - name_w : 300 - name_w, tLX->cFont);

			// Check
			if (split.size() == 0)  {  // More than weird...
				SendTextInternal(OldLxCompatibleString(StripHtmlTags(sText)), sWormName);
				return;
			}

			// Send the first chunk
			SendTextInternal(OldLxCompatibleString(split[0]), sWormName);

			// Send the text
			for (std::vector<std::string>::const_iterator it = split.begin() + 1; it != split.end(); it++)
				SendTextInternal(OldLxCompatibleString(*it), "");
		}
	}
}

void CClientNetEngineBeta7::SendChatCommandCompletionRequest(const std::string& startStr) {
	CBytestream bs;
	bs.writeByte(C2S_CHATCMDCOMPLREQ);
	bs.writeString(startStr);

	client->cNetChan->AddReliablePacketToSend(bs);
}

void CClientNetEngineBeta7::SendAFK(int wormid, AFK_TYPE afkType, const std::string & message ) {
	if( client->getServerVersion() < OLXBetaVersion(7) )
		return;
	
	std::string msg = message;
	if( msg == "" )
	{
		if( afkType == AFK_TYPING_CHAT )
			msg = "(typing)";
		else
		if( afkType == AFK_AWAY )
			msg = "(away)";
	};


	for( int i=0; i < client->getNumWorms(); i++ )
		if(	client->getWorm(i)->getID() == wormid )
			client->getWorm(i)->setAFK(afkType, msg);
	
	CBytestream bs;
	bs.writeByte(C2S_AFK);
	bs.writeByte(wormid);
	bs.writeByte(afkType);
	if(msg.size() > 127)
		msg = msg.substr(0, 127);
	bs.writeString(msg);

	client->cNetChan->AddReliablePacketToSend(bs);
}

/////////////////////
// Internal function for text sending, does not do any checks or parsing
void CClientNetEngine::SendTextInternal(const std::string& sText, const std::string& sWormName)
{
	CBytestream bs;
	bs.writeByte(C2S_CHATTEXT);
	if (sWormName.size() == 0)
		bs.writeString(sText);
	else
		bs.writeString(sWormName + ": " + sText);

	client->cNetChan->AddReliablePacketToSend(bs);
}

#ifdef FUZZY_ERROR_TESTING
//////////////////
// Send a random packet to server (used for debugging)
void CClientNetEngine::SendRandomPacket()
{
	// don't send random packets from the local client to our own server
	if( tGameInfo.iGameType != GME_JOIN ) return;

	CBytestream bs;

	int random_length = GetRandomInt(50);
	for (int i=0; i < random_length; i++)
		bs.writeByte((uchar)GetRandomInt(255));

	client->cNetChan->AddReliablePacketToSend(bs);

	bs.Clear();

	// Test connectionless packets
	if (GetRandomInt(100) >= 75)  {
		bs.writeInt(-1, 4);
		static const std::string commands[] = { "lx::getchallenge", "lx::connect", "lx::ping", "lx::query",
			"lx::getinfo", "lx::wantsjoin", "lx::traverse", "lx::registered"};
		bs.writeString(commands[GetRandomInt(500) % (sizeof(commands)/sizeof(std::string))]);
		for (int i=0; i < random_length; i++)
			bs.writeByte((uchar)GetRandomInt(255));
		SetRemoteNetAddr(client->tSocket, client->cNetChan->getAddress());
		bs.Send(client->tSocket);
	}
}
#endif

void CClientNetEngine::SendGrabBonus(int id, int wormid)
{
	CBytestream bs;
	bs.writeByte(C2S_GRABBONUS);
	bs.writeByte(id);
	bs.writeByte(wormid);
	bs.writeByte(client->cRemoteWorms[wormid].getCurrentWeapon());

	client->cNetChan->AddReliablePacketToSend(bs);
}

void CClientNetEngine::SendUpdateLobby(bool ready)
{
	CBytestream bs;
	bs.writeByte(C2S_UPDATELOBBY);
	bs.writeByte(1);
	client->getChannel()->AddReliablePacketToSend(bs);
}

void CClientNetEngine::SendDisconnect()
{
	CBytestream bs;

	bs.writeByte(C2S_DISCONNECT);

	// Send the pack a few times to make sure the server gets the packet
	if( client->cNetChan != NULL)
		for(int i=0;i<3;i++)
			client->cNetChan->Transmit(&bs);
}

void CClientNetEngine::SendFileData()
{
	CBytestream bs;
	bs.writeByte(C2S_SENDFILE);
	client->getUdpFileDownloader()->send(&bs);
	client->getChannel()->AddReliablePacketToSend(bs);
}

