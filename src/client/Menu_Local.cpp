/////////////////////////////////////////
//
//             Liero Xtreme
//
//     Copyright Auxiliary Software 2002
//
//
/////////////////////////////////////////


// Local menu
// Created 30/6/02
// Jason Boettcher


#include "defs.h"
#include "LieroX.h"
#include "Menu.h"
#include "CListview.h"


CGuiLayout cLocalMenu;

// Gui layout id's
enum {
	Back=0,
	Start,
	Playing,
	PlayerList,
	LevelList,
	//LoadingTimes,
	//LoadingTimeLabel,
	Gametype,
	ModName,
	//Lives,
	//MaxKills,
	//TimeLimit,
	//TagLimitLbl,
	//TagLimitTxt,
	GameSettings,
    WeaponOptions
};

int iGameType = GMT_DEATHMATCH;

bool	bGameSettings = false;
bool    bWeaponRest = false;

local_ply_t sLocalPlayers[MAX_PLAYERS];


///////////////////
// Initialize the local menu
void Menu_LocalInitialize(void)
{
	tMenu->iMenuType = MNU_LOCAL;
	iGameType = GMT_DEATHMATCH;
	bGameSettings = false;

	// Create the buffer
	DrawImage(tMenu->bmpBuffer,tMenu->bmpMainBack_wob,0,0);
	Menu_DrawSubTitleAdv(tMenu->bmpBuffer,SUB_LOCAL,15);
    Menu_DrawBox(tMenu->bmpBuffer, 15,100, 625, 465);
    Menu_DrawBoxInset(tMenu->bmpBuffer, 310,255,610,435);
    
    // Minimap box
    tLX->cFont.Draw(tMenu->bmpBuffer, 310,240,0xffff,"%s","Playing");
    //tLX->cFont.Draw(tMenu->bmpBuffer, 480,240,0xffff,"%s","H");
    //tLX->cFont.Draw(tMenu->bmpBuffer, 515,240,0xffff,"%s","T");
	Menu_DrawBox(tMenu->bmpBuffer, 133,129, 266, 230);

	Menu_RedrawMouse(true);

	// Shutdown any previous data from before
	cLocalMenu.Shutdown();
	cLocalMenu.Initialize();

	cLocalMenu.Add( new CButton(BUT_BACK, tMenu->bmpButtons), Back, 25,440, 50,15);
	cLocalMenu.Add( new CButton(BUT_START, tMenu->bmpButtons), Start, 555,440, 60,15);
	cLocalMenu.Add( new CListview(), PlayerList,  410,115, 200, 120);
	//cLocalMenu.Add( new CListview(), Playing,     310,250, 300, 185);

	cLocalMenu.Add( new CButton(BUT_GAMESETTINGS, tMenu->bmpButtons), GameSettings, 30, 325, 170,15);
    cLocalMenu.Add( new CButton(BUT_WEAPONOPTIONS,tMenu->bmpButtons), WeaponOptions,30, 350, 185,15);

	cLocalMenu.Add( new CLabel("Mod",0xffff),	    -1,         30,  284, 0,   0);
	cLocalMenu.Add( new CCombobox(),				ModName,    120, 283, 170, 17);
	cLocalMenu.Add( new CLabel("Game type",0xffff),	-1,         30,  260, 0,   0);
	cLocalMenu.Add( new CCombobox(),				Gametype,   120, 259, 170, 17);
    cLocalMenu.Add( new CLabel("Level",0xffff),	    -1,         30,  236, 0,   0);
	cLocalMenu.Add( new CCombobox(),				LevelList,  120, 235, 170, 17);

	cLocalMenu.SendMessage(Playing,		LVM_ADDCOLUMN, (DWORD)"Playing", 22);
	cLocalMenu.SendMessage(Playing,		LVM_ADDCOLUMN, (DWORD)"", 90);
	cLocalMenu.SendMessage(Playing,		LVM_ADDCOLUMN, (DWORD)"", 20);

	cLocalMenu.SendMessage(PlayerList,	LVM_ADDCOLUMN, (DWORD)"Players", 22);
	cLocalMenu.SendMessage(PlayerList,	LVM_ADDCOLUMN, (DWORD)"", 60);
	Menu_LocalAddProfiles();

	cLocalMenu.SendMessage(Gametype,    CBM_ADDITEM,   GMT_DEATHMATCH, (DWORD)"Deathmatch");
	cLocalMenu.SendMessage(Gametype,    CBM_ADDITEM,   GMT_TEAMDEATH,  (DWORD)"Team Deathmatch");
	cLocalMenu.SendMessage(Gametype,    CBM_ADDITEM,   GMT_TAG,        (DWORD)"Tag");
    cLocalMenu.SendMessage(Gametype,    CBM_ADDITEM,   GMT_DEMOLITION, (DWORD)"Demolitions");

	/*cLocalMenu.SendMessage(Gametype,    CBM_ADDITEM,   1, (DWORD)"Capture the flag");
	cLocalMenu.SendMessage(Gametype,    CBM_ADDITEM,   1, (DWORD)"Flag hunt");*/

    cLocalMenu.SendMessage(Gametype,    CBM_SETCURSEL, tLXOptions->tGameinfo.nGameType, 0);
    iGameType = tLXOptions->tGameinfo.nGameType;
		
	// Fill the level list
	Menu_FillLevelList( (CCombobox *)cLocalMenu.getWidget(LevelList), true);
	Menu_LocalShowMinimap(true);

	// Fill in the mod list
	Menu_Local_FillModList( (CCombobox *)cLocalMenu.getWidget(ModName));

	// Fill in some game details
	tGameInfo.iLoadingTimes = tLXOptions->tGameinfo.iLoadingTime;
	tGameInfo.iLives = tLXOptions->tGameinfo.iLives;
	tGameInfo.iKillLimit = tLXOptions->tGameinfo.iKillLimit;
	tGameInfo.iBonusesOn = tLXOptions->tGameinfo.iBonusesOn;
	tGameInfo.iShowBonusName = tLXOptions->tGameinfo.iShowBonusName;

    // Initialize the local players
    for(int i=0; i<MAX_PLAYERS; i++)
        sLocalPlayers[i].bUsed = false;
}


