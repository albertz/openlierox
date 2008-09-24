/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Floating options window file
// Created 24/9/08
// Karel Petranek

#include "Options.h"
#include "CClient.h"
#include "AuxLib.h"
#include "DeprecatedGUI/Graphics.h"
#include "DeprecatedGUI/Menu.h"
#include "GfxPrimitives.h"
#include "FindFile.h"
#include "StringUtils.h"
#include "DeprecatedGUI/CButton.h"
#include "DeprecatedGUI/CLabel.h"
#include "DeprecatedGUI/CCheckbox.h"
#include "DeprecatedGUI/CTextbox.h"
#include "DeprecatedGUI/CSlider.h"
#include "DeprecatedGUI/CMediaPlayer.h"


namespace DeprecatedGUI {

void SetupGameInputs()
{
	// Setup global keys
	cTakeScreenshot.Setup(tLXOptions->sGeneralControls[SIN_SCREENSHOTS]);
	cSwitchMode.Setup(tLXOptions->sGeneralControls[SIN_SWITCHMODE]);
#ifdef WITH_MEDIAPLAYER
	cToggleMediaPlayer.Setup(tLXOptions->sGeneralControls[SIN_MEDIAPLAYER]);
#endif

	cClient->SetupGameInputs();
};


int iFloatingOptionsMode = 0;
CGuiLayout	cFloatingOptions;
CGuiLayout	cFloatingOpt_Controls;
CGuiLayout	cFloatingOpt_System;
CGuiLayout	cFloatingOpt_Game;
CButton		cFloatingOpt_TopButtons[3];

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
	//os_ColourDepth,
	os_SoundOn,
	os_SoundVolume,
	os_NetworkPort,
	os_NetworkSpeed,
	os_UseIpToCountry,
	os_LoadDbAtStartup,
	os_NatTraverse,
	os_HttpProxy,
	os_ShowFPS,
	//os_OpenGL,
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
	og_AntilagMovementPrediction,
	og_AllowFileDownload,
	og_AllowDirtUpdates
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
	oc_Gen_TeamChat,
#ifdef WITH_MEDIAPLAYER
	oc_Gen_MediaPlayer
#endif
};


