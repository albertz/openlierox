/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Player menu
// Created 30/6/02
// Jason Boettcher


#include "defs.h"
#include "LieroX.h"
#include "Menu.h"

CGuiLayout	cNewPlayer;
CGuiLayout	cViewPlayers;
CButton		cPlyButtons[2];
int			iPlayerMode = 0;
float       fPlayerSkinFrame=0;
bool        bPlayerSkinAnimation = false;

// Generic
enum {
	Static=-1,
	pp_NewPlayerTab=0,
	pp_ViewPlayersTab
};

// New player widgets
enum {
	np_Back=0,
	np_Create,
	np_Name,
	np_Red, np_Blue, np_Green,
	np_Type,
	np_AIDiffLbl,
	np_AIDiff,
	np_PlySkin,
	np_Username,
	np_Password
};

// View players widgets
enum {
	vp_Back=0,
	vp_Name,
	vp_Red, vp_Blue, vp_Green,
	vp_Players,
	vp_Delete,
	vp_Apply,
	vp_Type,
	vp_AIDiffLbl,
	vp_AIDiff,
	vp_PlySkin
};

///////////////////
// Initialize the player menu
void Menu_PlayerInitialize(void)
{
	tMenu->iMenuType = MNU_PLAYER;

//	Uint32 blue = MakeColour(0,138,251); // TODO: not used
//	Uint32 grey = MakeColour(128,128,128); // TODO: not used
	iPlayerMode = 0;
	CListview *lv;

	// Create the buffer
	DrawImage(tMenu->bmpBuffer,tMenu->bmpMainBack_wob,0,0);
    Menu_DrawBox(tMenu->bmpBuffer, 15,130, 625, 465);
	Menu_DrawSubTitle(tMenu->bmpBuffer,SUB_PLAYER);

	Menu_RedrawMouse(true);



	// Setup the top buttons
	cPlyButtons[pp_NewPlayerTab]   = CButton(BUT_NEWPLAYER,	tMenu->bmpButtons);
	cPlyButtons[pp_ViewPlayersTab] = CButton(BUT_VIEWPLAYERS,	tMenu->bmpButtons);

	cPlyButtons[pp_NewPlayerTab].Setup(0, 150, 110, 120, 15);
	cPlyButtons[pp_ViewPlayersTab].Setup(1, 370, 110, 135, 15);
    cPlyButtons[pp_NewPlayerTab].Create();
    cPlyButtons[pp_ViewPlayersTab].Create();


	// New player
	cNewPlayer.Shutdown();
	cNewPlayer.Initialize();
	cNewPlayer.Add( new CButton(BUT_BACK, tMenu->bmpButtons),	np_Back, 25,440, 50,15);
	cNewPlayer.Add( new CButton(BUT_CREATE, tMenu->bmpButtons), np_Create, 540,440, 70,15);

	cNewPlayer.Add( new CLabel("Worm Details", tLX->clHeading),Static, 30, 170, 0,0);
	cNewPlayer.Add( new CTextbox(),		       np_Name,  120, 200,120,20);
	cNewPlayer.Add( new CLabel("Name",tLX->clNormalLabel), Static, 40, 202,0,  0);
	cNewPlayer.Add( new CLabel("Red",tLX->clNormalLabel),  Static, 40, 300,0,  0);
	cNewPlayer.Add( new CLabel("Green",tLX->clNormalLabel),Static, 40, 320,0,  0);
	cNewPlayer.Add( new CLabel("Blue",tLX->clNormalLabel), Static, 40, 340,0,  0);
	cNewPlayer.Add( new CSlider(255),	       np_Red,   115, 300,128,20);
	cNewPlayer.Add( new CSlider(255),	       np_Green, 115, 320,128,20);
	cNewPlayer.Add( new CSlider(255),	       np_Blue,  115, 340,128,20);

    cNewPlayer.Add( new CLabel("Skill",tLX->clNormalLabel),np_AIDiffLbl,40,362,0, 0);
    cNewPlayer.Add( new CSlider(3),            np_AIDiff,115, 360,128,20);
    cNewPlayer.Add( new CLabel("Skin", tLX->clNormalLabel),Static,40,  262,0,  0);
    cNewPlayer.Add( new CCombobox(),           np_PlySkin,120,260,120,20);
    cNewPlayer.Add( new CLabel("Type", tLX->clNormalLabel),Static,40,  232,0,  0);
    cNewPlayer.Add( new CCombobox(),           np_Type,  120, 230,120,17);

	cNewPlayer.SendMessage(np_Name,TXM_SETMAX,20,0);


	//cNewPlayer.Add( new CLabel("Multiplayer (optional)", tLX->clHeading),Static, 370, 170, 0,0);
	//cNewPlayer.Add( new CLabel("Username", tLX->clNormalLabel), Static, 380, 202, 0,0);
	//cNewPlayer.Add( new CLabel("Password", tLX->clNormalLabel), Static, 380, 232, 0,0);
	cNewPlayer.Add( new CTextbox(),			   np_Username, 470, 200, 110, 20);
	cNewPlayer.Add( new CTextbox(),			   np_Password, 470, 230, 110, 20);

	//cNewPlayer.Add( new CLabel("Note: To register a username, visit the OpenLieroX web site", tLX->clSubHeading),Static, 30, 410, 0,0);

	// Hide the multiplayer textboxes
	cNewPlayer.getWidget(np_Username)->setEnabled(false);
	cNewPlayer.getWidget(np_Password)->setEnabled(false);

    // Hide the AI stuff until 'Computer' type is selected
    cNewPlayer.getWidget(np_AIDiffLbl)->setEnabled(false);
	cNewPlayer.getWidget(np_AIDiff)->setEnabled(false);


	cNewPlayer.SendMessage( np_Password, TXM_SETFLAGS, TXF_PASSWORD, 0);
	cNewPlayer.SendMessage( np_Password, TXM_SETMAX, 15, 0);
	cNewPlayer.SendMessage( np_Username, TXM_SETMAX, 15, 0);

	// Set the default colour
	cNewPlayer.SendMessage( np_Red,		SLM_SETVALUE, 128, 0);
	cNewPlayer.SendMessage( np_Green,	SLM_SETVALUE, 128, 0);
	cNewPlayer.SendMessage( np_Blue,	SLM_SETVALUE, 128, 0);

	// Player type
    cNewPlayer.SendMessage( np_Type, CBM_ADDITEM, PRF_HUMAN, (DWORD)"Human" );
    cNewPlayer.SendMessage( np_Type, CBM_ADDITEM, PRF_COMPUTER, (DWORD)"Computer" );

    Menu_Player_NewPlayerInit();

	// View players
	cViewPlayers.Shutdown();
	cViewPlayers.Initialize();

	cViewPlayers.Add( new CButton(BUT_BACK, tMenu->bmpButtons),     vp_Back,   25, 440, 50, 15);
	cViewPlayers.Add( new CListview(),                              vp_Players,40, 150, 200,165);
    cViewPlayers.Add( new CLabel("Name", tLX->clNormalLabel),                   Static, 350,172, 0,  0);
    cViewPlayers.Add( new CTextbox(),                               vp_Name,  400,170, 120,20);
	cViewPlayers.Add( new CButton(BUT_DELETE, tMenu->bmpButtons),   vp_Delete, 330,340, 70, 15);
    cViewPlayers.Add( new CButton(BUT_APPLY, tMenu->bmpButtons),    vp_Apply,  500,340, 55, 15);

    cViewPlayers.Add( new CLabel("Red",tLX->clNormalLabel),                     Static, 350,250,0,0);
	cViewPlayers.Add( new CLabel("Green",tLX->clNormalLabel),                   Static, 350,270,0,0);
	cViewPlayers.Add( new CLabel("Blue",tLX->clNormalLabel),                    Static, 350,290,0,0);
    cViewPlayers.Add( new CSlider(255),	                            vp_Red,    400,250, 128,20);
	cViewPlayers.Add( new CSlider(255),	                            vp_Green,  400,270, 128,20);
	cViewPlayers.Add( new CSlider(255),	                            vp_Blue,   400,290, 128,20);

    cViewPlayers.Add( new CLabel("Skill", tLX->clNormalLabel),                  vp_AIDiffLbl,350,312,0, 0);
    cViewPlayers.Add( new CSlider(3),                               vp_AIDiff, 400,310, 128,20);
    cViewPlayers.Add( new CLabel("Skin", tLX->clNormalLabel),                   Static, 350,227, 0,  0);
    cViewPlayers.Add( new CCombobox(),                              vp_PlySkin,400,225, 120,17);
    cViewPlayers.Add( new CLabel("Type", tLX->clNormalLabel),                   Static, 350,202, 0,  0);
    cViewPlayers.Add( new CCombobox(),                              vp_Type,   400,200, 120,17);

	cViewPlayers.SendMessage(vp_Name,TXM_SETMAX,20,0);
	cViewPlayers.SendMessage(vp_Players,		LVM_SETOLDSTYLE, 0, 0);

    // Hide the AI stuff until 'Computer' type is selected
    cViewPlayers.getWidget(vp_AIDiffLbl)->setEnabled(false);
	cViewPlayers.getWidget(vp_AIDiff)->setEnabled(false);


	lv = (CListview *)cViewPlayers.getWidget(vp_Players);
	lv->AddColumn("Players",22);
	lv->AddColumn("",60);

    cViewPlayers.SendMessage( vp_Type, CBM_ADDITEM, PRF_HUMAN, (DWORD)"Human" );
    cViewPlayers.SendMessage( vp_Type, CBM_ADDITEM, PRF_COMPUTER, (DWORD)"Computer" );
}

