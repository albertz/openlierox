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
#include "AuxLib.h"
#include "Graphics.h"
#include "CClient.h"
#include "Menu.h"
#include "CListview.h"
#include "GfxPrimitives.h"
#include "FindFile.h"
#include "StringUtils.h"
#include "CButton.h"
#include "CLabel.h"
#include "CImage.h"
#include "CTextbox.h"
#include "CSlider.h"
#include "CCheckbox.h"
#include "CMediaPlayer.h"
#include "CTextButton.h"


CGuiLayout cLocalMenu;

// Gui layout id's
enum {
	ml_Back=0,
	ml_Start,
	ml_Playing,
	ml_PlayerList,
	ml_LevelList,
	ml_Gametype,
	ml_ModName,
	ml_GameSettings,
    ml_WeaponOptions
};

int iGameType = GMT_DEATHMATCH;

bool	bGameSettings = false;
bool    bWeaponRest = false;


///////////////////
// Initialize the local menu
void Menu_LocalInitialize(void)
{
	tMenu->iMenuType = MNU_LOCAL;
	iGameType = GMT_DEATHMATCH;
	bGameSettings = false;

	// Create the buffer
	DrawImage(tMenu->bmpBuffer.get(),tMenu->bmpMainBack_common,0,0);
	Menu_DrawSubTitleAdv(tMenu->bmpBuffer.get(),SUB_LOCAL,15);
	if (tMenu->tFrontendInfo.bPageBoxes)
		Menu_DrawBox(tMenu->bmpBuffer.get(), 15,100, 625, 465);

    // Minimap box
	Menu_DrawBox(tMenu->bmpBuffer.get(), 133,129, 266, 230);

	Menu_RedrawMouse(true);

	// Shutdown any previous data from before
	cLocalMenu.Shutdown();
	cLocalMenu.Initialize();

	cLocalMenu.Add( new CButton(BUT_BACK, tMenu->bmpButtons), ml_Back, 27,440, 50,15);
	cLocalMenu.Add( new CButton(BUT_START, tMenu->bmpButtons), ml_Start, 555,440, 60,15);
	cLocalMenu.Add( new CListview(), ml_PlayerList,  410,115, 200, 126);
	cLocalMenu.Add( new CListview(), ml_Playing,     310,250, 300, 185);

	cLocalMenu.Add( new CButton(BUT_GAMESETTINGS, tMenu->bmpButtons), ml_GameSettings, 27, 310, 170,15);
    cLocalMenu.Add( new CButton(BUT_WEAPONOPTIONS,tMenu->bmpButtons), ml_WeaponOptions,27, 335, 185,15);

	cLocalMenu.Add( new CLabel("Mod",tLX->clNormalLabel),	    -1,         30,  284, 0,   0);
	cLocalMenu.Add( new CCombobox(),				ml_ModName,    120, 283, 170, 17);
	cLocalMenu.Add( new CLabel("Game type",tLX->clNormalLabel),	-1,         30,  260, 0,   0);
	cLocalMenu.Add( new CCombobox(),				ml_Gametype,   120, 259, 170, 17);
    cLocalMenu.Add( new CLabel("Level",tLX->clNormalLabel),	    -1,         30,  236, 0,   0);
	cLocalMenu.Add( new CCombobox(),				ml_LevelList,  120, 235, 170, 17);

	cLocalMenu.SendMessage(ml_Playing,		LVS_ADDCOLUMN, "Playing", 24);
	cLocalMenu.SendMessage(ml_Playing,		LVS_ADDCOLUMN, "", 300 - gfxGame.bmpTeamColours[0].get()->w - 50);
	cLocalMenu.SendMessage(ml_Playing,		LVS_ADDCOLUMN, "", (DWORD)-1);

	cLocalMenu.SendMessage(ml_Playing,		LVM_SETOLDSTYLE, (DWORD)1, 0);

	cLocalMenu.SendMessage(ml_PlayerList,	LVS_ADDCOLUMN, "Players", 24);
	cLocalMenu.SendMessage(ml_PlayerList,	LVS_ADDCOLUMN, "", 60);

	cLocalMenu.SendMessage(ml_PlayerList,		LVM_SETOLDSTYLE, (DWORD)0, 0);

	cLocalMenu.SendMessage(ml_Gametype,    CBS_ADDITEM, "Deathmatch", GMT_DEATHMATCH);
	cLocalMenu.SendMessage(ml_Gametype,    CBS_ADDITEM, "Team Deathmatch", GMT_TEAMDEATH);
	cLocalMenu.SendMessage(ml_Gametype,    CBS_ADDITEM, "Tag", GMT_TAG);
    cLocalMenu.SendMessage(ml_Gametype,    CBS_ADDITEM, "Demolitions", GMT_DEMOLITION);
	cLocalMenu.SendMessage(ml_Gametype,	   CBS_ADDITEM, "VIP", GMT_VIP);
	cLocalMenu.SendMessage(ml_Gametype,	   CBS_ADDITEM, "Capture the Flag", GMT_CTF);
//	cLocalMenu.SendMessage(ml_Gametype,	   CBS_ADDITEM, "Teams Capture the Flag", GMT_TEAMCTF);

	/*cLocalMenu.SendMessage(ml_Gametype,    CBS_ADDITEM,  "Capture the flag",1);
	cLocalMenu.SendMessage(ml_Gametype,    CBS_ADDITEM,   "Flag hunt",1);*/

    cLocalMenu.SendMessage(ml_Gametype,    CBM_SETCURSEL, tLXOptions->tGameinfo.nGameType, 0);
    iGameType = tLXOptions->tGameinfo.nGameType;

	// Add players to player/playing lists
	Menu_LocalAddProfiles();

	// Fill the level list
	CCombobox* cbLevel = (CCombobox *)cLocalMenu.getWidget(ml_LevelList);
	Menu_FillLevelList( cbLevel, true);
	cbLevel->setCurItem(cbLevel->getSIndexItem(tLXOptions->tGameinfo.sMapFilename));
	Menu_LocalShowMinimap(true);

	// Fill in the mod list
	CCombobox* cbMod = (CCombobox *)cLocalMenu.getWidget(ml_ModName);
	Menu_Local_FillModList( cbMod );
	cbMod->setCurItem(cbMod->getSIndexItem(tLXOptions->tGameinfo.szModName));

	// Fill in some game details
	tGameInfo.iLoadingTimes = tLXOptions->tGameinfo.iLoadingTime;
	tGameInfo.iLives = tLXOptions->tGameinfo.iLives;
	tGameInfo.iKillLimit = tLXOptions->tGameinfo.iKillLimit;
	tGameInfo.bBonusesOn = tLXOptions->tGameinfo.bBonusesOn;
	tGameInfo.bShowBonusName = tLXOptions->tGameinfo.bShowBonusName;

}

