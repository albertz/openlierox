/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Local menu
// Created 30/6/02
// Jason Boettcher


#include <assert.h>
#include <string>

#include "LieroX.h"
#include "CGameScript.h"
#include "AuxLib.h"
#include "DeprecatedGUI/Graphics.h"
#include "CClient.h"
#include "CServer.h"
#include "DeprecatedGUI/Menu.h"
#include "DeprecatedGUI/CListview.h"
#include "GfxPrimitives.h"
#include "FindFile.h"
#include "StringUtils.h"
#include "DeprecatedGUI/CButton.h"
#include "DeprecatedGUI/CLabel.h"
#include "DeprecatedGUI/CImage.h"
#include "DeprecatedGUI/CTextbox.h"
#include "DeprecatedGUI/CSlider.h"
#include "DeprecatedGUI/CCheckbox.h"
#include "DeprecatedGUI/CTextButton.h"
#include "DeprecatedGUI/CListview.h"
#include "DeprecatedGUI/CGuiSkinnedLayout.h"
#include "DeprecatedGUI/CBrowser.h"
#include "Sounds.h"
#include "ProfileSystem.h"
#include "FeatureList.h"
#include "Options.h"
#include "CGameMode.h"


namespace DeprecatedGUI {

/*
=======================

  Weapons Restrictions

 For both local & host

=======================
*/


CGuiLayout		cWeaponsRest;
static CWpnRest        cWpnRestList;
static CGameScript     *cWpnGameScript = NULL;

// State strings
static const std::string szStates[] = {"Enabled", "Bonus", "Banned"};

// Weapons Restrictions
enum {
	wr_Ok,
    wr_Reset,
	wr_ListBox,
	wr_Cancel,
    wr_Random,
	wr_Load,
	wr_Save,
	wr_List,
};

static std::list<wpnrest_t *> tWeaponList;

/////////////////
// Updates the weapon list that is shown in the dialog
static void UpdateWeaponList()
{
	tWeaponList.clear();
	wpnrest_t *psWpn = cWpnRestList.getList();
	if (!psWpn)
		return;
	
	for(int i=0; i < cWpnRestList.getNumWeapons(); i++) {
        if(cWpnGameScript->weaponExists(psWpn[i].psLink->szName))
			tWeaponList.push_back(&psWpn[i]);
    }
}

static void UpdateWeaponListWidget()
{
	const Color cStateColours[] = { tLX->clNormalLabel, tLX->clSubHeading, tLX->clDisabled };

	int idx = 0;
	CListview *listWidget = dynamic_cast<CListview *>(cWeaponsRest.getWidget(wr_List));
	if (!listWidget)
		return;
	listWidget->Clear();
	for (auto weapon: tWeaponList) {
		std::string buf = weapon->psLink->szName;
		stripdot(buf,245);
		listWidget->AddItem(buf, idx, tLX->clNormalLabel);
		listWidget->AddSubitem(buf);
		listWidget->AddSubitem(szStates[weapon->psLink->nState], VALIGN_MIDDLE, cStateColours[weapon->psLink->nState]);
		idx++;
	}
}

///////////////////
// Initialize the weapons restrictions
void Menu_WeaponsRestrictions(const std::string& szMod)
{

	// Setup the buffer
	DrawImageAdv(tMenu->bmpBuffer.get(), tMenu->bmpMainBack_common, 120,150,120,150, 400,330);
	Menu_DrawBox(tMenu->bmpBuffer.get(), 120,150, 520,470);
	//DrawRectFillA(tMenu->bmpBuffer, 122,152, 518,438, 0, 100);


	Menu_RedrawMouse(true);

	cWeaponsRest.Initialize();
	cWeaponsRest.Add( new CLabel("Weapon Options", tLX->clNormalLabel),     -1,        275,155, 0, 0);
    cWeaponsRest.Add( new CButton(BUT_RESET, tMenu->bmpButtons),wr_Reset,  180,420, 60,15);
    cWeaponsRest.Add( new CButton(BUT_RANDOM, tMenu->bmpButtons),wr_Random,280,420, 80,15);
    cWeaponsRest.Add( new CButton(BUT_OK, tMenu->bmpButtons),	wr_Ok,     400,420, 30,15);
    cWeaponsRest.Add( new CButton(BUT_LOAD, tMenu->bmpButtons),	wr_Load,   250,445, 60,15);
    cWeaponsRest.Add( new CButton(BUT_SAVE, tMenu->bmpButtons),	wr_Save,   330,445, 60,15);
	CListview * listWidget = new CListview();
	cWeaponsRest.Add( listWidget,                               wr_List,   180,185, 320,230);
	listWidget->AddColumn("", 240);
	listWidget->AddColumn("", 55);

    // Load the weapons
    cWpnRestList.loadList("cfg/wpnrest.dat");


    //
    // Update the list with the currently selected mod
    //

    cWpnGameScript = new CGameScript;
    if( cWpnGameScript ) {
        if( cWpnGameScript->Load(szMod) )
            cWpnRestList.updateList( cWpnGameScript );
    }

    // Get the weapons for the list
	UpdateWeaponList();
	UpdateWeaponListWidget();
}

//////////////////
// Shutdown the weapon restrictions
void Menu_WeaponsRestrictionsShutdown()
{
	cWeaponsRest.Shutdown();

    cWpnRestList.saveList("cfg/wpnrest.dat");
    cWpnRestList.Shutdown();

	if (cWpnGameScript)  {
		// HINT: the gamescript is shut down by the cache
		delete cWpnGameScript;
		cWpnGameScript = NULL;
	}
}


///////////////////
// Weapons Restrictions frame
// Returns whether or not we have finished with the weapons restrictions
bool Menu_WeaponsRestrictions_Frame()
{
	gui_event_t *ev = NULL;
    //Uint32 blue = MakeColour(0,138,251);

    assert(cWpnGameScript);


	DrawImageAdv(VideoPostProcessor::videoSurface(), tMenu->bmpBuffer, 120,150, 120,150, 400,300);

	/*
    // Draw the list
    int count = cWeaponsRest.SendMessage(wr_Scroll, SCM_GETVALUE,(DWORD)0,0);

	int w, j;
	w = j = 0;
	for (std::list<wpnrest_t *>::iterator it = tWeaponList.begin(); it != tWeaponList.end(); it++)  {
        if( w++ < count )
            continue;
        if( j > 10 )
            break;

        int y = 190 + (j++)*20;
        Color Colour = tLX->clNormalLabel;
		Color StateColour = Colour;
		if( (*it)->psLink->nState == wpr_bonus ) // Different color will make it more comfortable for eyes
			StateColour = tLX->clSubHeading;
		if( (*it)->psLink->nState == wpr_banned )
			StateColour = tLX->clDisabled;

        // If a mouse is over the line, highlight it
        if( Mouse->X > 150 && Mouse->X < 450 ) {
            if( Mouse->Y > y && Mouse->Y < y+20 ) {
                Colour = tLX->clMouseOver;
				StateColour = tLX->clMouseOver;

                // If the mouse has been clicked, cycle through the states
                if( Mouse->Up & SDL_BUTTON(1) ) {
                    (*it)->psLink->nState++;
                    (*it)->psLink->nState %= 3;
                }
            }
        }

		std::string buf = (*it)->psLink->szName;
		stripdot(buf,245);
        tLX->cFont.Draw( VideoPostProcessor::videoSurface(), 150, y, Colour, buf );
        tLX->cFont.Draw( VideoPostProcessor::videoSurface(), 400, y, StateColour, szStates[(*it)->psLink->nState] );
	}

    // Adjust the scrollbar
    cWeaponsRest.SendMessage(wr_Scroll, SCM_SETITEMSPERBOX, 12, 0);
    cWeaponsRest.SendMessage(wr_Scroll, SCM_SETMIN, (DWORD)0, 0);
	if(tWeaponList.size() > 10)
		cWeaponsRest.SendMessage(wr_Scroll, SCM_SETMAX, tWeaponList.size() + 1, 0);
    else
        cWeaponsRest.SendMessage(wr_Scroll, SCM_SETMAX, (DWORD)0, 0);
	*/


	ev = cWeaponsRest.Process();
	cWeaponsRest.Draw(VideoPostProcessor::videoSurface());

	if(ev) {

		switch(ev->iControlID) {

			// OK, done
			case wr_Ok:
				if(ev->iEventMsg == BTN_CLICKED) {

					Menu_WeaponsRestrictionsShutdown();

					return true;
				}
				break;

            // Reset the list
            case wr_Reset:
                if( ev->iEventMsg == BTN_CLICKED ) {
                    cWpnRestList.cycleVisible(cWpnGameScript);
                    UpdateWeaponListWidget();
                }
                break;

            // Randomize the list
            case wr_Random:
                if(ev->iEventMsg == BTN_CLICKED) {
                    cWpnRestList.randomizeVisible(cWpnGameScript);
                    UpdateWeaponListWidget();
                }
                break;

            // Open the load dialog
            case wr_Load:
                if(ev->iEventMsg == BTN_CLICKED) {
                    Menu_WeaponPresets(false,&cWpnRestList);
                    UpdateWeaponListWidget();
                }
                break;

            // Open the save dialog
            case wr_Save:
                if(ev->iEventMsg == BTN_CLICKED) {
                    Menu_WeaponPresets(true,&cWpnRestList);
                    UpdateWeaponListWidget();
                }
                break;

			case wr_List:
				if (ev->iEventMsg == LV_ENTER || ev->iEventMsg == LV_DOUBLECLK) {
					const Color cStateColours[] = { tLX->clNormalLabel, tLX->clSubHeading, tLX->clDisabled };
					CListview *listWidget = dynamic_cast<CListview *>(cWeaponsRest.getWidget(wr_List));
					lv_subitem_t *sub = listWidget->getCurSubitem(1);
					if (sub) {
						int idx = 0;
						for (auto weapon: tWeaponList) {
							if (idx == listWidget->getCurIndex()) {
								weapon->psLink->nState++;
								weapon->psLink->nState %= 3;
								sub->sText = szStates[weapon->psLink->nState];
								sub->iColour = cStateColours[weapon->psLink->nState];
								break;
							}
							idx++;
						}
					}
				}
				break;
		}
	}

	// Draw the mouse
	DrawCursor(VideoPostProcessor::videoSurface());

	return false;
}


///////////////////
// Weapon presets load/save
// For both local and host
enum  {
	wp_Cancel=0,
	wp_Ok,
	wp_PresetList,
	wp_PresetName
};

CGuiLayout cWpnPresets;

