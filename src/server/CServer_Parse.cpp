/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Server class - Parsing
// Created 1/7/02
// Jason Boettcher


#include "defs.h"
#include "LieroX.h"
#include "Menu.h"
#include "CServer.h"
#include "CClient.h"
#include "CChannel.h"
#include "StringUtils.h"


/*
=======================================
		Connected Packets
=======================================
*/


///////////////////
// Parses a general packet
void GameServer::ParseClientPacket(CClient *cl, CBytestream *bs)
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
void GameServer::ParsePacket(CClient *cl, CBytestream *bs)
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
void GameServer::ParseImReady(CClient *cl, CBytestream *bs)
{
	if (iState != SVS_GAME)  {
		printf("GameServer::ParseImReady: Not playing, packet is being ignored.\n");
		return;
	}

	int i,j;
	// Note: This isn't a lobby ready

	// Read the worms weapons
	int num = bs->readByte();
	for(i=0; i<num; i++) {
		int id = bs->readByte();
		if( id >= 0 && id < MAX_WORMS)  {
			cWorms[id].readWeapons(bs);
			for (j=0;j<5;j++)
				cWorms[id].getWeapon(j)->Enabled = cWeaponRestrictions.isEnabled(cWorms[id].getWeapon(j)->Weapon->Name);
		}
	}


	// Set this client to 'ready'
	cl->setGameReady(true);

	// Let everyone know this client is ready to play
	static CBytestream bytes;
	bytes.Clear();
	if (cl->getNumWorms() <= 2)  {
		bytes.writeByte(S2C_CLREADY);
		bytes.writeByte(cl->getNumWorms());
		for(i=0;i<cl->getNumWorms();i++) {
			// Send the weapon info here (also contains id)
			cWorms[cl->getWorm(i)->getID()].writeWeapons(&bytes);
		}

		SendGlobalPacket(&bytes);
	} else {
		int written = 0;
		while (written < cl->getNumWorms())  {
			bytes.writeByte(S2C_CLREADY);
			bytes.writeByte(1);
			cWorms[cl->getWorm(written)->getID()].writeWeapons(&bytes);
			SendGlobalPacket(&bytes);
			bytes.Clear();
			written++;
		}
	}


    // Check if all the clients are ready
    CheckReadyClient();
}


///////////////////
// Parse an update packet
void GameServer::ParseUpdate(CClient *cl, CBytestream *bs)
{
	for(short i=0; i<cl->getNumWorms(); i++) {
		CWorm *w = cl->getWorm(i);

		w->readPacket(bs, cWorms);

		// If the worm is shooting, handle it
		if(w->getWormState()->iShoot && w->getAlive() && iState == SVS_PLAYING)
			WormShoot(w);
	}
}


