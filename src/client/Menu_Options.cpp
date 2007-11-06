/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Options
// Created 30/7/02
// Jason Boettcher


#include "LieroX.h"
#include "AuxLib.h"
#include "Graphics.h"
#include "Menu.h"
#include "GfxPrimitives.h"
#include "FindFile.h"
#include "StringUtils.h"
#include "CButton.h"
#include "CLabel.h"
#include "CCheckbox.h"
#include "CTextbox.h"
#include "CSlider.h"
#include "CMediaPlayer.h"


int OptionsMode = 0;
CGuiLayout	cOptions;
CGuiLayout	cOpt_Controls;
CGuiLayout	cOpt_System;
CGuiLayout	cOpt_Game;
CButton		TopButtons[3];

// Control id's
enum {
	op_Back = -2,
	Static = -1,
	op_Controls=0,
	op_Game,
	op_System
};

enum {
	os_Fullscreen,
	os_ColourDepth,
	os_SoundOn,
	os_SoundVolume,
	os_NetworkPort,
	os_NetworkSpeed,
	os_UseIpToCountry,
	os_LoadDbAtStartup,
	os_STUNServer,
	os_ShowFPS,
	os_OpenGL,
	os_ShowPing,
	os_LogConvos,
	os_ScreenshotFormat,
	os_MaxFPS,
	os_Apply
};

enum {
	og_BloodAmount,
	og_Shadows,
	og_Particles,
	og_OldSkoolRope,
	og_AIDifficulty,
	og_ShowWormHealth,
	og_ColorizeNicks,
	og_AutoTyping,
	og_Antialiasing,
	og_MouseAiming,
	og_AllowMouseAiming,
	og_MatchLogging,
	og_UseNumericKeysToSwitchWeapons
};

enum {
	oc_Ply1_Up,
	oc_Ply1_Down,
	oc_Ply1_Left,
	oc_Ply1_Right,
	oc_Ply1_Shoot,
	oc_Ply1_Jump,
	oc_Ply1_Selweapon,
	oc_Ply1_Rope,
	oc_Ply1_Strafe,

	oc_Ply2_Up,
	oc_Ply2_Down,
	oc_Ply2_Left,
	oc_Ply2_Right,
	oc_Ply2_Shoot,
	oc_Ply2_Jump,
	oc_Ply2_Selweapon,
	oc_Ply2_Rope,
	oc_Ply2_Strafe,

	oc_Gen_Chat,
    oc_Gen_Score,
	oc_Gen_Health,
	oc_Gen_CurSettings,
	oc_Gen_TakeScreenshot,
	oc_Gen_ViewportManager,
	oc_Gen_SwitchMode,
	oc_Gen_ToggleTopBar,
#ifdef WITH_MEDIAPLAYER
	oc_Gen_MediaPlayer
#endif
};


std::string InputNames[] = {
	"Up",
	"Down",
	"Left",
	"Right",
	"Shoot",
	"Jump",
	"Select Weapon",
	"Ninja Rope",
	"Strafe"
};

std::string NetworkSpeeds[] = {
	"Modem",
	"ISDN",
	"LAN"
};


