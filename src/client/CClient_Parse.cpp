/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Client class - Parsing
// Created 1/7/02
// Jason Boettcher

#include <assert.h>

#include "LieroX.h"
#include "CClient.h"
#include "CServer.h"
#include "Menu.h"
#include "console.h"
#include "GfxPrimitives.h"
#include "FindFile.h"
#include "StringUtils.h"
#include "Protocol.h"
#include "CWorm.h"
#include "Error.h"
#include "Entity.h"
#include "MathLib.h"

///////////////////
// Parse a connectionless packet
void CClient::ParseConnectionlessPacket(CBytestream *bs)
{
	static std::string cmd;

	cmd = bs->readString(128);

	if(cmd == "lx::challenge")
		ParseChallenge(bs);

	else if(cmd == "lx::goodconnection")
		ParseConnected(bs);

	else if(cmd == "lx::pong")
		ParsePong();

	// A Bad Connection
	else if(cmd == "lx::badconnect") {
		iNetStatus = NET_DISCONNECTED;
		iBadConnection = true;
		strBadConnectMsg = Utf8String(bs->readString(256));
	}

	// Unknown
	else  {
		printf("CClient::ParseConnectionlessPacket: unknown command \""+cmd+"\"");
		bs->SetPos(bs->GetLength()-1); // Safety: ignore any data behind this unknown packet
	}
}


///////////////////
// Parse a challenge packet
void CClient::ParseChallenge(CBytestream *bs)
{
	CBytestream bytestr;
	bytestr.Clear();
	iChallenge = bs->readInt(4);


	// Tell the server we are connecting, and give the server our details
	bytestr.writeInt(-1,4);
	bytestr.writeString("%s","lx::connect");
	bytestr.writeInt(PROTOCOL_VERSION,1);
	bytestr.writeInt(iChallenge,4);
	bytestr.writeInt(iNetSpeed,1);
	bytestr.writeInt(iNumWorms, 1);

	// Send my worms info
    //
    // __MUST__ match the layout in CWorm::writeInfo() !!!
    //

	for(uint i=0;i<iNumWorms;i++) {
		bytestr.writeString(RemoveSpecialChars(tProfiles[i]->sName));
		bytestr.writeInt(tProfiles[i]->iType,1);
		bytestr.writeInt(tProfiles[i]->iTeam,1);
        bytestr.writeString(tProfiles[i]->szSkin);
		bytestr.writeInt(tProfiles[i]->R,1);
		bytestr.writeInt(tProfiles[i]->G,1);
		bytestr.writeInt(tProfiles[i]->B,1);
	}

	bytestr.Send(tSocket);
}


///////////////////
// Parse a connected packet
void CClient::ParseConnected(CBytestream *bs)
{
	NetworkAddr addr;


	// Setup the client
	iNetStatus = NET_CONNECTED;

	// Get the id's
	int id=0;
	for(ushort i=0;i<iNumWorms;i++) {
		id = bs->readInt(1);
		if (id < 0 || id >= MAX_WORMS)
			continue;
		cLocalWorms[i] = &cRemoteWorms[id];
		cLocalWorms[i]->setUsed(true);
		cLocalWorms[i]->setClient(this);
		//cLocalWorms[i]->setGameScript(&cGameScript);
		//cLocalWorms[i]->setLoadingTime(fLoadingTime);
		cLocalWorms[i]->setProfile(tProfiles[i]);
		cLocalWorms[i]->setTeam(tProfiles[i]->iTeam);
		cLocalWorms[i]->setLocal(true);
        cLocalWorms[i]->setType(tProfiles[i]->iType);
	}


	// Setup the viewports
	SetupViewports();

	// Setup the controls
	cLocalWorms[0]->SetupInputs( tLXOptions->sPlayerControls[0] );
	if(iNumWorms >= 2)
		cLocalWorms[1]->SetupInputs( tLXOptions->sPlayerControls[1] );

	// Create my channel
	GetRemoteNetAddr(tSocket, &addr);
	cNetChan.Create(&addr,0,tSocket);

	iJoin_Recolorize = true;
}

//////////////////
// Parse the server's ping reply
void CClient::ParsePong(void)
{
	if (fMyPingSent > 0)  {
		int png = (int) ((tLX->fCurTime-fMyPingSent)*1000);

		// Make the ping slighter
		if (png - iMyPing > 5 && iMyPing && png)
			png = (png + iMyPing + iMyPing)/3;
		if (iMyPing - png > 5 && iMyPing && png)
			png = (png + png + iMyPing)/3;

		iMyPing = png;
	}
}

