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

#include <iostream>

#include "LieroX.h"
#include "ConfigHandler.h"
#include "CClient.h"
#include "CServer.h"
#include "Graphics.h"
#include "CMediaPlayer.h"
#include "Menu.h"
#include "console.h"
#include "GfxPrimitives.h"
#include "StringUtils.h"
#include "CBar.h"
#include "CWorm.h"
#include "Protocol.h"
#include "Entity.h"
#include "Cursor.h"
#include "CButton.h"
#include "CLabel.h"
#include "CImage.h"
#include "CLine.h"
#include "CCombobox.h"
#include "CCheckbox.h"
#include "InputEvents.h"


using namespace std;

SDL_Surface		*bmpMenuButtons = NULL;
float			fLagFlash;


///////////////////
// Initialize the drawing routines
bool CClient::InitializeDrawing(void)
{
	LOAD_IMAGE_WITHALPHA(bmpMenuButtons,"data/frontend/buttons.png");

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

	// Initialize the scoreboard
	if (!bmpIngameScoreBg)  {  // Safety
		bmpIngameScoreBg = gfxCreateSurface(640, getBottomBarTop());
		if (!bmpIngameScoreBg)
			return false;

		FillSurface(bmpIngameScoreBg, tLX->clScoreBackground);
		SetPerSurfaceAlpha(bmpIngameScoreBg, 128);
	}
	InitializeIngameScore(true);

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

		// Options for second viewport in spectator mode - if they won't be initialized the game can crash
		// With these default positions all the texts are hidden behind the minimap
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
// Get the bottom border of the top bar
int CClient::getTopBarBottom()
{
	if (tGameInfo.iGameType == GME_LOCAL)
		return gfxGame.bmpGameLocalTopBar ? gfxGame.bmpGameLocalTopBar->h : tLX->cFont.GetHeight();
	else
		return gfxGame.bmpGameNetTopBar ? gfxGame.bmpGameNetTopBar->h : tLX->cFont.GetHeight();
}

/////////////////
// Get the top border of the bottom bar
int CClient::getBottomBarTop()
{
	if (tGameInfo.iGameType == GME_LOCAL)
		return gfxGame.bmpGameLocalBackground ? 480 - gfxGame.bmpGameLocalBackground->h : 382;
	else
		return gfxGame.bmpGameNetBackground ? 480 - gfxGame.bmpGameNetBackground->h : 382;
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
		DrawRectFill(dst, x + 1, y + 1, x + w - 1, y + tLX->cFont.GetHeight() - 1, tLX->clBoxDark);
		return;
	}

	int middle_w = w - bmpBoxLeft->w - bmpBoxRight->w;
	if (middle_w < 0)  // Too small
		return;

	DrawImage(dst, bmpBoxLeft, x, y);  // Left part
	DrawImageAdv(dst, bmpBoxBuffer, 0, 0, x + bmpBoxLeft->w, y, middle_w, bmpBoxBuffer->h);  // Middle part
	DrawImage(dst, bmpBoxRight, x + bmpBoxLeft->w + middle_w, y); // Right part
}

