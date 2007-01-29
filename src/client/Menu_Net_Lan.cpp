/////////////////////////////////////////
//
//             Liero Xtreme
//
//     Copyright Auxiliary Software 2002
//
//
/////////////////////////////////////////


// Net menu - LAN
// Created 21/8/02
// Jason Boettcher


#include "defs.h"
#include "LieroX.h"
#include "Menu.h"


CGuiLayout	cLan;
char        szLanCurServer[128];


// Lan widgets
enum {
	nl_Join=0,
	nl_ServerList,
	nl_Refresh,
	nl_Back,
    nl_PopupMenu,
	nl_PlayerSelection
};



///////////////////
// Initialize the LAN menu
int Menu_Net_LANInitialize(void)
{
//	Uint32 blue = MakeColour(0,138,251); // TODO: not used

	iNetMode = net_lan;

	cLan.Shutdown();
	cLan.Initialize();

	cLan.Add( new CButton(BUT_BACK, tMenu->bmpButtons),    nl_Back,       25, 440, 50,  15);
	cLan.Add( new CButton(BUT_REFRESH, tMenu->bmpButtons), nl_Refresh,	   280,440, 83,  15);
	cLan.Add( new CButton(BUT_JOIN, tMenu->bmpButtons),    nl_Join,	   570,440, 43,  15);
	cLan.Add( new CListview(),							   nl_ServerList, 40, 180, 560, 240);
	cLan.Add( new CLabel("Select player:",tLX->clNormalLabel),-1,		125, 152, 180,15);
	cLan.Add( new CCombobox(),								nl_PlayerSelection,		225,150, 170,  19);
	//cLan.Add( new CLabel("Local Area Network", tLX->clHeading),	   -1,		   40, 140, 0,   0);


	// Fill the players box
	profile_t *p = GetProfiles();
	for(;p;p=p->tNext) {
		/*if(p->iType == PRF_COMPUTER)
			continue;*/

		cLan.SendMessage( nl_PlayerSelection, CBM_ADDITEM, p->iID, (DWORD)p->sName);
		cLan.SendMessage( nl_PlayerSelection, CBM_SETIMAGE, p->iID, (DWORD)p->bmpWorm);
	}

	cLan.SendMessage( nl_PlayerSelection, CBM_SETCURINDEX, tLXOptions->tGameinfo.iLastSelectedPlayer, 0);

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

	cLan.SendMessage( nl_ServerList, LVM_ADDCOLUMN, (DWORD)"", tLXOptions->iLANList[0]);
	cLan.SendMessage( nl_ServerList, LVM_ADDCOLUMN, (DWORD)"Server Name", tLXOptions->iLANList[1]);
	cLan.SendMessage( nl_ServerList, LVM_ADDCOLUMN, (DWORD)"State", tLXOptions->iLANList[2]);
	cLan.SendMessage( nl_ServerList, LVM_ADDCOLUMN, (DWORD)"Players", tLXOptions->iLANList[3]);
	cLan.SendMessage( nl_ServerList, LVM_ADDCOLUMN, (DWORD)"Ping", tLXOptions->iLANList[4]);
	cLan.SendMessage( nl_ServerList, LVM_ADDCOLUMN, (DWORD)"Address", tLXOptions->iLANList[5]);

	// Clear the server list
	Menu_SvrList_Clear();
	Menu_SvrList_PingLAN();

	return true;
}


///////////////////
// Shutdown the LAN menu
void Menu_Net_LANShutdown(void)
{
	// Save the selected player
	cb_item_t *item = (cb_item_t *)cLan.SendMessage(nl_PlayerSelection,CBM_GETCURITEM,0,0);
	if (item)
		tLXOptions->tGameinfo.iLastSelectedPlayer = item->iIndex;

	if (iNetMode == net_lan)  {
		// Save the column widths
		for (int i=0;i<6;i++)
			tLXOptions->iLANList[i] = cLan.SendMessage(nl_ServerList,LVM_GETCOLUMNWIDTH,i,0);
	}

	cLan.Shutdown();
}


