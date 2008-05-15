/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie, Albert Zeyer and Martin Griffin
//
//
/////////////////////////////////////////


// Server class - Parsing
// Created 1/7/02
// Jason Boettcher

#include <iostream>

#include "LieroX.h"
#include "Menu.h"
#include "CServer.h"
#include "CClient.h"
#include "CChannel.h"
#include "StringUtils.h"
#include "CWorm.h"
#include "Protocol.h"
#include "ConfigHandler.h"
#include "ChatCommand.h"
#include "DedicatedControl.h"
#include "AuxLib.h"
#include "Version.h"
#include "OLXModInterface.h"
using namespace OlxMod;


using namespace std;

#ifdef _MSC_VER
#undef min
#undef max
#endif

/*
=======================================
		Connected Packets
=======================================
*/


///////////////////
// Parses a general packet
void GameServer::ParseClientPacket(CClient *cl, CBytestream *bs) {

	// TODO: That's hack, all processing should be done in CChannel_056b
	// Maybe server will work without that code at all, should check against 0.56b
	CChannel_056b *chan = dynamic_cast< CChannel_056b * > (cl->getChannel());
	if( chan != NULL )
	{
		// Ensure the incoming sequence matchs the outgoing sequence
		if (chan->getInSeq() >= chan->getOutSeq())  {
			//if (chan->getInSeq() != chan->getOutSeq())
			//	printf(cl->getWorm(0)->getName() + ": sequences not same (IN: " + itoa(chan->getInSeq()) + ", OUT: " + itoa(chan->getOutSeq()) + ")\n");
			//else
			//	printf(cl->getWorm(0)->getName() + ": sequences match!! (" + itoa(chan->getInSeq()) + ")\n");*/
			chan->setOutSeq(chan->getInSeq());
		} else {
			// Sequences have slipped
			// Karel said: it's bullshit from JasonB, so we can ignore this warning :)
			//printf(cl->getWorm(0)->getName() + ": sequences have slipped (IN: " + itoa(chan->getInSeq()) + ", OUT: " + itoa(chan->getOutSeq()) + ")\n");
			// TODO: Set the player's send_data property to false
		}
	}
	
	cl->setLastReceived(tLX->fCurTime);


	// Do not process empty packets
	if (bs->isPosAtEnd())
		return;

	// Parse the packet messages
	ParsePacket(cl, bs);
}


///////////////////
// Parse a packet
void GameServer::ParsePacket(CClient *cl, CBytestream *bs) {
	uchar cmd;

	while (!bs->isPosAtEnd()) {
		cmd = bs->readInt(1);

		switch (cmd) {

			// Client is ready
		case C2S_IMREADY:
			ParseImReady(cl, bs);
			break;

			// Update packet
		case C2S_UPDATE:
			ParseUpdate(cl, bs);
			break;

			// Death
		case C2S_DEATH:
			ParseDeathPacket(cl, bs);
			break;

			// Chat text
		case C2S_CHATTEXT:
			ParseChatText(cl, bs);
			break;

			// Update lobby
		case C2S_UPDATELOBBY:
			ParseUpdateLobby(cl, bs);
			break;

			// Disconnect
		case C2S_DISCONNECT:
			ParseDisconnect(cl);
			break;

			// Bonus grabbed
		case C2S_GRABBONUS:
			ParseGrabBonus(cl, bs);
			break;

		case C2S_SENDFILE:
			ParseSendFile(cl, bs);
			break;

		case C2S_OLXMOD_DATA:
			ParseOlxModData(cl, bs);
			break;

		default:
			// HACK, HACK: old olx/lxp clients send the ping twice, once normally once per channel
			// which leads to warnings here - we simply parse it here and avoid warnings

			/*
			It's a bug in LieroX Pro that sent the ping
			packet twice: once per CChannel - it looked like this:
			(8 bytes of CChannel's sequence)(Connectionless header)(Packet itself)
			and then the same packet normally (via CBytestream::Send)
			(Connectionless header) (Packet itself)

			The hack solved the first case which generated warning of unknown packet.
			*/

			// Avoid "reading from stream behind end" warning if this is really a bad packet
			// and print the bad command instead
			if (cmd == 0xff && bs->GetRestLen() > 3)
				if (bs->readInt(3) == 0xffffff) {
					std::string address;
					NetAddrToString(cl->getChannel()->getAddress(), address);
					ParseConnectionlessPacket(cl->getChannel()->getSocket(), bs, address);
					break;
				}

			// Really a bad packet
			printf("sv: Bad command in packet (" + itoa(cmd) + ")\n");
		}
	}
}


