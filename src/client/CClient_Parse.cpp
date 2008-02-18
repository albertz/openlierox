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
#include <iostream>
#include <time.h>

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
#include "EndianSwap.h"
#include "Physics.h"

using namespace std;

///////////////////
// Parse a connectionless packet
void CClient::ParseConnectionlessPacket(CBytestream *bs)
{
	std::string cmd = bs->readString(128);
	
	if(cmd == "lx::challenge")
		ParseChallenge(bs);

	else if(cmd == "lx::goodconnection")
		ParseConnected(bs);

	else if(cmd == "lx::pong")
		ParsePong();

	// A Bad Connection
	else if(cmd == "lx::badconnect") {
		iNetStatus = NET_DISCONNECTED;
		bBadConnection = true;
		strBadConnectMsg = "Server message: " + Utf8String(bs->readString(256));
	}

	// Host has OpenLX Beta 3+
	else if(cmd.find("lx::openbeta") == 0)  {
		cout << "HINT: host is at least using OLX Beta3" << endl;
		if (cmd.size() > 12)  {
			int ver = MAX(0, atoi(cmd.substr(12)));
			iHostOLXVer = MAX(ver, iHostOLXVer);
		}
	}

	else if(cmd == "lx::version")  {
		cout << "HINT: Host is using " << bs->readString() << endl;
		// TODO: we need a gamename interpreter here which parses the game name and read out the version (and other information)
		// I would recommend to create a new class GameInfo or something which does this and which is then saved in CClient::hostGameVersionInfo
		// we just set it to 4 now which is save also for future version because all servers which send this are at least OLX beta4
		iHostOLXVer = 4;
	}

	else if (cmd == "lx:mouseAllowed")
		bHostAllowsMouse = true;

	// Unknown
	else  {
		cout << "CClient::ParseConnectionlessPacket: unknown command \"" << cmd << "\"" << endl;
		bs->SkipAll(); // Safety: ignore any data behind this unknown packet
	}
}


