/////////////////////////////////////////
//
//             Liero Xtreme
//
//     Copyright Auxiliary Software 2002
//
//
/////////////////////////////////////////


// Net menu - Joining
// Created 25/8/02
// Jason Boettcher


#include "defs.h"
#include "LieroX.h"
#include "Menu.h"


/*
   Joining is broken into 2 parts:
   1) Joining the server
   2) If we succeeded in join, the lobby
*/


// Joining menu's
enum {
	//join_players=0,
	join_connecting=0,
	join_lobby
};

int		iJoinMenu = join_connecting;
char	sJoinAddress[256];


///////////////////
// Join a server
int Menu_Net_JoinInitialize(char *sAddress)
{
	iNetMode = net_join;
	iJoinMenu = join_connecting;
	strcpy(sJoinAddress, sAddress);	

	if(!Menu_Net_JoinConnectionInitialize(sAddress)) {
		// Error
		return false;
	}


	return true;
}


///////////////////
// Main join frame
void Menu_Net_JoinFrame(int mouse)
{
	switch(iJoinMenu) {

		// Select players
/*		case join_players:
			Menu_Net_JoinPlayersFrame(mouse);
			break;*/

		// Connecting
		case join_connecting:
			Menu_Net_JoinConnectionFrame(mouse);
			break;

		// Lobby
		case join_lobby:
			Menu_Net_JoinLobbyFrame(mouse);
			break;
	}
}



/*

   Select the players to play as

   DEPRECATED, NOW USES DROP_DOWN

*/

/*CGuiLayout	cPlayerSel;
enum {
	PlayerList=0,
	Playing,
	Back,
	Ok
};



///////////////////
// Initialize the select players menu
int Menu_Net_JoinInitializePlayers(void)
{
	// Player gui layout
	cPlayerSel.Shutdown();
	cPlayerSel.Initialize();

	cPlayerSel.Add( new CButton(BUT_BACK, tMenu->bmpButtons),	Back,		25, 440, 50,15);
	cPlayerSel.Add( new CButton(BUT_OK, tMenu->bmpButtons),		Ok,			585,440, 30,15);
	cPlayerSel.Add( new CListview(),							PlayerList,	100,210, 200, 160);
	cPlayerSel.Add( new CListview(),							Playing,	340,210, 200, 160);
	cPlayerSel.Add( new CLabel("Select player to play as:",MakeColour(0,138,251)), -1, 100, 190,0,0);	


	// Add columns
	cPlayerSel.SendMessage( PlayerList, LVM_ADDCOLUMN, (DWORD)"Players",22);
	cPlayerSel.SendMessage( PlayerList, LVM_ADDCOLUMN, (DWORD)"",60);
	cPlayerSel.SendMessage( Playing, LVM_ADDCOLUMN, (DWORD)"Playing",22);
	cPlayerSel.SendMessage( Playing, LVM_ADDCOLUMN, (DWORD)"",60);

	// Add players to the list
	profile_t *p = GetProfiles();
	for(;p;p=p->tNext) {
		if(p->iType == PRF_COMPUTER)
			continue;

		cPlayerSel.SendMessage( PlayerList, LVM_ADDITEM, (DWORD)"", p->iID);
		cPlayerSel.SendMessage( PlayerList, LVM_ADDSUBITEM, LVS_IMAGE, (DWORD)p->bmpWorm);
		cPlayerSel.SendMessage( PlayerList, LVM_ADDSUBITEM, LVS_TEXT,  (DWORD)p->sName);
	}

    Menu_redrawBufferRect(0, 0, 640, 480);


	return true;
}


///////////////////
// Shutdown the join menu
void Menu_Net_JoinShutdown(void)
{
	cPlayerSel.Shutdown();
}


///////////////////
// Select players frame
void Menu_Net_JoinPlayersFrame(int mouse)
{
	mouse_t		*Mouse = GetMouse();
	gui_event_t *ev;
	CListview	*lv, *lv2;
	profile_t	*ply;

	// Process & Draw the gui
	ev = cPlayerSel.Process();
	cPlayerSel.Draw( tMenu->bmpScreen );


	// Process any events
	if(ev) {

		// Mouse type
		if(ev->cWidget->getType() == wid_Button)
			mouse = 1;
		if(ev->cWidget->getType() == wid_Textbox)
			mouse = 2;


		switch(ev->iControlID) {

			// Back
			case Back:
				if(ev->iEventMsg == BTN_MOUSEUP) {

					// Click!
					PlaySoundSample(sfxGeneral.smpClick);

					// Shutdown
					cPlayerSel.Shutdown();

					// TODO: Back to menu we came from
					Menu_NetInitialize();
				}
				break;


			// Player list
			case PlayerList:
				if(ev->iEventMsg == LV_DOUBLECLK) {
					// Add the item to the players list
					lv = (CListview *)cPlayerSel.getWidget(PlayerList);
					lv2 = (CListview *)cPlayerSel.getWidget(Playing);
					int index = lv->getCurIndex();

					// Make sure there is 0-1 players in the list
					if(lv2->getItemCount() < 1) {

						// Remove the item from the list
						lv->RemoveItem(index);

						ply = FindProfile(index);

						if(ply) {
							lv2->AddItem("",index,0xffff);					
							lv2->AddSubitem(LVS_IMAGE, "", ply->bmpWorm);
							lv2->AddSubitem(LVS_TEXT, ply->sName, NULL);
						}
					}
				}
				break;


			// Playing list
			case Playing:
				if(ev->iEventMsg == LV_DOUBLECLK) {
					// Add the item to the players list
					lv = (CListview *)cPlayerSel.getWidget(Playing);
					lv2 = (CListview *)cPlayerSel.getWidget(PlayerList);
					int index = lv->getCurIndex();

					// Remove the item from the list
					lv->RemoveItem(index);

					ply = FindProfile(index);

					if(ply) {
						lv2->AddItem("",index,0xffff);					
						lv2->AddSubitem(LVS_IMAGE, "", ply->bmpWorm);
						lv2->AddSubitem(LVS_TEXT, ply->sName, NULL);
					}
				}
				break;

			// Ok
			case Ok:
				if(ev->iEventMsg == BTN_MOUSEUP) {

					lv = (CListview *)cPlayerSel.getWidget(Playing);

					// Make sure there is 1-2 players in the list
					if(lv->getItemCount() > 0 && lv->getItemCount() < 3) {

						tGameInfo.iNumPlayers = lv->getItemCount();

						// Fill in the game structure												
						lv_item_t *item = lv->getItems();
		
						// Add the players to the list
						int count=0;
						for(;item;item=item->tNext) {
							if(item->iIndex < 0)
								continue;

							profile_t *ply = FindProfile(item->iIndex);
	
							if(ply)		
								tGameInfo.cPlayers[count++] = ply;
						}

						// Shutdown
						cPlayerSel.Shutdown();

						
						// Click
						PlaySoundSample(sfxGeneral.smpClick);

						// Connect to the server
						Menu_Net_JoinConnectionInitialize();
						return;
					}
				}
				break;
		}
	}


	// Draw the mouse
	DrawImage(tMenu->bmpScreen,gfxGUI.bmpMouse[mouse], Mouse->X,Mouse->Y);
}*/






