/////////////////////////////////////////
//
//             Liero Xtreme
//
//     Copyright Auxiliary Software 2002
//
//
/////////////////////////////////////////


// Main menu
// Created 30/6/02
// Jason Boettcher


#include "defs.h"
#include "LieroX.h"
#include "Menu.h"


CGuiLayout	cMainMenu;
float		alpha = 0;
int			lastimg = -1;


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

	cMainMenu.Shutdown();
	cMainMenu.Initialize();

	for(i=0;i<5;i++)
		cMainMenu.Add( new CTitleButton(i, tMenu->bmpMainTitles), i, 50, 160+i*60, tMenu->bmpMainTitles->w, 35);

	// Quit
	cMainMenu.Add( new CButton(BUT_QUIT, tMenu->bmpButtons), 5, 25,440, 50,15);
}


///////////////////
// Main menu frame
void Menu_MainFrame(void)
{
	gui_event_t *ev;
	mouse_t *Mouse = GetMouse();

	//DrawImageAdv(tMenu->bmpScreen, tMenu->bmpBuffer, 50,160, 50,160, 320,290);
	//DrawImageAdv(tMenu->bmpScreen, tMenu->bmpBuffer, 20,430, 20,430, 60,40);
	
	// Process the buttons
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
			case 0:
                mouseover = true;
                img=0;
                if( ev->iEventMsg == TBT_MOUSEUP ) {
// TODO: implement sound system
PlaySoundSample(sfxGeneral.smpClick);
				    cMainMenu.Shutdown();
				    Menu_LocalInitialize();
				    return;
                }
				break;

			// Network
			case 1:
                mouseover = true;
                img=1;
                if( ev->iEventMsg == TBT_MOUSEUP ) {
// TODO: implement sound system
PlaySoundSample(sfxGeneral.smpClick);
				    cMainMenu.Shutdown();
				    Menu_NetInitialize();
				    return;
                }
				break;

			// Player
			case 2:
                if( ev->iEventMsg == TBT_MOUSEUP ) {
// TODO: implement sound system
PlaySoundSample(sfxGeneral.smpClick);
				    cMainMenu.Shutdown();
				    Menu_PlayerInitialize();
				    return;
                }
				break;

			// Level editor
			case 3:
                if( ev->iEventMsg == TBT_MOUSEUP ) {
// TODO: implement sound system
                    PlaySoundSample(sfxGeneral.smpClick);
				    cMainMenu.Shutdown();
				    Menu_MapEdInitialize();
				    return;
                }
				break;

			// Options
			case 4:
                if( ev->iEventMsg == TBT_MOUSEUP ) {
// TODO: implement sound system
PlaySoundSample(sfxGeneral.smpClick);
				    cMainMenu.Shutdown();
				    Menu_OptionsInitialize();
				    return;
                }
				break;
            
            // Quit
			case 5:
                if( ev->iEventMsg == BTN_MOUSEUP ) {
// TODO: implement sound system
        PlaySoundSample(sfxGeneral.smpClick);
				
                    cMainMenu.Draw(tMenu->bmpBuffer);

                    if( Menu_MessageBox("Liero Xtreme","Quit Liero Xtreme?", LMB_YESNO) == MBR_YES ) {
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

		int y = 640 - (int)(alpha * 10.0f)*64;

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
	tLX->cFont.Draw(tMenu->bmpScreen, 365, 404, MakeColour(96,96,96), "Liero Xtreme Professional v%.2f beta",LX_VERSION);
	tLX->cFont.Draw(tMenu->bmpScreen, 365, 417, MakeColour(96,96,96),"%s", "By [RIP] Clan 2006");
	tLX->cFont.Draw(tMenu->bmpScreen, 365, 430, MakeColour(96,96,96),"%s", "Release date: 22. Oct 2006");
	tLX->cFont.Draw(tMenu->bmpScreen, 365, 443, MakeColour(96,96,96),"%s", "Original code by Jason Boetcher");

	// Draw the mouse
	DrawImage(tMenu->bmpScreen,gfxGUI.bmpMouse[mouse], Mouse->X,Mouse->Y);
}


///////////////////
// Shutdown the main menu
void Menu_MainShutdown(void)
{
	cMainMenu.Shutdown();
}
