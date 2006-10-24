/////////////////////////////////////////
//
//             Liero Xtreme
//
//     Copyright Auxiliary Software 2002
//
//
/////////////////////////////////////////


// Net menu - Favourites
// Created 21/8/02
// Jason Boettcher


#include "defs.h"
#include "LieroX.h"
#include "Menu.h"


CGuiLayout	cFavourites;
char        szFavouritesCurServer[128];


// Widgets
enum {
	Join=0,
	ServerList,
	Refresh,
	Add,
	Back,
    PopupMenu,
	PlayerSelection
};



///////////////////
// Initialize the favourites menu
int Menu_Net_FavouritesInitialize(void)
{
	Uint32 blue = MakeColour(0,138,251);

	iNetMode = net_favourites;	

	cFavourites.Shutdown();
	cFavourites.Initialize();

	cFavourites.Add( new CButton(BUT_BACK, tMenu->bmpButtons),    Back,        25, 440, 50,  15);
	cFavourites.Add( new CButton(BUT_ADD, tMenu->bmpButtons),	  Add,		   190,440, 83,  15);
	cFavourites.Add( new CButton(BUT_REFRESH, tMenu->bmpButtons), Refresh,	   350,440, 83,  15);
	cFavourites.Add( new CButton(BUT_JOIN, tMenu->bmpButtons),    Join,	   570,440, 43,  15);
	cFavourites.Add( new CListview(),							   ServerList, 40, 180, 560, 240);
	cFavourites.Add( new CLabel("Select player:",0xffff),-1,		125, 152, 180,15);
	cFavourites.Add( new CCombobox(),								PlayerSelection,		225,150, 170,  19);


	// Fill the players box
	profile_t *p = GetProfiles();
	for(;p;p=p->tNext) {
		if(p->iType == PRF_COMPUTER)
			continue;

		cFavourites.SendMessage( PlayerSelection, CBM_ADDITEM, p->iID, (DWORD)p->sName);
		cFavourites.SendMessage( PlayerSelection, CBM_SETIMAGE, p->iID, (DWORD)p->bmpWorm);
	}

	cFavourites.SendMessage( PlayerSelection, CBM_SETCURINDEX, tLXOptions->tGameinfo.iLastSelectedPlayer, 0);

    Menu_redrawBufferRect(0, 0, 640, 480);


	/*
	  Server list columns

      Connection speed
	  Name
	  State
	  Players
	  Ping
	  Address
    */

	cFavourites.SendMessage( ServerList, LVM_ADDCOLUMN, (DWORD)"", 32);
	cFavourites.SendMessage( ServerList, LVM_ADDCOLUMN, (DWORD)"Server Name", 180);
	cFavourites.SendMessage( ServerList, LVM_ADDCOLUMN, (DWORD)"State", 70);
	cFavourites.SendMessage( ServerList, LVM_ADDCOLUMN, (DWORD)"Players", 80);
	cFavourites.SendMessage( ServerList, LVM_ADDCOLUMN, (DWORD)"Ping", 60);
	cFavourites.SendMessage( ServerList, LVM_ADDCOLUMN, (DWORD)"Address", 150);

	// Fill the server list
	Menu_SvrList_Clear();
	Menu_SvrList_LoadList("cfg/favourites.dat");
	Menu_SvrList_FillList( (CListview *)cFavourites.getWidget( ServerList ) );
	Menu_SvrList_RefreshList();

	return true;
}


///////////////////
// Shutdown the favourites menu
void Menu_Net_FavouritesShutdown(void)
{
	// Save the selected player
	cb_item_t *item = (cb_item_t *)cFavourites.SendMessage(PlayerSelection,CBM_GETCURITEM,0,0);
	if (item)
		tLXOptions->tGameinfo.iLastSelectedPlayer = item->iIndex;

	// Save the list
	if (iNetMode == net_favourites)
		Menu_SvrList_SaveList("cfg/favourites.dat");

	cFavourites.Shutdown();
}


