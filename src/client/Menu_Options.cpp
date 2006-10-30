/////////////////////////////////////////
//
//             Liero Xtreme
//
//     Copyright Auxiliary Software 2002
//
//
/////////////////////////////////////////


// Options
// Created 30/7/02
// Jason Boettcher


#include "defs.h"
#include "LieroX.h"
#include "Menu.h"


int OptionsMode = 0;
CGuiLayout	cOptions;
CGuiLayout	cOpt_Controls;
CGuiLayout	cOpt_System;
CGuiLayout	cOpt_Game;
CButton		TopButtons[3];

// Control id's
enum {
	Static = -1,
	Back = 0,
	Fullscreen,
	SoundOn,
	SoundVolume,
	NetworkPort,
	NetworkSpeed,
	ShowFPS,
	ShowPing,
	Filtered,
	LogConvos,
	Apply,

	BloodAmount,
	Shadows,
	Particles,
	OldSkoolRope,
	AIDifficulty,
	ShowWormHealth,
	ColorizeNicks,
	AutoTyping,
	ScreenshotFormat,

	Ply1_Up,
	Ply1_Down,
	Ply1_Left,
	Ply1_Right,
	Ply1_Shoot,
	Ply1_Jump,
	Ply1_Selweapon,
	Ply1_Rope,

	Ply2_Up,
	Ply2_Down,
	Ply2_Left,
	Ply2_Right,
	Ply2_Shoot,
	Ply2_Jump,
	Ply2_Selweapon,
	Ply2_Rope,

	Gen_Chat,
    Gen_Score,
	Gen_Health,
	Gen_CurSettings,
	Gen_TakeScreenshot,
	Gen_ViewportManager,
	Gen_SwitchMode
};


char *InputNames[] = {
	"Up",
	"Down",
	"Left",
	"Right",
	"Shoot",
	"Jump",
	"Select Weapon",
	"Ninja Rope"
};