///////////////////
// Parse a 'im ready' packet
void GameServer::ParseImReady(CClient *cl, CBytestream *bs) {
	if ( iState != SVS_GAME )
	{
		printf("GameServer::ParseImReady: Not playing, packet is being ignored.\n");

		// Skip to get the correct position in the stream
		int num = bs->readByte();
		for (int i = 0;i < num;i++)  {
			bs->Skip(1);
			CWorm::skipWeapons(bs);
		}

		return;
	}

	int i, j;
	// Note: This isn't a lobby ready

	// Read the worms weapons
	int num = bs->readByte();
	for (i = 0; i < num; i++) {
		int id = bs->readByte();
		if (id >= 0 && id < MAX_WORMS)  {
			if(!cWorms[id].isUsed()) {
				printf("WARNING: got unused worm-ID!\n");
				CWorm::skipWeapons(bs);
				continue;
			}
			cWorms[id].readWeapons(bs);
			for (j = 0; j < 5; j++)
				cWorms[id].getWeapon(j)->Enabled =
					cWeaponRestrictions.isEnabled(
						cWorms[id].getWeapon(j)->Weapon->Name) ||
						cWeaponRestrictions.isBonus(cWorms[id].getWeapon(j)->Weapon->Name);
		} else { // Skip to get the right position
			CWorm::skipWeapons(bs);
		}
	}


	// Set this client to 'ready'
	cl->setGameReady(true);

	// Let everyone know this client is ready to play
	CBytestream bytes;
	if (cl->getNumWorms() <= 2)  {
		bytes.writeByte(S2C_CLREADY);
		bytes.writeByte(cl->getNumWorms());
		for (i = 0;i < cl->getNumWorms();i++) {
			// Send the weapon info here (also contains id)
			cWorms[cl->getWorm(i)->getID()].writeWeapons(&bytes);
		}

		SendGlobalPacket(&bytes);
		// HACK: because of old 0.56b clients we have to pretend there are clients handling the bots
		// Otherwise, 0.56b would not parse the packet correctly
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
void GameServer::ParseUpdate(CClient *cl, CBytestream *bs) {
	for (short i = 0; i < cl->getNumWorms(); i++) {
		CWorm *w = cl->getWorm(i);

		w->readPacket(bs, cWorms);

		// If the worm is shooting, handle it
		if (w->getWormState()->bShoot && w->getAlive() && iState == SVS_PLAYING)
			WormShoot(w, this); // handle shot and add to shootlist to send it later to the clients
	}
}


///////////////////
// Parse a death packet
void GameServer::ParseDeathPacket(CClient *cl, CBytestream *bs) {
	// No kills in lobby
	if (iState != SVS_PLAYING)  {
		printf("GameServer::ParseDeathPacket: Not playing, ignoring the packet.\n");

		// Skip to get the correct position in the stream
		bs->Skip(2);

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
	if (bGameOver)  {
		printf("GameServer::ParseDeathPacket: Game is over, ignoring.\n");
		return;
	}


	// Safety check
	if (victim < 0 || victim >= MAX_WORMS)  {
		printf("GameServer::ParseDeathPacket: victim ID out of bounds.\n");
		return;
	}
	if (killer < 0 || killer >= MAX_WORMS)  {
		printf("GameServer::ParseDeathPacket: killer ID out of bounds.\n");
		return;
	}

	CWorm *vict = &cWorms[victim];
	CWorm *kill = &cWorms[killer];

	if (tLXOptions->bServerSideHealth)  {
		// Cheat prevention check (God Mode etc), make sure killer is the host or the packet is sent by the client owning the worm
		if (!cl->isLocalClient())  {
			if (cl->OwnsWorm(vict->getID()))  {  // He wants to die, let's fulfill his dream ;)
				CWorm *w = cClient->getRemoteWorms() + vict->getID();
				if (!w->getAlreadyKilled())  // Prevents killing the worm twice (once by server and once by the client itself)
					cClient->SendDeath(victim, killer);
			} else {
				printf("GameServer::ParseDeathPacket: victim is not one of the client's worms.\n");
			}


			// The client on this machine will send the death again, then we'll parse it
			return;
		}
	} else {
		// Cheat prevention check: make sure the victim is one of the client's worms
		// or if the client is host (host can kill anyone - /suicide command in chat)
		if (!cl->OwnsWorm(vict->getID()) && !cl->isLocalClient())  {
			std::string clientWorms;
			for(int i=cl->getNumWorms();i > 0 && i <= 2;i++) {
				CWorm* w = cl->getWorm(i);
				if (w) {
					if (!i%2)
						clientWorms += " ";
					clientWorms += itoa(w->getID()) + "," + w->getName() + ".";
				}
			}
			printf("GameServer::ParseDeathPacket: victim (%i,%s) is not one of the client's worms (%s).\n",
				   vict->getID(),vict->getName().c_str(),clientWorms.c_str());
			return;
		}
	}

	// Cheat prevention, game behaves weird if this happens
	if (vict->getLives() < 0 && iLives >= 0)  {
		vict->setLives(WRM_OUT);  // Safety
		printf("GameServer::ParseDeathPacket: victim is already out of the game.\n");
		return;
	}

	std::string buf;

	// Kill
	if (networkTexts->sKilled != "<none>")  { // Take care of the <none> tag
		if (killer != victim)  {
			replacemax(networkTexts->sKilled, "<killer>", kill->getName(), buf, 1);
			replacemax(buf, "<victim>", vict->getName(), buf, 1);
		} else
			replacemax(networkTexts->sCommitedSuicide, "<player>", vict->getName(), buf, 1);

		SendGlobalText(OldLxCompatibleString(buf), TXT_NORMAL);
	}

	// First blood
	if (bFirstBlood && killer != victim && networkTexts->sFirstBlood != "<none>")  {
		replacemax(networkTexts->sFirstBlood, "<player>", kill->getName(), buf, 1);
		bFirstBlood = false;
		SendGlobalText(OldLxCompatibleString(buf), TXT_NORMAL);
	}

	// Teamkill
	if ((iGameType == GMT_TEAMDEATH || iGameType == GMT_VIP) && vict->getTeam() == kill->getTeam() && killer != victim)  {
		//Take care of the <none> tag
		if (networkTexts->sTeamkill != "<none>")  {
			replacemax(networkTexts->sTeamkill, "<player>", kill->getName(), buf, 1);
			SendGlobalText(OldLxCompatibleString(buf), TXT_NORMAL);
		}
	}

	vict->setKillsInRow(0);
	vict->addDeathInRow();

	// Kills don't count in capture the flag
	if (killer != victim && (iGameType != GMT_CTF && iGameType != GMT_TEAMCTF))  {
		// Don't add a kill for teamkilling (if enabled in options)
		// Client's score and scoreboard is controlled by the server which makes this backward compatible
		if((vict->getTeam() != kill->getTeam() && killer != victim) || iGameType != GMT_TEAMDEATH || tLXOptions->bCountTeamkills ) {
			kill->addKillInRow();
			kill->AddKill();
			kill->setDeathsInRow(0);
		}
	}

	// Suicided or killed team member - decrease score if selected in options
	if( tLXOptions->tGameinfo.bSuicideDecreasesScore && ( killer == victim ||
		( (iGameType == GMT_TEAMDEATH || iGameType == GMT_VIP) && vict->getTeam() == kill->getTeam() )))
			if( kill->getKills() > 0 )
				kill->setKills( kill->getKills() - 1 );

	// If the flag was attached to the dead worm then release the flag
	for(int j=0;j<MAX_WORMS;j++)
		if(getFlagHolder(j) == victim && (iGameType == GMT_CTF || iGameType == GMT_TEAMCTF))
			setFlagHolder(-1, j);

	// Killing spree message
	switch (kill->getKillsInRow())  {
	case 3:
		if (networkTexts->sSpree1 != "<none>")  {
			replacemax(networkTexts->sSpree1, "<player>", kill->getName(), buf, 1);
			SendGlobalText(OldLxCompatibleString(buf), TXT_NORMAL);
		}
		break;
	case 5:
		if (networkTexts->sSpree2 != "<none>")  {
			replacemax(networkTexts->sSpree2, "<player>", kill->getName(), buf, 1);
			SendGlobalText(OldLxCompatibleString(buf), TXT_NORMAL);
		}
		break;
	case 7:
		if (networkTexts->sSpree3 != "<none>")  {
			replacemax(networkTexts->sSpree3, "<player>", kill->getName(), buf, 1);
			SendGlobalText(OldLxCompatibleString(buf), TXT_NORMAL);
		}
		break;
	case 9:
		if (networkTexts->sSpree4 != "<none>")  {
			replacemax(networkTexts->sSpree4, "<player>", kill->getName(), buf, 1);
			SendGlobalText(OldLxCompatibleString(buf), TXT_NORMAL);
		}
		break;
	case 10:
		if (networkTexts->sSpree5 != "<none>")  {
			replacemax(networkTexts->sSpree5, "<player>", kill->getName(), buf, 1);
			SendGlobalText(OldLxCompatibleString(buf), TXT_NORMAL);
		}
		break;
	}

	// Dying spree message
	switch (vict->getDeathsInRow()) {
	case 3:
		if (networkTexts->sDSpree1 != "<none>")  {
			replacemax(networkTexts->sDSpree1, "<player>", vict->getName(), buf, 1);
			SendGlobalText(OldLxCompatibleString(buf), TXT_NORMAL);
		}
		break;
	case 5:
		if (networkTexts->sDSpree2 != "<none>")  {
			replacemax(networkTexts->sDSpree2, "<player>", vict->getName(), buf, 1);
			SendGlobalText(OldLxCompatibleString(buf), TXT_NORMAL);
		}
		break;
	case 7:
		if (networkTexts->sDSpree3 != "<none>")  {
			replacemax(networkTexts->sDSpree3, "<player>", vict->getName(), buf, 1);
			SendGlobalText(OldLxCompatibleString(buf), TXT_NORMAL);
		}
		break;
	case 9:
		if (networkTexts->sDSpree4 != "<none>")  {
			replacemax(networkTexts->sDSpree4, "<player>", vict->getName(), buf, 1);
			SendGlobalText(OldLxCompatibleString(buf), TXT_NORMAL);
		}
		break;
	case 10:
		if (networkTexts->sDSpree5 != "<none>")  {
			replacemax(networkTexts->sDSpree5, "<player>", vict->getName(), buf, 1);
			SendGlobalText(OldLxCompatibleString(buf), TXT_NORMAL);
		}
		break;
	}


	if (vict->Kill()) {


		// This worm is out of the game
		if (networkTexts->sPlayerOut != "<none>") {
			replacemax(networkTexts->sPlayerOut, "<player>", vict->getName(), buf, 1);
			SendGlobalText(OldLxCompatibleString(buf), TXT_NORMAL);
		}

		// Check if only one person is left
		int wormsleft = 0;
		int wormid = 0;
		CWorm *w = cWorms;
		int i;
		for (i = 0;i < MAX_WORMS;i++, w++) {
			if (w->isUsed() && w->getLives() != WRM_OUT) {
				wormsleft++;
				wormid = i;
			}
		}

		if (wormsleft <= 1) { // There can be also 0 players left (you play alone and suicide)
			// Declare the winner
			switch (iGameType)  {
			case GMT_DEATHMATCH:
				if (networkTexts->sPlayerHasWon != "<none>")  {
					CWorm *winner = cWorms + wormid;
					replacemax(networkTexts->sPlayerHasWon, "<player>", winner->getName(), buf, 1);
					SendGlobalText(OldLxCompatibleString(buf), TXT_NORMAL);
				}
				break;  // DEATHMATCH
			case GMT_TAG:
				// Get the worm with greatest tag time
				float fMaxTagTime = 0;
				wormid = 0;
				CWorm *w = cWorms;
				for (int i = 0;i < MAX_WORMS;i++)
					if (w->isUsed())
						if (w->getTagTime() > fMaxTagTime)  {
							fMaxTagTime = w->getTagTime();
							wormid = i;
						}

				// Worm with greatest tag time
				w = cWorms + wormid;

				// Send the text
				if (networkTexts->sPlayerHasWon != "<none>")  {
					replacemax(networkTexts->sPlayerHasWon, "<player>", w->getName(), buf, 1);
					SendGlobalText(OldLxCompatibleString(buf), TXT_NORMAL);
				}
				break;  // TAG

				// TEAM DEATHMATCH is handled below
			}

			cout << "only one player left" << endl;
			GameOver(wormid);
		}



		// If the game is still going and this is a teamgame, check if the team this worm was in still
		// exists
		if (!bGameOver && iGameType == GMT_TEAMDEATH) {
			int team = vict->getTeam();
			int teamcount = 0;

			for (i = 0;i < 4;i++)
				TeamCount[i] = 0;

			// Check if anyone else is left on the team
			w = cWorms;
			for (i = 0;i < MAX_WORMS;i++, w++) {
				if (w->isUsed()) {
					if (w->getLives() != WRM_OUT && w->getTeam() == team)
						teamcount++;

					if (w->getLives() != WRM_OUT)
						TeamCount[w->getTeam()]++;
				}
			}

			// No-one left in the team
			if (teamcount == 0) {
				if (networkTexts->sTeamOut != "<none>")  {
					replacemax(networkTexts->sTeamOut, "<team>", TeamNames[team], buf, 1);
					SendGlobalText(OldLxCompatibleString(buf), TXT_NORMAL);
				}
			}

			// If there is only 1 team left, declare the last team the winner
			int teamsleft = 0;
			team = 0;
			for (i = 0;i < 4;i++) {
				if (TeamCount[i]) {
					teamsleft++;
					team = i;
				}
			}

			if (teamsleft <= 1) { // There can be also 0 teams left (you play TDM alone and suicide)
				if (networkTexts->sTeamHasWon != "<none>")  {
					replacemax(networkTexts->sTeamHasWon, "<team>", TeamNames[team], buf, 1);
					SendGlobalText(OldLxCompatibleString(buf), TXT_NORMAL);
				}

				cout << "no other team left" << endl;
				GameOver(team);
			}
		}
		if (!bGameOver && (iGameType == GMT_VIP || iGameType == GMT_CTF || iGameType == GMT_TEAMCTF))
			RecheckGame();
	}


	// Check if the max kills has been reached
	if (iMaxKills != -1 && killer != victim && kill->getKills() == iMaxKills) {
		cout << "max kills reached" << endl;

		// Game over (max kills reached)
		GameOver(kill->getID());
	}


	// If the worm killed is IT, then make the killer now IT
	if (iGameType == GMT_TAG && !bGameOver)  {
		if (killer != victim) {
			if (vict->getTagIT()) {
				vict->setTagIT(false);

				// If the killer is dead, tag a worm randomly
				if (!kill->getAlive() || kill->getLives() == WRM_OUT)
					TagRandomWorm();
				else
					TagWorm(kill->getID());
			}
		} else {
			// If it's a suicide and the worm was it, tag a random person
			if (vict->getTagIT()) {
				vict->setTagIT(false);
				TagRandomWorm();
			}
		}
	}

	// Update everyone on the victims & killers score
	vict->writeScore(&byte);
	if (killer != victim)
		kill->writeScore(&byte);
	if( tLXOptions->tGameinfo.bGroupTeamScore && (iGameType == GMT_TEAMDEATH || iGameType == GMT_VIP) )
	{	// All worms in the same team will have same grouped kill count
		CWorm *w = cWorms;
		for( int f = 0; f < MAX_WORMS; f++, w++ )
			if( w->isUsed() && f != killer )
				if ( w->getLives() != WRM_OUT && w->getTeam() == kill->getTeam() )
				{
					w->setKills( kill->getKills() );
					w->writeScore(&byte);
				}
	}

	// Let everyone know that the worm is now dead
	byte.writeByte(S2C_WORMDOWN);
	byte.writeByte(victim);


	SendGlobalPacket(&byte);

	if( DedicatedControl::Get() )
		DedicatedControl::Get()->WormDied_Signal(vict,kill);

}


///////////////////
// Parse a chat text packet
void GameServer::ParseChatText(CClient *cl, CBytestream *bs) {
	if(cl->getNumWorms() == 0) return;

	std::string buf = bs->readString(256);

	if (buf == "")  // Ignore empty messages
		return;

	std::string command_buf = buf;
	if (buf.size() > cl->getWorm(0)->getName().size() + 2)
		command_buf = Utf8String(buf.substr(cl->getWorm(0)->getName().size() + 2));  // Special buffer used for parsing special commands (begin with /)

	printf("Chat: "); printf(buf); printf("\n");

	// Check for special commands
	if (command_buf.size() > 2)
		if (command_buf[0] == '/' && command_buf[1] != '/')  {  // When two slashes at the beginning, parse as a normal message
			ParseChatCommand(command_buf, cl);
			return;
		}

	// Check for Clx (a cheating version of lx)
	if(buf[0] == 0x04) {
		SendGlobalText(cl->getWorm(0)->getName() + " seems to have CLX or some other hack", TXT_NORMAL);
	}

	// Don't send text from muted players
	if (cl)
		if (cl->getMuted())
			return;

	SendGlobalText(buf, TXT_CHAT);
	
	if( DedicatedControl::Get() && buf.size() > cl->getWorm(0)->getName().size() + 2 )
		DedicatedControl::Get()->ChatMessage_Signal(cl->getWorm(0),buf.substr(cl->getWorm(0)->getName().size() + 2));
}


///////////////////
// Parse a 'update lobby' packet
void GameServer::ParseUpdateLobby(CClient *cl, CBytestream *bs) {
	// Must be in lobby
	if ( iState != SVS_LOBBY )  {
		printf("GameServer::ParseUpdateLobby: Not in lobby.\n");

		// Skip to get the right position
		bs->Skip(1);

		return;
	}

	bool ready = bs->readBool();
	int i;

	// Set the client worms lobby ready state
	for (i = 0; i < cl->getNumWorms(); i++) {
		lobbyworm_t *l = cl->getWorm(i)->getLobby();
		if (l)
			l->bReady = ready;
	}

	// Let all the worms know about the new lobby state
	CBytestream bytestr;
	if (cl->getNumWorms() <= 2)  {
		bytestr.writeByte(S2C_UPDATELOBBY);
		bytestr.writeByte(cl->getNumWorms());
		bytestr.writeByte(ready);
		for (i = 0; i < cl->getNumWorms(); i++) {
			bytestr.writeByte(cl->getWorm(i)->getID());
			bytestr.writeByte(cl->getWorm(i)->getLobby()->iTeam);
		}
		SendGlobalPacket(&bytestr);
		// HACK: pretend there are clients handling the bots to get around the
		// "bug" in 0.56b
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
};


///////////////////
// Parse a disconnect packet
void GameServer::ParseDisconnect(CClient *cl) {
	// Check if the client hasn't already left
	if (cl->getStatus() == NET_DISCONNECTED)  {
		printf("GameServer::ParseDisconnect: Client has already disconnected.\n");
		return;
	}

	// Host cannot leave...
	if (cl->isLocalClient())  {
		printf("WARNING: host tried to leave\n");
		return;
	}

	DropClient(cl, CLL_QUIT);
}


///////////////////
// Parse a weapon list packet
void GameServer::ParseWeaponList(CClient *cl, CBytestream *bs) {
	int id = bs->readByte();

	if (id >= 0 && id < MAX_WORMS)
		cWorms[id].readWeapons(bs);
	else  {
		printf("GameServer::ParseWeaponList: worm ID out of bounds.\n");
		CWorm::skipWeapons(bs);  // Skip to get to the right position in the stream
	}
}


///////////////////
// Parse a 'grab bonus' packet
void GameServer::ParseGrabBonus(CClient *cl, CBytestream *bs) {
	int id = bs->readByte();
	int wormid = bs->readByte();
	int curwpn = bs->readByte();

	// Check
	if (iState != SVS_PLAYING)  {
		printf("GameServer::ParseGrabBonus: Not playing.\n");
		return;
	}


	// Worm ID ok?
	if (wormid >= 0 && wormid < MAX_WORMS) {
		CWorm *w = &cWorms[wormid];

		// Bonus id ok?
		if (id >= 0 && id < MAX_BONUSES) {
			CBonus *b = &cBonuses[id];

			if (b->getUsed()) {

				// If it's a weapon, change the worm's current weapon
				if (b->getType() == BNS_WEAPON) {

					if (curwpn >= 0 && curwpn < 5) {

						wpnslot_t *wpn = w->getWeapon(curwpn);
						wpn->Weapon = cGameScript.get()->GetWeapons() + b->getWeapon();
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

void GameServer::ParseSendFile(CClient *cl, CBytestream *bs)
{
	cl->setLastFileRequestPacketReceived( tLX->fCurTime - 10 ); // Set time in the past to force sending next packet
	if( cl->getUdpFileDownloader()->receive(bs) )
	{
		if(	cl->getUdpFileDownloader()->isFinished() &&
			cl->getUdpFileDownloader()->getFilename() == "dirt:" )
		{	// Dirt comes first - it is processed even if server disabled file downloading
			cl->setPartialDirtUpdateCount(0);
			*cl->getPreviousDirtMap() = "";
			cl->setLastDirtUpdate(tLX->fCurTime - 20.0f);	// Re-send dirt immediately
		}
		else
		if( ! tLXOptions->bAllowFileDownload )
		{	// Server disabled file downloading - send ABORT on each client request
			cl->getUdpFileDownloader()->abortDownload();
			CBytestream bs;
			bs.writeByte(S2C_SENDFILE);
			cl->getUdpFileDownloader()->send(&bs);
			SendPacket( &bs, cl );
		}
		else
		if( cl->getUdpFileDownloader()->isFinished() &&
			( cl->getUdpFileDownloader()->getFilename() == "GET:" || cl->getUdpFileDownloader()->getFilename() == "STAT:" ) )
		{
			cl->getUdpFileDownloader()->abortDownload();	// We can't provide that file or statistics on it
		};
	};
};

void GameServer::ParseOlxModData(CClient *cl, CBytestream *bs)
{
	//printf("GameServer::ParseOlxModData()\n");
	CBytestream data;
	int packetSize = OlxMod_NetPacketSize();
	int f;
	for( f=0; f<packetSize; f++ )
	{
		data.writeByte(bs->readByte());
	};

	CClient *clSend;
	for( f=0, clSend = cClients; f < MAX_CLIENTS; f++, clSend++ )
	{
		if( clSend != cl && clSend->getStatus() == NET_CONNECTED )
		{
			CBytestream dataSend;
			dataSend.writeByte( S2C_OLXMOD_DATA );
			dataSend.writeByte( cl->getWorm(0)->getID() );
			dataSend.Append(&data);
			SendPacket( &dataSend, clSend );
		};
	};
};


/*
===========================

  Connectionless packets

===========================
*/


///////////////////
// Parses connectionless packets
void GameServer::ParseConnectionlessPacket(NetworkSocket tSocket, CBytestream *bs, const std::string& ip) {
	std::string cmd = bs->readString(128);

	if (cmd == "lx::getchallenge")
		ParseGetChallenge(tSocket, bs);
	else if (cmd == "lx::connect")
		ParseConnect(tSocket, bs);
	else if (cmd == "lx::ping")
		ParsePing(tSocket);
	else if (cmd == "lx::query")
		ParseQuery(tSocket, bs, ip);
	else if (cmd == "lx::getinfo")
		ParseGetInfo(tSocket);
	else if (cmd == "lx::wantsjoin")
		ParseWantsJoin(tSocket, bs, ip);
	else if (cmd == "lx::traverse")
		ParseTraverse(tSocket, bs, ip);
	else if (cmd == "lx::registered")
		ParseServerRegistered(tSocket);
	else  {
		cout << "GameServer::ParseConnectionlessPacket: unknown packet \"" << cmd << "\"" << endl;
		bs->SkipAll(); // Safety: ignore any data behind this unknown packet
	}
}


///////////////////
// Handle a "getchallenge" msg
void GameServer::ParseGetChallenge(NetworkSocket tSocket, CBytestream *bs_in) {
	int			i;
	NetworkAddr	adrFrom;
	float		OldestTime = 99999;
	int			ChallengeToSet = -1;
	CBytestream	bs;

	GetRemoteNetAddr(tSocket, adrFrom);

	// If were in the game, deny challenges
	if ( iState != SVS_LOBBY ) {
		bs.Clear();
		// TODO: move this out here
		bs.writeInt(-1, 4);
		bs.writeString("lx::badconnect");
		bs.writeString(OldLxCompatibleString(networkTexts->sGameInProgress));
		bs.Send(tSocket);
		printf("GameServer::ParseGetChallenge: Cannot join, the game is in progress.\n");
		return;
	}


	// see if we already have a challenge for this ip
	for (i = 0;i < MAX_CHALLENGES;i++) {

		if (IsNetAddrValid(tChallenges[i].Address)) {
			if (AreNetAddrEqual(adrFrom, tChallenges[i].Address))
				continue;
			if (ChallengeToSet < 0 || tChallenges[i].fTime < OldestTime) {
				OldestTime = tChallenges[i].fTime;
				ChallengeToSet = i;
			}
		} else {
			ChallengeToSet = i;
			break;
		}
	}

	std::string client_version;
	if( ! bs_in->isPosAtEnd() )
		client_version = bs_in->readString(128);

	if (ChallengeToSet >= 0) {

		// overwrite the oldest
		tChallenges[ChallengeToSet].iNum = (rand() << 16) ^ rand();
		tChallenges[ChallengeToSet].Address = adrFrom;
		tChallenges[ChallengeToSet].fTime = tLX->fCurTime;
		tChallenges[ChallengeToSet].sClientVersion = client_version;

		i = ChallengeToSet;
	}

	// Send the challenge details back to the client
	SetRemoteNetAddr(tSocket, adrFrom);


	// TODO: move this out here
	bs.writeInt(-1, 4);
	bs.writeString("lx::challenge");
	bs.writeInt(tChallenges[i].iNum, 4);
	if( client_version != "" )
		bs.writeString(GetFullGameName());
	bs.Send(tSocket);
}


///////////////////
// Handle a 'connect' message
void GameServer::ParseConnect(NetworkSocket tSocket, CBytestream *bs) {
	CBytestream		bytestr;
	NetworkAddr		adrFrom;
	int				i, p, player = -1;
	int				numplayers;
	CClient			*cl, *newcl;

	// Connection details
	int		ProtocolVersion;
	//int		Port = LX_PORT;
	int		ChallId;
	int		iNetSpeed = 0;


	// Ignore if we are playing (the challenge should have denied the client with a msg)
	if ( iState != SVS_LOBBY )  {
//	if (iState == SVS_PLAYING) {
		printf("GameServer::ParseConnect: In game, ignoring.");
		return;
	}

	// User Info to get

	GetRemoteNetAddr(tSocket, adrFrom);

	// Read packet
	ProtocolVersion = bs->readInt(1);
	if (ProtocolVersion != PROTOCOL_VERSION) {
		printf("Wrong protocol version, server protocol version is %d\n", PROTOCOL_VERSION);

		// Get the string to send
		std::string buf;
		if (networkTexts->sWrongProtocol != "<none>")  {
			replacemax(networkTexts->sWrongProtocol, "<version>", itoa(PROTOCOL_VERSION), buf, 1);
		} else
			buf = " ";

		// Wrong protocol version, don't connect client
		bytestr.Clear();
		bytestr.writeInt(-1, 4);
		bytestr.writeString("lx::badconnect");
		bytestr.writeString(OldLxCompatibleString(buf));
		bytestr.Send(tSocket);
		printf("GameServer::ParseConnect: Wrong protocol version");
		return;
	}

	std::string szAddress;
	NetAddrToString(adrFrom, szAddress);

	// Is this IP banned?
	if (getBanList()->isBanned(szAddress))  {
		printf("Banned client %s was trying to connect\n", szAddress.c_str());
		bytestr.Clear();
		bytestr.writeInt(-1, 4);
		bytestr.writeString("lx::badconnect");
		bytestr.writeString(OldLxCompatibleString(networkTexts->sYouAreBanned));
		bytestr.Send(tSocket);
		return;
	}

	//Port = pack->ReadShort();
	ChallId = bs->readInt(4);
	iNetSpeed = bs->readInt(1);

	// Make sure the net speed is within bounds, because it's used for indexing
	iNetSpeed = CLAMP(iNetSpeed, 0, 3);

	// Get user info
	int numworms = bs->readInt(1);
	numworms = CLAMP(numworms, 0, MAX_PLAYERS);
	CWorm worms[MAX_PLAYERS];
	for (i = 0;i < numworms;i++) {
		worms[i].readInfo(bs);
		// If bots aren't allowed, disconnect the client
		if (worms[i].getType() == PRF_COMPUTER && !tLXOptions->tGameinfo.bAllowRemoteBots && !strincludes(szAddress, "127.0.0.1"))  {
			printf("Bot was trying to connect\n");
			bytestr.Clear();
			bytestr.writeInt(-1, 4);
			bytestr.writeString("lx::badconnect");
			bytestr.writeString(OldLxCompatibleString(networkTexts->sBotsNotAllowed));
			bytestr.Send(tSocket);
			return;
		}
	}

	// If we ignored this challenge verification, there could be double connections

	// See if the challenge is valid
	bool valid_challenge = false;
	for (i = 0; i < MAX_CHALLENGES; i++) {
		if (IsNetAddrValid(tChallenges[i].Address) && AreNetAddrEqual(adrFrom, tChallenges[i].Address)) {

			if (ChallId == tChallenges[i].iNum)  { // good
				SetNetAddrValid(tChallenges[i].Address, false); // Invalidate it here to avoid duplicate connections
				tChallenges[i].iNum = 0;
				valid_challenge = true;
				break;
			} else { // bad
				valid_challenge = false;

				// HINT: we could receive another connect packet which will contain this challenge
				// and therefore get the worm connected twice. To avoid it, we clear the challenge here
				SetNetAddrValid(tChallenges[i].Address, false);
				tChallenges[i].iNum = 0;
				printf("HINT: deleting a doubled challenge\n");

				// There can be more challanges from one client, if this one doesn't match,
				// perhaps some other does
			}
		}
	}

	if (!valid_challenge)  {
		printf("Bad connection verification of client\n");
		bytestr.Clear();
		bytestr.writeInt(-1, 4);
		bytestr.writeString("lx::badconnect");
		bytestr.writeString(OldLxCompatibleString(networkTexts->sBadVerification));
		bytestr.Send(tSocket);
		return;
	}

	// Ran out of challenges
	if ( i == MAX_CHALLENGES ) {
		printf("No connection verification for client found\n");
		bytestr.Clear();
		bytestr.writeInt(-1, 4);
		bytestr.writeString("lx::badconnect");
		bytestr.writeString(OldLxCompatibleString(networkTexts->sNoIpVerification));
		bytestr.Send(tSocket);
		return;
	}

	const std::string & ClientVersion = tChallenges[i].sClientVersion;

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
	for (p = 0;p < MAX_CLIENTS;p++, cl++) {
		if (cl->getStatus() == NET_DISCONNECTED) {
			newcl = cl;
			break;
		}
	}

	// Calculate number of players
	numplayers = 0;
	CWorm *w = cWorms;
	for (p = 0;p < MAX_WORMS;p++, w++) {
		if (w->isUsed())
			numplayers++;
	}

	// Ran out of slots
	if (!newcl) {
		printf("I have no more open slots for the new client\n");
		printf("%s - Server Error report",GetTime().c_str());
		printf("fCurTime is %f . Numplayers is %i\n",tLX->fCurTime,numplayers);
		cl = cClients;
		std::string msg;
		
		FILE* ErrorFile = OpenGameFile("Server_error.txt","at");
		if(ErrorFile == NULL)
			printf("Great. We can't even open a bloody file.\n");
		if(ErrorFile != NULL)
		{
			fprintf(ErrorFile,"%s - Server Error report",GetTime().c_str());
			fprintf(ErrorFile,"fCurTime is %f . Numplayers is %i\n",tLX->fCurTime,numplayers);
		}
		for (p = 0;p < MAX_CLIENTS;p++, cl++) 
		{
			msg = "Client id " + itoa(p) + ". Status: ";
			if (cl->getStatus() == NET_DISCONNECTED) 
				msg += "Disconnected.";
			else if (cl->getStatus() == NET_CONNECTED)
				msg += "Connected.";
			else if (cl->getStatus() == NET_ZOMBIE)
				msg += "Zombie. Zombie time is " + ftoa(cl->getZombieTime()) + ".";
			else
				msg += "Odd.";
			msg += "\n";
			printf(msg.c_str());
			if(ErrorFile != NULL)
				fprintf(ErrorFile,"%s",msg.c_str());
		}
		
		if(ErrorFile)
			fclose(ErrorFile);
		ErrorFile = NULL;
		
		
		
	
		bytestr.Clear();
		bytestr.writeInt(-1, 4);
		bytestr.writeString("lx::badconnect");
		bytestr.writeString(OldLxCompatibleString(networkTexts->sNoEmptySlots));
		bytestr.Send(tSocket);
		return;
	}

	// Server full (maxed already, or the number of extra worms wanting to join will go over the max)
	if (numplayers >= iMaxWorms || numplayers + numworms > iMaxWorms) {
		printf("I am full, so the new client cannot join\n");
		bytestr.Clear();
		bytestr.writeInt(-1, 4);
		bytestr.writeString("lx::badconnect");
		bytestr.writeString(OldLxCompatibleString(networkTexts->sServerFull));
		bytestr.Send(tSocket);
		return;
	}


	// Connect
	if (newcl) {
		// If this is the first client connected, it is our local client
		if (!bLocalClientConnected)  {
			std::string addr;
			NetAddrToString(adrFrom, addr);
			if (addr.find("127.0.0.1") == 0)  { // Safety: check the IP
				newcl->setLocalClient(true);
				bLocalClientConnected = true;
				printf("GameServer: our local client has connected\n");
			}
		} else {
			newcl->setLocalClient(false);
			printf("GameServer: new %s client connected\n", ClientVersion.c_str());
		}

		newcl->setClientVersion( ClientVersion );

		if( ! newcl->createChannel( std::min( newcl->getClientVersion(), GetGameVersion() ) ) )
		{	// This should not happen - just in case
			printf("Cannot create CChannel for client - invalid client version %s\n", ClientVersion.c_str() );
			bytestr.Clear();
			bytestr.writeInt(-1, 4);
			bytestr.writeString("lx::badconnect");
			bytestr.writeString(OldLxCompatibleString("Your client is incompatible to this server"));
			bytestr.Send(tSocket);
			return;
		};

		newcl->setStatus(NET_CONNECTED);

		newcl->getRights()->Nothing();  // Reset the rights here

		// Set the worm info
		newcl->setNumWorms(numworms);
		//newcl->SetupWorms(numworms, worms);

		// Find spots in our list for the worms
		int ids[MAX_PLAYERS];
		int i;
		int id = 0;
		for (i = 0;i < numworms;i++) {

			w = cWorms;
			for (p = 0;p < MAX_WORMS;p++, w++) {
				if (w->isUsed())
					continue;

				*w = worms[i]; // TODO: recode this, it's unsafe!
				w->setID(p);
				id = p;
				w->setClient(newcl);
				w->setUsed(true);
				w->setupLobby();
				if (tGameInfo.iGameType == GME_HOST)
					w->setTeam(0);
				newcl->setWorm(i, w);
				ids[i] = p;

				if( DedicatedControl::Get() )
					DedicatedControl::Get()->NewWorm_Signal(w);
				break;
			}
		}


		iNumPlayers = numplayers + numworms;

		// Let em know they connected good
		bytestr.Clear();
		bytestr.writeInt(-1, 4);
		bytestr.writeString("lx::goodconnection");

		// Tell the client the id's of the worms
		for (i = 0;i < numworms;i++)
			bytestr.writeInt(ids[i], 1);

		bytestr.Send(tSocket);

		// Let them know our version
		bytestr.Clear();
		bytestr.writeInt(-1, 4);
		// sadly we have to send this because it was not thought about any forward-compatibility when it was implemented in Beta3
		// TODO: or should we just drop compatibility with Beta3 and leave this out?
		// TODO: All these messages are sent over unreliable protocol and may be lost, we should use reliable CChannel.
		// Server version is added to challenge packet so client will receive it for sure (or won't connect at all).
		bytestr.writeString("lx::openbeta3");
		// sadly we have to send this for Beta4
		// we are sending the version string already in the challenge
		bytestr.writeInt(-1, 4);
		bytestr.writeString("lx::version");
		bytestr.writeString(GetFullGameName());
		bytestr.Send(tSocket);

		if (tLXOptions->bAllowMouseAiming)
		{
			bytestr.Clear();
			bytestr.writeInt(-1, 4);
			bytestr.writeString("lx:mouseAllowed");
			bytestr.Send(tSocket);
		}

		if (tLXOptions->bAllowStrafing)
		{
			bytestr.Clear();
			bytestr.writeInt(-1, 4);
			bytestr.writeString("lx:strafingAllowed");
			bytestr.Send(tSocket);
		}

		newcl->getChannel()->Create(&adrFrom, tSocket);
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
		for (i = 0;i < MAX_WORMS;i++, w++) {

			if (!w->isUsed())
				continue;

			bytestr.writeByte(S2C_WORMINFO);
			bytestr.writeInt(w->getID(), 1);
			w->writeInfo(&bytestr);
		}

		SendGlobalPacket(&bytestr);


		std::string buf;
		// "Has connected" message
		if (networkTexts->sHasConnected != "<none>")  {
			for (i = 0;i < numworms;i++) {
				buf = replacemax(networkTexts->sHasConnected, "<player>", worms[i].getName(), 1);
				SendGlobalText(OldLxCompatibleString(buf), TXT_NETWORK);
			}
		}

		// Welcome message
		buf = tGameInfo.sWelcomeMessage;
		if (buf.size() > 0)  {

			// Server name3
			replacemax(buf, "<server>", tGameInfo.sServername, buf, 1);

			// Host name
			replacemax(buf, "<me>", cWorms[0].getName(), buf, 1);

			// Country
			if (buf.find("<country>") != std::string::npos)  {
				static IpInfo info;
				static std::string str_addr;
				NetAddrToString(newcl->getChannel()->getAddress(), str_addr);
				if (str_addr != "")  {
					info = tIpToCountryDB->GetInfoAboutIP(str_addr);
					replacemax(buf, "<country>", info.Country, buf, 1);
				}
			}

			// Continent
			if (buf.find("<continent>") != std::string::npos)  {
				static IpInfo info;
				static std::string str_addr;
				NetAddrToString(newcl->getChannel()->getAddress(), str_addr);
				if (str_addr != "")  {
					info = tIpToCountryDB->GetInfoAboutIP(str_addr);
					replacemax(buf, "<continent>", info.Continent, buf, 1);
				}
			}


			// Address
			std::string str_addr;
			NetAddrToString(newcl->getChannel()->getAddress(), str_addr);
			// Remove port
			size_t pos = str_addr.rfind(':');
			if (pos != std::string::npos)
				str_addr.erase(pos);
			replacemax(buf, "<ip>", str_addr, buf, 1);

			for (int i = 0; i < numworms; i++)  {
				// Player name
				// Send the welcome message
				SendGlobalText(OldLxCompatibleString(replacemax(buf, "<player>", worms[i].getName(), 1)),
								TXT_NETWORK);
			}
		}

		// it doesn't make sense to save the nicks permanently on a IP-base
		// also it's very annoying to get my olx-config-dir spammed with hundreds of files
		// TODO: remove this or do it in another way
		// why do we need this at all? where is it used? what is the sense to have multiple nicks?
		/*
		// Address
		std::string str_addr;
		NetAddrToString(newcl->getChannel()->getAddress(), str_addr);
		// Remove port
		size_t pos = str_addr.rfind(':');
		if (pos != std::string::npos)
			str_addr.erase(pos);

		// Load Player Details
		std::string f = "netplay/" + str_addr + ".dat";
		int aliascount = 0;
		ReadInteger(f, "Player", "Nicks", &aliascount, 0);
		// Setup the default settings if the player is new
		if(aliascount == 0) {
			FILE *fp = OpenGameFile(f, "wt");
			fprintf(fp, "[Player]\n");
			fprintf(fp, "Nicks = 1\n");
			fprintf(fp, "Nick 0 = %s", worms[0].getName().c_str());
			fclose(fp);
		}
		else {
			bool newnick = true;
			std::string curnick;
			std::list<std::string> nicks;
			std::list<std::string>::iterator nick_it;
			for(i=0;i<aliascount;i++) {
				ReadString(f, "Player", "Nick "+itoa(i), curnick, "");
				if(!stringcasecmp(curnick, worms[0].getName()))
					newnick = false;
				nicks.push_back(curnick);
			}
			buf = replacemax("<player> is also known as: ", "<player>", worms[0].getName(), 1);
			for(nick_it=nicks.begin();nick_it != nicks.end();nick_it++)
				if(stringcasecmp(*nick_it, worms[0].getName()))
					buf = buf + *nick_it + ", ";
			buf.erase(buf.length()-2);
			if(aliascount+newnick > 1 && stringcasecmp(str_addr, "127.0.0.1"))
				SendGlobalText(OldLxCompatibleString(buf), TXT_NETWORK);
			if(newnick) {
				FILE *fp = OpenGameFile(f, "wt");
				fprintf(fp, "[Player]\n");
				fprintf(fp, "Nicks = %d\n", aliascount+1);
				for(i=0,nick_it=nicks.begin();nick_it!=nicks.end(),i<aliascount;i++,nick_it++)
					fprintf(fp, "Nick %d = %s\n", i, (*nick_it).c_str());
				fprintf(fp, "Nick %d = %s\n", i, worms[0].getName().c_str());
				fclose(fp);
			}

			// It's really useless.
			// If the worm has more than 5 names, ban it
			//if(aliascount>5) {
			//	banWorm(id);
			//	SendGlobalText(worms[0].getName()+" had too many nicks", TXT_NETWORK);
			//}
		}
		*/

		// Tell the client the game lobby details
		// Note: This sends a packet to ALL clients, not just the new client
		// TODO: if connecting during game update game lobby only for new client
		UpdateGameLobby();
		if (tGameInfo.iGameType != GME_LOCAL)
			SendWormLobbyUpdate();

		// Update players listbox
		bHost_Update = true;

		// Make host authorised
		if(newcl->isLocalClient())
			newcl->getRights()->Everything();
	}
}


///////////////////
// Parse a ping packet
void GameServer::ParsePing(NetworkSocket tSocket) {
	NetworkAddr		adrFrom;
	GetRemoteNetAddr(tSocket, adrFrom);

	// Send the challenge details back to the client
	SetRemoteNetAddr(tSocket, adrFrom);

	CBytestream bs;

	bs.Clear();
	bs.writeInt(-1, 4);
	bs.writeString("lx::pong");

	bs.Send(tSocket);
}

///////////////////
// Parse a "wants to join" packet
void GameServer::ParseWantsJoin(NetworkSocket tSocket, CBytestream *bs, const std::string& ip) {

	std::string Nick = bs->readString();

	// Allowed?
	if (!tLXOptions->tGameinfo.bAllowWantsJoinMsg)
		return;

	// Accept these messages from banned clients?
	if (!tLXOptions->tGameinfo.bWantsJoinBanned && cBanList.isBanned(ip))
		return;

	// Notify about the wants to join
	if (networkTexts->sWantsJoin != "<none>")  {
		std::string buf;
		replacemax(networkTexts->sWantsJoin, "<player>", Nick, buf, 1);
		SendGlobalText(OldLxCompatibleString(buf), TXT_NORMAL);
	}
}


///////////////////
// Parse a query packet
void GameServer::ParseQuery(NetworkSocket tSocket, CBytestream *bs, const std::string& ip) {
	static CBytestream bytestr;

	int num = bs->readByte();

	bytestr.Clear();
	bytestr.writeInt(-1, 4);
	bytestr.writeString("lx::queryreturn");

	// Get Port
	size_t pos = ip.rfind(':');
	if (pos != std::string::npos)
		ip.substr(pos);
	if(ip == "23401")
		bytestr.writeString(OldLxCompatibleString(sName+" (private)"));
	else
		bytestr.writeString(OldLxCompatibleString(sName));
	bytestr.writeByte(iNumPlayers);
	bytestr.writeByte(iMaxWorms);
	bytestr.writeByte(iState);
	bytestr.writeByte(num);

	bytestr.Send(tSocket);
}


///////////////////
// Parse a get_info packet
void GameServer::ParseGetInfo(NetworkSocket tSocket) {
	CBytestream     bs;
	game_lobby_t    *gl = &tGameLobby;

	bs.Clear();
	bs.writeInt(-1, 4);
	bs.writeString("lx::serverinfo");

	bs.writeString(OldLxCompatibleString(sName));
	bs.writeByte(iMaxWorms);
	bs.writeByte(iState);

	// If in lobby
	if (iState == SVS_LOBBY && gl->bSet) {
		bs.writeString(gl->szMapName);
		bs.writeString(gl->szModName);
		bs.writeByte(gl->nGameMode);
		bs.writeInt16(gl->nLives);
		bs.writeInt16(gl->nMaxKills);
		bs.writeInt16(gl->nLoadingTime);
		bs.writeBool(gl->bBonuses);
	}
	// If in game
	else if (iState == SVS_PLAYING) {
		bs.writeString(sMapFilename);
		bs.writeString(sModName);
		bs.writeByte(iGameType);
		bs.writeInt16(iLives);
		bs.writeInt16(iMaxKills);
		bs.writeInt16(iLoadingTimes);
		bs.writeByte(bBonusesOn);
	}
	// Loading
	else {
		bs.writeString(tGameInfo.sMapFile);
		bs.writeString(tGameInfo.sModName);
		bs.writeByte(tGameInfo.iGameType);
		bs.writeInt16(tGameInfo.iLives);
		bs.writeInt16(tGameInfo.iKillLimit);
		bs.writeInt16(tGameInfo.iLoadingTimes);
		bs.writeBool(tGameInfo.bBonusesOn);
	}


	// Players
	int     numplayers = 0;
	int     p;
	CWorm *w = cWorms;
	for (p = 0;p < MAX_WORMS;p++, w++) {
		if (w->isUsed())
			numplayers++;
	}

	bs.writeByte(numplayers);
	w = cWorms;
	for (p = 0;p < MAX_WORMS;p++, w++) {
		if (w->isUsed()) {
			bs.writeString(RemoveSpecialChars(w->getName()));
			bs.writeInt(w->getKills(), 2);
		}
	}

	w = cWorms;
	// Write out lives
	for (p = 0;p < MAX_WORMS;p++, w++) {
		if (w->isUsed())
			bs.writeInt(w->getLives(), 2);
	}

	// Write out IPs
	w = cWorms;
	for (p = 0; p < MAX_WORMS; p++, w++)  {
		if (w->isUsed())  {
			std::string addr;
			if (NetAddrToString(w->getClient()->getChannel()->getAddress(), addr))  {
				size_t pos = addr.find(':');
				if (pos != std::string::npos)
					addr.erase(pos, std::string::npos);
			} else {
				printf("ERROR: Cannot convert address for worm " + w->getName() + "\n");
			}

			if (addr.size() == 0)
				addr = "0.0.0.0";
			bs.writeString(addr);
		}
	}

	// Write out my version (we do this since Beta5)
	bs.writeString(GetFullGameName());

	bs.Send(tSocket);
}

struct SendConnectHereAfterTimeout_Data
{
	SendConnectHereAfterTimeout_Data( NetworkSocket _sock, NetworkAddr _addr ):
		sock(_sock), addr(_addr) {};
	NetworkSocket sock;
	NetworkAddr addr;
};

bool SendConnectHereAfterTimeout (Timer* sender, void* userData)
{
	SendConnectHereAfterTimeout_Data * data = (SendConnectHereAfterTimeout_Data *) userData;
	NetworkAddr addr;
	GetRemoteNetAddr( data->sock, addr );
	if( AreNetAddrEqual( addr, data->addr ) && GetNetAddrPort(addr) == GetNetAddrPort(data->addr) )
	{	// This socket wasn't used 'till we set timer routine
		CBytestream bs;
		bs.writeInt(-1, 4);
		bs.writeString("lx::connect_here");// In case server behind symmetric NAT and client has IP-restricted NAT or above
		bs.Send(data->sock);
		bs.Send(data->sock);
		bs.Send(data->sock);
	};
	delete data;
	return false;
};

// Parse NAT traverse packet - can be received only with CServer::tSocket, send responce to one of tNatTraverseSockets[]
void GameServer::ParseTraverse(NetworkSocket tSocket, CBytestream *bs, const std::string& ip)
{
	NetworkAddr		adrFrom, adrClient;
	GetRemoteNetAddr(tSocket, adrFrom);
	std::string adrClientStr = bs->readString();
	StringToNetAddr( adrClientStr, adrClient );
	printf("GameServer::ParseTraverse() %s\n", adrClientStr.c_str());

	if( !tLXOptions->bNatTraverse )
		return;

	// Find unused socket
	int socknum=-1;
	for( int f=0; f<MAX_CLIENTS; f++ )
	{
		int f1=0;
		for( ; f1<MAX_CLIENTS; f1++ )
		{
			NetworkAddr addr1, addr2;
			if( cClients[f1].getStatus() == NET_DISCONNECTED || cClients[f1].getChannel() == NULL )
				continue;
			GetLocalNetAddr( cClients[f1].getChannel()->getSocket(), addr1 );
			GetLocalNetAddr( tNatTraverseSockets[f], addr2 );
			if( GetNetAddrPort(addr1) == GetNetAddrPort(addr2) )
				break;
		};
		if( f1 >= MAX_CLIENTS )
		{
			if( socknum == -1 )
				socknum = f;
			else if( fNatTraverseSocketsLastAccessTime[socknum] < fNatTraverseSocketsLastAccessTime[f] )
				socknum = f;
		};
	};
	if( socknum >= MAX_CLIENTS || socknum < 0 )
		return;

	fNatTraverseSocketsLastAccessTime[socknum] = tLX->fCurTime;


	//printf("Sending lx:::traverse back, socknum %i\n", socknum);
	// Send lx::traverse to udp server and lx::pong to client
	CBytestream bs1;

	bs1.Clear();
	bs1.writeInt(-1, 4);
	bs1.writeString("lx::traverse");
	bs1.writeString(adrClientStr);

	// Send traverse to server
	SetRemoteNetAddr(tNatTraverseSockets[socknum], adrFrom);
	bs1.Send(tNatTraverseSockets[socknum]);

	// Send ping to client to open NAT port
	bs1.Clear();
	bs1.writeInt(-1, 4);
	bs1.writeString("lx::pong");
	SetRemoteNetAddr(tNatTraverseSockets[socknum], adrClient);
	// Send 3 times - first packet may be ignored by remote NAT
	bs1.Send(tNatTraverseSockets[socknum]);
	bs1.Send(tNatTraverseSockets[socknum]);
	bs1.Send(tNatTraverseSockets[socknum]);
	// Send "lx::connect_here" after some time if we're behind symmetric NAT and client has restricted cone NAT or global IP
	Timer( &SendConnectHereAfterTimeout,
			new SendConnectHereAfterTimeout_Data(tNatTraverseSockets[socknum], adrClient), 3000, true ).startHeadless();
};

// Server sent us "lx::registered", that means it's alive - record that
void GameServer::ParseServerRegistered(NetworkSocket tSocket)
{
	// TODO: add code here
	printf("GameServer::ParseServerRegistered()\n");
};

/////////////////
// Parse a command from chat
bool GameServer::ParseChatCommand(const std::string& message, CClient *cl)
{
	// Parse the message
	const std::vector<std::string>& parsed = ParseCommandMessage(message, false);

	// Invalid
	if (parsed.size() == 0)
		return false;

	// Get the command
	ChatCommand *cmd = GetCommand(parsed[0]);
	if (!cmd)  {
		SendText(cl, "The command is not supported.", TXT_NETWORK);
		return false;
	}

	// Check the params
	size_t num_params = parsed.size() - 1;
	if (num_params < cmd->iMinParamCount || num_params > cmd->iMaxParamCount)  {
		SendText(cl, "Invalid parameter count.", TXT_NETWORK);
		return false;
	}

	// Get the parameters
	std::vector<std::string> parameters = std::vector<std::string>(parsed.begin() + 1, parsed.end());

	// Process the command
	std::string error = cmd->tProcFunc(parameters, (cl->getNumWorms() > 0) ? cl->getWorm(0)->getID() : 0); // TODO: is this handling for worm0 correct? fix if not
	if (error.size() != 0)  {
		SendText(cl, error, TXT_NETWORK);
		return false;
	}

	return true;
}
