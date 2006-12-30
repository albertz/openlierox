/////////////////////////////////////////
//
//             Liero Xtreme
//
//     Copyright Auxiliary Software 2002
//
//
/////////////////////////////////////////


// Net menu - Hosting
// Created 12/8/02
// Jason Boettcher


#include "defs.h"
#include "LieroX.h"
#include "Menu.h"


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
	hs_AllowRemoteBots
};


///////////////////
// Initialize the host menu
int Menu_Net_HostInitialize(void)
{
	char buf[64];

	iNetMode = net_host;
	iHostType = 0;

	// Player gui layout
	cHostPly.Shutdown();
	cHostPly.Initialize();

    cHostPly.Add( new CLabel("Player settings",tLX->clHeading), -1, 350,140,0,0);
	cHostPly.Add( new CLabel("Server settings",tLX->clHeading), -1, 30, 140,0,0);
	cHostPly.Add( new CButton(BUT_BACK, tMenu->bmpButtons), hs_Back,	25, 440,50, 15);
	cHostPly.Add( new CButton(BUT_OK, tMenu->bmpButtons),   hs_Ok,			585,440,30, 15);

	cHostPly.Add( new CListview(),							hs_PlayerList,	360,160,200,130);
	cHostPly.Add( new CListview(),							hs_Playing,		360,310,200,70);	
	cHostPly.Add( new CLabel("Server name",	tLX->clNormalLabel),		-1,			40, 178,0,  0);
	cHostPly.Add( new CTextbox(),							hs_Servername,	175,175,140,20);
	cHostPly.Add( new CLabel("Max Players",	tLX->clNormalLabel),		-1,			40,	208,0,  0);
	cHostPly.Add( new CTextbox(),							hs_MaxPlayers,	175,205,140,20);
    //cHostPly.Add( new CLabel("Password",	tLX->clNormalLabel),	    -1,			40, 238,0,  0);
	//cHostPly.Add( new CTextbox(),		                    hs_Password,	175,235,140,20);
	cHostPly.Add( new CLabel("Welcome message",	tLX->clNormalLabel),		-1,			40, 238,0,  0);
	cHostPly.Add( new CTextbox(),							hs_WelcomeMessage, 175,235,140,20);
	cHostPly.Add( new CLabel("Register Server",	tLX->clNormalLabel),	-1,			153, 268,0,  0);
	cHostPly.Add( new CCheckbox(0),		                    hs_Register,	270,265,17, 17);    
	cHostPly.Add( new CLabel("Allow \"Wants join\" messages",	tLX->clNormalLabel),-1,	60, 298,0,  0);
	cHostPly.Add( new CCheckbox(0),		                    hs_AllowWantsJoin,	270,295,17, 17);
	cHostPly.Add( new CLabel("Allow bots in server",			tLX->clNormalLabel),-1,	110, 328,0,  0);
	cHostPly.Add( new CCheckbox(0),		                    hs_AllowRemoteBots,	270,325,17, 17);

	cHostPly.SendMessage(hs_Servername,TXM_SETMAX,32,0);
	//cHostPly.SendMessage(hs_Password,TXM_SETMAX,32,0);
	cHostPly.SendMessage(hs_MaxPlayers,TXM_SETMAX,6,0);
	cHostPly.SendMessage(hs_WelcomeMessage,TXM_SETMAX,128,0);

	// Use previous settings
	cHostPly.SendMessage( hs_MaxPlayers, TXM_SETTEXT, (DWORD)itoa(tLXOptions->tGameinfo.iMaxPlayers,buf,10), 0);
	cHostPly.SendMessage( hs_Servername, TXM_SETTEXT, (DWORD)tLXOptions->tGameinfo.sServerName, 0);	
	cHostPly.SendMessage( hs_WelcomeMessage, TXM_SETTEXT, (DWORD)tLXOptions->tGameinfo.sWelcomeMessage, 0);	
	cHostPly.SendMessage( hs_Register,   CKM_SETCHECK, tLXOptions->tGameinfo.bRegServer, 0);
	cHostPly.SendMessage( hs_AllowWantsJoin,   CKM_SETCHECK, tLXOptions->tGameinfo.bAllowWantsJoinMsg, 0);
	cHostPly.SendMessage( hs_AllowRemoteBots,   CKM_SETCHECK, tLXOptions->tGameinfo.bAllowRemoteBots, 0);
    //cHostPly.SendMessage( hs_Password,   TXM_SETTEXT, (DWORD)tLXOptions->tGameinfo.szPassword, 0 );

	// Add columns
	cHostPly.SendMessage( hs_PlayerList,   LVM_ADDCOLUMN, (DWORD)"Players",22);
	cHostPly.SendMessage( hs_PlayerList,   LVM_ADDCOLUMN, (DWORD)"",60);
	cHostPly.SendMessage( hs_Playing,      LVM_ADDCOLUMN, (DWORD)"Playing",22);
	cHostPly.SendMessage( hs_Playing,      LVM_ADDCOLUMN, (DWORD)"",60);	

	// Add players to the list
	profile_t *p = GetProfiles();
	for(;p;p=p->tNext) {
		/*if(p->iType == PRF_COMPUTER)
			continue;*/

		cHostPly.SendMessage( hs_PlayerList, LVM_ADDITEM, (DWORD)"", p->iID);
		cHostPly.SendMessage( hs_PlayerList, LVM_ADDSUBITEM, LVS_IMAGE, (DWORD)p->bmpWorm);
		cHostPly.SendMessage( hs_PlayerList, LVM_ADDSUBITEM, LVS_TEXT,  (DWORD)p->sName);
	}


	return true;
}