char *NetworkSpeeds[] = {
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
	Uint32 blue = MakeColour(0,138,251);
	Uint32 ltblue = MakeColour(143,176,207);
    int i;

	// Create the buffer
	DrawImage(tMenu->bmpBuffer,tMenu->bmpMainBack_wob,0,0);
    Menu_DrawBox(tMenu->bmpBuffer, 15,130, 625, 465);
	Menu_DrawSubTitle(tMenu->bmpBuffer,SUB_OPTIONS);

	Menu_RedrawMouse(true);

	// Setup the top buttons
	TopButtons[0] = CButton(BUT_CONTROLS,	tMenu->bmpButtons);
	TopButtons[1] = CButton(BUT_GAME,		tMenu->bmpButtons);
	TopButtons[2] = CButton(BUT_SYSTEM,		tMenu->bmpButtons);

	TopButtons[0].Setup(0, 180, 110, 100, 15);
	TopButtons[1].Setup(1, 310, 110, 50, 15);
	TopButtons[2].Setup(2, 390, 110, 70, 15);
    for(i=0; i<3; i++)
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
	cOptions.Add( new CButton(BUT_BACK, tMenu->bmpButtons), Back, 25,440, 50,15);


	// Controls
	cOpt_Controls.Add( new CLabel("Player Controls", blue), Static, 40,  150, 0,0);
	cOpt_Controls.Add( new CLabel("Player 1",ltblue),       Static, 163, 170, 0,0);
	cOpt_Controls.Add( new CLabel("Player 2",ltblue),       Static, 268, 170, 0,0);
	cOpt_Controls.Add( new CLabel("General Controls", blue),Static, 380, 150, 0,0);

	int y = 190;
	for(i=0;i<8;i++,y+=25) {
		cOpt_Controls.Add( new CLabel(InputNames[i],0xffff), Static, 40, y, 0,0);

		cOpt_Controls.Add( new CInputbox(SIN_UP+i, tLXOptions->sPlayer1Controls[SIN_UP+i], tMenu->bmpInputbox, InputNames[i]),
			               Ply1_Up+i, 165, y, 50,17);

		cOpt_Controls.Add( new CInputbox(SIN_UP+i, tLXOptions->sPlayer2Controls[SIN_UP+i], tMenu->bmpInputbox, InputNames[i]),
			               Ply2_Up+i, 270, y, 50,17);
	}

	// General Controls
	cOpt_Controls.Add( new CLabel("Chat", 0xffff), Static, 380, 190, 0,0);
	cOpt_Controls.Add( new CInputbox(SIN_CHAT, tLXOptions->sGeneralControls[SIN_CHAT], tMenu->bmpInputbox, "Chat"),
						   Gen_Chat, 515, 190, 50,17);

    cOpt_Controls.Add( new CLabel("Scoreboard", 0xffff), Static, 380, 215, 0,0);
	cOpt_Controls.Add( new CInputbox(SIN_SCORE, tLXOptions->sGeneralControls[SIN_SCORE], tMenu->bmpInputbox, "Scoreboard"),
						   Gen_Score, 515, 215, 50,17);

    cOpt_Controls.Add( new CLabel("Health bar", 0xffff), Static, 380, 240, 0,0);
	cOpt_Controls.Add( new CInputbox(SIN_HEALTH, tLXOptions->sGeneralControls[SIN_HEALTH], tMenu->bmpInputbox, "Health bar"),
						   Gen_Health, 515, 240, 50,17);

    cOpt_Controls.Add( new CLabel("Current settings", 0xffff), Static, 380, 265, 0,0);
	cOpt_Controls.Add( new CInputbox(SIN_SETTINGS, tLXOptions->sGeneralControls[SIN_SETTINGS], tMenu->bmpInputbox, "Current settings"),
						   Gen_CurSettings, 515, 265, 50,17);

    cOpt_Controls.Add( new CLabel("Take screenshot", 0xffff), Static, 380, 290, 0,0);
	cOpt_Controls.Add( new CInputbox(SIN_SCREENSHOTS, tLXOptions->sGeneralControls[SIN_SCREENSHOTS], tMenu->bmpInputbox, "Take Screenshot"),
						   Gen_TakeScreenshot, 515, 290, 50,17);

    cOpt_Controls.Add( new CLabel("Viewport manager", 0xffff), Static, 380, 315, 0,0);
	cOpt_Controls.Add( new CInputbox(SIN_VIEWPORTS, tLXOptions->sGeneralControls[SIN_VIEWPORTS], tMenu->bmpInputbox, "Viewport Manager"),
						   Gen_ViewportManager, 515, 315, 50,17);

    cOpt_Controls.Add( new CLabel("Switch video mode", 0xffff), Static, 380, 340, 0,0);
	cOpt_Controls.Add( new CInputbox(SIN_SWITCHMODE, tLXOptions->sGeneralControls[SIN_SWITCHMODE], tMenu->bmpInputbox, "Switch video mode"),
						   Gen_SwitchMode, 515, 340, 50,17);
	



	// System	
	cOpt_System.Add( new CLabel("Video",blue),              Static, 40, 150, 0,0);
	cOpt_System.Add( new CLabel("Fullscreen",0xffff),       Static, 60, 170, 0,0);
	cOpt_System.Add( new CCheckbox(tLXOptions->iFullscreen),Fullscreen, 170, 170, 17,17);

	cOpt_System.Add( new CLabel("Audio",blue),              Static, 40, 205, 0,0);
	cOpt_System.Add( new CLabel("Sound on",0xffff),         Static, 60, 225, 0,0);
	cOpt_System.Add( new CCheckbox(tLXOptions->iSoundOn),   SoundOn, 170, 225, 17,17);
	cOpt_System.Add( new CLabel("Sound volume",0xffff),     Static, 60, 245, 0,0);
	cOpt_System.Add( new CSlider(100),                      SoundVolume, 165, 242, 110, 20);

	cOpt_System.Add( new CLabel("Network",blue),            Static, 40, 280, 0,0);
	cOpt_System.Add( new CLabel("Network port",0xffff),     Static, 60, 300, 0,0);
	cOpt_System.Add( new CTextbox(),                        NetworkPort, 170, 297, 100,20);
	cOpt_System.Add( new CLabel("Network speed",0xffff),    Static, 60,330, 0,0);	
	
	cOpt_System.Add( new CLabel("Miscellanous",blue),       Static, 40, 365, 0,0);
	cOpt_System.Add( new CLabel("Show FPS",0xffff),         Static, 60, 385, 0,0);
	cOpt_System.Add( new CCheckbox(tLXOptions->iShowFPS),   ShowFPS, 170, 385, 17,17);
	cOpt_System.Add( new CLabel("Filtered level",0xffff),   Static, 60, 415, 0,0);
	cOpt_System.Add( new CCheckbox(tLXOptions->iFiltered),  Filtered, 170, 415, 17,17);
	cOpt_System.Add( new CLabel("Show ping",0xffff),		Static, 215, 385, 0,0);
	cOpt_System.Add( new CCheckbox(tLXOptions->iShowPing),  ShowPing, 355,385,17,17);
	cOpt_System.Add( new CLabel("Log Conversations",0xffff),Static, 215, 415, 0,0);
	cOpt_System.Add( new CCheckbox(tLXOptions->iLogConvos), LogConvos, 355,415,17,17);
	cOpt_System.Add( new CLabel("Screenshot format",0xffff),Static, 395,385, 0,0);	

	cOpt_System.SendMessage(NetworkPort,TXM_SETMAX,8,0);

	cOpt_System.Add( new CButton(BUT_APPLY, tMenu->bmpButtons), Apply, 555,440, 60,15);

	// Put the combo box after the other widgets to get around the problem with widget layering
	cOpt_System.Add( new CCombobox(), NetworkSpeed, 170, 327, 130,17);
	cOpt_System.Add( new CCombobox(), ScreenshotFormat, 530, 383, 70,17);

	// Set the values
	CSlider *s = (CSlider *)cOpt_System.getWidget(SoundVolume);
	s->setValue( tLXOptions->iSoundVolume );

	CTextbox *t = (CTextbox *)cOpt_System.getWidget(NetworkPort);
	char buf[64];
	sprintf(buf,"%d",tLXOptions->iNetworkPort);
	t->setText( buf );

	// Network speed
	for(i=0; i<3; i++)
		cOpt_System.SendMessage(NetworkSpeed, CBM_ADDITEM, i, (DWORD)NetworkSpeeds[i]);

	cOpt_System.SendMessage(NetworkSpeed, CBM_SETCURSEL, tLXOptions->iNetworkSpeed, 0);

	// Screenshot format
	cOpt_System.SendMessage(ScreenshotFormat, CBM_ADDITEM, FMT_BMP, (DWORD)"Bmp");
	cOpt_System.SendMessage(ScreenshotFormat, CBM_ADDITEM, FMT_PNG, (DWORD)"Png");
	cOpt_System.SendMessage(ScreenshotFormat, CBM_ADDITEM, FMT_GIF, (DWORD)"Gif");
	cOpt_System.SendMessage(ScreenshotFormat, CBM_ADDITEM, FMT_JPG, (DWORD)"Jpg");

	cOpt_System.SendMessage(ScreenshotFormat, CBM_SETCURSEL, tLXOptions->iScreenshotFormat, 0);


	// Disable apply for now
	cOpt_System.getWidget(Apply)->setEnabled(false);


	// Game
	cOpt_Game.Add( new CLabel("Blood Amount",0xffff),       Static, 40, 150, 0,0);
	cOpt_Game.Add( new CSlider(5000),                       BloodAmount, 175, 147, 210, 20);
	cOpt_Game.Add( new CLabel("Shadows",0xffff),            Static, 40, 180, 0,0);
	cOpt_Game.Add( new CCheckbox(tLXOptions->iShadows),     Shadows, 280, 180, 17,17);
	cOpt_Game.Add( new CLabel("Particles",0xffff),          Static, 40, 210, 0,0);
	cOpt_Game.Add( new CCheckbox(tLXOptions->iParticles),   Particles, 280, 210, 17,17);
	cOpt_Game.Add( new CLabel("Classic Rope throw",0xffff), Static, 40, 240, 0,0);
	cOpt_Game.Add( new CCheckbox(tLXOptions->iOldSkoolRope),OldSkoolRope, 280, 240, 17,17);
	//cOpt_Game.Add( new CLabel("Show worm's health",0xffff), Static, 40, 270, 0,0);
	//cOpt_Game.Add( new CCheckbox(tLXOptions->iShowHealth),  ShowWormHealth, 280, 270, 17,17);
	cOpt_Game.Add( new CLabel("Colorize nicks by teams",0xffff), Static, 40, 270, 0,0);
	cOpt_Game.Add( new CCheckbox(tLXOptions->iColorizeNicks),ColorizeNicks, 280, 270, 17,17);
	cOpt_Game.Add( new CLabel("Start typing after any key press",0xffff), Static, 40, 300, 0,0);
	cOpt_Game.Add( new CCheckbox(tLXOptions->iAutoTyping),AutoTyping, 280, 300, 17,17);
	//cOpt_Game.Add( new CLabel("AI Difficulty",0xffff), Static, 40, 270, 0,0);
	//cOpt_Game.Add( new CSlider(3), AIDifficulty,   175, 267, 100, 20);

	// Set the values	
	cOpt_Game.SendMessage( BloodAmount,  SLM_SETVALUE, tLXOptions->iBloodAmount, 0);
	//cOpt_Game.SendMessage( AIDifficulty, SLM_SETVALUE, tLXOptions->iAIDifficulty, 0);

	


	
	return true;
}


