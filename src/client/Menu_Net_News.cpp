/////////////////////////////////////////
//
//             Liero Xtreme
//
//     Copyright Auxiliary Software 2002
//
//
/////////////////////////////////////////


// Net menu - News
// Created 12/8/02
// Jason Boettcher


#include "defs.h"
#include "LieroX.h"
#include "Menu.h"

CGuiLayout	cNews;

enum {
	Back = 0,
	NewsBrowser
};


///////////////////
// Initialize the news net menu
int Menu_Net_NewsInitialize(void)
{
	iNetMode = net_main;

	// Setup the gui layout
	cNews.Shutdown();
	cNews.Initialize();

	cNews.Add( new CButton(BUT_BACK, tMenu->bmpButtons), Back, 25,440, 50,15);
	cNews.Add( new CBrowser(), NewsBrowser, 50, 160, 540, 260);


	// Load the news
	CBrowser *b = (CBrowser *)cNews.getWidget(NewsBrowser);
	b->Load("news/news.txt");
 


	
	

	return true;
}


///////////////////
// The net news menu frame
void Menu_Net_NewsFrame(int mouse)
{
	mouse_t		*Mouse = GetMouse();
	gui_event_t *ev;


	// Process & Draw the gui
	ev = cNews.Process();
	cNews.Draw( tMenu->bmpScreen );


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
// TODO: implement sound system
PlaySoundSample(sfxGeneral.smpClick);

					// Shutdown
					cNews.Shutdown();

					// Back to main menu					
					Menu_MainInitialize();
				}
				break;
		}

	}


	// Draw the mouse
	DrawImage(tMenu->bmpScreen,gfxGUI.bmpMouse[mouse], Mouse->X,Mouse->Y);
}