///////////////////
// Initialize the options
int Menu_OptionsInitialize(void)
{
	tMenu->iMenuType = MNU_OPTIONS;
	OptionsMode = 0;
    int i;

	// Create the buffer
	DrawImage(tMenu->bmpBuffer,tMenu->bmpMainBack_common,0,0);
	if (tMenu->tFrontendInfo.bPageBoxes)
		Menu_DrawBox(tMenu->bmpBuffer, 15,130, 625, 465);
	Menu_DrawSubTitle(tMenu->bmpBuffer,SUB_OPTIONS);

	Menu_RedrawMouse(true);

	// Setup the top buttons
	TopButtons[op_Controls] =	CButton(BUT_CONTROLS,	tMenu->bmpButtons);
	TopButtons[op_Game] =		CButton(BUT_GAME,		tMenu->bmpButtons);
	TopButtons[op_System] =		CButton(BUT_SYSTEM,		tMenu->bmpButtons);

	TopButtons[0].Setup(op_Controls, 180, 110, 100, 15);
	TopButtons[1].Setup(op_Game, 310, 110, 50, 15);
	TopButtons[2].Setup(op_System, 390, 110, 70, 15);
    for(i=op_Controls; i<=op_System; i++)
        TopButtons[i].Create();

	cOptions.Shutdown();
	cOptions.Initialize();

	cOpt_System.Shutdown();
	cOpt_System.Initialize();

	cOpt_Controls.Shutdown();
	cOpt_Controls.Initialize();

	cOpt_Game.Shutdown();
	cOpt_Game.Initialize();


	// Add the controls
	cOptions.Add( new CButton(BUT_BACK, tMenu->bmpButtons), op_Back, 25,440, 50,15);


	// Controls
	cOpt_Controls.Add( new CLabel("Player Controls", tLX->clHeading), Static, 40,  150, 0,0);
	cOpt_Controls.Add( new CLabel("Player 1",tLX->clSubHeading),      Static, 163, 170, 0,0);
	cOpt_Controls.Add( new CLabel("Player 2",tLX->clSubHeading),      Static, 268, 170, 0,0);
	cOpt_Controls.Add( new CLabel("General Controls", tLX->clHeading),Static, 380, 150, 0,0);

	int y = 190;
	for(i=0;i<9;i++,y+=25) {
		cOpt_Controls.Add( new CLabel(InputNames[i],tLX->clNormalLabel), Static, 40, y, 0,0);

		cOpt_Controls.Add( new CInputbox(SIN_UP+i, tLXOptions->sPlayerControls[0][SIN_UP+i], tMenu->bmpInputbox, InputNames[i]),
			               oc_Ply1_Up+i, 165, y, 50,17);

		cOpt_Controls.Add( new CInputbox(SIN_UP+i, tLXOptions->sPlayerControls[1][SIN_UP+i], tMenu->bmpInputbox, InputNames[i]),
			               oc_Ply2_Up+i, 270, y, 50,17);
	}

	// General Controls
	cOpt_Controls.Add( new CLabel("Chat", tLX->clNormalLabel), Static, 380, 190, 0,0);
	cOpt_Controls.Add( new CInputbox(SIN_CHAT, tLXOptions->sGeneralControls[SIN_CHAT], tMenu->bmpInputbox, "Chat"),
						   oc_Gen_Chat, 515, 190, 50,17);

    cOpt_Controls.Add( new CLabel("Scoreboard", tLX->clNormalLabel), Static, 380, 215, 0,0);
	cOpt_Controls.Add( new CInputbox(SIN_SCORE, tLXOptions->sGeneralControls[SIN_SCORE], tMenu->bmpInputbox, "Scoreboard"),
						   oc_Gen_Score, 515, 215, 50,17);

    cOpt_Controls.Add( new CLabel("Health Bar", tLX->clNormalLabel), Static, 380, 240, 0,0);
	cOpt_Controls.Add( new CInputbox(SIN_HEALTH, tLXOptions->sGeneralControls[SIN_HEALTH], tMenu->bmpInputbox, "Health Bar"),
						   oc_Gen_Health, 515, 240, 50,17);

    cOpt_Controls.Add( new CLabel("Current Settings", tLX->clNormalLabel), Static, 380, 265, 0,0);
	cOpt_Controls.Add( new CInputbox(SIN_SETTINGS, tLXOptions->sGeneralControls[SIN_SETTINGS], tMenu->bmpInputbox, "Current Settings"),
						   oc_Gen_CurSettings, 515, 265, 50,17);

    cOpt_Controls.Add( new CLabel("Take Screenshot", tLX->clNormalLabel), Static, 380, 290, 0,0);
	cOpt_Controls.Add( new CInputbox(SIN_SCREENSHOTS, tLXOptions->sGeneralControls[SIN_SCREENSHOTS], tMenu->bmpInputbox, "Take Screenshot"),
						   oc_Gen_TakeScreenshot, 515, 290, 50,17);

    cOpt_Controls.Add( new CLabel("Viewport Manager", tLX->clNormalLabel), Static, 380, 315, 0,0);
	cOpt_Controls.Add( new CInputbox(SIN_VIEWPORTS, tLXOptions->sGeneralControls[SIN_VIEWPORTS], tMenu->bmpInputbox, "Viewport Manager"),
						   oc_Gen_ViewportManager, 515, 315, 50,17);

    cOpt_Controls.Add( new CLabel("Switch Video Mode", tLX->clNormalLabel), Static, 380, 340, 0,0);
	cOpt_Controls.Add( new CInputbox(SIN_SWITCHMODE, tLXOptions->sGeneralControls[SIN_SWITCHMODE], tMenu->bmpInputbox, "Switch Video Mode"),
						   oc_Gen_SwitchMode, 515, 340, 50,17);

    cOpt_Controls.Add( new CLabel("Toggle Top Bar", tLX->clNormalLabel), Static, 380, 365, 0,0);
	cOpt_Controls.Add( new CInputbox(SIN_TOGGLETOPBAR, tLXOptions->sGeneralControls[SIN_TOGGLETOPBAR], tMenu->bmpInputbox, "Toggle Top Bar"),
						   oc_Gen_ToggleTopBar, 515, 365, 50,17);


#ifdef WITH_MEDIAPLAYER
	cOpt_Controls.Add( new CLabel("Toggle Media Player", tLX->clNormalLabel), Static, 380, 390, 0,0);
	cOpt_Controls.Add( new CInputbox(SIN_MEDIAPLAYER, tLXOptions->sGeneralControls[SIN_MEDIAPLAYER], tMenu->bmpInputbox, "Toggle Media Player"),
						   oc_Gen_MediaPlayer, 515, 390, 50,17);
#endif



	// System
	cOpt_System.Add( new CLabel("Video",tLX->clHeading),              Static, 40, 150, 0,0);
	cOpt_System.Add( new CLabel("Fullscreen",tLX->clNormalLabel),       Static, 60, 170, 0,0);
	cOpt_System.Add( new CLabel("Colour depth",tLX->clNormalLabel),       Static, 175, 170, 0,0);
	cOpt_System.Add( new CCheckbox(tLXOptions->iFullscreen),os_Fullscreen, 140, 170, 17,17);
	cOpt_System.Add( new CLabel("Use OpenGL Rendering",tLX->clNormalLabel),Static, 440, 170, 0,0);
	cOpt_System.Add( new CCheckbox(tLXOptions->bOpenGL),    os_OpenGL, 590, 170, 17,17);

	cOpt_System.Add( new CLabel("Audio",tLX->clHeading),              Static, 40, 205, 0,0);
	cOpt_System.Add( new CLabel("Sound on",tLX->clNormalLabel),         Static, 60, 225, 0,0);
	cOpt_System.Add( new CCheckbox(tLXOptions->iSoundOn),   os_SoundOn, 170, 225, 17,17);
	cOpt_System.Add( new CLabel("Sound volume",tLX->clNormalLabel),     Static, 330, 225, 0,0);
	cOpt_System.Add( new CSlider(100),                      os_SoundVolume, 435, 222, 110, 20);

	cOpt_System.Add( new CLabel("Network",tLX->clHeading),            Static, 40, 260, 0,0);
	cOpt_System.Add( new CLabel("Network port",tLX->clNormalLabel),     Static, 60, 280, 0,0);
	cOpt_System.Add( new CTextbox(),                        os_NetworkPort, 170, 277, 100,tLX->cFont.GetHeight());
	cOpt_System.Add( new CLabel("Network speed",tLX->clNormalLabel),    Static, 60,310, 0,0);
	cOpt_System.Add( new CLabel("Use IP To Country Database",tLX->clNormalLabel),	Static, 330, 280, 0,0);
	cOpt_System.Add( new CCheckbox(tLXOptions->bUseIpToCountry),  os_UseIpToCountry, 530,280,17,17);
	cOpt_System.Add( new CLabel("Load Database at Startup",tLX->clNormalLabel),	Static, 330, 310, 0,0);
	cOpt_System.Add( new CCheckbox(tLXOptions->bLoadDbAtStartup),  os_LoadDbAtStartup, 530,310,17,17);
	cOpt_System.Add( new CLabel("STUN server",tLX->clNormalLabel),     Static, 60, 340, 0,0);
	cOpt_System.Add( new CTextbox(),                        os_STUNServer, 170, 340, 130,tLX->cFont.GetHeight());

	cOpt_System.Add( new CLabel("Miscellanous",tLX->clHeading),       Static, 40, 365, 0,0);
	cOpt_System.Add( new CLabel("Show FPS",tLX->clNormalLabel),         Static, 60, 385, 0,0);
	cOpt_System.Add( new CCheckbox(tLXOptions->iShowFPS),   os_ShowFPS, 200, 385, 17,17);
	cOpt_System.Add( new CLabel("Log Conversations",tLX->clNormalLabel),Static, 60, 415, 0,0);
	cOpt_System.Add( new CCheckbox(tLXOptions->iLogConvos), os_LogConvos, 200,415,17,17);
	cOpt_System.Add( new CLabel("Show ping",tLX->clNormalLabel),		Static, 230, 415, 0,0);
	cOpt_System.Add( new CCheckbox(tLXOptions->iShowPing),  os_ShowPing, 365,415,17,17);
	cOpt_System.Add( new CLabel("Screenshot format",tLX->clNormalLabel),Static, 230,385, 0,0);
	cOpt_System.Add( new CLabel("Max FPS",tLX->clNormalLabel),Static, 480,385, 0,0);
	cOpt_System.Add( new CTextbox(),                        os_MaxFPS, 540, 383, 50,tLX->cFont.GetHeight());


	cOpt_System.SendMessage(os_NetworkPort,TXM_SETMAX,8,0);
	cOpt_System.SendMessage(os_STUNServer,TXM_SETMAX,256,0);

	cOpt_System.Add( new CButton(BUT_APPLY, tMenu->bmpButtons), os_Apply, 555,440, 60,15);

	// Put the combo box after the other widgets to get around the problem with widget layering
	cOpt_System.Add( new CCombobox(), os_NetworkSpeed, 170, 307, 130,17);
	cOpt_System.Add( new CCombobox(), os_ScreenshotFormat, 365, 383, 70,17);
	cOpt_System.Add( new CCombobox(), os_ColourDepth, 275, 170, 145, 17); 

	// Set the values
	CSlider *s = (CSlider *)cOpt_System.getWidget(os_SoundVolume);
	s->setValue( tLXOptions->iSoundVolume );

	CTextbox *t = (CTextbox *)cOpt_System.getWidget(os_NetworkPort);
	t->setText( itoa(tLXOptions->iNetworkPort) );
	t = (CTextbox *)(cOpt_System.getWidget(os_MaxFPS));
	t->setText(itoa(tLXOptions->nMaxFPS));
	t = (CTextbox *)cOpt_System.getWidget(os_STUNServer);
	t->setText( tLXOptions->sSTUNServer.c_str() );

	// Network speed
	for(i=0; i<3; i++)
		cOpt_System.SendMessage(os_NetworkSpeed, CBS_ADDITEM, NetworkSpeeds[i], i);

	cOpt_System.SendMessage(os_NetworkSpeed, CBM_SETCURSEL, tLXOptions->iNetworkSpeed, 0);
	cOpt_System.SendMessage(os_NetworkSpeed, CBM_SETCURINDEX, tLXOptions->iNetworkSpeed, 0);

	// Screenshot format
	cOpt_System.SendMessage(os_ScreenshotFormat, CBS_ADDITEM, "Bmp", FMT_BMP);
	cOpt_System.SendMessage(os_ScreenshotFormat, CBS_ADDITEM, "Png", FMT_PNG);
	cOpt_System.SendMessage(os_ScreenshotFormat, CBS_ADDITEM, "Gif", FMT_GIF);
	cOpt_System.SendMessage(os_ScreenshotFormat, CBS_ADDITEM, "Jpg", FMT_JPG);

	cOpt_System.SendMessage(os_ScreenshotFormat, CBM_SETCURSEL, tLXOptions->iScreenshotFormat, 0);
	cOpt_System.SendMessage(os_ScreenshotFormat, CBM_SETCURINDEX, tLXOptions->iScreenshotFormat, 0);

	// Color depth
	cOpt_System.SendMessage(os_ColourDepth, CBS_ADDITEM, "Automatic", 0);
	cOpt_System.SendMessage(os_ColourDepth, CBS_ADDITEM, "High Color (16 bit)", 1);
	cOpt_System.SendMessage(os_ColourDepth, CBS_ADDITEM, "True Color (24 bit)", 2);
	cOpt_System.SendMessage(os_ColourDepth, CBS_ADDITEM, "True Color (32 bit)", 3);

	switch (tLXOptions->iColourDepth) {
	case 0:  // Automatic
		cOpt_System.SendMessage(os_ColourDepth, CBM_SETCURSEL, (DWORD)0, 0);
		cOpt_System.SendMessage(os_ColourDepth, CBM_SETCURINDEX, (DWORD)0, 0);
		break;
	case 16:  // 16 bit
		cOpt_System.SendMessage(os_ColourDepth, CBM_SETCURSEL, (DWORD)1, 0);
		cOpt_System.SendMessage(os_ColourDepth, CBM_SETCURINDEX, (DWORD)1, 0);
		break;
	case 24:  // 24 bit
		cOpt_System.SendMessage(os_ColourDepth, CBM_SETCURSEL, (DWORD)2, 0);
		cOpt_System.SendMessage(os_ColourDepth, CBM_SETCURINDEX, (DWORD)2, 0);
		break;
	case 32:  // 32 bit
		cOpt_System.SendMessage(os_ColourDepth, CBM_SETCURSEL, (DWORD)3, 0);
		cOpt_System.SendMessage(os_ColourDepth, CBM_SETCURINDEX, (DWORD)3, 0);
		break;
	}


	// Disable apply for now
	cOpt_System.getWidget(os_Apply)->setEnabled(false);


	// Game
	cOpt_Game.Add( new CLabel("Blood Amount",tLX->clNormalLabel),       Static, 40, 150, 0,0);
	cOpt_Game.Add( new CSlider(5000),                       og_BloodAmount, 175, 147, 210, 20);
	cOpt_Game.Add( new CLabel("Shadows",tLX->clNormalLabel),            Static, 40, 180, 0,0);
	cOpt_Game.Add( new CCheckbox(tLXOptions->iShadows),     og_Shadows, 280, 180, 17,17);
	cOpt_Game.Add( new CLabel("Particles",tLX->clNormalLabel),          Static, 40, 210, 0,0);
	cOpt_Game.Add( new CCheckbox(tLXOptions->iParticles),   og_Particles, 280, 210, 17,17);
	cOpt_Game.Add( new CLabel("Classic Rope throw",tLX->clNormalLabel), Static, 40, 240, 0,0);
	cOpt_Game.Add( new CCheckbox(tLXOptions->iOldSkoolRope),og_OldSkoolRope, 280, 240, 17,17);
	//cOpt_Game.Add( new CLabel("Show worm's health",tLX->clNormalLabel), Static, 40, 270, 0,0);
	//cOpt_Game.Add( new CCheckbox(tLXOptions->iShowHealth),  og_ShowWormHealth, 280, 270, 17,17);
	cOpt_Game.Add( new CLabel("Colorize nicks by teams",tLX->clNormalLabel), Static, 40, 270, 0,0);
	cOpt_Game.Add( new CCheckbox(tLXOptions->iColorizeNicks),og_ColorizeNicks, 280, 270, 17,17);
	cOpt_Game.Add( new CLabel("Start typing after any key press",tLX->clNormalLabel), Static, 40, 300, 0,0);
	cOpt_Game.Add( new CCheckbox(tLXOptions->iAutoTyping),og_AutoTyping, 280, 300, 17,17);
	cOpt_Game.Add( new CLabel("Use antialiasing (slow)",tLX->clNormalLabel), Static, 40, 330, 0,0);
	cOpt_Game.Add( new CCheckbox(tLXOptions->bAntiAliasing),og_Antialiasing, 280, 330, 17,17);
	//cOpt_Game.Add( new CLabel("AI Difficulty",tLX->clNormalLabel), Static, 40, 270, 0,0);
	//cOpt_Game.Add( new CSlider(3), og_AIDifficulty,   175, 267, 100, 20);
	cOpt_Game.Add( new CLabel("Use mouse for aiming (Player 1)",tLX->clNormalLabel), Static, 40, 360, 0,0);
	cOpt_Game.Add( new CCheckbox(tLXOptions->bMouseAiming),og_MouseAiming, 280, 360, 17,17);
	cOpt_Game.Add( new CLabel("Log my game results",tLX->clNormalLabel), Static, 40, 390, 0,0);
	cOpt_Game.Add( new CCheckbox(tLXOptions->tGameinfo.bMatchLogging),og_MatchLogging, 280, 390, 17,17);

	cOpt_Game.Add( new CLabel("Allow mouse aiming (Server)",tLX->clNormalLabel), Static, 330, 360, 0,0);
	cOpt_Game.Add( new CCheckbox(tLXOptions->bAllowMouseAiming),og_AllowMouseAiming, 550, 360, 17,17);

	cOpt_Game.Add( new CLabel("Use 1-5 keys to switch weapons", tLX->clNormalLabel), Static, 330, 390, 0,0);
	cOpt_Game.Add( new CCheckbox(tLXOptions->bUseNumericKeysToSwitchWeapons),og_UseNumericKeysToSwitchWeapons, 550, 390, 17,17);

	// TODO: Fix cSlider so it's value thing doesn't take up a square of 100x100 pixels.

	// Set the values
	cOpt_Game.SendMessage( og_BloodAmount,  SLM_SETVALUE, tLXOptions->iBloodAmount, 0);
	//cOpt_Game.SendMessage( og_AIDifficulty, SLM_SETVALUE, tLXOptions->iAIDifficulty, 0);





	return true;
}