///////////////
// Shutdown
void Menu_PlayerShutdown(void)
{
	cNewPlayer.Shutdown();
	cViewPlayers.Shutdown();
}


///////////////////
// Player frame
void Menu_PlayerFrame(void)
{
	mouse_t *Mouse = GetMouse();
	int mouse = 0;

	DrawImageAdv(tMenu->bmpScreen, tMenu->bmpBuffer, 20,140, 20,140, 620,340);
	DrawImageAdv(tMenu->bmpScreen, tMenu->bmpBuffer, 140,110,  140,110,  400,30);


	// Process the top buttons
	cPlyButtons[iPlayerMode].MouseOver(Mouse);
	for(int i=0;i<2;i++) {

		cPlyButtons[i].Draw(tMenu->bmpScreen);

		if(i==iPlayerMode)
			continue;

		if(cPlyButtons[i].InBox(Mouse->X,Mouse->Y)) {
			cPlyButtons[i].MouseOver(Mouse);
			mouse = 1;
			if(Mouse->Up) {
				iPlayerMode = i;
                if( i == 0 )
                    Menu_Player_NewPlayerInit();
                else
                    Menu_Player_ViewPlayerInit();
				PlaySoundSample(sfxGeneral.smpClick);
			}
		}
	}


	if(iPlayerMode == 0)
		Menu_Player_NewPlayer(mouse);

	if(iPlayerMode == 1)
		Menu_Player_ViewPlayers(mouse);
}


