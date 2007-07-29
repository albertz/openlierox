/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Client class - Drawing routines
// Created 9/7/02
// Jason Boettcher

#include "LieroX.h"
#include "ConfigHandler.h"
#include "CClient.h"
#include "CServer.h"
#include "Graphics.h"
#include "Menu.h"
#include "console.h"
#include "GfxPrimitives.h"
#include "StringUtils.h"
#include "CBar.h"
#include "CWorm.h"
#include "Protocol.h"
#include "Entity.h"


SDL_Surface		*bmpMenuButtons = NULL;
float			fLagFlash;


///////////////////
// Initialize the drawing routines
bool CClient::InitializeDrawing(void)
{
	LOAD_IMAGE_WITHALPHA(bmpMenuButtons,"data/frontend/buttons.png");

	// Initialize the score buffer
	bmpScoreBuffer = gfxCreateSurfaceAlpha(gfxGame.bmpScoreboard->w,gfxGame.bmpScoreboard->h); // Must be alpha, like scoreboard, gameover etc.
	SetColorKey(bmpScoreBuffer);
	DrawRectFill(bmpScoreBuffer,0,0,bmpScoreBuffer->w,bmpScoreBuffer->h,COLORKEY(bmpScoreBuffer));

	// Load the right and left part of box
	bmpBoxLeft = LoadImage("data/frontend/box_left.png",true);
	bmpBoxRight = LoadImage("data/frontend/box_right.png",true);

	// Initialize the box buffer
	SDL_Surface *box_middle = LoadImage("data/frontend/box_middle.png",true);
	if (box_middle)  { // Doesn't have to exist
		// Tile the buffer with the middle box part
		bmpBoxBuffer = gfxCreateSurface(640,box_middle->h);
		for (int i=0; i < bmpBoxBuffer->w; i += box_middle->w)
			DrawImage( bmpBoxBuffer, box_middle, i, 0);
	} else {
		bmpBoxBuffer = NULL;
	}

	// Local and network have different layouts and sections in the config file
	std::string section = "";
	if (tGameInfo.iGameType == GME_LOCAL)
		section = "LocalGameInterface";
	else
		section = "NetworkGameInterface";

	// Read the info from the frontend.cfg file
	ReadInteger("data/frontend/frontend.cfg",section,"GameChatterX",&tInterfaceSettings.ChatterX, 0);
	ReadInteger("data/frontend/frontend.cfg",section,"GameChatterY",&tInterfaceSettings.ChatterY, 366);
	ReadInteger("data/frontend/frontend.cfg",section,"ChatBoxX",&tInterfaceSettings.ChatBoxX, 165);
	ReadInteger("data/frontend/frontend.cfg",section,"ChatBoxY",&tInterfaceSettings.ChatBoxY, 382);
	ReadInteger("data/frontend/frontend.cfg",section,"ChatBoxW",&tInterfaceSettings.ChatBoxW, 346);
	ReadInteger("data/frontend/frontend.cfg",section,"ChatBoxH",&tInterfaceSettings.ChatBoxH, 98);
	ReadInteger("data/frontend/frontend.cfg",section,"FpsX",&tInterfaceSettings.FpsX, 575);
	ReadInteger("data/frontend/frontend.cfg",section,"FpsY",&tInterfaceSettings.FpsY, 1);
	ReadInteger("data/frontend/frontend.cfg",section,"FpsW",&tInterfaceSettings.FpsW, 65);
	ReadInteger("data/frontend/frontend.cfg",section,"MinimapW",&tInterfaceSettings.MiniMapW, 128);
	ReadInteger("data/frontend/frontend.cfg",section,"MinimapH",&tInterfaceSettings.MiniMapH, 96);

	ReadInteger("data/frontend/frontend.cfg",section,"CurrentSettingsX",&tInterfaceSettings.CurrentSettingsX, 0);
	ReadInteger("data/frontend/frontend.cfg",section,"CurrentSettingsY",&tInterfaceSettings.CurrentSettingsY, 0);
	ReadInteger("data/frontend/frontend.cfg",section,"CurrentSettingsTwoPlayersX",&tInterfaceSettings.CurrentSettingsTwoPlayersX, 75);
	ReadInteger("data/frontend/frontend.cfg",section,"CurrentSettingsTwoPlayersY",&tInterfaceSettings.CurrentSettingsTwoPlayersY, 195);
	ReadInteger("data/frontend/frontend.cfg",section,"ScoreboardX",&tInterfaceSettings.ScoreboardX, 0);
	ReadInteger("data/frontend/frontend.cfg",section,"ScoreboardY",&tInterfaceSettings.ScoreboardY, 180);


	if (tGameInfo.iGameType == GME_LOCAL)  {  // Local play can handle two players, it means all the top boxes twice
		ReadInteger("data/frontend/frontend.cfg",section,"MinimapX",&tInterfaceSettings.MiniMapX, 255);
		ReadInteger("data/frontend/frontend.cfg",section,"MinimapY",&tInterfaceSettings.MiniMapY, 382);
		ReadInteger("data/frontend/frontend.cfg",section,"ScoreboardTwoPlayersX",&tInterfaceSettings.ScoreboardOtherPosX, 200);
		ReadInteger("data/frontend/frontend.cfg",section,"ScoreboardTwoPlayersY",&tInterfaceSettings.ScoreboardOtherPosY, 195);

		ReadInteger("data/frontend/frontend.cfg",section,"Lives1X",&tInterfaceSettings.Lives1X, 1);
		ReadInteger("data/frontend/frontend.cfg",section,"Lives1Y",&tInterfaceSettings.Lives1Y, 1);
		ReadInteger("data/frontend/frontend.cfg",section,"Lives1W",&tInterfaceSettings.Lives1W, 75);
		ReadInteger("data/frontend/frontend.cfg",section,"Kills1X",&tInterfaceSettings.Kills1X, 80);
		ReadInteger("data/frontend/frontend.cfg",section,"Kills1Y",&tInterfaceSettings.Kills1Y, 1);
		ReadInteger("data/frontend/frontend.cfg",section,"Kills1W",&tInterfaceSettings.Kills1W, 65);
		ReadInteger("data/frontend/frontend.cfg",section,"Team1X",&tInterfaceSettings.Team1X, 150);
		ReadInteger("data/frontend/frontend.cfg",section,"Team1Y",&tInterfaceSettings.Team1Y, 1);
		ReadInteger("data/frontend/frontend.cfg",section,"Team1W",&tInterfaceSettings.Team1W, 70);
		ReadInteger("data/frontend/frontend.cfg",section,"SpecMsg1X",&tInterfaceSettings.SpecMsg1X, 150);
		ReadInteger("data/frontend/frontend.cfg",section,"SpecMsg1Y",&tInterfaceSettings.SpecMsg1Y, 1);
		ReadInteger("data/frontend/frontend.cfg",section,"SpecMsg1W",&tInterfaceSettings.SpecMsg1W, 100);
		ReadInteger("data/frontend/frontend.cfg",section,"HealthLabel1X",&tInterfaceSettings.HealthLabel1X, 5);
		ReadInteger("data/frontend/frontend.cfg",section,"HealthLabel1Y",&tInterfaceSettings.HealthLabel1Y, 400);
		ReadInteger("data/frontend/frontend.cfg",section,"WeaponLabel1X",&tInterfaceSettings.WeaponLabel1X, 5);
		ReadInteger("data/frontend/frontend.cfg",section,"WeaponLabel1Y",&tInterfaceSettings.WeaponLabel1Y, 425);

		ReadInteger("data/frontend/frontend.cfg",section,"Lives2X",&tInterfaceSettings.Lives2X, 323);
		ReadInteger("data/frontend/frontend.cfg",section,"Lives2Y",&tInterfaceSettings.Lives2Y, 1);
		ReadInteger("data/frontend/frontend.cfg",section,"Lives2W",&tInterfaceSettings.Lives2W, 75);
		ReadInteger("data/frontend/frontend.cfg",section,"Kills2X",&tInterfaceSettings.Kills2X, 403);
		ReadInteger("data/frontend/frontend.cfg",section,"Kills2Y",&tInterfaceSettings.Kills2Y, 1);
		ReadInteger("data/frontend/frontend.cfg",section,"Kills2W",&tInterfaceSettings.Kills2W, 65);
		ReadInteger("data/frontend/frontend.cfg",section,"Team2X",&tInterfaceSettings.Team2X, 473);
		ReadInteger("data/frontend/frontend.cfg",section,"Team2Y",&tInterfaceSettings.Team2Y, 1);
		ReadInteger("data/frontend/frontend.cfg",section,"Team2W",&tInterfaceSettings.Team2W, 70);
		ReadInteger("data/frontend/frontend.cfg",section,"SpecMsg2X",&tInterfaceSettings.SpecMsg2X, 473);
		ReadInteger("data/frontend/frontend.cfg",section,"SpecMsg2Y",&tInterfaceSettings.SpecMsg2Y, 1);
		ReadInteger("data/frontend/frontend.cfg",section,"SpecMsg2W",&tInterfaceSettings.SpecMsg2W, 100);
		ReadInteger("data/frontend/frontend.cfg",section,"HealthLabel2X",&tInterfaceSettings.HealthLabel2X, 390);
		ReadInteger("data/frontend/frontend.cfg",section,"HealthLabel2Y",&tInterfaceSettings.HealthLabel2Y, 400);
		ReadInteger("data/frontend/frontend.cfg",section,"WeaponLabel2X",&tInterfaceSettings.WeaponLabel2X, 390);
		ReadInteger("data/frontend/frontend.cfg",section,"WeaponLabel2Y",&tInterfaceSettings.WeaponLabel2Y, 425);

		ReadInteger("data/frontend/frontend.cfg",section,"LocalChatX",&tInterfaceSettings.LocalChatX, 0);
		ReadInteger("data/frontend/frontend.cfg",section,"LocalChatY",&tInterfaceSettings.LocalChatY, tLX->cFont.GetHeight());
	} else {  // Network allows only one player
		ReadInteger("data/frontend/frontend.cfg",section,"MinimapX",&tInterfaceSettings.MiniMapX, 511);
		ReadInteger("data/frontend/frontend.cfg",section,"MinimapY",&tInterfaceSettings.MiniMapY, 382);
		ReadInteger("data/frontend/frontend.cfg",section,"ScoreboardNotReadyX",&tInterfaceSettings.ScoreboardOtherPosX, 125);
		ReadInteger("data/frontend/frontend.cfg",section,"ScoreboardNotReadyY",&tInterfaceSettings.ScoreboardOtherPosY, 180);

		ReadInteger("data/frontend/frontend.cfg",section,"PingX",&tInterfaceSettings.PingX, 540);
		ReadInteger("data/frontend/frontend.cfg",section,"PingY",&tInterfaceSettings.PingY, 1);
		ReadInteger("data/frontend/frontend.cfg",section,"PingW",&tInterfaceSettings.PingW, 50);
		ReadInteger("data/frontend/frontend.cfg",section,"LivesX",&tInterfaceSettings.Lives1X, 1);
		ReadInteger("data/frontend/frontend.cfg",section,"LivesY",&tInterfaceSettings.Lives1Y, 1);
		ReadInteger("data/frontend/frontend.cfg",section,"LivesW",&tInterfaceSettings.Lives1W, 75);
		ReadInteger("data/frontend/frontend.cfg",section,"KillsX",&tInterfaceSettings.Kills1X, 80);
		ReadInteger("data/frontend/frontend.cfg",section,"KillsY",&tInterfaceSettings.Kills1Y, 1);
		ReadInteger("data/frontend/frontend.cfg",section,"KillsW",&tInterfaceSettings.Kills1W, 65);
		ReadInteger("data/frontend/frontend.cfg",section,"TeamX",&tInterfaceSettings.Team1X, 150);
		ReadInteger("data/frontend/frontend.cfg",section,"TeamY",&tInterfaceSettings.Team1Y, 1);
		ReadInteger("data/frontend/frontend.cfg",section,"TeamW",&tInterfaceSettings.Team1W, 70);
		ReadInteger("data/frontend/frontend.cfg",section,"SpecMsgX",&tInterfaceSettings.SpecMsg1X, 150);
		ReadInteger("data/frontend/frontend.cfg",section,"SpecMsgY",&tInterfaceSettings.SpecMsg1Y, 1);
		ReadInteger("data/frontend/frontend.cfg",section,"SpecMsgW",&tInterfaceSettings.SpecMsg1W, 100);
		ReadInteger("data/frontend/frontend.cfg",section,"HealthLabelX",&tInterfaceSettings.HealthLabel1X, 5);
		ReadInteger("data/frontend/frontend.cfg",section,"HealthLabelY",&tInterfaceSettings.HealthLabel1Y, 400);
		ReadInteger("data/frontend/frontend.cfg",section,"WeaponLabelX",&tInterfaceSettings.WeaponLabel1X, 5);
		ReadInteger("data/frontend/frontend.cfg",section,"WeaponLabelY",&tInterfaceSettings.WeaponLabel1Y, 425);

		ReadInteger("data/frontend/frontend.cfg",section,"ChatboxScrollbarX",&tInterfaceSettings.ChatboxScrollbarX, -1);
		ReadInteger("data/frontend/frontend.cfg",section,"ChatboxScrollbarY",&tInterfaceSettings.ChatboxScrollbarY, -1);
		ReadInteger("data/frontend/frontend.cfg",section,"ChatboxScrollbarH",&tInterfaceSettings.ChatboxScrollbarH, -1);
		ReadKeyword("data/frontend/frontend.cfg",section,"ChatboxScrollbarAlwaysVisible",&tInterfaceSettings.ChatboxScrollbarAlwaysVisible, false);
	}

    // Set the appropriate chatbox width
    if(tGameInfo.iGameType == GME_LOCAL)
        cChatbox.setWidth(600);
    else
        cChatbox.setWidth(tInterfaceSettings.ChatBoxW-4);  // -4 - leave some space

	// Setup the loading boxes
	int NumBars = tGameInfo.iGameType == GME_LOCAL ? 4 : 2;
	for (byte i=0; i<NumBars; i++)  
		if (!InitializeBar(i))
			return false;


	return true;
}

