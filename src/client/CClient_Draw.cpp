/////////////////////////////////////////
//
//             Liero Xtreme
//
//     Copyright Auxiliary Software 2002
//
//
/////////////////////////////////////////


// Client class - Drawing routines
// Created 9/7/02
// Jason Boettcher


#include "defs.h"
#include "LieroX.h"
#include "Menu.h"
#include "console.h"


SDL_Surface		*bmpMenuButtons = NULL;
float			fLagFlash;


///////////////////
// Initialize the drawing routines
int CClient::InitializeDrawing(void)
{
	LOAD_IMAGE(bmpMenuButtons,"data/frontend/buttons.png");

	//fLagFlash = 0;

    // Set the appropriate chatbox width
    if(tGameInfo.iGameType == GME_LOCAL)
        cChatbox.setWidth(600);
    else
        cChatbox.setWidth(300);

	// Initialize the score buffer
	bmpScoreBuffer = gfxCreateSurface(gfxGame.bmpScoreboard->w,gfxGame.bmpScoreboard->h);
	DrawRectFill(bmpScoreBuffer,0,0,bmpScoreBuffer->w,bmpScoreBuffer->h,tLX->clPink);
	SDL_SetColorKey(bmpScoreBuffer, SDL_SRCCOLORKEY, tLX->clPink);

	return true;
}


///////////////////
// Main drawing routines
void CClient::Draw(SDL_Surface *bmpDest)
{
	// Not for bots
	/*if (bBotClient)
		return;*/

	int i,num;
	float dt = tLX->fDeltaTime;


	num = iNumWorms;
	num = MIN(2,iNumWorms);


	// Check for any communication errors
	if(iServerError) {

		// Show message box, shutdown and quit back to menu
		DrawImage(tMenu->bmpBuffer,tMenu->bmpMainBack_wob,0,0);
		Menu_RedrawMouse(true);
        SDL_ShowCursor(SDL_DISABLE);

		Menu_MessageBox("Connection error", strServerErrorMsg, LMB_OK);

		QuittoMenu();
		return;
	}

    // Draw the borders
    DrawRectFill(bmpDest,0,382,640,480,0);
    if(cViewports[1].getUsed())
        DrawRectFill(bmpDest,318,0,322,384,0);


	// Draw the viewports
	if(iNetStatus == NET_PLAYING) {

        // Draw the viewports
        for( i=0; i<NUM_VIEWPORTS; i++ ) {
            if( cViewports[i].getUsed() )  {
				cViewports[i].Process(cRemoteWorms, cViewports, cMap->GetWidth(), cMap->GetHeight(), iGameType);
                DrawViewport(bmpDest, &cViewports[i]);
			}
        }

        //
        // Mini-Map
        //

		// Single screen
		if( !cViewports[1].getUsed() )
			cMap->DrawMiniMap( bmpDest, 511,383, dt, cRemoteWorms, iGameType );
        else
            // Split screen
			cMap->DrawMiniMap( bmpDest, 257,383, dt, cRemoteWorms, iGameType);
	}


	// Connected
	else if(iNetStatus == NET_CONNECTED && iGameReady) {
		int ready = true;

		// Go through and draw the first two worms select menus
		for(i=0;i<num;i++) {

			// Draw Map
            if( cViewports[i].getUsed() )  {
			    cMap->Draw(bmpDest, &cViewports[i]);
			}

			// Select weapons
			if(!cLocalWorms[i]->getWeaponsReady()) {
				ready = false;
				cLocalWorms[i]->SelectWeapons(bmpDest, &cViewports[i]);
			}
		}

		// If we're ready, let the server know
		if(ready && !iReadySent) {
			iReadySent = true;
			CBytestream *bytes = cNetChan.getMessageBS();
			bytes->writeByte(C2S_IMREADY);
			bytes->writeByte(iNumWorms);

			// Send my worm's weapon details
			for(i=0;i<iNumWorms;i++)
				cLocalWorms[i]->writeWeapons(bytes);
		}
	}

	// DEBUG
	//DrawRectFill(bmpDest,0,0,100,40,0);
	//tLX->cFont.Draw(bmpDest,0,0,0xffff,"iNetStatus = %i",iNetStatus);
	//tLX->cFont.Draw(bmpDest,0,20,0xffff,"iGameReady = %i",iGameReady);

	// Draw the chatbox for either a local game, or remote game
	if(tGameInfo.iGameType == GME_LOCAL)
		DrawLocalChat(bmpDest);
	else
		DrawRemoteChat(bmpDest);

	bool bScoreAndSett = true;

	// Game over
    if(iGameOver) {
        if(tLX->fCurTime - fGameOverTime > GAMEOVER_WAIT)  {
			bScoreAndSett = false;
		    DrawGameOver(bmpDest);
			if (cServer)
				if (!cServer->getScreenshotToken() && tGameInfo.iGameType == GME_HOST && tGameInfo.bTournament)
					cServer->setTakeScreenshot(true);
		}
        else
            tLX->cOutlineFont.DrawCentre(bmpDest, 320, 200, tLX->clNormalText,"%s", "Game Over");
    }

	// Game menu
	if(iGameMenu)  {
		bScoreAndSett = false;
		DrawGameMenu(bmpDest);
	}

    // Viewport manager
    if(bViewportMgr)  {
		bScoreAndSett = false;
        DrawViewportManager(bmpDest);
	}

    // Scoreboard and Current settings
	if(bScoreAndSett)  {
		DrawScoreboard(bmpDest);
		DrawCurrentSettings(bmpDest);
	}

	// Chatter
	if(iChat_Typing)  {
		tLX->cOutlineFont.Draw(bmpDest, 4, 366, tLX->clNormalText, "Talk: %s",sChat_Text);
		if (iChat_CursorVisible)  {
			static char buf[256];
			iChat_Pos = MIN((unsigned int)iChat_Pos,sizeof(buf)-1);
			strncpy(buf,sChat_Text,iChat_Pos);
			buf[iChat_Pos] = '\0';
			DrawVLine(bmpDest, 368, 378, 4+tLX->cFont.GetWidth("Talk: ")+tLX->cFont.GetWidth(buf), tLX->clNormalText);
		}
	}

	// Console
    Con_Draw(bmpDest);


	// Health bar toggle
	iCanToggle = (iCanToggle || cShowHealth.isUp()) && (!iChat_Typing);

	if (cShowHealth.isDown() && iCanToggle)  {
		tLXOptions->iShowHealth = !tLXOptions->iShowHealth;
		iCanToggle = false;
	}


	// Lag icon
	/*if(tGameInfo.iGameType != GME_LOCAL) {
		if(cNetChan.getOutSeq() - cNetChan.getInAck() > 127) {
			fLagFlash += dt;
			if(fLagFlash>=2)
				fLagFlash=0;

			// Draw the lag icon
			int y = 0;
			if(fLagFlash>1)
				y = 43;
			//DrawImageAdv(bmpDest, gfxGame.bmpLag, 0,y, 640-gfxGame.bmpLag->w, 0, gfxGame.bmpLag->w,43);
		}
	}*/


	// FPS on the top right
	if(tLXOptions->iShowFPS) {
		int fps = GetFPS();
		tLX->cOutlineFont.Draw(bmpDest, 570, 0, tLX->clNormalText, "FPS: %d",fps);
	}

	// Ping on the top right
	if(tLXOptions->iShowPing && tGameInfo.iGameType == GME_JOIN)  {

		// Put it below FPS, if it's displayed
		int pos = tLXOptions->iShowFPS*16;

		tLX->cOutlineFont.Draw(bmpDest, 570, pos, tLX->clNormalText, "Ping: %d",iMyPing);

		// Send every second
		if (tLX->fCurTime - fMyPingRefreshed > 1) {
			CBytestream *bs = cClient->getChannel()->getMessageBS();
			CBytestream ping;

			ping.Clear();
			ping.writeInt(-1,4);
			ping.writeString("%s","lx::ping");

			bs->Append(&ping);
			bs->Send(cClient->getChannel()->getSocket());

			fMyPingSent = tLX->fCurTime;
			fMyPingRefreshed = tLX->fCurTime;
		}
	}

    //tLX->cOutlineFont.Draw(bmpDest, 4,20, tLX->clNormalText, "%s",tLX->debug_string);
    //tLX->cOutlineFont.Draw(bmpDest, 4,40, tLX->clNormalText, "%f",tLX->debug_float);
}


