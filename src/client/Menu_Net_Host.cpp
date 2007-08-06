/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Net menu - Hosting
// Created 12/8/02
// Jason Boettcher


#include "LieroX.h"
#include "Graphics.h"
#include "CClient.h"
#include "CServer.h"
#include "Menu.h"
#include "GfxPrimitives.h"
#include "StringUtils.h"
#include "CWorm.h"
#include "Protocol.h"
#include "AuxLib.h"


/*
   The hosting menu is in two parts
     1) Selecting the players _we_ want to play as
     2) The lobby screen were other players can connect
*/



/*
==================================

		Player selection

==================================
*/



CGuiLayout	cHostPly;

// Player gui
enum {
	hs_Back=0,
	hs_Ok,
	hs_PlayerList,
	hs_Playing,
	hs_Servername,
	hs_MaxPlayers,
	hs_Register,
    hs_Password,
	hs_WelcomeMessage,
	hs_AllowWantsJoin,
	hs_AllowRemoteBots,
	hs_AllowNickChange
};

int iHumanPlayers = 0;


///////////////////
// Initialize the host menu
int Menu_Net_HostInitialize(void)
{
	iNetMode = net_host;
	iHostType = 0;

	// Player gui layout
	cHostPly.Shutdown();
	cHostPly.Initialize();

    cHostPly.Add( new CLabel("Player settings",tLX->clHeading), -1, 350,140,0,0);
	cHostPly.Add( new CLabel("Server settings",tLX->clHeading), -1, 30, 140,0,0);
	cHostPly.Add( new CButton(BUT_BACK, tMenu->bmpButtons), hs_Back,	25, 440,50, 15);
	cHostPly.Add( new CButton(BUT_OK, tMenu->bmpButtons),   hs_Ok,			585,440,30, 15);

	cHostPly.Add( new CListview(),							hs_PlayerList,	360,160,200,140);
	cHostPly.Add( new CListview(),							hs_Playing,		360,320,200,100);
	cHostPly.Add( new CLabel("Server name",	tLX->clNormalLabel),		-1,			40, 178,0,  0);
	cHostPly.Add( new CTextbox(),							hs_Servername,	175,175,140,tLX->cFont.GetHeight());
	cHostPly.Add( new CLabel("Max Players",	tLX->clNormalLabel),		-1,			40,	208,0,  0);
	cHostPly.Add( new CTextbox(),							hs_MaxPlayers,	175,205,140,tLX->cFont.GetHeight());
    //cHostPly.Add( new CLabel("Password",	tLX->clNormalLabel),	    -1,			40, 238,0,  0);
	//cHostPly.Add( new CTextbox(),		                    hs_Password,	175,235,140,tLX->cFont.GetHeight());
	cHostPly.Add( new CLabel("Welcome Message",	tLX->clNormalLabel),		-1,			40, 238,0,  0);
	cHostPly.Add( new CTextbox(),							hs_WelcomeMessage, 175,235,140,tLX->cFont.GetHeight());
	cHostPly.Add( new CLabel("Register Server",	tLX->clNormalLabel),	-1,			153, 268,0,  0);
	cHostPly.Add( new CCheckbox(0),		                    hs_Register,	270,265,17, 17);
	cHostPly.Add( new CLabel("Allow \"Wants to join\" Messages",	tLX->clNormalLabel),-1,	35, 298,0,  0);
	cHostPly.Add( new CCheckbox(0),		                    hs_AllowWantsJoin,	270,295,17, 17);
	cHostPly.Add( new CLabel("Allow Bots in Server",			tLX->clNormalLabel),-1,	115, 328,0,  0);
	cHostPly.Add( new CCheckbox(0),		                    hs_AllowRemoteBots,	270,325,17, 17);
	cHostPly.Add( new CLabel("Allow Nick Changing",			tLX->clNormalLabel),-1,	115, 358,0,  0);
	cHostPly.Add( new CCheckbox(0),		                    hs_AllowNickChange,	270,355,17, 17);

	cHostPly.SendMessage(hs_Playing,		LVM_SETOLDSTYLE, (DWORD)0, 0);
	cHostPly.SendMessage(hs_PlayerList,		LVM_SETOLDSTYLE, (DWORD)0, 0);

	cHostPly.SendMessage(hs_Servername,TXM_SETMAX,32,0);
	//cHostPly.SendMessage(hs_Password,TXM_SETMAX,32,0);
	cHostPly.SendMessage(hs_MaxPlayers,TXM_SETMAX,6,0);
	cHostPly.SendMessage(hs_WelcomeMessage,TXM_SETMAX,128,0);

	// Use previous settings
	cHostPly.SendMessage( hs_MaxPlayers, TXS_SETTEXT, itoa(tLXOptions->tGameinfo.iMaxPlayers), 0);
	cHostPly.SendMessage( hs_Servername, TXS_SETTEXT, tLXOptions->tGameinfo.sServerName, 0);
	cHostPly.SendMessage( hs_WelcomeMessage, TXS_SETTEXT, tLXOptions->tGameinfo.sWelcomeMessage, 0);
	cHostPly.SendMessage( hs_Register,   CKM_SETCHECK, tLXOptions->tGameinfo.bRegServer, 0);
	cHostPly.SendMessage( hs_AllowWantsJoin,   CKM_SETCHECK, tLXOptions->tGameinfo.bAllowWantsJoinMsg, 0);
	cHostPly.SendMessage( hs_AllowRemoteBots,   CKM_SETCHECK, tLXOptions->tGameinfo.bAllowRemoteBots, 0);
	cHostPly.SendMessage( hs_AllowNickChange,   CKM_SETCHECK, tLXOptions->tGameinfo.bAllowNickChange, 0);
    //cHostPly.SendMessage( hs_Password,   TXS_SETTEXT, tLXOptions->tGameinfo.szPassword, 0 );

	// Add columns
	cHostPly.SendMessage( hs_PlayerList,   LVS_ADDCOLUMN, "Players",22);
	cHostPly.SendMessage( hs_PlayerList,   LVS_ADDCOLUMN, "",60);
	cHostPly.SendMessage( hs_Playing,      LVS_ADDCOLUMN, "Playing",22);
	cHostPly.SendMessage( hs_Playing,      LVS_ADDCOLUMN, "",60);

	// Add players to the list
	profile_t *p = GetProfiles();
	for(;p;p=p->tNext) {
		/*if(p->iType == PRF_COMPUTER)
			continue;*/

		cHostPly.SendMessage( hs_PlayerList, LVS_ADDITEM, "", p->iID);
		cHostPly.SendMessage( hs_PlayerList, LVS_ADDSUBITEM, (DWORD)p->bmpWorm, LVS_IMAGE );
		cHostPly.SendMessage( hs_PlayerList, LVS_ADDSUBITEM, p->sName, LVS_TEXT);
	}

	iHumanPlayers = 0;

	return true;
}


///////////////////
// Shutdown the host menu
void Menu_Net_HostShutdown(void)
{
	switch(iHostType) {

		// Player selection
		case 0:
			Menu_Net_HostPlyShutdown();
			break;

		// Lobby
		case 1:
			Menu_Net_HostLobbyShutdown();
			break;
	}
}


