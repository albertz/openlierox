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
#include "sound/SoundsBase.h"
#include "Clipboard.h"
#include "AuxLib.h"
#include "HTTP.h"
#include "DeprecatedGUI/Graphics.h"
#include "CClient.h"
#include "DeprecatedGUI/Menu.h"
#include "GfxPrimitives.h"
#include "FindFile.h"
#include "StringUtils.h"
#include "DeprecatedGUI/CButton.h"
#include "DeprecatedGUI/CListview.h"
#include "DeprecatedGUI/CLabel.h"
#include "DeprecatedGUI/CCombobox.h"
#include "DeprecatedGUI/CMenu.h"
#include "DeprecatedGUI/CTextbox.h"
#include "DeprecatedGUI/CChatWidget.h"
#include "ProfileSystem.h"
#include "IpToCountryDB.h"
#include "Debug.h"




namespace DeprecatedGUI {

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
	mi_PlayerSelection,
};

///////////////////
// Initialize the Internet menu
bool Menu_Net_NETInitialize()
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
	if( tLXOptions->bEnableChat && tLXOptions->bEnableMiniChat )
		cInternet.Add( new CChatWidget(),						-1,	25, 15, 585, 85 );


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
	bool validName = false;
	for(;p;p=p->tNext) {
		/*if(p->iType == PRF_COMPUTER)
			continue;*/

		int i = ((CCombobox*) cInternet.getWidget( mi_PlayerSelection ))->addItem(p->sName, p->sName);
		((CCombobox*) cInternet.getWidget( mi_PlayerSelection ))->setImage(p->cSkin.getPreview(), i);
		if( p->sName == tLXOptions->sLastSelectedPlayer )
			validName=true;
	}

	if( ! validName )
		tLXOptions->sLastSelectedPlayer = GetProfiles()->sName;

	((CCombobox*) cInternet.getWidget( mi_PlayerSelection ))->setCurSIndexItem( tLXOptions->sLastSelectedPlayer );

    Menu_redrawBufferRect(0, 0, 640, 480);

	cInternet.SendMessage( mi_ServerList, LVS_ADDCOLUMN, "", tLXOptions->iInternetList[0]);
	cInternet.SendMessage( mi_ServerList, LVS_ADDCOLUMN, "Server Name", tLXOptions->iInternetList[1]);
	cInternet.SendMessage( mi_ServerList, LVS_ADDCOLUMN, "State", tLXOptions->iInternetList[2]);
	cInternet.SendMessage( mi_ServerList, LVS_ADDCOLUMN, "Players", tLXOptions->iInternetList[3]);
	cInternet.SendMessage( mi_ServerList, LVS_ADDCOLUMN, "Ping", tLXOptions->iInternetList[4]);

	if (tLXOptions->bUseIpToCountry)
	{	// Too lazy to update tLXOptions, so I'll calculate last column width from width of listview
		//int CountryColumnWidth = 21; // TODO: not used

		// HINT: because this column is optional, it is at the end of the array from options
		cInternet.SendMessage( mi_ServerList, LVS_ADDCOLUMN, "Country", tLXOptions->iInternetList[5]);
	}

	cInternet.SendMessage( mi_ServerList, LVS_ADDCOLUMN, "Address", tLXOptions->iInternetList[6]);

	((CListview*) cInternet.getWidget( mi_ServerList ))->SetSortColumn( tLXOptions->iInternetSortColumn, true ); // Sorting

	// Clear the server list & grab an update
	Menu_SvrList_Clear();

    // Load the list
    Menu_SvrList_LoadList("cfg/svrlist.dat");
    Menu_SvrList_FillList( (CListview *)cInternet.getWidget( mi_ServerList ) );
	Menu_SvrList_UpdateUDPList();
	
	Timer("Menu_Net_NETInitialize serverlist timeout", null, NULL, SVRLIST_TIMEOUT, true).startHeadless();
	
	return true;
}