///////////////////
// Parse a challenge packet
void CClient::ParseChallenge(CBytestream *bs)
{
	CBytestream bytestr;
	bytestr.Clear();
	iChallenge = bs->readInt(4);
	if( ! bs->isPosAtEnd() )
		setServerVersion( bs->readString(128) );


	// TODO: move this out here
	// Tell the server we are connecting, and give the server our details
	bytestr.writeInt(-1,4);
	bytestr.writeString("lx::connect");
	bytestr.writeInt(PROTOCOL_VERSION,1);
	bytestr.writeInt(iChallenge,4);
	bytestr.writeInt(iNetSpeed,1);
	bytestr.writeInt(iNumWorms, 1);

	// Send my worms info
    //
    // __MUST__ match the layout in CWorm::writeInfo() !!!
    //

	for(uint i=0;i<iNumWorms;i++) {
		// TODO: move this out here
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
		cLocalWorms[i]->setGameScript(&cGameScript); // TODO: why was this commented out?
		//cLocalWorms[i]->setLoadingTime(fLoadingTime);  // TODO: why is this commented out?
		cLocalWorms[i]->setProfile(tProfiles[i]);
		cLocalWorms[i]->setTeam(tProfiles[i]->iTeam);
		cLocalWorms[i]->setLocal(true);
        cLocalWorms[i]->setType(tProfiles[i]->iType);
	}

	if(!bDedicated) {
		// Setup the viewports
		SetupViewports();
	
		// Setup the controls
		cLocalWorms[0]->SetupInputs( tLXOptions->sPlayerControls[0] );
		// TODO: setup also more viewports
		if(iNumWorms >= 2)
			cLocalWorms[1]->SetupInputs( tLXOptions->sPlayerControls[1] );
	}

	// Create my channel
	GetRemoteNetAddr(tSocket, addr);
	cNetChan.Create(&addr,0,tSocket);

	bJoin_Update = true;
	bHost_Update = true;

	bHostAllowsMouse = false;
	
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

	while(!bs->isPosAtEnd()) {
		cmd = bs->readInt(1);

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

            case S2C_SENDFILE:
                ParseSendFile(bs);
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
	printf("Got ParsePrepareGame\n");

	if(tLX->bQuitEngine) {
		printf("HINT: some previous action tried to quit the GameLoop; we are ignoring this now\n");
		tLX->bQuitEngine = false;
	}
	
	// We've already got this packet
	if (bGameReady && iNetStatus != NET_CONNECTED)  {
		printf("CClient::ParsePrepareGame: we already got this\n");
		return false;
	}

	// If we're playing, the game has to be ready
	if (iNetStatus == NET_PLAYING)  {
		printf("CClient::ParsePrepareGame: playing, had to get this\n");
		bGameReady = true;
		return false;
	}


	// remove from notifier; we don't want events anymore, we have a fixed FPS rate ingame
	RemoveSocketFromNotifierGroup( tSocket );

	bGameReady = true;
	bForceWeaponsReady = false;

	int random = bs->readInt(1);

	// Clear any previous instances of the map
	if(tGameInfo.iGameType == GME_JOIN) {
		if(cMap) {
			cMap->Shutdown();
			delete cMap;
			cMap = NULL;
		}
	}

	// HINT: gamescript is ishut down by the cache

    //bs->Dump();


	if(tGameInfo.iGameType == GME_JOIN) {
		cMap = new CMap;
		if(cMap == NULL) {

			// Disconnect
			Disconnect();

			Menu_MessageBox("Out of memory","Out of memory when allocating the map.",LMB_OK);

			bGameReady = false;

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
				bGameReady = false;
				printf("CClient::ParsePrepareGame: could not create random map\n");
				return false;
			}
			cMap->ApplyRandom();
		} else {
			// Otherwise, grab the server's copy
			assert(cServer);

			cMap = cServer->getMap();
			if (!cMap)  {  // Bad packet
				bGameReady = false;
				return false;
			} else {
				cMap->SetMinimapDimensions(tInterfaceSettings.MiniMapW, tInterfaceSettings.MiniMapH);
				bMapGrabbed = true;
			}
		}

	} else {

		// Load the map from a file
		std::string buf = bs->readString();

		// Invalid packet
		if (buf == "")  {
			printf("CClient::ParsePrepareGame: bad map name (none)\n");
			bGameReady = false;
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
                bClientError = true;

				// Go back to the menu
				QuittoMenu();
				bGameReady = false;

				printf("CClient::ParsePrepareGame: could not load map "+buf+"\n");
				return false;
			}
		} else {
			assert(cServer);

            // Grab the server's copy of the map
			cMap = cServer->getMap();
			if (!cMap)  {  // Bad packet
				bGameReady = false;
				return false;
			} else {
				cMap->SetMinimapDimensions(tInterfaceSettings.MiniMapW, tInterfaceSettings.MiniMapH);
				bMapGrabbed = true;
			}
		}

	}

	PhysicsEngine::Get()->initGame(cMap);

	// Other game details
	iGameType = bs->readInt(1);
	iLives = bs->readInt16();
	iMaxKills = bs->readInt16();
	iTimeLimit = bs->readInt16();
	int l = bs->readInt16();
	fLoadingTime = (float)l/100.0f;
	bBonusesOn = bs->readBool();
	bShowBonusName = bs->readBool();

	if(iGameType == GMT_TAG)
		iTagLimit = bs->readInt16();

	// Load the gamescript
	sModName = bs->readString();

	// Bad packet
	if (sModName == "")  {
		printf("CClient::ParsePrepareGame: invalid mod name (none)\n");
		bGameReady = false;
		return false;
	}

	int result = cGameScript.Load(sModName);

	if(result != GSE_OK) {

		// Show any error messages
		FillSurface(tMenu->bmpBuffer, tLX->clBlack);
		std::string err("Error load game mod: ");
		err += sModName + "\r\nError code: " + itoa(result);
		Menu_MessageBox("Loading Error", err, LMB_OK);
        bClientError = true;

		// Go back to the menu
		GotoNetMenu();
		bGameReady = false;

		printf("CClient::ParsePrepareGame: error loading mod "+sModName+"\n");
        return false;
	}

    // Read the weapon restrictions
    cWeaponRestrictions.updateList(&cGameScript);
    cWeaponRestrictions.readList(bs);


	// TODO: Load any other stuff
	bGameReady = true;

	
	// Copy the chat text from lobby to ingame chatbox
	switch (tGameInfo.iGameType)  {
	case GME_HOST:
		sChat_Text = Menu_Net_HostLobbyGetText();
		break;
	case GME_JOIN:
		sChat_Text = Menu_Net_JoinLobbyGetText();
		break;
	}

	if (!sChat_Text.empty())  {
		bChat_Typing = true;
		bChat_CursorVisible = true;
		iChat_Pos = sChat_Text.size();
	}

	cChatbox.setWidth(tInterfaceSettings.ChatBoxW - 4);

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
				lv->AddSubitem(LVS_TEXT, it->strLine, NULL, NULL);
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
	int num_worms = 0;
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

			num_worms++;
		}
	}

	// Start the game logging
	StartLogging(num_worms);

	UpdateScoreboard();
	bShouldRepaintInfo = true;

	bJoin_Update = true;
	
	getUdpFileDownloader()->reset();
	getPreviousDirtMap()->Clear();
	if( cMap )
		cMap->SendDirtUpdate( getPreviousDirtMap() );
	setPartialDirtUpdateCount( 0 );

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
	
	printf("Client: get start game signal\n");
	fLastSimulationTime = tLX->fCurTime;
	iNetStatus = NET_PLAYING;

	// Set the local players to dead so we wait until the server spawns us
	for(uint i=0;i<iNumWorms;i++)
		cLocalWorms[i]->setAlive(false);

	// Initialize some variables
	iServerFrame = 0;
	fServerTime = 0;

	// Re-initialize the ingame scoreboard
	InitializeIngameScore(false);
	bUpdateScore = true;


	// let our worms know that the game starts know
	for(uint i=0;i<iNumWorms;i++) {
		cLocalWorms[i]->StartGame();
	}	
}


