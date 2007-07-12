/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Net menu - News
// Created 12/8/02
// Jason Boettcher


#include "LieroX.h"
#include "Graphics.h"
#include "Menu.h"
#include "GfxPrimitives.h"

CGuiLayout	cNews;

enum {
	nw_Back = 0,
	nw_NewsBrowser
};


///////////////////
// Initialize the news net menu
int Menu_Net_NewsInitialize(void)
{
	iNetMode = net_main;

	// Setup the gui layout
	cNews.Shutdown();
	cNews.Initialize();

	cNews.Add( new CButton(BUT_BACK, tMenu->bmpButtons), nw_Back, 25,440, 50,15);
	cNews.Add( new CBrowser(), nw_NewsBrowser, 50, 160, 540, 260);


	// Load the news
	CBrowser *b = (CBrowser *)cNews.getWidget(nw_NewsBrowser);
	b->Load("news/news.txt");
 


	
	

	return true;
}


///////////////////
// The net news menu frame
void Menu_Net_NewsFrame(int mouse)
{
	gui_event_t *ev = NULL;


	// Process & Draw the gui
#ifdef WITH_MEDIAPLAYER
	if (!cMediaPlayer.GetDrawPlayer())
#endif
		ev = cNews.Process();
	cNews.Draw( tMenu->bmpScreen );


	// Process any events
	if(ev) {

		switch(ev->iControlID) {

			// Back
			case nw_Back:
				if(ev->iEventMsg == BTN_MOUSEUP) {

					// Click!
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
	DrawCursor(tMenu->bmpScreen);
}