///////////////////
// Local frame
void Menu_LocalFrame(void)
{
	gui_event_t *ev;
	mouse_t *Mouse = GetMouse();
	int mouse = 0;
    int i;
	CListview *lv;
	profile_t *ply = NULL;


    // Game Settings
	if(bGameSettings) {
		if(Menu_GameSettings_Frame()) {
			// Re-do the buffer
			DrawImage(tMenu->bmpBuffer,tMenu->bmpMainBack_wob,0,0);
	        Menu_DrawSubTitleAdv(tMenu->bmpBuffer,SUB_LOCAL,15);
            Menu_DrawBox(tMenu->bmpBuffer, 15,100, 625, 465);
	        Menu_DrawBox(tMenu->bmpBuffer, 133,129, 266, 230);
            Menu_DrawBoxInset(tMenu->bmpBuffer, 310,255,610,435);
            tLX->cFont.Draw(tMenu->bmpBuffer, 310,240,0xffff,"%s","Playing");
	
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
			DrawImage(tMenu->bmpBuffer,tMenu->bmpMainBack_wob,0,0);
	        Menu_DrawSubTitleAdv(tMenu->bmpBuffer,SUB_LOCAL,15);
            Menu_DrawBox(tMenu->bmpBuffer, 15,100, 625, 465);
	        Menu_DrawBox(tMenu->bmpBuffer, 133,129, 266, 230);
            Menu_DrawBoxInset(tMenu->bmpBuffer, 310,255,610,435);
            tLX->cFont.Draw(tMenu->bmpBuffer, 310,240,0xffff,"%s","Playing");

			Menu_RedrawMouse(true);
			Menu_LocalShowMinimap(false);

			bWeaponRest = false;
		}
		return;
	}


    // Was the mouse clicked on the map section
    if( Mouse->Up & SDL_BUTTON(1) ) {
        if( MouseInRect(136,132,128,96) )
            Menu_LocalShowMinimap(true);
    }


    // Draw & Process the playing list
    Menu_LocalDrawPlayingList();



	//DrawImageAdv(tMenu->bmpScreen, tMenu->bmpBuffer, 20,140, 20,140, 620,340);

	ev = cLocalMenu.Process();
	cLocalMenu.Draw(tMenu->bmpScreen);

	if(ev) {

		if(ev->cWidget->getType() == wid_Button)
			mouse = 1;
		if(ev->cWidget->getType() == wid_Textbox)
			mouse = 2;

		switch(ev->iControlID) {
			// Back
			case Back:
				if(ev->iEventMsg == BTN_MOUSEUP) {

					// Shutdown
					cLocalMenu.Shutdown();

					// Leave
// TODO: sound
//					BASS_SamplePlay(sfxGeneral.smpClick);
					Menu_MainInitialize();
					return;
				}
				break;

			// Start
			case Start:
				if(ev->iEventMsg == BTN_MOUSEUP) {
// TODO: sound
//					BASS_SamplePlay(sfxGeneral.smpClick);
					// Start the game
					Menu_LocalStartGame();
				}
				break;

			// Player list
			case PlayerList:
				if(ev->iEventMsg == LV_DOUBLECLK) {
					// Add the item to the players list
					lv = (CListview *)cLocalMenu.getWidget(PlayerList);
					int index = lv->getCurIndex();

					// Check if we have enough room for another player
					if(Menu_LocalCheckPlaying(index)) {

						// Remove the item from the list
						lv->RemoveItem(index);

						ply = FindProfile(index);

						if(ply) {
                            // Add a player onto the list
                            for(i=0; i<MAX_PLAYERS; i++) {
                                if(sLocalPlayers[i].bUsed)
                                    continue;

                                sLocalPlayers[i].bUsed = true;
                                sLocalPlayers[i].nHealth = 0;
                                sLocalPlayers[i].nTeam = 0;
                                sLocalPlayers[i].psProfile = ply;
                                break;
                            }
							/*lv = (CListview *)cLocalMenu.getWidget(Playing);
							lv->AddItem("",index,0xffff);
							lv->AddSubitem(LVS_IMAGE, "", ply->bmpWorm);
							lv->AddSubitem(LVS_TEXT, ply->sName, NULL);
							lv->AddSubitem(LVS_IMAGE, "", tMenu->bmpTeamColours[0]);

							// If we're in deathmatch, make the team colour invisible
							lv_item_t *l= lv->getLastItem();
							lv_subitem_t *sub = NULL;
							if(l)
								sub = l->tSubitems;
							
							for(int i=0;i<2;i++) {
								if(sub)
									sub=sub->tNext;
							}
							if(sub) {
								if(iGameType != GMT_TEAMDEATH)
									sub->iVisible = false;
								sub->iExtra = 0;
							}*/
						}
					}
				}
				break;

			// Playing list
			/*case Playing:
				if(ev->iEventMsg == LV_DOUBLECLK) {

					// Put the player back into the players list
					lv = (CListview *)cLocalMenu.getWidget(Playing);
					int index = lv->getCurIndex();
					int i = lv->getClickedSub();

					// If we double clicked on the colour box, modify this event to changed
					if(i==2 && iGameType == GMT_TEAMDEATH)
						ev->iEventMsg = LV_CHANGED;

					else {
						// Double clicked on something else, remove the player

						// Remove the item from the list
						lv->RemoveItem(index);

						ply = FindProfile(index);

						// Add the player into the players list
						if(ply) {
							lv = (CListview *)cLocalMenu.getWidget(PlayerList);
							lv->AddItem("",index,0xffff);
							lv->AddSubitem(LVS_IMAGE, "", ply->bmpWorm);
							lv->AddSubitem(LVS_TEXT, ply->sName, NULL);
						}
					}
				}


				if(ev->iEventMsg == LV_CHANGED && iGameType == GMT_TEAMDEATH) {

					// If the team colour item was clicked on, change it
					lv = (CListview *)cLocalMenu.getWidget(Playing);
					int i = lv->getClickedSub();

					if(i==2) {
						lv_subitem_t *sub = lv->getCurSub();

						for(int i=0;i<2;i++) {
							if(sub)
								sub=sub->tNext;
						}
						if(sub) {
							// Left mouse button increments, right mouse button decrements
							if(Mouse->Up & SDL_BUTTON(1))
								sub->iExtra++;
							if(Mouse->Up & SDL_BUTTON(3))
								sub->iExtra--;

							// Loop around
							if(sub->iExtra >= 4)
								sub->iExtra=0;
							if(sub->iExtra < 0)
								sub->iExtra=3;

							// Change the image
							sub->bmpImage = tMenu->bmpTeamColours[sub->iExtra];
						}
					}
				}
				break;*/


			// Game type
			case Gametype:
				if(ev->iEventMsg == CMB_CHANGED) {
					iGameType = cLocalMenu.SendMessage(Gametype, CBM_GETCURINDEX, 0, 0);
				}
				break;

			// Level list combo box
			case LevelList:
				if(ev->iEventMsg == CMB_CHANGED) {
					
					Menu_LocalShowMinimap(true);
				}
				break;


			// Game settings button
			case GameSettings:
				if(ev->iEventMsg == BTN_MOUSEUP) {

					cLocalMenu.Draw( tMenu->bmpBuffer );

					bGameSettings = true;
					Menu_GameSettings();
				}
				break;


            // Weapons Restrictions button
            case WeaponOptions:
                if( ev->iEventMsg == BTN_MOUSEUP ) {
                    
					cLocalMenu.Draw( tMenu->bmpBuffer );

                    // Get the current mod
                    char buf[256];
                    cb_item_t *it = (cb_item_t *)cLocalMenu.SendMessage(ModName,CBM_GETCURITEM,0,0);
                    if(it) {
                        lx_strncpy(buf, it->sIndex, 255);

					    bWeaponRest = true;
					    Menu_WeaponsRestrictions(buf);
                    }
                }
                break;

		}
	}

	// Draw the mouse
	DrawImage(tMenu->bmpScreen,gfxGUI.bmpMouse[mouse], Mouse->X,Mouse->Y);
}