/*

   Connect to the server

*/


CGuiLayout cConnecting;
enum {
	Cancel=0
};


///////////////////
// Initialize the connection menu
int Menu_Net_JoinConnectionInitialize(char *sAddress)
{
	iJoinMenu = join_connecting;
	tGameInfo.iGameType = GME_JOIN;
	strcpy(sJoinAddress, sAddress);	
	cConnecting.Shutdown();
	cConnecting.Initialize();

	cConnecting.Add( new CButton(BUT_CANCEL, tMenu->bmpButtons),	Cancel, 	25, 440, 75,15);

	if(!cClient->Initialize()) {
		// Error

		// Go back to the network menu
		Menu_Net_MainInitialize();
		cConnecting.Shutdown();
		return false;
	}

	game_lobby_t *gl = cClient->getGameLobby();
	gl->nSet = false;

	cClient->Connect(sJoinAddress);

    Menu_redrawBufferRect(0, 0, 640, 480);


	return true;
}


///////////////////
// Connection frame
void Menu_Net_JoinConnectionFrame(int mouse)
{
	mouse_t		*Mouse = GetMouse();
	gui_event_t *ev;


	// Process the client frame
	tLX->cFont.DrawCentre(tMenu->bmpScreen, 320, 180, 0xffff, "Connecting to %s",sJoinAddress);
	cClient->Frame();


	// Connected??
	if(cClient->getStatus() == NET_CONNECTED) {

		// Leave this connection screen & go to the lobby
		cConnecting.Shutdown();

		if(!Menu_Net_JoinLobbyInitialize()) {
			// Error

			cConnecting.Shutdown();
			Menu_Net_MainInitialize();
			return;
		}
	}

	// Check for a bad connection
	if(cClient->getBadConnection()) {
		Menu_MessageBox("Connection Error",cClient->getBadConnectionMsg(), LMB_OK);

		cClient->Shutdown();

		// Shutdown
		cConnecting.Shutdown();
		// TODO: Back to menu we came from
		Menu_NetInitialize();		
		return;
	}


	// Process & Draw the gui
	ev = cConnecting.Process();
	cConnecting.Draw( tMenu->bmpScreen );


	// Process any events
	if(ev) {

		// Mouse type
		if(ev->cWidget->getType() == wid_Button)
			mouse = 1;
		if(ev->cWidget->getType() == wid_Textbox)
			mouse = 2;


		switch(ev->iControlID) {

			// Cancel
			case Cancel:
				if(ev->iEventMsg == BTN_MOUSEUP) {

					// Click!
// TODO: implement sound system
PlaySoundSample(sfxGeneral.smpClick);

					// Shutdown
					cConnecting.Shutdown();

					// TODO: Back to menu we came from
					Menu_NetInitialize();
				}
				break;
		}
	}


	// Draw the mouse
	DrawImage(tMenu->bmpScreen,gfxGUI.bmpMouse[mouse], Mouse->X,Mouse->Y);
}







