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

#include <iostream>

#include "LieroX.h"
#include "Sounds.h"
#include "Error.h"
#include "DeprecatedGUI/Menu.h"
#include "GfxPrimitives.h"
#include "DeprecatedGUI/CButton.h"
#include "DeprecatedGUI/CAnimation.h"
#include "DeprecatedGUI/CProgressbar.h"
#include "DeprecatedGUI/CLabel.h"
#include "AuxLib.h"
#include "IpToCountryDB.h"


using namespace std;

namespace DeprecatedGUI {

enum {
	mn_Internet=0,
	mn_LAN,
	mn_Host,
	mn_Favourites,
	mn_News,
};


// TODO: remove globals
int		iNetMode = net_main;
int		iHostType = 0;
CButton	cNetButtons[5];
CAnimation cIpToCountryAnim;
CProgressBar cIpToCountryProgress;
CLabel		cIpToCountryLabel;
SDL_Rect tLoadingRect;



///////////////////
// Initialize the net menu
bool Menu_NetInitialize(void)
{
	if(bDedicated) return true; // just ignore
	
	tMenu->iMenuType = MNU_NETWORK;
	iNetMode = net_main;

	// Create the buffer
	DrawImage(tMenu->bmpBuffer.get(),tMenu->bmpMainBack_common,0,0);
	if (tMenu->tFrontendInfo.bPageBoxes)
		Menu_DrawBox(tMenu->bmpBuffer.get(), 15,130, 625, 465);
	Menu_DrawSubTitle(tMenu->bmpBuffer.get(),SUB_NETWORK);
	Menu_RedrawMouse(true);

	// Setup the animation
	cIpToCountryAnim = CAnimation("data/frontend/iploading_anim.png", tMenu->tFrontendInfo.fLoadingAnimFrameTime);
	cIpToCountryAnim.Setup(-1, tMenu->tFrontendInfo.iLoadingAnimLeft, tMenu->tFrontendInfo.iLoadingAnimTop, 0, 0);
	cIpToCountryAnim.Create();

	// Setup the progressbar
	cIpToCountryProgress = CProgressBar(LoadGameImage("data/frontend/iploading_progress.png",true), 0, 0, false, 1);
	cIpToCountryProgress.Setup(-1, tMenu->tFrontendInfo.iLoadingBarLeft, tMenu->tFrontendInfo.iLoadingBarTop, 0, 0);
	cIpToCountryProgress.SetPosition(0);
	cIpToCountryProgress.Create();
	cIpToCountryProgress.setRedrawMenu(false);

	// Setup the label
	cIpToCountryLabel = CLabel("Loading IpToCountry database...", tLX->clIpLoadingLabel);
	cIpToCountryLabel.Setup(-1, tMenu->tFrontendInfo.iLoadingLabelLeft, tMenu->tFrontendInfo.iLoadingLabelTop, 0, 0);
	cIpToCountryLabel.Create();
	cIpToCountryLabel.setRedrawMenu(false);

	// Get the loading rect (used for redrawing)
	tLoadingRect.x = MIN(tMenu->tFrontendInfo.iLoadingLabelLeft, MIN(tMenu->tFrontendInfo.iLoadingAnimLeft, tMenu->tFrontendInfo.iLoadingBarLeft));
	tLoadingRect.y = MIN(tMenu->tFrontendInfo.iLoadingLabelTop, MIN(tMenu->tFrontendInfo.iLoadingAnimTop, tMenu->tFrontendInfo.iLoadingBarTop));
	tLoadingRect.w = MAX(cIpToCountryLabel.getX() + cIpToCountryLabel.getWidth(),
					 MAX(cIpToCountryAnim.getX() + cIpToCountryAnim.getWidth(),
						 cIpToCountryProgress.getX() + cIpToCountryProgress.getWidth())) - tLoadingRect.x;
	tLoadingRect.h = MAX(cIpToCountryLabel.getY() + cIpToCountryLabel.getHeight(),
					 MAX(cIpToCountryAnim.getY() + cIpToCountryAnim.getHeight(),
						 cIpToCountryProgress.getY() + cIpToCountryProgress.getHeight())) - tLoadingRect.y;

	// Setup the top buttons
	int image_ids[] = {BUT_INTERNET, BUT_LAN, BUT_HOST, BUT_FAVOURITES, BUT_NEWS};
    for(size_t i=0; i < sizeof(image_ids) / sizeof(int); ++i) {
    	cNetButtons[i].setImage(tMenu->bmpButtons);
    	cNetButtons[i].setImageID(image_ids[i]);
        cNetButtons[i].Create();
    }

	cNetButtons[mn_Internet].Setup(mn_Internet, 95, 110, 95, 15);
	cNetButtons[mn_LAN].Setup(mn_LAN, 210, 110, 40, 15);
	cNetButtons[mn_Host].Setup(mn_Host, 275, 110, 50, 15);
	cNetButtons[mn_Favourites].Setup(mn_Favourites, 350, 110, 105, 15);
	cNetButtons[mn_News].Setup(mn_News, 480, 110, 105, 15);
	//cNetButtons[4].Setup(4, 400, 110, 105, 15);


	// Init the IP to country database (if not already inited)
	if (!tIpToCountryDB)  {
		tIpToCountryDB = new IpToCountryDB("ip_to_country.csv");
		if (!tIpToCountryDB)  {
			SystemError("Could not allocate the IP to Country database.");
			return false;
		}
	}

	// In case the database hasn't been loaded yet (for example user was here -> went to
	// Options -> enalbed this DB and came back)
	// HINT: if the file is already loaded/being loaded, the database won't try to load it again
	tIpToCountryDB->LoadDBFile("ip_to_country.csv");

	// Return back to the menu we came from
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
	default:
		Menu_Net_NETInitialize();
	}

