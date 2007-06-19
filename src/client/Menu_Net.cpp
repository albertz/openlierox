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


#include "defs.h"
#include "LieroX.h"
#include "Menu.h"
#include "GfxPrimitives.h"


enum {
	mn_Internet=0,
	mn_LAN,
	mn_Host,
	mn_Favourites
};


int		iNetMode = net_main;
int		iHostType = 0;
CButton	cNetButtons[5];


///////////////////
// Initialize the net menu
int Menu_NetInitialize(void)
{
	tMenu->iMenuType = MNU_NETWORK;
	iNetMode = net_main;

	// Create the buffer
	DrawImage(tMenu->bmpBuffer,tMenu->bmpMainBack_wob,0,0);
	if (tMenu->tFrontendInfo.bPageBoxes)
		Menu_DrawBox(tMenu->bmpBuffer, 15,130, 625, 465);
	Menu_DrawSubTitle(tMenu->bmpBuffer,SUB_NETWORK);
	Menu_RedrawMouse(true);

	// Setup the top buttons
    for(int i=mn_Internet; i<=mn_Favourites; i++) {
		cNetButtons[i] = CButton(BUT_INTERNET+i,	tMenu->bmpButtons);
        cNetButtons[i].Create();
    }
	
	cNetButtons[mn_Internet].Setup(mn_Internet, 145, 110, 95, 15);
	cNetButtons[mn_LAN].Setup(mn_LAN, 260, 110, 40, 15);
	cNetButtons[mn_Host].Setup(mn_Host, 325, 110, 50, 15);
	cNetButtons[mn_Favourites].Setup(mn_Favourites, 400, 110, 105, 15);
	//cNetButtons[4].Setup(4, 400, 110, 105, 15);

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
	default:
		Menu_Net_NETInitialize();
	}

	return true;
}


///////////////////
// Go to the host lobby
void Menu_Net_GotoHostLobby(void)
{
	tMenu->iMenuType = MNU_NETWORK;
	tMenu->iMenuRunning = true;
	tMenu->bmpScreen = SDL_GetVideoSurface();
	iNetMode = net_host;

	// Create the buffer
	DrawImage(tMenu->bmpBuffer,tMenu->bmpMainBack_wob,0,0);
	if (tMenu->tFrontendInfo.bPageBoxes)
		Menu_DrawBox(tMenu->bmpBuffer, 15,130, 625, 465);
	Menu_DrawSubTitle(tMenu->bmpBuffer,SUB_NETWORK);
	Menu_RedrawMouse(true);

	// Setup the top buttons
	for(int i=mn_Internet; i<=mn_Favourites; i++)
		cNetButtons[i] = CButton(BUT_MAIN+i,	tMenu->bmpButtons);
	
	cNetButtons[mn_Internet].Setup(0, 205, 110, 95, 15);
	cNetButtons[mn_LAN].Setup(1, 320, 110, 40, 15);
	cNetButtons[mn_Host].Setup(2, 385, 110, 50, 15);
	cNetButtons[mn_Favourites].Setup(3, 460, 110, 50, 15);
	//cNetButtons[4].Setup(4, 460, 110, 50, 15);

	Menu_Net_HostGotoLobby();

	tLX->iQuitEngine = true;
	iSkipStart = true;
}


///////////////////
// Go to the join lobby
void Menu_Net_GotoJoinLobby(void)
{
    printf("Menu_Net_GotoJoinLobby()\n");

	tMenu->bmpScreen = SDL_GetVideoSurface();
	tMenu->iMenuRunning = true;

	tMenu->iMenuType = MNU_NETWORK;
	iNetMode = net_join;

	// Create the buffer
	DrawImage(tMenu->bmpBuffer,tMenu->bmpMainBack_wob,0,0);
	if (tMenu->tFrontendInfo.bPageBoxes)
		Menu_DrawBox(tMenu->bmpBuffer, 15,130, 625, 465);
	Menu_DrawSubTitle(tMenu->bmpBuffer,SUB_NETWORK);
	Menu_RedrawMouse(true);

	// Setup the top buttons
	for(int i=0; i<5; i++)
		cNetButtons[i] = CButton(BUT_MAIN+i,	tMenu->bmpButtons);
	
	cNetButtons[mn_Internet].Setup(mn_Internet, 205, 110, 95, 15);
	cNetButtons[mn_LAN].Setup(mn_LAN, 320, 110, 40, 15);
	cNetButtons[mn_Host].Setup(mn_Host, 385, 110, 50, 15);
	cNetButtons[mn_Favourites].Setup(mn_Favourites, 460, 110, 50, 15);
	//cNetButtons[4].Setup(4, 460, 110, 50, 15);

	Menu_Net_JoinGotoLobby();

	tLX->iQuitEngine = true;
	iSkipStart = true;
}


///////////////////
// The main net menu frame
void Menu_NetFrame(void)
{
	int		mouse = 0;
	mouse_t	*Mouse = GetMouse();


	// Refresh the main window of the screen
	//DrawImageAdv(tMenu->bmpScreen, tMenu->bmpBuffer, 20,140,  20,140,  620,340);
	//DrawImageAdv(tMenu->bmpScreen, tMenu->bmpBuffer, 120,110, 120,110, 420,30);


	// Process the top buttons
	if((iNetMode != net_host || iHostType == 0) &&
	   (iNetMode != net_join)) {

		cNetButtons[iNetMode-1].MouseOver(Mouse);
		for(int i=mn_Internet;i<=mn_Favourites;i++) {
		
			cNetButtons[i].Draw(tMenu->bmpScreen);

			if(i==iNetMode-1)
				continue;

			if(cNetButtons[i].InBox(Mouse->X,Mouse->Y)) {
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

                    iNetMode = i+1;

                    // Redraw the window section
                    DrawImageAdv(tMenu->bmpScreen, tMenu->bmpBuffer, 20,140,  20,140,  620,340);

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
					}
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
	}
}
