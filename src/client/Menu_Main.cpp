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

#include <assert.h>

#include "defs.h"
#include "LieroX.h"
#include "Graphics.h"
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
	int i;
	assert(tMenu);
	tMenu->iMenuRunning = true;
	tMenu->iMenuType = MNU_MAIN;

	// Create the buffer
	assert(tMenu->bmpBuffer);
	DrawImage(tMenu->bmpBuffer,tMenu->bmpMainBack_wob,0,0);
	DrawImage(tMenu->bmpBuffer,tMenu->bmpLieroXtreme, 320 - tMenu->bmpLieroXtreme->w/2, 10);
	if (tMenu->tFrontendInfo.bPageBoxes)
		Menu_DrawBox(tMenu->bmpBuffer, 15,130, 625, 465);

	Menu_RedrawMouse(true);

	alpha = 0;
	lastimg = -1;

	// Menu buttons
	int titleheight = tMenu->bmpMainTitles->h/((mm_Quit-mm_LocalPlay)*2);
	for(i=mm_LocalPlay;i<mm_Quit;i++)
		cMainMenu.Add( new CTitleButton(i, tMenu->bmpMainTitles), i, tMenu->tFrontendInfo.iMainTitlesLeft, tMenu->tFrontendInfo.iMainTitlesTop+i*(titleheight+tMenu->tFrontendInfo.iMainTitlesSpacing), tMenu->bmpMainTitles->w, titleheight);

	// Quit
	cMainMenu.Add( new CButton(BUT_QUIT, tMenu->bmpButtons), mm_Quit, 25,440, 50,15);
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
#ifdef WITH_MEDIAPLAYER
	if (!cMediaPlayer.GetDrawPlayer())
#endif
		ev = cMainMenu.Process();
	cMainMenu.Draw(tMenu->bmpScreen);

	int mouseover = false;
	int img = lastimg;

	if(ev) {

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
						if (tMenu->tFrontendInfo.bPageBoxes)
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

	/*static const std::string credits[] = {
				"  " + GetGameName() + " v" + LX_VERSION,
				"¤ Original code by Jason Boettcher",
				"¤ Ported and enhanced by",
				"  Dark Charlie and Albert Zeyer",
				"¤ Supported by the [RIP] clan"
	};*/

	static const std::string credits1 = "  " + GetGameName() + " v" + LX_VERSION;

	static const std::string credits2 = std::string("¤ Original code by Jason Boettcher\n") +
										std::string("¤ Ported and enhanced by\n") +
										std::string("  Dark Charlie and Albert Zeyer\n") +
										std::string("¤ Supported by the [RIP] clan");
										//std::string("¤ Enhanced by FilE");// TODO: include this, if he join the team :)


	//
	// Draw the version number
	//

	// Set special spacing for credits
	int orig_spacing = tLX->cFont.GetVSpacing();
	tLX->cFont.SetVSpacing(tMenu->tFrontendInfo.iCreditsSpacing);

	int x = tMenu->tFrontendInfo.iCreditsLeft;
	int y = tMenu->tFrontendInfo.iCreditsTop;
	static int w = 0;
	if (!w)
		w = MAX(tLX->cFont.GetWidth(credits1),tLX->cFont.GetWidth(credits2));
	static int h = 0;
	if (!h)
		h = tLX->cFont.GetHeight()+tLX->cFont.GetHeight(credits2);

	Menu_redrawBufferRect(x,y,w,h);
	tLX->cFont.Draw(tMenu->bmpScreen,x,y,tLX->clCredits1,credits1);
	tLX->cFont.Draw(tMenu->bmpScreen,x,y+tLX->cFont.GetHeight(),tLX->clCredits2,credits2);


	// Restore the original spacing
	tLX->cFont.SetVSpacing(orig_spacing);


	// Draw the mouse
	DrawCursor(tMenu->bmpScreen);
}


///////////////////
// Shutdown the main menu
void Menu_MainShutdown(void)
{
	cMainMenu.Shutdown();
}
