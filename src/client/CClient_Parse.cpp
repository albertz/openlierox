/////////////////////////////////////////
//
//             Liero Xtreme
//
//     Copyright Auxiliary Software 2002
//
//
/////////////////////////////////////////


// Client class - Parsing
// Created 1/7/02
// Jason Boettcher


#include "defs.h"
#include "LieroX.h"
#include "Menu.h"
#include "console.h"


///////////////////
// Parse a connectionless packet
void CClient::ParseConnectionlessPacket(CBytestream *bs)
{
	char cmd[128];
	bool valid = false;

	bs->readString(cmd);

	if(!strcmp(cmd,"lx::challenge"))
		ParseChallenge(bs);

	else if(!strcmp(cmd,"lx::goodconnection"))
		ParseConnected(bs);

	else if(!strcmp(cmd,"lx::pong"))
		ParsePong();

	// A Bad Connection
	else if(!strcmp(cmd,"lx::badconnect")) {
		iNetStatus = NET_DISCONNECTED;

		iBadConnection = true;
		bs->readString(strBadConnectMsg);
	}
}


///////////////////
// Parse a challenge packet
void CClient::ParseChallenge(CBytestream *bs)
{
	CBytestream bytestr;
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

	for(int i=0;i<iNumWorms;i++) {
		bytestr.writeString("%s",tProfiles[i]->sName);
		bytestr.writeInt(tProfiles[i]->iType,1);
		bytestr.writeInt(tProfiles[i]->iTeam,1);
        bytestr.writeString("%s",tProfiles[i]->szSkin);
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
	for(int i=0;i<iNumWorms;i++) {
		int id = bs->readInt(1);
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
	cLocalWorms[0]->SetupInputs( tLXOptions->sPlayer1Controls );
	if(iNumWorms >= 2)
		cLocalWorms[1]->SetupInputs( tLXOptions->sPlayer2Controls );
		
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
	if (iGameReady)
		return true;

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

			// TODO: Show 'out-of-memory' error
			return false;
		}
	}

	
	if(random) {
		// Just create a random map

		// If we're remotely joining a server, we need to load the map
		// Note: This shouldn't happen, coz network games can't use random maps
		if(tGameInfo.iGameType == GME_JOIN) {
			if(!cMap->New(504,350,"dirt")) {
				Disconnect();
				return false;
			}
			cMap->ApplyRandom();
		} else {
			// Otherwise, grab the server's copy
			assert(cServer);

			cMap = cServer->getMap();
		}

	} else {

		// Load the map from a file
		char buf[256];
		bs->readString(buf);

		// Invalid packet
		if (!strlen(buf))
			return false;

		if(tGameInfo.iGameType == GME_JOIN) {
			if(!cMap->Load(buf)) {
				// Show a cannot load level error message
			
				SDL_FillRect(tMenu->bmpBuffer, NULL, 0);
				char err[256];
				sprintf(err, "Could not load the level '%s'\n%s",buf,LxGetLastError());			
				//sprintf(err, "Could not load the level '%s'",buf);
				Menu_MessageBox("Loading Error",err, LMB_OK);
                iClientError = true;
			
				// Go back to the menu
				QuittoMenu();
				return false;
			}
		} else {
			assert(cServer);

            // Grab the server's copy of the map
			cMap = cServer->getMap();
			bMapGrabbed = true;
		}

	}

	// Other game details
	iGameType = bs->readInt(1);
	iLives = bs->readShort();
	iMaxKills = bs->readShort();
	iTimeLimit = bs->readShort();
	int l = bs->readShort();
	fLoadingTime = (float)l/100.0f;
	iBonusesOn = bs->readInt(1);
	iShowBonusName = bs->readInt(1);
	
	if(iGameType == GMT_TAG)
		iTagLimit = bs->readShort();
	
	// Load the gamescript
	bs->readString(sModName);

	// Bad packet
	if (!strlen(sModName))
		return false;

	int result = cGameScript.Load(sModName);

	if(result != GSE_OK) {

		// Show any error messages
		SDL_FillRect(tMenu->bmpBuffer, NULL, 0);
		char err[256];
		sprintf(err, "Error load game mod: %s\r\nError code: %d", sModName, result);
		Menu_MessageBox("Loading Error",err, LMB_OK);
        iClientError = true;
			
		// Go back to the menu
		QuittoMenu();
        return false;
	}

    // Read the weapon restrictions
    cWeaponRestrictions.updateList(&cGameScript);
    cWeaponRestrictions.readList(bs);
	

	// TODO: Load any other stuff
	iGameReady = true;

	setDefaultChatbox();


	// Initialize the worms weapon selection menu & other stuff
	int i;
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
	if (iNetStatus == NET_PLAYING)
		return;

	iNetStatus = NET_PLAYING;

	// Set the local players to dead so we wait until the server spawns us
	for(int i=0;i<iNumWorms;i++)
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
	if (x > cMap->GetWidth() || x < 0)
		return;
	if (y > cMap->GetHeight() || y < 0)
		return;

	CVec p = CVec( (float)x, (float)y );

	if (id < 0 || id >= MAX_PLAYERS)
		return;

	cRemoteWorms[id].setAlive(true);
	cRemoteWorms[id].Spawn(p);

	cMap->CarveHole(SPAWN_HOLESIZE,p);

	// Show a spawn entity
	if (!bBotClient)
		SpawnEntity(ENT_SPAWN,0,p,CVec(0,0),0,NULL);

	UpdateScoreboard();
}