std::string sFloatingOpt_InputNames[] = {
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

std::string sFloatingOpt_NetworkSpeeds[] = {
	"Modem",
	"ISDN",
	"LAN"
};


///////////////////
// Initialize the options
bool Menu_FloatingOptionsInitialize(void)
{
	iFloatingOptionsMode = 0;
    int i;

	// Background image is redrawn each frame, so commented this out
	/*
	// Create the buffer
	DrawImage(tMenu->bmpBuffer.get(),tMenu->bmpMainBack_common,0,0);
	if (tMenu->tFrontendInfo.bPageBoxes)
		Menu_DrawBox(tMenu->bmpBuffer.get(), 15,130, 625, 465);
	Menu_DrawSubTitle(tMenu->bmpBuffer.get(),SUB_OPTIONS);

	Menu_RedrawMouse(true);
	*/

	// Setup the top buttons
	cFloatingOpt_TopButtons[op_Controls] =	CButton(BUT_CONTROLS,	tMenu->bmpButtons);
	cFloatingOpt_TopButtons[op_Game] =		CButton(BUT_GAME,		tMenu->bmpButtons);
	cFloatingOpt_TopButtons[op_System] =		CButton(BUT_SYSTEM,		tMenu->bmpButtons);

	cFloatingOpt_TopButtons[0].Setup(op_Controls, 180, 110, 100, 15);
	cFloatingOpt_TopButtons[1].Setup(op_Game, 310, 110, 50, 15);
	cFloatingOpt_TopButtons[2].Setup(op_System, 390, 110, 70, 15);
    for(i=op_Controls; i<=op_System; i++)
        cFloatingOpt_TopButtons[i].Create();

	cFloatingOptions.Shutdown();
	cFloatingOptions.Initialize();

	cFloatingOpt_System.Shutdown();
	cFloatingOpt_System.Initialize();

	cFloatingOpt_Controls.Shutdown();
	cFloatingOpt_Controls.Initialize();

	cFloatingOpt_Game.Shutdown();
	cFloatingOpt_Game.Initialize();


	// Add the controls
	cFloatingOptions.Add( new CButton(BUT_BACK, tMenu->bmpButtons), op_Back, 25,440, 50,15);


	// Controls
	cFloatingOpt_Controls.Add( new CLabel("Player Controls", tLX->clHeading), Static, 40,  150, 0,0);
	cFloatingOpt_Controls.Add( new CLabel("Player 1",tLX->clSubHeading),      Static, 163, 170, 0,0);
	cFloatingOpt_Controls.Add( new CLabel("Player 2",tLX->clSubHeading),      Static, 268, 170, 0,0);
	cFloatingOpt_Controls.Add( new CLabel("General Controls", tLX->clHeading),Static, 390, 150, 0,0);

	int y = 190;
	for(i=0;i<9;i++,y+=25) {
		cFloatingOpt_Controls.Add( new CLabel(sFloatingOpt_InputNames[i],tLX->clNormalLabel), Static, 40, y, 0,0);

		cFloatingOpt_Controls.Add( new CInputbox(SIN_UP+i, tLXOptions->sPlayerControls[0][SIN_UP+i], tMenu->bmpInputbox, sFloatingOpt_InputNames[i]),
			               oc_Ply1_Up+i, 165, y, 50,17);
		cFloatingOpt_Controls.Add( new CInputbox(SIN_UP+i, tLXOptions->sPlayerControls[1][SIN_UP+i], tMenu->bmpInputbox, sFloatingOpt_InputNames[i]),
			               oc_Ply2_Up+i, 270, y, 50,17);

	}

	// General Controls
	cFloatingOpt_Controls.Add( new CLabel("Chat", tLX->clNormalLabel), Static, 390, 190, 0,0);
	cFloatingOpt_Controls.Add( new CInputbox(SIN_CHAT, tLXOptions->sGeneralControls[SIN_CHAT], tMenu->bmpInputbox, "Chat"),
						   oc_Gen_Chat, 525, 190, 50,17);

    cFloatingOpt_Controls.Add( new CLabel("Scoreboard", tLX->clNormalLabel), Static, 390, 215, 0,0);
	cFloatingOpt_Controls.Add( new CInputbox(SIN_SCORE, tLXOptions->sGeneralControls[SIN_SCORE], tMenu->bmpInputbox, "Scoreboard"),
						   oc_Gen_Score, 525, 215, 50,17);

    cFloatingOpt_Controls.Add( new CLabel("Health Bar", tLX->clNormalLabel), Static, 390, 240, 0,0);
	cFloatingOpt_Controls.Add( new CInputbox(SIN_HEALTH, tLXOptions->sGeneralControls[SIN_HEALTH], tMenu->bmpInputbox, "Health Bar"),
						   oc_Gen_Health, 525, 240, 50,17);

    cFloatingOpt_Controls.Add( new CLabel("Current Settings", tLX->clNormalLabel), Static, 390, 265, 0,0);
	cFloatingOpt_Controls.Add( new CInputbox(SIN_SETTINGS, tLXOptions->sGeneralControls[SIN_SETTINGS], tMenu->bmpInputbox, "Current Settings"),
						   oc_Gen_CurSettings, 525, 265, 50,17);

    cFloatingOpt_Controls.Add( new CLabel("Take Screenshot", tLX->clNormalLabel), Static, 390, 290, 0,0);
	cFloatingOpt_Controls.Add( new CInputbox(SIN_SCREENSHOTS, tLXOptions->sGeneralControls[SIN_SCREENSHOTS], tMenu->bmpInputbox, "Take Screenshot"),
						   oc_Gen_TakeScreenshot, 525, 290, 50,17);

    cFloatingOpt_Controls.Add( new CLabel("Viewport Manager", tLX->clNormalLabel), Static, 390, 315, 0,0);
	cFloatingOpt_Controls.Add( new CInputbox(SIN_VIEWPORTS, tLXOptions->sGeneralControls[SIN_VIEWPORTS], tMenu->bmpInputbox, "Viewport Manager"),
						   oc_Gen_ViewportManager, 525, 315, 50,17);

    cFloatingOpt_Controls.Add( new CLabel("Switch Video Mode", tLX->clNormalLabel), Static, 390, 340, 0,0);
	cFloatingOpt_Controls.Add( new CInputbox(SIN_SWITCHMODE, tLXOptions->sGeneralControls[SIN_SWITCHMODE], tMenu->bmpInputbox, "Switch Video Mode"),
						   oc_Gen_SwitchMode, 525, 340, 50,17);

    cFloatingOpt_Controls.Add( new CLabel("Toggle Top Bar", tLX->clNormalLabel), Static, 390, 365, 0,0);
	cFloatingOpt_Controls.Add( new CInputbox(SIN_TOGGLETOPBAR, tLXOptions->sGeneralControls[SIN_TOGGLETOPBAR], tMenu->bmpInputbox, "Toggle Top Bar"),
						   oc_Gen_ToggleTopBar, 525, 365, 50,17);


#ifdef WITH_MEDIAPLAYER
	cFloatingOpt_Controls.Add( new CLabel("Toggle Media Player", tLX->clNormalLabel), Static, 390, 390, 0,0);
	cFloatingOpt_Controls.Add( new CInputbox(SIN_MEDIAPLAYER, tLXOptions->sGeneralControls[SIN_MEDIAPLAYER], tMenu->bmpInputbox, "Toggle Media Player"),
						   oc_Gen_MediaPlayer, 525, 390, 50,17);
#endif

	cFloatingOpt_Controls.Add( new CLabel("Teamchat", tLX->clNormalLabel), Static, 390, 415, 0,0);
	cFloatingOpt_Controls.Add( new CInputbox(SIN_TEAMCHAT, tLXOptions->sGeneralControls[SIN_TEAMCHAT], tMenu->bmpInputbox, "Teamchat"),
						   oc_Gen_TeamChat, 525, 415, 50,17);



	// System
	cFloatingOpt_System.Add( new CLabel("Video",tLX->clHeading),              Static, 40, 150, 0,0);
	cFloatingOpt_System.Add( new CLabel("Fullscreen",tLX->clNormalLabel),       Static, 60, 170, 0,0);
	/*
	cFloatingOpt_System.Add( new CLabel("Colour depth",tLX->clNormalLabel),       Static, 175, 170, 0,0);
	cFloatingOpt_System.Add( new CCheckbox(tLXOptions->bFullscreen),os_Fullscreen, 140, 170, 17,17);
	cFloatingOpt_System.Add( new CLabel("Use OpenGL Rendering",tLX->clNormalLabel),Static, 440, 170, 0,0);
	cFloatingOpt_System.Add( new CCheckbox(tLXOptions->bOpenGL),    os_OpenGL, 590, 170, 17,17);
	*/

	cFloatingOpt_System.Add( new CLabel("Audio",tLX->clHeading),              Static, 40, 205, 0,0);
	cFloatingOpt_System.Add( new CLabel("Sound on",tLX->clNormalLabel),         Static, 60, 225, 0,0);
	cFloatingOpt_System.Add( new CCheckbox(tLXOptions->bSoundOn),   os_SoundOn, 170, 225, 17,17);
	cFloatingOpt_System.Add( new CLabel("Sound volume",tLX->clNormalLabel),     Static, 330, 225, 0,0);
	cFloatingOpt_System.Add( new CSlider(100),                      os_SoundVolume, 435, 222, 110, 20);

	cFloatingOpt_System.Add( new CLabel("Network",tLX->clHeading),            Static, 40, 260, 0,0);
	cFloatingOpt_System.Add( new CLabel("Network port",tLX->clNormalLabel),     Static, 60, 280, 0,0);
	cFloatingOpt_System.Add( new CTextbox(),                        os_NetworkPort, 170, 277, 100,tLX->cFont.GetHeight());
	cFloatingOpt_System.Add( new CLabel("Network speed",tLX->clNormalLabel),    Static, 60,310, 0,0);
	cFloatingOpt_System.Add( new CLabel("HTTP proxy",tLX->clNormalLabel),    Static, 60,340, 0,0);
	cFloatingOpt_System.Add( new CTextbox(),                        os_HttpProxy, 170, 337, 130,tLX->cFont.GetHeight());
	cFloatingOpt_System.Add( new CLabel("Use IP To Country Database",tLX->clNormalLabel),	Static, 330, 280, 0,0);
	cFloatingOpt_System.Add( new CCheckbox(tLXOptions->bUseIpToCountry),  os_UseIpToCountry, 530,280,17,17);
	cFloatingOpt_System.Add( new CLabel("Load Database at Startup",tLX->clNormalLabel),	Static, 330, 310, 0,0);
	cFloatingOpt_System.Add( new CCheckbox(tLXOptions->bLoadDbAtStartup),  os_LoadDbAtStartup, 530,310,17,17);
	cFloatingOpt_System.Add( new CLabel("Use UDP masterserver",tLX->clNormalLabel),     Static, 330, 340, 0,0);
	cFloatingOpt_System.Add( new CCheckbox(tLXOptions->bNatTraverse),  os_NatTraverse, 530,340,17,17);

	cFloatingOpt_System.Add( new CLabel("Miscellanous",tLX->clHeading),       Static, 40, 365, 0,0);
	cFloatingOpt_System.Add( new CLabel("Show FPS",tLX->clNormalLabel),         Static, 60, 385, 0,0);
	cFloatingOpt_System.Add( new CCheckbox(tLXOptions->bShowFPS),   os_ShowFPS, 200, 385, 17,17);
	cFloatingOpt_System.Add( new CLabel("Log Conversations",tLX->clNormalLabel),Static, 60, 415, 0,0);
	cFloatingOpt_System.Add( new CCheckbox(tLXOptions->bLogConvos), os_LogConvos, 200,415,17,17);
	cFloatingOpt_System.Add( new CLabel("Show ping",tLX->clNormalLabel),		Static, 230, 415, 0,0);
	cFloatingOpt_System.Add( new CCheckbox(tLXOptions->bShowPing),  os_ShowPing, 365,415,17,17);
	cFloatingOpt_System.Add( new CLabel("Screenshot format",tLX->clNormalLabel),Static, 230,385, 0,0);
	cFloatingOpt_System.Add( new CLabel("Max FPS",tLX->clNormalLabel),Static, 480,385, 0,0);
	cFloatingOpt_System.Add( new CTextbox(),                        os_MaxFPS, 540, 383, 50,tLX->cFont.GetHeight());


	cFloatingOpt_System.SendMessage(os_NetworkPort,TXM_SETMAX,5,0);

	cFloatingOpt_System.Add( new CButton(BUT_APPLY, tMenu->bmpButtons), os_Apply, 555,440, 60,15);

	// Put the combo box after the other widgets to get around the problem with widget layering
	cFloatingOpt_System.Add( new CCombobox(), os_NetworkSpeed, 170, 307, 130,17);
	cFloatingOpt_System.Add( new CCombobox(), os_ScreenshotFormat, 365, 383, 70,17);
	//cFloatingOpt_System.Add( new CCombobox(), os_ColourDepth, 275, 170, 145, 17);

	// Set the values
	CSlider *s = (CSlider *)cFloatingOpt_System.getWidget(os_SoundVolume);
	s->setValue( tLXOptions->iSoundVolume );

	CTextbox *t = (CTextbox *)cFloatingOpt_System.getWidget(os_NetworkPort);
	t->setText( itoa(tLXOptions->iNetworkPort) );
	t = (CTextbox *)(cFloatingOpt_System.getWidget(os_MaxFPS));
	t->setText(itoa(tLXOptions->nMaxFPS));
	t = (CTextbox *)(cFloatingOpt_System.getWidget(os_HttpProxy));
	t->setText(tLXOptions->sHttpProxy);

	// Network speed
	for(i=0; i<3; i++)
		cFloatingOpt_System.SendMessage(os_NetworkSpeed, CBS_ADDITEM, sFloatingOpt_NetworkSpeeds[i], i);

	cFloatingOpt_System.SendMessage(os_NetworkSpeed, CBM_SETCURSEL, tLXOptions->iNetworkSpeed, 0);
	cFloatingOpt_System.SendMessage(os_NetworkSpeed, CBM_SETCURINDEX, tLXOptions->iNetworkSpeed, 0);

	// Screenshot format
	cFloatingOpt_System.SendMessage(os_ScreenshotFormat, CBS_ADDITEM, "Bmp", FMT_BMP);
	cFloatingOpt_System.SendMessage(os_ScreenshotFormat, CBS_ADDITEM, "Png", FMT_PNG);
	cFloatingOpt_System.SendMessage(os_ScreenshotFormat, CBS_ADDITEM, "Gif", FMT_GIF);
	cFloatingOpt_System.SendMessage(os_ScreenshotFormat, CBS_ADDITEM, "Jpg", FMT_JPG);

	cFloatingOpt_System.SendMessage(os_ScreenshotFormat, CBM_SETCURSEL, tLXOptions->iScreenshotFormat, 0);
	cFloatingOpt_System.SendMessage(os_ScreenshotFormat, CBM_SETCURINDEX, tLXOptions->iScreenshotFormat, 0);

	/*
	// Color depth
	cFloatingOpt_System.SendMessage(os_ColourDepth, CBS_ADDITEM, "Automatic", 0);
	cFloatingOpt_System.SendMessage(os_ColourDepth, CBS_ADDITEM, "High Color (16 bit)", 1);
	cFloatingOpt_System.SendMessage(os_ColourDepth, CBS_ADDITEM, "True Color (24 bit)", 2);
	cFloatingOpt_System.SendMessage(os_ColourDepth, CBS_ADDITEM, "True Color (32 bit)", 3);
	
	switch (tLXOptions->iColourDepth) {
	case 0:  // Automatic
		cFloatingOpt_System.SendMessage(os_ColourDepth, CBM_SETCURSEL, (DWORD)0, 0);
		cFloatingOpt_System.SendMessage(os_ColourDepth, CBM_SETCURINDEX, (DWORD)0, 0);
		break;
	case 16:  // 16 bit
		cFloatingOpt_System.SendMessage(os_ColourDepth, CBM_SETCURSEL, (DWORD)1, 0);
		cFloatingOpt_System.SendMessage(os_ColourDepth, CBM_SETCURINDEX, (DWORD)1, 0);
		break;
	case 24:  // 24 bit
		cFloatingOpt_System.SendMessage(os_ColourDepth, CBM_SETCURSEL, (DWORD)2, 0);
		cFloatingOpt_System.SendMessage(os_ColourDepth, CBM_SETCURINDEX, (DWORD)2, 0);
		break;
	case 32:  // 32 bit
		cFloatingOpt_System.SendMessage(os_ColourDepth, CBM_SETCURSEL, (DWORD)3, 0);
		cFloatingOpt_System.SendMessage(os_ColourDepth, CBM_SETCURINDEX, (DWORD)3, 0);
		break;
	}
	*/


	// Disable apply for now
	cFloatingOpt_System.getWidget(os_Apply)->setEnabled(false);


	// Game
	cFloatingOpt_Game.Add( new CLabel("Blood Amount",tLX->clNormalLabel),       Static, 40, 150, 0,0);
	cFloatingOpt_Game.Add( new CSlider(5000),                       og_BloodAmount, 175, 147, 210, 20);
	cFloatingOpt_Game.Add( new CLabel("Shadows",tLX->clNormalLabel),            Static, 40, 180, 0,0);
	cFloatingOpt_Game.Add( new CCheckbox(tLXOptions->bShadows),     og_Shadows, 280, 180, 17,17);
	cFloatingOpt_Game.Add( new CLabel("Particles",tLX->clNormalLabel),          Static, 40, 210, 0,0);
	cFloatingOpt_Game.Add( new CCheckbox(tLXOptions->bParticles),   og_Particles, 280, 210, 17,17);
	cFloatingOpt_Game.Add( new CLabel("Classic Rope throw",tLX->clNormalLabel), Static, 40, 240, 0,0);
	cFloatingOpt_Game.Add( new CCheckbox(tLXOptions->bOldSkoolRope),og_OldSkoolRope, 280, 240, 17,17);
	//cFloatingOpt_Game.Add( new CLabel("Show worm's health",tLX->clNormalLabel), Static, 40, 270, 0,0);
	//cFloatingOpt_Game.Add( new CCheckbox(tLXOptions->bShowHealth),  og_ShowWormHealth, 280, 270, 17,17);
	cFloatingOpt_Game.Add( new CLabel("Colorize nicks by teams",tLX->clNormalLabel), Static, 40, 270, 0,0);
	cFloatingOpt_Game.Add( new CCheckbox(tLXOptions->bColorizeNicks),og_ColorizeNicks, 280, 270, 17,17);
	cFloatingOpt_Game.Add( new CLabel("Start typing after any key press",tLX->clNormalLabel), Static, 40, 300, 0,0);
	cFloatingOpt_Game.Add( new CCheckbox(tLXOptions->bAutoTyping),og_AutoTyping, 280, 300, 17,17);
	cFloatingOpt_Game.Add( new CLabel("Use antialiasing (slow)",tLX->clNormalLabel), Static, 40, 330, 0,0);
	cFloatingOpt_Game.Add( new CCheckbox(tLXOptions->bAntiAliasing),og_Antialiasing, 280, 330, 17,17);
	//cFloatingOpt_Game.Add( new CLabel("AI Difficulty",tLX->clNormalLabel), Static, 40, 270, 0,0);
	//cFloatingOpt_Game.Add( new CSlider(3), og_AIDifficulty,   175, 267, 100, 20);
	cFloatingOpt_Game.Add( new CLabel("Enable mouse control (Player 1)",tLX->clNormalLabel), Static, 40, 360, 0,0);
	cFloatingOpt_Game.Add( new CCheckbox(tLXOptions->bMouseAiming),og_MouseAiming, 280, 360, 17,17);
	cFloatingOpt_Game.Add( new CLabel("Log my game results",tLX->clNormalLabel), Static, 40, 390, 0,0);
	cFloatingOpt_Game.Add( new CCheckbox(tLXOptions->tGameinfo.bMatchLogging),og_MatchLogging, 280, 390, 17,17);

	cFloatingOpt_Game.Add( new CLabel("Network antilag prediction",tLX->clNormalLabel), Static, 330, 330, 0,0);
	cFloatingOpt_Game.Add( new CCheckbox(tLXOptions->bAntilagMovementPrediction),og_AntilagMovementPrediction, 550, 330, 17,17);

	cFloatingOpt_Game.Add( new CLabel("Allow mouse control (Server)",tLX->clNormalLabel), Static, 330, 360, 0,0);
	cFloatingOpt_Game.Add( new CCheckbox(tLXOptions->bAllowMouseAiming),og_AllowMouseAiming, 550, 360, 17,17);

	cFloatingOpt_Game.Add( new CLabel("Allow strafing (Server)",tLX->clNormalLabel), Static, 330, 390, 0,0);
	cFloatingOpt_Game.Add( new CCheckbox(&tLXOptions->bAllowStrafing), Static, 550, 390, 17,17);

	// TODO: Fix cSlider so it's value thing doesn't take up a square of 100x100 pixels.

	// Set the values
	cFloatingOpt_Game.SendMessage( og_BloodAmount,  SLM_SETVALUE, tLXOptions->iBloodAmount, 0);
	//cFloatingOpt_Game.SendMessage( og_AIDifficulty, SLM_SETVALUE, tLXOptions->iAIDifficulty, 0);


	return true;
}



///////////////////
// Options main frame
void Menu_FloatingOptionsFrame(void)
{
	mouse_t		*Mouse = GetMouse();
	gui_event_t *ev = NULL;
	int			val;

	CCheckbox	*c;
	//CSlider		*s;

	//DrawImageAdv(VideoPostProcessor::videoSurface(), tMenu->bmpBuffer,  180,110,  180,110,  300,30);
	//DrawImageAdv(VideoPostProcessor::videoSurface(), tMenu->bmpBuffer, 20,140, 20,140, 620,340);

	// Redraw background image
	DrawImage(VideoPostProcessor::videoSurface(), tMenu->bmpMainBack_common,0,0);
	if (tMenu->tFrontendInfo.bPageBoxes)
		Menu_DrawBox(VideoPostProcessor::videoSurface(), 15, 130, 625, 465);
	Menu_DrawSubTitle(VideoPostProcessor::videoSurface(), SUB_OPTIONS);

	// Process the top buttons
	cFloatingOpt_TopButtons[iFloatingOptionsMode].MouseOver(Mouse);
	SetGameCursor(CURSOR_ARROW); // Hack: button changed the cursor to hand, we need to change it back
	for(int i=op_Controls;i<=op_System;i++) {

		cFloatingOpt_TopButtons[i].Draw(VideoPostProcessor::videoSurface());

		if(i==iFloatingOptionsMode)
			continue;

		if(cFloatingOpt_TopButtons[i].InBox(Mouse->X,Mouse->Y)) {
			cFloatingOpt_TopButtons[i].MouseOver(Mouse);
			if(Mouse->Up) {
                DrawImageAdv(VideoPostProcessor::videoSurface(), tMenu->bmpBuffer, 20,140, 20,140, 620,340);
				iFloatingOptionsMode = i;
				PlaySoundSample(sfxGeneral.smpClick);
			}
		}
	}



	// Process the gui layout
#ifdef WITH_MEDIAPLAYER
	if (!cMediaPlayer.GetDrawPlayer())
#endif
	ev = cFloatingOptions.Process();
	cFloatingOptions.Draw(VideoPostProcessor::videoSurface());

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


	if(iFloatingOptionsMode == 0) {

		// Controls
#ifdef WITH_MEDIAPLAYER
		if (!cMediaPlayer.GetDrawPlayer())
#endif
			ev = cFloatingOpt_Controls.Process();
		cFloatingOpt_Controls.Draw(VideoPostProcessor::videoSurface());

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
					Menu_FloatingOptionsWaitInput(ply, b->getName(), b);
					
					SetupGameInputs();

				}
			}
		}
	}


	if(iFloatingOptionsMode == 1) {

		// Game
#ifdef WITH_MEDIAPLAYER
		if (!cMediaPlayer.GetDrawPlayer())
#endif
			ev = cFloatingOpt_Game.Process();
		cFloatingOpt_Game.Draw(VideoPostProcessor::videoSurface());

		val = cFloatingOpt_Game.SendMessage(og_BloodAmount, SLM_GETVALUE, (DWORD)0, 0);
		//s = (CSlider *)cFloatingOpt_Game.getWidget(og_BloodAmount);
        DrawImageAdv(VideoPostProcessor::videoSurface(), tMenu->bmpBuffer, 385,140, 385,140, 70,40);
		tLX->cFont.Draw(VideoPostProcessor::videoSurface(),385, 148, tLX->clNormalLabel, itoa(val)+"%");

		//val = cFloatingOpt_Game.SendMessage(og_AIDifficulty, SLM_GETVALUE, 0, 0);
        //DrawImageAdv(VideoPostProcessor::videoSurface(), tMenu->bmpBuffer, 285,260, 285,260, 100,50);
		//tLX->cFont.Draw(VideoPostProcessor::videoSurface(),285, 268, tLX->clNormalLabel,Difficulties[val]);



		if(ev) {

			switch(ev->iControlID) {

				// Blood amount
				case og_BloodAmount:
					if(ev->iEventMsg == SLD_CHANGE) {
						val = cFloatingOpt_Game.SendMessage(og_BloodAmount, SLM_GETVALUE, (DWORD)0, 0);
						tLXOptions->iBloodAmount = val;
					}
					break;

				// Shadows
				case og_Shadows:
					if(ev->iEventMsg == CHK_CHANGED) {
						c = (CCheckbox *)cFloatingOpt_Game.getWidget(og_Shadows);
						tLXOptions->bShadows = c->getValue();
					}
					break;

				// Particles
				case og_Particles:
					if(ev->iEventMsg == CHK_CHANGED) {
						c = (CCheckbox *)cFloatingOpt_Game.getWidget(og_Particles);
						tLXOptions->bParticles = c->getValue();
					}
					break;

				// AI Difficulty
				/*case og_AIDifficulty:
					if(ev->iEventMsg == SLD_CHANGE) {
						val = cFloatingOpt_Game.SendMessage(og_AIDifficulty, SLM_GETVALUE, 0, 0);
						tLXOptions->iAIDifficulty = val;
					}
					break;*/

				// Old skool rope throw
				case og_OldSkoolRope:
					if(ev->iEventMsg == CHK_CHANGED) {
						tLXOptions->bOldSkoolRope = cFloatingOpt_Game.SendMessage(og_OldSkoolRope,CKM_GETCHECK,(DWORD)0,0) != 0;
					}
					break;

				// Show the worm's health below name
/*				case og_ShowWormHealth:
					if(ev->iEventMsg == CHK_CHANGED)
						tLXOptions->iShowHealth = cFloatingOpt_Game.SendMessage(og_ShowWormHealth, CKM_GETCHECK, (DWORD)0, 0);
					break;*/

				// TDM nick colorizing
				case og_ColorizeNicks:
					if(ev->iEventMsg == CHK_CHANGED)
						tLXOptions->bColorizeNicks = cFloatingOpt_Game.SendMessage(og_ColorizeNicks, CKM_GETCHECK, (DWORD)0, 0) != 0;
					break;

				// Auto typing
				case og_AutoTyping:
					if(ev->iEventMsg == CHK_CHANGED)
						tLXOptions->bAutoTyping = cFloatingOpt_Game.SendMessage(og_AutoTyping, CKM_GETCHECK, (DWORD)0, 0) != 0;
					break;

				// Antialiasing
				case og_Antialiasing:
					if(ev->iEventMsg == CHK_CHANGED)
						tLXOptions->bAntiAliasing = cFloatingOpt_Game.SendMessage(og_Antialiasing, CKM_GETCHECK, (DWORD)0, 0) != 0;
					break;

				// Mouse aiming
				case og_MouseAiming:
					if(ev->iEventMsg == CHK_CHANGED)
						tLXOptions->bMouseAiming = cFloatingOpt_Game.SendMessage(og_MouseAiming, CKM_GETCHECK, (DWORD)0, 0) != 0;
					break;

				case og_AllowMouseAiming:
					if(ev->iEventMsg == CHK_CHANGED)
						tLXOptions->bAllowMouseAiming = cFloatingOpt_Game.SendMessage(og_AllowMouseAiming, CKM_GETCHECK, (DWORD)0, 0) != 0;
					break;

				// Match logging
				case og_MatchLogging:
					if(ev->iEventMsg == CHK_CHANGED)
						tLXOptions->tGameinfo.bMatchLogging = cFloatingOpt_Game.SendMessage(og_MatchLogging, CKM_GETCHECK, (DWORD)0, 0) != 0;
					break;

				case og_AntilagMovementPrediction:
					if(ev->iEventMsg == CHK_CHANGED)
						tLXOptions->bAntilagMovementPrediction = cFloatingOpt_Game.SendMessage(og_AntilagMovementPrediction, CKM_GETCHECK, (DWORD)0, 0) != 0;
					break;
			}
		}

	}


	// Process the different pages
	if(iFloatingOptionsMode == 2) {

		// Fullscreen value
		c = (CCheckbox *)cFloatingOpt_System.getWidget(os_Fullscreen);
		bool fullscr = c->getValue();


		// System
#ifdef WITH_MEDIAPLAYER
		if (!cMediaPlayer.GetDrawPlayer())
#endif
			ev = cFloatingOpt_System.Process();
		cFloatingOpt_System.Draw(VideoPostProcessor::videoSurface());

		if(ev) {

			switch(ev->iControlID) {

				// Apply
				case os_Apply:
					if(ev->iEventMsg == BTN_MOUSEUP) {

						// Set to fullscreen
						tLXOptions->bFullscreen = fullscr;

						CTextbox *t = (CTextbox *)cFloatingOpt_System.getWidget(os_MaxFPS);
						bool fail = false;
						tLXOptions->nMaxFPS = from_string<int>(t->getText(), fail);
						tLXOptions->nMaxFPS = fail ? 0 : MAX(0, tLXOptions->nMaxFPS);
						t->setText(itoa(tLXOptions->nMaxFPS));
						PlaySoundSample(sfxGeneral.smpClick);

						// Set the new video mode
						SetVideoMode();
						Menu_RedrawMouse(true);
						SDL_ShowCursor(SDL_DISABLE);
						
					}
					break;

				// Sound on/off
				case os_SoundOn:
					if(ev->iEventMsg == CHK_CHANGED) {

						bool old = tLXOptions->bSoundOn;

						c = (CCheckbox *)cFloatingOpt_System.getWidget(os_SoundOn);
						tLXOptions->bSoundOn = c->getValue();

						if(old != tLXOptions->bSoundOn) {
							if(tLXOptions->bSoundOn)
								StartSoundSystem();
							else
								StopSoundSystem();
						}
					}
					break;

				// Sound volume
				case os_SoundVolume:
					if(ev->iEventMsg == SLD_CHANGE) {
						CSlider *s = (CSlider *)cFloatingOpt_System.getWidget(os_SoundVolume);
						tLXOptions->iSoundVolume = s->getValue();

						SetSoundVolume( tLXOptions->iSoundVolume );
					}
					break;

				// Show FPS
				case os_ShowFPS:
					if(ev->iEventMsg == CHK_CHANGED)
						tLXOptions->bShowFPS = cFloatingOpt_System.SendMessage(os_ShowFPS, CKM_GETCHECK, (DWORD)0, 0) != 0;
					break;

				// Logging
				case os_LogConvos:
					if(ev->iEventMsg == CHK_CHANGED)  {
						tLXOptions->bLogConvos = cFloatingOpt_System.SendMessage(os_LogConvos, CKM_GETCHECK, (DWORD)0, 0) != 0;
						FILE *f;

						f = OpenGameFile("Conversations.log","a");
						if (f)  {
							if (tLXOptions->bLogConvos)  {
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
						tLXOptions->bShowPing = cFloatingOpt_System.SendMessage(os_ShowPing, CKM_GETCHECK, (DWORD)0, 0) != 0;
					break;

				// Use Ip To Country
				case os_UseIpToCountry:
					if(ev->iEventMsg == CHK_CHANGED)
						tLXOptions->bUseIpToCountry = cFloatingOpt_System.SendMessage(os_UseIpToCountry, CKM_GETCHECK, (DWORD)0, 0) != 0;
					break;

				// Load Database at Startup
				case os_LoadDbAtStartup:
					if(ev->iEventMsg == CHK_CHANGED)
						tLXOptions->bLoadDbAtStartup = cFloatingOpt_System.SendMessage(os_LoadDbAtStartup, CKM_GETCHECK, (DWORD)0, 0) != 0;
					break;

				case os_NatTraverse:
					if(ev->iEventMsg == CHK_CHANGED)
						tLXOptions->bNatTraverse = cFloatingOpt_System.SendMessage(os_NatTraverse, CKM_GETCHECK, (DWORD)0, 0) != 0;
			}
		}


		// Get the values
		CTextbox *t = (CTextbox *)cFloatingOpt_System.getWidget(os_NetworkPort);
		tLXOptions->iNetworkPort = atoi(t->getText());
		t = (CTextbox *)cFloatingOpt_System.getWidget(os_HttpProxy);
		tLXOptions->sHttpProxy = t->getText();

		tLXOptions->iNetworkSpeed = cFloatingOpt_System.SendMessage(os_NetworkSpeed, CBM_GETCURINDEX,(DWORD)0,0);
		tLXOptions->iScreenshotFormat = cFloatingOpt_System.SendMessage(os_ScreenshotFormat, CBM_GETCURINDEX,(DWORD)0,0);

		// FPS and fullscreen
		t = (CTextbox *)cFloatingOpt_System.getWidget(os_MaxFPS);

		if(fullscr != tLXOptions->bFullscreen || atoi(t->getText()) != tLXOptions->nMaxFPS) {
			cFloatingOpt_System.getWidget(os_Apply)->setEnabled(true);
			cFloatingOpt_System.getWidget(os_Apply)->Draw( VideoPostProcessor::videoSurface() );
        } else {
			cFloatingOpt_System.getWidget(os_Apply)->setEnabled(false);
			cFloatingOpt_System.getWidget(os_Apply)->redrawBuffer();
        }
	}


	// Draw the mouse
	DrawCursor(VideoPostProcessor::videoSurface());
}


///////////////////
// Process an input box waiting thing
// ply=-1 : general ; ply>=0 : normal player
void Menu_FloatingOptionsWaitInput(int ply, const std::string& name, CInputbox *b)
{
	keyboard_t *kb = GetKeyboard();
	mouse_t *Mouse = GetMouse();

	// Draw the back buffer
	cFloatingOptions.Draw(tMenu->bmpBuffer.get());
	cFloatingOpt_Controls.Draw(tMenu->bmpBuffer.get());

	Menu_DrawBox(tMenu->bmpBuffer.get(), 210, 170, 430, 310);
	//DrawImageAdv(tMenu->bmpBuffer, tMenu->bmpMainBack, 212,172, 212,172, 217,137);
    DrawRectFill(tMenu->bmpBuffer.get(), 212,172,429,309,tLX->clDialogBackground);

	tLX->cFont.DrawCentre(tMenu->bmpBuffer.get(),320,180,MakeColour(128,200,255),"Input for:");
	tLX->cFont.DrawCentre(tMenu->bmpBuffer.get(),320,205,MakeColour(255,255,255),name);

	tLX->cFont.DrawCentre(tMenu->bmpBuffer.get(),320,270,MakeColour(255,255,255),"Press any key/mouse");
	tLX->cFont.DrawCentre(tMenu->bmpBuffer.get(),320,285,MakeColour(128,128,128),"(Escape to cancel)");

	cFloatingOpt_TopButtons[iFloatingOptionsMode].MouseOver(Mouse);
	for(ushort i=0;i<3;i++) {
		cFloatingOpt_TopButtons[i].Draw(tMenu->bmpBuffer.get());
	}


	Menu_RedrawMouse(true);

	Mouse->Up = 0;
	Mouse->Down = 0;

	ProcessEvents();
	SetGameCursor(CURSOR_ARROW);
	CInput::InitJoysticksTemp();
	ProcessEvents(); // drop all current events in queue
	while(!tLX->bQuitGame) {
		Menu_RedrawMouse(false);

		DrawCursor(VideoPostProcessor::videoSurface());

		// Escape quits the wait for user input
		// TODO: make this event-based (don't check GetKeyboard() directly)
		if(kb->KeyUp[SDLK_ESCAPE])
			break;

		std::string tmp;
		if(CInput::Wait(tmp)) {
			b->setText(tmp);
			break;
		}

		VideoPostProcessor::process();
		CapFPS();
		WaitForNextEvent();
	}
	CInput::UnInitJoysticksTemp();

	// Change the options
	if(ply >= 0) {
		tLXOptions->sPlayerControls[ply][b->getValue()] = b->getText();
	} else
		tLXOptions->sGeneralControls[b->getValue()] = b->getText();

	// Disable quick weapon selection keys if they collide with other keys
	for( int ply1 = 0; ply1 < 2; ply1 ++ )
	{
		for( int key1 = SIN_WEAPON1; key1 <= SIN_WEAPON5; key1 ++ )
		{
			for( int ply2 = 0; ply2 < 2; ply2 ++ )
				for( int key2 = SIN_UP; key2 < SIN_WEAPON1; key2 ++ )
					if( tLXOptions->sPlayerControls[ply1][key1] ==
						tLXOptions->sPlayerControls[ply2][key2] )
						tLXOptions->sPlayerControls[ply1][key1] = "";
			int lastkey = SIN_TEAMCHAT;
#ifdef WITH_MEDIAPLAYER
			lastkey = SIN_MEDIAPLAYER;
#endif
			for( int key2 = SIN_CHAT; key2 < lastkey; key2 ++ )
				if( tLXOptions->sPlayerControls[ply1][key1] ==
					tLXOptions->sGeneralControls[key2] )
					tLXOptions->sPlayerControls[ply1][key1] = "";
		};
	};


	Mouse->Down = 0;
	Mouse->Up = 0;

	DrawImage(tMenu->bmpBuffer.get(),tMenu->bmpMainBack_common,0,0);
	if (tMenu->tFrontendInfo.bPageBoxes)
		Menu_DrawBox(tMenu->bmpBuffer.get(), 15,130, 625, 465);
	Menu_DrawSubTitle(tMenu->bmpBuffer.get(),SUB_OPTIONS);

	Menu_RedrawMouse(true);
}


///////////////////
// Shutdown the options menu
void Menu_FloatingOptionsShutdown(void)
{
	cFloatingOptions.Shutdown();
	cFloatingOpt_Controls.Shutdown();
	cFloatingOpt_System.Shutdown();
	cFloatingOpt_Game.Shutdown();
}

}; // namespace DeprecatedGUI