///////////////////
// Initialize the newplayer settings
void Menu_Player_NewPlayerInit(void)
{
    cNewPlayer.SendMessage( np_Name,    TXM_SETTEXT, (DWORD)"", 0);
    cNewPlayer.SendMessage( np_Type,    CBM_SETCURSEL, PRF_HUMAN, 0 );
    cNewPlayer.SendMessage( np_Red,		SLM_SETVALUE, 128, 0);
	cNewPlayer.SendMessage( np_Green,	SLM_SETVALUE, 128, 0);
	cNewPlayer.SendMessage( np_Blue,	SLM_SETVALUE, 128, 0);

    // Hide the AI stuff until 'Computer' type is selected
    cNewPlayer.getWidget(np_AIDiffLbl)->setEnabled(false);
	cNewPlayer.getWidget(np_AIDiff)->setEnabled(false);

    // Fill the skin combo box
    Menu_Player_FillSkinCombo( (CCombobox *)cNewPlayer.getWidget(np_PlySkin) );

    // Load the default skin
    tMenu->bmpWorm = LoadImage("skins/default.png", 16);
    fPlayerSkinFrame = 0;
    bPlayerSkinAnimation = false;
}


///////////////////
// Initialize the viewplayer settings
void Menu_Player_ViewPlayerInit(void)
{
    // Add the players to the list
	CListview *lv = (CListview *)cViewPlayers.getWidget(vp_Players);
    lv->Clear();

	profile_t *p = GetProfiles();
	for(; p; p=p->tNext) {
		lv->AddItem("",p->iID,tLX->clListView);
		lv->AddSubitem(LVS_IMAGE,"",p->bmpWorm);
		lv->AddSubitem(LVS_TEXT, p->sName, NULL);
	}


    // Fill the skin combo box
    Menu_Player_FillSkinCombo( (CCombobox *)cViewPlayers.getWidget(vp_PlySkin) );

    // Set the name of the first item in the list
    int sel = cViewPlayers.SendMessage( vp_Players, LVM_GETCURINDEX,0,0);
	p = FindProfile(sel);
    if(p) {
        cViewPlayers.SendMessage( vp_Name,    TXM_SETTEXT, (DWORD)&p->sName,0);

        cViewPlayers.SendMessage( vp_Red,	    SLM_SETVALUE, p->R, 0);
	    cViewPlayers.SendMessage( vp_Green,		SLM_SETVALUE, p->G, 0);
	    cViewPlayers.SendMessage( vp_Blue,	    SLM_SETVALUE, p->B, 0);
        cViewPlayers.SendMessage( vp_Type,		CBM_SETCURSEL,  p->iType, 0);
        cViewPlayers.SendMessage( vp_AIDiff,	SLM_SETVALUE,   p->nDifficulty, 0);
        cViewPlayers.SendMessage( vp_PlySkin,	CBM_SETCURSINDEX,(DWORD)&p->szSkin, 0);

        // Hide the AI stuff if it is a human type of player
        cViewPlayers.getWidget(vp_AIDiffLbl)->setEnabled(p->iType == PRF_COMPUTER);
	    cViewPlayers.getWidget(vp_AIDiff)->setEnabled(p->iType == PRF_COMPUTER);

        // Load the skin
        tMenu->bmpWorm = LoadImage("skins/"+p->szSkin, 16);
        fPlayerSkinFrame = 0;
        bPlayerSkinAnimation = false;
    }
}