///////////////////
// Parse a worm info packet
void CClient::ParseWormInfo(CBytestream *bs)
{
	int id = bs->readInt(1);

	// Validate the id
	if (id < 0 || id >= MAX_WORMS)
		return;

	cRemoteWorms[id].setUsed(true);
	cRemoteWorms[id].readInfo(bs);
	
	// Load the worm graphics
	if(!cRemoteWorms[id].LoadGraphics(iGameType)) {
		// TODO: Some sort of error
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

	Uint32 col = 0xffff;
	switch(type) {
		// Chat
		case TXT_CHAT:		col = MakeColour(128,255,128);	break;
		// Normal
		case TXT_NORMAL:	col = MakeColour(255,255,255);	break;
		// Notice
		case TXT_NOTICE:	col = MakeColour(200,200,200);	break;
		// Network
		case TXT_NETWORK:	col = MakeColour(200,200,200);	break;
	}

	char buf[256];
	bs->readString(buf);

	// If we are playing a local game, discard network messages
	if(tGameInfo.iGameType == GME_LOCAL) {
		if(type == TXT_NETWORK)
			return;
		if(type != TXT_CHAT)
			col = 0xffff;
    }
	
    FILE *f;

	// Safety
	buf[127] = '\0';

	pChatbox->AddText(buf,col,tLX->fCurTime);


	// Log the conversation
	if (tLXOptions->iLogConvos)  {
		if(!bInServer)  {
			for (int i=0;i<sizeof(buf);i++)
				cIConnectedBuf[i] = buf[i];
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
		fputs(buf,f);
		fputs("\" />\r\n",f);
		fclose(f);
	}
}


///////////////////
// Parse a score update packet
void CClient::ParseScoreUpdate(CBytestream *bs)
{
	int id = bs->readInt(1);
	
	if(id >= 0 && id < MAX_WORMS)
		cRemoteWorms[id].readScore(bs);

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
	int type = bs->readInt(1);

	if(type == BNS_WEAPON)
		wpn = bs->readInt(1);

	int id = bs->readInt(1);
	int x = bs->readInt(2);
	int y = bs->readInt(2);

	CVec p = CVec( (float)x, (float)y );

	cBonuses[id].Spawn(p, type, wpn, &cGameScript);
	cMap->CarveHole(SPAWN_HOLESIZE,p);

	if (!bBotClient)
		SpawnEntity(ENT_SPAWN,0,p,CVec(0,0),0,NULL);
}


///////////////////
// Parse a tag update packet
void CClient::ParseTagUpdate(CBytestream *bs)
{
	if (iNetStatus != NET_PLAYING)
		return;

	int id = bs->readInt(1);
	float time = bs->readFloat();

	// Safety check
	if(id <0 || id >= MAX_WORMS)
		return;

	if (tGameInfo.iGameMode != GMT_TAG)
		return;

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
		printf("Bad numworms count on CLREADY packet\n");
		return;
	}


	for(int i=0;i<numworms;i++) {
		int id = bs->readByte();

		if( id < 0 || id > MAX_WORMS) {
			d_printf("Bad worm id on CLREADY packet\n");
			continue;
		}

		CWorm *w = &cRemoteWorms[id];
		if(w) {
			w->setGameReady(true);

			// Read the weapon info
			w->readWeapons(bs);
		}
		
	}
}


///////////////////
// Parse an update-lobby packet
void CClient::ParseUpdateLobby(CBytestream *bs)
{
	if (iNetStatus != NET_CONNECTED)
		return;

	int numworms = bs->readByte();
	int ready = bs->readByte();

	if(numworms < 1 || numworms > MAX_PLAYERS) {
		// bad packet
		printf("Bad numworms count(%d) on UPDATELOBBY packet\n",numworms);
		return;
	}

	char *HostName;

	for(int i=0;i<numworms;i++) {
		int id = bs->readByte();
        int team = bs->readByte();

		if( id < 0 || id > MAX_WORMS) {
			printf("Bad worm id on UPDATELOBBY packet\n");
			continue;
		}
		

		CWorm *w = &cRemoteWorms[id];
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
		fputs(HostName,f);
		char cTime[26];
		GetTime(cTime);
		fprintf(f,"\" jointime=\"%s\">\r\n",cTime);
		if(cIConnectedBuf[0] != '\0')  {
			fputs("    <message type=\"NETWORK\" text=\"",f);
			fputs(cIConnectedBuf,f);
			fputs("\" />\r\n",f);
			cIConnectedBuf[0] = '\0';
		}
		fclose(f);
	}

}


///////////////////
// Parse a 'client-left' packet
void CClient::ParseClientLeft(CBytestream *bs)
{
	int numworms = bs->readByte();

	if(numworms < 1 || numworms > 2) {
		// bad packet
		printf("Bad numworms count on CLLEFT packet\n");
		return;
	}


	for(int i=0;i<numworms;i++) {
		int id = bs->readByte();

		if( id < 0 || id > MAX_WORMS) {
			printf("Bad worm id on CLLEFT packet\n");
			continue;
		}
	
		CWorm *w = &cRemoteWorms[id];
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
	int count = bs->readByte();

	for(int i=0;i<count;i++) {
		int id = bs->readByte();

		if (id < 0 || id >= MAX_WORMS)
			continue;

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
	if (iNetStatus != NET_CONNECTED)
		return;

	game_lobby_t    *gl = &tGameLobby;
    static char		buf[256] = "";
    FILE            *fp = NULL;

	if (!gl)  {
		return;
		//TODO: uniform message system
		//MessageBox(0,"Could not find lobby","Error",MB_OK);
		printf("Could not find lobby\n");
	}

	gl->nSet = true;
	gl->nMaxWorms = bs->readByte();
	bs->readString(gl->szMapName);
    bs->readString(gl->szModName);
    bs->readString(gl->szModDir);
	gl->nGameMode = bs->readByte();
	gl->nLives = bs->readShort();
	gl->nMaxKills = bs->readShort();
	gl->nLoadingTime = bs->readShort();
    gl->nBonuses = bs->readByte();

    // Check if we have the level & mod
    gl->bHaveMap = true;
    gl->bHaveMod = true;
    
    // Does the level file exist
    sprintf(buf, "levels/%s",gl->szMapName);
    fp = OpenGameFile(buf,"rb");
    if(!fp)
        gl->bHaveMap = false;
    else
        fclose(fp);

	// Convert the map filename to map name
	if (gl->bHaveMap)  {
		char *MapName;
		MapName = Menu_GetLevelName(gl->szMapName);
		if (MapName)
			sprintf(&gl->szDecodedMapName[0],"%s",MapName);
		else
			sprintf(&gl->szDecodedMapName[0],"%s",&gl->szMapName[0]);
	}

    // Does the 'script.lgs' file exist in the mod dir?
    sprintf(buf, "%s/script.lgs",gl->szModDir);
    fp = OpenGameFile(buf,"rb");
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
	if (iNetStatus != NET_PLAYING)
		return;

	int id = bs->readByte();

	if(id >= 0 && id < MAX_WORMS) {
		cRemoteWorms[id].setAlive(false);

		// Make a death sound
		int s = GetRandomInt(2);
		if (!bBotClient)
			StartSound( sfxGame.smpDeath[s], cRemoteWorms[id].getPos(), cRemoteWorms[id].getLocal(), -1, cLocalWorms[0]);

		// Spawn some giblets
		CWorm *w = &cRemoteWorms[id];
		int n;

		if (!bBotClient) {
			for(n=0;n<7;n++)
				SpawnEntity(ENT_GIB,0,w->getPos(),CVec(GetRandomNum()*80,GetRandomNum()*80),0,w->getGibimg());

			// Blood
			float amount = 50.0f * ((float)tLXOptions->iBloodAmount / 100.0f);
			for(int i=0;i<amount;i++) {
				float sp = GetRandomNum()*100+50;
				SpawnEntity(ENT_BLOODDROPPER,0,w->getPos(),CVec(GetRandomNum()*sp,GetRandomNum()*sp),MakeColour(128,0,0),NULL);
				SpawnEntity(ENT_BLOOD,0,w->getPos(),CVec(GetRandomNum()*sp,GetRandomNum()*sp),MakeColour(200,0,0),NULL);
				SpawnEntity(ENT_BLOOD,0,w->getPos(),CVec(GetRandomNum()*sp,GetRandomNum()*sp),MakeColour(128,0,0),NULL);
			}
		}
	}
}


///////////////////
// Parse a 'server left' packet
void CClient::ParseServerLeaving(CBytestream *bs)
{
	// Set the server error details
	
	// Not so much an error, but rather a disconnection of communication between us & server
	iServerError = true;
	strcpy(strServerErrorMsg, "Server has quit");

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
	if(iNetStatus != NET_PLAYING)
		return;

	cShootList.readSingle(bs);

	// Process the shots
	ProcessShots();

}


///////////////////
// Parse a 'multi shot' packet
void CClient::ParseMultiShot(CBytestream *bs)
{
	if(iNetStatus != NET_PLAYING)
		return;

	cShootList.readMulti(bs);

	// Process the shots
	ProcessShots();
}


///////////////////
// Update the worms stats
void CClient::ParseUpdateStats(CBytestream *bs)
{
	int num = bs->readByte();
	num = MIN(num,MAX_PLAYERS-1);

	for(int i=0; i<num; i++)
		getWorm(i)->readStatUpdate(bs);
}


///////////////////
// Parse a 'destroy bonus' packet
void CClient::ParseDestroyBonus(CBytestream *bs)
{
	int id = bs->readByte();

	if( id >= 0 && id < MAX_BONUSES )
		cBonuses[id].setUsed(false);
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
    static char buf[256] = "";

    // Set the server error details
	
	// Not so much an error, but i message as to why i was dropped
	iServerError = true;
	strcpy(strServerErrorMsg, bs->readString(buf));

	// Clear the bot
	if (bBotClient)  {
		MinorClear();
		iNetStatus = NET_DISCONNECTED;
	}

	if (tLXOptions->iLogConvos)  {
		if(!bInServer)
			return;

		FILE *f;

		f = OpenGameFile("Conversations.log","a");
		if (!f) 
			return;
		fputs("    <message type=\"NETWORK\" text=\"",f);
		fputs(strServerErrorMsg,f);
		fputs("\" />",f);
		fputs("  </server>\r\n",f);
		bInServer = false;
		fclose(f);
	}
}