/*

   Join lobby

*/

CGuiLayout	cJoinLobby;
CChatBox	cJoinChat;
int			iJoinSpeaking=-1;
int			iJoin_Recolorize = true;
enum {
	Back2=0,
	Ready,
	ChatText,
	ChatList
};



///////////////////
// Initialize the joining lobby
int Menu_Net_JoinLobbyInitialize(void)
{
    Menu_Net_JoinDrawLobby();

    Menu_Net_JoinLobbyCreateGui();
	
	iNetMode = net_join;
	iJoinMenu = join_lobby;
	
	cClient->setChatbox(&cJoinChat);
	cJoinChat.Clear();
    cJoinChat.setWidth(570);
    iJoinSpeaking=-1;

	return true;
}


///////////////////
// Draw the join lobby
void Menu_Net_JoinDrawLobby(void)
{   
	// Create the buffer	
	DrawImage(tMenu->bmpBuffer,tMenu->bmpMainBack_wob,0,0);
    Menu_DrawBox(tMenu->bmpBuffer, 5,5, 635, 475);	

    // Title
    DrawImageAdv(tMenu->bmpBuffer, tMenu->bmpMainBack_wob, 281,0, 281,0, 79,20);
    tLX->cFont.DrawCentre(tMenu->bmpBuffer, 320, -1, 0xffff,"%s", "[  Lobby  ]");

	// Chat box
    DrawRectFill(tMenu->bmpBuffer, 16, 270, 624, 417, 0);

    // Player box
    Menu_DrawBox(tMenu->bmpBuffer, 15, 29, 340, 235);

	Menu_RedrawMouse(true);
}


///////////////////
// Create the lobby gui stuff
void Menu_Net_JoinLobbyCreateGui(void)
{
    Uint32 blue = MakeColour(0,138,251);

    cJoinLobby.Shutdown();
	cJoinLobby.Initialize();

	cJoinLobby.Add( new CButton(BUT_LEAVE, tMenu->bmpButtons),Back2,	15,  450, 60,  15);
    cJoinLobby.Add( new CButton(BUT_READY, tMenu->bmpButtons),Ready,	560, 450, 65,  15);    
	cJoinLobby.Add( new CLabel("Players",blue),				  -1,		15,  15,  0,   0);
	cJoinLobby.Add( new CTextbox(),							  ChatText, 15,  421, 610, 20);
    cJoinLobby.Add( new CListview(),                          ChatList, 15,  253, 610, 165);	

	cJoinLobby.SendMessage(ChatText,TXM_SETMAX,64,0);
}


///////////////////
// Go straight back to the join lobby
void Menu_Net_JoinGotoLobby(void)
{
    //Menu_redrawBufferRect(0, 0, 640, 480);

    Menu_Net_JoinDrawLobby();

    Menu_Net_JoinLobbyCreateGui();

	iNetMode = net_join;
	iJoinMenu = join_lobby;

	cClient->setChatbox(&cJoinChat);
	cJoinChat.Clear();
    iJoinSpeaking=-1;
}


