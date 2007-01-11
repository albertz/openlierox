/////////////////////////////////////////
//
//             Liero Xtreme
//
//     Copyright Auxiliary Software 2002
//
//
/////////////////////////////////////////


// Server class - Parsing
// Created 1/7/02
// Jason Boettcher


#include "defs.h"
#include "LieroX.h"
#include "Menu.h"


/*
=======================================
		Connected Packets
=======================================
*/


///////////////////
// Parses a general packet
void CServer::ParseClientPacket(CClient *cl, CBytestream *bs)
{
	CChannel *chan = cl->getChannel();
	ping_t *ping;
	bool bResetPing = false;

	// Calculate the ping time
	ping = cl->getPingStruct();
	if (ping->iSequence <= chan->getInAck())  {
		if ((tLX->fCurTime - cl->getLastPingTime()) > 1)  {  // Update ping once per second
			int png = (int)((tLX->fCurTime - ping->fSentTime)*1000-20);

			if (cl->getPing() > 99999)
				cl->setPing(0);

			// Make the ping slighter
			if (png - cl->getPing() > 5 && cl->getPing() && png)
				png = (png + cl->getPing() + cl->getPing())/3;
			if (cl->getPing() - png > 5 && cl->getPing() && png)
				png = (png + png + cl->getPing())/3;

			if (png > 99999)
				png = 0;

			cl->setPing(png);
			cl->setLastPingTime(tLX->fCurTime);
			
		}
		bResetPing = true;
	}

	// Ensure the incoming sequence matchs the outgoing sequence
	if(chan->getInSeq() >= chan->getOutSeq())
		chan->setOutSeq(chan->getInSeq());
	else
		// Sequences have slipped
		bResetPing = true;
		// TODO: Set the player's send_data property to false


	// Set the sent out time for ping calculations
	ping = cl->getPingStruct();
	if (bResetPing || !ping->bInitialized) {
		ping->fSentTime = chan->getLastSent();
		ping->iSequence = chan->getOutSeq();
		ping->bInitialized  = true;
	}

	cl->setLastReceived(tLX->fCurTime);

	//player->SetLocalTime(ServerTime);
	
	// Parse the packet messages
	ParsePacket(cl,bs);
}


///////////////////
// Parse a packet
void CServer::ParsePacket(CClient *cl, CBytestream *bs)
{
	uchar cmd;
	
	if(bs->GetLength()==0)
		return;

	while(1) {
		cmd = bs->readInt(1);

		if(bs->GetPos() > bs->GetLength())
			break;

		switch(cmd) {

			// Client is ready
			case C2S_IMREADY:
				ParseImReady(cl,bs);
				break;

			// Update packet
			case C2S_UPDATE:
				ParseUpdate(cl,bs);
				break;

			// Death
			case C2S_DEATH:
				ParseDeathPacket(cl,bs);
				break;

			// Chat text
			case C2S_CHATTEXT:
				ParseChatText(cl,bs);
				break;

			// Update lobby
			case C2S_UPDATELOBBY:
				ParseUpdateLobby(cl,bs);
				break;

			// Disconnect
			case C2S_DISCONNECT:
				ParseDisconnect(cl);
				break;

			// Bonus grabbed
			case C2S_GRABBONUS:
				ParseGrabBonus(cl, bs);
				break;

			default:
				printf("sv: Bad command in packet\n");
		}
	}
}


///////////////////
// Parse a 'im ready' packet
void CServer::ParseImReady(CClient *cl, CBytestream *bs)
{
	if (iState != SVS_GAME)
		return;

	int i;
	// Note: This isn't a lobby ready

	// Read the worms weapons
	int num = bs->readByte();
	for(i=0; i<num; i++) {
		int id = bs->readByte();
		if( id >= 0 && id < MAX_WORMS)
			cWorms[id].readWeapons(bs);
	}


	// Set this client to 'ready'
	cl->setGameReady(true);

	// Let everyone know this client is ready to play
	CBytestream bytes;
	bytes.writeByte(S2C_CLREADY);
	bytes.writeByte(cl->getNumWorms());
	for(i=0;i<cl->getNumWorms();i++) {
		// Send the weapon info here (also contains id)
		cWorms[cl->getWorm(i)->getID()].writeWeapons(&bytes);
	}

	SendGlobalPacket(&bytes);


    // Check if all the clients are ready
    CheckReadyClient();
}


