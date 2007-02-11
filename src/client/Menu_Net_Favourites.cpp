/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
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
	mf_Join=0,
	mf_ServerList,
	mf_Refresh,
	mf_Add,
	mf_Back,
    mf_PopupMenu,
	mf_PlayerSelection
};



///////////////////
// Initialize the favourites menu
int Menu_Net_FavouritesInitialize(void)
{
//	Uint32 blue = MakeColour(0,138,251);  // TODO: not used

	iNetMode = net_favourites;

	cFavourites.Shutdown();
	cFavourites.Initialize();

	cFavourites.Add( new CButton(BUT_BACK, tMenu->bmpButtons),    mf_Back,        25, 440, 50,  15);
	cFavourites.Add( new CButton(BUT_ADD, tMenu->bmpButtons),	  mf_Add,		   190,440, 83,  15);
	cFavourites.Add( new CButton(BUT_REFRESH, tMenu->bmpButtons), mf_Refresh,	   350,440, 83,  15);
	cFavourites.Add( new CButton(BUT_JOIN, tMenu->bmpButtons),    mf_Join,	   570,440, 43,  15);
	cFavourites.Add( new CListview(),							   mf_ServerList, 40, 180, 560, 240);
	cFavourites.Add( new CLabel("Select player:",tLX->clNormalLabel),-1,		125, 152, 180,15);
	cFavourites.Add( new CCombobox(),								mf_PlayerSelection,		225,150, 170,  19);


	// Fill the players box
	profile_t *p = GetProfiles();
	for(;p;p=p->tNext) {
		/*if(p->iType == PRF_COMPUTER)
			continue;*/

		cFavourites.SendMessage( mf_PlayerSelection, CBM_ADDITEM, p->iID, (DWORD)p->sName);
		cFavourites.SendMessage( mf_PlayerSelection, CBM_SETIMAGE, p->iID, (DWORD)p->bmpWorm);
	}

	cFavourites.SendMessage( mf_PlayerSelection, CBM_SETCURINDEX, tLXOptions->tGameinfo.iLastSelectedPlayer, 0);

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

	cFavourites.SendMessage( mf_ServerList, LVM_ADDCOLUMN, (DWORD)"", tLXOptions->iFavouritesList[0]);
	cFavourites.SendMessage( mf_ServerList, LVM_ADDCOLUMN, (DWORD)"Server Name", tLXOptions->iFavouritesList[1]);
	cFavourites.SendMessage( mf_ServerList, LVM_ADDCOLUMN, (DWORD)"State", tLXOptions->iFavouritesList[2]);
	cFavourites.SendMessage( mf_ServerList, LVM_ADDCOLUMN, (DWORD)"Players", tLXOptions->iFavouritesList[3]);
	cFavourites.SendMessage( mf_ServerList, LVM_ADDCOLUMN, (DWORD)"Ping", tLXOptions->iFavouritesList[4]);
	cFavourites.SendMessage( mf_ServerList, LVM_ADDCOLUMN, (DWORD)"Address", tLXOptions->iFavouritesList[5]);

	// Fill the server list
	Menu_SvrList_Clear();
	Menu_SvrList_LoadList("cfg/favourites.dat");
	Menu_SvrList_FillList( (CListview *)cFavourites.getWidget( mf_ServerList ) );
	Menu_SvrList_RefreshList();

	return true;
}


///////////////////
// Shutdown the favourites menu
void Menu_Net_FavouritesShutdown(void)
{
	// Save the selected player
	cb_item_t *item = (cb_item_t *)cFavourites.SendMessage(mf_PlayerSelection,CBM_GETCURITEM,0,0);
	if (item)
		tLXOptions->tGameinfo.iLastSelectedPlayer = item->iIndex;

	// Save the list
	if (iNetMode == net_favourites)  {
		Menu_SvrList_SaveList("cfg/favourites.dat");

		// Save the column widths
		for (int i=0;i<6;i++)
			tLXOptions->iFavouritesList[i] = cFavourites.SendMessage(mf_ServerList,LVM_GETCOLUMNWIDTH,i,0);
	}

	cFavourites.Shutdown();
}


