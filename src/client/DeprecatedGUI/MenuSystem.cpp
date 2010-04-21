/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Menu System functions
// Created 30/6/02
// Jason Boettcher


#include <assert.h>

#include <set>

#include "Debug.h"
#include "LieroX.h"
#include "OLXConsole.h"
#include "EndianSwap.h"
#include "AuxLib.h"
#include "Error.h"
#include "ConfigHandler.h"
#include "CClient.h"
#include "IpToCountryDB.h"
#include "DeprecatedGUI/Graphics.h"
#include "DeprecatedGUI/Menu.h"
#include "GfxPrimitives.h"
#include "FindFile.h"
#include "StringUtils.h"
#include "CWorm.h"
#include "Cursor.h"
#include "DeprecatedGUI/CButton.h"
#include "DedicatedControl.h"
#include "OLXG15.h"
#include "Timer.h"
#include "IRC.h"
#include "FileUtils.h"
#include "OLXCommand.h"
#include "HTTP.h"
#include "Version.h"
#include "CrashHandler.h"
#include "game/Level.h"
#include "sound/sfx.h"
#include "TaskManager.h"


// TODO: move this out here
// declare them only locally here as nobody really should use them explicitly
std::string Utf8String(const std::string &OldLxString);




namespace DeprecatedGUI {

menu_t	*tMenu = NULL;


bool		*bGame = NULL;
int			iSkipStart = false;
CWidgetList	LayoutWidgets[LAYOUT_COUNT];


///////////////////
// Initialize the menu system
bool Menu_Initialize(bool *game)
{
	bGame = game;
	*bGame = false;
	bJoin_Update = true;
	bHost_Update = true;

	// Allocate the menu structure
	tMenu = new menu_t;
    if(tMenu == NULL) {
        SystemError("Error: Out of memory in for menu");
		return false;
    }

	if(bDedicated) return true;
	
	// Load the frontend info
	Menu_LoadFrontendInfo();

	tMenu->iReturnTo = net_internet;
	tMenu->bForbidConsole = false;

	// Load the images
	//LOAD_IMAGE(tMenu->bmpMainBack,"data/frontend/background.png");
    //LOAD_IMAGE(tMenu->bmpMainBack_lg,"data/frontend/background_lg.png");
    LOAD_IMAGE(tMenu->bmpMainBack_wob,"data/frontend/background_wob.png");

	// bmpMainBack_common, for backward compatibility: if it doesn't exist, we use bmpMainBack_wob
	tMenu->bmpMainBack_common = LoadGameImage("data/frontend/background_common.png");
	if (!tMenu->bmpMainBack_common.get())
		tMenu->bmpMainBack_common = tMenu->bmpMainBack_wob;


	tMenu->bmpBuffer = gfxCreateSurface(640,480);
    if(tMenu->bmpBuffer.get() == NULL) {
        SystemError("Error: Out of memory back buffer");
		return false;
    }


	tMenu->bmpMsgBuffer = gfxCreateSurface(640,480);
    if(tMenu->bmpMsgBuffer.get() == NULL) {
        SystemError("Error: Out of memory in MsgBuffer");
		return false;
	}

    tMenu->bmpMiniMapBuffer = gfxCreateSurface(128,96);
    if(tMenu->bmpMiniMapBuffer.get() == NULL) {
        SystemError("Error: Out of memory in MiniMapBuffer");
		return false;
    }

	SmartPointer<SDL_Surface> lobby_state = NULL;
	LOAD_IMAGE_WITHALPHA(tMenu->bmpMainTitles,"data/frontend/maintitles.png");
	LOAD_IMAGE_WITHALPHA(tMenu->bmpLieroXtreme,"data/frontend/lierox.png");
	LOAD_IMAGE_WITHALPHA(tMenu->bmpSubTitles,"data/frontend/subtitles.png");
	LOAD_IMAGE_WITHALPHA(tMenu->bmpButtons,"data/frontend/buttons.png");
	LOAD_IMAGE_WITHALPHA(tMenu->bmpMapEdTool,"data/frontend/map_toolbar.png");
	LOAD_IMAGE_WITHALPHA(tMenu->bmpCheckbox,"data/frontend/checkbox.png");
	LOAD_IMAGE_WITHALPHA(tMenu->bmpInputbox,"data/frontend/inputbox.png");
	//LOAD_IMAGE_WITHALPHA(tMenu->bmpAI,"data/frontend/cpu.png");
	LOAD_IMAGE_WITHALPHA(lobby_state, "data/frontend/lobbyready.png");;
	LOAD_IMAGE_WITHALPHA(tMenu->bmpConnectionSpeeds[0], "data/frontend/con_good.png");
	LOAD_IMAGE_WITHALPHA(tMenu->bmpConnectionSpeeds[1], "data/frontend/con_average.png");
	LOAD_IMAGE_WITHALPHA(tMenu->bmpConnectionSpeeds[2], "data/frontend/con_bad.png");
	LOAD_IMAGE_WITHALPHA(tMenu->bmpConnectionSpeeds[3], "data/frontend/con_none.png");
	LOAD_IMAGE_WITHALPHA2(tMenu->bmpConnectionSpeeds[4], "data/frontend/con_nat.png", "data/frontend/con_bad.png");
	LOAD_IMAGE_WITHALPHA(tMenu->bmpTriangleUp, "data/frontend/triangle_up.png");
	LOAD_IMAGE_WITHALPHA(tMenu->bmpTriangleDown, "data/frontend/triangle_down.png");
	tMenu->bmpDownload = LoadGameImage("data/frontend/download.png", true); // Doesn't have to exist
	tMenu->bmpChatBackground = LoadGameImage("data/frontend/background_chat.png", true);
	tMenu->bmpChatBackgroundMain = LoadGameImage("data/frontend/background_chat_main.png", true);

	// Split up the lobby ready image
	tMenu->bmpLobbyReady = gfxCreateSurfaceAlpha(lobby_state.get()->w, 12);
	if (!tMenu->bmpLobbyReady.get())  {
		errors << "Out of memory while creating tMenu->bmpLobbyReady" << endl;
		return false;
	}
	CopySurface(tMenu->bmpLobbyReady.get(), lobby_state, 0, 0, 0, 0, lobby_state.get()->w, 12);

	tMenu->bmpLobbyNotReady = gfxCreateSurfaceAlpha(lobby_state.get()->w, 12);
	if (!tMenu->bmpLobbyNotReady.get())  {
		errors << "Out of memory while creating tMenu->bmpLobbyNotReady" << endl;
		return false;
	}
	CopySurface(tMenu->bmpLobbyNotReady.get(), lobby_state, 0, 12, 0, 0, lobby_state.get()->w, 12);


	for (size_t i = 0; i < sizeof(tMenu->tSocket)/sizeof(tMenu->tSocket[0]); ++i)
		tMenu->tSocket[i] = new NetworkSocket();

	// HACK: open an unreliable foo socket
	// Some routers simply ignore first open socket and don't let any data through, this is a workaround
	tMenu->tSocket[SCK_FOO]->OpenUnreliable(0);
	// Open a socket for broadcasting over a LAN (UDP)
	tMenu->tSocket[SCK_LAN]->OpenBroadcast(0);
	// Open a socket for communicating over the net (UDP)
	tMenu->tSocket[SCK_NET]->OpenUnreliable(0);	
	
	if(!tMenu->tSocket[SCK_LAN]->isOpen() || !tMenu->tSocket[SCK_NET]->isOpen()) {
		SystemError("Error: Failed to open a socket for networking");
		return false;
	}

	// Send some random data to some random IP
	if (tMenu->tSocket[SCK_FOO]->isOpen())  {
		NetworkAddr a; StringToNetAddr("1.2.3.4:5678", a);
		// For example, if no network is connected, you likely only have 127.* in your routing table.
		if(IsNetAddrAvailable(a)) {
			tMenu->tSocket[SCK_FOO]->setRemoteAddress(a);
			tMenu->tSocket[SCK_FOO]->Write("foo");
		}
	}

	// Add default widget IDs to the widget list
	//Menu_AddDefaultWidgets();

	return true;
}

/////////////////////////
// Load the infor about frontend
void Menu_LoadFrontendInfo()
{
	ReadInteger("data/frontend/frontend.cfg","MainTitles","X",&tMenu->tFrontendInfo.iMainTitlesLeft,50);
	ReadInteger("data/frontend/frontend.cfg","MainTitles","Y",&tMenu->tFrontendInfo.iMainTitlesTop,160);
	ReadInteger("data/frontend/frontend.cfg","Credits","X",&tMenu->tFrontendInfo.iCreditsLeft,370);
	ReadInteger("data/frontend/frontend.cfg","Credits","Y",&tMenu->tFrontendInfo.iCreditsTop,379);
	ReadInteger("data/frontend/frontend.cfg","Credits","Spacing",&tMenu->tFrontendInfo.iCreditsSpacing,0);
	ReadString ("data/frontend/frontend.cfg","Credits","FrontendCredits", tMenu->tFrontendInfo.sFrontendCredits, " ");
	ReadInteger("data/frontend/frontend.cfg","MainTitles","Spacing",&tMenu->tFrontendInfo.iMainTitlesSpacing,15);
	ReadKeyword("data/frontend/frontend.cfg","PageBoxes","Visible",&tMenu->tFrontendInfo.bPageBoxes,true);
	ReadInteger("data/frontend/frontend.cfg","IpToCountryLoading","AnimX",&tMenu->tFrontendInfo.iLoadingAnimLeft,5);
	ReadInteger("data/frontend/frontend.cfg","IpToCountryLoading","AnimY",&tMenu->tFrontendInfo.iLoadingAnimTop,5);
	ReadFloat("data/frontend/frontend.cfg","IpToCountryLoading","AnimFrameTime",&tMenu->tFrontendInfo.fLoadingAnimFrameTime,0.2f);
	ReadInteger("data/frontend/frontend.cfg","IpToCountryLoading","BarX",&tMenu->tFrontendInfo.iLoadingBarLeft,5);
	ReadInteger("data/frontend/frontend.cfg","IpToCountryLoading","BarY",&tMenu->tFrontendInfo.iLoadingBarTop,80);
	ReadInteger("data/frontend/frontend.cfg","IpToCountryLoading","LabelX",&tMenu->tFrontendInfo.iLoadingLabelLeft,5);
	ReadInteger("data/frontend/frontend.cfg","IpToCountryLoading","LabelY",&tMenu->tFrontendInfo.iLoadingLabelTop,60);
}

///////////////////
// Shutdown the menu
void Menu_Shutdown()
{
	Menu_Current_Shutdown();
	
	if(tMenu) {
		// The rest get free'd in the cache
		delete tMenu;
		tMenu = NULL;
	}

	// Shutdown the layouts
	//for (int i=0; i<LAYOUT_COUNT; i++)
	//	LayoutWidgets[i].Shutdown();

	Menu_SvrList_Shutdown();
}


///////////////////
// Start the menu
void Menu_Start()
{
	tMenu->bMenuRunning = true;

	if(!bDedicated) {
		if(!iSkipStart) {
			notes << "Loading main menu" << endl;
			tMenu->iMenuType = MNU_MAIN;
			Menu_MainInitialize();
		} else
			Menu_RedrawMouse(true);
	}

	iSkipStart = false;
	Menu_Loop();
}


///////////////////
// Set the skip start bit
void Menu_SetSkipStart(int s)
{
    iSkipStart = s;
}
	
void Menu_Frame() {
	HandlePendingCommands();
	
	sfx.think();
	
	if(bDedicated) {
		DedicatedControl::Get()->Menu_Frame();
		return;
	}

	if(!tMenu->bMenuRunning) return; // could be already quitted
	
	// Check if user pressed screenshot key
	if (tLX->cTakeScreenshot.isDownOnce())  {
		PushScreenshot("scrshots", "");
	}
	
	Menu_RedrawMouse(true);

#ifdef WITH_G15
	if (OLXG15)
		OLXG15->menuFrame();
#endif //WITH_G15
	
	switch(tMenu->iMenuType) {

		// Main
		case MNU_MAIN:
			Menu_MainFrame();
			break;

		// Local
		case MNU_LOCAL:
			Menu_LocalFrame();
			break;

		// News
		case MNU_NETWORK:
			Menu_NetFrame();
			break;

		// Player
		case MNU_PLAYER:
			Menu_PlayerFrame();
			break;

		// Map editor
		case MNU_MAPED:
			Menu_MapEdFrame(VideoPostProcessor::videoSurface(),true);
			break;

		// Options
		case MNU_OPTIONS:
			Menu_OptionsFrame();
			break;

		case MNU_GUISKIN:
			Menu_CGuiSkinFrame();
			break;
	}

	// DEBUG: show FPS
#ifdef DEBUG
	if(tLX->fDeltaTime != TimeDiff()) {
		Menu_redrawBufferRect(0, 0, 100, 20);
		tLX->cFont.Draw(VideoPostProcessor::videoSurface(), 0, 0, tLX->clWhite, "FPS: " + itoa((int)(1.0f/tLX->fDeltaTime.seconds())));
	}
#endif

	taskManager->renderTasksStatus(VideoPostProcessor::videoSurface());
	
	if (!tMenu->bForbidConsole)  {
		Con_Process(tLX->fDeltaTime);
		Con_Draw(VideoPostProcessor::videoSurface());
	}
	tMenu->bForbidConsole = false; // Reset it here, it might get recovered next frame

	// we need to clone the screen buffer because of the current way we are drawing the menu
	struct CopyScreenBuffer : Action {
		int handle() { VideoPostProcessor::cloneBuffer(); return 0; }
	};
	doVppOperation(new CopyScreenBuffer());
	
	// now do the actual flip&draw
	doVideoFrameInMainThread();
}


///////////////////
// Main menu loop
void Menu_Loop()
{	
	AbsTime menuStartTime = tLX->currentTime = GetTime();
		
	bool last_frame_was_because_of_an_event = false;
	last_frame_was_because_of_an_event = ProcessEvents();
	
	while(tMenu->bMenuRunning) {
		AbsTime oldtime = tLX->currentTime;

		Menu_Frame();
		if(!tMenu->bMenuRunning) break;
		CapFPS();
		SetCrashHandlerReturnPoint("Menu_Loop");
		
		if(last_frame_was_because_of_an_event || bDedicated) {
			// Use ProcessEvents() here to handle other processes in queue.
			// There aren't probably any but it has also the effect that
			// we run the loop another time after an event which is sometimes
			// because of the current code needed. Sometimes after an event,
			// some new menu elements got initialised but not drawn.
			last_frame_was_because_of_an_event = ProcessEvents();
		} else {
			last_frame_was_because_of_an_event = WaitForNextEvent();
		}
		
		ProcessIRC();

		tLX->currentTime = GetTime();
		tLX->fDeltaTime = tLX->currentTime - oldtime;
		tLX->fRealDeltaTime = tLX->fDeltaTime;
		
		// If we have run fine for >=5 seconds, it is probably safe & make sense
		// to restart the game in case of a crash.
		if(tLX->currentTime - menuStartTime >= TimeDiff(5.0f))
			CrashHandler::restartAfterCrash = true;			
	}
	
	// If we go out of the menu, it means the user has selected something.
	// This indicates that everything is fine, so we should restart in case of a crash.
	// Note that we will set this again to false later on in case the user quitted.
	CrashHandler::restartAfterCrash = true;
}


///////////////////
// Redraw the rectangle under the mouse (total means a total buffer redraw)
// TODO: rename this function (one would expect that it redraws the mouse)
void Menu_RedrawMouse(bool total)
{
	if(total) {
		SDL_BlitSurface(tMenu->bmpBuffer.get(),NULL,VideoPostProcessor::videoSurface(),NULL);
		return;
	}

	int hw = GetMaxCursorWidth() / 2 - 1;
	int hh = GetMaxCursorHeight() / 2 - 1;

	mouse_t *m = GetMouse();
	DrawImageAdv(VideoPostProcessor::videoSurface(),tMenu->bmpBuffer,
				m->X - hw - m->deltaX,
				m->Y - hh - m->deltaY,

				m->X - hw - m->deltaX,
				m->Y - hh - m->deltaY,
				GetMaxCursorWidth() * 2, GetMaxCursorHeight() * 2);
}


///////////////////
// Draw a sub title
void Menu_DrawSubTitle(SDL_Surface * bmpDest, int id)
{
	int x = VideoPostProcessor::videoSurface()->w/2;
	x -= tMenu->bmpSubTitles.get()->w/2;

	DrawImageAdv(bmpDest,tMenu->bmpSubTitles, 0, id*70, x,30, tMenu->bmpSubTitles.get()->w, 65);
}


///////////////////
// Draw a sub title advanced
void Menu_DrawSubTitleAdv(SDL_Surface * bmpDest, int id, int y)
{
	int x = VideoPostProcessor::videoSurface()->w/2;
	x -= tMenu->bmpSubTitles.get()->w/2;

	DrawImageAdv(bmpDest,tMenu->bmpSubTitles, 0, id*70, x,y, tMenu->bmpSubTitles.get()->w, 65);
}

	
////////////////
// Draws advanced box
void Menu_DrawBoxAdv(SDL_Surface * bmpDest, int x, int y, int x2, int y2, int border, Color LightColour, Color DarkColour, Color BgColour, uchar type)
{
	// First draw the background
	if (BgColour != tLX->clPink)
		DrawRectFill(bmpDest,x+border,y+border,x2-border+1,y2-border+1,BgColour);

	if (!border)
		return;

	int i;

	// Switch the light and dark colour when inset
	if (type == BX_INSET)  {
		Color tmp = LightColour;
		LightColour = DarkColour;
		DarkColour = tmp;
	}

	// Create gradient when needed
	int r_step,g_step,b_step;
	const Uint8 r1 = DarkColour.r, g1 = DarkColour.g, b1 = DarkColour.b;
	const Uint8 r2 = LightColour.r, g2 = LightColour.b, b2 = LightColour.b;

	if (type != BX_SOLID)  {
		r_step = (r2-r1)/border;
		g_step = (g2-g1)/border;
		b_step = (b2-b1)/border;
	}
	else {
		r_step = g_step = b_step = 0;
	}


	// Draw the box
	for (i=0;i<border;i++)
		DrawRect(bmpDest,x+i,y+i,x2-i,y2-i,Color(r1+r_step*i,g1+g_step*i,b1+b_step*i));
}


///////////////////
// Draw a box
void Menu_DrawBox(SDL_Surface * bmpDest, int x, int y, int x2, int y2)
{
	if(bmpDest == NULL) {
		errors << "Menu_DrawBox: bmpDest == NULL" << endl;
		return;
	}
	
	DrawRect( bmpDest,x+1, y+1, x2-1,y2-1, tLX->clBoxLight);
    //DrawRect( bmpDest,x+2, y+2, x2-2,y2-2, tLX->clBoxDark);
	DrawHLine(bmpDest,x+2, x2-1,y,  tLX->clBoxDark);
	DrawHLine(bmpDest,x+2, x2-1,y2, tLX->clBoxDark);

	DrawVLine(bmpDest,y+2, y2-1,x,  tLX->clBoxDark);
	DrawVLine(bmpDest,y+2, y2-1,x2, tLX->clBoxDark);

    Uint32 dark = tLX->clBoxDark.get(bmpDest->format);

	LOCK_OR_QUIT(bmpDest);
	PutPixel( bmpDest,x+1, y+1,     dark);
	PutPixel( bmpDest,x2-1,y+1,     dark);
	PutPixel( bmpDest,x+1, y2-1,    dark);
	PutPixel( bmpDest,x2-1,y2-1,    dark);
	UnlockSurface(bmpDest);
}


///////////////////
// Draw an inset box
void Menu_DrawBoxInset(SDL_Surface * bmpDest, int x, int y, int x2, int y2)
{
	if(bmpDest == NULL) {
		errors << "Menu_DrawBoxInset: bmpDest is NULL" << endl;
		return;
	}
	
	// Clipping
	if (x < 0) { x2 += x; x = 0; }
	if (x2 >= bmpDest->w) { x2 = bmpDest->w - 1; }
	if (y < 0) { y2 += y; y = 0; }
	if (y2 >= bmpDest->h) { y2 = bmpDest->h - 1; }

	DrawRect( bmpDest,x+1, y+1, x2-1,y2-1, tLX->clBoxDark);
	DrawHLine(bmpDest,x+2, x2-1,y,  tLX->clBoxLight);
	DrawHLine(bmpDest,x+2, x2-1,y2, tLX->clBoxLight);

	DrawVLine(bmpDest,y+2, y2-1,x,  tLX->clBoxLight);
	DrawVLine(bmpDest,y+2, y2-1,x2, tLX->clBoxLight);

	Uint32 light = tLX->clBoxLight.get(bmpDest->format);

	LOCK_OR_QUIT(bmpDest);
	if(PointInRect(x+1,y+1,bmpDest->clip_rect)) PutPixel( bmpDest,x+1, y+1,     light);
	if(PointInRect(x2-1,y+1,bmpDest->clip_rect)) PutPixel( bmpDest,x2-1,y+1,     light);
	if(PointInRect(x+1,y2-1,bmpDest->clip_rect)) PutPixel( bmpDest,x+1, y2-1,    light);
	if(PointInRect(x2-1,y2-1,bmpDest->clip_rect)) PutPixel( bmpDest,x2-1,y2-1,    light);
	UnlockSurface(bmpDest);
}


///////////////////
// Draw a windows style button
void Menu_DrawWinButton(SDL_Surface * bmpDest, int x, int y, int w, int h, bool down)
{
    DrawRectFill(bmpDest, x,y, x+w, y+h, tLX->clWinBtnBody);
    const Color dark = tLX->clWinBtnDark;
    const Color light = tLX->clWinBtnLight;
    if(down) {
        DrawHLine(bmpDest, x, x+w, y, dark);
        DrawHLine(bmpDest, x, x+w, y+h, light);
        DrawVLine(bmpDest, y, y+h, x, dark);
        DrawVLine(bmpDest, y, y+h, x+w, light);
    } else {
        DrawHLine(bmpDest, x, x+w, y, light);
        DrawHLine(bmpDest, x, x+w, y+h, dark);
        DrawVLine(bmpDest, y, y+h, x, light);
        DrawVLine(bmpDest, y, y+h, x+w, dark);
    }
}


///////////////////
// Show a message box
MessageBoxReturnType Menu_MessageBox(const std::string& sTitle, const std::string& sText, MessageBoxType type)
{
	if(bDedicated) {
		hints << "Menu_MessageBox: " << sTitle << ": " << sText << endl;
		switch(type) {
			case LMB_OK: return MBR_OK;
			case LMB_YESNO:
				hints << "Dedicated server is positive and says YES." << endl;
				return MBR_YES;
		}
		return MBR_OK;
	}

	MessageBoxReturnType ret = MBR_INVALID;
	gui_event_t *ev = NULL;

	SetGameCursor(CURSOR_ARROW);

	int minw = 350;
	int maxw = 500;
	int x = 160;
	int y = 170;
	int w = minw; // the whole box
	int h = 140; // including caption and button, the whole box

	// Adjust the width
	int longest_line = w;
	std::vector<std::string> lines = explode(sText, "\n");
	std::vector<std::string>::const_iterator it;
	for (it=lines.begin(); it!=lines.end(); it++)  {
		int tw = tLX->cFont.GetWidth(*it);
		if (tw > longest_line)
			longest_line = tw;
	}
	w = CLAMP(longest_line + 40, minw, maxw);
	x = (VideoPostProcessor::get()->screenWidth() - w) / 2;

	// Handle multiline messages
	lines = splitstring(sText, (size_t)-1, w - 2, tLX->cFont);
	
	const int line_hspace = 2;
	const int button_h = 24;
	const int caption_h = 25;

	if( (tLX->cFont.GetHeight() + line_hspace) * (int)lines.size() + button_h + caption_h + 2 > h ) {
		// TODO: hardcoded screen height (480)
		h = (int)MIN( (tLX->cFont.GetHeight() + line_hspace) * lines.size() + 90, (size_t)478);		
		y = 240-h/2;
	}

	int cx = x+w/2;
	int cy = y + caption_h;
	if(lines.size() > 0) {
		cy += (h - button_h - caption_h) / 2;
		cy -= ((int)(lines.size() - 1) * (tLX->cFont.GetHeight() + line_hspace)) / 2;
		cy -= tLX->cFont.GetHeight() / 2;
	}

	//
	// Setup the gui
	//
	CGuiLayout msgbox;

	msgbox.Initialize();

	if(type == LMB_OK)
		msgbox.Add( new CButton(BUT_OK,tMenu->bmpButtons), 0, cx-20,y+h-button_h, 40,15);
	else if(type == LMB_YESNO) {
		msgbox.Add( new CButton(BUT_YES,tMenu->bmpButtons), 1, x+15,y+h-button_h, 35,15);
		msgbox.Add( new CButton(BUT_NO,tMenu->bmpButtons),  2, x+w-35,y+h-button_h, 30,15);
	}

	// Store the old buffer into a temp buffer to keep it
	SDL_BlitSurface(tMenu->bmpBuffer.get(), NULL, tMenu->bmpMsgBuffer.get(), NULL);


	// Draw to the buffer
	//DrawImage(tMenu->bmpBuffer, shadow, 177,167);
	Menu_DrawBox(tMenu->bmpBuffer.get(), x, y, x+w, y+h);
	DrawRectFill(tMenu->bmpBuffer.get(), x+2,y+2, x+w-1,y+h-1,tLX->clDialogBackground);
	DrawRectFill(tMenu->bmpBuffer.get(), x+2,y+2, x+w-1,y+caption_h,tLX->clDialogCaption);

	tLX->cFont.DrawCentre(tMenu->bmpBuffer.get(), cx, y+5, tLX->clNormalLabel,sTitle);
	for (it=lines.begin(); it!=lines.end(); it++)  {
		cx = x+w/2;//-(tLX->cFont.GetWidth(lines[i])+30)/2;
		tLX->cFont.DrawCentre(tMenu->bmpBuffer.get(), cx, cy, tLX->clNormalLabel, *it);
		cy += tLX->cFont.GetHeight()+line_hspace;
	}

	Menu_RedrawMouse(true);


	ProcessEvents();

	
	// TODO: make this event-based (don't check GetKeyboard() directly)
	while(true) {
		Menu_RedrawMouse(true);

		SetGameCursor(CURSOR_ARROW);

		DrawImageAdv(VideoPostProcessor::videoSurface(),tMenu->bmpBuffer, x,y, x,y, w, h);

		// Process the gui
		ev = msgbox.Process();
		msgbox.Draw(VideoPostProcessor::videoSurface());

		if(ev) {

			if(ev->cWidget->getType() == wid_Button)
				SetGameCursor(CURSOR_HAND);
			if(ev->cWidget->getType() == wid_Textbox)
				SetGameCursor(CURSOR_TEXT);

			if(ev->iEventMsg == BTN_CLICKED) {
				switch(ev->iControlID) {

					// OK
					case 0:
						ret = MBR_OK;
						break;

					// Yes
					case 1:
						ret = MBR_YES;
						break;

					// No
					case 2:
						ret = MBR_NO;
						break;
				}
			}
		}

		// Handle the Enter key
		if (WasKeyboardEventHappening(SDLK_RETURN) || WasKeyboardEventHappening(SDLK_KP_ENTER))
		{
			if (type == LMB_YESNO)  {
				ret = MBR_YES;
				break;
			}
			else  {
				ret = MBR_OK;
				break;
			}
		}

		if(!WasKeyboardEventHappening(SDLK_ESCAPE) && !tLX->bQuitGame && ret == MBR_INVALID) {
			DrawCursor(VideoPostProcessor::videoSurface());
			doVideoFrameInMainThread();
			CapFPS();
			tLX->currentTime = GetTime(); // we need this for CapFPS()
			WaitForNextEvent();
		} else
			break;
	}

	SetGameCursor(CURSOR_ARROW);
	msgbox.Shutdown();

	// Restore the old buffer
	SDL_BlitSurface(tMenu->bmpMsgBuffer.get(), NULL, tMenu->bmpBuffer.get(), NULL);
	//Menu_RedrawMouse(true);
	//doVideoFrameInMainThread();

	return ret;
}

///////////////////
// Add all the default widgets
void Menu_AddDefaultWidgets()
{
// 34 layouts total

// L_MAINMENU: 6 widgets
    LayoutWidgets[L_MAINMENU].Add("LocalPlay");
    LayoutWidgets[L_MAINMENU].Add("NetPlay");
    LayoutWidgets[L_MAINMENU].Add("PlayerProfiles");
    LayoutWidgets[L_MAINMENU].Add("LevelEditor");
    LayoutWidgets[L_MAINMENU].Add("Options");
    LayoutWidgets[L_MAINMENU].Add("Quit");

// L_LOCALPLAY: 9 widgets
    LayoutWidgets[L_LOCALPLAY].Add("Back");
    LayoutWidgets[L_LOCALPLAY].Add("Start");
    LayoutWidgets[L_LOCALPLAY].Add("Playing");
    LayoutWidgets[L_LOCALPLAY].Add("PlayerList");
    LayoutWidgets[L_LOCALPLAY].Add("LevelList");
    LayoutWidgets[L_LOCALPLAY].Add("Gametype");
    LayoutWidgets[L_LOCALPLAY].Add("ModName");
    LayoutWidgets[L_LOCALPLAY].Add("GameSettings");
    LayoutWidgets[L_LOCALPLAY].Add("WeaponOptions");

// L_GAMESETTINGS: 9 widgets
    LayoutWidgets[L_GAMESETTINGS].Add("gs_Ok");
    LayoutWidgets[L_GAMESETTINGS].Add("gs_Default");
    LayoutWidgets[L_GAMESETTINGS].Add("Lives");
    LayoutWidgets[L_GAMESETTINGS].Add("MaxKills");
    LayoutWidgets[L_GAMESETTINGS].Add("LoadingTime");
    LayoutWidgets[L_GAMESETTINGS].Add("LoadingTimeLabel");
    LayoutWidgets[L_GAMESETTINGS].Add("Bonuses");
    LayoutWidgets[L_GAMESETTINGS].Add("ShowBonusNames");
    LayoutWidgets[L_GAMESETTINGS].Add("MaxTime");

// L_WEAPONOPTIONS: 8 widgets
    LayoutWidgets[L_WEAPONOPTIONS].Add("wr_Ok");
    LayoutWidgets[L_WEAPONOPTIONS].Add("wr_Scroll");
    LayoutWidgets[L_WEAPONOPTIONS].Add("wr_Reset");
    LayoutWidgets[L_WEAPONOPTIONS].Add("wr_ListBox");
    LayoutWidgets[L_WEAPONOPTIONS].Add("wr_Cancel");
    LayoutWidgets[L_WEAPONOPTIONS].Add("wr_Random");
    LayoutWidgets[L_WEAPONOPTIONS].Add("wr_Load");
    LayoutWidgets[L_WEAPONOPTIONS].Add("wr_Save");

// L_LOADWEAPONS: 4 widgets
    LayoutWidgets[L_LOADWEAPONS].Add("wp_Cancel");
    LayoutWidgets[L_LOADWEAPONS].Add("wp_Ok");
    LayoutWidgets[L_LOADWEAPONS].Add("wp_PresetList");
    LayoutWidgets[L_LOADWEAPONS].Add("wp_PresetName");

// L_SAVEWEAPONS: 4 widgets
    LayoutWidgets[L_SAVEWEAPONS].Add("wp_Cancel");
    LayoutWidgets[L_SAVEWEAPONS].Add("wp_Ok");
    LayoutWidgets[L_SAVEWEAPONS].Add("wp_PresetList");
    LayoutWidgets[L_SAVEWEAPONS].Add("wp_PresetName");

// L_NET: 4 widgets
    LayoutWidgets[L_NET].Add("InternetTab");
    LayoutWidgets[L_NET].Add("LANTab");
    LayoutWidgets[L_NET].Add("HostTab");
    LayoutWidgets[L_NET].Add("FavouritesTab");

// L_NETINTERNET: 8 widgets
    LayoutWidgets[L_NETINTERNET].Add("Join");
    LayoutWidgets[L_NETINTERNET].Add("ServerList");
    LayoutWidgets[L_NETINTERNET].Add("Refresh");
    LayoutWidgets[L_NETINTERNET].Add("UpdateList");
    LayoutWidgets[L_NETINTERNET].Add("AddServer");
    LayoutWidgets[L_NETINTERNET].Add("Back");
    LayoutWidgets[L_NETINTERNET].Add("PopupMenu");
    LayoutWidgets[L_NETINTERNET].Add("PlayerSelection");

// L_INTERNETDETAILS: 1 widgets
    LayoutWidgets[L_INTERNETDETAILS].Add("id_Ok");

// L_ADDSERVER: 3 widgets
    LayoutWidgets[L_ADDSERVER].Add("na_Cancel");
    LayoutWidgets[L_ADDSERVER].Add("na_Add");
    LayoutWidgets[L_ADDSERVER].Add("na_Address");

// L_NETLAN: 6 widgets
    LayoutWidgets[L_NETLAN].Add("Join");
    LayoutWidgets[L_NETLAN].Add("ServerList");
    LayoutWidgets[L_NETLAN].Add("Refresh");
    LayoutWidgets[L_NETLAN].Add("Back");
    LayoutWidgets[L_NETLAN].Add("PopupMenu");
    LayoutWidgets[L_NETLAN].Add("PlayerSelection");

// L_LANDETAILS: 1 widgets
    LayoutWidgets[L_LANDETAILS].Add("ld_Ok");

// L_NETHOST: 10 widgets
    LayoutWidgets[L_NETHOST].Add("Back");
    LayoutWidgets[L_NETHOST].Add("Ok");
    LayoutWidgets[L_NETHOST].Add("PlayerList");
    LayoutWidgets[L_NETHOST].Add("Playing");
    LayoutWidgets[L_NETHOST].Add("Servername");
    LayoutWidgets[L_NETHOST].Add("MaxPlayers");
    LayoutWidgets[L_NETHOST].Add("Register");
    LayoutWidgets[L_NETHOST].Add("Password");
    LayoutWidgets[L_NETHOST].Add("WelcomeMessage");
    LayoutWidgets[L_NETHOST].Add("AllowWantsJoin");

// L_NETFAVOURITES: 7 widgets
    LayoutWidgets[L_NETFAVOURITES].Add("Join");
    LayoutWidgets[L_NETFAVOURITES].Add("ServerList");
    LayoutWidgets[L_NETFAVOURITES].Add("Refresh");
    LayoutWidgets[L_NETFAVOURITES].Add("Add");
    LayoutWidgets[L_NETFAVOURITES].Add("Back");
    LayoutWidgets[L_NETFAVOURITES].Add("PopupMenu");
    LayoutWidgets[L_NETFAVOURITES].Add("PlayerSelection");

// L_FAVOURITESDETAILS: 1 widgets
    LayoutWidgets[L_FAVOURITESDETAILS].Add("fd_Ok");

// L_RENAMESERVER: 3 widgets
    LayoutWidgets[L_RENAMESERVER].Add("rs_Cancel");
    LayoutWidgets[L_RENAMESERVER].Add("rs_Ok");
    LayoutWidgets[L_RENAMESERVER].Add("rs_NewName");

// L_ADDFAVOURITE: 4 widgets
    LayoutWidgets[L_ADDFAVOURITE].Add("fa_Cancel");
    LayoutWidgets[L_ADDFAVOURITE].Add("fa_Add");
    LayoutWidgets[L_ADDFAVOURITE].Add("fa_Address");
    LayoutWidgets[L_ADDFAVOURITE].Add("fa_Name");

// L_CONNECTING: 1 widgets
    LayoutWidgets[L_CONNECTING].Add("Cancel");

// L_NETJOINLOBBY: 4 widgets
    LayoutWidgets[L_NETJOINLOBBY].Add("Back2");
    LayoutWidgets[L_NETJOINLOBBY].Add("Ready");
    LayoutWidgets[L_NETJOINLOBBY].Add("ChatText");
    LayoutWidgets[L_NETJOINLOBBY].Add("ChatList");

// L_NETHOSTLOBBY: 14 widgets
    LayoutWidgets[L_NETHOSTLOBBY].Add("Back2");
    LayoutWidgets[L_NETHOSTLOBBY].Add("Start");
    LayoutWidgets[L_NETHOSTLOBBY].Add("ChatText");
    LayoutWidgets[L_NETHOSTLOBBY].Add("ChatList");
    LayoutWidgets[L_NETHOSTLOBBY].Add("LevelList");
    LayoutWidgets[L_NETHOSTLOBBY].Add("Lives");
    LayoutWidgets[L_NETHOSTLOBBY].Add("MaxKills");
    LayoutWidgets[L_NETHOSTLOBBY].Add("ModName");
    LayoutWidgets[L_NETHOSTLOBBY].Add("Gametype");
    LayoutWidgets[L_NETHOSTLOBBY].Add("GameSettings");
    LayoutWidgets[L_NETHOSTLOBBY].Add("WeaponOptions");
    LayoutWidgets[L_NETHOSTLOBBY].Add("PopupMenu");
    LayoutWidgets[L_NETHOSTLOBBY].Add("Banned");
    LayoutWidgets[L_NETHOSTLOBBY].Add("ServerSettings");

// L_SERVERSETTINGS: 7 widgets
    LayoutWidgets[L_SERVERSETTINGS].Add("ss_Ok");
    LayoutWidgets[L_SERVERSETTINGS].Add("ss_Cancel");
    LayoutWidgets[L_SERVERSETTINGS].Add("ss_AllowOnlyList");
    LayoutWidgets[L_SERVERSETTINGS].Add("ss_WelcomeMessage");
    LayoutWidgets[L_SERVERSETTINGS].Add("ss_ServerName");
    LayoutWidgets[L_SERVERSETTINGS].Add("ss_AllowWantsJoin");
    LayoutWidgets[L_SERVERSETTINGS].Add("ss_MaxPlayers");

// L_BANLIST: 4 widgets
    LayoutWidgets[L_BANLIST].Add("bl_Close");
    LayoutWidgets[L_BANLIST].Add("bl_Clear");
    LayoutWidgets[L_BANLIST].Add("bl_Unban");
    LayoutWidgets[L_BANLIST].Add("bl_ListBox");

// L_PLAYERPROFILES: 2 widgets
    LayoutWidgets[L_PLAYERPROFILES].Add("NewPlayerTab");
    LayoutWidgets[L_PLAYERPROFILES].Add("ViewPlayersTab");

// L_CREATEPLAYER: 12 widgets
    LayoutWidgets[L_CREATEPLAYER].Add("np_Back");
    LayoutWidgets[L_CREATEPLAYER].Add("np_Create");
    LayoutWidgets[L_CREATEPLAYER].Add("np_Name");
    LayoutWidgets[L_CREATEPLAYER].Add("np_Red");
    LayoutWidgets[L_CREATEPLAYER].Add("np_Blue");
    LayoutWidgets[L_CREATEPLAYER].Add("np_Green");
    LayoutWidgets[L_CREATEPLAYER].Add("np_Type");
    LayoutWidgets[L_CREATEPLAYER].Add("np_AIDiffLbl");
    LayoutWidgets[L_CREATEPLAYER].Add("np_AIDiff");
    LayoutWidgets[L_CREATEPLAYER].Add("np_PlySkin");
    LayoutWidgets[L_CREATEPLAYER].Add("np_Username");
    LayoutWidgets[L_CREATEPLAYER].Add("np_Password");

// L_VIEWPLAYERS: 12 widgets
    LayoutWidgets[L_VIEWPLAYERS].Add("vp_Back");
    LayoutWidgets[L_VIEWPLAYERS].Add("vp_Name");
    LayoutWidgets[L_VIEWPLAYERS].Add("vp_Red");
    LayoutWidgets[L_VIEWPLAYERS].Add("vp_Blue");
    LayoutWidgets[L_VIEWPLAYERS].Add("vp_Green");
    LayoutWidgets[L_VIEWPLAYERS].Add("vp_Players");
    LayoutWidgets[L_VIEWPLAYERS].Add("vp_Delete");
    LayoutWidgets[L_VIEWPLAYERS].Add("vp_Apply");
    LayoutWidgets[L_VIEWPLAYERS].Add("vp_Type");
    LayoutWidgets[L_VIEWPLAYERS].Add("vp_AIDiffLbl");
    LayoutWidgets[L_VIEWPLAYERS].Add("vp_AIDiff");
    LayoutWidgets[L_VIEWPLAYERS].Add("vp_PlySkin");

// L_LEVELEDITOR: 5 widgets
    LayoutWidgets[L_LEVELEDITOR].Add("map_new");
    LayoutWidgets[L_LEVELEDITOR].Add("map_random");
    LayoutWidgets[L_LEVELEDITOR].Add("map_load");
    LayoutWidgets[L_LEVELEDITOR].Add("map_save");
    LayoutWidgets[L_LEVELEDITOR].Add("map_quit");

// L_NEWDIALOG: 5 widgets
    LayoutWidgets[L_NEWDIALOG].Add("mn_Cancel");
    LayoutWidgets[L_NEWDIALOG].Add("mn_Ok");
    LayoutWidgets[L_NEWDIALOG].Add("mn_Width");
    LayoutWidgets[L_NEWDIALOG].Add("mn_Height");
    LayoutWidgets[L_NEWDIALOG].Add("mn_Scheme");

// L_SAVELOADLEVEL: 4 widgets
    LayoutWidgets[L_SAVELOADLEVEL].Add("sl_Cancel");
    LayoutWidgets[L_SAVELOADLEVEL].Add("sl_Ok");
    LayoutWidgets[L_SAVELOADLEVEL].Add("sl_FileList");
    LayoutWidgets[L_SAVELOADLEVEL].Add("sl_FileName");

// L_OPTIONS: 3 widgets
    LayoutWidgets[L_OPTIONS].Add("ControlsTab");
    LayoutWidgets[L_OPTIONS].Add("GameTab");
    LayoutWidgets[L_OPTIONS].Add("SystemTab");

// L_OPTIONSCONTROLS: 23 widgets
    LayoutWidgets[L_OPTIONSCONTROLS].Add("Ply1_Up");
    LayoutWidgets[L_OPTIONSCONTROLS].Add("Ply1_Down");
    LayoutWidgets[L_OPTIONSCONTROLS].Add("Ply1_Left");
    LayoutWidgets[L_OPTIONSCONTROLS].Add("Ply1_Right");
    LayoutWidgets[L_OPTIONSCONTROLS].Add("Ply1_Shoot");
    LayoutWidgets[L_OPTIONSCONTROLS].Add("Ply1_Jump");
    LayoutWidgets[L_OPTIONSCONTROLS].Add("Ply1_Selweapon");
    LayoutWidgets[L_OPTIONSCONTROLS].Add("Ply1_Rope");
    LayoutWidgets[L_OPTIONSCONTROLS].Add("Ply2_Up");
    LayoutWidgets[L_OPTIONSCONTROLS].Add("Ply2_Down");
    LayoutWidgets[L_OPTIONSCONTROLS].Add("Ply2_Left");
    LayoutWidgets[L_OPTIONSCONTROLS].Add("Ply2_Right");
    LayoutWidgets[L_OPTIONSCONTROLS].Add("Ply2_Shoot");
    LayoutWidgets[L_OPTIONSCONTROLS].Add("Ply2_Jump");
    LayoutWidgets[L_OPTIONSCONTROLS].Add("Ply2_Selweapon");
    LayoutWidgets[L_OPTIONSCONTROLS].Add("Ply2_Rope");
    LayoutWidgets[L_OPTIONSCONTROLS].Add("Gen_Chat");
    LayoutWidgets[L_OPTIONSCONTROLS].Add("Gen_Score");
    LayoutWidgets[L_OPTIONSCONTROLS].Add("Gen_Health");
    LayoutWidgets[L_OPTIONSCONTROLS].Add("Gen_CurSettings");
    LayoutWidgets[L_OPTIONSCONTROLS].Add("Gen_TakeScreenshot");
    LayoutWidgets[L_OPTIONSCONTROLS].Add("Gen_ViewportManager");
    LayoutWidgets[L_OPTIONSCONTROLS].Add("Gen_SwitchMode");

// L_OPTIONSGAME: 9 widgets
    LayoutWidgets[L_OPTIONSGAME].Add("BloodAmount");
    LayoutWidgets[L_OPTIONSGAME].Add("Shadows");
    LayoutWidgets[L_OPTIONSGAME].Add("Particles");
    LayoutWidgets[L_OPTIONSGAME].Add("OldSkoolRope");
    LayoutWidgets[L_OPTIONSGAME].Add("AIDifficulty");
    LayoutWidgets[L_OPTIONSGAME].Add("ShowWormHealth");
    LayoutWidgets[L_OPTIONSGAME].Add("ColorizeNicks");
    LayoutWidgets[L_OPTIONSGAME].Add("AutoTyping");
    LayoutWidgets[L_OPTIONSGAME].Add("ScreenshotFormat");

// L_OPTIONSSYSTEM: 12 widgets
    LayoutWidgets[L_OPTIONSSYSTEM].Add("Back");
    LayoutWidgets[L_OPTIONSSYSTEM].Add("Fullscreen");
    LayoutWidgets[L_OPTIONSSYSTEM].Add("OpenGL");
    LayoutWidgets[L_OPTIONSSYSTEM].Add("SoundOn");
    LayoutWidgets[L_OPTIONSSYSTEM].Add("SoundVolume");
    LayoutWidgets[L_OPTIONSSYSTEM].Add("NetworkPort");
    LayoutWidgets[L_OPTIONSSYSTEM].Add("NetworkSpeed");
    LayoutWidgets[L_OPTIONSSYSTEM].Add("ShowFPS");
    LayoutWidgets[L_OPTIONSSYSTEM].Add("ShowPing");
    LayoutWidgets[L_OPTIONSSYSTEM].Add("Filtered");
    LayoutWidgets[L_OPTIONSSYSTEM].Add("LogConvos");
    LayoutWidgets[L_OPTIONSSYSTEM].Add("Apply");

// L_MESSAGEBOXOK: 1 widgets
    LayoutWidgets[L_MESSAGEBOXOK].Add("mb_Ok");

// L_MESSAGEBOXYESNO: 2 widgets
    LayoutWidgets[L_MESSAGEBOXYESNO].Add("mb_Yes");
    LayoutWidgets[L_MESSAGEBOXYESNO].Add("mb_No");
}