/*
=======================================
		Connected Packets
=======================================
*/


///////////////////
// Parse a packet
void CClient::ParsePacket(CBytestream *bs)
{
	uchar cmd;

	if(bs->GetLength()==0)
		return;


	while(1) {
		cmd = bs->readInt(1);

		if(bs->GetPos() > bs->GetLength())
			break;

		switch(cmd) {


			// Prepare the game
			case S2C_PREPAREGAME:
				if(!ParsePrepareGame(bs))
                    return;
				break;

			// Start the game
			case S2C_STARTGAME:
				ParseStartGame(bs);
				break;

			// Spawn a worm
			case S2C_SPAWNWORM:
				ParseSpawnWorm(bs);
				break;

			// Worm info
			case S2C_WORMINFO:
				ParseWormInfo(bs);
				break;

			// Text
			case S2C_TEXT:
				ParseText(bs);
				break;

			// Worm score
			case S2C_SCOREUPDATE:
				ParseScoreUpdate(bs);
				break;

			// Game over
			case S2C_GAMEOVER:
				ParseGameOver(bs);
				break;

			// Spawn bonus
			case S2C_SPAWNBONUS:
				ParseSpawnBonus(bs);
				break;

			// Tag update
			case S2C_TAGUPDATE:
				ParseTagUpdate(bs);
				break;

			// Some worms are ready to play (not lobby)
			case S2C_CLREADY:
				ParseCLReady(bs);
				break;

			// Update the lobby state of some worms
			case S2C_UPDATELOBBY:
				ParseUpdateLobby(bs);
				break;

			// Client has left
			case S2C_CLLEFT:
				ParseClientLeft(bs);
				break;

			// Worm state update
			case S2C_UPDATEWORMS:
				ParseUpdateWorms(bs);
				break;

			// Game lobby update
			case S2C_UPDATELOBBYGAME:
				ParseUpdateLobbyGame(bs);
				break;

			// Worm down packet
			case S2C_WORMDOWN:
				ParseWormDown(bs);
				break;

			// Server has left
			case S2C_LEAVING:
				ParseServerLeaving(bs);
				break;

			// Single shot
			case S2C_SINGLESHOOT:
				ParseSingleShot(bs);
				break;

			// Multiple shots
			case S2C_MULTISHOOT:
				ParseMultiShot(bs);
				break;

			// Stats
			case S2C_UPDATESTATS:
				ParseUpdateStats(bs);
				break;

			// Destroy bonus
			case S2C_DESTROYBONUS:
				ParseDestroyBonus(bs);
				break;

			// Goto lobby
			case S2C_GOTOLOBBY:
				ParseGotoLobby(bs);
				break;

            // I have been dropped
            case S2C_DROPPED:
                ParseDropped(bs);
                break;

			default:
				printf("cl: Unknown packet\n");
				return;

		}
	}
}


