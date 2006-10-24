/////////////////////////////////////////
//
//             Liero Xtreme
//
//     Copyright Auxiliary Software 2002
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
char        szNetCurServer[128];

// Lan widgets
enum {
	Join=0,
	ServerList,
	Refresh,
	UpdateList,
	AddServer,
	Back,
    PopupMenu,
	PlayerSelection
};


///////////////////
// Initialize the Internet menu
int Menu_Net_NETInitialize(void)
{
	Uint32 blue = MakeColour(0,138,251);

	iNetMode = net_internet;
    szNetCurServer[0] = '\0';

	cInternet.Shutdown();
	cInternet.Initialize();
	
	cInternet.Add( new CListview(),								ServerList, 40, 180, 560, 240);
	cInternet.Add( new CButton(BUT_BACK, tMenu->bmpButtons),    Back,       25, 440, 50,  15);
	cInternet.Add( new CButton(BUT_ADD, tMenu->bmpButtons),		AddServer,  140,440, 40,  15);
	cInternet.Add( new CButton(BUT_REFRESH, tMenu->bmpButtons), Refresh,	250,440, 83,  15);	
	cInternet.Add( new CButton(BUT_UPDATELIST, tMenu->bmpButtons),	UpdateList,  390,440, 125,  15);
	cInternet.Add( new CButton(BUT_JOIN, tMenu->bmpButtons),    Join,		570,440, 43,  15);
	cInternet.Add( new CLabel("Select player:",0xffff),-1,		125, 152, 180,15);
	cInternet.Add( new CCombobox(),								PlayerSelection,		225,150, 170,  19);


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

		cInternet.SendMessage( PlayerSelection, CBM_ADDITEM, p->iID, (DWORD)p->sName);
		cInternet.SendMessage( PlayerSelection, CBM_SETIMAGE, p->iID, (DWORD)p->bmpWorm);
	}

	cInternet.SendMessage( PlayerSelection, CBM_SETCURINDEX, tLXOptions->tGameinfo.iLastSelectedPlayer, 0);

    Menu_redrawBufferRect(0, 0, 640, 480);

	cInternet.SendMessage( ServerList, LVM_ADDCOLUMN, (DWORD)"", 32);
	cInternet.SendMessage( ServerList, LVM_ADDCOLUMN, (DWORD)"Server Name", 180);
	cInternet.SendMessage( ServerList, LVM_ADDCOLUMN, (DWORD)"State", 70);
	cInternet.SendMessage( ServerList, LVM_ADDCOLUMN, (DWORD)"Players", 80);
	cInternet.SendMessage( ServerList, LVM_ADDCOLUMN, (DWORD)"Ping", 60);
	cInternet.SendMessage( ServerList, LVM_ADDCOLUMN, (DWORD)"Address", 150);	

	// Clear the server list & grab an update
	Menu_SvrList_Clear();

    // Load the list
    Menu_SvrList_LoadList("cfg/svrlist.dat");
    Menu_SvrList_FillList( (CListview *)cInternet.getWidget( ServerList ) );

	return true;
}


///////////////////
// Shutdown the internet menu
void Menu_Net_NETShutdown(void)
{
    // Save the list
    if( iNetMode == net_internet )
        Menu_SvrList_SaveList("cfg/svrlist.dat");

	// Save the selected player
	cb_item_t *item = (cb_item_t *)cInternet.SendMessage(PlayerSelection,CBM_GETCURITEM,0,0);
	if (item)
		tLXOptions->tGameinfo.iLastSelectedPlayer = item->iIndex;

	cInternet.Shutdown();
}