	struct WeaponPresetsAdder {
		CListview* listview;
		WeaponPresetsAdder(CListview* lv_) : listview(lv_) {}
		bool operator() (const std::string& f) {
			if(stringcaseequal(GetFileExtension(f),"wps")) {
				std::vector<std::string> fnparts = ListAsVector(SplitFilename(f, 3));
				if(fnparts.size() < 3) return true; // strange
				std::string fname = fnparts[1] + "/" + fnparts[2];
				if(stringcaseequal(fnparts[0], "cfg") && stringcaseequal(fnparts[1], "presets"))
					fname = fnparts[2];
				std::string name = GetBaseFilenameWithoutExt(fname);
				if(!listview->getItem(fname)) {
					listview->AddItem(fname,0,tLX->clListView);
					listview->AddSubitem(LVS_TEXT, name, (DynDrawIntf*)NULL, NULL);
				}
			}
			return true;
		}
	};

void Menu_WeaponPresets(bool save, CWpnRest *wpnrest)
{
	if (!wpnrest)
		return;

	gui_event_t *ev = NULL;
	int quitloop = false;
	CTextbox *t;

	// Save the background
	DrawImageAdv(tMenu->bmpBuffer.get(), VideoPostProcessor::videoSurface(), 0,0, 0,0, 640,480);

	Menu_RedrawMouse(true);

	cWpnPresets.Initialize();

	cWpnPresets.Add( new CButton(BUT_CANCEL, tMenu->bmpButtons), wp_Cancel, 180,310, 75,15);
	cWpnPresets.Add( new CButton(BUT_OK, tMenu->bmpButtons),     wp_Ok, 430,310, 40,15);
	cWpnPresets.Add( new CListview(),                            wp_PresetList, 180,170, 280,110+(!save)*20);
	cWpnPresets.Add( new CTextbox(),                             wp_PresetName, 270,285, 190,tLX->cFont.GetHeight());

	cWpnPresets.SendMessage(wp_PresetList,LVM_SETOLDSTYLE,(DWORD)0,0);

	t = (CTextbox *)cWpnPresets.getWidget(wp_PresetName);

	// Hide the textbox for Load
	t->setEnabled(save);

	// Load the level list

	CListview *lv = (CListview *)cWpnPresets.getWidget(wp_PresetList);
	lv->AddColumn("Weapon presets",60);

	WeaponPresetsAdder adder(lv);
	FindFiles(adder, "cfg/presets/" + cWpnGameScript->directory(), false, FM_REG);
	FindFiles(adder, "cfg/presets", false, FM_REG);
	
	lv->SortBy( 0, true );

	ProcessEvents();
	while(!WasKeyboardEventHappening(SDLK_ESCAPE,false) && !quitloop && tMenu->bMenuRunning) {
		Menu_RedrawMouse(true);

		//DrawImageAdv(VideoPostProcessor::videoSurface(),tMenu->bmpBuffer, 170,150, 170,150, 300, 180);
		Menu_DrawBox(VideoPostProcessor::videoSurface(), 170, 150, 470, 330);
		DrawImageAdv(VideoPostProcessor::videoSurface(), tMenu->bmpMainBack_common, 172,152, 172,152, 297,177);
		DrawImageAdv(tMenu->bmpBuffer.get(), tMenu->bmpMainBack_common, 172,152, 172,152, 297,177);

		tLX->cFont.DrawCentre(VideoPostProcessor::videoSurface(), 320, 155, tLX->clNormalLabel, save ? "Save" : "Load");
		if (save)
			tLX->cFont.Draw(VideoPostProcessor::videoSurface(), 180,288,tLX->clNormalLabel,"Preset name");

		ev = cWpnPresets.Process();
		cWpnPresets.Draw(VideoPostProcessor::videoSurface());

		// Process the widgets
		if(ev)  {

			switch(ev->iControlID) {
				// Cancel
				case wp_Cancel:
					if(ev->iEventMsg == BTN_CLICKED) {
						PlaySoundSample(sfxGeneral.smpClick);
						quitloop = true;
					}
					break;

				// Presets list
				case wp_PresetList:
					if(ev->iEventMsg != LV_NONE) {
						if(save) t->setText( GetBaseFilenameWithoutExt(lv->getCurSIndex()) );
						else t->setText( lv->getCurSIndex() );
					}
				break;
			}

			// OK and double click on listview
			if (ev->iControlID == wp_Ok || ev->iControlID == wp_PresetList)  {
				if((ev->iEventMsg == BTN_CLICKED && ev->iControlID == 1) || ev->iEventMsg == LV_DOUBLECLK || ev->iEventMsg == LV_ENTER) {

					// Play the sound only for OK button
					if (ev->iControlID == wp_Ok)
						PlaySoundSample(sfxGeneral.smpClick);

					// Don't process when nothing is selected
					if(t->getText().length() > 0) {

						quitloop = true;
						if(save) {

							// Save
							std::string fn = "cfg/presets/" + cWpnGameScript->directory() + "/" + t->getText(); // + ".wps";
							if(fn.find(".wps") == std::string::npos )
								fn += ".wps";

							// Check if it exists already. If so, ask user if they wanna overwrite
							if(Menu_WeaponPresetsOkSave(fn))
								wpnrest->saveList(fn);
							else
								quitloop = false;
						} else {

							// Load
							std::string fn = "cfg/presets/" + t->getText();
							wpnrest->loadList(fn);
							wpnrest->updateList(cWpnGameScript);
							UpdateWeaponList();
						}
					}
				}
			}
		}

		// Draw mouse
		DrawCursor(VideoPostProcessor::videoSurface());

		// Display the dialog
		doVideoFrameInMainThread();

		CapFPS();
		WaitForNextEvent();
	}

	// Redraw back to normal
	DrawImageAdv(tMenu->bmpBuffer.get(), tMenu->bmpMainBack_common, 120,150,122,152, 396,316);
	DrawImage(VideoPostProcessor::videoSurface(),tMenu->bmpBuffer,0,0);

	Menu_RedrawMouse(true);

	Menu_WeaponPresetsShutdown();
}

/////////////
// Shutdown
void Menu_WeaponPresetsShutdown()
{
	cWpnPresets.Shutdown();
}


///////////////////
// Check if there is a possible overwrite
bool Menu_WeaponPresetsOkSave(const std::string& szFilename)
{
	std::string filename = szFilename;

	// Adjust the filename
	if( stringcasecmp(GetFileExtension( szFilename ), "wps") != 0)
		filename += ".wps";

	FILE *fp = OpenGameFile(filename,"rb");
	if( fp == NULL)
		// File doesn't exist, ok to save
		return true;

	fclose(fp);


	// The file already exists, show a message box to confirm the overwrite
	int nResult = Menu_MessageBox("Confirmation","The preset already exists. Overwrite?", LMB_YESNO);
	if( nResult == MBR_YES )
		return true;


	// No overwrite
	return false;
}

}; // namespace DeprecatedGUI