///////////////////
// Parse a prepare game packet
bool CClient::ParsePrepareGame(CBytestream *bs)
{
	// We've already got this packet
	if (iGameReady && iNetStatus != NET_CONNECTED)  {
		printf("CClient::ParsePrepareGame: we already got this\n");
		return false;
	}

	// If we're playing, the game has to be ready
	if (iNetStatus == NET_PLAYING)  {
		printf("CClient::ParsePrepareGame: playing, had to get this\n");
		iGameReady = true;
		return false;
	}


	iGameReady = true;

	int random = bs->readInt(1);

	// Clear any previous instances of the map
	if(tGameInfo.iGameType == GME_JOIN) {
		if(cMap) {
			cMap->Shutdown();
			assert(cMap);
			delete cMap;
			cMap = NULL;
		}
	}

	cGameScript.Shutdown();

    //bs->Dump();


	if(tGameInfo.iGameType == GME_JOIN) {
		cMap = new CMap;
		if(cMap == NULL) {

			// Disconnect
			Disconnect();

			Menu_MessageBox("Out of memory","Out of memory when allocating the map.",LMB_OK);

			iGameReady = false;

			printf("CClient::ParsePrepareGame: out of memory when allocating map\n");

			return false;
		}
	}


	if(random) {
		// Just create a random map

		// If we're remotely joining a server, we need to load the map
		// Note: This shouldn't happen, coz network games can't use random maps
		if(tGameInfo.iGameType == GME_JOIN) {
			if(!cMap->New(504,350,"dirt",tInterfaceSettings.MiniMapW,tInterfaceSettings.MiniMapH)) {
				Disconnect();
				iGameReady = false;
				printf("CClient::ParsePrepareGame: could not create random map\n");
				return false;
			}
			cMap->ApplyRandom();
		} else {
			// Otherwise, grab the server's copy
			assert(cServer);

			cMap = cServer->getMap();
			cMap->SetMinimapDimensions(tInterfaceSettings.MiniMapW, tInterfaceSettings.MiniMapH);
		}

	} else {

		// Load the map from a file
		static std::string buf;
		buf = bs->readString();

		// Invalid packet
		if (buf == "")  {
			printf("CClient::ParsePrepareGame: bad map name (none)\n");
			iGameReady = false;
			return false;
		}

		if(tGameInfo.iGameType == GME_JOIN) {
			cMap->SetMinimapDimensions(tInterfaceSettings.MiniMapW, tInterfaceSettings.MiniMapH);
			if(!cMap->Load(buf)) {
				// Show a cannot load level error message

				FillSurface(tMenu->bmpBuffer, tLX->clBlack);
				std::string err;
				err = std::string("Could not load the level'") + buf + "'\n" + LxGetLastError();

				Menu_MessageBox("Loading Error",err, LMB_OK);
                iClientError = true;

				// Go back to the menu
				QuittoMenu();
				iGameReady = false;

				printf("CClient::ParsePrepareGame: could not load map "+buf+"\n");
				return false;
			}
		} else {
			assert(cServer);

            // Grab the server's copy of the map
			cMap = cServer->getMap();
			cMap->SetMinimapDimensions(tInterfaceSettings.MiniMapW, tInterfaceSettings.MiniMapH);
			bMapGrabbed = true;
		}

	}

	// Other game details
	iGameType = bs->readInt(1);
	iLives = bs->readInt16();
	iMaxKills = bs->readInt16();
	iTimeLimit = bs->readInt16();
	int l = bs->readInt16();
	fLoadingTime = (float)l/100.0f;
	iBonusesOn = bs->readInt(1);
	iShowBonusName = bs->readInt(1);

	if(iGameType == GMT_TAG)
		iTagLimit = bs->readInt16();

	// Load the gamescript
	sModName = bs->readString();

	// Bad packet
	if (sModName == "")  {
		printf("CClient::ParsePrepareGame: invalid mod name (none)\n");
		iGameReady = false;
		return false;
	}

	int result = cGameScript.Load(sModName);

	if(result != GSE_OK) {

		// Show any error messages
		FillSurface(tMenu->bmpBuffer,tLX->clBlack);
		std::string err("Error load game mod: ");
		err += sModName + "\r\nError code: " + itoa(result);
		Menu_MessageBox("Loading Error",err, LMB_OK);
        iClientError = true;

		// Go back to the menu
		QuittoMenu();
		iGameReady = false;

		printf("CClient::ParsePrepareGame: error loading mod "+sModName+"\n");
        return false;
	}

    // Read the weapon restrictions
    cWeaponRestrictions.updateList(&cGameScript);
    cWeaponRestrictions.readList(bs);


	// TODO: Load any other stuff
	iGameReady = true;

	// TODO: make this working
	// problem is: Menu_Net_*GetText reads the textbox of the lobby,
	//	which already doesn't exist anymore here
	/*char *chattext = NULL;
	switch (tGameInfo.iGameType)  {
	case GME_HOST:
		chattext = Menu_Net_HostLobbyGetText();
		break;
	case GME_JOIN:
		chattext = Menu_Net_JoinLobbyGetText();
		break;
	}
	if (chattext)
		if (chattext[0])  {
			iChat_Typing = true;
			iChat_CursorVisible = true;
			fix_strncpy(sChat_Text,chattext);
		}*/

	cChatbox.setWidth(tInterfaceSettings.ChatBoxW);

	// Load the chat
	CListview *lv = (CListview *)cChatList;
	if (lv)  {
		lv->Clear();
		lines_iterator it = cChatbox.At(cChatbox.getNumLines()-256); // If there's more than 256 messages, we start not from beginning but from end()-256
		int id = (lv->getLastItem() && lv->getItems()) ? lv->getLastItem()->iIndex + 1 : 0;

		for (; it != cChatbox.End(); it++)  {

			// Add only chat text
			if (it->iColour == tLX->clChatText)  {
				lv->AddItem("", id, it->iColour);
				lv->AddSubitem(LVS_TEXT, it->strLine, NULL);
				id++;
			}
		}
		lv->scrollLast();
	}


	// Initialize the worms weapon selection menu & other stuff
	ushort i;
	for(i=0;i<iNumWorms;i++) {
		cLocalWorms[i]->setGameScript(&cGameScript);
        cLocalWorms[i]->setWpnRest(&cWeaponRestrictions);
		cLocalWorms[i]->Prepare(cMap);

		cLocalWorms[i]->InitWeaponSelection();
	}



	// (If this is a local game?), we need to reload the worm graphics
	// We do this again because we've only just found out what type of game it is
    // Team games require changing worm colours to match the team colour
	// Inefficient, but i'm not going to redesign stuff for a simple gametype
	CWorm *w = cRemoteWorms;
	for(i=0;i<MAX_WORMS;i++,w++) {
		if(w->isUsed()) {
			w->LoadGraphics(iGameType);

			// Also set some game details
			w->setLives(iLives);
            w->setKills(0);
			w->setHealth(100);
			w->setGameScript(&cGameScript);
            w->setWpnRest(&cWeaponRestrictions);
			w->setLoadingTime(fLoadingTime);

			// Prepare for battle!
			w->Prepare(cMap);
		}
	}

	UpdateScoreboard();

    return true;
}


