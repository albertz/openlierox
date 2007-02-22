/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Net menu - Internet
// Created 29/12/02
// Jason Boettcher


#include "defs.h"
#include "LieroX.h"
#include "Menu.h"


CGuiLayout	cInternet;
std::string szNetCurServer;

// Internet widgets
enum {
	mi_Join=0,
	mi_ServerList,
	mi_Refresh,
	mi_UpdateList,
	mi_AddServer,
	mi_Back,
    mi_PopupMenu,
	mi_PlayerSelection
};


///////////////////
// Initialize the Internet menu
int Menu_Net_NETInitialize(void)
{
//	Uint32 blue = MakeColour(0,138,251);  // TODO: not used

	iNetMode = net_internet;
    szNetCurServer = "";

	cInternet.Shutdown();
	cInternet.Initialize();

	cInternet.Add( new CListview(),								mi_ServerList, 40, 180, 560, 240);
	cInternet.Add( new CButton(BUT_BACK, tMenu->bmpButtons),    mi_Back,       25, 440, 50,  15);
	cInternet.Add( new CButton(BUT_ADD, tMenu->bmpButtons),		mi_AddServer,  140,440, 40,  15);
	cInternet.Add( new CButton(BUT_REFRESH, tMenu->bmpButtons), mi_Refresh,	250,440, 83,  15);
	cInternet.Add( new CButton(BUT_UPDATELIST, tMenu->bmpButtons),	mi_UpdateList,  390,440, 125,  15);
	cInternet.Add( new CButton(BUT_JOIN, tMenu->bmpButtons),    mi_Join,		570,440, 43,  15);
	cInternet.Add( new CLabel("Select player:",tLX->clNormalLabel),-1,		125, 152, 180,15);
	cInternet.Add( new CCombobox(),								mi_PlayerSelection,		225,150, 170,  19);


	/*
	  Server list columns

      Connection speed
	  Name
	  State
	  Players
	  Ping
	  Address
    */

	// Add players to the list
	profile_t *p = GetProfiles();
	for(;p;p=p->tNext) {
		/*if(p->iType == PRF_COMPUTER)
			continue;*/

		cInternet.SendMessage( mi_PlayerSelection, CBS_ADDITEM, p->sName, p->iID);
		cInternet.SendMessage( mi_PlayerSelection, CBM_SETIMAGE, (DWORD)p->iID, (DWORD)p->bmpWorm);
	}

	cInternet.SendMessage( mi_PlayerSelection, CBM_SETCURINDEX, tLXOptions->tGameinfo.iLastSelectedPlayer, 0);

    Menu_redrawBufferRect(0, 0, 640, 480);

	cInternet.SendMessage( mi_ServerList, LVS_ADDCOLUMN, "", tLXOptions->iInternetList[0]);
	cInternet.SendMessage( mi_ServerList, LVS_ADDCOLUMN, "Server Name", tLXOptions->iInternetList[1]);
	cInternet.SendMessage( mi_ServerList, LVS_ADDCOLUMN, "State", tLXOptions->iInternetList[2]);
	cInternet.SendMessage( mi_ServerList, LVS_ADDCOLUMN, "Players", tLXOptions->iInternetList[3]);
	cInternet.SendMessage( mi_ServerList, LVS_ADDCOLUMN, "Ping", tLXOptions->iInternetList[4]);
	cInternet.SendMessage( mi_ServerList, LVS_ADDCOLUMN, "Address", tLXOptions->iInternetList[5]);

	// Clear the server list & grab an update
	Menu_SvrList_Clear();

    // Load the list
    Menu_SvrList_LoadList("cfg/svrlist.dat");
    Menu_SvrList_FillList( (CListview *)cInternet.getWidget( mi_ServerList ) );

	return true;
}