///////////////////
// Shutdown the internet menu
void Menu_Net_NETShutdown()
{
	if (tLXOptions)  {

		// Save the list
		if( iNetMode == net_internet )  {
			Menu_SvrList_SaveList("cfg/svrlist.dat");

			CListview* l = (CListview *)cInternet.getWidget(mi_ServerList);
			if(l) {
				// Save the column widths
				for (int i=0;i<7;i++)
					tLXOptions->iInternetList[i] = l->GetColumnWidth(i);

				// Save the sorting column
				tLXOptions->iInternetSortColumn = l->GetSortColumn();
			}
		}

		// Save the selected player
		cb_item_t *item = (cb_item_t *)cInternet.SendMessage(mi_PlayerSelection,CBM_GETCURITEM,(DWORD)0,0); // TODO: 64bit unsafe (pointer cast)
		if (item)
			tLXOptions->sLastSelectedPlayer = item->sIndex;
	}

	cInternet.Shutdown();
}


///////////////////
// Net Internet frame
void Menu_Net_NETFrame(int mouse)
{
	gui_event_t *ev = NULL;
	std::string	addr;


	// Process & Draw the gui
	ev = cInternet.Process();
	cInternet.Draw( VideoPostProcessor::videoSurface() );


	// Process the server list
	static bool wasLoadedBefore = false;
	if( Menu_SvrList_Process() || (tIpToCountryDB->Loaded() && !wasLoadedBefore) ) {
		// Add the servers to the listview
		Menu_SvrList_FillList( (CListview *)cInternet.getWidget( mi_ServerList ) );
		wasLoadedBefore = tIpToCountryDB->Loaded();
	}



	// Process any events
	if(ev) {

		switch(ev->iControlID) {

			// Add Server
			case mi_AddServer:
				if(ev->iEventMsg == BTN_CLICKED) {
					// Click!
					PlaySoundSample(sfxGeneral.smpClick);

					Menu_Net_NETAddServer();
				}
				break;

			// Back
			case mi_Back:
				if(ev->iEventMsg == BTN_CLICKED) {

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
				if(ev->iEventMsg == BTN_CLICKED) {

					// Click!
					PlaySoundSample(sfxGeneral.smpClick);

					// Refresh the currently visible servers
					Menu_SvrList_RefreshList();
					Menu_SvrList_FillList( (CListview *)cInternet.getWidget( mi_ServerList ) );
				}
				break;

			// Join
			case mi_Join:
                if(ev->iEventMsg == BTN_CLICKED) {

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
							std::string Nick;
							cInternet.SendMessage(mi_PlayerSelection, CBS_GETCURNAME, &Nick, 0);
							if (sv)
								Menu_SvrList_WantsJoin(Nick, sv);
						}
                        break;

					// Copy the IP to clipboard
					case MNU_USER+5:
						{
							copy_to_clipboard(szNetCurServer);
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
				if(ev->iEventMsg == BTN_CLICKED) {
					// Click!
					PlaySoundSample(sfxGeneral.smpClick);

					Menu_Net_NETUpdateList();
				}
				break;
		}

	}

	// F5 updates the list
	if (WasKeyboardEventHappening(SDLK_F5))
		Menu_Net_NETUpdateList();

	// Draw the mouse
	DrawCursor(VideoPostProcessor::videoSurface());

}



///////////////////
// Join a server
void Menu_Net_NETJoinServer(const std::string& sAddress, const std::string& sName)
{
	// Fill in the game structure
	CCombobox* combo = (CCombobox *) cInternet.getWidget(mi_PlayerSelection);
	const cb_item_t* item = combo->getSelectedItem();
	if(!item) {
		errors << "no player selected" << endl;
		return;
	}
		
	tLXOptions->sLastSelectedPlayer = item->sIndex;

	if(!JoinServer(sAddress, sName, item->sIndex))
		return;

	// Shutdown
	cInternet.Shutdown();
	
	iNetMode = net_join;
	tMenu->iReturnTo = net_internet;
	
	// Connect to the server
	Menu_Net_JoinConnectionInitialize(sAddress);
}



// TODO: remove this here!
extern CButton	cNetButtons[6];

///////////////////
// Show an 'add server' box to enter in an address
enum  {
	na_Cancel=0,
	na_Add,
	na_Address
};

void Menu_Net_NETAddServer()
{
	CGuiLayout	cAddSvr;
	gui_event_t *ev = NULL;
	bool		addServerMsg = true;


	// Create the background
	cInternet.Draw( tMenu->bmpBuffer.get() );
	Menu_DrawBox(tMenu->bmpBuffer.get(), 200, 220, 440, 340);
	//DrawImageAdv(tMenu->bmpBuffer, tMenu->bmpMainBack, 202,222, 202,222, 237,117);
    DrawRectFill(tMenu->bmpBuffer.get(), 202,222,439,339,tLX->clDialogBackground);
	for(int i=0;i<6;i++)
		cNetButtons[i].Draw(tMenu->bmpBuffer.get());
	Menu_RedrawMouse(true);


	cAddSvr.Initialize();
	cAddSvr.Add( new CButton(BUT_ADD, tMenu->bmpButtons),	na_Add, 220, 320, 40,15);
	cAddSvr.Add( new CButton(BUT_CANCEL, tMenu->bmpButtons),na_Cancel, 350, 320, 70,15);
	cAddSvr.Add( new CLabel("Add a server", tLX->clNormalLabel),		-1,275, 225, 0, 0);
	cAddSvr.Add( new CLabel("Address", tLX->clNormalLabel),				-1,215, 267, 0, 0);
	cAddSvr.Add( new CTextbox(),							na_Address, 280, 265, 140, tLX->cFont.GetHeight());

	ProcessEvents();
	while(!WasKeyboardEventHappening(SDLK_ESCAPE,false) && addServerMsg && tMenu->bMenuRunning) {
		Menu_RedrawMouse(true);
		DrawImageAdv(VideoPostProcessor::videoSurface(),tMenu->bmpBuffer, 200,220, 200,220, 240, 240);

		ProcessEvents();

		// Process the server list
		if( Menu_SvrList_Process() ) {
			// Add the servers to the listview
			Menu_SvrList_FillList( (CListview *)cInternet.getWidget( mi_ServerList ) );
		}

		cAddSvr.Draw( VideoPostProcessor::videoSurface() );
		ev = cAddSvr.Process();

		// Process any events
		if(ev) {

			switch(ev->iControlID) {

				// Add
				case na_Add:
					if(ev->iEventMsg == BTN_CLICKED) {

						std::string addr;
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
					if(ev->iEventMsg == BTN_CLICKED) {
						// Click!
						PlaySoundSample(sfxGeneral.smpClick);

						addServerMsg = false;
					}
					break;
			}
		}


		DrawCursor(VideoPostProcessor::videoSurface());
		doVideoFrameInMainThread();
		CapFPS();
	}


	cAddSvr.Shutdown();

	// Re-draw the background
	DrawImage(tMenu->bmpBuffer.get(),tMenu->bmpMainBack_common,0,0);
	Menu_DrawSubTitle(tMenu->bmpBuffer.get(),SUB_NETWORK);
	if (tMenu->tFrontendInfo.bPageBoxes)
		Menu_DrawBox(tMenu->bmpBuffer.get(), 15,130, 625, 465);
	Menu_RedrawMouse(true);
}



///////////////////
// Update the server list
void Menu_Net_NETUpdateList()
{
	CGuiLayout	cListUpdate;
	gui_event_t *ev = NULL;
	bool		updateList = true;
	int			http_result = 0;
	std::string szLine;

    // Clear the server list
    Menu_SvrList_ClearAuto();

	// UDP list
	Menu_SvrList_UpdateUDPList();

    //
    // Get the number of master servers for a progress bar
    //
    int SvrCount = 0;
    int CurServer = 0;
    bool SentRequest = false;
    FILE *fp = OpenGameFile("cfg/masterservers.txt","rt");
	if( !fp )  {
		errors << "Cannot update list because there is no masterservers.txt file available\n" << endl;
        return;
	}

	// TODO: i don't understand it, why are we doing it so complicated here, why not just save it in a list?
    while( !feof(fp) ) {
        szLine = ReadUntil(fp);
		TrimSpaces(szLine);

        if( szLine.length() > 0 && szLine[0] != '#' )
            SvrCount++;
    }

    // Back to the start
    fseek(fp, 0, SEEK_SET);

	// Create the background
	cInternet.Draw( tMenu->bmpBuffer.get() );
	Menu_DrawBox(tMenu->bmpBuffer.get(), 200, 220, 440, 340);
	//DrawImageAdv(tMenu->bmpBuffer, tMenu->bmpMainBack, 202,222, 202,222, 237,117);
    DrawRectFill(tMenu->bmpBuffer.get(), 202, 222, 439, 339, tLX->clDialogBackground);
    Menu_DrawBox(tMenu->bmpBuffer.get(), 220, 280, 420, 300);
	for(ushort i=0;i<6;i++)
		cNetButtons[i].Draw(tMenu->bmpBuffer.get());
	Menu_RedrawMouse(true);


	cListUpdate.Initialize();
	cListUpdate.Add( new CButton(BUT_CANCEL, tMenu->bmpButtons),0, 285, 320, 70,15);
	cListUpdate.Add( new CLabel("Getting server list", tLX->clNormalLabel),	-1,260, 227, 0, 0);


	CHttp http;

	while(!WasKeyboardEventHappening(SDLK_ESCAPE,false) && updateList && tMenu->bMenuRunning) {
		tLX->currentTime = GetTime();

		Menu_RedrawMouse(true);
		ProcessEvents();
		DrawImageAdv(VideoPostProcessor::videoSurface(),tMenu->bmpBuffer, 200,220, 200,220, 240, 240);

        if( SvrCount > 0 ) {
            DrawRectFill(VideoPostProcessor::videoSurface(), 222,282, (int) (222+((float)CurServer/(float)SvrCount)*200.0f), 299, tLX->clProgress);
            tLX->cOutlineFont.DrawCentre(VideoPostProcessor::videoSurface(), 320,283,tLX->clWhite, itoa(CurServer) + "/" + itoa(SvrCount));
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

                if( szLine.length() > 0 && szLine[0] != '#' ) {

                    // Send the request
					//notes << "Getting serverlist from " + szLine + "..." << endl;
					http.RequestData(szLine + LX_SVRLIST, tLXOptions->sHttpProxy);
					SentRequest = true;

                    break;
                }
            }
        } else { // Process the http request
            http_result = http.ProcessRequest();

            // Parse the list if the request was successful
            if (http_result == HTTP_PROC_FINISHED) {
		        Menu_Net_NETParseList(http);

				// Other master servers could have more server so we process them anyway
				SentRequest = false;
				CurServer++;
			} else if (http_result == HTTP_PROC_ERROR)  {
				if (http.GetError().iError != HTTP_NO_ERROR)
            		errors << "HTTP ERROR: " << http.GetError().sErrorMsg << endl;
				// Jump to next server
				SentRequest = false;
				CurServer++;
				http.CancelProcessing();
			}
        }

		cListUpdate.Draw( VideoPostProcessor::videoSurface() );
		ev = cListUpdate.Process();

		// Process any events
		if(ev) {
			if (ev->iControlID == 0 && ev->iEventMsg == BTN_CLICKED)  {  // Cancel
				// Click!
				PlaySoundSample(sfxGeneral.smpClick);

				http_result = 0;
				updateList = false;
				break;
			}
		}


		DrawCursor(VideoPostProcessor::videoSurface());
		doVideoFrameInMainThread();
		CapFPS();
	}

	cListUpdate.Shutdown();
	fclose(fp);

	Menu_SvrList_FillList( (CListview *)cInternet.getWidget( mi_ServerList ) );


	// Re-draw the background
	DrawImage(tMenu->bmpBuffer.get(),tMenu->bmpMainBack_common,0,0);
	if (tMenu->tFrontendInfo.bPageBoxes)
		Menu_DrawBox(tMenu->bmpBuffer.get(), 15,130, 625, 465);
	Menu_DrawSubTitle(tMenu->bmpBuffer.get(),SUB_NETWORK);
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
		std::vector<std::string> tokens = explode(content.substr(startpos, i-startpos), ",");
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

	// Update the GUI
	Timer("Menu_Net_NETParseList ping waiter", null, NULL, PingWait, true).startHeadless();
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
    DrawImage(tMenu->bmpBuffer.get(),tMenu->bmpMainBack_common,0,0);
    Menu_DrawBox(tMenu->bmpBuffer.get(), 15,130, 625, 465);
	Menu_DrawSubTitle(tMenu->bmpBuffer.get(),SUB_NETWORK);
	cInternet.Draw(tMenu->bmpBuffer.get());

	//for(ushort i=1;i<4;i++)
	//	cNetButtons[i].Draw(tMenu->bmpBuffer.get());

	Menu_RedrawMouse(true);

	int center = VideoPostProcessor::videoSurface()->w/2;
	int y = VideoPostProcessor::videoSurface()->h/2 - INFO_H/2;
	
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
	fStart = AbsTime();

    while(!WasKeyboardEventHappening(SDLK_ESCAPE,false) && tMenu->bMenuRunning) {
		tLX->currentTime = GetTime();

		Menu_RedrawMouse(true);
		ProcessEvents();
		//DrawImageAdv(VideoPostProcessor::videoSurface(),tMenu->bmpBuffer, 200,220, 200,220, 240, 240);

		Menu_SvrList_DrawInfo(szAddress, INFO_W, INFO_H);

        cDetails.Draw(VideoPostProcessor::videoSurface());
        gui_event_t *ev = NULL;

		ev = cDetails.Process();
        if(ev) {

			// Ok
            if(ev->iControlID == nd_Ok && ev->iEventMsg == BTN_CLICKED) {
                break;
			// Refresh
            } else if (ev->iControlID == nd_Refresh && ev->iEventMsg == BTN_CLICKED)  {
				fStart = AbsTime();
				bGotDetails = false;
				bOldLxBug = false;
				nTries = 0;
			} else if (ev->iControlID == nd_Join && ev->iEventMsg == BTN_CLICKED)  {
                // Save the list
                Menu_SvrList_SaveList("cfg/svrlist.dat");

				lv_subitem_t *sub = ((CListview *)cInternet.getWidget(mi_ServerList))->getCurSubitem(1);

				// Join
				if (sub)
					Menu_Net_NETJoinServer(szAddress, sub->sText);

				break;
			}
        }

        DrawCursor(VideoPostProcessor::videoSurface());
		doVideoFrameInMainThread();
		CapFPS();
    }

	cDetails.Shutdown();


    // Redraw the background
	DrawImage(tMenu->bmpBuffer.get(),tMenu->bmpMainBack_common,0,0);
	if (tMenu->tFrontendInfo.bPageBoxes)
		Menu_DrawBox(tMenu->bmpBuffer.get(), 15,130, 625, 465);
	Menu_DrawSubTitle(tMenu->bmpBuffer.get(),SUB_NETWORK);
	Menu_RedrawMouse(true);
}

}; // namespace DeprecatedGUI