///////////////////
// Add the profiles to the players list
void Menu_LocalAddProfiles(void)
{	
	profile_t *p = GetProfiles();
	
	for(; p; p=p->tNext) {
		cLocalMenu.SendMessage( PlayerList, LVM_ADDITEM, (DWORD)"", p->iID);
		cLocalMenu.SendMessage( PlayerList, LVM_ADDSUBITEM, LVS_IMAGE, (DWORD)p->bmpWorm);
		cLocalMenu.SendMessage( PlayerList, LVM_ADDSUBITEM, LVS_TEXT,  (DWORD)p->sName);
	}
}


///////////////////
// Show the minimap
void Menu_LocalShowMinimap(bool bReload)
{
	CMap map;
	char buf[256];
	char blah[256];

	cLocalMenu.SendMessage(LevelList, CBM_GETCURSINDEX, (DWORD)buf, sizeof(buf));

    tGameInfo.sMapRandom.bUsed = false;

	// Draw a background over the old minimap
	//DrawImageAdv(tMenu->bmpBuffer, tMenu->bmpMainBack, 126,132,126,132,128,96);
	
	// Load the map
	sprintf(blah, "levels/%s",buf);
    if( bReload ) {
        
        // Create a random map
        if( strcmp(buf, "_random_") == 0 ) {
            if( map.New(504,350,map.findRandomTheme(buf)) ) {
			    map.ApplyRandom();

                // Free any old random map object list
                if( tGameInfo.sMapRandom.psObjects )
                    delete[] tGameInfo.sMapRandom.psObjects;

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
		        map.UpdateMiniMap(true);
		        DrawImage(tMenu->bmpMiniMapBuffer, map.GetMiniMap(), 0,0);
		        map.Shutdown();
            }

        } else {
            // Load the file
            if(map.Load(blah)) {

		        // Draw the minimap
		        map.UpdateMiniMap(true);
		        DrawImage(tMenu->bmpMiniMapBuffer, map.GetMiniMap(), 0,0);
		        map.Shutdown();
	        }
        }
    }

	// Update the screen
    DrawImage(tMenu->bmpBuffer, tMenu->bmpMiniMapBuffer, 136,132);
	DrawImageAdv(tMenu->bmpScreen, tMenu->bmpBuffer, 130,130,130,130,140,110);
}