///////////////////
// Draw a viewport
void CClient::DrawViewport(SDL_Surface *bmpDest, CViewport *v)
{
    // If the viewport is null, or not used: exit
    if( !v )  {
		printf("The viewport is NULL");
        return;
	}
    if( !v->getUsed() )
        return;

    Uint32 grey = MakeColour(128,128,128);

    //CWorm *worm = v->getTarget();

	// Set the clipping
	SDL_Rect r = v->getRect();
	SDL_SetClipRect(bmpDest,&r);

    // Weather
    //cWeather.Draw(bmpDest, v);

	cMap->Draw(bmpDest, v);

    if( tLXOptions->iShadows ) {
		// Draw the projectile shadows
		DrawProjectileShadows(bmpDest, v);

        // Draw the worm shadows
        CWorm *w = cRemoteWorms;
        for(int i=0;i<MAX_WORMS;i++,w++) {
            if(w->isUsed() && w->getAlive())
                w->DrawShadow(bmpDest, cMap, v);
        }
    }

	// Draw the entities
	DrawEntities(bmpDest, v);

	// Draw the projectiles
	DrawProjectiles(bmpDest, v);

	// Draw the bonuses
	DrawBonuses(bmpDest, v);

	// Draw all the worms in the game
	int i;
	CWorm *w = cRemoteWorms;
	for(i=0;i<MAX_WORMS;i++,w++) {
		if(w->isUsed() && w->getAlive())
			w->Draw(bmpDest, cMap, v);
	}

	// Disable the special clipping
	SDL_SetClipRect(bmpDest,NULL);

	// Draw the worm details
	int x = v->GetLeft();
	int y = v->GetTop() + v->GetVirtH();

	if(x > 0)
		x = 386;

	// The following is only drawn for viewports with a worm target
    if( v->getType() > VW_CYCLE )
        return;

    CWorm *worm = v->getTarget();

	// Health
	tLX->cFont.Draw(bmpDest, x+2, y+2, tLX->clNormalLabel,"%s","Health:");
	DrawRectFill(bmpDest,x+63,y+6,x+165,y+13,grey);
	DrawRectFill(bmpDest,x+64,y+7,x+64+worm->getHealth(),y+12,MakeColour(64,255,64));

	// Weapon
	wpnslot_t *Slot = worm->getCurWeapon();
	tLX->cFont.Draw(bmpDest, x+2, y+20,tLX->clNormalLabel,"%s","Weapon:");
	DrawRectFill(bmpDest,x+63,y+24,x+165,y+31,grey);
	Uint32 col = MakeColour(64,64,255);
	if(Slot->Reloading)
		col = MakeColour(128,64,64);

	float c = 1;
	c = Slot->Charge;
	DrawRectFill(bmpDest,x+64,y+25,x+64+(int)(c*100.0f),y+30,col);


	// Lives
	tLX->cFont.Draw(bmpDest,x+2, y+38, tLX->clNormalLabel, "%s", "Lives:");
	if(worm->getLives() >= 0)
		tLX->cFont.Draw(bmpDest,x+61,y+38, tLX->clNormalLabel, "%d",worm->getLives());
	else if(worm->getLives() == WRM_OUT)
		tLX->cFont.Draw(bmpDest,x+61,y+38, tLX->clNormalLabel, "%s", "Out");
	else if(worm->getLives() == WRM_UNLIM)
		DrawImage(bmpDest, gfxGame.bmpInfinite, x+61,y+41);

	// Kills
	tLX->cFont.Draw(bmpDest,x+2, y+56, tLX->clNormalLabel, "%s", "Kills:");
	tLX->cFont.Draw(bmpDest,x+61,y+56, tLX->clNormalLabel, "%d",worm->getKills());

	// Am i IT?
	if(worm->getTagIT() && iGameType == GMT_TAG)
		tLX->cFont.Draw(bmpDest, x+2, y+75, tLX->clNormalLabel, "%s", "You are IT!!");

    // Dirt count
    if( iGameType == GMT_DEMOLITION ) {
        tLX->cFont.Draw(bmpDest, x+2, y+75, tLX->clNormalLabel, "%s", "Dirt Count:");
        static char buf[64];
        int count = worm->getDirtCount();

        // Draw short versions
        snprintf(buf,sizeof(buf),"%d",count);
        if( count >= 1000 )
            snprintf(buf,sizeof(buf),"%dk",count/1000);
		fix_markend(buf);

        tLX->cFont.Draw(bmpDest,x+85,y+75, tLX->clNormalLabel, "%s",buf);
    }

	// Debug
	/*CViewport *view = worm->getViewport();
	int wx = view->GetWorldX();
	int wy = view->GetWorldY();
	int l = view->GetLeft();
	int t = view->GetTop();

	CVec vPosition = tLX->debug_pos;
	x=((int)vPosition.x-wx)*2+l;
	y=((int)vPosition.y-wy)*2+t;

	DrawRectFill(bmpDest, x-2,y-2,x+2,y+2,0xffff);*/
}

bool bDrawProjectiles = true;