///////////////////
// Options main frame
void Menu_OptionsFrame(void)
{
	mouse_t		*Mouse = GetMouse();
	gui_event_t *ev = NULL;
	int			val;

	CCheckbox	*c,*c2;
	//CSlider		*s;

	//DrawImageAdv(tMenu->bmpScreen, tMenu->bmpBuffer,  180,110,  180,110,  300,30);
	//DrawImageAdv(tMenu->bmpScreen, tMenu->bmpBuffer, 20,140, 20,140, 620,340);

	
	// Process the top buttons
	TopButtons[OptionsMode].MouseOver(Mouse);
	SetGameCursor(CURSOR_ARROW); // Hack: button changed the cursor to hand, we need to change it back
	for(int i=op_Controls;i<=op_System;i++) {

		TopButtons[i].Draw(tMenu->bmpScreen);

		if(i==OptionsMode)
			continue;

		if(TopButtons[i].InBox(Mouse->X,Mouse->Y)) {
			TopButtons[i].MouseOver(Mouse);
			if(Mouse->Up) {
                DrawImageAdv(tMenu->bmpScreen, tMenu->bmpBuffer, 20,140, 20,140, 620,340);
				OptionsMode = i;
				PlaySoundSample(sfxGeneral.smpClick);
			}
		}
	}



	// Process the gui layout
#ifdef WITH_MEDIAPLAYER
	if (!cMediaPlayer.GetDrawPlayer())
#endif
	ev = cOptions.Process();
	cOptions.Draw(tMenu->bmpScreen);

	if(ev) {

		switch(ev->iControlID) {

			// Back button
			case op_Back:
				if(ev->iEventMsg == BTN_MOUSEUP) {

					// Shutdown & save
					Menu_OptionsShutdown();
					tLXOptions->SaveToDisc();

					// Leave
					PlaySoundSample(sfxGeneral.smpClick);
					Menu_MainInitialize();
					return;
				}
				break;
		}
	}


	if(OptionsMode == 0) {

		// Controls
#ifdef WITH_MEDIAPLAYER
		if (!cMediaPlayer.GetDrawPlayer())
#endif
			ev = cOpt_Controls.Process();
		cOpt_Controls.Draw(tMenu->bmpScreen);

		if(ev) {

			if(ev->cWidget->getType() == wid_Inputbox) {

				if(ev->iEventMsg == INB_MOUSEUP) {

					int ply = 0;
					if(ev->iControlID >= oc_Ply2_Up && ev->iControlID <= oc_Ply2_Strafe)
						ply = 1;
					if(ev->iControlID >= oc_Gen_Chat)
						ply = -1;

					// Get an input
					CInputbox *b = (CInputbox *)ev->cWidget;
					Menu_OptionsWaitInput(ply, b->getName(), b);
					// Re-setup the Take Screenshot, Switch Mode and Media Player keys
					if (ev->iControlID == oc_Gen_TakeScreenshot)
						cTakeScreenshot.Setup(tLXOptions->sGeneralControls[SIN_SCREENSHOTS]);
					if (ev->iControlID == oc_Gen_SwitchMode)
						cSwitchMode.Setup(tLXOptions->sGeneralControls[SIN_SWITCHMODE]);
#ifdef WITH_MEDIAPLAYER
					if (ev->iControlID == oc_Gen_MediaPlayer)
						cToggleMediaPlayer.Setup(tLXOptions->sGeneralControls[SIN_MEDIAPLAYER]);
#endif
				}
			}
		}
	}


	if(OptionsMode == 1) {

		// Game
#ifdef WITH_MEDIAPLAYER
		if (!cMediaPlayer.GetDrawPlayer())
#endif
			ev = cOpt_Game.Process();
		cOpt_Game.Draw(tMenu->bmpScreen);

		val = cOpt_Game.SendMessage(og_BloodAmount, SLM_GETVALUE, (DWORD)0, 0);
		//s = (CSlider *)cOpt_Game.getWidget(og_BloodAmount);
        DrawImageAdv(tMenu->bmpScreen, tMenu->bmpBuffer, 385,140, 385,140, 70,50);
		tLX->cFont.Draw(tMenu->bmpScreen,385, 148, tLX->clNormalLabel, itoa(val)+"%");

		//val = cOpt_Game.SendMessage(og_AIDifficulty, SLM_GETVALUE, 0, 0);
        //DrawImageAdv(tMenu->bmpScreen, tMenu->bmpBuffer, 285,260, 285,260, 100,50);
		//tLX->cFont.Draw(tMenu->bmpScreen,285, 268, tLX->clNormalLabel,Difficulties[val]);



		if(ev) {

			switch(ev->iControlID) {

				// Blood amount
				case og_BloodAmount:
					if(ev->iEventMsg == SLD_CHANGE) {
						val = cOpt_Game.SendMessage(og_BloodAmount, SLM_GETVALUE, (DWORD)0, 0);
						tLXOptions->iBloodAmount = val;
					}
					break;

				// Shadows
				case og_Shadows:
					if(ev->iEventMsg == CHK_CHANGED) {
						c = (CCheckbox *)cOpt_Game.getWidget(og_Shadows);
						tLXOptions->iShadows = c->getValue();
					}
					break;

				// Particles
				case og_Particles:
					if(ev->iEventMsg == CHK_CHANGED) {
						c = (CCheckbox *)cOpt_Game.getWidget(og_Particles);
						tLXOptions->iParticles = c->getValue();
					}
					break;

				// AI Difficulty
				/*case og_AIDifficulty:
					if(ev->iEventMsg == SLD_CHANGE) {
						val = cOpt_Game.SendMessage(og_AIDifficulty, SLM_GETVALUE, 0, 0);
						tLXOptions->iAIDifficulty = val;
					}
					break;*/

				// Old skool rope throw
				case og_OldSkoolRope:
					if(ev->iEventMsg == CHK_CHANGED) {
						tLXOptions->iOldSkoolRope = cOpt_Game.SendMessage(og_OldSkoolRope,CKM_GETCHECK,(DWORD)0,0);
					}
					break;

				// Show the worm's health below name
/*				case og_ShowWormHealth:
					if(ev->iEventMsg == CHK_CHANGED)
						tLXOptions->iShowHealth = cOpt_Game.SendMessage(og_ShowWormHealth, CKM_GETCHECK, (DWORD)0, 0);
					break;*/

				// TDM nick colorizing
				case og_ColorizeNicks:
					if(ev->iEventMsg == CHK_CHANGED)
						tLXOptions->iColorizeNicks = cOpt_Game.SendMessage(og_ColorizeNicks, CKM_GETCHECK, (DWORD)0, 0);
					break;

				// Auto typing
				case og_AutoTyping:
					if(ev->iEventMsg == CHK_CHANGED)
						tLXOptions->iAutoTyping = cOpt_Game.SendMessage(og_AutoTyping, CKM_GETCHECK, (DWORD)0, 0);
					break;

				// Antialiasing
				case og_Antialiasing:
					if(ev->iEventMsg == CHK_CHANGED)
						tLXOptions->bAntiAliasing = cOpt_Game.SendMessage(og_Antialiasing, CKM_GETCHECK, (DWORD)0, 0) != 0;
					break;

				// Mouse aiming
				case og_MouseAiming:
					if(ev->iEventMsg == CHK_CHANGED)
						tLXOptions->bMouseAiming = cOpt_Game.SendMessage(og_MouseAiming, CKM_GETCHECK, (DWORD)0, 0) != 0;
					break;

				case og_AllowMouseAiming:
					if(ev->iEventMsg == CHK_CHANGED)
						tLXOptions->bAllowMouseAiming = cOpt_Game.SendMessage(og_AllowMouseAiming, CKM_GETCHECK, (DWORD)0, 0) != 0;
					break;

				// Match logging
				case og_MatchLogging:
					if(ev->iEventMsg == CHK_CHANGED)
						tLXOptions->tGameinfo.bMatchLogging = cOpt_Game.SendMessage(og_MatchLogging, CKM_GETCHECK, (DWORD)0, 0) != 0;
					break;

				case og_UseNumericKeysToSwitchWeapons:
					if(ev->iEventMsg == CHK_CHANGED)
						tLXOptions->bUseNumericKeysToSwitchWeapons = cOpt_Game.SendMessage(og_UseNumericKeysToSwitchWeapons, CKM_GETCHECK, (DWORD)0, 0) != 0;
					break;

			}
		}

	}


	// Process the different pages
	if(OptionsMode == 2) {

		// Fullscreen value
		c = (CCheckbox *)cOpt_System.getWidget(os_Fullscreen);
		int fullscr = c->getValue();
		// OpenGL accel value
		c2 = (CCheckbox *)cOpt_System.getWidget(os_OpenGL);
		bool opengl = c2->getValue () != 0;
		// Color depth
		int cdepth = ((CCombobox *)cOpt_System.getWidget(os_ColourDepth))->getSelectedIndex();
		switch (cdepth)  {
		case 0: cdepth = 0; break;
		case 1: cdepth = 16; break;
		case 2: cdepth = 24; break;
		case 3: cdepth = 32; break;
		default: cdepth = 16;
		}

		// FIXME: WARNING! If OpenGL acceleration is not supported,
		//                 this could lead to a crash!



		// System
#ifdef WITH_MEDIAPLAYER
		if (!cMediaPlayer.GetDrawPlayer())
#endif
			ev = cOpt_System.Process();
		cOpt_System.Draw(tMenu->bmpScreen);

		if(ev) {

			switch(ev->iControlID) {

				// Apply
				case os_Apply:
					if(ev->iEventMsg == BTN_MOUSEUP) {

						// Set to fullscreen / OpenGL / change colour depth
						tLXOptions->iFullscreen = fullscr;
						tLXOptions->bOpenGL = opengl;
						tLXOptions->iColourDepth = cdepth;
						PlaySoundSample(sfxGeneral.smpClick);

						// Set the new video mode
						SetVideoMode();

						tMenu->bmpScreen = SDL_GetVideoSurface();
						Menu_RedrawMouse(true);
						SDL_ShowCursor(SDL_DISABLE);
					}
					break;

				// Sound on/off
				case os_SoundOn:
					if(ev->iEventMsg == CHK_CHANGED) {

						int old = tLXOptions->iSoundOn;

						c = (CCheckbox *)cOpt_System.getWidget(os_SoundOn);
						tLXOptions->iSoundOn = c->getValue();

						if(old != tLXOptions->iSoundOn) {
							if(tLXOptions->iSoundOn)
								StartSoundSystem();
							else
								StopSoundSystem();
						}
					}
					break;

				// Sound volume
				case os_SoundVolume:
					if(ev->iEventMsg == SLD_CHANGE) {
						CSlider *s = (CSlider *)cOpt_System.getWidget(os_SoundVolume);
						tLXOptions->iSoundVolume = s->getValue();

						SetSoundVolume( tLXOptions->iSoundVolume );
					}
					break;

				// Show FPS
				case os_ShowFPS:
					if(ev->iEventMsg == CHK_CHANGED)
						tLXOptions->iShowFPS = cOpt_System.SendMessage(os_ShowFPS, CKM_GETCHECK, (DWORD)0, 0);
					break;

				// Logging
				case os_LogConvos:
					if(ev->iEventMsg == CHK_CHANGED)  {
						tLXOptions->iLogConvos = cOpt_System.SendMessage(os_LogConvos, CKM_GETCHECK, (DWORD)0, 0);
						FILE *f;

						f = OpenGameFile("Conversations.log","a");
						if (f)  {
							if (tLXOptions->iLogConvos)  {
								static std::string cTime = GetTime();
								fprintf(f,"<game starttime=\"%s\">\r\n",cTime.c_str());
							}
							else
								fprintf(f,"</game>\r\n");
							fclose(f);
						} // if (f)
					}
					break;

				// Show ping
				case os_ShowPing:
					if(ev->iEventMsg == CHK_CHANGED)
						tLXOptions->iShowPing = cOpt_System.SendMessage(os_ShowPing, CKM_GETCHECK, (DWORD)0, 0);
					break;

				// Use Ip To Country
				case os_UseIpToCountry:
					if(ev->iEventMsg == CHK_CHANGED)
						tLXOptions->bUseIpToCountry = cOpt_System.SendMessage(os_UseIpToCountry, CKM_GETCHECK, (DWORD)0, 0) != 0;
					break;

				// Load Database at Startup
				case os_LoadDbAtStartup:
					if(ev->iEventMsg == CHK_CHANGED)
						tLXOptions->bLoadDbAtStartup = cOpt_System.SendMessage(os_LoadDbAtStartup, CKM_GETCHECK, (DWORD)0, 0) != 0;
					break;
			}
		}


		// Get the values
		CTextbox *t = (CTextbox *)cOpt_System.getWidget(os_NetworkPort);
		tLXOptions->iNetworkPort = atoi(t->getText());
		t = (CTextbox *)cOpt_System.getWidget(os_STUNServer);
		tLXOptions->sSTUNServer = t->getText();
		t = (CTextbox *)cOpt_System.getWidget(os_MaxFPS);
		tLXOptions->nMaxFPS = MAX(1,atoi(t->getText()));

		tLXOptions->iNetworkSpeed = cOpt_System.SendMessage(os_NetworkSpeed, CBM_GETCURINDEX,(DWORD)0,0);

		tLXOptions->iScreenshotFormat = cOpt_System.SendMessage(os_ScreenshotFormat, CBM_GETCURINDEX,(DWORD)0,0);

		tLXOptions->iColourDepth = cOpt_System.SendMessage(os_ColourDepth, CBM_GETCURINDEX,(DWORD)0,0); 
		switch (tLXOptions->iColourDepth)  {
		case 0: tLXOptions->iColourDepth = 0; break;
		case 1: tLXOptions->iColourDepth = 16; break;
		case 2: tLXOptions->iColourDepth = 24; break;
		case 3: tLXOptions->iColourDepth = 32; break;
		default: tLXOptions->iColourDepth = 16;
		}

		if ((cdepth != tLXOptions->iColourDepth) || (opengl != tLXOptions->bOpenGL))  {
			Menu_MessageBox("Information","You need to restart LieroX for the changes to take effect",LMB_OK);
			Menu_redrawBufferRect(0,0,640,480);
			tLXOptions->bOpenGL = opengl;
		}


		if(fullscr != tLXOptions->iFullscreen)
			cOpt_System.getWidget(os_Apply)->setEnabled(true);
        else {
			cOpt_System.getWidget(os_Apply)->setEnabled(false);

            // Redraw the section around the apply button
			if (!cOpt_System.SendMessage(os_ScreenshotFormat, CBM_ISDROPPED,(DWORD)0,0))
				Menu_redrawBufferRect(550,435, 80,25);
        }
	}


	// Draw the mouse
	DrawCursor(tMenu->bmpScreen);
}


