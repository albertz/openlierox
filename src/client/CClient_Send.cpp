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
void CClient::SendText(const std::string& sText)
{
	CBytestream *bs = cNetChan.getMessageBS();

	// TODO: this way is not good; it assumes, that the message is always of the format 'NICK: MSG'
	// Do not allow client to send / commands is the host in not on beta 3
	if (cLocalWorms[0]->getName().size() + 2 < sText.size())  {
		std::string command_buf = sText;

		command_buf = Utf8String(sText.substr(cLocalWorms[0]->getName().size() + 2)); // get the message after "NICK: "
		if(command_buf.size() == 0)
			return;
		
		if(command_buf[0] == '/' && !bHostOLXb3) {
			cChatbox.AddText("HINT: server cannot execute commands, only OLX beta3 can", tLX->clNotice, tLX->fCurTime);
			return;
		}
	}

	// If the text is too long, split it in smaller pieces and then send (backward comaptibility)
	const std::vector<std::string>& split = clever_split(sText, 63);

	for (std::vector<std::string>::const_iterator it=split.begin(); it != split.end(); it++)  {
		bs->writeByte(C2S_CHATTEXT);
		bs->writeString(*it);
	}
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
