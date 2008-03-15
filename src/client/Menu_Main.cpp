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

#include "LieroX.h"
#include "AuxLib.h"
#include "Graphics.h"
#include "Menu.h"
#include "GfxPrimitives.h"
#include "CTitleButton.h"
#include "CButton.h"
#include "CMediaPlayer.h"
#include "CGuiSkin.h"
#include "CLabel.h"
#include "CCombobox.h"
#include "CCheckbox.h"
#include "CBrowser.h"


CGuiLayout	cMainMenu;
float		alpha = 0;
int			lastimg = -1;

enum {
	mm_LocalPlay=0,
	mm_NetPlay,
	mm_PlayerProfiles,
	mm_LevelEditor,
	mm_Options,
	mm_Quit,
	mm_NewsBox,
	mm_ShowNews
};

#ifdef DEBUG
static void Menu_Main_GuiSkinComboboxCreate();
#endif
static void Menu_Main_GuiThemeComboboxCreate();

///////////////////
// Initialize the main menu
void Menu_MainInitialize(void)
{
	int i;
	assert(tMenu);
	tMenu->bMenuRunning = true;
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

	#ifdef DEBUG
	Menu_Main_GuiSkinComboboxCreate();	// Just moved ugly code to function so it won't stand out too much
	#endif
	// selection of different themes (like default, LX56, ...)
	Menu_Main_GuiThemeComboboxCreate();

	// Check if skin should be loaded instead of main menu ( also when selecting different skin from skinned menu )
	if( tLXOptions->sSkinPath != "" )
	{
		Menu_MainShutdown();
		Menu_CGuiSkinInitialize();
	}
}