///////////////////
// Process an input box waiting thing
// ply=-1 : general ; ply>=0 : normal player
void Menu_OptionsWaitInput(int ply, const std::string& name, CInputbox *b)
{
	keyboard_t *kb = GetKeyboard();
	mouse_t *Mouse = GetMouse();

	// Draw the back buffer
	cOptions.Draw(tMenu->bmpBuffer);
	cOpt_Controls.Draw(tMenu->bmpBuffer);

	Menu_DrawBox(tMenu->bmpBuffer, 210, 170, 430, 310);
	//DrawImageAdv(tMenu->bmpBuffer, tMenu->bmpMainBack, 212,172, 212,172, 217,137);
    DrawRectFill(tMenu->bmpBuffer, 212,172,429,309,tLX->clDialogBackground);

	tLX->cFont.DrawCentre(tMenu->bmpBuffer,320,180,MakeColour(128,200,255),"Input for:");
	tLX->cFont.DrawCentre(tMenu->bmpBuffer,320,205,MakeColour(255,255,255),name);

	tLX->cFont.DrawCentre(tMenu->bmpBuffer,320,270,MakeColour(255,255,255),"Press any key/mouse");
	tLX->cFont.DrawCentre(tMenu->bmpBuffer,320,285,MakeColour(128,128,128),"(Escape to cancel)");

	TopButtons[OptionsMode].MouseOver(Mouse);
	for(ushort i=0;i<3;i++) {
		TopButtons[i].Draw(tMenu->bmpBuffer);
	}


	Menu_RedrawMouse(true);

	CInput inp;

	Mouse->Up = 0;
	Mouse->Down = 0;


	SetGameCursor(CURSOR_ARROW);
	while(true) {
		Menu_RedrawMouse(false);
		ProcessEvents();

		DrawCursor(tMenu->bmpScreen);

		// Escape quits the wait for user input
		if(kb->KeyUp[SDLK_ESCAPE])
			break;

		static std::string tmp;
		if(inp.Wait(tmp)) {
			b->setText(tmp);
			break;
		}

		FlipScreen(tMenu->bmpScreen);
	}

	// Change the options
	if(ply >= 0) {
		tLXOptions->sPlayerControls[ply][b->getValue()] = b->getText();
	} else
		tLXOptions->sGeneralControls[b->getValue()] = b->getText();

	Mouse->Down = 0;
	Mouse->Up = 0;

	DrawImage(tMenu->bmpBuffer,tMenu->bmpMainBack_common,0,0);
	if (tMenu->tFrontendInfo.bPageBoxes)
		Menu_DrawBox(tMenu->bmpBuffer, 15,130, 625, 465);
	Menu_DrawSubTitle(tMenu->bmpBuffer,SUB_OPTIONS);

	Menu_RedrawMouse(true);
}


///////////////////
// Shutdown the options menu
void Menu_OptionsShutdown(void)
{
	cOptions.Shutdown();
	cOpt_Controls.Shutdown();
	cOpt_System.Shutdown();
	cOpt_Game.Shutdown();
}
