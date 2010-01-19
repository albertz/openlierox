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
#include "game/Mod.h"
#include "IniReader.h"
#include "game/SinglePlayer.h"


namespace DeprecatedGUI {

CGuiLayout cLocalMenu;

// Gui layout id's
enum {
	ml_GameLabel,
	ml_Game,
		
	ml_Back,
	ml_Start,
	
	__ml_AutoRemoveBegin,
	
	ml_Playing,
	ml_PlayerList,
	ml_LevelList,
	ml_Gametype,
	ml_ModName,
	ml_GameSettings,
    ml_WeaponOptions,
	
	ml_LevelLabel,
	ml_ModLabel,
	ml_LevelDescription,
	ml_LevelNumber,
	
	__ml_LowerLimit,
};


bool	bGameSettings = false;
bool    bWeaponRest = false;
	
static void Menu_Local_InitCustomLevel() {
	// Minimap box
	Menu_DrawBox(tMenu->bmpBuffer.get(), 133,129, 266, 230);

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
	
static int selectedGameIndex() {
	CCombobox* cb = (CCombobox *)cLocalMenu.getWidget(ml_Game);
	if(cb == NULL) {
		errors << "Local menu: selectedGameIndex: game combobox not found" << endl;
		return -1;
	}
	
	const cb_item_t* item = cb->getSelectedItem();
	if(item == NULL) {
		errors << "Local menu: selectedGameIndex: game combobox has not selected anything" << endl;
		return -1;		
	}
	
	return from_string<int>(item->sIndex);
}

static std::string selectedGameName() {
	CCombobox* cb = (CCombobox *)cLocalMenu.getWidget(ml_Game);
	if(cb == NULL) {
		errors << "Local menu: selectedGameName: game combobox not found" << endl;
		return "";
	}
	
	const cb_item_t* item = cb->getSelectedItem();
	if(item == NULL) {
		errors << "Local menu: selectedGameName: game combobox has not selected anything" << endl;
		return "";		
	}
	
	return item->sName;
}
	
static void newGameListEntry(const std::string& game, int index) {
	((CCombobox *)cLocalMenu.getWidget(ml_Game)) ->addItem(-1, itoa(index), game);
}
	
static void setCurGameComboIndex(int index) {
	((CCombobox *)cLocalMenu.getWidget(ml_Game)) ->setCurSIndexItem(itoa(index));
}
	
static void fillGameList() {
	newGameListEntry("- Custom level/mod -", -1);
	
	struct Ini : public IniReader {
		int index;
		Ini() : IniReader("games/games.cfg"), index(0) {}
		bool OnNewSection (const std::string& section) {
			newGameListEntry(section, index);
			index++;
			return true;
		}
	} ini;
	
	if(!ini.Parse())
		warnings << "error while parsing games.cfg" << endl;
	
	setCurGameComboIndex(tLXOptions->iLocalPlayGame);
}	
	
static bool deleteWidgets(int index) {
	bool one = false;
	while(cLocalMenu.getWidget(index)) {
		cLocalMenu.removeWidget(index);
		one = true;
	}
	return one;
}
	
static void uninitCurrentGameMenu() {
	deleteWidgets(-1);
	for(int i = __ml_AutoRemoveBegin + 1; i < __ml_LowerLimit; ++i)
		deleteWidgets(i);
}

static const int boxw = 600;
static const int boxh = 150;
static const int boxx = (640 - boxw) / 2;
static const int boxy = 80;
	
static void redrawGamePreviewImage() {
	DrawImageAdv(tMenu->bmpBuffer.get(),tMenu->bmpMainBack_common, boxx, boxy, boxx, boxy, boxw, boxh);
	
	if(singlePlayerGame.image.get())
		DrawImageResampledAdv(tMenu->bmpBuffer.get(), singlePlayerGame.image, 0, 0, boxx, boxy, singlePlayerGame.image->w, singlePlayerGame.image->h, boxw, boxh);
}

static void updateGameLevel() {
	redrawGamePreviewImage();

	CLabel* lvlLb = (CLabel *)cLocalMenu.getWidget(ml_LevelLabel);
	if(lvlLb) lvlLb->setText(singlePlayerGame.levelInfo.name);

	CLabel* modLb = (CLabel *)cLocalMenu.getWidget(ml_ModLabel);
	if(modLb) modLb->setText(singlePlayerGame.modInfo.name);

	CBrowser *b = (CBrowser *)cLocalMenu.getWidget(ml_LevelDescription);
	if(b) b->LoadFromString(singlePlayerGame.description);
	
	CSlider* s = (CSlider*)cLocalMenu.getWidget(ml_LevelNumber);
	if(s) {
		s->setMin(1);
		s->setMax(singlePlayerGame.maxAllowedLevelForCurrentGame());
		s->setValue(singlePlayerGame.currentLevel);
	}
}
	
static void Menu_Local_InitGameMenu() {
	std::string game = selectedGameName();
	singlePlayerGame.setGame(game);
	
	// box for preview image
	Menu_DrawBox(tMenu->bmpBuffer.get(), boxx - 4, boxy - 4, boxx + boxw + 3, boxy + boxh + 3);
	
	cLocalMenu.Add( new CLabel("Level",tLX->clHeading),			-1,    120, boxy + boxh + 10, 0, 0);
	cLocalMenu.Add( new CLabel("",tLX->clNormalLabel),	ml_LevelLabel,   170, boxy + boxh + 10, 0, 0);
	cLocalMenu.Add( new CLabel("Mod",tLX->clHeading),			-1,	   300, boxy + boxh + 10, 0, 0);
	cLocalMenu.Add( new CLabel("",tLX->clNormalLabel),		ml_ModLabel,  350, boxy + boxh + 10, 0, 0);

	static const int browsery = boxy + boxh + 30;
	static const int bottombuttonsh = 50;
	cLocalMenu.Add( new CBrowser(),		ml_LevelDescription,	   30, boxy + boxh + 30, 640 - 2*30, 480 - browsery - bottombuttonsh);

	cLocalMenu.Add( new CLabel("Level number", tLX->clHeading),		-1,	   150, 445, 0, 0);
	cLocalMenu.Add( new CSlider(1,1,1,true,204,-3,tLX->clNormalLabel),		ml_LevelNumber,	   250, 445 + 2, 200, 10);
	
	updateGameLevel();
}
	
static void initCurrentGameMenu() {
	// Create the buffer
	DrawImage(tMenu->bmpBuffer.get(),tMenu->bmpMainBack_common,0,0);
	Menu_DrawSubTitleAdv(tMenu->bmpBuffer.get(),SUB_LOCAL,15);
	if (tMenu->tFrontendInfo.bPageBoxes)
		Menu_DrawBox(tMenu->bmpBuffer.get(), 15,100, 625, 465);

	if(tLXOptions->iLocalPlayGame < 0)
		Menu_Local_InitCustomLevel();
	else
		Menu_Local_InitGameMenu();
}
	
static void handleGameSwitch() {
	int newGame = selectedGameIndex();
	if(newGame != tLXOptions->iLocalPlayGame) {
		uninitCurrentGameMenu();
		tLXOptions->iLocalPlayGame = newGame;
		initCurrentGameMenu();
	}
}
	
///////////////////
// Initialize the local menu
void Menu_LocalInitialize()
{
	tMenu->iMenuType = MNU_LOCAL;
	bGameSettings = false;


	// Shutdown any previous data from before
	cLocalMenu.Shutdown();
	cLocalMenu.Initialize();

    cLocalMenu.Add( new CLabel("Game",tLX->clNormalLabel),	  ml_GameLabel,  180,  15, 0,   0);
	cLocalMenu.Add( new CCombobox(),	ml_Game,  250, 15, 170, 17);
	fillGameList();

	cLocalMenu.Add( new CButton(BUT_BACK, tMenu->bmpButtons), ml_Back, 27,440, 50,15);
	cLocalMenu.Add( new CButton(BUT_START, tMenu->bmpButtons), ml_Start, 555,440, 60,15);
	
	initCurrentGameMenu();
	
	
	Menu_RedrawMouse(true);
	cLocalMenu.Draw(VideoPostProcessor::videoSurface());
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
	if (tLXOptions->iLocalPlayGame < 0 && tLXOptions->bAutoFileCacheRefresh && bActivated)  {
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
    if( (tLXOptions->iLocalPlayGame < 0) && (Mouse->Up & SDL_BUTTON(1)) ) {
    	// TODO: hardcoded sizes here
        if( MouseInRect(136,132,128,96) )
	    	// TODO: why are we redrawing the minimap here?
            Menu_LocalShowMinimap(true);
    }


	ev = cLocalMenu.Process();
	cLocalMenu.Draw(VideoPostProcessor::videoSurface());

	if(ev) {

		switch(ev->iControlID) {
			case ml_Game:
				//if(ev->iEventMsg == BTN_CLICKED) {
				handleGameSwitch();
				//}
				break;

			case ml_LevelNumber:
				if(ev->iEventMsg == SLD_CHANGE) {
					CSlider *s = (CSlider *)cLocalMenu.getWidget(ml_LevelNumber);
					singlePlayerGame.setLevel( s->getValue() );
					updateGameLevel();
				}
				break;
								
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
				if(ev->iEventMsg == LV_DOUBLECLK || ev->iEventMsg == LV_RIGHTCLK) {
					Menu_LocalAddPlaying();
				}
				break;

			// Playing list
			case ml_Playing:
				if(ev->iEventMsg == LV_DOUBLECLK || ev->iEventMsg == LV_RIGHTCLK) {
					Menu_LocalRemovePlaying();
				}


				if(ev->iEventMsg == LV_WIDGETEVENT && tLXOptions->tGameInfo.gameMode->GameTeams() > 1) {

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
							((CImage *)ev->cWidget)->Change(DynDrawFromSurface(gfxGame.bmpTeamColours[sub->iExtra]));
						}

						tMenu->sLocalPlayers[ev->iControlID].setTeam(sub->iExtra);
						tMenu->sLocalPlayers[ev->iControlID].getProfile()->iTeam = sub->iExtra;
						tMenu->sLocalPlayers[ev->iControlID].ChangeGraphics(tLXOptions->tGameInfo.gameMode->GeneralGameType());

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

static bool Menu_LocalStartGame_CustomGame() {
	
	// Level
	cLocalMenu.SendMessage(ml_LevelList, CBS_GETCURSINDEX, &tLXOptions->tGameInfo.sMapFile, 0);
	cLocalMenu.SendMessage(ml_LevelList, CBS_GETCURNAME, &tLXOptions->tGameInfo.sMapName, 0);
	
	
	//
	// Players
	//
	CListview *lv_playing = (CListview *)cLocalMenu.getWidget(ml_Playing);
	
	if (lv_playing->getItemCount() == 0) {
		Menu_MessageBox("Too few players", "You have to select at least one player.");
		return false;
	}
	
	if(lv_playing->getItemCount() > MAX_PLAYERS) {
		Menu_MessageBox("Too many players",
						"You have selected " + itoa(lv_playing->getItemCount()) + " players "
						"but only " + itoa(MAX_PLAYERS) + " players are possible.");
		return false;
	}
	
	tLX->iGameType = GME_LOCAL;
	
	if(! cClient->Initialize() )
	{
		errors << "Could not initialize client" << endl;
		return false;
	}
	
	if(!cServer->StartServer()) {
		errors << "Could not start server" << endl;
		return false;
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
		return false;
	
	// Save the current level in the options
	cLocalMenu.SendMessage(ml_LevelList, CBS_GETCURSINDEX, &tLXOptions->tGameInfo.sMapFile, 0);
	
	//
	// Game Info
	//
	tLXOptions->tGameInfo.gameMode = GameMode((GameModeIndex)cLocalMenu.SendMessage(ml_Gametype, CBM_GETCURINDEX, (DWORD)0, 0));
	
    //tLXOptions->sServerPassword = ""; // TODO: we have set this, why? it overwrites the password which is very annoying
	tLXOptions->tGameInfo.features[FT_NewNetEngine] = false; // May become buggy otherwise, new net engine doesn't support any kind of pause
	
	
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
		return false;
	}
	
	return true;
}

///////////////////
// Start a local game
void Menu_LocalStartGame()
{
	bool ok = false;
	if(tLXOptions->iLocalPlayGame < 0)
		ok = Menu_LocalStartGame_CustomGame();
	else
		ok = singlePlayerGame.startGame();

	if(ok) {
		*bGame = true;
		tMenu->bMenuRunning = false;
		tLX->iGameType = GME_LOCAL;
		
		// Tell the client to connect to the server
		cClient->Connect("127.0.0.1:" + itoa(cServer->getPort()));
		
		cLocalMenu.Shutdown();	
	}
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
			ModInfo info = infoForMod(abs_filename, true);
			if(info.valid)
				combobox->addItem(info.path, "[" + info.typeShort + "] " + info.name);

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

	cWpnRestList.reset();
	
    //
    // Update the list with the currently selected mod
    //

    cWpnGameScript = new CGameScript;
    if( cWpnGameScript ) {
        if( cWpnGameScript->Load(szMod) ) {
			// Load the weapons
			cWpnRestList.loadList(tLXOptions->tGameInfo.sWeaponRestFile, cWpnGameScript->directory());

            cWpnRestList.updateList( cWpnGameScript );
		}
    }

    // Get the weapons for the list
	UpdateWeaponList();
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
    
					tLXOptions->tGameInfo.sWeaponRestFile = "cfg/wpnrest.dat";
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
				if(ev->iEventMsg == BTN_CLICKED) {

					Menu_WeaponsRestrictionsShutdown();

					return true;
				}
				break;

            // Reset the list
            case wr_Reset:
                if( ev->iEventMsg == BTN_CLICKED ) {
					tLXOptions->tGameInfo.sWeaponRestFile = "cfg/wpnrest.dat";
                    cWpnRestList.cycleVisible(cWpnGameScript);
                }
                break;

            // Randomize the list
            case wr_Random:
                if(ev->iEventMsg == BTN_CLICKED) {
					tLXOptions->tGameInfo.sWeaponRestFile = "cfg/wpnrest.dat";
                    cWpnRestList.randomizeVisible(cWpnGameScript);
                }
                break;

            // Open the load dialog
            case wr_Load:
                if(ev->iEventMsg == BTN_CLICKED) {
                    Menu_WeaponPresets(false,&cWpnRestList);
                }
                break;

            // Open the save dialog
            case wr_Save:
                if(ev->iEventMsg == BTN_CLICKED) {
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
	
	if(tLXOptions->tGameInfo.sWeaponRestFile != "cfg/wpnrest.dat") {
		std::string fn = tLXOptions->tGameInfo.sWeaponRestFile;
		if(fn.find(".wps") == std::string::npos )
			fn += ".wps";
		
		lv_item_t* it = lv->getItem(cWpnGameScript->directory() + "/" + fn);
		if(!it) it = lv->getItem(fn);;
		if(it) {
			lv->setSelectedID(it->_iID);
			lv->setFocused(true); // just looks nicer			
		}
	}
	
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
				if((ev->iEventMsg == BTN_CLICKED && ev->iControlID == 1) || ev->iEventMsg == LV_DOUBLECLK) {

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
							if(Menu_WeaponPresetsOkSave(fn)) {
								wpnrest->saveList(fn);
								tLXOptions->tGameInfo.sWeaponRestFile = GetBaseFilename(fn);
							} else
								quitloop = false;
						} else {

							// Load
							std::string fn = "cfg/presets/" + t->getText();
							wpnrest->loadList(fn, "");
							wpnrest->updateList(cWpnGameScript);
							UpdateWeaponList();
							tLXOptions->tGameInfo.sWeaponRestFile = GetBaseFilename(t->getText());
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