///////////////////
// Shutdown the internet menu
void Menu_Net_NETShutdown(void)
{
    // Save the list
    if( iNetMode == net_internet )  {
        Menu_SvrList_SaveList("cfg/svrlist.dat");

		// Save the column widths
		for (int i=0;i<6;i++)
			tLXOptions->iInternetList[i] = cInternet.SendMessage(mi_ServerList,LVM_GETCOLUMNWIDTH,i,0);
	}

	// Save the selected player
	cb_item_t *item = (cb_item_t *)cInternet.SendMessage(mi_PlayerSelection,CBM_GETCURITEM,(DWORD)0,0);
	if (item)
		tLXOptions->tGameinfo.iLastSelectedPlayer = item->iIndex;

	cInternet.Shutdown();
}


///////////////////
// Net Internet frame
void Menu_Net_NETFrame(int mouse)
{
	mouse_t		*Mouse = GetMouse();
	gui_event_t *ev = NULL;
	static std::string	addr;


	// Process & Draw the gui
	if (!cMediaPlayer.GetDrawPlayer())
		ev = cInternet.Process();
	cInternet.Draw( tMenu->bmpScreen );


	// Process the server list
	if( Menu_SvrList_Process() ) {
		// Add the servers to the listview
		Menu_SvrList_FillList( (CListview *)cInternet.getWidget( mi_ServerList ) );
	}



	// Process any events
	if(ev) {

		// Mouse type
		if(ev->cWidget->getType() == wid_Button)
			mouse = 1;
		if(ev->cWidget->getType() == wid_Textbox)
			mouse = 2;
		if(ev->cWidget->getType() == wid_Listview)
			mouse = ((CListview *)(ev->cWidget))->getCursor();

		switch(ev->iControlID) {

			// Add Server
			case mi_AddServer:
				if(ev->iEventMsg == BTN_MOUSEUP) {
					// Click!
					PlaySoundSample(sfxGeneral.smpClick);

					Menu_Net_NETAddServer();
				}
				break;

			// Back
			case mi_Back:
				if(ev->iEventMsg == BTN_MOUSEUP) {

					// Shutdown
					Menu_Net_NETShutdown();

					// Click!
					PlaySoundSample(sfxGeneral.smpClick);

					// Back to main menu
					Menu_MainInitialize();
				}
				break;

			// Refresh
			case mi_Refresh:
				if(ev->iEventMsg == BTN_MOUSEUP) {

					// Click!
					PlaySoundSample(sfxGeneral.smpClick);

					// Refresh the currently visible servers
					Menu_SvrList_RefreshList();
					Menu_SvrList_FillList( (CListview *)cInternet.getWidget( mi_ServerList ) );
				}
				break;

			// Join
			case mi_Join:
                if(ev->iEventMsg == BTN_MOUSEUP) {

					addr = "";
					int result = cInternet.SendMessage(mi_ServerList, LVS_GETCURSINDEX, &addr, 0);
					if(result != -1 && addr != "") {

                        // Save the list
                        Menu_SvrList_SaveList("cfg/svrlist.dat");

						// Click!
						PlaySoundSample(sfxGeneral.smpClick);

						lv_subitem_t *sub = ((CListview *)cInternet.getWidget(mi_ServerList))->getCurSubitem(1);

						// Join
						if (sub)
							Menu_Net_NETJoinServer(addr,sub->sText);
						return;
					}
				}
				break;

			// Serverlist
			case mi_ServerList:

                // Double click
				if(ev->iEventMsg == LV_DOUBLECLK) {

					/*
					  Now.... Should a double click refresh the server (like tribes)?
					  Or should it join the server like other games???
					*/

					// Just join for the moment
					addr = "";
					int result = cInternet.SendMessage(mi_ServerList, LVS_GETCURSINDEX, &addr, 0);
					lv_subitem_t *sub = ((CListview *)cInternet.getWidget(mi_ServerList))->getCurSubitem(1);
					if(result != -1 && addr != "" && sub) {
                        // Save the list
                        Menu_SvrList_SaveList("cfg/svrlist.dat");

						Menu_Net_NETJoinServer(addr,sub->sText);
						return;
					}
				}

                // Right click
                if( ev->iEventMsg == LV_RIGHTCLK ) {
                    addr = "";
					int result = cInternet.SendMessage(mi_ServerList, LVS_GETCURSINDEX, &addr, 0);
					if(result && addr != "") {
                        // Display a menu
                        szNetCurServer = addr;
                        mouse_t *m = GetMouse();

                        cInternet.Add( new CMenu(m->X, m->Y), mi_PopupMenu, 0,0, 640,480 );
                        cInternet.SendMessage( mi_PopupMenu, MNS_ADDITEM, "Delete server",				0 );
                        cInternet.SendMessage( mi_PopupMenu, MNS_ADDITEM, "Refresh server",				1 );
                        cInternet.SendMessage( mi_PopupMenu, MNS_ADDITEM, "Join server",				2 );
						cInternet.SendMessage( mi_PopupMenu, MNS_ADDITEM, "Add to favourites",			3 );
						cInternet.SendMessage( mi_PopupMenu, MNS_ADDITEM, "Send \"I want join message\"",4 );
						cInternet.SendMessage( mi_PopupMenu, MNS_ADDITEM, "Copy IP to clipboard",		5 );
                        cInternet.SendMessage( mi_PopupMenu, MNS_ADDITEM, "Server details",				6 );
                    }
                }


				// Enter key
				if( ev->iEventMsg == LV_ENTER )  {
					// Join
					addr = "";
					int result = cInternet.SendMessage(mi_ServerList, LVS_GETCURSINDEX, &addr, 0);
					lv_subitem_t *sub = ((CListview *)cInternet.getWidget(mi_ServerList))->getCurSubitem(1);
					if(result != -1 && addr != "" && sub) {
                        // Save the list
                        Menu_SvrList_SaveList("cfg/svrlist.dat");

						Menu_Net_NETJoinServer(addr,sub->sText);
						return;
					}
				}

				// Delete
				if( ev->iEventMsg == LV_DELETE )  {
					addr = "";
					int result = cInternet.SendMessage(mi_ServerList, LVS_GETCURSINDEX, &addr, 0);
					if(result && addr != "") {
						Menu_SvrList_RemoveServer(addr);
						// Re-Fill the server list
						Menu_SvrList_FillList( (CListview *)cInternet.getWidget( mi_ServerList ) );
					}
				}
				break;

            // Popup menu
            case mi_PopupMenu:
                switch( ev->iEventMsg ) {
                    // Delete the server
                    case MNU_USER+0:
                        Menu_SvrList_RemoveServer(szNetCurServer);
                        break;

                    // Refresh the server
                    case MNU_USER+1:
                        {
                            server_t *sv = Menu_SvrList_FindServerStr(szNetCurServer);
                            if(sv)
                                Menu_SvrList_RefreshServer(sv);
                        }
                        break;

                    // Join a server
                    case MNU_USER+2:  {
                        // Save the list
                        Menu_SvrList_SaveList("cfg/svrlist.dat");
						lv_subitem_t *sub = ((CListview *)cInternet.getWidget(mi_ServerList))->getCurSubitem(1);
						if (sub)
							Menu_Net_NETJoinServer(szNetCurServer,sub->sText);
						}
                        return;

                    // Add server to favourites
                    case MNU_USER+3:
						{
							server_t *sv = Menu_SvrList_FindServerStr(szNetCurServer);
							if (sv)
								Menu_SvrList_AddFavourite(sv->szName,sv->szAddress);
						}
                        break;

					// Send a "wants join" message
                    case MNU_USER+4:
						{
							server_t *sv = Menu_SvrList_FindServerStr(szNetCurServer);
							static std::string Nick;
							cInternet.SendMessage(mi_PlayerSelection, CBS_GETCURNAME, &Nick, 0);
							if (sv)
								Menu_SvrList_WantsJoin(Nick, sv);
						}
                        break;

					// Copy the IP to clipboard
					case MNU_USER+5:
						{
							SetClipboardText(szNetCurServer);
						}
						break;

                    // Show server details
                    case MNU_USER+6:
						cInternet.removeWidget(mi_PopupMenu);
                        Menu_Net_NETShowServer(szNetCurServer);
                        break;
                }

                // Re-Fill the server list
                Menu_SvrList_FillList( (CListview *)cInternet.getWidget( mi_ServerList ) );

                // Remove the menu widget
                cInternet.SendMessage( mi_PopupMenu, MNM_REDRAWBUFFER, (DWORD)0, 0);
                cInternet.removeWidget(mi_PopupMenu);
                break;

			// Update server list
			case mi_UpdateList:
				if(ev->iEventMsg == BTN_MOUSEUP) {
					// Click!
					PlaySoundSample(sfxGeneral.smpClick);

					Menu_Net_NETUpdateList();
				}
				break;
		}

	}

	// Draw the mouse
	if (mouse !=3)
		DrawImage(tMenu->bmpScreen,gfxGUI.bmpMouse[mouse], Mouse->X,Mouse->Y);
	else
		DrawImage(tMenu->bmpScreen,gfxGUI.bmpMouse[mouse], Mouse->X-(gfxGUI.bmpMouse[mouse]->w/2),Mouse->Y-(gfxGUI.bmpMouse[mouse]->h/2));

}