/////////////////
// Initialize one of the game bars
bool CClient::InitializeBar(byte number)  {
	int x, y, label_x, label_y, direction, numforestates, numbgstates;
	std::string dir,key;
	std::string fname = "data/frontend/";
	CBar **bar;
	Uint32 foreCl;

	// Fill in the details according to the index given
	switch (number)  {
	case 0: 
		key = "FirstHealthBar"; 
		fname += "healthbar1.png";
		bar = &cHealthBar1;
		foreCl = MakeColour(64, 255, 64);

		// Defaults
		x = 70;
		y = 405;
		label_x = 163;
		label_y = 395;
		numforestates = numbgstates = 1;

		break;

	case 1: 
		key = "FirstWeaponBar";
		fname += "weaponbar1.png";
		bar = &cWeaponBar1;
		foreCl = MakeColour(64, 64, 255);

		// Defaults
		x = 70;
		y = 430;
		label_x = 163;
		label_y = 425;
		numforestates = numbgstates = 2;

		break;

	case 2: 
		key = "SecondHealthBar"; 
		fname += "healthbar2.png";
		bar = &cHealthBar2;
		foreCl = MakeColour(64, 255, 64);

		// Defaults
		x = 450;
		y = 405;
		label_x = 550;
		label_y = 395;
		numforestates = numbgstates = 1;


		break;

	case 3: 
		key = "SecondWeaponBar";
		fname += "weaponbar2.png";
		bar = &cWeaponBar2;
		foreCl = MakeColour(64, 64, 255);

		// Defaults
		x = 450;
		y = 430;
		label_x = 550;
		label_y = 420;
		numforestates = numbgstates = 2;

		break;
	default: return false;
	}

	// Local and network have different layouts and sections in the config file
	std::string section = "";
	if (tGameInfo.iGameType == GME_LOCAL)
		section = "LocalGameInterface";
	else
		section = "NetworkGameInterface";

	// Read the info
	ReadInteger("data/frontend/frontend.cfg",section, key+"X",&x, x);
	ReadInteger("data/frontend/frontend.cfg",section, key+"Y", &y, y);
	ReadInteger("data/frontend/frontend.cfg",section, key+"LabelX", &label_x, label_x);
	ReadInteger("data/frontend/frontend.cfg",section, key+"LabelY",&label_y, label_y);
	ReadString("data/frontend/frontend.cfg",section, key+"Direction", dir, "lefttoright");

	// Convert the direction
	if (!stringcasecmp(dir,"lefttoright"))
		direction = BAR_LEFTTORIGHT;
	else if (!stringcasecmp(dir,"righttoleft"))
		direction = BAR_RIGHTTOLEFT;
	else if (!stringcasecmp(dir,"toptobottom"))
		direction = BAR_TOPTOBOTTOM;
	else if (!stringcasecmp(dir,"bottomtotop"))
		direction = BAR_BOTTOMTOTOP;
	else
		return false;

	// Create the bar
	*bar = new CBar(LoadImage(fname, true), x, y, label_x, label_y, direction, numforestates, numbgstates);
	if ( !(*bar) )
		return false;

	// Some default colors in case the image does not exist
	(*bar)->SetBgColor(MakeColour(128,128,128));
	(*bar)->SetForeColor(foreCl);

	return true;

}