//////////////
// Shutdown
void Menu_LocalShutdown(void)
{
	if (bGameSettings)
		Menu_GameSettingsShutdown();
	if (bWeaponRest)
		Menu_WeaponsRestrictionsShutdown();


	// Save the level and mod
	if (tLXOptions)  {
		cLocalMenu.SendMessage(ml_LevelList,CBS_GETCURSINDEX, &tLXOptions->tGameinfo.sMapFilename, 0);
		cLocalMenu.SendMessage(ml_ModName,CBS_GETCURSINDEX, &tLXOptions->tGameinfo.szModName, 0);
	}

	cLocalMenu.Shutdown();
}


///////////////////
// Local frame
void Menu_LocalFrame(void)
{
	gui_event_t *ev = NULL;
	mouse_t *Mouse = GetMouse();
	CListview *lv;

    // Game Settings
	if(bGameSettings) {
		if(Menu_GameSettings_Frame()) {
			// Re-do the buffer
			DrawImage(tMenu->bmpBuffer.get(),tMenu->bmpMainBack_common,0,0);
	        Menu_DrawSubTitleAdv(tMenu->bmpBuffer.get(),SUB_LOCAL,15);
			if (tMenu->tFrontendInfo.bPageBoxes)
				Menu_DrawBox(tMenu->bmpBuffer.get(), 15,100, 625, 465);
	        Menu_DrawBox(tMenu->bmpBuffer.get(), 133,129, 266, 230);

			Menu_RedrawMouse(true);
			Menu_LocalShowMinimap(false);

			bGameSettings = false;
		}
		return;
	}


    // Weapons Restrictions
    if(bWeaponRest) {
		if(Menu_WeaponsRestrictions_Frame()) {
			// Re-do the buffer
			DrawImage(tMenu->bmpBuffer.get(),tMenu->bmpMainBack_common,0,0);
	        Menu_DrawSubTitleAdv(tMenu->bmpBuffer.get(),SUB_LOCAL,15);
			if (tMenu->tFrontendInfo.bPageBoxes)
				Menu_DrawBox(tMenu->bmpBuffer.get(), 15,100, 625, 465);
	        Menu_DrawBox(tMenu->bmpBuffer.get(), 133,129, 266, 230);

			Menu_RedrawMouse(true);
			Menu_LocalShowMinimap(false);

			bWeaponRest = false;
		}
		return;
	}

	// Reload the list if user switches back to the game
	// Do not reload when a dialog is open
	if (bActivated)  {
		// Get the mod name
		CCombobox* cbMod = (CCombobox *)cLocalMenu.getWidget(ml_ModName);
		const cb_item_t *it = cbMod->getItem(cbMod->getSelectedIndex());
		if(it) tLXOptions->tGameinfo.szModName = it->sIndex;

		// Fill in the mod list
		Menu_Local_FillModList( cbMod );
		cbMod->setCurItem(cbMod->getSIndexItem(tLXOptions->tGameinfo.szModName));

		// Fill in the levels list
		CCombobox* cbLevel = (CCombobox *)cLocalMenu.getWidget(ml_LevelList);
		tLXOptions->tGameinfo.sMapFilename = cbLevel->getItem( cbLevel->getSelectedIndex() )->sIndex;
		Menu_FillLevelList( cbLevel, true);
		cbLevel->setCurItem(cbLevel->getSIndexItem(tLXOptions->tGameinfo.sMapFilename));

		// Reload the minimap
		if (tLXOptions->tGameinfo.sMapFilename != "_random_")
			Menu_LocalShowMinimap(true);
	}


    // Was the mouse clicked on the map section
    if( Mouse->Up & SDL_BUTTON(1) ) {
    	// TODO: hardcoded sizes here
        if( MouseInRect(136,132,128,96) )
	    	// TODO: why are we redrawing the minimap here?
            Menu_LocalShowMinimap(true);
    }


#ifdef WITH_MEDIAPLAYER
	if (!cMediaPlayer.GetDrawPlayer())
#endif
		ev = cLocalMenu.Process();
	cLocalMenu.Draw(VideoPostProcessor::videoSurface());

	if(ev) {

		switch(ev->iControlID) {
			// Back
			case ml_Back:
				if(ev->iEventMsg == BTN_MOUSEUP) {

					// Shutdown
					Menu_LocalShutdown();

					// Leave
					PlaySoundSample(sfxGeneral.smpClick);
					Menu_MainInitialize();
					return;
				}
				break;

			// Start
			case ml_Start:
				if(ev->iEventMsg == BTN_MOUSEUP) {
					PlaySoundSample(sfxGeneral.smpClick);

					// Start the game
					Menu_LocalStartGame();
				}
				break;

			// Player list
			case ml_PlayerList:
				if(ev->iEventMsg == LV_DOUBLECLK || ev->iEventMsg == LV_RIGHTCLK) {
					Menu_LocalAddPlaying();
				}
				break;

			// Playing list
			case ml_Playing:
				if(ev->iEventMsg == LV_DOUBLECLK || ev->iEventMsg == LV_RIGHTCLK) {
					Menu_LocalRemovePlaying();
				}


				if(ev->iEventMsg == LV_WIDGETEVENT && (iGameType == GMT_TEAMDEATH || iGameType == GMT_VIP)) {

					// If the team colour item was clicked on, change it
					lv = (CListview *)cLocalMenu.getWidget(ml_Playing);

					ev = lv->getWidgetEvent();
					if (ev->cWidget->getType() == wid_Image && ev->iEventMsg == IMG_CLICK)  {
						lv_item_t *it = lv->getItem(ev->iControlID);
						lv_subitem_t *sub = lv->getSubItem(it, 2);

						if(sub) {
							sub->iExtra++;
							sub->iExtra %= 4;

							// Change the image
							((CImage *)ev->cWidget)->Change(gfxGame.bmpTeamColours[sub->iExtra]);
						}

						tMenu->sLocalPlayers[ev->iControlID].setTeam(sub->iExtra);
						tMenu->sLocalPlayers[ev->iControlID].getProfile()->iTeam = sub->iExtra;
						tMenu->sLocalPlayers[ev->iControlID].ChangeGraphics(iGameType);

						// Reload the skin
						sub = lv->getSubItem(it, 0);
						if (sub)  {
							sub->bmpImage = tMenu->sLocalPlayers[ev->iControlID].getPicimg();
						}
					}
				}
				break;


			// Game type
			case ml_Gametype:
				if(ev->iEventMsg == CMB_CHANGED) {
					iGameType = cLocalMenu.SendMessage(ml_Gametype, CBM_GETCURINDEX, (DWORD)0, 0);

					// Go through the items and enable/disable the team flags and update worm graphics
					bool teams_on = (iGameType == GMT_TEAMDEATH) || (iGameType == GMT_VIP);
					CListview *lv = (CListview *)cLocalMenu.getWidget(ml_Playing);
					lv_item_t *it = lv->getItems();
					lv_subitem_t *sub = NULL;
					for (; it; it = it->tNext)  {
						sub = lv->getSubItem(it, 2);
						if (sub)
							sub->bVisible = teams_on;

						// Update the skin
						sub = lv->getSubItem(it, 0);
						if (sub)  {
							tMenu->sLocalPlayers[it->iIndex].ChangeGraphics(iGameType);
							sub->bmpImage = tMenu->sLocalPlayers[it->iIndex].getPicimg();
						}

					}
				}
				break;

			// Level list combo box
			case ml_LevelList:
				if(ev->iEventMsg == CMB_CHANGED) {

					Menu_LocalShowMinimap(true);
				}
				break;


			// Game settings button
			case ml_GameSettings:
				if(ev->iEventMsg == BTN_MOUSEUP) {

					cLocalMenu.Draw( tMenu->bmpBuffer.get() );

					bGameSettings = true;
					Menu_GameSettings();
				}
				break;


            // Weapons Restrictions button
            case ml_WeaponOptions:
                if( ev->iEventMsg == BTN_MOUSEUP ) {

					cLocalMenu.Draw( tMenu->bmpBuffer.get() );

                    // Get the current mod
					cb_item_t *it = (cb_item_t *)cLocalMenu.SendMessage(ml_ModName,CBM_GETCURITEM,(DWORD)0,0); // TODO: 64bit unsafe (pointer cast)
                    if(it) {

					    bWeaponRest = true;
					    Menu_WeaponsRestrictions(it->sIndex);
                    }
                }
                break;

		}
	}

	// Draw the mouse
	DrawCursor(VideoPostProcessor::videoSurface());
}