///////////////////
// The net host menu frame
void Menu_Net_HostFrame(int mouse)
{
	switch(iHostType) {

		// Player selection
		case 0:
			Menu_Net_HostPlyFrame(mouse);
			break;

		// Lobby
		case 1:
			Menu_Net_HostLobbyFrame(mouse);
			break;
	}
}

///////////////////
// Player selection frame
void Menu_Net_HostPlyFrame(int mouse)
{
	gui_event_t *ev = NULL;
	CListview	*lv, *lv2;
	profile_t	*ply;

	// Process & Draw the gui
#ifdef WITH_MEDIAPLAYER
	if (!cMediaPlayer.GetDrawPlayer())
#endif
		ev = cHostPly.Process();
	cHostPly.Draw( tMenu->bmpScreen );


	// Process any events
	if(ev) {

		switch(ev->iControlID) {

			// Back
			case hs_Back:
				if(ev->iEventMsg == BTN_MOUSEUP) {

					// Click!
					PlaySoundSample(sfxGeneral.smpClick);

					// Shutdown
					Menu_Net_HostShutdown();

					// Back to net menu
					Menu_MainInitialize();
				}
				break;


			// Player list
			case hs_PlayerList:
				if(ev->iEventMsg == LV_DOUBLECLK || ev->iEventMsg == LV_RIGHTCLK) {
					// Add the item to the players list
					lv = (CListview *)cHostPly.getWidget(hs_PlayerList);
					lv2 = (CListview *)cHostPly.getWidget(hs_Playing);
					int index = lv->getCurIndex();

					// Make sure there is 0-MAX_PLAYERS players in the list
					if(lv2->getItemCount() < MAX_PLAYERS) {

						// Get the profile
						ply = FindProfile(index);

						if(ply) {
							if (ply->iType == PRF_COMPUTER || iHumanPlayers < 1)  {
								lv2->AddItem("",index,tLX->clListView);
								lv2->AddSubitem(LVS_IMAGE, "", ply->bmpWorm, NULL);
								lv2->AddSubitem(LVS_TEXT, ply->sName, NULL, NULL);
								if (ply->iType == PRF_HUMAN)
									iHumanPlayers++;

								// Remove the item from the player list
								lv->RemoveItem(index);
							}
						}

					}
				}
				break;


			// Playing list
			case hs_Playing:
				if(ev->iEventMsg == LV_DOUBLECLK || ev->iEventMsg == LV_RIGHTCLK) {
					// Add the item to the players list
					lv = (CListview *)cHostPly.getWidget(hs_Playing);
					lv2 = (CListview *)cHostPly.getWidget(hs_PlayerList);
					int index = lv->getCurIndex();

					// Remove the item from the list
					lv->RemoveItem(index);

					ply = FindProfile(index);

					if(ply) {
						lv2->AddItem("",index,tLX->clListView);
						lv2->AddSubitem(LVS_IMAGE, "", ply->bmpWorm, NULL);
						lv2->AddSubitem(LVS_TEXT, ply->sName, NULL, NULL);
						if (ply->iType == PRF_HUMAN)
							iHumanPlayers--;
					}
				}
				break;

			// Ok
			case hs_Ok:
				if(ev->iEventMsg == BTN_MOUSEUP) {

					lv = (CListview *)cHostPly.getWidget(hs_Playing);

					// Make sure there is 1-7 players in the list
					if (lv->getItemCount() > 0 && lv->getItemCount() <= MAX_PLAYERS - 1) {

						tGameInfo.iNumPlayers = lv->getItemCount();

						// Fill in the game structure
						lv_item_t* item;
						int count=0;

						int i=0;

						// Add the human players to the list
						for(item = lv->getItems(); item != NULL; item = item->tNext) {
							if(item->iIndex < 0)
								continue;

							profile_t *ply = FindProfile(item->iIndex);

							if(ply != NULL && ply->iType == PRF_HUMAN)  {
								// Max two humans
								if(i > 2)
									break;

								tGameInfo.cPlayers[count++] = ply;
								i++;
							}

						}

						// Add the unhuman players to the list
						for(item = lv->getItems(); item != NULL; item = item->tNext) {
							if(item->iIndex < 0)
								continue;

							profile_t *ply = FindProfile(item->iIndex);

							if(ply != NULL && ply->iType != PRF_HUMAN)  {
								tGameInfo.cPlayers[count++] = ply;
							}
						}


						// Get the server name
						cHostPly.SendMessage( hs_Servername, TXS_GETTEXT, &tGameInfo.sServername, 0);
						cHostPly.SendMessage( hs_WelcomeMessage, TXS_GETTEXT, &tGameInfo.sWelcomeMessage, 0);
                        //cHostPly.SendMessage( hs_Password, TXS_GETTEXT, &tGameInfo.sPassword, 0);

						// Save the info
						static std::string buf;
						cHostPly.SendMessage( hs_Servername, TXS_GETTEXT, &tLXOptions->tGameinfo.sServerName, 0);
						cHostPly.SendMessage( hs_WelcomeMessage, TXS_GETTEXT, &tLXOptions->tGameinfo.sWelcomeMessage, 0);
                        //cHostPly.SendMessage( hs_Password, TXS_GETTEXT, &tLXOptions->tGameinfo.szPassword, 0);
						cHostPly.SendMessage( hs_MaxPlayers, TXS_GETTEXT, &buf, 0);

						tLXOptions->tGameinfo.iMaxPlayers = atoi(buf);
						// At least 2 players, and max MAX_PLAYERS
						tLXOptions->tGameinfo.iMaxPlayers = MAX(tLXOptions->tGameinfo.iMaxPlayers,2);
						tLXOptions->tGameinfo.iMaxPlayers = MIN(tLXOptions->tGameinfo.iMaxPlayers,MAX_PLAYERS);
						tLXOptions->tGameinfo.bRegServer =  cHostPly.SendMessage( hs_Register, CKM_GETCHECK, (DWORD)0, 0) != 0;
						tLXOptions->tGameinfo.bAllowWantsJoinMsg = cHostPly.SendMessage( hs_AllowWantsJoin, CKM_GETCHECK, (DWORD)0, 0) != 0;
						tLXOptions->tGameinfo.bAllowRemoteBots = cHostPly.SendMessage( hs_AllowRemoteBots, CKM_GETCHECK, (DWORD)0, 0) != 0;
						tLXOptions->tGameinfo.bAllowNickChange = cHostPly.SendMessage( hs_AllowNickChange, CKM_GETCHECK, (DWORD)0, 0) != 0;

						cHostPly.Shutdown();

						// Click
						PlaySoundSample(sfxGeneral.smpClick);

						iHumanPlayers = 0;

						// Start the lobby
						Menu_Net_HostLobbyInitialize();
					}
				}
				break;
		}
	}


	// Draw the mouse
	DrawCursor(tMenu->bmpScreen);
}

//////////////
// Shutdown
void Menu_Net_HostPlyShutdown(void)
{
	cHostPly.Shutdown();
}




/*
==================================

		Hosting Lobby

==================================
*/