///////////////////
// Join a server
void Menu_Net_NETJoinServer(const std::string& sAddress, const std::string& sName)
{
	tGameInfo.iNumPlayers = 1;

	// Fill in the game structure
	cb_item_t *item = (cb_item_t *)cInternet.SendMessage(mi_PlayerSelection,CBM_GETCURITEM,(DWORD)0,0);

	// Add the player to the list
	if (item)  {
		profile_t *ply = FindProfile(item->iIndex);
		if(ply)
			tGameInfo.cPlayers[0] = ply;
	}

	if(item->iIndex < 0)
		item->iIndex = 0;
	tLXOptions->tGameinfo.iLastSelectedPlayer = item->iIndex;

	cClient->setServerName(sName);

	// Shutdown
	cInternet.Shutdown();

	iNetMode = net_join;

	tMenu->iReturnTo = net_internet;

	// Connect to the server
	Menu_Net_JoinConnectionInitialize(sAddress);
}



extern CButton	cNetButtons[5];

///////////////////
// Show an 'add server' box to enter in an address
enum  {
	na_Cancel=0,
	na_Add,
	na_Address
};

void Menu_Net_NETAddServer(void)
{
	CGuiLayout	cAddSvr;
	int			mouse = 0;
	gui_event_t *ev = NULL;
	mouse_t		*Mouse = GetMouse();
	bool		addServerMsg = true;


	// Create the background
	cInternet.Draw( tMenu->bmpBuffer );
	Menu_DrawBox(tMenu->bmpBuffer, 200, 220, 440, 340);
	//DrawImageAdv(tMenu->bmpBuffer, tMenu->bmpMainBack, 202,222, 202,222, 237,117);
    DrawRectFill(tMenu->bmpBuffer, 202,222,439,339,0);
	for(int i=1;i<5;i++)
		cNetButtons[i].Draw(tMenu->bmpBuffer);
	Menu_RedrawMouse(true);


	cAddSvr.Initialize();
	cAddSvr.Add( new CButton(BUT_ADD, tMenu->bmpButtons),	na_Add, 220, 320, 40,15);
	cAddSvr.Add( new CButton(BUT_CANCEL, tMenu->bmpButtons),na_Cancel, 350, 320, 70,15);
	cAddSvr.Add( new CLabel("Add a server", tLX->clNormalLabel),		-1,275, 225, 0, 0);
	cAddSvr.Add( new CLabel("Address", tLX->clNormalLabel),				-1,215, 267, 0, 0);
	cAddSvr.Add( new CTextbox(),							na_Address, 280, 265, 140, 20);

	cAddSvr.SendMessage(2,TXM_SETMAX,21,0);


	while(!GetKeyboard()->KeyUp[SDLK_ESCAPE] && addServerMsg && tMenu->iMenuRunning) {
		mouse = 0;
		Menu_RedrawMouse(false);
		ProcessEvents();
		DrawImageAdv(tMenu->bmpScreen,tMenu->bmpBuffer, 200,220, 200,220, 240, 240);

		// Process the server list
		if( Menu_SvrList_Process() ) {
			// Add the servers to the listview
			Menu_SvrList_FillList( (CListview *)cInternet.getWidget( mi_ServerList ) );
		}


		cAddSvr.Draw( tMenu->bmpScreen );
		ev = cAddSvr.Process();

		// Process any events
		if(ev) {

			// Mouse type
			if(ev->cWidget->getType() == wid_Button)
				mouse = 1;
			if(ev->cWidget->getType() == wid_Textbox)
				mouse = 2;

			switch(ev->iControlID) {

				// Add
				case na_Add:
					if(ev->iEventMsg == BTN_MOUSEUP) {

						static std::string addr;
						cAddSvr.SendMessage(na_Address, TXS_GETTEXT, &addr, 0);

						Menu_SvrList_AddServer(addr, true);
						Menu_SvrList_FillList( (CListview *)cInternet.getWidget( mi_ServerList ) );

						// Click!
						PlaySoundSample(sfxGeneral.smpClick);

						addServerMsg = false;
					}
					break;

				// Cancel
				case na_Cancel:
					if(ev->iEventMsg == BTN_MOUSEUP) {
						// Click!
						PlaySoundSample(sfxGeneral.smpClick);

						addServerMsg = false;
					}
					break;
			}
		}


		DrawImage(tMenu->bmpScreen,gfxGUI.bmpMouse[mouse], Mouse->X,Mouse->Y);
		FlipScreen(tMenu->bmpScreen);
	}


	cAddSvr.Shutdown();

	// Re-draw the background
	DrawImage(tMenu->bmpBuffer,tMenu->bmpMainBack_wob,0,0);
	Menu_DrawSubTitle(tMenu->bmpBuffer,SUB_NETWORK);
    Menu_DrawBox(tMenu->bmpBuffer, 15,130, 625, 465);
	Menu_RedrawMouse(true);
}



