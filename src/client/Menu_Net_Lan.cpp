/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Net menu - LAN
// Created 21/8/02
// Jason Boettcher


#include "LieroX.h"
#include "AuxLib.h"
#include "Graphics.h"
#include "CClient.h"
#include "Menu.h"
#include "GfxPrimitives.h"


CGuiLayout	cLan;
std::string szLanCurServer;


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
	iNetMode = net_lan;

	cLan.Shutdown();
	cLan.Initialize();

	cLan.Add( new CButton(BUT_BACK, tMenu->bmpButtons),    nl_Back,       25, 440, 50,  15);
	cLan.Add( new CButton(BUT_REFRESH, tMenu->bmpButtons), nl_Refresh,	   280,440, 83,  15);
	cLan.Add( new CButton(BUT_JOIN, tMenu->bmpButtons),    nl_Join,	   570,440, 43,  15);
	cLan.Add( new CListview(),							   nl_ServerList, 40, 180, 560, 242);
	cLan.Add( new CLabel("Select player:",tLX->clNormalLabel),-1,		125, 152, 180,15);
	cLan.Add( new CCombobox(),								nl_PlayerSelection,		225,150, 170,  19);
	//cLan.Add( new CLabel("Local Area Network", tLX->clHeading),	   -1,		   40, 140, 0,   0);


	// Fill the players box
	profile_t *p = GetProfiles();
	for(;p;p=p->tNext) {
		/*if(p->iType == PRF_COMPUTER)
			continue;*/

		cLan.SendMessage( nl_PlayerSelection, CBS_ADDITEM, p->sName, p->iID);
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

	cLan.SendMessage( nl_ServerList, LVS_ADDCOLUMN, "", tLXOptions->iLANList[0]);
	cLan.SendMessage( nl_ServerList, LVS_ADDCOLUMN, "Server Name", tLXOptions->iLANList[1]);
	cLan.SendMessage( nl_ServerList, LVS_ADDCOLUMN, "State", tLXOptions->iLANList[2]);
	cLan.SendMessage( nl_ServerList, LVS_ADDCOLUMN, "Players", tLXOptions->iLANList[3]);
	cLan.SendMessage( nl_ServerList, LVS_ADDCOLUMN, "Ping", tLXOptions->iLANList[4]);
	cLan.SendMessage( nl_ServerList, LVS_ADDCOLUMN, "Address", tLXOptions->iLANList[5]);

	// Clear the server list
	Menu_SvrList_Clear();
	Menu_SvrList_PingLAN();

	return true;
}


///////////////////
// Shutdown the LAN menu
void Menu_Net_LANShutdown(void)
{
	if (tLXOptions)  {

		// Save the selected player
		cb_item_t *item = (cb_item_t *)cLan.SendMessage(nl_PlayerSelection,CBM_GETCURITEM,(DWORD)0,0);
		if (item)
			tLXOptions->tGameinfo.iLastSelectedPlayer = item->iIndex;

		if (iNetMode == net_lan)  {
			// Save the column widths
			for (int i=0;i<6;i++)
				tLXOptions->iLANList[i] = cLan.SendMessage(nl_ServerList,LVM_GETCOLUMNWIDTH,i,0);
		}
	}

	cLan.Shutdown();
}


