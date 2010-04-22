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
#include "sound/SoundsBase.h"
#include "Clipboard.h"
#include "AuxLib.h"
#include "DeprecatedGUI/Graphics.h"
#include "CClient.h"
#include "DeprecatedGUI/Menu.h"
#include "GfxPrimitives.h"
#include "DeprecatedGUI/CButton.h"
#include "DeprecatedGUI/CLabel.h"
#include "DeprecatedGUI/CMenu.h"
#include "Timer.h"
#include "ProfileSystem.h"
#include "Debug.h"



namespace DeprecatedGUI {

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
bool Menu_Net_LANInitialize()
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
	CCombobox* PlayerSelection = (CCombobox*) cLan.getWidget( nl_PlayerSelection );
	profile_t *p = GetProfiles();
	bool validName = false;
	for(;p;p=p->tNext) {
		/*if(p->iType == PRF_COMPUTER)
			continue;*/

		int index = PlayerSelection->addItem( p->sName, p->sName );
		PlayerSelection->setImage( p->cSkin.getPreview(), index );
		if( p->sName == tLXOptions->sLastSelectedPlayer )
			validName=true;
	}

	if( ! validName )
		tLXOptions->sLastSelectedPlayer = GetProfiles()->sName;

	PlayerSelection->setCurSIndexItem( tLXOptions->sLastSelectedPlayer );

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

	((CListview*) cLan.getWidget( nl_ServerList ))->SetSortColumn( tLXOptions->iLANSortColumn, true ); // Sorting

	// Clear the server list
	Menu_SvrList_Clear();
	Menu_SvrList_PingLAN();

	return true;
}


///////////////////
// Shutdown the LAN menu
void Menu_Net_LANShutdown()
{
	if (tLXOptions)  {

		// Save the selected player
		CCombobox* combo = (CCombobox *) cLan.getWidget(nl_PlayerSelection);
		if( combo )
		{
			const GuiListItem::Pt item = combo->getSelectedItem();
			if (item.get())
				tLXOptions->sLastSelectedPlayer = item->index();
		}

		if (iNetMode == net_lan)  {
			// Save the column widths
			for (int i=0;i<6;i++)
				tLXOptions->iLANList[i] = cLan.SendMessage(nl_ServerList,LVM_GETCOLUMNWIDTH,i,0);

			// Save the sorting column
			tLXOptions->iLANSortColumn = ((CListview *)cLan.getWidget(nl_ServerList))->GetSortColumn();
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
	ev = cLan.Process();
	cLan.Draw( VideoPostProcessor::videoSurface() );


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
				if(ev->iEventMsg == BTN_CLICKED) {

					CCombobox* combo = (CCombobox *) cLan.getWidget(nl_PlayerSelection);
					const GuiListItem::Pt item = combo->getSelectedItem();
					if (item.get())
						tLXOptions->sLastSelectedPlayer = item->index();

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
				if(ev->iEventMsg == BTN_CLICKED) {

					// Click!
					PlaySoundSample(sfxGeneral.smpClick);

					// Send out a ping over the lan
					Menu_SvrList_Clear();
					Menu_SvrList_PingLAN();
				}
				break;

			// Join
			case nl_Join:
				if(ev->iEventMsg == BTN_CLICKED) {

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
							server_t::Ptr sv = Menu_SvrList_FindServerStr(szLanCurServer);
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
							server_t::Ptr sv = Menu_SvrList_FindServerStr(szLanCurServer);
							if (sv)
								Menu_SvrList_AddFavourite(sv->szName,sv->szAddress);
						}
                        break;

                    // Send a "wants to join" message
                    case MNU_USER+4:
						{
							server_t::Ptr sv = Menu_SvrList_FindServerStr(szLanCurServer);
							std::string Nick;
							cLan.SendMessage(nl_PlayerSelection, CBS_GETCURNAME, &Nick, 0);
							if (sv)
								Menu_SvrList_WantsJoin(Nick, sv);
						}
                        break;

					// Copy the IP to clipboard
					case MNU_USER+5:
						{
							copy_to_clipboard(szLanCurServer);
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

	// F5 updates the list
	if (WasKeyboardEventHappening(SDLK_F5))  {
		Menu_SvrList_Clear();
		Menu_SvrList_PingLAN();
	}


	// Draw the mouse
	DrawCursor(VideoPostProcessor::videoSurface());
}


///////////////////
// Join a server
void Menu_Net_LANJoinServer(const std::string& sAddress, const std::string& sName)
{
	// Fill in the game structure
	CCombobox* combo = (CCombobox *) cLan.getWidget(nl_PlayerSelection);
	const GuiListItem::Pt item = combo->getSelectedItem();
	if(!item.get()) {
		errors << "no player selected" << endl;
		return;
	}
	
	tLXOptions->sLastSelectedPlayer = item->index();
	
	if(!JoinServer(sAddress, sName, item->index()))
		return;

	// Shutdown
	cLan.Shutdown();

	tMenu->iReturnTo = net_lan;

	Menu_Net_JoinInitialize(sAddress);
}

// TODO: remove this here!
extern CButton	cNetButtons[5];

enum {
	ld_Ok=0,
	ld_Refresh,
	ld_Join
};

///////////////////
// Show a server's details
// TODO: join this with Menu_Net_NETShowServer()
void Menu_Net_LanShowServer(const std::string& szAddress)
{
    CGuiLayout  cDetails;

    // Create the buffer
    DrawImage(tMenu->bmpBuffer.get(),tMenu->bmpMainBack_common,0,0);
	if (tMenu->tFrontendInfo.bPageBoxes)
		Menu_DrawBox(tMenu->bmpBuffer.get(), 15,130, 625, 465);
	Menu_DrawSubTitle(tMenu->bmpBuffer.get(),SUB_NETWORK);
	cLan.Draw(tMenu->bmpBuffer.get());

	for(int i=1;i<4;i++)
		cNetButtons[i].Draw(tMenu->bmpBuffer.get());

	Menu_RedrawMouse(true);

	int center = VideoPostProcessor::videoSurface()->w/2;
	int y = VideoPostProcessor::videoSurface()->h/2 - INFO_H/2;

    cDetails.Initialize();
	cDetails.Add( new CButton(BUT_REFRESH, tMenu->bmpButtons),  ld_Refresh,	center - 105, y+INFO_H-20, 85,15);
    cDetails.Add( new CButton(BUT_JOIN, tMenu->bmpButtons),	    ld_Join,      center, y+INFO_H-20, 40,15);
	cDetails.Add( new CButton(BUT_OK, tMenu->bmpButtons),	    ld_Ok,      center + 60, y+INFO_H-20, 40,15);
	((CButton *)cDetails.getWidget(ld_Refresh))->setRedrawMenu(false);
	((CButton *)cDetails.getWidget(ld_Ok))->setRedrawMenu(false);
	((CButton *)cDetails.getWidget(ld_Join))->setRedrawMenu(false);

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
            if(ev->iControlID == ld_Ok && ev->iEventMsg == BTN_CLICKED) {
                break;
			// Refresh
            } else if (ev->iControlID == ld_Refresh && ev->iEventMsg == BTN_CLICKED)  {
				fStart = AbsTime();
				bGotDetails = false;
				bOldLxBug = false;
				nTries = 0;
			} else if (ev->iControlID == ld_Join && ev->iEventMsg == BTN_CLICKED)  {

				lv_subitem_t *sub = ((CListview *)cLan.getWidget(nl_ServerList))->getCurSubitem(1);

				// Join
				if (sub)
					Menu_Net_LANJoinServer(szAddress, sub->sText);

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