//////////////////
// Move a worm from player list to playing list
void Menu_LocalAddPlaying(int index)
{
	// Add the item to the players list
	CListview *lv = (CListview *)cLocalMenu.getWidget(ml_PlayerList);
	if (index < 0)
		index = lv->getCurIndex();

	// Check if we have enough room for another player
	if(!Menu_LocalCheckPlaying(index))
		return;


	// Remove the item from the list
	lv->RemoveItem(index);

	profile_t *ply = FindProfile(index);

	if(!ply)
		return;

	// Add a player onto the local players list
	if (!tMenu->sLocalPlayers[index].isUsed())  {  // If the players persists from previous game, don't reset its team
											// and other game details
		tMenu->sLocalPlayers[index].setUsed(true);
		tMenu->sLocalPlayers[index].setHealth(0);
		tMenu->sLocalPlayers[index].setTeam(0);
	}

	// Reload the graphics in case the gametype has changed
	tMenu->sLocalPlayers[index].setProfile(ply);
	tMenu->sLocalPlayers[index].setSkin(ply->cSkin);
	tMenu->sLocalPlayers[index].ChangeGraphics(iGameType);


	// Add the item
	CImage *img = new CImage(gfxGame.bmpTeamColours[tMenu->sLocalPlayers[index].getTeam()]);
	if (img)  {
		img->setID(index);
		img->setRedrawMenu(false);
	} else
		printf("WARNING: cannot load teamcolor image\n");
	lv = (CListview *)cLocalMenu.getWidget(ml_Playing);
	lv->AddItem("",index,tLX->clListView);
	lv->AddSubitem(LVS_IMAGE, "", tMenu->sLocalPlayers[index].getPicimg(), NULL);
	lv->AddSubitem(LVS_TEXT, ply->sName, NULL, NULL);
	lv->AddSubitem(LVS_WIDGET, "", NULL, img);

	// If we're in deathmatch, make the team colour invisible
	lv_subitem_t *sub = lv->getSubItem(lv->getLastItem(), 2);
	if(sub) {
		if(iGameType != GMT_TEAMDEATH && iGameType != GMT_VIP)
			sub->bVisible = false;
		sub->iExtra = 0;
	} else
		printf("WARNING: strange: did not found teamcolor subitem\n");
}

//////////////////
// Move a player from playing list back to player list
void Menu_LocalRemovePlaying(int index)
{
	// Put the player back into the players list
	CListview *lv = (CListview *)cLocalMenu.getWidget(ml_Playing);
	if (index < 0)
		index = lv->getCurIndex();

	// Remove the item from the list
	lv->RemoveItem(index);
	tMenu->sLocalPlayers[index].setUsed(false);

	profile_t *ply = FindProfile(index);

	// Add the player into the players list
	if(ply) {
		ply->cSkin.RemoveColorization();
		lv = (CListview *)cLocalMenu.getWidget(ml_PlayerList);
		lv->AddItem("", index, tLX->clListView);
		lv->AddSubitem(LVS_IMAGE, "", ply->cSkin.getPreview(), NULL);
		lv->AddSubitem(LVS_TEXT, ply->sName, NULL, NULL);
	}
}


///////////////////
// Add the profiles to the players list
void Menu_LocalAddProfiles(void)
{
	profile_t *p = GetProfiles();

	for(; p; p=p->tNext) {
		cLocalMenu.SendMessage( ml_PlayerList, LVS_ADDITEM, "", p->iID);
		//cLocalMenu.SendMessage( ml_PlayerList, LVS_ADDSUBITEM, (DWORD)p->bmpWorm, LVS_IMAGE); // TODO: 64bit unsafe (pointer cast)
		//cLocalMenu.SendMessage( ml_PlayerList, LVS_ADDSUBITEM, p->sName, LVS_TEXT);
		CListview * w = (CListview *) cLocalMenu.getWidget(ml_PlayerList);
		w->AddSubitem( LVS_IMAGE, "", p->cSkin.getPreview(), NULL );
		w->AddSubitem( LVS_TEXT, p->sName, NULL, NULL );
	}

	// Add players from previous game to playing list
	// TODO: add the players in the same order as last time
	// it's a bit annoying if you add 2 human players and they switch always after a game the role
	if (tMenu->sLocalPlayers != NULL)  {
		for (int i=0; i < MAX_PLAYERS; i++)  {
			if (tMenu->sLocalPlayers[i].isUsed())  {
				Menu_LocalAddPlaying(i);
			}
		}
	}


}