///////////////////
// Parse an update packet
void CServer::ParseUpdate(CClient *cl, CBytestream *bs)
{
	for(int i=0; i<cl->getNumWorms(); i++) {
		CWorm *w = cl->getWorm(i);

		w->readPacket(bs, cWorms);

		// If the worm is shooting, handle it
		if(w->getWormState()->iShoot && w->getAlive() && iState == SVS_PLAYING)
			WormShoot(w);
	}
}


///////////////////
// Parse a death packet
void CServer::ParseDeathPacket(CClient *cl, CBytestream *bs)
{
	// No kills in lobby
	if (iState != SVS_PLAYING)
		return;

	static char buf[128];
	CBytestream byte;
	int victim = bs->readInt(1);
	int killer = bs->readInt(1);
	
	// Team names
	char *TeamNames[] = {"blue", "red", "green", "yellow"};
	int TeamCount[4];

    // If the game is already over, ignore this
    if(iGameOver)
        return;


	// Safety check
	if(victim < 0 || victim >= MAX_WORMS)
		return;
	if(killer < 0 || killer >= MAX_WORMS)
		return;

	CWorm *vict = &cWorms[victim];
	CWorm *kill = &cWorms[killer];
	log_worm_t *log_vict = GetLogWorm(vict->getID());
	log_worm_t *log_kill = GetLogWorm(kill->getID());

	// TODO: Cheat prevention check: Make sure the victim is one of the client's worms

	// Cheat prevention, game behaves weird if this happens
	if(vict->getLives() < 0)
		return;

	
	// Kill
	if (strcmp(NetworkTexts->sKilled,"<none>"))  { // Take care of the <none> tag
		if(killer != victim)  {
			replacemax(NetworkTexts->sKilled,"<killer>",kill->getName(),buf,1);
			replacemax(buf,"<victim>",vict->getName(),buf,1);
		}
		else
			replacemax(NetworkTexts->sCommitedSuicide,"<player>",vict->getName(),buf,1);

		SendGlobalText(buf,TXT_NORMAL);
	}

	// First blood
	if (bFirstBlood && killer != victim && strcmp(NetworkTexts->sFirstBlood,"<none>"))  {
		replacemax(NetworkTexts->sFirstBlood,"<player>",kill->getName(),buf,1);
		bFirstBlood = false;
		SendGlobalText(buf,TXT_NORMAL);
	}

	// Teamkill
	if (iGameType == GMT_TEAMDEATH && vict->getTeam() == kill->getTeam() && killer != victim)  {
		//Take care of the <none> tag
		if (strcmp(NetworkTexts->sTeamkill,"<none>"))  {
			replacemax(NetworkTexts->sTeamkill,"<player>",kill->getName(),buf,1);
			SendGlobalText(buf,TXT_NORMAL);
		}
	}

	vict->setKillsInRow(0);

	if(killer != victim)  {
		kill->addKillInRow();
		kill->AddKill();
		if (log_kill)
			log_kill->iKills++;
	} else {
		// Log the suicide
		if (log_vict)
			log_vict->iSuicides++;
	}

	// Log
	if (log_vict)
		log_vict->iLives--;

	// Killing spree message
	switch (kill->getKillsInRow())  {
	case 3:
		replacemax(NetworkTexts->sSpree1,"<player>",kill->getName(),buf,1);
		SendGlobalText(buf,TXT_NORMAL);
		break;
	case 5: 
		replacemax(NetworkTexts->sSpree2,"<player>",kill->getName(),buf,1);
		SendGlobalText(buf,TXT_NORMAL);
		break;
	case 7:
		replacemax(NetworkTexts->sSpree3,"<player>",kill->getName(),buf,1);
		SendGlobalText(buf,TXT_NORMAL);
		break;
	case 10:
		replacemax(NetworkTexts->sSpree4,"<player>",kill->getName(),buf,1);
		SendGlobalText(buf,TXT_NORMAL);
		break;
	case 15:
		replacemax(NetworkTexts->sSpree5,"<player>",kill->getName(),buf,1);
		SendGlobalText(buf,TXT_NORMAL);
		break;
	}


	if(vict->Kill()) {


		// This worm is out of the game
		if(strcmp(NetworkTexts->sPlayerOut,"<none>")) {
			replacemax(NetworkTexts->sPlayerOut,"<player>",vict->getName(),buf,1);
			SendGlobalText(buf,TXT_NORMAL);
		}

		// Check if only one person is left
		int wormsleft = 0;
		int wormid = 0;
		CWorm *w = cWorms;
		int i;
		for(i=0;i<MAX_WORMS;i++,w++) {
			if(w->isUsed() && w->getLives() != WRM_OUT) {
				wormsleft++;
				wormid = i;
			}
		}

		if(wormsleft == 1 /*temphack*/ || wormsleft == 0) {
			// Declare the winner
			switch (iGameType)  {
			case GMT_DEATHMATCH:
				if (strcmp(NetworkTexts->sPlayerHasWon,"<none>"))  {
					CWorm *winner = cWorms + wormid;
					replacemax(NetworkTexts->sPlayerHasWon,"<player>",winner->getName(),buf,1);
					SendGlobalText(buf,TXT_NORMAL);
				}
			break;  // DEATHMATCH
			case GMT_TAG:
				// Get the worm with greatest tag time
				float fMaxTagTime = 0;
				int wormid = 0;
				CWorm *w = cWorms;
				for (int i=0;i<MAX_WORMS;i++)
					if (w->isUsed())
						if (w->getTagTime() > fMaxTagTime)  {
							fMaxTagTime = w->getTagTime();
							wormid = i;
						}

				// Worm with greatest tag time
				w = cWorms+wormid;

				// Send the text
				if (strcmp(NetworkTexts->sPlayerHasWon,"<none>"))  {
					replacemax(NetworkTexts->sPlayerHasWon,"<player>",w->getName(),buf,1);
					SendGlobalText(buf,TXT_NORMAL);
				}
			break;  // TAG

			// TEAM DEATHMATCH is handled below
			}

			// Let everyone know that the game is over
			byte.writeByte(S2C_GAMEOVER);
			byte.writeInt(wormid,1);

			// Game over
			if (iGameType != GMT_TEAMDEATH)  {  // Team deathmatch is handled below
				iGameOver = true;
				fGameOverTime = tLX->fCurTime;
			}

			// It's sent in the packet below
		}



		// If the game is still going and this is a teamgame, check if the team this worm was in still
		// exists
		if(!iGameOver && iGameType == GMT_TEAMDEATH) {
			int team = vict->getTeam();
			int teamcount = 0;

			for(i=0;i<4;i++)
				TeamCount[i]=0;

			// Check if anyone else is left on the team
			w = cWorms;
			for(i=0;i<MAX_WORMS;i++,w++) {
				if(w->isUsed()) {
					if(w->getLives() != WRM_OUT && w->getTeam() == team)
						teamcount++;

					if(w->getLives() != WRM_OUT)
						TeamCount[w->getTeam()]++;
				}
			}

			// No-one left in the team
			if(teamcount==0) {
				if (strcmp(NetworkTexts->sTeamOut,"<none>"))  {
					replacemax(NetworkTexts->sTeamOut,"<team>",TeamNames[team],buf,1);
					SendGlobalText(buf,TXT_NORMAL);
				}
			}

			// If there is only 1 team left, declare the last team the winner
			int teamsleft = 0;
			team=0;
			for(i=0;i<4;i++) {
				if(TeamCount[i]) {
					teamsleft++;
					team=i;
				}
			}

			if(teamsleft == 1) {
				if (strcmp(NetworkTexts->sTeamHasWon,"<none>"))  {
					replacemax(NetworkTexts->sTeamHasWon,"<team>",TeamNames[team],buf,1);
					SendGlobalText(buf,TXT_NORMAL);
				}

				byte.Clear();
				byte.writeByte(S2C_GAMEOVER);
				byte.writeInt(team,1);
				iGameOver = true;
				fGameOverTime = tLX->fCurTime;

				// This packet is sent below
			}
		}
	}


	// Check if the max kills has been reached
	if(iMaxKills != -1 && killer != victim && kill->getKills() == iMaxKills && !iGameOver) {
		
		// Game over (max kills reached)
		byte.writeByte(S2C_GAMEOVER);
		byte.writeInt(kill->getID(),1);
		iGameOver = true;        	
		fGameOverTime = tLX->fCurTime;
	}


	// If the worm killed is IT, then make the killer now IT
	if (iGameType == GMT_TAG && !iGameOver)  {
		if(killer != victim) {
			if(vict->getTagIT()) {
				vict->setTagIT(false);

				// If the killer is dead, tag a worm randomly
				if(!kill->getAlive() || kill->getLives() == WRM_OUT)
					TagRandomWorm();
				else
					TagWorm(kill->getID());
			}
		} else {
			// If it's a suicide and the worm was it, tag a random person
			if(vict->getTagIT()) {
				vict->setTagIT(false);
				TagRandomWorm();
			}
		}
	}

	// Update everyone on the victims & killers score
	vict->writeScore(&byte);
	if(killer != victim)
		kill->writeScore(&byte);

	// Let everyone know that the worm is now dead
    byte.writeByte(S2C_WORMDOWN);
    byte.writeByte(victim);
    

	SendGlobalPacket(&byte);
}