///////////////////
// Parse a start game packet
void CClient::ParseStartGame(CBytestream *bs)
{
	// Already got this
	if (iNetStatus == NET_PLAYING)  {
		printf("CClient::ParseStartGame: already playing - ignoring\n");
		return;
	}

	iNetStatus = NET_PLAYING;

	// Set the local players to dead so we wait until the server spawns us
	for(uint i=0;i<iNumWorms;i++)
		cLocalWorms[i]->setAlive(false);

	// Initialize some variables
	iServerFrame = 0;
	fServerTime = 0;
}


///////////////////
// Parse a spawn worm packet
void CClient::ParseSpawnWorm(CBytestream *bs)
{
	int id = bs->readByte();
	int x = bs->readInt(2);
	int y = bs->readInt(2);

	// Is the spawnpoint in the map?
	if (x > (int)cMap->GetWidth() || x < 0)  {
		printf("CClient::ParseSpawnWorm: X-coordinate not in map ("+itoa(x)+")\n");
		return;
	}
	if (y > (int)cMap->GetHeight() || y < 0)  {
		printf("CClient::ParseSpawnWorm: Y-coordinate not in map ("+itoa(y)+")\n");
		return;
	}

	CVec p = CVec( (float)x, (float)y );

	if (id < 0 || id >= MAX_PLAYERS)  {
		printf("CClient::ParseSpawnWorm: invalid ID ("+itoa(id)+")\n");
		return;
	}

	cRemoteWorms[id].setAlive(true);
	cRemoteWorms[id].Spawn(p);

	cMap->CarveHole(SPAWN_HOLESIZE,p);

	// Show a spawn entity
	SpawnEntity(ENT_SPAWN,0,p,CVec(0,0),0,NULL);

	UpdateScoreboard();
}


///////////////////
// Parse a worm info packet
void CClient::ParseWormInfo(CBytestream *bs)
{
	int id = bs->readInt(1);

	// Validate the id
	if (id < 0 || id >= MAX_WORMS)  {
		printf("CClient::ParseWormInfo: invalid ID ("+itoa(id)+")\n");
		CWorm::skipInfo(bs); // Skip not to break other packets
		return;
	}

	cRemoteWorms[id].setUsed(true);
	cRemoteWorms[id].readInfo(bs);

	// Load the worm graphics
	if(!cRemoteWorms[id].LoadGraphics(iGameType)) {
        printf("CClient::ParseWormInfo(): LoadGraphics() failed\n");
	}

	cRemoteWorms[id].setupLobby();

	UpdateScoreboard();
}