///////////////////
// Draw the projectiles
void CClient::DrawProjectiles(SDL_Surface *bmpDest, CViewport *v)
{
	/*if (!bDrawProjectiles)  {
		return;
	}*/
	// Not for bots
	/*if (bBotClient)
		return;*/

	CProjectile *prj = cProjectiles;

    prj = cProjectiles;
	for(int p=0;p<nTopProjectile;p++,prj++) {
		if(!prj->isUsed())
			continue;

		prj->Draw(bmpDest, v);
	}
}


///////////////////
// Draw the projectile shadows
void CClient::DrawProjectileShadows(SDL_Surface *bmpDest, CViewport *v)
{
/*	fProjDrawTime += tLX->fDeltaTime;
	if (fProjDrawTime < 0.02f)  {
		fProjDrawTime = 0;
		bDrawProjectiles = false;
		return;
	}

	bDrawProjectiles = true;*/
	// Not for bots
	/*if (bBotClient)
		return;*/

    CProjectile *prj = cProjectiles;

    for(int p=0;p<nTopProjectile;p++,prj++) {
        if(!prj->isUsed())
            continue;

        prj->DrawShadow(bmpDest, v, cMap);
    }
}


///////////////////
// Simulate the hud
void CClient::SimulateHud(void)
{
	// Not for bots
	/*if (bBotClient)
		return;*/

	float dt = tLX->fDeltaTime;
	float ScrollSpeed=5;
    bool  con = Con_IsUsed();

    if(!iGameReady)
        return;

	for(int i=0;i<iChat_Numlines;i++) {
		tChatLines[i].fScroll += dt*ScrollSpeed;
		tChatLines[i].fScroll = MIN((float)1,tChatLines[i].fScroll);

		if(tChatLines[i].fTime + 4 < tLX->fCurTime) {
			iChat_Numlines--;
			iChat_Numlines = MAX(0,iChat_Numlines);
		}
	}

    // Console
    if(!iChat_Typing && !iGameMenu && !bViewportMgr)
        Con_Process(tLX->fDeltaTime);


	// Game Menu
    if(GetKeyboard()->KeyUp[SDLK_ESCAPE] && !iChat_Typing && !con) {
        if( !bViewportMgr )
		    iGameMenu = !iGameMenu;
        else
            bViewportMgr = false;
    }

    // Viewport manager
    if(cViewportMgr.isDownOnce() && !iChat_Typing && !iGameMenu && !con)
        InitializeViewportManager();

    // Process Chatter
    if(!iGameMenu && !bViewportMgr && !con)
	    processChatter();
}


///////////////////
// Draw the game over
void CClient::DrawGameOver(SDL_Surface *bmpDest)
{
	mouse_t *Mouse = GetMouse();
	keyboard_t *Keyboard = GetKeyboard();
	int mouse;

	DrawScore(bmpDest,gfxGame.bmpGameover);

	// Network games have a different game over screen
	if(tGameInfo.iGameType != GME_LOCAL) {
		DrawRemoteGameOver(bmpDest);
		return;
	}


	// Buttons
	mouse = 0;
//	int width = gfxGame.bmpGameover->w;  // TODO: not used
	int height = gfxGame.bmpGameover->h;

//	int x = 320 - width/2;  // TODO: not used
	int y = 200 - height/2;

	int j = y+height-27;
	CButton ok = CButton(BUT_OK,bmpMenuButtons);

	ok.Setup(0, 305,j, 30, 15);
    ok.Create();

	// OK
	if(ok.InBox(Mouse->X,Mouse->Y)) {
		ok.MouseOver(Mouse);
		mouse=1;
	}
	ok.Draw2(bmpDest);


	if(Mouse->Up) {
		if(ok.InBox(Mouse->X,Mouse->Y)) {
			// Quit
			tLX->iQuitEngine = true;
		}
	}

	// Quit
	if (Keyboard->KeyDown[SDLK_RETURN] || Keyboard->KeyDown[SDLK_KP_ENTER] || Keyboard->KeyDown[SDLK_ESCAPE])
		tLX->iQuitEngine = true;

	// Draw the mouse
	DrawImage(bmpDest,gfxGUI.bmpMouse[mouse], Mouse->X,Mouse->Y);
}


///////////////////
// Draw a remote game over screen
void CClient::DrawRemoteGameOver(SDL_Surface *bmpDest)
{
	mouse_t *Mouse = GetMouse();
	int mouse;

	// Buttons
	mouse = 0;
	int width = gfxGame.bmpGameover->w;
	int height = gfxGame.bmpGameover->h;

	int x = 320 - width/2;
	int y = 200 - height/2;

	int j = y+height-27;
	CButton leave = CButton(BUT_LEAVE,bmpMenuButtons);

	leave.Setup(0, x+20,j, 60, 15);
    leave.Create();

	// OK
	if(leave.InBox(Mouse->X,Mouse->Y)) {
		leave.MouseOver(Mouse);
		mouse=1;
	}
	leave.Draw(bmpDest);


	if(Mouse->Up) {
		if(leave.InBox(Mouse->X,Mouse->Y)) {

			// If this is a host, we go back to the lobby
			// The host can only quit the game via the lobby
			if(tGameInfo.iGameType == GME_HOST) {

				cServer->gotoLobby();

			} else {

				// Quit
				QuittoMenu();
			}
		}
	}

	// Draw a timer when we're going back to the lobby
	float timeleft = LX_ENDWAIT - (tLX->fCurTime - fGameOverTime);
	timeleft = MAX(timeleft,(float)0);
	tLX->cFont.Draw(bmpDest, x+width-180, j+2, MakeColour(200,200,200), "Returning to lobby in %d", (int)timeleft);


	// Draw the mouse
	DrawImage(bmpDest,gfxGUI.bmpMouse[mouse], Mouse->X,Mouse->Y);

}


///////////////////
// Draw the in-game menu
void CClient::DrawGameMenu(SDL_Surface *bmpDest)
{
	mouse_t *Mouse = GetMouse();
	int mouse = 0;

	DrawScore(bmpDest, gfxGame.bmpScoreboard);

	CButton quit = CButton(BUT_QUITGAME,bmpMenuButtons);
	CButton resume = CButton(BUT_RESUME,bmpMenuButtons);

	int width = gfxGame.bmpScoreboard->w;
	int height = gfxGame.bmpScoreboard->h;

	int x = 320 - width/2;
	int y = 200 - height/2;

	int j = y+height-27;
	quit.Setup(0, x+width-130,j, 105, 15);
	resume.Setup(1, x+20,j, 75, 15);
    quit.Create();
    resume.Create();

	// Quit
	if(quit.InBox(Mouse->X,Mouse->Y)) {
		quit.MouseOver(Mouse);
		mouse=1;
	}
	quit.Draw2(bmpDest);

	// Resume
	if(resume.InBox(Mouse->X,Mouse->Y)) {
		resume.MouseOver(Mouse);
		mouse=1;
	}
	resume.Draw2(bmpDest);

	if(Mouse->Up) {
		if(quit.InBox(Mouse->X,Mouse->Y)) {

			// If we're the host tell all the clients that we're going back to the lobby
			// We don't directly go back to the lobby, instead we send the message
			// and the client on this machine does the goto lobby when it gets the packet
			if(tGameInfo.iGameType == GME_HOST) {

				cServer->gotoLobby();

			} else {

				// Quit
				QuittoMenu();
			}
		}

		if(resume.InBox(Mouse->X,Mouse->Y))
			// Resume
			iGameMenu = false;
	}

	// Draw the mouse
	DrawImage(bmpDest,gfxGUI.bmpMouse[mouse], Mouse->X,Mouse->Y);
}