///////////////////
// Parse a chat text packet
void CServer::ParseChatText(CClient *cl, CBytestream *bs)
{
	// Don't send text from muted players
	if (cl)
		if (cl->getMuted())
			return;

	static char buf[256];
	SendGlobalText( bs->readString(buf, sizeof(buf)), TXT_CHAT);
}


///////////////////
// Parse a 'update lobby' packet
void CServer::ParseUpdateLobby(CClient *cl, CBytestream *bs)
{
	// Must be in lobby
	if (iState != SVS_LOBBY)
		return;

	int ready = bs->readByte();
	int i;

	// Set the client worms lobby ready state
	for(i=0; i<cl->getNumWorms(); i++) {
		lobbyworm_t *l = cl->getWorm(i)->getLobby();
		if(l)
			l->iReady = ready;
	}

	// Let all the worms know about the new lobby state
	CBytestream bytestr;
	bytestr.writeByte(S2C_UPDATELOBBY);
	bytestr.writeByte(cl->getNumWorms());
	bytestr.writeByte(ready);
    for(i=0; i<cl->getNumWorms(); i++) {
		bytestr.writeByte(cl->getWorm(i)->getID());
        bytestr.writeByte(cl->getWorm(i)->getLobby()->iTeam);
    }

	SendGlobalPacket(&bytestr);
}


