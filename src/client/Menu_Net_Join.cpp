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


#include "defs.h"
#include "LieroX.h"
#include "CClient.h"
#include "Graphics.h"
#include "Menu.h"
#include "GfxPrimitives.h"
#include "StringUtils.h"


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
int Menu_Net_JoinInitialize(const std::string& sAddress)
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
int Menu_Net_JoinConnectionInitialize(const std::string& sAddress)
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
	gl->nSet = false;

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
	mouse_t		*Mouse = GetMouse();
	gui_event_t *ev = NULL;

	Menu_redrawBufferRect(0,180,640,tLX->cFont.GetHeight());

	// Process the client frame
	tLX->cFont.DrawCentre(tMenu->bmpScreen, 320, 180, tLX->clNormalLabel, "Connecting to " + sJoinAddress);
	cClient->Frame();


	// Connected??
	if(cClient->getStatus() == NET_CONNECTED) {

		// Leave this connection screen & go to the lobby
		Menu_Net_JoinConnectionShutdown();

		if(!Menu_Net_JoinLobbyInitialize()) {
			// Error
			Menu_Net_MainInitialize();
			return;
		}
	}

	// Check for a bad connection
	if(cClient->getBadConnection()) {
		Menu_MessageBox("Connection Error",cClient->getBadConnectionMsg(), LMB_OK);

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
	cConnecting.Draw( tMenu->bmpScreen );


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
	DrawCursor(tMenu->bmpScreen);
}







/*

   Join lobby

*/

CGuiLayout	cJoinLobby;
int			iJoinSpeaking=-1;
int			iJoin_Recolorize = true;
enum {
	jl_Back=0,
	jl_Ready,
	jl_ChatText,
	jl_ChatList,
	jl_Favourites
};



///////////////////
// Initialize the joining lobby
int Menu_Net_JoinLobbyInitialize(void)
{
    Menu_Net_JoinDrawLobby();

    Menu_Net_JoinLobbyCreateGui();

	iNetMode = net_join;
	iJoinMenu = join_lobby;

	cClient->getChatbox()->Clear();
    cClient->getChatbox()->setWidth(570);
    iJoinSpeaking=-1;

	return true;
}

////////////////////
// Shutdown the join lobby
void Menu_Net_JoinLobbyShutdown(void)
{
	cClient->Disconnect();
	cJoinLobby.Shutdown();
	cClient->Shutdown();
}


///////////////////
// Draw the join lobby
void Menu_Net_JoinDrawLobby(void)
{
	// Create the buffer
	DrawImage(tMenu->bmpBuffer,tMenu->bmpMainBack_wob,0,0);
	if (tMenu->tFrontendInfo.bPageBoxes)
		Menu_DrawBox(tMenu->bmpBuffer, 5,5, 635, 475);

    // Title
    DrawImageAdv(tMenu->bmpBuffer, tMenu->bmpMainBack_wob, 281,0, 281,0, 79,20);
    tLX->cFont.DrawCentre(tMenu->bmpBuffer, 320, -1, tLX->clNormalLabel, "[  Lobby  ]");

	// Chat box
    DrawRectFill(tMenu->bmpBuffer, 16, 270, 624, 417, tLX->clBlack);

    // Player box
    Menu_DrawBox(tMenu->bmpBuffer, 15, 29, 340, 235);

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
	cJoinLobby.Add( new CLabel("Players",tLX->clHeading),	  -1,		15,  15,  0,   0);
	cJoinLobby.Add( new CTextbox(),							  jl_ChatText, 15,  421, 610, 20);
    cJoinLobby.Add( new CListview(),                          jl_ChatList, 15,  268, 610, 150);

	cJoinLobby.SendMessage(jl_ChatText,TXM_SETMAX,64,0);
	//cJoinLobby.SendMessage(jl_ChatList,		LVM_SETOLDSTYLE, 0, 0);
}


///////////////////
// Go straight back to the join lobby
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
		line_t *l = NULL;
		for (uint i=(uint)MAX((int)0,(int)(cClient->getChatbox()->getNumLines()-255));i<cClient->getChatbox()->getNumLines();i++)  {
			l = cClient->getChatbox()->GetLine(i);
			if (l) if (l->iColour == tLX->clChatText)  {
				if(lv->getLastItem())
					lv->AddItem("", lv->getLastItem()->iIndex+1, l->iColour);
				else
					lv->AddItem("", 0, l->iColour);
				lv->AddSubitem(LVS_TEXT, l->strLine, NULL);
			}
		}
		lv->scrollLast();
		lv->setShowSelect(false);
	}

    iJoinSpeaking=-1;
}

//////////////////////
// Get the content of the chatbox
std::string Menu_Net_JoinLobbyGetText(void)
{
	static std::string buf;
	cJoinLobby.SendMessage(jl_ChatText, TXS_GETTEXT, &buf, 256);
	return buf;
}