///////////////////
// Net Internet frame
void Menu_Net_NETFrame(int mouse)
{
	mouse_t		*Mouse = GetMouse();
	gui_event_t *ev;
	char		addr[256];
	

	// Process & Draw the gui
	ev = cInternet.Process();
	cInternet.Draw( tMenu->bmpScreen );


	// Process the server list
	if( Menu_SvrList_Process() ) {
		// Add the servers to the listview
		Menu_SvrList_FillList( (CListview *)cInternet.getWidget( ServerList ) );
	}



	// Process any events
	if(ev) {

		// Mouse type
		if(ev->cWidget->getType() == wid_Button)
			mouse = 1;
		if(ev->cWidget->getType() == wid_Textbox)
			mouse = 2;


		switch(ev->iControlID) {

			// Add Server
			case AddServer:
				if(ev->iEventMsg == BTN_MOUSEUP) {
					// Click!
					BASS_SamplePlay(sfxGeneral.smpClick);

					Menu_Net_NETAddServer();
				}
				break;

			// Back
			case Back:
				if(ev->iEventMsg == BTN_MOUSEUP) {

					// Save the selected player
					cb_item_t *item = (cb_item_t *)cInternet.SendMessage(PlayerSelection,CBM_GETCURITEM,0,0);
					if (item)
						tLXOptions->tGameinfo.iLastSelectedPlayer = item->iIndex;

                    // Save the list
                    Menu_SvrList_SaveList("cfg/svrlist.dat");

					// Click!
					BASS_SamplePlay(sfxGeneral.smpClick);

					// Shutdown
					cInternet.Shutdown();

					// Back to main menu					
					Menu_MainInitialize();
				}
				break;

			// Refresh
			case Refresh:
				if(ev->iEventMsg == BTN_MOUSEUP) {

					// Click!
					BASS_SamplePlay(sfxGeneral.smpClick);

					// Refresh the currently visible servers
					Menu_SvrList_RefreshList();
					Menu_SvrList_FillList( (CListview *)cInternet.getWidget( ServerList ) );
				}
				break;

			// Join
			case Join:
                if(ev->iEventMsg == BTN_MOUSEUP) {

					addr[0] = 0;
					int result = cInternet.SendMessage(ServerList, LVM_GETCURSINDEX, (DWORD)addr, sizeof(addr));
					if(result != -1 && addr[0]) {

                        // Save the list
                        Menu_SvrList_SaveList("cfg/svrlist.dat");
						
						// Click!
						BASS_SamplePlay(sfxGeneral.smpClick);

						// Join
						Menu_Net_NETJoinServer(addr);
						return;
					}
				}
				break;

			// Serverlist
			case ServerList:

                // Double click
				if(ev->iEventMsg == LV_DOUBLECLK) {

					/*
					  Now.... Should a double click refresh the server (like tribes)?
					  Or should it join the server like other games???
					*/

					// Just join for the moment
					addr[0] = 0;
					int result = cInternet.SendMessage(ServerList, LVM_GETCURSINDEX, (DWORD)addr, sizeof(addr));
					if(result != -1 && addr[0]) {
                        // Save the list
                        Menu_SvrList_SaveList("cfg/svrlist.dat");

						Menu_Net_NETJoinServer(addr);
						return;
					}
				}

                // Right click
                if( ev->iEventMsg == LV_RIGHTCLK ) {
                    addr[0] = 0;
					int result = cInternet.SendMessage(ServerList, LVM_GETCURSINDEX, (DWORD)addr, sizeof(addr));
					if(result && addr[0]) {
                        // Display a menu
                        strcpy(szNetCurServer, addr);
                        mouse_t *m = GetMouse();
                        
                        cInternet.Add( new CMenu(m->X, m->Y), PopupMenu, 0,0, 640,480 );
                        cInternet.SendMessage( PopupMenu, MNM_ADDITEM, 0, (DWORD)"Delete server" );
                        cInternet.SendMessage( PopupMenu, MNM_ADDITEM, 1, (DWORD)"Refresh server" );
                        cInternet.SendMessage( PopupMenu, MNM_ADDITEM, 2, (DWORD)"Join server" );
						cInternet.SendMessage( PopupMenu, MNM_ADDITEM, 3, (DWORD)"Add to favourites" );
						cInternet.SendMessage( PopupMenu, MNM_ADDITEM, 4, (DWORD)"Send \"I want join message\"" );
                        cInternet.SendMessage( PopupMenu, MNM_ADDITEM, 5, (DWORD)"Server details" );
                    }
                }
				break;

            // Popup menu
            case PopupMenu:
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
                    case MNU_USER+2:
                        // Save the list
                        Menu_SvrList_SaveList("cfg/svrlist.dat");
						Menu_Net_NETJoinServer(szNetCurServer);
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
							char Nick[256];
							cInternet.SendMessage(PlayerSelection, CBM_GETCURNAME, (DWORD)Nick, sizeof(Nick));
							Nick[255] = '\0'; // safety
							char *sNick = Nick;
							if (sv)
								Menu_SvrList_WantsJoin(sNick, sv);
						}
                        break;

                    // Show server details
                    case MNU_USER+5:
                        Menu_Net_NETShowServer(szNetCurServer);
                        break;
                }

                // Re-Fill the server list
                Menu_SvrList_FillList( (CListview *)cInternet.getWidget( ServerList ) );

                // Remove the menu widget
                cInternet.SendMessage( PopupMenu, MNM_REDRAWBUFFER, 0, 0);
                cInternet.removeWidget(PopupMenu);
                break;

			// Update server list
			case UpdateList:
				if(ev->iEventMsg == BTN_MOUSEUP) {
					// Click!
					BASS_SamplePlay(sfxGeneral.smpClick);

					Menu_Net_NETUpdateList();
				}
				break;
		}

	}

	// Draw the mouse
	DrawImage(tMenu->bmpScreen,gfxGUI.bmpMouse[mouse], Mouse->X,Mouse->Y);

}