///////////////////
// Parse a disconnect packet
void CServer::ParseDisconnect(CClient *cl)
{
	// Check if the client hasn't already left
	if(cl->getStatus() == NET_DISCONNECTED)
		return;

	DropClient(cl, CLL_QUIT);
}


///////////////////
// Parse a weapon list packet
void CServer::ParseWeaponList(CClient *cl, CBytestream *bs)
{
	int id = bs->readByte();

	if( id >= 0 && id < MAX_WORMS)
		cWorms[id].readWeapons(bs);
}


///////////////////
// Parse a 'grab bonus' packet
void CServer::ParseGrabBonus(CClient *cl, CBytestream *bs)
{
	if (iState != SVS_PLAYING)
		return;

	int id = bs->readByte();
	int wormid = bs->readByte();
	int curwpn = bs->readByte();


	// Worm ID ok?
	if( wormid >= 0 && wormid < MAX_WORMS ) {
		CWorm *w = &cWorms[wormid];

		// Bonus id ok?
		if( id >= 0 && id < MAX_BONUSES ) {
			CBonus *b = &cBonuses[id];

			if( b->getUsed() ) {

				// If it's a weapon, change the worm's current weapon
				if( b->getType() == BNS_WEAPON ) {

					if( curwpn >= 0 && curwpn < 5 ) {

						wpnslot_t *wpn = w->getWeapon(curwpn);
						wpn->Weapon = cGameScript.GetWeapons()+b->getWeapon();
						wpn->Charge = 1;
						wpn->Reloading = false;
					}
				}

				// Tell all the players that the bonus is now gone
				CBytestream bs;
				bs.writeByte(S2C_DESTROYBONUS);
				bs.writeByte(id);
				SendGlobalPacket(&bs);
			}
		}
	}
}





/*
===========================

  Connectionless packets

===========================
*/