///////////////////
// Options main frame
void Menu_OptionsFrame(void)
{
	mouse_t		*Mouse = GetMouse();
	int			mouse = 0;
	gui_event_t *ev;
	int fullscr = tLXOptions->iFullscreen;
	char		*Difficulties[] = {"Easy", "Medium", "Hard", "Xtreme"};
	int			val;

	CCheckbox	*c;
	//CSlider		*s;

	//DrawImageAdv(tMenu->bmpScreen, tMenu->bmpBuffer,  180,110,  180,110,  300,30);
	//DrawImageAdv(tMenu->bmpScreen, tMenu->bmpBuffer, 20,140, 20,140, 620,340);

	
	// Process the top buttons
	TopButtons[OptionsMode].MouseOver(Mouse);
	for(int i=0;i<3;i++) {
		
		TopButtons[i].Draw(tMenu->bmpScreen);

		if(i==OptionsMode)
			continue;

		if(TopButtons[i].InBox(Mouse->X,Mouse->Y)) {
			TopButtons[i].MouseOver(Mouse);
			mouse = 1;
			if(Mouse->Up) {
                DrawImageAdv(tMenu->bmpScreen, tMenu->bmpBuffer, 20,140, 20,140, 620,340);
				OptionsMode = i;
// TODO: sound
PlaySoundSample(sfxGeneral.smpClick);
			}
		}
	}



	// Process the gui layout
	ev = cOptions.Process();
	cOptions.Draw(tMenu->bmpScreen);

	if(ev) {

		if(ev->cWidget->getType() == wid_Button)
			mouse = 1;
		if(ev->cWidget->getType() == wid_Textbox)
			mouse = 2;

		switch(ev->iControlID) {

			// Back button
			case Back:
				if(ev->iEventMsg == BTN_MOUSEUP) {

					// Shutdown & save
					Menu_OptionsShutdown();
					SaveOptions();

					// Leave
// TODO: sound
PlaySoundSample(sfxGeneral.smpClick);
					Menu_MainInitialize();
					return;
				}
				break;
		}
	}


	if(OptionsMode == 0) {

		// Controls
		ev = cOpt_Controls.Process();
		cOpt_Controls.Draw(tMenu->bmpScreen);

		if(ev) {

			if(ev->cWidget->getType() == wid_Inputbox) {

				if(ev->iEventMsg == INB_MOUSEUP) {

					int ply = 0;
					if(ev->iControlID >= Ply2_Up && ev->iControlID <= Ply2_Rope)
						ply = 1;
					if(ev->iControlID >= Gen_Chat)
						ply = 2;

					// Get an input
					CInputbox *b = (CInputbox *)ev->cWidget;
					Menu_OptionsWaitInput(ply, b->getName(), b);
					// Re-setup the Take Screenshot key
					if (ev->iControlID == Gen_TakeScreenshot)
						cTakeScreenshot.Setup(tLXOptions->sGeneralControls[SIN_SCREENSHOTS]);
				}
			}
		}
	}


	if(OptionsMode == 1) {

		// Game
		ev = cOpt_Game.Process();
		cOpt_Game.Draw(tMenu->bmpScreen);

		val = cOpt_Game.SendMessage(BloodAmount, SLM_GETVALUE, 0, 0);
		//s = (CSlider *)cOpt_Game.getWidget(BloodAmount);
        DrawImageAdv(tMenu->bmpScreen, tMenu->bmpBuffer, 385,140, 385,140, 70,50);
		tLX->cFont.Draw(tMenu->bmpScreen,385, 148, 0xffff,"%d%%",val);

		//val = cOpt_Game.SendMessage(AIDifficulty, SLM_GETVALUE, 0, 0);
        //DrawImageAdv(tMenu->bmpScreen, tMenu->bmpBuffer, 285,260, 285,260, 100,50);
		//tLX->cFont.Draw(tMenu->bmpScreen,285, 268, 0xffff,"%s",Difficulties[val]);



		if(ev) {

			if(ev->cWidget->getType() == wid_Button)
				mouse = 1;
			if(ev->cWidget->getType() == wid_Textbox)
				mouse = 2;

			switch(ev->iControlID) {

				// Blood amount
				case BloodAmount:
					if(ev->iEventMsg == SLD_CHANGE) {
						val = cOpt_Game.SendMessage(BloodAmount, SLM_GETVALUE, 0, 0);
						tLXOptions->iBloodAmount = val;
					}
					break;

				// Shadows
				case Shadows:
					if(ev->iEventMsg == CHK_CHANGED) {
						c = (CCheckbox *)cOpt_Game.getWidget(Shadows);
						tLXOptions->iShadows = c->getValue();
					}
					break;

				// Particles
				case Particles:
					if(ev->iEventMsg == CHK_CHANGED) {
						c = (CCheckbox *)cOpt_Game.getWidget(Particles);
						tLXOptions->iParticles = c->getValue();
					}
					break;

				// AI Difficulty
				/*case AIDifficulty:
					if(ev->iEventMsg == SLD_CHANGE) {
						val = cOpt_Game.SendMessage(AIDifficulty, SLM_GETVALUE, 0, 0);
						tLXOptions->iAIDifficulty = val;
					}
					break;*/

				// Old skool rope throw
				case OldSkoolRope:
					if(ev->iEventMsg == CHK_CHANGED) {
						tLXOptions->iOldSkoolRope = cOpt_Game.SendMessage(OldSkoolRope,CKM_GETCHECK,0,0);
					}
					break;

				// Show the worm's health below name
				case ShowWormHealth:
					if(ev->iEventMsg == CHK_CHANGED)
						tLXOptions->iShowHealth = cOpt_Game.SendMessage(ShowWormHealth, CKM_GETCHECK, 0, 0);
					break;

				// TDM nick colorizing
				case ColorizeNicks:
					if(ev->iEventMsg == CHK_CHANGED)
						tLXOptions->iColorizeNicks = cOpt_Game.SendMessage(ColorizeNicks, CKM_GETCHECK, 0, 0);
					break;

				// Auto typing
				case AutoTyping:
					if(ev->iEventMsg == CHK_CHANGED)
						tLXOptions->iAutoTyping = cOpt_Game.SendMessage(AutoTyping, CKM_GETCHECK, 0, 0);
					break;

			}
		}

	}


	// Process the different pages
	if(OptionsMode == 2) {

		// Fullscreen value
		c = (CCheckbox *)cOpt_System.getWidget(Fullscreen);
		int fullscr = c->getValue();


		// System
		ev = cOpt_System.Process();
		cOpt_System.Draw(tMenu->bmpScreen);

		if(ev) {

			if(ev->cWidget->getType() == wid_Button)
				mouse = 1;
			if(ev->cWidget->getType() == wid_Textbox)
				mouse = 2;


			switch(ev->iControlID) {

				// Apply
				case Apply:
					if(ev->iEventMsg == BTN_MOUSEUP) {

						// Set to fullscreen
						tLXOptions->iFullscreen = fullscr;
// TODO: sound
PlaySoundSample(sfxGeneral.smpClick);

						// Set the new video mode
						SetVideoMode();

						tMenu->bmpScreen = SDL_GetVideoSurface();
						Menu_RedrawMouse(true);
						SDL_ShowCursor(SDL_DISABLE);
					}
					break;

				// Sound on/off
				case SoundOn:
					if(ev->iEventMsg == CHK_CHANGED) {

						int old = tLXOptions->iSoundOn;
						
						c = (CCheckbox *)cOpt_System.getWidget(SoundOn);
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
				case SoundVolume:
					if(ev->iEventMsg == SLD_CHANGE) {
						CSlider *s = (CSlider *)cOpt_System.getWidget(SoundVolume);
						tLXOptions->iSoundVolume = s->getValue();

						SetSoundVolume( tLXOptions->iSoundVolume );
					}
					break;

				// Show FPS
				case ShowFPS:
					if(ev->iEventMsg == CHK_CHANGED)
						tLXOptions->iShowFPS = cOpt_System.SendMessage(ShowFPS, CKM_GETCHECK, 0, 0);
					break;

				// Filtered
				case Filtered:
					if(ev->iEventMsg == CHK_CHANGED)
						tLXOptions->iFiltered = cOpt_System.SendMessage(Filtered, CKM_GETCHECK, 0, 0);
					break;
					
				// Logging
				case LogConvos:
					if(ev->iEventMsg == CHK_CHANGED)  {
						tLXOptions->iLogConvos = cOpt_System.SendMessage(LogConvos, CKM_GETCHECK, 0, 0);
						FILE *f;
						
						f = fopen_i("Conversations.log","a");
						if (f)  {
							if (tLXOptions->iLogConvos)  {
								char cTime[26];
								GetTime(cTime);
								fprintf(f,"<game starttime=\"%s\">\r\n",cTime);
							} 
							else 
								fprintf(f,"</game>\r\n");
							fclose(f);
						} // if (f)
					}
					break;

				// Show ping
				case ShowPing:
					if(ev->iEventMsg == CHK_CHANGED)
						tLXOptions->iShowPing = cOpt_System.SendMessage(ShowPing, CKM_GETCHECK, 0, 0);
					break;
			}
		}


		// Get the values
		CTextbox *t = (CTextbox *)cOpt_System.getWidget(NetworkPort);
		tLXOptions->iNetworkPort = atoi(t->getText());
		
		tLXOptions->iNetworkSpeed = cOpt_System.SendMessage(NetworkSpeed, CBM_GETCURINDEX,0,0);

		tLXOptions->iScreenshotFormat = cOpt_System.SendMessage(ScreenshotFormat, CBM_GETCURINDEX,0,0);
		

		if(fullscr != tLXOptions->iFullscreen)
			cOpt_System.getWidget(Apply)->setEnabled(true);
        else {
			cOpt_System.getWidget(Apply)->setEnabled(false);
            
            // Redraw the section around the apply button
			if (!cOpt_System.SendMessage(ScreenshotFormat, CBM_ISDROPPED,0,0))
				Menu_redrawBufferRect(550,435, 80,25);
        }
	}


	// Draw the mouse
	DrawImage(tMenu->bmpScreen,gfxGUI.bmpMouse[mouse], Mouse->X,Mouse->Y);
}


///////////////////
// Process an input box waiting thing
void Menu_OptionsWaitInput(int ply, char *name, CInputbox *b)
{
	keyboard_t *kb = GetKeyboard();
	mouse_t *Mouse = GetMouse();

	// Draw the back buffer
	cOptions.Draw(tMenu->bmpBuffer);
	cOpt_Controls.Draw(tMenu->bmpBuffer);

	Menu_DrawBox(tMenu->bmpBuffer, 210, 170, 430, 310);
	//DrawImageAdv(tMenu->bmpBuffer, tMenu->bmpMainBack, 212,172, 212,172, 217,137);
    DrawRectFill(tMenu->bmpBuffer, 212,172,429,309,0);

	tLX->cFont.DrawCentre(tMenu->bmpBuffer,320,180,MakeColour(128,200,255),"%s","Input for:");
	tLX->cFont.DrawCentre(tMenu->bmpBuffer,320,205,MakeColour(255,255,255),"%s",name);
	
	tLX->cFont.DrawCentre(tMenu->bmpBuffer,320,270,MakeColour(255,255,255),"%s","Press any key/mouse");
	tLX->cFont.DrawCentre(tMenu->bmpBuffer,320,285,MakeColour(128,128,128),"%s","(Escape to cancel)");

	TopButtons[OptionsMode].MouseOver(Mouse);
	for(int i=0;i<3;i++) {	
		TopButtons[i].Draw(tMenu->bmpBuffer);
	}


	Menu_RedrawMouse(true);

	CInput inp;

	Mouse->Up = 0;
	Mouse->Down = 0;


	while(1) {
		Menu_RedrawMouse(false);
		ProcessEvents();

		DrawImage(tMenu->bmpScreen,gfxGUI.bmpMouse[0], Mouse->X,Mouse->Y);

		// Escape quits the wait for user input
		if(kb->KeyUp[SDLK_ESCAPE])
			break;

		if(inp.Wait(b->getText()))
			break;

		FlipScreen(tMenu->bmpScreen);
	}

	// Change the options
	if(ply == 0)
		strcpy(tLXOptions->sPlayer1Controls[b->getValue()],b->getText());
	else if(ply == 1)
		strcpy(tLXOptions->sPlayer2Controls[b->getValue()],b->getText());
	else if(ply == 2)
		strcpy(tLXOptions->sGeneralControls[b->getValue()],b->getText());

	Mouse->Down = 0;
	Mouse->Up = 0;

	DrawImage(tMenu->bmpBuffer,tMenu->bmpMainBack_wob,0,0);
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
