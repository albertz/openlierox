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


#include "defs.h"
#include "LieroX.h"
#include "Menu.h"

menu_t	*tMenu = NULL;


int			*iGame = NULL;
int			iSkipStart = false;
CWidgetList	LayoutWidgets[LAYOUT_COUNT];
CCssParser	cWidgetStyles;


///////////////////
// Initialize the menu system
int Menu_Initialize(int *game)
{
	iGame = game;
	*iGame = false;
	iJoin_Recolorize = true;
	iHost_Recolorize = true;

	// Load the CSS of all widgets
	cWidgetStyles.Clear();
	static char path[1024];
	path[0] = '\0';
	//sprintf(path,"%s/%s/widgets.css",tLXOptions->sSkinPath,tLXOptions->sResolution);
	//cWidgetStyles.Parse(path);
	fix_markend(path);

	// Allocate the menu structure
	tMenu = new menu_t;
    if(tMenu == NULL) {
        SystemError("Error: Out of memory in for menu");
		return false;
    }

	tMenu->iReturnTo = net_internet;

	// Load the images
	//LOAD_IMAGE_BPP(tMenu->bmpMainBack,"data/frontend/background.png");
    //LOAD_IMAGE_BPP(tMenu->bmpMainBack_lg,"data/frontend/background_lg.png");
    LOAD_IMAGE_BPP(tMenu->bmpMainBack_wob,"data/frontend/background_wob.png");


	tMenu->bmpBuffer = gfxCreateSurface(640,480);
    if(tMenu->bmpBuffer == NULL) {
        SystemError("Error: Out of memory back buffer");
		return false;
    }


	tMenu->bmpMsgBuffer = gfxCreateSurface(640,480);
    if(tMenu->bmpMsgBuffer == NULL) {
        SystemError("Error: Out of memory in MsgBuffer");
		return false;
	}

    tMenu->bmpMiniMapBuffer = gfxCreateSurface(128,96);
    if(tMenu->bmpMiniMapBuffer == NULL) {
        SystemError("Error: Out of memory in MiniMapBuffer");
		return false;
    }

	LOAD_IMAGE(tMenu->bmpMainTitles,"data/frontend/maintitles.png");
	LOAD_IMAGE(tMenu->bmpLieroXtreme,"data/frontend/lierox.png");
	LOAD_IMAGE(tMenu->bmpMainLocal,"data/frontend/img_local.png");
	LOAD_IMAGE(tMenu->bmpMainNet,"data/frontend/img_net.png");
	LOAD_IMAGE(tMenu->bmpSubTitles,"data/frontend/subtitles.png");
	LOAD_IMAGE(tMenu->bmpButtons,"data/frontend/buttons.png");
	LOAD_IMAGE_BPP(tMenu->bmpMapEdTool,"data/frontend/map_toolbar.png");
	LOAD_IMAGE(tMenu->bmpCheckbox,"data/frontend/checkbox.png");
	LOAD_IMAGE(tMenu->bmpInputbox,"data/frontend/inputbox.png");
	//LOAD_IMAGE_BPP(tMenu->bmpAI,"data/frontend/cpu.png");
	//LOAD_IMAGE_BPP(tMenu->bmpWorm, "data/frontend/wormimage.bmp");
	LOAD_IMAGE(tMenu->bmpLobbyState, "data/frontend/lobbyready.png");
	LOAD_IMAGE(tMenu->bmpHost, "data/frontend/host.png");
	LOAD_IMAGE(tMenu->bmpConnectionSpeeds[0], "data/frontend/con_good.png");
	LOAD_IMAGE(tMenu->bmpConnectionSpeeds[1], "data/frontend/con_average.png");
	LOAD_IMAGE(tMenu->bmpConnectionSpeeds[2], "data/frontend/con_bad.png");
	LOAD_IMAGE(tMenu->bmpConnectionSpeeds[3], "data/frontend/con_none.png");
	LOAD_IMAGE(tMenu->bmpSpeech, "data/frontend/speech.png");
	LOAD_IMAGE(tMenu->bmpTeamColours[0], "data/frontend/team_1.png");
	LOAD_IMAGE(tMenu->bmpTeamColours[1], "data/frontend/team_2.png");
	LOAD_IMAGE(tMenu->bmpTeamColours[2], "data/frontend/team_3.png");
	LOAD_IMAGE(tMenu->bmpTeamColours[3], "data/frontend/team_4.png");
    LOAD_IMAGE(tMenu->bmpHandicap, "data/frontend/handicap.png");
	LOAD_IMAGE(tMenu->bmpTriangleUp, "data/frontend/triangle_up.png");
	LOAD_IMAGE(tMenu->bmpTriangleDown, "data/frontend/triangle_down.png");

#ifndef DEBUG
	// TODO: I don't know why until now, but the SDL_DisplayFormatAlpha causes
	//		some problems in valgrind (it causes in an invalid instruction
	//		call on my PPC with SDL-1.2.11)
	// Convert the buttons to the display format (faster)
	SDL_Surface *tmp = NULL;
	tmp = SDL_DisplayFormatAlpha(tMenu->bmpButtons);
	if (!tmp)
		return false;
	// NOTE: Don't free the original surface, it's freed by the cache
	tMenu->bmpButtons = tmp;
#endif

    tMenu->bmpWorm = NULL;
	tMenu->bmpScreen = SDL_GetVideoSurface();


	// Open a socket for communicating over the net (UDP)
	tMenu->tSocket[SCK_NET] = OpenUnreliableSocket(0);
	// Open a socket for broadcasting over a LAN (UDP)
	tMenu->tSocket[SCK_LAN] = OpenBroadcastSocket(0);

	if(!IsSocketStateValid(tMenu->tSocket[SCK_LAN]) || !IsSocketStateValid(tMenu->tSocket[SCK_NET])) {
		SystemError("Error: Failed to open a socket for networking");
		return false;
	}

	// Add default widget IDs to the widget list
	//Menu_AddDefaultWidgets();

	return true;
}