///////////////////
// New player section
void Menu_Player_NewPlayer(int mouse)
{
	gui_event_t *ev = NULL;
	mouse_t *Mouse = GetMouse();

	// Process & draw the gui
	if (!cMediaPlayer.GetDrawPlayer())
		ev = cNewPlayer.Process();
	cNewPlayer.Draw(tMenu->bmpScreen);

	Uint8 r = ((CSlider *)cNewPlayer.getWidget(np_Red))->getValue();
	Uint8 g = ((CSlider *)cNewPlayer.getWidget(np_Green))->getValue();
	Uint8 b = ((CSlider *)cNewPlayer.getWidget(np_Blue))->getValue();



	if(ev) {

		if(ev->cWidget->getType() == wid_Button)
			mouse = 1;
		if(ev->cWidget->getType() == wid_Textbox)
			mouse = 2;

		switch(ev->iControlID) {

			// Back button
			case np_Back:
				if(ev->iEventMsg == BTN_MOUSEUP) {

					// Shutdown
					Menu_PlayerShutdown();
					//SaveProfiles();

					// Leave
					PlaySoundSample(sfxGeneral.smpClick);
					Menu_MainInitialize();
					return;
				}
				break;

			// Create
			case np_Create:
				if(ev->iEventMsg == BTN_MOUSEUP) {
					PlaySoundSample(sfxGeneral.smpClick);

					// Get the details
					std::string name = ((CTextbox *)cNewPlayer.getWidget(np_Name))->getText();
					std::string skin;
                    cNewPlayer.SendMessage(np_PlySkin, CBM_GETCURSINDEX, (DWORD)&skin, 255);

					// Add the profile
                    int type = cNewPlayer.SendMessage(np_Type,CBM_GETCURINDEX,0,0);
                    int level = cNewPlayer.SendMessage(np_AIDiff,SLM_GETVALUE,0,0);

					AddProfile(name, skin, "", "",r, g, b, type,level);

					// Shutdown
					cNewPlayer.Shutdown();
					cViewPlayers.Shutdown();
					//SaveProfiles();

					Menu_MainInitialize();
					return;
				}
				break;

            // Type
            case np_Type:
                if(ev->iEventMsg == CMB_CHANGED) {

                    int type = cNewPlayer.SendMessage(np_Type,CBM_GETCURINDEX,0,0);

                    // Hide the AI stuff if it is a human type of player
                    cNewPlayer.getWidget(np_AIDiffLbl)->setEnabled(type == PRF_COMPUTER);
	                cNewPlayer.getWidget(np_AIDiff)->setEnabled(type == PRF_COMPUTER);
                }
                break;

            // Skin
            case np_PlySkin:
                if(ev->iEventMsg == CMB_CHANGED) {
                    static char skin[256];
                    static char buf[256];
                    cNewPlayer.SendMessage(np_PlySkin, CBM_GETCURSINDEX, (DWORD)skin, 255);

                    // Load the skin
                   snprintf(buf,sizeof(buf),"skins/%s",skin); fix_markend(buf);
                    tMenu->bmpWorm = LoadImage(buf, 16);
                }
                break;
		}
	}


	// Draw the colour
	//DrawRectFill(tMenu->bmpScreen, 260, 230, 280, 250, MakeColour(r,g,b));
	DrawRectFill(tMenu->bmpScreen,  255, 195, 285, 225, 0);
	Menu_DrawBox(tMenu->bmpScreen,  255, 195, 285, 225);
	//DrawRect(tMenu->bmpScreen,		255, 195, 285, 225, MakeColour(128,128,128));


	// Draw the colour component values
	tLX->cFont.Draw(tMenu->bmpScreen, 250, 303, 0xffff, "%d",r);
	tLX->cFont.Draw(tMenu->bmpScreen, 250, 323, 0xffff, "%d",g);
	tLX->cFont.Draw(tMenu->bmpScreen, 250, 343, 0xffff, "%d",b);

    if(MouseInRect(255,195,30,30) && Mouse->Up)
        bPlayerSkinAnimation = !bPlayerSkinAnimation;

	Menu_Player_DrawWormImage(tMenu->bmpScreen, (int)(fPlayerSkinFrame)*7+4, 257, 200, r,g,b);
    if(bPlayerSkinAnimation)
        fPlayerSkinFrame += tLX->fDeltaTime*5;
    if( fPlayerSkinFrame >= 3 )
        fPlayerSkinFrame = 0;


    // Draw the difficulty level
    int type = cNewPlayer.SendMessage(np_Type,CBM_GETCURINDEX,0,0);
    if( type == PRF_COMPUTER ) {
        static std::string difflevels[] = {"Easy", "Medium", "Hard", "Xtreme"};
        int level = cNewPlayer.SendMessage(np_AIDiff,SLM_GETVALUE,0,0);
        tLX->cFont.Draw(tMenu->bmpScreen, 250,363,0xffff,"%s",difflevels[level].c_str());

    }

	// Draw the mouse
	DrawImage(tMenu->bmpScreen,gfxGUI.bmpMouse[mouse], Mouse->X,Mouse->Y);
}


