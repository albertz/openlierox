/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Net menu - Joining
// Created 25/8/02
// Jason Boettcher


#include <iostream>

#include "LieroX.h"
#include "Sounds.h"
#include "CClient.h"
#include "DeprecatedGUI/Graphics.h"
#include "DeprecatedGUI/Menu.h"
#include "GfxPrimitives.h"
#include "StringUtils.h"
#include "CWorm.h"
#include "Protocol.h"
#include "DeprecatedGUI/CButton.h"
#include "DeprecatedGUI/CTextbox.h"
#include "DeprecatedGUI/CImage.h"
#include "DeprecatedGUI/CCheckbox.h"
#include "DeprecatedGUI/CLabel.h"
#include "DeprecatedGUI/CTextButton.h"
#include "DeprecatedGUI/CProgressbar.h"
#include "DeprecatedGUI/CBrowser.h"
#include "AuxLib.h"
#include "CClientNetEngine.h"


using namespace std;


namespace DeprecatedGUI {

/*
   Joining is broken into 2 parts:
   1) Joining the server
   2) If we succeeded in join, the lobby
*/


// Joining menu's
enum {
	//join_players=0,
	join_connecting=0,
	join_lobby
};

int		iJoinMenu = join_connecting;
std::string	sJoinAddress;


///////////////////
// Join a server
bool Menu_Net_JoinInitialize(const std::string& sAddress)
{
	iNetMode = net_join;
	iJoinMenu = join_connecting;
	sJoinAddress = sAddress;

	if(!Menu_Net_JoinConnectionInitialize(sAddress)) {
		// Error
		return false;
	}


	return true;
}


///////////////////
// Main join frame
void Menu_Net_JoinFrame(int mouse)
{
	switch(iJoinMenu) {

		// Select players
/*		case join_players:
			Menu_Net_JoinPlayersFrame(mouse);
			break;*/

		// Connecting
		case join_connecting:
			Menu_Net_JoinConnectionFrame(mouse);
			break;

		// Lobby
		case join_lobby:
			Menu_Net_JoinLobbyFrame(mouse);
			break;
	}
}

void Menu_Net_JoinShutdown(void)
{
	Menu_Net_JoinConnectionShutdown();
	Menu_Net_JoinLobbyShutdown();
}






/*

   Connect to the server

*/


CGuiLayout cConnecting;
enum {
	cm_Cancel=0
};


///////////////////
// Initialize the connection menu
bool Menu_Net_JoinConnectionInitialize(const std::string& sAddress)
{
	iJoinMenu = join_connecting;
	tLX->iGameType = GME_JOIN;
	sJoinAddress = sAddress;
	cConnecting.Shutdown();
	cConnecting.Initialize();

	cConnecting.Add( new CButton(BUT_CANCEL, tMenu->bmpButtons),	cm_Cancel, 	25, 440, 75,15);

	cClient->Connect(sJoinAddress);

    Menu_redrawBufferRect(0, 0, 640, 480);

	return true;
}

void Menu_Net_JoinConnectionShutdown(void)
{
	cConnecting.Shutdown();
}


///////////////////
// Connection frame
void Menu_Net_JoinConnectionFrame(int mouse)
{
	gui_event_t *ev = NULL;

	Menu_redrawBufferRect(0,180,640,tLX->cFont.GetHeight());

	// Process the client frame
	tLX->cFont.DrawCentre(VideoPostProcessor::videoSurface(), 320, 180, tLX->clNormalLabel, "Connecting to " + cClient->getServerAddr_HumanReadable());
	cClient->Frame();


	// Connected??
	if(cClient->getStatus() == NET_CONNECTED) {

		// Leave this connection screen & go to the lobby
		Menu_Net_JoinConnectionShutdown();

		if (tLXOptions->bMouseAiming && !cClient->isHostAllowingMouse())
			// TODO: a msgbox for this is annoying. but perhaps we can add it in the chatbox?
			printf("HINT: This host doesn't allow mouse aiming. Using keyboard controls.\n");

		if(!Menu_Net_JoinLobbyInitialize()) {
			// Error
			Menu_Net_MainInitialize();
			return;
		}
	}

	// Check for a bad connection
	if(cClient->getBadConnection()) {
		cout << "Bad connection: " << cClient->getBadConnectionMsg() << endl;
		Menu_MessageBox("Connection Error", cClient->getBadConnectionMsg(), LMB_OK);

		cClient->Shutdown();

		// Shutdown
		Menu_Net_JoinConnectionShutdown();
		Menu_NetInitialize();
		return;
	}


	// Process & Draw the gui
	ev = cConnecting.Process();
	cConnecting.Draw( VideoPostProcessor::videoSurface() );


	// Process any events
	if(ev) {

		switch(ev->iControlID) {

			// Cancel
			case cm_Cancel:
				if(ev->iEventMsg == BTN_MOUSEUP) {

					// Click!
					PlaySoundSample(sfxGeneral.smpClick);

					// Shutdown
					Menu_Net_JoinConnectionShutdown();

					Menu_NetInitialize();
				}
				break;
		}
	}


	// Draw the mouse
	DrawCursor(VideoPostProcessor::videoSurface());
}







/*

   Join lobby

*/

CGuiLayout	cJoinLobby;
int			iJoinSpeaking = 0;
bool		bJoin_Update = true;
enum {
	jl_Back=0,
	jl_Ready,
	jl_ChatText,
	jl_ChatList,
	jl_Favourites,
	jl_PlayerList,
	jl_Details,
	jl_Spectate,
	jl_CancelDownload,
	jl_DownloadProgress
};



///////////////////
// Initialize the joining lobby
bool Menu_Net_JoinLobbyInitialize(void)
{
    Menu_Net_JoinDrawLobby();

    Menu_Net_JoinLobbyCreateGui();

	// Add the chat
	CBrowser *lv = (CBrowser *)cJoinLobby.getWidget(jl_ChatList);
	if (lv)  {
		lv->InitializeChatBox();
		CChatBox *Chatbox = cClient->getChatbox();
		lines_iterator it = Chatbox->Begin();

		// Copy the chat text
		for ( ; it != Chatbox->End(); it++ )  {
			lv->AddChatBoxLine(it->strLine, it->iColour, it->iTextType);
		}
	}

	iNetMode = net_join;
	iJoinMenu = join_lobby;

	cClient->getChatbox()->Clear();
    iJoinSpeaking = 0;  // The first player is always speaking
	tMenu->sSavedChatText = "";

	tLX->iGameType = GME_JOIN;

	return true;
}

////////////////////
// Shutdown the join lobby
void Menu_Net_JoinLobbyShutdown(void)
{
	if (cClient)
		cClient->Disconnect();

	cJoinLobby.Shutdown();

	if (cClient)
		cClient->Shutdown();

	tMenu->sSavedChatText = "";
}

	
///////////////////
// Draw the join lobby
void Menu_Net_JoinDrawLobby(void)
{
	// Create the buffer
	DrawImage(tMenu->bmpBuffer.get(),tMenu->bmpMainBack_common,0,0);
	if (tMenu->tFrontendInfo.bPageBoxes)
		Menu_DrawBox(tMenu->bmpBuffer.get(), 5,5, 635, 475);

    // Title
    DrawImageAdv(tMenu->bmpBuffer.get(), tMenu->bmpMainBack_common, 281,0, 281,0, 79,20);
    tLX->cFont.DrawCentre(tMenu->bmpBuffer.get(), 320, -1, tLX->clNormalLabel, "[  Lobby  ]");

	// Chat box
    DrawRectFill(tMenu->bmpBuffer.get(), 16, 270, 624, 417, tLX->clChatBoxBackground);	
	
	Menu_RedrawMouse(true);
}


static void initDetailsList(CListview* l) {
#define SUBS(title)	l->AddSubitem(LVS_TEXT, title, NULL, NULL); l->AddSubitem(LVS_TEXT, "", NULL, NULL);
	int index = 0;
	l->AddItem("servername", index++, tLX->clNormalLabel); SUBS("Server name:");
	l->AddItem("serverversion", index++, tLX->clNormalLabel); SUBS("Server version:");
	l->AddItem("level", index++, tLX->clNormalLabel); SUBS("Level:");
	l->AddItem("gamemode", index++, tLX->clNormalLabel); SUBS("Game Mode:");
	l->AddItem("mod", index++, tLX->clNormalLabel); SUBS("Mod:");
	l->AddItem("lives", index++, tLX->clNormalLabel); SUBS("Lives:");
	l->AddItem("maxkills", index++, tLX->clNormalLabel); SUBS("Max Kills:");
	l->AddItem("loadingtime", index++, tLX->clNormalLabel); SUBS("Loading time:");
	l->AddItem("bonuses", index++, tLX->clNormalLabel); SUBS("Bonuses:");
	foreach( Feature*, f, Array(featureArray,featureArrayLen()) ) {
		l->AddItem("feature:" + f->get()->name, index++, tLX->clNormalLabel); SUBS(f->get()->humanReadableName + ":");		
	}
#undef SUBS
}

static void updateDetailsList(CListview* l) {
#define SETI	{ i = l->getItem(index++); si = i->tSubitems->tNext; }
	int index = 0;
	lv_item_t* i;
	lv_subitem_t* si;
	SETI; si->sText = cClient->getServerName(); // servername
	SETI; si->sText = cClient->getServerVersion().asString(); // serverversion
	SETI;
	if(cClient->getHaveMap()) {
		si->sText = cClient->getGameLobby()->sMapName;
		si->iColour = tLX->clNormalLabel;
	} else {  // Don't have the map
		si->sText = cClient->getGameLobby()->sMapFile;
		si->iColour = tLX->clError;
		if (!cClient->getDownloadingMap())  {  // not downloading the map
/*			
			if (tMenu->bmpDownload.get())
				DrawImage(VideoPostProcessor::videoSurface(), tMenu->bmpDownload, x2 + f->GetWidth(cClient->getGameLobby()->sMapFile) + 5, y + (f->GetHeight() - tMenu->bmpDownload->h)/2);
			
			if (MouseInRect(x2, y, 640-x2, tLX->cFont.GetHeight()))  {
				SetGameCursor(CURSOR_HAND);
				if (GetMouse()->Up)
					cClient->DownloadMap(cClient->getGameLobby()->sMapFile); // Download the map
			} else {
				SetGameCursor(CURSOR_ARROW);
			}
*/ 
		}
	}

	const std::string gamemodes[] = {"Deathmatch","Team Deathmatch", "Tag", "Demolitions"};
	SETI; si->sText = gamemodes[cClient->getGameLobby()->iGameMode];
	SETI;
	if(cClient->getHaveMod()) {
		si->sText = cClient->getGameLobby()->sModName;
		si->iColour = tLX->clNormalLabel;
	} else {
		si->sText = cClient->getGameLobby()->sModName;
		si->iColour = tLX->clError;
		if (!cClient->getDownloadingMod()) {
/*
			if (tMenu->bmpDownload.get())
				DrawImage(VideoPostProcessor::videoSurface(), tMenu->bmpDownload, x2 + f->GetWidth(cClient->getGameLobby()->sModName) + 5, y + (f->GetHeight() - tMenu->bmpDownload->h)/2);
			
			if (MouseInRect(x2, y, 640-x2, tLX->cFont.GetHeight()))  {
				SetGameCursor(CURSOR_HAND);
				if (GetMouse()->Up)  {
					cClient->DownloadMod(cClient->getGameLobby()->sModDir);
				}
			} else {
				SetGameCursor(CURSOR_ARROW);
			}
*/
		}
	}
	
	SETI;
	if(cClient->getGameLobby()->iLives >= 0) {
		si->sText = itoa(cClient->getGameLobby()->iLives);
		si->iColour = tLX->clNormalLabel;
	} else {
		si->sText = "infinity";
		si->iColour = tLX->clDisabled;
	}
	
	SETI;
	if(cClient->getGameLobby()->iKillLimit >= 0) {
		si->sText = itoa(cClient->getGameLobby()->iKillLimit);
		si->iColour = tLX->clNormalLabel;
	} else {
		si->sText = "infinity";
		si->iColour = tLX->clDisabled;
	}

	SETI; si->sText = itoa(cClient->getGameLobby()->iLoadingTime) + "%";
	SETI; si->sText = cClient->getGameLobby()->bBonusesOn ? "On" : "Off";

	foreach( Feature*, f, Array(featureArray,featureArrayLen()) ) {
		i = l->getItem("feature:" + f->get()->name);

	
	}

	foreach( FeatureCompatibleSettingList::Feature&, f, cClient->getUnknownFeatures().list ) {
		i = l->getItem("feature:" + f->get().name);
		if(!i) {
			i = l->AddItem("feature:" + f->get().name, 0, tLX->clNormalLabel);
			l->AddSubitem(LVS_TEXT, f->get().humanName + ":", NULL, NULL);
			l->AddSubitem(LVS_TEXT, "", NULL, NULL);
		}
		si = i->tSubitems->tNext;

		Uint32 col;
		switch(f->get().type) {
			case FeatureCompatibleSettingList::Feature::FCSL_JUSTUNKNOWN: col = tLX->clDisabled; break;
			case FeatureCompatibleSettingList::Feature::FCSL_INCOMPATIBLE: col = tLX->clError; break;
			default: col = tLX->clNormalLabel;
		}
		si->iColour = col;
		si->sText = f->get().var.toString();
	}
	
#undef SETI
}
	
///////////////////
// Create the lobby gui stuff
void Menu_Net_JoinLobbyCreateGui(void)
{
    cJoinLobby.Shutdown();
	cJoinLobby.Initialize();

	cJoinLobby.Add( new CButton(BUT_LEAVE, tMenu->bmpButtons),jl_Back,	15,  450, 60,  15);
    cJoinLobby.Add( new CButton(BUT_READY, tMenu->bmpButtons),jl_Ready,	560, 450, 65,  15);
	cJoinLobby.Add( new CButton(BUT_ADDTOFAVOURITES, tMenu->bmpButtons), jl_Favourites, 230,450,150,15);
	cJoinLobby.Add( new CTextbox(),							  jl_ChatText, 15,  421, 610, tLX->cFont.GetHeight());
    cJoinLobby.Add( new CBrowser(),                           jl_ChatList, 15,  268, 610, 150);
	cJoinLobby.Add( new CListview(),						  jl_PlayerList, 15, 15, 325, 220);
	cJoinLobby.Add( new CListview(),						  jl_Details, 350, 15, 290, 250);	
	cJoinLobby.Add( new CCheckbox(cClient->getSpectate()),	  jl_Spectate, 15, 244, 17, 17 );
	cJoinLobby.Add( new CLabel( "Spectate only", tLX->clNormalLabel ), -1, 40, 245, 0, 0 );

	// Downloading stuff
	CProgressBar *dl = new CProgressBar(LoadGameImage("data/frontend/downloadbar_lobby.png", true), 0, 0, false, 1);
	CButton *cancel = new CButton(BUT_CANCEL, tMenu->bmpButtons);
	cJoinLobby.Add( dl, jl_DownloadProgress, 360, 245, 0, 0);
	cJoinLobby.Add( cancel, jl_CancelDownload, 360 + dl->getWidth() + 5, 245 + (dl->getHeight() - 20)/2, 0, 0);
	dl->setEnabled(false);
	cancel->setEnabled(false);

	// Setup the player list
	CListview *player_list = (CListview *)cJoinLobby.getWidget(jl_PlayerList);
	if (player_list)  {
		player_list->setShowSelect(false);
		player_list->setOldStyle(true);
		player_list->AddColumn("Players", tMenu->bmpLobbyReady.get()->w + 2, tLX->clHeading);  // Lobby ready/Players label
		player_list->AddColumn("", 30);  // Skin
		player_list->AddColumn("", 220); // Name
		player_list->AddColumn("", -1); // Team
	}


	CListview* details = (CListview *)cJoinLobby.getWidget(jl_Details);	
	//details->Setup(0, x + 15, y+5, w - 30, h - 25); // is this done already in .Add(...) above?
	details->setDrawBorder(false);
	details->setRedrawMenu(false);
	details->setShowSelect(false);
	details->setOldStyle(true);

	// like in Menu_SvrList_DrawInfo
	int first_column_width = tLX->cFont.GetWidth("Loading Times:") + 30; // Width of the widest item in this column + some space
	int last_column_width = tLX->cFont.GetWidth("999"); // Kills width
	details->AddColumn("", first_column_width);
	details->AddColumn("", details->getWidth() - first_column_width - (last_column_width*2) - gfxGUI.bmpScrollbar.get()->w); // The rest
	details->AddColumn("", last_column_width);
	details->AddColumn("", last_column_width);
	
	initDetailsList(details);
	
	iJoinSpeaking = 0; // The first client is always speaking
}


///////////////////
// Go straight back to the join lobby
// TODO: please comment the difference between Menu_Net_JoinGotoLobby and Menu_Net_GotoJoinLobby
void Menu_Net_JoinGotoLobby(void)
{
    //Menu_redrawBufferRect(0, 0, 640, 480);

    Menu_Net_JoinDrawLobby();

    Menu_Net_JoinLobbyCreateGui();

	iNetMode = net_join;
	iJoinMenu = join_lobby;

	// Add the chat
	CBrowser *lv = (CBrowser *)cJoinLobby.getWidget(jl_ChatList);
	if (lv)  {
		lv->InitializeChatBox();
		CChatBox *Chatbox = cClient->getChatbox();
		lines_iterator it = Chatbox->At((int)Chatbox->getNumLines()-256); // If there's more than 256 messages, we start not from beginning but from end()-256
		//int id = (lv->getLastItem() && lv->getItems()) ? lv->getLastItem()->iIndex + 1 : 0;

		// Copy the chat text
		for (; it != Chatbox->End(); it++)  {
			if (it->iTextType == TXT_CHAT || it->iTextType == TXT_PRIVATE || it->iTextType == TXT_TEAMPM)  {  // Add only chat messages
				lv->AddChatBoxLine(it->strLine, it->iColour, it->iTextType);
			}
		}
	}

	// Add the ingame chatter text to lobby chatter
	Menu_Net_JoinLobbySetText(cClient->chatterText());

    iJoinSpeaking = 0; // The first player is always speaking
	tMenu->sSavedChatText = "";
}

//////////////////////
// Get the content of the chatbox
std::string Menu_Net_JoinLobbyGetText()
{
	if (tMenu->bMenuRunning)  {
		std::string buf;
		cJoinLobby.SendMessage(jl_ChatText, TXS_GETTEXT, &buf, 256);
		return buf;
	} else {
		return tMenu->sSavedChatText;
	}
}

void Menu_Net_JoinLobbySetText(const std::string& str) {
	cJoinLobby.SendMessage(jl_ChatText, TXS_SETTEXT, str, 0);
}


///////////////////
// Join lobby frame
void Menu_Net_JoinLobbyFrame(int mouse)
{
	gui_event_t *ev = NULL;
	int			i,y;

	// Process the client
	cClient->Frame();

    // If there is a client error, leave
    if(cClient->getClientError()) {

		Menu_Net_JoinShutdown();
        Menu_NetInitialize();
		bJoin_Update = true;
		return;
    }


	// If we have started, leave the frontend
	if(cClient->getGameReady()) {

		cJoinLobby.Shutdown();

        // Setup the client
        cClient->SetupViewports();

		// Leave the frontend
		*bGame = true;
		tMenu->bMenuRunning = false;
		tLX->iGameType = GME_JOIN;

		// Save the chat text
		cJoinLobby.SendMessage(jl_ChatText, TXS_GETTEXT, &tMenu->sSavedChatText, 256);

		bJoin_Update = true;

		return;
	}


	// Check if the communication link between us & server is still ok
	if(cClient->getServerError()) {
		// Create the buffer
		DrawImage(tMenu->bmpBuffer.get(),tMenu->bmpMainBack_common,0,0);
		Menu_DrawSubTitle(tMenu->bmpBuffer.get(),SUB_LOBBY);

		Menu_MessageBox("Communication", cClient->getServerErrorMsg(), LMB_OK);

        cJoinLobby.Shutdown();

		Menu_NetInitialize();

		bJoin_Update = true;
		return;
	}


    // Clear the player list and game settings
    Menu_redrawBufferRect(15, 15,  400, 230);
    Menu_redrawBufferRect(360,15,  280, 200);


	// Add chat to the listbox
	CBrowser *lv = (CBrowser *)cJoinLobby.getWidget(jl_ChatList);
    line_t *ln = NULL;
	while((ln = cClient->getChatbox()->GetNewLine()) != NULL) {
		lv->AddChatBoxLine(ln->strLine, ln->iColour, ln->iTextType);
	}


	// Draw the connected players
	if (bJoin_Update)  {
		CListview *player_list = (CListview *)cJoinLobby.getWidget(jl_PlayerList);
		if (!player_list) { // Weird, shouldn't happen
			printf("WARNING: Menu_Net_JoinLobbyFrame: player_list unset\n");
			return;
		}
		player_list->SaveScrollbarPos();
		player_list->Clear();  // Clear first

		CWorm *w = cClient->getRemoteWorms();
		lobbyworm_t *lobby_worm = NULL;
		CImage *team_img = NULL;

		for (i=0; i < MAX_PLAYERS; i++, w++)  {
			if (!w->isUsed())  // Don't bother with unused worms
				continue;

			lobby_worm = w->getLobby();

			// Reload the worm graphics
			w->setTeam(lobby_worm->iTeam);
			w->ChangeGraphics(cClient->getGameLobby()->iGameMode);

			// Add the item
			player_list->AddItem(w->getName(), i, tLX->clNormalLabel);
			if (lobby_worm->bReady)  // Ready control
				player_list->AddSubitem(LVS_IMAGE, "", tMenu->bmpLobbyReady, NULL);
			else
				player_list->AddSubitem(LVS_IMAGE, "", tMenu->bmpLobbyNotReady, NULL);
			player_list->AddSubitem(LVS_IMAGE, "", w->getPicimg(), NULL);  // Skin
			player_list->AddSubitem(LVS_TEXT, "#"+itoa(w->getID())+" "+w->getName(), NULL, NULL);  // Name

			// Display the team mark if TDM
			if (cClient->getGameLobby()->iGameMode == GMT_TEAMDEATH)  {
				team_img = new CImage(gfxGame.bmpTeamColours[lobby_worm->iTeam]);
				if (!team_img)
					continue;
				team_img->setID(w->getID());
				team_img->setRedrawMenu(false);

				player_list->AddSubitem(LVS_WIDGET, "", NULL, team_img); // Team
			}
		}

		player_list->RestoreScrollbarPos();  // Scroll back to where we were before the update

		bJoin_Update = false;
	}

	// Draw the game info
	if(true) {	//if(GameLobby->bSet) {
		CFont *f = &tLX->cFont;
        int x = 360;
        y = 15;

		f->Draw(VideoPostProcessor::videoSurface(), x, y,  tLX->clHeading, "Game Details");
		CListview* details = (CListview *)cJoinLobby.getWidget(jl_Details);
		
		updateDetailsList(details);
	}

	{
		/*if( cClient->getDownloadingMap() )
			tLX->cFont.Draw(VideoPostProcessor::videoSurface(),     410, 195, tLX->clNormalLabel,
			itoa( cClient->getMapDlProgress() ) + "%: " + gl->szMapFile );
		else if( cClient->getUdpFileDownloader()->getFileDownloading() != "" )
			tLX->cFont.Draw(VideoPostProcessor::videoSurface(),     410, 195, tLX->clNormalLabel,
			itoa( int(cClient->getUdpFileDownloader()->getFileDownloadingProgress()*100.0) ) + "%: " +
			cClient->getUdpFileDownloader()->getFileDownloading() );
		else if( cClient->getUdpFileDownloader()->getFilesPendingAmount() > 0 )
			tLX->cFont.Draw(VideoPostProcessor::videoSurface(),     410, 195, tLX->clNormalLabel,
			to_string<size_t>( cClient->getUdpFileDownloader()->getFilesPendingAmount() ) + " files left" );

		CTextButton * dlButton = (CTextButton *)cJoinLobby.getWidget(jl_StartStopUdpFileDownload);
		if( dlButton )
		{
			if( ! cClient->getGameLobby()->bHaveMap ||
				( ! cClient->getGameLobby()->bHaveMod && cClient->getServerVersion() >= OLXBetaVersion(4) ) )
			{
				if( cClient->getDownloadingMap() || cClient->getUdpFileDownloader()->getFilesPendingAmount() > 0 )
					dlButton->SendMessage( LBS_SETTEXT, "Abort", 0 );
				else
					dlButton->SendMessage( LBS_SETTEXT, "Download files", 0 );
			}
			else
				dlButton->SendMessage( LBS_SETTEXT, "", 0 );
		};

		if( cClient->getDownloadingMapError() )
		{
			// Put notice about downloaded file in chatbox
			cClient->clearDownloadingMapError();
			CBytestream bs;
			bs.writeByte(TXT_NETWORK);
			bs.writeString( cClient->getDownloadingMapErrorMessage() );
			cClient->ParseText( &bs );
		};*/

		CButton *cancel = (CButton *)cJoinLobby.getWidget(jl_CancelDownload);
		CProgressBar *progress = (CProgressBar *)cJoinLobby.getWidget(jl_DownloadProgress);
		if (cClient->getDownloadingMap() || cClient->getDownloadingMod())  {

			progress->setEnabled(true);
			cancel->setEnabled(true);
			progress->SetPosition(cClient->getDlProgress());
		} else {
			progress->setEnabled(false);
			cancel->setEnabled(false);
		}
	}

	// Process & Draw the gui
	ev = cJoinLobby.Process();

	cJoinLobby.Draw( VideoPostProcessor::videoSurface() );

	// Process any events
	if(ev) {
		switch(ev->iControlID) {

			// Back
			case jl_Back:
				if(ev->iEventMsg == BTN_MOUSEUP) {
					// Click
					PlaySoundSample(sfxGeneral.smpClick);

					Menu_Net_JoinLobbyShutdown();

					// Back to net menu
					Menu_NetInitialize();
				}
				break;

			// Ready
			case jl_Ready:
				if(ev->iEventMsg == BTN_MOUSEUP) {
					// Let the server know that my worms are now ready
					cClient->getNetEngine()->SendUpdateLobby();

					// Hide the ready button
					CButton *btn = (CButton *)cJoinLobby.getWidget(jl_Ready);
					btn->setEnabled(false);
                    btn->redrawBuffer();
				}
				break;

			// Add to favourites
			case jl_Favourites:
				if (ev->iEventMsg == BTN_MOUSEUP) {
					Menu_SvrList_AddFavourite(cClient->getServerName(),cClient->getServerAddress());
				}
				break;

			// Cancel file transfers
			case jl_CancelDownload:
				if (ev->iEventMsg == BTN_MOUSEUP)  {
					cClient->AbortDownloads();
				}
				break;

			// Chat box
			case jl_ChatList:
				if (ev->iEventMsg == BRW_KEY_NOT_PROCESSED)  {
					// Activate the textbox automatically
					cJoinLobby.FocusWidget(jl_ChatText);

					// Hack: add the just-pressed key to the textbox
					CTextbox *txt = (CTextbox *)cJoinLobby.getWidget(jl_ChatText);
					if (txt && GetKeyboard()->queueLength > 0)  {
						KeyboardEvent kbev = GetKeyboard()->keyQueue[GetKeyboard()->queueLength - 1];
						txt->KeyDown(kbev.ch, kbev.sym, *GetCurrentModstate());
					}
				}
				break;


			// Chat textbox
			case jl_ChatText:
				if(ev->iEventMsg == TXT_ENTER && iJoinSpeaking >= 0) {
					// Send the msg to the server

					// Get the text
					std::string text;
					cJoinLobby.SendMessage(jl_ChatText, TXS_GETTEXT, &text, 0);

                    // Don't send empty messages
                    if(text.size() == 0)
                        break;

					// Clear the text box
					cJoinLobby.SendMessage(jl_ChatText, TXS_SETTEXT, "",0);

					// Send
					cClient->getNetEngine()->SendText(text, cClient->getWorm(0)->getName());
				} else
				if(ev->iEventMsg == TXT_TAB) {
					if(strSeemsLikeChatCommand(Menu_Net_JoinLobbyGetText())) {
						cClient->getNetEngine()->SendChatCommandCompletionRequest(Menu_Net_JoinLobbyGetText().substr(1));
						return;
					}
				}
				break;

			case jl_Spectate:
				if(ev->iEventMsg == CHK_CHANGED) {
					cClient->setSpectate(((CCheckbox *)cJoinLobby.getWidget(jl_Spectate))->getValue());
				}
				break;

			/*case jl_StartStopUdpFileDownload:
				if(ev->iEventMsg == TXB_MOUSEUP) {
					if( cClient->getUdpFileDownloader()->getFilesPendingAmount() > 0 )
					{
						cClient->AbortMapDownloads();
						if( ! cClient->getUdpFileDownloader()->isFinished() ||
							cClient->getUdpFileDownloader()->getFilesPendingAmount() > 0 )
						{
							cClient->getUdpFileDownloader()->abortDownload();
							cClient->getNetEngine()->SendFileData();
							cClient->setLastFileRequest( tLX->fCurTime + 10000.0f ); // Disable file download for current session
						}
					}
					else
					{
						cClient->setLastFileRequest( tLX->fCurTime - 10.0f ); // Re-enable file requests
						if( ! cClient->getGameLobby()->bHaveMod && cClient->getGameLobby()->szModDir != "" )
							cClient->getUdpFileDownloader()->requestFileInfo(cClient->getGameLobby()->szModDir, true);
						if( ! cClient->getGameLobby()->bHaveMap && cClient->getGameLobby()->szMapFile != "" )
							cClient->DownloadMap(cClient->getGameLobby()->szMapFile);	// Download map with HTTP, then try UDP
					}
				}
				break;*/
		}
	}

	// Draw the mouse
	DrawCursor(VideoPostProcessor::videoSurface());
}

}; // namespace DeprecatedGUI