	// Load the level list
	struct LevelComboFiller {
		CCombobox* cmb;
		LevelComboFiller(CCombobox* c) : cmb(c) {}
		bool operator() (const std::string& filename) {
			LevelInfo info = infoForLevel(filename, true);
			if(info.valid)
				cmb->addItem(info.path, "[" + info.typeShort + "] " + info.name);

			return true;
		}
	};


///////////////////
// Fill a listbox with the levels
void Menu_FillLevelList(CCombobox *cmb, int random)
{
	cmb->clear();
	cmb->setSorted(SORT_ASC);
	cmb->setUnique(true);

	LevelComboFiller filler(cmb);
	FindFiles(filler, "levels", false);

	// Disable sorting and add the random level at the beginning
	cmb->setSorted(SORT_NONE);
	//if(random) // If random is true, we add the 'random' level to the list
	//	cmb->addItem(0, "_random_", "- Random level -");

	cmb->setCurSIndexItem(gameSettings[FT_Map].as<LevelInfo>()->path);
}


///////////////////
// Redraw a section from the buffer to the screen
void Menu_redrawBufferRect(int x, int y, int w, int h)
{
    DrawImageAdv(VideoPostProcessor::videoSurface(), tMenu->bmpBuffer, x,y, x,y, w,h);
}



void Menu_DisableNetEvents()
{
	for (size_t i = 0; i < sizeof(tMenu->tSocket)/sizeof(tMenu->tSocket[0]); ++i)
		if(i != SCK_FOO)
			tMenu->tSocket[i]->setWithEvents(false);
}

void Menu_EnableNetEvents()
{
	for (size_t i = 0; i < sizeof(tMenu->tSocket)/sizeof(tMenu->tSocket[0]); ++i)
		if(i != SCK_FOO)
			tMenu->tSocket[i]->setWithEvents(true);
}


/*
============================

	Server list functions

============================
*/


std::list<server_t> psServerList;

// Maximum number of pings/queries before we ignore the server
static const int	MaxPings = 4;
static const int	MaxQueries = MAX_QUERIES;



///////////////////
// Clear the server list
void Menu_SvrList_Clear()
{
	Menu_SvrList_Shutdown();
}


///////////////////
// Clear any servers automatically added
void Menu_SvrList_ClearAuto()
{
    for(std::list<server_t>::iterator it = psServerList.begin(); it != psServerList.end(); it++)
    {
        if(!it->bManual) 
        {
        	psServerList.erase(it);
			if (psServerList.empty())
				return;
        	it = psServerList.begin();
        }
    }
}


///////////////////
// Shutdown the server list
void Menu_SvrList_Shutdown()
{
	psServerList.clear();
}


	
static void SendBroadcastPing(int port) {
	// Broadcast a ping on the LAN
	CBytestream bs;
	bs.writeInt(-1,4);
	bs.writeString("lx::ping");
	
	NetworkAddr a;
	StringToNetAddr("255.255.255.255", a);
	SetNetAddrPort(a,  port);
	tMenu->tSocket[SCK_LAN]->setRemoteAddress(a);
	
	// Send the ping
	bs.Send(tMenu->tSocket[SCK_LAN]);
}
	
///////////////////
// Send a ping out to the LAN (LAN menu)
void Menu_SvrList_PingLAN()
{
	SendBroadcastPing(LX_PORT);
	if(tLXOptions->iNetworkPort != LX_PORT)
		SendBroadcastPing(tLXOptions->iNetworkPort); // try also our own port
}


///////////////////
// Ping a server
void Menu_SvrList_PingServer(server_t *svr)
{
	// If not available, probably the network is not connected right now.
	if(!IsNetAddrAvailable(svr->sAddress)) return;
	
	if( svr->ports.size() == 0 )
	{
		errors << "svr->ports.size() == 0 at " << FILELINE << endl;
		return;
	}
		
	NetworkAddr addr = svr->sAddress;
	//hints << "Pinging server " << tmp << " real addr " << svr->szAddress << " name " << svr->szName << endl;
	svr->lastPingedPort++;
	if( svr->lastPingedPort >= (int)svr->ports.size() || svr->lastPingedPort < 0 )
		svr->lastPingedPort = 0;
	SetNetAddrPort(addr, svr->ports[svr->lastPingedPort].first);
	
	tMenu->tSocket[SCK_NET]->setRemoteAddress(addr);
	
	CBytestream bs;
	bs.writeInt(-1,4);
	bs.writeString("lx::ping");
	bs.Send(tMenu->tSocket[SCK_NET]);

	svr->bProcessing = true;
	svr->nPings++;
	svr->fLastPing = tLX->currentTime;
}

///////////////////
// Send Wants To Join message
void Menu_SvrList_WantsJoin(const std::string& Nick, server_t *svr)
{
	tMenu->tSocket[SCK_NET]->setRemoteAddress(svr->sAddress);

	CBytestream bs;
	bs.writeInt(-1,4);

	if( svr->bBehindNat )
	{
		NetworkAddr masterserverAddr;
		SetNetAddrValid(masterserverAddr, false);
		if( ! GetNetAddrFromNameAsync( Menu_SvrList_GetUdpMasterserverForServer(svr->szAddress), masterserverAddr ) )
			return;

		for( int count = 0; !IsNetAddrValid(masterserverAddr) && count < 5; count++ )
			SDL_Delay(20);

		if( !IsNetAddrValid(masterserverAddr) )
			return;

		tMenu->tSocket[SCK_NET]->setRemoteAddress(masterserverAddr);
		bs.writeString("lx::traverse");
		bs.writeString(svr->szAddress);
	}

	bs.writeString("lx::wantsjoin");
	bs.writeString(RemoveSpecialChars(Nick));
	bs.Send(tMenu->tSocket[SCK_NET]);
}

///////////////////
// Get server info
void Menu_SvrList_GetServerInfo(server_t *svr)
{
	// Send a getinfo request
	tMenu->tSocket[SCK_NET]->setRemoteAddress(svr->sAddress);

	CBytestream bs;
	bs.writeInt(-1,4);

	if( svr->bBehindNat )
	{
		NetworkAddr masterserverAddr;
		SetNetAddrValid(masterserverAddr, false);
		if( ! GetNetAddrFromNameAsync( Menu_SvrList_GetUdpMasterserverForServer(svr->szAddress), masterserverAddr ) )
			return;

		for( int count = 0; !IsNetAddrValid(masterserverAddr) && count < 5; count++ )
			SDL_Delay(20);

		if( !IsNetAddrValid(masterserverAddr) )
			return;

		tMenu->tSocket[SCK_NET]->setRemoteAddress(masterserverAddr);
		bs.writeString("lx::traverse");
		bs.writeString(svr->szAddress);
	}
	
	bs.writeString("lx::getinfo");
	bs.Send(tMenu->tSocket[SCK_NET]);
}

///////////////////
// Query a server
void Menu_SvrList_QueryServer(server_t *svr)
{
	tMenu->tSocket[SCK_NET]->setRemoteAddress(svr->sAddress);

	CBytestream bs;
	bs.writeInt(-1,4);
	bs.writeString("lx::query");
    bs.writeByte(svr->nQueries);
	bs.Send(tMenu->tSocket[SCK_NET]);
    svr->fQueryTimes[svr->nQueries] = tLX->currentTime;

	svr->bProcessing = true;
	svr->nQueries++;
	svr->fLastQuery = tLX->currentTime;
}


///////////////////
// Refresh the server list (Internet menu)
void Menu_SvrList_RefreshList()
{
	// Set all the servers to be pinged
	for(std::list<server_t>::iterator it = psServerList.begin(); it != psServerList.end(); it++) 
	{
		if( ! it->bBehindNat )
			Menu_SvrList_RefreshServer(&(*it), false);
	}

	// Update the GUI
	Timer("Menu_SvrList_RefreshList ping waiter", null, NULL, PingWait, true).startHeadless();

	//Menu_SvrList_UpdateUDPList(); // It adds duplicate server entries
}


///////////////////
// Refresh a single server
void Menu_SvrList_RefreshServer(server_t *s, bool updategui)
{
	if (!tLX)
		return;

    s->bProcessing = true;
	s->bgotPong = false;
	s->bgotQuery = false;
	s->bIgnore = false;
	s->fLastPing = AbsTime();
	s->fLastQuery = AbsTime();
	s->nPings = 0;
	s->fInitTime = tLX->currentTime;
	s->nQueries = 0;
	s->nPing = 0;
	s->bAddrReady = false;
	s->lastPingedPort = 0;


	if(!StringToNetAddr(s->szAddress, s->sAddress)) {
		hints << "Menu_SvrList_RefreshServer(): cannot parse server addr " << s->szAddress << endl;
		int oldPort = LX_PORT; //GetNetAddrPort(s->sAddress);
		s->sAddress = NetworkAddr(); // assign new addr (needed to avoid problems with possible other still running thread)
		SetNetAddrPort(s->sAddress, oldPort);

		SetNetAddrValid(s->sAddress, false);
		size_t f = s->szAddress.find(":");
		GetNetAddrFromNameAsync(s->szAddress.substr(0, f), s->sAddress);
	} else {
		s->bAddrReady = true;
		size_t f = s->szAddress.find(":");
		if(f != std::string::npos) {
			SetNetAddrPort(s->sAddress, from_string<int>(s->szAddress.substr(f + 1)));
		} else
			SetNetAddrPort(s->sAddress, LX_PORT);

		if (updategui)
			Timer("Menu_SvrList_RefreshServer ping waiter", null, NULL, PingWait, true).startHeadless();
	}

	if( s->ports.size() == 0 )
	{
		s->ports.push_back(std::make_pair((int)GetNetAddrPort(s->sAddress), -1));
	}
}


///////////////////
// Add a server onto the list (for list and manually)
server_t *Menu_SvrList_AddServer(const std::string& address, bool bManual, const std::string & name, int udpMasterserverIndex)
{
    // Check if the server is already in the list
    // If it is, don't bother adding it
	NetworkAddr ad;
	std::string tmp_address = address;
    TrimSpaces(tmp_address);
    int port = -1;
    if(StringToNetAddr(tmp_address, ad)) 
    {
    	port = GetNetAddrPort(ad);
    	if( port == 0 )
    		port = LX_PORT;
    }

	server_t * found = Menu_SvrList_FindServerStr(tmp_address, name);
    if( found && port != -1 && port != 0 )
    {
    	if( found->szName == "Untitled" )
    		found->szName = name;
    	//hints << "Menu_SvrList_AddServer(): merging duplicate " << found->szName << " " << found->szAddress << endl;

		for( size_t i = 0; i < found->ports.size(); i++ )
			if( found->ports[i].first == port )
				return found;
		found->ports.push_back( std::make_pair( port, udpMasterserverIndex ) );
		return found;
    }

    // Didn't find one, so create it
    psServerList.push_back(server_t());
	server_t * svr = & psServerList.back();

	// Fill in the details
    svr->bManual = bManual;
	svr->szAddress = tmp_address;
	ResetNetAddr(svr->sAddress);

	Menu_SvrList_RefreshServer(svr, bManual);
	
	if( svr->ports.size() > 0 )
		svr->ports[0].second = udpMasterserverIndex;
		
	// Default game details
	svr->szName = name;
	TrimSpaces(svr->szName);
	svr->nMaxPlayers = 0;
	svr->nNumPlayers = 0;
	svr->nState = 0;
	svr->nPing = -3; // Put it at the end of server list, after NAT servers
	if( udpMasterserverIndex >= 0 )
	{
		svr->bBehindNat = true;
		svr->nPing = -2;
	}
	else
		svr->bBehindNat = false;
	
	return svr;
}


///////////////////
// Remove a server from the server list
void Menu_SvrList_RemoveServer(const std::string& szAddress)
{
	for(std::list<server_t>::iterator it = psServerList.begin(); it != psServerList.end(); it++)
		if( it->szAddress == szAddress )
		{
			psServerList.erase( it );
			it = psServerList.begin();
			break;
		}
}


///////////////////
// Find a server based on a string address
server_t *Menu_SvrList_FindServerStr(const std::string& szAddress, const std::string & name)
{
	NetworkAddr addr;
	if( ! StringToNetAddr(szAddress, addr) )
		return NULL;
    
    return Menu_SvrList_FindServer(addr, name);
}


///////////////////
// Fill a listview box with the server list
void Menu_SvrList_FillList(CListview *lv)
{
	if (!lv)
		return;

	std::string		addr;
	static const std::string states[] = {"Open", "Loading", "Playing", "Open/Loading", "Open/Playing"};

    // Store the ID of the currently selected item
    int curID = lv->getSelectedID();

	lv->SaveScrollbarPos();
	lv->Clear();

	for(std::list<server_t>::iterator s = psServerList.begin(); s != psServerList.end(); s++)
	{

		bool processing = s->bProcessing && Menu_SvrList_GetUdpMasterserverForServer( s->szAddress ) == "";

		// Ping Image
		int num = 3;
		if(s->nPing < 700)  num = 2;
		if(s->nPing < 400)  num = 1;
		if(s->nPing < 200)  num = 0;

		if(s->bIgnore || processing)
			num = 3;

		if(s->nPing == -2)	num = 4; // Server behind a NAT

		// Address
		//GetRemoteNetAddr(tMenu->tSocket, &s->sAddress);
		//NetAddrToString(&s->sAddress, addr);

		// show port if special
		addr = s->szAddress;
		size_t p = addr.rfind(':');
		if(p != std::string::npos) {
			std::string sPort = addr.substr(p + 1);
			addr.erase(p);
			if(from_string<int>(sPort) != LX_PORT)
				addr += ":" + sPort;
		}

		// State
		int state = 0;
		if(s->nState >= 0 && s->nState < 3)
			state = s->nState;
		if( state != 0 && s->bAllowConnectDuringGame && s->nNumPlayers < s->nMaxPlayers )
			state += 2;

		// Colour
		Color colour = tLX->clListView;
		if(processing)
			colour = tLX->clDisabled;


		// Add the server to the list
		lv->AddItem(s->szAddress, 0, colour);
		lv->AddSubitem(LVS_IMAGE, itoa(num,10), tMenu->bmpConnectionSpeeds[num], NULL);
		lv->AddSubitem(LVS_TEXT, s->szName, (DynDrawIntf*)NULL, NULL);
        if(processing) {
			if(IsNetAddrValid(s->sAddress))
				lv->AddSubitem(LVS_TEXT, "Querying...", (DynDrawIntf*)NULL, NULL);
			else
				lv->AddSubitem(LVS_TEXT, "Lookup...", (DynDrawIntf*)NULL, NULL);
        } else if( num == 3 )
            lv->AddSubitem(LVS_TEXT, "Down", (DynDrawIntf*)NULL, NULL);
        else
		    lv->AddSubitem(LVS_TEXT, states[state], (DynDrawIntf*)NULL, NULL);

		bool unknownData = ( s->bProcessing || num == 3 ) && 
							Menu_SvrList_GetUdpMasterserverForServer( s->szAddress ) == "";

		// Players
		lv->AddSubitem(LVS_TEXT,
					   unknownData ? "?" : (itoa(s->nNumPlayers,10)+"/"+itoa(s->nMaxPlayers,10)),
					   (DynDrawIntf*)NULL, NULL);

		if (s->nPing <= -2) // Server behind a NAT or not queried, it will add spaces if s->nPing == -3 so not queried servers will be below NAT ones
			lv->AddSubitem(LVS_TEXT, "N/A" + std::string(' ', -2 - s->nPing), (DynDrawIntf*)NULL, NULL);
		else
			lv->AddSubitem(LVS_TEXT, unknownData ? "∞" : itoa(s->nPing,10), (DynDrawIntf*)NULL, NULL); // TODO: the infinity symbol isn't shown correctly

		// Country
		if (tLXOptions->bUseIpToCountry && iNetMode == net_internet) {
			IpInfo inf = tIpToCountryDB->GetInfoAboutIP(addr);
			if( tLXOptions->bShowCountryFlags )
			{
				SmartPointer<SDL_Surface> flag = tIpToCountryDB->GetCountryFlag(inf.countryCode);
				if (flag.get())
					lv->AddSubitem(LVS_IMAGE, "", flag, NULL, VALIGN_MIDDLE, inf.countryName);
				else
					lv->AddSubitem(LVS_TEXT, inf.countryCode, (DynDrawIntf*)NULL, NULL);
			}
			else
			{
				lv->AddSubitem(LVS_TEXT, inf.countryName, (DynDrawIntf*)NULL, NULL);
			}
		}

		// Address
		lv->AddSubitem(LVS_TEXT, addr, (DynDrawIntf*)NULL, NULL);
	}

	lv->ReSort();
    lv->setSelectedID(curID);
	lv->RestoreScrollbarPos();
}

static bool bUpdateFromUdpThread = false;
///////////////////
// Process the network connection
// Returns true if a server in the list was added/modified
bool Menu_SvrList_Process()
{
	CBytestream		bs;
	bool			update = false;


	// Process any packets on the net socket
	while(bs.Read(tMenu->tSocket[SCK_NET])) {

		if( Menu_SvrList_ParsePacket(&bs, tMenu->tSocket[SCK_NET]) )
			update = true;

	}

	// Process any packets on the LAN socket
	while(bs.Read(tMenu->tSocket[SCK_LAN])) {

		if( Menu_SvrList_ParsePacket(&bs, tMenu->tSocket[SCK_LAN]) )
			update = true;
	}
	
	if( bUpdateFromUdpThread )
	{
		bUpdateFromUdpThread = false;
		update = true;
	}

	bool repaint = false;


	// Ping or Query any servers in the list that need it
	for(std::list<server_t>::iterator s = psServerList.begin(); s != psServerList.end(); s++)
	{

		// Ignore this server? (timed out)
		if(s->bIgnore)
			continue;

		if(!IsNetAddrValid(s->sAddress)) {
			if(tLX->currentTime - s->fInitTime >= DNS_TIMEOUT) {
				s->bIgnore = true; // timeout
				update = true;
			}
			continue;
		} else {
			if(!s->bAddrReady) {
				s->bAddrReady = true;
				update = true;

				size_t f = s->szAddress.find(":");
				if(f != std::string::npos) {
					SetNetAddrPort(s->sAddress, from_string<int>(s->szAddress.substr(f + 1)));
				} else
					SetNetAddrPort(s->sAddress, LX_PORT);

			}
		}

		// Need a pingin'?
		if(!s->bgotPong) {
			if(tLX->currentTime - s->fLastPing > (float)PingWait / 1000.0f) {

				if(s->nPings >= MaxPings) {
					s->bIgnore = true;
					
					update = true;
				}
				else  {
					// Ping the server
					Menu_SvrList_PingServer(&(*s));
					repaint = true;
				}
			}
		}

		// Need a querying?
		if(s->bgotPong && !s->bgotQuery) {
			if(tLX->currentTime - s->fLastQuery > (float)QueryWait / 1000.0f) {

				if(s->nQueries >= MaxQueries) {
					s->bIgnore = true;

					update = true;
				}
				else  {
					// Query the server
					Menu_SvrList_QueryServer(&(*s));
					repaint = true;
				}
			}
		}

		// If we are ignoring this server now, set it to not processing
		if(s->bIgnore) {
			s->bProcessing = false;
			update = true;
		}

	}

	// Make sure the list repaints when the ping/query is received
	if (repaint)
		Timer("Menu_SvrList_Process ping waiter", null, NULL, PingWait + 100, true).startHeadless();

	return update;
}


///////////////////
// Parse a packet
// Returns true if we should update the list
bool Menu_SvrList_ParsePacket(CBytestream *bs, const SmartPointer<NetworkSocket>& sock)
{
	NetworkAddr		adrFrom;
	bool			update = false;
	std::string cmd,buf;

	// Check for connectionless packet header
	if(bs->readInt(4) == -1) {
		cmd = bs->readString();

		adrFrom = sock->remoteAddress();

		// Check for a pong
		if(cmd == "lx::pong") {

			// Look the the list and find which server returned the ping
			server_t *svr = Menu_SvrList_FindServer(adrFrom);
			if( svr ) {

				// It pinged, so fill in the ping info so it will now be queried
				svr->bgotPong = true;
				svr->nQueries = 0;
				svr->bBehindNat = false;
				svr->lastPingedPort = 0;
				SetNetAddrPort(svr->sAddress, GetNetAddrPort(adrFrom));
				NetAddrToString(svr->sAddress, svr->szAddress);
				svr->ports.clear();
				svr->ports.push_back( std::make_pair( (int)GetNetAddrPort(adrFrom), -1 ) );

			} else {

				// If we didn't ping this server directly (eg, subnet), add the server to the list
				// HINT: in favourites list, only user should add servers
				if (iNetMode != net_favourites)  {
					NetAddrToString( adrFrom, buf );
					svr = Menu_SvrList_AddServer(buf, false);

					if( svr ) {

						// Only update the list if this is the first ping
						if(!svr->bgotPong)
							update = true;

						// Set it the ponged
						svr->bgotPong = true;
						svr->nQueries = 0;

						//Menu_SvrList_RemoveDuplicateNATServers(svr); // We don't know the name of server yet
					}
				}
			}
		}

		// Check for a query return
		else if(cmd == "lx::queryreturn") {

			// Look the the list and find which server returned the ping
			server_t *svr = Menu_SvrList_FindServer(adrFrom);
			if( svr ) {

				// Only update the list if this is the first query
				if(!svr->bgotQuery)
					update = true;

				svr->bgotQuery = true;
				svr->bBehindNat = false;
				Menu_SvrList_ParseQuery(svr, bs);

			}

			// If we didn't query this server, then we should ignore it
		}

		else if(cmd == "lx::serverlist2") // This should not happen, we have another thread for polling UDP servers
		{
			Menu_SvrList_ParseUdpServerlist(bs, 0);
			update = true;
		}

	}

	return update;
}


///////////////////
// Find a server from the list by address
server_t *Menu_SvrList_FindServer(const NetworkAddr& addr, const std::string & name)
{
	for(std::list<server_t>::iterator s = psServerList.begin(); s != psServerList.end(); s++)
	{
		if( AreNetAddrEqual( addr, s->sAddress ) )
			return &(*s);
	}

    NetworkAddr addr1 = addr;
    SetNetAddrPort(addr1, LX_PORT);

	for(std::list<server_t>::iterator s = psServerList.begin(); s != psServerList.end(); s++)
	{
		// Check if any port number match from the server entry
		NetworkAddr addr2 = s->sAddress;
		for( size_t i = 0; i < s->ports.size(); i++ )
		{
			SetNetAddrPort(addr2, s->ports[i].first);
			if( AreNetAddrEqual( addr, addr2 ) )
				return &(*s);
		}
			
		// Check if IP without port and name match
		SetNetAddrPort(addr2, LX_PORT);
		if( AreNetAddrEqual( addr1, addr2 ) && name == s->szName && name != "Untitled" )
			return &(*s);
	}

	/*
	for(std::list<server_t>::iterator s = psServerList.begin(); s != psServerList.end(); s++)
	{
		// Check if just an IP without port match
		NetworkAddr addr2 = s->sAddress;
		SetNetAddrPort(addr2, LX_PORT);
		if( AreNetAddrEqual( addr1, addr2 ) )
			return &(*s);
	}
	*/

	// None found
	return NULL;
}


///////////////////
// Parse the server query return packet
void Menu_SvrList_ParseQuery(server_t *svr, CBytestream *bs)
{
	// TODO: move this net protocol stuff out here
	
	// Don't update the name in favourites
	std::string buf = Utf8String(bs->readString());
	if(iNetMode != net_favourites)
		svr->szName = buf;
	TrimSpaces(svr->szName);
	//hints << "Menu_SvrList_ParseQuery(): " << svr->szName << " " << svr->szAddress << endl;
	svr->nNumPlayers = bs->readByte();
	svr->nMaxPlayers = bs->readByte();
	svr->nState = bs->readByte();
    int num = bs->readByte();
	svr->bProcessing = false;
	svr->bAllowConnectDuringGame = false;
	svr->tVersion.reset();

    if(num < 0 || num >= MAX_QUERIES-1)
        num=0;

	svr->nPing = (int)( (tLX->currentTime - svr->fQueryTimes[num]).milliseconds() );

	if(svr->nPing < 0)
		svr->nPing = 999;
    if(svr->nPing > 999)
        svr->nPing = 999;
		
	if( !bs->isPosAtEnd() )
	{
		// Beta8+
		svr->tVersion.setByString( bs->readString(64) );
		svr->bAllowConnectDuringGame = bs->readBool();
	}
	
	// We got server name in a query. let's remove servers with the same name and IP, which we got from UDP masterserver
	for(std::list<server_t>::iterator it = psServerList.begin(); it != psServerList.end(); it++)
	{
		NetworkAddr addr1 = it->sAddress;
		SetNetAddrPort(addr1, LX_PORT);
		NetworkAddr addr2 = svr->sAddress;
		SetNetAddrPort(addr2, LX_PORT);
		if( it->szName == svr->szName && AreNetAddrEqual(addr1, addr2) && svr != &(*it) )
		{
			//Duplicate server - delete it
			//hints << "Menu_SvrList_ParseQuery(): removing duplicate " << it->szName << " " << it->szAddress << endl;
			psServerList.erase(it);
			it = psServerList.begin();
		}
	}
}

/*************************
*
* UDP server list
*
************************/

std::list<std::string> tUdpMasterServers;
std::map<size_t, ThreadPoolItem *> tUpdateThreads;
size_t threadId = 0;

struct UdpServerlistData  {
	CBytestream *bs;
	int UdpServerIndex;
	UdpServerlistData(CBytestream *b, int _UdpServerIndex) : bs(b), UdpServerIndex(_UdpServerIndex) {}
};

void Menu_UpdateUDPListEventHandler(UdpServerlistData data)
{
	if (iNetMode == net_internet) // Only add them if the Internet tab is active
		Menu_SvrList_ParseUdpServerlist(data.bs, data.UdpServerIndex);
	delete data.bs;
}

void Menu_UpdateUDPListEnd(size_t thread)
{
	std::map<size_t, ThreadPoolItem *>::iterator it = tUpdateThreads.find(thread);
	if (it != tUpdateThreads.end())
		threadPool->wait(it->second, NULL);
}

Event<UdpServerlistData> serverlistEvent;
Event<size_t> updateEndEvent;
int Menu_SvrList_UpdaterThread(void *id)
{
	// Setup event handlers
	updateEndEvent.handler() = getEventHandler(&Menu_UpdateUDPListEnd);
	serverlistEvent.handler() = getEventHandler(&Menu_UpdateUDPListEventHandler);

	// Open socket for networking
	NetworkSocket sock;
	if (!sock.OpenUnreliable(0))  {
		updateEndEvent.pushToMainQueue((size_t)id);
		return -1;
	}

	// Get serverlist from all the servers in the file
	int UdpServerIndex = 0;
	for (std::list<std::string>::iterator it = tUdpMasterServers.begin(); it != tUdpMasterServers.end(); ++it, ++UdpServerIndex)  
	{
		std::string& server = *it;
		NetworkAddr addr;
		if (server.find(':') == std::string::npos)
			server += ":23450";  // Default port

		// Split to domain and port
		std::string domain = server.substr(0, server.find(':'));
		int port = atoi(server.substr(server.find(':') + 1));

		// Resolve the address
		if (!GetNetAddrFromNameAsync(domain, addr))
			continue;

		AbsTime start = GetTime();
		while (GetTime() - start <= 5.0f) {
			SDL_Delay(40);
			if(IsNetAddrValid(addr)) 
				break;
		}
		
		if( !IsNetAddrValid(addr) )
		{
			notes << "UDP masterserver failed: cannot resolve domain name " << domain << endl;
			continue;
		}
		
		// Setup the socket
		SetNetAddrPort(addr, port);
		sock.setRemoteAddress(addr);

		// Send the getserverlist packet
		CBytestream *bs = new CBytestream();
		bs->writeInt(-1, 4);
		bs->writeString("lx::getserverlist2");
		if(!bs->Send(&sock)) { delete bs; warnings << "error while sending data to " << server << ", ignoring"; continue; }
		bs->Clear();

		//notes << "Sent getserverlist to " << server << endl;

		// Wait for the reply
		AbsTime timeoutTime = GetTime() + 5.0f;
		bool firstPacket = true;
		while( true ) {

			while (GetTime() <= timeoutTime)  {
				SDL_Delay(40); // TODO: do it event based

				// Got a reply?
				if (bs->Read(&sock))  {
					//notes << "Got a reply from " << server << endl;
					break;
				}
				
				
			}

			// Parse the reply
			if (bs->GetLength() && bs->readInt(4) == -1 && bs->readString() == "lx::serverlist2") {
				serverlistEvent.pushToMainQueue(UdpServerlistData(bs, UdpServerIndex));
				timeoutTime = GetTime() + 0.5f;	// Check for another packet
				bs = new CBytestream(); // old bs pointer is in mainqueue now
				firstPacket = false;
			} else  {
				if( firstPacket )
					warnings << "Error getting serverlist from " << server << endl;
				delete bs;
				break;
			}
		}
	}

	// Cleanup
	sock.Close();

	updateEndEvent.pushToMainQueue((size_t)id);
	return 0;
}

void Menu_SvrList_UpdateUDPList()
{
	if (tUdpMasterServers.size() == 0)  {  // Load the list of servers only if not already loaded
		// Open the masterservers file
		FILE *fp1 = OpenGameFile("cfg/udpmasterservers.txt", "rt");
		if(!fp1)  {
			warnings << "could not open udpmasterservers.txt file, NAT traversal will be inaccessible" << endl;
			return;
		}

		// Get the list of servers
		while( !feof(fp1) ) {
			std::string szLine = ReadUntil(fp1);
			TrimSpaces(szLine);

			if( szLine.length() == 0 )
				continue;

			tUdpMasterServers.push_back(szLine);
		}
		fclose(fp1);
	}

	// Run the update	
	ThreadPoolItem *thread = threadPool->start(Menu_SvrList_UpdaterThread, (void *)(++threadId), "serverlist updater");
	tUpdateThreads[threadId] = thread;
}

void Menu_SvrList_ParseUdpServerlist(CBytestream *bs, int UdpMasterserverIndex)
{
	// Look the the list and find which server returned the ping
	int amount = bs->readByte();
	//notes << "Menu_SvrList_ParseUdpServerlist " << amount << endl;
	for( int f=0; f<amount; f++ )
	{
		std::string addr = bs->readString();
		std::string name = bs->readString();
		TrimSpaces(name);
		TrimSpaces(addr);
		//hints << "Menu_SvrList_ParseUdpServerlist(): " << name << " " << addr << endl;
		int players = bs->readByte();
		int maxplayers = bs->readByte();
		int state = bs->readByte();
		Version version = bs->readString(64);
		bool allowConnectDuringGame = bs->readBool();
		// UDP server info is updated once per 40 seconds, so if we have more recent entry ignore it
		server_t *svr = Menu_SvrList_FindServerStr(addr, name);
		if( svr != NULL )
		{
			//hints << "Menu_SvrList_ParseUdpServerlist(): got duplicate " << name << " " << addr << " pong " << svr->bgotPong << " query " << svr->bgotQuery << endl;
			if( svr->bgotPong )
				continue;
			// It will merge existing server with new info
			Menu_SvrList_AddServer(addr, false, name, UdpMasterserverIndex);
			continue;
		}

		// In favourites/LAN only the user should add servers
		if (iNetMode == net_internet)  {
			svr = Menu_SvrList_AddServer( addr, false, name, UdpMasterserverIndex );
			svr->nNumPlayers = players;
			svr->nMaxPlayers = maxplayers;
			svr->nState = state;
			svr->nPing = -2;
			svr->nQueries = 0;
			svr->bgotPong = false;
			svr->bgotQuery = false;
			svr->bProcessing = false;
			svr->tVersion = version;
			svr->bAllowConnectDuringGame = allowConnectDuringGame;
			svr->bBehindNat = true;
		}
	};

	bUpdateFromUdpThread = true;
	// Update the GUI when ping times out
	Timer("Menu_SvrList_ParseUdpServerlist ping waiter", null, NULL, PingWait, true).startHeadless();
};

///////////////////
// Save the server list
void Menu_SvrList_SaveList(const std::string& szFilename)
{
    FILE *fp = OpenGameFile(szFilename,"wt");
    if( !fp )
        return;

	for(std::list<server_t>::iterator s = psServerList.begin(); s != psServerList.end(); s++)
	{
		int UdpMasterServer = -1;
		for( size_t port = 0; s->bBehindNat && port < s->ports.size() && UdpMasterServer == -1; port++ )
			if( s->ports[port].second >= 0 )
				UdpMasterServer = s->ports[port].second;

        fprintf(fp, "%s, %s, %s",s->bManual ? "1" : "0", s->szName.c_str(), s->szAddress.c_str() );
        if( UdpMasterServer != -1 && !s->bManual )
        	fprintf(fp, ", %i", UdpMasterServer );
       	fprintf(fp, "\n" );
	}

    fclose(fp);
}

///////////////////
// Add a favourite server
void Menu_SvrList_AddFavourite(const std::string& szName, const std::string& szAddress)
{
    FILE *fp = OpenGameFile("cfg/favourites.dat","a");  // We're appending
    if( !fp )  {
        fp = OpenGameFile("cfg/favourites.dat","wb");  // Try to create the file
		if (!fp)
			return;
	}

	// Append the server
    fprintf(fp,"%s, %s, %s\n","1", szName.c_str(), szAddress.c_str());

    fclose(fp);
}


///////////////////
// Load the server list
void Menu_SvrList_LoadList(const std::string& szFilename)
{
    FILE *fp = OpenGameFile(szFilename,"rt");
    if( !fp )
        return;

    // Go through every line
    while( !feof(fp) ) {
		std::string szLine = ReadUntil(fp);
        if( szLine == "" )
            continue;

		// explode and copy it
		std::vector<std::string> parsed = explode(szLine,",");

        if( parsed.size() >= 3 ) {
			TrimSpaces(parsed[0]);
			TrimSpaces(parsed[1]);
			TrimSpaces(parsed[2]); // Address

			int UdpMasterServer = -1;
			if( parsed.size() >= 4 )
				UdpMasterServer = atoi(parsed[3]);

            Menu_SvrList_AddServer(parsed[2], parsed[0] == "1", parsed[1], UdpMasterServer);
        }
    }

	// Update the GUI after the ping timed out
	Timer("Menu_SvrList_LoadList ping waiter", null, NULL, PingWait, true).startHeadless();

    fclose(fp);
}

std::string Menu_SvrList_GetUdpMasterserverForServer(const std::string & addr)
{
	server_t * svr = Menu_SvrList_FindServerStr(addr);
	if( !svr )
		return "";
	if( !svr->bBehindNat )
		return "";

	for( size_t port = 0; port < svr->ports.size(); port++ )
	{
		if( svr->ports[port].second < 0 )
			continue;
		int idx = 0;
		for( std::list<std::string>::iterator it = tUdpMasterServers.begin(); it != tUdpMasterServers.end(); ++it, ++idx )
			if( idx == svr->ports[port].second )
				return *it;
	}

	return "";
}


bool bGotDetails = false;
bool bOldLxBug = false;
int nTries = 0;
AbsTime fStart;
CListview lvInfo;


///////////////////
// Draw a 'server info' box
void Menu_SvrList_DrawInfo(const std::string& szAddress, int w, int h)
{
	int y = tMenu->bmpBuffer.get()->h/2 - h/2;
	int x = tMenu->bmpBuffer.get()->w/2 - w/2;

	Menu_redrawBufferRect(x,y,w,h);

    Menu_DrawBox(VideoPostProcessor::videoSurface(), x,y, x+w, y+h);
	DrawRectFillA(VideoPostProcessor::videoSurface(), x+2,y+2, x+w-1, y+h-1, tLX->clDialogBackground, 230);
    tLX->cFont.DrawCentre(VideoPostProcessor::videoSurface(), x+w/2, y+5, tLX->clNormalLabel, "Server Details");


	server_t* svr = Menu_SvrList_FindServerStr(szAddress);
	NetworkAddr origAddr;
	if(svr) {
		if(IsNetAddrValid(svr->sAddress)) {
			origAddr = svr->sAddress;
		} else {
			tLX->cFont.DrawCentre(VideoPostProcessor::videoSurface(), x+w/2, y+h/2-8, tLX->clNormalLabel,  "Resolving domain ...");
			return;
		}
	} else {
		warnings << "Querying server not from svr list: " << szAddress << endl;
		std::string tmp_addr = szAddress;
		TrimSpaces(tmp_addr);
		if(!StringToNetAddr(tmp_addr, origAddr)) {
			// TODO: this happens also, if the server is not in the serverlist
			// we should do the domain resolving also here by ourselfs
			tLX->cFont.DrawCentre(VideoPostProcessor::videoSurface(), x+w/2,y+tLX->cFont.GetHeight()+10, tLX->clError, "DNS not resolved");
			return;
		}
	}


    // Get the server details
    std::string		szName;
    int				nMaxWorms = 0;
    int				nState = 0;
    std::string		szMapName;
    std::string		szModName;
    int				nGameMode = 0;
    int				nLives = 0;
    int				nMaxKills = 0;
    int				nLoadingTime = 0;
    int				nBonuses = 0;
    int				nNumPlayers = 0;
	IpInfo			tIpInfo;
	std::string		sIP;
    CWorm			cWorms[MAX_WORMS];
	bool			bHaveLives = false;
	bool			bHaveVersion = false;
	std::string		sServerVersion;
	bool			bHaveGameSpeed = false;
	float			fGameSpeed = 1.0f;
	FeatureCompatibleSettingList features;
	
    CBytestream inbs;
    NetworkAddr   addr;

    if(nTries < 3 && !bGotDetails && !bOldLxBug) {

		tLX->cFont.DrawCentre(VideoPostProcessor::videoSurface(), x+w/2, y+h/2-8, tLX->clNormalLabel,  "Loading info...");

        if (inbs.Read(tMenu->tSocket[SCK_NET])) {
            // Check for connectionless packet header
	        if(inbs.readInt(4) == -1) {
                std::string cmd = inbs.readString();

		        addr = tMenu->tSocket[SCK_NET]->remoteAddress();
				
				if(cmd == "lx::traverse") // Response from UDP masterserver
				{
					sIP = inbs.readString();
					StringToNetAddr(sIP, addr);
					if( !inbs.isPosAtEnd() )
						cmd = inbs.readString();
				}
					
		        // Check for server info
		        if(cmd == "lx::serverinfo") {
                    bGotDetails = true;

					sServerVersion = "LieroX 0.56";

					// Get the IP info
					if (NetAddrToString(addr, sIP))
						tIpInfo = tIpToCountryDB->GetInfoAboutIP(sIP);
					else  {
						tIpInfo.countryName = "Hackerland";
						tIpInfo.continent = "Hackerland";
						sIP = "x.x.x.x";
					}


                    // Read the info
                    szName = Utf8String(inbs.readString(64));
	                nMaxWorms = MIN(MAX_PLAYERS, MAX((int)inbs.readByte(), 0));
	                nState = inbs.readByte();

					if (nState < 0)  {
						bOldLxBug = true;
					}

                    szMapName = inbs.readString(256);
					// Adjust the map name
					if (szMapName.find("levels/") == 0)
						szMapName.erase(0,7); // Remove the path if present
					szMapName = CMap::GetLevelName(szMapName);


                    szModName = inbs.readString(256);
	                nGameMode = inbs.readByte();
	                nLives = inbs.readInt16();
	                nMaxKills = inbs.readInt16();
	                nLoadingTime = inbs.readInt16();
					if(nLoadingTime < 0)  {
						bOldLxBug = true;
					}
                    nBonuses = inbs.readByte();

                    // Worms
                    nNumPlayers = inbs.readByte();
					if (nNumPlayers < 0)  {
						bOldLxBug = true;
					}

					// Check
					nNumPlayers = MIN(nMaxWorms,nNumPlayers);

					int i;
                    for(i=0; i<nNumPlayers; i++) {
                        cWorms[i].setName(inbs.readString());
                        cWorms[i].setKills(inbs.readInt(2));
                    }

					if (nState == 1 && !bOldLxBug)  { // Loading and no bug? Must be a fixed version -> LXP/OLX b1 or 2
						sServerVersion = "LXP or OLX beta1/beta2";
					}

					// Lives (only OLX servers)
					if(!inbs.isPosAtEnd())  {
						sServerVersion = "OpenLieroX/0.57_Beta3";
						bHaveLives = true;
						for(i=0; i<nNumPlayers; i++)
							cWorms[i].setLives(inbs.readInt(2));
					}

					// IPs
					if(!inbs.isPosAtEnd())  {
						sServerVersion = "OpenLieroX/0.57_Beta4";
						for(i=0; i<nNumPlayers; i++)
							inbs.readString(); // ignore
					}

					if(!inbs.isPosAtEnd())  {
						bHaveVersion = true;
						sServerVersion = inbs.readString();
					}

					if(!inbs.isPosAtEnd())  {
						bHaveGameSpeed = true;
						fGameSpeed = inbs.readFloat();
					}

					// since Beta9
					if(!inbs.isPosAtEnd())  {
						int ftC = inbs.readInt(2);
						for(int i = 0; i < ftC; ++i) {
							std::string name = inbs.readString();
							std::string humanName = inbs.readString();
							ScriptVar_t value; inbs.readVar(value);
							bool olderClientsSupported = inbs.readBool();
							Feature* f = featureByName(name);
							if(f) {
								features.set(name, humanName, value, FeatureCompatibleSettingList::Feature::FCSL_SUPPORTED);
							} else if(!olderClientsSupported) {
								features.set(name, humanName, value, FeatureCompatibleSettingList::Feature::FCSL_JUSTUNKNOWN);
							} else {
								features.set(name, humanName, value, FeatureCompatibleSettingList::Feature::FCSL_INCOMPATIBLE);
							}
						}
					}
					
                }
            }
        }

        if((tLX->currentTime - fStart > 1) && !bGotDetails) {
            nTries++;
            fStart = tLX->currentTime;
			bGotDetails = false;
			bOldLxBug = false;

			if(svr)
				Menu_SvrList_GetServerInfo(svr);
        }

		// Got details, fill in the listview
		if (bGotDetails && !bOldLxBug)  {

			// States and gamemodes
   			const std::string states[] = {"Open", "Loading", "Playing", "Unknown"};
			const std::string gamemodes[] = {"Deathmatch","Team Deathmatch", "Tag", "Demolitions", "Unknown"};


			// Checks
			if (nState < 0 || nState > 2)
				nState = 3;
			if (nGameMode < 0 || nGameMode > 3)
				nGameMode = 4;
			nNumPlayers = MIN(nNumPlayers, MAX_WORMS-1);


			// Setup the listview
			lvInfo.Setup(0, x + 15, y+5, w - 30, h - 25);
			lvInfo.setDrawBorder(false);
			lvInfo.setRedrawMenu(false);
			lvInfo.setShowSelect(false);
			lvInfo.setOldStyle(true);
			// TODO: will the listview have scrollbars if too long? if not, please add this

			lvInfo.Destroy(); // Clear any old info

			// Columns
			int first_column_width = tLX->cFont.GetWidth("Loading Times:") + 30; // Width of the widest item in this column + some space
			int last_column_width = tLX->cFont.GetWidth("999"); // Kills width
			lvInfo.AddColumn("", first_column_width);
			lvInfo.AddColumn("", lvInfo.getWidth() - first_column_width - (last_column_width*2) - gfxGUI.bmpScrollbar.get()->w); // The rest
			lvInfo.AddColumn("", last_column_width);
			lvInfo.AddColumn("", last_column_width);

			int index = 0;  // Current item index

			// Server name
			lvInfo.AddItem("servername", index, tLX->clNormalLabel);
			lvInfo.AddSubitem(LVS_TEXT, "Server name:", (DynDrawIntf*)NULL, NULL);
			lvInfo.AddSubitem(LVS_TEXT, szName, (DynDrawIntf*)NULL, NULL);

			// server version
			lvInfo.AddItem("serverversion", index, tLX->clNormalLabel);
			lvInfo.AddSubitem(LVS_TEXT, "Server version:", (DynDrawIntf*)NULL, NULL);
			lvInfo.AddSubitem(LVS_TEXT, sServerVersion, (DynDrawIntf*)NULL, NULL);

			// Country and continent
			lvInfo.AddItem("country", ++index, tLX->clNormalLabel);
			lvInfo.AddSubitem(LVS_TEXT, "Location:", (DynDrawIntf*)NULL, NULL);
			if (tIpInfo.hasCityLevel)
				lvInfo.AddSubitem(LVS_TEXT, tIpInfo.city + ", " + tIpInfo.countryName + " (" + tIpInfo.continent + ")", (DynDrawIntf*)NULL, NULL);
			else
				lvInfo.AddSubitem(LVS_TEXT, tIpInfo.countryName + " (" + tIpInfo.continent + ")", (DynDrawIntf*)NULL, NULL);

			// IP address
			lvInfo.AddItem("ip", ++index, tLX->clNormalLabel);
			lvInfo.AddSubitem(LVS_TEXT, "IP Address:", (DynDrawIntf*)NULL, NULL);
			lvInfo.AddSubitem(LVS_TEXT, sIP, (DynDrawIntf*)NULL, NULL);

			// Map name
			lvInfo.AddItem("mapname", ++index, tLX->clNormalLabel);
			lvInfo.AddSubitem(LVS_TEXT, "Level name:", (DynDrawIntf*)NULL, NULL);
			lvInfo.AddSubitem(LVS_TEXT, szMapName, (DynDrawIntf*)NULL, NULL);

			// Mod name
			lvInfo.AddItem("modname", ++index, tLX->clNormalLabel);
			lvInfo.AddSubitem(LVS_TEXT, "Mod name:", (DynDrawIntf*)NULL, NULL);
			lvInfo.AddSubitem(LVS_TEXT, szModName, (DynDrawIntf*)NULL, NULL);

			// State
			lvInfo.AddItem("state", ++index, tLX->clNormalLabel);
			lvInfo.AddSubitem(LVS_TEXT, "State:", (DynDrawIntf*)NULL, NULL);
			lvInfo.AddSubitem(LVS_TEXT, states[nState], (DynDrawIntf*)NULL, NULL);

			// Playing
			lvInfo.AddItem("playing", ++index, tLX->clNormalLabel);
			lvInfo.AddSubitem(LVS_TEXT, "Playing:", (DynDrawIntf*)NULL, NULL);
			lvInfo.AddSubitem(LVS_TEXT, itoa(nNumPlayers) + " / " + itoa(nMaxWorms), (DynDrawIntf*)NULL, NULL);

			// Game type
			lvInfo.AddItem("game type", ++index, tLX->clNormalLabel);
			lvInfo.AddSubitem(LVS_TEXT, "Game Type:", (DynDrawIntf*)NULL, NULL);
			lvInfo.AddSubitem(LVS_TEXT, gamemodes[nGameMode], (DynDrawIntf*)NULL, NULL);

			// Lives
			lvInfo.AddItem("lives", ++index, tLX->clNormalLabel);
			lvInfo.AddSubitem(LVS_TEXT, "Lives:", (DynDrawIntf*)NULL, NULL);
			if (nLives < 0)
				lvInfo.AddSubitem(LVS_IMAGE, "", gfxGame.bmpInfinite, NULL);
			else
				lvInfo.AddSubitem(LVS_TEXT, itoa(nLives), (DynDrawIntf*)NULL, NULL);

			// Max kills
			lvInfo.AddItem("maxkills", ++index, tLX->clNormalLabel);
			lvInfo.AddSubitem(LVS_TEXT, "Max Kills:", (DynDrawIntf*)NULL, NULL);
			if (nMaxKills < 0)
				lvInfo.AddSubitem(LVS_IMAGE, "", gfxGame.bmpInfinite, NULL);
			else
				lvInfo.AddSubitem(LVS_TEXT, itoa(nMaxKills), (DynDrawIntf*)NULL, NULL);

			// Loading times
			lvInfo.AddItem("loading", ++index, tLX->clNormalLabel);
			lvInfo.AddSubitem(LVS_TEXT, "Loading Times:", (DynDrawIntf*)NULL, NULL);
			lvInfo.AddSubitem(LVS_TEXT, itoa(nLoadingTime) + " %", (DynDrawIntf*)NULL, NULL);

			// Separator
			lvInfo.AddItem("", ++index, tLX->clNormalLabel);

			// Players / kills / lives
			lvInfo.AddItem("players", ++index, tLX->clNormalLabel);
			if (nState)  {
				lvInfo.AddSubitem(LVS_TEXT, "Players/Kills/Lives:", (DynDrawIntf*)NULL, NULL);

				// First player (located next to the Players/Kills/Lives label)
				lvInfo.AddSubitem(LVS_TEXT, cWorms[0].getName(), (DynDrawIntf*)NULL, NULL);
				lvInfo.AddSubitem(LVS_TEXT, itoa(cWorms[0].getKills()), (DynDrawIntf*)NULL, NULL);
				if (bHaveLives)  {
					switch ((short)cWorms[0].getLives())  {
					case -1:  // Out
						lvInfo.AddSubitem(LVS_TEXT, "Out", (DynDrawIntf*)NULL, NULL);
						break;
					case -2:  // Unlim
						lvInfo.AddSubitem(LVS_IMAGE, "", gfxGame.bmpInfinite, NULL);
						break;
					default:
						lvInfo.AddSubitem(LVS_TEXT, itoa(cWorms[0].getLives()), (DynDrawIntf*)NULL, NULL);
					}
				}

				// Rest of the players
				for (int i=1; i < nNumPlayers; i++)  {
					lvInfo.AddItem("players"+itoa(i+1), ++index, tLX->clNormalLabel);
					lvInfo.AddSubitem(LVS_TEXT, "", (DynDrawIntf*)NULL, NULL);
					lvInfo.AddSubitem(LVS_TEXT, cWorms[i].getName(), (DynDrawIntf*)NULL, NULL);
					lvInfo.AddSubitem(LVS_TEXT, itoa(cWorms[i].getKills()), (DynDrawIntf*)NULL, NULL);
					if (bHaveLives)  {
						switch ((short)cWorms[i].getLives())  {
						case -1:  // Out
							lvInfo.AddSubitem(LVS_TEXT, "Out", (DynDrawIntf*)NULL, NULL);
							break;
						case -2:  // Unlim
							lvInfo.AddSubitem(LVS_IMAGE, "", gfxGame.bmpInfinite, NULL);
							break;
						default:
							lvInfo.AddSubitem(LVS_TEXT, itoa(cWorms[i].getLives()), (DynDrawIntf*)NULL, NULL);
						}
					}
				}
			}

			else  { // Don't draw kills when the server is open
				lvInfo.AddSubitem(LVS_TEXT, "Players:", (DynDrawIntf*)NULL, NULL);

				// First player (located next to the Players/Kills label)
				lvInfo.AddSubitem(LVS_TEXT, cWorms[0].getName(), (DynDrawIntf*)NULL, NULL);

				// Rest of the players
				for (int i = 1; i < nNumPlayers; i++)  {
					lvInfo.AddItem("players"+itoa(i+1), ++index, tLX->clNormalLabel);
					lvInfo.AddSubitem(LVS_TEXT, "", (DynDrawIntf*)NULL, NULL);
					lvInfo.AddSubitem(LVS_TEXT, cWorms[i].getName(), (DynDrawIntf*)NULL, NULL);
				}
			}

			// Separator
			lvInfo.AddItem("", ++index, tLX->clNormalLabel);
			
			// Bonuses
			lvInfo.AddItem("bonuses", ++index, tLX->clNormalLabel);
			lvInfo.AddSubitem(LVS_TEXT, "Bonuses:", (DynDrawIntf*)NULL, NULL);
			lvInfo.AddSubitem(LVS_TEXT, nBonuses ? "On" : "Off", (DynDrawIntf*)NULL, NULL);

			if(bHaveGameSpeed) {
				// Loading times
				lvInfo.AddItem("gamespeed", ++index, tLX->clNormalLabel);
				lvInfo.AddSubitem(LVS_TEXT, "Game speed:", (DynDrawIntf*)NULL, NULL);
				lvInfo.AddSubitem(LVS_TEXT, ftoa(fGameSpeed), (DynDrawIntf*)NULL, NULL);
			}
			
			for_each_iterator( FeatureCompatibleSettingList::Feature&, it, features.list ){
				Color col;
				switch(it->get().type) {
					case FeatureCompatibleSettingList::Feature::FCSL_JUSTUNKNOWN: col = tLX->clDisabled; break;
					case FeatureCompatibleSettingList::Feature::FCSL_INCOMPATIBLE: col = tLX->clError; break;
					default: col = tLX->clNormalLabel;
				}
				lvInfo.AddItem("feature:" + it->get().name, ++index, col);
				if(tLX->cFont.GetWidth(it->get().humanName + ":") + 20 <= first_column_width) {
					lvInfo.AddSubitem(LVS_TEXT, it->get().humanName + ":", (DynDrawIntf*)NULL, NULL);
					lvInfo.AddSubitem(LVS_TEXT, it->get().var.toString(), (DynDrawIntf*)NULL, NULL);					
				} else
					lvInfo.AddSubitem(LVS_TEXT, it->get().humanName + ":      " + it->get().var.toString(), (DynDrawIntf*)NULL, NULL);
			}

		}
		else // No details yet
			return;
    }

	// No details, server down
    if(!bGotDetails) {
        tLX->cFont.DrawCentre(VideoPostProcessor::videoSurface(), x+w/2,y+tLX->cFont.GetHeight()+10, tLX->clError, "Unable to query server");
        return;
    }

	// Old bug
    if(bOldLxBug) {
        tLX->cFont.Draw(VideoPostProcessor::videoSurface(), x+15,y+tLX->cFont.GetHeight()+10, tLX->clError, "You can't view details\nof this server because\nLieroX v0.56 contains a bug.\n\nPlease wait until the server\nchanges its state to Playing\nand try again.");
        return;
    }

	// Process the listview events
	mouse_t *Mouse = GetMouse();
	if (lvInfo.InBox(Mouse->X, Mouse->Y))  {
		lvInfo.MouseOver(Mouse);
		if (Mouse->Down)
			lvInfo.MouseDown(Mouse, true);
		else if (Mouse->Up)
			lvInfo.MouseUp(Mouse, false);

		if (Mouse->WheelScrollUp)
			lvInfo.MouseWheelUp(Mouse);
		else if (Mouse->WheelScrollDown)
			lvInfo.MouseWheelDown(Mouse);
	}

	// All ok, draw the details
	lvInfo.Draw( VideoPostProcessor::videoSurface() );
}

void Menu_Current_Shutdown() {
	if(!tMenu) return;
	
	// Shutdown all sub-menus
	if(!bDedicated)
		switch(tMenu->iMenuType) {
				
				// Main
			case MNU_MAIN:
				Menu_MainShutdown();
				break;
				
				// Local
			case MNU_LOCAL:
				Menu_LocalShutdown();
				break;
				
				// News
			case MNU_NETWORK:
				Menu_NetShutdown();
				break;
				
				// Player
			case MNU_PLAYER:
				Menu_PlayerShutdown();
				break;
				
				// Map editor
			case MNU_MAPED:
				Menu_MapEdShutdown();
				break;
				
				// Options
			case MNU_OPTIONS:
				Menu_OptionsShutdown();
				break;
				
			case MNU_GUISKIN:
				Menu_CGuiSkinShutdown();
				break;
		}
	
	/*Menu_MainShutdown();
	Menu_LocalShutdown();
	Menu_PlayerShutdown();
	Menu_MapEdShutdown();
	Menu_GameSettingsShutdown();
	Menu_WeaponsRestrictionsShutdown();
	Menu_WeaponPresetsShutdown();
	Menu_BanListShutdown();
	Menu_ServerSettingsShutdown();
	Menu_OptionsShutdown();
	Menu_FloatingOptionsShutdown();
	Menu_SpeedTest_Shutdown();
	Menu_NetShutdown();
	Menu_Net_MainShutdown();
	Menu_Net_HostPlyShutdown();
	Menu_Net_HostLobbyShutdown();
	Menu_Net_LANShutdown();
	Menu_Net_JoinShutdown();
	Menu_Net_FavouritesShutdown();
	Menu_Net_NewsShutdown();
	Menu_Net_JoinShutdown();
	Menu_Net_ChatShutdown();
	Menu_Net_JoinConnectionShutdown();
	Menu_Net_JoinLobbyShutdown();
	Menu_Net_NETShutdown();
	Menu_CGuiSkinShutdown();*/
}
	
} // namespace DeprecatedGUI