// Lobby gui
enum {
	hl_Back=0,
	hl_Start,
	hl_ChatText,
	hl_ChatList,
	hl_LevelList,
	hl_PlayerList,
	hl_Lives,
	hl_MaxKills,
	hl_ModName,
	hl_Gametype,
	hl_GameSettings,
    hl_WeaponOptions,
    hl_PopupMenu,
	hl_Banned,
	hl_ServerSettings
};

bool		bHostGameSettings = false;
bool		bHostWeaponRest = false;
bool		bBanList = false;
bool		bServerSettings = false;
CGuiLayout	cHostLobby;
int			iSpeaking = 0;
int         g_nLobbyWorm = -1;
bool		bHost_Update = false;


///////////////////
// Initialize the hosting lobby
int Menu_Net_HostLobbyInitialize(void)
{
	tGameInfo.iGameType = GME_HOST;
	tGameInfo.bTournament = tLXOptions->tGameinfo.bTournament;
	iNetMode = net_host;
	iHostType = 1;
	bHostGameSettings = false;
    bHostWeaponRest = false;
    iSpeaking = -1;

    // Draw the lobby
	Menu_Net_HostLobbyDraw();

	// Fill in some game details
	tGameInfo.iLoadingTimes = tLXOptions->tGameinfo.iLoadingTime;
	tGameInfo.iLives = tLXOptions->tGameinfo.iLives;
	tGameInfo.iKillLimit = tLXOptions->tGameinfo.iKillLimit;
	tGameInfo.iBonusesOn = tLXOptions->tGameinfo.iBonusesOn;
	tGameInfo.iShowBonusName = tLXOptions->tGameinfo.iShowBonusName;
    tGameInfo.iGameMode = tLXOptions->tGameinfo.nGameType;

	cClient->Shutdown();
    cClient->Clear();


	// Start the server
	if(!cServer->StartServer( tGameInfo.sServername, tLXOptions->iNetworkPort, tLXOptions->tGameinfo.iMaxPlayers, tLXOptions->tGameinfo.bRegServer )) {
		// Crappy
		printf("Server wouldn't start\n");
		cHostLobby.Shutdown();
		return false;
	}


	// Lets connect me to the server
	if(!cClient->Initialize()) {
		// Crappy
		printf("Client wouldn't initialize\n");
		cHostLobby.Shutdown();
		return false;
	}

	cClient->Connect("127.0.0.1");

	cClient->getChatbox()->setWidth(590);

	// Set up the server's lobby details
	game_lobby_t *gl = cServer->getLobby();
	gl->nSet = true;
	gl->nGameMode = tLXOptions->tGameinfo.nGameType;
	gl->nLives = tLXOptions->tGameinfo.iLives;
	gl->nMaxKills = tLXOptions->tGameinfo.iKillLimit;
	gl->nLoadingTime = tLXOptions->tGameinfo.iLoadingTime;
    gl->nBonuses = tLXOptions->tGameinfo.iBonusesOn;
	gl->bTournament = tLXOptions->tGameinfo.bTournament;


    // Create the GUI
    Menu_Net_HostLobbyCreateGui();

	return true;
}


///////////////////
// Draw the lobby screen
void Menu_Net_HostLobbyDraw(void)
{
    // Create the buffer
	DrawImage(tMenu->bmpBuffer,tMenu->bmpMainBack_common,0,0);
	if (tMenu->tFrontendInfo.bPageBoxes)
		Menu_DrawBox(tMenu->bmpBuffer, 5,5, 635, 475);
	Menu_DrawBox(tMenu->bmpBuffer, 460,29, 593, 130);
    DrawImageAdv(tMenu->bmpBuffer, tMenu->bmpMainBack_common, 281,0, 281,0, 79,20);

    tLX->cFont.DrawCentre(tMenu->bmpBuffer, 320, -1, tLX->clNormalLabel, "[  Lobby  ]");

	// Chat box
    DrawRectFill(tMenu->bmpBuffer, 16, 270, 624, 417, tLX->clChatBoxBackground);

	Menu_RedrawMouse(true);
}


///////////////////
// Create the lobby gui
void Menu_Net_HostLobbyCreateGui(void)
{   
    // Lobby gui layout
	cHostLobby.Shutdown();
	cHostLobby.Initialize();

	cHostLobby.Add( new CButton(BUT_LEAVE, tMenu->bmpButtons),hl_Back,	15,  450, 60,  15);
	cHostLobby.Add( new CButton(BUT_START, tMenu->bmpButtons),hl_Start,	560, 450, 60,  15);
	cHostLobby.Add( new CButton(BUT_BANNED, tMenu->bmpButtons),hl_Banned,	450, 450, 90,  15);
	cHostLobby.Add( new CButton(BUT_SERVERSETTINGS, tMenu->bmpButtons),hl_ServerSettings,	250, 450, 190, 15);
	cHostLobby.Add( new CTextbox(),							  hl_ChatText, 15,  421, 610, tLX->cFont.GetHeight());
    cHostLobby.Add( new CListview(),                          hl_ChatList, 15,  268, 610, 150);

	cHostLobby.Add( new CButton(BUT_GAMESETTINGS, tMenu->bmpButtons), hl_GameSettings, 360, 210, 170,15);
    cHostLobby.Add( new CButton(BUT_WEAPONOPTIONS,tMenu->bmpButtons), hl_WeaponOptions,360, 235, 185,15);

	cHostLobby.Add( new CLabel("Mod",tLX->clNormalLabel),	    -1,         360, 180, 0,   0);
	cHostLobby.Add( new CCombobox(),				hl_ModName,    440, 179, 170, 17);
	cHostLobby.Add( new CLabel("Game type",tLX->clNormalLabel),	-1,         360, 158, 0,   0);
	cHostLobby.Add( new CCombobox(),				hl_Gametype,   440, 157, 170, 17);
    cHostLobby.Add( new CLabel("Level",tLX->clNormalLabel),	    -1,         360, 136, 0,   0);
    cHostLobby.Add( new CCombobox(),				hl_LevelList,  440, 135, 170, 17);
	cHostLobby.Add( new CListview(),				hl_PlayerList, 15, 15, 325, 220);

	cHostLobby.SendMessage(hl_ChatText,TXM_SETMAX,64,0);

	//cHostLobby.SendMessage(hl_ChatList,		LVM_SETOLDSTYLE, 0, 0);

	// Fill in the game details
	cHostLobby.SendMessage(hl_Gametype,    CBS_ADDITEM, "Deathmatch", GMT_DEATHMATCH);
	cHostLobby.SendMessage(hl_Gametype,    CBS_ADDITEM, "Team Deathmatch", GMT_TEAMDEATH);
	cHostLobby.SendMessage(hl_Gametype,    CBS_ADDITEM, "Tag", GMT_TAG);

	// Fill in the mod list
	Menu_Local_FillModList( (CCombobox *)cHostLobby.getWidget(hl_ModName));

	// Fill in the levels list
	Menu_FillLevelList( (CCombobox *)cHostLobby.getWidget(hl_LevelList), false);
	Menu_HostShowMinimap();

    // Don't show chat box selection
    CListview *lv = (CListview *)cHostLobby.getWidget(hl_ChatList);
    lv->setShowSelect(false);

    game_lobby_t *gl = cServer->getLobby();
    cHostLobby.SendMessage(hl_LevelList, CBS_GETCURSINDEX, &gl->szMapName, 0);
    cHostLobby.SendMessage(hl_ModName,	 CBS_GETCURNAME, &gl->szModName, 0);
    cHostLobby.SendMessage(hl_ModName,	 CBS_GETCURSINDEX, &gl->szModDir, 0);
    cHostLobby.SendMessage(hl_Gametype,  CBM_SETCURINDEX, gl->nGameMode, 0);

	// Setup the player list
	CListview *player_list = (CListview *)cHostLobby.getWidget(hl_PlayerList);
	if (player_list)  {
		player_list->setShowSelect(false);
		player_list->setOldStyle(true);
		player_list->AddColumn("Players", gfxGUI.bmpCommandBtn->w / 2 + 2, tLX->clHeading);  // Command button/Player label
		player_list->AddColumn("", tMenu->bmpLobbyReady->w + 2);  // Lobby ready
		player_list->AddColumn("", 30);  // Skin
		player_list->AddColumn("", 200 - gfxGame.bmpTeamColours[0]->w); // Name
		player_list->AddColumn("", gfxGame.bmpTeamColours[0]->w + 10);  // Team
		player_list->AddColumn("", -1); // Ping
	}

	iSpeaking = 0; // The first player always speaks
}