///////////////////
// Parse a text packet
void CClient::ParseText(CBytestream *bs)
{
	int type = bs->readInt(1);

	// Check for the max
	/*if(iChat_Numlines+1 >= MAX_CHATLINES) {

		// Move the list up one
		for(int i=0;i<iChat_Numlines-1;i++) {
			strcpy( tChatLines[i].sText,  tChatLines[i+1].sText);
			tChatLines[i].fTime =   tChatLines[i+1].fTime;
			tChatLines[i].iType =   tChatLines[i+1].iType;
			tChatLines[i].fScroll = tChatLines[i+1].fScroll;
		}
		iChat_Numlines--;
	}

	chat_line_t *t = &tChatLines[iChat_Numlines++];

	t->fScroll = 0;
	t->fTime = tLX->fCurTime;
	t->iType = type;
	bs->readString(t->sText);*/

	Uint32 col = tLX->clWhite;
	switch(type) {
		// Chat
		case TXT_CHAT:		col = tLX->clChatText;		break;
		// Normal
		case TXT_NORMAL:	col = tLX->clNormalText;	break;
		// Notice
		case TXT_NOTICE:	col = tLX->clNotice;		break;
		// Network
		case TXT_NETWORK:	col = tLX->clNetworkText;	break;
	}

	static std::string buf;
	buf = bs->readString();

	// If we are playing a local game, discard network messages
	if(tGameInfo.iGameType == GME_LOCAL) {
		if(type == TXT_NETWORK)
			return;
		if(type != TXT_CHAT)
			col = tLX->clNormalText;
    }

    FILE *f;

	buf = Utf8String(buf);  // Convert any possible pseudo-UTF8 (old LX compatible) to normal UTF8 string

	cChatbox.AddText(buf,col,tLX->fCurTime);


	// Log the conversation
	if (tLXOptions->iLogConvos)  {
		if(!bInServer)  {
			cIConnectedBuf = buf;
			return;
		}

		f = OpenGameFile("Conversations.log","a");
		if (!f)
			return;
		fputs("    <message type=\"",f);

		switch(type) {
			// Chat
			case TXT_CHAT:		fputs("CHAT",f);	break;
			// Normal
			case TXT_NORMAL:	fputs("NORMAL",f);	break;
			// Notice
			case TXT_NOTICE:	fputs("NOTICE",f);	break;
			// Network
			case TXT_NETWORK:	fputs("NETWORK",f);	break;
		}

		fputs("\" text=\"",f);
		fputs(buf.c_str(),f);
		fputs("\" />\r\n",f);
		fclose(f);
	}
}


///////////////////
// Parse a score update packet
void CClient::ParseScoreUpdate(CBytestream *bs)
{	
	short id = bs->readInt(1);

	if(id >= 0 && id < MAX_WORMS)
		cRemoteWorms[id].readScore(bs);
	else
		// do this to get the right position in net stream
		CWorm::skipScore(bs);
	
	UpdateScoreboard();
}


///////////////////
// Parse a game over packet
void CClient::ParseGameOver(CBytestream *bs)
{
	iMatchWinner = bs->readInt(1);
	iGameOver = true;
	fGameOverTime = tLX->fCurTime;

    // Clear the projectiles
    for(int i=0; i<MAX_PROJECTILES; i++)
		cProjectiles[i].setUsed(false);
    nTopProjectile = 0;

	UpdateScoreboard();
}


///////////////////
// Parse a spawn bonus packet
void CClient::ParseSpawnBonus(CBytestream *bs)
{
	int wpn = 0;
	int type = MAX(0,MIN((int)bs->readByte(),2));

	if(type == BNS_WEAPON)
		wpn = bs->readInt(1);

	int id = bs->readByte();
	int x = bs->readInt(2);
	int y = bs->readInt(2);

	if (id < 0 || id >= MAX_BONUSES)  {
		printf("CClient::ParseSpawnBonus: invalid bonus ID ("+itoa(id)+")\n");
		return;
	}

	if (x > (int)cMap->GetWidth() || x < 0)  {
		printf("CClient::ParseSpawnBonus: X-coordinate not in map ("+itoa(x)+")\n");
		return;
	}

	if (y > (int)cMap->GetHeight() || y < 0)  {
		printf("CClient::ParseSpawnBonus: Y-coordinate not in map ("+itoa(y)+")\n");
		return;
	}

	CVec p = CVec( (float)x, (float)y );

	cBonuses[id].Spawn(p, type, wpn, &cGameScript);
	cMap->CarveHole(SPAWN_HOLESIZE,p);

	SpawnEntity(ENT_SPAWN,0,p,CVec(0,0),0,NULL);
}