///////////////////
// Main menu frame
void Menu_MainFrame(void)
{
	gui_event_t *ev = NULL;

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
				    Menu_MainShutdown();
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
				    Menu_MainShutdown();
				    Menu_NetInitialize();
				    return;
                }
				break;

			// Player
			case mm_PlayerProfiles:
                if( ev->iEventMsg == TBT_MOUSEUP ) {
					PlaySoundSample(sfxGeneral.smpClick);
				    Menu_MainShutdown();
				    Menu_PlayerInitialize();
				    return;
                }
				break;

			// Level editor
			case mm_LevelEditor:
                if( ev->iEventMsg == TBT_MOUSEUP ) {
                    PlaySoundSample(sfxGeneral.smpClick);
				    Menu_MainShutdown();
				    Menu_MapEdInitialize();
				    return;
                }
				break;

			// Options
			case mm_Options:
                if( ev->iEventMsg == TBT_MOUSEUP ) {
					PlaySoundSample(sfxGeneral.smpClick);
				    Menu_MainShutdown();
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
					    tMenu->bMenuRunning = false;
					    Menu_MainShutdown();
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

			default:
                if( ev->iEventMsg == CMB_CHANGED )
				{
					ev->cWidget->ProcessGuiSkinEvent(ev->iEventMsg);
				    return;
				};
                break;
		}
	}

	CGuiSkin::ProcessUpdateCallbacks();	// Process the news box

	if(mouseover) {
		alpha += tLX->fDeltaTime*5;
		alpha = MIN(1.0f,alpha);
	} else {
		alpha -= tLX->fDeltaTime*5;
		alpha = MAX(0.0f,alpha);
	}

	if(alpha) {

		DrawImageAdv(tMenu->bmpScreen,tMenu->bmpBuffer, 410,260, 410,260, 200,64);

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


	// Credits

	static const std::string credits1 = "  " + GetGameName() + " v" + LX_VERSION;

	static const std::string credits2 = std::string(
		"- Original code by Jason Boettcher\n"
		"- Ported and enhanced by\n"
		"  K. Petránek, Albert Zeyer, Daniel Sjoholm,\n" // In order of joining
		"  Martin Griffin, Pelya\n"
		"- Supported by the [RIP] clan\n"
		//"- Enhanced by FilE\n" + // TODO: include this, if he join the team :)
		)
		+ tMenu->tFrontendInfo.sFrontendCredits;


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
		w = MAX(tLX->cFont.GetWidth(credits1), tLX->cFont.GetWidth(credits2)) + 4;

	static int h = 0;
	if (!h)
		h = tLX->cFont.GetHeight() + tLX->cFont.GetHeight(credits2) + 4;

	Menu_redrawBufferRect(x, y, w, h);
	tLX->cFont.Draw(tMenu->bmpScreen, x, y, tLX->clCredits1, credits1);
	tLX->cFont.Draw(tMenu->bmpScreen, x, y + tLX->cFont.GetHeight(), tLX->clCredits2, credits2);


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


struct Menu_Main_GuiThemeComboboxCreate__Executer {
	CCombobox* combobox;

	void execute() {
		// GUI theme combobox

		CScriptableVars::RegisterVars("GUI")
			( & ThemeCombobox_OnChange, "ThemeCombobox_OnChange" );

		std::vector< CScriptableVars::ScriptVar_t > GuiThemeInit;
		GuiThemeInit.push_back( CScriptableVars::ScriptVar_t ( "None#" ) );	// List of items
		GuiThemeInit.push_back( CScriptableVars::ScriptVar_t ( "GameOptions.Game.Theme" ) );	// Attached var
		GuiThemeInit.push_back( CScriptableVars::ScriptVar_t ( "GUI.MakeSound() GUI.ThemeCombobox_OnChange()" ) );	// OnClick handler

		cMainMenu.Add( new CLabel("Theme", tLX->clNormalLabel), -1, 465,10,0,0);
		combobox = (CCombobox*) CCombobox::WidgetCreator(GuiThemeInit, &cMainMenu, -1, 515,8,115,17);


		// Find all directories in the the lierox
		combobox->clear();
		combobox->setSorted(SORT_ASC);
		combobox->setUnique(true);

		combobox->addItem("", "- Default -");
		FindFiles(*this, "themes", false, FM_DIR);
		combobox->setCurSIndexItem(tLXOptions->sTheme);


		combobox->ProcessGuiSkinEvent( CGuiSkin::SHOW_WIDGET );
	}

	static void ThemeCombobox_OnChange( const std::string & param, CWidget * source ) {
		printf("Changed theme to: " + tLXOptions->sTheme + "\n");

		// Atm it's the easiest way and we ensure that every gfx is newly loaded.
		// The problem is that we would have to clear all the caches because
		// the order of the searchpaths changed etc.

		// restart game
		tMenu->bMenuRunning = false; // quit
		Menu_MainShutdown(); // cleanup for this menu
		bRestartGameAfterQuit = true; // set restart-flag
	}

	// handler for FindFile
	bool operator() (std::string abs_filename) {
		size_t sep = findLastPathSep(abs_filename);
		if(sep != std::string::npos) abs_filename.erase(0, sep+1);

		if(abs_filename != "" && abs_filename[0] != '.')
			combobox->addItem(abs_filename, abs_filename);

		return true;
	}
};


void Menu_Main_GuiThemeComboboxCreate() {
	Menu_Main_GuiThemeComboboxCreate__Executer initialiser;
	initialiser.execute();
}

#ifdef DEBUG
void Menu_Main_GuiSkinComboboxCreate()
{
	// GUI skin combobox
	// TODO: hacky hacky, non-skinned code with skinned widgets, maybe move to different function
	cMainMenu.Add( new CLabel("Skin",tLX->clNormalLabel), -1, 480,40,0,0);
	std::vector< CScriptableVars::ScriptVar_t > GuiSkinInit;
	GuiSkinInit.push_back( CScriptableVars::ScriptVar_t ( "None#" ) );	// List of items
	GuiSkinInit.push_back( CScriptableVars::ScriptVar_t ( "GameOptions.Game.SkinPath" ) );	// Attached var
	GuiSkinInit.push_back( CScriptableVars::ScriptVar_t ( "GUI.MakeSound() GUI.SkinCombobox_Change()" ) );	// OnClick handler
	// TODO: position as constant, will remove this code when only skins will be left
	CWidget * GuiSkin = CCombobox::WidgetCreator(GuiSkinInit, &cMainMenu, -1, 515,38,115,17);
	CGuiSkin::CallbackHandler c_init( "GUI.SkinCombobox_Init()", GuiSkin );
	c_init.Call();
	GuiSkin->ProcessGuiSkinEvent( CGuiSkin::SHOW_WIDGET );
}
#endif
