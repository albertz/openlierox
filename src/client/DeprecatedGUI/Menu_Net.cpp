/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Net menu
// Created 12/8/02
// Jason Boettcher



#include "LieroX.h"
#include "sound/SoundsBase.h"
#include "Error.h"
#include "DeprecatedGUI/Menu.h"
#include "GfxPrimitives.h"
#include "DeprecatedGUI/CButton.h"
#include "DeprecatedGUI/CAnimation.h"
#include "DeprecatedGUI/CProgressbar.h"
#include "DeprecatedGUI/CLabel.h"
#include "DeprecatedGUI/CChatWidget.h"
#include "AuxLib.h"
#include "IRC.h"
#include "DedicatedControl.h"
#include "Debug.h"



namespace DeprecatedGUI {

enum {
	mn_Internet=0,
	mn_LAN,
	mn_Host,
	mn_Favourites,
	mn_News,
	mn_Chat,
};


// TODO: remove globals
int		iNetMode = net_main;
int		iHostType = 0;
CButton	cNetButtons[6];
SDL_Rect tLoadingRect;



///////////////////
// Initialize the net menu
bool Menu_NetInitialize(bool withSubMenu)
{
	if(bDedicated) return true; // just ignore
	
	tMenu->iMenuType = MNU_NETWORK;
	iNetMode = net_main;

	// Add all the network sockets to the notifier groups (for events)
	Menu_EnableNetEvents();

	// Create the buffer
	DrawImage(tMenu->bmpBuffer.get(),tMenu->bmpMainBack_common,0,0);
	if (tMenu->tFrontendInfo.bPageBoxes)
		Menu_DrawBox(tMenu->bmpBuffer.get(), 15,130, 625, 465);
	Menu_DrawSubTitle(tMenu->bmpBuffer.get(),SUB_NETWORK);
	Menu_RedrawMouse(true);

	// Setup the top buttons
	int image_ids[] = {BUT_INTERNET, BUT_LAN, BUT_HOST, BUT_FAVOURITES, BUT_NEWS, BUT_CHAT};
    for(size_t i=0; i < sizeof(image_ids) / sizeof(int); ++i) {
    	cNetButtons[i].setImage(tMenu->bmpButtons);
    	cNetButtons[i].setImageID(image_ids[i]);
        cNetButtons[i].Create();
    }

	cNetButtons[mn_Internet].Setup(mn_Internet, 75, 110, 95, 15);
	cNetButtons[mn_LAN].Setup(mn_LAN, 190, 110, 40, 15);
	cNetButtons[mn_Host].Setup(mn_Host, 255, 110, 50, 15);
	cNetButtons[mn_Favourites].Setup(mn_Favourites, 330, 110, 105, 15);
	cNetButtons[mn_News].Setup(mn_News, 460, 110, 50, 15);
	cNetButtons[mn_Chat].Setup(mn_Chat, 535, 110, 50, 15);
	//cNetButtons[4].Setup(4, 400, 110, 105, 15);

	// Initialize the IRC support
	InitializeIRC();

	// Return back to the menu we came from
	if(withSubMenu)
		switch(tMenu->iReturnTo)  {
		case net_internet:
			Menu_Net_NETInitialize();
			break;
		case net_lan:
			Menu_Net_LANInitialize();
			break;
		case net_favourites:
			Menu_Net_FavouritesInitialize();
			break;
		case net_news:
			Menu_Net_NewsInitialize();
			break;
		case net_chat:
			Menu_Net_ChatInitialize();
			break;
		default:
			Menu_Net_NETInitialize();
		}

	return true;
}


///////////////////
// Go to the host lobby
void Menu_Net_GotoHostLobby()
{
	notes << "Menu_Net_GotoHostLobby" << endl;

	tMenu->iMenuType = MNU_NETWORK;
	iNetMode = net_host;

	if(!bDedicated) {
		// Create the buffer
		DrawImage(tMenu->bmpBuffer.get(),tMenu->bmpMainBack_common,0,0);
		if (tMenu->tFrontendInfo.bPageBoxes)
			Menu_DrawBox(tMenu->bmpBuffer.get(), 15,130, 625, 465);
		Menu_DrawSubTitle(tMenu->bmpBuffer.get(),SUB_NETWORK);
		Menu_RedrawMouse(true);


		cNetButtons[mn_Internet].Setup(mn_Internet, 205, 110, 95, 15);
		cNetButtons[mn_LAN].Setup(mn_LAN, 320, 110, 40, 15);
		cNetButtons[mn_Host].Setup(mn_Host, 385, 110, 50, 15);
		cNetButtons[mn_Favourites].Setup(mn_Favourites, 460, 110, 50, 15);
		//cNetButtons[4].Setup(4, 460, 110, 50, 15);
	}

	Menu_Net_HostGotoLobby();

	SetQuitEngineFlag("Menu_Net_GotoHostLobby");
	iSkipStart = true;
}


///////////////////
// The main net menu frame
void Menu_NetFrame()
{
	int		mouse = 0;
	mouse_t	*Mouse = GetMouse();


	// Refresh the main window of the screen
	//DrawImageAdv(VideoPostProcessor::videoSurface(), tMenu->bmpBuffer, 20,140,  20,140,  620,340);
	//DrawImageAdv(VideoPostProcessor::videoSurface(), tMenu->bmpBuffer, 120,110, 120,110, 420,30);


	// Process the top buttons
	if((iNetMode != net_host || iHostType == 0) &&
	   (iNetMode != net_join)) {

		cNetButtons[iNetMode-1].MouseOver(Mouse);
		for(int i=mn_Internet; i<=mn_Chat && mouse == 0; i++) {

			cNetButtons[i].Draw(VideoPostProcessor::videoSurface());

			if( i == iNetMode-1 )
				continue;

			if( cNetButtons[i].InBox(Mouse->X, Mouse->Y) ) {
				cNetButtons[i].MouseOver(Mouse);
			}
			
			if( (cNetButtons[i].InBox(Mouse->X, Mouse->Y) && Mouse->Up) || 
				( i == mn_Chat && CChatWidget::GlobalEnabled() && iNetMode != net_chat ) ) {

				CChatWidget::GlobalSetEnabled(false);
				mouse = 1;

				PlaySoundSample(sfxGeneral.smpClick);

				// Call a shutdown on all the highest net menu's
				Menu_Net_MainShutdown();
				Menu_Net_LANShutdown();
				Menu_Net_NETShutdown();
				Menu_Net_HostShutdown();
				Menu_Net_FavouritesShutdown();
				Menu_Net_NewsShutdown();
				Menu_Net_ChatShutdown();

                iNetMode = i+1;

                // Redraw the window section
                DrawImageAdv(VideoPostProcessor::videoSurface(), tMenu->bmpBuffer, 20,140,  20,140,  620,340);

				// Initialize the appropriate menu
				switch(iNetMode) {

					// Main
					case net_main:
						Menu_Net_MainInitialize();
						break;

					// LAN
					case net_lan:
						Menu_Net_LANInitialize();
						break;

					// Internet
					case net_internet:
						Menu_Net_NETInitialize();
						break;

					// Host
					case net_host:
						Menu_Net_HostInitialize();
						break;

					// Favourites
					case net_favourites:
						Menu_Net_FavouritesInitialize();
						break;

					// News
					case net_news:
						Menu_Net_NewsInitialize();
						break;

					// Chat
					case net_chat:
						Menu_Net_ChatInitialize();
						break;
				}
			}
		}
	}

	// Run the frame of the current menu screen
	switch(iNetMode) {

		// Main
		case net_main:
			Menu_Net_MainFrame(mouse);
			break;

		// Internet
		case net_internet:
			Menu_Net_NETFrame(mouse);
			break;

		// LAN
		case net_lan:
			Menu_Net_LANFrame(mouse);
			break;

		// Host
		case net_host:
			Menu_Net_HostFrame(mouse);
			break;

		// Favourites
		case net_favourites:
			Menu_Net_FavouritesFrame(mouse);
			break;

		// Join
		case net_join:
			Menu_Net_JoinFrame(mouse);
			break;

		// News
		case net_news:
			Menu_Net_NewsFrame(mouse);
			break;

		// Chat
		case net_chat:
			Menu_Net_ChatFrame(mouse);
			break;
	}
}

////////////
// Shutdown
void Menu_NetShutdown()
{
	switch(iNetMode) {

		// Main
		case net_main:
			Menu_Net_MainShutdown();
			break;

		// Internet
		case net_internet:
			Menu_Net_NETShutdown();
			break;

		// LAN
		case net_lan:
			Menu_Net_LANShutdown();
			break;

		// Host
		case net_host:
			Menu_Net_HostShutdown();
			break;

		// Favourites
		case net_favourites:
			Menu_Net_FavouritesShutdown();
			break;

		// Join
		case net_join:
			Menu_Net_JoinShutdown();
			break;

		// News
		case net_news:
			Menu_Net_NewsShutdown();
			break;

		// Chat
		case net_chat:
			Menu_Net_ChatShutdown();
			break;
	}

	// Remove all the network sockets from the notifier groups (we don't want any events anymore)
	Menu_DisableNetEvents();
}

void Menu_Net_ServerList_Refresher() {
	if(bDedicated) return; // not needed to do that

	struct Refresher : Action {
		Result handle() {
			switch(iNetMode) {
				case net_internet:
					Menu_Net_NET_ServerList_Refresher();
					break;
				case net_lan:
					Menu_Net_LAN_ServerList_Refresher();
					break;
				case net_favourites:
					Menu_Net_Favourites_ServerList_Refresher();
					break;
				default:
					// ignore
					break;
			}
			return true;
		}
	};
	mainQueue->push(new Refresher());
}
	
}; // namespace DeprecatedGUI