///////////////////
// Show the minimap
void Menu_LocalShowMinimap(bool bReload)
{
	// TODO: optimize or recode this!
	CMap map;
	std::string buf;

	cLocalMenu.SendMessage(ml_LevelList, CBS_GETCURSINDEX, &buf, 0);

    tGameInfo.sMapRandom.bUsed = false;

	// Draw a background over the old minimap
	//DrawImageAdv(tMenu->bmpBuffer, tMenu->bmpMainBack, 126,132,126,132,128,96);

	// Load the map
    if( bReload ) {

        // Create a random map
        if( buf == "_random_" ) {
            if( map.Create(504,350,map.findRandomTheme(), 128, 96) ) {
				map.TileMap();
			    map.ApplyRandom();

                // Free any old random map object list
                if( tGameInfo.sMapRandom.psObjects ) {
                    delete[] tGameInfo.sMapRandom.psObjects;
                    tGameInfo.sMapRandom.psObjects = NULL;
                }

                // Copy the layout
                maprandom_t *psRand = map.getRandomLayout();
                tGameInfo.sMapRandom = *psRand;
                tGameInfo.sMapRandom.bUsed = true;

                // Copy the objects, not link
                tGameInfo.sMapRandom.psObjects = new object_t[tGameInfo.sMapRandom.nNumObjects];
                if( tGameInfo.sMapRandom.psObjects ) {
                    for( int i=0; i<tGameInfo.sMapRandom.nNumObjects; i++ ) {
                        tGameInfo.sMapRandom.psObjects[i] = psRand->psObjects[i];
                    }
                }

                // Draw the minimap
		        DrawImage(tMenu->bmpMiniMapBuffer.get(), map.GetMiniMap(), 0,0);
		        map.Shutdown();
            }

        } else {
            // Load the file
            if(map.Load("levels/" + buf)) {

		        // Draw the minimap
		        DrawImage(tMenu->bmpMiniMapBuffer.get(), map.GetMiniMap(), 0,0);
	        }
        }
    }

	// Update the screen
    DrawImage(tMenu->bmpBuffer.get(), tMenu->bmpMiniMapBuffer, 136,132);
	DrawImageAdv(VideoPostProcessor::videoSurface(), tMenu->bmpBuffer, 130,130,130,130,140,110);
}


///////////////////
// Start a local game
void Menu_LocalStartGame(void)
{
	// Level
	cLocalMenu.SendMessage(ml_LevelList, CBS_GETCURSINDEX, &tGameInfo.sMapFile, 0);
	cLocalMenu.SendMessage(ml_LevelList, CBS_GETCURNAME, &tGameInfo.sMapName, 0);


	//
	// Players
	//
	CListview *lv_playing = (CListview *)cLocalMenu.getWidget(ml_Playing);

    // Calculate the number of players
    tGameInfo.iNumPlayers = lv_playing->getNumItems();

	// Can't start a game with no-one playing
	if(tGameInfo.iNumPlayers == 0)
		return;

    int count = 0;

    // Add the human players onto the list
    for(lv_item_t* item = lv_playing->getItems(); item != NULL; item = item->tNext) {
    	int i = item->iIndex;
		if(tMenu->sLocalPlayers[i].isUsed() && tMenu->sLocalPlayers[i].getProfile() && tMenu->sLocalPlayers[i].getProfile()->iType == PRF_HUMAN) {
			tMenu->sLocalPlayers[i].getProfile()->iTeam = tMenu->sLocalPlayers[i].getTeam();
			tGameInfo.cPlayers[count++] = tMenu->sLocalPlayers[i].getProfile();
        }
    }

    // Add the unhuman players onto the list
    for(lv_item_t* item = lv_playing->getItems(); item != NULL; item = item->tNext) {
    	int i = item->iIndex;
		if(tMenu->sLocalPlayers[i].isUsed() && tMenu->sLocalPlayers[i].getProfile() && tMenu->sLocalPlayers[i].getProfile()->iType != PRF_HUMAN) {
			tMenu->sLocalPlayers[i].getProfile()->iTeam = tMenu->sLocalPlayers[i].getTeam();
			tGameInfo.cPlayers[count++] = tMenu->sLocalPlayers[i].getProfile();
        }
    }

	// Save the current level in the options
	cLocalMenu.SendMessage(ml_LevelList, CBS_GETCURSINDEX, &tLXOptions->tGameinfo.sMapFilename, 0);

	//
	// Game Info
	//
	tGameInfo.iGameMode = cLocalMenu.SendMessage(ml_Gametype, CBM_GETCURINDEX, (DWORD)0, 0);
    tLXOptions->tGameinfo.nGameType = tGameInfo.iGameMode;

    tGameInfo.sPassword = "";


    // Get the mod name
	cb_item_t *it = (cb_item_t *)cLocalMenu.SendMessage(ml_ModName,CBM_GETCURITEM,(DWORD)0,0); // TODO: 64bit unsafe (pointer cast)
    if(it) {
        tGameInfo.sModName = it->sName;
		tGameInfo.sModDir = it->sIndex;
        tLXOptions->tGameinfo.szModName = it->sIndex;
    } else {

		// Couldn't find a mod to load
		cLocalMenu.Draw(tMenu->bmpBuffer.get());
		Menu_MessageBox("Error","Could not find a mod to load!", LMB_OK);
		DrawImage(tMenu->bmpBuffer.get(),tMenu->bmpMainBack_common,0,0);
        Menu_DrawBox(tMenu->bmpBuffer.get(), 15,130, 625, 465);
		Menu_DrawSubTitle(tMenu->bmpBuffer.get(),SUB_LOCAL);
		Menu_RedrawMouse(true);
		return;
	}

	*bGame = true;
	tMenu->bMenuRunning = false;
	tGameInfo.iGameType = GME_LOCAL;

	cLocalMenu.Shutdown();
}

///////////////////
// Check if we can add another player to the list
bool Menu_LocalCheckPlaying(int index)
{
	uint		plycount = 0;
	uint		hmncount = 0;
	profile_t	*p;
	CListview *lv_playing = (CListview *)cLocalMenu.getWidget(ml_Playing);


	// Go through the playing list
    for(lv_item_t* item = lv_playing->getItems(); item != NULL; item = item->tNext) {
    	int i = item->iIndex;
        if(!tMenu->sLocalPlayers[i].isUsed())
            continue;

		if(tMenu->sLocalPlayers[i].getProfile()->iType == PRF_HUMAN)
			hmncount++;
		plycount++;
	}

	p = FindProfile(index);

	// TODO: make it possible to use more than 2 local players

	// Check if there is too many players
	if(plycount >= MAX_PLAYERS)
		return false;

	// Check if there is too many human players (MAX: 2)
	if(p) {
		if(p->iType == PRF_HUMAN && hmncount >= 2)
			return false;
	}

	return true;
}


	class ModAdder { public:
		CCombobox* combobox;
		ModAdder(CCombobox* cb_) : combobox(cb_) {}
		bool operator() (const std::string& abs_filename) {
			size_t sep = findLastPathSep(abs_filename);
			if(sep != std::string::npos) {
				std::string name;
				if(CGameScript::CheckFile(abs_filename, name, true))
					combobox->addItem(abs_filename.substr(sep+1), name);
			}

			return true;
		}
	};