///////////////////
// Parses connectionless packets
void CServer::ParseConnectionlessPacket(CBytestream *bs)
{
	static char cmd[128];
	bool valid = false;

	// This solves the problem with hosting!
//	GetRemoteNetAddr(tSocket,&adrFrom);
//	SetRemoteNetAddr(tSocket,&adrFrom);

	bs->readString(cmd, sizeof(cmd));

	//strcpy(tLX->debug_string, cmd);

	if(!strcmp(cmd,"lx::getchallenge"))
		ParseGetChallenge();
	else if(!strcmp(cmd,"lx::connect"))
		ParseConnect(bs);
	else if(!strcmp(cmd,"lx::ping"))
		ParsePing();
	else if(!strcmp(cmd,"lx::query"))
		ParseQuery(bs);
    else if(!strcmp(cmd,"lx::getinfo"))
        ParseGetInfo();
	else if(!strcmp(cmd,"lx::wantsjoin"))
		ParseWantsJoin(bs);
}


///////////////////
// Handle a "getchallenge" msg
void CServer::ParseGetChallenge(void)
{
	int			i;
	NetworkAddr	adrFrom;
	float		OldestTime = 99999;
	int			ChallengeToSet = -1;
	CBytestream	bs;

	GetRemoteNetAddr(tSocket,&adrFrom);

	// If were in the game, deny challenges
	if(iState != SVS_LOBBY) {
		bs.Clear();
		bs.writeInt(-1,4);
		bs.writeString("%s","lx::badconnect");
		bs.writeString("%s",NetworkTexts->sGameInProgress);
		bs.Send(tSocket);
		return;
	}


	// see if we already have a challenge for this ip
	for(i=0;i<MAX_CHALLENGES;i++) {
		
		if(IsNetAddrValid(&tChallenges[i].Address)) {
			if(AreNetAddrEqual(&adrFrom, &tChallenges[i].Address))
				break;
			if(ChallengeToSet < 0 || tChallenges[i].fTime < OldestTime) {
				OldestTime = tChallenges[i].fTime;
				ChallengeToSet = i;
			}
		} else {
			ChallengeToSet = i;
			break;
		}
	}

	if(ChallengeToSet >= 0) {
		
		// overwrite the oldest
		tChallenges[ChallengeToSet].iNum = (rand() << 16) ^ rand();
		tChallenges[ChallengeToSet].Address = adrFrom;
		tChallenges[ChallengeToSet].fTime = tLX->fCurTime;
		
		i = ChallengeToSet;
	}

	// Send the challenge details back to the client
	SetRemoteNetAddr(tSocket,&adrFrom);


	bs.writeInt(-1,4);
	bs.writeString("%s","lx::challenge");
	bs.writeInt(tChallenges[i].iNum,4);
	bs.Send(tSocket);
}