///////////////////
// Join lobby frame
void Menu_Net_JoinLobbyFrame(int mouse)
{
	mouse_t		*Mouse = GetMouse();
	gui_event_t *ev = NULL;
	int			i,y,local;

	// Process the client
	cClient->Frame();

    // If there is a client error, leave
    if(cClient->getClientError()) {

        cJoinLobby.Shutdown();
        Menu_NetInitialize();
		return;
    }


	// If we have started, leave the frontend
	if(cClient->getGameReady()) {

		cJoinLobby.Shutdown();

        // Setup the client
        cClient->SetupViewports();

		// Leave the frontend
		*bGame = true;
		tMenu->iMenuRunning = false;
		tGameInfo.iGameType = GME_JOIN;

		return;
	}


	// Check if the communication link between us & server is still ok
	if(cClient->getServerError()) {
		// Create the buffer
		DrawImage(tMenu->bmpBuffer,tMenu->bmpMainBack_wob,0,0);
		Menu_DrawSubTitle(tMenu->bmpBuffer,SUB_LOBBY);

		Menu_MessageBox("Communication", cClient->getServerErrorMsg(), LMB_OK);

        cJoinLobby.Shutdown();

		Menu_NetInitialize();
		return;
	}


    // Clear the player list and game settings
    Menu_redrawBufferRect(15, 20,  335, 230);
    Menu_redrawBufferRect(360,20,  280, 200);


	// Process & Draw the gui
#ifdef WITH_MEDIAPLAYER
	if (!cMediaPlayer.GetDrawPlayer())
#endif
		ev = cJoinLobby.Process();
	cJoinLobby.Draw( tMenu->bmpScreen );


	// Add chat to the listbox
	CListview *lv = (CListview *)cJoinLobby.getWidget(jl_ChatList);
    line_t *ln = NULL;
	while((ln = cClient->getChatbox()->GetNewLine()) != NULL) {

        if(lv->getLastItem())
            lv->AddItem("", lv->getLastItem()->iIndex+1, ln->iColour);
        else
            lv->AddItem("", 0, ln->iColour);
        lv->AddSubitem(LVS_TEXT, ln->strLine, NULL);
        lv->setShowSelect(false);

        // If there are too many lines, remove the top line
        if(lv->getItemCount() > 256) {
            if(lv->getItems())
                lv->RemoveItem(lv->getItems()->iIndex);
        }

        lv->scrollLast();
	}


	// Draw the connected players
	CWorm *w = cClient->getRemoteWorms();
	game_lobby_t *gl = cClient->getGameLobby();
	lobbyworm_t *l;
	local = false;
    y = 40;
	bool bRecolorized = false;
	for(i=0; i<MAX_WORMS; i++, w++) {
		if( !w->isUsed() )
            continue;

		l = w->getLobby();

		local = false;
		if(w->isUsed())
			local = w->getLocal();

		if(local && iJoinSpeaking == -1)
			iJoinSpeaking = i;

		// Check for a click
		if(Mouse->Up & SDL_BUTTON(1)) {
			if(Mouse->Y > y && Mouse->Y < y+18) {

				// Speech
				/*if(Mouse->X > 20 && Mouse->X < 34) {
					if(l->iType == LBY_USED &&
					   local &&
					   cClient->getNumWorms()>1 &&
					   iJoinSpeaking != i)
					      iJoinSpeaking = i;
				}*/
			}
		}

        int x = 20;
		switch(l->iType) {

			// Open
			case LBY_OPEN:
				//tLX->cFont.Draw(tMenu->bmpScreen, 77, y, tLX->clNormalLabel,"%s", "------");
				break;

			// Closed
			case LBY_CLOSED:
				//tLX->cFont.Draw(tMenu->bmpScreen, 77, y, tLX->clNormalLabel,"%s", "Closed");
				break;

			// Used
			case LBY_USED:
				// Ready icon
                if(l->iReady)
					DrawImageAdv(tMenu->bmpScreen, tMenu->bmpLobbyState, 0,0, x+15,y-1,12,12);
				else
					DrawImageAdv(tMenu->bmpScreen, tMenu->bmpLobbyState, 0,12,x+15,y-1,12,12);

                // Worm
                DrawImage(tMenu->bmpScreen, w->getPicimg(), x+30, y-2);
				tLX->cFont.Draw(tMenu->bmpScreen, x+55, y-2, tLX->clNormalLabel, w->getName());

                // Team
                if( gl->nGameMode == GMT_TEAMDEATH )  {
					// Colorize the worm
					if (iJoin_Recolorize) {
						w->setTeam(l->iTeam);
						w->LoadGraphics(GMT_TEAMDEATH);
						bRecolorized = true;
					}

					DrawImage(tMenu->bmpScreen, tMenu->bmpTeamColours[l->iTeam], x+200, y-2);
				}
				else  {
					// Recolorize the skin
					if (iJoin_Recolorize) {
						w->LoadGraphics(gl->nGameMode);
						bRecolorized = true;
					}
				}

				break;
		}

        // Dividing line
        DrawHLine(tMenu->bmpScreen, x, x+315, y+20, MakeColour(64,64,64));
        y+=25;
	}

	if (bRecolorized)
		iJoin_Recolorize = false;


	// Draw the game info
	if(gl->nSet) {
		CFont *f = &tLX->cFont;
		Uint32 blue = MakeColour(0,138,251);
        int x = 360;
        int x2 = x+105;
        y = 15;

		const std::string gamemodes[] = {"Deathmatch","Team Deathmatch", "Tag", "Demolitions"};

        tGameInfo.sMapname = gl->szMapName;
        tGameInfo.sModName = gl->szModName;
        tGameInfo.sModDir = gl->szModDir;
		tGameInfo.iBonusesOn = gl->nBonuses;
		tGameInfo.iGameMode = gl->nGameMode;
		tGameInfo.iKillLimit = gl->nMaxKills;
		tGameInfo.iLives = gl->nLives;
		tGameInfo.iLoadingTimes = gl->nLoadingTime;

		f->Draw(tMenu->bmpScreen, x, y,  blue, "Game Details");
		f->Draw(tMenu->bmpScreen, x, y+20,  tLX->clNormalLabel, "Level:");
        if(gl->bHaveMap)  {
			f->Draw(tMenu->bmpScreen, x2, y+20, tLX->clNormalLabel, gl->szDecodedMapName);
		}
        else
            f->Draw(tMenu->bmpScreen, x2, y+20,  tLX->clError, gl->szMapName);
		f->Draw(tMenu->bmpScreen, x, y+40, tLX->clNormalLabel, "Game Mode:");
		f->Draw(tMenu->bmpScreen, x2, y+40, tLX->clNormalLabel, gamemodes[gl->nGameMode]);
        f->Draw(tMenu->bmpScreen, x, y+60, tLX->clNormalLabel,  "Mod:");
        if(gl->bHaveMod)
            f->Draw(tMenu->bmpScreen, x2, y+60, tLX->clNormalLabel,  gl->szModName);
        else
            f->Draw(tMenu->bmpScreen, x2, y+60, tLX->clError, gl->szModName);

		f->Draw(tMenu->bmpScreen, x, y+80, tLX->clNormalLabel, "Lives:");
		if(gl->nLives >= 0)
			f->Draw(tMenu->bmpScreen, x2, y+80, tLX->clNormalLabel, itoa(gl->nLives));

		f->Draw(tMenu->bmpScreen, x, y+100, tLX->clNormalLabel, "Max Kills:");
		if(gl->nMaxKills >= 0)
			f->Draw(tMenu->bmpScreen, x2, y+100, tLX->clNormalLabel, itoa(gl->nMaxKills));
		f->Draw(tMenu->bmpScreen,     x, y+120, tLX->clNormalLabel, "Loading time:");
		f->Draw(tMenu->bmpScreen,     x2, y+120, tLX->clNormalLabel, itoa(gl->nLoadingTime) + "%");
        f->Draw(tMenu->bmpScreen,     x, y+140, tLX->clNormalLabel, "Bonuses:");
        f->Draw(tMenu->bmpScreen,     x2, y+140, tLX->clNormalLabel, gl->nBonuses ? "On" : "Off");
	}




	// Process any events
	if(ev) {

		switch(ev->iControlID) {

			// Back
			case jl_Back:
				if(ev->iEventMsg == BTN_MOUSEUP) {
					// Disconnect
					/*cClient->Disconnect();

					// Click!
					PlaySoundSample(sfxGeneral.smpClick);

					// Shutdown
					cJoinLobby.Shutdown();

					// Disconnect & shutdown
					//cClient->Disconnect();
					cClient->Shutdown();*/

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
					cClient->getChannel()->getMessageBS()->Append(&bs);


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


			// Chat textbox
			case jl_ChatText:
				if(ev->iEventMsg == TXT_ENTER && iJoinSpeaking >= 0) {
					// Send the msg to the server

					// Get the text
					static std::string buf;
					cJoinLobby.SendMessage(jl_ChatText, TXS_GETTEXT, &buf, 0);

                    // Don't send empty messages
                    if(buf == "")
                        break;

					// Clear the text box
					cJoinLobby.SendMessage(jl_ChatText, TXS_SETTEXT, "",0);

					// Get name
					std::string text;
					CWorm *rw = cClient->getRemoteWorms() + iJoinSpeaking;
					if(!strincludes(buf,"/me"))
						text = rw->getName() + ": " + buf;
					else
						text = replacemax(buf,"/me",rw->getName(),text,2);
					cClient->SendText(text);
				}
				break;
		}
	}



	// Draw the mouse
	DrawCursor(tMenu->bmpScreen);
}