///////////////////
// Fill in the mod list
// HINT: also used by Menu_Net_Host
// TODO: move to MenuSystem
void Menu_Local_FillModList( CCombobox *cb )
{
	// Find all directories in the the lierox
	cb->clear();
	cb->setSorted(SORT_ASC);
	cb->setUnique(true);
	ModAdder adder(cb);
	FindFiles(adder,".",false,FM_DIR);
	
	cb->setCurSIndexItem(tLXOptions->tGameinfo.szModName);
}




/*
=======================

	 Game Settings

 For both local & host

=======================
*/

enum
{
	gs_Ok,
	gs_Default,
	gs_btnGenTab,
	gs_btnBonusTab
};

enum
{
	gs_GenTab = 0,
	gs_BonusTab
};

short			GameTabPane = 0;
CGuiLayout		cGameSettings;
CGuiLayout		cGeneralSettings;
CGuiLayout		cBonusSettings;

// Game Settings
enum {
	gs_Lives,
	gs_MaxKills,
	gs_LoadingTime,
	gs_LoadingTimeLabel,
	gs_TimeLimit,
	gs_RespawnTime,
	gs_RespawnInWaves,
	gs_RespawnGroupTeams,
	gs_SuicideDecreasesScore,
	gs_GroupTeamScore,
	gs_EmptyWeaponsOnRespawn,

	gs_Bonuses,
	gs_ShowBonusNames,
	gs_BonusSpawnTime,
	gs_BonusLifeTime,
	gs_HealthToWeaponChance,
	gs_HealthChance,
	gs_WeaponChance

};



///////////////////
// Initialize the game settings
void Menu_GameSettings(void)
{
	GameTabPane = 0;
	// Setup the buffer
	Menu_DrawBox(tMenu->bmpBuffer.get(), 120,130, 520,470);
	DrawRectFillA(tMenu->bmpBuffer.get(), 122,132, 518,468, tLX->clDialogBackground, 200);

	Menu_RedrawMouse(true);


	// Shutdowns all 3 following instances
	Menu_GameSettingsShutdown();

	cGameSettings.Initialize();
	cGeneralSettings.Initialize();
	cBonusSettings.Initialize();

	// Keep text, it's the window text - the rest you can easily figure out by yourself.
	cGameSettings.Add( new CLabel("Game Settings", tLX->clNormalLabel),		    -1,	        280,145, 0, 0);

	// Game settings, stuff on each pane.

	cGameSettings.Add( new CButton(BUT_GENERAL, tMenu->bmpButtons),	gs_btnGenTab, 240, 165, 0, 0);
	cGameSettings.Add( new CButton(BUT_BONUS, tMenu->bmpButtons),	gs_btnBonusTab, 340, 165, 0, 0);
	cGameSettings.Add( new CButton(BUT_OK, tMenu->bmpButtons),	    gs_Ok,      220,445, 40,15);
    cGameSettings.Add( new CButton(BUT_DEFAULT, tMenu->bmpButtons), gs_Default, 350,445, 80,15);




	// General settings, general stuffies!
	cGeneralSettings.Add( new CLabel("Lives", tLX->clNormalLabel),				    -1,	        140,200, 0, 0);
	cGeneralSettings.Add( new CTextbox(),										gs_Lives,		300,197, 30,tLX->cFont.GetHeight());
	cGeneralSettings.SendMessage(gs_Lives,TXM_SETMAX,6,0);
	if(tLXOptions->tGameinfo.iLives >= 0)
		cGeneralSettings.SendMessage(gs_Lives, TXS_SETTEXT, itoa(tLXOptions->tGameinfo.iLives), 0);

	cGeneralSettings.Add( new CLabel("Max Kills", tLX->clNormalLabel),			    -1,	        140,230, 0, 0);
	cGeneralSettings.Add( new CTextbox(),										gs_MaxKills,	300,227, 30,tLX->cFont.GetHeight());
	cGeneralSettings.SendMessage(gs_MaxKills,TXM_SETMAX,6,0);
	if(tLXOptions->tGameinfo.iKillLimit >= 0)
		cGeneralSettings.SendMessage(gs_MaxKills, TXS_SETTEXT, itoa(tLXOptions->tGameinfo.iKillLimit), 0);

	cGeneralSettings.Add( new CLabel("Loading Time", tLX->clNormalLabel),		    -1,	        140,260, 0, 0);
	cGeneralSettings.Add( new CSlider(500),									gs_LoadingTime,	295,257, 160,20);
	cGeneralSettings.Add( new CLabel("", tLX->clNormalLabel),					gs_LoadingTimeLabel, 470, 260, 0, 0);
	cGeneralSettings.SendMessage(gs_LoadingTime, SLM_SETVALUE, tLXOptions->tGameinfo.iLoadingTime, 0);


	cGeneralSettings.Add( new CLabel("Time limit, minutes", tLX->clNormalLabel),	-1,	        140,300, 0, 0);
	cGeneralSettings.Add( new CTextbox(),										gs_TimeLimit,	300,297, 30,tLX->cFont.GetHeight());
	cGeneralSettings.SendMessage(gs_TimeLimit,TXM_SETMAX,3,0);
	if(tLXOptions->tGameinfo.fTimeLimit > 0)
		cGeneralSettings.SendMessage(gs_TimeLimit, TXS_SETTEXT, ftoa(tLXOptions->tGameinfo.fTimeLimit), 0);

	cGeneralSettings.Add( new CLabel("Respawn time, seconds", tLX->clNormalLabel),	-1,	        140,340, 0, 0);
	cGeneralSettings.Add( new CTextbox(),										gs_RespawnTime,	300,337, 30,tLX->cFont.GetHeight());
	cGeneralSettings.SendMessage(gs_RespawnTime,TXM_SETMAX,3,0);
	cGeneralSettings.SendMessage(gs_RespawnTime, TXS_SETTEXT, ftoa(tLXOptions->tGameinfo.fRespawnTime), 0);

	cGeneralSettings.Add( new CLabel("Empty weapons", tLX->clNormalLabel),			-1,	        350,330, 0, 0);
	cGeneralSettings.Add( new CLabel("when respawning", tLX->clNormalLabel),		-1,	        350,345, 0, 0);
	cGeneralSettings.Add( new CCheckbox(tLXOptions->tGameinfo.bEmptyWeaponsOnRespawn),	gs_EmptyWeaponsOnRespawn, 470,337,17,17);

	cGeneralSettings.Add( new CLabel("Respawn in waves", tLX->clNormalLabel),		-1,	        140,370, 0, 0);
	cGeneralSettings.Add( new CCheckbox(tLXOptions->tGameinfo.bRespawnInWaves),	gs_RespawnInWaves,    300,367,17,17);

	cGeneralSettings.Add( new CLabel("Group teams", tLX->clNormalLabel),			-1,         350,370, 0, 0);
	cGeneralSettings.Add( new CCheckbox(tLXOptions->tGameinfo.bRespawnGroupTeams),	gs_RespawnGroupTeams, 470,367,17,17);

	cGeneralSettings.Add( new CLabel("Suicide or teamkill", tLX->clNormalLabel),	-1,         140,390, 0, 0);
	cGeneralSettings.Add( new CLabel("decreases score", tLX->clNormalLabel),		-1,         140,405, 0, 0);
	cGeneralSettings.Add( new CCheckbox(tLXOptions->tGameinfo.bSuicideDecreasesScore),	gs_SuicideDecreasesScore,    300,397,17,17);

	cGeneralSettings.Add( new CLabel("Group team score", tLX->clNormalLabel),		-1,         350,400, 0, 0);
	cGeneralSettings.Add( new CCheckbox(tLXOptions->tGameinfo.bGroupTeamScore),	gs_GroupTeamScore, 470,397,17,17);




	// Bonus settings - Bonus stuffies!
	cBonusSettings.Add( new CLabel("Bonuses", tLX->clNormalLabel),			    -1,	        140,200, 0, 0);
	cBonusSettings.Add( new CCheckbox(tLXOptions->tGameinfo.bBonusesOn),		gs_Bonuses,		300,197,17,17);

	cBonusSettings.Add( new CLabel("Bonus spawn time", tLX->clNormalLabel),		-1,	        140,230, 0, 0);
	cBonusSettings.Add( new CTextbox(),										gs_BonusSpawnTime,	300,227, 30,tLX->cFont.GetHeight());
	cBonusSettings.SendMessage(gs_BonusSpawnTime, TXS_SETTEXT, ftoa(tLXOptions->tGameinfo.fBonusFreq), 0);

	cBonusSettings.Add( new CLabel("Show Bonus names", tLX->clNormalLabel),	    -1,			140,260, 0, 0);
	cBonusSettings.Add( new CCheckbox(tLXOptions->tGameinfo.bShowBonusName),	gs_ShowBonusNames, 300,257,17,17);

	cBonusSettings.Add( new CLabel("Bonus life time", tLX->clNormalLabel),	    -1,	        140,290, 0, 0);
	cBonusSettings.Add( new CTextbox(),										gs_BonusLifeTime,	300,287, 30,tLX->cFont.GetHeight());
	cBonusSettings.SendMessage(gs_BonusLifeTime, TXS_SETTEXT, ftoa(tLXOptions->tGameinfo.fBonusLife), 0);

	cBonusSettings.Add( new CLabel("Bonuses", tLX->clNormalLabel),				-1,			290,320, 0, 0);
	cBonusSettings.Add( new CLabel("Health", tLX->clNormalLabel),				-1,			146,340, 0, 0);
	cBonusSettings.Add( new CLabel("Weapons", tLX->clNormalLabel),				-1,			443,340, 0, 0);
	cBonusSettings.Add( new CLabel("", tLX->clNormalLabel),			gs_HealthChance,		153,360, 0, 0);
	cBonusSettings.Add( new CLabel("", tLX->clNormalLabel),			gs_WeaponChance,		453,360, 0, 0);
	cBonusSettings.Add( new CSlider(100),									gs_HealthToWeaponChance,	188,343, 250,12);
	cBonusSettings.SendMessage(gs_HealthToWeaponChance, SLM_SETVALUE, int(tLXOptions->tGameinfo.fBonusHealthToWeaponChance*100.0f), 0);


	int bonusChance = cBonusSettings.SendMessage(gs_HealthToWeaponChance, SLM_GETVALUE, 100, 0);
	std::string buf = itoa(100-bonusChance) + "%";
	cBonusSettings.SendMessage(gs_HealthChance, LBS_SETTEXT, buf, 0);
	buf = itoa(bonusChance) + "%";
	cBonusSettings.SendMessage(gs_WeaponChance, LBS_SETTEXT, buf, 0);


}