///////////////////
// Update the server list
void Menu_Net_NETUpdateList(void)
{
	CGuiLayout	cListUpdate;
	int			mouse = 0;
	gui_event_t *ev = NULL;
	mouse_t		*Mouse = GetMouse();
	bool		updateList = true;
	int			http_result = 0;

    //
    // Get the number of master servers for a progress bar
    //
    int SvrCount = 0;
    int CurServer = 0;
    bool SentRequest = false;
    static char szLine[1024];
    FILE *fp = OpenGameFile("cfg/masterservers.txt","rt");
    if( !fp )
        return;

    while( fgets(szLine, 1024, fp) ) {
        fix_strncpy( szLine, StripLine(szLine));

        if( fix_strnlen(szLine) > 0 )
            SvrCount++;
    }


    // Clear the server list
    Menu_SvrList_ClearAuto();

    // Back to the start
    fseek(fp,0,SEEK_SET);

	// Create the background
	cInternet.Draw( tMenu->bmpBuffer );
	Menu_DrawBox(tMenu->bmpBuffer, 200, 220, 440, 340);
	//DrawImageAdv(tMenu->bmpBuffer, tMenu->bmpMainBack, 202,222, 202,222, 237,117);
    DrawRectFill(tMenu->bmpBuffer, 202, 222, 439, 339, 0);
    Menu_DrawBox(tMenu->bmpBuffer, 220, 280, 420, 300);
	for(int i=0;i<4;i++)
		cNetButtons[i].Draw(tMenu->bmpBuffer);
	Menu_RedrawMouse(true);


	cListUpdate.Initialize();
	cListUpdate.Add( new CButton(BUT_CANCEL, tMenu->bmpButtons),0, 285, 320, 70,15);
	cListUpdate.Add( new CLabel("Getting server list", tLX->clNormalLabel),	-1,260, 227, 0, 0);


	while(!GetKeyboard()->KeyUp[SDLK_ESCAPE] && updateList && tMenu->iMenuRunning) {
		tLX->fCurTime = GetMilliSeconds();

		mouse = 0;
		Menu_RedrawMouse(false);
		ProcessEvents();
		DrawImageAdv(tMenu->bmpScreen,tMenu->bmpBuffer, 200,220, 200,220, 240, 240);

        if( SvrCount > 0 ) {
            DrawRectFill(tMenu->bmpScreen, 222,282, (int) (222+((float)CurServer/(float)SvrCount)*200.0f), 299, MakeColour(0,136,250));
            tLX->cOutlineFont.DrawCentre(tMenu->bmpScreen, 320,283,0xffff,"%d/%d", CurServer,SvrCount);
        }

        // Do the HTTP requests of the master servers
        if( !SentRequest ) {

            // Have we gone through all the servers?
            if( CurServer >= SvrCount )
                break;

            // Get the next server in the list
            while( fgets(szLine, 1024, fp) ) {
                fix_strncpy( szLine, StripLine(szLine) );

                if( fix_strnlen(szLine) > 0 ) {

                    // Send the request
                    if( !http_InitializeRequest(szLine, LX_SVRLIST) ) {

                        // Clear the data for another request
		                SentRequest = false;
                        CurServer++;
                    } else
                        SentRequest = true;

                    break;
                }
            }
        }


        // Process the http request
        if( SentRequest ) {
			static std::string szError;
            http_result = http_ProcessRequest(&szError);

            // Parse the list if the request was successful
            if( http_result == 1 ) {
		        Menu_Net_NETParseList();
            } else if( http_result == -1 )
            	printf("HTTP ERROR: %s\n", szError.c_str());

            if( http_result != 0 ) {
                SentRequest = false;
                CurServer++;
                http_Quit();
            }
        }

		// Process the server list
		if( Menu_SvrList_Process() ) {
			// Add the servers to the listview
			Menu_SvrList_FillList( (CListview *)cInternet.getWidget( mi_ServerList ) );
		}


		cListUpdate.Draw( tMenu->bmpScreen );
		ev = cListUpdate.Process();

		// Process any events
		if(ev) {

			// Mouse type
			if(ev->cWidget->getType() == wid_Button)
				mouse = 1;
			if(ev->cWidget->getType() == wid_Textbox)
				mouse = 2;

			switch(ev->iControlID) {

				// Cancel
				case 0:
					if(ev->iEventMsg == BTN_MOUSEUP) {

						// Click!
						PlaySoundSample(sfxGeneral.smpClick);

						http_result = 0;
						updateList = false;
					}
					break;
			}
		}


		DrawImage(tMenu->bmpScreen,gfxGUI.bmpMouse[mouse], Mouse->X,Mouse->Y);
		FlipScreen(tMenu->bmpScreen);
	}

	cListUpdate.Shutdown();


	Menu_SvrList_FillList( (CListview *)cInternet.getWidget( mi_ServerList ) );


	// Re-draw the background
	DrawImage(tMenu->bmpBuffer,tMenu->bmpMainBack_wob,0,0);
    Menu_DrawBox(tMenu->bmpBuffer, 15,130, 625, 465);
	Menu_DrawSubTitle(tMenu->bmpBuffer,SUB_NETWORK);
	Menu_RedrawMouse(true);
}