//////////////////////
// Get the content of the chatbox
std::string Menu_Net_HostLobbyGetText(void)
{
	static std::string buf;
	cHostLobby.SendMessage(hl_ChatText, TXS_GETTEXT, &buf, 256);
	return buf;
}

///////////////////
// Go straight to the lobby, without clearing the server & client
void Menu_Net_HostGotoLobby(void)
{
	tGameInfo.iGameType = GME_HOST;
	iNetMode = net_host;
	iHostType = 1;
	bHostGameSettings = false;
    bHostWeaponRest = false;
    iSpeaking = 0;

    // Draw the lobby
	Menu_Net_HostLobbyDraw();

	// Fill in some game details
	tGameInfo.iLoadingTimes = tLXOptions->tGameinfo.iLoadingTime;
	tGameInfo.iLives = tLXOptions->tGameinfo.iLives;
	tGameInfo.iKillLimit = tLXOptions->tGameinfo.iKillLimit;
	tGameInfo.iBonusesOn = tLXOptions->tGameinfo.iBonusesOn;
	tGameInfo.iShowBonusName = tLXOptions->tGameinfo.iShowBonusName;
    tGameInfo.iGameMode = tLXOptions->tGameinfo.nGameType;
	tGameInfo.bTournament = tLXOptions->tGameinfo.bTournament;

	cClient->getChatbox()->setWidth(590);

	// Set up the server's lobby details
	game_lobby_t *gl = cServer->getLobby();
	gl->nSet = true;
	gl->nGameMode = tLXOptions->tGameinfo.nGameType;
	gl->nLives = tLXOptions->tGameinfo.iLives;
	gl->nMaxKills = tLXOptions->tGameinfo.iKillLimit;
	gl->nLoadingTime = tLXOptions->tGameinfo.iLoadingTime;
	gl->bTournament = tLXOptions->tGameinfo.bTournament;

    // Create the GUI
    Menu_Net_HostLobbyCreateGui();

	// Add the chat to the chatbox
	CListview *lv = (CListview *)cHostLobby.getWidget(hl_ChatList);
	if (lv)  {
		CChatBox *Chatbox = cClient->getChatbox();
		lines_iterator it = Chatbox->At(Chatbox->getNumLines()-256); // If there's more than 256 messages, we start not from beginning but from end()-256
		int id = (lv->getLastItem() && lv->getItems()) ? lv->getLastItem()->iIndex + 1 : 0;

		// Copy the chat text
		for (; it != Chatbox->End(); it++)  {
			if (it->iColour == tLX->clChatText)  {  // Add only chat messages
				lv->AddItem("", id, it->iColour);
				lv->AddSubitem(LVS_TEXT, it->strLine, NULL, NULL);
				id++;
			}
		}

		lv->scrollLast();
		lv->setShowSelect(false);
	}

	cServer->UpdateGameLobby();
}