///////////////////
// Go to the join lobby
void GotoJoinLobby()
{
    notes << "GotoJoinLobby()" << endl;
	
	using namespace DeprecatedGUI;
	
	tMenu->iMenuType = MNU_NETWORK;
	iNetMode = net_join;
	
	if(!bDedicated) {
		
		// Create the buffer
		DrawImage(tMenu->bmpBuffer.get(),tMenu->bmpMainBack_common,0,0);
		if (tMenu->tFrontendInfo.bPageBoxes)
			Menu_DrawBox(tMenu->bmpBuffer.get(), 15,130, 625, 465);
		Menu_DrawSubTitle(tMenu->bmpBuffer.get(),SUB_NETWORK);
		Menu_RedrawMouse(true);
		
		cNetButtons[mn_Internet].Setup(mn_Internet, 205, 110, 95, 15);
		cNetButtons[mn_LAN].Setup(mn_LAN, 320, 110, 40, 15);
		cNetButtons[mn_Host].Setup(mn_Host, 385, 110, 50, 15);
		cNetButtons[mn_Favourites].Setup(mn_Favourites, 460, 110, 50, 15);
		//cNetButtons[4].Setup(4, 460, 110, 50, 15);
		
		Menu_Net_JoinGotoLobby();
	} else {
		if( DedicatedControl::Get() )
			DedicatedControl::Get()->BackToClientLobby_Signal();
	}
	
	SetQuitEngineFlag("GotoJoinLobby");
	iSkipStart = true;
}

