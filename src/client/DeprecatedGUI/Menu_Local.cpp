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


#define DEFAULT_LOADING_TIME 500

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


bool	bGameSettings = false;
bool    bWeaponRest = false;


///////////////////
// Initialize the local menu
void Menu_LocalInitialize()
{
	tMenu->iMenuType = MNU_LOCAL;
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

	cLocalMenu.getWidget(ml_Playing)->setKeyboardNavigationOrder(1);
	cLocalMenu.getWidget(ml_Back)->setKeyboardNavigationOrder(1);
	cLocalMenu.getWidget(ml_Start)->setKeyboardNavigationOrder(1);

	cLocalMenu.SendMessage(ml_Playing,		LVS_ADDCOLUMN, "Playing", 24);
	cLocalMenu.SendMessage(ml_Playing,		LVS_ADDCOLUMN, "", 300 - gfxGame.bmpTeamColours[0].get()->w - 50);
	cLocalMenu.SendMessage(ml_Playing,		LVS_ADDCOLUMN, "", (DWORD)-1);

	cLocalMenu.SendMessage(ml_Playing,		LVM_SETOLDSTYLE, (DWORD)1, 0);

	cLocalMenu.SendMessage(ml_PlayerList,	LVS_ADDCOLUMN, "Players", 24);
	cLocalMenu.SendMessage(ml_PlayerList,	LVS_ADDCOLUMN, "", 60);

	cLocalMenu.SendMessage(ml_PlayerList,		LVM_SETOLDSTYLE, (DWORD)0, 0);

	for(Iterator<CGameMode* const&>::Ref i = GameModeIterator(); i->isValid(); i->next()) {
		cLocalMenu.SendMessage(ml_Gametype,    CBS_ADDITEM, i->get()->Name(), GetGameModeIndex(i->get()));
	}		
    cLocalMenu.SendMessage(ml_Gametype,    CBM_SETCURSEL, GetGameModeIndex(tLXOptions->tGameInfo.gameMode), 0);

	// Add players to player/playing lists
	Menu_LocalAddProfiles();

	// Fill the level list
	CCombobox* cbLevel = (CCombobox *)cLocalMenu.getWidget(ml_LevelList);
	Menu_FillLevelList( cbLevel, true);
	cbLevel->setCurItem(cbLevel->getSIndexItem(tLXOptions->tGameInfo.sMapFile));
	Menu_LocalShowMinimap(true);

	// Fill in the mod list
	CCombobox* cbMod = (CCombobox *)cLocalMenu.getWidget(ml_ModName);
	Menu_Local_FillModList( cbMod );
	cbMod->setCurItem(cbMod->getSIndexItem(tLXOptions->tGameInfo.sModDir));

}

//////////////
// Shutdown
void Menu_LocalShutdown()
{
	if (bGameSettings)
		Menu_GameSettingsShutdown();
	if (bWeaponRest)
		Menu_WeaponsRestrictionsShutdown();


	// Save the level and mod
	if (tLXOptions)  {
		cLocalMenu.SendMessage(ml_LevelList,CBS_GETCURSINDEX, &tLXOptions->tGameInfo.sMapFile, 0);
		cLocalMenu.SendMessage(ml_ModName,CBS_GETCURSINDEX, &tLXOptions->tGameInfo.sModDir, 0);
	}

	cLocalMenu.Shutdown();
}