///////////////////
// Parse the downloaded server list
void Menu_Net_NETParseList(void)
{
	char *content = http_GetContent();

	static char Line[HTTP_CONTENT_LEN];
	static char temp[HTTP_CONTENT_LEN];


	int i = 0;
	int count = strnlen(content, HTTP_CONTENT_LEN);

	while(i < count) {
		sscanf(content+i,"%[^\n]\n",Line);

		fix_strncpy(temp, Line);

		char *addr = strtok(temp, ",");
		char *prt = strtok(NULL, ",");

		// Check if the tokens are valid
		if(!addr || !prt) {
			i += fix_strnlen(Line)+1;
			continue;
		}


		static char address[256];
		static char port[256];
        static char a[256], p[256];

		fix_strncpy(a, TrimSpaces(addr));
		fix_strncpy(p, TrimSpaces(prt));

		// If the address, or port does NOT have quotes around it, the line must be mangled and cannot be used
		if(a[0] != '\"' || p[0] != '\"') {
			i += fix_strnlen(Line)+1;
			continue;
		}
		if(a[fix_strnlen(a)-1] != '\"' || p[fix_strnlen(p)-1] != '\"') {
			i += fix_strnlen(Line)+1;
			continue;
		}


		StripQuotes(address, a);
		StripQuotes(port, p);

		// Create the server address
		snprintf(temp, sizeof(temp), "%s:%s",address,port); fix_markend(temp);
		Menu_SvrList_AddServer(temp, false);

		i += fix_strnlen(Line)+1;
	}
}