///////////////////
// Host lobby frame
void Menu_Net_HostLobbyFrame(int mouse)
{
	gui_event_t *ev = NULL;

	// Process the server & client frames
	cServer->Frame();
	cClient->Frame();


    // Game settings
	if(bHostGameSettings) {
		if(Menu_GameSettings_Frame()) {
			Menu_Net_HostLobbyDraw();
			Menu_HostShowMinimap();

			bHostGameSettings = false;

			// Move the settings into the server
			cServer->getLobby()->nLives = tGameInfo.iLives;
			cServer->getLobby()->nMaxKills = tGameInfo.iKillLimit;
			cServer->getLobby()->nLoadingTime = tGameInfo.iLoadingTimes;
            cServer->getLobby()->nBonuses = tGameInfo.iBonusesOn;
			cServer->getLobby()->bTournament = tGameInfo.bTournament;
			cServer->UpdateGameLobby();
		}
		return;
	}


    // Weapons Restrictions
    if(bHostWeaponRest) {
		if(Menu_WeaponsRestrictions_Frame()) {
			Menu_Net_HostLobbyDraw();
			Menu_HostShowMinimap();

			bHostWeaponRest = false;
		}
		return;
	}

    // Ban List
    if(bBanList) {
		if(Menu_BanList_Frame()) {
			Menu_Net_HostLobbyDraw();
			Menu_HostShowMinimap();

			bBanList = false;
		}
		return;
	}

    // Server Settings
    if(bServerSettings) {
		if(Menu_ServerSettings_Frame()) {
			Menu_Net_HostLobbyDraw();
			Menu_HostShowMinimap();

			bServerSettings = false;
		}
		return;
	}

	// Reload the level and mod list when the user switches back to game
	// Do not reaload when any of the dialogs is open
	if (bActivated)  {

		// Get the mod name
		cb_item_t *it = (cb_item_t *)cHostLobby.SendMessage(hl_ModName,CBM_GETCURITEM,(DWORD)0,0);
		if(it)
			tLXOptions->tGameinfo.szModName = it->sIndex;

		// Fill in the mod list
		Menu_Local_FillModList( (CCombobox *)cHostLobby.getWidget(hl_ModName));

		// Fill in the levels list
		cHostLobby.SendMessage(hl_LevelList,CBS_GETCURSINDEX, &tLXOptions->tGameinfo.sMapFilename, 0);
		std::string oldmap = tLXOptions->tGameinfo.sMapFilename;
		Menu_FillLevelList( (CCombobox *)cHostLobby.getWidget(hl_LevelList), false);

		// Redraw the minimap if the level has changed
		if (stringcasecmp(oldmap, tLXOptions->tGameinfo.sMapFilename))
			Menu_HostShowMinimap();
	}


    // Add chat to the listbox
	CListview *lv = (CListview *)cHostLobby.getWidget(hl_ChatList);
	line_t *l = NULL;
	while((l = cClient->getChatbox()->GetNewLine()) != NULL) {

        if(lv->getLastItem() != NULL)
            lv->AddItem("", lv->getLastItem()->iIndex+1, l->iColour);
        else
            lv->AddItem("", 0, l->iColour);
        lv->AddSubitem(LVS_TEXT, l->strLine, NULL, NULL);
        lv->setShowSelect(false);

        // If there are too many lines, remove the top one
        if(lv->getItemCount() > 256) {
            if(lv->getItems())
                lv->RemoveItem(lv->getItems()->iIndex);
        }

        lv->scrollLast();
	}



    // Clear the chat box & player list & around combo boxes
    //Menu_redrawBufferRect(25, 315, 590, 100);
    Menu_redrawBufferRect(15, 20,  335, 225);
	Menu_redrawBufferRect(540, 150,  100, 200);


    // Draw the host lobby details
	Menu_HostDrawLobby(tMenu->bmpScreen);

	// Process & Draw the gui
#ifdef WITH_MEDIAPLAYER
	if (!cMediaPlayer.GetDrawPlayer())
#endif
		ev = cHostLobby.Process();
	cHostLobby.Draw( tMenu->bmpScreen );



	// Process any events
	if(ev) {

		switch(ev->iControlID) {

			// Back
			case hl_Back:
				if(ev->iEventMsg == BTN_MOUSEUP) {
					// Click!
					PlaySoundSample(sfxGeneral.smpClick);

					// De-register the server
					Menu_Net_HostDeregister();

					// Shutdown
					Menu_Net_HostLobbyShutdown();

					// Back to main net menu
					Menu_NetInitialize();
				}
				break;


			// Chat textbox
			case hl_ChatText:
				if(ev->iEventMsg == TXT_ENTER && iSpeaking >= 0) {
					// Send the msg to the server

					// Get the text
					static std::string buf;
					cHostLobby.SendMessage(hl_ChatText, TXS_GETTEXT, &buf, 0);

                    // Don't send empty messages
                    if(buf.length() == 0)
                        break;

					// Clear the text box
					cHostLobby.SendMessage(hl_ChatText, TXS_SETTEXT, "", 0);

					// Get name
					std::string text;
					CWorm *rw = cClient->getRemoteWorms() + iSpeaking;
					if(!strincludes(buf,"/me"))
						text = rw->getName() + ": " + buf;
					else
						text = replacemax(buf,"/me",rw->getName(),text,2);

					text = OldLxCompatibleString(text);

//					cServer->SendGlobalText(text,TXT_CHAT);
					// Allows host to use /setname etc in lobby
					cClient->SendText(text);
				}
				break;

			// Level change
			case hl_LevelList:
				if(ev->iEventMsg == CMB_CHANGED) {
					Menu_HostShowMinimap();

					cHostLobby.SendMessage(hl_LevelList, CBS_GETCURSINDEX, &cServer->getLobby()->szMapName, 0);
					cServer->UpdateGameLobby();
				}
				break;

            // Mod change
            case hl_ModName:
                if(ev->iEventMsg == CMB_CHANGED) {
                    cHostLobby.SendMessage(hl_ModName, CBS_GETCURNAME, &cServer->getLobby()->szModName, 0);
                    cHostLobby.SendMessage(hl_ModName, CBS_GETCURSINDEX, &cServer->getLobby()->szModDir, 0);
					cServer->UpdateGameLobby();
                }
                break;

			// Game type change
			case hl_Gametype:
				if(ev->iEventMsg == CMB_CHANGED) {
					cServer->getLobby()->nGameMode = cHostLobby.SendMessage(hl_Gametype, CBM_GETCURINDEX, (DWORD)0, 0);
					bHost_Update = true;
					cServer->UpdateGameLobby();
				}
				break;

			// Lives change
			case hl_Lives:
				if(ev->iEventMsg == TXT_CHANGE) {
					static std::string buf;
					cHostLobby.SendMessage(hl_Lives, TXS_GETTEXT, &buf, 0);
					if(buf != "")
						cServer->getLobby()->nLives = atoi(buf);
					else
						cServer->getLobby()->nLives = -2;

					cServer->UpdateGameLobby();
				}
				break;


			// Max Kills
			case hl_MaxKills:
				if(ev->iEventMsg == TXT_CHANGE) {
					static std::string buf;
					cHostLobby.SendMessage(hl_MaxKills, TXS_GETTEXT, &buf, 0);
					if(buf != "")
						cServer->getLobby()->nMaxKills = atoi(buf);
					else
						cServer->getLobby()->nMaxKills = -2;

					cServer->UpdateGameLobby();
				}
				break;

			// Game Settings
			case hl_GameSettings:
				if(ev->iEventMsg == BTN_MOUSEUP) {

					// Draw the lobby screen to the buffer
					cHostLobby.Draw( tMenu->bmpBuffer);
					Menu_HostDrawLobby(tMenu->bmpBuffer);

					Menu_GameSettings();
					bHostGameSettings = true;
				}
				break;

            // Weapon restrictions
            case hl_WeaponOptions:
                if(ev->iEventMsg == BTN_MOUSEUP) {

                    // Draw the lobby screen to the buffer
					cHostLobby.Draw( tMenu->bmpBuffer);
					Menu_HostDrawLobby(tMenu->bmpBuffer);

                    cb_item_t *it = (cb_item_t *)cHostLobby.SendMessage(hl_ModName,CBM_GETCURITEM,(DWORD)0,0);
                    if(it) {
		                bHostWeaponRest = true;
					    Menu_WeaponsRestrictions(it->sIndex);
                    }
                }
                break;

			// Ban list
			case hl_Banned:
				if(ev->iEventMsg == BTN_MOUSEUP)   {
                    // Draw the lobby screen to the buffer
					cHostLobby.Draw( tMenu->bmpBuffer);
					Menu_HostDrawLobby(tMenu->bmpBuffer);

	                bBanList = true;
				    Menu_BanList();
				}
				break;

			// Server settings
			case hl_ServerSettings:
				if(ev->iEventMsg == BTN_MOUSEUP)   {
                    // Draw the lobby screen to the buffer
					cHostLobby.Draw( tMenu->bmpBuffer);
					Menu_HostDrawLobby(tMenu->bmpBuffer);

	                bServerSettings = true;
				    Menu_ServerSettings();
				}
				break;


			// Start the game
			case hl_Start:
				if(ev->iEventMsg == BTN_MOUSEUP) {

					// Get the mod
					cb_item_t *it = (cb_item_t *)cHostLobby.SendMessage(hl_ModName,CBM_GETCURITEM,(DWORD)0,0);
                    if(it) {
		                tGameInfo.sModName = it->sIndex;
						tGameInfo.sModDir = it->sIndex;
                        tLXOptions->tGameinfo.szModName = it->sIndex;
                    }

                    // Get the game type
                    tGameInfo.iGameMode = cHostLobby.SendMessage(hl_Gametype, CBM_GETCURINDEX, (DWORD)0, 0);
                    tLXOptions->tGameinfo.nGameType = tGameInfo.iGameMode;

					// Get the map name
					cHostLobby.SendMessage(hl_LevelList, CBS_GETCURSINDEX, &tGameInfo.sMapname, 0);
					// Save the current level in the options
					cHostLobby.SendMessage(hl_LevelList, CBS_GETCURSINDEX, &tLXOptions->tGameinfo.sMapFilename, 0);
					cHostLobby.Shutdown();

                    // Setup the client
                    cClient->SetupViewports();

					// Start the game
					cServer->StartGame();

					// Leave the frontend
					*bGame = true;
					tMenu->iMenuRunning = false;
					tGameInfo.iGameType = GME_HOST;

					return;
				}
				break;

			// Player list
			case hl_PlayerList:
				if (ev->iEventMsg == LV_WIDGETEVENT)  {
					ev = ((CListview *)ev->cWidget)->getWidgetEvent();

					// Click on the command button
					if (ev->cWidget->getType() == wid_Button && ev->iEventMsg == BTN_MOUSEUP)  {
						// Remove old popup menu
						cHostLobby.removeWidget(hl_PopupMenu);

						g_nLobbyWorm = ev->cWidget->getID();  // Widget ID is the same as player ID
						if (g_nLobbyWorm == 0) // Not for host
							break;

						CClient *remote_cl = cServer->getClient(g_nLobbyWorm);
						mouse_t *Mouse = GetMouse();

						cHostLobby.Add( new CMenu(Mouse->X, Mouse->Y), hl_PopupMenu, 0,0, 640,480 );
						cHostLobby.SendMessage( hl_PopupMenu, MNS_ADDITEM, "Kick player", 0 );
						cHostLobby.SendMessage( hl_PopupMenu, MNS_ADDITEM, "Ban player", 1 );
						if (remote_cl)  {
							if (remote_cl->getMuted())
								cHostLobby.SendMessage( hl_PopupMenu, MNS_ADDITEM, "Unmute player",2 );
							else
								cHostLobby.SendMessage( hl_PopupMenu, MNS_ADDITEM, "Mute player",2 );
						}
						cHostLobby.SendMessage( hl_PopupMenu, MNS_ADDITEM, "Authorise player", 3 );

					// Click on the team mark
					} else if (ev->cWidget->getType() == wid_Image && ev->iEventMsg == IMG_CLICK)  {
						int id = ev->cWidget->getID();
						if (id >= MAX_WORMS || id < 0 || !cServer->getWorms()) // Safety
							break;

						// Set the team
						CWorm *w = cServer->getWorms() + id;
						w->setTeam((w->getTeam() + 1) % 4);
						w->getLobby()->iTeam = w->getTeam();

						cServer->SendWormLobbyUpdate();  // Update
						bHost_Update = true;
					}
				}
				break;

            // Popup menu
            case hl_PopupMenu:
                switch( ev->iEventMsg ) {
                	
                    // Kick the player
                    case MNU_USER+0:
                        if( g_nLobbyWorm > 0 )
                            cServer->kickWorm( g_nLobbyWorm );
                        break;

					// Ban the player
					case MNU_USER+1:
						if ( g_nLobbyWorm > 0 )
							cServer->banWorm( g_nLobbyWorm );
						break;

					// Mute/unmute
					case MNU_USER+2:
						if ( g_nLobbyWorm > 0 )  {
							CClient *remote_cl = cServer->getClient(g_nLobbyWorm);
							if (remote_cl)  {
								if (remote_cl->getMuted())
									cServer->unmuteWorm(g_nLobbyWorm);
								else
									cServer->muteWorm(g_nLobbyWorm);
							}
						}
						break;

					// Authorize
					case MNU_USER+3:
							CClient *remote_cl = cServer->getClient(g_nLobbyWorm);
							if (remote_cl) 
								remote_cl->getRights()->Everything();
						break;
                }

                // Remove the menu widget
                cHostLobby.SendMessage( hl_PopupMenu, MNM_REDRAWBUFFER, (DWORD)0, 0);
                cHostLobby.removeWidget(hl_PopupMenu);
                break;
		}
	}


	// Draw the mouse
	DrawCursor(tMenu->bmpScreen);
}