///////////////////
// Handle a 'connect' message
void CServer::ParseConnect(CBytestream *bs)
{
	CBytestream		bytestr;
	NetworkAddr		adrFrom;
	int				i,p,player=-1;
	int				numplayers;
	CClient			*cl,*newcl;

	// Connection details
	int		ProtocolVersion;
	int		Port = LX_PORT;
	int		ChallId;
	int		iNetSpeed = 0;


	// Ignore if we are playing (the challenge should have denied the client with a msg)
	if(iState != SVS_LOBBY)
		return;

	// User Info to get
	
	GetRemoteNetAddr(tSocket,&adrFrom);

	// Read packet
	ProtocolVersion = bs->readInt(1);
	if(ProtocolVersion != PROTOCOL_VERSION) {
		printf("Wrong protocol version, server protocol version is %d\n", PROTOCOL_VERSION);
		
		// Get the string to send
		static char buf[256];
		if (strcmp(NetworkTexts->sTeamHasWon,"<none>"))  {
			char buf2[4];
			snprintf(buf2,sizeof(buf2),"%d",PROTOCOL_VERSION);
			fix_markend(buf2);
			replacemax(NetworkTexts->sWrongProtocol,"<version>",buf2,buf,1);
		}
		else
			strcpy(buf," ");

		// Wrong protocol version, don't connect client
		bytestr.Clear();
		bytestr.writeInt(-1,4);
		bytestr.writeString("%s","lx::badconnect");
		bytestr.writeString("%s",buf);
		bytestr.Send(tSocket);
		return;
	}

	static char szAddress[21];
	NetAddrToString(&adrFrom,szAddress);
	fix_markend(szAddress);

	// Is this IP banned?
	if (getBanList()->isBanned(szAddress))  {
		printf("Banned client was trying to connect\n");
		bytestr.Clear();
		bytestr.writeInt(-1,4);
		bytestr.writeString("%s","lx::badconnect");
		bytestr.writeString("%s",NetworkTexts->sYouAreBanned);
		bytestr.Send(tSocket);
		return;
	}

	//Port = pack->ReadShort();
	ChallId = bs->readInt(4);
	iNetSpeed = bs->readInt(1);

	// Make sure the net speed is within bounds, because it's used for indexing
	iNetSpeed = MIN(iNetSpeed,3);
	iNetSpeed = MAX(iNetSpeed,0);

	
	// Get user info
	int numworms = bs->readInt(1);
	MIN(numworms,MAX_PLAYERS-1);
	CWorm worms[MAX_PLAYERS];
	for(i=0;i<numworms;i++) {
		worms[i].readInfo(bs);
		// If bots aren't allowed, disconnect the client
		if (worms[i].getType() == PRF_COMPUTER && !tLXOptions->tGameinfo.bAllowRemoteBots && !strstr(szAddress,"127.0.0.1"))  {
			printf("Bot was trying to connect\n");
			bytestr.Clear();
			bytestr.writeInt(-1,4);
			bytestr.writeString("%s","lx::badconnect");
			bytestr.writeString("%s",NetworkTexts->sBotsNotAllowed);
			bytestr.Send(tSocket);
			return;
		}
	}


	// See if the challenge is valid
	for(i=0;i<MAX_CHALLENGES;i++) {
		if(IsNetAddrValid(&tChallenges[i].Address) && AreNetAddrEqual(&adrFrom, &tChallenges[i].Address)) {

			if(ChallId == tChallenges[i].iNum)
				break;		// good

			printf("Bad connection verification of client\n");
			bytestr.Clear();
			bytestr.writeInt(-1,4);
			bytestr.writeString("%s","lx::badconnect");
			bytestr.writeString("%s",NetworkTexts->sBadVerification);
			bytestr.Send(tSocket);
			return;
		}
	}

	// Ran out of challenges
	if(i == MAX_CHALLENGES-1) {
		printf("No connection verification for client found\n");
		bytestr.Clear();
		bytestr.writeInt(-1,4);
		bytestr.writeString("%s","lx::badconnect");
		bytestr.writeString("%s",NetworkTexts->sNoIpVerification);
		bytestr.Send(tSocket);
		return;
	}

	// Check if this ip isn't already connected
	/*cl = cClients;
	for(p=0;p<MAX_CLIENTS;p++,cl++) {

		if(cl->getStatus() == NET_DISCONNECTED)
			continue;

		if(AreNetAddrEqual(&adrFrom, cl->getChannel()->getAddress())) {

			// Must not have got the connection good packet, tis ok though
			if(cl->getStatus() == NET_CONNECTED) {
				//conprintf("Duplicate connection\n");

				// Resend the 'good connection' packet
				bytestr.Clear();
				bytestr.writeInt(-1,4);
				bytestr.writeString("%s","lx::goodconnection");
                // Send the worm ids
                for( int i=0; i<cl->getNumWorms(); i++ )
                    bytestr.writeInt(cl->getWorm(i)->getID(), 1);
				bytestr.Send(tSocket);
				return;
			}

			// Trying to connect while playing? Drop the client
			if(cl->getStatus() == NET_PLAYING) {
				//conprintf("Client tried to reconnect\n");
				//DropClient(&players[p]);
				return;
			}
		}
	}*/

	// Find a spot for the client
	player = -1;
	cl = cClients;
	newcl = NULL;
	for(p=0;p<MAX_CLIENTS;p++,cl++) {
		if(cl->getStatus() == NET_DISCONNECTED) {
			newcl = cl;
			break;
		}
	}

	// Calculate number of players
	numplayers=0;
	CWorm *w = cWorms;
	for(p=0;p<MAX_WORMS;p++,w++) {
		if(w->isUsed())
			numplayers++;
	}	

	// Ran out of slots
	if(!newcl) {
		printf("I have no more open slots for the new client\n");
		bytestr.Clear();
		bytestr.writeInt(-1,4);
		bytestr.writeString("%s","lx::badconnect");
		bytestr.writeString("%s",NetworkTexts->sNoEmptySlots);
		bytestr.Send(tSocket);
		return;
	}

	// Server full (maxed already, or the number of extra worms wanting to join will go over the max)
	if(numplayers >= iMaxWorms || numplayers+numworms > iMaxWorms) {
		printf("I am full, so the new client cannot join\n");
		bytestr.Clear();
		bytestr.writeInt(-1,4);
		bytestr.writeString("%s","lx::badconnect");
		bytestr.writeString("%s",NetworkTexts->sServerFull);
		bytestr.Send(tSocket);
		return;
	}	


	// Connect
	if(newcl) {
		
		newcl->setStatus(NET_CONNECTED);

		// Set the worm info
		newcl->setNumWorms(numworms);
		//newcl->SetupWorms(numworms, worms);

		// Find spots in our list for the worms
		int ids[MAX_PLAYERS];
		int i;
		for(i=0;i<numworms;i++) {
			
			w = cWorms;
			for(p=0;p<MAX_WORMS;p++,w++) {
				if(w->isUsed())
					continue;

				*w = worms[i];
				w->setID(p);				
				w->setClient(newcl);
				w->setUsed(true);
				w->setupLobby();
				newcl->setWorm(i,w);
				ids[i] = p;
				break;
			}
		}

		
		iNumPlayers = numplayers+numworms;

		// Let em know they connected good
		bytestr.Clear();
		bytestr.writeInt(-1,4);
		bytestr.writeString("%s","lx::goodconnection");

		// Tell the client the id's of the worms
		for(i=0;i<numworms;i++)
			bytestr.writeInt(ids[i],1);

		bytestr.Send(tSocket);


		newcl->getChannel()->Create(&adrFrom,Port,tSocket);
		newcl->setLastReceived(tLX->fCurTime);
		newcl->setNetSpeed(iNetSpeed);


		// Tell all the connected clients the info about these worm(s)
		bytestr.Clear();
		/*for(i=0;i<numworms;i++) {
			w = &cWorms[ids[i]];

			bytestr.writeByte(S2C_WORMINFO);
			bytestr.writeInt(w->getID(),1);
			w->writeInfo(&bytestr);
		}*/


        //
		// Resend data about all worms to everybody
        // This is so the new client knows about all the worms
        // And the current client knows about the new worms
        //
		w = cWorms;
		for(i=0;i<MAX_WORMS;i++,w++) {

			if(!w->isUsed())
				continue;

			bytestr.writeByte(S2C_WORMINFO);
			bytestr.writeInt(w->getID(),1);
			w->writeInfo(&bytestr);
		}

		SendGlobalPacket(&bytestr);



		// TODO: Set socket info
		// TODO: This better

		static char buf[256];
		static char buf2[256];
		// "Has connected" message
		if (strcmp(NetworkTexts->sHasConnected,"<none>"))  {
			for(i=0;i<numworms;i++) {
				replacemax(NetworkTexts->sHasConnected,"<player>",worms[i].getName(),buf,1);
				SendGlobalText(buf,TXT_NETWORK);
			}
		}

		// Welcome message
		fix_strncpy(buf,tGameInfo.sWelcomeMessage);
		if(fix_strnlen(tGameInfo.sWelcomeMessage) > 0)  {

			// Server name
			replacemax(buf,"<server>",tGameInfo.sServername,buf,1);

			// Host name
			replacemax(buf,"<me>",cWorms[0].getName(),buf,1);

			// Country
			if (strstr(buf,"<country>") != NULL)  {
				static char country[128];
				static char str_addr[22];
				NetAddrToString(newcl->getChannel()->getAddress(),str_addr);
				if (fix_strnlen(str_addr))  {
					GetCountryFromIP(str_addr,country);
					replacemax(buf,"<country>",country,buf,1);
				}
			}

			// Address
			static char str_addr[22];
			NetAddrToString(newcl->getChannel()->getAddress(),str_addr);
			fix_markend(str_addr);
			// Remove port
			char* pos = strrchr(str_addr,':');
			if(pos != NULL)
				str_addr[(int) (pos-str_addr)] = '\0';
			replacemax(buf,"<ip>",str_addr,buf,1);


			for(int i=0; i<numworms; i++)  {
				fix_strncpy(buf2,buf);

				// Player name
				replacemax(buf2,"<player>",worms[i].getName(),buf2,1);

				// Send the welcome message
				SendGlobalText(buf2,TXT_NETWORK);
			}
		}


		// Tell the client the game lobby details
		// Note: This sends a packet to ALL clients, not just the new client
		UpdateGameLobby();
        if( tGameInfo.iGameType != GME_LOCAL )
            SendWormLobbyUpdate();

		// Recolorize the nicks in lobby
		iHost_Recolorize = true;

		

		// Client spawns when the game starts
	}
}