///////////////////
// Main drawing routines
void CClient::Draw(SDL_Surface *bmpDest)
{
	// TODO: clean this function up
	// currently both control structure and the drawing itself is in here

	ushort i,num;
	float dt = tLX->fDeltaTime;

	// TODO: allow more worms
	num = (ushort)MIN(2,iNumWorms);


	// Check for any communication errors
	if(bServerError) {

		if(bDedicated) {
			cout << "ERROR: servererror: " << strServerErrorMsg << endl;
		} else {
			// Show message box, shutdown and quit back to menu
			DrawImage(tMenu->bmpBuffer, tMenu->bmpMainBack_common, 0, 0);
			Menu_RedrawMouse(true);
			SDL_ShowCursor(SDL_DISABLE);

			Menu_MessageBox("Connection error", strServerErrorMsg, LMB_OK);
		}
		
		GotoNetMenu();
		return;
	}

	bool bScoreboard = true;

	if(!bDedicated) {
		// Local and network use different background images
		SDL_Surface *bgImage = gfxGame.bmpGameNetBackground;
		if (tGameInfo.iGameType == GME_LOCAL)
			bgImage = gfxGame.bmpGameLocalBackground;

		// TODO: allow more viewports
		// Draw the borders
		if (bShouldRepaintInfo || tLX->bVideoModeChanged || bCurrentSettings)  {
			// Fill the viewport area with black
			DrawRectFill(bmpDest, 0, tLXOptions->tGameinfo.bTopBarVisible ? getTopBarBottom() : 0,
				SDL_GetVideoSurface()->w, getBottomBarTop(), tLX->clBlack);

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
		}
	
		// if 2 viewports, draw special
		if(cViewports[1].getUsed())
			DrawRectFill(bmpDest,318,0,322, bgImage ? (480-bgImage->h) : (384), tLX->clViewportSplit);

		// Top bar
		if (tLXOptions->tGameinfo.bTopBarVisible && !bGameMenu && (bShouldRepaintInfo || tLX->bVideoModeChanged))  {
			SDL_Surface *top_bar = tGameInfo.iGameType == GME_LOCAL ? gfxGame.bmpGameLocalTopBar : gfxGame.bmpGameNetTopBar;
			if (top_bar)
				DrawImage( bmpDest, top_bar, 0, 0);
			else
				DrawRectFill( bmpDest, 0, 0, 640, tLX->cFont.GetHeight() + 4, tLX->clGameBackground ); // Backward compatibility
		}

		// Draw the viewports
		// HINT: before we get packet with map info and load the map, cMap is undefined
		// bGameReady says if the game (including cMap) has been initialized
		if((iNetStatus == NET_CONNECTED  && bGameReady ) || (iNetStatus == NET_PLAYING)) {

			// Draw the viewports
			for( i=0; i<NUM_VIEWPORTS; i++ ) {
				if( cViewports[i].getUsed() )  {
					if (cMap != NULL)
						cViewports[i].Process(cRemoteWorms, cViewports, cMap->GetWidth(), cMap->GetHeight(), iGameType);
					DrawViewport(bmpDest, (byte)i);
				}
			}
			bShouldRepaintInfo = false;  // Just repainted it

			// Mini-Map
			if (cMap != NULL)  {
				if (iNetStatus == NET_PLAYING)
					cMap->DrawMiniMap( bmpDest, tInterfaceSettings.MiniMapX, tInterfaceSettings.MiniMapY, dt, cRemoteWorms, iGameType );
				else
					DrawImage( bmpDest, cMap->GetMiniMap(), tInterfaceSettings.MiniMapX, tInterfaceSettings.MiniMapY);
			}

		}
	}

	//
	// check if Players not yet ready
	//
	if (iNetStatus == NET_CONNECTED && bGameReady)  {
		bool ready = true;

		// Go through and draw the first two worms select menus
		for(i=0;i<num;i++) {

			// Select weapons
			if(!cLocalWorms[i]->getWeaponsReady()) {
				cLocalWorms[i]->SelectWeapons(bmpDest, &cViewports[i]);

				// Forced to finish weapon selection
				if (bForceWeaponsReady)  {
					cLocalWorms[i]->setWeaponsReady(true);
				}
				
				if(cLocalWorms[i]->getWeaponsReady()) {
					cout << "Client: worm " << i << " is ready with weapon-selection" << endl;
					if(bDownloadingMap)
						cout << "but we still have to wait for the download process" << endl;
				}
				
				ready = ready && cLocalWorms[i]->getWeaponsReady();
			}
		}

		// If we're ready, let the server know
		if(ready && !bReadySent && !bDownloadingMap) {
			cout << "Client: we are ready, waiting now for start game signal" << endl;
			bReadySent = true;
			
			// TODO: move this out here
			CBytestream bs;
			bs.writeByte(C2S_IMREADY);
			bs.writeByte(iNumWorms);

			// Send my worm's weapon details
			for(i=0;i<iNumWorms;i++)
				cLocalWorms[i]->writeWeapons( &bs );

			cNetChan.AddReliablePacketToSend(bs);
		}
	}

	if (!bDedicated)  {

		// DEBUG
		//DrawRectFill(bmpDest,0,0,100,40,tLX->clBlack);
		//tLX->cFont.Draw(bmpDest,0,0,tLX->clWhite,"iNetStatus = %i",iNetStatus);
		//tLX->cFont.Draw(bmpDest,0,20,tLX->clWhite,"iGameReady = %i",iGameReady);

		// Draw the chatbox for either a local game, or remote game
		if(tGameInfo.iGameType == GME_LOCAL)
			DrawLocalChat(bmpDest);
		else
			DrawRemoteChat(bmpDest);

		// FPS
		if(tLXOptions->bShowFPS && tLXOptions->tGameinfo.bTopBarVisible) {
			DrawBox( bmpDest, tInterfaceSettings.FpsX, tInterfaceSettings.FpsY, tInterfaceSettings.FpsW);  // Draw the box around it
			tLX->cFont.Draw( // Draw the text
						bmpDest,
						tInterfaceSettings.FpsX + 2,
						tInterfaceSettings.FpsY,
						tLX->clFPSLabel,
						"FPS: " + itoa(GetFPS()) // Get the string and its width
					);
		}

		// Ping on the top right
		if(tLXOptions->bShowPing && tGameInfo.iGameType == GME_JOIN && tLXOptions->tGameinfo.bTopBarVisible)  {

			// Draw the box around it
			DrawBox( bmpDest, tInterfaceSettings.PingX, tInterfaceSettings.PingY, tInterfaceSettings.PingW); 

			tLX->cFont.Draw( // Draw the text
						bmpDest,
						tInterfaceSettings.PingX + 2,
						tInterfaceSettings.PingY,
						tLX->clPingLabel,
						"Ping: " + itoa(iMyPing));

			// Send every second
			// TODO: move this somewhere else
			if (tLX->fCurTime - fMyPingRefreshed > 1) {
				CBytestream ping;

				// TODO: move this out here
				ping.Clear();
				ping.writeInt(-1,4);
				ping.writeString("lx::ping");

				ping.Send(cClient->getChannel()->getSocket());

				fMyPingSent = tLX->fCurTime;
				fMyPingRefreshed = tLX->fCurTime;
			}
		}

		if( sSpectatorViewportMsg != "" )
			tLX->cOutlineFont.DrawCentre( bmpDest, 320, 200, tLX->clPingLabel, sSpectatorViewportMsg );
	/*#ifdef DEBUG
		// Upload and download rates
		float up = 0;
		float down = 0;

		// Get the rates
		switch (tGameInfo.iGameType)  {
		case GME_JOIN:
			down = cClient->getChannel()->getIncomingRate() / 1024.0f;
			up = cClient->getChannel()->getOutgoingRate() / 1024.0f;
			break;
		case GME_HOST:
			down = cServer->GetDownload() / 1024.0f;
			up = cServer->GetUpload() / 1024.0f;
			break;
		}

		tLX->cOutlineFont.Draw(bmpDest, 550, 20, tLX->clWhite, "Down: " + ftoa(down, 3) + " kB/s");
		tLX->cOutlineFont.Draw(bmpDest, 550, 20 + tLX->cOutlineFont.GetHeight(), tLX->clWhite, "Up: " + ftoa(up, 3) + " kB/s");

		// Client and server velocity
		if (tGameInfo.iGameType != GME_JOIN)  {
			if (cClient->getWorm(0) && cServer->getClient(0)->getWorm(0))  {
				static std::string cl = "0.000";
				static std::string sv = "0.000";
				static float last_update = -9999;
				if (tLX->fCurTime - last_update >= 0.5f)  {
					cl = ftoa(cClient->getWorm(0)->getVelocity()->GetLength(), 3);
					sv = ftoa(cServer->getClient(0)->getWorm(0)->getVelocity()->GetLength(), 3);
					last_update = tLX->fCurTime;
				}

				tLX->cOutlineFont.Draw(bmpDest, 550, 20 + tLX->cOutlineFont.GetHeight() * 2, tLX->clWhite, cl);
				tLX->cOutlineFont.Draw(bmpDest, 550, 20 + tLX->cOutlineFont.GetHeight() * 3, tLX->clWhite, sv);
			}
		}
	#endif*/

		// Game over
		static bool was_gameovermenu = false;
		if(bGameOver) {
			if(tLX->fCurTime - fGameOverTime > GAMEOVER_WAIT && !was_gameovermenu)  {
				InitializeGameMenu();

				// If this is a tournament, take screenshot of the final screen
				if (tLXOptions->tGameinfo.bMatchLogging && tGameInfo.iGameType != GME_LOCAL)  {
					screenshot_t scrn;
					scrn.sDir = "game_results";
					GetLogData(scrn.sData);
					tLX->tScreenshotQueue.push_back(scrn);
				}

				was_gameovermenu = true;
			} else
				tLX->cOutlineFont.DrawCentre(bmpDest, 320, 200, tLX->clNormalText, "Game Over");
		} else
			was_gameovermenu = false;

		// Viewport manager
		if(bViewportMgr)  {
			bScoreboard = false;
			DrawViewportManager(bmpDest);
		}

		if(iNetStatus == NET_CONNECTED && !bReadySent)  {
			bScoreboard = false;
			DrawPlayerWaiting(bmpDest);
		}

		// Scoreboard
		if(bScoreboard && !bGameMenu)
			DrawScoreboard(bmpDest);

		// Current Settings
		DrawCurrentSettings(bmpDest);

		// Game menu
		if(bGameMenu)
			DrawGameMenu(bmpDest);

		// Chatter
		if(bChat_Typing)  {
			DrawChatter(bmpDest);
		}

#ifdef WITH_MEDIAPLAYER
		DrawMediaPlayer(bmpDest);
#endif

		// Console
		Con_Draw(bmpDest);


		//tLX->cOutlineFont.Draw(bmpDest, 4,20, tLX->clNormalText, "%s",tLX->debug_string);
		//tLX->cOutlineFont.Draw(bmpDest, 4,40, tLX->clNormalText, "%f",tLX->debug_float);
	}
}