void TaskManager::renderTasksStatus(SDL_Surface* s) {
	std::list<std::string> statusTxts;
	
	static const unsigned int MaxEntries = 4;
	{
		ScopedLock lock(mutex);
		for(std::set<Task*>::const_iterator i = runningTasks.begin(); i != runningTasks.end(); ++i) {
			std::string statusTxt;
			{
				Mutex::ScopedLock lock(*(*i)->mutex);
				statusTxt = (*i)->statusText();
			}
			if(statusTxt != "") {
				statusTxts.push_back(statusTxt);
				if(statusTxts.size() >= MaxEntries)
					break;
			}
		}
	}
	
	if(!statusTxts.empty()) {
		static const int StatusLoadingLeft = 5;
		static const int StatusLeft = 30;
		static const int StatusTop = 5;
		const int StatusHeight = tLX->cFont.GetHeight() + 5;
		DrawRectFill(s, 0, 0, s->w, StatusTop + StatusHeight * statusTxts.size(), Color(42,73,145,180));
		
		int y = StatusTop;
		for(std::list<std::string>::iterator i = statusTxts.begin(); i != statusTxts.end(); ++i) {
			tLX->cFont.Draw(s, StatusLeft, y, Color(255,255,255), *i);
			y += StatusHeight;
		}
	
		static const int StatusLoadingSize = 9;
		DrawLoadingAni(s, StatusLoadingLeft + StatusLoadingSize, StatusHeight * statusTxts.size() - StatusLoadingSize, StatusLoadingSize, StatusLoadingSize, Color(255,255,255), Color(128,128,128), LAT_CIRCLES);
	}
}