///////////////////
// Draw the score
void CClient::DrawScore(SDL_Surface *bmpDest, SDL_Surface *bmpImage)
{
	if (bUpdateScore)  {
		DrawRectFill(bmpScoreBuffer,0,0,bmpScoreBuffer->w,bmpScoreBuffer->h,tLX->clPink);
		UpdateScoreBuf(bmpScoreBuffer,bmpImage);
	}

	static int x = 320 - bmpImage->w/2;
	static int y = 200 - bmpImage->h/2;

	DrawImage(bmpDest,bmpScoreBuffer,x,y);
}

///////////////////
// Display the score
void CClient::UpdateScoreBuf(SDL_Surface *bmpDest, SDL_Surface *bmpImage)
{
	int i,j,n;

	bUpdateScore = false;

	// Teams
	Uint8 teamcolours[] = {102,153,255,  255,51,0,  51,153,0,  255,255,0};
	char *teamnames[] = {"Blue", "Red", "Green", "Yellow"};

	int width = bmpImage->w;
	int height = bmpImage->h;

	const int x = 0;
	const int y = 0;

	// Draw the back image
	DrawImage(bmpDest,bmpImage,x,y);

	// Deathmatch scoreboard
	switch(iGameType) {
	case GMT_DEATHMATCH:  {

		tLX->cFont.Draw(bmpDest, x+15, y+45, tLX->clNormalLabel,"%s","Players");
		if(iLives != WRM_UNLIM)
			tLX->cFont.Draw(bmpDest, x+300, y+45, tLX->clNormalLabel,"%s","Lives");

		// Draw ping if host
		if (tGameInfo.iGameType == GME_HOST)  {
			tLX->cFont.Draw(bmpDest, x+340, y+45, tLX->clNormalLabel,"%s","Kills");
			tLX->cFont.Draw(bmpDest, x+380, y+45, tLX->clNormalLabel,"%s","Ping");
		} else
			tLX->cFont.Draw(bmpDest, x+380, y+45, tLX->clNormalLabel,"%s","Kills");

		DrawHLine(bmpDest, x+15,x+width-15, y+60,0xffff);
		DrawHLine(bmpDest, x+15,x+width-15, y+height-30,0xffff);

		// Draw the players
		j = y+65;
		for(i=0;i<iScorePlayers;i++) {
			CWorm *p = &cRemoteWorms[iScoreboard[i]];

			DrawImage(bmpDest, p->getPicimg(), x+15, j);

			tLX->cFont.Draw(bmpDest, x+40, j, tLX->clNormalLabel, "%s", p->getName());

			if(p->getLives() >= 0)
				tLX->cFont.DrawCentre(bmpDest, x+317, j, tLX->clNormalLabel, "%d",p->getLives());
			else if(p->getLives() == WRM_OUT)
				tLX->cFont.DrawCentre(bmpDest, x+317, j, tLX->clNormalLabel, "%s", "out");

			// Draw ping if host
			if (tGameInfo.iGameType == GME_HOST)  {
				tLX->cFont.DrawCentre(bmpDest, x+353, j, tLX->clNormalLabel, "%d",p->getKills());
				CClient *remoteClient = cServer->getClient(p->getID());
				if (remoteClient && p->getID())
					tLX->cFont.DrawCentre(bmpDest, x+393, j, tLX->clNormalLabel, "%d",remoteClient->getPing());
			} else
				tLX->cFont.DrawCentre(bmpDest, x+393, j, tLX->clNormalLabel, "%d",p->getKills());

			j+=20;
		}
	}
	break; // TEAM DEATHMATCH


    // Demolitions scoreboard
	case GMT_DEMOLITION: {

		tLX->cFont.Draw(bmpDest, x+15, y+45, tLX->clNormalLabel,"%s","Players");
		if(iLives != WRM_UNLIM)
			tLX->cFont.Draw(bmpDest, x+270, y+45, tLX->clNormalLabel,"%s","Lives");
		tLX->cFont.Draw(bmpDest, x+340, y+45, tLX->clNormalLabel,"%s","Dirt Count");
		DrawHLine(bmpDest, x+15,x+width-15, y+60,0xffff);
		DrawHLine(bmpDest, x+15,x+width-15, y+height-30,0xffff);

        int dirtcount = 0;

		// Draw the players
		j = y+65;
		for(i=0;i<iScorePlayers;i++) {
			CWorm *p = &cRemoteWorms[iScoreboard[i]];

			DrawImage(bmpDest, p->getPicimg(), x+15, j);

			tLX->cFont.Draw(bmpDest, x+40, j, tLX->clNormalLabel, "%s", p->getName());

			if(p->getLives() >= 0)
				tLX->cFont.DrawCentre(bmpDest, x+287, j, tLX->clNormalLabel, "%d",p->getLives());
			else if(p->getLives() == WRM_OUT)
				tLX->cFont.DrawCentre(bmpDest, x+287, j, tLX->clNormalLabel, "%s", "out");


			tLX->cFont.DrawCentre(bmpDest, x+372, j, tLX->clNormalLabel, "%d",p->getDirtCount());
            dirtcount += p->getDirtCount();

			j+=20;
		}

        // Draw the total
        j+=10;
        DrawHLine(bmpDest,x+15,x+width-15, j, 0xffff);
        j+=5;
        tLX->cFont.Draw(bmpDest, x+250, j, tLX->clNormalLabel,"%s","Total");
        tLX->cFont.DrawCentre(bmpDest, x+372, j, tLX->clNormalLabel,"%dk / %.0fk", dirtcount / 1000, ((float)cMap->GetDirtCount()*0.8f) / 1000);
	}
	break;  // DEMOLITIONS


	// Tag scoreboard
	case GMT_TAG: {

		tLX->cFont.Draw(bmpDest, x+15, y+45, tLX->clNormalLabel,"%s","Players");
		if(iLives != WRM_UNLIM)
			tLX->cFont.Draw(bmpDest, x+220, y+45, tLX->clNormalLabel,"%s","Lives");

		// Draw the ping if host
		if(tGameInfo.iGameType == GME_HOST)  {
			tLX->cFont.Draw(bmpDest, x+280, y+45, tLX->clNormalLabel,"%s","Kills");
			tLX->cFont.Draw(bmpDest, x+322, y+45, tLX->clNormalLabel,"%s","Ping");
		}  else
			tLX->cFont.Draw(bmpDest, x+290, y+45, tLX->clNormalLabel,"%s","Kills");


		tLX->cFont.Draw(bmpDest, x+360, y+45, tLX->clNormalLabel,"%s","Tag time");
		DrawHLine(bmpDest, x+15,x+width-15, y+60,0xffff);
		DrawHLine(bmpDest, x+15,x+width-15, y+height-30,0xffff);

		// Draw the players
		j = y+65;
		for(i=0;i<iScorePlayers;i++) {
			CWorm *p = &cRemoteWorms[iScoreboard[i]];

			DrawImage(bmpDest, p->getPicimg(), x+15, j);

			tLX->cFont.Draw(bmpDest, x+40, j, tLX->clNormalLabel,"%s", p->getName());

			// Check if it
			if(p->getTagIT())
				tLX->cFont.Draw( bmpDest, x+160, j, MakeColour(255,0,0),"%s", "IT");

			if(p->getLives() >= 0)
				tLX->cFont.DrawCentre(bmpDest, x+237, j, tLX->clNormalLabel, "%d",p->getLives());
			else if(p->getLives() == WRM_OUT)
				tLX->cFont.DrawCentre(bmpDest, x+237, j, tLX->clNormalLabel,"%s", "out");

			// Draw the ping if host
			if (tGameInfo.iGameType == GME_HOST)  {
				tLX->cFont.DrawCentre(bmpDest, x+293, j, tLX->clNormalLabel, "%d",p->getKills());
				CClient *remoteClient = cServer->getClient(p->getID());
				if (remoteClient && p->getID())
					tLX->cFont.DrawCentre(bmpDest, x+333, j, tLX->clNormalLabel, "%d",remoteClient->getPing());
			} else
				tLX->cFont.DrawCentre(bmpDest, x+303, j, tLX->clNormalLabel, "%d",p->getKills());

			// Total time of being IT
			int h,m,s;
			static char buf[32];
			ConvertTime(p->getTagTime(), &h,&m,&s);
			snprintf(buf,sizeof(buf),"%d:%s%d",m,s<10 ? "0" : "",s);
			fix_markend(buf);
			Uint32 col = tLX->clNormalLabel;
			if(p->getTagIT())
				col = MakeColour(255,0,0);
			tLX->cFont.Draw(bmpDest, x+375, j, col, "%s", buf);

			j+=20;
		}
	}
	break;  // TAG


	// Team deathmatch scoreboard
	case GMT_TEAMDEATH: {

		DrawHLine(bmpDest, x+15,x+width-15, y+height-30,0xffff);

		// Go through each team
		j = y+50;
		for(n=0;n<4;n++) {
			int team = iTeamList[n];
			int score = iTeamScores[team];

			// Check if the team has any players
			if(score == -1)
				continue;

			Uint32 colour = MakeColour( teamcolours[team*3], teamcolours[team*3+1],teamcolours[team*3+2]);

			tLX->cFont.Draw(bmpDest, x+15, j, colour, "%s team  (%d)",teamnames[team],score);
			if(iLives != WRM_UNLIM)
				tLX->cFont.Draw(bmpDest, x+300, j, colour,"%s","Lives");
			if(tGameInfo.iGameType == GME_HOST)  {
				tLX->cFont.Draw(bmpDest, x+343, j, colour,"%s","Kills");
				tLX->cFont.Draw(bmpDest, x+380, j, colour,"%s","Ping");
			}
			else
				tLX->cFont.Draw(bmpDest, x+380, j, colour,"%s","Kills");
			DrawHLine(bmpDest, x+15,x+width-15, j+15,0xffff);
			j+=20;


			// Draw the players
			for(i=0;i<iScorePlayers;i++) {
				CWorm *p = &cRemoteWorms[iScoreboard[i]];

				if(p->getTeam() != team)
					continue;

				DrawImage(bmpDest, p->getPicimg(), x+15, j);

				tLX->cFont.Draw(bmpDest, x+40, j, tLX->clNormalLabel,"%s", p->getName());

				if(p->getLives() >= 0)
					tLX->cFont.DrawCentre(bmpDest, x+317, j, tLX->clNormalLabel, "%d",p->getLives());
				else if(p->getLives() == WRM_OUT)
					tLX->cFont.DrawCentre(bmpDest, x+317, j, tLX->clNormalLabel,"%s", "out");

				// Draw ping if host
				if (tGameInfo.iGameType == GME_HOST)  {
					tLX->cFont.DrawCentre(bmpDest, x+353, j, tLX->clNormalLabel, "%d",p->getKills());
					CClient *remoteClient = cServer->getClient(p->getID());
					if (remoteClient && p->getID())
						tLX->cFont.DrawCentre(bmpDest, x+396, j, tLX->clNormalLabel, "%d",remoteClient->getPing());
				} else
					tLX->cFont.DrawCentre(bmpDest, x+393, j, tLX->clNormalLabel, "%d",p->getKills());

				j+=20;
			}

			j+=15;
		}
	}
	break; // DEATHMATCH
	} // switch
}