///////////////////
// Start a local game
void Menu_LocalStartGame(void)
{
    int i;

	// Level
	cLocalMenu.SendMessage(LevelList, CBM_GETCURSINDEX, (DWORD)tGameInfo.sMapname, sizeof(tGameInfo.sMapname));


	//
	// Players
	//
    // Calculate the number of players
    tGameInfo.iNumPlayers = 0;
    for(i=0; i<MAX_PLAYERS; i++) {
        if(sLocalPlayers[i].bUsed)
            tGameInfo.iNumPlayers++;
    }
	// Can't start a game with no-one playing
	if(tGameInfo.iNumPlayers == 0)
		return;

    int count = 0;
    
    // Add the human players onto the list    
    for(i=0; i<MAX_PLAYERS; i++) {
        if(!sLocalPlayers[i].bUsed)
            continue;
        
        if(sLocalPlayers[i].psProfile) {
            if(sLocalPlayers[i].psProfile->iType == PRF_HUMAN) {

                tGameInfo.cPlayers[count++] = sLocalPlayers[i].psProfile;
                sLocalPlayers[i].psProfile->iTeam = sLocalPlayers[i].nTeam;
            }
        }
    }

    // Add the AI players onto the list
    for(i=0; i<MAX_PLAYERS; i++) {
        if(!sLocalPlayers[i].bUsed)
            continue;
        
        if(sLocalPlayers[i].psProfile) {
            if(sLocalPlayers[i].psProfile->iType == PRF_COMPUTER) {

                tGameInfo.cPlayers[count++] = sLocalPlayers[i].psProfile;
                sLocalPlayers[i].psProfile->iTeam = sLocalPlayers[i].nTeam;
            }
        }
    }



	// Save the current level in the options
	cLocalMenu.SendMessage(LevelList, CBM_GETCURSINDEX, (DWORD)tLXOptions->tGameinfo.sMapName, sizeof(tLXOptions->tGameinfo.sMapName));

	//
	// Game Info
	//
	tGameInfo.iGameMode = cLocalMenu.SendMessage(Gametype, CBM_GETCURINDEX, 0, 0);
    tLXOptions->tGameinfo.nGameType = tGameInfo.iGameMode;

    strcpy( tGameInfo.sPassword, "" );

	
    // Get the mod name
	cb_item_t *it = (cb_item_t *)cLocalMenu.SendMessage(ModName,CBM_GETCURITEM,0,0);
    if(it) {
        strcpy(tGameInfo.sModName,it->sIndex);
        strcpy(tLXOptions->tGameinfo.szModName, it->sIndex);
    } else {

		// Couldn't find a mod to load
		cLocalMenu.Draw(tMenu->bmpBuffer);
		Menu_MessageBox("Error","Could not find a mod to load!", LMB_OK);
		DrawImage(tMenu->bmpBuffer,tMenu->bmpMainBack_wob,0,0);
        Menu_DrawBox(tMenu->bmpBuffer, 15,130, 625, 465);
		Menu_DrawSubTitle(tMenu->bmpBuffer,SUB_LOCAL);
		Menu_RedrawMouse(true);
		return;
	}

	*iGame = true;
	tMenu->iMenuRunning = false;
	tGameInfo.iGameType = GME_LOCAL;

	cLocalMenu.Shutdown();
}


///////////////////
// Check if we can add another player to the list
int Menu_LocalCheckPlaying(int index)
{
	int			plycount = 0;
	int			hmncount = 0;
	int			i, count;
	profile_t	*p;

    count = 0;
    for(i=0; i<MAX_PLAYERS; i++) {
        if(sLocalPlayers[i].bUsed)
            count++;
    }

	// Go through the playing list
	for(i=0; i<MAX_PLAYERS; i++) {
        if(!sLocalPlayers[i].bUsed)
            continue;

		if(sLocalPlayers[i].psProfile->iType == PRF_HUMAN)
			hmncount++;
		plycount++;
	}

	p = FindProfile(index);

	// Check if there is too many players (MAX: 8)
	if(plycount+1 > 8)
		return false;

	// Check if there is too many human players (MAX: 2)
	if(p) {
		if(p->iType == PRF_HUMAN && hmncount+1 > 2)
			return false;
	}

	return true;
}