	return true;
}


///////////////////
// Go to the host lobby
void Menu_Net_GotoHostLobby(void)
{
	cout << "Menu_Net_GotoHostLobby" << endl;

	tMenu->iMenuType = MNU_NETWORK;
	tMenu->bMenuRunning = true;
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
		cNetButtons[mn_News].Setup(mn_News, 460, 110, 50, 15);
		//cNetButtons[4].Setup(4, 460, 110, 50, 15);
	}

	Menu_Net_HostGotoLobby();

	SetQuitEngineFlag("Menu_Net_GotoHostLobby");
	iSkipStart = true;
}


///////////////////
// Go to the join lobby
void Menu_Net_GotoJoinLobby(void)
{
    printf("Menu_Net_GotoJoinLobby()\n");

	tMenu->bMenuRunning = true;

	tMenu->iMenuType = MNU_NETWORK;
	iNetMode = net_join;

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

	SetQuitEngineFlag("Menu_Net_GotoJoinLobby");
	iSkipStart = true;
}


///////////////////
// The main net menu frame
void Menu_NetFrame(void)
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
		for(int i=mn_Internet; i<=mn_News; i++) {

			cNetButtons[i].Draw(VideoPostProcessor::videoSurface());

			if( i == iNetMode-1 )
				continue;

			if( cNetButtons[i].InBox(Mouse->X, Mouse->Y) ) {
				cNetButtons[i].MouseOver(Mouse);
				mouse = 1;
				if(Mouse->Up) {
					PlaySoundSample(sfxGeneral.smpClick);

					// Call a shutdown on all the highest net menu's
					Menu_Net_MainShutdown();
					Menu_Net_LANShutdown();
					Menu_Net_NETShutdown();
					Menu_Net_HostShutdown();
					Menu_Net_FavouritesShutdown();
					Menu_Net_NewsShutdown();

                    iNetMode = i+1;

                    // Redraw the window section
                    DrawImageAdv(VideoPostProcessor::videoSurface(), tMenu->bmpBuffer, 20,140,  20,140,  620,340);

					// Initialize the appropriate menu
					switch(iNetMode) {

						// Main
						case 0:
							Menu_Net_MainInitialize();
							break;

						// LAN
						case 2:
							Menu_Net_LANInitialize();
							break;

						// Internet
						case 1:
							Menu_Net_NETInitialize();
							break;

						// Host
						case 3:
							Menu_Net_HostInitialize();
							break;

						// Favourites
						case 4:
							Menu_Net_FavouritesInitialize();
							break;

						// News
						case 5:
							Menu_Net_NewsInitialize();
							break;
					}
				}
			}
		}
	}

	// Don't draw in host lobby/join (no space for it)
	if (!(iNetMode == net_host && iHostType == 1) && iNetMode != net_join)  {
		Menu_redrawBufferRect(tLoadingRect.x, tLoadingRect.y, tLoadingRect.w, tLoadingRect.h);
		if (!tIpToCountryDB->Loaded())  {

			// Draw the animation
			cIpToCountryAnim.Draw( VideoPostProcessor::videoSurface() );

			// Draw the label
			cIpToCountryLabel.Draw( VideoPostProcessor::videoSurface() );

			// Setup and draw the progressbar
			cIpToCountryProgress.SetPosition(tIpToCountryDB->GetProgress());
			cIpToCountryProgress.Draw( VideoPostProcessor::videoSurface() );
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
	}
}

////////////
// Shutdown
void Menu_NetShutdown(void)
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
	}
}

}; // namespace DeprecatedGUI
