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
#include "DeprecatedGUI/CMediaPlayer.h"
#include "DeprecatedGUI/CCheckbox.h"
#include "DeprecatedGUI/CLabel.h"
#include "DeprecatedGUI/CTextButton.h"
#include "DeprecatedGUI/CProgressbar.h"
#include "AuxLib.h"

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
	tGameInfo.iGameType = GME_JOIN;
	sJoinAddress = sAddress;
	cConnecting.Shutdown();
	cConnecting.Initialize();

	cConnecting.Add( new CButton(BUT_CANCEL, tMenu->bmpButtons),	cm_Cancel, 	25, 440, 75,15);

	if(!cClient->Initialize()) {
		// Error

		// Go back to the network menu
		Menu_Net_MainInitialize();
		cConnecting.Shutdown();
		return false;
	}

	game_lobby_t *gl = cClient->getGameLobby();
	gl->bSet = false;

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
#ifdef WITH_MEDIAPLAYER
	if (!cMediaPlayer.GetDrawPlayer())
#endif
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
	CListview *lv = (CListview *)cJoinLobby.getWidget(jl_ChatList);
	if (lv)  {
		CChatBox *Chatbox = cClient->getChatbox();
		lines_iterator it = Chatbox->Begin();

		// Copy the chat text
		for (int id = 0; it != Chatbox->End(); it++, id++)  {
			lv->AddItem("", id, it->iColour);
			lv->AddSubitem(LVS_TEXT, it->strLine, NULL, NULL);
			id++;
		}

		lv->scrollLast();
		lv->setShowSelect(false);
	}

	iNetMode = net_join;
	iJoinMenu = join_lobby;

	cClient->getChatbox()->Clear();
    cClient->getChatbox()->setWidth(570);
    iJoinSpeaking = 0;  // The first player is always speaking
	tMenu->sSavedChatText = "";

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


///////////////////
// Create the lobby gui stuff
void Menu_Net_JoinLobbyCreateGui(void)
{
    cJoinLobby.Shutdown();
	cJoinLobby.Initialize();

	cJoinLobby.Add( new CButton(BUT_LEAVE, tMenu->bmpButtons),jl_Back,	15,  450, 60,  15);
    cJoinLobby.Add( new CButton(BUT_READY, tMenu->bmpButtons),jl_Ready,	560, 450, 65,  15);
	cJoinLobby.Add( new CButton(BUT_ADDTOFAVOURITES, tMenu->bmpButtons), jl_Favourites,360,220,150,15);
	cJoinLobby.Add( new CTextbox(),							  jl_ChatText, 15,  421, 610, tLX->cFont.GetHeight());
    cJoinLobby.Add( new CListview(),                          jl_ChatList, 15,  268, 610, 150);
	cJoinLobby.Add( new CListview(),						  jl_PlayerList, 15, 15, 325, 220);
	cJoinLobby.Add( new CCheckbox(cClient->getSpectate()),	  jl_Spectate, 15, 244, 17, 17 );
	cJoinLobby.Add( new CLabel( "Spectate only", tLX->clNormalLabel ), -1, 40, 245, 0, 0 );

	// Downloading stuff
	CProgressBar *dl = new CProgressBar(LoadGameImage("data/frontend/downloadbar_lobby.png", true), 0, 0, false, 1);
	CButton *cancel = new CButton(BUT_CANCEL, tMenu->bmpButtons);
	cJoinLobby.Add( dl, jl_DownloadProgress, 360, 195, 0, 0);
	cJoinLobby.Add( cancel, jl_CancelDownload, 360 + dl->getWidth() + 5, 195 + (dl->getHeight() - 20)/2, 0, 0);
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

	cClient->getChatbox()->setWidth(570);

	// Add the chat
	CListview *lv = (CListview *)cJoinLobby.getWidget(jl_ChatList);
	if (lv)  {
		CChatBox *Chatbox = cClient->getChatbox();
		lines_iterator it = Chatbox->At((int)Chatbox->getNumLines()-256); // If there's more than 256 messages, we start not from beginning but from end()-256
		int id = (lv->getLastItem() && lv->getItems()) ? lv->getLastItem()->iIndex + 1 : 0;

		// Copy the chat text
		for (; it != Chatbox->End(); it++)  {
			if (it->iColour == tLX->clChatText)  {  // Add only chat messages
				lv->AddItem("", id, it->iColour);
				lv->AddSubitem(LVS_TEXT, it->strLine, NULL, NULL);
				id++;
			}
		}

		lv->scrollLast();
		lv->setShowSelect(false);
	}

	// Add the ingame chatter text to lobby chatter
	cJoinLobby.SendMessage(jl_ChatText, TXS_SETTEXT, cClient->getChatterText(), 0);

    iJoinSpeaking = 0; // The first player is always speaking
	tMenu->sSavedChatText = "";
}