/////////////
// Shutdown
void Menu_GameSettingsShutdown(void)
{
	cGameSettings.Shutdown();
	cGeneralSettings.Shutdown();
	cBonusSettings.Shutdown();
}


///////////////////
// Game settings frame
// Returns whether of not we have finised with the game settings
bool Menu_GameSettings_Frame(void)
{
	gui_event_t *ev = NULL;

	DrawImageAdv(VideoPostProcessor::videoSurface(), tMenu->bmpBuffer, 120,150, 120,150, 400,300);

	cGameSettings.Draw(VideoPostProcessor::videoSurface());


#ifdef WITH_MEDIAPLAYER
	if (!cMediaPlayer.GetDrawPlayer())
#endif

		ev = cGameSettings.Process();


	if(ev)
	{

		switch(ev->iControlID)
		{

			case gs_btnGenTab:
				if (ev->iEventMsg == BTN_MOUSEUP)
				{
					GameTabPane = gs_GenTab;
				}
				break;
			case gs_btnBonusTab:
				if (ev->iEventMsg == BTN_MOUSEUP)
				{
					GameTabPane = gs_BonusTab;
				}
				break;

			// OK, done
			case gs_Ok:
				if(ev->iEventMsg == BTN_MOUSEUP)
				{
					Menu_GameSettings_GrabInfo();
					Menu_GameSettingsShutdown();

					return true;
				}
				break;

            // Set the default values
            case gs_Default:
                if( ev->iEventMsg == BTN_MOUSEUP )
				{
                    Menu_GameSettings_Default();
                }
                break;
		}
	}


#ifdef WITH_MEDIAPLAYER
	if (!cMediaPlayer.GetDrawPlayer())
#endif

	if (GameTabPane == gs_GenTab)
	{
		cGeneralSettings.Draw(VideoPostProcessor::videoSurface());
		ev = cGeneralSettings.Process();
		if (ev)
		{
			switch (ev->iControlID)
			{

				case gs_RespawnInWaves:
					if( cGeneralSettings.SendMessage( gs_RespawnInWaves, CKM_GETCHECK, (DWORD)0, 0) == 0 )
						cGeneralSettings.SendMessage( gs_RespawnGroupTeams, CKM_SETCHECK, (DWORD)0, 0);
					break;

				case gs_RespawnGroupTeams:
					if( cGeneralSettings.SendMessage( gs_RespawnGroupTeams, CKM_GETCHECK, (DWORD)0, 0) != 0 )
						cGeneralSettings.SendMessage( gs_RespawnInWaves, CKM_SETCHECK, (DWORD)1, 0);
					break;
			}
		}

	// Set the value of the loading time label
	int l = cGeneralSettings.SendMessage(gs_LoadingTime, SLM_GETVALUE, 100, 0);
	static std::string lstr; lstr = itoa(l)+"%";
	cGeneralSettings.SendMessage(gs_LoadingTimeLabel, LBS_SETTEXT, lstr, 0);

	}

	if (GameTabPane == gs_BonusTab)
	{
		cBonusSettings.Draw(VideoPostProcessor::videoSurface());
		DrawRect (VideoPostProcessor::videoSurface(),140,337,500,377,tLX->clBoxLight);
		ev = cBonusSettings.Process();
		if (ev)
		{
			switch (ev->iControlID)
			{
				case gs_HealthToWeaponChance:
					int bonusChance = cBonusSettings.SendMessage(gs_HealthToWeaponChance, SLM_GETVALUE, 100, 0);
					std::string buf = itoa(100-bonusChance) + "%";
					cBonusSettings.SendMessage(gs_HealthChance, LBS_SETTEXT, buf, 0);
					buf = itoa(bonusChance) + "%";
					cBonusSettings.SendMessage(gs_WeaponChance, LBS_SETTEXT, buf, 0);
					break;
			}
		}
	}



	// Draw the mouse
	DrawCursor(VideoPostProcessor::videoSurface());

	return false;
}