///////////////////
// Parse a death packet
void GameServer::ParseDeathPacket(CClient *cl, CBytestream *bs)
{
	// No kills in lobby
	if (iState != SVS_PLAYING)  {
		printf("GameServer::ParseDeathPacket: Not playing, ignoring the packet.\n");
		return;
	}

	CBytestream byte;
	int victim = bs->readInt(1);
	int killer = bs->readInt(1);

	// Bad packet
	if (bs->GetPos() > bs->GetLength())  {
		printf("GameServer::ParseDeathPacket: Reading beyond the end of stream.\n");
		return;
	}

	// Team names
	static const std::string TeamNames[] = {"blue", "red", "green", "yellow"};
	int TeamCount[4];

    // If the game is already over, ignore this
	// TODO: is this safe?
    if(iGameOver)  {
		printf("GameServer::ParseDeathPacket: Game is over, ignoring.\n");
        return;
	}


	// Safety check
	if(victim < 0 || victim >= MAX_WORMS)  {
		printf("GameServer::ParseDeathPacket: victim ID out of bounds.\n");
		return;
	}
	if(killer < 0 || killer >= MAX_WORMS)  {
		printf("GameServer::ParseDeathPacket: killer ID out of bounds.\n");
		return;
	}

	CWorm *vict = &cWorms[victim];
	CWorm *kill = &cWorms[killer];
	log_worm_t *log_vict = GetLogWorm(vict->getID());
	log_worm_t *log_kill = GetLogWorm(kill->getID());

	// Cheat prevention check: Make sure the victim is one of the client's worms
	if (!cl->OwnsWorm(vict))  {
		printf("GameServer::ParseDeathPacket: victim is not one of the client's worms.\n");
		return;
	}

	// Cheat prevention, game behaves weird if this happens
	if(vict->getLives() < 0 && iLives >= 0)  {
		printf("GameServer::ParseDeathPacket: victim is already out of the game.\n");
		return;
	}

	static std::string buf;

	// Kill
	if (networkTexts->sKilled != "<none>")  { // Take care of the <none> tag
		if(killer != victim)  {
			replacemax(networkTexts->sKilled,"<killer>",kill->getName(),buf,1);
			replacemax(buf,"<victim>",vict->getName(),buf,1);
		}
		else
			replacemax(networkTexts->sCommitedSuicide,"<player>",vict->getName(),buf,1);

		SendGlobalText(buf,TXT_NORMAL);
	}

	// First blood
	if (bFirstBlood && killer != victim && networkTexts->sFirstBlood!="<none>")  {
		replacemax(networkTexts->sFirstBlood,"<player>",kill->getName(),buf,1);
		bFirstBlood = false;
		SendGlobalText(buf,TXT_NORMAL);
	}

	// Teamkill
	if (iGameType == GMT_TEAMDEATH && vict->getTeam() == kill->getTeam() && killer != victim)  {
		//Take care of the <none> tag
		if (networkTexts->sTeamkill!="<none>")  {
			replacemax(networkTexts->sTeamkill,"<player>",kill->getName(),buf,1);
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
		if (networkTexts->sSpree1!="<none>")  {
			replacemax(networkTexts->sSpree1,"<player>",kill->getName(),buf,1);
			SendGlobalText(buf,TXT_NORMAL);
		}
		break;
	case 5:
		if (networkTexts->sSpree2!="<none>")  {
			replacemax(networkTexts->sSpree2,"<player>",kill->getName(),buf,1);
			SendGlobalText(buf,TXT_NORMAL);
		}
		break;
	case 7:
		if (networkTexts->sSpree3!="<none>")  {
			replacemax(networkTexts->sSpree3,"<player>",kill->getName(),buf,1);
			SendGlobalText(buf,TXT_NORMAL);
		}
		break;
	case 9:
		if (networkTexts->sSpree4!="<none>")  {
			replacemax(networkTexts->sSpree4,"<player>",kill->getName(),buf,1);
			SendGlobalText(buf,TXT_NORMAL);
		}
		break;
	case 10:
		if (networkTexts->sSpree5!="<none>")  {
			replacemax(networkTexts->sSpree5,"<player>",kill->getName(),buf,1);
			SendGlobalText(buf,TXT_NORMAL);
		}
		break;
	}


	if(vict->Kill()) {


		// This worm is out of the game
		if(networkTexts->sPlayerOut!="<none>") {
			replacemax(networkTexts->sPlayerOut,"<player>",vict->getName(),buf,1);
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
				if (networkTexts->sPlayerHasWon!="<none>")  {
					CWorm *winner = cWorms + wormid;
					replacemax(networkTexts->sPlayerHasWon,"<player>",winner->getName(),buf,1);
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
				if (networkTexts->sPlayerHasWon!="<none>")  {
					replacemax(networkTexts->sPlayerHasWon,"<player>",w->getName(),buf,1);
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
				if (networkTexts->sTeamOut!="<none>")  {
					replacemax(networkTexts->sTeamOut,"<team>",TeamNames[team],buf,1);
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
				if (networkTexts->sTeamHasWon!="<none>")  {
					replacemax(networkTexts->sTeamHasWon,"<team>",TeamNames[team],buf,1);
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
void GameServer::ParseChatText(CClient *cl, CBytestream *bs)
{
	static char buf[256];
	bs->readString(buf, sizeof(buf));

	// Don't send text from muted players
	if (cl)
		if (cl->getMuted())
			return;

	SendGlobalText( buf, TXT_CHAT);
}


///////////////////
// Parse a 'update lobby' packet
void GameServer::ParseUpdateLobby(CClient *cl, CBytestream *bs)
{
	// Must be in lobby
	if (iState != SVS_LOBBY)  {
		printf("GameServer::ParseUpdateLobby: Not in lobby.\n");
		return;
	}

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
	bytestr.Clear();
	if (cl->getNumWorms() <= 2)  {
		bytestr.writeByte(S2C_UPDATELOBBY);
		bytestr.writeByte(cl->getNumWorms());
		bytestr.writeByte(ready);
		for(i=0; i<cl->getNumWorms(); i++) {
			bytestr.writeByte(cl->getWorm(i)->getID());
			bytestr.writeByte(cl->getWorm(i)->getLobby()->iTeam);
		}
		SendGlobalPacket(&bytestr);
	} else {
		int written = 0;
		while (written < cl->getNumWorms())  {
			bytestr.writeByte(S2C_UPDATELOBBY);
			bytestr.writeByte(1);
			bytestr.writeByte(ready);
			bytestr.writeByte(cl->getWorm(written)->getID());
			bytestr.writeByte(cl->getWorm(written)->getLobby()->iTeam);
			written++;
			SendGlobalPacket(&bytestr);
			bytestr.Clear();
		}
	}
}


///////////////////
// Parse a disconnect packet
void GameServer::ParseDisconnect(CClient *cl)
{
	// Check if the client hasn't already left
	if(cl->getStatus() == NET_DISCONNECTED)  {
		printf("GameServer::ParseDisconnect: Client has already disconnected.\n");
		return;
	}

	DropClient(cl, CLL_QUIT);
}


///////////////////
// Parse a weapon list packet
void GameServer::ParseWeaponList(CClient *cl, CBytestream *bs)
{
	int id = bs->readByte();

	if( id >= 0 && id < MAX_WORMS)
		cWorms[id].readWeapons(bs);
	else
		printf("GameServer::ParseWeaponList: worm ID out of bounds.\n");
}


///////////////////
// Parse a 'grab bonus' packet
void GameServer::ParseGrabBonus(CClient *cl, CBytestream *bs)
{
	if (iState != SVS_PLAYING)  {
		printf("GameServer::ParseGrabBonus: Not playing.\n");
		return;
	}

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
			} else {
				printf("GameServer::ParseGrabBonus: Bonus already destroyed.\n");
			}
		} else {
			printf("GameServer::ParseGrabBonus: Invalid bonus ID\n");
		}
	} else {
		printf("GameServer::ParseGrabBonus: invalid worm ID\n");
	}
}





/*
===========================

  Connectionless packets

===========================
*/


///////////////////
// Parses connectionless packets
void GameServer::ParseConnectionlessPacket(CBytestream *bs)
{
	static std::string cmd;

	cmd = bs->readString(128);

	if(cmd == "lx::getchallenge")
		ParseGetChallenge();
	else if(cmd == "lx::connect")
		ParseConnect(bs);
	else if(cmd == "lx::ping")
		ParsePing();
	else if(cmd == "lx::query")
		ParseQuery(bs);
    else if(cmd == "lx::getinfo")
        ParseGetInfo();
	else if(cmd == "lx::wantsjoin")
		ParseWantsJoin(bs);
	else 
		printf("GameServer::ParseConnectionlessPacket: unknown packet\n");
}


///////////////////
// Handle a "getchallenge" msg
void GameServer::ParseGetChallenge(void)
{
	int			i;
	NetworkAddr	adrFrom;
	float		OldestTime = 99999;
	int			ChallengeToSet = -1;
	static CBytestream	bs;
	bs.Clear();

	GetRemoteNetAddr(tSocket,&adrFrom);

	// If were in the game, deny challenges
	if(iState != SVS_LOBBY) {
		bs.Clear();
		bs.writeInt(-1,4);
		bs.writeString("%s","lx::badconnect");
		bs.writeString(networkTexts->sGameInProgress);
		bs.Send(tSocket);
		printf("GameServer::ParseGetChallenge: Cannot join, the game is in progress.");
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
void GameServer::ParseConnect(CBytestream *bs)
{
	static CBytestream		bytestr;
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
	if(iState != SVS_LOBBY)  {
		printf("GameServer::ParseConnect: In game, ignoring.");
		return;
	}

	bytestr.Clear();

	// User Info to get

	GetRemoteNetAddr(tSocket,&adrFrom);

	// Read packet
	ProtocolVersion = bs->readInt(1);
	if(ProtocolVersion != PROTOCOL_VERSION) {
		printf("Wrong protocol version, server protocol version is %d\n", PROTOCOL_VERSION);

		// Get the string to send
		static std::string buf;
		if(networkTexts->sTeamHasWon != "<none>")  {
			replacemax(networkTexts->sWrongProtocol,"<version>",itoa(PROTOCOL_VERSION),buf,1);
		}
		else
			buf = " ";

		// Wrong protocol version, don't connect client
		bytestr.Clear();
		bytestr.writeInt(-1,4);
		bytestr.writeString("%s","lx::badconnect");
		bytestr.writeString(buf);
		bytestr.Send(tSocket);
		printf("GameServer::ParseConnect: Wrong protocol version");
		return;
	}

	static std::string szAddress;
	NetAddrToString(&adrFrom,szAddress);

	// Is this IP banned?
	if (getBanList()->isBanned(szAddress))  {
		printf("Banned client %s was trying to connect\n", szAddress.c_str());
		bytestr.Clear();
		bytestr.writeInt(-1,4);
		bytestr.writeString("%s","lx::badconnect");
		bytestr.writeString(networkTexts->sYouAreBanned);
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
	numworms = MIN(numworms,MAX_PLAYERS);
	CWorm worms[MAX_PLAYERS];
	for(i=0;i<numworms;i++) {
		worms[i].readInfo(bs);
		// If bots aren't allowed, disconnect the client
		if (worms[i].getType() == PRF_COMPUTER && !tLXOptions->tGameinfo.bAllowRemoteBots && !strincludes(szAddress,"127.0.0.1"))  {
			printf("Bot was trying to connect\n");
			bytestr.Clear();
			bytestr.writeInt(-1,4);
			bytestr.writeString("%s","lx::badconnect");
			bytestr.writeString(networkTexts->sBotsNotAllowed);
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
			bytestr.writeString(networkTexts->sBadVerification);
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
		bytestr.writeString(networkTexts->sNoIpVerification);
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
		bytestr.writeString(networkTexts->sNoEmptySlots);
		bytestr.Send(tSocket);
		return;
	}

	// Server full (maxed already, or the number of extra worms wanting to join will go over the max)
	if(numplayers >= iMaxWorms || numplayers+numworms > iMaxWorms) {
		printf("I am full, so the new client cannot join\n");
		bytestr.Clear();
		bytestr.writeInt(-1,4);
		bytestr.writeString("%s","lx::badconnect");
		bytestr.writeString(networkTexts->sServerFull);
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

		static std::string buf;
		// "Has connected" message
		if (networkTexts->sHasConnected != "<none>")  {
			for(i=0;i<numworms;i++) {
				SendGlobalText(replacemax(networkTexts->sHasConnected,"<player>",worms[i].getName(),1),TXT_NETWORK);
			}
		}

		// Welcome message
		buf = tGameInfo.sWelcomeMessage;
		if(buf.size() > 0)  {

			// Server name
			replacemax(buf,"<server>",tGameInfo.sServername,buf,1);

			// Host name
			replacemax(buf,"<me>",cWorms[0].getName(),buf,1);

			// Country
			if (buf.find("<country>") != std::string::npos)  {
				static IpInfo info;
				static std::string str_addr;
				NetAddrToString(newcl->getChannel()->getAddress(),str_addr);
				if (str_addr != "")  {
					info = tIpToCountry->GetInfoAboutIP(str_addr);
					replacemax(buf,"<country>",info.Country,buf,1);
				}
			}

			// Continent
			if (buf.find("<continent>") != std::string::npos)  {
				static IpInfo info;
				static std::string str_addr;
				NetAddrToString(newcl->getChannel()->getAddress(),str_addr);
				if (str_addr != "")  {
					info = tIpToCountry->GetInfoAboutIP(str_addr);
					replacemax(buf,"<continent>",info.Continent,buf,1);
				}
			}


			// Address
			static std::string str_addr;
			NetAddrToString(newcl->getChannel()->getAddress(),str_addr);
			// Remove port
			size_t pos = str_addr.rfind(':');
			if(pos != std::string::npos)
				str_addr.erase(pos);
			replacemax(buf,"<ip>",str_addr,buf,1);

			for(int i=0; i<numworms; i++)  {
				// Player name
				// Send the welcome message
				SendGlobalText(replacemax(buf,"<player>",worms[i].getName(),1),TXT_NETWORK);
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
void GameServer::ParsePing(void)
{
	static NetworkAddr		adrFrom;
	GetRemoteNetAddr(tSocket,&adrFrom);

	// Send the challenge details back to the client
	SetRemoteNetAddr(tSocket,&adrFrom);

	static CBytestream bs;

	bs.Clear();
	bs.writeInt(-1,4);
	bs.writeString("%s","lx::pong");

	bs.Send(tSocket);
}

///////////////////
// Parse a "wants to join" packet
void GameServer::ParseWantsJoin(CBytestream *bs)
{
	// TODO: Accept these messages from banned clients?

	if (!tLXOptions->tGameinfo.bAllowWantsJoinMsg)
		return;
	static std::string Nick;
	Nick = bs->readString();
	static std::string buf;

	// Notify about the wants to join
	if (networkTexts->sWantsJoin!="<none>")  {
		replacemax(networkTexts->sWantsJoin,"<player>",Nick,buf,1);
		SendGlobalText(buf,TXT_NORMAL);
	}
}


///////////////////
// Parse a query packet
void GameServer::ParseQuery(CBytestream *bs)
{
	static CBytestream bytestr;

    int num = bs->readByte();

	bytestr.Clear();
	bytestr.writeInt(-1,4);
	bytestr.writeString("%s","lx::queryreturn");

	bytestr.writeString(sName);
	bytestr.writeByte(iNumPlayers);
	bytestr.writeByte(iMaxWorms);
	bytestr.writeByte(iState);
    bytestr.writeByte(num);

	bytestr.Send(tSocket);

	d_printf("sv: Sending query return\n");
}


///////////////////
// Parse a get_info packet
void GameServer::ParseGetInfo(void)
{
	// TODO: more info

    static CBytestream     bs;
    game_lobby_t    *gl = &tGameLobby;

    bs.Clear();
	bs.writeInt(-1,4);
	bs.writeString("%s","lx::serverinfo");

	bs.writeString(sName);
	bs.writeByte(iMaxWorms);
	bs.writeByte(iState);

    // If in lobby
    if(iState == SVS_LOBBY && gl->nSet) {
	    bs.writeString(gl->szMapName);
        bs.writeString(gl->szModName);
	    bs.writeByte(gl->nGameMode);
	    bs.writeInt16(gl->nLives);
	    bs.writeInt16(gl->nMaxKills);
	    bs.writeInt16(gl->nLoadingTime);
        bs.writeByte(gl->nBonuses);
    }
    // If in game
    else if(iState == SVS_PLAYING) {
        bs.writeString(sMapFilename);
        bs.writeString(sModName);
	    bs.writeByte(iGameType);
	    bs.writeInt16(iLives);
	    bs.writeInt16(iMaxKills);
	    bs.writeInt16(iLoadingTimes);
        bs.writeByte(iBonusesOn);
    }
	// Loading
	else {
        bs.writeString(tGameInfo.sMapname);
        bs.writeString(tGameInfo.sModName);
	    bs.writeByte(tGameInfo.iGameType);
	    bs.writeInt16(tGameInfo.iLives);
	    bs.writeInt16(tGameInfo.iKillLimit);
	    bs.writeInt16(tGameInfo.iLoadingTimes);
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
            bs.writeString(w->getName());
            bs.writeInt(w->getKills(), 2);
        }
    }


	bs.Send(tSocket);
}