////////////////////
// Shutdown
void Menu_Net_HostLobbyShutdown(void)
{
	// Shutdown all dialogs
	if (bHostGameSettings)
		Menu_GameSettingsShutdown();
	if (bHostWeaponRest)
		Menu_WeaponsRestrictionsShutdown();
	if (bBanList)
		Menu_BanListShutdown();
	if (bServerSettings)
		Menu_ServerSettingsShutdown();

	// Shutdown
	cHostLobby.Shutdown();

	// Tell any clients that we're leaving
	cServer->SendDisconnect();

	// Shutdown server & clients
	cServer->Shutdown();
	cClient->Shutdown();

	// Recover the host type
	iHostType = 0;
}


///////////////////
// Draw the host lobby screen
void Menu_HostDrawLobby(SDL_Surface *bmpDest)
{
	CListview *player_list = (CListview *)cHostLobby.getWidget(hl_PlayerList);
	if (!player_list)  // Weird, shouldn't happen
		return;

	// Update the pings first
	CWorm *w = cClient->getRemoteWorms() + 1;
	int i;
	for (i=1; i < MAX_PLAYERS; i++, w++)  {  // Start from 1 (exclude host)
		CClient *client = cServer->getClient(w->getID());
		if (client)  {
			lv_subitem_t *subit = player_list->getSubItem(i, 5);
			if (subit)
				subit->sText = itoa(client->getPing());
		}
	}


	if (!bHost_Update)  // If no further update is needed, do not do it
		return;

	player_list->SaveScrollbarPos();
	player_list->Clear();  // Clear any old info

	game_lobby_t *gl = cClient->getGameLobby();
	lobbyworm_t *lobby_worm = NULL;
	w = cClient->getRemoteWorms();
	CButton *cmd_button = NULL;
	CImage *team_img = NULL;

	for (i=0; i < MAX_PLAYERS; i++, w++)  {
		if (!w->isUsed())  // Don't bother with unused worms
			continue;

		lobby_worm = w->getLobby();

		// Reload the worm graphics
		w->setTeam(lobby_worm->iTeam);
		w->LoadGraphics(cClient->getGameLobby()->nGameMode);

		// Create and setup the command button
		cmd_button = new CButton(0, gfxGUI.bmpCommandBtn);
		if (!cmd_button)
			continue;
		cmd_button->setType(BUT_TWOSTATES);
		cmd_button->setRedrawMenu(false);
		cmd_button->Setup(w->getID(), 0, 0, gfxGUI.bmpCommandBtn->w, gfxGUI.bmpCommandBtn->h);

		// Add the item
		player_list->AddItem(w->getName(), i, tLX->clNormalLabel); 
		player_list->AddSubitem(LVS_WIDGET, "", NULL, cmd_button);  // Command button
		if (lobby_worm->iReady)  // Ready control
			player_list->AddSubitem(LVS_IMAGE, "", tMenu->bmpLobbyReady, NULL);
		else
			player_list->AddSubitem(LVS_IMAGE, "", tMenu->bmpLobbyNotReady, NULL);
		player_list->AddSubitem(LVS_IMAGE, "", w->getPicimg(), NULL);  // Skin
		player_list->AddSubitem(LVS_TEXT, w->getName(), NULL, NULL);  // Name

		// Display the team mark if TDM
		if (gl->nGameMode == GMT_TEAMDEATH)  {
			team_img = new CImage(gfxGame.bmpTeamColours[lobby_worm->iTeam]);
			if (!team_img)
				continue;
			team_img->setID(w->getID());
			team_img->setRedrawMenu(false);

			player_list->AddSubitem(LVS_WIDGET, "", NULL, team_img); // Team
		} else {
			player_list->AddSubitem(LVS_TEXT, "", NULL, NULL); // Ping has to be the fifth subitem
		}

		// Ping
		CClient *cl = cServer->getClient(w->getID());
		int ping = 0;
		if (cl)	ping = cl->getPing();
		player_list->AddSubitem(LVS_TEXT, w->getID() != 0 ? itoa(ping) : "", NULL, NULL); // Don't draw for host
	}

	player_list->RestoreScrollbarPos();  // Scroll back to where we were before

	// Updated :)
	bHost_Update = false;
}