///////////////////
// Net favourites frame
void Menu_Net_FavouritesFrame(int mouse)
{
	mouse_t		*Mouse = GetMouse();
	gui_event_t *ev;
	char		addr[256];
	

	// Process & Draw the gui
	ev = cFavourites.Process();
	cFavourites.Draw( tMenu->bmpScreen );

	// Process the server list
	if( Menu_SvrList_Process() ) {
		// Add the servers to the listview
		Menu_SvrList_FillList( (CListview *)cFavourites.getWidget( ServerList ) );
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
			case Back:
				if(ev->iEventMsg == BTN_MOUSEUP) {

					cb_item_t *item = (cb_item_t *)cFavourites.SendMessage(PlayerSelection,CBM_GETCURITEM,0,0);
					if (item)
						tLXOptions->tGameinfo.iLastSelectedPlayer = item->iIndex;

					// Click!
					BASS_SamplePlay(sfxGeneral.smpClick);

					// Save the list
					Menu_SvrList_SaveList("cfg/favourites.dat");

					// Shutdown
					cFavourites.Shutdown();

					// Back to main menu					
					Menu_MainInitialize();
				}
				break;

			// Add
			case Add:
				if(ev->iEventMsg == BTN_MOUSEUP) {

					// Click!
					BASS_SamplePlay(sfxGeneral.smpClick);

					Menu_Net_FavouritesAddServer();
				}
				break;

			// Refresh
			case Refresh:
				if(ev->iEventMsg == BTN_MOUSEUP) {

					// Click!
					BASS_SamplePlay(sfxGeneral.smpClick);

					// Send out a ping
					Menu_SvrList_RefreshList();
					Menu_SvrList_FillList( (CListview *)cFavourites.getWidget( ServerList ) );
				}
				break;

			// Join
			case Join:
				if(ev->iEventMsg == BTN_MOUSEUP) {				

					addr[0] = 0;
					int result = cFavourites.SendMessage(ServerList, LVM_GETCURSINDEX, (DWORD)addr, sizeof(addr));
					if(result != -1 && addr[0]) {
						
						// Click!
						BASS_SamplePlay(sfxGeneral.smpClick);

						// Join
						Menu_Net_FavouritesJoinServer(addr);
						return;
					}
				}
				break;

			// Serverlist
			case ServerList:
				if(ev->iEventMsg == LV_DOUBLECLK) {

					/*
					  Now.... Should a double click refresh the server (like tribes)?
					  Or should it join the server like other games???
					*/

					// Just join for the moment
					addr[0] = 0;
					int result = cFavourites.SendMessage(ServerList, LVM_GETCURSINDEX, (DWORD)addr, sizeof(addr));
					if(result != -1 && addr[0]) {
						Menu_Net_FavouritesJoinServer(addr);
						return;
					}
				}

                // Right click
                if( ev->iEventMsg == LV_RIGHTCLK ) {
                    addr[0] = 0;
					int result = cFavourites.SendMessage(ServerList, LVM_GETCURSINDEX, (DWORD)addr, sizeof(addr));
					if(result && addr[0]) {
                        // Display a menu
                        strcpy(szFavouritesCurServer, addr);
                        mouse_t *m = GetMouse();
                        
                        cFavourites.Add( new CMenu(m->X, m->Y), PopupMenu, 0,0, 640,480 );
                        cFavourites.SendMessage( PopupMenu, MNM_ADDITEM, 0, (DWORD)"Remove from favourites" );
						cFavourites.SendMessage( PopupMenu, MNM_ADDITEM, 1, (DWORD)"Rename server" );
                        cFavourites.SendMessage( PopupMenu, MNM_ADDITEM, 2, (DWORD)"Refresh server" );
                        cFavourites.SendMessage( PopupMenu, MNM_ADDITEM, 3, (DWORD)"Join server" );
						cFavourites.SendMessage( PopupMenu, MNM_ADDITEM, 4, (DWORD)"Send \"I want join\" message" );
                        cFavourites.SendMessage( PopupMenu, MNM_ADDITEM, 5, (DWORD)"Server details" );
                    }
                }
				break;

            // Popup menu
            case PopupMenu:
                switch( ev->iEventMsg ) {
                     // Remove server from favourites
                    case MNU_USER+0:
                        Menu_SvrList_RemoveServer(szFavouritesCurServer);
						Menu_SvrList_SaveList("cfg/favourites.dat");  // Save changes
                        break;

					// Rename server
                    case MNU_USER+1:  
						{
							// Remove the menu widget
							cFavourites.SendMessage(PopupMenu, MNM_REDRAWBUFFER, 0, 0);
							cFavourites.removeWidget(PopupMenu);

							server_t *sv = Menu_SvrList_FindServerStr(szFavouritesCurServer);
							Menu_Net_RenameServer(sv->szName);
							Menu_SvrList_SaveList("cfg/favourites.dat");  // Save changes
						}
                        break;

                    // Refresh the server
                    case MNU_USER+2:
                        {
                            server_t *sv = Menu_SvrList_FindServerStr(szFavouritesCurServer);
                            if(sv)
                                Menu_SvrList_RefreshServer(sv);
                        }
                        break;

                    // Join a server
                    case MNU_USER+3:
                        // Save the list
						Menu_Net_FavouritesJoinServer(szFavouritesCurServer);
                        return;

					// Send a "wants join" message
                    case MNU_USER+4:
						{
							server_t *sv = Menu_SvrList_FindServerStr(szFavouritesCurServer);
							char Nick[256];
							cFavourites.SendMessage(PlayerSelection, CBM_GETCURNAME, (DWORD)Nick, sizeof(Nick));
							Nick[255] = '\0'; // safety
							char *sNick = Nick;
							if (sv)
								Menu_SvrList_WantsJoin(sNick, sv);
						}
                        break;

                    // Show server details
                    case MNU_USER+5:
                        Menu_Net_FavouritesShowServer(szFavouritesCurServer);
                        break;
                }

                // Re-Fill the server list
                Menu_SvrList_FillList( (CListview *)cFavourites.getWidget( ServerList ) );

                // Remove the menu widget
                cFavourites.SendMessage(PopupMenu, MNM_REDRAWBUFFER, 0, 0);
                cFavourites.removeWidget(PopupMenu);
                break;
		}

	}


	// Draw the mouse
	DrawImage(tMenu->bmpScreen,gfxGUI.bmpMouse[mouse], Mouse->X,Mouse->Y);
}