///////////////////
// Shutdown the host menu
void Menu_Net_HostShutdown(void)
{
	cHostPly.Shutdown();
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
	mouse_t		*Mouse = GetMouse();
	gui_event_t *ev;
	CListview	*lv, *lv2;
	profile_t	*ply;

	// Process & Draw the gui
	ev = cHostPly.Process();
	cHostPly.Draw( tMenu->bmpScreen );


	// Process any events
	if(ev) {

		// Mouse type
		if(ev->cWidget->getType() == wid_Button)
			mouse = 1;
		if(ev->cWidget->getType() == wid_Textbox)
			mouse = 2;


		switch(ev->iControlID) {

			// Back
			case hs_Back:
				if(ev->iEventMsg == BTN_MOUSEUP) {

					// Click!
					PlaySoundSample(sfxGeneral.smpClick);

					// Shutdown
					cHostPly.Shutdown();

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

					// Make sure there is 0-2 players in the list
					if(lv2->getItemCount() < 2) {

						// Remove the item from the list
						lv->RemoveItem(index);

						ply = FindProfile(index);

						if(ply) {
							lv2->AddItem("",index,tLX->clListView);					
							lv2->AddSubitem(LVS_IMAGE, "", ply->bmpWorm);
							lv2->AddSubitem(LVS_TEXT, ply->sName, NULL);
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
						lv2->AddSubitem(LVS_IMAGE, "", ply->bmpWorm);
						lv2->AddSubitem(LVS_TEXT, ply->sName, NULL);
					}
				}
				break;

			// Ok
			case hs_Ok:
				if(ev->iEventMsg == BTN_MOUSEUP) {

					lv = (CListview *)cHostPly.getWidget(hs_Playing);

					// Make sure there is 1-2 players in the list
					if (lv->getItemCount() > 0 && lv->getItemCount() <= 2) {

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
								tGameInfo.cPlayers[count++] = ply;
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
						cHostPly.SendMessage( hs_Servername, TXM_GETTEXT, (DWORD)tGameInfo.sServername, sizeof(tGameInfo.sServername));
						cHostPly.SendMessage( hs_WelcomeMessage, TXM_GETTEXT, (DWORD)tGameInfo.sWelcomeMessage, sizeof(tGameInfo.sWelcomeMessage));
                        //cHostPly.SendMessage( hs_Password, TXM_GETTEXT, (DWORD)tGameInfo.sPassword, sizeof(tGameInfo.sPassword));
						
						// Save the info
						char buf[64];
						cHostPly.SendMessage( hs_Servername, TXM_GETTEXT, (DWORD)tLXOptions->tGameinfo.sServerName, sizeof(tLXOptions->tGameinfo.sServerName));
						cHostPly.SendMessage( hs_WelcomeMessage, TXM_GETTEXT, (DWORD)tLXOptions->tGameinfo.sWelcomeMessage, sizeof(tLXOptions->tGameinfo.sWelcomeMessage));
                        //cHostPly.SendMessage( hs_Password, TXM_GETTEXT, (DWORD)tLXOptions->tGameinfo.szPassword, sizeof(tLXOptions->tGameinfo.szPassword));
						cHostPly.SendMessage( hs_MaxPlayers, TXM_GETTEXT, (DWORD)buf, sizeof(buf));
						
						tLXOptions->tGameinfo.iMaxPlayers = atoi(buf);
						// At least 2 players, and max 8
						tLXOptions->tGameinfo.iMaxPlayers = MAX(tLXOptions->tGameinfo.iMaxPlayers,2);
						tLXOptions->tGameinfo.iMaxPlayers = MIN(tLXOptions->tGameinfo.iMaxPlayers,8);
						tLXOptions->tGameinfo.bRegServer =  cHostPly.SendMessage( hs_Register, CKM_GETCHECK, 0, 0) != 0;
						tLXOptions->tGameinfo.bAllowWantsJoinMsg = cHostPly.SendMessage( hs_AllowWantsJoin, CKM_GETCHECK, 0, 0) != 0;
						tLXOptions->tGameinfo.bAllowRemoteBots = cHostPly.SendMessage( hs_AllowRemoteBots, CKM_GETCHECK, 0, 0) != 0;

						cHostPly.Shutdown();

						// Click
						PlaySoundSample(sfxGeneral.smpClick);

						// Start the lobby
						Menu_Net_HostLobbyInitialize();
					}
				}
				break;
		}
	}


	// Draw the mouse
	DrawImage(tMenu->bmpScreen,gfxGUI.bmpMouse[mouse], Mouse->X,Mouse->Y);
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
CChatBox	cChat;
int			iSpeaking=-1;
int         g_nLobbyWorm = -1;
int			iHost_Recolorize = false;


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

	cClient->setChatbox(&cChat);
	cChat.Clear();
    cChat.setWidth(590);

	// Initialize the bots
	/*cBots = new CClient[tGameInfo.iNumBots];
	if (cBots)  {
		for (int i=0; i<tGameInfo.iNumBots; i++) {
			if (!cBots[i].Initialize(true,i))  {
				cHostLobby.Shutdown();
				return false;
			}

			// Connect the bot
			cBots[i].Connect("127.0.0.1");
		}
	}*/

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
	DrawImage(tMenu->bmpBuffer,tMenu->bmpMainBack_wob,0,0);
    Menu_DrawBox(tMenu->bmpBuffer, 5,5, 635, 475);
	Menu_DrawBox(tMenu->bmpBuffer, 460,29, 593, 130);
    DrawImageAdv(tMenu->bmpBuffer, tMenu->bmpMainBack_wob, 281,0, 281,0, 79,20);

    tLX->cFont.DrawCentre(tMenu->bmpBuffer, 320, -1, tLX->clNormalLabel,"%s", "[  Lobby  ]");

	// Chat box
    DrawRectFill(tMenu->bmpBuffer, 16, 270, 624, 417, 0);

    // Player box
    Menu_DrawBox(tMenu->bmpBuffer, 15, 29, 340, 235);

	Menu_RedrawMouse(true);
}


///////////////////
// Create the lobby gui
void Menu_Net_HostLobbyCreateGui(void)
{
    //Uint32 blue = MakeColour(0,138,251);

    // Lobby gui layout
	cHostLobby.Shutdown();
	cHostLobby.Initialize();

	cHostLobby.Add( new CButton(BUT_LEAVE, tMenu->bmpButtons),hl_Back,	15,  450, 60,  15);
	cHostLobby.Add( new CButton(BUT_START, tMenu->bmpButtons),hl_Start,	560, 450, 60,  15);
	cHostLobby.Add( new CButton(BUT_BANNED, tMenu->bmpButtons),hl_Banned,	450, 450, 90,  15);	
	cHostLobby.Add( new CButton(BUT_SERVERSETTINGS, tMenu->bmpButtons),hl_ServerSettings,	250, 450, 190, 15);	
	cHostLobby.Add( new CLabel("Players",tLX->clHeading),				  -1,		15,  15,  0,   0);
	cHostLobby.Add( new CTextbox(),							  hl_ChatText, 15,  421, 610, 20);
    cHostLobby.Add( new CListview(),                          hl_ChatList, 15,  253, 610, 165);

	cHostLobby.Add( new CButton(BUT_GAMESETTINGS, tMenu->bmpButtons), hl_GameSettings, 360, 210, 170,15);
    cHostLobby.Add( new CButton(BUT_WEAPONOPTIONS,tMenu->bmpButtons), hl_WeaponOptions,360, 235, 185,15);

	cHostLobby.Add( new CLabel("Mod",tLX->clNormalLabel),	    -1,         360, 180, 0,   0);
	cHostLobby.Add( new CCombobox(),				hl_ModName,    440, 179, 170, 17);
	cHostLobby.Add( new CLabel("Game type",tLX->clNormalLabel),	-1,         360, 158, 0,   0);
	cHostLobby.Add( new CCombobox(),				hl_Gametype,   440, 157, 170, 17);
    cHostLobby.Add( new CLabel("Level",tLX->clNormalLabel),	    -1,         360, 136, 0,   0);
    cHostLobby.Add( new CCombobox(),				hl_LevelList,  440, 135, 170, 17);

	cHostLobby.SendMessage(hl_ChatText,TXM_SETMAX,64,0);

	// Fill in the game details
	cHostLobby.SendMessage(hl_Gametype,    CBM_ADDITEM,   GMT_DEATHMATCH, (DWORD)"Deathmatch");
	cHostLobby.SendMessage(hl_Gametype,    CBM_ADDITEM,   GMT_TEAMDEATH,  (DWORD)"Team Deathmatch");
	cHostLobby.SendMessage(hl_Gametype,    CBM_ADDITEM,   GMT_TAG,        (DWORD)"Tag");

	// Fill in the mod list
	Menu_Local_FillModList( (CCombobox *)cHostLobby.getWidget(hl_ModName));

	// Fill in the levels list
	Menu_FillLevelList( (CCombobox *)cHostLobby.getWidget(hl_LevelList), false);
	Menu_HostShowMinimap();
    
    // Don't show chat box selection
    CListview *lv = (CListview *)cHostLobby.getWidget(hl_ChatList);
    lv->setShowSelect(false);

    game_lobby_t *gl = cServer->getLobby();
    cHostLobby.SendMessage(hl_LevelList, CBM_GETCURSINDEX, (DWORD)gl->szMapName, sizeof(gl->szMapName));
    cHostLobby.SendMessage(hl_ModName,	 CBM_GETCURNAME, (DWORD)gl->szModName, sizeof(gl->szModName));
    cHostLobby.SendMessage(hl_ModName,	 CBM_GETCURSINDEX, (DWORD)gl->szModDir, sizeof(gl->szModDir));
    cHostLobby.SendMessage(hl_Gametype,  CBM_SETCURINDEX, gl->nGameMode, 0);
}


///////////////////
// Go straight to the lobby, without clearing the server & client
void Menu_Net_HostGotoLobby(void)
{
	Uint32 blue = MakeColour(0,138,251);

	tGameInfo.iGameType = GME_HOST;
	iNetMode = net_host;
	iHostType = 1;
	bHostGameSettings = false;
    bHostWeaponRest = false;
    iSpeaking =- 1;

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

	cClient->setChatbox(&cChat);
	cChat.Clear();

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
	
	cServer->UpdateGameLobby();
}


///////////////////
// Host lobby frame
void Menu_Net_HostLobbyFrame(int mouse)
{
	mouse_t		*Mouse = GetMouse();
	gui_event_t *ev;

	// Process the server & client frames
	cServer->Frame();
	cClient->Frame();
	if (cBots)
		for (int i=0;i<tGameInfo.iNumBots;i++)
			cBots[i].Frame();


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


    // Add chat to the listbox
    for(int i=0;i<MAX_CLINES;i++) {
		line_t *l = cChat.GetLine(i);
		if(l->iUsed) {
            l->iUsed = false;
            CListview *lv = (CListview *)cHostLobby.getWidget(hl_ChatList);

            if(lv->getLastItem())
                lv->AddItem("", lv->getLastItem()->iIndex+1, l->iColour);
            else
                lv->AddItem("", 0, l->iColour);
            lv->AddSubitem(LVS_TEXT, l->strLine, NULL);
            lv->setShowSelect(false);

            // If there are too many lines, remove the top line
            if(lv->getItemCount() > 256) {
                if(lv->getItems())
                    lv->RemoveItem(lv->getItems()->iIndex);
            }

            lv->scrollLast();           
		}
	}



    // Clear the chat box & player list & around combo boxes
    //Menu_redrawBufferRect(25, 315, 590, 100);
    Menu_redrawBufferRect(15, 20,  335, 225);
	Menu_redrawBufferRect(540, 150,  100, 200);


    // Draw the host lobby details
	Menu_HostDrawLobby(tMenu->bmpScreen);
	
	// Process & Draw the gui
	ev = cHostLobby.Process();
	cHostLobby.Draw( tMenu->bmpScreen );

	

	// Process any events
	if(ev) {

		// Mouse type
		if(ev->cWidget->getType() == wid_Button)
			mouse = 1;
		if(ev->cWidget->getType() == wid_Textbox)
			mouse = 2;


		switch(ev->iControlID) {

			// Back
			case hl_Back:
				if(ev->iEventMsg == BTN_MOUSEUP) {
					// Click!
					PlaySoundSample(sfxGeneral.smpClick);

					// Shutdown
					cHostLobby.Shutdown();

					// Tell any clients that we're leaving
					cServer->SendDisconnect();

					// De-register the server
					Menu_Net_HostDeregister();

					// Shutdown server & clients
					cServer->Shutdown();
					if (cBots)
						for (int i=0;i<tGameInfo.iNumBots;i++)
							cBots[i].Shutdown();
					cClient->Shutdown();

					// Back to main net menu
					Menu_NetInitialize();
				}
				break;


			// Chat textbox
			case hl_ChatText:
				if(ev->iEventMsg == TXT_ENTER && iSpeaking >= 0) {
					// Send the msg to the server

					// Get the text
					char buf[128];
					cHostLobby.SendMessage(hl_ChatText, TXM_GETTEXT, (DWORD)buf, sizeof(buf));
                    
                    // Don't send empty messages
                    if(strlen(buf) == 0)
                        break;

					// Clear the text box
					cHostLobby.SendMessage(hl_ChatText, TXM_SETTEXT, (DWORD)"", 0);

					// Get name
					char text[256];
					CWorm *rw = cClient->getRemoteWorms() + iSpeaking;
					if (strstr(buf,"/me") == NULL)
						sprintf(text, "%s: %s",rw->getName(), buf);
					else
						sprintf(text, "%s", replacemax(buf,"/me",rw->getName(),buf,2));
					cServer->SendGlobalText(text,TXT_CHAT);
				}
				break;

			// Level change
			case hl_LevelList:
				if(ev->iEventMsg == CMB_CHANGED) {
					Menu_HostShowMinimap();
					
					cHostLobby.SendMessage(hl_LevelList, CBM_GETCURSINDEX, (DWORD)cServer->getLobby()->szMapName, sizeof(cServer->getLobby()->szMapName));
					cServer->UpdateGameLobby();
				}
				break;

            // Mod change
            case hl_ModName:
                if(ev->iEventMsg == CMB_CHANGED) {
                    cHostLobby.SendMessage(hl_ModName, CBM_GETCURNAME, (DWORD)cServer->getLobby()->szModName, sizeof(cServer->getLobby()->szModName));
                    cHostLobby.SendMessage(hl_ModName, CBM_GETCURSINDEX, (DWORD)cServer->getLobby()->szModDir, sizeof(cServer->getLobby()->szModDir));
					cServer->UpdateGameLobby();
                }
                break;

			// Game type change
			case hl_Gametype:
				if(ev->iEventMsg == CMB_CHANGED) {
					cServer->getLobby()->nGameMode = cHostLobby.SendMessage(hl_Gametype, CBM_GETCURINDEX, 0, 0);
					iHost_Recolorize = true;
					cServer->UpdateGameLobby();
				}
				break;

			// Lives change
			case hl_Lives:
				if(ev->iEventMsg == TXT_CHANGE) {
					char buf[128];
					cHostLobby.SendMessage(hl_Lives, TXM_GETTEXT, (DWORD)buf, sizeof(buf));
					if(*buf)
						cServer->getLobby()->nLives = atoi(buf);
					else
						cServer->getLobby()->nLives = -2;

					cServer->UpdateGameLobby();
				}
				break;


			// Max Kills
			case hl_MaxKills:
				if(ev->iEventMsg == TXT_CHANGE) {
					char buf[128];
					cHostLobby.SendMessage(hl_MaxKills, TXM_GETTEXT, (DWORD)buf, sizeof(buf));
					if(*buf)
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

                    cb_item_t *it = (cb_item_t *)cHostLobby.SendMessage(hl_ModName,CBM_GETCURITEM,0,0);
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
					cb_item_t *it = (cb_item_t *)cHostLobby.SendMessage(hl_ModName,CBM_GETCURITEM,0,0);
                    if(it) {
		                strcpy(tGameInfo.sModName,it->sIndex);
						strcpy(tGameInfo.sModDir,it->sIndex);
                        strcpy(tLXOptions->tGameinfo.szModName, it->sIndex);
                    }

                    // Get the game type
                    tGameInfo.iGameMode = cHostLobby.SendMessage(hl_Gametype, CBM_GETCURINDEX, 0, 0);
                    tLXOptions->tGameinfo.nGameType = tGameInfo.iGameMode;
					
					// Get the map name
					cHostLobby.SendMessage(hl_LevelList, CBM_GETCURSINDEX, (DWORD)tGameInfo.sMapname, sizeof(tGameInfo.sMapname));
					// Save the current level in the options
					cHostLobby.SendMessage(hl_LevelList, CBM_GETCURSINDEX, (DWORD)tLXOptions->tGameinfo.sMapName, sizeof(tLXOptions->tGameinfo.sMapName));
					cHostLobby.Shutdown();

                    // Setup the client
                    cClient->SetupViewports();

					// Start the game
					cServer->StartGame();
					
					// Leave the frontend
					*iGame = true;
					tMenu->iMenuRunning = false;
					tGameInfo.iGameType = GME_HOST;					

					return;
				}
				break;

            // Popup menu
            case hl_PopupMenu:
                switch( ev->iEventMsg ) {
                    // Kick the player
                    case MNU_USER+0:
                        // Kick the player
                        if( g_nLobbyWorm > 0 )
                            cServer->kickWorm( g_nLobbyWorm );
                        break;
					case MNU_USER+1:
						if ( g_nLobbyWorm > 0 )
							cServer->banWorm( g_nLobbyWorm );
						break;
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
                }

                // Remove the menu widget
                cHostLobby.SendMessage( hl_PopupMenu, MNM_REDRAWBUFFER, 0, 0);
                cHostLobby.removeWidget(hl_PopupMenu);
                break;
		}
	}


	// Draw the mouse
	DrawImage(tMenu->bmpScreen,gfxGUI.bmpMouse[mouse], Mouse->X,Mouse->Y);
}


///////////////////
// Draw the host lobby screen
void Menu_HostDrawLobby(SDL_Surface *bmpDest)
{
	int		i,y,local;
	mouse_t *Mouse = GetMouse();

	// Draw the connected players
	CWorm *w = cClient->getRemoteWorms();
	game_lobby_t *gl = cClient->getGameLobby();
	lobbyworm_t *l;
	local = false;
    y = 40;
	bool bRecolorized = false;
	for(i=0; i<gl->nMaxWorms; i++, w++) {
        if( !w->isUsed() )
            continue;

		l = w->getLobby();

		local = false;
		if(w->isUsed())
			local = w->getLocal();

		if(local && iSpeaking == -1)
			iSpeaking = i;

		// Check for a click
		if(Mouse->Up & SDL_BUTTON(1)) {
			if(Mouse->Y > y && Mouse->Y < y+18) {

				// Speech
				/*if(Mouse->X > 20 && Mouse->X < 34) {
					if(l->iType == LBY_USED &&
					   local &&
					   cClient->getNumWorms()>1 &&
					   iSpeaking != i)
					      iSpeaking = i;
				}*/

				// Name
				if(Mouse->X > 77 && Mouse->X < 120) {

					// Open/Closed
					/*if(l->iType == LBY_OPEN)
						l->iType = LBY_CLOSED;
					else if(l->iType == LBY_CLOSED)
						l->iType = LBY_OPEN;*/

					// Name selection
				}
			}
		}

        int x = 20;

		switch(l->iType) {

			// Open
			case LBY_OPEN:
				//tLX->cFont.Draw(bmpDest, 77, y, tLX->clNormalLabel,"%s", "------");

                //DrawHLine(bmpDest, x, x+315, y+20, MakeColour(64,64,64));
				break;

			// Closed
			case LBY_CLOSED:
				//tLX->cFont.Draw(bmpDest, 77, y, tLX->clNormalLabel,"%s", "Closed");
				break;

			// Used
			case LBY_USED:

                //                
                // Draw the worm details
                //

                // Function button
                bool down = false;
                if( MouseInRect(x,y, 10,10) && Mouse->Down )
                    down = true;
                Menu_DrawWinButton(bmpDest, x,y, 10,10, down);

                // Check if the function button was clicked
                // Note: Function button only used on clients, not the host worm
                if( MouseInRect(x,y,10,10) && Mouse->Up && i > 0 ) {
                    g_nLobbyWorm = i;
                    // Remove old popup menu
                    cHostLobby.removeWidget(hl_PopupMenu);

					CClient *remote_cl = cServer->getClient(i);

                    cHostLobby.Add( new CMenu(Mouse->X, Mouse->Y), hl_PopupMenu, 0,0, 640,480 );
                    cHostLobby.SendMessage( hl_PopupMenu, MNM_ADDITEM, 0, (DWORD)"Kick player" );
					cHostLobby.SendMessage( hl_PopupMenu, MNM_ADDITEM, 1, (DWORD)"Ban player" );
					if (remote_cl)  {
						if (remote_cl->getMuted())
							cHostLobby.SendMessage( hl_PopupMenu, MNM_ADDITEM, 2, (DWORD)"Unmute player" );
						else
							cHostLobby.SendMessage( hl_PopupMenu, MNM_ADDITEM, 2, (DWORD)"Mute player" );
					}

                }

                // Ready icon
                if(l->iReady)
					DrawImageAdv(bmpDest, tMenu->bmpLobbyState, 0,0, x+15,y-1,12,12);
				else
					DrawImageAdv(bmpDest, tMenu->bmpLobbyState, 0,12,x+15,y-1,12,12);

                // Worm		
                DrawImage(bmpDest, w->getPicimg(), x+30, y-2);
				tLX->cFont.Draw(bmpDest, x+55, y-2, tLX->clNormalLabel,"%s", w->getName());    
                
                // Team
                CWorm *sv_w = cServer->getWorms() + i;
                sv_w->getLobby()->iTeam = l->iTeam;
                if( gl->nGameMode == GMT_TEAMDEATH ) {

                    DrawImage(bmpDest, tMenu->bmpTeamColours[l->iTeam], x+200, y-2);

                    // Check if the team colour was clicked on
                    if( MouseInRect(x+200, y-2, 18,16) && Mouse->Up ) {                        
                        sv_w->getLobby()->iTeam = (l->iTeam+1) % 4;

                        // Send the worm state updates
                        cServer->SendWormLobbyUpdate();

						iHost_Recolorize = true;
                    }

                    sv_w->setTeam(sv_w->getLobby()->iTeam);
					w->setTeam(sv_w->getLobby()->iTeam);

				}

				// Colorize the worm
				if (iHost_Recolorize)  {
					w->LoadGraphics(gl->nGameMode);
					bRecolorized = true;
				}


				// Ping
				CClient *client = cServer->getClient(sv_w->getID());
				if (client && sv_w->getID())  {
					int png = client->getPing();
					if (png > 99999 || png < 0)
						png = 0;
					tLX->cFont.Draw(bmpDest, x+280, y-2, tLX->clNormalLabel, "%d", png);
				}

				break;
		}

        // Dividing line
        DrawHLine(bmpDest, x, x+315, y+20, MakeColour(64,64,64));

        y+=25;
	}

	if (bRecolorized)
		iHost_Recolorize = false;

}


///////////////////
// Show the minimap
void Menu_HostShowMinimap(void)
{
	CMap map;
	char buf[256];
	char blah[256];

	cHostLobby.SendMessage(hl_LevelList, CBM_GETCURSINDEX, (DWORD)buf, sizeof(buf));					

	// Draw a background over the old minimap
	DrawImageAdv(tMenu->bmpBuffer, tMenu->bmpMainBack_wob, 463,32,463,32,128,96);
	
	// Load the map
	sprintf(blah, "levels/%s",buf);
	if(map.Load(blah)) {

		// Draw the minimap
		map.UpdateMiniMap(true);
		DrawImage(tMenu->bmpBuffer, map.GetMiniMap(), 463,32);		
		map.Shutdown();
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
	mouse_t *Mouse = GetMouse();


	DrawImage(tMenu->bmpBuffer,tMenu->bmpMainBack_wob,0,0);
    Menu_DrawBox(tMenu->bmpBuffer, 15,60, 625, 465);
	Menu_DrawBox(tMenu->bmpBuffer, x, y, x+w, y+h);
	DrawRectFill(tMenu->bmpBuffer, x+2,y+2, x+w-1,y+h-1,0);

	tLX->cFont.DrawCentre(tMenu->bmpBuffer, cx, cy, tLX->clNormalLabel,"%s", "De-Registering server...");

	Menu_RedrawMouse(true);

	float starttime = tLX->fCurTime;

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

		DrawImage(tMenu->bmpScreen,gfxGUI.bmpMouse[0], Mouse->X,Mouse->Y);
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
	ss_MaxPlayers
};



///////////////////
// Initialize the server settings window
void Menu_ServerSettings(void)
{
	Uint32 blue = MakeColour(0,138,251);

	// Setup the buffer
	//DrawImageAdv(tMenu->bmpBuffer, tMenu->bmpMainBack_wob, 120,130,120,130, 400,200);
	DrawRectFillA(tMenu->bmpBuffer, 120,130, 490,325, 0, 200);
	Menu_DrawBox(tMenu->bmpBuffer, 120,130, 490,325);

    CListview *tListBox = new CListview();

	Menu_RedrawMouse(true);

	cServerSettings.Initialize();
	cServerSettings.Add( new CLabel("Server settings", tLX->clNormalLabel),		  -1,        275,140,  0, 0);	
    cServerSettings.Add( new CButton(BUT_OK, tMenu->bmpButtons),	  ss_Ok,	 360,300,  60,15);
	cServerSettings.Add( new CButton(BUT_CANCEL, tMenu->bmpButtons),  ss_Cancel, 220,300,  70,15);
	cServerSettings.Add( new CLabel("Server name:", tLX->clNormalLabel),		  -1,        130,165,  0, 0);
	cServerSettings.Add( new CLabel("Welcome message:", tLX->clNormalLabel),	  -1,        130,193,  0, 0);
	cServerSettings.Add( new CLabel("Max. Players:", tLX->clNormalLabel),		  -1,        130,218,  0, 0);
	cServerSettings.Add( new CTextbox(),							  ss_ServerName, 265,165,  200, 20);
	cServerSettings.Add( new CTextbox(),							  ss_WelcomeMessage,        265,190,  200, 20);
	cServerSettings.Add( new CTextbox(),							  ss_MaxPlayers, 265,215,  50, 20);
	cServerSettings.Add( new CLabel("Allow \"Wants join\" messages",	tLX->clNormalLabel),-1,	130, 245,0,  0);
	cServerSettings.Add( new CCheckbox(0),		                    ss_AllowWantsJoin,	340,245,17, 17);
	cServerSettings.Add( new CLabel("Allow bots in server",				tLX->clNormalLabel),-1,	130, 275,0,  0);
	cServerSettings.Add( new CCheckbox(0),		                    ss_AllowRemoteBots,	340,275,17, 17);

	cServerSettings.SendMessage(ss_ServerName,TXM_SETMAX,32,0);
	cServerSettings.SendMessage(ss_WelcomeMessage,TXM_SETMAX,256,0);

	// Use the actual settings as default
	cServerSettings.SendMessage(ss_AllowWantsJoin, CKM_SETCHECK, tLXOptions->tGameinfo.bAllowWantsJoinMsg, 0);
	cServerSettings.SendMessage(ss_AllowRemoteBots, CKM_SETCHECK, tLXOptions->tGameinfo.bAllowRemoteBots, 0);
	cServerSettings.SendMessage(ss_ServerName,TXM_SETTEXT,(DWORD) tGameInfo.sServername, 0);
	cServerSettings.SendMessage(ss_WelcomeMessage,TXM_SETTEXT,(DWORD) tGameInfo.sWelcomeMessage, 0);
	char buf[4];
	cServerSettings.SendMessage(ss_MaxPlayers, TXM_SETTEXT, (DWORD)itoa(tLXOptions->tGameinfo.iMaxPlayers,buf,10), 0);
}


///////////////////
// Server settings frame
// Returns whether or not we have finished setting up the server
bool Menu_ServerSettings_Frame(void)
{
	gui_event_t *ev = NULL;
	mouse_t *Mouse = GetMouse();
	int mouse = 0;
    //Uint32 blue = MakeColour(0,138,251);

	DrawImageAdv(tMenu->bmpScreen, tMenu->bmpBuffer, 120,150, 120,150, 400,300);

    // Process events and draw gui

	ev = cServerSettings.Process();
	cServerSettings.Draw(tMenu->bmpScreen);

	if(ev) {

		if(ev->cWidget->getType() == wid_Button)
			mouse = 1;
		if(ev->cWidget->getType() == wid_Textbox)
			mouse = 2;

		switch(ev->iControlID) {

			// OK, done
			case ss_Ok:
				if(ev->iEventMsg == BTN_MOUSEUP) {

					// Save the info
					cServerSettings.SendMessage(ss_ServerName,TXM_GETTEXT, (DWORD) tGameInfo.sServername, sizeof(tGameInfo.sServername));
					cServerSettings.SendMessage(ss_WelcomeMessage,TXM_GETTEXT, (DWORD) tGameInfo.sWelcomeMessage, sizeof(tGameInfo.sWelcomeMessage));
					cServerSettings.SendMessage(ss_ServerName, TXM_GETTEXT, (DWORD)tLXOptions->tGameinfo.sServerName, sizeof(tLXOptions->tGameinfo.sServerName));
					cServerSettings.SendMessage(ss_WelcomeMessage, TXM_GETTEXT, (DWORD)tLXOptions->tGameinfo.sWelcomeMessage, sizeof(tLXOptions->tGameinfo.sWelcomeMessage));
			
					char buf[4];
					cServerSettings.SendMessage(ss_MaxPlayers, TXM_GETTEXT, (DWORD)buf, sizeof(buf));
					tLXOptions->tGameinfo.iMaxPlayers = atoi(buf);
					// At least 2 players, and max 8
					tLXOptions->tGameinfo.iMaxPlayers = MAX(tLXOptions->tGameinfo.iMaxPlayers,2);
					tLXOptions->tGameinfo.iMaxPlayers = MIN(tLXOptions->tGameinfo.iMaxPlayers,8);

					// Set up the server
					if(cServer)  {
						cServer->setName(tGameInfo.sServername);
						cServer->setMaxWorms(tLXOptions->tGameinfo.iMaxPlayers);
					}

					tLXOptions->tGameinfo.bAllowWantsJoinMsg = cServerSettings.SendMessage( ss_AllowWantsJoin, CKM_GETCHECK, 0, 0) != 0;
					tLXOptions->tGameinfo.bAllowRemoteBots = cServerSettings.SendMessage( ss_AllowRemoteBots, CKM_GETCHECK, 0, 0) != 0;

					cServerSettings.Shutdown();							

					return true;
				}
				break;

			// Cancel, don't save changes
			case ss_Cancel:
				if(ev->iEventMsg == BTN_MOUSEUP) {
					cServerSettings.Shutdown();

					return true;
				}
				break;
		}
	}

	// Draw the mouse
	DrawImage(tMenu->bmpScreen,gfxGUI.bmpMouse[mouse], Mouse->X,Mouse->Y);
	
	return false;
}

/*
=======================

      Ban List

=======================
*/



CGuiLayout		cBanListGui;
CBanList        *cBanList;

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
	Uint32 blue = MakeColour(0,138,251);

	// Setup the buffer
	DrawImageAdv(tMenu->bmpBuffer, tMenu->bmpMainBack_wob, 120,130,120,130, 400,320);
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
		if (!item || !item->szNick || !item->szAddress)
			continue;
		tListBox->AddItem(item->szAddress,i,tLX->clListView);
		tListBox->AddSubitem(LVS_TEXT, item->szAddress, NULL);
		tListBox->AddSubitem(LVS_TEXT, item->szNick, NULL);
	}
}


///////////////////
// Ban List frame
// Returns whether or not we have finished with the ban list management
bool Menu_BanList_Frame(void)
{
	gui_event_t *ev = NULL;
	mouse_t *Mouse = GetMouse();
	int mouse = 0;
    Uint32 blue = MakeColour(0,138,251);
	CListview *tListBox = (CListview *)cBanListGui.getWidget(bl_ListBox);


	DrawImageAdv(tMenu->bmpScreen, tMenu->bmpBuffer, 120,150, 120,150, 400,300);

    // Process events and draw gui

	ev = cBanListGui.Process();
	cBanListGui.Draw(tMenu->bmpScreen);

	if(ev) {

		if(ev->cWidget->getType() == wid_Button)
			mouse = 1;
		if(ev->cWidget->getType() == wid_Textbox)
			mouse = 2;

		switch(ev->iControlID) {

			// Close
			case bl_Close:
				if(ev->iEventMsg == BTN_MOUSEUP) {

					cBanListGui.Shutdown();

                    (*cBanList).saveList((*cBanList).getPath());							

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
	DrawImage(tMenu->bmpScreen,gfxGUI.bmpMouse[mouse], Mouse->X,Mouse->Y);
	
	return false;
}