///////////////////
// Draw the bonuses
void CClient::DrawBonuses(SDL_Surface *bmpDest, CViewport *v)
{
	if(!tGameInfo.iBonusesOn/* || bBotClient*/)
		return;

	CBonus *b = cBonuses;

	for(int i=0;i<MAX_BONUSES;i++,b++) {
		if(!b->getUsed())
			continue;

		b->Draw(bmpDest, v, iShowBonusName);
	}
}


///////////////////
// Draw text that is shadowed
void CClient::DrawText(SDL_Surface *bmpDest, int centre, int x, int y, Uint32 fgcol, char *fmt, ...)
{
	// Not for bots
	/*if (bBotClient)
		return;*/

	static char buf[512];
	va_list arg;

	va_start(arg, fmt);
	vsnprintf(buf, sizeof(buf), fmt, arg);
	fix_markend(buf);
	va_end(arg);

	if(centre) {
		//tLX->cOutlineFont.DrawCentre(bmpDest, x+1, y+1, 0,"%s", buf);
		tLX->cOutlineFont.DrawCentre(bmpDest, x, y, fgcol,"%s", buf);
	}
	else {
		//tLX->cOutlineFont.Draw(bmpDest, x+1, y+1, 0,"%s", buf);
		tLX->cOutlineFont.Draw(bmpDest, x, y, fgcol,"%s", buf);
	}
}


