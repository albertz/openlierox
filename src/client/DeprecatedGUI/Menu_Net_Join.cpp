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




#include "LieroX.h"
#include "sound/SoundsBase.h"
#include "CClient.h"
#include "DeprecatedGUI/Graphics.h"
#include "DeprecatedGUI/Menu.h"
#include "GfxPrimitives.h"
#include "StringUtils.h"
#include "game/CWorm.h"
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
#include "Debug.h"
#include "OLXConsole.h"
#include "DeprecatedGUI/CChatWidget.h"
#include "game/Level.h"
#include "game/Mod.h"
#include "game/ServerList.h"


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

void Menu_Net_JoinShutdown()
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

    Menu_redrawBufferRect(0, 0, 640, 480);

	return true;
}

void Menu_Net_JoinConnectionShutdown()
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

		if(!Menu_Net_JoinLobbyInitialize()) {
			// Error
			Menu_Net_MainInitialize();
			return;
		}
	}

	// Check for a bad connection
	if(cClient->getBadConnection()) {
		warnings << "Bad connection: " << cClient->getBadConnectionMsg() << endl;
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
				if(ev->iEventMsg == BTN_CLICKED) {

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
	jl_CancelDownload,
	jl_DownloadProgress,
	jl_DownloadMap,
	jl_DownloadMod,
	jl_More,
	jl_Less
};



///////////////////
// Initialize the joining lobby
bool Menu_Net_JoinLobbyInitialize()
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
void Menu_Net_JoinLobbyShutdown()
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
void Menu_Net_JoinDrawLobby()
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

static void updateDetailsList(CListview* l);
	
static void initDetailsList(CListview* l) {
#define SUBS(title)	l->AddSubitem(LVS_TEXT, title, (DynDrawIntf*)NULL, NULL); l->AddSubitem(LVS_TEXT, "", (DynDrawIntf*)NULL, NULL);
	int index = 0;
	l->Clear();
	l->AddItem("servername", index++, tLX->clNormalLabel); SUBS("Server name:");
	l->AddItem("level", index++, tLX->clNormalLabel); SUBS("Level:"); 
	if (tMenu->bmpDownload.get())  {
		CImage *img = new CImage(tMenu->bmpDownload);
		img->Setup(jl_DownloadMap, 0, 0, img->getWidth(), img->getHeight());
		l->AddSubitem(LVS_WIDGET, "", (DynDrawIntf*)NULL, img);
	}
	l->AddItem("gamemode", index++, tLX->clNormalLabel); SUBS("Game Mode:");
	l->AddItem("mod", index++, tLX->clNormalLabel); SUBS("Mod:");
	if (tMenu->bmpDownload.get())  {
		CImage *img = new CImage(tMenu->bmpDownload);
		img->Setup(jl_DownloadMod, 0, 0, img->getWidth(), img->getHeight());
		l->AddSubitem(LVS_WIDGET, "", (DynDrawIntf*)NULL, img);
	}
	l->AddItem("lives", index++, tLX->clNormalLabel); SUBS("Lives:");
	l->AddItem("maxkills", index++, tLX->clNormalLabel); SUBS("Max Kills:");
	l->AddItem("timelimit", index++, tLX->clNormalLabel); SUBS("Timelimit:");	
	l->AddItem("loadingtime", index++, tLX->clNormalLabel); SUBS("Loading time:");
	l->AddItem("bonuses", index++, tLX->clNormalLabel); SUBS("Bonuses:");

	if (tLXOptions->bAdvancedLobby)  {
		l->AddItem("serverversion", index++, tLX->clNormalLabel); SUBS("Server version:");
	} else {
		// More button
		lv_item_t *it = l->AddItem("more", index++, tLX->clNormalLabel);
		l->AddSubitem(LVS_TEXT, "", (DynDrawIntf*)NULL, NULL);
		CButton *more = new CButton(BUT_MORE, tMenu->bmpButtons);
		more->setID(jl_More);
		more->Create();
		it->iHeight = more->getHeight() + 10;
		l->AddSubitem(LVS_WIDGET, "", (DynDrawIntf*)NULL, more);
	}
#undef SUBS
	updateDetailsList(l);
}

static void updateDetailsList(CListview* l) {
#define SETI	{ i = l->getItem(index++); si = i->tSubitems->tNext; si->iColour = defaultColor; }
	int index = 0;
	lv_item_t* i;
	lv_subitem_t* si;
	l->SaveScrollbarPos();
	Color defaultColor = tLX->clPrivateText;
	SETI; si->sText = cClient->getServerName(); // servername
	SETI;
	if(cClient->getHaveMap()) {
		si->sText = cClient->getGameLobby()[FT_Map].as<LevelInfo>()->name;
		if (si->tNext)
			si->tNext->bVisible = false;  // Hide the download button
	} else {  // Don't have the map
		si->sText = cClient->getGameLobby()[FT_Map].as<LevelInfo>()->path;
		si->iColour = tLX->clError;
		if (si->tNext)  {
			// not downloading the map, display an option for doing so
			si->tNext->bVisible = !cClient->getDownloadingMap();
		} 
	}

	SETI;
	si->sText = cClient->getGameLobby()[FT_GameMode].as<GameModeInfo>()->toString();
	SETI;
	if(cClient->getHaveMod()) {
		si->sText = cClient->getGameLobby()[FT_Mod].as<ModInfo>()->name;
		if (si->tNext)
			si->tNext->bVisible = false;  // Hide the download button
	} else {
		si->sText = cClient->getGameLobby()[FT_Mod].as<ModInfo>()->name;
		si->iColour = tLX->clError;
		if (si->tNext)
			si->tNext->bVisible = !cClient->getDownloadingMod();
	}
	
	SETI;
	if((int)cClient->getGameLobby()[FT_Lives] >= 0) {
		si->sText = itoa((int)cClient->getGameLobby()[FT_Lives]);
	} else {
		si->sText = "infinity";
		si->iColour = tLX->clDisabled;
	}
	
	SETI;
	if((int)cClient->getGameLobby()[FT_KillLimit] >= 0) {
		si->sText = itoa((int)cClient->getGameLobby()[FT_KillLimit]);
	} else {
		si->sText = "infinity";
		si->iColour = tLX->clDisabled;
	}

	SETI;
	if((float)cClient->getGameLobby()[FT_TimeLimit] >= 0) {
		si->sText = ftoa(cClient->getGameLobby()[FT_TimeLimit]) + " min";		
	} else if((float)cClient->getGameLobby()[FT_TimeLimit] <= -100) {
		si->sText = "unknown";
		si->iColour = tLX->clDisabled;
	} else {
		si->sText = "disabled";
		si->iColour = tLX->clDisabled;		
	}
	
	SETI; si->sText = itoa((int)cClient->getGameLobby()[FT_LoadingTime]) + "%";
	SETI; si->sText = cClient->getGameLobby()[FT_Bonuses] ? "On" : "Off";

	// Advanced info
	if (tLXOptions->bAdvancedLobby)  {
		SETI; si->sText = cClient->getServerVersion().asString(); // serverversion
		
		int numItems = l->getNumItems();
		for( int rm = index; rm < numItems; rm++ )
			l->RemoveItem(rm);
		
		for_each_iterator( Feature*, f, Array(featureArray,featureArrayLen()) ) {
			if( cClient->getGameLobby()[f->get()] == f->get()->unsetValue )
				continue;
			i = l->AddItem("feature:" + f->get()->name, index++, tLX->clNormalLabel);
			l->AddSubitem(LVS_TEXT, f->get()->humanReadableName + ":", (DynDrawIntf*)NULL, NULL); 
			l->AddSubitem(LVS_TEXT, "", (DynDrawIntf*)NULL, NULL);
			si = i->tSubitems->tNext;
			
			si->iColour = defaultColor;
			si->sText = cClient->getGameLobby()[f->get()].toString();
		}

		for_each_iterator( FeatureCompatibleSettingList::Feature&, f, GetIteratorRef_second(cClient->getUnknownFeatures().list) ) {
			i = l->AddItem("feature:" + f->get().name, index++, tLX->clNormalLabel);
			l->AddSubitem(LVS_TEXT, f->get().humanName + ":", (DynDrawIntf*)NULL, NULL);
			l->AddSubitem(LVS_TEXT, "", (DynDrawIntf*)NULL, NULL);
			
			si = i->tSubitems->tNext;
			Color col;
			switch(f->get().type) {
				case FeatureCompatibleSettingList::Feature::FCSL_JUSTUNKNOWN: col = tLX->clDisabled; break;
				case FeatureCompatibleSettingList::Feature::FCSL_INCOMPATIBLE: col = tLX->clError; break;
				default: col = defaultColor;
			}
			si->iColour = col;
			si->sText = f->get().var.toString();
		}

		// Less button
		lv_item_t *it = l->AddItem("less", index++, tLX->clNormalLabel);
		l->AddSubitem(LVS_TEXT, "", (DynDrawIntf*)NULL, NULL);
		CButton *less = new CButton(BUT_LESS, tMenu->bmpButtons);
		less->setID(jl_Less);
		less->Create();
		it->iHeight = less->getHeight() + 10;
		l->AddSubitem(LVS_WIDGET, "", (DynDrawIntf*)NULL, less);
	}

	l->RestoreScrollbarPos();
	
#undef SETI
}

////////////////////
// Finished the mod/map download (callback)
void Menu_Net_JoinDlFinished()
{
	CButton *cancel = (CButton *)cJoinLobby.getWidget(jl_CancelDownload);
	CProgressBar *progress = (CProgressBar *)cJoinLobby.getWidget(jl_DownloadProgress);
	CListview *details = (CListview *)cJoinLobby.getWidget(jl_Details);
	if (!cancel || !progress || !details)
		return;

	updateDetailsList(details);

	// If we are still downloading, don't hide the progress
	if (cClient->getDownloadingMap() || cClient->getDownloadingMod())
		return;

	// Hide the progress
	details->Setup(details->getID(), details->getX(), details->getY(), details->getWidth(), 250);
	details->ReadjustScrollbar();

	progress->setEnabled(false);
	cancel->setEnabled(false);
}

////////////////////
// Changes the GUI to display downloading things
void Menu_Net_JoinStartDownload()
{
	CButton *cancel = (CButton *)cJoinLobby.getWidget(jl_CancelDownload);
	CProgressBar *progress = (CProgressBar *)cJoinLobby.getWidget(jl_DownloadProgress);
	CListview *details = (CListview *)cJoinLobby.getWidget(jl_Details);
	if (!cancel || !progress || !details)
		return;

	// Show the progress
	details->Setup(details->getID(), details->getX(), details->getY(), details->getWidth(), 210);
	details->ReadjustScrollbar();

	progress->setEnabled(true);
	cancel->setEnabled(true);
}

	
///////////////////
// Create the lobby gui stuff
void Menu_Net_JoinLobbyCreateGui()
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

	// Downloading stuff
	CProgressBar *dl = new CProgressBar(LoadGameImage("data/frontend/downloadbar_lobby.png", true), 0, 0, false, 1);
	CButton *cancel = new CButton(BUT_CANCEL, tMenu->bmpButtons);
	cJoinLobby.Add( dl, jl_DownloadProgress, 360, 245, 0, 0);
	cJoinLobby.Add( cancel, jl_CancelDownload, 360 + dl->getWidth() + 5, 245 + (dl->getHeight() - 20)/2, 0, 0);
	dl->setEnabled(false);
	cancel->setEnabled(false);
	cClient->setOnMapDlFinished(&Menu_Net_JoinDlFinished);
	cClient->setOnModDlFinished(&Menu_Net_JoinDlFinished);

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
	details->subItemsAreAligned() = true;
	
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
// TODO: please comment the difference between Menu_Net_JoinGotoLobby and GotoJoinLobby
void Menu_Net_JoinGotoLobby()
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
	int			y;

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
		tMenu->bMenuWantsGameStart = true;
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
    line_t ln;
	while(cClient->getChatbox()->GetNewLine(ln)) {
		lv->AddChatBoxLine(ln.strLine, ln.iColour, ln.iTextType);
	}


	// Downloading progress
	if (cClient->getDownloadingMap() || cClient->getDownloadingMod())  {
		CProgressBar *bar = (CProgressBar *)cJoinLobby.getWidget(jl_DownloadProgress);
		if (bar)
			bar->SetPosition(cClient->getDlProgress());
	}
		

	// Draw the connected players
	if (bJoin_Update)  {
		CListview *player_list = (CListview *)cJoinLobby.getWidget(jl_PlayerList);
		if (!player_list) { // Weird, shouldn't happen
			warnings << "Menu_Net_JoinLobbyFrame: player_list unset" << endl;
			return;
		}
		player_list->SaveScrollbarPos();
		player_list->Clear();  // Clear first

		CImage *team_img = NULL;

		for_each_iterator(CWorm*, w_, game.worms()) {
			CWorm* w = w_->get();

			w->ChangeGraphics(cClient->getGameLobby()[FT_GameMode].as<GameModeInfo>()->generalGameType);

			// Add the item
			player_list->AddItem(w->getName(), w->getID(), tLX->clNormalLabel);
			if (w->getLobbyReady())  // Ready control
				player_list->AddSubitem(LVS_IMAGE, "", tMenu->bmpLobbyReady, NULL);
			else
				player_list->AddSubitem(LVS_IMAGE, "", tMenu->bmpLobbyNotReady, NULL);
			player_list->AddSubitem(LVS_IMAGE, "", w->getPicimg(), NULL);  // Skin
			player_list->AddSubitem(LVS_TEXT, "#"+itoa(w->getID())+" "+w->getName(), (DynDrawIntf*)NULL, NULL);  // Name

			// Display the team mark if TDM
			if (cClient->getGameLobby()[FT_GameMode].as<GameModeInfo>()->generalGameType == GMT_TEAMS)  {
				team_img = new CImage(gfxGame.bmpTeamColours[w->getTeam()]);
				if (!team_img)
					continue;
				team_img->setID(w->getID());
				team_img->setRedrawMenu(false);

				player_list->AddSubitem(LVS_WIDGET, "", (DynDrawIntf*)NULL, team_img); // Team
			}
		}

		player_list->RestoreScrollbarPos();  // Scroll back to where we were before the update

		CListview* details = (CListview *)cJoinLobby.getWidget(jl_Details);
		updateDetailsList(details);
		
		bJoin_Update = false;
	}

	// Draw the game info
	if(true) {	//if(GameLobby->bSet) {
		CFont *f = &tLX->cFont;
        int x = 360;
        y = 15;

		f->Draw(VideoPostProcessor::videoSurface(), x, y,  tLX->clHeading, "Game Details");
	}

	// If any textbox is selected, forbid to show the console
	if (cJoinLobby.getFocusedWidget() && !Con_IsVisible())  {
		tMenu->bForbidConsole = cJoinLobby.getFocusedWidget()->getType() == wid_Textbox;
	}

	// Process & Draw the gui
	if (!Con_IsVisible() && !CChatWidget::GlobalEnabled())  // Don't process when the console is opened
		ev = cJoinLobby.Process();

	cJoinLobby.Draw( VideoPostProcessor::videoSurface() );

	if(CChatWidget::GlobalEnabled())
	{
		CChatWidget::GlobalProcessAndDraw(VideoPostProcessor::videoSurface());
		return;
	}

	// Process any events
	if(ev) {
		switch(ev->iControlID) {

			// Back
			case jl_Back:
				if(ev->iEventMsg == BTN_CLICKED) {
					// Click
					PlaySoundSample(sfxGeneral.smpClick);

					Menu_Net_JoinLobbyShutdown();

					// Back to net menu
					Menu_NetInitialize();
				}
				break;

			// Ready
			case jl_Ready:
				if(ev->iEventMsg == BTN_CLICKED) {
					// Let the server know that my worms are now ready
					bool ready = true;
					for_each_iterator(CWorm*, w, game.localWorms())
						ready &= w->get()->getLobbyReady();
					ready = !ready;
					cClient->getNetEngine()->SendUpdateLobby(ready);

					// Hide the ready button
					/*
					CButton *btn = (CButton *)cJoinLobby.getWidget(jl_Ready);
					btn->setEnabled(false);
                    btn->redrawBuffer();
                    */
				}
				break;

			// Add to favourites
			case jl_Favourites:
				if (ev->iEventMsg == BTN_CLICKED) {
					ServerList::get()->addFavourite(cClient->getServerName(),cClient->getServerAddress());
				}
				break;

			// Cancel file transfers
			case jl_CancelDownload:
				if (ev->iEventMsg == BTN_CLICKED)  {
					cClient->AbortDownloads();
				}
				break;

			// Chat box
			case jl_ChatList:
				if (ev->iEventMsg == BRW_KEY_NOT_PROCESSED)  {
					// Activate the textbox automatically
					cJoinLobby.FocusWidget(jl_ChatText);
					tMenu->bForbidConsole = true;

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
					cClient->getNetEngine()->SendText(text, ifTrue<std::string,CWorm*>(game.firstLocalHumanWorm(), &CWorm::getName, ""));
				} else
				if(ev->iEventMsg == TXT_TAB) {
					if(strSeemsLikeChatCommand(Menu_Net_JoinLobbyGetText())) {
						cClient->getNetEngine()->SendChatCommandCompletionRequest(Menu_Net_JoinLobbyGetText().substr(1));
						return;
					}
				}
				break;

			case jl_Details:
				if (ev->iEventMsg == LV_WIDGETEVENT)  {
					CListview *details = (CListview *)cJoinLobby.getWidget(jl_Details);
					gui_event_t *ev2 = details->getWidgetEvent();
					if (ev2)  {
						switch (ev2->iControlID)  {
						case jl_DownloadMap:
							if (ev2->iEventMsg == IMG_CLICK)  {
								cClient->DownloadMap(cClient->getGameLobby()[FT_Map].as<LevelInfo>()->path);  // Download the map
								Menu_Net_JoinStartDownload();
								updateDetailsList(details);
							}
						break;
						case jl_DownloadMod:
							if (ev2->iEventMsg == IMG_CLICK)  {
								cClient->DownloadMod(cClient->getGameLobby()[FT_Mod].as<ModInfo>()->path); // Download the mod
								Menu_Net_JoinStartDownload();
								updateDetailsList(details);
							}
						break;
						case jl_Less:
							if (ev2->iEventMsg == BTN_CLICKED)  {
								tLXOptions->bAdvancedLobby = false;
								initDetailsList(details);
							}
						break;
						case jl_More:
							if (ev2->iEventMsg == BTN_CLICKED)  {
								tLXOptions->bAdvancedLobby = true;
								initDetailsList(details);
							}
						break;
						}
					}
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
							cClient->setLastFileRequest( tLX->currentTime + 10000.0f ); // Disable file download for current session
						}
					}
					else
					{
						cClient->setLastFileRequest( tLX->currentTime - 10.0f ); // Re-enable file requests
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