///////////////////
// Join a server
void Menu_Net_FavouritesJoinServer(char *sAddress)
{
	tGameInfo.iNumPlayers = 1;

	// Fill in the game structure												
	cb_item_t *item = (cb_item_t *)cFavourites.SendMessage(PlayerSelection,CBM_GETCURITEM,0,0);
		
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
	cFavourites.Shutdown();

	iNetMode = net_join;

	// Save the list
	Menu_SvrList_SaveList("cfg/favourites.dat");

	// Connect to the server
	Menu_Net_JoinConnectionInitialize(sAddress);
}

///////////////////
// Show a server's details
void Menu_Net_FavouritesShowServer(char *szAddress)
{
    mouse_t     *Mouse = GetMouse();
    int         nMouseCur = 0;
    CGuiLayout  cDetails;

    // Create the buffer
    DrawImage(tMenu->bmpBuffer,tMenu->bmpMainBack_wob,0,0);
    Menu_DrawBox(tMenu->bmpBuffer, 15,130, 625, 465);
	Menu_DrawSubTitle(tMenu->bmpBuffer,SUB_NETWORK);
	cFavourites.Draw(tMenu->bmpBuffer);
  
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

///////////////////
// Show an 'rename server' box
void Menu_Net_RenameServer(char *szName)
{
	CGuiLayout	cRename;
	int			mouse = 0;
	gui_event_t *ev;
	mouse_t		*Mouse = GetMouse();
	bool		renameServerMsg = true;


	// Create the background
	cFavourites.Draw( tMenu->bmpBuffer );
	Menu_DrawBox(tMenu->bmpBuffer, 200, 210, 470, 340);
	//DrawImageAdv(tMenu->bmpBuffer, tMenu->bmpMainBack, 202,222, 202,222, 237,117);
    DrawRectFill(tMenu->bmpBuffer, 202,212,469,339,0);
	Menu_RedrawMouse(true);


	cRename.Initialize();
	cRename.Add( new CButton(BUT_OK, tMenu->bmpButtons),	0, 220, 310, 40,15);
	cRename.Add( new CButton(BUT_CANCEL, tMenu->bmpButtons),1, 350, 310, 70,15);
	cRename.Add( new CLabel("Rename a server", 0xffff),		-1,275, 225, 0, 0);
	cRename.Add( new CLabel("New name", 0xffff),			-1,215, 267, 0, 0);
	cRename.Add( new CTextbox(),							2, 300, 265, 150, 20);

	cRename.SendMessage(2,TXM_SETMAX,30,0);
	cRename.SendMessage(2,TXM_SETTEXT,(DWORD)szName,0); // Fill in the current server name


	while(!GetKeyboard()->KeyUp[SDLK_ESCAPE] && renameServerMsg) {
		mouse = 0;
		Menu_RedrawMouse(false);
		ProcessEvents();
		DrawImageAdv(tMenu->bmpScreen,tMenu->bmpBuffer, 200,220, 200,220, 240, 240);

		// Process the server list
		if( Menu_SvrList_Process() ) {
			// Add the servers to the listview
			Menu_SvrList_FillList( (CListview *)cFavourites.getWidget( ServerList ) );
		}


		cRename.Draw( tMenu->bmpScreen );
		ev = cRename.Process();

		// Process any events
		if(ev) {

			// Mouse type
			if(ev->cWidget->getType() == wid_Button)
				mouse = 1;
			if(ev->cWidget->getType() == wid_Textbox)
				mouse = 2;

			switch(ev->iControlID) {

				// Ok
				case 0:
					if(ev->iEventMsg == BTN_MOUSEUP) {

						cRename.SendMessage(2, TXM_GETTEXT, (DWORD)szName, cRename.SendMessage(2, TXM_GETTEXTLENGTH, 0, 0)+1);

						Menu_SvrList_FillList( (CListview *)cFavourites.getWidget( ServerList ) );

						// Click!
						BASS_SamplePlay(sfxGeneral.smpClick);

						renameServerMsg = false;
					}
					break;

				// Cancel
				case 1:
					if(ev->iEventMsg == BTN_MOUSEUP) {
						// Click!
						BASS_SamplePlay(sfxGeneral.smpClick);

						renameServerMsg = false;
					}
					break;
			}
		}


		DrawImage(tMenu->bmpScreen,gfxGUI.bmpMouse[mouse], Mouse->X,Mouse->Y);
		FlipScreen(tMenu->bmpScreen);
	}


	cRename.Shutdown();

	// Re-draw the background
	DrawImage(tMenu->bmpBuffer,tMenu->bmpMainBack_wob,0,0);
	Menu_DrawSubTitle(tMenu->bmpBuffer,SUB_NETWORK);
    Menu_DrawBox(tMenu->bmpBuffer, 15,130, 625, 465);
	Menu_RedrawMouse(true);
}

///////////////////
// Show an 'add server' box to enter in an address and name
void Menu_Net_FavouritesAddServer(void)
{
	CGuiLayout	cAddSvr;
	int			mouse = 0;
	gui_event_t *ev;
	mouse_t		*Mouse = GetMouse();
	bool		addServerMsg = true;


	// Create the background
	cFavourites.Draw( tMenu->bmpBuffer );
	Menu_DrawBox(tMenu->bmpBuffer, 200, 220, 440, 340);
	//DrawImageAdv(tMenu->bmpBuffer, tMenu->bmpMainBack, 202,222, 202,222, 237,117);
    DrawRectFill(tMenu->bmpBuffer, 202,222,439,339,0);
	Menu_RedrawMouse(true);


	cAddSvr.Initialize();
	cAddSvr.Add( new CButton(BUT_ADD, tMenu->bmpButtons),	0, 220, 320, 40,15);
	cAddSvr.Add( new CButton(BUT_CANCEL, tMenu->bmpButtons),1, 350, 320, 70,15);
	cAddSvr.Add( new CLabel("Add a server", 0xffff),		-1,275, 225, 0, 0);
	cAddSvr.Add( new CLabel("Address", 0xffff),				-1,215, 267, 0, 0);
	cAddSvr.Add( new CTextbox(),							2, 280, 265, 140, 20);
	cAddSvr.Add( new CLabel("Name", 0xffff),				-1,215, 290, 0, 0);
	cAddSvr.Add( new CTextbox(),							3, 280, 288, 140, 20);

	cAddSvr.SendMessage(2,TXM_SETMAX,21,0);
	cAddSvr.SendMessage(3,TXM_SETMAX,32,0);


	while(!GetKeyboard()->KeyUp[SDLK_ESCAPE] && addServerMsg) {
		mouse = 0;
		Menu_RedrawMouse(false);
		ProcessEvents();
		DrawImageAdv(tMenu->bmpScreen,tMenu->bmpBuffer, 200,220, 200,220, 240, 240);

		// Process the server list
		if( Menu_SvrList_Process() ) {
			// Add the servers to the listview
			Menu_SvrList_FillList( (CListview *)cFavourites.getWidget( ServerList ) );
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
						char name[32];
						cAddSvr.SendMessage(2, TXM_GETTEXT, (DWORD)addr, sizeof(addr));
						cAddSvr.SendMessage(3, TXM_GETTEXT, (DWORD)name, sizeof(name));

						Menu_SvrList_AddNamedServer(addr, name);
						Menu_SvrList_FillList( (CListview *)cFavourites.getWidget( ServerList ) );

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

