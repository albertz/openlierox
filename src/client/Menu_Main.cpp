/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Main menu
// Created 30/6/02
// Jason Boettcher


#include "defs.h"
#include "LieroX.h"
#include "Menu.h"
#include "GfxPrimitives.h"


CGuiLayout	cMainMenu;
float		alpha = 0;
int			lastimg = -1;

enum {
	mm_LocalPlay=0,
	mm_NetPlay,
	mm_PlayerProfiles,
	mm_LevelEditor,
	mm_Options,
	mm_Quit
};


///////////////////
// Initialize the main menu
void Menu_MainInitialize(void)
{
	// TEMP FOR NOW ??
	//Menu_Net_NETInitialize();

	int i;
	assert(tMenu);
	tMenu->iMenuRunning = true;
	tMenu->iMenuType = MNU_MAIN;

	// Create the buffer
	assert(tMenu->bmpBuffer);
	DrawImage(tMenu->bmpBuffer,tMenu->bmpMainBack_wob,0,0);
	DrawImage(tMenu->bmpBuffer,tMenu->bmpLieroXtreme, 320 - tMenu->bmpLieroXtreme->w/2, 10);
    Menu_DrawBox(tMenu->bmpBuffer, 15,130, 625, 465);

	Menu_RedrawMouse(true);

	alpha = 0;
	lastimg = -1;

	// Menu buttons
	for(i=mm_LocalPlay;i<mm_Quit;i++)
		cMainMenu.Add( new CTitleButton(i, tMenu->bmpMainTitles), i, 50, 160+i*60, tMenu->bmpMainTitles->w, 35);

	// Quit
	cMainMenu.Add( new CButton(BUT_QUIT, tMenu->bmpButtons), mm_Quit, 25,440, 50,15);

	// Temp
	//cMainMenu.Add( new CBox(10,2,MakeColour(128,128,128),MakeColour(64,64,64),MakeColour(0,0,0)),10,100,100,100,100);

}


///////////////////
// Main menu frame
void Menu_MainFrame(void)
{
	gui_event_t *ev = NULL;
	mouse_t *Mouse = GetMouse();

	//DrawImageAdv(tMenu->bmpScreen, tMenu->bmpBuffer, 50,160, 50,160, 320,290);
	//DrawImageAdv(tMenu->bmpScreen, tMenu->bmpBuffer, 20,430, 20,430, 60,40);

	// Process the buttons
	if (!cMediaPlayer.GetDrawPlayer())
		ev = cMainMenu.Process();
	cMainMenu.Draw(tMenu->bmpScreen);

	int mouseover = false;
	int img = lastimg;
	int mouse = 0;

	if(ev) {

        if( ev->cWidget->getType() == wid_Titlebutton ||
            ev->cWidget->getType() == wid_Button )
            mouse = 1;

		switch(ev->iControlID) {

			// local
			case mm_LocalPlay:
                mouseover = true;
                img=0;
                if( ev->iEventMsg == TBT_MOUSEUP ) {
					PlaySoundSample(sfxGeneral.smpClick);
				    cMainMenu.Shutdown();
				    Menu_LocalInitialize();
				    return;
                }
				break;

			// Network
			case mm_NetPlay:
                mouseover = true;
                img=1;
                if( ev->iEventMsg == TBT_MOUSEUP ) {
					PlaySoundSample(sfxGeneral.smpClick);
				    cMainMenu.Shutdown();
				    Menu_NetInitialize();
				    return;
                }
				break;

			// Player
			case mm_PlayerProfiles:
                if( ev->iEventMsg == TBT_MOUSEUP ) {
					PlaySoundSample(sfxGeneral.smpClick);
				    cMainMenu.Shutdown();
				    Menu_PlayerInitialize();
				    return;
                }
				break;

			// Level editor
			case mm_LevelEditor:
                if( ev->iEventMsg == TBT_MOUSEUP ) {
                    PlaySoundSample(sfxGeneral.smpClick);
				    cMainMenu.Shutdown();
				    Menu_MapEdInitialize();
				    return;
                }
				break;

			// Options
			case mm_Options:
                if( ev->iEventMsg == TBT_MOUSEUP ) {
					PlaySoundSample(sfxGeneral.smpClick);
				    cMainMenu.Shutdown();
				    Menu_OptionsInitialize();
				    return;
                }
				break;

            // Quit
			case mm_Quit:
                if( ev->iEventMsg == BTN_MOUSEUP ) {
			        PlaySoundSample(sfxGeneral.smpClick);

                    cMainMenu.Draw(tMenu->bmpBuffer);

                    if( Menu_MessageBox(GetGameName(),"Quit OpenLieroX?", LMB_YESNO) == MBR_YES ) {
					    tMenu->iMenuRunning = false;
					    cMainMenu.Shutdown();
				    } else {

					    // Create the buffer
					    DrawImage(tMenu->bmpBuffer,tMenu->bmpMainBack_wob,0,0);
                        Menu_DrawBox(tMenu->bmpBuffer, 15,130, 625, 465);
					    DrawImage(tMenu->bmpBuffer,tMenu->bmpLieroXtreme, 320 - tMenu->bmpLieroXtreme->w/2, 10);
					    Menu_RedrawMouse(true);
				    }
				    return;
                }
                break;
		}
	}

	if(mouseover) {
		alpha += tLX->fDeltaTime*5;
		alpha = MIN(1.0f,alpha);
	} else {
		alpha -= tLX->fDeltaTime*5;
		alpha = MAX(0.0f,alpha);
	}

	if(alpha) {

		DrawImageAdv(tMenu->bmpScreen,tMenu->bmpBuffer, 410,260, 410,260, 200,64);

//		int y = 640 - (int)(alpha * 10.0f)*64;  // TODO: not used

		switch(img) {
			case 0:
				//DrawImageAdv(tMenu->bmpScreen, tMenu->bmpMainLocal, 0,y, 410, 260, tMenu->bmpMainLocal->w,64);
				break;
			case 1:
				//DrawImageAdv(tMenu->bmpScreen, tMenu->bmpMainNet, 0,y, 410, 260, tMenu->bmpMainNet->w,64);
				break;
		}
		lastimg = img;
	}


	// Draw the version number
	tLX->cFont.Draw(tMenu->bmpScreen, 370, 379, tLX->clCredits1, "  %s v%s",GetGameName().c_str(),LX_VERSION);
	tLX->cFont.Draw(tMenu->bmpScreen, 370, 391, tLX->clCredits2,"%s", "+ Original code by Jason Boettcher");
	tLX->cFont.Draw(tMenu->bmpScreen, 370, 404, tLX->clCredits2,"%s", "+ Ported and enhanced by");
	tLX->cFont.Draw(tMenu->bmpScreen, 370, 417, tLX->clCredits2,"%s", "  Dark Charlie and Albert Zeyer");
	tLX->cFont.Draw(tMenu->bmpScreen, 370, 430, tLX->clCredits2,"%s", "+ Supported by the [RIP] clan");
// TODO: include this, if he join the team :)
//	tLX->cFont.Draw(tMenu->bmpScreen, 370, 443, tLX->clCredits2,"%s", "+ Enhanced by FilE");


	// Draw the mouse
	DrawImage(tMenu->bmpScreen,gfxGUI.bmpMouse[mouse], Mouse->X,Mouse->Y);
}


///////////////////
// Shutdown the main menu
void Menu_MainShutdown(void)
{
	cMainMenu.Shutdown();
}