///////////////////
// View the players screen
void Menu_Player_ViewPlayers(int mouse)
{
	mouse_t *Mouse = GetMouse();
	gui_event_t *ev = NULL;
	static std::string buf;

	// Process & draw the gui
	if (!cMediaPlayer.GetDrawPlayer())
		ev = cViewPlayers.Process();
	cViewPlayers.Draw(tMenu->bmpScreen);


	if(ev) {

		if(ev->cWidget->getType() == wid_Button)
			mouse = 1;
		if(ev->cWidget->getType() == wid_Textbox)
			mouse = 2;

		switch(ev->iControlID) {

			// Back button
			case vp_Back:
				if(ev->iEventMsg == BTN_MOUSEUP) {

					// Shutdown
					Menu_PlayerShutdown();
					//SaveProfiles();

					// Leave
					PlaySoundSample(sfxGeneral.smpClick);
					Menu_MainInitialize();
					return;
				}
				break;

			// Delete
			case vp_Delete:
				if(ev->iEventMsg == BTN_MOUSEUP) {

					CListview *lv = (CListview *)cViewPlayers.getWidget(vp_Players);

					int sel = lv->getCurIndex();
					profile_t *p = FindProfile(sel);
					if(p) {
						/*if(p->iType == PRF_HUMAN)*/ {

							//
							// Show a msgbox confirming deletion
							//

							// Setup the buffer
							Mouse->Button = Mouse->Up = Mouse->Down = 0;
							Mouse->X = Mouse->Y = 0;
							cViewPlayers.Draw(tMenu->bmpBuffer);

							for(int i=0;i<2;i++)
								cPlyButtons[i].Draw(tMenu->bmpBuffer);


							// Ask if they are sure they wanna delete it
							buf = std::string("Delete player ") + p->sName;
							if(Menu_MessageBox("Confirmation",buf,LMB_YESNO) == MBR_YES) {

								// Delete the profile
								DeleteProfile(p->iID);

								// Add the players to the list
								lv->Create();
								lv->AddColumn("Players",22);
								lv->AddColumn("",60);
								p = GetProfiles();
								for(; p; p=p->tNext) {
									//if(p->iType == PRF_COMPUTER)
									//	continue;
									lv->AddItem("",p->iID,tLX->clListView);
									lv->AddSubitem(LVS_IMAGE,"",p->bmpWorm);
									lv->AddSubitem(LVS_TEXT, p->sName, NULL);
								}
							}
						}

                        // Update the details
                        int sel = cViewPlayers.SendMessage(vp_Players,LVM_GETCURINDEX,0,0);
	                    p = FindProfile(sel);
                        if(p) {
                            cViewPlayers.SendMessage( vp_Name,		TXM_SETTEXT,    (DWORD)&p->sName,0);
                            cViewPlayers.SendMessage( vp_Red,	    SLM_SETVALUE,   p->R, 0);
	                        cViewPlayers.SendMessage( vp_Green,		SLM_SETVALUE,   p->G, 0);
	                        cViewPlayers.SendMessage( vp_Blue,	    SLM_SETVALUE,   p->B, 0);
                            cViewPlayers.SendMessage( vp_Type,		CBM_SETCURSEL,  p->iType, 0);
                            cViewPlayers.SendMessage( vp_AIDiff,	SLM_SETVALUE,   p->nDifficulty, 0);

                            // Hide the AI stuff if it is a human type of player
                            cViewPlayers.getWidget(vp_AIDiffLbl)->setEnabled(p->iType == PRF_COMPUTER);
	                        cViewPlayers.getWidget(vp_AIDiff)->setEnabled(p->iType == PRF_COMPUTER);
                        }



						// Re-draw the buffer again
						DrawImage(tMenu->bmpBuffer,tMenu->bmpMainBack_wob,0,0);
                        Menu_DrawBox(tMenu->bmpBuffer, 15,130, 625, 465);
						Menu_DrawSubTitle(tMenu->bmpBuffer,SUB_PLAYER);
						Menu_RedrawMouse(true);

					}

					// Play a click
					PlaySoundSample(sfxGeneral.smpClick);
				}
				break;


            // Apply button
            case vp_Apply:
                if( ev->iEventMsg == BTN_MOUSEUP ) {
                    int sel = cViewPlayers.SendMessage(vp_Players, LVM_GETCURINDEX, 0,0);
	                profile_t *p = FindProfile(sel);
	                if(p) {
                        cViewPlayers.SendMessage(vp_Name, TXM_GETTEXT, (DWORD)&p->sName, p->sName.length());
                        p->R = cViewPlayers.SendMessage(vp_Red,SLM_GETVALUE,0,0);
                        p->G = cViewPlayers.SendMessage(vp_Green,SLM_GETVALUE,0,0);
                        p->B = cViewPlayers.SendMessage(vp_Blue,SLM_GETVALUE,0,0);
                        p->iType = cViewPlayers.SendMessage(vp_Type, CBM_GETCURINDEX,0,0);
                        p->nDifficulty = cViewPlayers.SendMessage(vp_AIDiff, SLM_GETVALUE,0,0);
                        cViewPlayers.SendMessage(vp_PlySkin, CBM_GETCURSINDEX, (DWORD)&p->szSkin, p->szSkin.length());

                        // Re-load the profile's graphics
                        LoadProfileGraphics(p);

                        // Update the item
                        lv_item_t *it = (lv_item_t *)cViewPlayers.SendMessage(vp_Players, LVM_GETCURITEM, 0,0);
                        if(it) {
                            if(it->tSubitems) {
                                it->tSubitems->bmpImage = p->bmpWorm;
                                if(it->tSubitems->tNext)
                                    it->tSubitems->tNext->sText = p->sName;
                            }
                        }

                        // Add the players to the list

                        /*CListview *lv = (CListview *)cViewPlayers.getWidget(vp_Players);
						lv->Create();
						lv->AddColumn("Players",22);
						lv->AddColumn("",60);
						p = GetProfiles();
						for(; p; p=p->tNext) {
							lv->AddItem("",p->iID,tLX->clListView);
							lv->AddSubitem(LVS_IMAGE,"",p->bmpWorm);
							lv->AddSubitem(LVS_TEXT, p->sName, NULL);
						}*/
                    }
                }
                break;


            // Player listbox
            case vp_Players:
                if( ev->iEventMsg == LV_CHANGED ) {
                    int sel = cViewPlayers.SendMessage(vp_Players,LVM_GETCURINDEX,0,0);
	                profile_t *p = FindProfile(sel);
                    if(p) {
                        cViewPlayers.SendMessage( vp_Name,		TXM_SETTEXT,    (DWORD)&p->sName,0);
                        cViewPlayers.SendMessage( vp_Red,	    SLM_SETVALUE,   p->R, 0);
	                    cViewPlayers.SendMessage( vp_Green,		SLM_SETVALUE,   p->G, 0);
	                    cViewPlayers.SendMessage( vp_Blue,	    SLM_SETVALUE,   p->B, 0);
                        cViewPlayers.SendMessage( vp_Type,		CBM_SETCURSEL,  p->iType, 0);
                        cViewPlayers.SendMessage( vp_AIDiff,	SLM_SETVALUE,   p->nDifficulty, 0);
                        cViewPlayers.SendMessage( vp_PlySkin,	CBM_SETCURSINDEX,(DWORD)&p->szSkin, 0);

                        // Load the skin
                        static std::string buf;
                        buf = "skins/" + p->szSkin;
                        tMenu->bmpWorm = LoadImage(buf, 16);

                        // Hide the AI stuff if it is a human type of player
                        cViewPlayers.getWidget(vp_AIDiffLbl)->setEnabled(p->iType == PRF_COMPUTER);
	                    cViewPlayers.getWidget(vp_AIDiff)->setEnabled(p->iType == PRF_COMPUTER);
                    }
                }
                break;

            // Type
            case vp_Type:
                if( ev->iEventMsg == CMB_CHANGED ) {

                    int type = cViewPlayers.SendMessage(vp_Type,CBM_GETCURINDEX,0,0);

                    // Hide the AI stuff if it is a human type of player
                    cViewPlayers.getWidget(vp_AIDiffLbl)->setEnabled(type == PRF_COMPUTER);
	                cViewPlayers.getWidget(vp_AIDiff)->setEnabled(type == PRF_COMPUTER);
                }
                break;

            // Skin
            case vp_PlySkin:
                if(ev->iEventMsg == CMB_CHANGED) {
                    static char skin[256];
                    static char buf[256];
                    cViewPlayers.SendMessage(vp_PlySkin, CBM_GETCURSINDEX, (DWORD)skin, 255);

                    // Load the skin
                    snprintf(buf,sizeof(buf),"skins/%s",skin);
                    fix_markend(buf);
                    tMenu->bmpWorm = LoadImage(buf, 16);
                }
                break;
		}
	}


	// Show info about the selected player
	CListview *lv = (CListview *)cViewPlayers.getWidget(vp_Players);

	int sel = lv->getCurIndex();
	profile_t *p = FindProfile(sel);
	if(p) {

        Uint8 r = ((CSlider *)cViewPlayers.getWidget(vp_Red))->getValue();
	    Uint8 g = ((CSlider *)cViewPlayers.getWidget(vp_Green))->getValue();
	    Uint8 b = ((CSlider *)cViewPlayers.getWidget(vp_Blue))->getValue();

        tLX->cFont.Draw(tMenu->bmpScreen, 530, 253, 0xffff, "%d",r);
	    tLX->cFont.Draw(tMenu->bmpScreen, 530, 273, 0xffff, "%d",g);
	    tLX->cFont.Draw(tMenu->bmpScreen, 530, 293, 0xffff, "%d",b);

		// Draw the worm image
		DrawRectFill(tMenu->bmpScreen,  300, 165, 330, 195, 0);
		Menu_DrawBox(tMenu->bmpScreen,  300, 165, 330, 195);

		Menu_Player_DrawWormImage(tMenu->bmpScreen,(int)(fPlayerSkinFrame)*7+4, 301, 170, r, g, b);
        if(MouseInRect(300,165,30,30) && Mouse->Up)
            bPlayerSkinAnimation = !bPlayerSkinAnimation;

        if(bPlayerSkinAnimation)
            fPlayerSkinFrame += tLX->fDeltaTime*5;
        if( fPlayerSkinFrame >= 3 )
            fPlayerSkinFrame = 0;
	}

    // Draw the difficulty level
    int type = cViewPlayers.SendMessage(vp_Type,CBM_GETCURINDEX,0,0);
    if( type == PRF_COMPUTER ) {
        static const char* difflevels[] = {"Easy", "Medium", "Hard", "Xtreme"};
        int level = cViewPlayers.SendMessage(vp_AIDiff,SLM_GETVALUE,0,0);
        tLX->cFont.Draw(tMenu->bmpScreen, 530,313,0xffff,"%s",difflevels[level].c_str());
    }


	// Draw the mouse
	DrawImage(tMenu->bmpScreen,gfxGUI.bmpMouse[mouse], Mouse->X,Mouse->Y);
}