///////////////////
// Parse a spawn worm packet
void CClient::ParseSpawnWorm(CBytestream *bs)
{
	int id = bs->readByte();
	int x = bs->readInt(2);
	int y = bs->readInt(2);

	// Check
	if (iNetStatus != NET_PLAYING)  {
		printf("CClient::ParseSpawnWorm: Cannot spawn when not playing (packet ignored)\n");
		return;
	}

	if (!cMap)
		return;

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
	if (cRemoteWorms[id].getLocal())
		bShouldRepaintInfo = true;
	
	if( bSpectate && iNumWorms > 0 && cLocalWorms[0] == &cRemoteWorms[id] && cLocalWorms[0]->getType() == PRF_HUMAN )
	{
		SendDeath( id, id ); // Suicide myself as long as I spawned
	};
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

	UpdateScoreboard();
	if (cRemoteWorms[id].getLocal())
		bShouldRepaintInfo = true;

	bJoin_Update = true;
	bHost_Update = true;
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
	int	t = bDedicated ? 0 : cLocalWorms[0]->getTeam();
	switch(type) {
		// Chat
		case TXT_CHAT:		col = tLX->clChatText;		break;
		// Normal
		case TXT_NORMAL:	col = tLX->clNormalText;	break;
		// Notice
		case TXT_NOTICE:	col = tLX->clNotice;		break;
		// Network
		case TXT_NETWORK:	col = tLX->clNetworkText;	break;
		// Private
		case TXT_PRIVATE:	col = tLX->clPrivateText;	break;
		// Team Private Chat
		case TXT_TEAMPM:	col = tLX->clTeamColors[t];	break;
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

	cChatbox.AddText(buf, col, tLX->fCurTime);


	// Log the conversation
	if (tLXOptions->bLogConvos)  {
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
			// Private
			case TXT_PRIVATE:	fputs("PRIVATE",f);	break;
			// Team Private Chat
			case TXT_TEAMPM:	fputs("TEAMPM",f);	break;
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

	if(id >= 0 && id < MAX_WORMS)  {
		log_worm_t *l = GetLogWorm(id);

		// Update the score
		cRemoteWorms[id].readScore(bs);
		if (cRemoteWorms[id].getLocal())
			bShouldRepaintInfo = true;

		// Logging
		if (l)  {
			// Check if the stats changed
			bool stats_changed = false;
			if (l->iLives != cRemoteWorms[id].getLives())  {
				l->iLives = cRemoteWorms[id].getLives();
				iLastVictim = id;
				stats_changed = true;
			}

			if (l->iKills != cRemoteWorms[id].getKills())  {
				l->iKills = cRemoteWorms[id].getKills();
				iLastKiller = id;
				stats_changed = true;
			}

			// If the update was sent but no changes made -> this is a killer that made a teamkill
			// See CServer::ParseDeathPacket for more info
			if (!stats_changed)
				iLastKiller = id;
		}
	}
	else
		// do this to get the right position in net stream
		CWorm::skipScore(bs);
	
	UpdateScoreboard();

	bJoin_Update = true;
	bHost_Update = true;
}


///////////////////
// Parse a game over packet
void CClient::ParseGameOver(CBytestream *bs)
{
	iMatchWinner = CLAMP(bs->readInt(1), 0, MAX_PLAYERS - 1);

	// Get the winner team if TDM (old servers send wrong info here, better when we find it out)
	if (tGameInfo.iGameMode == GMT_TEAMDEATH)  {

		if (tGameInfo.iKillLimit != -1)  {
			iMatchWinner = cRemoteWorms[iMatchWinner].getTeam();
		} else if (tGameInfo.iLives != -2)  {
			for (int i=0; i < MAX_WORMS; i++)  {
				if (cRemoteWorms[i].getLives() >= 0)  {
					iMatchWinner = cRemoteWorms[i].getTeam();
					break;
				}
			}
		}
	}

	// Older servers send wrong info about tag winner, better if we count it ourself
	if (tGameInfo.iGameMode == GMT_TAG)  {
		float max = 0;

		for (int i=0; i < MAX_WORMS; i++)  {
			if (cRemoteWorms[i].isUsed() && cRemoteWorms[i].getTagTime() > max)  {
				max = cRemoteWorms[i].getTagTime();
				iMatchWinner = i;
			}
		}
	}

	// Game over
	cout << "the game is over" << endl;
	bGameOver = true;
	fGameOverTime = tLX->fCurTime;

	if (tGameLog)
		tGameLog->iWinner = iMatchWinner;

    // Clear the projectiles
    for(int i=0; i < MAX_PROJECTILES; i++)
		cProjectiles[i].setUnused();
    nTopProjectile = 0;

	UpdateScoreboard();
	bShouldRepaintInfo = true;
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

	// Check
	if (iNetStatus != NET_PLAYING)  {
		printf("CClient::ParseSpawnBonus: Cannot spawn bonus when not playing (packet ignored)\n");
		return;
	}

	if (id < 0 || id >= MAX_BONUSES)  {
		printf("CClient::ParseSpawnBonus: invalid bonus ID ("+itoa(id)+")\n");
		return;
	}

	if (!cMap) { // Weird
		printf("WARNING: CClient::ParseSpawnBonus: cMap not set\n");
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

	// Log it
	log_worm_t *l = GetLogWorm(id);
	if (l)  {
		for (int i=0; i < tGameLog->iNumWorms; i++)
			tGameLog->tWorms[i].bTagIT = false;

		l->fTagTime = time;
		l->bTagIT = true;
	}
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

	bUpdateScore = true; // Change the ingame scoreboard
}


///////////////////
// Parse an update-lobby packet
void CClient::ParseUpdateLobby(CBytestream *bs)
{
	int numworms = bs->readByte();
	bool ready = bs->readBool();

	if (iNetStatus != NET_CONNECTED || numworms < 0 || numworms > MAX_WORMS)  {
		if (iNetStatus != NET_CONNECTED)
			printf("CClient::ParseUpdateLobby: not in lobby - ignoring\n");
		else
			printf("CClient::ParseUpdateLobby: invalid strange numworms value ("+itoa(numworms)+")\n");

		// Skip to get the right position in stream
		bs->Skip(numworms);
		return;
	}
	if(numworms == 0)
		printf("CClient::ParseUpdateLobby: warning: numworms == 0\n");
		
	std::string HostName;

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
			w->getLobby()->bReady = ready;
            w->getLobby()->iTeam = team;
			w->setTeam(team);
			if(i==0)
				HostName = w->getName();
        }
	}

	// Update lobby
	bJoin_Update = true;
	bHost_Update = true;

	// Log the conversation
	if (tLXOptions->bLogConvos)  {
		if(bInServer)
			return;

		bInServer = true;

		FILE *f;

		f = OpenGameFile("Conversations.log","a");
		if (!f)
			return;
		fputs("  <server hostname=\"",f);
		fputs(HostName.c_str(),f);
		std::string cTime = GetTime();
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

			// Log this
			if (tGameLog)  {
				log_worm_t *l = GetLogWorm(id);
				if (l)  {
					l->bLeft = true;
					l->fTimeLeft = tLX->fCurTime;
				}
			}

		}
	}

	bJoin_Update = true;
	bHost_Update = true;

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

	bJoin_Update = true;
	bHost_Update = true;
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

	gl->bSet = true;
	gl->nMaxWorms = bs->readByte();
	gl->szMapName = bs->readString();
    gl->szModName = bs->readString();
    gl->szModDir = bs->readString();
	gl->nGameMode = bs->readByte();
	gl->nLives = bs->readInt16();
	gl->nMaxKills = bs->readInt16();
	gl->nLoadingTime = bs->readInt16();
    gl->bBonuses = bs->readBool();

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

	bJoin_Update = true;
	bHost_Update = true;
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
		if (cRemoteWorms[id].getLocal() && cRemoteWorms[id].getType() == PRF_HUMAN)
			cRemoteWorms[id].clearInput();

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

	// Someone has been killed, log it
	if (iLastVictim != -1)  {
		log_worm_t *l_vict = GetLogWorm(iLastVictim);
		log_worm_t *l_kill = l_vict;

		// If we haven't received killer's update score, it has been a suicide
		if (iLastKiller != -1)
			l_kill = GetLogWorm(iLastKiller);

		if (l_kill && l_vict)  {
			// HINT: lives and kills are updated in ParseScoreUpdate

			// Suicide
			if (l_kill == l_vict)  {
				l_vict->iSuicides++;
			}

			// Teamkill
			else if (cRemoteWorms[iLastKiller].getTeam() == cRemoteWorms[iLastVictim].getTeam())  {
				l_kill->iTeamKills++;
				l_vict->iTeamDeaths++;
			}
		}
	}

	// Reset
	iLastVictim = iLastKiller = -1;
}


