/////////////////////////////////////////
//
//             Liero Xtreme
//
//     Copyright Auxiliary Software 2002
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


int		*iGame = NULL;
int		iSkipStart = false;


///////////////////
// Initialize the menu system
int Menu_Initialize(int *game)
{
	iGame = game;
	*iGame = false;
	iJoin_Recolorize = true;
	iHost_Recolorize = true;


	// Allocate the menu structure
	tMenu = new menu_t;
    if(tMenu == NULL) {
        SystemError("Error: Out of memory in for menu");
		return false;
    }

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

    tMenu->bmpWorm = NULL;
	tMenu->bmpScreen = SDL_GetVideoSurface();

	// Open a socket for broadcasting over a LAN (UDP)	
	tMenu->tSocket[SCK_LAN] = nlOpen(0, NL_BROADCAST);
	// Open a socket for communicating over the net (UDP)	
	tMenu->tSocket[SCK_NET] = nlOpen(0, NL_UNRELIABLE);

	if(tMenu->tSocket[SCK_LAN] == NL_INVALID || tMenu->tSocket[SCK_NET] == NL_INVALID) {
		SystemError("Error: Failed to open a socket for networking");
		return false;
	}

	return true;
}


///////////////////
// Shutdown the menu
void Menu_Shutdown(void)
{
	if(tMenu) {
		
		// Manually free some items
		if(tMenu->bmpBuffer)
			SDL_FreeSurface(tMenu->bmpBuffer);

		if(tMenu->bmpMsgBuffer)
			SDL_FreeSurface(tMenu->bmpMsgBuffer);

        if(tMenu->bmpMiniMapBuffer)
			SDL_FreeSurface(tMenu->bmpMiniMapBuffer);

		if(tMenu->tSocket[SCK_LAN] != NL_INVALID)
			nlClose(tMenu->tSocket[SCK_LAN]);
		if(tMenu->tSocket[SCK_NET] != NL_INVALID)
			nlClose(tMenu->tSocket[SCK_NET]);

		tMenu->tSocket[SCK_LAN] = NL_INVALID;
		tMenu->tSocket[SCK_NET] = NL_INVALID;

		// The rest get free'd in the cache
		assert(tMenu);
		delete tMenu;
		tMenu = NULL;
	}

	Menu_SvrList_Shutdown();

	Menu_MainShutdown();
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
	keyboard_t *kb = GetKeyboard();
	mouse_t *mouse = GetMouse();

	tLX->fCurTime = GetMilliSeconds();
	float oldtime = tLX->fCurTime;

	while(tMenu->iMenuRunning) {
		Menu_RedrawMouse(false);
		ProcessEvents();

		tLX->fCurTime = GetMilliSeconds();
		tLX->fDeltaTime = tLX->fCurTime - oldtime;
		oldtime = tLX->fCurTime;

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
char *Menu_GetLevelName(char *filename)
{
	char	id[32], name[256];
	int		version;
	int		index = 0;
	int		selected = -1;
	char	*Result;
	char	fn[1024];
	char	*Path;

	if(strlen(filename) > 512)
		return NULL;

	sprintf(&fn[0],"%s%s","levels/",filename);
	fn[1023] = '\0';
	Path = fn;

 // Liero Xtreme level
	if( stricmp(filename + strlen(filename)-4, ".lxl") == 0) {
		FILE *fp = fopen_i(Path,"rb");
		if(fp) {
			fread(id,		sizeof(char),	32,	fp);
			fread(&version,	sizeof(int),	1,	fp);
			fread(name,		sizeof(char),	64,	fp);

			if(strcmp(id,"LieroX Level") == 0 && version == MAP_VERSION) {
				name[255] = '\0'; // safety
				Result = name;
				return Result;
			}
			fclose(fp);
		}
		return NULL;
	}

 // Liero level
	if( stricmp(filename + strlen(filename)-4, ".lev") == 0) {
		FILE *fp = fopen_i(filename,"rb");
			
		if(fp) {

			// Make sure it's the right size to be a liero level
			fseek(fp,0,SEEK_END);
			// 176400 is liero maps
			// 176402 is worm hole maps (same, but 2 bytes bigger)
			// 177178 is a powerlevel
			if( ftell(fp) == 176400 || ftell(fp) == 176402 || ftell(fp) == 177178) {
				char *f = MAX(strrchr(filename,'\\'),strrchr(filename,'/'));
				if(f)
					return f+1;
			}
			fclose(fp);
		} // if(fp)
	}
  return NULL;
}


///////////////////
// Draw a box
void Menu_DrawBox(SDL_Surface *bmpDest, int x, int y, int x2, int y2)
{
    Uint32 dark = MakeColour(60,60,60);
    Uint32 light = MakeColour(130,130,130);

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
    Uint32 dark = MakeColour(60,60,60);
    Uint32 light = MakeColour(130,130,130);

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
    DrawRectFill(bmpDest, x,y, x+w, y+h, MakeColour(128,128,128));
    Uint32 dark = MakeColour(64,64,64);
    Uint32 light = MakeColour(192,192,192);
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
int Menu_MessageBox(char *sTitle, char *sText, int type)
{
	int ret = -1;
	keyboard_t *kb = GetKeyboard();
	mouse_t *Mouse = GetMouse();
	int mouse = 0;
	gui_event_t *ev;

	int x = 160;
	int y = 170;
	int w = 320;
	int h = 140;

	// Handle multiline messages
	int linescount = 0;  // contains the count of lines - 1 !!
	int maxwidth = 0;
	int maxwidthline = 0;
	char lines[8][512];
	int j = 0;
	unsigned int i;
	for (i=0; i<strlen(sText) && linescount < 8; i++)  {
		if (*(sText+i) == '\n')  { // linebreak
			lines[linescount][j] = '\0';  // add terminating character at the end of line

			// Get the widest line
			if (j>maxwidth)  {
				maxwidth = j;
				maxwidthline = linescount;
			}

			linescount++;  // increase number of lines

			j=0;
			continue;
		}
		lines[linescount][j] = *(sText+i);
		j++;
	}
	lines[linescount][j] = '\0';

	if(tLX->cFont.GetWidth(lines[maxwidthline])+10 > w) {
		w = tLX->cFont.GetWidth(lines[maxwidthline])+30;
		x = 320-w/2;
	}

	if((tLX->cFont.GetHeight()*linescount)+5 > h) {
		h = (tLX->cFont.GetHeight()*linescount)+20;
		y = 240-h/2;
	}

	int cx = x+w/2;
	int cy = y+h/2-(linescount*tLX->cFont.GetHeight())/2;


	SDL_Surface *shadow = LoadImage("data/frontend/msgshadow.png",0);


	//
	// Setup the gui
	//
	CGuiLayout msgbox;

	msgbox.Initialize();

	if(type == LMB_OK)
		msgbox.Add( new CButton(BUT_OK,tMenu->bmpButtons), 0, cx-20,y+h-24, 40,15);
	if(type == LMB_YESNO) {
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

	tLX->cFont.DrawCentre(tMenu->bmpBuffer, cx, y+5, 0xffff,"%s", sTitle);
	for (i=0; (int)i<=linescount; i++)  {
		cx = x+w/2;//-(tLX->cFont.GetWidth(lines[i])+30)/2;
		tLX->cFont.DrawCentre(tMenu->bmpBuffer, cx, cy, 0xffff,"%s", lines[i]);
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
	while(!kb->KeyUp[SDLK_ESCAPE] && ret == -1) {
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
// Fill a listbox with the levels
void Menu_FillLevelList(CCombobox *cmb, int random)
{
	char	filename[256];
	char	id[32], name[64];
	int		version;
	int		index = 0;
	int		selected = -1;

	// If random is true, we add the 'random' level to the list
	if(random)
		cmb->addItem(index++, "_random_", "Random level");


	// Load the level list
	int done = false;
	if(!FindFirst("levels","*",filename))
		done = true;

	while(!done) {

		// Liero Xtreme level
		if( stricmp(filename + strlen(filename)-4, ".lxl") == 0) {
			FILE *fp = fopen_i(filename,"rb");
			if(fp) {
				 fread(id,			sizeof(char),	32,	fp);
				fread(&version,	sizeof(int),	1,	fp);
				fread(name,		sizeof(char),	64,	fp);

				if(strcmp(id,"LieroX Level") == 0 && version == MAP_VERSION) {
					// Remove the 'levels' bit from the filename
					char *f = MAX(strrchr(filename,'\\'),strrchr(filename,'/'));
					if(f) {
						cmb->addItem(index++, f+1, name);

						// If this is the same as the old map, select it
						if( stricmp(f+1, tLXOptions->tGameinfo.sMapName) == 0 )
							selected = index-1;

						//lv->AddItem(f+1,0);
						//lv->AddSubitem(LVS_TEXT,name,NULL);
					}
				}
				fclose(fp);
			}
		}

		// Liero level
		if( stricmp(filename + strlen(filename)-4, ".lev") == 0) {
			FILE *fp = fopen_i(filename,"rb");
			
			if(fp) {

				// Make sure it's the right size to be a liero level
				fseek(fp,0,SEEK_END);
				// 176400 is liero maps
				// 176402 is worm hole maps (same, but 2 bytes bigger)
				// 177178 is a powerlevel
				if( ftell(fp) == 176400 || ftell(fp) == 176402 || ftell(fp) == 177178) {

					char *f = MAX(strrchr(filename,'\\'),strrchr(filename,'/'));
					if(f) {
						cmb->addItem(index++, f+1, f+1);

						//d_printf("Original level = %s\n",f+1);
						//d_printf("Saved level = %s\n",tLXOptions->tGameinfo.sMapName);

						if( stricmp(f+1, tLXOptions->tGameinfo.sMapName) == 0 )
							selected = index-1;

						//lv->AddItem(f+1,0);
						//lv->AddSubitem(LVS_TEXT,f+1,NULL);
					}
				}

				fclose(fp);
			}
		}

		if(!FindNext(filename))
			break;
	}

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

	char addr[] = {"255.255.255.255"}; 
	NLaddress a;
	nlStringToAddr(addr,&a);
	nlSetAddrPort(&a,LX_PORT);
	nlSetRemoteAddr(tMenu->tSocket[SCK_LAN],&a);

	// Send the ping
	bs.Send(tMenu->tSocket[SCK_LAN]);
}


///////////////////
// Ping a server
void Menu_SvrList_PingServer(server_t *svr)
{
	nlSetRemoteAddr(tMenu->tSocket[SCK_NET], &svr->sAddress);
	
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
void Menu_SvrList_WantsJoin(char *Nick, server_t *svr)
{
	nlSetRemoteAddr(tMenu->tSocket[SCK_NET], &svr->sAddress);
	
	CBytestream bs;
	bs.writeInt(-1,4);
	bs.writeString("%s","lx::wantsjoin");
	bs.writeString("%s",Nick);
	bs.Send(tMenu->tSocket[SCK_NET]);
}


///////////////////
// Query a server
void Menu_SvrList_QueryServer(server_t *svr)
{
	nlSetRemoteAddr(tMenu->tSocket[SCK_NET], &svr->sAddress);
	
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
server_t *Menu_SvrList_AddServer(char *address, bool bManual)
{
    // Check if the server is already in the list
    // If it is, don't bother adding it
    server_t *sv = psServerList;
    NLaddress ad;
    TrimSpaces(address);
    nlStringToAddr(address, &ad);

    for(; sv; sv=sv->psNext) {
        if( nlAddrCompare(&sv->sAddress, &ad) )
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

	strcpy( svr->szAddress, address );

	// If the address doesn't have a port number, use the default lierox port number
	if( strrchr(svr->szAddress,':') == NULL) {
		char buf[64];
		strcat(svr->szAddress,":");
		strcat(svr->szAddress,itoa(LX_PORT,buf,10));
	}

	nlStringToAddr(address, &svr->sAddress);


	// Default game details
	strcpy(svr->szName, "Untitled");
	svr->nMaxPlayers = 0;
	svr->nNumPlayers = 0;
	svr->nState = 0;
	svr->nPing = 0;


	// If the address doesn't have a port number set, use the default lierox port number
	if(nlGetPortFromAddr(&svr->sAddress) == 0)
		nlSetAddrPort(&svr->sAddress, LX_PORT);	


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
server_t *Menu_SvrList_AddNamedServer(char *address, char *name)
{
    // Check if the server is already in the list
    // If it is, don't bother adding it
    server_t *sv = psServerList;
    NLaddress ad;
    TrimSpaces(address);
    nlStringToAddr(address, &ad);

    for(; sv; sv=sv->psNext) {
        if( nlAddrCompare(&sv->sAddress, &ad) )
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
	strcpy(svr->szName, name);

	strcpy( svr->szAddress, address );

	// If the address doesn't have a port number, use the default lierox port number
	if( strrchr(svr->szAddress,':') == NULL) {
		char buf[64];
		strcat(svr->szAddress,":");
		strcat(svr->szAddress,itoa(LX_PORT,buf,10));
	}

	nlStringToAddr(address, &svr->sAddress);


	// Default game details
	svr->nMaxPlayers = 0;
	svr->nNumPlayers = 0;
	svr->nState = 0;
	svr->nPing = 0;


	// If the address doesn't have a port number set, use the default lierox port number
	if(nlGetPortFromAddr(&svr->sAddress) == 0)
		nlSetAddrPort(&svr->sAddress, LX_PORT);	


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
void Menu_SvrList_RemoveServer(char *szAddress)
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
server_t *Menu_SvrList_FindServerStr(char *szAddress)
{
    // Find a matching server
    server_t *sv = psServerList;
    for(; sv; sv=sv->psNext) {

        if( stricmp(sv->szAddress, szAddress) == 0 )
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
	char		buf[256], addr[256];
	char		*states[] = {"Open", "Loading", "Playing"};

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
		//nlGetRemoteAddr(tMenu->tSocket, &s->sAddress);
		//nlAddrToString(&s->sAddress, addr);

		// Remove the port from the address (save space)
		strcpy(addr, s->szAddress);
		char *p = strrchr(addr,':');
		if(p) {
			int pos = p - addr + 1;
			addr[pos-1] = '\0';
		}

		// State
		int state = 0;
		if(s->nState >= 0 && s->nState < 3)
			state = s->nState;

		// Colour
		int colour = 0xffff;
		if(s->bProcessing)
			colour = MakeColour(128,128,128);
		

		// Add the server to the list
		lv->AddItem(s->szAddress, 0, colour);
		lv->AddSubitem(LVS_IMAGE, "", tMenu->bmpConnectionSpeeds[num]);
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
		sprintf(buf,"%d/%d",s->nNumPlayers,s->nMaxPlayers);
		lv->AddSubitem(LVS_TEXT, buf, NULL);
		
		lv->AddSubitem(LVS_TEXT, itoa(s->nPing,buf,10), NULL);
		lv->AddSubitem(LVS_TEXT, addr, NULL);
	}

    lv->setSelectedID(curID);
	lv->RestoreScrollbarPos();
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
int Menu_SvrList_ParsePacket(CBytestream *bs, NLsocket sock)
{
	char			cmd[128], buf[128];
	NLaddress		adrFrom;
	int				update = false;

	// Check for connectionless packet header
	if(*(int *)bs->GetData() == -1) {
		bs->SetPos(4);
		bs->readString(cmd);

		nlGetRemoteAddr(sock,&adrFrom);

		// Check for a pong
		if(strcmp(cmd, "lx::pong") == 0) {

			// Look the the list and find which server returned the ping
			server_t *svr = Menu_SvrList_FindServer(&adrFrom);
			if( svr ) {

				// It pinged, so fill in the ping info so it will now be queried
				svr->bgotPong = true;
				svr->nQueries = 0;
			} else {

				// If we didn't ping this server directly (eg, subnet), add the server to the list
				nlAddrToString( &adrFrom, buf );
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

		// Check for a query return
		if(strcmp(cmd, "lx::queryreturn") == 0) {

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
server_t *Menu_SvrList_FindServer(NLaddress *addr)
{
	server_t *s = psServerList;

	for(; s; s=s->psNext) {

		nlStringToAddr(s->szAddress, &s->sAddress);

		if( nlAddrCompare( addr, &s->sAddress ) )
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
	char buf[64];
	bs->readString( buf );
	if(iNetMode != net_favourites)
		strcpy(svr->szName,buf);
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
void Menu_SvrList_SaveList(char *szFilename)
{
    FILE *fp = fopen_i(szFilename,"wt");
    if( !fp )
        return;

    server_t *s = psServerList;
	for(; s; s=s->psNext) {
        fprintf(fp,"%s, %s, %s\n",s->bManual ? "1" : "0", s->szName, s->szAddress);
    }

    fclose(fp);
}

///////////////////
// Add a favourite server
void Menu_SvrList_AddFavourite(char *szName, char *szAddress)
{
    FILE *fp = fopen_i("cfg/favourites.dat","a");  // We're appending
    if( !fp )  {
        fp = fopen_i("cfg/favourites.dat","wb");  // Try to create the file
		if (!fp)
			return;
	}

	// Append the server
    fprintf(fp,"%s, %s, %s\n","1", szName, szAddress);

    fclose(fp);
}


///////////////////
// Load the server list
void Menu_SvrList_LoadList(char *szFilename)
{
    FILE *fp = fopen_i(szFilename,"rt");
    if( !fp )
        return;

    char szLine[1024];

    // Go through every line
    while( fgets(szLine, 1024, fp) ) {

        strcpy(szLine, StripLine(szLine));
        if( strlen(szLine) == 0 )
            continue;

        char *man = strtok(szLine,",");
        char *name = strtok(NULL,",");
        char *addr = strtok(NULL,",");

        if( man && name && addr ) {
            char address[256];
            strcpy(address,addr);
            strcpy(address,TrimSpaces(address));

            server_t *sv = Menu_SvrList_AddServer(address, man[0] == '1');

            // Fill in the name
            if( sv ) {
                strcpy(sv->szName, name);
                strcpy(sv->szName, TrimSpaces(sv->szName));
            }
        }
    }

    fclose(fp);
}



///////////////////
// Draw a 'server info' box
void Menu_SvrList_DrawInfo(char *szAddress)
{
	int w = 450;
	int h = 420;
	int y = tMenu->bmpBuffer->h/2 - h/2;
	int x = tMenu->bmpBuffer->w/2 - w/2;

    Menu_DrawBox(tMenu->bmpBuffer, x,y, w,h);
	DrawRectFillA(tMenu->bmpBuffer, x+1,y+1, w-1,h-1, 0, 200);
    tLX->cFont.DrawCentre(tMenu->bmpBuffer, x+w/2-tLX->cFont.GetWidth("Server Details")/2, y+5, 0xffff,"%s", "Server Details");


    // Get the server details
    bool    bGotDetails = false;
    char    szName[256];
    int     nMaxWorms;
    int     nState;
    char    szMapName[256];
    char    szModName[256];
    int     nGameMode;
    int     nLives;
    int     nMaxKills;
    int     nLoadingTime;
    int     nBonuses;
    int     nNumPlayers;
    CWorm   cWorms[MAX_WORMS];

    CBytestream inbs;
    NLaddress   addr;


    float   fStart = -99999;
    int     nTries = 0;

	bool OldLxBug = false;

    while(nTries < 3 && !bGotDetails) {
		if (OldLxBug)
			break;

        ProcessEvents();
        tLX->fCurTime = GetMilliSeconds();

        if(tLX->fCurTime - fStart > 1) {
            nTries++;
            fStart = tLX->fCurTime;

            // Send a getinfo request
            TrimSpaces(szAddress);
            nlStringToAddr(szAddress, &addr);

            nlSetRemoteAddr(tMenu->tSocket[SCK_NET], &addr);
	
	        CBytestream bs;
	        bs.writeInt(-1,4);
	        bs.writeString("%s","lx::getinfo");
	        bs.Send(tMenu->tSocket[SCK_NET]);
        }

        while(inbs.Read(tMenu->tSocket[SCK_NET])) {
            // Check for connectionless packet header
	        if(*(int *)inbs.GetData() == -1) {
                char        cmd[128];                

		        inbs.SetPos(4);
		        inbs.readString(cmd);


		        nlGetRemoteAddr(tMenu->tSocket[SCK_NET],&addr);

		        // Check for server info
		        if(strcmp(cmd, "lx::serverinfo") == 0) {
                    bGotDetails = true;

                    // Read the info
                    inbs.readString(szName);
	                nMaxWorms = inbs.readByte();
	                nState = inbs.readByte();

					if (nState < 0)  {
						OldLxBug = true;
						break;
					}
    
                    inbs.readString(szMapName);
                    inbs.readString(szModName);
	                nGameMode = inbs.readByte();
	                nLives = inbs.readShort();
	                nMaxKills = inbs.readShort();        
	                nLoadingTime = inbs.readShort();
					if(nLoadingTime < 0 || nLoadingTime > 500)  {
						OldLxBug = true;
						break;
					}
                    nBonuses = inbs.readByte();

                    // Worms
                    nNumPlayers = inbs.readByte();
					if (nNumPlayers <= 0)  {
						OldLxBug = true;
						break;
					}

                    for(int i=0; i<nNumPlayers; i++) {
                        inbs.readString(cWorms[i].getName());
                        cWorms[i].setKills(inbs.readInt(2));
                    }
                }
            }
        }
    }

	y+=25;
	x+=15;
    // Draw the server details    
    tLX->cFont.Draw(tMenu->bmpBuffer, x,y, 0xffff,"%s", "Server name:");
    tLX->cFont.Draw(tMenu->bmpBuffer, x,y+20, 0xffff,"%s", "Level name:");
    tLX->cFont.Draw(tMenu->bmpBuffer, x,y+40, 0xffff,"%s", "Mod name:");
    tLX->cFont.Draw(tMenu->bmpBuffer, x,y+60, 0xffff,"%s", "State:");
    tLX->cFont.Draw(tMenu->bmpBuffer, x,y+80, 0xffff,"%s", "Playing:");

    tLX->cFont.Draw(tMenu->bmpBuffer, x,y+100, 0xffff,"%s", "Game Type:");
    tLX->cFont.Draw(tMenu->bmpBuffer, x,y+120, 0xffff,"%s", "Lives:");
    tLX->cFont.Draw(tMenu->bmpBuffer, x,y+140, 0xffff,"%s", "Max Kills:");
    tLX->cFont.Draw(tMenu->bmpBuffer, x,y+160, 0xffff,"%s", "Loading Times:");
    tLX->cFont.Draw(tMenu->bmpBuffer, x,y+180, 0xffff,"%s", "Bonuses:");
	if (!nState)  // Dont show kills when the server is open
		tLX->cFont.Draw(tMenu->bmpBuffer, x,y+200, 0xffff,"%s", "Players:");
	else
		tLX->cFont.Draw(tMenu->bmpBuffer, x,y+200, 0xffff,"%s", "Players/Kills:");
    
	x+=110;

    if(!bGotDetails) {
        tLX->cFont.Draw(tMenu->bmpBuffer, x,y, MakeColour(200,100,100),"%s", "Unable to query server");
        return;
    }

    if(OldLxBug) {
        tLX->cFont.Draw(tMenu->bmpBuffer, x,y, MakeColour(200,100,100),"%s", "You can't view details\nof this server because\nLieroX v0.56 contains a bug.\n\nPlease wait until the server\nchanges its state to Playing\nand try again.");
        return;
    }

   	char	*states[] = {"Open", "Loading", "Playing", "Unknown"};
    char    *gamemodes[] = {"Deathmatch","Team Deathmatch", "Tag", "Demolitions", "Unknown"};
    if(nState < 0 || nState > 2)
        nState = 3;
    if(nGameMode < 0 || nGameMode > 3)
        nGameMode = 4;

    tLX->cFont.Draw(tMenu->bmpBuffer, x,y, 0xffff, "%s", szName);
    tLX->cFont.Draw(tMenu->bmpBuffer, x,y+20, 0xffff, "%s", szMapName);
    tLX->cFont.Draw(tMenu->bmpBuffer, x,y+40, 0xffff, "%s", szModName);
    tLX->cFont.Draw(tMenu->bmpBuffer, x,y+60, 0xffff, "%s", states[nState]);
    tLX->cFont.Draw(tMenu->bmpBuffer, x,y+80, 0xffff, "%d / %d", nNumPlayers, nMaxWorms);

    tLX->cFont.Draw(tMenu->bmpBuffer, x,y+100, 0xffff, "%s", gamemodes[nGameMode]);
    tLX->cFont.Draw(tMenu->bmpBuffer, x,y+120, 0xffff, "%d", nLives);
    tLX->cFont.Draw(tMenu->bmpBuffer, x,y+140, 0xffff, "%d", nMaxKills);
    tLX->cFont.Draw(tMenu->bmpBuffer, x,y+160, 0xffff, "%d", nLoadingTime);
    tLX->cFont.Draw(tMenu->bmpBuffer, x,y+180, 0xffff, "%s", nBonuses ? "on" : "off");

	// Don't draw kills when the server is open
	if(!nState)
		for (int i=0; i<nNumPlayers; i++) 
			tLX->cFont.Draw(tMenu->bmpBuffer, x,y+200+i*18, 0xffff, "%s", cWorms[i].getName());
	else
		for (int i=0; i<nNumPlayers; i++)  {
			tLX->cFont.Draw(tMenu->bmpBuffer, x,y+200+i*18, 0xffff, "%s", cWorms[i].getName());
			tLX->cFont.Draw(tMenu->bmpBuffer, x+150,y+200+i*18, 0xffff, "%d", cWorms[i].getKills());
		}

}
