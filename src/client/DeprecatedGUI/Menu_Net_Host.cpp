/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Net menu - Hosting
// Created 12/8/02
// Jason Boettcher

#include "LieroX.h"
#include "IpToCountryDB.h"
#include "DeprecatedGUI/Graphics.h"
#include "CClient.h"
#include "CServer.h"
#include "DeprecatedGUI/Menu.h"
#include "GfxPrimitives.h"
#include "StringUtils.h"
#include "CWorm.h"
#include "Protocol.h"
#include "AuxLib.h"
#include "DeprecatedGUI/CLabel.h"
#include "DeprecatedGUI/CButton.h"
#include "DeprecatedGUI/CListview.h"
#include "DeprecatedGUI/CTextbox.h"
#include "DeprecatedGUI/CCheckbox.h"
#include "DeprecatedGUI/CMenu.h"
#include "DeprecatedGUI/CImage.h"
#include "DeprecatedGUI/CBox.h"
#include "DeprecatedGUI/CGuiSkinnedLayout.h"
#include "DeprecatedGUI/CBrowser.h"
#include "MathLib.h"
#include "Clipboard.h"
#include "CClientNetEngine.h"
#include "CChannel.h"
#include "CServerConnection.h"
#include "ProfileSystem.h"
#include "Debug.h"
#include "console.h"
#include "CGameMode.h"
#include "DeprecatedGUI/CChatWidget.h"


/*
   The hosting menu is in two parts
     1) Selecting the players _we_ want to play as
     2) The lobby screen were other players can connect
*/



/*
==================================

		Player selection

==================================
*/