///////////////////
// Net LAN frame
void Menu_Net_LANFrame(int mouse)
{
	mouse_t		*Mouse = GetMouse();
	gui_event_t *ev;
	static char		addr[256];


	// Process & Draw the gui
	ev = cLan.Process();
	cLan.Draw( tMenu->bmpScreen );


	// Process the server list
	if( Menu_SvrList_Process() ) {
		// Add the servers to the listview
		Menu_SvrList_FillList( (CListview *)cLan.getWidget( nl_ServerList ) );
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
			case nl_Back:
				if(ev->iEventMsg == BTN_MOUSEUP) {

					cb_item_t *item = (cb_item_t *)cLan.SendMessage(nl_PlayerSelection,CBM_GETCURITEM,0,0);
					if (item)
						tLXOptions->tGameinfo.iLastSelectedPlayer = item->iIndex;

					// Click!
					PlaySoundSample(sfxGeneral.smpClick);

					// Shutdown
					cLan.Shutdown();

					// Back to main menu
					Menu_MainInitialize();
				}
				break;

			// Refresh
			case nl_Refresh:
				if(ev->iEventMsg == BTN_MOUSEUP) {

					// Click!
					PlaySoundSample(sfxGeneral.smpClick);

					// Send out a ping over the lan
					Menu_SvrList_Clear();
					Menu_SvrList_PingLAN();
				}
				break;

			// Join
			case nl_Join:
				if(ev->iEventMsg == BTN_MOUSEUP) {

					addr[0] = 0;
					int result = cLan.SendMessage(nl_ServerList, LVM_GETCURSINDEX, (DWORD)addr, sizeof(addr));
					if(result != -1 && addr[0]) {

						// Click!
						PlaySoundSample(sfxGeneral.smpClick);

						lv_subitem_t *sub = ((CListview *)cLan.getWidget(nl_ServerList))->getCurSubitem(1);

						// Join
						if (sub)
							Menu_Net_LANJoinServer(addr,sub->sText);
						return;
					}
				}
				break;

			// Serverlist
			case nl_ServerList:
				if(ev->iEventMsg == LV_DOUBLECLK) {

					/*
					  Now.... Should a double click refresh the server (like tribes)?
					  Or should it join the server like other games???
					*/

					// Just join for the moment
					addr[0] = 0;
					int result = cLan.SendMessage(nl_ServerList, LVM_GETCURSINDEX, (DWORD)addr, sizeof(addr));
					lv_subitem_t *sub = ((CListview *)cLan.getWidget(nl_ServerList))->getCurSubitem(1);
					if(result != -1 && addr[0] && sub) {
						Menu_Net_LANJoinServer(addr,sub->sText);
						return;
					}
				}

                // Right click
                if( ev->iEventMsg == LV_RIGHTCLK ) {
                    addr[0] = 0;
					int result = cLan.SendMessage(nl_ServerList, LVM_GETCURSINDEX, (DWORD)addr, sizeof(addr));
					if(result && addr[0]) {
                        // Display a menu
                        fix_strncpy(szLanCurServer, addr);
                        mouse_t *m = GetMouse();

                        cLan.Add( new CMenu(m->X, m->Y), nl_PopupMenu, 0,0, 640,480 );
                        cLan.SendMessage( nl_PopupMenu, MNM_ADDITEM, 0, (DWORD)"Delete server" );
                        cLan.SendMessage( nl_PopupMenu, MNM_ADDITEM, 1, (DWORD)"Refresh server" );
                        cLan.SendMessage( nl_PopupMenu, MNM_ADDITEM, 2, (DWORD)"Join server" );
						cLan.SendMessage( nl_PopupMenu, MNM_ADDITEM, 3, (DWORD)"Add to favourites" );
						cLan.SendMessage( nl_PopupMenu, MNM_ADDITEM, 4, (DWORD)"Send \"I want join message\"" );
						cLan.SendMessage( nl_PopupMenu, MNM_ADDITEM, 5, (DWORD)"Copy IP to clipboard" );
                        cLan.SendMessage( nl_PopupMenu, MNM_ADDITEM, 6, (DWORD)"Server details" );
                    }
                }

				// Enter key
				if( ev->iEventMsg == LV_ENTER )  {
					// Join
					addr[0] = 0;
					int result = cLan.SendMessage(nl_ServerList, LVM_GETCURSINDEX, (DWORD)addr, sizeof(addr));
					lv_subitem_t *sub = ((CListview *)cLan.getWidget(nl_ServerList))->getCurSubitem(1);
					if(result != -1 && addr[0] && sub) {
                        // Save the list
                        Menu_SvrList_SaveList("cfg/svrlist.dat");

						Menu_Net_LANJoinServer(addr,sub->sText);
						return;
					}
				}

				// Delete
				if( ev->iEventMsg == LV_DELETE )  {
					addr[0] = 0;
					int result = cLan.SendMessage(nl_ServerList, LVM_GETCURSINDEX, (DWORD)addr, sizeof(addr));
					if(result && addr[0]) {
						Menu_SvrList_RemoveServer(addr);
						// Re-Fill the server list
						Menu_SvrList_FillList( (CListview *)cLan.getWidget( nl_ServerList ) );
					}
				}

				break;

            // Popup menu
            case nl_PopupMenu:
                switch( ev->iEventMsg ) {
                    // Delete the server
                    case MNU_USER+0:
                        Menu_SvrList_RemoveServer(szLanCurServer);
                        break;

                    // Refresh the server
                    case MNU_USER+1:
                        {
                            server_t *sv = Menu_SvrList_FindServerStr(szLanCurServer);
                            if(sv)
                                Menu_SvrList_RefreshServer(sv);
                        }
                        break;

                    // Join a server
                    case MNU_USER+2:  {
                        // Save the list
                        Menu_SvrList_SaveList("cfg/svrlist.dat");
						lv_subitem_t *sub = ((CListview *)cLan.getWidget(nl_ServerList))->getCurSubitem(1);
						if (sub)
							Menu_Net_LANJoinServer(szLanCurServer,sub->sText);
						}
                        return;

                    // Add server to favourites
                    case MNU_USER+3:
						{
							server_t *sv = Menu_SvrList_FindServerStr(szLanCurServer);
							if (sv)
								Menu_SvrList_AddFavourite(sv->szName,sv->szAddress);
						}
                        break;

                    // Send a "wants join" message
                    case MNU_USER+4:
						{
							server_t *sv = Menu_SvrList_FindServerStr(szLanCurServer);
							static char Nick[256];
							cLan.SendMessage(nl_PlayerSelection, CBM_GETCURNAME, (DWORD)Nick, sizeof(Nick));
							fix_markend(Nick); // safety
							char *sNick = Nick;
							if (sv)
								Menu_SvrList_WantsJoin(sNick, sv);
						}
                        break;

					// Copy the IP to clipboard
					case MNU_USER+5:
						{
							SetClipboardText(szLanCurServer);
						}
						break;

                    // Show server details
                    case MNU_USER+6:
						cLan.removeWidget(nl_PopupMenu);
                        Menu_Net_LanShowServer(szLanCurServer);
                        break;
                }

                // Re-Fill the server list
                Menu_SvrList_FillList( (CListview *)cLan.getWidget( nl_ServerList ) );

                // Remove the menu widget
                cLan.SendMessage(nl_PopupMenu, MNM_REDRAWBUFFER, 0, 0);
                cLan.removeWidget(nl_PopupMenu);
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
void Menu_Net_LANJoinServer(char *sAddress, char *sName)
{

	// Fill in the game structure
	tGameInfo.iNumPlayers = 1;
	cb_item_t *item = (cb_item_t *)cLan.SendMessage(nl_PlayerSelection,CBM_GETCURITEM,0,0);

	// Add the player to the list
	if (item)  {
		profile_t *ply = FindProfile(item->iIndex);
		if(ply)
			tGameInfo.cPlayers[0] = ply;
	}

	tLXOptions->tGameinfo.iLastSelectedPlayer = item->iIndex;

	cClient->setServerName(sName);

	// Shutdown
	cLan.Shutdown();

	tMenu->iReturnTo = net_lan;

	Menu_Net_JoinInitialize(sAddress);
}

extern CButton	cNetButtons[5];

enum {
	ld_Ok=0,
	ld_Refresh
};

///////////////////
// Show a server's details
void Menu_Net_LanShowServer(char *szAddress)
{
    mouse_t     *Mouse = GetMouse();
    int         nMouseCur = 0;
    CGuiLayout  cDetails;

    // Create the buffer
    DrawImage(tMenu->bmpBuffer,tMenu->bmpMainBack_wob,0,0);
    Menu_DrawBox(tMenu->bmpBuffer, 15,130, 625, 465);
	Menu_DrawSubTitle(tMenu->bmpBuffer,SUB_NETWORK);
	cLan.Draw(tMenu->bmpBuffer);

	for(int i=1;i<4;i++)
		cNetButtons[i].Draw(tMenu->bmpBuffer);

	Menu_RedrawMouse(true);

    cDetails.Initialize();
	cDetails.Add( new CButton(BUT_REFRESH, tMenu->bmpButtons),  ld_Refresh,			200,400, 85,15);
    cDetails.Add( new CButton(BUT_OK, tMenu->bmpButtons),	    ld_Ok,      310,400, 40,15);

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
            if(ev->iControlID == ld_Ok && ev->iEventMsg == BTN_MOUSEUP) {
                break;
			// Refresh
            } else if (ev->iControlID == ld_Refresh && ev->iEventMsg == BTN_MOUSEUP)  {
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