///////////////////
// Parse a ping packet
void CServer::ParsePing(void)
{
	NetworkAddr		adrFrom;
	GetRemoteNetAddr(tSocket,&adrFrom);

	// Send the challenge details back to the client
	SetRemoteNetAddr(tSocket,&adrFrom);

	CBytestream bs;

	bs.Clear();
	bs.writeInt(-1,4);
	bs.writeString("%s","lx::pong");
	
	bs.Send(tSocket);
}

///////////////////
// Parse a "wants join" packet
void CServer::ParseWantsJoin(CBytestream *bs)
{
	// TODO: Accept these messages from banned clients?

	if (!tLXOptions->tGameinfo.bAllowWantsJoinMsg)
		return;
	static char Nick[128];
	bs->readString(Nick, sizeof(Nick));
	static char buf[256];

	// Notify about the wants join
	if (strcmp(NetworkTexts->sWantsJoin,"<none>"))  {
		replacemax(NetworkTexts->sWantsJoin,"<player>",Nick,buf,1);
		SendGlobalText(buf,TXT_NORMAL);
	}
}


///////////////////
// Parse a query packet
void CServer::ParseQuery(CBytestream *bs)
{
	CBytestream bytestr;

    int num = bs->readByte();

	bytestr.Clear();
	bytestr.writeInt(-1,4);
	bytestr.writeString("%s","lx::queryreturn");

	bytestr.writeString("%s",sName);
	bytestr.writeByte(iNumPlayers);
	bytestr.writeByte(iMaxWorms);
	bytestr.writeByte(iState);
    bytestr.writeByte(num);

	bytestr.Send(tSocket);

	d_printf("sv: Sending query return\n");
}