///////////////////
// Join lobby frame
void Menu_Net_JoinLobbyFrame(int mouse)
{
	mouse_t		*Mouse = GetMouse();
	gui_event_t *ev;
	int			i,y,local;

	// Process the client
	cClient->Frame();

    // If there is a client error, leave
    if(cClient->getClientError()) {

        cJoinLobby.Shutdown();
        Menu_NetInitialize();
		return;
    }


	// If we have started, leave the frontend
	if(cClient->getGameReady()) {

		cJoinLobby.Shutdown();

        // Setup the client        
        cClient->SetupViewports();

		// Leave the frontend
		*iGame = true;
		tMenu->iMenuRunning = false;
		tGameInfo.iGameType = GME_JOIN;

		return;
	}


	// Check if the communication link between us & server is still ok
	if(cClient->getServerError()) {
		// Create the buffer
		DrawImage(tMenu->bmpBuffer,tMenu->bmpMainBack_wob,0,0);
		Menu_DrawSubTitle(tMenu->bmpBuffer,SUB_LOBBY);

		Menu_MessageBox("Communication", cClient->getServerErrorMsg(), LMB_OK);

        cJoinLobby.Shutdown();

		Menu_NetInitialize();
		return;
	}


    // Clear the player list and game settings
    Menu_redrawBufferRect(15, 20,  335, 230);
    Menu_redrawBufferRect(360,20,  280, 200);

	
	// Process & Draw the gui
	ev = cJoinLobby.Process();
	cJoinLobby.Draw( tMenu->bmpScreen );


	// Add chat to the listbox
    for(i=0;i<MAX_CLINES;i++) {
		line_t *l = cJoinChat.GetLine(i);
		if(l->iUsed) {
            l->iUsed = false;
            CListview *lv = (CListview *)cJoinLobby.getWidget(ChatList);

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

		if(local && iJoinSpeaking == -1)
			iJoinSpeaking = i;

		// Check for a click
		if(Mouse->Up & SDL_BUTTON(1)) {
			if(Mouse->Y > y && Mouse->Y < y+18) {

				// Speech
				/*if(Mouse->X > 20 && Mouse->X < 34) {
					if(l->iType == LBY_USED &&
					   local &&
					   cClient->getNumWorms()>1 &&
					   iJoinSpeaking != i)
					      iJoinSpeaking = i;
				}*/
			}
		}

        int x = 20;
		switch(l->iType) {

			// Open
			case LBY_OPEN:
				//tLX->cFont.Draw(tMenu->bmpScreen, 77, y, 0xffff,"%s", "------");
				break;

			// Closed
			case LBY_CLOSED:
				//tLX->cFont.Draw(tMenu->bmpScreen, 77, y, 0xffff,"%s", "Closed");
				break;

			// Used
			case LBY_USED:
				// Ready icon
                if(l->iReady)
					DrawImageAdv(tMenu->bmpScreen, tMenu->bmpLobbyState, 0,0, x+15,y-1,12,12);
				else
					DrawImageAdv(tMenu->bmpScreen, tMenu->bmpLobbyState, 0,12,x+15,y-1,12,12);

                // Worm
                DrawImage(tMenu->bmpScreen, w->getPicimg(), x+30, y-2);
				tLX->cFont.Draw(tMenu->bmpScreen, x+55, y-2, 0xffff,"%s", w->getName());    
                
                // Team
                if( gl->nGameMode == GMT_TEAMDEATH )  {
					// Colorize the worm
					if (iJoin_Recolorize) {
						w->setTeam(l->iTeam);
						w->LoadGraphics(GMT_TEAMDEATH);
						bRecolorized = true;
					}

					DrawImage(tMenu->bmpScreen, tMenu->bmpTeamColours[l->iTeam], x+200, y-2);
				}
				else  {
					if (iJoin_Recolorize) {
						w->LoadGraphics(gl->nGameMode);
						bRecolorized = true;
					}
				}

				break;
		}

        // Dividing line
        DrawHLine(tMenu->bmpScreen, x, x+315, y+20, MakeColour(64,64,64));
        y+=25;
	}

	if (bRecolorized)
		iJoin_Recolorize = false;


	// Draw the game info
	if(gl->nSet) {
		CFont *f = &tLX->cFont;
		Uint32 blue = MakeColour(0,138,251);
        int x = 360;
        int x2 = x+105;
        y = 15;

		char *gamemodes[] = {"Deathmatch","Team Deathmatch", "Tag", "Demolitions"};

		sprintf(&tGameInfo.sMapname[0],"%s",&gl->szMapName);
		sprintf(&tGameInfo.sModName[0],"%s",&gl->szModName);
		sprintf(&tGameInfo.sModDir[0],"%s",&gl->szModName);
		tGameInfo.iBonusesOn = gl->nBonuses;
		tGameInfo.iGameMode = gl->nGameMode;
		tGameInfo.iKillLimit = gl->nMaxKills;
		tGameInfo.iLives = gl->nLives;
		tGameInfo.iLoadingTimes = gl->nLoadingTime;

		f->Draw(tMenu->bmpScreen, x, y,  blue, "Game Details");
		f->Draw(tMenu->bmpScreen, x, y+20,  0xffff, "Level:");
        if(gl->bHaveMap)  {
			f->Draw(tMenu->bmpScreen, x2, y+20, 0xffff,"%s", gl->szDecodedMapName);
		    //f->Draw(tMenu->bmpScreen, x2, y+20,  0xffff,"%s", gl->szMapName);
		}
        else
            f->Draw(tMenu->bmpScreen, x2, y+20,  MakeColour(255,64,64), gl->szMapName);
		f->Draw(tMenu->bmpScreen, x, y+40, 0xffff,"%s", "Game Mode:");
		f->Draw(tMenu->bmpScreen, x2, y+40, 0xffff, "%s",gamemodes[gl->nGameMode]);
        f->Draw(tMenu->bmpScreen, x, y+60, 0xffff, "%s", "Mod:");
        if(gl->bHaveMod)
            f->Draw(tMenu->bmpScreen, x2, y+60, 0xffff, "%s", gl->szModName);
        else
            f->Draw(tMenu->bmpScreen, x2, y+60, MakeColour(255,64,64), "%s", gl->szModName);

		f->Draw(tMenu->bmpScreen, x, y+80, 0xffff, "%s", "Lives:");
		if(gl->nLives >= 0)
			f->Draw(tMenu->bmpScreen, x2, y+80, 0xffff, "%d",gl->nLives);

		f->Draw(tMenu->bmpScreen, x, y+100, 0xffff, "%s", "Max Kills:");
		if(gl->nMaxKills >= 0)
			f->Draw(tMenu->bmpScreen, x2, y+100, 0xffff, "%d",gl->nMaxKills);
		f->Draw(tMenu->bmpScreen,     x, y+120, 0xffff, "%s", "Loading time:");
		f->Draw(tMenu->bmpScreen,     x2, y+120, 0xffff, "%d%%", gl->nLoadingTime);
        f->Draw(tMenu->bmpScreen,     x, y+140, 0xffff, "%s", "Bonuses:");
        f->Draw(tMenu->bmpScreen,     x2, y+140, 0xffff, "%s", gl->nBonuses ? "On" : "Off");
	}




	// Process any events
	if(ev) {

		// Mouse type
		if(ev->cWidget->getType() == wid_Button)
			mouse = 1;
		if(ev->cWidget->getType() == wid_Textbox)
			mouse = 2;


		switch(ev->iControlID) {

			// Back
			case Back2:
				if(ev->iEventMsg == BTN_MOUSEUP) {
					// Disconnect
					cClient->Disconnect();

					// Click!
// TODO: implement sound system
PlaySoundSample(sfxGeneral.smpClick);

					// Shutdown
					cJoinLobby.Shutdown();

					// Disconnect & shutdown
					//cClient->Disconnect();
					cClient->Shutdown();

					// Back to net menu
					Menu_NetInitialize();
				}
				break;

			// Ready
			case Ready:
				if(ev->iEventMsg == BTN_MOUSEUP) {
					// Let the server know that my worms are now ready
					CBytestream bs;
					bs.Clear();
					bs.writeByte(C2S_UPDATELOBBY);
					bs.writeByte(1);
					cClient->getChannel()->getMessageBS()->Append(&bs);
					

					// Hide the ready button
					CButton *btn = (CButton *)cJoinLobby.getWidget(Ready);
					btn->setEnabled(false);
                    btn->redrawBuffer();
				}
				break;


			// Chat textbox
			case ChatText:
				if(ev->iEventMsg == TXT_ENTER && iJoinSpeaking >= 0) {
					// Send the msg to the server

					// Get the text
					char buf[128];
					cJoinLobby.SendMessage(ChatText, TXM_GETTEXT, (DWORD)buf, sizeof(buf));

                    // Don't send empty messages
                    if(strlen(buf) == 0)
                        break;

					// Clear the text box
					cJoinLobby.SendMessage(ChatText, TXM_SETTEXT, (DWORD)"", 0);

					// Get name
					char text[256];
					CWorm *rw = cClient->getRemoteWorms() + iJoinSpeaking;
					if (strstr(buf,"/me") == NULL)
						sprintf(text, "%s: %s",rw->getName(), buf);
					else
						sprintf(text, "%s", replacemax(buf,"/me",rw->getName(),buf,2));
					cClient->SendText(text);					
				}
				break;
		}
	}



	// Draw the mouse
	DrawImage(tMenu->bmpScreen,gfxGUI.bmpMouse[mouse], Mouse->X,Mouse->Y);
}
