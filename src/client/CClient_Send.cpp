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

#include "CClientNetEngine.h"
#include "LieroX.h"
#include "StringUtils.h"
#include "CClient.h"
#include "Protocol.h"
#include "CWorm.h"
#include "MathLib.h"
#include "ChatCommand.h"
#include "Command.h"
#include "EndianSwap.h"
#include "CServer.h"



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
/*	bool update = false;
	w = cLocalWorms[0];
	for(i = 0; i < iNumWorms; i++, w++)
		if (w->checkPacketNeeded())  {
			update = true;
			break;
		}

	// No update, just quit
	if (!update)
		return;
*/
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
	bool chat_command = sText.size() >= 2 && sText[0] == '/' && sText[1] != '/';
	std::string message;
	std::string command;  // this should be repeated before every chunk
	bool cannot_split = false;

	// HINT: since beta 4 the chat can have an unlimited length and is internally split up into chunks
	// This works fine for normal chat but for chat commands like /teamchat or /pm it makes trouble
	// because if the message is split into more pieces, all the pieces except for the first one are sent
	// as a normal chat, for example:
	// /teamchat some ultra long text that will be split for sure!
	// would get split to "/teamchat some ultra long", "text that will be split", "for sure!"
	// and the chunks would be sent as a chat. The second and third chunk don't containt the /teamchat command
	// and will therefore be interpreted as a normal chat by the server
	// This function is so complicated because it avoids this kind of trouble and splits the chat command
	// messages correctly.

	if (chat_command && sText.find("/me ") != 0) { // Ignores "/me" special command
		// Don't allow sending commands to servers that don't support it
		if (client->getServerVersion() < OLXBetaVersion(3)) {
			// Try if we can execute the same command in console (mainly for "/suicide" command to work on all servers)
			if( ! Cmd_ParseLine(sText.substr(1)) ) {
				client->cChatbox.AddText("HINT: server cannot execute commands, only OLX beta3+ can", tLX->clNotice, tLX->fCurTime);
			}
			return;
		}

		// Get the command
		const std::vector<std::string>& cmd = ParseCommandMessage(sText, false);
		if (cmd.size() == 0)  {  // Weird
			SendTextInternal(OldLxCompatibleString(sText), sWormName);
			return;
		}

		ChatCommand *chat_cmd = GetCommand(cmd[0]);
		if (chat_cmd == NULL)  {
			SendTextInternal(OldLxCompatibleString(sText), sWormName);
			return;
		}

		// The message cannot be split
		cannot_split = chat_cmd->iParamsToRepeat == (size_t)-1;

		// Join all params that should be repeated before each part to the command
		std::vector<std::string>::const_iterator it = cmd.begin();
		if (cannot_split)  {  // Cannot split the message, nothing will be repeated
			command = "/" + *it + ' ';  // Just add the command, and skip to parameters
			it++;
		} else {
			command = "/";
			for (uint i=0; it != cmd.end() && i <= chat_cmd->iParamsToRepeat; it++, i++)  { // HINT: +1 - command itself
				command += *it;
				command += ' ';
			}
			// HINT: we keep the last space here, because we would have to add it later anyway
		}

		// Check for param count
		if (it == cmd.end())  {
			SendTextInternal(OldLxCompatibleString(sText), sWormName);
			return;
		}

		// Join all params that should not be repeated into a message
		for (; it != cmd.end(); it++)  {
			std::string prm = *it;
			bool contains_quot = replace(prm, "\\", "\\\\");
			bool contains_slash = replace(prm, "\"", "\\\"");
			if (prm.find(' ') != std::string::npos || contains_quot || contains_slash)  { // If the parameter contains spaces, put it in quotes
				message += "\"" + prm + "\"";
			} else
				message += prm;
			message += ' ';
		}
		message.erase(message.size() - 1); // erase the last space

	} else { // Not a command
		// Allow /me for non-command messages
		if (replace(sText, "/me", sWormName, message))
			sWormName = "";  // No worm name if /me is present
	}

	// OLD LieroX compatibility (because of unicode)
	message = OldLxCompatibleString(message);

	// If we cannot split the message, truncate it and send
	if (cannot_split)  {
		if (message.size() > 63 - sWormName.size() - command.size() - 2)
			message.erase(64 - sWormName.size() - command.size() - 2, std::string::npos);  // truncate
		SendTextInternal(command + message, sWormName);
		return;
	}

	// If the part we should repeat before each message is longer than the limit, just quit
	// HINT: should not happen
	if (command.size() + sWormName.size() >= 64)  {
		client->cChatbox.AddText("Could not send the message.", tLX->clNotice, tLX->fCurTime);
		return;
	}

	// If the text is too long, split it in smaller pieces and then send (backward comaptibility)
	// HINT: in command messages the name has to be repeated for all chunks so we have to count with that here
	int name_w = tLX->cFont.GetWidth(sWormName + ": ");
	size_t repeat_length = command.size() ? (command.size() + sWormName.size()) : 0;  // length of repeated string
	const std::vector<std::string>& split = splitstring(message, 63 - repeat_length,
		client->iNetStatus == NET_CONNECTED ? 600 - (command.size() ? name_w : 0) :
		300 - (command.size() ? name_w : 0), tLX->cFont);

	// Check
	if (split.size() == 0)  {  // More than weird...
		SendTextInternal(command + message, sWormName);
		return;
	}

	// Send the first chunk
	SendTextInternal(command + split[0], sWormName);

	// Send the text
	for (std::vector<std::string>::const_iterator it=split.begin() + 1; it != split.end(); it++)
		SendTextInternal(command + (*it), command.size() ? sWormName : ""); // in command messages
																			// the name has to be repeated for all chunks
}

void CClientNetEngine::SendChatCommandCompletionRequest(const std::string& startStr) {
	CBytestream bs;
	bs.writeByte(C2S_CHATCMDCOMPLREQ);
	bs.writeString(startStr);

	client->cNetChan->AddReliablePacketToSend(bs);
}

void CClientNetEngine::SendAFK(int wormid, AFK_TYPE afkType) {
	if( client->getServerVersion() < OLXBetaVersion(7) )
		return;
	
	for( int i=0; i < client->getNumWorms(); i++ )
		if(	client->getWorm(i)->getID() == wormid )
			client->getWorm(i)->setAFK(afkType);
	
	CBytestream bs;
	bs.writeByte(C2S_AFK);
	bs.writeByte(wormid);
	bs.writeByte(afkType);

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
		SetRemoteNetAddr(tSocket, client->cNetChan->getAddress());
		bs.Send(tSocket);
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
};

void CClientNetEngine::SendUpdateLobby(bool ready)
{
	CBytestream bs;
	bs.writeByte(C2S_UPDATELOBBY);
	bs.writeByte(1);
	client->getChannel()->AddReliablePacketToSend(bs);
};

void CClientNetEngine::SendDisconnect()
{
	CBytestream bs;

	bs.writeByte(C2S_DISCONNECT);

	// Send the pack a few times to make sure the server gets the packet
	if( client->cNetChan != NULL)
		for(int i=0;i<3;i++)
			client->cNetChan->Transmit(&bs);
};

void CClientNetEngine::SendFileData()
{
	CBytestream bs;
	bs.writeByte(C2S_SENDFILE);
	client->getUdpFileDownloader()->send(&bs);
	client->getChannel()->AddReliablePacketToSend(bs);
};

