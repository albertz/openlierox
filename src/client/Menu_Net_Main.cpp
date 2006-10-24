/////////////////////////////////////////
//
//             Liero Xtreme
//
//     Copyright Auxiliary Software 2002
//
//
/////////////////////////////////////////


// Net menu - Main
// Created 16/12/02
// Jason Boettcher


#include "defs.h"
#include "LieroX.h"
#include "Menu.h"

CGuiLayout	cMain;

enum {
	Back = 0,
	PlayerList,
    NewsBrowser
};


///////////////////
// Initialize the main net menu
int Menu_Net_MainInitialize(void)
{
	iNetMode = net_main;

	// Setup the gui layout
	cMain.Shutdown();
	cMain.Initialize();

	cMain.Add( new CButton(BUT_BACK, tMenu->bmpButtons),	Back, 25,440, 50,15);
   	cMain.Add( new CBrowser(), NewsBrowser, 40, 160, 560, 270);


	// Load the news
	CBrowser *b = (CBrowser *)cMain.getWidget(NewsBrowser);
	b->Load("cfg/news.txt");
 
	/*cMain.Add( new CListview(),								PlayerList, 40,150,150,150);
	

	cMain.SendMessage(PlayerList,	LVM_ADDCOLUMN, (DWORD)"Players", 22);
	cMain.SendMessage(PlayerList,	LVM_ADDCOLUMN, (DWORD)"", 60);

	// Add the players to the list
	CListview *lv = (CListview *)cMain.getWidget(PlayerList);

	profile_t *p = GetProfiles();
	for(; p; p=p->tNext) {
		if(p->iType == PRF_COMPUTER)
			continue;
		lv->AddItem("",p->iID,0xffff);
		lv->AddSubitem(LVS_IMAGE,"",p->bmpWorm);
		lv->AddSubitem(LVS_TEXT, p->sName, NULL);
	}*/

	cMain.Add( new CLabel("Liero Xtreme News", 0xffff), -1, 255, 140, 0,0);


	return true;
}


///////////////////
// Shutdown the main net menu
void Menu_Net_MainShutdown(void)
{
	cMain.Shutdown();
}


///////////////////
// The net main menu frame
void Menu_Net_MainFrame(int mouse)
{
	mouse_t		*Mouse = GetMouse();
	gui_event_t *ev;


	// Process & Draw the gui
	ev = cMain.Process();
	cMain.Draw( tMenu->bmpScreen );


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

					// Click!
					BASS_SamplePlay(sfxGeneral.smpClick);

					// Shutdown
					cMain.Shutdown();

					// Back to main menu					
					Menu_MainInitialize();
				}
				break;
		}

	}


	// Draw the mouse
	DrawImage(tMenu->bmpScreen,gfxGUI.bmpMouse[mouse], Mouse->X,Mouse->Y);
}