///////////////////
// Draw the playing list
void Menu_LocalDrawPlayingList(void)
{
    int     i;
    int     y = 260;
    mouse_t *Mouse = GetMouse();

    int m_leftup = (Mouse->Up & SDL_BUTTON(1));
    int m_rightup = (Mouse->Up & SDL_BUTTON(3));

    int mode = cLocalMenu.SendMessage(Gametype, CBM_GETCURINDEX, 0, 0);
    bool team = (mode == GMT_TEAMDEATH);

    DrawImageAdv(tMenu->bmpScreen, tMenu->bmpBuffer, 310,255,310,255,300,180);

    for(i=0; i<MAX_PLAYERS; i++) {
        if(!sLocalPlayers[i].bUsed)
            continue;

        // Pic & Name
        DrawImage(tMenu->bmpScreen, sLocalPlayers[i].psProfile->bmpWorm, 315,y);
        

        // Click on the name removes the player from the playing list
        int nameCol = 0xffff;
        int w = tLX->cFont.GetWidth(sLocalPlayers[i].psProfile->sName);
        w = MAX(25,w);
        if(MouseInRect(345,y,w,20)) {
            nameCol = MakeColour(0,138,251);
            if(m_leftup) {
                // Remove the player
                sLocalPlayers[i].bUsed = false;
                m_leftup = false;

                // Add it back onto the player list
                CListview *lv = (CListview *)cLocalMenu.getWidget(PlayerList);
				lv->AddItem("",sLocalPlayers[i].psProfile->iID,0xffff);
				lv->AddSubitem(LVS_IMAGE, "", sLocalPlayers[i].psProfile->bmpWorm);
				lv->AddSubitem(LVS_TEXT, sLocalPlayers[i].psProfile->sName, NULL);

                continue;
            }
        }
        tLX->cFont.Draw(tMenu->bmpScreen, 345, y, nameCol,"%s", sLocalPlayers[i].psProfile->sName);


        // Team
        if(team) {
            DrawImage(tMenu->bmpScreen, tMenu->bmpTeamColours[sLocalPlayers[i].nTeam], 510, y);
            // Check for a click
            if(MouseInRect(510,y,18,16)) {
                if(m_leftup)
                    sLocalPlayers[i].nTeam++;
                if(m_rightup)
                    sLocalPlayers[i].nTeam--;

                // Wrap around
                if(sLocalPlayers[i].nTeam >= 4)
                    sLocalPlayers[i].nTeam = 0;
                if(sLocalPlayers[i].nTeam < 0)
                    sLocalPlayers[i].nTeam = 3;
            }
        }

        // Handicap (health)
        //DrawImageAdv(tMenu->bmpScreen, tMenu->bmpHandicap, (sLocalPlayers[i].nHealth+1)*19, 0, 475,y, 18,16);
        if(MouseInRect(475,y,18,16)) {
            if(m_leftup)
                sLocalPlayers[i].nHealth++;
            if(m_rightup)
                sLocalPlayers[i].nHealth--;

            // Wrap around
            if(sLocalPlayers[i].nHealth >= 5)
                sLocalPlayers[i].nHealth = -1;
            if(sLocalPlayers[i].nHealth < -1)
                sLocalPlayers[i].nHealth = 4;
        }

        y += 22;
    }
}


///////////////////
// Get the team number from a worm in the playing list
int Menu_LocalGetTeam(int count)
{
	int team = 0;
	CListview *lv = (CListview *)cLocalMenu.getWidget(Playing);
	lv_item_t *items = lv->getItems();

	int i;
	for(i=0;i<count;i++)
		items=items->tNext;

	if(items) {
		lv_subitem_t *sub = items->tSubitems;
		for(i=0;i<2;i++) {
			if(sub)
				sub=sub->tNext;
		}

		// Return the team id
		if(sub)
			return sub->iExtra;
	}

	return team;
}


///////////////////
// Fill in the mod list
void Menu_Local_FillModList( CCombobox *cb )
{
	// Find all directories in the the lierox
	char dir[256];
	char curdir[_MAX_PATH];
	char *d;
	char name[32];
	CGameScript gs;
	int baseid = 0;
	int i=0;

	_getcwd(curdir,_MAX_PATH);

	if(FindFirstDir(curdir,dir)) {
		while(1) {
			
            // Remove the full directory section so we only have the mod dir name
			d = strrchr(dir,'\\')+1;
            if(!d)
               d = dir;

			// Check if this dir has a valid script.lgs file in it
			if(gs.CheckFile(dir,name)) {
				cb->addItem(i,d,name);

				// Store the index of the last used mod
				if(stricmp(d,tLXOptions->tGameinfo.szModName) == 0)
					baseid = i;
                i++;
			}			
			
			if(!FindNextDir(dir))
				break;
		}
	}

	// Set the last used mod as default
	if(baseid >= 0)
		cb->setCurItem(baseid);
}




/*
=======================

	 Game Settings

 For both local & host

=======================
*/


CGuiLayout		cGameSettings;

// Game Settings
enum {
	gs_Ok,
    gs_Default,
	Lives,
	MaxKills,
	LoadingTime,
	LoadingTimeLabel,
	Bonuses,
	ShowBonusNames,
	MaxTime
};