enum  {
	nd_Ok=0,
	nd_Refresh
};

///////////////////
// Show a server's details
void Menu_Net_NETShowServer(const std::string& szAddress)
{
    mouse_t     *Mouse = GetMouse();
    int         nMouseCur = 0;
    CGuiLayout  cDetails;

    // Create the buffer
    DrawImage(tMenu->bmpBuffer,tMenu->bmpMainBack_wob,0,0);
    Menu_DrawBox(tMenu->bmpBuffer, 15,130, 625, 465);
	Menu_DrawSubTitle(tMenu->bmpBuffer,SUB_NETWORK);
	cInternet.Draw(tMenu->bmpBuffer);

	for(int i=1;i<4;i++)
		cNetButtons[i].Draw(tMenu->bmpBuffer);

	Menu_RedrawMouse(true);

    cDetails.Initialize();
	cDetails.Add( new CButton(BUT_REFRESH, tMenu->bmpButtons),  nd_Refresh,		200,400, 85,15);
    cDetails.Add( new CButton(BUT_OK, tMenu->bmpButtons),	    nd_Ok,      310,400, 40,15);

	bGotDetails = false;
	bOldLxBug = false;
	nTries = 0;
	fStart = -9999;

	DrawRectFillA(tMenu->bmpBuffer,200,400,350,420,0,230); // Dirty; because of button redrawing

    while(!GetKeyboard()->KeyUp[SDLK_ESCAPE] && tMenu->iMenuRunning) {
		tLX->fCurTime = GetMilliSeconds();

		nMouseCur = 0;
		Menu_RedrawMouse(false);
		ProcessEvents();
		//DrawImageAdv(tMenu->bmpScreen,tMenu->bmpBuffer, 200,220, 200,220, 240, 240);

		cMediaPlayer.Frame();

		Menu_SvrList_DrawInfo(szAddress);

        cDetails.Draw(tMenu->bmpScreen);
        gui_event_t *ev = NULL;
		if (!cMediaPlayer.GetDrawPlayer())
			ev = cDetails.Process();
        if(ev) {
            if(ev->cWidget->getType() == wid_Button)
                nMouseCur = 1;

			// Ok
            if(ev->iControlID == nd_Ok && ev->iEventMsg == BTN_MOUSEUP) {
                break;
			// Refresh
            } else if (ev->iControlID == nd_Refresh && ev->iEventMsg == BTN_MOUSEUP)  {
				fStart = -9999;
				bGotDetails = false;
				bOldLxBug = false;
				nTries = 0;
			}
        }

		cMediaPlayer.Draw(tMenu->bmpScreen);

        DrawImage(tMenu->bmpScreen,gfxGUI.bmpMouse[nMouseCur], Mouse->X,Mouse->Y);
		FlipScreen(tMenu->bmpScreen);
    }

	cDetails.Shutdown();


    // Redraw the background
	DrawImage(tMenu->bmpBuffer,tMenu->bmpMainBack_wob,0,0);
    Menu_DrawBox(tMenu->bmpBuffer, 15,130, 625, 465);
	Menu_DrawSubTitle(tMenu->bmpBuffer,SUB_NETWORK);
	Menu_RedrawMouse(true);
}