///////////////////
// Grab the game settings info
void Menu_GameSettings_GrabInfo(void)
{
	static std::string buf;


	// Default to no setting
	tGameInfo.iLives = tLXOptions->tGameinfo.iLives = -2;
	tGameInfo.iKillLimit = tLXOptions->tGameinfo.iKillLimit = -1;
	tGameInfo.fTimeLimit = tLXOptions->tGameinfo.fTimeLimit = -1;
	tGameInfo.iTagLimit = tLXOptions->tGameinfo.iTagLimit = -1;
	tLXOptions->tGameinfo.fRespawnTime = 2.5;
	tGameInfo.bBonusesOn = true;
	tGameInfo.bShowBonusName = true;

	// Store the game info into the options structure as well



	// General

	tGameInfo.iLoadingTimes = tLXOptions->tGameinfo.iLoadingTime = cGeneralSettings.SendMessage(gs_LoadingTime, SLM_GETVALUE, 100, 0);

	cGeneralSettings.SendMessage(gs_Lives, TXS_GETTEXT, &buf, 0);
	if(buf != "")
		tGameInfo.iLives = tLXOptions->tGameinfo.iLives = atoi(buf);

	cGeneralSettings.SendMessage(gs_MaxKills, TXS_GETTEXT, &buf, 0);
	if(buf != "")
		tGameInfo.iKillLimit = tLXOptions->tGameinfo.iKillLimit = atoi(buf);

	cGeneralSettings.SendMessage(gs_TimeLimit, TXS_GETTEXT, &buf, 0);
	if(buf != "")
		tLXOptions->tGameinfo.fTimeLimit = tGameInfo.fTimeLimit  = atof(buf);

	cGeneralSettings.SendMessage(gs_RespawnTime, TXS_GETTEXT, &buf, 0);
	if(buf != "")
		tLXOptions->tGameinfo.fRespawnTime = atof(buf);

	tLXOptions->tGameinfo.bRespawnInWaves = cGeneralSettings.SendMessage( gs_RespawnInWaves, CKM_GETCHECK, (DWORD)0, 0) != 0;

	tLXOptions->tGameinfo.bRespawnGroupTeams = cGeneralSettings.SendMessage( gs_RespawnGroupTeams, CKM_GETCHECK, (DWORD)0, 0) != 0;

	tLXOptions->tGameinfo.bSuicideDecreasesScore = cGeneralSettings.SendMessage( gs_SuicideDecreasesScore, CKM_GETCHECK, (DWORD)0, 0) != 0;

	tLXOptions->tGameinfo.bGroupTeamScore = cGeneralSettings.SendMessage( gs_GroupTeamScore, CKM_GETCHECK, (DWORD)0, 0) != 0;

	tLXOptions->tGameinfo.bEmptyWeaponsOnRespawn = cGeneralSettings.SendMessage( gs_EmptyWeaponsOnRespawn, CKM_GETCHECK, (DWORD)0, 0) != 0;

	// Bonus
	cBonusSettings.SendMessage(gs_BonusSpawnTime, TXS_GETTEXT, &buf, 0);
	if(buf != "")
		tLXOptions->tGameinfo.fBonusFreq = atof(buf);

	cBonusSettings.SendMessage(gs_BonusLifeTime, TXS_GETTEXT, &buf, 0);
	if(buf != "")
		tLXOptions->tGameinfo.fBonusLife = atof(buf);

	tGameInfo.bBonusesOn = cBonusSettings.SendMessage( gs_Bonuses, CKM_GETCHECK, (DWORD)0, 0) != 0;
	tLXOptions->tGameinfo.bBonusesOn = tGameInfo.bBonusesOn;

	tGameInfo.bShowBonusName = cBonusSettings.SendMessage( gs_ShowBonusNames, CKM_GETCHECK, (DWORD)0, 0) != 0;
	tLXOptions->tGameinfo.bShowBonusName = tGameInfo.bShowBonusName;

	tLXOptions->tGameinfo.fBonusHealthToWeaponChance = float(cBonusSettings.SendMessage(gs_HealthToWeaponChance, SLM_GETVALUE, 100, 0)) / 100.0f ;
}


///////////////////
// Set the default game settings info
void Menu_GameSettings_Default(void)
{

	// General
    cGeneralSettings.SendMessage(gs_LoadingTime, SLM_SETVALUE, 100, 0);
    cGeneralSettings.SendMessage(gs_Lives, TXS_SETTEXT, "10", 0);
	cGeneralSettings.SendMessage(gs_MaxKills, TXS_SETTEXT, "", 0);
	cGeneralSettings.SendMessage(gs_TimeLimit, TXS_SETTEXT, "", 0);

	// Bonus
    cBonusSettings.SendMessage(gs_Bonuses, CKM_SETCHECK, (DWORD)1, 0);
    cBonusSettings.SendMessage(gs_ShowBonusNames, CKM_SETCHECK, (DWORD)1, 0);
}




/*
=======================

  Weapons Restrictions

 For both local & host

=======================
*/


CGuiLayout		cWeaponsRest;
CWpnRest        cWpnRestList;
CGameScript     *cWpnGameScript = NULL;

// Weapons Restrictions
enum {
	wr_Ok,
    wr_Scroll,
    wr_Reset,
	wr_ListBox,
	wr_Cancel,
    wr_Random,
	wr_Load,
	wr_Save
};


std::list<wpnrest_t *> tWeaponList;

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
    cWeaponsRest.Add( new CScrollbar(),                         wr_Scroll, 490,185, 14,230);

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
}

