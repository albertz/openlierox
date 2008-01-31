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
#include "StringUtils.h"
#include "CClient.h"
#include "Protocol.h"
#include "CWorm.h"
#ifdef DEBUG
#include "MathLib.h"
#endif
#include "ChatCommand.h"
#include "EndianSwap.h"


///////////////////
// Send the worm details
void CClient::SendWormDetails(void)
{
	// Don't flood packets so often
	if ((tLX->fCurTime - fLastUpdateSent) <= tLXOptions->fUpdatePeriod)
		if (tGameInfo.iGameType != GME_LOCAL)  {
			fLastUpdateSent = tLX->fCurTime;
			return;
		}

	CBytestream bs;
	CWorm *w;
	uint i;

	// If all me worms are not alive, don't send
	bool Alive = false;
	for(i=0;i<iNumWorms;i++) {
		if(cLocalWorms[i]->getAlive()) {
			Alive = true;
			break;		
		}
	}

	if(!Alive)
		return;


	// Check if we need to write the state update
	bool update = false;
	w = cLocalWorms[0];
	for(i = 0; i < iNumWorms; i++, w++)
		if (w->checkPacketNeeded())  {
			update = true;
			break;
		}
	// No update, just quit
	if (!update)
		return;

	// Write the update
	bs.writeByte(C2S_UPDATE);
	
	w = cLocalWorms[0];
	for(i = 0; i < iNumWorms; i++, w++)
		w->writePacket(&bs);

	bsUnreliable.Append(&bs);

	if( ! bDemoReplay && cDemoRecordFile )
	{
		// Save packet to demofile (convert C2S_UPDATE to S2C_UPDATEWORMS)
		bs.Clear();
		bs.writeByte(S2C_UPDATEWORMS);
		bs.writeByte(iNumWorms);
		w = cLocalWorms[0];
		for(i = 0; i < iNumWorms; i++, w++)
		{
			bs.writeByte(w->getID());
			w->writePacket(&bs);
		};
		float curtime = tLX->fCurTime;
		Uint16 size = bs.GetRestLen(), size1=size;
		EndianSwap(curtime);
		EndianSwap(size1);
		fwrite( &curtime, sizeof(curtime), 1, cDemoRecordFile );
		fwrite( &size1, sizeof(size1), 1, cDemoRecordFile );
		fwrite( bs.readData(size).c_str(), size, 1, cDemoRecordFile );
	};
}


///////////////////
// Send a death
void CClient::SendDeath(int victim, int killer)
{
	CBytestream *bs = cNetChan.getMessageBS();

	bs->writeByte(C2S_DEATH);
	bs->writeInt(victim,1);
	bs->writeInt(killer,1);
}


///////////////////
// Send a string of text
void CClient::SendText(const std::string& sText, std::string sWormName)
{
	bool chat_command = sText[0] == '/' && sText[1] != '/';
	std::string message;
	std::string command;  // this should be repeated before every chunk
	bool cannot_split = false;

	if (chat_command)  {
		// Don't allow sending commands to servers that don't support it
		if (iHostOLXVer < 3)  {
			cChatbox.AddText("HINT: server cannot execute commands, only OLX beta3+ can", tLX->clNotice, tLX->fCurTime);
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
			message += *it;
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
		cChatbox.AddText("Could not send the message.", tLX->clNotice, tLX->fCurTime);
		return;
	}

	// If the text is too long, split it in smaller pieces and then send (backward comaptibility)
	// HINT: in command messages the name has to be repeated for all chunks so we have to count with that here
	int name_w = tLX->cFont.GetWidth(sWormName + ": ");
	int repeat_length = command.size() ? (command.size() + sWormName.size()) : 0;  // length of repeated string
	const std::vector<std::string>& split = splitstring(message, 63 - repeat_length,
		iNetStatus == NET_CONNECTED ? 600 - (command.size() ? name_w : 0) : 
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

/////////////////////
// Internal function for text sending, does not do any checks or parsing
void CClient::SendTextInternal(const std::string& sText, const std::string& sWormName)
{
	CBytestream *bs = cNetChan.getMessageBS();
	bs->writeByte(C2S_CHATTEXT);
	if (sWormName.size() == 0)
		bs->writeString(sText);
	else
		bs->writeString(sWormName + ": " + sText);
}

#ifdef DEBUG
//////////////////
// Send a random packet to server (used for debugging)
void CClient::SendRandomPacket()
{
	printf("Sending a random packet to server\n");

	CBytestream *bs = cNetChan.getMessageBS();

	int random_length = GetRandomInt(50);
	for (int i=0; i < random_length; i++)
		bs->writeByte((uchar)GetRandomInt(255));
}
#endif