namespace DeprecatedGUI {

CGuiLayout	cHostPly;

// Player gui
enum {
	hs_Back=0,
	hs_Ok,
	hs_PlayerList,
	hs_Playing,
	hs_Servername,
	hs_MaxPlayers,
	hs_Register,
    hs_Password,
	hs_WelcomeMessage,
	hs_AllowWantsJoin,
	hs_WantsJoinBanned,
	hs_AllowRemoteBots,
	hs_AllowNickChange,
	hs_ServerSideHealth
};

int iHumanPlayers = 0;

bool		bSpeedTestDialog = false;
bool		bTestedSpeed = false;
AbsTime		fHostMenuPlyStart = AbsTime();

///////////////////
// Initialize the host menu
bool Menu_Net_HostInitialize()
{
	iNetMode = net_host;
	iHostType = 0;
	bTestedSpeed = false;
	bSpeedTestDialog = false;
	fHostMenuPlyStart = tLX->currentTime;

	// Player gui layout
	cHostPly.Shutdown();
	cHostPly.Initialize();

    cHostPly.Add( new CLabel("Player settings",tLX->clHeading), -1, 350,140,0,0);
	cHostPly.Add( new CLabel("Server settings",tLX->clHeading), -1, 30, 140,0,0);
	cHostPly.Add( new CButton(BUT_BACK, tMenu->bmpButtons), hs_Back,	25, 440,50, 15);
	cHostPly.Add( new CButton(BUT_OK, tMenu->bmpButtons),   hs_Ok,			585,440,30, 15);

	cHostPly.Add( new CListview(),							hs_PlayerList,	360,160,200,105);
	cHostPly.Add( new CListview(),							hs_Playing,		360,275,200,105);
	cHostPly.Add( new CLabel("Server name",	tLX->clNormalLabel),		-1,			40, 178,0,  0);
	cHostPly.Add( new CTextbox(),							hs_Servername,	175,175,140,tLX->cFont.GetHeight());
	cHostPly.Add( new CLabel("Max Players",	tLX->clNormalLabel),		-1,			40,	208,0,  0);
	cHostPly.Add( new CTextbox(),							hs_MaxPlayers,	175,205,140,tLX->cFont.GetHeight());
    //cHostPly.Add( new CLabel("Password",	tLX->clNormalLabel),	    -1,			40, 238,0,  0);
	//cHostPly.Add( new CTextbox(),		                    hs_Password,	175,235,140,tLX->cFont.GetHeight());
	cHostPly.Add( new CLabel("Welcome Message",	tLX->clNormalLabel),		-1,			40, 238,0,  0);
	cHostPly.Add( new CTextbox(),							hs_WelcomeMessage, 175,235,140,tLX->cFont.GetHeight());
	cHostPly.Add( new CLabel("Register Server",	tLX->clNormalLabel),	-1,			153, 268,0,  0);
	cHostPly.Add( new CCheckbox(false),	                    hs_Register,	270,265,17, 17);
	cHostPly.Add( new CLabel("Allow \"Wants to Join\" Messages",	tLX->clNormalLabel),-1,	47, 298,0,  0);
	cHostPly.Add( new CCheckbox(false),	                    hs_AllowWantsJoin,	270,295,17, 17);
	cHostPly.Add( new CLabel("\"Wants to Join\" from Banned Clients",			tLX->clNormalLabel),-1,	22, 328,0,  0);
	cHostPly.Add( new CCheckbox(false),	                    hs_WantsJoinBanned,	270,325,17, 17);
	cHostPly.Add( new CLabel("Allow Bots in Server",		tLX->clNormalLabel),-1,	115, 358,0,  0);
	cHostPly.Add( new CCheckbox(false),	                    hs_AllowRemoteBots,	270,355,17, 17);
	cHostPly.Add( new CLabel("Allow Nick Changing",			tLX->clNormalLabel),-1,	115, 388,0,  0);
	cHostPly.Add( new CCheckbox(false),	                    hs_AllowNickChange,	270,385,17, 17);
	//cHostPly.Add( new CLabel("Server-side Health",			tLX->clNormalLabel),-1,	125, 418,0,  0);
	//cHostPly.Add( new CCheckbox(false),	                    hs_ServerSideHealth,	270,415,17, 17);

	Menu_Net_AddTabBarButtons(&cHostPly);

	cHostPly.getWidget(hs_Playing)->setKeyboardNavigationOrder(1);
	cHostPly.getWidget(hs_Back)->setKeyboardNavigationOrder(1);
	cHostPly.getWidget(hs_Ok)->setKeyboardNavigationOrder(1);

	cHostPly.SendMessage(hs_Playing,		LVM_SETOLDSTYLE, (DWORD)0, 0);
	cHostPly.SendMessage(hs_PlayerList,		LVM_SETOLDSTYLE, (DWORD)0, 0);

	cHostPly.SendMessage(hs_Servername,TXM_SETMAX,32,0);
	//cHostPly.SendMessage(hs_Password,TXM_SETMAX,32,0);
	cHostPly.SendMessage(hs_MaxPlayers,TXM_SETMAX,6,0);

	// Use previous settings
	cHostPly.SendMessage( hs_MaxPlayers, TXS_SETTEXT, itoa(tLXOptions->tGameInfo.iMaxPlayers), 0);
	cHostPly.SendMessage( hs_Servername, TXS_SETTEXT, tLXOptions->sServerName, 0);
	cHostPly.SendMessage( hs_WelcomeMessage, TXS_SETTEXT, tLXOptions->sWelcomeMessage, 0);
	cHostPly.SendMessage( hs_Register,   CKM_SETCHECK, tLXOptions->bRegServer, 0);
	cHostPly.SendMessage( hs_AllowWantsJoin,   CKM_SETCHECK, tLXOptions->bAllowWantsJoinMsg, 0);
	cHostPly.SendMessage( hs_WantsJoinBanned,   CKM_SETCHECK, tLXOptions->bWantsJoinBanned, 0);
	cHostPly.SendMessage( hs_AllowRemoteBots,   CKM_SETCHECK, tLXOptions->bAllowRemoteBots, 0);
	cHostPly.SendMessage( hs_AllowNickChange,   CKM_SETCHECK, tLXOptions->tGameInfo.bAllowNickChange, 0);
	//cHostPly.SendMessage( hs_ServerSideHealth,  CKM_SETCHECK, tLXOptions->tGameInfo.bServerSideHealth, 0);
    //cHostPly.SendMessage( hs_Password,   TXS_SETTEXT, tLXOptions->tGameInfo.szPassword, 0 );

	// Add columns
	cHostPly.SendMessage( hs_PlayerList,   LVS_ADDCOLUMN, "Players",24);
	cHostPly.SendMessage( hs_PlayerList,   LVS_ADDCOLUMN, "",60);
	cHostPly.SendMessage( hs_Playing,      LVS_ADDCOLUMN, "Playing",24);
	cHostPly.SendMessage( hs_Playing,      LVS_ADDCOLUMN, "",60);

	// Add players to the list
	profile_t *p = GetProfiles();
	for(;p;p=p->tNext) {
		/*if(p->iType == PRF_COMPUTER)
			continue;*/

		cHostPly.SendMessage( hs_PlayerList, LVS_ADDITEM, "", p->iID);
		//cHostPly.SendMessage( hs_PlayerList, LVS_ADDSUBITEM, (DWORD)p->bmpWorm, LVS_IMAGE ); // TODO: 64bit unsafe (pointer cast)
		//cHostPly.SendMessage( hs_PlayerList, LVS_ADDSUBITEM, p->sName, LVS_TEXT);
		CListview * w = (CListview *) cHostPly.getWidget(hs_PlayerList);
		w->AddSubitem( LVS_IMAGE, "", p->cSkin.getPreview(), NULL );
		w->AddSubitem( LVS_TEXT, p->sName, (DynDrawIntf*)NULL, NULL );
	}

	iHumanPlayers = 0;

	return true;
}


///////////////////
// Shutdown the host menu
void Menu_Net_HostShutdown()
{
	switch(iHostType) {

		// Player selection
		case 0:
			Menu_Net_HostPlyShutdown();
			break;

		// Lobby
		case 1:
			Menu_Net_HostLobbyShutdown();
			break;
	}
}


///////////////////
// The net host menu frame
void Menu_Net_HostFrame(int mouse)
{
	switch(iHostType) {

		// Player selection
		case 0:
			Menu_Net_HostPlyFrame(mouse);
			break;

		// Lobby
		case 1:
			Menu_Net_HostLobbyFrame(mouse);
			break;
	}
}

// return value in bytes/second
static float estimatedMinNeededUploadSpeed(int wormCount, int hostWormCount) {
	if(wormCount <= 0) return 0.0f;
	if(hostWormCount >= wormCount) return 0.0f;
	
	// HINT: This calculation assumes that there is one worm per client. (Except for the local client.)
	
	return
		(wormCount - hostWormCount) * // amount of remote worms
		(wormCount - 1) * // for each such player, we have to inform about all worms except his own 
		500.0f; // just an estimation (bytes for worm updates of one single worm)
}

static int maxPossibleWormCountForNetwork(int hostWormCount) {
	int i = 0;
	while(i < MAX_PLAYERS) {
		if(estimatedMinNeededUploadSpeed(i + 1, hostWormCount) > GameServer::getMaxUploadBandwidth())
			return i;
		i++;
	}
	return i;
}
	

	
	
void Menu_Net_HostUpdateUploadSpeed(float speed)
{
	tLXOptions->iMaxUploadBandwidth = (int)speed;
	
	// Update the network speed accordingly
	if (tLXOptions->iMaxUploadBandwidth >= 7500)
		tLXOptions->iNetworkSpeed = NST_LAN;
	else if (tLXOptions->iMaxUploadBandwidth >= 2500)
		tLXOptions->iNetworkSpeed = NST_ISDN;
	else
		tLXOptions->iNetworkSpeed = NST_MODEM;
}
	
///////////////////
// Player selection frame
void Menu_Net_HostPlyFrame(int mouse)
{
	gui_event_t *ev = NULL;
	CListview	*lv, *lv2;
	profile_t	*ply;

	// Process & Draw the gui
	ev = cHostPly.Process();
	cHostPly.Draw( VideoPostProcessor::videoSurface() );

	if (Menu_Net_ProcessTabBarButtons(ev))
		return;
	
	// Speed test dialog
	if (bSpeedTestDialog)  {
		if (Menu_SpeedTest_Frame())  {
			Menu_Net_HostUpdateUploadSpeed(Menu_SpeedTest_GetSpeed() * 0.9f); // lower a bit to leave some rest free
			bSpeedTestDialog = false;
			Menu_SpeedTest_Shutdown();
		}
	}
	
	// If hosting for the first time or the network speed is low, ask for the upload speed test
	if (!bDedicated && tLX->currentTime - fHostMenuPlyStart >= 0.5f && !bTestedSpeed)  { // Show a half second after coming to the lobby
		if (tLXOptions->bFirstHosting)  {  
			bTestedSpeed = true;
			
			std::string message = "You are hosting for the first time. To reduce lag, OpenLieroX needs to know the speed of your connection.\n\nDo you want to perform a connection test now?";
			if (tLXOptions->iNetworkSpeed == NST_MODEM || tLXOptions->iMaxUploadBandwidth < 5000)
				message = "The network speed is set to a very low value. This can cause lag issues on your server. It is recommended to run a connection test.\n\nDo you want to perform a connection test now?";
			
			// TODO: Move that out here. Horrible how many hacks are needed to show a message box
			//DrawImage(tMenu->bmpBuffer.get(), VideoPostProcessor::videoSurface(), 0, 0);
			if (Menu_MessageBox("Perform a Connection Test", message, LMB_YESNO) == MBR_YES)  {
				Menu_SpeedTest_Initialize();
				bSpeedTestDialog = true;
			}
			//Menu_Net_HostLobbyDraw();  // Restore the buffer
			//Menu_HostShowMinimap();
			//Menu_HostDrawLobby(VideoPostProcessor::videoSurface());
		}
	}
	
	// Process any events
	if(ev) {

		switch(ev->iControlID) {

			// Back
			case hs_Back:
				if(ev->iEventMsg == BTN_CLICKED) {

					// Click!
					PlaySoundSample(sfxGeneral.smpClick);

					// Shutdown
					Menu_Net_HostShutdown();

					// Back to net menu
					Menu_MainInitialize();
				}
				break;


			// Player list
			case hs_PlayerList:
				if(ev->iEventMsg == LV_DOUBLECLK || ev->iEventMsg == LV_RIGHTCLK || ev->iEventMsg == LV_ENTER) {
					// Add the item to the players list
					lv = (CListview *)cHostPly.getWidget(hs_PlayerList);
					lv2 = (CListview *)cHostPly.getWidget(hs_Playing);
					int index = lv->getCurIndex();

					// Make sure there is 0-NumPlayers players in the list
					std::string num_players;
					cHostPly.SendMessage(hs_MaxPlayers, TXS_GETTEXT, &num_players, 0);
					if(lv2->getItemCount() < MIN((int)MAX_PLAYERS, atoi(num_players))) {
						lv->SaveScrollbarPos();

						// Get the profile
						ply = FindProfile(index);

						if(ply) {
							if (ply->iType == PRF_COMPUTER->toInt() || iHumanPlayers < 1)  {
								lv2->AddItem("",index,tLX->clListView);
								lv2->AddSubitem(LVS_IMAGE, "", ply->cSkin.getPreview(), NULL);
								lv2->AddSubitem(LVS_TEXT, ply->sName, (DynDrawIntf*)NULL, NULL);
								if (ply->iType == PRF_HUMAN->toInt())
									iHumanPlayers++;

								// Remove the item from the player list
								lv->RemoveItem(index);
							}
						}

						lv->RestoreScrollbarPos();

					}
				}
				break;


			// Playing list
			case hs_Playing:
				if(ev->iEventMsg == LV_DOUBLECLK || ev->iEventMsg == LV_RIGHTCLK || ev->iEventMsg == LV_ENTER) {
					// Add the item to the players list
					lv = (CListview *)cHostPly.getWidget(hs_Playing);
					lv2 = (CListview *)cHostPly.getWidget(hs_PlayerList);
					int index = lv->getCurIndex();

					lv->SaveScrollbarPos();

					// Remove the item from the list
					lv->RemoveItem(index);

					ply = FindProfile(index);

					if(ply) {
						lv2->AddItem("",index,tLX->clListView);
						lv2->AddSubitem(LVS_IMAGE, "", ply->cSkin.getPreview(), NULL);
						lv2->AddSubitem(LVS_TEXT, ply->sName, (DynDrawIntf*)NULL, NULL);
						if (ply->iType == PRF_HUMAN->toInt())
							iHumanPlayers--;
					}

					lv->RestoreScrollbarPos();
				}
				break;

			// Ok
			case hs_Ok:
				if(ev->iEventMsg == BTN_CLICKED) {

					lv = (CListview *)cHostPly.getWidget(hs_Playing);
					
					if (lv->getItemCount() == 0) {
						Menu_MessageBox("Too less players", "You have to select at least one player.");
						break;
					}
					
					if(lv->getItemCount() > MAX_PLAYERS) {
						Menu_MessageBox("Too much players",
										"You have selected " + itoa(lv->getItemCount()) + " players "
										"but only " + itoa(MAX_PLAYERS) + " players are possible.");
						break;
					}
					
					std::string buf;
					cHostPly.SendMessage( hs_MaxPlayers, TXS_GETTEXT, &buf, 0);
					tLXOptions->tGameInfo.iMaxPlayers = atoi(buf);
					// At least 2 players, and max MAX_PLAYERS
					tLXOptions->tGameInfo.iMaxPlayers = MAX(tLXOptions->tGameInfo.iMaxPlayers,2);
					tLXOptions->tGameInfo.iMaxPlayers = MIN(tLXOptions->tGameInfo.iMaxPlayers,MAX_PLAYERS);

					if(tLXOptions->bCheckBandwidthSanity || tLXOptions->iNetworkSpeed != NST_LAN) {
						float maxRate = GameServer::getMaxUploadBandwidth() / 1024.0f;
						float minRate = estimatedMinNeededUploadSpeed(tLXOptions->tGameInfo.iMaxPlayers, lv->getItemCount()) / 1024.0f;
						if(maxRate < minRate) {
							int maxPossibleWorms = maxPossibleWormCountForNetwork(lv->getItemCount());
							notes << "minEstimatedUploadRate=" << minRate << ", maxRate=" << maxRate << ", maxPossibleWorms=" << maxPossibleWorms << endl;
							std::string netSettingsText;
							switch(tLXOptions->iNetworkSpeed) {
								case NST_MODEM: netSettingsText = "Modem"; break;
								case NST_ISDN: netSettingsText = "ISDN"; break;
								case NST_LAN: netSettingsText = "DSL/LAN"; break;
								default: netSettingsText = "???"; break;
							}
							netSettingsText += ", max " + ftoa(maxRate) + " kB/sec upload"; 
							if(Menu_MessageBox("Check network settings",
											   "You allowed " + itoa(tLXOptions->tGameInfo.iMaxPlayers) + " players on your server. "
											   "A minimum upload rate of " + ftoa(minRate) + " kB/sec is needed for such an amount.\n"
											   "Your current network settings (" + netSettingsText + ") only allow up to " +
										   	   itoa(maxPossibleWorms) + " players.\n\n"
											   "Would you like to create a server for " + itoa(maxPossibleWorms) + " players?\n"
											   "(Otherwise please check your network settings in the option dialog.)",
											   LMB_YESNO) == MBR_YES) {
								notes << "setting maxplayers to " << maxPossibleWorms << " as user whishes" << endl;
								tLXOptions->tGameInfo.iMaxPlayers = maxPossibleWorms;
							}
							else break;
						}
					}
						
					tLX->iGameType = GME_HOST;

					cClient->Shutdown();
				    cClient->Clear();
					if(!cClient->Initialize()) {
						errors << "Client wouldn't initialize" << endl;
						break;
					}

					// Fill in the game structure
					lv_item_t* item;
					int count=0;

					int i=0;

					// Add the human players to the list
					for(item = lv->getItems(); item != NULL; item = item->tNext) {
						if(item->iIndex < 0)
							continue;

						profile_t *ply = FindProfile(item->iIndex);

						if(ply != NULL && ply->iType == PRF_HUMAN->toInt())  {
							// Max two humans
							// TODO: extend this
							if(i > 2)
								break;

							cClient->getLocalWormProfiles()[count++] = ply;
							i++;
						}

					}

					// Add the unhuman players to the list
					for(item = lv->getItems(); item != NULL; item = item->tNext) {
						if(item->iIndex < 0)
							continue;

						profile_t *ply = FindProfile(item->iIndex);

						if(ply != NULL && ply->iType != PRF_HUMAN->toInt())  {
							cClient->getLocalWormProfiles()[count++] = ply;
						}
					}
					cClient->setNumWorms(count);

					// Get the server name
					cHostPly.SendMessage( hs_Servername, TXS_GETTEXT, &tLXOptions->sServerName, 0);
					cHostPly.SendMessage( hs_WelcomeMessage, TXS_GETTEXT, &tLXOptions->sWelcomeMessage, 0);
					//cHostPly.SendMessage( hs_Password, TXS_GETTEXT, &tGameInfo.sPassword, 0);

					tLXOptions->bRegServer =  cHostPly.SendMessage( hs_Register, CKM_GETCHECK, (DWORD)0, 0) != 0;
					tLXOptions->bAllowWantsJoinMsg = cHostPly.SendMessage( hs_AllowWantsJoin, CKM_GETCHECK, (DWORD)0, 0) != 0;
					tLXOptions->bWantsJoinBanned = cHostPly.SendMessage( hs_WantsJoinBanned,   CKM_GETCHECK, (DWORD)0, 0) != 0;
					tLXOptions->bAllowRemoteBots = cHostPly.SendMessage( hs_AllowRemoteBots, CKM_GETCHECK, (DWORD)0, 0) != 0;
					tLXOptions->tGameInfo.bAllowNickChange = cHostPly.SendMessage( hs_AllowNickChange, CKM_GETCHECK, (DWORD)0, 0) != 0;
					tLXOptions->tGameInfo.bServerSideHealth = false; // HINT: disable ssh for normal (non-dedicated) servers, as it is very cheaty
					//tLXOptions->tGameInfo.bServerSideHealth = cHostPly.SendMessage( hs_ServerSideHealth, CKM_GETCHECK, (DWORD)0, 0) != 0;

					cHostPly.Shutdown();

					// Click
					PlaySoundSample(sfxGeneral.smpClick);

					iHumanPlayers = 0;

					// Start the lobby
					if (!Menu_Net_HostLobbyInitialize())  {
						errors << "could not start hosting lobby menu" << endl;
						Menu_Net_HostInitialize();
					}
				}
				break;
		}
	}


	// Draw the mouse
	DrawCursor(VideoPostProcessor::videoSurface());
}

//////////////
// Shutdown
void Menu_Net_HostPlyShutdown()
{
	cHostPly.Shutdown();
}




/*
==================================

		Hosting Lobby

==================================
*/


// Lobby gui
enum {
	hl_Back=0,
	hl_Start,
	hl_ChatText,
	hl_ChatList,
	hl_LevelList,
	hl_PlayerList,
	hl_Lives,
	hl_MaxKills,
	hl_ModName,
	hl_Gamemode,
	hl_GameSettings,
    hl_WeaponOptions,
    hl_PopupMenu,
	hl_PopupPlayerInfo,
	hl_Banned,
	hl_ServerSettings,
	hl_StartDedicated,
	hl_StartDedicatedSeconds,
	hl_StartDedicatedMinPlayers,
};

bool		bHostGameSettings = false;
bool		bHostWeaponRest = false;
bool		bBanList = false;
bool		bServerSettings = false;
CGuiLayout	cHostLobby;
int			iSpeaking = 0;
int         g_nLobbyWorm = -1;
bool		bHost_Update = false;
bool		bStartDedicated = false;
int			iStartDedicatedSeconds = 15;
int			iStartDedicatedMinPlayers = 4;
AbsTime		fStartDedicatedSecondsPassed = AbsTime();
int			iStartDedicatedServerSpamsSomeInfoTimeout = 15;
int			secondsAnnounced = -1;

static bool register_vars = CScriptableVars::RegisterVars("GameServer")
			( bStartDedicated, "bStartDedicated" )
			( iStartDedicatedSeconds, "iStartDedicatedSeconds" )
			( iStartDedicatedMinPlayers, "iStartDedicatedMinPlayers" )
//			( fStartDedicatedSecondsPassed, "fStartDedicatedSecondsPassed" )
			( iStartDedicatedServerSpamsSomeInfoTimeout, "iStartDedicatedServerSpamsSomeInfoTimeout" )
			;

///////////////////
// Initialize the hosting lobby
bool Menu_Net_HostLobbyInitialize()
{
	tLX->iGameType = GME_HOST;
	// TODO: please comment these vars
	iNetMode = net_host;
	iHostType = 1;
	bHostGameSettings = false;
    bHostWeaponRest = false;
    iSpeaking = -1;

	// Kinda sloppy, but else the background will look sloppy. (Map preview window & the others will be visible
	// If put below the client connect. Either this or move the draw.


	// Start the server
	if(!cServer->StartServer()) {
		// Crappy
		errors << "Server wouldn't start" << endl;
		cHostLobby.Shutdown();
		return false;
	}

	// Lets connect me to the server	
	cClient->Connect("127.0.0.1:" + itoa(cServer->getPort()));

	// Draw the lobby
	Menu_Net_HostLobbyDraw();


	// Clear the saved chat text
	tMenu->sSavedChatText = "";

    // Create the GUI
    Menu_Net_HostLobbyCreateGui();

	// Add the chat
	CBrowser *lv = (CBrowser *)cHostLobby.getWidget(hl_ChatList);
	if (lv)  {
		lv->InitializeChatBox();
		CChatBox *Chatbox = cClient->getChatbox();
		lines_iterator it = Chatbox->Begin();

		// Copy the chat text
		for (int id = 0; it != Chatbox->End(); it++, id++)  {
			lv->AddChatBoxLine(it->strLine, it->iColour, it->iTextType);
			id++;
		}
	}

	return true;
}


///////////////////
// Draw the lobby screen
void Menu_Net_HostLobbyDraw()
{
    // Create the buffer
	DrawImage(tMenu->bmpBuffer.get(),tMenu->bmpMainBack_common,0,0);
	if (tMenu->tFrontendInfo.bPageBoxes)
		Menu_DrawBox(tMenu->bmpBuffer.get(), 5,5, 635, 475);
	Menu_DrawBox(tMenu->bmpBuffer.get(), 460,29, 593, 130);
    DrawImageAdv(tMenu->bmpBuffer.get(), tMenu->bmpMainBack_common, 281,0, 281,0, 79,20);

    tLX->cFont.DrawCentre(tMenu->bmpBuffer.get(), 320, -1, tLX->clNormalLabel, "[  Lobby  ]");

	// Chat box
    DrawRectFill(tMenu->bmpBuffer.get(), 16, 270, 624, 417, tLX->clChatBoxBackground);

	Menu_RedrawMouse(true);
}


///////////////////
// Create the lobby gui
void Menu_Net_HostLobbyCreateGui()
{
    // Lobby gui layout
	cHostLobby.Shutdown();
	cHostLobby.Initialize();

	cHostLobby.Add( new CButton(BUT_LEAVE, tMenu->bmpButtons),hl_Back,	15,  450, 60,  15);
	cHostLobby.Add( new CButton(BUT_START, tMenu->bmpButtons),hl_Start,	560, 450, 60,  15);
	cHostLobby.Add( new CButton(BUT_BANNED, tMenu->bmpButtons),hl_Banned,	450, 450, 90,  15);
	cHostLobby.Add( new CButton(BUT_SERVERSETTINGS, tMenu->bmpButtons),hl_ServerSettings,	250, 450, 190, 15);
    cHostLobby.Add( new CBrowser(),                          hl_ChatList, 15,  268, 610, 150);

	cHostLobby.Add( new CButton(BUT_GAMESETTINGS, tMenu->bmpButtons), hl_GameSettings, 360, 210, 170,15);
    cHostLobby.Add( new CButton(BUT_WEAPONOPTIONS,tMenu->bmpButtons), hl_WeaponOptions,360, 235, 185,15);

	cHostLobby.Add( new CLabel("Mod",tLX->clNormalLabel),	    -1,         360, 180, 0,   0);
	cHostLobby.Add( new CCombobox(),				hl_ModName,    440, 179, 170, 17);
	cHostLobby.Add( new CLabel("Game type",tLX->clNormalLabel),	-1,         360, 158, 0,   0);
	cHostLobby.Add( new CCombobox(),				hl_Gamemode,   440, 157, 170, 17);
    cHostLobby.Add( new CLabel("Level",tLX->clNormalLabel),	    -1,         360, 136, 0,   0);
    cHostLobby.Add( new CCombobox(),				hl_LevelList,  440, 135, 170, 17);
	cHostLobby.Add( new CListview(),				hl_PlayerList, 15, 15, 325, 220);
	cHostLobby.Add( new CCheckbox(bStartDedicated),	hl_StartDedicated,			15,  244, 17, 17);
	cHostLobby.Add( new CLabel("Auto-start in",tLX->clNormalLabel),	-1,			40,  245, 0,   0);
	cHostLobby.Add( new CTextbox(),					hl_StartDedicatedSeconds,	122, 245, 25, tLX->cFont.GetHeight());
	cHostLobby.Add( new CLabel("seconds with min",tLX->clNormalLabel),	-1,		155, 245, 0,   0);
	cHostLobby.Add( new CTextbox(),					hl_StartDedicatedMinPlayers,263, 245, 25, tLX->cFont.GetHeight());
	cHostLobby.Add( new CLabel("players",tLX->clNormalLabel),	-1,				295, 245, 0,   0);

	// HINT: must be last, when player presses a key in lobby, this will be the first textbox found
	cHostLobby.Add( new CTextbox(),							  hl_ChatText, 15,  421, 610, tLX->cFont.GetHeight());

	cHostLobby.SendMessage(hl_StartDedicatedSeconds,TXM_SETMAX,8,0);
	cHostLobby.SendMessage(hl_StartDedicatedMinPlayers,TXM_SETMAX,8,0);
	CTextbox *t = (CTextbox *)cHostLobby.getWidget(hl_StartDedicatedSeconds);
	t->setText( itoa(iStartDedicatedSeconds) );
	t = (CTextbox *)cHostLobby.getWidget(hl_StartDedicatedMinPlayers);
	t->setText( itoa(iStartDedicatedMinPlayers) );

	//cHostLobby.SendMessage(hl_ChatList,		LVM_SETOLDSTYLE, 0, 0);

	// Fill in the game details
	for(Iterator<CGameMode* const&>::Ref i = GameModeIterator(); i->isValid(); i->next()) {
		cHostLobby.SendMessage(hl_Gamemode,    CBS_ADDITEM, i->get()->Name(), GetGameModeIndex(i->get()));
	}		
    cHostLobby.SendMessage(hl_Gamemode,    CBM_SETCURSEL, GetGameModeIndex(tLXOptions->tGameInfo.gameMode), 0);

	// Fill in the mod list
	CCombobox* cbMod = (CCombobox *)cHostLobby.getWidget(hl_ModName);
	Menu_Local_FillModList( cbMod );
	cbMod->setCurSIndexItem(tLXOptions->tGameInfo.sModDir);

	// Fill in the levels list
	Menu_FillLevelList( (CCombobox *)cHostLobby.getWidget(hl_LevelList), false);
	CCombobox* cbLevel = (CCombobox *) cHostLobby.getWidget(hl_LevelList);
	cbLevel->setCurSIndexItem(tLXOptions->tGameInfo.sMapFile);
	Menu_HostShowMinimap();

	CBrowser *lv = (CBrowser *)cHostLobby.getWidget(hl_ChatList);
	lv->InitializeChatBox();

	// Don't show chat box selection
	//lv->setShowSelect(false);

	cHostLobby.SendMessage(hl_LevelList, CBS_GETCURSINDEX, &tLXOptions->tGameInfo.sMapFile, 0);
	cHostLobby.SendMessage(hl_ModName,	 CBS_GETCURNAME, &tLXOptions->tGameInfo.sModName, 0);
	cHostLobby.SendMessage(hl_ModName,	 CBS_GETCURSINDEX, &tLXOptions->tGameInfo.sModDir, 0);

	CCombobox *gtype = (CCombobox *)cHostLobby.getWidget(hl_Gamemode);
	if (gtype)  {
		gtype->setCurItem( GetGameModeIndex(tLXOptions->tGameInfo.gameMode) );
	}

	// Setup the player list
	CListview *player_list = (CListview *)cHostLobby.getWidget(hl_PlayerList);
	if (player_list)  {
		player_list->setShowSelect(Menu_IsKeyboardNavigationUsed());
		player_list->setOldStyle(true);
		player_list->AddColumn("Players", gfxGUI.bmpCommandBtn.get()->w / 2 + 2, tLX->clHeading);  // Command button/Player label
		player_list->AddColumn("", tMenu->bmpLobbyReady.get()->w + 2);  // Lobby ready
		player_list->AddColumn("", 30);  // Skin
		player_list->AddColumn("", 200 - gfxGame.bmpTeamColours[0].get()->w); // Name
		player_list->AddColumn("", gfxGame.bmpTeamColours[0].get()->w + 10);  // Team
		player_list->AddColumn("", -1); // Ping
	}

	iSpeaking = 0; // The first player always speaks
	fStartDedicatedSecondsPassed = tLX->currentTime;	// Reset timers
	cHostLobby.FocusWidget(hl_ChatText);
}

//////////////////////
// Get the content of the chatbox
std::string Menu_Net_HostLobbyGetText()
{
	if (tMenu->bMenuRunning)  {
		std::string buf;
		cHostLobby.SendMessage(hl_ChatText, TXS_GETTEXT, &buf, 256);
		return buf;
	} else {
		return tMenu->sSavedChatText;
	}
}


void Menu_Net_HostLobbySetText(const std::string& str) {
	cHostLobby.SendMessage(hl_ChatText, TXS_SETTEXT, str, 0);
}



///////////////////
// Go straight to the lobby, without clearing the server & client
// TODO: describe the difference between Menu_Net_GotoHostLobby and Menu_Net_HostGotoLobby
void Menu_Net_HostGotoLobby()
{
	tLX->iGameType = GME_HOST;
	iNetMode = net_host;
	iHostType = 1;
	bHostGameSettings = false;
    bHostWeaponRest = false;
    iSpeaking = 0;

    // Draw the lobby
	if(!bDedicated)
		Menu_Net_HostLobbyDraw();

    if(!bDedicated) {
	    // Create the GUI
    	Menu_Net_HostLobbyCreateGui();

		// Add the chat to the chatbox
		CBrowser *lv = (CBrowser *)cHostLobby.getWidget(hl_ChatList);
		if (lv)  {
			lv->InitializeChatBox();
			CChatBox *Chatbox = cClient->getChatbox();
			lines_iterator it = Chatbox->At((int)Chatbox->getNumLines()-256); // If there's more than 256 messages, we start not from beginning but from end()-256
			//int id = (lv->getLastItem() && lv->getItems()) ? lv->getLastItem()->iIndex + 1 : 0;

			// Copy the chat text
			for (; it != Chatbox->End(); it++)  {
				if (it->iTextType == TXT_CHAT || it->iTextType == TXT_PRIVATE || it->iTextType == TXT_TEAMPM )  {  // Add only chat messages
					lv->AddChatBoxLine(it->strLine, it->iColour, it->iTextType);
					//id++;
				}
			}

			//lv->setShowSelect(false);
		}

		// Add the ingame chatter text to lobby chatter
		cHostLobby.SendMessage(hl_ChatText, TXS_SETTEXT, cClient->chatterText(), 0);
	}

	cServer->UpdateGameLobby();
	tMenu->sSavedChatText = "";
}

//////////////////////
// Change the mod displayed in the lobby
void Menu_Net_HostLobbySetMod(const std::string& moddir)
{
	CCombobox *cb = (CCombobox *)cHostLobby.getWidget(hl_ModName);
	if (!cb)
		return;

	cb->setCurSIndexItem(moddir);
}

/////////////////////
// Change level displayed in the lobby
void Menu_Net_HostLobbySetLevel(const std::string& filename)
{
	CCombobox *cb = (CCombobox *)cHostLobby.getWidget(hl_LevelList);
	if (!cb)
		return;

	cb->setCurSIndexItem(filename);
	Menu_HostShowMinimap();
}

///////////////////
// Host lobby frame
void Menu_Net_HostLobbyFrame(int mouse)
{
	gui_event_t *ev = NULL;

	// Process the server & client frames
	cServer->Frame();
	cClient->Frame();

    // Game settings
	if(bHostGameSettings) {
		if(Menu_GameSettings_Frame()) {
			Menu_Net_HostLobbyDraw();
			Menu_HostShowMinimap();

			bHostGameSettings = false;

			cServer->UpdateGameLobby();
			
			cServer->checkVersionCompatibilities(false);
		}
		return;
	}


    // Weapons Restrictions
    if(bHostWeaponRest) {
		if(Menu_WeaponsRestrictions_Frame()) {
			Menu_Net_HostLobbyDraw();
			Menu_HostShowMinimap();

			bHostWeaponRest = false;
		}
		return;
	}

    // Ban List
    if(bBanList) {
		if(Menu_BanList_Frame()) {
			Menu_Net_HostLobbyDraw();
			Menu_HostShowMinimap();

			bBanList = false;
		}
		return;
	}

    // Server Settings
    if(bServerSettings) {
		if(Menu_ServerSettings_Frame()) {
			Menu_Net_HostLobbyDraw();
			Menu_HostShowMinimap();

			bServerSettings = false;
		}
		return;
	}

	// Reload the level and mod list when the user switches back to game
	// Do not reaload when any of the dialogs is open
	if (bActivated)  {
		notes << "Switched back to OLX, re-filling the level and mod lists" << endl;

		// Get the mod name
		CCombobox* cbMod = (CCombobox *)cHostLobby.getWidget(hl_ModName);
		const cb_item_t *it = cbMod->getSelectedItem();
		if(it) 
			tLXOptions->tGameInfo.sModDir = it->sIndex;
		else
			errors << "No mod is selected, cannot save it to options" << endl;

		// Fill in the mod list
		Menu_Local_FillModList( cbMod );

		// Fill in the levels list
		CCombobox* cbLevel = (CCombobox *) cHostLobby.getWidget(hl_LevelList);
		it = cbLevel->getSelectedItem();
		if(it) 
			tLXOptions->tGameInfo.sMapFile = it->sIndex;
		else
			errors << "No level is selected, cannot save it to options" << endl;

		Menu_FillLevelList( (CCombobox *)cHostLobby.getWidget(hl_LevelList), false);
		cHostLobby.SendMessage(hl_LevelList, CBS_GETCURNAME, &tLXOptions->tGameInfo.sMapName, 0);

		Menu_HostShowMinimap();
	}


    // Add chat to the listbox
	CBrowser *lv = (CBrowser *)cHostLobby.getWidget(hl_ChatList);
	line_t l;

	while(cClient->getChatbox()->GetNewLine(l)) {

        //if(lv->getLastItem() != NULL)
        lv->AddChatBoxLine(l.strLine, l.iColour, l.iTextType);

        // If there are too many lines, remove the top one
        //if(lv->getItemCount() > 256) {
        //    if(lv->getItems())
        //        lv->RemoveItem(lv->getItems()->iIndex);
        //}
	}



    // Clear the chat box & player list & around combo boxes
    //Menu_redrawBufferRect(25, 315, 590, 100);
    Menu_redrawBufferRect(15, 20,  335, 225);
	Menu_redrawBufferRect(540, 150,  100, 200);

	// If any textbox is selected, forbid to show the console
	if (cHostLobby.getFocusedWidget() && !Con_IsVisible())  {
		tMenu->bForbidConsole = cHostLobby.getFocusedWidget()->getType() == wid_Textbox;
	}

    // Draw the host lobby details
	Menu_HostDrawLobby(VideoPostProcessor::videoSurface());

	// Process & Draw the gui
	if (!Con_IsVisible() && !bSpeedTestDialog && !CChatWidget::GlobalEnabled())  // Don't process when the console is opened or speed test is being performed
		ev = cHostLobby.Process();
	cHostLobby.Draw( VideoPostProcessor::videoSurface() );
	if(CChatWidget::GlobalEnabled())
	{
		CChatWidget::GlobalProcessAndDraw(VideoPostProcessor::videoSurface());
		return;
	}
	
	bool bStartPressed = false;

	// Process any events
	if(ev) {

		switch(ev->iControlID) {

			// Back
			case hl_Back:
				if(ev->iEventMsg == BTN_CLICKED) {
					// Click!
					PlaySoundSample(sfxGeneral.smpClick);

					// De-register the server
					Menu_Net_HostDeregister();

					// Shutdown
					Menu_Net_HostLobbyShutdown();

					// Back to main net menu
					Menu_NetInitialize();
				}
				break;

			// Chat box
			case hl_ChatList:
				if (ev->iEventMsg == BRW_KEY_NOT_PROCESSED)  {
					// Activate the textbox automatically
					cHostLobby.FocusWidget(hl_ChatText);
					tMenu->bForbidConsole = true;

					// Hack: add the just-pressed key to the textbox
					CTextbox *txt = (CTextbox *)cHostLobby.getWidget(hl_ChatText);
					if (txt && GetKeyboard()->queueLength > 0)  {
						KeyboardEvent kbev = GetKeyboard()->keyQueue[GetKeyboard()->queueLength - 1];
						txt->KeyDown(kbev.ch, kbev.sym, *GetCurrentModstate());
					}
				}
				break;


			// Chat textbox
			case hl_ChatText:
				if(ev->iEventMsg == TXT_ENTER && iSpeaking >= 0) {
					// Send the msg to the server

					// Get the text
					std::string text;
					cHostLobby.SendMessage(hl_ChatText, TXS_GETTEXT, &text, 0);

                    // Don't send empty messages
                    if(text.size() == 0)
                        break;

					// Clear the text box
					cHostLobby.SendMessage(hl_ChatText, TXS_SETTEXT, "", 0);

					// Send
					cClient->getNetEngine()->SendText(text, cClient->getWorm(0) ? cClient->getWorm(0)->getName() : "");
				}
				else if(ev->iEventMsg == TXT_TAB) {
					if(strSeemsLikeChatCommand(Menu_Net_HostLobbyGetText())) {
						cClient->getNetEngine()->SendChatCommandCompletionRequest(Menu_Net_HostLobbyGetText().substr(1));
						return;
					}
				}

				break;

			// Level change
			case hl_LevelList:
				if(ev->iEventMsg == CMB_CHANGED) {
					Menu_HostShowMinimap();

					cHostLobby.SendMessage(hl_LevelList, CBS_GETCURSINDEX, &tLXOptions->tGameInfo.sMapFile, 0);
					cServer->UpdateGameLobby();
				}
				break;

            // Mod change
            case hl_ModName:
                if(ev->iEventMsg == CMB_CHANGED) {
                    cHostLobby.SendMessage(hl_ModName, CBS_GETCURNAME, &tLXOptions->tGameInfo.sModName, 0);
                    cHostLobby.SendMessage(hl_ModName, CBS_GETCURSINDEX, &tLXOptions->tGameInfo.sModDir, 0);
					cServer->UpdateGameLobby();
                }
                break;

			// Game type change
			case hl_Gamemode:
				if(ev->iEventMsg == CMB_CHANGED) {
					tLXOptions->tGameInfo.gameMode = GameMode((GameModeIndex)cHostLobby.SendMessage(hl_Gamemode, CBM_GETCURINDEX, (DWORD)0, 0));
					bHost_Update = true;
					cServer->UpdateGameLobby();
				}
				break;

			// Lives change
			case hl_Lives:
				if(ev->iEventMsg == TXT_CHANGE) {
					std::string buf;
					cHostLobby.SendMessage(hl_Lives, TXS_GETTEXT, &buf, 0);
					TrimSpaces(buf);
					bool fail = false;
					if(buf != "")
						tLXOptions->tGameInfo.iLives = from_string<int>(buf, fail);
					if(buf == "" || fail)
						tLXOptions->tGameInfo.iLives = WRM_UNLIM;

					cServer->UpdateGameLobby();
				}
				break;


			// Max Kills
			case hl_MaxKills:
				if(ev->iEventMsg == TXT_CHANGE) {
					std::string buf;
					cHostLobby.SendMessage(hl_MaxKills, TXS_GETTEXT, &buf, 0);
					TrimSpaces(buf);
					bool fail = false;
					if(buf != "")
						tLXOptions->tGameInfo.iKillLimit = from_string<int>(buf, fail);
					if(buf == "" || fail)
						tLXOptions->tGameInfo.iKillLimit = -2;

					cServer->UpdateGameLobby();
				}
				break;

			// Game Settings
			case hl_GameSettings:
				if(ev->iEventMsg == BTN_CLICKED) {

					// Draw the lobby screen to the buffer
					cHostLobby.Draw(tMenu->bmpBuffer.get());
					Menu_HostDrawLobby(tMenu->bmpBuffer.get());

					Menu_GameSettings();
					bHostGameSettings = true;
				}
				break;

            // Weapon restrictions
            case hl_WeaponOptions:
                if(ev->iEventMsg == BTN_CLICKED) {

                    // Draw the lobby screen to the buffer
					cHostLobby.Draw(tMenu->bmpBuffer.get());
					Menu_HostDrawLobby(tMenu->bmpBuffer.get());

                    cb_item_t *it = (cb_item_t *)cHostLobby.SendMessage(hl_ModName,CBM_GETCURITEM,(DWORD)0,0); // TODO: 64bit unsafe (pointer cast)
                    if(it) {
		                bHostWeaponRest = true;
					    Menu_WeaponsRestrictions(it->sIndex);
                    }
                }
                break;

			// Ban list
			case hl_Banned:
				if(ev->iEventMsg == BTN_CLICKED)   {
                    // Draw the lobby screen to the buffer
					cHostLobby.Draw(tMenu->bmpBuffer.get());
					Menu_HostDrawLobby(tMenu->bmpBuffer.get());

	                bBanList = true;
				    Menu_BanList();
				}
				break;

			// Server settings
			case hl_ServerSettings:
				if(ev->iEventMsg == BTN_CLICKED)   {
                    // Draw the lobby screen to the buffer
					cHostLobby.Draw(tMenu->bmpBuffer.get());
					Menu_HostDrawLobby(tMenu->bmpBuffer.get());

	                bServerSettings = true;
				    Menu_ServerSettings();
				}
				break;


			// Start the game
			case hl_Start:
				if(ev->iEventMsg == BTN_CLICKED)
					bStartPressed = true;
				break;

			// Player list
			case hl_PlayerList:
				if (ev->iEventMsg == LV_WIDGETEVENT)  {
					ev = ((CListview *)ev->cWidget)->getWidgetEvent();

					// Click on the command button
					if (ev->cWidget->getType() == wid_Button && ev->iEventMsg == BTN_CLICKED)  {
						g_nLobbyWorm = ev->cWidget->getID();
						Menu_HostActionsPopupMenuInitialize(cHostLobby, hl_PopupMenu, hl_PopupPlayerInfo,  g_nLobbyWorm);
					// Click on the team mark
					} else if (ev->cWidget->getType() == wid_Image && ev->iEventMsg == IMG_CLICK)  {
						int id = ev->cWidget->getID();
						if (id >= MAX_WORMS || id < 0 || !cServer->getWorms()) // Safety
							break;

						// Set the team
						CWorm *w = cServer->getWorms() + id;
						w->setTeam((w->getTeam() + 1) % cServer->getGameMode()->GameTeams());

						cServer->SendWormLobbyUpdate();  // Update
						bHost_Update = true;
					}
				}
				break;

            // Popup menu
			case hl_PopupMenu:
				if (ev->iEventMsg >= MNU_USER) {
					Menu_HostActionsPopupMenuClick(cHostLobby, hl_PopupMenu, hl_PopupPlayerInfo, g_nLobbyWorm, ev->iEventMsg);
				} else if (ev->iEventMsg == MNU_LOSTFOCUS) {
					// If popup menu not focused, remove it
					cHostLobby.SendMessage(hl_PopupMenu, MNM_REDRAWBUFFER, (DWORD)0, 0);
					cHostLobby.removeWidget(hl_PopupMenu);
					cHostLobby.SendMessage(hl_PopupPlayerInfo, MNM_REDRAWBUFFER, (DWORD)0, 0);
					cHostLobby.removeWidget(hl_PopupPlayerInfo);
				}
				break;

			case hl_PopupPlayerInfo:
				if (ev->iEventMsg >= MNU_USER) {
					Menu_HostActionsPopupPlayerInfoClick(cHostLobby, hl_PopupMenu, hl_PopupPlayerInfo, g_nLobbyWorm, ev->iEventMsg);
				} 
				break;

			case hl_StartDedicated:
				if(ev->iEventMsg == CHK_CHANGED) {
						CCheckbox *c = (CCheckbox *)cHostLobby.getWidget(hl_StartDedicated);
						bStartDedicated = c->getValue() != 0;
						if( bStartDedicated )
							fStartDedicatedSecondsPassed = tLX->currentTime;
				}
                break;

			case hl_StartDedicatedSeconds:
				{
					CTextbox *t = (CTextbox *)cHostLobby.getWidget(hl_StartDedicatedSeconds);
					iStartDedicatedSeconds = atoi(t->getText());
				}
                break;

			case hl_StartDedicatedMinPlayers:
				{
					CTextbox *t = (CTextbox *)cHostLobby.getWidget(hl_StartDedicatedMinPlayers);
					iStartDedicatedMinPlayers = atoi(t->getText());
					iStartDedicatedMinPlayers = CLAMP( iStartDedicatedMinPlayers, 1, tLXOptions->tGameInfo.iMaxPlayers );
				}
                break;
		}
	}

	if (Menu_IsKeyboardNavigationUsed() && (!cHostLobby.getFocusedWidget() || cHostLobby.getFocusedWidget()->getID() != hl_PopupMenu)) {
		// If popup menu not focused, remove it
		cHostLobby.SendMessage(hl_PopupMenu, MNM_REDRAWBUFFER, (DWORD)0, 0);
		cHostLobby.removeWidget(hl_PopupMenu);
		cHostLobby.SendMessage(hl_PopupPlayerInfo, MNM_REDRAWBUFFER, (DWORD)0, 0);
		cHostLobby.removeWidget(hl_PopupPlayerInfo);
	}
	
	// Draw the mouse
	DrawCursor(VideoPostProcessor::videoSurface());

	int secondsTillGameStart = iStartDedicatedSeconds - Round( (tLX->currentTime - fStartDedicatedSecondsPassed).seconds() );
	if( bStartDedicated && cServer->getNumPlayers() < iStartDedicatedMinPlayers )
	{
		if( tLX->currentTime - fStartDedicatedSecondsPassed > (float)iStartDedicatedServerSpamsSomeInfoTimeout )
		{
			cClient->getNetEngine()->SendText( "Game will start when " +
					itoa(iStartDedicatedMinPlayers) + " players connect", "");
			fStartDedicatedSecondsPassed = tLX->currentTime;
			secondsAnnounced = -1;
		}
	}
	else if( bStartDedicated &&
			( ( secondsTillGameStart != secondsAnnounced ) ||
				secondsAnnounced == -1 ) )
	{
		if( secondsTillGameStart > 0 )
			cClient->getNetEngine()->SendText( "Game will start in " + itoa( secondsTillGameStart ) + " seconds", "" );
		secondsAnnounced = secondsTillGameStart;
	}

	if( bStartPressed ||
		( bStartDedicated && cServer->getNumPlayers() >= iStartDedicatedMinPlayers && secondsTillGameStart <= 0 ) )
	{
		// Local client will spectate if this is an auto-start (auto-select weapons)
		if (cClient->getWorm(0))  {
			cServer->getWorms()[cClient->getWorm(0)->getID()].setSpectating(!bStartPressed);
		}

		Menu_Net_HostStartGame();
	}
}

bool Menu_Net_HostStartGame()
{
	secondsAnnounced = -1;	
	// Save the chat text
	cHostLobby.SendMessage(hl_ChatText, TXS_GETTEXT, &tMenu->sSavedChatText, 256);

	// Get the mod
	cb_item_t *it = (cb_item_t *)cHostLobby.SendMessage(hl_ModName,CBM_GETCURITEM,(DWORD)0,0);
	if(it) {
		tLXOptions->tGameInfo.sModName = it->sName;
		tLXOptions->tGameInfo.sModDir = it->sIndex;
	} else {
		errors << "Could not get the selected mod" << endl;
		return false;
	}
	
	// Get the game type
	tLXOptions->tGameInfo.gameMode = GameMode((GameModeIndex)cHostLobby.SendMessage(hl_Gamemode, CBM_GETCURINDEX, (DWORD)0, 0));

	// Start the game
	std::string errMsg;
	if(!cServer->StartGame(&errMsg)) {	// Dedicated if no start button pressed
		errors << "Could not start the game for reason: " << errMsg << endl;
		cClient->getChatbox()->AddText("Error while starting game: " + errMsg, tLX->clNotice, TXT_IMPORTANT, tLX->currentTime);
		return false;
	}
	
	// Get the map name
	cHostLobby.SendMessage(hl_LevelList, CBS_GETCURSINDEX, &tLXOptions->tGameInfo.sMapFile, 0);
	cHostLobby.SendMessage(hl_LevelList, CBS_GETCURNAME, &tLXOptions->tGameInfo.sMapName, 0);
	// Save the current level in the options
	cHostLobby.SendMessage(hl_LevelList, CBS_GETCURSINDEX, &tLXOptions->tGameInfo.sMapFile, 0);
	cHostLobby.Shutdown();

	// Setup the client
	cClient->SetupViewports();

	// Leave the frontend
	*bGame = true;
	tMenu->bMenuRunning = false;
	tLX->iGameType = GME_HOST;

	return true;
}

////////////////////
// Shutdown
void Menu_Net_HostLobbyShutdown()
{
	// Shutdown all dialogs
	if (bHostGameSettings)
		Menu_GameSettingsShutdown();
	if (bHostWeaponRest)
		Menu_WeaponsRestrictionsShutdown();
	if (bBanList)
		Menu_BanListShutdown();
	if (bServerSettings)
		Menu_ServerSettingsShutdown();

	// Shutdown
	cHostLobby.Shutdown();

	// Tell any clients that we're leaving
	cServer->SendDisconnect();

	// Shutdown server & clients
	cClient->Shutdown();
	cServer->Shutdown();

	// Recover the host type
	iHostType = 0;
}


///////////////////
// Draw the host lobby screen
void Menu_HostDrawLobby(SDL_Surface * bmpDest)
{
	CListview *player_list = (CListview *)cHostLobby.getWidget(hl_PlayerList);
	if (!player_list) { // Weird, shouldn't happen
		errors << "Menu_HostDrawLobby: player_list not set" << endl;
		return;
	}

	if(cClient->getRemoteWorms() == NULL) {
		errors << "Menu_HostDrawLobby: remote worms are not initialised" << endl;
		return;
	}
	
	// Update the pings first
	CWorm *w = cClient->getRemoteWorms();
	for (int i=0; i < MAX_PLAYERS; i++, w++)  {
		CServerConnection *client = cServer->getClient(w->getID());
		if(!client) continue;
		if(client->isLocalClient()) continue; // don't show ping for host worm

		lv_subitem_t *subit = player_list->getSubItem(i, 5);
		if (subit)
			subit->sText = itoa(client->getPing());
	}


	if (!bHost_Update)  // If no further update is needed, do not do it
		return;

	player_list->SaveScrollbarPos();
	player_list->Clear();  // Clear any old info

	w = cServer->getWorms();
	CButton *cmd_button = NULL;
	CImage *team_img = NULL;

	for (int i=0; i < MAX_PLAYERS; i++, w++)  {
		if (!w->isUsed())  // Don't bother with unused worms
			continue;

		w->ChangeGraphics(cServer->getGameMode()->GeneralGameType());

		// Create and setup the command button
		cmd_button = new CButton(0, gfxGUI.bmpCommandBtn);
		if (!cmd_button)
			continue;
		cmd_button->setType(BUT_TWOSTATES);
		cmd_button->setRedrawMenu(false);
		cmd_button->Setup(w->getID(), 0, 0, gfxGUI.bmpCommandBtn.get()->w, gfxGUI.bmpCommandBtn.get()->h);

		// Add the item
		player_list->AddItem(w->getName(), i, tLX->clNormalLabel);
		player_list->AddSubitem(LVS_WIDGET, "", (DynDrawIntf*)NULL, cmd_button);  // Command button
		if (w->getLobbyReady())  // Ready control
			player_list->AddSubitem(LVS_IMAGE, "", tMenu->bmpLobbyReady, NULL);
		else
			player_list->AddSubitem(LVS_IMAGE, "", tMenu->bmpLobbyNotReady, NULL);
		player_list->AddSubitem(LVS_IMAGE, "", w->getPicimg(), NULL);  // Skin
		
		bool compatible = true;
		CServerConnection *client = cServer->getClient(w->getID());
		if(client && !cServer->checkVersionCompatibility(client, false, false)) compatible = false;
		player_list->AddSubitem(LVS_TEXT, "#"+itoa(w->getID())+" "+w->getName(), (DynDrawIntf*)NULL, NULL, VALIGN_MIDDLE,
								compatible ? tLX->clPink : tLX->clError);  // Name

		// Display the team mark if the game mode requires teams
		if(cServer->getGameMode()->GameTeams() > 1) {
			w->setTeam( CLAMP(w->getTeam(), 0, 4) );
			team_img = new CImage(gfxGame.bmpTeamColours[w->getTeam()]);
			if (!team_img)
				continue;
			team_img->setID(w->getID());
			team_img->setRedrawMenu(false);

			player_list->AddSubitem(LVS_WIDGET, "", (DynDrawIntf*)NULL, team_img); // Team

		} else {
			player_list->AddSubitem(LVS_TEXT, "", (DynDrawIntf*)NULL, NULL); // Ping has to be the fifth subitem
		}

		// Ping
		CServerConnection *cl = cServer->getClient(w->getID());
		int ping = 0;
		if (cl)	ping = cl->getPing();
		player_list->AddSubitem(LVS_TEXT, w->getID() != 0 ? itoa(ping) : "", (DynDrawIntf*)NULL, NULL); // Don't draw for host
	}

	player_list->RestoreScrollbarPos();  // Scroll back to where we were before

	// Updated :)
	bHost_Update = false;
}


///////////////////
// Show the minimap
void Menu_HostShowMinimap()
{
	CMap map;
	std::string buf;

	cHostLobby.SendMessage(hl_LevelList, CBS_GETCURSINDEX, &buf, 0);

	// Draw a background over the old minimap
	DrawImageAdv(tMenu->bmpBuffer.get(), tMenu->bmpMainBack_common, 463,32,463,32,128,96);

	// Load the map
	map.SetMinimapDimensions(128, 96);
	if(map.Load("levels/"+buf)) {

		// Draw the minimap
		if(map.GetMiniMap().get())
			DrawImage(tMenu->bmpBuffer.get(), map.GetMiniMap(), 463,32);
		else
			DrawRectFill(tMenu->bmpMiniMapBuffer.get(), 0, 0, tMenu->bmpMiniMapBuffer->w, tMenu->bmpMiniMapBuffer->h, Color());
	} else {
		errors << "Cannot load minimap for level " << buf << endl;
	}

	// Update the screen
	DrawImageAdv(VideoPostProcessor::videoSurface(), tMenu->bmpBuffer, 457,30,457,30,140,110);
}


///////////////////
// Deregister the server
void Menu_Net_HostDeregister()
{
	// If the server wasn't registered, just leave
	if( !tLXOptions->bRegServer )
		return;

	// Initialize the request
	if( !cServer->DeRegisterServer() )
		return;


	// Setup the background & show a messagebox
	int x = 160;
	int y = 170;
	int w = 320;
	int h = 140;
	int cx = x+w/2;
	int cy = y+h/2;


	DrawImage(tMenu->bmpBuffer.get(),tMenu->bmpMainBack_common,0,0);
	if (tMenu->tFrontendInfo.bPageBoxes)
		Menu_DrawBox(tMenu->bmpBuffer.get(), 15,60, 625, 465);
	Menu_DrawBox(tMenu->bmpBuffer.get(), x, y, x+w, y+h);
	DrawRectFill(tMenu->bmpBuffer.get(), x+2,y+2, x+w-1,y+h-1,tLX->clDialogBackground);

	tLX->cFont.DrawCentre(tMenu->bmpBuffer.get(), cx, cy, tLX->clNormalLabel, "De-Registering server...");

	Menu_RedrawMouse(true);

	AbsTime starttime = tLX->currentTime;
	SetGameCursor(CURSOR_ARROW);

	while(true) {
		Menu_RedrawMouse(true);
		ProcessEvents();
		tLX->currentTime = GetTime();

		// If we have gone over a 4 second limit, just leave
		if( tLX->currentTime - starttime > 4.0f ) {
			cServer->getHttp()->CancelProcessing();
			break;
		}

		if( cServer->ProcessDeRegister() ) {
			cServer->getHttp()->CancelProcessing();
			break;
		}

		DrawCursor(VideoPostProcessor::videoSurface());
		doVideoFrameInMainThread();
		CapFPS();
	}
}

/*
=======================

      Server settings

=======================
*/



CGuiLayout		cServerSettings;

// Server settings dialog
enum {
	ss_Ok,
	ss_Cancel,
	ss_AllowOnlyList,
	ss_WelcomeMessage,
	ss_ServerName,
	ss_AllowWantsJoin,
	ss_WantsJoinBanned,
	ss_AllowRemoteBots,
	ss_AllowNickChange,
	ss_MaxPlayers,
	ss_ServerSideHealth,
	ss_WeaponSelectionMaxTime
};



///////////////////
// Initialize the server settings window
void Menu_ServerSettings()
{
	// Setup the buffer
	//DrawImageAdv(tMenu->bmpBuffer, tMenu->bmpMainBack_common, 120,130,120,130, 400,200);
	DrawRectFillA(tMenu->bmpBuffer.get(), 120,130, 490,445, tLX->clDialogBackground, 200);
	Menu_DrawBox(tMenu->bmpBuffer.get(), 120,130, 490,445);

	Menu_RedrawMouse(true);

	cServerSettings.Initialize();
	cServerSettings.Add( new CLabel("Server Settings", tLX->clNormalLabel),		  -1,        275,140,  0, 0);
    cServerSettings.Add( new CButton(BUT_OK, tMenu->bmpButtons),	  ss_Ok,	 360,420,  60,15);
	cServerSettings.Add( new CButton(BUT_CANCEL, tMenu->bmpButtons),  ss_Cancel, 220,420,  70,15);
	cServerSettings.Add( new CLabel("Server Name:", tLX->clNormalLabel),		  -1,        130,165,  0, 0);
	cServerSettings.Add( new CLabel("Welcome Message:", tLX->clNormalLabel),	  -1,        130,193,  0, 0);
	cServerSettings.Add( new CLabel("Max. Players:", tLX->clNormalLabel),		  -1,        130,218,  0, 0);
	cServerSettings.Add( new CTextbox(),							  ss_ServerName, 265,165,  200, tLX->cFont.GetHeight());
	cServerSettings.Add( new CTextbox(),							  ss_WelcomeMessage,        265,190,  200, tLX->cFont.GetHeight());
	cServerSettings.Add( new CTextbox(),							  ss_MaxPlayers, 265,215,  50, tLX->cFont.GetHeight());
	cServerSettings.Add( new CLabel("Allow \"Wants to join\" Messages",	tLX->clNormalLabel),-1,	130, 245,0,  0);
	cServerSettings.Add( new CCheckbox(false),		                    ss_AllowWantsJoin,	360,245,17, 17);
	cServerSettings.Add( new CLabel("\"Wants to Join\" from Banned Clients",	tLX->clNormalLabel),-1,	130, 275,0,  0);
	cServerSettings.Add( new CCheckbox(false),		                    ss_WantsJoinBanned,	360,275,17, 17);
	cServerSettings.Add( new CLabel("Allow Bots in Server",				tLX->clNormalLabel),-1,	130, 305,0,  0);
	cServerSettings.Add( new CCheckbox(false),		                    ss_AllowRemoteBots,	360,305,17, 17);
	cServerSettings.Add( new CLabel("Allow Nick Change",				tLX->clNormalLabel),-1,	130, 335,0,  0);
	cServerSettings.Add( new CCheckbox(false),		                    ss_AllowNickChange,	360,335,17, 17);
	//cServerSettings.Add( new CLabel("Server-side Health",				tLX->clNormalLabel),-1,	130, 365,0,  0);
	//cServerSettings.Add( new CCheckbox(false),		                    ss_ServerSideHealth,	360,365,17, 17);
	cServerSettings.Add( new CLabel("Max weapon selection time:",	tLX->clNormalLabel),-1,	130, 365,0,  0);
	cServerSettings.Add( new CTextbox(),							ss_WeaponSelectionMaxTime, 360,365,  30, tLX->cFont.GetHeight());

	cServerSettings.SendMessage(ss_ServerName,TXM_SETMAX,32,0);
	cServerSettings.SendMessage(ss_WelcomeMessage,TXM_SETMAX,256,0);
	cServerSettings.SendMessage(ss_WeaponSelectionMaxTime,TXM_SETMAX,10,0);

	// Use the actual settings as default
	cServerSettings.SendMessage(ss_AllowWantsJoin, CKM_SETCHECK, tLXOptions->bAllowWantsJoinMsg, 0);
	cServerSettings.SendMessage(ss_WantsJoinBanned, CKM_SETCHECK, tLXOptions->bWantsJoinBanned, 0);
	cServerSettings.SendMessage(ss_AllowRemoteBots, CKM_SETCHECK, tLXOptions->bAllowRemoteBots, 0);
	cServerSettings.SendMessage(ss_AllowNickChange, CKM_SETCHECK, tLXOptions->tGameInfo.bAllowNickChange, 0);
	//cServerSettings.SendMessage(ss_ServerSideHealth, CKM_SETCHECK, tLXOptions->tGameInfo.bServerSideHealth, 0);
	cServerSettings.SendMessage(ss_ServerName,TXS_SETTEXT,tLXOptions->sServerName, 0);
	cServerSettings.SendMessage(ss_WelcomeMessage,TXS_SETTEXT,tLXOptions->sWelcomeMessage, 0);
	cServerSettings.SendMessage(ss_MaxPlayers, TXS_SETTEXT, itoa(tLXOptions->tGameInfo.iMaxPlayers), 0);
	cServerSettings.SendMessage(ss_WeaponSelectionMaxTime, TXS_SETTEXT, itoa(tLXOptions->tGameInfo.iWeaponSelectionMaxTime), 0);
}


///////////////////
// Server settings frame
// Returns whether or not we have finished setting up the server
bool Menu_ServerSettings_Frame()
{
	gui_event_t *ev = NULL;

	DrawImageAdv(VideoPostProcessor::videoSurface(), tMenu->bmpBuffer, 120,150, 120,150, 400,300);

    // Process events and draw gui
	ev = cServerSettings.Process();
	cServerSettings.Draw(VideoPostProcessor::videoSurface());

	if(ev) {

		switch(ev->iControlID) {

			// OK, done
			case ss_Ok:
				if(ev->iEventMsg == BTN_CLICKED) {

					// Save the info
					cServerSettings.SendMessage(ss_ServerName, TXS_GETTEXT, &tLXOptions->sServerName, 0);
					cServerSettings.SendMessage(ss_WelcomeMessage,TXS_GETTEXT, &tLXOptions->sWelcomeMessage, 0);

					std::string buf;
					cServerSettings.SendMessage(ss_MaxPlayers, TXS_GETTEXT, &buf, 0);
					tLXOptions->tGameInfo.iMaxPlayers = atoi(buf);
					// At least 2 players, and max MAX_PLAYERS
					tLXOptions->tGameInfo.iMaxPlayers = MAX(tLXOptions->tGameInfo.iMaxPlayers,2);
					tLXOptions->tGameInfo.iMaxPlayers = MIN(tLXOptions->tGameInfo.iMaxPlayers,MAX_PLAYERS);

					cServerSettings.SendMessage(ss_WeaponSelectionMaxTime, TXS_GETTEXT, &buf, 0);
					tLXOptions->tGameInfo.iWeaponSelectionMaxTime = MAX( 5, atoi(buf) );	// At least 5 seconds (hit Random - Done)

					// Set up the server
					if(cServer)  {
						cServer->setName(tLXOptions->sServerName);
					}

					tLXOptions->bAllowWantsJoinMsg = cServerSettings.SendMessage( ss_AllowWantsJoin, CKM_GETCHECK, (DWORD)0, 0) != 0;
					tLXOptions->bWantsJoinBanned = cServerSettings.SendMessage( ss_WantsJoinBanned, CKM_GETCHECK, (DWORD)0, 0) != 0;
					tLXOptions->bAllowRemoteBots = cServerSettings.SendMessage( ss_AllowRemoteBots, CKM_GETCHECK, (DWORD)0, 0) != 0;
					tLXOptions->tGameInfo.bAllowNickChange = cServerSettings.SendMessage( ss_AllowNickChange, CKM_GETCHECK, (DWORD)0, 0) != 0;
					//tLXOptions->tGameInfo.bServerSideHealth = cServerSettings.SendMessage( ss_ServerSideHealth, CKM_GETCHECK, (DWORD)0, 0) != 0;

					Menu_ServerSettingsShutdown();

					return true;
				}
				break;

			// Cancel, don't save changes
			case ss_Cancel:
				if(ev->iEventMsg == BTN_CLICKED) {
					Menu_ServerSettingsShutdown();

					return true;
				}
				break;
		}
	}

	// Draw the mouse
	DrawCursor(VideoPostProcessor::videoSurface());

	return false;
}

void Menu_ServerSettingsShutdown()
{
	cServerSettings.Shutdown();
}

/*
=======================

      Ban List

=======================
*/



CGuiLayout		cBanListGui;
CBanList        *cBanList = NULL;

// Ban List
enum {
	bl_Close=0,
	bl_Clear,
	bl_Unban,
	bl_ListBox
};



///////////////////
// Initialize the ban list window
void Menu_BanList()
{
	// Setup the buffer
	DrawImageAdv(tMenu->bmpBuffer.get(), tMenu->bmpMainBack_common, 120,130,120,130, 400,320);
	Menu_DrawBox(tMenu->bmpBuffer.get(), 120,130, 520,440);
	//DrawRectFillA(tMenu->bmpBuffer.get(), 125,155, 380,260, 0, 100);

    CListview *tListBox = new CListview();

	Menu_RedrawMouse(true);

	cBanListGui.Initialize();
	cBanListGui.Add( new CLabel("Ban List", tLX->clNormalLabel),     -1,        275,135,  0, 0);
    cBanListGui.Add( new CButton(BUT_OK, tMenu->bmpButtons),	  bl_Close,   400,420, 60,15);
    cBanListGui.Add( new CButton(BUT_CLEAR, tMenu->bmpButtons),	  bl_Clear,     180,420, 60,15);
	cBanListGui.Add( new CButton(BUT_UNBAN, tMenu->bmpButtons),	  bl_Unban,     260,420, 60,15);
	cBanListGui.Add( tListBox,									  bl_ListBox,125,155, 380,260);


	tListBox->AddColumn("IP Address",130);
	tListBox->AddColumn("Nick",160);

	if (!cServer->getBanList())
		return;

	// Load the list
	cBanList = cServer->getBanList();

	tListBox->Clear();
	banlist_t *item;
	for (int i=0;i<(*cBanList).getNumItems(); i++)  {
		item = (*cBanList).getItemById(i);
		if (!item)
			continue;
		tListBox->AddItem(item->szAddress,i,tLX->clListView);
		tListBox->AddSubitem(LVS_TEXT, item->szAddress, (DynDrawIntf*)NULL, NULL);
		tListBox->AddSubitem(LVS_TEXT, item->szNick, (DynDrawIntf*)NULL, NULL);
	}
}


///////////////////
// Ban List frame
// Returns whether or not we have finished with the ban list management
bool Menu_BanList_Frame()
{
	gui_event_t *ev = NULL;
	CListview *tListBox = (CListview *)cBanListGui.getWidget(bl_ListBox);


	DrawImageAdv(VideoPostProcessor::videoSurface(), tMenu->bmpBuffer, 120,150, 120,150, 400,300);

    // Process events and draw gui
	ev = cBanListGui.Process();
	cBanListGui.Draw(VideoPostProcessor::videoSurface());

	if(ev) {

		switch(ev->iControlID) {

			// Close
			case bl_Close:
				if(ev->iEventMsg == BTN_CLICKED) {

					Menu_BanListShutdown();

					return true;
				}
				break;
			// Unban
			case bl_Unban:
				if(ev->iEventMsg == BTN_CLICKED) {

					if (tListBox->getItemCount() > 0)  {

						(*cBanList).removeBanned(tListBox->getCurSub()->sText);

						tListBox->RemoveItem(tListBox->getCurIndex());
					}
				}
				break;
			// Clear
			case bl_Clear:
				if(ev->iEventMsg == BTN_CLICKED) {
					(*cBanList).Clear();

					tListBox->Clear();
				}
				break;

		}
	}

	// Draw the mouse
	DrawCursor(VideoPostProcessor::videoSurface());

	return false;
}

//////////////
// Shutdown
void Menu_BanListShutdown()
{
	if (cBanList)
		cBanList->saveList(cBanList->getPath());
	cBanListGui.Shutdown();
}

void Menu_HostActionsPopupMenuInitialize( CGuiLayout & layout, int id_PopupMenu, int id_PopupPlayerInfo, int wormid )
{
						// Remove old popup menu
						layout.removeWidget(id_PopupMenu);
						cHostLobby.removeWidget(id_PopupPlayerInfo);

						if (wormid < 0 || wormid >= MAX_WORMS)
							return;
						CWorm *w = &cServer->getWorms()[wormid];
						if (!w->isUsed())
							return;

						CServerConnection *remote_cl = cServer->getClient(wormid);
						mouse_t *Mouse = GetMouse();
						
						CMenu *mnu = NULL;

						if( !( cServer->getState() == SVS_PLAYING && wormid == 0 ) )	// Do not add the empty menu
						{
							mnu = new CMenu(Mouse->X, Mouse->Y);
							layout.Add(mnu, id_PopupMenu, 0, 0, 640, 480 );
							if (wormid > 0)  {  // These items make no sense for host
								if (tLXOptions->tGameInfo.gameMode->GameTeams() > 1) {
									mnu->addItem(5, "Change team");
								}
								mnu->addItem(0, "Kick player");
								mnu->addItem(1, "Ban player");
								if (remote_cl)  {
									if (remote_cl->getMuted())
										mnu->addItem(2, "Unmute player");
									else
										mnu->addItem(2, "Mute player");
								}
								mnu->addItem(3, "Authorise player");
							}
							if( cServer->getState() != SVS_PLAYING )
								mnu->addItem(4, "Spectator", true, w->isSpectating());

							layout.FocusWidget(id_PopupMenu);
						}

						CMenu * info = new CMenu( Mouse->X + (mnu ? mnu->getMenuWidth() : 0) + 10, Mouse->Y );
						if( cServer->getState() == SVS_PLAYING && wormid == 0 ) // Player info is the only popup menu
							layout.Add(info, id_PopupPlayerInfo, 0, 0, 640, 480 );
						else
							layout.Add(info, id_PopupPlayerInfo, info->getMenuX(), info->getMenuY(), 200, 200 );
						
						NetworkAddr addr = w->getClient()->getChannel()->getAddress();
						std::string addrStr;
						NetAddrToString(addr, addrStr);
						info->addItem(0, "IP: " + addrStr);
						
						if( tIpToCountryDB && tIpToCountryDB->Loaded() )
							info->addItem(1, "Country: " + tIpToCountryDB->GetInfoAboutIP(addrStr).countryName);

						info->addItem(2, "Version: " + w->getClient()->getClientVersion().asString() );
						
						// Update the menu clickable area - all items below are not clickable, they just for info
						if( !( cServer->getState() == SVS_PLAYING && wormid == 0 ) )
							info->Setup(id_PopupPlayerInfo, info->getMenuX(), info->getMenuY(), info->getMenuWidth(), info->getMenuHeight()-2);

						info->addItem(3, "Received: " + itoa(w->getClient()->getChannel()->getIncoming()/1024) + " Kb");
						info->addItem(4, "Sent: " + itoa(w->getClient()->getChannel()->getOutgoing()/1024) + " Kb");
						info->addItem(5, "Connected for " + 
							itoa( (int)((tLX->currentTime - w->getClient()->getConnectTime()).seconds() / 60.0f)) + " minutes");
						info->addItem(6, "Total " + 
							itoa( w->getTotalKills() ) + " kills / " +
							itoa( w->getTotalDeaths() ) + " deaths / " +
							itoa( w->getTotalSuicides() ) + " suicides / " +
							itoa( w->getTotalWins() + w->getTotalLosses() ) + " games / " +
							itoa( w->getTotalWins() ) + " wins" );
}

void Menu_HostActionsPopupMenuClick(CGuiLayout & layout, int id_PopupMenu, int id_PopupPlayerInfo, int wormid, int menuItem)
{
				CMenu *mnu = (CMenu *)layout.getWidget(id_PopupMenu);
                switch( menuItem ) {

                    // Kick the player
                    case MNU_USER+0:
						if( wormid > 0 )
                            cServer->kickWorm( wormid, "kicked via GUI menu", false );
                        break;

					// Ban the player
					case MNU_USER+1:
						if ( wormid > 0 )
							cServer->banWorm( wormid, "banned via GUI menu", false );
						break;

					// Mute/unmute
					case MNU_USER+2:
						if ( wormid > 0 )  {
							CServerConnection *remote_cl = cServer->getClient(wormid);
							if (remote_cl)  {
								if (remote_cl->getMuted())
									cServer->unmuteWorm(wormid);
								else
									cServer->muteWorm(wormid);
							}
						}
						break;

					// Authorize
					case MNU_USER+3:  {
							CServerConnection *remote_cl = cServer->getClient(wormid);
							if (remote_cl)
								remote_cl->getRights()->Everything();
						} break;

					// Spectate
					case MNU_USER+4:  {
							if( cServer->getState() == SVS_PLAYING )
								break;

							if (mnu->getItem(4) && g_nLobbyWorm >= 0 && wormid < MAX_WORMS)  {
								bool spec = mnu->getItem(4)->bChecked;
								CWorm *w = &cServer->getWorms()[wormid];
								w->setSpectating(spec);
								std::string buf;
								if (spec)  {
									if (networkTexts->sIsSpectating != "<none>")
										cServer->SendGlobalText(replacemax(networkTexts->sIsSpectating, "<player>", w->getName(), buf, 1), TXT_NETWORK);
								} else {
									if (networkTexts->sIsPlaying != "<none>")
										cServer->SendGlobalText(replacemax(networkTexts->sIsPlaying, "<player>", w->getName(), buf, 1), TXT_NETWORK);
								}
							}
						} break;

					// Team
					case MNU_USER+5:  {
							CWorm *w = &cServer->getWorms()[wormid];
							w->setTeam((w->getTeam() + 1) % cServer->getGameMode()->GameTeams());

							cServer->SendWormLobbyUpdate();  // Update
							bHost_Update = true;
						} break;
                }

                // Remove the menu widget
                layout.SendMessage( id_PopupMenu, MNM_REDRAWBUFFER, (DWORD)0, 0);
                layout.removeWidget(id_PopupMenu);
                layout.SendMessage( id_PopupPlayerInfo, MNM_REDRAWBUFFER, (DWORD)0, 0);
				layout.removeWidget(id_PopupPlayerInfo);
};

void Menu_HostActionsPopupPlayerInfoClick(CGuiLayout & layout, int id_PopupMenu, int id_PopupPlayerInfo, int wormid, int menuItem)
{
				if (wormid < 0 || wormid >= MAX_WORMS)
					return;
				CWorm *w = &cServer->getWorms()[wormid];
				if (!w->isUsed())
					return;

				NetworkAddr addr = w->getClient()->getChannel()->getSocket()->remoteAddress();
				std::string addrStr;
				NetAddrToString(addr, addrStr);

                switch( menuItem ) {

                    case MNU_USER+0:
						copy_to_clipboard(addrStr);
                        break;

                    case MNU_USER+1:
						if( tIpToCountryDB->Loaded() )
							copy_to_clipboard(tIpToCountryDB->GetInfoAboutIP(addrStr).countryName);
                        break;
						
                    case MNU_USER+2:
						copy_to_clipboard(w->getClient()->getClientVersion().asString());
						break;

				};

                // Remove the menu widget
                layout.SendMessage( id_PopupMenu, MNM_REDRAWBUFFER, (DWORD)0, 0);
                layout.removeWidget(id_PopupMenu);
                layout.SendMessage( id_PopupPlayerInfo, MNM_REDRAWBUFFER, (DWORD)0, 0);
				layout.removeWidget(id_PopupPlayerInfo);
};

}; // namespace DeprecatedGUI