///////////////////
// Net LAN frame
void Menu_Net_LANFrame(int mouse)
{
	gui_event_t *ev = NULL;
	std::string		addr;


	// Process & Draw the gui
#ifdef WITH_MEDIAPLAYER
	if (!cMediaPlayer.GetDrawPlayer())
#endif
		ev = cLan.Process();
	cLan.Draw( tMenu->bmpScreen );


	// Process the server list
	if( Menu_SvrList_Process() ) {
		// Add the servers to the listview
		Menu_SvrList_FillList( (CListview *)cLan.getWidget( nl_ServerList ) );
	}



	// Process any events
	if(ev) {

		switch(ev->iControlID) {

			// Back
			case nl_Back:
				if(ev->iEventMsg == BTN_MOUSEUP) {

					cb_item_t *item = (cb_item_t *)cLan.SendMessage(nl_PlayerSelection,CBM_GETCURITEM,(DWORD)0,0);
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

					addr = "";
					int result = cLan.SendMessage(nl_ServerList, LVS_GETCURSINDEX, &addr, 0);
					if(result != -1 && addr != "") {

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
					addr = "";
					int result = cLan.SendMessage(nl_ServerList, LVS_GETCURSINDEX, &addr, 0);
					lv_subitem_t *sub = ((CListview *)cLan.getWidget(nl_ServerList))->getCurSubitem(1);
					if(result != -1 && addr != "" && sub) {
						Menu_Net_LANJoinServer(addr,sub->sText);
						return;
					}
				}

                // Right click
                if( ev->iEventMsg == LV_RIGHTCLK ) {
                    addr = "";
					int result = cLan.SendMessage(nl_ServerList, LVS_GETCURSINDEX, &addr, 0);
					if(result && addr != "") {
                        // Display a menu
                        szLanCurServer = addr;
                        mouse_t *m = GetMouse();

                        cLan.Add( new CMenu(m->X, m->Y), nl_PopupMenu, 0,0, 640,480 );
                        cLan.SendMessage( nl_PopupMenu, MNS_ADDITEM, "Delete server",					0 );
                        cLan.SendMessage( nl_PopupMenu, MNS_ADDITEM, "Refresh server",					1 );
                        cLan.SendMessage( nl_PopupMenu, MNS_ADDITEM, "Join server",						2 );
						cLan.SendMessage( nl_PopupMenu, MNS_ADDITEM, "Add to favourites",				3 );
						cLan.SendMessage( nl_PopupMenu, MNS_ADDITEM, "Send \"I want to join message\"",	4 );
						cLan.SendMessage( nl_PopupMenu, MNS_ADDITEM, "Copy IP to clipboard",			5 );
                        cLan.SendMessage( nl_PopupMenu, MNS_ADDITEM, "Server details",					6 );
                    }
                }

				// Enter key
				if( ev->iEventMsg == LV_ENTER )  {
					// Join
					addr = "";
					int result = cLan.SendMessage(nl_ServerList, LVS_GETCURSINDEX, &addr, 0);
					lv_subitem_t *sub = ((CListview *)cLan.getWidget(nl_ServerList))->getCurSubitem(1);
					if(result != -1 && addr != "" && sub) {
						Menu_Net_LANJoinServer(addr,sub->sText);
						return;
					}
				}

				// Delete
				if( ev->iEventMsg == LV_DELETE )  {
					addr = "";
					int result = cLan.SendMessage(nl_ServerList, LVS_GETCURSINDEX, &addr, 0);
					if(result && addr != "") {
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

                    // Send a "wants to join" message
                    case MNU_USER+4:
						{
							server_t *sv = Menu_SvrList_FindServerStr(szLanCurServer);
							static std::string Nick;
							cLan.SendMessage(nl_PlayerSelection, CBS_GETCURNAME, &Nick, 0);
							if (sv)
								Menu_SvrList_WantsJoin(Nick, sv);
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
                cLan.SendMessage(nl_PopupMenu, MNM_REDRAWBUFFER, (DWORD)0, 0);
                cLan.removeWidget(nl_PopupMenu);
                break;
		}

	}


	// Draw the mouse
	DrawCursor(tMenu->bmpScreen);
}


///////////////////
// Join a server
void Menu_Net_LANJoinServer(const std::string& sAddress, const std::string& sName)
{

	// Fill in the game structure
	tGameInfo.iNumPlayers = 1;
	cb_item_t *item = (cb_item_t *)cLan.SendMessage(nl_PlayerSelection,CBM_GETCURITEM,(DWORD)0,0);

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
void Menu_Net_LanShowServer(const std::string& szAddress)
{
    CGuiLayout  cDetails;

    // Create the buffer
    DrawImage(tMenu->bmpBuffer,tMenu->bmpMainBack_common,0,0);
	if (tMenu->tFrontendInfo.bPageBoxes)
		Menu_DrawBox(tMenu->bmpBuffer, 15,130, 625, 465);
	Menu_DrawSubTitle(tMenu->bmpBuffer,SUB_NETWORK);
	cLan.Draw(tMenu->bmpBuffer);

	for(int i=1;i<4;i++)
		cNetButtons[i].Draw(tMenu->bmpBuffer);

	Menu_RedrawMouse(true);

	int center = tMenu->bmpScreen->w/2;
	int y = tMenu->bmpScreen->h/2 - INFO_H/2;

    cDetails.Initialize();
	cDetails.Add( new CButton(BUT_REFRESH, tMenu->bmpButtons),  ld_Refresh,	center - 105, y+INFO_H-20, 85,15);
    cDetails.Add( new CButton(BUT_OK, tMenu->bmpButtons),	    ld_Ok,      center + 20, y+INFO_H-20, 40,15);
	((CButton *)cDetails.getWidget(ld_Refresh))->setRedrawMenu(false);
	((CButton *)cDetails.getWidget(ld_Ok))->setRedrawMenu(false);

	bGotDetails = false;
	bOldLxBug = false;
	nTries = 0;
	fStart = -9999;

    while(!GetKeyboard()->KeyUp[SDLK_ESCAPE] && tMenu->iMenuRunning) {
		tLX->fCurTime = GetMilliSeconds();

		Menu_RedrawMouse(false);
		ProcessEvents();
		//DrawImageAdv(tMenu->bmpScreen,tMenu->bmpBuffer, 200,220, 200,220, 240, 240);

#ifdef WITH_MEDIAPLAYER
		cMediaPlayer.Frame();
#endif

		Menu_SvrList_DrawInfo(szAddress, INFO_W, INFO_H);

        cDetails.Draw(tMenu->bmpScreen);
        gui_event_t *ev = NULL;
#ifdef WITH_MEDIAPLAYER
		if (!cMediaPlayer.GetDrawPlayer())
#endif
			ev = cDetails.Process();
        if(ev) {

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

#ifdef WITH_MEDIAPLAYER
		cMediaPlayer.Draw(tMenu->bmpScreen);
#endif

        DrawCursor(tMenu->bmpScreen);
		FlipScreen(tMenu->bmpScreen);
    }

	cDetails.Shutdown();


    // Redraw the background
	DrawImage(tMenu->bmpBuffer,tMenu->bmpMainBack_common,0,0);
	if (tMenu->tFrontendInfo.bPageBoxes)
		Menu_DrawBox(tMenu->bmpBuffer, 15,130, 625, 465);
	Menu_DrawSubTitle(tMenu->bmpBuffer,SUB_NETWORK);
	Menu_RedrawMouse(true);
}