///////////////////
// Parse a 'server left' packet
void CClient::ParseServerLeaving(CBytestream *bs)
{
	// Set the server error details

	// Not so much an error, but rather a disconnection of communication between us & server
	bServerError = true;
	strServerErrorMsg = "Server has quit";

	if (tLXOptions->bLogConvos)  {
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
	num = (byte)MIN(num,MAX_PLAYERS);

	// For death logging
	// TODO: not used
//	int killer = -1;
//	int victim = -1;

	short i;
	for(i=0; i<num; i++)
		if (getWorm(i))  {
			if (getWorm(i)->getLocal())
				bShouldRepaintInfo = true;

			getWorm(i)->readStatUpdate(bs);
		}

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

	if (iNetStatus != NET_PLAYING)  {
		printf("CClient::ParseDestroyBonus: Ignoring, the game is not running.\n");
		return;
	}

	if( id < MAX_BONUSES )
		cBonuses[id].setUsed(false);
	else
		printf("CClient::ParseDestroyBonus: invalid bonus ID ("+itoa(id)+")\n");
}


///////////////////
// Parse a 'goto lobby' packet
void CClient::ParseGotoLobby(CBytestream *)
{
	printf("Client: received gotoLobby signal\n");
	
	// in lobby we need the events again
	AddSocketToNotifierGroup( tSocket );

	// Do a minor clean up
	MinorClear();

	// Hide the console
	Con_Hide();


	if(tGameInfo.iGameType == GME_JOIN) {

		// Tell server my worms aren't ready
		CBytestream bs;
		bs.Clear();
		bs.writeByte(C2S_UPDATELOBBY);
		bs.writeByte(0);
		cNetChan.AddReliablePacketToSend(bs);


		// Goto the join lobby
		Menu_Net_GotoJoinLobby();
	}

	ShutdownLog();
}


///////////////////
// Parse a 'dropped' packet
void CClient::ParseDropped(CBytestream *bs)
{
    // Set the server error details

	// Not so much an error, but i message as to why i was dropped
	bServerError = true;
	strServerErrorMsg = Utf8String(bs->readString(256));

	// Stop any file downloads
	if (bDownloadingMap && cHttpDownloader)
		cHttpDownloader->CancelFileDownload(sMapDownloadName);
	getUdpFileDownloader()->reset();

	if (tLXOptions->bLogConvos)  {
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

// Server sent us some file
void CClient::ParseSendFile(CBytestream *bs)
{
	fLastFileRequestPacketReceived = tLX->fCurTime;
	if( getUdpFileDownloader()->receive(bs) )
	{
		if( getUdpFileDownloader()->getState() == CUdpFileDownloader::S_ERROR )
		{
			if( ! getUdpFileDownloader()->requestFilesPending() ) // More files to receive
				getUdpFileDownloader()->reset();
			return;
		};
		if( getUdpFileDownloader()->getFilename() == "dirt:" &&
			getUdpFileDownloader()->getState() == CUdpFileDownloader::S_FINISHED )
		{	// Parse a full dirt mask
			setPartialDirtUpdateCount(0);
			getPreviousDirtMap()->Clear();
			getPreviousDirtMap()->writeData( getUdpFileDownloader()->getData() );
			getPreviousDirtMap()->ResetPosToBegin();
			if( cMap )
				cMap->RecvDirtUpdate( getPreviousDirtMap() );
		}
		else
		if( getUdpFileDownloader()->getFilename().find( "dirt:" ) == 0 &&
			getUdpFileDownloader()->getState() == CUdpFileDownloader::S_FINISHED )
		{	// Parse a partial dirt mask
			int updateCount = atoi(getUdpFileDownloader()->getFilename().substr(strlen("dirt:")));
			if( updateCount != getPartialDirtUpdateCount() )
			{	// Request full dirt mask
				getUdpFileDownloader()->setDataToSend("dirt:", "");
				CBytestream bs;
				bs.writeByte(C2S_SENDFILE);
				getUdpFileDownloader()->send( &bs );	// Single packet
				cNetChan.AddReliablePacketToSend(bs);
				return;
			};
			setPartialDirtUpdateCount( getPartialDirtUpdateCount() + 1 );
			CBytestream bsDiff, bsNew;
			bsDiff.writeData( getUdpFileDownloader()->getData() );
			bsDiff.ResetPosToBegin();
			getPreviousDirtMap()->ResetPosToBegin();
			while( ! bsDiff.isPosAtEnd() )
				bsNew.writeBit( getPreviousDirtMap()->readBit() ^ bsDiff.readBit() );
			* getPreviousDirtMap() = bsNew;
			getPreviousDirtMap()->ResetPosToBegin();
			if( cMap )
				cMap->RecvDirtUpdate( getPreviousDirtMap() );
		}
		else
		if( CUdpFileDownloader::isPathValid( getUdpFileDownloader()->getFilename() ) &&
			! IsFileAvailable( getUdpFileDownloader()->getFilename() ) &&
			getUdpFileDownloader()->getState() == CUdpFileDownloader::S_FINISHED )
		{
			// Server sent us some file we don't have - okay, save it
			FILE * ff=OpenGameFile( getUdpFileDownloader()->getFilename(), "wb" );
			if( ff == NULL )
			{
				printf("CClient::ParseSendFile(): cannot write file %s\n", getUdpFileDownloader()->getFilename().c_str() );
				return;
			};
			fwrite( getUdpFileDownloader()->getData().c_str(), 1, getUdpFileDownloader()->getData().size(), ff );
			fclose(ff);
			// Put notice about downloaded file in chatbox
			CBytestream bs;
			bs.writeByte(TXT_NETWORK);
			bs.writeString( "Downloaded file \"" + getUdpFileDownloader()->getFilename() + 
								"\" size " + itoa( getUdpFileDownloader()->getData().size() ) );
			ParseText( &bs );
			if( getUdpFileDownloader()->getFilename().find("levels/") == 0 && 
					IsFileAvailable( "levels/" + tGameLobby.szMapName ) )
			{
				tGameLobby.bHaveMap = true;
				tGameLobby.szDecodedMapName = Menu_GetLevelName(tGameLobby.szMapName);

				if (cMap) delete cMap;

				cMap = new CMap;
				if (!cMap->Load("levels/" + tGameLobby.szMapName))  
				{
					printf("Could not load the downloaded map!\n");
					Disconnect();
					GotoNetMenu();
				};
				bJoin_Update = true;
				bHost_Update = true;
			};
			if( getUdpFileDownloader()->getFilename().find("skins/") == 0 )
			{
				// Loads skin from disk automatically on next frame
				bJoin_Update = true;
				bHost_Update = true;
			};
			if( ! tGameLobby.bHaveMod &&
				getUdpFileDownloader()->getFilename().find( tGameLobby.szModDir ) == 0 )
			{
				tGameLobby.bHaveMod = true;
			    FILE * fp = OpenGameFile(tGameLobby.szModDir + "/script.lgs", "rb");
			    if(!fp)
			        tGameLobby.bHaveMod = false;
    			else
			        fclose(fp);
				bJoin_Update = true;
				bHost_Update = true;
			};
		}
		else
		if( getUdpFileDownloader()->getFilename() == "STAT_ACK:" &&
			getUdpFileDownloader()->getFileInfo().size() > 0 &&
			! tGameLobby.bHaveMod &&
			getUdpFileDownloader()->getState() == CUdpFileDownloader::S_FINISHED )
		{
			// Got filenames list of mod dir - push "script.lgs" to the end of list to download all other data before
			uint f;
			for( f=0; f<getUdpFileDownloader()->getFileInfo().size(); f++ )
			{
				if( getUdpFileDownloader()->getFileInfo()[f].filename.find( tGameLobby.szModDir ) == 0 &&
					! IsFileAvailable( getUdpFileDownloader()->getFileInfo()[f].filename ) &&
					stringcaserfind( getUdpFileDownloader()->getFileInfo()[f].filename, "/script.lgs" ) != std::string::npos )
					getUdpFileDownloader()->requestFile( getUdpFileDownloader()->getFileInfo()[f].filename, true );
			};
			for( f=0; f<getUdpFileDownloader()->getFileInfo().size(); f++ )
			{
				if( getUdpFileDownloader()->getFileInfo()[f].filename.find( tGameLobby.szModDir ) == 0 &&
					! IsFileAvailable( getUdpFileDownloader()->getFileInfo()[f].filename ) &&
					stringcaserfind( getUdpFileDownloader()->getFileInfo()[f].filename, "/script.lgs" ) == std::string::npos )
					getUdpFileDownloader()->requestFile( getUdpFileDownloader()->getFileInfo()[f].filename, true );
			};
			getUdpFileDownloader()->setDataToSend( "", "" );	// Delay file request - half-second pause added automatically
		};
	};
	if( getUdpFileDownloader()->getState() == CUdpFileDownloader::S_RECEIVE )
	{
		// TODO: move this out here
		// Speed up download - server will send next packet when receives ping, or once in 0.5 seconds
		CBytestream bs;
		bs.writeByte(C2S_SENDFILE);
		getUdpFileDownloader()->sendPing( &bs );
		cNetChan.AddReliablePacketToSend(bs);
	}
	else
	{
		fLastFileRequestPacketReceived = tLX->fCurTime - 10.0f;	// Set timeout in past so we'll check out other data immediately
		fLastFileRequest = tLX->fCurTime + 0.3f;	// Small pause so many small files won't come garbled
		Timer(&Timer::DummyHandler, NULL, 300, true).startHeadless();	// Set timer so client will send the request
	};
};