///////////////////
// Initialize the game settings
void Menu_GameSettings(void)
{
	Uint32 blue = MakeColour(0,138,251);
	char buf[256];

	// Setup the buffer
	//DrawImageAdv(tMenu->bmpBuffer, tMenu->bmpMainBack, 120,150,120,150, 400,300);
	Menu_DrawBox(tMenu->bmpBuffer, 120,150, 520,440);
	DrawRectFillA(tMenu->bmpBuffer, 122,152, 518,438, 0, 200);

	// Lives
	// Max kills
	// Weapon Loading time
	// Bonuses
	// Show bonus names
	// Max Time
	// 


	Menu_RedrawMouse(true);

	cGameSettings.Initialize();
	cGameSettings.Add( new CLabel("Game Settings", 0xffff),		    -1,	        270,155, 0, 0);
	cGameSettings.Add( new CButton(BUT_OK, tMenu->bmpButtons),	    gs_Ok,      220,420, 40,15);
    cGameSettings.Add( new CButton(BUT_DEFAULT, tMenu->bmpButtons), gs_Default, 350,420, 80,15);
	cGameSettings.Add( new CLabel("Lives", 0xffff),				    -1,	        150,200, 0, 0);
	cGameSettings.Add( new CLabel("Max Kills", 0xffff),			    -1,	        150,230, 0, 0);
	cGameSettings.Add( new CLabel("Loading Time", 0xffff),		    -1,	        150,260, 0, 0);
	cGameSettings.Add( new CLabel("Bonuses", 0xffff),			    -1,	        150,290, 0, 0);
	cGameSettings.Add( new CLabel("Show Bonus names", 0xffff),	    -1,	        150,320, 0, 0);
	//cGameSettings.Add( new CLabel("Max Kills", 0xffff),			-1,	   150,240, 0, 0);

	cGameSettings.Add( new CTextbox(),							Lives,		320,197, 100,20);
	cGameSettings.Add( new CTextbox(),							MaxKills,	320,227, 100,20);
	cGameSettings.Add( new CSlider(500),						LoadingTime,315,257, 160,20);
	cGameSettings.Add( new CLabel("", 0xffff),					LoadingTimeLabel, 480, 260, 0, 0);
	cGameSettings.Add( new CCheckbox(tLXOptions->tGameinfo.iBonusesOn),	Bonuses, 320,287,17,17);
	cGameSettings.Add( new CCheckbox(tLXOptions->tGameinfo.iShowBonusName),ShowBonusNames, 320,317,17,17);

	cGameSettings.SendMessage(Lives,TXM_SETMAX,6,0);
	cGameSettings.SendMessage(MaxKills,TXM_SETMAX,6,0);

	cGameSettings.SendMessage(LoadingTime, SLM_SETVALUE, tLXOptions->tGameinfo.iLoadingTime, 0);

	if(tLXOptions->tGameinfo.iLives >= 0)
		cGameSettings.SendMessage(Lives, TXM_SETTEXT, (DWORD)itoa(tLXOptions->tGameinfo.iLives,buf,10), 0);
	if(tLXOptions->tGameinfo.iKillLimit >= 0)
		cGameSettings.SendMessage(MaxKills, TXM_SETTEXT, (DWORD)itoa(tLXOptions->tGameinfo.iKillLimit,buf,10), 0);
	//if(tLXOptions->tGameinfo.iTimeLimit >= 0)
	//	cLocalMenu.SendMessage(TimeLimit, TXM_SETTEXT, (DWORD)itoa(tLXOptions->tGameinfo.iTimeLimit,buf,10), 0);
	//if(tLXOptions->tGameinfo.iTagLimit >= 0)
		//cLocalMenu.SendMessage(TagLimitTxt, TXM_SETTEXT, (DWORD)itoa(tLXOptions->tGameinfo.iTagLimit,buf,10), 0);
}


///////////////////
// Game settings frame
// Returns whether of not we have finised with the game settings
bool Menu_GameSettings_Frame(void)
{
	gui_event_t *ev = NULL;
	mouse_t *Mouse = GetMouse();
	int mouse = 0;

	DrawImageAdv(tMenu->bmpScreen, tMenu->bmpBuffer, 120,150, 120,150, 400,300);
	
	ev = cGameSettings.Process();
	cGameSettings.Draw(tMenu->bmpScreen);

	if(ev) {

		if(ev->cWidget->getType() == wid_Button)
			mouse = 1;
		if(ev->cWidget->getType() == wid_Textbox)
			mouse = 2;

		switch(ev->iControlID) {

			// OK, done
			case gs_Ok:
				if(ev->iEventMsg == BTN_MOUSEUP) {

					Menu_GameSettings_GrabInfo();

					cGameSettings.Shutdown();				

					return true;
				}
				break;

            // Set the default values
            case gs_Default:
                if( ev->iEventMsg == BTN_MOUSEUP ) {
                    Menu_GameSettings_Default();
                }
                break;
		}
	}

	// Set the value of the loading time label
	int l = cGameSettings.SendMessage(LoadingTime, SLM_GETVALUE, 100, 0);
	char buf[64];
	sprintf(buf, "%d%%",l);		// 2 %'s because it gets parsed as a va_list twice
	cGameSettings.SendMessage(LoadingTimeLabel, LBM_SETTEXT, (DWORD)buf, 0);

	// Draw the mouse
	DrawImage(tMenu->bmpScreen,gfxGUI.bmpMouse[mouse], Mouse->X,Mouse->Y);
	
	return false;
}


///////////////////
// Grab the game settings info
void Menu_GameSettings_GrabInfo(void)
{
	char buf[256];

	tLXOptions->tGameinfo.iLoadingTime = cGameSettings.SendMessage(LoadingTime, SLM_GETVALUE, 100, 0);
	tGameInfo.iLoadingTimes = tLXOptions->tGameinfo.iLoadingTime;

	// Default to no setting
	tGameInfo.iLives = tLXOptions->tGameinfo.iLives = -2;
	tGameInfo.iKillLimit = tLXOptions->tGameinfo.iKillLimit = -1;
	tGameInfo.iTimeLimit = tLXOptions->tGameinfo.iTimeLimit = -1;
	tGameInfo.iTagLimit = tLXOptions->tGameinfo.iTagLimit = -1;
	tGameInfo.iBonusesOn = true;
	tGameInfo.iShowBonusName = true;    

	
	// Store the game info into the options structure as well
	cGameSettings.SendMessage(Lives, TXM_GETTEXT, (DWORD)buf, sizeof(buf));
	if(*buf) {
		tLXOptions->tGameinfo.iLives = atoi(buf);
		tGameInfo.iLives = atoi(buf);
	}
	cGameSettings.SendMessage(MaxKills, TXM_GETTEXT, (DWORD)buf, sizeof(buf));
	if(*buf) {
		tLXOptions->tGameinfo.iKillLimit = atoi(buf);
		tGameInfo.iKillLimit = atoi(buf);
	}

	tGameInfo.iBonusesOn = cGameSettings.SendMessage( Bonuses, CKM_GETCHECK, 0, 0);
	tLXOptions->tGameinfo.iBonusesOn = tGameInfo.iBonusesOn;

	tGameInfo.iShowBonusName = cGameSettings.SendMessage( ShowBonusNames, CKM_GETCHECK, 0, 0);
	tLXOptions->tGameinfo.iShowBonusName = tGameInfo.iShowBonusName;
}