//////////////////////
// Get the content of the chatbox
std::string Menu_Net_JoinLobbyGetText(void)
{
	if (tMenu->bMenuRunning)  {
		std::string buf;
		cJoinLobby.SendMessage(jl_ChatText, TXS_GETTEXT, &buf, 256);
		return buf;
	} else {
		return tMenu->sSavedChatText;
	}
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
		tGameInfo.iGameType = GME_JOIN;

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
	CListview *lv = (CListview *)cJoinLobby.getWidget(jl_ChatList);
    line_t *ln = NULL;
	while((ln = cClient->getChatbox()->GetNewLine()) != NULL) {

        if(lv->getLastItem())
            lv->AddItem("", lv->getLastItem()->iIndex+1, ln->iColour);
        else
            lv->AddItem("", 0, ln->iColour);
        lv->AddSubitem(LVS_TEXT, ln->strLine, NULL, NULL);
        lv->setShowSelect(false);

        // If there are too many lines, remove the top line
        if(lv->getItemCount() > 256) {
            if(lv->getItems())
                lv->RemoveItem(lv->getItems()->iIndex);
        }

        lv->scrollLast();
	}


	game_lobby_t *gl = cClient->getGameLobby();

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
			w->ChangeGraphics(gl->nGameMode);

			// Add the item
			player_list->AddItem(w->getName(), i, tLX->clNormalLabel);
			if (lobby_worm->bReady)  // Ready control
				player_list->AddSubitem(LVS_IMAGE, "", tMenu->bmpLobbyReady, NULL);
			else
				player_list->AddSubitem(LVS_IMAGE, "", tMenu->bmpLobbyNotReady, NULL);
			player_list->AddSubitem(LVS_IMAGE, "", w->getPicimg(), NULL);  // Skin
			player_list->AddSubitem(LVS_TEXT, "#"+itoa(w->getID())+" "+w->getName(), NULL, NULL);  // Name

			// Display the team mark if TDM
			if (gl->nGameMode == GMT_TEAMDEATH)  {
				team_img = new CImage(gfxGame.bmpTeamColours[lobby_worm->iTeam]);
				if (!team_img)
					continue;
				team_img->setID(w->getID());
				team_img->setRedrawMenu(false);

				player_list->AddSubitem(LVS_WIDGET, "", NULL, team_img); // Team

				gl->nLastGameMode = gl->nGameMode;
			}
		}

		player_list->RestoreScrollbarPos();  // Scroll back to where we were before the update

		bJoin_Update = false;
	}

	// Draw the game info
	if(gl->bSet) {
		CFont *f = &tLX->cFont;
        int x = 360;
        int x2 = x+105;
        y = 15;

		const std::string gamemodes[] = {"Deathmatch","Team Deathmatch", "Tag", "Demolitions"};

        tGameInfo.sMapFile = gl->szMapName;
		tGameInfo.sMapName = gl->szDecodedMapName;
        tGameInfo.sModName = gl->szModName;
        tGameInfo.sModDir = gl->szModDir;
		tGameInfo.bBonusesOn = gl->bBonuses;
		tGameInfo.iGameMode = gl->nGameMode;
		tGameInfo.iKillLimit = gl->nMaxKills;
		tGameInfo.iLives = gl->nLives;
		tGameInfo.iLoadingTimes = gl->nLoadingTime;

		f->Draw(VideoPostProcessor::videoSurface(), x, y,  tLX->clHeading, "Game Details");
		f->Draw(VideoPostProcessor::videoSurface(), x, y+20,  tLX->clNormalLabel, "Level:");
        if(gl->bHaveMap)  {
			f->Draw(VideoPostProcessor::videoSurface(), x2, y+20, tLX->clNormalLabel, gl->szDecodedMapName);
		} else {  // Don't have the map
			if (cClient->getDownloadingMap())  {  // Currently downloading the map
				f->Draw(VideoPostProcessor::videoSurface(), x2, y+20,  tLX->clError, gl->szMapName);
			} else { // Not downloading
				f->Draw(VideoPostProcessor::videoSurface(), x2, y+20,  tLX->clError, gl->szMapName);
				if (tMenu->bmpDownload.get())
					DrawImage(VideoPostProcessor::videoSurface(), tMenu->bmpDownload, x2 + f->GetWidth(gl->szMapName) + 5, y + 20 + (f->GetHeight() - tMenu->bmpDownload->h)/2);

				if (MouseInRect(x2, y+20, 640-x2, tLX->cFont.GetHeight()))  {
					SetGameCursor(CURSOR_HAND);
					if (GetMouse()->Up)
						cClient->DownloadMap(gl->szMapName); // Download the map
				} else {
					SetGameCursor(CURSOR_ARROW);
				}
			}
		}
		f->Draw(VideoPostProcessor::videoSurface(), x, y+40, tLX->clNormalLabel, "Game Mode:");
		f->Draw(VideoPostProcessor::videoSurface(), x2, y+40, tLX->clNormalLabel, gamemodes[gl->nGameMode]);
        f->Draw(VideoPostProcessor::videoSurface(), x, y+60, tLX->clNormalLabel,  "Mod:");
        if(gl->bHaveMod)
            f->Draw(VideoPostProcessor::videoSurface(), x2, y+60, tLX->clNormalLabel,  gl->szModName);
		else {
			if (cClient->getDownloadingMod())
				f->Draw(VideoPostProcessor::videoSurface(), x2, y+60, tLX->clError, gl->szModName);
			else {
				f->Draw(VideoPostProcessor::videoSurface(), x2, y+60, tLX->clError, gl->szModName);
				if (tMenu->bmpDownload.get())
					DrawImage(VideoPostProcessor::videoSurface(), tMenu->bmpDownload, x2 + f->GetWidth(gl->szModName) + 5, y + 60 + (f->GetHeight() - tMenu->bmpDownload->h)/2);

				if (MouseInRect(x2, y+60, 640-x2, tLX->cFont.GetHeight()))  {
					SetGameCursor(CURSOR_HAND);
					if (GetMouse()->Up)  {
						cClient->DownloadMod(cClient->getGameLobby()->szModDir);
					}
				} else {
					SetGameCursor(CURSOR_ARROW);
				}
			}
		}

		f->Draw(VideoPostProcessor::videoSurface(), x, y+80, tLX->clNormalLabel, "Lives:");
		if(gl->nLives >= 0)
			f->Draw(VideoPostProcessor::videoSurface(), x2, y+80, tLX->clNormalLabel, itoa(gl->nLives));

		f->Draw(VideoPostProcessor::videoSurface(), x, y+100, tLX->clNormalLabel, "Max Kills:");
		if(gl->nMaxKills >= 0)
			f->Draw(VideoPostProcessor::videoSurface(), x2, y+100, tLX->clNormalLabel, itoa(gl->nMaxKills));
		f->Draw(VideoPostProcessor::videoSurface(),     x, y+120, tLX->clNormalLabel, "Loading time:");
		f->Draw(VideoPostProcessor::videoSurface(),     x2, y+120, tLX->clNormalLabel, itoa(gl->nLoadingTime) + "%");
        f->Draw(VideoPostProcessor::videoSurface(),     x, y+140, tLX->clNormalLabel, "Bonuses:");
        f->Draw(VideoPostProcessor::videoSurface(),     x2, y+140, tLX->clNormalLabel, gl->bBonuses ? "On" : "Off");
	}

	{
		/*if( cClient->getDownloadingMap() )
			tLX->cFont.Draw(VideoPostProcessor::videoSurface(),     410, 195, tLX->clNormalLabel,
			itoa( cClient->getMapDlProgress() ) + "%: " + gl->szMapName );
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
			if (cClient->getDownloadingMap())
				progress->SetPosition(cClient->getMapDlProgress());
			else
				progress->SetPosition(cClient->getModDlProgress());
		} else {
			progress->setEnabled(false);
			cancel->setEnabled(false);
		}
	}

	// Process & Draw the gui
#ifdef WITH_MEDIAPLAYER
	if (!cMediaPlayer.GetDrawPlayer())
#endif
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
					CBytestream bs;
					bs.Clear();
					bs.writeByte(C2S_UPDATELOBBY);
					bs.writeByte(1);
					cClient->getChannel()->AddReliablePacketToSend(bs);


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
					cClient->SendText(text, cClient->getWorm(0)->getName());
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
							CBytestream bs;
							bs.writeByte(C2S_SENDFILE);
							cClient->getUdpFileDownloader()->send(&bs);
							cClient->getChannel()->AddReliablePacketToSend(bs);
							cClient->setLastFileRequest( tLX->fCurTime + 10000.0f ); // Disable file download for current session
						}
					}
					else
					{
						cClient->setLastFileRequest( tLX->fCurTime - 10.0f ); // Re-enable file requests
						if( ! cClient->getGameLobby()->bHaveMod && cClient->getGameLobby()->szModDir != "" )
							cClient->getUdpFileDownloader()->requestFileInfo(cClient->getGameLobby()->szModDir, true);
						if( ! cClient->getGameLobby()->bHaveMap && cClient->getGameLobby()->szMapName != "" )
							cClient->DownloadMap(cClient->getGameLobby()->szMapName);	// Download map with HTTP, then try UDP
					}
				}
				break;*/
		}
	}

	// Draw the mouse
	DrawCursor(VideoPostProcessor::videoSurface());
}

}; // namespace DeprecatedGUI