///////////////////
// Draw the local chat
void CClient::DrawLocalChat(SDL_Surface *bmpDest)
{
	int y = 0;
	int i;
	for(i=MAX(0,cChatbox.getNumLines()-6);i<cChatbox.getNumLines();i++) {
		line_t *l = cChatbox.GetLine(i);

		// This chat times out after a few seconds AND is on the top of the screen
		if(l && tLX->fCurTime - l->fTime < 3) {
			tLX->cFont.Draw(bmpDest, 6, y+1, 0,"%s", l->strLine);
			tLX->cFont.Draw(bmpDest, 5, y, l->iColour,"%s", l->strLine);
			y+=18;
		}
	}
}


///////////////////
// Draw the remote chat
void CClient::DrawRemoteChat(SDL_Surface *bmpDest)
{
	if (!cChatList)
		return;
	/*int i;
	int y = 386;

	for(i=MAX(0,cChatbox.getNumLines()-6);i<cChatbox.getNumLines();i++) {
		line_t *l = cChatbox.GetLine(i);

		// This chat is in the black region of the screen
		if(l) {
			//tLX->cFont.Draw(bmpDest, 190, y+1, 0,"%s", l->strLine);
			tLX->cFont.Draw(bmpDest, 190, y, l->iColour,"%s", l->strLine);
			y+=15;
		}
	}*/
	CListview *lv = (CListview *)cChatList;

	line_t *l = NULL;
	while(l = cChatbox.GetNewLine()) {

        if(lv->getLastItem())
            lv->AddItem("", lv->getLastItem()->iIndex+1, l->iColour);
        else
            lv->AddItem("", 0, l->iColour);
        lv->AddSubitem(LVS_TEXT, l->strLine, NULL);
		lv->scrollLast();
	}

    // If there are too many lines, remove the top one
    while(lv->getItemCount() > 256) {
        if(lv->getItems())
            lv->RemoveItem(lv->getItems()->iIndex);
    }

	lv->Draw(bmpDest);


	// Events
	mouse_t *Mouse = GetMouse();
	if (Mouse->WheelScrollDown)
		lv->MouseWheelDown(Mouse);
	else if (Mouse->WheelScrollUp)
		lv->MouseWheelUp(Mouse);

	if (lv->InBox(Mouse->X,Mouse->Y))  {
		// Draw the mouse
		DrawImage(bmpDest,gfxGUI.bmpMouse[0],Mouse->X,Mouse->Y);
		lv->MouseOver(Mouse);
		if (Mouse->Down)
			lv->MouseDown(Mouse,true);
		else if (Mouse->Up)
			lv->MouseUp(Mouse,false);
	}
}




CGuiLayout ViewportMgr;
enum {
    v1_On,
    v1_Type,
    v2_On,
    v2_Type,
    v_ok,
	v1_Target,
	v2_Target
};

///////////////////
// Initialize the viewport manager
void CClient::InitializeViewportManager(void)
{
	/*if (bBotClient)
		return;*/

    int x = 320-gfxGame.bmpViewportMgr->w/2;
    int y = 200-gfxGame.bmpViewportMgr->h/2;
    int x2 = x+gfxGame.bmpViewportMgr->w/2+40;


    bViewportMgr = true;

    // Initialize the gui
    ViewportMgr.Initialize();

    bool v2On = true;
    // If there is only 1 player total, turn the second viewport off
    int count = 0;
	int i;
    for(i=0; i<MAX_WORMS; i++ ) {
        if(cRemoteWorms[i].isUsed())
            count++;
    }

    if( count <= 1 )
        v2On = false;

    // Viewport 1
    ViewportMgr.Add( new CLabel("Used",tLX->clNormalLabel), -1,     x+15,  y+80,  0,   0);
    ViewportMgr.Add( new CLabel("Type",tLX->clNormalLabel), -1,     x+15,  y+110,  0,   0);
    //ViewportMgr.Add( new CCheckbox(true),       v1_On,  x+75,  y+80,  20,  20);
	ViewportMgr.Add( new CCombobox(),           v1_Target,x+75,    y+135, 150, 17);
    ViewportMgr.Add( new CCombobox(),           v1_Type,x+75,  y+110, 150, 17);
    ViewportMgr.Add( new CCheckbox(v2On),       v2_On,  x2,    y+80,  20,  20);
	ViewportMgr.Add( new CCombobox(),           v2_Target,x2,    y+135, 150, 17);
    ViewportMgr.Add( new CCombobox(),           v2_Type,x2,    y+110, 150, 17);
    ViewportMgr.Add( new CButton(BUT_OK, tMenu->bmpButtons),    v_ok,310,y+gfxGame.bmpViewportMgr->h-25,30,15);

    // Fill in the combo boxes

    // If the first player is a human, and is still playing: Only show the follow option
    ViewportMgr.SendMessage( v1_Type, CBM_ADDITEM, VW_FOLLOW, (DWORD)"Follow" );
    if( cLocalWorms[0]->getLives() == WRM_OUT || cLocalWorms[0]->getType() == PRF_COMPUTER ) {
        ViewportMgr.SendMessage( v1_Type, CBM_ADDITEM, VW_CYCLE, (DWORD)"Cycle" );
        ViewportMgr.SendMessage( v1_Type, CBM_ADDITEM, VW_FREELOOK, (DWORD)"Free Look" );
        ViewportMgr.SendMessage( v1_Type, CBM_ADDITEM, VW_ACTIONCAM, (DWORD)"Action Cam" );
    }

    // If the second player is a human and is still playing: Only show the follow option
    bool show = true;
    if( iNumWorms > 1 )
        if( cLocalWorms[1]->getLives() != WRM_OUT && cLocalWorms[1]->getType() == PRF_HUMAN )
            show = false;

    ViewportMgr.SendMessage( v2_Type, CBM_ADDITEM, VW_FOLLOW, (DWORD)"Follow" );
    if( show ) {
        ViewportMgr.SendMessage( v2_Type, CBM_ADDITEM, VW_CYCLE, (DWORD)"Cycle" );
        ViewportMgr.SendMessage( v2_Type, CBM_ADDITEM, VW_FREELOOK, (DWORD)"Free Look" );
        ViewportMgr.SendMessage( v2_Type, CBM_ADDITEM, VW_ACTIONCAM, (DWORD)"Action Cam" );
    }

	// Fill in the target worms boxes
    for(i=0; i<MAX_WORMS; i++ ) {
        if(!cRemoteWorms[i].isUsed() || cRemoteWorms[i].getLives() == WRM_OUT)
            continue;

		ViewportMgr.SendMessage( v1_Target, CBM_ADDITEM, cRemoteWorms[i].getID(), (DWORD)cRemoteWorms[i].getName() );
		ViewportMgr.SendMessage( v1_Target, CBM_SETIMAGE, cRemoteWorms[i].getID(), (DWORD)cRemoteWorms[i].getPicimg());
		ViewportMgr.SendMessage( v2_Target, CBM_ADDITEM, cRemoteWorms[i].getID(), (DWORD)cRemoteWorms[i].getName() );
		ViewportMgr.SendMessage( v2_Target, CBM_SETIMAGE, cRemoteWorms[i].getID(), (DWORD)cRemoteWorms[i].getPicimg());
    }

	CWorm *trg = cViewports[0].getTarget();
	if (trg)  {
		ViewportMgr.SendMessage( v1_Target, CBM_SETCURINDEX, trg->getID(), 0);
    }

	if (cViewports[1].getUsed())  {
		trg = cViewports[1].getTarget();
		if (trg)
			ViewportMgr.SendMessage( v2_Target, CBM_SETCURINDEX, trg->getID(), 0);
	}



    // Restore old settings
    ViewportMgr.SendMessage( v1_Type, CBM_SETCURINDEX, cViewports[0].getType(), 0);
    ViewportMgr.SendMessage( v2_On, CKM_SETCHECK, cViewports[1].getUsed(), 0);
    if( cViewports[1].getUsed() )
        ViewportMgr.SendMessage( v2_Type, CBM_SETCURINDEX, cViewports[1].getType(), 0);


    // Draw the background into the menu buffer
    DrawImage(tMenu->bmpBuffer,gfxGame.bmpViewportMgr,x,y);
}