///////////////////
// Local frame
void Menu_LocalFrame()
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
		if(it) tLXOptions->tGameInfo.sModDir = it->sIndex;

		// Fill in the mod list
		Menu_Local_FillModList( cbMod );
		cbMod->setCurItem(cbMod->getSIndexItem(tLXOptions->tGameInfo.sModDir));

		// Fill in the levels list
		CCombobox* cbLevel = (CCombobox *)cLocalMenu.getWidget(ml_LevelList);
		const cb_item_t *item = cbLevel->getItem( cbLevel->getSelectedIndex() );
		if (item)
			tLXOptions->tGameInfo.sMapFile = item->sIndex;
		Menu_FillLevelList( cbLevel, true);
		cbLevel->setCurItem(cbLevel->getSIndexItem(tLXOptions->tGameInfo.sMapFile));

		// Reload the minimap
		//if (tLXOptions->tGameInfo.sMapFile != "_random_")
		Menu_LocalShowMinimap(true);
	}


    // Was the mouse clicked on the map section
    if( Mouse->Up & SDL_BUTTON(1) ) {
    	// TODO: hardcoded sizes here
        if( MouseInRect(136,132,128,96) )
	    	// TODO: why are we redrawing the minimap here?
            Menu_LocalShowMinimap(true);
    }


	ev = cLocalMenu.Process();
	cLocalMenu.Draw(VideoPostProcessor::videoSurface());

	if(ev) {
		int changeTeamPlayerId = -1;

		switch(ev->iControlID) {
			// Back
			case ml_Back:
				if(ev->iEventMsg == BTN_CLICKED) {

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
				if(ev->iEventMsg == BTN_CLICKED) {
					PlaySoundSample(sfxGeneral.smpClick);

					// Start the game
					Menu_LocalStartGame();
				}
				break;

			// Player list
			case ml_PlayerList:
				if(ev->iEventMsg == LV_DOUBLECLK || ev->iEventMsg == LV_RIGHTCLK || ev->iEventMsg == LV_ENTER) {
					Menu_LocalAddPlaying();
				}
				break;

			// Playing list
			case ml_Playing:
				if (ev->iEventMsg == LV_DOUBLECLK || ev->iEventMsg == LV_RIGHTCLK || ev->iEventMsg == LV_ENTER) {
					if (!Menu_IsKeyboardNavigationUsed() || tLXOptions->tGameInfo.gameMode->GameTeams() <= 1)
						Menu_LocalRemovePlaying();
				}

				lv = (CListview *)cLocalMenu.getWidget(ml_Playing);

				if (tLXOptions->tGameInfo.gameMode->GameTeams() > 1 && ev->iEventMsg == LV_WIDGETEVENT) {
					// If the team colour item was clicked on, change it
					ev = lv->getWidgetEvent();
					if (ev->cWidget->getType() == wid_Image && ev->iEventMsg == IMG_CLICK)  {
						changeTeamPlayerId = ev->iControlID;
					}
					if (Menu_IsKeyboardNavigationUsed()) {
						changeTeamPlayerId = lv->getCurIndex();
					}
				}

				if (changeTeamPlayerId >= 0) {
					lv_item_t *it = lv->getItem(changeTeamPlayerId);
					lv_subitem_t *sub = lv->getSubItem(it, 2);

					if(sub) {
						sub->iExtra++;
						sub->iExtra %= 4;

						// Change the image
						((CImage *)sub->tWidget)->Change(DynDrawFromSurface(gfxGame.bmpTeamColours[sub->iExtra]));

						tMenu->sLocalPlayers[changeTeamPlayerId].setTeam(sub->iExtra);
						tMenu->sLocalPlayers[changeTeamPlayerId].getProfile()->iTeam = sub->iExtra;
					}

					tMenu->sLocalPlayers[changeTeamPlayerId].ChangeGraphics(tLXOptions->tGameInfo.gameMode->GeneralGameType());

					// Reload the skin
					sub = lv->getSubItem(it, 0);
					if (sub)  {
						sub->bmpImage = tMenu->sLocalPlayers[changeTeamPlayerId].getPicimg();
					}
				}
				break;


			// Game type
			case ml_Gametype:
				if(ev->iEventMsg == CMB_CHANGED) {
					tLXOptions->tGameInfo.gameMode = GameMode((GameModeIndex)cLocalMenu.SendMessage(ml_Gametype, CBM_GETCURINDEX, (DWORD)0, 0));

					// Go through the items and enable/disable the team flags and update worm graphics
					bool teams_on = tLXOptions->tGameInfo.gameMode->GameTeams() > 1;
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
							tMenu->sLocalPlayers[it->iIndex].ChangeGraphics(tLXOptions->tGameInfo.gameMode->GeneralGameType());
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
				if(ev->iEventMsg == BTN_CLICKED) {

					cLocalMenu.Draw( tMenu->bmpBuffer.get() );

					bGameSettings = true;
					Menu_GameSettings();
				}
				break;


            // Weapons Restrictions button
            case ml_WeaponOptions:
                if( ev->iEventMsg == BTN_CLICKED ) {

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

	if(index < 0 || index >= MAX_PLAYERS) {
		errors << "Menu_LocalAddPlaying: invalid index = " << index << endl;
		return;
	}
	
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
	tMenu->sLocalPlayers[index].ChangeGraphics(tLXOptions->tGameInfo.gameMode->GeneralGameType());


	// Add the item
	CImage *img = new CImage(gfxGame.bmpTeamColours[tMenu->sLocalPlayers[index].getTeam()]);
	if (img)  {
		img->setID(index);
		img->setRedrawMenu(false);
	} else
		warnings << "Cannot load teamcolor image" << endl;
	lv = (CListview *)cLocalMenu.getWidget(ml_Playing);
	lv->AddItem("",index,tLX->clListView);
	lv->AddSubitem(LVS_IMAGE, "", tMenu->sLocalPlayers[index].getPicimg(), NULL);
	lv->AddSubitem(LVS_TEXT, ply->sName, (DynDrawIntf*)NULL, NULL);
	lv->AddSubitem(LVS_WIDGET, "", (DynDrawIntf*)NULL, img);

	// If we're in deathmatch, make the team colour invisible
	lv_subitem_t *sub = lv->getSubItem(lv->getLastItem(), 2);
	if(sub) {
		if(tLXOptions->tGameInfo.gameMode->GameTeams() <= 1)
			sub->bVisible = false;
		sub->iExtra = 0;
	} else
		warnings << "Strange: did not found teamcolor subitem" << endl;
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
		lv->AddSubitem(LVS_TEXT, ply->sName, (DynDrawIntf*)NULL, NULL);
	}
}


///////////////////
// Add the profiles to the players list
void Menu_LocalAddProfiles()
{
	profile_t *p = GetProfiles();

	for(; p; p=p->tNext) {
		cLocalMenu.SendMessage( ml_PlayerList, LVS_ADDITEM, "", p->iID);
		//cLocalMenu.SendMessage( ml_PlayerList, LVS_ADDSUBITEM, (DWORD)p->bmpWorm, LVS_IMAGE); // TODO: 64bit unsafe (pointer cast)
		//cLocalMenu.SendMessage( ml_PlayerList, LVS_ADDSUBITEM, p->sName, LVS_TEXT);
		CListview * w = (CListview *) cLocalMenu.getWidget(ml_PlayerList);
		w->AddSubitem( LVS_IMAGE, "", p->cSkin.getPreview(), NULL );
		w->AddSubitem( LVS_TEXT, p->sName, (DynDrawIntf*)NULL, NULL );
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

    //tGameInfo.sMapRandom.bUsed = false;

	// Draw a background over the old minimap
	//DrawImageAdv(tMenu->bmpBuffer, tMenu->bmpMainBack, 126,132,126,132,128,96);

	// Load the map
    if( bReload ) {
		/*
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
        */

		// Load the file
		if(map.Load("levels/" + buf)) {
			// Draw the minimap
			if(map.GetMiniMap().get())
				DrawImage(tMenu->bmpMiniMapBuffer.get(), map.GetMiniMap(), 0,0);
			else
				DrawRectFill(tMenu->bmpMiniMapBuffer.get(), 0, 0, tMenu->bmpMiniMapBuffer->w, tMenu->bmpMiniMapBuffer->h, Color());
        }
    }

	// Update the screen
    DrawImage(tMenu->bmpBuffer.get(), tMenu->bmpMiniMapBuffer, 136,132);
	DrawImageAdv(VideoPostProcessor::videoSurface(), tMenu->bmpBuffer, 130,130,130,130,140,110);
}


///////////////////
// Start a local game
void Menu_LocalStartGame()
{
	// Level
	cLocalMenu.SendMessage(ml_LevelList, CBS_GETCURSINDEX, &tLXOptions->tGameInfo.sMapFile, 0);
	cLocalMenu.SendMessage(ml_LevelList, CBS_GETCURNAME, &tLXOptions->tGameInfo.sMapName, 0);


	//
	// Players
	//
	CListview *lv_playing = (CListview *)cLocalMenu.getWidget(ml_Playing);

	if (lv_playing->getItemCount() == 0) {
		Menu_MessageBox("Too few players", "You have to select at least one player.");
		return;
	}
	
	if(lv_playing->getItemCount() > MAX_PLAYERS) {
		Menu_MessageBox("Too many players",
						"You have selected " + itoa(lv_playing->getItemCount()) + " players "
						"but only " + itoa(MAX_PLAYERS) + " players are possible.");
		return;
	}
	
	tLX->iGameType = GME_LOCAL;

	if(! cClient->Initialize() )
	{
		errors << "Could not initialize client" << endl;
		return;
	}

	if(!cServer->StartServer()) {
		errors << "Could not start server" << endl;
		return;
	}
	
    int count = 0;

    // Add the human players onto the list
    for(lv_item_t* item = lv_playing->getItems(); item != NULL; item = item->tNext) {
    	int i = item->iIndex;
		if(tMenu->sLocalPlayers[i].isUsed() && tMenu->sLocalPlayers[i].getProfile() && tMenu->sLocalPlayers[i].getProfile()->iType == PRF_HUMAN->toInt()) {
			tMenu->sLocalPlayers[i].getProfile()->iTeam = tMenu->sLocalPlayers[i].getTeam();
			cClient->getLocalWormProfiles()[count++] = tMenu->sLocalPlayers[i].getProfile();
        }
    }

    // Add the unhuman players onto the list
    for(lv_item_t* item = lv_playing->getItems(); item != NULL; item = item->tNext) {
    	int i = item->iIndex;
		if(tMenu->sLocalPlayers[i].isUsed() && tMenu->sLocalPlayers[i].getProfile() && tMenu->sLocalPlayers[i].getProfile()->iType != PRF_HUMAN->toInt()) {
			tMenu->sLocalPlayers[i].getProfile()->iTeam = tMenu->sLocalPlayers[i].getTeam();
			cClient->getLocalWormProfiles()[count++] = tMenu->sLocalPlayers[i].getProfile();
        }
    }
    
    cClient->setNumWorms(count);

	// Can't start a game with no-one playing
	if(count == 0)
		return;

	// Save the current level in the options
	cLocalMenu.SendMessage(ml_LevelList, CBS_GETCURSINDEX, &tLXOptions->tGameInfo.sMapFile, 0);

	//
	// Game Info
	//
	tLXOptions->tGameInfo.gameMode = GameMode((GameModeIndex)cLocalMenu.SendMessage(ml_Gametype, CBM_GETCURINDEX, (DWORD)0, 0));

    //tLXOptions->sServerPassword = ""; // TODO: we have set this, why? it overwrites the password which is very annoying

    // Get the mod name
	cb_item_t *it = (cb_item_t *)cLocalMenu.SendMessage(ml_ModName,CBM_GETCURITEM,(DWORD)0,0); // TODO: 64bit unsafe (pointer cast)
    if(it) {
        tLXOptions->tGameInfo.sModName = it->sName;
		tLXOptions->tGameInfo.sModDir = it->sIndex;
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
	tLX->iGameType = GME_LOCAL;

	// Tell the client to connect to the server
	cClient->Connect("127.0.0.1:" + itoa(cServer->getPort()));

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

		if(tMenu->sLocalPlayers[i].getProfile()->iType == PRF_HUMAN->toInt())
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
		if(p->iType == PRF_HUMAN->toInt() && hmncount >= 2)
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
	FindFiles(adder,"",false,FM_DIR);
	
	cb->setCurSIndexItem(tLXOptions->tGameInfo.sModDir);
}




/*
=======================

	 Game Settings

 For both local & host

=======================
*/

//short			GameTabPane = 0;
CGuiLayout		cGameSettings;
//CGuiLayout		cGeneralSettings;
//CGuiLayout		cBonusSettings;

// Game Settings
enum {
	gs_Ok,
	gs_Default,
	gs_AdvancedLevel,
	gs_AdvancedLevelLabel,
	
	gs_FeaturesList,
	gs_FeaturesListLabel,
	
};

static void initFeaturesList(CListview* l);

///////////////////
// Initialize the game settings
void Menu_GameSettings()
{
	//GameTabPane = 0;
	// Setup the buffer
	Menu_DrawBox(tMenu->bmpBuffer.get(), 80,120, 560,460);
	DrawRectFillA(tMenu->bmpBuffer.get(), 82,122, 558,458, tLX->clDialogBackground, 245);

	Menu_RedrawMouse(true);


	// Shutdowns all 3 following instances
	Menu_GameSettingsShutdown();

	cGameSettings.Initialize();
	//cGeneralSettings.Initialize();

	// Keep text, it's the window text - the rest you can easily figure out by yourself.
	cGameSettings.Add( new CLabel("Game Settings", tLX->clNormalLabel),		    -1,	        280,135, 0, 0);

	// Game settings, stuff on each pane.

	cGameSettings.Add( new CButton(BUT_OK, DeprecatedGUI::tMenu->bmpButtons),	    gs_Ok,      180,435, 40,15);
    cGameSettings.Add( new CButton(BUT_DEFAULT, DeprecatedGUI::tMenu->bmpButtons), gs_Default, 390,435, 80,15);

	cGameSettings.Add( new CSlider(__AdvancedLevelType_Count - 1, 0, tLXOptions->iAdvancedLevelLimit), gs_AdvancedLevel, 365, 155, 80,15);
	cGameSettings.Add( new CLabel("Detail Level:", tLX->clNormalLabel), -1, 285, 155, 70, 15);
	float warningCoeff = CLAMP((float)tLXOptions->iAdvancedLevelLimit / (__AdvancedLevelType_Count - 1), 0.0f, 1.0f);
	cGameSettings.Add( new CLabel(AdvancedLevelShortDescription((AdvancedLevel)tLXOptions->iAdvancedLevelLimit), tLX->clNormalLabel * (1.0f - warningCoeff) + tLX->clError * warningCoeff), gs_AdvancedLevelLabel, 450, 155, 70, 15);

	CListview* features = new CListview();
	cGameSettings.Add( features, gs_FeaturesList, 95, 170, 450, 205);

	features->setDrawBorder(true);
	features->setRedrawMenu(false);
	features->setShowSelect(false);
	features->setOldStyle(true);
	features->subItemsAreAligned() = true;
	features->setMouseOverEventEnabled(true);
	
	int maxWidth = 0; // Width of the widest item in this column + some space
	CScriptableVars::const_iterator upper_bound = CScriptableVars::upper_bound("GameOptions.");
	for( CScriptableVars::const_iterator it = CScriptableVars::lower_bound("GameOptions."); it != upper_bound; it++ ) 
	{
		if( tLX->cFont.GetWidth(it->second.shortDesc) > maxWidth )
			maxWidth = tLX->cFont.GetWidth(it->second.shortDesc);
	}
	
	features->AddColumn("", maxWidth + 10); 
	features->AddColumn("", 190); 
	
	initFeaturesList(features);

	cGameSettings.Add( new CLabel("", tLX->clNormalLabel), gs_FeaturesListLabel, 95, 390, 450, 40);
}

// Features listview

static void addFeautureListGroupHeading(CListview* l, GameInfoGroup group) {
	if( l->getItemCount() > 0 )
		l->AddItem("", l->getNumItems(), tLX->clNormalLabel); // Empty line
	l->AddItem(GameInfoGroupDescriptions[group][0], l->getNumItems(), tLX->clHeading);
	l->AddSubitem(LVS_TEXT, std::string("--- ") + GameInfoGroupDescriptions[group][0] + " ---" + 
				  (tLXOptions->iGameInfoGroupsShown[group] ? " [-]" : " [+]"), (DynDrawIntf*)NULL, NULL);	
}

static int getListItemGroupInfoNr(const std::string& sindex) {
	for( int group = 0; group < GIG_Size; group++ )
		if( sindex == GameInfoGroupDescriptions[group][0] )
			return group;
	return -1;
}
	
	
static void updateFeatureListItemColor(lv_item_t* item) {
	lv_subitem_t* sub = item->tSubitems; if(!sub) return;
	if(((CListview*)cGameSettings.getWidget(gs_FeaturesList))->getMouseOverSIndex() == item->sIndex) {
		sub->iColour = tLX->clMouseOver;
		return;
	}
	
	int group = getListItemGroupInfoNr(item->sIndex);
	if(group >= 0) sub->iColour = tLX->clHeading;
	else {
		// Note: commented out because I am not sure if it is that nice
		//RegisteredVar* var = CScriptableVars::GetVar(item->sIndex);
		float warningCoeff = 0.0f; //CLAMP((float)var->advancedLevel / (__AdvancedLevelType_Count - 1), 0.0f, 1.0f);
		sub->iColour = tLX->clNormalLabel * (1.0f - warningCoeff) + tLX->clError * warningCoeff;
	}
}


static void initFeaturesList(CListview* l)
{
	l->Clear();
	for( GameInfoGroup group = (GameInfoGroup)0; group < GIG_Size; group = (GameInfoGroup)(group + 1) )
	{
		if( group == GIG_GameModeSpecific_Start )
			continue;
		if( group > GIG_GameModeSpecific_Start && 
			tLXOptions->tGameInfo.gameMode->getGameInfoGroupInOptions() != group )
			continue;

		size_t countGroupOpts = 0;
		CScriptableVars::const_iterator upper_bound = CScriptableVars::upper_bound("GameOptions.");
		for( CScriptableVars::const_iterator it = CScriptableVars::lower_bound("GameOptions."); it != upper_bound; it++ ) 
		{
			if( it->second.group != group ) continue;
			if( (int)it->second.advancedLevel > tLXOptions->iAdvancedLevelLimit ) continue;
			
			if( it->second.var.s == &tLXOptions->tGameInfo.sModDir || 
				it->second.var.s == &tLXOptions->tGameInfo.sMapFile ||
				it->first == "GameOptions.GameInfo.GameType" ||
				it->second.var.i == &tLXOptions->tGameInfo.iMaxPlayers )
				continue;	// We have nice comboboxes for them, skip them in the list
			
			if( tMenu && tMenu->iMenuType == MNU_LOCAL )
				if( it->second.var.b == &tLXOptions->tGameInfo.bAllowConnectDuringGame )
					continue;
			
			if( !tLXOptions->tGameInfo.gameMode || !tLXOptions->tGameInfo.gameMode->isTeamGame() ) {
				if( it->second.var.i == &tLXOptions->iRandomTeamForNewWorm ) continue;
				if( it->second.var.b == &tLXOptions->tGameInfo.bRespawnGroupTeams ) continue;
				if( it->second.var == &tLXOptions->tGameInfo.features[FT_TeamScoreLimit] ) continue;
				if( it->second.var == &tLXOptions->tGameInfo.features[FT_TeamkillDecreasesScore] ) continue;
				if( it->second.var == &tLXOptions->tGameInfo.features[FT_TeamInjure] ) continue;				
				if( it->second.var == &tLXOptions->tGameInfo.features[FT_TeamHit] ) continue;				
			}
			
			if(countGroupOpts == 0)
				addFeautureListGroupHeading(l, group);
			countGroupOpts++;

			if( ! tLXOptions->iGameInfoGroupsShown[group] )
				continue;
			
			lv_item_t * item = l->AddItem(it->first, l->getNumItems(), tLX->clNormalLabel);
			updateFeatureListItemColor(item);
			l->AddSubitem(LVS_TEXT, it->second.shortDesc, (DynDrawIntf*)NULL, NULL); 
			item->iHeight = 24; // So checkbox / textbox will fit okay

			if( it->second.var.type == SVT_BOOL )
			{
				CCheckbox * cb = new CCheckbox( * it->second.var.b );
				l->AddSubitem(LVS_WIDGET, "", (DynDrawIntf*)NULL, cb);
				cb->Create();
				cb->Setup(-1, 0, 0, 20, 20);
			}
			else
			{
				int textboxSize = 228;
				if( it->second.haveMinMax() )
				{
					int imin=0, imax=0;
					float fScale = 1.0f;
					int iVal = 0;
					if( it->second.var.type == SVT_FLOAT )
					{
						// Adding some small number to round it up correctly
						imin = int( float(it->second.min) *10.0f + 0.00001f );	// Scale them up
						imax = int( float(it->second.max) *10.0f + 0.00001f );
						iVal = int( (*it->second.var.f) * 10.0f + 0.00001f );
						fScale = 0.1f;
					} else {
						imin = it->second.min;
						imax = it->second.max;
						iVal = * it->second.var.i;
					}
					CSlider * sld = new CSlider( imax, imin, imin, false, 190, 0, tLX->clNormalLabel, fScale );
					CLAMP_DIRECT(iVal, sld->getMin(), sld->getMax() );
					sld->setValue(iVal);
					l->AddSubitem(LVS_WIDGET, "", (DynDrawIntf*)NULL, sld);
					sld->Create();
					sld->Setup(-1, 0, 0, 180, tLX->cFont.GetHeight());
					textboxSize = 40;
				}
				CTextbox * txt = new CTextbox();
				l->AddSubitem(LVS_WIDGET, "", (DynDrawIntf*)NULL, txt);
				txt->Create();
				txt->Setup(-1, 0, 0, textboxSize, tLX->cFont.GetHeight());
				if ((it->second.var.type == SVT_INT && it->second.var.isUnsigned && *it->second.var.i < 0) ||
					(it->second.var.type == SVT_FLOAT && it->second.var.isUnsigned && *it->second.var.f < 0))
					txt->setText("");  // Leave blank for infinite values
				else
					txt->setText( it->second.var.toString() );
			}
		}
	}
}


// Copy values from listview to features list
static void updateFeaturesList(CListview* l) 
{
	CScriptableVars::const_iterator upper_bound = CScriptableVars::upper_bound("GameOptions.");
	for( CScriptableVars::const_iterator it = CScriptableVars::lower_bound("GameOptions."); it != upper_bound; it++ ) 
	{
		if( it->second.group == GIG_Invalid ) continue;

		lv_item_t * item = l->getItem(it->first);
		if( ! item )
			continue;
		lv_subitem_t * si = item->tSubitems->tNext;
		if( ! si )
			continue;
		CWidget * w = si->tWidget;
		if( ! w )
			continue;
			
		if( it->second.var.type == SVT_BOOL )
		{
			if( w->getType() == wid_Checkbox )
			{
				* it->second.var.b = ((CCheckbox *)w)->getValue();
			}
		}
		else
		{
			if( w->getType() == wid_Textbox )
			{
				it->second.var.fromString( ((CTextbox *)w)->getText() );
			}
			if( w->getType() == wid_Slider && 
				si->tNext && si->tNext->tWidget && si->tNext->tWidget->getType() == wid_Textbox &&
				l->getWidgetEvent() && l->getWidgetEvent()->cWidget )
			{
				CSlider *slider = (CSlider *)w;
				CTextbox *textBox = (CTextbox *)si->tNext->tWidget;
					
				if( l->getWidgetEvent()->cWidget->getType() == wid_Slider ) // User moved slider - update textbox
				{
					int iVal = slider->getValue();
					if( it->second.var.type == SVT_INT )
					{
						* it->second.var.i = iVal;
						textBox->setText(itoa(iVal));
						if( it->second.var.isUnsigned && iVal < 0 )
							textBox->setText("");
					}
					if( it->second.var.type == SVT_FLOAT )
					{
						* it->second.var.f = iVal / 10.0f;
						textBox->setText(to_string<float>(iVal / 10.0f));
						if( it->second.var.isUnsigned && iVal < 0 )
							textBox->setText("");
					}
				}
				if( l->getWidgetEvent()->cWidget->getType() == wid_Textbox ) // User typed in textbox - update slider
				{
					it->second.var.fromString(textBox->getText());
					int iVal = 0;
					if( it->second.var.type == SVT_INT )
					{
						// Do not do min/max check on typed value, it's sole user responsibility if game crashes (though it should not)
						//CLAMP_DIRECT(* it->second.var.i, it->second.min.i, it->second.max.i );
						iVal = * it->second.var.i;
					}
					if( it->second.var.type == SVT_FLOAT )
					{
						// Do not do min/max check on typed value, it's sole user responsibility if game crashes (though it should not)
						//CLAMP_DIRECT(*it->second.var.f, it->second.min.f, it->second.max.f );
						iVal = int(* it->second.var.f * 10.0f);
					}
					CLAMP_DIRECT(iVal, slider->getMin(), slider->getMax() );
					slider->setValue(iVal);
				}
			}
		}
	}
	if( tLXOptions->tGameInfo.iLives < 0 )
		tLXOptions->tGameInfo.iLives = WRM_UNLIM;
}

/////////////
// Shutdown
void Menu_GameSettingsShutdown()
{
	cGameSettings.Shutdown();
}



///////////////////
// Game settings frame
// Returns whether of not we have finised with the game settings
bool Menu_GameSettings_Frame()
{
	gui_event_t *ev = NULL;

	ev = cGameSettings.Process();

	if(ev)
	{

		switch(ev->iControlID)
		{

			// OK, done
			case gs_Ok:
				if(ev->iEventMsg == BTN_CLICKED)
				{
					Menu_GameSettings_GrabInfo();
					Menu_GameSettingsShutdown();

					return true;
				}
				break;

            // Set the default values
            case gs_Default:
                if( ev->iEventMsg == BTN_CLICKED ) {
                    Menu_GameSettings_Default();
                }
                break;

			case gs_AdvancedLevel:
				if( ev->iEventMsg == SLD_CHANGE ) {
					tLXOptions->iAdvancedLevelLimit = ((CSlider*)cGameSettings.getWidget(gs_AdvancedLevel))->getValue();
					CListview* features = (CListview*)cGameSettings.getWidget(gs_FeaturesList);
					features->SaveScrollbarPos();
					initFeaturesList(features);
					features->RestoreScrollbarPos();

					CLabel* featuresLabel = (CLabel*)cGameSettings.getWidget(gs_FeaturesListLabel);
					float warningCoeff = CLAMP((float)tLXOptions->iAdvancedLevelLimit / (__AdvancedLevelType_Count - 1), 0.0f, 1.0f);
					featuresLabel->ChangeColour( tLX->clNormalLabel * (1.0f - warningCoeff) + tLX->clError * warningCoeff );
					featuresLabel->setText( splitStringWithNewLine(AdvancedLevelDescription((AdvancedLevel)tLXOptions->iAdvancedLevelLimit), (size_t)-1, 450, tLX->cFont) );		
					
					CLabel* advancenessLabel = (CLabel*)cGameSettings.getWidget(gs_AdvancedLevelLabel);
					advancenessLabel->setText( AdvancedLevelShortDescription((AdvancedLevel)tLXOptions->iAdvancedLevelLimit) );
					advancenessLabel->ChangeColour( tLX->clNormalLabel * (1.0f - warningCoeff) + tLX->clError * warningCoeff );
				}
				break;
				
			case gs_FeaturesList:
				CListview* features = (CListview*)ev->cWidget;
				if( ev->iEventMsg == LV_WIDGETEVENT )
				{
					updateFeaturesList(features);
				}
				if( ev->iEventMsg == LV_MOUSEOVER )
				{
					CLabel* featuresLabel = (CLabel*)cGameSettings.getWidget(gs_FeaturesListLabel);
					featuresLabel->ChangeColour( tLX->clNormalLabel );
					featuresLabel->setText( "" );
					for(lv_item_t* item = features->getItems(); item != NULL; item = item->tNext)
						updateFeatureListItemColor(item);
					if(	features->getMouseOverSIndex() != "" )
					{
						{
							lv_item_t* item = features->getItem(features->getMouseOverSIndex());
							if(item && item->tSubitems)
								item->tSubitems->iColour = tLX->clMouseOver;
						}
						std::string desc;
						{
							int group = getListItemGroupInfoNr(features->getMouseOverSIndex());
							if(group >= 0)
								desc = GameInfoGroupDescriptions[group][1];
						}
						if(desc == "") {
							RegisteredVar* var = CScriptableVars::GetVar( features->getMouseOverSIndex() );
							if(var) {
								desc = var->longDesc;
								float warningCoeff = CLAMP((float)var->advancedLevel / (__AdvancedLevelType_Count - 1), 0.0f, 1.0f);
								featuresLabel->ChangeColour( tLX->clNormalLabel * (1.0f - warningCoeff) + tLX->clError * warningCoeff );
							}
						}
						featuresLabel->setText( splitStringWithNewLine(desc, (size_t)-1, 450, tLX->cFont) );
					}
				}
				if( ev->iEventMsg == LV_CHANGED )
				{
					for( int group = 0; group < GIG_Size; group++ )
						if( features->getMouseOverSIndex() == GameInfoGroupDescriptions[group][0] ) {
							tLXOptions->iGameInfoGroupsShown[group] = ! tLXOptions->iGameInfoGroupsShown[group];
							features->SaveScrollbarPos();
							initFeaturesList(features);
							features->RestoreScrollbarPos();
							break;
						}
				}
				break;
		}

	}

	DrawImageAdv(VideoPostProcessor::videoSurface(), tMenu->bmpBuffer, 120,150, 120,150, 400,300);
	cGameSettings.Draw(VideoPostProcessor::videoSurface());

	// Draw the mouse
	DrawCursor(VideoPostProcessor::videoSurface());

	return false;
}


///////////////////
// Grab the game settings info
void Menu_GameSettings_GrabInfo()
{
	// Stub
}


///////////////////
// Set the default game settings info
void Menu_GameSettings_Default()
{
	CScriptableVars::const_iterator upper_bound = CScriptableVars::upper_bound("GameOptions.");
	for( CScriptableVars::const_iterator it = CScriptableVars::lower_bound("GameOptions."); it != upper_bound; it++ ) 
	{
		if( it->second.group == GIG_Invalid ) continue;
		
		if( it->first == "GameOptions.GameInfo.ModName" || 
			it->first == "GameOptions.GameInfo.LevelName" ||
			it->first == "GameOptions.GameInfo.GameType" )
			continue;	// We have nice comboboxes for them, skip them in the list

		it->second.var.setDefault();
    }

    CListview * features = (CListview *)cGameSettings.getWidget(gs_FeaturesList);
    features->Clear();
	initFeaturesList(features);
    
}

}; // namespace DeprecatedGUI
