/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Net menu - Main
// Created 16/12/02
// Jason Boettcher


#include "LieroX.h"
#include "Sounds.h"
#include "DeprecatedGUI/Graphics.h"
#include "DeprecatedGUI/Menu.h"
#include "GfxPrimitives.h"
#include "DeprecatedGUI/CButton.h"
#include "DeprecatedGUI/CBrowser.h"
#include "DeprecatedGUI/CLabel.h"
#include "AuxLib.h"


namespace DeprecatedGUI {

CGuiLayout	cMain;

enum {
	nm_Back = 0,
	nm_PlayerList,
    nm_NewsBrowser
};


///////////////////
// Initialize the main net menu
bool Menu_Net_MainInitialize()
{
	iNetMode = net_main;

	// Setup the gui layout
	cMain.Shutdown();
	cMain.Initialize();

	cMain.Add( new CButton(BUT_BACK, tMenu->bmpButtons),	nm_Back, 25,440, 50,15);
   	cMain.Add( new CBrowser(), nm_NewsBrowser, 40, 160, 560, 270);


	// Load the news
	CBrowser *b = (CBrowser *)cMain.getWidget(nm_NewsBrowser);
	b->LoadFromFile("cfg/news.txt");

	/*cMain.Add( new CListview(),								PlayerList, 40,150,150,150);


	cMain.SendMessage(PlayerList,	LVM_ADDCOLUMN, (DWORD)"Players", 22);
	cMain.SendMessage(PlayerList,	LVM_ADDCOLUMN, (DWORD)"", 60);

	// Add the players to the list
	CListview *lv = (CListview *)cMain.getWidget(PlayerList);

	profile_t *p = GetProfiles();
	for(; p; p=p->tNext) {
		if(p->iType == PRF_COMPUTER)
			continue;
		lv->AddItem("",p->iID,tLX->clListView);
		lv->AddSubitem(LVS_IMAGE,"",p->bmpWorm);
		lv->AddSubitem(LVS_TEXT, p->sName, NULL);
	}*/

	cMain.Add( new CLabel("OpenLieroX News", tLX->clNormalLabel), -1, 255, 140, 0,0);

	Menu_Net_AddTabBarButtons(&cMain);


	return true;
}


///////////////////
// Shutdown the main net menu
void Menu_Net_MainShutdown()
{
	cMain.Shutdown();
}


///////////////////
// The net main menu frame
void Menu_Net_MainFrame(int mouse)
{
	gui_event_t *ev = NULL;


	// Process & Draw the gui
	ev = cMain.Process();
	cMain.Draw( VideoPostProcessor::videoSurface() );

	if (Menu_Net_ProcessTabBarButtons(ev))
		return;

	// Process any events
	if(ev) {

		switch(ev->iControlID) {

			// Back
			case nm_Back:
				if(ev->iEventMsg == BTN_CLICKED) {

					// Click!
					PlaySoundSample(sfxGeneral.smpClick);

					// Shutdown
					cMain.Shutdown();

					// Back to main menu
					Menu_MainInitialize();
				}
				break;
		}

	}


	// Draw the mouse
	DrawCursor(VideoPostProcessor::videoSurface());
}

}; // namespace DeprecatedGUI