///////////////////
// Draw the viewport manager
void CClient::DrawViewportManager(SDL_Surface *bmpDest)
{
    mouse_t *Mouse = GetMouse();
    int mcursor = 0;

    int x = 320-gfxGame.bmpViewportMgr->w/2;
    int y = 200-gfxGame.bmpViewportMgr->h/2;

	// Draw the back image
	DrawImage(bmpDest,gfxGame.bmpViewportMgr,x,y);

    tLX->cFont.Draw(bmpDest, x+75,y+50, tLX->clNormalLabel,"%s","Viewport 1");
    tLX->cFont.Draw(bmpDest, x+gfxGame.bmpViewportMgr->w/2+40,y+50, tLX->clNormalLabel,"%s","Viewport 2");

    ViewportMgr.Draw(bmpDest);
    gui_event_t *ev = ViewportMgr.Process();

    if( ViewportMgr.getWidget(v_ok)->InBox(Mouse->X,Mouse->Y) )
        mcursor = 1;

    // Draw the mouse
    DrawImage(bmpDest,gfxGUI.bmpMouse[mcursor], Mouse->X,Mouse->Y);

    if(!ev)
        return;


    // Get the worm count
    int Wormcount = 0;
    for(int i=0; i<MAX_WORMS; i++ ) {
        if(cRemoteWorms[i].isUsed())
            Wormcount++;
    }

    switch(ev->iControlID) {

        // V2 On
        case v2_On:
            if(ev->iEventMsg == CHK_CHANGED) {
                // If there is only one worm, disable the 2nd viewport
                if( Wormcount <= 1 )
                    ViewportMgr.SendMessage(v2_On, CKM_SETCHECK,false,0);
            }
            break;

        // OK
        case v_ok:
            if(ev->iEventMsg == BTN_MOUSEUP) {

                // If there is only one worm, disable the 2nd viewport
                if( Wormcount <= 1 )
                    ViewportMgr.SendMessage(v2_On, CKM_SETCHECK,false,0);

                // Grab settings
                int a_type = ViewportMgr.SendMessage(v1_Type, CBM_GETCURINDEX, 0,0);
                int b_on = ViewportMgr.SendMessage(v2_On, CKM_GETCHECK, 0,0);
                int b_type = ViewportMgr.SendMessage(v2_Type, CBM_GETCURINDEX, 0,0);
				int v1_target = ViewportMgr.SendMessage(v1_Target, CBM_GETCURINDEX, 0,0);
				int v2_target = ViewportMgr.SendMessage(v2_Target, CBM_GETCURINDEX, 0,0);

                for( int i=0; i<NUM_VIEWPORTS; i++ ) {
                    cViewports[i].setUsed(false);
                    cViewports[i].reset();
                }

                // If both viewports are used, resize the viewports
                if( !b_on ) {
                    cViewports[0].Setup(0,0,640,382,a_type);
		            cViewports[0].setUsed(true);
                    cViewports[0].setTarget(cLocalWorms[0]);
                }

                if( b_on ) {
                    cViewports[0].Setup(0,0,318,382,a_type);
                    cViewports[0].setTarget(cLocalWorms[0]);
		            cViewports[0].setUsed(true);

		            cViewports[1].Setup(322,0,318,382,b_type);
		            cViewports[1].setUsed(true);
                    if( iNumWorms >= 2 )
                        cViewports[1].setTarget(cLocalWorms[1]);
                    else
                        cViewports[1].setTarget(NULL);

                }

				// Set the worms to follow
				CWorm *trg_v1 = &cRemoteWorms[v1_target];
				if (trg_v1)
					if (trg_v1->isUsed() && trg_v1->getAlive())
						cViewports[0].setTarget(trg_v1);

				CWorm *trg_v2 = &cRemoteWorms[v2_target];
				if (trg_v2)
					if (trg_v2->isUsed() && trg_v2->getAlive())
						cViewports[1].setTarget(trg_v2);

                // Shutdown & leave
                ViewportMgr.Shutdown();
                bViewportMgr = false;
                return;
            }
            break;
    }
}