///////////////////
// Shutdown the menu
void Menu_Shutdown(void)
{
	if(tMenu) {
		// Shutdown all sub-menus
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
		}

		// Manually free some items
		if(tMenu->bmpBuffer)
			SDL_FreeSurface(tMenu->bmpBuffer);

		if(tMenu->bmpMsgBuffer)
			SDL_FreeSurface(tMenu->bmpMsgBuffer);

        if(tMenu->bmpMiniMapBuffer)
			SDL_FreeSurface(tMenu->bmpMiniMapBuffer);

		if(IsSocketStateValid(tMenu->tSocket[SCK_LAN]))
			CloseSocket(tMenu->tSocket[SCK_LAN]);
		if(IsSocketStateValid(tMenu->tSocket[SCK_NET]))
			CloseSocket(tMenu->tSocket[SCK_NET]);

		SetSocketStateValid(tMenu->tSocket[SCK_LAN], false);
		SetSocketStateValid(tMenu->tSocket[SCK_NET], false);

		// The rest get free'd in the cache
		assert(tMenu);
		delete tMenu;
		tMenu = NULL;
	}

	// Shutdown the layouts
	for (int i=0; i<LAYOUT_COUNT; i++)
		LayoutWidgets[i].Shutdown();

	Menu_SvrList_Shutdown();
}


///////////////////
// Start the menu
void Menu_Start(void)
{
	tMenu->iMenuRunning = true;
	// User can switch video mode while playing
	tMenu->bmpScreen = SDL_GetVideoSurface();

	if(!iSkipStart) {
		tMenu->iMenuType = MNU_MAIN;
		Menu_MainInitialize();
    } else
		Menu_RedrawMouse(true);

	Menu_Loop();
	iSkipStart = false;
}


///////////////////
// Set the skip start bit
void Menu_SetSkipStart(int s)
{
    iSkipStart = s;
}


///////////////////
// Main menu loop
void Menu_Loop(void)
{
//	keyboard_t *kb = GetKeyboard();  // TODO: not used
//	mouse_t *mouse = GetMouse();   // TODO: not used

	tLX->fCurTime = GetMilliSeconds();
	float oldtime = tLX->fCurTime;
	float fMaxFrameTime = 1.0f / (float)tLXOptions->nMaxFPS;

	while(tMenu->iMenuRunning) {

		tLX->fCurTime = GetMilliSeconds();
		tLX->fDeltaTime = tLX->fCurTime - oldtime;

	        // Cap the FPS
		if(tLX->fDeltaTime < fMaxFrameTime) {
			SDL_Delay((int)((fMaxFrameTime - tLX->fDeltaTime)*1000));
		        continue;
		}
		oldtime = tLX->fCurTime;

		Menu_RedrawMouse(false);
		ProcessEvents();

		// Media player frame
		cMediaPlayer.Frame();

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
				Menu_MapEdFrame(tMenu->bmpScreen,true);
				break;

			// Options
			case MNU_OPTIONS:
				Menu_OptionsFrame();
				break;
		}

		// At last draw the media player
		cMediaPlayer.Draw(tMenu->bmpScreen);

        FlipScreen(tMenu->bmpScreen);
	}
}


///////////////////
// Redraw the rectangle under the mouse (total means a total buffer redraw)
void Menu_RedrawMouse(int total)
{
	if(total) {
		SDL_BlitSurface(tMenu->bmpBuffer,NULL,tMenu->bmpScreen,NULL);
		return;
	}

	mouse_t *m = GetMouse();
	DrawImageAdv(tMenu->bmpScreen,tMenu->bmpBuffer,
				m->X-30,m->Y-30,
				m->X-30,m->Y-30,
				60,60);
}


///////////////////
// Draw a sub title
void Menu_DrawSubTitle(SDL_Surface *bmpDest, int id)
{
	int x = tMenu->bmpScreen->w/2;
	x -= tMenu->bmpSubTitles->w/2;

	DrawImageAdv(bmpDest,tMenu->bmpSubTitles, 0, id*70, x,30, tMenu->bmpSubTitles->w, 65);
}


///////////////////
// Draw a sub title advanced
void Menu_DrawSubTitleAdv(SDL_Surface *bmpDest, int id, int y)
{
	int x = tMenu->bmpScreen->w/2;
	x -= tMenu->bmpSubTitles->w/2;

	DrawImageAdv(bmpDest,tMenu->bmpSubTitles, 0, id*70, x,y, tMenu->bmpSubTitles->w, 65);
}

///////////////////
// Get the level name from specified file
std::string Menu_GetLevelName(const std::string& filename)
{
	static char	id[32], name[128];
	int		version;
	static std::string	Path;
	static std::string  Result;

	Path = "levels/"+filename;

 // Liero Xtreme level
	if( filename.substr(filename.length()-4) == ".lxl") {
		FILE *fp = OpenGameFile(Path,"rb");
		if(fp) {
			fread(id,		sizeof(char),	32,	fp);
			fread(&version,	sizeof(int),	1,	fp);
			EndianSwap(version);
			fread(name,		sizeof(char),	64,	fp);
			fix_markend(id); fix_markend(name);

			if(strcmp(id,"LieroX Level") == 0 && version == MAP_VERSION) {
				Result = name;
				fclose(fp);
				return Result;
			}
			fclose(fp);
		}
		return filename;
	}

 // Liero level
	// TODO
	if( filename.substr(filename.length()-4) == ".lev") {
		FILE *fp = OpenGameFile(Path,"rb");

		if(fp) {

			// Make sure it's the right size to be a liero level
			fseek(fp,0,SEEK_END);
			// 176400 is liero maps
			// 176402 is worm hole maps (same, but 2 bytes bigger)
			// 177178 is a powerlevel
			if( ftell(fp) == 176400 || ftell(fp) == 176402 || ftell(fp) == 177178) {
				Result = findLastPathSep(Path);
				fclose(fp);
				return Result;
			}
			fclose(fp);
		} // if(fp)
	}
  return filename;
}