///////////////////
// Parse a tag update packet
void CClient::ParseTagUpdate(CBytestream *bs)
{
	if (iNetStatus != NET_PLAYING)  {
		printf("CClient::ParseTagUpdate: not playing - ignoring\n");
		return;
	}

	int id = bs->readInt(1);
	float time = bs->readFloat();

	// Safety check
	if(id <0 || id >= MAX_WORMS)  {
		printf("CClient::ParseTagUpdate: invalid worm ID ("+itoa(id)+")\n");
		return;
	}

	if (tGameInfo.iGameMode != GMT_TAG)  {
		printf("CClient::ParseTagUpdate: game mode is not tag - ignoring\n");
		return;
	}

	// Set all the worms 'tag' property to false
	CWorm *w = cRemoteWorms;
	for(int i=0;i<MAX_WORMS;i++,w++) {
		if(w->isUsed())
			w->setTagIT(false);
	}

	// Tag the worm
	cRemoteWorms[id].setTagIT(true);
	cRemoteWorms[id].setTagTime(time);
}


///////////////////
// Parse client-ready packet
void CClient::ParseCLReady(CBytestream *bs)
{
	int numworms = bs->readByte();

	if((numworms < 1 || numworms > MAX_PLAYERS) && tGameInfo.iGameType != GME_LOCAL) {
		// bad packet
		printf("CClient::ParseCLReady: invalid numworms ("+itoa(numworms)+")\n");
		// Skip to get the right position
		for (short i=0;i<numworms;i++)  {
			bs->Skip(1); // id
			CWorm::skipWeapons(bs);
		}
		return;
	}


	byte id;
	CWorm *w;
	for(short i=0;i<numworms;i++) {
		id = bs->readByte();

		if( id >= MAX_WORMS) {
			printf("CClient::ParseCLReady: bad worm ID ("+itoa(id)+")\n");
			continue;
		}

		w = &cRemoteWorms[id];
		if(w) {
			w->setGameReady(true);

			// Read the weapon info
			w->readWeapons(bs);
		} else {
			// Skip the info and if end of packet, just end
			if (CWorm::skipWeapons(bs))
				break;
		}

	}
}


///////////////////
// Parse an update-lobby packet
void CClient::ParseUpdateLobby(CBytestream *bs)
{
	int numworms = bs->readByte();
	int ready = bs->readByte();

	if (iNetStatus != NET_CONNECTED || numworms < 1 || numworms > MAX_WORMS)  {
		if (iNetStatus != NET_CONNECTED)
			printf("CClient::ParseUpdateLobby: not in lobby - ignoring\n");
		else
			printf("CClient::ParseUpdateLobby: invalid numworms value ("+itoa(numworms)+")\n");

		// Skip to get the right position in stream
		bs->Skip(numworms);
		return;
	}

	static std::string HostName;

	byte id;
	CWorm *w;
	for(short i=0;i<numworms;i++) {
		id = bs->readByte();
        int team = MAX(0,MIN(3,(int)bs->readByte()));

		if( id >= MAX_WORMS) {
			printf("CClient::ParseUpdateLobby: invalid worm ID ("+itoa(id)+")\n");
			continue;
		}


		w = &cRemoteWorms[id];
        if(w) {
			w->getLobby()->iReady = ready;
            w->getLobby()->iTeam = team;
			if(i==0)
				HostName = w->getName();
        }
	}

	// Recolorize the skins
	iJoin_Recolorize = true;

	// Log the conversation
	if (tLXOptions->iLogConvos)  {
		if(bInServer)
			return;

		bInServer = true;

		FILE *f;

		f = OpenGameFile("Conversations.log","a");
		if (!f)
			return;
		fputs("  <server hostname=\"",f);
		fputs(HostName.c_str(),f);
		static std::string cTime = GetTime();
		fprintf(f,"\" jointime=\"%s\">\r\n",cTime.c_str());
		if(cIConnectedBuf != "")  {
			fputs("    <message type=\"NETWORK\" text=\"",f);
			fputs(cIConnectedBuf.c_str(),f);
			fputs("\" />\r\n",f);
			cIConnectedBuf = "";
		}
		fclose(f);
	}

}


///////////////////
// Parse a 'client-left' packet
void CClient::ParseClientLeft(CBytestream *bs)
{
	byte numworms = bs->readByte();

	if(numworms < 1 || numworms > MAX_PLAYERS) {
		// bad packet
		printf("CClient::ParseClientLeft: bad numworms count ("+itoa(numworms)+")\n");

		// Skip to the right position
		bs->Skip(numworms);

		return;
	}


	byte id;
	CWorm *w;
	for(byte i=0;i<numworms;i++) {
		id = bs->readByte();

		if( id >= MAX_WORMS) {
			printf("CClient::ParseClientLeft: invalid worm ID ("+itoa(id)+")\n");
			continue;
		}

		w = &cRemoteWorms[id];
		if(w) {
			w->setUsed(false);
			w->setAlive(false);
			w->getLobby()->iType = LBY_OPEN;
		}
	}

	UpdateScoreboard();
}