///////////////////
// Draw the chatter
void CClient::DrawChatter(SDL_Surface *bmpDest)
{
	int x = tInterfaceSettings.ChatterX;
	int y = tInterfaceSettings.ChatterY;
	std::string text = "Talk: " + sChat_Text;
	if( bTeamChat )
		text = "Team: " + sChat_Text;
	const std::vector<std::string>& lines = splitstring(text, (size_t)-1, 640 - x, tLX->cOutlineFont);

	y -= MAX(0, (int)lines.size() - 1) * tLX->cOutlineFont.GetHeight();

	// Draw the lines of text
	int drawn_size = -6;  // 6 = length of "Talk: "
	int i = 0;
	for (std::vector<std::string>::const_iterator it = lines.begin(); it != lines.end(); it++, i++)  {
		tLX->cOutlineFont.Draw(bmpDest, x, y, tLX->clGameChatter, *it);
		drawn_size += (int)Utf8StringSize((*it));
		if (drawn_size >= (int)iChat_Pos - i && bChat_CursorVisible)  {  // Draw the cursor
			int cursor_x = tLX->cOutlineFont.GetWidth(Utf8SubStr((*it), 0, iChat_Pos - (drawn_size - (int)Utf8StringSize((*it)))));
			DrawVLine(bmpDest, y, MIN(381, y + tLX->cOutlineFont.GetHeight()), cursor_x, tLX->clGameChatCursor);
		}
		y += tLX->cOutlineFont.GetHeight();
	}
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

	// When game menu is visible, it covers all this anyway, so we won't bother to draw it)
	if (!bGameMenu)  {
		if (cMap)
			cMap->Draw(bmpDest, v);

		// The following will be drawn only when playing
		if (iNetStatus == NET_PLAYING)  {
			// update the drawing position
			CWorm *w = cRemoteWorms;
			for(short i=0;i<MAX_WORMS;i++,w++) {
				if(w->isUsed() && w->getAlive())
					w->UpdateDrawPos();
			}

			if( tLXOptions->bShadows ) {
				// Draw the projectile shadows
				DrawProjectileShadows(bmpDest, v);

				// Draw the worm shadows
				w = cRemoteWorms;
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
			w = cRemoteWorms;
			for(i=0;i<MAX_WORMS;i++,w++) {
				if(w->isUsed() && w->getAlive())
					w->Draw(bmpDest, v);
			}
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

	// Do we need to draw this?
	if (!bShouldRepaintInfo && !tLX->bVideoModeChanged)
		return;

	// TODO: allow more viewports
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

	// Draw the details only when current settings is not displayed
	if (!bCurrentSettings)  {
		// Health
		tLX->cFont.Draw(bmpDest, *HealthLabelX, *HealthLabelY, tLX->clHealthLabel, "Health:");
		if (HealthBar)  {
			HealthBar->SetPosition(worm->getHealth());
			HealthBar->Draw(bmpDest);
		}

		// Weapon
		wpnslot_t *Slot = worm->getCurWeapon();
		if(Slot->Weapon) {
			std::string weapon_name = Slot->Weapon->Name;
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
		}

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
    if(bDedicated || !bGameReady)
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
    if(!bChat_Typing && !bGameMenu && !bViewportMgr)
        Con_Process(tLX->fDeltaTime);


	// Game Menu
	// TODO: make this event-based (don't check GetKeyboard() directly)
    if(GetKeyboard()->KeyUp[SDLK_ESCAPE] && !bChat_Typing && !con) {
        if( !bViewportMgr )
			if (!bGameMenu)
				InitializeGameMenu();
        else
            bViewportMgr = false;
    }

    // Viewport manager
    if(cViewportMgr.isDownOnce() && !bChat_Typing && !bGameMenu && !con)
        InitializeViewportManager();

    ProcessSpectatorViewportKeys(); // If local worm is dead move viewport instead of worm

    // Process Chatter
    if(!con)
	    processChatter();
}


/////////////
// Adds default columns to a scoreboard listview, used internally by InitializeGameMenu
inline void AddColumns(CListview *lv)
{
	lv->AddColumn("", 25); // Skin
	lv->AddColumn("", 180); // Player
	lv->AddColumn("", 30); // Lives
	if (tGameInfo.iGameMode == GMT_DEMOLITION)
		lv->AddColumn("", 40); // Dirt count
	else  
		lv->AddColumn("", 30);  // Kills
	if (tGameInfo.iGameType == GME_HOST) 
		lv->AddColumn("", 35);  // Ping
}

///////////////////
// Initialize the game menu/game over screen
enum {
	gm_Ok,
	gm_Leave,
	gm_QuitGame,
	gm_Resume,
	gm_Coutdown,
	gm_TopMessage,
	gm_TopSkin,
	gm_Winner,
	gm_LeftList,
	gm_RightList
};

void CClient::InitializeGameMenu()
{
	bGameMenu = true;
	GetKeyboard()->KeyUp[SDLK_ESCAPE] = false;  // Prevents immediate closing of the scoreboard
	SetGameCursor(CURSOR_HAND);

	// Shutdown any previous instances
	cGameMenuLayout.Shutdown();
	bUpdateScore = true;

	if (tGameInfo.iGameType == GME_LOCAL)  {
		if (bGameOver)
			cGameMenuLayout.Add(new CButton(BUT_OK, bmpMenuButtons), gm_Ok, 310, 360, 30, 20);
		else  {
			cGameMenuLayout.Add(new CButton(BUT_QUITGAME, bmpMenuButtons), gm_QuitGame, 25, 360, 100, 20);
			cGameMenuLayout.Add(new CButton(BUT_RESUME, bmpMenuButtons), gm_Resume, 540, 360, 80, 20);
		}
	} else {  // Remote playing
		if (bGameOver)  {
			std::string ReturningTo = "Returning to Lobby in ";
			int ReturningToWidth = tLX->cFont.GetWidth(ReturningTo);
			cGameMenuLayout.Add(new CButton(BUT_LEAVE, bmpMenuButtons), gm_Leave, 25, 360, 80, 20);
			cGameMenuLayout.Add(new CLabel(ReturningTo, tLX->clReturningToLobby), -1, 600 - ReturningToWidth, 360, ReturningToWidth, tLX->cFont.GetHeight());
			cGameMenuLayout.Add(new CLabel("5", tLX->clReturningToLobby), gm_Coutdown, 600, 360, 0, 0);
		} else {
			cGameMenuLayout.Add(new CButton(BUT_LEAVE, bmpMenuButtons), gm_Leave, 25, 360, 80, 20);
			cGameMenuLayout.Add(new CButton(BUT_RESUME, bmpMenuButtons), gm_Resume, 540, 360, 80, 20);
		}
	}

	cGameMenuLayout.Add(new CLabel("", tLX->clNormalLabel), gm_TopMessage, 440, 5, 0, 0);
	if (bGameOver)  {
		if (tGameInfo.iGameMode == GMT_TEAMDEATH)  {
			static const std::string teamnames[] = {"Blue team", "Red team", "Green team", "Yellow team"};
			iMatchWinner = CLAMP(iMatchWinner, 0, 4); // Safety
			cGameMenuLayout.Add(new CLabel(teamnames[iMatchWinner], tLX->clNormalLabel), gm_Winner, 515, 5, 0, 0);
			cGameMenuLayout.Add(new CImage(gfxGame.bmpTeamColours[iMatchWinner]), gm_TopSkin, 490, 5, 0, 0);
		} else {
			cGameMenuLayout.Add(new CLabel(cRemoteWorms[iMatchWinner].getName(), tLX->clNormalLabel), gm_Winner, 515, 5, 0, 0);
			cGameMenuLayout.Add(new CImage(cRemoteWorms[iMatchWinner].getPicimg()), gm_TopSkin, 490, 5, 0, 0);
		}
	}

	cGameMenuLayout.SetGlobalProperty(PRP_REDRAWMENU, false);

	// Player lists
	CListview *Left = new CListview();
	CListview *Right = new CListview();
	cGameMenuLayout.Add(Left, gm_LeftList, 17, 36 - tLX->cFont.GetHeight(), 305, gfxGame.bmpScoreboard->h - 43);
	cGameMenuLayout.Add(Right, gm_RightList, 318, 36 - tLX->cFont.GetHeight(), 305, gfxGame.bmpScoreboard->h - 43);

	Left->setDrawBorder(false);
	Right->setDrawBorder(false);
	Left->setShowSelect(false);
	Right->setShowSelect(false);
	Left->setRedrawMenu(false);
	Right->setRedrawMenu(false);
	Left->setOldStyle(true);
	Right->setOldStyle(true);

	AddColumns(Left);
	AddColumns(Right);
}


///////////////////
// Draw the game menu/game over screen (the scoreboard)
void CClient::DrawGameMenu(SDL_Surface *bmpDest)
{
	// Background
	if (bGameOver)  {
		DrawImage(bmpDest, gfxGame.bmpGameover, 0, 0);
	} else {
		DrawImage(bmpDest, gfxGame.bmpScoreboard, 0, 0);
	}

	if (GetMouse()->Y >= gfxGame.bmpScoreboard->h - GetMaxCursorHeight())
		bShouldRepaintInfo = true;

	// Update the coutdown
	if (bGameOver)  {
		int sec = 5 + GAMEOVER_WAIT - (int)(tLX->fCurTime - fGameOverTime);
		sec = MAX(0, sec); // Safety
		cGameMenuLayout.SendMessage(gm_Coutdown, LBS_SETTEXT, itoa(sec), 0);
	}

	// Update the top message (winner/dirt count)
	if (tGameInfo.iGameType == GMT_DEMOLITION)  {
		// Get the dirt count
		int dirtcount, i;
		dirtcount = i = 0;
		for (CWorm *w = cRemoteWorms; i < MAX_WORMS; i++, w++)  { if (w->isUsed()) dirtcount += w->getDirtCount(); }

		cGameMenuLayout.SendMessage(gm_TopMessage, LBS_SETTEXT, "Total: " + itoa(dirtcount), 0);
	}

	if (bGameOver)  {
		cGameMenuLayout.SendMessage(gm_TopMessage, LBS_SETTEXT, "Winner:", 0);
	}


	// Update the scoreboard if needed
	UpdateScore((CListview *) cGameMenuLayout.getWidget(gm_LeftList), (CListview *) cGameMenuLayout.getWidget(gm_RightList));

	// Draw the gui
	gui_event_t *ev = cGameMenuLayout.Process();
	cGameMenuLayout.Draw(bmpDest);

	// Draw the mouse
	DrawCursor(bmpDest);

	// Process the gui
	if (ev)  {
		switch (ev->iControlID)  {

		// Ok
		case gm_Ok:
			if (ev->iEventMsg == BTN_MOUSEUP)  {
				GotoLocalMenu();
			}
			break;

		// Leave
		case gm_Leave:
		case gm_QuitGame:
			if (ev->iEventMsg == BTN_MOUSEUP)  {

				// If this is a host, we go back to the lobby
				// The host can only quit the game via the lobby
				switch (tGameInfo.iGameType)  {
				case GME_HOST:
					cServer->gotoLobby();
					break;

				case GME_JOIN:
					GotoNetMenu();
					break;

				case GME_LOCAL:
					GotoLocalMenu();
					break;

				// Should not happen
				default:
					QuittoMenu();
				}
					
			}
			break;

		// Resume
		case gm_Resume:
			if (ev->iEventMsg == BTN_MOUSEUP)  {
				bGameMenu = false;
				bRepaintChatbox = true;
				SetGameCursor(CURSOR_NONE);
			}
			break;
		}
	}

	// TODO: why is processing events in a draw-function? move it out here
	// Process the keyboard
	if (!bChat_Typing)  {
		keyboard_t *Keyboard = GetKeyboard();

		if (Keyboard->KeyUp[SDLK_RETURN] || Keyboard->KeyUp[SDLK_KP_ENTER] || Keyboard->KeyUp[SDLK_ESCAPE])  {
			if (tGameInfo.iGameType == GME_LOCAL && bGameOver)  {
				GotoLocalMenu();
			} else if (!bGameOver)  {
				bGameMenu = false;
				bRepaintChatbox = true;
				SetGameCursor(CURSOR_NONE);
			}
		}
	}
}

///////////////////
// Update the player list in game menu
void CClient::UpdateScore(CListview *Left, CListview *Right)	
{
	// No need to update
	if (!bUpdateScore)
		return;

	short i, n;

	bUpdateScore = false;

	// Clear any previous info
	Left->Clear();
	Right->Clear();

	// Teams
	static const std::string teamnames[] = {"Blue", "Red", "Green", "Yellow"};
	static const std::string VIPteamnames[] = {"VIP Defenders", "VIP Attackers", "VIPs"};

	// Deathmatch scoreboard
	switch(iGameType) {
	case GMT_DEATHMATCH:  {

		// Fill the left listview
		CListview *lv = Left;
		for(i=0; i < iScorePlayers; i++) {
			// Left listview overflowed, fill the right one
			if (i >= 16)
				lv = Right;

			CWorm *p = &cRemoteWorms[iScoreboard[i]];

			// Add the player
			lv->AddItem(p->getName(), i, tLX->clNormalLabel);

			// Skin
			lv->AddSubitem(LVS_IMAGE, "", p->getPicimg(), NULL);

			// Name
			lv->AddSubitem(LVS_TEXT, p->getName(), NULL, NULL);

			// Lives
			switch (p->getLives())  {
			case WRM_UNLIM:
				lv->AddSubitem(LVS_IMAGE, "", gfxGame.bmpInfinite, NULL);
				break;
			case WRM_OUT:
				lv->AddSubitem(LVS_TEXT, "out", NULL, NULL);
				break;
			default:
				lv->AddSubitem(LVS_TEXT, itoa(p->getLives()), NULL, NULL);
				break;
			}

			// Kills 
			lv->AddSubitem(LVS_TEXT, itoa(p->getKills()), NULL, NULL);

			// Ping
			if (tGameInfo.iGameType == GME_HOST)  {
				CClient *remoteClient = cServer->getClient(p->getID());
				if (remoteClient && p->getID())
					lv->AddSubitem(LVS_TEXT, itoa(remoteClient->getPing()), NULL, NULL);
			}
		}
	}
	break; // DEATHMATCH


    // Demolitions scoreboard
	case GMT_DEMOLITION: {

		// Draw the players
		CListview *lv = Left;
		for(i = 0; i < iScorePlayers; i++) {
			// If the left listview overflowed, use the right one
			if (i >= 16)
				lv = Right;

			CWorm *p = &cRemoteWorms[iScoreboard[i]];

			// Add the player
			lv->AddItem(p->getName(), i, tLX->clNormalLabel);

			// Skin
			lv->AddSubitem(LVS_IMAGE, "", p->getPicimg(), NULL);

			// Name
			lv->AddSubitem(LVS_TEXT, p->getName(), NULL, NULL);

			// Lives
			switch (p->getLives())  {
			case WRM_UNLIM:
				lv->AddSubitem(LVS_IMAGE, "", gfxGame.bmpInfinite, NULL);
				break;
			case WRM_OUT:
				lv->AddSubitem(LVS_TEXT, "out", NULL, NULL);
				break;
			default:
				lv->AddSubitem(LVS_TEXT, itoa(p->getLives()), NULL, NULL);
				break;
			}

			// Dirt count
			lv->AddSubitem(LVS_TEXT, itoa(p->getDirtCount()), NULL, NULL);
		}
	}
	break;  // DEMOLITIONS


	// Tag scoreboard
	case GMT_TAG: {

		// Draw the players
		CListview *lv = Left;
		for(i = 0; i < iScorePlayers; i++) {
			// If the left listview overflowed, use the right one
			if (i >= 16)
				lv = Right;

			CWorm *p = &cRemoteWorms[iScoreboard[i]];

			if(p->getTagIT())
				lv->AddItem(p->getName(), i, tLX->clTagHighlight);
			else
				lv->AddItem(p->getName(), i, tLX->clNormalLabel);

			// Skin
			lv->AddSubitem(LVS_IMAGE, "", p->getPicimg(), NULL);

			// Name
			lv->AddSubitem(LVS_TEXT, p->getName(), NULL, NULL);

			// Lives
			switch (p->getLives())  {
			case WRM_UNLIM:
				lv->AddSubitem(LVS_IMAGE, "", gfxGame.bmpInfinite, NULL);
				break;
			case WRM_OUT:
				lv->AddSubitem(LVS_TEXT, "out", NULL, NULL);
				break;
			default:
				lv->AddSubitem(LVS_TEXT, itoa(p->getLives()), NULL, NULL);
				break;
			}

			// Kills
			lv->AddSubitem(LVS_TEXT, itoa(p->getKills()), NULL, NULL);

			// Ping
			if (tGameInfo.iGameType == GME_HOST)  {
				CClient *remoteClient = cServer->getClient(p->getID());
				if (remoteClient && p->getID())
					lv->AddSubitem(LVS_TEXT, itoa(remoteClient->getPing()), NULL, NULL);
			}

			// Total time of being IT
			int h,m,s;
			ConvertTime(p->getTagTime(),  &h, &m, &s);
			lv->AddSubitem(LVS_TEXT, itoa(m) + ":" + (s < 10 ? "0" : "") + itoa(s), NULL, NULL);
		}
	}
	break;  // TAG


	// Team deathmatch scoreboard
	case GMT_TEAMDEATH: {


		// Go through each team
		CListview *lv = Left;
		int team, score;
		Uint32 colour;

		for(n = 0; n < 4; n++) {
			team = iTeamList[n];
			score = iTeamScores[team];

			// Check if the team has any players
			if(score == -1)
				continue;

			// If left would overflow after adding team header, switch to right
			if (lv->getNumItems() + 1 >= 16)
				lv = Right;

			// Header
			colour = tLX->clTeamColors[team];

			lv->AddItem(teamnames[team], n + 1024, colour);

			lv->AddSubitem(LVS_WIDGET, "", NULL, new CLine(0, 0, lv->getWidth() - 30, 0, colour), VALIGN_BOTTOM);  // Separating line
			lv->AddSubitem(LVS_TEXT, teamnames[team] + " (" + itoa(score) + ")", NULL, NULL);  // Name and score
			lv->AddSubitem(LVS_TEXT, "L", NULL, NULL);  // Lives label
			lv->AddSubitem(LVS_TEXT, "K", NULL, NULL);  // Kills label
			if (tGameInfo.iGameType == GME_HOST)  // Ping label
				lv->AddSubitem(LVS_TEXT, "P", NULL, NULL);

			// Draw the players
			CWorm *p;
			for(i = 0; i < iScorePlayers; i++) {
				// If the left listview overflowed, fill the right one
				if (lv->getItemCount() >= 16)
					lv = Right;

				p = &cRemoteWorms[iScoreboard[i]];

				if(p->getTeam() != team)
					continue;

				lv->AddItem(p->getName(), lv->getItemCount(), tLX->clNormalLabel);

				// Skin
				lv->AddSubitem(LVS_IMAGE, "", p->getPicimg(), NULL);

				// Name
				lv->AddSubitem(LVS_TEXT, p->getName(), NULL, NULL);

				// Lives
				switch (p->getLives())  {
				case WRM_UNLIM:
					lv->AddSubitem(LVS_IMAGE, "", gfxGame.bmpInfinite, NULL);
					break;
				case WRM_OUT:
					lv->AddSubitem(LVS_TEXT, "out", NULL, NULL);
					break;
				default:
					lv->AddSubitem(LVS_TEXT, itoa(p->getLives()), NULL, NULL);
					break;
				}

				// Kills
				lv->AddSubitem(LVS_TEXT, itoa(p->getKills()), NULL, NULL);

				// Ping
				if (tGameInfo.iGameType == GME_HOST)  {
					CClient *remoteClient = cServer->getClient(p->getID());
					if (remoteClient && p->getID())
						lv->AddSubitem(LVS_TEXT, itoa(remoteClient->getPing()), NULL, NULL);
				}
			}
		}
	}
	break; // TEAM DEATHMATCH
	case GMT_CTF:  {

		// Fill the left listview
		CListview *lv = Left;
		for(i=0; i < iScorePlayers; i++) {
			// Left listview overflowed, fill the right one
			if (i >= 16)
				lv = Right;

			CWorm *p = &cRemoteWorms[iScoreboard[i]];

			lv->AddItem(p->getName(), i, tLX->clNormalLabel);

			// Skin
			lv->AddSubitem(LVS_IMAGE, "", p->getPicimg(), NULL);

			// Name
			lv->AddSubitem(LVS_TEXT, p->getName(), NULL, NULL);

			// Lives
			switch (p->getLives())  {
			case WRM_UNLIM:
				lv->AddSubitem(LVS_IMAGE, "", gfxGame.bmpInfinite, NULL);
				break;
			case WRM_OUT:
				lv->AddSubitem(LVS_TEXT, "out", NULL, NULL);
				break;
			default:
				lv->AddSubitem(LVS_TEXT, itoa(p->getLives()), NULL, NULL);
				break;
			}

			// Kills 
			lv->AddSubitem(LVS_TEXT, itoa(p->getKills()), NULL, NULL);

			// Ping
			if (tGameInfo.iGameType == GME_HOST)  {
				CClient *remoteClient = cServer->getClient(p->getID());
				if (remoteClient && p->getID())
					lv->AddSubitem(LVS_TEXT, itoa(remoteClient->getPing()), NULL, NULL);
			}
		}
	}
	break; // CTF
		// Team CTF scoreboard
	case GMT_TEAMCTF: {


		// Go through each team
		CListview *lv = Left;
		int team, score;
		Uint32 colour;

		for(n = 0; n < 4; n++) {
			team = iTeamList[n];
			score = iTeamScores[team];

			// Check if the team has any players
			if(score == -1)
				continue;

			// If left would overflow after adding team header, switch to right
			if (lv->getNumItems() + 1 >= 16)
				lv = Right;

			// Header
			colour = tLX->clTeamColors[team];

			lv->AddItem(teamnames[team], n + 1024, colour);

			lv->AddSubitem(LVS_WIDGET, "", NULL, new CLine(0, 0, lv->getWidth() - 30, 0, colour), VALIGN_BOTTOM);  // Separating line
			lv->AddSubitem(LVS_TEXT, teamnames[team] + " (" + itoa(score) + ")", NULL, NULL);  // Name and score
			lv->AddSubitem(LVS_TEXT, "L", NULL, NULL);  // Lives label
			lv->AddSubitem(LVS_TEXT, "K", NULL, NULL);  // Kills label
			if (tGameInfo.iGameType == GME_HOST)  // Ping label
				lv->AddSubitem(LVS_TEXT, "P", NULL, NULL);

			// Draw the players
			CWorm *p;
			for(i = 0; i < iScorePlayers; i++) {
				// If the left listview overflowed, fill the right one
				if (lv->getItemCount() >= 16)
					lv = Right;

				p = &cRemoteWorms[iScoreboard[i]];

				if(p->getTeam() != team)
					continue;

				lv->AddItem(p->getName(), lv->getItemCount(), tLX->clNormalLabel);

				// Skin
				lv->AddSubitem(LVS_IMAGE, "", p->getPicimg(), NULL);

				// Name
				lv->AddSubitem(LVS_TEXT, p->getName(), NULL, NULL);

				// Lives
				switch (p->getLives())  {
				case WRM_UNLIM:
					lv->AddSubitem(LVS_IMAGE, "", gfxGame.bmpInfinite, NULL);
					break;
				case WRM_OUT:
					lv->AddSubitem(LVS_TEXT, "out", NULL, NULL);
					break;
				default:
					lv->AddSubitem(LVS_TEXT, itoa(p->getLives()), NULL, NULL);
					break;
				}
				
				// Kills
				lv->AddSubitem(LVS_TEXT, itoa(p->getKills()), NULL, NULL);

				// Ping
				if (tGameInfo.iGameType == GME_HOST)  {
					CClient *remoteClient = cServer->getClient(p->getID());
					if (remoteClient && p->getID())
						lv->AddSubitem(LVS_TEXT, itoa(remoteClient->getPing()), NULL, NULL);
				}
			}
		}
	}
	break; // TEAM CTF
	// VIP scoreboard
	case GMT_VIP: {


		// Go through each team
		CListview *lv = Left;
		int team, score;
		Uint32 colour;

		for(n = 0; n < 4; n++) {
			team = iTeamList[n];
			score = iTeamScores[team];

			// Check if the team has any players
			if(score == -1)
				continue;

			// If left would overflow after adding team header, switch to right
			if (lv->getNumItems() + 1 >= 16)
				lv = Right;

			// Header
			colour = tLX->clTeamColors[team];

			lv->AddItem(VIPteamnames[team], n + 1024, colour);

			lv->AddSubitem(LVS_WIDGET, "", NULL, new CLine(0, 0, lv->getWidth() - 30, 0, colour), VALIGN_BOTTOM);  // Separating line
			lv->AddSubitem(LVS_TEXT, VIPteamnames[team] + " (" + itoa(score) + ")", NULL, NULL);  // Name and score
			lv->AddSubitem(LVS_TEXT, "L", NULL, NULL);  // Lives label
			lv->AddSubitem(LVS_TEXT, "K", NULL, NULL);  // Kills label
			if (tGameInfo.iGameType == GME_HOST)  // Ping label
				lv->AddSubitem(LVS_TEXT, "P", NULL, NULL);

			// Draw the players
			CWorm *p;
			for(i = 0; i < iScorePlayers; i++) {
				// If the left listview overflowed, fill the right one
				if (lv->getItemCount() >= 16)
					lv = Right;

				p = &cRemoteWorms[iScoreboard[i]];

				if(p->getTeam() != team && (p->getTeam()!=2 || team !=0))
					continue;

				lv->AddItem(p->getName(), lv->getItemCount(), tLX->clNormalLabel);

				// Skin
				lv->AddSubitem(LVS_IMAGE, "", p->getPicimg(), NULL);

				// Name
				lv->AddSubitem(LVS_TEXT, p->getName(), NULL, NULL);

				// Lives
				switch (p->getLives())  {
				case WRM_UNLIM:
					lv->AddSubitem(LVS_IMAGE, "", gfxGame.bmpInfinite, NULL);
					break;
				case WRM_OUT:
					lv->AddSubitem(LVS_TEXT, "out", NULL, NULL);
					break;
				default:
					lv->AddSubitem(LVS_TEXT, itoa(p->getLives()), NULL, NULL);
					break;
				}

				// Kills
				lv->AddSubitem(LVS_TEXT, itoa(p->getKills()), NULL, NULL);

				// Ping
				if (tGameInfo.iGameType == GME_HOST)  {
					CClient *remoteClient = cServer->getClient(p->getID());
					if (remoteClient && p->getID())
						lv->AddSubitem(LVS_TEXT, itoa(remoteClient->getPing()), NULL, NULL);
				}
			}
		}
	}
	break; // VIP
	} // switch	
}

///////////////////
// Draw the bonuses
void CClient::DrawBonuses(SDL_Surface *bmpDest, CViewport *v)
{
	if(!tGameInfo.bBonusesOn)
		return;

	CBonus *b = cBonuses;

	for(short i=0;i<MAX_BONUSES;i++,b++) {
		if(!b->getUsed())
			continue;

		b->Draw(bmpDest, v, bShowBonusName);
	}
}


///////////////////
// Draw text that is shadowed
void CClient::DrawText(SDL_Surface *bmpDest, bool centre, int x, int y, Uint32 fgcol, const std::string& buf)
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
	if (cChatbox.getNumLines() == 0)
		return;

	int y = tInterfaceSettings.LocalChatY;
	lines_riterator it = cChatbox.RBegin();
	if(it != cChatbox.REnd()) it++;

	for(byte i = 0; i < 6 && it != cChatbox.REnd(); i++, it++) { } // Last 6 messages

	// Draw the lines of text
	while(it != cChatbox.RBegin()) {
		it--;
		
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
	int inbox = MouseInRect(lv->getX(),lv->getY(), lv->getWidth() + GetCursorWidth(CURSOR_ARROW), lv->getHeight()+GetCursorHeight(CURSOR_ARROW)) ||
				MouseInRect(tInterfaceSettings.ChatboxScrollbarX, tInterfaceSettings.ChatboxScrollbarY, 14 + GetCursorWidth(CURSOR_ARROW), tInterfaceSettings.ChatboxScrollbarH);

	if (lv->NeedsRepaint() || (inbox && (Mouse->deltaX || Mouse->deltaY)) || bRepaintChatbox || tLX->bVideoModeChanged)  {	// Repainting when new messages/scrolling, 
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
						 MIN(tInterfaceSettings.ChatBoxW + tInterfaceSettings.ChatBoxX + GetCursorWidth(CURSOR_ARROW),
							 tInterfaceSettings.MiniMapX) - tInterfaceSettings.ChatBoxX,
						 bgImage->h);
		else
			DrawRectFill(bmpDest,165,382,541,480,tLX->clGameBackground);
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
                if( !b_on) {
                    SetupViewports(&cRemoteWorms[v1_target], NULL, a_type, VW_FOLLOW);
                } else {
                    SetupViewports(&cRemoteWorms[v1_target], &cRemoteWorms[v2_target], a_type, b_type);
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

void CClient::InitializeSpectatorViewportKeys()
{
	cSpectatorViewportKeys.Up.Setup( tLXOptions->sPlayerControls[0][SIN_UP] );
	cSpectatorViewportKeys.Down.Setup( tLXOptions->sPlayerControls[0][SIN_DOWN] );
	cSpectatorViewportKeys.Left.Setup( tLXOptions->sPlayerControls[0][SIN_LEFT] );
	cSpectatorViewportKeys.Right.Setup( tLXOptions->sPlayerControls[0][SIN_RIGHT] );
	cSpectatorViewportKeys.V1Type.Setup( tLXOptions->sPlayerControls[0][SIN_SHOOT] );
	cSpectatorViewportKeys.V2Type.Setup( tLXOptions->sPlayerControls[0][SIN_JUMP] );
	cSpectatorViewportKeys.V2Toggle.Setup( tLXOptions->sPlayerControls[0][SIN_SELWEAP] );
}

void CClient::ProcessSpectatorViewportKeys()
{
	if( ! bSpectate)
	{
		// Only process keyboard input if all local human worms are dead at least 3 seconds
		if( cLocalWorms[0]->getAlive() || cLocalWorms[0]->getLives() != WRM_OUT ||
			(tLX->fCurTime - cLocalWorms[0]->getTimeofDeath() <= 3.0f))
			return;
		if(cLocalWorms[1])
			if( cLocalWorms[1]->getType() == PRF_HUMAN &&
				( cLocalWorms[1]->getAlive() || cLocalWorms[0]->getLives() != WRM_OUT ||
				(tLX->fCurTime - cLocalWorms[0]->getTimeofDeath() <= 3.0f)) )
				return;
	}

	// Don't process when typing a message
	if (bChat_Typing)
		return;
	
	bool v2_on = cViewports[1].getUsed();
	int v1_type = cViewports[0].getType();
	int v2_type = cViewports[1].getType();
	CWorm * v1_targetPtr = cViewports[0].getTarget();
	CWorm * v2_targetPtr = cViewports[1].getTarget();
	int v1_target = -1, v2_target = -1, v1_prev = -1, v2_prev = -1, v1_next = -1, v2_next = -1;

    for(int i=0; i<MAX_WORMS; i++ ) 
	{
        if( ! cRemoteWorms[i].isUsed() || cRemoteWorms[i].getLives() == WRM_OUT )
			continue;
		if( v1_target != -1 && v1_next == -1 )
				v1_next = i;
		if( v2_target != -1 && v2_next == -1 )
				v2_next = i;
		if( v1_targetPtr == &cRemoteWorms[i] )
			v1_target = i;
		if( v2_targetPtr == &cRemoteWorms[i] )
			v2_target = i;
		if( v1_target == -1 )
			v1_prev = i;
		if( v2_target == -1 )
			v2_prev = i;
    }

	if( v1_target == -1 )
		v1_target = v1_prev;
	if( v2_target == -1 )
		v2_target = v2_prev;
	if( v1_next == -1 )
		v1_next = v1_target;
	if( v2_next == -1 )
		v2_next = v2_target;
	if( v1_prev == -1 )
		v1_prev = v1_target;
	if( v2_prev == -1 )
		v2_prev = v2_target;

	bool Changed = false;
	if( v1_type != VW_FREELOOK )
	{
		if( cSpectatorViewportKeys.Left.isDownOnce() )
		{
			v1_target = v1_prev;
			Changed = true;
		}
		if( cSpectatorViewportKeys.Right.isDownOnce() )
		{
			v1_target = v1_next;
			Changed = true;
		}
		if( cSpectatorViewportKeys.Up.isDownOnce() )
		{
			v2_target = v2_prev;
			Changed = true;
		}
		if( cSpectatorViewportKeys.Down.isDownOnce() )
		{
			v2_target = v2_next;
			Changed = true;
		}
	}
	
	int iMsgType = -1, iViewportNum = -1;
	if( cSpectatorViewportKeys.V2Toggle.isDownOnce() )
	{
		v2_on = ! v2_on;
		iViewportNum = 1;
		iMsgType = v1_type;
		if( v2_on )
		{
			iViewportNum = 2;
			iMsgType = v2_type;
		};
		Changed = true;
	}

	if( cSpectatorViewportKeys.V1Type.isDownOnce() )
	{
		v1_type ++;
		if( v1_type > VW_ACTIONCAM )
			v1_type = VW_FOLLOW;
		iViewportNum = 1;
		iMsgType = v1_type;
		Changed = true;
	}

	if( cSpectatorViewportKeys.V2Type.isDownOnce() )
	{
		v2_type ++;
		if( v2_type > VW_ACTIONCAM )
			v2_type = VW_FOLLOW;
		iViewportNum = 2;
		iMsgType = v2_type;
		Changed = true;
	}

	if( iMsgType != -1 && iViewportNum != -1 )
	{
		sSpectatorViewportMsg = "Viewport " + itoa(iViewportNum) + ": ";
		if( iMsgType == VW_FOLLOW )
			sSpectatorViewportMsg += "Follow";
		if( iMsgType == VW_CYCLE )
			sSpectatorViewportMsg += "Cycle";
		if( iMsgType == VW_FREELOOK )
			sSpectatorViewportMsg += "Free look";
		if( iMsgType == VW_ACTIONCAM )
			sSpectatorViewportMsg += "Action Cam";
		fSpectatorViewportMsgTimeout = tLX->fCurTime;
	}
	
	if( fSpectatorViewportMsgTimeout + 1.0 < tLX->fCurTime )
		sSpectatorViewportMsg = "";

	if( Changed )
	{
		if( !v2_on )
			SetupViewports(&cRemoteWorms[v1_target], NULL, v1_type, v2_type);
		else
			SetupViewports(&cRemoteWorms[v1_target], &cRemoteWorms[v2_target], v1_type, v2_type);
	}
}

/////////////////////
// Initialize the scoreboard
enum  {
	sb_Left,
	sb_Right,
};

void CClient::InitializeIngameScore(bool WaitForPlayers)
{
	// Clear and initialize
	cScoreLayout.Shutdown();
	cScoreLayout.Initialize();

	CListview *Left = new CListview();
	CListview *Right = new CListview();

	cScoreLayout.Add(Left, sb_Left, 17, getTopBarBottom() + 10, 305, getBottomBarTop() - getTopBarBottom() - 10);
	cScoreLayout.Add(Right, sb_Right, 318, getTopBarBottom() + 10, 305, getBottomBarTop() - getTopBarBottom() - 10);

	// Set the styles
	Left->setDrawBorder(false);
	Right->setDrawBorder(false);
	Left->setShowSelect(false);
	Right->setShowSelect(false);
	Left->setRedrawMenu(false);
	Right->setRedrawMenu(false);
	Left->setOldStyle(true);
	Right->setOldStyle(true);

	// Add columns
	Left->AddColumn("ID", 20, tLX->clHeading);  // ID
	Right->AddColumn("ID", 20, tLX->clHeading);
	Left->AddColumn("", 35, tLX->clHeading);  // Skin
	Right->AddColumn("", 35, tLX->clHeading);
	Left->AddColumn("Player", 140, tLX->clHeading);  // Player
	Right->AddColumn("Player", 140, tLX->clHeading);
	if (WaitForPlayers)  {
		Left->AddColumn("", 80, tLX->clHeading);
		Right->AddColumn("", 80, tLX->clHeading);
	} else {
		Left->AddColumn("L", 40, tLX->clHeading);  // Lives
		Right->AddColumn("L", 40, tLX->clHeading);
		Left->AddColumn("K", 30, tLX->clHeading);  // Kills
		Right->AddColumn("K", 30, tLX->clHeading);
	}

	if (tGameInfo.iGameType == GME_HOST)  {
		Left->AddColumn("P", 40, tLX->clHeading);  // Ping
		Right->AddColumn("P", 40, tLX->clHeading);
	}

}

////////////////////
// Update the scoreboard
void CClient::UpdateIngameScore(CListview *Left, CListview *Right, bool WaitForPlayers)
{
	CListview *lv = Left;
	Uint32 iColor;

	// Clear them first
	Left->Clear();
	Right->Clear();

	// Fill the listviews
    for(int i=0; i < iScorePlayers; i++) {
		if (i >= 16)  // Left listview overflowed, fill in the right one
			lv = Right;

        CWorm *p = &cRemoteWorms[iScoreboard[i]];

		// Get colour
		if (tLXOptions->bColorizeNicks && (tGameInfo.iGameMode == GMT_TEAMDEATH || tGameInfo.iGameMode == GMT_VIP || tGameInfo.iGameMode == GMT_TEAMCTF))
			iColor = tLX->clTeamColors[p->getTeam()];
		else
			iColor = tLX->clNormalLabel;

        // Add the player and if this player is local & human, highlight it
		lv->AddItem(p->getName(), i, tLX->clNormalLabel);
		if (p->getLocal() && (p->getType() != PRF_COMPUTER || tGameInfo.iGameType != GME_LOCAL))  {
			lv_item_t *it = lv->getItem(i);
			it->iBgAlpha = 64;
			it->iBgColour = tLX->clScoreHighlight;
		}

		// ID
		lv->AddSubitem(LVS_TEXT, itoa(p->getID()), NULL, NULL);

        // Skin
		lv->AddSubitem(LVS_IMAGE, "", p->getPicimg(), NULL, VALIGN_TOP);

		// Name
		lv->AddSubitem(LVS_TEXT, p->getName(), NULL, NULL, VALIGN_MIDDLE, iColor);

		if (WaitForPlayers)
			lv->AddSubitem(LVS_TEXT, p->getGameReady() ? "Ready" : "Waiting", NULL, NULL, VALIGN_MIDDLE, p->getGameReady() ? tLX->clReady : tLX->clWaiting);
		else  {
			// Lives
			switch (p->getLives())  {
			case WRM_OUT:
				lv->AddSubitem(LVS_TEXT, "out", NULL, NULL);
				break;
			case WRM_UNLIM:
				lv->AddSubitem(LVS_IMAGE, "", gfxGame.bmpInfinite, NULL);
				break;
			default:
				lv->AddSubitem(LVS_TEXT, itoa(p->getLives()), NULL, NULL);
				break;
			}

			// Kills
			lv->AddSubitem(LVS_TEXT, itoa(p->getKills()), NULL, NULL);
		}

		// Ping
		if(tGameInfo.iGameType == GME_HOST)  {
			CClient *remoteClient = cServer->getClient(p->getID());
			if (remoteClient && p->getID())
				lv->AddSubitem(LVS_TEXT, itoa(remoteClient->getPing()), NULL, NULL);
		}
    }

	bUpdateScore = false;
}

#define WAIT_COL_W 180

////////////////////
// Helper function for DrawPlayerWaiting
void CClient::DrawPlayerWaitingColumn(SDL_Surface *bmpDest, int x, int y, std::list<CWorm *>::iterator& it, const std::list<CWorm *>::iterator& last, int num)
{
	const int h = getBottomBarTop() - y;

	SDL_Rect oldclip;
	SDL_GetClipRect(bmpDest, &oldclip);
	SDL_Rect newclip = {x, y, WAIT_COL_W, h};
	SDL_SetClipRect(bmpDest, &newclip);

	DrawRectFillA(bmpDest, x, y, x + WAIT_COL_W, y + h, tLX->clScoreBackground, 128);
	
	int cur_y = y + 5;
	for (int i=0; i < num && (it != last); i++, it++)  {
		CWorm *wrm = *it;

		int cur_x = x + 5;

		// Ready
		if (wrm->getGameReady())
			DrawImage(bmpDest, tMenu->bmpLobbyReady, cur_x, cur_y + (wrm->getPicimg()->h - tMenu->bmpLobbyReady->h) / 2);
		else
			DrawImage(bmpDest, tMenu->bmpLobbyNotReady, cur_x, cur_y + (wrm->getPicimg()->h - tMenu->bmpLobbyNotReady->h) / 2);
		cur_x += tMenu->bmpLobbyReady->w;

		// Skin
		DrawImage(bmpDest, wrm->getPicimg(), cur_x, cur_y);
		cur_x += wrm->getPicimg()->w + 5; // 5 - leave some space

		// Name
		tLX->cFont.Draw(bmpDest, cur_x, cur_y, tLX->clNormalLabel, wrm->getName());

		cur_y += MAX(wrm->getPicimg()->h, tLX->cFont.GetHeight()) + 2;
	}

	SDL_SetClipRect(bmpDest, &oldclip);
}

///////////////////
// Draws a simple scoreboard when waiting for players
void CClient::DrawPlayerWaiting(SDL_Surface *bmpDest)
{
	int x = 0;
	int y = tLXOptions->tGameinfo.bTopBarVisible ? getTopBarBottom() : 0;

	if (iNetStatus == NET_PLAYING || tGameInfo.iGameType == GME_LOCAL || bGameMenu)
		return;

	// Get the number of players
	std::list<CWorm *> worms;
	CWorm *w = cRemoteWorms;
	for (int i=0; i < MAX_WORMS; i++, w++)  {
		if (w->isUsed())
			worms.push_back(w);
	}

	std::list<CWorm *>::iterator it = worms.begin();

	// Two columns
	if (worms.size() > 16)  {
		DrawPlayerWaitingColumn(bmpDest, x, y, it, worms.end(), 16);
		DrawPlayerWaitingColumn(bmpDest, SDL_GetVideoSurface()->w - WAIT_COL_W, y, it, worms.end(), worms.size() - 16);

	// One column
	} else {
		DrawPlayerWaitingColumn(bmpDest, x, y, it, worms.end(), worms.size());
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
    if(cShowScore.isDown() && !bChat_Typing)
        bShowScore = true;
    if(iNetStatus == NET_CONNECTED && bGameReady && tGameInfo.iGameType != GME_LOCAL) {
        bShowScore = true;
        bShowReady = true;
    }
    if(!bShowScore)
        return;

	// Background
	DrawImageAdv(bmpDest, bmpIngameScoreBg, 0, tLXOptions->tGameinfo.bTopBarVisible ? getTopBarBottom() : 0, 0,
				tLXOptions->tGameinfo.bTopBarVisible ? getTopBarBottom() : 0, bmpIngameScoreBg->w, bmpIngameScoreBg->h);
	
	if (bUpdateScore)
		UpdateIngameScore(((CListview *)cScoreLayout.getWidget(sb_Left)), ((CListview *)cScoreLayout.getWidget(sb_Right)), bShowReady);

	// Hide the second list if there are no players
	cScoreLayout.getWidget(sb_Right)->setEnabled(iNumWorms > 16);
	
	// Draw it!
	cScoreLayout.Draw(bmpDest);
}

///////////////////
// Draw the current game settings
void CClient::DrawCurrentSettings(SDL_Surface *bmpDest)
{
	bCurrentSettings = true;

    // Do checks on whether or not to show
    if(iNetStatus != NET_CONNECTED && !cShowSettings.isDown() && !bGameOver)
		bCurrentSettings = false;

	if (bGameOver && bGameMenu)
		bCurrentSettings = true;

	if (!bCurrentSettings)
		return;

    int y = tInterfaceSettings.CurrentSettingsY;
    int x = tInterfaceSettings.CurrentSettingsX;
	if (cViewports[1].getUsed())  {
		x = tInterfaceSettings.CurrentSettingsTwoPlayersX;
		y = tInterfaceSettings.CurrentSettingsTwoPlayersY;
	}

	int cur_y = y;
	/*tLX->cFont.Draw(bmpDest, x+5, y+25, tLX->clNormalLabel,"%s","Level:");
	tLX->cFont.Draw(bmpDest, x+105, y+25, tLX->clNormalLabel,"%s",tGameInfo.sMapname.c_str());*/
	tLX->cFont.Draw(bmpDest, x+5, cur_y, tLX->clNormalLabel, "Mod:");
	std::string mod = tGameInfo.sModName;
	stripdot(mod, tInterfaceSettings.ChatBoxX - 105); // 10 - leave some space
	tLX->cFont.Draw(bmpDest, x+95, cur_y, tLX->clNormalLabel, mod);
	cur_y += tLX->cFont.GetHeight();

	static const std::string gmt_names[] = {"Deathmatch", "Team DM", "Tag", "Demolition", "VIP", "CTF", "Teams CTF"};
	tLX->cFont.Draw(bmpDest, x+5, cur_y, tLX->clNormalLabel,"Game Type:");
	tLX->cFont.Draw(bmpDest, x+95, cur_y, tLX->clNormalLabel, gmt_names[CLAMP(tGameInfo.iGameMode, (int)0, (int)(sizeof(gmt_names)/sizeof(std::string)))]);
	cur_y += tLX->cFont.GetHeight();

	tLX->cFont.Draw(bmpDest, x+5, cur_y, tLX->clNormalLabel,"Loading Time:");
	tLX->cFont.Draw(bmpDest, x+95, cur_y, tLX->clNormalLabel,itoa(tGameInfo.iLoadingTimes) + "%");
	cur_y += tLX->cFont.GetHeight();

	tLX->cFont.Draw(bmpDest, x+5, cur_y, tLX->clNormalLabel,"Lives:");
	if (tGameInfo.iLives < 0)
		DrawImage(bmpDest,gfxGame.bmpInfinite, x+95, cur_y + (tLX->cFont.GetHeight() - gfxGame.bmpInfinite->h)/2);
	else
		tLX->cFont.Draw(bmpDest, x+95, cur_y, tLX->clNormalLabel,itoa(tGameInfo.iLives));
	cur_y += tLX->cFont.GetHeight();

	tLX->cFont.Draw(bmpDest, x+5, cur_y, tLX->clNormalLabel,"Max Kills:");
	if (tGameInfo.iKillLimit < 0)
		DrawImage(bmpDest,gfxGame.bmpInfinite,x+95,cur_y + (tLX->cFont.GetHeight() - gfxGame.bmpInfinite->h)/2);
	else
		tLX->cFont.Draw(bmpDest, x+95, cur_y, tLX->clNormalLabel,itoa(tGameInfo.iKillLimit));
	cur_y += tLX->cFont.GetHeight();

	tLX->cFont.Draw(bmpDest, x+5, cur_y, tLX->clNormalLabel,"Bonuses:");
	if (tGameInfo.bBonusesOn)
		tLX->cFont.Draw(bmpDest, x+95, cur_y, tLX->clNormalLabel,"On");
	else
		tLX->cFont.Draw(bmpDest, x+95, cur_y, tLX->clNormalLabel,"Off");

}

///////////////////
// Draws the media player
#ifdef WITH_MEDIAPLAYER
void CClient::DrawMediaPlayer(SDL_Surface *bmpDest)
{
	cMediaPlayer.Draw(bmpDest);
}
#endif