///////////////////
// Show the minimap
void Menu_HostShowMinimap(void)
{
	static CMap map;
	static std::string buf;

	cHostLobby.SendMessage(hl_LevelList, CBS_GETCURSINDEX, &buf, 0);

	// Draw a background over the old minimap
	DrawImageAdv(tMenu->bmpBuffer, tMenu->bmpMainBack_common, 463,32,463,32,128,96);

	// Load the map
	buf ="levels/"+buf;
	map.SetMinimapDimensions(128, 96);
	if(map.Load(buf)) {

		// Draw the minimap
		DrawImage(tMenu->bmpBuffer, map.GetMiniMap(), 463,32);
	}

	// Update the screen
	DrawImageAdv(tMenu->bmpScreen, tMenu->bmpBuffer, 457,30,457,30,140,110);
}


///////////////////
// Deregister the server
void Menu_Net_HostDeregister(void)
{
	// If the server wasn't registered, just leave
	if( !tLXOptions->tGameinfo.bRegServer )
		return;

	// Initialize the request
	if( !cServer->DeRegisterServer() )
		return;


	// Setup the background & show a messagebox
	int x = 160;
	int y = 170;
	int w = 320;
	int h = 140;
	int cx = x+w/2;
	int cy = y+h/2;


	DrawImage(tMenu->bmpBuffer,tMenu->bmpMainBack_common,0,0);
	if (tMenu->tFrontendInfo.bPageBoxes)
		Menu_DrawBox(tMenu->bmpBuffer, 15,60, 625, 465);
	Menu_DrawBox(tMenu->bmpBuffer, x, y, x+w, y+h);
	DrawRectFill(tMenu->bmpBuffer, x+2,y+2, x+w-1,y+h-1,tLX->clDialogBackground);

	tLX->cFont.DrawCentre(tMenu->bmpBuffer, cx, cy, tLX->clNormalLabel, "De-Registering server...");

	Menu_RedrawMouse(true);

	float starttime = tLX->fCurTime;
	SetGameCursor(CURSOR_ARROW);

	while(1) {
		Menu_RedrawMouse(false);
		ProcessEvents();
		tLX->fCurTime = GetMilliSeconds();

		// If we have gone over a 4 second limit, just leave
		if( tLX->fCurTime - starttime > 4.0f ) {
			http_Quit();
			break;
		}

		if( cServer->ProcessDeRegister() ) {
			http_Quit();
			break;
		}

		DrawCursor(tMenu->bmpScreen);
		FlipScreen(tMenu->bmpScreen);
	}
}

/*
=======================

      Server settings

=======================
*/



CGuiLayout		cServerSettings;

// Server settings dialog
enum {
	ss_Ok,
	ss_Cancel,
	ss_AllowOnlyList,
	ss_WelcomeMessage,
	ss_ServerName,
	ss_AllowWantsJoin,
	ss_AllowRemoteBots,
	ss_AllowNickChange,
	ss_MaxPlayers
};



///////////////////
// Initialize the server settings window
void Menu_ServerSettings(void)
{
	// Setup the buffer
	//DrawImageAdv(tMenu->bmpBuffer, tMenu->bmpMainBack_common, 120,130,120,130, 400,200);
	DrawRectFillA(tMenu->bmpBuffer, 120,130, 490,355, tLX->clDialogBackground, 200);
	Menu_DrawBox(tMenu->bmpBuffer, 120,130, 490,355);

	Menu_RedrawMouse(true);

	cServerSettings.Initialize();
	cServerSettings.Add( new CLabel("Server Settings", tLX->clNormalLabel),		  -1,        275,140,  0, 0);
    cServerSettings.Add( new CButton(BUT_OK, tMenu->bmpButtons),	  ss_Ok,	 360,330,  60,15);
	cServerSettings.Add( new CButton(BUT_CANCEL, tMenu->bmpButtons),  ss_Cancel, 220,330,  70,15);
	cServerSettings.Add( new CLabel("Server Name:", tLX->clNormalLabel),		  -1,        130,165,  0, 0);
	cServerSettings.Add( new CLabel("Welcome Message:", tLX->clNormalLabel),	  -1,        130,193,  0, 0);
	cServerSettings.Add( new CLabel("Max. Players:", tLX->clNormalLabel),		  -1,        130,218,  0, 0);
	cServerSettings.Add( new CTextbox(),							  ss_ServerName, 265,165,  200, tLX->cFont.GetHeight());
	cServerSettings.Add( new CTextbox(),							  ss_WelcomeMessage,        265,190,  200, tLX->cFont.GetHeight());
	cServerSettings.Add( new CTextbox(),							  ss_MaxPlayers, 265,215,  50, tLX->cFont.GetHeight());
	cServerSettings.Add( new CLabel("Allow \"Wants to join\" Messages",	tLX->clNormalLabel),-1,	130, 245,0,  0);
	cServerSettings.Add( new CCheckbox(0),		                    ss_AllowWantsJoin,	340,245,17, 17);
	cServerSettings.Add( new CLabel("Allow Bots in Server",				tLX->clNormalLabel),-1,	130, 275,0,  0);
	cServerSettings.Add( new CCheckbox(0),		                    ss_AllowRemoteBots,	340,275,17, 17);
	cServerSettings.Add( new CLabel("Allow Nick Change",				tLX->clNormalLabel),-1,	130, 305,0,  0);
	cServerSettings.Add( new CCheckbox(0),		                    ss_AllowNickChange,	340,305,17, 17);

	cServerSettings.SendMessage(ss_ServerName,TXM_SETMAX,32,0);
	cServerSettings.SendMessage(ss_WelcomeMessage,TXM_SETMAX,256,0);

	// Use the actual settings as default
	cServerSettings.SendMessage(ss_AllowWantsJoin, CKM_SETCHECK, tLXOptions->tGameinfo.bAllowWantsJoinMsg, 0);
	cServerSettings.SendMessage(ss_AllowRemoteBots, CKM_SETCHECK, tLXOptions->tGameinfo.bAllowRemoteBots, 0);
	cServerSettings.SendMessage(ss_AllowNickChange, CKM_SETCHECK, tLXOptions->tGameinfo.bAllowNickChange, 0);
	cServerSettings.SendMessage(ss_ServerName,TXS_SETTEXT,tGameInfo.sServername, 0);
	cServerSettings.SendMessage(ss_WelcomeMessage,TXS_SETTEXT,tGameInfo.sWelcomeMessage, 0);
	cServerSettings.SendMessage(ss_MaxPlayers, TXS_SETTEXT, itoa(tLXOptions->tGameinfo.iMaxPlayers), 0);
}