///////////////////
// Parse an 'update-worms' packet
void CClient::ParseUpdateWorms(CBytestream *bs)
{
	byte count = bs->readByte();
	if (count >= MAX_WORMS)  {
		printf("CClient::ParseUpdateWorms: invalid worm count ("+itoa(count)+")\n");

		// Skip to the right position
		for (byte i=0;i<count;i++)  {
			bs->Skip(1);
			CWorm::skipPacketState(bs);
		}

		return;
	}

	byte id;

	for(byte i=0;i<count;i++) {
		id = bs->readByte();

		if (id >= MAX_WORMS)  {
			printf("CClient::ParseUpdateWorms: invalid worm ID ("+itoa(id)+")\n");
			if (CWorm::skipPacketState(bs))  {  // Skip not to lose the right position
				break;
			}
			continue;
		}

		/*if (!cRemoteWorms[id].isUsed())  {
			i--;
			continue;
		}*/

		cRemoteWorms[id].readPacketState(bs,cRemoteWorms);
	}

	iJoin_Recolorize = true;
}


///////////////////
// Parse an 'update game lobby' packet
void CClient::ParseUpdateLobbyGame(CBytestream *bs)
{
	if (iNetStatus != NET_CONNECTED)  {
		printf("CClient::ParseUpdateLobbyGame: not in lobby - ignoring\n");

		// Skip to get the right position
		bs->Skip(1);
		bs->SkipString();
		bs->SkipString();
		bs->SkipString();
		bs->Skip(8); // All other info

		return;
	}

	game_lobby_t    *gl = &tGameLobby;
    FILE            *fp = NULL;

	if (!gl)  {
		//TODO: uniform message system
		//MessageBox(0,"Could not find lobby","Error",MB_OK);
		printf("CClient::ParseUpdateLobbyGame: Could not find lobby\n");
		return;
	}

	gl->nSet = true;
	gl->nMaxWorms = bs->readByte();
	gl->szMapName = bs->readString();
    gl->szModName = bs->readString();
    gl->szModDir = bs->readString();
	gl->nGameMode = bs->readByte();
	gl->nLives = bs->readInt16();
	gl->nMaxKills = bs->readInt16();
	gl->nLoadingTime = bs->readInt16();
    gl->nBonuses = bs->readByte();

    // Check if we have the level & mod
    gl->bHaveMap = true;
    gl->bHaveMod = true;

    // Does the level file exist
    fp = OpenGameFile("levels/" + gl->szMapName,"rb");
    if(!fp)
        gl->bHaveMap = false;
    else
        fclose(fp);

	// Convert the map filename to map name
	if (gl->bHaveMap)  {
		std::string MapName = Menu_GetLevelName(gl->szMapName);
		gl->szDecodedMapName = (MapName != "") ? MapName : gl->szMapName;
	}

    // Does the 'script.lgs' file exist in the mod dir?
    fp = OpenGameFile(gl->szModDir + "/script.lgs", "rb");
    if(!fp)
        gl->bHaveMod = false;
    else
        fclose(fp);

	iJoin_Recolorize = true;

}


///////////////////
// Parse a 'worm down' packet
void CClient::ParseWormDown(CBytestream *bs)
{
	// Don't allow anyone to kill us in lobby
	if (iNetStatus != NET_PLAYING)  {
		printf("CClient::ParseWormDown: not playing - ignoring\n");
		bs->Skip(1);  // ID
		return;
	}

	byte id = bs->readByte();
	byte n;
	CWorm *w;
	float amount;
	short i;

	if(id < MAX_WORMS) {
		cRemoteWorms[id].setAlive(false);

		// Make a death sound
		int s = GetRandomInt(2);
		StartSound( sfxGame.smpDeath[s], cRemoteWorms[id].getPos(), cRemoteWorms[id].getLocal(), -1, cLocalWorms[0]);

		// Spawn some giblets
		w = &cRemoteWorms[id];

		for(n=0;n<7;n++)
			SpawnEntity(ENT_GIB,0,w->getPos(),CVec(GetRandomNum()*80,GetRandomNum()*80),0,w->getGibimg());

		// Blood
		amount = 50.0f * ((float)tLXOptions->iBloodAmount / 100.0f);
		for(i=0;i<amount;i++) {
			float sp = GetRandomNum()*100+50;
			SpawnEntity(ENT_BLOODDROPPER,0,w->getPos(),CVec(GetRandomNum()*sp,GetRandomNum()*sp),MakeColour(128,0,0),NULL);
			SpawnEntity(ENT_BLOOD,0,w->getPos(),CVec(GetRandomNum()*sp,GetRandomNum()*sp),MakeColour(200,0,0),NULL);
			SpawnEntity(ENT_BLOOD,0,w->getPos(),CVec(GetRandomNum()*sp,GetRandomNum()*sp),MakeColour(128,0,0),NULL);
		}
	} else {
		printf("CClient::ParseWormDown: invalid worm ID ("+itoa(id)+")\n");
	}
}


