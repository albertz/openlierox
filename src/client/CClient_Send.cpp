/////////////////////////////////////////
//
//             Liero Xtreme
//
//     Copyright Auxiliary Software 2002
//
//
/////////////////////////////////////////


// Client class - Sending routines
// Created 22/7/02
// Jason Boettcher


#include "defs.h"
#include "LieroX.h"


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
	int i;

	// If all me worms are not alive, don't send
	int Alive = false;
	for(i=0;i<iNumWorms;i++) {
		if(cLocalWorms[i]->getAlive())
			Alive = true;
	}

	if(!Alive)
		return;



	bs.writeByte(C2S_UPDATE);
	
	for(i=0;i<iNumWorms;i++) {
		w = cLocalWorms[i];

		w->writePacket(&bs);
	}

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
void CClient::SendText(char *sText)
{
	CBytestream *bs = cNetChan.getMessageBS();

	bs->writeByte(C2S_CHATTEXT);
	bs->writeString("%s",sText);
}