////////////////
// Draws advanced box
void Menu_DrawBoxAdv(SDL_Surface *bmpDest, int x, int y, int x2, int y2, int border, Uint32 LightColour, Uint32 DarkColour, Uint32 BgColour, uchar type)
{
	// First draw the background
	if (BgColour != tLX->clPink)
		DrawRectFill(bmpDest,x+border,y+border,x2-border+1,y2-border+1,BgColour);

	if (!border)
		return;

	int i;

	// Switch the light and dark colour when inset
	if (type == BX_INSET)  {
		Uint32 tmp = LightColour;
		LightColour = DarkColour;
		DarkColour = tmp;
	}

	// Create gradient when needed
	int r_step,g_step,b_step;
	Uint8 r1,g1,b1,r2,g2,b2;
	SDL_GetRGB(DarkColour,bmpDest->format,&r1,&g1,&b1);
	SDL_GetRGB(LightColour,bmpDest->format,&r2,&g2,&b2);

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
		DrawRect(bmpDest,x+i,y+i,x2-i,y2-i,MakeColour(r1+r_step*i,g1+g_step*i,b1+b_step*i));
}


///////////////////
// Draw a box
void Menu_DrawBox(SDL_Surface *bmpDest, int x, int y, int x2, int y2)
{
    Uint32 dark = tLX->clBoxDark;//MakeColour(60,60,60);
    Uint32 light = tLX->clBoxLight;//MakeColour(130,130,130);

	DrawRect( bmpDest,x+1, y+1, x2-1,y2-1, light);
    //DrawRect( bmpDest,x+2, y+2, x2-2,y2-2, dark);
	DrawHLine(bmpDest,x+2, x2-1,y,  dark);
	DrawHLine(bmpDest,x+2, x2-1,y2, dark);

	DrawVLine(bmpDest,y+2, y2-1,x,  dark);
	DrawVLine(bmpDest,y+2, y2-1,x2, dark);
	PutPixel( bmpDest,x+1, y+1,     dark);
	PutPixel( bmpDest,x2-1,y+1,     dark);
	PutPixel( bmpDest,x+1, y2-1,    dark);
	PutPixel( bmpDest,x2-1,y2-1,    dark);
}


///////////////////
// Draw an inset box
void Menu_DrawBoxInset(SDL_Surface *bmpDest, int x, int y, int x2, int y2)
{
    Uint32 dark = tLX->clBoxDark;
    Uint32 light = tLX->clBoxLight;

	DrawRect( bmpDest,x+1, y+1, x2-1,y2-1, dark);
	DrawHLine(bmpDest,x+2, x2-1,y,  light);
	DrawHLine(bmpDest,x+2, x2-1,y2, light);

	DrawVLine(bmpDest,y+2, y2-1,x,  light);
	DrawVLine(bmpDest,y+2, y2-1,x2, light);
	PutPixel( bmpDest,x+1, y+1,     light);
	PutPixel( bmpDest,x2-1,y+1,     light);
	PutPixel( bmpDest,x+1, y2-1,    light);
	PutPixel( bmpDest,x2-1,y2-1,    light);
}