//////////////////
// Shutdown the weapon restrictions
void Menu_WeaponsRestrictionsShutdown(void)
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
bool Menu_WeaponsRestrictions_Frame(void)
{
	gui_event_t *ev = NULL;
	mouse_t *Mouse = GetMouse();
    //Uint32 blue = MakeColour(0,138,251);

    assert(cWpnGameScript);

    // State strings
    static const std::string    szStates[] = {"Enabled", "Bonus", "Banned"};

	DrawImageAdv(VideoPostProcessor::videoSurface(), tMenu->bmpBuffer, 120,150, 120,150, 400,300);

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
        Uint32 Colour = tLX->clNormalLabel;

        // If a mouse is over the line, highlight it
        if( Mouse->X > 150 && Mouse->X < 450 ) {
            if( Mouse->Y > y && Mouse->Y < y+20 ) {
                Colour = tLX->clMouseOver;

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
        tLX->cFont.Draw( VideoPostProcessor::videoSurface(), 400, y, Colour, szStates[(*it)->psLink->nState] );
	}

    // Adjust the scrollbar
    cWeaponsRest.SendMessage(wr_Scroll, SCM_SETITEMSPERBOX, 12, 0);
    cWeaponsRest.SendMessage(wr_Scroll, SCM_SETMIN, (DWORD)0, 0);
	if(tWeaponList.size() > 10)
		cWeaponsRest.SendMessage(wr_Scroll, SCM_SETMAX, tWeaponList.size() + 1, 0);
    else
        cWeaponsRest.SendMessage(wr_Scroll, SCM_SETMAX, (DWORD)0, 0);


#ifdef WITH_MEDIAPLAYER
	if (!cMediaPlayer.GetDrawPlayer())
#endif
		ev = cWeaponsRest.Process();
	cWeaponsRest.Draw(VideoPostProcessor::videoSurface());

	if(ev) {

		if(ev->iEventMsg == SDL_BUTTON_WHEELUP)  {
			CScrollbar *tScrollbar = (CScrollbar *)cWeaponsRest.getWidget(wr_Scroll);
			tScrollbar->MouseWheelUp(NULL);
		}

		if(ev->iEventMsg == SDL_BUTTON_WHEELDOWN)  {
			CScrollbar *tScrollbar = (CScrollbar *)cWeaponsRest.getWidget(wr_Scroll);
			tScrollbar->MouseWheelDown(NULL);
		}


		switch(ev->iControlID) {

			// OK, done
			case wr_Ok:
				if(ev->iEventMsg == BTN_MOUSEUP) {

					Menu_WeaponsRestrictionsShutdown();

					return true;
				}
				break;

            // Reset the list
            case wr_Reset:
                if( ev->iEventMsg == BTN_MOUSEUP ) {
                    cWpnRestList.cycleVisible(cWpnGameScript);
                }
                break;

            // Randomize the list
            case wr_Random:
                if(ev->iEventMsg == BTN_MOUSEUP) {
                    cWpnRestList.randomizeVisible(cWpnGameScript);
                }
                break;

            // Open the load dialog
            case wr_Load:
                if(ev->iEventMsg == BTN_MOUSEUP) {
                    Menu_WeaponPresets(false,&cWpnRestList);
                }
                break;

            // Open the save dialog
            case wr_Save:
                if(ev->iEventMsg == BTN_MOUSEUP) {
                    Menu_WeaponPresets(true,&cWpnRestList);
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

	class WeaponPresetsAdder { public:
		CListview* listview;
		WeaponPresetsAdder(CListview* lv_) : listview(lv_) {}
		inline bool operator() (const std::string& f) {
			if(stringcaseequal(GetFileExtension(f),"wps")) {
				std::string fname = GetBaseFilename(f);
				std::string name = fname.substr(0, fname.size() - 4); // remove the extension, the size calcing is safe here
				if(!listview->getItem(fname)) {
					listview->AddItem(fname,0,tLX->clListView);
					listview->AddSubitem(LVS_TEXT, name, NULL, NULL);
				}
			}
			return true;
		}
	};

void Menu_WeaponPresets(bool save, CWpnRest *wpnrest)
{
	if (!wpnrest)
		return;

	keyboard_t *kb = GetKeyboard();
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
	FindFiles(adder,"cfg/presets/",false,FM_REG);



	ProcessEvents();
	// TODO: make this event-based (don't check GetKeyboard() directly)
	while(!kb->KeyUp[SDLK_ESCAPE] && !quitloop && tMenu->bMenuRunning) {
		Menu_RedrawMouse(false);

		//DrawImageAdv(VideoPostProcessor::videoSurface(),tMenu->bmpBuffer, 170,150, 170,150, 300, 180);
		Menu_DrawBox(VideoPostProcessor::videoSurface(), 170, 150, 470, 330);
		DrawImageAdv(VideoPostProcessor::videoSurface(), tMenu->bmpMainBack_common, 172,152, 172,152, 297,177);
		DrawImageAdv(tMenu->bmpBuffer.get(), tMenu->bmpMainBack_common, 172,152, 172,152, 297,177);

		tLX->cFont.DrawCentre(VideoPostProcessor::videoSurface(), 320, 155, tLX->clNormalLabel, save ? "Save" : "Load");
		if (save)
			tLX->cFont.Draw(VideoPostProcessor::videoSurface(), 180,288,tLX->clNormalLabel,"Preset name");

#ifdef WITH_MEDIAPLAYER
		if (!cMediaPlayer.GetDrawPlayer())
#endif
			ev = cWpnPresets.Process();
		cWpnPresets.Draw(VideoPostProcessor::videoSurface());

		// Process the widgets
#ifdef WITH_MEDIAPLAYER
		if(ev && !cMediaPlayer.GetDrawPlayer()) {
#else
		if(ev)  {
#endif

			switch(ev->iControlID) {
				// Cancel
				case wp_Cancel:
					if(ev->iEventMsg == BTN_MOUSEUP) {
						PlaySoundSample(sfxGeneral.smpClick);
						quitloop = true;
					}
					break;

				// Presets list
				case wp_PresetList:
					if(ev->iEventMsg != LV_NONE) {
						t->setText( lv->getCurSIndex() );
					}
				break;
			}

			// OK and double click on listview
			if (ev->iControlID == wp_Ok || ev->iControlID == wp_PresetList)  {
				if((ev->iEventMsg == BTN_MOUSEUP && ev->iControlID == 1) || ev->iEventMsg == LV_DOUBLECLK) {

					// Play the sound only for OK button
					if (ev->iControlID == wp_Ok)
						PlaySoundSample(sfxGeneral.smpClick);

					// Don't process when nothing is selected
					if(t->getText().length() > 0) {

						quitloop = true;
						std::string buf;
						if(save) {

							// Save
							buf = std::string("cfg/presets/") + t->getText() + ".wps";

							// Check if it exists already. If so, ask user if they wanna overwrite
							if(Menu_WeaponPresetsOkSave(buf))
								wpnrest->saveList(buf);
							else
								quitloop = false;
						} else {

							// Load
							buf = std::string("cfg/presets/") + t->getText();
							wpnrest->loadList(buf);
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
		VideoPostProcessor::process();

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
void Menu_WeaponPresetsShutdown(void)
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