///////////////////
// Draw the scoreboard
void CClient::DrawScoreboard(SDL_Surface *bmpDest)
{
    bool bShowScore = false;
    bool bShowReady = false;

    // Do checks on whether or not to show the scoreboard
    if(Con_IsUsed())
        return;
    if(cShowScore.isDown() && !iChat_Typing)
        bShowScore = true;
    if(iNetStatus == NET_CONNECTED && iGameReady && tGameInfo.iGameType != GME_LOCAL) {
        bShowScore = true;
        bShowReady = true;
    }
    if(!bShowScore/* || bBotClient*/)
        return;

    int y = 180;
    int x = 0;
    int w = 240;
    int h = 185;
	if (tGameInfo.iGameType == GME_HOST)  {
		w = 260;
		if (bShowReady)  {
			x = 200;
			y = 195;
			w = 300;
			h = 185;
		}
		if (cViewports[1].getUsed())
			x += 125;
	}
    DrawRectFill(bmpDest, x+1, y, x+w-1, y+h-1, 0);
    Menu_DrawBox(bmpDest, x, y, x+w, y+h);

    tLX->cFont.Draw(bmpDest, x+5, y+4, tLX->clNormalLabel,"%s", "Players");
    if(!bShowReady) {
        tLX->cFont.Draw(bmpDest, x+180, y+4, tLX->clNormalLabel,"%s", "L");
        tLX->cFont.Draw(bmpDest, x+210, y+4, tLX->clNormalLabel,"%s", "K");
		if(tGameInfo.iGameType == GME_HOST)
			tLX->cFont.Draw(bmpDest, x+237, y+4, tLX->clNormalLabel,"%s", "P");
    }
	else
		if (tGameInfo.iGameType == GME_HOST)
			tLX->cFont.Draw(bmpDest, x+250, y+4, tLX->clNormalLabel,"%s", "Ping");

    DrawHLine(bmpDest, x+4, x+w-4, y+20, 0xffff);


    // Draw the players
	int j = y+25;
    int i;
    for(i=0;i<iScorePlayers;i++) {
        CWorm *p = &cRemoteWorms[iScoreboard[i]];

		// Get the team colour
								// Blue				Red				Green			Yellow
		Uint8 teamcolours[] = {0x02,0xB8,0xFC,  0xFF,0x02,0x02,  0x20,0xFD,0x00,  0xFD,0xF4,0x00};
		Uint8 clR = teamcolours[p->getTeam()*3];
		Uint8 clG = teamcolours[p->getTeam()*3+1];
		Uint8 clB = teamcolours[p->getTeam()*3+2];
		Uint32 iColor = MakeColour(clR,clG,clB);

        // If this player is local & human, highlight it
        if(p->getType() == PRF_HUMAN && p->getLocal())
            DrawRectFill(bmpDest, x+2,j-2, x+w-1, j+18, MakeColour(52,52,52));

		tLX->cFont.Draw(bmpDest, x+5, j+1, 0xffff,"#%d",p->getID());

        // Pic & Name
        DrawImage(bmpDest, p->getPicimg(), x+30, j);
		if (tGameInfo.iGameMode == GMT_TEAMDEATH  && tLXOptions->iColorizeNicks)
			tLX->cFont.Draw(bmpDest, x+56, j, iColor,"%s", p->getName());
		else
			tLX->cFont.Draw(bmpDest, x+56, j, tLX->clNormalLabel,"%s", p->getName());

        // Score
        if(!bShowReady) {
            if(p->getLives() >= 0)
                tLX->cFont.DrawCentre(bmpDest, x+185, j, tLX->clNormalLabel, "%d",p->getLives());
            else if(p->getLives() == WRM_OUT)
                tLX->cFont.DrawCentre(bmpDest, x+185, j, tLX->clNormalLabel,"%s", "out");

            tLX->cFont.DrawCentre(bmpDest, x+215, j, tLX->clNormalLabel, "%d",p->getKills());

			if(tGameInfo.iGameType == GME_HOST)  {
				CClient *remoteClient = cServer->getClient(p->getID());
				if (remoteClient && p->getID())
					tLX->cFont.DrawCentre(bmpDest, x+240, j, tLX->clNormalLabel, "%d",remoteClient->getPing());
			}
        } else {
            // Ready state
            if(p->getGameReady())
                tLX->cFont.Draw(bmpDest, x+180, j, tLX->clReady,"%s", "Ready");
            else
                tLX->cFont.Draw(bmpDest, x+180, j, tLX->clWaiting,"%s", "Waiting");

			// Show ping if host
			if(tGameInfo.iGameType == GME_HOST)  {
				CClient *remoteClient = cServer->getClient(p->getID());
				if (remoteClient && p->getID())
					tLX->cFont.DrawCentre(bmpDest, x+260, j, tLX->clNormalLabel, "%d",remoteClient->getPing());
			}
        }

        j+=20;
    }
}

///////////////////
// Draw the current game settings
void CClient::DrawCurrentSettings(SDL_Surface *bmpDest)
{
    if(Con_IsUsed() /*|| bBotClient*/)
        return;

    // Do checks on whether or not to show
    if(iNetStatus != NET_CONNECTED && !cShowSettings.isDown())
		return;

    int y = 0;
    int x = 0;
	int w = 240;
	int h = 132;
	if (cViewports[1].getUsed())  {
		y = 250;
		if (tGameInfo.iGameType == GME_LOCAL)
			x = 200;
		else {
			x = 75;
			y = 195;
		}
	}

    DrawRectFill(bmpDest, x+1, y, x+w-1, y+h-1, 0);
    Menu_DrawBox(bmpDest, x, y, x+w, y+h);

    tLX->cFont.Draw(bmpDest, x+60, y+5, tLX->clNormalLabel,"%s", "Current settings");
    DrawHLine(bmpDest, x+4, x+w-4, y+22, 0xffff);

	/*tLX->cFont.Draw(bmpDest, x+5, y+25, tLX->clNormalLabel,"%s","Level:");
	tLX->cFont.Draw(bmpDest, x+105, y+25, tLX->clNormalLabel,"%s",tGameInfo.sMapname);*/
	tLX->cFont.Draw(bmpDest, x+5, y+25, tLX->clNormalLabel,"%s", "Mod:");
	tLX->cFont.Draw(bmpDest, x+105, y+25, tLX->clNormalLabel,"%s", tGameInfo.sModName);
	tLX->cFont.Draw(bmpDest, x+5, y+43, tLX->clNormalLabel,"%s","Game Type:");
	switch (tGameInfo.iGameMode)  {
	case GMT_DEATHMATCH:
	  tLX->cFont.Draw(bmpDest, x+105, y+43, tLX->clNormalLabel,"%s","Deathmatch");
	  break;
	case GMT_TAG:
	  tLX->cFont.Draw(bmpDest, x+105, y+43, tLX->clNormalLabel,"%s","Tag");
	  break;
	case GMT_TEAMDEATH:
	  tLX->cFont.Draw(bmpDest, x+105, y+43, tLX->clNormalLabel,"%s","Team Deathmatch");
	  break;
	case GMT_DEMOLITION:
	  tLX->cFont.Draw(bmpDest, x+105, y+43, tLX->clNormalLabel,"%s","Demolition");
	  break;
	}  // switch
	tLX->cFont.Draw(bmpDest, x+5, y+61, tLX->clNormalLabel,"%s","Loading Time:");
	tLX->cFont.Draw(bmpDest, x+105, y+61, tLX->clNormalLabel,"%d%%",tGameInfo.iLoadingTimes);
	tLX->cFont.Draw(bmpDest, x+5, y+79, tLX->clNormalLabel,"%s","Lives:");
	tLX->cFont.Draw(bmpDest, x+105, y+79, tLX->clNormalLabel,"%d",tGameInfo.iLives);
	tLX->cFont.Draw(bmpDest, x+5, y+97, tLX->clNormalLabel,"%s","Max Kills:");
	tLX->cFont.Draw(bmpDest, x+105, y+97, tLX->clNormalLabel,"%d",tGameInfo.iKillLimit);
	tLX->cFont.Draw(bmpDest, x+5, y+115, tLX->clNormalLabel,"%s","Bonuses:");
	if (tGameInfo.iBonusesOn)
		tLX->cFont.Draw(bmpDest, x+105, y+115, tLX->clNormalLabel,"On");
	else
		tLX->cFont.Draw(bmpDest, x+105, y+115, tLX->clNormalLabel,"Off");

}