///////////////////
// Draw a windows style button
void Menu_DrawWinButton(SDL_Surface *bmpDest, int x, int y, int w, int h, bool down)
{
    DrawRectFill(bmpDest, x,y, x+w, y+h, tLX->clWinBtnBody);
    Uint32 dark = tLX->clWinBtnDark;
    Uint32 light = tLX->clWinBtnLight;
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
int Menu_MessageBox(const std::string& sTitle, const std::string& sText, int type)
{
	int ret = -1;
	keyboard_t *kb = GetKeyboard();
	mouse_t *Mouse = GetMouse();
	int mouse = 0;
	gui_event_t *ev = NULL;

	int x = 160;
	int y = 170;
	int w = 320;
	int h = 140;

	// Handle multiline messages
	int maxwidth = 0;
	int tmp=0;
	static std::vector<std::string> lines = explode(sText,"\n");
	int i;
	for (i=0; i<lines.size(); i++)  {
		maxwidth = MAX(maxwidth,tLX->cFont.GetWidth(lines[i]));
	}

	if(maxwidth > w) {
		w = MIN(maxwidth+30,638);
		x = 320-w/2;
	}

	if((tLX->cFont.GetHeight()*lines.size())+5 > h) {
		h = MIN((tLX->cFont.GetHeight()*lines.size())+20,478);
		y = 240-h/2;
	}

	int cx = x+w/2;
	int cy = y+h/2-(lines.size()*tLX->cFont.GetHeight())/2;


//	SDL_Surface *shadow = LoadImage("data/frontend/msgshadow.png",0);   // TODO: not used


	//
	// Setup the gui
	//
	CGuiLayout msgbox;

	msgbox.Initialize();

	if(type == LMB_OK)
		msgbox.Add( new CButton(BUT_OK,tMenu->bmpButtons), 0, cx-20,y+h-24, 40,15);
	else if(type == LMB_YESNO) {
		msgbox.Add( new CButton(BUT_YES,tMenu->bmpButtons), 1, x+15,y+h-24, 35,15);
		msgbox.Add( new CButton(BUT_NO,tMenu->bmpButtons),  2, x+w-35,y+h-24, 30,15);
	}


	// Store the old buffer into a temp buffer to keep it
	SDL_BlitSurface(tMenu->bmpBuffer, NULL, tMenu->bmpMsgBuffer, NULL);


	// Draw to the buffer
	//DrawImage(tMenu->bmpBuffer, shadow, 177,167);
	Menu_DrawBox(tMenu->bmpBuffer, x, y, x+w, y+h);
	DrawRectFill(tMenu->bmpBuffer, x+2,y+2, x+w-1,y+h-1,0);
	DrawRectFill(tMenu->bmpBuffer, x+2,y+2, x+w-1,y+25,MakeColour(64,64,64));

	tLX->cFont.DrawCentre(tMenu->bmpBuffer, cx, y+5, tLX->clNormalLabel,sTitle);
	for (i=0; (int)i<=lines.size(); i++)  {
		cx = x+w/2;//-(tLX->cFont.GetWidth(lines[i])+30)/2;
		tLX->cFont.DrawCentre(tMenu->bmpBuffer, cx, cy, tLX->clNormalLabel, lines[i]);
		cy += tLX->cFont.GetHeight()+2;
	}

	Menu_RedrawMouse(true);


    // This fixes a problem with the escape/enter key sometimes not functioning, and preventing the game quiting
    kb->KeyDown[SDLK_ESCAPE] = false;
    kb->KeyUp[SDLK_ESCAPE] = false;
    kb->KeyDown[SDLK_RETURN] = false;
    kb->KeyUp[SDLK_RETURN] = false;
    kb->KeyDown[SDLK_KP_ENTER] = false;
    kb->KeyUp[SDLK_KP_ENTER] = false;


	ProcessEvents();
	while(!kb->KeyUp[SDLK_ESCAPE] && tMenu->iMenuRunning && ret == -1) {
		Menu_RedrawMouse(false);
		ProcessEvents();
		mouse = 0;

		DrawImageAdv(tMenu->bmpScreen,tMenu->bmpBuffer, x,y, x,y, w, h);

		// Process the gui
		ev = msgbox.Process();
		msgbox.Draw(tMenu->bmpScreen);

		if(ev) {

			if(ev->cWidget->getType() == wid_Button)
				mouse = 1;
			if(ev->cWidget->getType() == wid_Textbox)
				mouse = 2;

			if(ev->iEventMsg == BTN_MOUSEUP) {
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
		if (kb->KeyUp[SDLK_RETURN] || kb->KeyUp[SDLK_KP_ENTER])
			if (type == LMB_YESNO)  {
				ret = MBR_YES;
				break;
			}
			else  {
				ret = MBR_OK;
				break;
			}



		DrawImage(tMenu->bmpScreen,gfxGUI.bmpMouse[mouse], Mouse->X,Mouse->Y);

		FlipScreen(tMenu->bmpScreen);
	}


	msgbox.Shutdown();

	// Restore the old buffer
	SDL_BlitSurface(tMenu->bmpMsgBuffer, NULL, tMenu->bmpBuffer, NULL);


	return ret;
}

///////////////////
// Add all the default widgets
void Menu_AddDefaultWidgets(void)
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
	class LevelListFiller { public:
		CCombobox* cmb;
		int* index;
		int* selected;
		LevelListFiller(CCombobox* c, int* i, int* s) : cmb(c), index(i), selected(s) {}
		inline bool operator() (const std::string& filename) {
			size_t pos = findLastPathSep(filename);
			std::string f = filename.substr(pos);
	
			// Liero Xtreme level
			if( stringcasecmp(filename.substr(filename.size()-4), ".lxl") == 0) {
				FILE *fp = OpenGameFile(filename,"rb");
				if(fp) {
					static char id[33];
					int version;
					static char name[65];
					fread(id,		sizeof(char),	32,	fp);
					fread(&version,	sizeof(int),	1,	fp);
					fread(name,		sizeof(char),	64,	fp);
					id[32] = '\0';
					name[64] = '\0';
					
					if(strcmp(id,"LieroX Level") == 0 && version == MAP_VERSION) {
						// Remove the 'levels' bit from the filename
						if(!cmb->getItem(name)) {
							cmb->addItem((*index)++, f, name);
	
							// If this is the same as the old map, select it
							if( f == tLXOptions->tGameinfo.sMapName )
								*selected = *index-1;
						}
					}
					fclose(fp);
				}
			}
	
			// Liero level
			if( stringcasecmp(filename.substr(filename.size()-4), ".lev") == 0) {
				FILE *fp = OpenGameFile(filename,"rb");
	
				if(fp) {	
					// Make sure it's the right size to be a liero level
					fseek(fp,0,SEEK_END);
					// 176400 is liero maps
					// 176402 is worm hole maps (same, but 2 bytes bigger)
					// 177178 is a powerlevel
					if( ftell(fp) == 176400 || ftell(fp) == 176402 || ftell(fp) == 177178) {
	
						if(!cmb->getItem(f)) {
							cmb->addItem((*index)++, f, f);
		
							if( f == tLXOptions->tGameinfo.sMapName )
								*selected = *index-1;
						}
					}
	
					fclose(fp);
				}
			}
			// TODO: this was a bit wired, that the above stuff was
			//     commented out and used was the following code.
			//     this seems to be wrong. but why was it so?
			/*std::string name = Menu_GetLevelName(filename);
			if (name == tLXOptions->tGameinfo.sMapName)
				*selected = *index;
	
			(*index)++;	*/
			return true;
		}
	};


///////////////////
// Fill a listbox with the levels
void Menu_FillLevelList(CCombobox *cmb, int random)
{
	int		index = 0;
	int		selected = -1;

	cmb->clear();

	// If random is true, we add the 'random' level to the list
	if(random) {
		cmb->addItem(index++, "_random_", "- Random level -");
		if( tLXOptions->tGameinfo.sMapName == "_random_" )
			selected = index-1;
	}

	FindFiles(LevelListFiller(cmb, &index, &selected), "levels", FM_REG);
	
	// Sort it ascending
	cmb->Sort(true);
	
	if( selected >= 0 )
		cmb->setCurItem( selected );
}


///////////////////
// Redraw a section from the buffer to the screen
void Menu_redrawBufferRect(int x, int y, int w, int h)
{
    DrawImageAdv(tMenu->bmpScreen, tMenu->bmpBuffer, x,y, x,y, w,h);
}




/*
============================

	Server list functions

============================
*/


server_t *psServerList = NULL;

// Maximum number of pings/queries before we ignore the server
int		MaxPings = 4;
int		MaxQueries = MAX_QUERIES;

// Time to wait before pinging/querying the server again (in seconds)
float	PingWait = 1;
float	QueryWait = 1;



///////////////////
// Clear the server list
void Menu_SvrList_Clear(void)
{
	Menu_SvrList_Shutdown();
}


///////////////////
// Clear any servers automatically added
void Menu_SvrList_ClearAuto(void)
{
    server_t *s = psServerList;
    server_t *sn = NULL;

    for(; s; s=sn) {
        sn = s->psNext;

        if(!s->bManual) {
            // Unlink the server
            if( s->psPrev )
                s->psPrev->psNext = s->psNext;
            else
                psServerList = s->psNext;

            if( s->psNext )
                s->psNext->psPrev = s->psPrev;

            // Free it
            delete s;
        }
    }
}


///////////////////
// Shutdown the server list
void Menu_SvrList_Shutdown(void)
{
	server_t *s = psServerList;
	server_t *sn = NULL;

	for(; s; s=sn ) {
		sn=s->psNext;

		delete s;
	}

	psServerList = NULL;
}


///////////////////
// Send a ping out to the LAN (LAN menu)
void Menu_SvrList_PingLAN(void)
{
	// Broadcast a ping on the LAN
	CBytestream bs;
	bs.Clear();
	bs.writeInt(-1,4);
	bs.writeString("%s","lx::ping");

	NetworkAddr a;
	StringToNetAddr("255.255.255.255",&a);
	SetNetAddrPort(&a,tLXOptions->iNetworkPort);
	SetRemoteNetAddr(tMenu->tSocket[SCK_LAN],&a);

	// Send the ping
	bs.Send(tMenu->tSocket[SCK_LAN]);

	// Try also the default LX port
	SetNetAddrPort(&a,LX_PORT);

	bs.Send(tMenu->tSocket[SCK_LAN]);
}


///////////////////
// Ping a server
void Menu_SvrList_PingServer(server_t *svr)
{
	SetRemoteNetAddr(tMenu->tSocket[SCK_NET], &svr->sAddress);

	CBytestream bs;
	bs.writeInt(-1,4);
	bs.writeString("%s","lx::ping");
	bs.Send(tMenu->tSocket[SCK_NET]);

	svr->bProcessing = true;
	svr->nPings++;
	svr->fLastPing = tLX->fCurTime;
}

///////////////////
// Send Wants Join message
void Menu_SvrList_WantsJoin(const std::string& Nick, server_t *svr)
{
	SetRemoteNetAddr(tMenu->tSocket[SCK_NET], &svr->sAddress);

	CBytestream bs;
	bs.writeInt(-1,4);
	bs.writeString("%s","lx::wantsjoin");
	bs.writeString("%s",Nick.c_str());
	bs.Send(tMenu->tSocket[SCK_NET]);
}


///////////////////
// Query a server
void Menu_SvrList_QueryServer(server_t *svr)
{
	SetRemoteNetAddr(tMenu->tSocket[SCK_NET], &svr->sAddress);

	CBytestream bs;
	bs.writeInt(-1,4);
	bs.writeString("%s","lx::query");
    bs.writeByte(svr->nQueries);
	bs.Send(tMenu->tSocket[SCK_NET]);
    svr->fQueryTimes[svr->nQueries] = tLX->fCurTime;

	svr->bProcessing = true;
	svr->nQueries++;
	svr->fLastQuery = tLX->fCurTime;
}


///////////////////
// Refresh the server list (Internet menu)
void Menu_SvrList_RefreshList(void)
{
	// Set all the servers to be pinged
	server_t *s = psServerList;
	for(; s; s=s->psNext) {

        Menu_SvrList_RefreshServer(s);
	}
}


///////////////////
// Refresh a single server
void Menu_SvrList_RefreshServer(server_t *s)
{
    s->bProcessing = true;
	s->bgotPong = false;
	s->bgotQuery = false;
	s->bIgnore = false;
	s->fLastPing = -9999;
	s->fLastQuery = -9999;
	s->nPings = 0;
	s->nQueries = 0;
	s->nPing = 0;
}


///////////////////
// Add a server onto the list (for list and manually)
server_t *Menu_SvrList_AddServer(const std::string& address, bool bManual)
{
    // Check if the server is already in the list
    // If it is, don't bother adding it
    server_t *sv = psServerList;
    NetworkAddr ad;
	std::string tmp_address = address;
    TrimSpaces(tmp_address);
    StringToNetAddr(tmp_address, &ad);

    for(; sv; sv=sv->psNext) {
        if( AreNetAddrEqual(&sv->sAddress, &ad) )
            return sv;
    }

    // Didn't find one, so create it
	server_t *svr = new server_t;
	if(svr == NULL)
		return NULL;


	// Fill in the details
    svr->bManual = bManual;
	svr->bProcessing = true;
	svr->bgotPong = false;
	svr->bgotQuery = false;
	svr->bIgnore = false;
	svr->fLastPing = -9999;
	svr->fLastQuery = -9999;
	svr->nPings = 0;
	svr->nQueries = 0;
    svr->psNext = NULL;
    svr->psPrev = NULL;
	svr->szAddress = tmp_address;

	// If the address doesn't have a port number, use the default lierox port number
	if( svr->szAddress.rfind(':') == std::string::npos) {
		svr->szAddress += ":"+itoa(LX_PORT,10);
	}

	StringToNetAddr(tmp_address, &svr->sAddress);


	// Default game details
	svr->szName = "Untitled";
	svr->nMaxPlayers = 0;
	svr->nNumPlayers = 0;
	svr->nState = 0;
	svr->nPing = 0;


	// If the address doesn't have a port number set, use the default lierox port number
	if(GetNetAddrPort(&svr->sAddress) == 0)
		SetNetAddrPort(&svr->sAddress, LX_PORT);


	// Link it in at the end of the list
    sv = psServerList;
    for(; sv; sv=sv->psNext) {
        if( sv->psNext == NULL ) {
            sv->psNext = svr;
            svr->psPrev = sv;
            break;
        }
    }

    if( !psServerList )
        psServerList = svr;

	return svr;
}

///////////////////
// Add a server onto the list and specify the name
server_t *Menu_SvrList_AddNamedServer(const std::string& address, const std::string& name)
{
    // Check if the server is already in the list
    // If it is, don't bother adding it
    server_t *sv = psServerList;
    NetworkAddr ad;
	std::string tmp_address = address;
    TrimSpaces(tmp_address);
    StringToNetAddr(tmp_address, &ad);

    for(; sv; sv=sv->psNext) {
        if( AreNetAddrEqual(&sv->sAddress, &ad) )
            return sv;
    }

    // Didn't find one, so create it
	server_t *svr = new server_t;
	if(svr == NULL)
		return NULL;


	// Fill in the details
    svr->bManual = true;
	svr->bProcessing = true;
	svr->bgotPong = false;
	svr->bgotQuery = false;
	svr->bIgnore = false;
	svr->fLastPing = -9999;
	svr->fLastQuery = -9999;
	svr->nPings = 0;
	svr->nQueries = 0;
    svr->psNext = NULL;
    svr->psPrev = NULL;
	svr->szName = name;
	svr->szAddress = tmp_address;

	// If the address doesn't have a port number, use the default lierox port number
	if( svr->szAddress.rfind(':') == std::string::npos) {
		svr->szAddress += ":"+itoa(LX_PORT,10);
	}

	StringToNetAddr(tmp_address, &svr->sAddress);


	// Default game details
	svr->nMaxPlayers = 0;
	svr->nNumPlayers = 0;
	svr->nState = 0;
	svr->nPing = 0;


	// If the address doesn't have a port number set, use the default lierox port number
	if(GetNetAddrPort(&svr->sAddress) == 0)
		SetNetAddrPort(&svr->sAddress, LX_PORT);


	// Link it in at the end of the list
    sv = psServerList;
    for(; sv; sv=sv->psNext) {
        if( sv->psNext == NULL ) {
            sv->psNext = svr;
            svr->psPrev = sv;
            break;
        }
    }

    if( !psServerList )
        psServerList = svr;

	return svr;
}


///////////////////
// Remove a server from the server list
void Menu_SvrList_RemoveServer(const std::string& szAddress)
{
    server_t *sv = Menu_SvrList_FindServerStr(szAddress);
    if( !sv )
        return;

    // Unlink the server
    if( sv->psPrev )
        sv->psPrev->psNext = sv->psNext;
    else
        psServerList = sv->psNext;

    if( sv->psNext )
        sv->psNext->psPrev = sv->psPrev;

    // Free it
    delete sv;
}


///////////////////
// Find a server based on a string address
server_t *Menu_SvrList_FindServerStr(const std::string& szAddress)
{
    // Find a matching server
    server_t *sv = psServerList;
    for(; sv; sv=sv->psNext) {

        if( sv->szAddress == szAddress)
            return sv;
    }

    // Not found
    return NULL;
}


///////////////////
// Fill a listview box with the server list
void Menu_SvrList_FillList(CListview *lv)
{
	server_t	*s = psServerList;
	std::string		addr;
	static const char*	states[] = {"Open", "Loading", "Playing"};

    // Store the ID of the currently selected item
    int curID = lv->getSelectedID();

	lv->SaveScrollbarPos();
	lv->Clear();

	for(; s; s=s->psNext) {

		// Ping Image
		int num = 3;
		if(s->nPing < 700)  num=2;
		if(s->nPing < 400)  num=1;
		if(s->nPing < 200)  num=0;

		if(s->bIgnore || s->bProcessing)
			num = 3;

		// Address
		//GetRemoteNetAddr(tMenu->tSocket, &s->sAddress);
		//NetAddrToString(&s->sAddress, addr);

		// Remove the port from the address (save space)
		addr = s->szAddress;
		int p = addr.rfind(':');
		if(p != std::string::npos)
			addr = addr.substr(0,p);


		// State
		int state = 0;
		if(s->nState >= 0 && s->nState < 3)
			state = s->nState;

		// Colour
		int colour = tLX->clListView;
		if(s->bProcessing)
			colour = tLX->clDisabled;


		// Add the server to the list
		lv->AddItem(s->szAddress, 0, colour);
		lv->AddSubitem(LVS_IMAGE, itoa(num,10), tMenu->bmpConnectionSpeeds[num]);
		lv->AddSubitem(LVS_TEXT, s->szName, NULL);
        if(s->bProcessing)
            lv->AddSubitem(LVS_TEXT, "querying", NULL);
        else if(num == 3)
            lv->AddSubitem(LVS_TEXT, "down", NULL);
        else
		    lv->AddSubitem(LVS_TEXT, states[state], NULL);

        if(num == 3)
            continue;

		// Players
		lv->AddSubitem(LVS_TEXT, itoa(s->nNumPlayers,10)+"/"+itoa(s->nMaxPlayers,10), NULL);

		lv->AddSubitem(LVS_TEXT, itoa(s->nPing,10), NULL);
		lv->AddSubitem(LVS_TEXT, addr, NULL);
	}

    lv->setSelectedID(curID);
	lv->RestoreScrollbarPos();
	lv->ReSort();
}

///////////////////
// Process the network connection
// Returns true if a server in the list was added/modified
bool Menu_SvrList_Process(void)
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



	// Ping or Query any servers in the list that need it
	server_t *s = psServerList;
	for(; s; s=s->psNext) {

		// Ignore this server? (timed out)
		if(s->bIgnore)
			continue;

		// Need a pingin'?
		if(!s->bgotPong) {
			if(tLX->fCurTime - s->fLastPing > PingWait) {

				if(s->nPings >= MaxPings) {
					s->bIgnore = true;
					update = true;
				}
				else
					// Ping the server
					Menu_SvrList_PingServer(s);
			}
		}

		// Need a querying?
		if(s->bgotPong && !s->bgotQuery) {
			if(tLX->fCurTime - s->fLastQuery > QueryWait) {

				if(s->nQueries >= MaxQueries) {
					s->bIgnore = true;
					update = true;
				}
				else
					// Query the server
					Menu_SvrList_QueryServer(s);
			}
		}

		// If we are ignoring this server now, set it to not processing
		if(s->bIgnore) {
			s->bProcessing = false;
			update = true;
		}

	}

	return update;
}


///////////////////
// Parse a packet
// Returns true if we should update the list
int Menu_SvrList_ParsePacket(CBytestream *bs, NetworkSocket sock)
{
	NetworkAddr		adrFrom;
	int				update = false;
	static std::string cmd,buf;

	// Check for connectionless packet header
	if(*(int *)bs->GetData() == -1) {
		bs->SetPos(4);
		cmd = bs->readString();

		GetRemoteNetAddr(sock,&adrFrom);

		// Check for a pong
		if(cmd == "lx::pong") {

			// Look the the list and find which server returned the ping
			server_t *svr = Menu_SvrList_FindServer(&adrFrom);
			if( svr ) {

				// It pinged, so fill in the ping info so it will now be queried
				svr->bgotPong = true;
				svr->nQueries = 0;
			} else {

				// If we didn't ping this server directly (eg, subnet), add the server to the list
				// HINT: in favourites list, only user should add servers
				if (iNetMode != net_favourites)  {
					NetAddrToString( &adrFrom, buf );
					svr = Menu_SvrList_AddServer(buf, false);

					if( svr ) {

						// Only update the list if this is the first ping
						if(!svr->bgotPong)
							update = true;

						// Set it the ponged
						svr->bgotPong = true;
						svr->nQueries = 0;
					}
				}
			}
		}

		// Check for a query return
		else if(cmd == "lx::queryreturn") {

			// Look the the list and find which server returned the ping
			server_t *svr = Menu_SvrList_FindServer(&adrFrom);
			if( svr ) {

				// Only update the list if this is the first query
				if(!svr->bgotQuery)
					update = true;

				svr->bgotQuery = true;
				Menu_SvrList_ParseQuery(svr, bs);
			}

			// If we didn't query this server, then we should ignore it
		}
	}

	return update;
}


///////////////////
// Find a server from the list by address
server_t *Menu_SvrList_FindServer(NetworkAddr *addr)
{
	server_t *s = psServerList;

	for(; s; s=s->psNext) {

		StringToNetAddr(s->szAddress, &s->sAddress);

		if( AreNetAddrEqual( addr, &s->sAddress ) )
			return s;
	}

	// None found
	return NULL;
}


///////////////////
// Parse the server query return packet
void Menu_SvrList_ParseQuery(server_t *svr, CBytestream *bs)
{
	// Don't update the name in favourites
	static char buf[64];
	bs->readString( buf, sizeof(buf) );
	if(iNetMode != net_favourites)
		svr->szName = buf;
	svr->nNumPlayers = bs->readByte();
	svr->nMaxPlayers = bs->readByte();
	svr->nState = bs->readByte();
    int num = bs->readByte();
	svr->bProcessing = false;

    if(num < 0 || num >= MAX_QUERIES-1)
        num=0;

	svr->nPing = (int)( (tLX->fCurTime - svr->fQueryTimes[num])*1000.0f );

	if(svr->nPing < 0)
		svr->nPing = 999;
    if(svr->nPing > 999)
        svr->nPing = 999;
}


///////////////////
// Save the server list
void Menu_SvrList_SaveList(const std::string& szFilename)
{
    FILE *fp = OpenGameFile(szFilename,"wt");
    if( !fp )
        return;

    server_t *s = psServerList;
	for(; s; s=s->psNext) {
        fprintf(fp,"%s, %s, %s\n",s->bManual ? "1" : "0", s->szName.c_str(), s->szAddress.c_str());
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

    static std::string szLine = "";
	static char tmp[1024] = "";
    static std::string address = "";
	std::vector<std::string> parsed;

    // Go through every line
    while( fgets(tmp, 1024, fp) ) {

        szLine = StripLine(tmp);
        if( szLine == "" )
            continue;
	
		parsed = explode(szLine,",");

        if( parsed.size() == 3 ) {
			address = parsed[2];
			TrimSpaces(parsed[2]);

            server_t *sv = Menu_SvrList_AddServer(address, parsed[0][0] == '1');

            // Fill in the name
            if( sv ) {
				TrimSpaces(parsed[1]);
                sv->szName = parsed[1];
            }
        }
    }

    fclose(fp);
}



bool bGotDetails = false;
bool bOldLxBug = false;
int nTries = 0;
float fStart = -9999;

///////////////////
// Draw a 'server info' box
void Menu_SvrList_DrawInfo(const std::string& szAddress)
{
	int w = 450;
	int h = 420;
	int y = tMenu->bmpBuffer->h/2 - h/2;
	int x = tMenu->bmpBuffer->w/2 - w/2;

	Menu_redrawBufferRect(x,y,w,h);

    Menu_DrawBox(tMenu->bmpScreen, x,y, w,h);
	DrawRectFillA(tMenu->bmpScreen, x+1,y+1, w-1,h-1, 0, 230);
    tLX->cFont.DrawCentre(tMenu->bmpScreen, x+w/2-tLX->cFont.GetWidth("Server Details")/2, y+5, tLX->clNormalLabel,"%s", "Server Details");


    // Get the server details
	// NOTE: must be static else doesn't work
    static std::string    szName;
    static int     nMaxWorms;
    static int     nState;
    static std::string    szMapName;
    static std::string    szModName;
    static int     nGameMode;
    static int     nLives;
    static int     nMaxKills;
    static int     nLoadingTime;
    static int     nBonuses;
    static int     nNumPlayers;
    static CWorm   cWorms[MAX_WORMS];

    CBytestream inbs;
    NetworkAddr   addr;

    if(nTries < 3 && !bGotDetails && !bOldLxBug) {

		tLX->cFont.DrawCentre(tMenu->bmpScreen, x+w/2-tLX->cFont.GetWidth("Loading info...")/2, y+h/2-8, tLX->clNormalLabel, "%s", "Loading info...");

        if (inbs.Read(tMenu->tSocket[SCK_NET])) {
            // Check for connectionless packet header
	        if(*(int *)inbs.GetData() == -1) {
                static std::string        cmd;

		        inbs.SetPos(4);
		        cmd = inbs.readString();


		        GetRemoteNetAddr(tMenu->tSocket[SCK_NET],&addr);

		        // Check for server info
		        if(cmd == "lx::serverinfo") {
                    bGotDetails = true;

                    // Read the info
                    szName = inbs.readString(64);
	                nMaxWorms = MIN(MAX_PLAYERS,MAX((int)inbs.readByte(),0));
	                nState = inbs.readByte();

					if (nState < 0)  {
						bOldLxBug = true;
					}

                    szMapName = inbs.readString(256);
                    szModName = inbs.readString(256);
	                nGameMode = inbs.readByte();
	                nLives = inbs.readShort();
	                nMaxKills = inbs.readShort();
	                nLoadingTime = inbs.readShort();
					if(nLoadingTime < 0 || nLoadingTime > 500)  {
						bOldLxBug = true;
					}
                    nBonuses = inbs.readByte();

                    // Worms
                    nNumPlayers = inbs.readByte();
					if (nNumPlayers <= 0)  {
						bOldLxBug = true;
					}

					// Check
					nNumPlayers = MIN(nMaxWorms,nNumPlayers);

                    for(int i=0; i<nNumPlayers; i++) {
                        cWorms[i].setName(inbs.readString());
                        cWorms[i].setKills(inbs.readInt(2));
                    }
                }
            }
        }

        if((tLX->fCurTime - fStart > 1) && !bGotDetails) {
            nTries++;
            fStart = tLX->fCurTime;
			bGotDetails = false;
			bOldLxBug = false;

            // Send a getinfo request
			std::string tmp_addr = szAddress;
            TrimSpaces(tmp_addr);
            StringToNetAddr(tmp_addr, &addr);

            SetRemoteNetAddr(tMenu->tSocket[SCK_NET], &addr);

	        CBytestream bs;
	        bs.writeInt(-1,4);
	        bs.writeString("%s","lx::getinfo");
	        bs.Send(tMenu->tSocket[SCK_NET]);
        }

		if (!bGotDetails)  {
			return;
		}
    }

	y+=25;
	x+=15;
    // Draw the server details
    tLX->cFont.Draw(tMenu->bmpScreen, x,y, tLX->clNormalLabel,"%s", "Server name:");
    tLX->cFont.Draw(tMenu->bmpScreen, x,y+20, tLX->clNormalLabel,"%s", "Level name:");
    tLX->cFont.Draw(tMenu->bmpScreen, x,y+40, tLX->clNormalLabel,"%s", "Mod name:");
    tLX->cFont.Draw(tMenu->bmpScreen, x,y+60, tLX->clNormalLabel,"%s", "State:");
    tLX->cFont.Draw(tMenu->bmpScreen, x,y+80, tLX->clNormalLabel,"%s", "Playing:");

    tLX->cFont.Draw(tMenu->bmpScreen, x,y+100, tLX->clNormalLabel,"%s", "Game Type:");
    tLX->cFont.Draw(tMenu->bmpScreen, x,y+120, tLX->clNormalLabel,"%s", "Lives:");
    tLX->cFont.Draw(tMenu->bmpScreen, x,y+140, tLX->clNormalLabel,"%s", "Max Kills:");
    tLX->cFont.Draw(tMenu->bmpScreen, x,y+160, tLX->clNormalLabel,"%s", "Loading Times:");
    tLX->cFont.Draw(tMenu->bmpScreen, x,y+180, tLX->clNormalLabel,"%s", "Bonuses:");
	if (!nState)  // Dont show kills when the server is open
		tLX->cFont.Draw(tMenu->bmpScreen, x,y+200, tLX->clNormalLabel,"%s", "Players:");
	else
		tLX->cFont.Draw(tMenu->bmpScreen, x,y+200, tLX->clNormalLabel,"%s", "Players/Kills:");

	x+=110;

    if(!bGotDetails) {
        tLX->cFont.Draw(tMenu->bmpScreen, x,y, tLX->clError,"%s", "Unable to query server");
        return;
    }

    if(bOldLxBug) {
        tLX->cFont.Draw(tMenu->bmpScreen, x,y, tLX->clError,"%s", "You can't view details\nof this server because\nLieroX v0.56 contains a bug.\n\nPlease wait until the server\nchanges its state to Playing\nand try again.");
        return;
    }

   	static const std::string states[] = {"Open", "Loading", "Playing", "Unknown"};
    static const std::string gamemodes[] = {"Deathmatch","Team Deathmatch", "Tag", "Demolitions", "Unknown"};
    if(nState < 0 || nState > 2)
        nState = 3;
    if(nGameMode < 0 || nGameMode > 3)
        nGameMode = 4;

    tLX->cFont.Draw(tMenu->bmpScreen, x,y, tLX->clNormalLabel,  szName);
    tLX->cFont.Draw(tMenu->bmpScreen, x,y+20, tLX->clNormalLabel, szMapName);
    tLX->cFont.Draw(tMenu->bmpScreen, x,y+40, tLX->clNormalLabel,  szModName);
    tLX->cFont.Draw(tMenu->bmpScreen, x,y+60, tLX->clNormalLabel,  states[nState]);
    tLX->cFont.Draw(tMenu->bmpScreen, x,y+80, tLX->clNormalLabel, "%d / %d", nNumPlayers, nMaxWorms);

    tLX->cFont.Draw(tMenu->bmpScreen, x,y+100, tLX->clNormalLabel,  gamemodes[nGameMode]);
    tLX->cFont.Draw(tMenu->bmpScreen, x,y+120, tLX->clNormalLabel, "%d", nLives);
    tLX->cFont.Draw(tMenu->bmpScreen, x,y+140, tLX->clNormalLabel, "%d", nMaxKills);
    tLX->cFont.Draw(tMenu->bmpScreen, x,y+160, tLX->clNormalLabel, "%d", nLoadingTime);
    tLX->cFont.Draw(tMenu->bmpScreen, x,y+180, tLX->clNormalLabel, "%s", nBonuses ? "on" : "off");

	// Don't draw kills when the server is open
	if(!nState)
		for (int i=0; i<nNumPlayers; i++)
			tLX->cFont.Draw(tMenu->bmpScreen, x,y+200+i*18, tLX->clNormalLabel, cWorms[i].getName());
	else
		for (int i=0; i<nNumPlayers; i++)  {
			tLX->cFont.Draw(tMenu->bmpScreen, x,y+200+i*18, tLX->clNormalLabel, cWorms[i].getName());
			tLX->cFont.Draw(tMenu->bmpScreen, x+150,y+200+i*18, tLX->clNormalLabel, "%d", cWorms[i].getKills());
		}

}