///////////////////
// Draw the worm image
void Menu_Player_DrawWormImage(SDL_Surface *bmpDest, int Frame, int dx, int dy, int ColR, int ColG, int ColB)
{
    if( !tMenu->bmpWorm )
        return;

    // Set the colour of the worm
	int x,y,sx;
	Uint8 r,g,b,a;
	Uint32 pixel, mask;
	float r2,g2,b2;

	for(y=0; y<18; y++) {
		for(x=Frame*32,sx=0; x<(Frame*32)+32; x++,sx++) {

			pixel = GetPixel(tMenu->bmpWorm,x,y);
            mask = GetPixel(tMenu->bmpWorm,x,y+18);
			GetColour4(pixel,tMenu->bmpWorm,&r,&g,&b,&a);

            //
            // Use the mask to check what colours to ignore
            //

            // Black means to just copy the colour but don't alter it
            if( mask == 0 ) {
                PutPixel(bmpDest, sx+dx,y+dy, pixel);
                continue;
            }

            // Pink means just ignore the pixel completely
            if( mask == tLX->clPink )
                continue;

            // Must be white (or some over unknown colour)
			float dr, dg, db;

			dr = (float)r / 96.0f;
			dg = (float)g / 156.0f;
			db = (float)b / 252.0f;

			r2 = (float)ColR * dr;
			g2 = (float)ColG * dg;
			b2 = (float)ColB * db;

			r2 = MIN((float)255,r2);
			g2 = MIN((float)255,g2);
			b2 = MIN((float)255,b2);


			// Bit of a hack to make sure it isn't completey pink (see through)
			if(MakeColour((int)r2, (int)g2, (int)b2) == tLX->clPink) {
				r2=240;
				b2=240;
			}

            // Put the colourised pixel
			PutPixel(bmpDest,sx+dx,y+dy, MakeColour((int)r2, (int)g2, (int)b2));
		}
	}
}



	class SkinAdder { public:
	   	CCombobox* cb;
	   	int* def;
	   	int index;
		SkinAdder(CCombobox* cb_, int* d) : cb(cb_), def(d), index(0) {}
		inline bool operator() (std::string file) {
			std::string ext = file.substr(file.size()-4);
			if(stringcasecmp(ext, ".tga")==0
			|| stringcasecmp(ext, ".png")==0
			|| stringcasecmp(ext, ".bmp")==0
			|| stringcasecmp(ext, ".pcx")==0) {
				size_t slash = findLastPathSep(file);
				if(slash != std::string::npos)
					file.erase(0, slash+1);
				
				std::string name = file.substr(0, file.size()-4);
				cb->addItem(index, file, name);
				
				if(stringcasecmp(name, "default")==0)
					*def = index;
				
				index++;
			}
			return true;
		}
	};


///////////////////
// Fill the skin combo box
void Menu_Player_FillSkinCombo(CCombobox *cb) {
    if( !cb )
        return;

	cb->clear();
    int def = -1;
        
    FindFiles(SkinAdder(cb, &def), "skins", FM_REG);
    
	// Ascending sort the list
	cb->Sort(true);

    // Select the default
    cb->setCurItem(def);
}