///////////////////
// Parse a get_info packet
void CServer::ParseGetInfo(void)
{
	// TODO: more info

    CBytestream     bs;
    game_lobby_t    *gl = &tGameLobby;

    bs.Clear();
	bs.writeInt(-1,4);
	bs.writeString("%s","lx::serverinfo");

	bs.writeString("%s",sName);
	bs.writeByte(iMaxWorms);
	bs.writeByte(iState);

    // If in lobby
    if(iState == SVS_LOBBY && gl->nSet) {
	    bs.writeString("%s",gl->szMapName);
        bs.writeString("%s",gl->szModName);
	    bs.writeByte(gl->nGameMode);
	    bs.writeShort(gl->nLives);
	    bs.writeShort(gl->nMaxKills);
	    bs.writeShort(gl->nLoadingTime);
        bs.writeByte(gl->nBonuses);
    }
    // If in game
    else if(iState == SVS_PLAYING) {
        bs.writeString("%s",sMapFilename);
        bs.writeString("%s",sModName);
	    bs.writeByte(iGameType);
	    bs.writeShort(iLives);
	    bs.writeShort(iMaxKills);        
	    bs.writeShort(iLoadingTimes);
        bs.writeByte(iBonusesOn);
    }
	// Loading
	else {
        bs.writeString("%s",tGameInfo.sMapname);
        bs.writeString("%s",tGameInfo.sModName);
	    bs.writeByte(tGameInfo.iGameType);
	    bs.writeShort(tGameInfo.iLives);
	    bs.writeShort(tGameInfo.iKillLimit);        
	    bs.writeShort(tGameInfo.iLoadingTimes);
        bs.writeByte(tGameInfo.iBonusesOn);
	}


    // Players
    int     numplayers=0;
    int     p;
	CWorm *w = cWorms;
	for(p=0;p<MAX_WORMS;p++,w++) {
		if(w->isUsed())
			numplayers++;
    }

    bs.writeByte(numplayers);
    w = cWorms;
	for(p=0;p<MAX_WORMS;p++,w++) {
        if(w->isUsed()) {
            bs.writeString("%s",w->getName());
            bs.writeInt(w->getKills(), 2);
        }
    }
   

	bs.Send(tSocket);
}
