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


#include "LieroX.h"
#include "AuxLib.h"
#include "HTTP.h"
#include "Graphics.h"
#include "CClient.h"
#include "Menu.h"
#include "GfxPrimitives.h"
#include "FindFile.h"
#include "StringUtils.h"
#include "CButton.h"
#include "CListview.h"
#include "CLabel.h"
#include "CCombobox.h"
#include "CMenu.h"
#include "CTextbox.h"
#include "CMediaPlayer.h"


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
	iNetMode = net_internet;
    szNetCurServer = "";

	cInternet.Shutdown();
	cInternet.Initialize();

	cInternet.Add( new CListview(),								mi_ServerList, 40, 180, 560, 242);
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
	if (tLXOptions)  {

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

	}

	cInternet.Shutdown();
}


///////////////////
// Net Internet frame
void Menu_Net_NETFrame(int mouse)
{
	gui_event_t *ev = NULL;
	static std::string	addr;


	// Process & Draw the gui
#ifdef WITH_MEDIAPLAYER
	if (!cMediaPlayer.GetDrawPlayer())
#endif
		ev = cInternet.Process();
	cInternet.Draw( tMenu->bmpScreen );


	// Process the server list
	if( Menu_SvrList_Process() ) {
		// Add the servers to the listview
		Menu_SvrList_FillList( (CListview *)cInternet.getWidget( mi_ServerList ) );
	}



	// Process any events
	if(ev) {

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
						cInternet.SendMessage( mi_PopupMenu, MNS_ADDITEM, "Send \"I want to join message\"",4 );
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

					// Send a "wants to join" message
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
	DrawCursor(tMenu->bmpScreen);

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
	gui_event_t *ev = NULL;
	bool		addServerMsg = true;


	// Create the background
	cInternet.Draw( tMenu->bmpBuffer );
	Menu_DrawBox(tMenu->bmpBuffer, 200, 220, 440, 340);
	//DrawImageAdv(tMenu->bmpBuffer, tMenu->bmpMainBack, 202,222, 202,222, 237,117);
    DrawRectFill(tMenu->bmpBuffer, 202,222,439,339,tLX->clDialogBackground);
	for(int i=0;i<4;i++)
		cNetButtons[i].Draw(tMenu->bmpBuffer);
	Menu_RedrawMouse(true);


	cAddSvr.Initialize();
	cAddSvr.Add( new CButton(BUT_ADD, tMenu->bmpButtons),	na_Add, 220, 320, 40,15);
	cAddSvr.Add( new CButton(BUT_CANCEL, tMenu->bmpButtons),na_Cancel, 350, 320, 70,15);
	cAddSvr.Add( new CLabel("Add a server", tLX->clNormalLabel),		-1,275, 225, 0, 0);
	cAddSvr.Add( new CLabel("Address", tLX->clNormalLabel),				-1,215, 267, 0, 0);
	cAddSvr.Add( new CTextbox(),							na_Address, 280, 265, 140, tLX->cFont.GetHeight());

	cAddSvr.SendMessage(2,TXM_SETMAX,21,0);


	while(!GetKeyboard()->KeyUp[SDLK_ESCAPE] && addServerMsg && tMenu->iMenuRunning) {
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


		DrawCursor(tMenu->bmpScreen);
		FlipScreen(tMenu->bmpScreen);
	}


	cAddSvr.Shutdown();

	// Re-draw the background
	DrawImage(tMenu->bmpBuffer,tMenu->bmpMainBack_common,0,0);
	Menu_DrawSubTitle(tMenu->bmpBuffer,SUB_NETWORK);
	if (tMenu->tFrontendInfo.bPageBoxes)
		Menu_DrawBox(tMenu->bmpBuffer, 15,130, 625, 465);
	Menu_RedrawMouse(true);
}



///////////////////
// Update the server list
void Menu_Net_NETUpdateList(void)
{
	CGuiLayout	cListUpdate;
	gui_event_t *ev = NULL;
	bool		updateList = true;
	int			http_result = 0;

    //
    // Get the number of master servers for a progress bar
    //
    int SvrCount = 0;
    int CurServer = 0;
    bool SentRequest = false;
    static std::string szLine;
    FILE *fp = OpenGameFile("cfg/masterservers.txt","rt");
    if( !fp )
        return;

    while( !feof(fp) ) {
        szLine = ReadUntil(fp);
		TrimSpaces(szLine);

        if( szLine.length() > 0 )
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
    DrawRectFill(tMenu->bmpBuffer, 202, 222, 439, 339, tLX->clDialogBackground);
    Menu_DrawBox(tMenu->bmpBuffer, 220, 280, 420, 300);
	for(ushort i=0;i<4;i++)
		cNetButtons[i].Draw(tMenu->bmpBuffer);
	Menu_RedrawMouse(true);


	cListUpdate.Initialize();
	cListUpdate.Add( new CButton(BUT_CANCEL, tMenu->bmpButtons),0, 285, 320, 70,15);
	cListUpdate.Add( new CLabel("Getting server list", tLX->clNormalLabel),	-1,260, 227, 0, 0);


	CHttp http;
	float senttime = 0;
	while(!GetKeyboard()->KeyUp[SDLK_ESCAPE] && updateList && tMenu->iMenuRunning) {
		tLX->fCurTime = GetMilliSeconds();

		Menu_RedrawMouse(false);
		ProcessEvents();
		DrawImageAdv(tMenu->bmpScreen,tMenu->bmpBuffer, 200,220, 200,220, 240, 240);

        if( SvrCount > 0 ) {
            DrawRectFill(tMenu->bmpScreen, 222,282, (int) (222+((float)CurServer/(float)SvrCount)*200.0f), 299, tLX->clProgress);
            tLX->cOutlineFont.DrawCentre(tMenu->bmpScreen, 320,283,tLX->clWhite, itoa(CurServer) + "/" + itoa(SvrCount));
        }

        // Do the HTTP requests of the master servers
        if( !SentRequest ) {

            // Have we gone through all the servers?
            if( CurServer >= SvrCount )
                break;

            // Get the next server in the list
            while( !feof(fp) ) {
				szLine = ReadUntil(fp);
				TrimSpaces(szLine);

                if( szLine.length() > 0 ) {

                    // Send the request
                    http.RequestData(szLine + LX_SVRLIST);
					senttime = GetMilliSeconds();
					SentRequest = true;

                    break;
                }
            }
        } else { // Process the http request
            http_result = http.ProcessRequest();

            // Parse the list if the request was successful
            if (http_result == HTTP_PROC_FINISHED) {
		        Menu_Net_NETParseList(http);
				break;
			} else if (http_result == HTTP_PROC_ERROR || (tLX->fCurTime - senttime) >= 5.0f)  {
				if (http.GetError().iError != HTTP_NO_ERROR)
            		printf("HTTP ERROR: " + http.GetError().sErrorMsg + "\n");

				// Jump to next server
				SentRequest = false;
				CurServer++;
				http.CancelProcessing();
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
			if (ev->iControlID == 0 && ev->iEventMsg == BTN_MOUSEUP)  {  // Cancel
				// Click!
				PlaySoundSample(sfxGeneral.smpClick);

				http_result = 0;
				updateList = false;
				break;
			}
		}


		DrawCursor(tMenu->bmpScreen);
		FlipScreen(tMenu->bmpScreen);
	}

	cListUpdate.Shutdown();


	Menu_SvrList_FillList( (CListview *)cInternet.getWidget( mi_ServerList ) );


	// Re-draw the background
	DrawImage(tMenu->bmpBuffer,tMenu->bmpMainBack_common,0,0);
	if (tMenu->tFrontendInfo.bPageBoxes)
		Menu_DrawBox(tMenu->bmpBuffer, 15,130, 625, 465);
	Menu_DrawSubTitle(tMenu->bmpBuffer,SUB_NETWORK);
	Menu_RedrawMouse(true);
}


///////////////////
// Parse the downloaded server list
void Menu_Net_NETParseList(CHttp &http)
{
	const std::string& content = http.GetData();

	std::string addr, ptr;
	
	std::string::const_iterator it = content.begin();
	size_t i = 0;
	size_t startpos = 0;	
	for(; it != content.end(); it++, i++) {	
		if(*it != '\n') continue;
		const std::vector<std::string>& tokens = explode(content.substr(startpos, i-startpos), ",");
		startpos = i+1;
		
		// we need at least 2 items
		if(tokens.size() < 2) continue;
		
		addr = tokens[0];
		ptr = tokens[1];
		
		TrimSpaces(addr);
		TrimSpaces(ptr);

		// If the address, or port does NOT have quotes around it, the line must be mangled and cannot be used		
		if(addr.size() <= 2 || ptr.size() <= 2) continue;
		if(addr[0] != '\"' || ptr[0] != '\"') continue;
		if(addr[addr.size()-1] != '\"' || ptr[ptr.size()-1] != '\"') continue;

		StripQuotes(addr);
		StripQuotes(ptr);

		// Create the server address
		Menu_SvrList_AddServer(addr + ":" + ptr, false);
	}
}

enum  {
	nd_Ok=0,
	nd_Refresh,
	nd_Join
};

///////////////////
// Show a server's details
void Menu_Net_NETShowServer(const std::string& szAddress)
{
    CGuiLayout  cDetails;

    // Create the buffer
    DrawImage(tMenu->bmpBuffer,tMenu->bmpMainBack_common,0,0);
    Menu_DrawBox(tMenu->bmpBuffer, 15,130, 625, 465);
	Menu_DrawSubTitle(tMenu->bmpBuffer,SUB_NETWORK);
	cInternet.Draw(tMenu->bmpBuffer);

	for(ushort i=1;i<4;i++)
		cNetButtons[i].Draw(tMenu->bmpBuffer);

	Menu_RedrawMouse(true);

	int center = tMenu->bmpScreen->w/2;
	int y = tMenu->bmpScreen->h/2 - INFO_H/2;
	
    cDetails.Initialize();
	cDetails.Add( new CButton(BUT_REFRESH, tMenu->bmpButtons),  nd_Refresh,	center - 105, y+INFO_H-20, 85,15);
    cDetails.Add( new CButton(BUT_JOIN, tMenu->bmpButtons),	    nd_Join,    center, y+INFO_H-20, 40,45);
	cDetails.Add( new CButton(BUT_OK, tMenu->bmpButtons),	    nd_Ok,      center + 60, y+INFO_H-20, 40,15);
	((CButton *)cDetails.getWidget(nd_Refresh))->setRedrawMenu(false);
	((CButton *)cDetails.getWidget(nd_Ok))->setRedrawMenu(false);
	((CButton *)cDetails.getWidget(nd_Join))->setRedrawMenu(false);

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
            if(ev->iControlID == nd_Ok && ev->iEventMsg == BTN_MOUSEUP) {
                break;
			// Refresh
            } else if (ev->iControlID == nd_Refresh && ev->iEventMsg == BTN_MOUSEUP)  {
				fStart = -9999;
				bGotDetails = false;
				bOldLxBug = false;
				nTries = 0;
			} else if (ev->iControlID == nd_Join && ev->iEventMsg == BTN_MOUSEUP)  {
                // Save the list
                Menu_SvrList_SaveList("cfg/svrlist.dat");

				lv_subitem_t *sub = ((CListview *)cInternet.getWidget(mi_ServerList))->getCurSubitem(1);

				// Join
				if (sub)
					Menu_Net_NETJoinServer(szAddress, sub->sText);

				break;
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