///////////////////
// Parse a 'server left' packet
void CClient::ParseServerLeaving(CBytestream *bs)
{
	// Set the server error details

	// Not so much an error, but rather a disconnection of communication between us & server
	iServerError = true;
	strServerErrorMsg = "Server has quit";

	if (tLXOptions->iLogConvos)  {
		if(!bInServer)
			return;

		FILE *f;

		f = OpenGameFile("Conversations.log","a");
		if (!f)
			return;
		fputs("  </server>\r\n",f);
		bInServer = false;
		fclose(f);
	}
}


///////////////////
// Parse a 'single shot' packet
void CClient::ParseSingleShot(CBytestream *bs)
{
	if(iNetStatus != NET_PLAYING)  {
		printf("CClient::ParseSingleShot: not playing - ignoring\n");
		CShootList::skipSingle(bs); // Skip to get to the correct position
		return;
	}

	cShootList.readSingle(bs);

	// Process the shots
	ProcessShots();

}


///////////////////
// Parse a 'multi shot' packet
void CClient::ParseMultiShot(CBytestream *bs)
{
	if(iNetStatus != NET_PLAYING)  {
		printf("CClient::ParseMultiShot: not playing - ignoring\n");
		CShootList::skipMulti(bs); // Skip to get to the correct position 
		return;
	}

	cShootList.readMulti(bs);

	// Process the shots
	ProcessShots();
}


///////////////////
// Update the worms stats
void CClient::ParseUpdateStats(CBytestream *bs)
{
	byte num = bs->readByte();
	if (num > MAX_PLAYERS)
		printf("CClient::ParseUpdateStats: invalid worm count ("+itoa(num)+") - clamping\n");

	short oldnum = num;
	num = MIN(num,MAX_PLAYERS);

	short i;
	for(i=0; i<num; i++)
		getWorm(i)->readStatUpdate(bs);

	// Skip if there were some clamped worms
	for (i=0;i<oldnum-num;i++)
		if (CWorm::skipStatUpdate(bs))
			break;
}


///////////////////
// Parse a 'destroy bonus' packet
void CClient::ParseDestroyBonus(CBytestream *bs)
{
	byte id = bs->readByte();

	if( id < MAX_BONUSES )
		cBonuses[id].setUsed(false);
	else
		printf("CClient::ParseDestroyBonus: invalid bonus ID ("+itoa(id)+")\n");
}


///////////////////
// Parse a 'goto lobby' packet
void CClient::ParseGotoLobby(CBytestream *bs)
{
	// Do a minor clean up
	MinorClear();

	// Hide the console
	Con_Hide();

	if(tGameInfo.iGameType == GME_HOST) {
		// Goto the host lobby
		Menu_Net_GotoHostLobby();

	} else
	if(tGameInfo.iGameType == GME_JOIN) {

		// Tell server my worms aren't ready
		CBytestream bs;
		bs.Clear();
		bs.writeByte(C2S_UPDATELOBBY);
		bs.writeByte(0);
		cNetChan.getMessageBS()->Append(&bs);


		// Goto the join lobby
		Menu_Net_GotoJoinLobby();
	}
}


///////////////////
// Parse a 'dropped' packet
void CClient::ParseDropped(CBytestream *bs)
{
    // Set the server error details

	// Not so much an error, but i message as to why i was dropped
	iServerError = true;
	strServerErrorMsg = Utf8String(bs->readString(256));

	if (tLXOptions->iLogConvos)  {
		if(!bInServer)
			return;

		FILE *f;

		f = OpenGameFile("Conversations.log","a");
		if (!f)
			return;
		fputs("    <message type=\"NETWORK\" text=\"",f);
		fputs(strServerErrorMsg.c_str(),f);
		fputs("\" />",f);
		fputs("  </server>\r\n",f);
		bInServer = false;
		fclose(f);
	}
}