///////////////////
// Join a server
void Menu_Net_NETJoinServer(char *sAddress)
{
	tGameInfo.iNumPlayers = 1;

	// Fill in the game structure												
	cb_item_t *item = (cb_item_t *)cInternet.SendMessage(PlayerSelection,CBM_GETCURITEM,0,0);
		
	// Add the player to the list
	if (item)  {
		profile_t *ply = FindProfile(item->iIndex);
		if(ply)		
			tGameInfo.cPlayers[0] = ply;
	}

	if(item->iIndex < 0)
		item->iIndex = 0;
	tLXOptions->tGameinfo.iLastSelectedPlayer = item->iIndex;

	// Shutdown
	cInternet.Shutdown();

	iNetMode = net_join;

	// Connect to the server
	Menu_Net_JoinConnectionInitialize(sAddress);
}



extern CButton	cNetButtons[5];

///////////////////
// Show an 'add server' box to enter in an address
void Menu_Net_NETAddServer(void)
{
	CGuiLayout	cAddSvr;
	int			mouse = 0;
	gui_event_t *ev;
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
	cAddSvr.Add( new CButton(BUT_ADD, tMenu->bmpButtons),	0, 220, 320, 40,15);
	cAddSvr.Add( new CButton(BUT_CANCEL, tMenu->bmpButtons),1, 350, 320, 70,15);
	cAddSvr.Add( new CLabel("Add a server", 0xffff),		-1,275, 225, 0, 0);
	cAddSvr.Add( new CLabel("Address", 0xffff),				-1,215, 267, 0, 0);
	cAddSvr.Add( new CTextbox(),							2, 280, 265, 140, 20);

	cAddSvr.SendMessage(2,TXM_SETMAX,21,0);


	while(!GetKeyboard()->KeyUp[SDLK_ESCAPE] && addServerMsg) {
		mouse = 0;
		Menu_RedrawMouse(false);
		ProcessEvents();
		DrawImageAdv(tMenu->bmpScreen,tMenu->bmpBuffer, 200,220, 200,220, 240, 240);

		// Process the server list
		if( Menu_SvrList_Process() ) {
			// Add the servers to the listview
			Menu_SvrList_FillList( (CListview *)cInternet.getWidget( ServerList ) );
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
				case 0:
					if(ev->iEventMsg == BTN_MOUSEUP) {

						char addr[512];
						cAddSvr.SendMessage(2, TXM_GETTEXT, (DWORD)addr, sizeof(addr));

						Menu_SvrList_AddServer(addr, true);
						Menu_SvrList_FillList( (CListview *)cInternet.getWidget( ServerList ) );

						// Click!
						BASS_SamplePlay(sfxGeneral.smpClick);

						addServerMsg = false;
					}
					break;

				// Cancel
				case 1:
					if(ev->iEventMsg == BTN_MOUSEUP) {
						// Click!
						BASS_SamplePlay(sfxGeneral.smpClick);

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
	gui_event_t *ev;
	mouse_t		*Mouse = GetMouse();
	bool		updateList = true;
	int			http_result = 0;

    //
    // Get the number of master servers for a progress bar
    //
    int SvrCount = 0;
    int CurServer = 0;
    bool SentRequest = false;
    char szLine[1024];
    FILE *fp = fopen("cfg/masterservers.txt","rt");
    if( !fp )
        return;

    while( fgets(szLine, 1024, fp) ) {
        strcpy( szLine, StripLine(szLine) );

        if( strlen(szLine) > 0 )
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
	for(int i=1;i<5;i++)
		cNetButtons[i].Draw(tMenu->bmpBuffer);
	Menu_RedrawMouse(true);


	cListUpdate.Initialize();
	cListUpdate.Add( new CButton(BUT_CANCEL, tMenu->bmpButtons),0, 285, 320, 70,15);	
	cListUpdate.Add( new CLabel("Getting server list", 0xffff),	-1,260, 227, 0, 0);	


	while(!GetKeyboard()->KeyUp[SDLK_ESCAPE] && updateList) {
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
                strcpy( szLine, StripLine(szLine) );
        
                if( strlen(szLine) > 0 ) {

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
            http_result = http_ProcessRequest(NULL);            

            // Parse the list if the request was successful
            if( http_result == 1 ) {
		        Menu_Net_NETParseList();
            }

            if( http_result != 0 ) {                
                SentRequest = false;
                CurServer++;
                http_Quit();
            }	        
        }

		// Process the server list
		if( Menu_SvrList_Process() ) {
			// Add the servers to the listview
			Menu_SvrList_FillList( (CListview *)cInternet.getWidget( ServerList ) );
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
						BASS_SamplePlay(sfxGeneral.smpClick);

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


	Menu_SvrList_FillList( (CListview *)cInternet.getWidget( ServerList ) );


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

	char Line[1024];
	char temp[1024];


	int i = 0;
	int count = strlen(content);

	while(i < count) {
		sscanf(content+i,"%[^\n]\n",Line);

		strcpy(temp, Line);

		char *addr = strtok(temp, ",");
		char *prt = strtok(NULL, ",");

		// Check if the tokens are valid
		if(!addr || !prt) {
			i += strlen(Line)+1;
			continue;
		}

		
		char address[256];
		char port[256];
        char a[256], p[256];

		strcpy(a, TrimSpaces(addr));
		strcpy(p, TrimSpaces(prt));

		// If the address, or port does NOT have quotes around it, the line must be mangled and cannot be used
		if(a[0] != '\"' || p[0] != '\"') {
			i += strlen(Line)+1;
			continue;
		}
		if(a[strlen(a)-1] != '\"' || p[strlen(p)-1] != '\"') {
			i += strlen(Line)+1;
			continue;
		}


		StripQuotes(address, a);
		StripQuotes(port, p);

		// Create the server address
		sprintf(temp, "%s:%s",address,port);		
		Menu_SvrList_AddServer(temp, false);

		i += strlen(Line)+1;
	}
}


///////////////////
// Show a server's details
void Menu_Net_NETShowServer(char *szAddress)
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
    
	Menu_SvrList_DrawInfo(szAddress);

	Menu_RedrawMouse(true);

    cDetails.Initialize();
    cDetails.Add( new CButton(BUT_OK, tMenu->bmpButtons),	    1,      260,400, 40,15);


    while(!GetKeyboard()->KeyUp[SDLK_ESCAPE]) {
		nMouseCur = 0;
		Menu_RedrawMouse(false);
		ProcessEvents();
		DrawImageAdv(tMenu->bmpScreen,tMenu->bmpBuffer, 200,220, 200,220, 240, 240);

        cDetails.Draw(tMenu->bmpScreen);
        gui_event_t *ev = cDetails.Process();
        if(ev) {
            if(ev->cWidget->getType() == wid_Button)
                nMouseCur = 1;

            if(ev->iControlID == 1 && ev->iEventMsg == BTN_MOUSEUP) {
                break;
            }
        }


        DrawImage(tMenu->bmpScreen,gfxGUI.bmpMouse[nMouseCur], Mouse->X,Mouse->Y);
		FlipScreen(tMenu->bmpScreen);	
    }


    // Redraw the background    
	DrawImage(tMenu->bmpBuffer,tMenu->bmpMainBack_wob,0,0);
    Menu_DrawBox(tMenu->bmpBuffer, 15,130, 625, 465);
	Menu_DrawSubTitle(tMenu->bmpBuffer,SUB_NETWORK);
	Menu_RedrawMouse(true);
}