///////////////////
// Server settings frame
// Returns whether or not we have finished setting up the server
bool Menu_ServerSettings_Frame(void)
{
	gui_event_t *ev = NULL;

	DrawImageAdv(tMenu->bmpScreen, tMenu->bmpBuffer, 120,150, 120,150, 400,300);

    // Process events and draw gui
#ifdef WITH_MEDIAPLAYER
	if (!cMediaPlayer.GetDrawPlayer())
#endif
		ev = cServerSettings.Process();
	cServerSettings.Draw(tMenu->bmpScreen);

	if(ev) {

		switch(ev->iControlID) {

			// OK, done
			case ss_Ok:
				if(ev->iEventMsg == BTN_MOUSEUP) {

					// Save the info
					cServerSettings.SendMessage(ss_ServerName,TXS_GETTEXT, &tGameInfo.sServername, 0);
					cServerSettings.SendMessage(ss_WelcomeMessage,TXS_GETTEXT, &tGameInfo.sWelcomeMessage, 0);
					cServerSettings.SendMessage(ss_ServerName, TXS_GETTEXT, &tLXOptions->tGameinfo.sServerName, 0);
					cServerSettings.SendMessage(ss_WelcomeMessage, TXS_GETTEXT, &tLXOptions->tGameinfo.sWelcomeMessage, 0);

					std::string buf;
					cServerSettings.SendMessage(ss_MaxPlayers, TXS_GETTEXT, &buf, 0);
					tLXOptions->tGameinfo.iMaxPlayers = atoi(buf);
					// At least 2 players, and max MAX_PLAYERS
					tLXOptions->tGameinfo.iMaxPlayers = MAX(tLXOptions->tGameinfo.iMaxPlayers,2);
					tLXOptions->tGameinfo.iMaxPlayers = MIN(tLXOptions->tGameinfo.iMaxPlayers,MAX_PLAYERS);

					// Set up the server
					if(cServer)  {
						cServer->setName(tGameInfo.sServername);
						cServer->setMaxWorms(tLXOptions->tGameinfo.iMaxPlayers);
					}

					tLXOptions->tGameinfo.bAllowWantsJoinMsg = cServerSettings.SendMessage( ss_AllowWantsJoin, CKM_GETCHECK, (DWORD)0, 0) != 0;
					tLXOptions->tGameinfo.bAllowRemoteBots = cServerSettings.SendMessage( ss_AllowRemoteBots, CKM_GETCHECK, (DWORD)0, 0) != 0;
					tLXOptions->tGameinfo.bAllowNickChange = cServerSettings.SendMessage( ss_AllowNickChange, CKM_GETCHECK, (DWORD)0, 0) != 0;

					Menu_ServerSettingsShutdown();

					return true;
				}
				break;

			// Cancel, don't save changes
			case ss_Cancel:
				if(ev->iEventMsg == BTN_MOUSEUP) {
					Menu_ServerSettingsShutdown();

					return true;
				}
				break;
		}
	}

	// Draw the mouse
	DrawCursor(tMenu->bmpScreen);

	return false;
}

void Menu_ServerSettingsShutdown(void)
{
	cServerSettings.Shutdown();
}

/*
=======================

      Ban List

=======================
*/



CGuiLayout		cBanListGui;
CBanList        *cBanList = NULL;

// Ban List
enum {
	bl_Close=0,
	bl_Clear,
	bl_Unban,
	bl_ListBox
};



///////////////////
// Initialize the ban list window
void Menu_BanList(void)
{
	// Setup the buffer
	DrawImageAdv(tMenu->bmpBuffer, tMenu->bmpMainBack_common, 120,130,120,130, 400,320);
	Menu_DrawBox(tMenu->bmpBuffer, 120,130, 520,440);
	//DrawRectFillA(tMenu->bmpBuffer, 125,155, 380,260, 0, 100);

    CListview *tListBox = new CListview();

	Menu_RedrawMouse(true);

	cBanListGui.Initialize();
	cBanListGui.Add( new CLabel("Ban List", tLX->clNormalLabel),     -1,        275,135,  0, 0);
    cBanListGui.Add( new CButton(BUT_OK, tMenu->bmpButtons),	  bl_Close,   400,420, 60,15);
    cBanListGui.Add( new CButton(BUT_CLEAR, tMenu->bmpButtons),	  bl_Clear,     180,420, 60,15);
	cBanListGui.Add( new CButton(BUT_UNBAN, tMenu->bmpButtons),	  bl_Unban,     260,420, 60,15);
	cBanListGui.Add( tListBox,									  bl_ListBox,125,155, 380,260);


	tListBox->AddColumn("IP Address",130);
	tListBox->AddColumn("Nick",160);

	if (!cServer->getBanList())
		return;

	// Load the list
	cBanList = cServer->getBanList();

	tListBox->Clear();
	banlist_t *item;
	for (int i=0;i<(*cBanList).getNumItems(); i++)  {
		item = (*cBanList).getItemById(i);
		if (!item)
			continue;
		tListBox->AddItem(item->szAddress,i,tLX->clListView);
		tListBox->AddSubitem(LVS_TEXT, item->szAddress, NULL, NULL);
		tListBox->AddSubitem(LVS_TEXT, item->szNick, NULL, NULL);
	}
}


///////////////////
// Ban List frame
// Returns whether or not we have finished with the ban list management
bool Menu_BanList_Frame(void)
{
	gui_event_t *ev = NULL;
	CListview *tListBox = (CListview *)cBanListGui.getWidget(bl_ListBox);


	DrawImageAdv(tMenu->bmpScreen, tMenu->bmpBuffer, 120,150, 120,150, 400,300);

    // Process events and draw gui
#ifdef WITH_MEDIAPLAYER
	if (!cMediaPlayer.GetDrawPlayer())
#endif
		ev = cBanListGui.Process();
	cBanListGui.Draw(tMenu->bmpScreen);

	if(ev) {

		switch(ev->iControlID) {

			// Close
			case bl_Close:
				if(ev->iEventMsg == BTN_MOUSEUP) {

					Menu_BanListShutdown();

					return true;
				}
				break;
			// Unban
			case bl_Unban:
				if(ev->iEventMsg == BTN_MOUSEUP) {

					if (tListBox->getItemCount() > 0)  {

						(*cBanList).removeBanned(tListBox->getCurSub()->sText);

						tListBox->RemoveItem(tListBox->getCurIndex());
					}
				}
				break;
			// Clear
			case bl_Clear:
				if(ev->iEventMsg == BTN_MOUSEUP) {
					(*cBanList).Clear();

					tListBox->Clear();
				}
				break;

		}
	}

	// Draw the mouse
	DrawCursor(tMenu->bmpScreen);

	return false;
}

//////////////
// Shutdown
void Menu_BanListShutdown(void)
{
	if (cBanList)
		cBanList->saveList(cBanList->getPath());
	cBanListGui.Shutdown();
}