//////////////////
// Draw a box
void CClient::DrawBox(SDL_Surface *dst, int x, int y, int w)
{
	// Check
	if (!bmpBoxBuffer || !bmpBoxLeft || !bmpBoxRight)  {
		DrawRect(dst, x, y, x+w, y+tLX->cFont.GetHeight(), tLX->clBoxLight); // backward compatibility
		return;
	}

	int middle_w = w - bmpBoxLeft->w - bmpBoxRight->w;
	if (middle_w < 0)  // Too small
		return; // TODO: any better way?

	DrawImage(dst, bmpBoxLeft, x, y);  // Left part
	DrawImageAdv(dst, bmpBoxBuffer, 0, 0, x + bmpBoxLeft->w, y, middle_w, bmpBoxBuffer->h);  // Middle part
	DrawImage(dst, bmpBoxRight, x + bmpBoxLeft->w + middle_w, y); // Right part
}


///////////////////
// Main drawing routines
void CClient::Draw(SDL_Surface *bmpDest)
{
	static ushort i,num;
	float dt = tLX->fDeltaTime;

	// TODO: allow more worms
	num = (ushort)MIN(2,iNumWorms);


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

	// Local and network use different background images
	SDL_Surface *bgImage = gfxGame.bmpGameNetBackground;
	if (tGameInfo.iGameType == GME_LOCAL)
		bgImage = gfxGame.bmpGameLocalBackground;

    // TODO: allow more viewports
    // Draw the borders
	if (tGameInfo.iGameType == GME_LOCAL)  {
		if (bgImage)  // Doesn't have to exist (backward compatibility)
			DrawImageAdv(bmpDest, bgImage, 0, 0, 0, 480 - bgImage->h, 640, bgImage->h);
		else
			DrawRectFill(bmpDest,0,382,640,480,tLX->clGameBackground);
	} else {
		if (bgImage)  { // Doesn't have to exist (backward compatibility)
			DrawImageAdv(bmpDest, bgImage, 0, 0, 0, 480 - bgImage->h, tInterfaceSettings.ChatBoxX, bgImage->h);
			DrawImageAdv(
						bmpDest,
						bgImage,
						tInterfaceSettings.ChatBoxX+tInterfaceSettings.ChatBoxW,
						0,
						tInterfaceSettings.ChatBoxX+tInterfaceSettings.ChatBoxW,
						480 - bgImage->h,
						640 - tInterfaceSettings.ChatBoxX+tInterfaceSettings.ChatBoxW,
						bgImage->h);
		} else {
			DrawRectFill(bmpDest,0,382,165,480,tLX->clGameBackground);  // Health area
			DrawRectFill(bmpDest,511,382,640,480,tLX->clGameBackground);  // Minimap area
		}
	}
	
    if(cViewports[1].getUsed())
        DrawRectFill(bmpDest,318,0,322, bgImage ? (480-bgImage->h) : (384), tLX->clViewportSplit);

	// Top bar
	if (tLXOptions->tGameinfo.bTopBarVisible)  {
		SDL_Surface *top_bar = tGameInfo.iGameType == GME_LOCAL ? gfxGame.bmpGameLocalTopBar : gfxGame.bmpGameNetTopBar;
		if (top_bar)
			DrawImage( bmpDest, top_bar, 0, 0);
		else
			DrawRectFill( bmpDest, 0, 0, 640, tLX->cFont.GetHeight() + 4, tLX->clGameBackground ); // Backward compatibility
	}


	// Draw the viewports
	if((iNetStatus == NET_CONNECTED && iGameReady) || (iNetStatus == NET_PLAYING)) {

        // Draw the viewports
        for( i=0; i<NUM_VIEWPORTS; i++ ) {
            if( cViewports[i].getUsed() )  {
				cViewports[i].Process(cRemoteWorms, cViewports, cMap->GetWidth(), cMap->GetHeight(), iGameType);
                DrawViewport(bmpDest, (byte)i);
			}
        }

        // Mini-Map
		if (iNetStatus == NET_PLAYING)
			cMap->DrawMiniMap( bmpDest, tInterfaceSettings.MiniMapX, tInterfaceSettings.MiniMapY, dt, cRemoteWorms, iGameType );
		else
			DrawImage( bmpDest, cMap->GetMiniMap(), tInterfaceSettings.MiniMapX, tInterfaceSettings.MiniMapY);

		//
		// Players not yet ready
		//

		if (iNetStatus == NET_CONNECTED)  {
			bool ready = true;

			// Go through and draw the first two worms select menus
			for(i=0;i<num;i++) {

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
	}

	// DEBUG
	//DrawRectFill(bmpDest,0,0,100,40,tLX->clBlack);
	//tLX->cFont.Draw(bmpDest,0,0,tLX->clWhite,"iNetStatus = %i",iNetStatus);
	//tLX->cFont.Draw(bmpDest,0,20,tLX->clWhite,"iGameReady = %i",iGameReady);

	// Draw the chatbox for either a local game, or remote game
	if(tGameInfo.iGameType == GME_LOCAL)
		DrawLocalChat(bmpDest);
	else
		DrawRemoteChat(bmpDest);

	bool bScoreAndSett = true;

	// FPS
	if(tLXOptions->iShowFPS && tLXOptions->tGameinfo.bTopBarVisible) {
		// Get the string and its width
		static std::string fps_str;
		fps_str = "FPS: " + itoa(GetFPS());

		DrawBox( bmpDest, tInterfaceSettings.FpsX, tInterfaceSettings.FpsY, tInterfaceSettings.FpsW);  // Draw the box around it
		tLX->cFont.Draw( // Draw the text
					bmpDest,
					tInterfaceSettings.FpsX + 2,
					tInterfaceSettings.FpsY,
					tLX->clFPSLabel,
					fps_str);
	}

	// Ping on the top right
	if(tLXOptions->iShowPing && tGameInfo.iGameType == GME_JOIN && tLXOptions->tGameinfo.bTopBarVisible)  {

		// Draw the box around it
		DrawBox( bmpDest, tInterfaceSettings.PingX, tInterfaceSettings.PingY, tInterfaceSettings.PingW); 

		tLX->cFont.Draw( // Draw the text
					bmpDest,
					tInterfaceSettings.PingX + 2,
					tInterfaceSettings.PingY,
					tLX->clPingLabel,
					"Ping: " + itoa(iMyPing));

		// Send every second
		if (tLX->fCurTime - fMyPingRefreshed > 1) {
			CBytestream ping;

			ping.Clear();
			ping.writeInt(-1,4);
			ping.writeString("lx::ping");

			ping.Send(cClient->getChannel()->getSocket());

			fMyPingSent = tLX->fCurTime;
			fMyPingRefreshed = tLX->fCurTime;
		}
	}

	// Game over
    if(iGameOver) {
        if(tLX->fCurTime - fGameOverTime > GAMEOVER_WAIT)  {
			bScoreAndSett = false;
		    DrawGameOver(bmpDest);
			// TODO: this better
			if (cServer)
				if (!cServer->getScreenshotToken() && tGameInfo.iGameType == GME_HOST && tGameInfo.bTournament)
					cServer->setTakeScreenshot(true);
		}
        else
            tLX->cOutlineFont.DrawCentre(bmpDest, 320, 200, tLX->clNormalText, "Game Over");
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
		const int text_start = tInterfaceSettings.ChatterX+tLX->cOutlineFont.GetWidth("Talk: ");
		tLX->cOutlineFont.Draw(bmpDest, tInterfaceSettings.ChatterX, tInterfaceSettings.ChatterY, tLX->clGameChatter, "Talk: " + sChat_Text);
		if (iChat_CursorVisible)  {
			DrawVLine(bmpDest,
					tInterfaceSettings.ChatterY,
					tInterfaceSettings.ChatterY + tLX->cOutlineFont.GetHeight(),
					text_start+tLX->cOutlineFont.GetWidth(Utf8SubStr(sChat_Text, 0, iChat_Pos)),
					tLX->clGameChatCursor );
		}
	}

#ifdef WITH_MEDIAPLAYER
	DrawMediaPlayer(bmpDest);
#endif

	// Console
    Con_Draw(bmpDest);


    //tLX->cOutlineFont.Draw(bmpDest, 4,20, tLX->clNormalText, "%s",tLX->debug_string);
    //tLX->cOutlineFont.Draw(bmpDest, 4,40, tLX->clNormalText, "%f",tLX->debug_float);
}


///////////////////
// Draw a viewport
void CClient::DrawViewport(SDL_Surface *bmpDest, byte viewport_index)
{
    // Check the parameters
	if (viewport_index >= NUM_VIEWPORTS)
		return;

    CViewport *v = &cViewports[viewport_index];
	if (!v->getUsed())
		return;

    //CWorm *worm = v->getTarget();

	// Set the clipping
	SDL_Rect rect = v->getRect();
	SDL_SetClipRect(bmpDest,&rect);

    // Weather
    //cWeather.Draw(bmpDest, v);

	cMap->Draw(bmpDest, v);

	// The following will be drawn only when playing
	if (iNetStatus == NET_PLAYING)  {
		if( tLXOptions->iShadows ) {
			// Draw the projectile shadows
			DrawProjectileShadows(bmpDest, v);

			// Draw the worm shadows
			CWorm *w = cRemoteWorms;
			for(short i=0;i<MAX_WORMS;i++,w++) {
				if(w->isUsed() && w->getAlive())
					w->DrawShadow(bmpDest, v);
			}
		}

		// Draw the entities
		DrawEntities(bmpDest, v);

		// Draw the projectiles
		DrawProjectiles(bmpDest, v);

		// Draw the bonuses
		DrawBonuses(bmpDest, v);

		// Draw all the worms in the game
		ushort i;
		CWorm *w = cRemoteWorms;
		for(i=0;i<MAX_WORMS;i++,w++) {
			if(w->isUsed() && w->getAlive())
				w->Draw(bmpDest, v);
		}
	}

	// Disable the special clipping
	SDL_SetClipRect(bmpDest,NULL);

	//
	// Draw the worm details
	//

	// The positions are different for different viewports
	int *HealthLabelX, *WeaponLabelX, *LivesX, *KillsX, *TeamX, *SpecMsgX;
	int *HealthLabelY, *WeaponLabelY, *LivesY, *KillsY, *TeamY, *SpecMsgY;
	int	*LivesW, *KillsW, *TeamW, *SpecMsgW;
	CBar *HealthBar, *WeaponBar;

	if (viewport_index == 0)  {  // Viewport 1
		HealthLabelX = &tInterfaceSettings.HealthLabel1X;	HealthLabelY = &tInterfaceSettings.HealthLabel1Y;
		WeaponLabelX = &tInterfaceSettings.WeaponLabel1X;	WeaponLabelY = &tInterfaceSettings.WeaponLabel1Y;

		LivesX = &tInterfaceSettings.Lives1X;
		LivesY = &tInterfaceSettings.Lives1Y;
		LivesW = &tInterfaceSettings.Lives1W;

		KillsX = &tInterfaceSettings.Kills1X;
		KillsY = &tInterfaceSettings.Kills1Y;
		KillsW = &tInterfaceSettings.Kills1W;

		TeamX = &tInterfaceSettings.Team1X;
		TeamY = &tInterfaceSettings.Team1Y;
		TeamW = &tInterfaceSettings.Team1W;

		SpecMsgX = &tInterfaceSettings.SpecMsg1X;	
		SpecMsgY = &tInterfaceSettings.SpecMsg1Y;
		SpecMsgW = &tInterfaceSettings.SpecMsg1W;

		HealthBar = cHealthBar1;
		WeaponBar = cWeaponBar1;
	} else { // Viewport 2
		HealthLabelX = &tInterfaceSettings.HealthLabel2X;	HealthLabelY = &tInterfaceSettings.HealthLabel2Y;
		WeaponLabelX = &tInterfaceSettings.WeaponLabel2X;	WeaponLabelY = &tInterfaceSettings.WeaponLabel2Y;

		LivesX = &tInterfaceSettings.Lives2X;
		LivesY = &tInterfaceSettings.Lives2Y;
		LivesW = &tInterfaceSettings.Lives2W;

		KillsX = &tInterfaceSettings.Kills2X;
		KillsY = &tInterfaceSettings.Kills2Y;
		KillsW = &tInterfaceSettings.Kills2W;

		TeamX = &tInterfaceSettings.Team2X;
		TeamY = &tInterfaceSettings.Team2Y;
		TeamW = &tInterfaceSettings.Team2W;

		SpecMsgX = &tInterfaceSettings.SpecMsg2X;	
		SpecMsgY = &tInterfaceSettings.SpecMsg2Y;
		SpecMsgW = &tInterfaceSettings.SpecMsg2W;		

		HealthBar = cHealthBar2;
		WeaponBar = cWeaponBar2;
	}



	// The following is only drawn for viewports with a worm target
    if( v->getType() > VW_CYCLE )
        return;

    CWorm *worm = v->getTarget();

	// Health
	static const std::string health = "Health:";
	tLX->cFont.Draw(bmpDest, *HealthLabelX, *HealthLabelY, tLX->clHealthLabel,health);
	if (HealthBar)  {
		HealthBar->SetPosition(worm->getHealth());
		HealthBar->Draw(bmpDest);
	}

	// Weapon
	wpnslot_t *Slot = worm->getCurWeapon();
	static std::string weapon_name;
	weapon_name = Slot->Weapon->Name;
	stripdot(weapon_name, 100);
	weapon_name += ":";
	tLX->cFont.Draw(bmpDest, *WeaponLabelX, *WeaponLabelY, tLX->clWeaponLabel, weapon_name);
	
	if (WeaponBar)  {
		if(Slot->Reloading)  {
			WeaponBar->SetForeColor(MakeColour(128,64,64));  // In case it's not loaded properly
			WeaponBar->SetCurrentForeState(1);  // Loading state
			WeaponBar->SetCurrentBgState(1);
		} else {
			WeaponBar->SetForeColor(MakeColour(64,64,255));
			WeaponBar->SetCurrentForeState(0);  // "Shooting" state 
			WeaponBar->SetCurrentBgState(0);
		}
		WeaponBar->SetPosition((int) ( Slot->Charge * 100.0f ));
		WeaponBar->Draw( bmpDest );
	}

	// The following are items on top bar, so don't draw them when we shouldn't
	if (!tLXOptions->tGameinfo.bTopBarVisible)
		return;


	// Lives
	DrawBox(bmpDest, *LivesX, *LivesY, *LivesW); // Box first

	static std::string lives_str;
	lives_str = "Lives: ";
	switch (worm->getLives())  {
	case WRM_OUT:
		lives_str += "Out";	
		tLX->cFont.Draw(bmpDest, *LivesX+2, *LivesY, tLX->clLivesLabel, lives_str); // Text
		break;
	case WRM_UNLIM:
		tLX->cFont.Draw(bmpDest, *LivesX+2, *LivesY, tLX->clLivesLabel, lives_str); // Text
		DrawImage(bmpDest, gfxGame.bmpInfinite, *LivesX + *LivesW - gfxGame.bmpInfinite->w, *LivesY); // Infinite
		break;
	default:
		if (worm->getLives() >= 0)  {
			lives_str += itoa( worm->getLives() );
			tLX->cFont.Draw(bmpDest,*LivesX + 2, *LivesY, tLX->clLivesLabel, lives_str);
		}
	}

	// Kills
	DrawBox( bmpDest, *KillsX, *KillsY, *KillsW );
	tLX->cFont.Draw(bmpDest,*KillsX+2, *KillsY, tLX->clKillsLabel, "Kills: " + itoa( worm->getKills() ));


	// Special message
	static std::string spec_msg;

	switch (iGameType)  {
	case GMT_TAG:
		// Am i IT?
		if(worm->getTagIT())  {
			spec_msg = "You are IT!";
			DrawBox( bmpDest, *SpecMsgX, *SpecMsgY, *SpecMsgW);
			tLX->cFont.Draw(bmpDest, *SpecMsgX+2, *SpecMsgY, tLX->clSpecMsgLabel, spec_msg);
		}
		break;


    case GMT_DEMOLITION: {
		    // Dirt count
			int count = worm->getDirtCount();
			spec_msg = "Dirt count: ";

			// Draw short versions
			if( count < 1000 )
				spec_msg += itoa(count);
			else
				spec_msg += itoa(count/1000)+"k";

			DrawBox( bmpDest, *SpecMsgX, *SpecMsgY, *SpecMsgW);
			tLX->cFont.Draw(bmpDest, *SpecMsgX+2, *SpecMsgY, tLX->clSpecMsgLabel, spec_msg);
		}
		break;

	case GMT_TEAMDEATH:  {
			if (worm->getTeam() >= 0 && worm->getTeam() < 4)  {
				int box_h = bmpBoxLeft ? bmpBoxLeft->h : tLX->cFont.GetHeight();
				DrawBox( bmpDest, *TeamX, *TeamY, *TeamW);
				tLX->cFont.Draw( bmpDest, *TeamX+2, *TeamY, tLX->clTeamColors[worm->getTeam()], "Team");
				DrawImage( bmpDest, gfxGame.bmpTeamColours[worm->getTeam()], 
						   *TeamX + *TeamW - gfxGame.bmpTeamColours[worm->getTeam()]->w - 2, 
						   *TeamY + MAX(1, box_h/2 - gfxGame.bmpTeamColours[worm->getTeam()]->h/2));
			}
		}
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

	DrawRectFill(bmpDest, x-2,y-2,x+2,y+2,tLX->clWhite);*/
}

///////////////////
// Draw the projectiles
void CClient::DrawProjectiles(SDL_Surface *bmpDest, CViewport *v)
{
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
    if(!iGameReady)
        return;
	
	float dt = tLX->fDeltaTime;
	float ScrollSpeed=5;
    bool  con = Con_IsUsed();

	for(short i=0;i<iChat_Numlines;i++) {
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
	keyboard_t *Keyboard = GetKeyboard();
	mouse_t *Mouse = GetMouse();

	SetGameCursor(CURSOR_ARROW);

	DrawScore(bmpDest,gfxGame.bmpGameover);

	// Network games have a different game over screen
	if(tGameInfo.iGameType != GME_LOCAL) {
		DrawRemoteGameOver(bmpDest);
		return;
	}


	// Buttons
//	int width = gfxGame.bmpGameover->w;  // TODO: not used
	int height = gfxGame.bmpGameover->h;

//	int x = 320 - width/2;  // TODO: not used
	int y = 200 - height/2;

	int j = y+height-27;
	static CButton ok = CButton(BUT_OK,bmpMenuButtons);

	ok.Setup(0, 305,j, 30, 15);
    ok.Create();

	// OK
	if(ok.InBox(Mouse->X,Mouse->Y)) {
		ok.MouseOver(Mouse);
		SetGameCursor(CURSOR_HAND);
	}
	ok.Draw2(bmpDest);


	if(Mouse->Up) {
		if(ok.InBox(Mouse->X,Mouse->Y)) {
			// Quit
			tLX->iQuitEngine = true;
			SetGameCursor(CURSOR_NONE);
		}
	}

	// Quit
	if (Keyboard->KeyDown[SDLK_RETURN] || Keyboard->KeyDown[SDLK_KP_ENTER] || Keyboard->KeyDown[SDLK_ESCAPE])  {
		tLX->iQuitEngine = true;
		SetGameCursor(CURSOR_NONE);
	}

	// Draw the mouse
	DrawCursor(bmpDest);
}


///////////////////
// Draw a remote game over screen
void CClient::DrawRemoteGameOver(SDL_Surface *bmpDest)
{
	mouse_t *Mouse = GetMouse();
	
	
	SetGameCursor(CURSOR_ARROW);

	// Buttons
	int width = gfxGame.bmpGameover->w;
	int height = gfxGame.bmpGameover->h;

	int x = 320 - width/2;
	int y = 200 - height/2;

	int j = y+height-27;
	static CButton leave = CButton(BUT_LEAVE,bmpMenuButtons);

	leave.Setup(0, x+20,j, 60, 15);
    leave.Create();

	// OK
	if(leave.InBox(Mouse->X,Mouse->Y)) {
		leave.MouseOver(Mouse);
		SetGameCursor(CURSOR_HAND);
	}
	leave.Draw2(bmpDest);


	if(Mouse->Up) {
		if(leave.InBox(Mouse->X,Mouse->Y)) {

			// If this is a host, we go back to the lobby
			// The host can only quit the game via the lobby
			if(tGameInfo.iGameType == GME_HOST) {
				SetGameCursor(CURSOR_NONE);

				cServer->gotoLobby();

			} else {
				SetGameCursor(CURSOR_NONE);

				// Quit
				QuittoMenu();
			}
		}
	}

	// Draw a timer when we're going back to the lobby
	float timeleft = LX_ENDWAIT - (tLX->fCurTime - fGameOverTime);
	timeleft = MAX(timeleft,(float)0);
	tLX->cFont.Draw(bmpDest, x+width-180, j+2, tLX->clReturningToLobby, "Returning to lobby in " + itoa((int)timeleft));


	// Draw the mouse
	DrawCursor(bmpDest);

}


///////////////////
// Draw the in-game menu
void CClient::DrawGameMenu(SDL_Surface *bmpDest)
{
	SetGameCursor(CURSOR_ARROW);
	mouse_t *Mouse = GetMouse();

	DrawScore(bmpDest, gfxGame.bmpScoreboard);

	static CButton quit = CButton(BUT_QUITGAME,bmpMenuButtons);
	static CButton resume = CButton(BUT_RESUME,bmpMenuButtons);

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
		SetGameCursor(CURSOR_HAND);
	}
	quit.Draw2(bmpDest);

	// Resume
	if(resume.InBox(Mouse->X,Mouse->Y)) {
		resume.MouseOver(Mouse);
		SetGameCursor(CURSOR_HAND);
	}
	resume.Draw2(bmpDest);

	if(Mouse->Up) {
		if(quit.InBox(Mouse->X,Mouse->Y)) {

			// If we're the host tell all the clients that we're going back to the lobby
			// We don't directly go back to the lobby, instead we send the message
			// and the client on this machine does the goto lobby when it gets the packet
			if(tGameInfo.iGameType == GME_HOST) {
				SetGameCursor(CURSOR_NONE);

				cServer->gotoLobby();

			} else {
				SetGameCursor(CURSOR_NONE);

				// Quit
				QuittoMenu();
			}
		}

		if(resume.InBox(Mouse->X,Mouse->Y))  {
			// Resume
			iGameMenu = false;

			bRepaintChatbox = true;

			SetGameCursor(CURSOR_NONE);
		}
	}

	// Draw the mouse
	DrawCursor(bmpDest);
}


///////////////////
// Draw the score
void CClient::DrawScore(SDL_Surface *bmpDest, SDL_Surface *bmpImage)
{
	if (bUpdateScore)  {
		DrawRectFill(bmpScoreBuffer,0,0,bmpScoreBuffer->w,bmpScoreBuffer->h,COLORKEY(bmpScoreBuffer));
		UpdateScoreBuf(bmpScoreBuffer,bmpImage);
	}

	DrawImage(
		bmpDest, bmpScoreBuffer,
		320 - bmpImage->w / 2,
		200 - bmpImage->h / 2);
}

///////////////////
// Display the score
void CClient::UpdateScoreBuf(SDL_Surface *bmpDest, SDL_Surface *bmpImage)
{
	short i,j,n;

	bUpdateScore = false;

	// Teams
	static const std::string teamnames[] = {"Blue", "Red", "Green", "Yellow"};

	int width = bmpImage->w;
	int height = bmpImage->h;

	const int x = 0;
	const int y = 0;

	// Draw the back image
	CopySurface(bmpDest,bmpImage,x,y,x,y,bmpImage->w,bmpImage->h);

	// Deathmatch scoreboard
	switch(iGameType) {
	case GMT_DEATHMATCH:  {

		tLX->cFont.Draw(bmpDest, x+15, y+45, tLX->clNormalLabel,"Players");
		if(iLives != WRM_UNLIM)
			tLX->cFont.Draw(bmpDest, x+300, y+45, tLX->clNormalLabel,"Lives");

		// Draw ping if host
		if (tGameInfo.iGameType == GME_HOST)  {
			tLX->cFont.Draw(bmpDest, x+340, y+45, tLX->clNormalLabel,"Kills");
			tLX->cFont.Draw(bmpDest, x+380, y+45, tLX->clNormalLabel,"Ping");
		} else
			tLX->cFont.Draw(bmpDest, x+380, y+45, tLX->clNormalLabel,"Kills");

		DrawHLine(bmpDest, x+15,x+width-15, y+60,tLX->clLine);
		DrawHLine(bmpDest, x+15,x+width-15, y+height-30,tLX->clLine);

		// Draw the players
		j = y+65;
		for(i=0;i<iScorePlayers;i++) {
			CWorm *p = &cRemoteWorms[iScoreboard[i]];

			DrawImage(bmpDest, p->getPicimg(), x+15, j);

			tLX->cFont.Draw(bmpDest, x+40, j, tLX->clNormalLabel, p->getName());

			if(p->getLives() >= 0)
				tLX->cFont.DrawCentre(bmpDest, x+317, j, tLX->clNormalLabel, itoa(p->getLives()));
			else if(p->getLives() == WRM_OUT)
				tLX->cFont.DrawCentre(bmpDest, x+317, j, tLX->clNormalLabel, "out");

			// Draw ping if host
			if (tGameInfo.iGameType == GME_HOST)  {
				tLX->cFont.DrawCentre(bmpDest, x+353, j, tLX->clNormalLabel, itoa(p->getKills()));
				CClient *remoteClient = cServer->getClient(p->getID());
				if (remoteClient && p->getID())
					tLX->cFont.DrawCentre(bmpDest, x+393, j, tLX->clNormalLabel, itoa(remoteClient->getPing()));
			} else
				tLX->cFont.DrawCentre(bmpDest, x+393, j, tLX->clNormalLabel, itoa(p->getKills()));

			j+=20;
		}
	}
	break; // TEAM DEATHMATCH


    // Demolitions scoreboard
	case GMT_DEMOLITION: {

		tLX->cFont.Draw(bmpDest, x+15, y+45, tLX->clNormalLabel,"Players");
		if(iLives != WRM_UNLIM)
			tLX->cFont.Draw(bmpDest, x+270, y+45, tLX->clNormalLabel,"Lives");
		tLX->cFont.Draw(bmpDest, x+340, y+45, tLX->clNormalLabel,"Dirt Count");
		DrawHLine(bmpDest, x+15,x+width-15, y+60,tLX->clLine);
		DrawHLine(bmpDest, x+15,x+width-15, y+height-30,tLX->clLine);

        int dirtcount = 0;

		// Draw the players
		j = y+65;
		for(i=0;i<iScorePlayers;i++) {
			CWorm *p = &cRemoteWorms[iScoreboard[i]];

			DrawImage(bmpDest, p->getPicimg(), x+15, j);

			tLX->cFont.Draw(bmpDest, x+40, j, tLX->clNormalLabel,  p->getName());

			if(p->getLives() >= 0)
				tLX->cFont.DrawCentre(bmpDest, x+287, j, tLX->clNormalLabel, itoa(p->getLives()));
			else if(p->getLives() == WRM_OUT)
				tLX->cFont.DrawCentre(bmpDest, x+287, j, tLX->clNormalLabel, "out");


			tLX->cFont.DrawCentre(bmpDest, x+372, j, tLX->clNormalLabel, itoa(p->getDirtCount()));
            dirtcount += p->getDirtCount();

			j+=20;
		}

        // Draw the total
        j+=10;
        DrawHLine(bmpDest,x+15,x+width-15, j, tLX->clLine);
        j+=5;
        tLX->cFont.Draw(bmpDest, x+250, j, tLX->clNormalLabel,"Total");
        tLX->cFont.DrawCentre(bmpDest, x+372, j, tLX->clNormalLabel, itoa(dirtcount / 1000) + "k / " + itoa((int)(((float)cMap->GetDirtCount()*0.8f) / 1000)) + "k");
	}
	break;  // DEMOLITIONS


	// Tag scoreboard
	case GMT_TAG: {

		tLX->cFont.Draw(bmpDest, x+15, y+45, tLX->clNormalLabel,"Players");
		if(iLives != WRM_UNLIM)
			tLX->cFont.Draw(bmpDest, x+220, y+45, tLX->clNormalLabel,"Lives");

		// Draw the ping if host
		if(tGameInfo.iGameType == GME_HOST)  {
			tLX->cFont.Draw(bmpDest, x+280, y+45, tLX->clNormalLabel,"Kills");
			tLX->cFont.Draw(bmpDest, x+322, y+45, tLX->clNormalLabel,"Ping");
		}  else
			tLX->cFont.Draw(bmpDest, x+290, y+45, tLX->clNormalLabel,"Kills");


		tLX->cFont.Draw(bmpDest, x+360, y+45, tLX->clNormalLabel,"Tag time");
		DrawHLine(bmpDest, x+15,x+width-15, y+60,tLX->clLine);
		DrawHLine(bmpDest, x+15,x+width-15, y+height-30,tLX->clLine);

		// Draw the players
		j = y+65;
		CWorm *p;
		for(i=0;i<iScorePlayers;i++) {
			p = &cRemoteWorms[iScoreboard[i]];

			DrawImage(bmpDest, p->getPicimg(), x+15, j);

			tLX->cFont.Draw(bmpDest, x+40, j, tLX->clNormalLabel, p->getName());

			// Check if it
			if(p->getTagIT())
				tLX->cFont.Draw( bmpDest, x+160, j, MakeColour(255,0,0), "IT");

			if(p->getLives() >= 0)
				tLX->cFont.DrawCentre(bmpDest, x+237, j, tLX->clNormalLabel, itoa(p->getLives()));
			else if(p->getLives() == WRM_OUT)
				tLX->cFont.DrawCentre(bmpDest, x+237, j, tLX->clNormalLabel, "out");

			// Draw the ping if host
			if (tGameInfo.iGameType == GME_HOST)  {
				tLX->cFont.DrawCentre(bmpDest, x+293, j, tLX->clNormalLabel, itoa(p->getKills()));
				CClient *remoteClient = cServer->getClient(p->getID());
				if (remoteClient && p->getID())
					tLX->cFont.DrawCentre(bmpDest, x+333, j, tLX->clNormalLabel, itoa(remoteClient->getPing()));
			} else
				tLX->cFont.DrawCentre(bmpDest, x+303, j, tLX->clNormalLabel, itoa(p->getKills()));

			// Total time of being IT
			int h,m,s;
			static std::string buf;
			ConvertTime(p->getTagTime(), &h,&m,&s);
			buf = itoa(m)+":"+(s<10 ? "0":"")+itoa(s);
			Uint32 col = tLX->clNormalLabel;
			if(p->getTagIT())
				col = MakeColour(255,0,0);
			tLX->cFont.Draw(bmpDest, x+375, j, col, buf);

			j+=20;
		}
	}
	break;  // TAG


	// Team deathmatch scoreboard
	case GMT_TEAMDEATH: {

		DrawHLine(bmpDest, x+15,x+width-15, y+height-30,tLX->clLine);

		// Go through each team
		j = y+50;
		int team,score;
		Uint32 colour;
		for(n=0;n<4;n++) {
			team = iTeamList[n];
			score = iTeamScores[team];

			// Check if the team has any players
			if(score == -1)
				continue;

			colour = tLX->clTeamColors[team];

			tLX->cFont.Draw(bmpDest, x+15, j, colour, teamnames[team] + " team  (" + itoa(score) + ")");
			if(iLives != WRM_UNLIM)
				tLX->cFont.Draw(bmpDest, x+300, j, colour,"Lives");
			if(tGameInfo.iGameType == GME_HOST)  {
				tLX->cFont.Draw(bmpDest, x+343, j, colour,"Kills");
				tLX->cFont.Draw(bmpDest, x+380, j, colour,"Ping");
			}
			else
				tLX->cFont.Draw(bmpDest, x+380, j, colour,"Kills");
			DrawHLine(bmpDest, x+15,x+width-15, j+15,tLX->clLine);
			j+=20;


			// Draw the players
			CWorm *p;
			for(i=0;i<iScorePlayers;i++) {
				p = &cRemoteWorms[iScoreboard[i]];

				if(p->getTeam() != team)
					continue;

				DrawImage(bmpDest, p->getPicimg(), x+15, j);

				tLX->cFont.Draw(bmpDest, x+40, j, tLX->clNormalLabel, p->getName());

				if(p->getLives() >= 0)
					tLX->cFont.DrawCentre(bmpDest, x+317, j, tLX->clNormalLabel, itoa(p->getLives()));
				else if(p->getLives() == WRM_OUT)
					tLX->cFont.DrawCentre(bmpDest, x+317, j, tLX->clNormalLabel, "out");

				// Draw ping if host
				if (tGameInfo.iGameType == GME_HOST)  {
					tLX->cFont.DrawCentre(bmpDest, x+353, j, tLX->clNormalLabel, itoa(p->getKills()));
					CClient *remoteClient = cServer->getClient(p->getID());
					if (remoteClient && p->getID())
						tLX->cFont.DrawCentre(bmpDest, x+396, j, tLX->clNormalLabel, itoa(remoteClient->getPing()));
				} else
					tLX->cFont.DrawCentre(bmpDest, x+393, j, tLX->clNormalLabel, itoa(p->getKills()));

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
	if(!tGameInfo.iBonusesOn)
		return;

	CBonus *b = cBonuses;

	for(short i=0;i<MAX_BONUSES;i++,b++) {
		if(!b->getUsed())
			continue;

		b->Draw(bmpDest, v, iShowBonusName);
	}
}


///////////////////
// Draw text that is shadowed
void CClient::DrawText(SDL_Surface *bmpDest, int centre, int x, int y, Uint32 fgcol, const std::string& buf)
{
	if(centre) {
		//tLX->cOutlineFont.DrawCentre(bmpDest, x+1, y+1, 0,"%s", buf);
		tLX->cOutlineFont.DrawCentre(bmpDest, x, y, fgcol, buf);
	}
	else {
		//tLX->cOutlineFont.Draw(bmpDest, x+1, y+1, 0,"%s", buf);
		tLX->cOutlineFont.Draw(bmpDest, x, y, fgcol, buf);
	}
}


///////////////////
// Draw the local chat
void CClient::DrawLocalChat(SDL_Surface *bmpDest)
{
	int y = tInterfaceSettings.LocalChatY;
	lines_riterator it = cChatbox.RBegin();
	byte i;

	for(i = 0; i < 6 && it != cChatbox.REnd(); i++, it++) { // Last 6 messages

		// This chat times out after a few seconds
		if(tLX->fCurTime - it->fTime < 3.0f) {
			tLX->cFont.Draw(bmpDest, tInterfaceSettings.LocalChatX + 1, y+1, tLX->clBlack, it->strLine); // Shadow
			tLX->cFont.Draw(bmpDest, tInterfaceSettings.LocalChatX, y, it->iColour, it->strLine);
			y += tLX->cFont.GetHeight()+1; // +1 - shadow
		}
	}
}


///////////////////
// Draw the remote chat
void CClient::DrawRemoteChat(SDL_Surface *bmpDest)
{
	if (!cChatList)
		return;

	CListview *lv = (CListview *)cChatList;

	// Get any new lines
	line_t *l = NULL;
	int id = (lv->getLastItem() && lv->getItems()) ? lv->getLastItem()->iIndex + 1 : 0;

	while((l = cChatbox.GetNewLine()) != NULL) {

		lv->AddItem("", id, l->iColour);
        lv->AddSubitem(LVS_TEXT, l->strLine, NULL, NULL);
		lv->scrollLast();
		id++;
	}

    // If there are too many lines, remove the top one
	if (lv->getItems())  
		while(lv->getItemCount() > 256) {
			lv->RemoveItem(lv->getItems()->iIndex);
			lv->scrollLast();
		}

	mouse_t *Mouse = GetMouse();

	// Small hack: count the mouse height so we avoid "freezing"
	// the mouse image when the user moves cursor away
	int inbox = MouseInRect(lv->getX(),lv->getY(), lv->getWidth(), lv->getHeight()+GetCursorHeight(CURSOR_ARROW)) ||
				MouseInRect(tInterfaceSettings.ChatboxScrollbarX, tInterfaceSettings.ChatboxScrollbarY, 14, tInterfaceSettings.ChatboxScrollbarH);

	if (lv->NeedsRepaint() || (inbox && (Mouse->deltaX || Mouse->deltaY)) || bRepaintChatbox)  {	// Repainting when new messages/scrolling, 
																				// or when user is moving the mouse over the chat

		// Local and net play use different backgrounds
		SDL_Surface *bgImage = gfxGame.bmpGameNetBackground;
		if (tGameInfo.iGameType == GME_LOCAL)
			bgImage = gfxGame.bmpGameLocalBackground;

		if (bgImage)  // Due to backward compatibility, this doesn't have to exist
			DrawImageAdv(bmpDest,
						 bgImage,
						 tInterfaceSettings.ChatBoxX,
						 0,
						 tInterfaceSettings.ChatBoxX,
						 480 - bgImage->h,
						 tInterfaceSettings.ChatBoxW,
						 bgImage->h);
		else
			DrawRectFill(bmpDest,165,382,511,480,tLX->clGameBackground);
		lv->Draw(bmpDest);
	}


	// Events
	if (Mouse->WheelScrollDown)
		lv->MouseWheelDown(Mouse);
	else if (Mouse->WheelScrollUp)
		lv->MouseWheelUp(Mouse);

	if (inbox)  {
		SetGameCursor(CURSOR_ARROW);

		// Draw the mouse
		DrawCursor(bmpDest);
		lv->MouseOver(Mouse);
		if (Mouse->Down)
			lv->MouseDown(Mouse,true);
		else if (Mouse->Up)
			lv->MouseUp(Mouse,false);
	} else {
		SetGameCursor(CURSOR_NONE);
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
    int x = 320-gfxGame.bmpViewportMgr->w/2;
    int y = 200-gfxGame.bmpViewportMgr->h/2;
    int x2 = x+gfxGame.bmpViewportMgr->w/2+40;


    bViewportMgr = true;

    // Initialize the gui
    ViewportMgr.Initialize();

    bool v2On = true;
    // If there is only 1 player total, turn the second viewport off
    short count = 0;
	short i;
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
    ViewportMgr.SendMessage( v1_Type, CBS_ADDITEM, "Follow", VW_FOLLOW );
    if( cLocalWorms[0]->getLives() == WRM_OUT || cLocalWorms[0]->getType() == PRF_COMPUTER ) {
        ViewportMgr.SendMessage( v1_Type, CBS_ADDITEM, "Cycle", VW_CYCLE);
        ViewportMgr.SendMessage( v1_Type, CBS_ADDITEM, "Free Look", VW_FREELOOK);
        ViewportMgr.SendMessage( v1_Type, CBS_ADDITEM, "Action Cam", VW_ACTIONCAM);
    }

    // If the second player is a human and is still playing: Only show the follow option
    bool show = true;
    if( iNumWorms > 1 )
        if( cLocalWorms[1]->getLives() != WRM_OUT && cLocalWorms[1]->getType() == PRF_HUMAN )
            show = false;

    ViewportMgr.SendMessage( v2_Type, CBS_ADDITEM, "Follow", VW_FOLLOW );
    if( show ) {
        ViewportMgr.SendMessage( v2_Type, CBS_ADDITEM, "Cycle", VW_CYCLE);
        ViewportMgr.SendMessage( v2_Type, CBS_ADDITEM, "Free Look",VW_FREELOOK);
        ViewportMgr.SendMessage( v2_Type, CBS_ADDITEM, "Action Cam",VW_ACTIONCAM);
    }

	// Fill in the target worms boxes
    for(i=0; i<MAX_WORMS; i++ ) {
        if(!cRemoteWorms[i].isUsed() || cRemoteWorms[i].getLives() == WRM_OUT)
            continue;

		ViewportMgr.SendMessage( v1_Target, CBS_ADDITEM, cRemoteWorms[i].getName(), cRemoteWorms[i].getID() );
		ViewportMgr.SendMessage( v1_Target, CBM_SETIMAGE, cRemoteWorms[i].getID(), (DWORD)cRemoteWorms[i].getPicimg());
		ViewportMgr.SendMessage( v2_Target, CBS_ADDITEM, cRemoteWorms[i].getName(), cRemoteWorms[i].getID() );
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
    int x = 320-gfxGame.bmpViewportMgr->w/2;
    int y = 200-gfxGame.bmpViewportMgr->h/2;

	SetGameCursor(CURSOR_ARROW);
	mouse_t *Mouse = GetMouse();

	// Draw the back image
	DrawImage(bmpDest,gfxGame.bmpViewportMgr,x,y);

    tLX->cFont.Draw(bmpDest, x+75,y+50, tLX->clNormalLabel,"Viewport 1");
    tLX->cFont.Draw(bmpDest, x+gfxGame.bmpViewportMgr->w/2+40,y+50, tLX->clNormalLabel,"Viewport 2");

    ViewportMgr.Draw(bmpDest);
    gui_event_t *ev = ViewportMgr.Process();

    if( ViewportMgr.getWidget(v_ok)->InBox(Mouse->X,Mouse->Y) )
        SetGameCursor(CURSOR_HAND);

    // Draw the mouse
    DrawCursor(bmpDest);

    if(!ev)
        return;


    // Get the worm count
    short Wormcount = 0;
    for(short i=0; i<MAX_WORMS; i++ ) {
        if(cRemoteWorms[i].isUsed())
            Wormcount++;
    }

    switch(ev->iControlID) {

        // V2 On
        case v2_On:
            if(ev->iEventMsg == CHK_CHANGED) {
                // If there is only one worm, disable the 2nd viewport
                if( Wormcount <= 1 )
                    ViewportMgr.SendMessage(v2_On, CKM_SETCHECK,(DWORD)0,0);
            }
            break;

        // OK
        case v_ok:
            if(ev->iEventMsg == BTN_MOUSEUP) {

                // If there is only one worm, disable the 2nd viewport
                if( Wormcount <= 1 )
                    ViewportMgr.SendMessage(v2_On, CKM_SETCHECK,(DWORD)0,0);

                // Grab settings
                int a_type = ViewportMgr.SendMessage(v1_Type, CBM_GETCURINDEX, (DWORD)0,0);
                int b_on = ViewportMgr.SendMessage(v2_On, CKM_GETCHECK, (DWORD)0,0);
                int b_type = ViewportMgr.SendMessage(v2_Type, CBM_GETCURINDEX, (DWORD)0,0);
				int v1_target = ViewportMgr.SendMessage(v1_Target, CBM_GETCURINDEX, (DWORD)0,0);
				int v2_target = ViewportMgr.SendMessage(v2_Target, CBM_GETCURINDEX, (DWORD)0,0);

                for( int i=0; i<NUM_VIEWPORTS; i++ ) {
                    cViewports[i].setUsed(false);
                    cViewports[i].reset();
                }

                // Re-setup the viewports
                if( !b_on || iNumWorms <= 1 ) {
                    SetupViewports(cLocalWorms[0], NULL, a_type, VW_FOLLOW);
                } else {
                    SetupViewports(cLocalWorms[0], cLocalWorms[1], a_type, b_type);
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
				SetGameCursor(CURSOR_NONE);
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
    if(!bShowScore)
        return;

    int y = tInterfaceSettings.ScoreboardY;
    int x = tInterfaceSettings.ScoreboardX;
    int w = 240;
    int h = 185;
	if (tGameInfo.iGameType == GME_HOST || cViewports[1].getUsed())  {
		w = 260;
		if (bShowReady)  {
			w = 300;
			h = 185;
		}

		x = tInterfaceSettings.ScoreboardOtherPosX;
		y = tInterfaceSettings.ScoreboardOtherPosY;
	}
    DrawRectFill(bmpDest, x+1, y, x+w-1, y+h-1, tLX->clScoreBackground);
    Menu_DrawBox(bmpDest, x, y, x+w, y+h);

    tLX->cFont.Draw(bmpDest, x+5, y+4, tLX->clNormalLabel, "Players");
    if(!bShowReady) {
        tLX->cFont.Draw(bmpDest, x+180, y+4, tLX->clNormalLabel, "L");
        tLX->cFont.Draw(bmpDest, x+210, y+4, tLX->clNormalLabel, "K");
		if(tGameInfo.iGameType == GME_HOST)
			tLX->cFont.Draw(bmpDest, x+237, y+4, tLX->clNormalLabel, "P");
    }
	else
		if (tGameInfo.iGameType == GME_HOST)
			tLX->cFont.Draw(bmpDest, x+250, y+4, tLX->clNormalLabel, "Ping");

    DrawHLine(bmpDest, x+4, x+w-4, y+20, tLX->clLine);


    // Draw the players
	int j = y+25;
    short i;
	Uint32 iColor;
	CWorm *p;
    for(i=0;i<iScorePlayers;i++) {
        p = &cRemoteWorms[iScoreboard[i]];

		// Get the team colour
		iColor = tLX->clTeamColors[p->getTeam()];

        // If this player is local & human, highlight it
        if(p->getType() == PRF_HUMAN && p->getLocal())
            DrawRectFill(bmpDest, x+2,j-2, x+w-1, j+18, MakeColour(52,52,52));

		tLX->cFont.Draw(bmpDest, x+5, j+1, tLX->clWhite, "#" + itoa(p->getID()));

        // Pic & Name
        DrawImage(bmpDest, p->getPicimg(), x+30, j);
		if (tGameInfo.iGameMode == GMT_TEAMDEATH  && tLXOptions->iColorizeNicks)
			tLX->cFont.DrawAdv(bmpDest, x+56, j, 130, iColor, p->getName());
		else
			tLX->cFont.DrawAdv(bmpDest, x+56, j, 130, tLX->clNormalLabel, p->getName());

        // Score
        if(!bShowReady) {
            if(p->getLives() >= 0)
                tLX->cFont.DrawCentre(bmpDest, x+185, j, tLX->clNormalLabel, itoa(p->getLives()));
            else if(p->getLives() == WRM_OUT)
                tLX->cFont.DrawCentre(bmpDest, x+185, j, tLX->clNormalLabel, "out");

            tLX->cFont.DrawCentre(bmpDest, x+215, j, tLX->clNormalLabel, itoa(p->getKills()));

			if(tGameInfo.iGameType == GME_HOST)  {
				CClient *remoteClient = cServer->getClient(p->getID());
				if (remoteClient && p->getID())
					tLX->cFont.DrawCentre(bmpDest, x+240, j, tLX->clNormalLabel, itoa(remoteClient->getPing()));
			}
        } else {
            // Ready state
            if(p->getGameReady())
                tLX->cFont.Draw(bmpDest, x+180, j, tLX->clReady, "Ready");
            else
                tLX->cFont.Draw(bmpDest, x+180, j, tLX->clWaiting, "Waiting");

			// Show ping if host
			if(tGameInfo.iGameType == GME_HOST)  {
				CClient *remoteClient = cServer->getClient(p->getID());
				if (remoteClient && p->getID())
					tLX->cFont.DrawCentre(bmpDest, x+260, j, tLX->clNormalLabel, itoa(remoteClient->getPing()));
			}
        }

        j+=20;
    }
}

///////////////////
// Draw the current game settings
void CClient::DrawCurrentSettings(SDL_Surface *bmpDest)
{
    if(Con_IsUsed())
        return;

    // Do checks on whether or not to show
    if(iNetStatus != NET_CONNECTED && !cShowSettings.isDown())
		return;

    int y = tInterfaceSettings.CurrentSettingsY;
    int x = tInterfaceSettings.CurrentSettingsX;
	int w = 240;
	int h = 132;
	if (cViewports[1].getUsed())  {
		x = tInterfaceSettings.CurrentSettingsTwoPlayersX;
		y = tInterfaceSettings.CurrentSettingsTwoPlayersY;
	}

    DrawRectFill(bmpDest, x+1, y, x+w-1, y+h-1, tLX->clCurrentSettingsBg);
    Menu_DrawBox(bmpDest, x, y, x+w, y+h);

    tLX->cFont.Draw(bmpDest, x+60, y+5, tLX->clNormalLabel, "Current settings");
    DrawHLine(bmpDest, x+4, x+w-4, y+22, tLX->clLine);

	/*tLX->cFont.Draw(bmpDest, x+5, y+25, tLX->clNormalLabel,"%s","Level:");
	tLX->cFont.Draw(bmpDest, x+105, y+25, tLX->clNormalLabel,"%s",tGameInfo.sMapname.c_str());*/
	tLX->cFont.Draw(bmpDest, x+5, y+25, tLX->clNormalLabel, "Mod:");
	tLX->cFont.Draw(bmpDest, x+105, y+25, tLX->clNormalLabel, tGameInfo.sModName);
	tLX->cFont.Draw(bmpDest, x+5, y+43, tLX->clNormalLabel,"Game Type:");
	switch (tGameInfo.iGameMode)  {
	case GMT_DEATHMATCH:
	  tLX->cFont.Draw(bmpDest, x+105, y+43, tLX->clNormalLabel,"Deathmatch");
	  break;
	case GMT_TAG:
	  tLX->cFont.Draw(bmpDest, x+105, y+43, tLX->clNormalLabel,"Tag");
	  break;
	case GMT_TEAMDEATH:
	  tLX->cFont.Draw(bmpDest, x+105, y+43, tLX->clNormalLabel,"Team Deathmatch");
	  break;
	case GMT_DEMOLITION:
	  tLX->cFont.Draw(bmpDest, x+105, y+43, tLX->clNormalLabel,"Demolition");
	  break;
	}  // switch
	tLX->cFont.Draw(bmpDest, x+5, y+61, tLX->clNormalLabel,"Loading Time:");
	tLX->cFont.Draw(bmpDest, x+105, y+61, tLX->clNormalLabel,itoa(tGameInfo.iLoadingTimes) + "%");
	tLX->cFont.Draw(bmpDest, x+5, y+79, tLX->clNormalLabel,"Lives:");
	if (tGameInfo.iLives < 0)
		DrawImage(bmpDest,gfxGame.bmpInfinite,x+105,y+88-gfxGame.bmpInfinite->h/2);
	else
		tLX->cFont.Draw(bmpDest, x+105, y+79, tLX->clNormalLabel,itoa(tGameInfo.iLives));
	tLX->cFont.Draw(bmpDest, x+5, y+97, tLX->clNormalLabel,"Max Kills:");
	if (tGameInfo.iKillLimit < 0)
		DrawImage(bmpDest,gfxGame.bmpInfinite,x+105,y+106-gfxGame.bmpInfinite->h/2);
	else
		tLX->cFont.Draw(bmpDest, x+105, y+97, tLX->clNormalLabel,itoa(tGameInfo.iKillLimit));
	tLX->cFont.Draw(bmpDest, x+5, y+115, tLX->clNormalLabel,"Bonuses:");
	if (tGameInfo.iBonusesOn)
		tLX->cFont.Draw(bmpDest, x+105, y+115, tLX->clNormalLabel,"On");
	else
		tLX->cFont.Draw(bmpDest, x+105, y+115, tLX->clNormalLabel,"Off");

}

///////////////////
// Draws the media player
#ifdef WITH_MEDIAPLAYER
void CClient::DrawMediaPlayer(SDL_Surface *bmpDest)
{
	cMediaPlayer.Draw(bmpDest);
}
#endif