///////////////////
// Set the default game settings info
void Menu_GameSettings_Default(void)
{
    cGameSettings.SendMessage(LoadingTime, SLM_SETVALUE, 100, 0);
	
    cGameSettings.SendMessage(Lives, TXM_SETTEXT, (DWORD)"10", 0);
	cGameSettings.SendMessage(MaxKills, TXM_SETTEXT, (DWORD)"", 0);

    cGameSettings.SendMessage( Bonuses, CKM_SETCHECK, true, 0);
    cGameSettings.SendMessage( ShowBonusNames, CKM_SETCHECK, true, 0);
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



///////////////////
// Initialize the weapons restrictions
void Menu_WeaponsRestrictions(char *szMod)
{
	Uint32 blue = MakeColour(0,138,251);

	// Setup the buffer
	DrawImageAdv(tMenu->bmpBuffer, tMenu->bmpMainBack_wob, 120,150,120,150, 400,330);
	Menu_DrawBox(tMenu->bmpBuffer, 120,150, 520,470);
	//DrawRectFillA(tMenu->bmpBuffer, 122,152, 518,438, 0, 100);


	Menu_RedrawMouse(true);

	cWeaponsRest.Initialize();
	cWeaponsRest.Add( new CLabel("Weapon Options", 0xffff),     -1,        275,155, 0, 0);	
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
}


///////////////////
// Weapons Restrictions frame
// Returns whether or not we have finished with the weapons restrictions
bool Menu_WeaponsRestrictions_Frame(void)
{
	gui_event_t *ev = NULL;
	mouse_t *Mouse = GetMouse();
	int mouse = 0;
    Uint32 blue = MakeColour(0,138,251);

    assert(cWpnGameScript);

    // State strings
    char    *szStates[] = {"Enabled", "Bonus", "Banned"};

	DrawImageAdv(tMenu->bmpScreen, tMenu->bmpBuffer, 120,150, 120,150, 400,300);

    // Draw the list
    wpnrest_t *psWpn = cWpnRestList.getList();
    int num = cWpnRestList.getNumWeapons();
    int count = cWeaponsRest.SendMessage(wr_Scroll, SCM_GETVALUE,0,0);
    int weaponCount = 0;
    int i, j = 0, w = 0;

    // Count the number of weapons in _this_ mod only
    for(i=0; i<num; i++) {
        if(cWpnGameScript->weaponExists(psWpn[i].psLink->szName))
            weaponCount++;
    }

    // Show the weapons
    for(i=0; i<num; i++) {
        if(!cWpnGameScript->weaponExists(psWpn[i].psLink->szName))
            continue;
        if( w++ < count )
            continue;
        if( j > 10 )
            break;
        
        
        int y = 190 + (j++)*20;
        Uint32 Colour = 0xffff;

        // If a mouse is over the line, highlight it
        if( Mouse->X > 150 && Mouse->X < 450 ) {
            if( Mouse->Y > y && Mouse->Y < y+20 ) {
                Colour = blue;

                // If the mouse has been clicked, cycle through the states
                if( Mouse->Up & SDL_BUTTON(1) ) {
                    psWpn[i].psLink->nState++;
                    psWpn[i].psLink->nState %= 3;
                }
            }
        }

        tLX->cFont.Draw( tMenu->bmpScreen, 150, y, Colour,"%s", psWpn[i].psLink->szName );
        tLX->cFont.Draw( tMenu->bmpScreen, 400, y, Colour,"%s", szStates[psWpn[i].psLink->nState] );
    }

    // Adjust the scrollbar
    cWeaponsRest.SendMessage(wr_Scroll, SCM_SETITEMSPERBOX, 12, 0);
    cWeaponsRest.SendMessage(wr_Scroll, SCM_SETMIN, 0, 0);    
    if(weaponCount>10)
        cWeaponsRest.SendMessage(wr_Scroll, SCM_SETMAX, weaponCount, 0);
    else
        cWeaponsRest.SendMessage(wr_Scroll, SCM_SETMAX, 0, 0);


	ev = cWeaponsRest.Process();
	cWeaponsRest.Draw(tMenu->bmpScreen);

	if(ev) {

		if(ev->cWidget->getType() == wid_Button)
			mouse = 1;
		if(ev->cWidget->getType() == wid_Textbox)
			mouse = 2;

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

					cWeaponsRest.Shutdown();

                    cWpnRestList.saveList("cfg/wpnrest.dat");
                    cWpnRestList.Shutdown();

                    cWpnGameScript->Shutdown();
                    delete cWpnGameScript;
                    cWpnGameScript = NULL;

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
	DrawImage(tMenu->bmpScreen,gfxGUI.bmpMouse[mouse], Mouse->X,Mouse->Y);
	
	return false;
}


///////////////////
// Weapon presets load/save
// For both local and host
void Menu_WeaponPresets(int save, CWpnRest *wpnrest)
{
	if (!wpnrest)
		return;

	keyboard_t *kb = GetKeyboard();
	mouse_t *Mouse = GetMouse();
	gui_event_t *ev;
	int mouse=0;
	int quitloop = false;
	CTextbox *t;

	// Save the background
	DrawImageAdv(tMenu->bmpBuffer, tMenu->bmpScreen, 0,0, 0,0, 640,480);

	Menu_RedrawMouse(true);

	CGuiLayout cg;

	cg.Initialize();

	cg.Add( new CButton(BUT_CANCEL, tMenu->bmpButtons), 0, 180,310, 75,15);
	cg.Add( new CButton(BUT_OK, tMenu->bmpButtons),     1, 430,310, 40,15);
	cg.Add( new CListview(),                            2, 180,170, 280,110+(!save)*20);
	cg.Add( new CTextbox(),                             3, 270,285, 190,20);
	
	t = (CTextbox *)cg.getWidget(3);

	// Hide the textbox for Load
	t->setEnabled(save);

	// Load the level list
	char	filename[256];
	char	name[64];

	_mkdir("cfg/presets/");

	int done = false;
	if(!FindFirst("cfg/presets/","*.*",filename))
		done = true;
	CListview *lv = (CListview *)cg.getWidget(2);
	lv->AddColumn("Weapon presets",60);


	while(!done) {
		if( stricmp(filename + strlen(filename)-4, ".wps") == 0) {
			// Remove the path
			char *f = strrchr(filename,'/');
			strncpy(name,f+2,strlen(f)-5);
			name[strlen(f)-6] = '\0';
			if(f) {
				lv->AddItem(f+1,0,0xffff);
				lv->AddSubitem(LVS_TEXT,name,NULL);
			}
		}

		done = !FindNext(filename);
	}


	
	
	ProcessEvents();
	while(!kb->KeyUp[SDLK_ESCAPE] && !quitloop) {
		Menu_RedrawMouse(false);
		ProcessEvents();

		//DrawImageAdv(tMenu->bmpScreen,tMenu->bmpBuffer, 170,150, 170,150, 300, 180);
		Menu_DrawBox(tMenu->bmpScreen, 170, 150, 470, 330);
		DrawImageAdv(tMenu->bmpScreen, tMenu->bmpMainBack_wob, 172,152, 172,152, 297,177);
		DrawImageAdv(tMenu->bmpBuffer, tMenu->bmpMainBack_wob, 172,152, 172,152, 297,177);

		tLX->cFont.DrawCentre(tMenu->bmpScreen, 320, 155, 0xffff,"%s", save ? "Save" : "Load");
		if (save)
			tLX->cFont.Draw(tMenu->bmpScreen, 180,288,0xffff,"%s","Preset name");

		ev = cg.Process();
		cg.Draw(tMenu->bmpScreen);

		// Process the widgets
		mouse = 0;
		if(ev) {
			if(ev->cWidget->getType() == wid_Button)
				mouse = 1;
			if(ev->cWidget->getType() == wid_Textbox)
				mouse = 2;

			switch(ev->iControlID) {
				// Cancel
				case 0:
					if(ev->iEventMsg == BTN_MOUSEUP) {
// TODO: sound
//						BASS_SamplePlay(sfxGeneral.smpClick);
						quitloop = true;
					}
					break;

				// Presets list
				case 2:
					if(ev->iEventMsg != LV_NONE) {
						t->setText( lv->getCurSIndex() );
					}
				break;
			}

			// OK and double click on listview
			if (ev->iControlID == 1 || ev->iControlID == 2)  {
				if((ev->iEventMsg == BTN_MOUSEUP && ev->iControlID == 1) || ev->iEventMsg == LV_DOUBLECLK) {

					// Play the sound only for OK button
					if (ev->iControlID == 1) {}
// TODO: sound
//						BASS_SamplePlay(sfxGeneral.smpClick);

					// Don't process when nothing is selected
					if(strlen(t->getText()) > 0) {

						quitloop = true;
						if(save) {

							// Save								
							char buf[256]; 
							sprintf(buf,"cfg/presets/%s",t->getText());

							// Check if it exists already. If so, ask user if they wanna overwrite
							if(Menu_WeaponPresetsOkSave(buf))
								wpnrest->saveList(buf);
							else
								quitloop = false;
						} else {
							
							// Load
							char buf[256];
							sprintf(buf,"cfg/presets/%s",t->getText());
							wpnrest->loadList(buf);
						}
					}					
				}
			}
		}

		// Draw mouse
		DrawImage(tMenu->bmpScreen,gfxGUI.bmpMouse[mouse], Mouse->X,Mouse->Y);

		// Display the dialog
		FlipScreen(tMenu->bmpScreen);
	}

	// Redraw back to normal
	DrawImageAdv(tMenu->bmpBuffer, tMenu->bmpMainBack_wob, 120,150,122,152, 396,316);
	DrawImage(tMenu->bmpScreen,tMenu->bmpBuffer,0,0);

	Menu_RedrawMouse(true);

	cg.Shutdown();
}


///////////////////
// Check if there is a possible overwrite
int Menu_WeaponPresetsOkSave(char *szFilename)
{
	// Adjust the filename
	if( stricmp( szFilename + strlen(szFilename) - 4, ".wps") != 0)
		strcat(szFilename,".wps");

	FILE *fp = fopen(szFilename,"rb");
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