///////////////////
// Net favourites frame
void Menu_Net_FavouritesFrame(int mouse)
{
	mouse_t		*Mouse = GetMouse();
	gui_event_t *ev;
	static char		addr[256];


	// Process & Draw the gui
	ev = cFavourites.Process();
	cFavourites.Draw( tMenu->bmpScreen );

	// Process the server list
	if( Menu_SvrList_Process() ) {
		// Add the servers to the listview
		Menu_SvrList_FillList( (CListview *)cFavourites.getWidget( mf_ServerList ) );
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

			// Back
			case mf_Back:
				if(ev->iEventMsg == BTN_MOUSEUP) {

					cb_item_t *item = (cb_item_t *)cFavourites.SendMessage(mf_PlayerSelection,CBM_GETCURITEM,0,0);
					if (item)
						tLXOptions->tGameinfo.iLastSelectedPlayer = item->iIndex;

					// Click!
					PlaySoundSample(sfxGeneral.smpClick);

					// Save the list
					Menu_SvrList_SaveList("cfg/favourites.dat");

					// Shutdown
					cFavourites.Shutdown();

					// Back to main menu
					Menu_MainInitialize();
				}
				break;

			// Add
			case mf_Add:
				if(ev->iEventMsg == BTN_MOUSEUP) {

					// Click!
					PlaySoundSample(sfxGeneral.smpClick);

					Menu_Net_FavouritesAddServer();
				}
				break;

			// Refresh
			case mf_Refresh:
				if(ev->iEventMsg == BTN_MOUSEUP) {

					// Click!
					PlaySoundSample(sfxGeneral.smpClick);

					// Send out a ping
					Menu_SvrList_RefreshList();
					Menu_SvrList_FillList( (CListview *)cFavourites.getWidget( mf_ServerList ) );
				}
				break;

			// Join
			case mf_Join:
				if(ev->iEventMsg == BTN_MOUSEUP) {

					addr[0] = 0;
					int result = cFavourites.SendMessage(mf_ServerList, LVM_GETCURSINDEX, (DWORD)addr, sizeof(addr));
					lv_subitem_t *sub = ((CListview *)cFavourites.getWidget(mf_ServerList))->getCurSubitem(1);
					if(result != -1 && addr[0] && sub) {

						// Click!
						PlaySoundSample(sfxGeneral.smpClick);

						// Join
						Menu_Net_FavouritesJoinServer(addr,sub->sText);
						return;
					}
				}
				break;

			// Serverlist
			case mf_ServerList:
				if(ev->iEventMsg == LV_DOUBLECLK) {

					/*
					  Now.... Should a double click refresh the server (like tribes)?
					  Or should it join the server like other games???
					*/

					// Just join for the moment
					addr[0] = 0;
					int result = cFavourites.SendMessage(mf_ServerList, LVM_GETCURSINDEX, (DWORD)addr, sizeof(addr));
					lv_subitem_t *sub = ((CListview *)cFavourites.getWidget(mf_ServerList))->getCurSubitem(1);
					if(result != -1 && addr[0] && sub) {
						Menu_Net_FavouritesJoinServer(addr,sub->sText);
						return;
					}
				}

                // Right click
                if( ev->iEventMsg == LV_RIGHTCLK ) {
                    addr[0] = 0;
					int result = cFavourites.SendMessage(mf_ServerList, LVM_GETCURSINDEX, (DWORD)addr, sizeof(addr));
					if(result && addr[0]) {
                        // Display a menu
                        fix_strncpy(szFavouritesCurServer, addr);
                        mouse_t *m = GetMouse();

                        cFavourites.Add( new CMenu(m->X, m->Y), mf_PopupMenu, 0,0, 640,480 );
                        cFavourites.SendMessage( mf_PopupMenu, MNM_ADDITEM, 0, (DWORD)"Remove from favourites" );
						cFavourites.SendMessage( mf_PopupMenu, MNM_ADDITEM, 1, (DWORD)"Rename server" );
                        cFavourites.SendMessage( mf_PopupMenu, MNM_ADDITEM, 2, (DWORD)"Refresh server" );
                        cFavourites.SendMessage( mf_PopupMenu, MNM_ADDITEM, 3, (DWORD)"Join server" );
						cFavourites.SendMessage( mf_PopupMenu, MNM_ADDITEM, 4, (DWORD)"Send \"I want join\" message" );
						cFavourites.SendMessage( mf_PopupMenu, MNM_ADDITEM, 5, (DWORD)"Copy IP to clipboard" );
                        cFavourites.SendMessage( mf_PopupMenu, MNM_ADDITEM, 6, (DWORD)"Server details" );
                    }
                }

				// Enter key
				if( ev->iEventMsg == LV_ENTER )  {
					// Join
					addr[0] = 0;
					int result = cFavourites.SendMessage(mf_ServerList, LVM_GETCURSINDEX, (DWORD)addr, sizeof(addr));
					lv_subitem_t *sub = ((CListview *)cFavourites.getWidget(mf_ServerList))->getCurSubitem(1);
					if(result != -1 && addr[0] && sub) {
     					Menu_Net_FavouritesJoinServer(addr,sub->sText);
						return;
					}
				}

				// Delete
				if( ev->iEventMsg == LV_DELETE )  {
					//DrawImage(tMenu->bmpBuffer,tMenu->bmpScreen,0,0);
					addr[0] = 0;
					int result = cFavourites.SendMessage(mf_ServerList, LVM_GETCURSINDEX, (DWORD)addr, sizeof(addr));
					server_t *sv = Menu_SvrList_FindServerStr(addr);
					static char buf[256] = "";
					if (sv)  {
						snprintf(buf,sizeof(buf),"Are you sure you want to remove\n \"%s\" server from favourites?",sv->szName);
						if (Menu_MessageBox("Confirmation",buf,LMB_YESNO) == MBR_YES)  {
							if(result && addr[0]) {
								Menu_SvrList_RemoveServer(addr);
								// Re-Fill the server list
								Menu_SvrList_FillList( (CListview *)cFavourites.getWidget( mf_ServerList ) );
							}
							Menu_SvrList_SaveList("cfg/favourites.dat");
						}
						Menu_redrawBufferRect(0,0,640,480);
					}

				}

				break;

            // Popup menu
            case mf_PopupMenu:
                switch( ev->iEventMsg ) {
                     // Remove server from favourites
				case MNU_USER+0:  {
						server_t *sv = Menu_SvrList_FindServerStr(szFavouritesCurServer);
						static char buf[256] = "";
						if (sv)  {
							snprintf(buf,sizeof(buf),"Are you sure you want to remove\n \"%s\" server from favourites?",sv->szName);
							if (Menu_MessageBox("Confirmation",buf,LMB_YESNO) == MBR_YES)  {
								Menu_SvrList_RemoveServer(szFavouritesCurServer);
								Menu_SvrList_SaveList("cfg/favourites.dat");  // Save changes
							}
							Menu_redrawBufferRect(0,0,640,480);
						}
						}
                        break;

					// Rename server
                    case MNU_USER+1:
						{
							// Remove the menu widget
							cFavourites.SendMessage(mf_PopupMenu, MNM_REDRAWBUFFER, 0, 0);
							cFavourites.removeWidget(mf_PopupMenu);

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
                    case MNU_USER+3:  {
                        // Join
						lv_subitem_t *sub = ((CListview *)cFavourites.getWidget(mf_ServerList))->getCurSubitem(1);
						if (sub)
							Menu_Net_FavouritesJoinServer(szFavouritesCurServer,sub->sText);
						}
                        return;

					// Send a "wants join" message
                    case MNU_USER+4:
						{
							server_t *sv = Menu_SvrList_FindServerStr(szFavouritesCurServer);
							static char Nick[256];
							cFavourites.SendMessage(mf_PlayerSelection, CBM_GETCURNAME, (DWORD)Nick, sizeof(Nick));
							fix_markend(Nick); // safety
							char *sNick = Nick;
							if (sv)
								Menu_SvrList_WantsJoin(sNick, sv);
						}
                        break;

					// Copy the IP to clipboard
					case MNU_USER+5:
						{
							SetClipboardText(szFavouritesCurServer);
						}
						break;

                    // Show server details
                    case MNU_USER+6:
						cFavourites.removeWidget(mf_PopupMenu);
                        Menu_Net_FavouritesShowServer(szFavouritesCurServer);
                        break;
                }

                // Re-Fill the server list
                Menu_SvrList_FillList( (CListview *)cFavourites.getWidget( mf_ServerList ) );

                // Remove the menu widget
                cFavourites.SendMessage(mf_PopupMenu, MNM_REDRAWBUFFER, 0, 0);
                cFavourites.removeWidget(mf_PopupMenu);
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
void Menu_Net_FavouritesJoinServer(char *sAddress, char *sName)
{
	tGameInfo.iNumPlayers = 1;

	// Fill in the game structure
	cb_item_t *item = (cb_item_t *)cFavourites.SendMessage(mf_PlayerSelection,CBM_GETCURITEM,0,0);

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
	cFavourites.Shutdown();

	iNetMode = net_join;

	// Save the list
	Menu_SvrList_SaveList("cfg/favourites.dat");

	tMenu->iReturnTo = net_favourites;

	// Connect to the server
	Menu_Net_JoinConnectionInitialize(sAddress);
}

enum  {
	fd_Ok,
	fd_Refresh
};

extern CButton cNetButtons[5];

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

	for(int i=1;i<4;i++)
		cNetButtons[i].Draw(tMenu->bmpBuffer);

	Menu_RedrawMouse(true);

    cDetails.Initialize();
	cDetails.Add( new CButton(BUT_REFRESH, tMenu->bmpButtons),  fd_Refresh,		200,400, 85,15);
    cDetails.Add( new CButton(BUT_OK, tMenu->bmpButtons),	    fd_Ok,      310,400, 40,15);

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

		Menu_SvrList_DrawInfo(szAddress);

        cDetails.Draw(tMenu->bmpScreen);
        gui_event_t *ev = cDetails.Process();
        if(ev) {
            if(ev->cWidget->getType() == wid_Button)
                nMouseCur = 1;

			// Ok
            if(ev->iControlID == fd_Ok && ev->iEventMsg == BTN_MOUSEUP) {
                break;
			// Refresh
            } else if (ev->iControlID == fd_Refresh && ev->iEventMsg == BTN_MOUSEUP)  {
				fStart = -9999;
				bGotDetails = false;
				bOldLxBug = false;
				nTries = 0;
			}
        }


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

///////////////////
// Show an 'rename server' box
enum  {
	rs_Cancel=0,
	rs_Ok,
	rs_NewName
};

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
	cRename.Add( new CButton(BUT_OK, tMenu->bmpButtons),	rs_Ok, 220, 310, 40,15);
	cRename.Add( new CButton(BUT_CANCEL, tMenu->bmpButtons),rs_Cancel, 350, 310, 70,15);
	cRename.Add( new CLabel("Rename a server", tLX->clNormalLabel),		-1,275, 225, 0, 0);
	cRename.Add( new CLabel("New name", tLX->clNormalLabel),			-1,215, 267, 0, 0);
	cRename.Add( new CTextbox(),							rs_NewName, 300, 265, 150, 20);

	cRename.SendMessage(2,TXM_SETMAX,30,0);
	cRename.SendMessage(2,TXM_SETTEXT,(DWORD)szName,0); // Fill in the current server name


	while(!GetKeyboard()->KeyUp[SDLK_ESCAPE] && renameServerMsg && tMenu->iMenuRunning) {
		mouse = 0;
		Menu_RedrawMouse(false);
		ProcessEvents();
		DrawImageAdv(tMenu->bmpScreen,tMenu->bmpBuffer, 200,220, 200,220, 240, 240);

		// Process the server list
		if( Menu_SvrList_Process() ) {
			// Add the servers to the listview
			Menu_SvrList_FillList( (CListview *)cFavourites.getWidget( mf_ServerList ) );
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
				case rs_Ok:
					if(ev->iEventMsg == BTN_MOUSEUP) {

						cRename.SendMessage(2, TXM_GETTEXT, (DWORD)szName, cRename.SendMessage(2, TXM_GETTEXTLENGTH, 0, 0)+1);

						Menu_SvrList_FillList( (CListview *)cFavourites.getWidget( mf_ServerList ) );

						// Click!
						PlaySoundSample(sfxGeneral.smpClick);

						renameServerMsg = false;
					}
					break;

				// Cancel
				case rs_Cancel:
					if(ev->iEventMsg == BTN_MOUSEUP) {
						// Click!
						PlaySoundSample(sfxGeneral.smpClick);

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
enum  {
	fa_Cancel=0,
	fa_Add,
	fa_Address,
	fa_Name
};

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
	cAddSvr.Add( new CButton(BUT_ADD, tMenu->bmpButtons),	fa_Add, 220, 320, 40,15);
	cAddSvr.Add( new CButton(BUT_CANCEL, tMenu->bmpButtons),fa_Cancel, 350, 320, 70,15);
	cAddSvr.Add( new CLabel("Add a server", tLX->clNormalLabel),		-1,275, 225, 0, 0);
	cAddSvr.Add( new CLabel("Address", tLX->clNormalLabel),				-1,215, 267, 0, 0);
	cAddSvr.Add( new CTextbox(),							fa_Address, 280, 265, 140, 20);
	cAddSvr.Add( new CLabel("Name", tLX->clNormalLabel),				-1,215, 290, 0, 0);
	cAddSvr.Add( new CTextbox(),							fa_Name, 280, 288, 140, 20);

	cAddSvr.SendMessage(2,TXM_SETMAX,21,0);
	cAddSvr.SendMessage(3,TXM_SETMAX,32,0);


	while(!GetKeyboard()->KeyUp[SDLK_ESCAPE] && addServerMsg && tMenu->iMenuRunning) {
		mouse = 0;
		Menu_RedrawMouse(false);
		ProcessEvents();
		DrawImageAdv(tMenu->bmpScreen,tMenu->bmpBuffer, 200,220, 200,220, 240, 240);

		// Process the server list
		if( Menu_SvrList_Process() ) {
			// Add the servers to the listview
			Menu_SvrList_FillList( (CListview *)cFavourites.getWidget( mf_ServerList ) );
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
				case fa_Add:
					if(ev->iEventMsg == BTN_MOUSEUP) {

						static char addr[512];
						static char name[32];
						cAddSvr.SendMessage(2, TXM_GETTEXT, (DWORD)addr, sizeof(addr)); fix_markend(addr);
						cAddSvr.SendMessage(3, TXM_GETTEXT, (DWORD)name, sizeof(name)); fix_markend(name);

						Menu_SvrList_AddNamedServer(addr, name);
						Menu_SvrList_FillList( (CListview *)cFavourites.getWidget( mf_ServerList ) );

						// Click!
						PlaySoundSample(sfxGeneral.smpClick);

						addServerMsg = false;
					}
					break;

				// Cancel
				case fa_Cancel:
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

