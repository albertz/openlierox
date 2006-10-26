/////////////////////////////////////////
//
//             Liero Xtreme
//
//     Copyright Auxiliary Software 2002
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

enum {
	Static=-1,
	Back=0,
	Create,
	Name,
    Name2,
	Red, Blue, Green,
	Players,
	Delete,
	Username,
	Password,
    Apply,
    Type,
    AIDiffLbl,
    AIDiff,
    PlySkin
};


///////////////////
// Initialize the player menu
void Menu_PlayerInitialize(void)
{
	tMenu->iMenuType = MNU_PLAYER;

	Uint32 blue = MakeColour(0,138,251);
	Uint32 grey = MakeColour(128,128,128);
	iPlayerMode = 0;
	CListview *lv;

	// Create the buffer
	DrawImage(tMenu->bmpBuffer,tMenu->bmpMainBack_wob,0,0);
    Menu_DrawBox(tMenu->bmpBuffer, 15,130, 625, 465);
	Menu_DrawSubTitle(tMenu->bmpBuffer,SUB_PLAYER);

	Menu_RedrawMouse(true);



	// Setup the top buttons
	cPlyButtons[0] = CButton(BUT_NEWPLAYER,		tMenu->bmpButtons);
	cPlyButtons[1] = CButton(BUT_VIEWPLAYERS,	tMenu->bmpButtons);	

	cPlyButtons[0].Setup(0, 150, 110, 120, 15);
	cPlyButtons[1].Setup(1, 370, 110, 135, 15);
    cPlyButtons[0].Create();
    cPlyButtons[1].Create();


	// New player
	cNewPlayer.Shutdown();
	cNewPlayer.Initialize();
	cNewPlayer.Add( new CButton(BUT_BACK, tMenu->bmpButtons),	Back, 25,440, 50,15);
	cNewPlayer.Add( new CButton(BUT_CREATE, tMenu->bmpButtons), Create, 540,440, 70,15);

	cNewPlayer.Add( new CLabel("Worm Details", blue),Static, 30, 170, 0,0);
	cNewPlayer.Add( new CTextbox(),		       Name,  120, 200,120,20);
	cNewPlayer.Add( new CLabel("Name",0xffff), Static, 40, 202,0,  0);
	cNewPlayer.Add( new CLabel("Red",0xffff),  Static, 40, 300,0,  0);
	cNewPlayer.Add( new CLabel("Green",0xffff),Static, 40, 320,0,  0);
	cNewPlayer.Add( new CLabel("Blue",0xffff), Static, 40, 340,0,  0);
	cNewPlayer.Add( new CSlider(255),	       Red,   115, 300,128,20);
	cNewPlayer.Add( new CSlider(255),	       Green, 115, 320,128,20);
	cNewPlayer.Add( new CSlider(255),	       Blue,  115, 340,128,20);

    cNewPlayer.Add( new CLabel("Skill",0xffff),AIDiffLbl,40,362,0, 0);
    cNewPlayer.Add( new CSlider(3),            AIDiff,115, 360,128,20);
    cNewPlayer.Add( new CLabel("Skin", 0xffff),Static,40,  262,0,  0);
    cNewPlayer.Add( new CCombobox(),           PlySkin,120,260,120,20);
    cNewPlayer.Add( new CLabel("Type", 0xffff),Static,40,  232,0,  0);
    cNewPlayer.Add( new CCombobox(),           Type,  120, 230,120,17); 
	
	cNewPlayer.SendMessage(Name,TXM_SETMAX,20,0);
	

	//cNewPlayer.Add( new CLabel("Multiplayer (optional)", blue),Static, 370, 170, 0,0);
	//cNewPlayer.Add( new CLabel("Username", 0xffff), Static, 380, 202, 0,0);
	//cNewPlayer.Add( new CLabel("Password", 0xffff), Static, 380, 232, 0,0);
	cNewPlayer.Add( new CTextbox(),			   Username, 470, 200, 110, 20);
	cNewPlayer.Add( new CTextbox(),			   Password, 470, 230, 110, 20);

	//cNewPlayer.Add( new CLabel("Note: To register a username, visit the Liero Xtreme web site", grey),Static, 30, 410, 0,0);

	// Hide the multiplayer textboxes
	cNewPlayer.getWidget(Username)->setEnabled(false);
	cNewPlayer.getWidget(Password)->setEnabled(false);

    // Hide the AI stuff until 'Computer' type is selected
    cNewPlayer.getWidget(AIDiffLbl)->setEnabled(false);
	cNewPlayer.getWidget(AIDiff)->setEnabled(false);

	
	cNewPlayer.SendMessage( Password, TXM_SETFLAGS, TXF_PASSWORD, 0);
	cNewPlayer.SendMessage( Password, TXM_SETMAX, 15, 0);
	cNewPlayer.SendMessage( Username, TXM_SETMAX, 15, 0);

	// Set the default colour
	cNewPlayer.SendMessage( Red,	SLM_SETVALUE, 128, 0);
	cNewPlayer.SendMessage( Green,	SLM_SETVALUE, 128, 0);
	cNewPlayer.SendMessage( Blue,	SLM_SETVALUE, 128, 0);

    Menu_Player_NewPlayerInit();

	// View players
	cViewPlayers.Shutdown();
	cViewPlayers.Initialize();

	cViewPlayers.Add( new CButton(BUT_BACK, tMenu->bmpButtons),     Back,   25, 440, 50, 15);
	cViewPlayers.Add( new CListview(),                              Players,40, 150, 200,165);
    cViewPlayers.Add( new CLabel("Name", 0xffff),                   Static, 350,172, 0,  0);
    cViewPlayers.Add( new CTextbox(),                               Name2,  400,170, 120,20);
	cViewPlayers.Add( new CButton(BUT_DELETE, tMenu->bmpButtons),   Delete, 330,340, 70, 15);
    cViewPlayers.Add( new CButton(BUT_APPLY, tMenu->bmpButtons),    Apply,  500,340, 55, 15);    

    cViewPlayers.Add( new CLabel("Red",0xffff),                     Static, 350,250,0,0);
	cViewPlayers.Add( new CLabel("Green",0xffff),                   Static, 350,270,0,0);
	cViewPlayers.Add( new CLabel("Blue",0xffff),                    Static, 350,290,0,0);
    cViewPlayers.Add( new CSlider(255),	                            Red,    400,250, 128,20);
	cViewPlayers.Add( new CSlider(255),	                            Green,  400,270, 128,20);
	cViewPlayers.Add( new CSlider(255),	                            Blue,   400,290, 128,20);

    cViewPlayers.Add( new CLabel("Skill", 0xffff),                  AIDiffLbl,350,312,0, 0);
    cViewPlayers.Add( new CSlider(3),                               AIDiff, 400,310, 128,20);
    cViewPlayers.Add( new CLabel("Skin", 0xffff),                   Static, 350,227, 0,  0);
    cViewPlayers.Add( new CCombobox(),                              PlySkin,400,225, 120,17);
    cViewPlayers.Add( new CLabel("Type", 0xffff),                   Static, 350,202, 0,  0);
    cViewPlayers.Add( new CCombobox(),                              Type,   400,200, 120,17);    

	cViewPlayers.SendMessage(Name2,TXM_SETMAX,20,0);

    // Hide the AI stuff until 'Computer' type is selected
    cViewPlayers.getWidget(AIDiffLbl)->setEnabled(false);
	cViewPlayers.getWidget(AIDiff)->setEnabled(false);


	lv = (CListview *)cViewPlayers.getWidget(Players);
	lv->AddColumn("Players",22);
	lv->AddColumn("",60);

    cNewPlayer.SendMessage( Type, CBM_ADDITEM, PRF_HUMAN, (DWORD)"Human" );
    cNewPlayer.SendMessage( Type, CBM_ADDITEM, PRF_COMPUTER, (DWORD)"Computer" );
    cViewPlayers.SendMessage( Type, CBM_ADDITEM, PRF_HUMAN, (DWORD)"Human" );
    cViewPlayers.SendMessage( Type, CBM_ADDITEM, PRF_COMPUTER, (DWORD)"Computer" );
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
// TODO: implement sound system
//				BASS_SamplePlay(sfxGeneral.smpClick);
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
    cNewPlayer.SendMessage( Name,   TXM_SETTEXT, (DWORD)"", 0);
    cNewPlayer.SendMessage( Type,   CBM_SETCURSEL, PRF_HUMAN, 0 );
    cNewPlayer.SendMessage( Red,	SLM_SETVALUE, 128, 0);
	cNewPlayer.SendMessage( Green,	SLM_SETVALUE, 128, 0);
	cNewPlayer.SendMessage( Blue,	SLM_SETVALUE, 128, 0);

    // Hide the AI stuff until 'Computer' type is selected
    cNewPlayer.getWidget(AIDiffLbl)->setEnabled(false);
	cNewPlayer.getWidget(AIDiff)->setEnabled(false);

    // Fill the skin combo box
    Menu_Player_FillSkinCombo( (CCombobox *)cNewPlayer.getWidget(PlySkin) );

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
	CListview *lv = (CListview *)cViewPlayers.getWidget(Players);
    lv->Clear();

	profile_t *p = GetProfiles();
	for(; p; p=p->tNext) {
		lv->AddItem("",p->iID,0xffff);
		lv->AddSubitem(LVS_IMAGE,"",p->bmpWorm);
		lv->AddSubitem(LVS_TEXT, p->sName, NULL);
	}

    
    // Fill the skin combo box
    Menu_Player_FillSkinCombo( (CCombobox *)cViewPlayers.getWidget(PlySkin) );

    // Set the name of the first item in the list
    int sel = cViewPlayers.SendMessage( Players, LVM_GETCURINDEX,0,0);
	p = FindProfile(sel);
    if(p) {
        cViewPlayers.SendMessage( Name2,    TXM_SETTEXT, (DWORD)p->sName,0);

        cViewPlayers.SendMessage( Red,	    SLM_SETVALUE, p->R, 0);
	    cViewPlayers.SendMessage( Green,	SLM_SETVALUE, p->G, 0);
	    cViewPlayers.SendMessage( Blue,	    SLM_SETVALUE, p->B, 0);
        cViewPlayers.SendMessage( Type,     CBM_SETCURSEL,  p->iType, 0);
        cViewPlayers.SendMessage( AIDiff,   SLM_SETVALUE,   p->nDifficulty, 0);
        cViewPlayers.SendMessage( PlySkin,  CBM_SETCURSINDEX,(DWORD)p->szSkin, 0);

        // Hide the AI stuff if it is a human type of player
        cViewPlayers.getWidget(AIDiffLbl)->setEnabled(p->iType == PRF_COMPUTER);
	    cViewPlayers.getWidget(AIDiff)->setEnabled(p->iType == PRF_COMPUTER); 

        // Load the skin
        char buf[256];
        sprintf(buf,"skins/%s",p->szSkin);
        tMenu->bmpWorm = LoadImage(buf, 16);
        fPlayerSkinFrame = 0;
        bPlayerSkinAnimation = false;
    }
}


///////////////////
// New player section
void Menu_Player_NewPlayer(int mouse)
{
	gui_event_t *ev;
	mouse_t *Mouse = GetMouse();
	
	// Process & draw the gui
	ev = cNewPlayer.Process();
	cNewPlayer.Draw(tMenu->bmpScreen);

	Uint8 r = ((CSlider *)cNewPlayer.getWidget(Red))->getValue();
	Uint8 g = ((CSlider *)cNewPlayer.getWidget(Green))->getValue();
	Uint8 b = ((CSlider *)cNewPlayer.getWidget(Blue))->getValue();	



	if(ev) {

		if(ev->cWidget->getType() == wid_Button)
			mouse = 1;
		if(ev->cWidget->getType() == wid_Textbox)
			mouse = 2;

		switch(ev->iControlID) {

			// Back button
			case Back:
				if(ev->iEventMsg == BTN_MOUSEUP) {

					// Shutdown
					cNewPlayer.Shutdown();
					cViewPlayers.Shutdown();
					//SaveProfiles();

					// Leave
// TODO: implement sound system
//					BASS_SamplePlay(sfxGeneral.smpClick);
					Menu_MainInitialize();
					return;
				}
				break;

			// Create
			case Create:
				if(ev->iEventMsg == BTN_MOUSEUP) {
// TODO: implement sound system
//					BASS_SamplePlay(sfxGeneral.smpClick);

					// Get the details
					char *name = ((CTextbox *)cNewPlayer.getWidget(Name))->getText();
                    char skin[256];
                    cNewPlayer.SendMessage(PlySkin, CBM_GETCURSINDEX, (DWORD)skin, 255);

					// Add the profile
                    int type = cNewPlayer.SendMessage(Type,CBM_GETCURINDEX,0,0);
                    int level = cNewPlayer.SendMessage(AIDiff,SLM_GETVALUE,0,0);

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
            case Type:
                if(ev->iEventMsg == CMB_CHANGED) {

                    int type = cNewPlayer.SendMessage(Type,CBM_GETCURINDEX,0,0);

                    // Hide the AI stuff if it is a human type of player
                    cNewPlayer.getWidget(AIDiffLbl)->setEnabled(type == PRF_COMPUTER);
	                cNewPlayer.getWidget(AIDiff)->setEnabled(type == PRF_COMPUTER);                   
                }
                break;

            // Skin
            case PlySkin:
                if(ev->iEventMsg == CMB_CHANGED) {
                    char skin[256];
                    char buf[256];
                    cNewPlayer.SendMessage(PlySkin, CBM_GETCURSINDEX, (DWORD)skin, 255);

                    // Load the skin
                    sprintf(buf,"skins/%s",skin);
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
    int type = cNewPlayer.SendMessage(Type,CBM_GETCURINDEX,0,0);
    if( type == PRF_COMPUTER ) {
        char *difflevels[] = {"Easy", "Medium", "Hard", "Xtreme"};
        int level = cNewPlayer.SendMessage(AIDiff,SLM_GETVALUE,0,0);
        tLX->cFont.Draw(tMenu->bmpScreen, 250,363,0xffff,"%s",difflevels[level]);

    }

	// Draw the mouse
	DrawImage(tMenu->bmpScreen,gfxGUI.bmpMouse[mouse], Mouse->X,Mouse->Y);
}


///////////////////
// View the players screen
void Menu_Player_ViewPlayers(int mouse)
{
	mouse_t *Mouse = GetMouse();
	gui_event_t *ev;
	char buf[128];

	// Process & draw the gui
	ev = cViewPlayers.Process();
	cViewPlayers.Draw(tMenu->bmpScreen);


	if(ev) {

		if(ev->cWidget->getType() == wid_Button)
			mouse = 1;
		if(ev->cWidget->getType() == wid_Textbox)
			mouse = 2;

		switch(ev->iControlID) {

			// Back button
			case Back:
				if(ev->iEventMsg == BTN_MOUSEUP) {

					// Shutdown
					cNewPlayer.Shutdown();
					cViewPlayers.Shutdown();
					//SaveProfiles();

					// Leave
					BASS_SamplePlay(sfxGeneral.smpClick);
					Menu_MainInitialize();
					return;
				}
				break;

			// Delete
			case Delete:
				if(ev->iEventMsg == BTN_MOUSEUP) {

					CListview *lv = (CListview *)cViewPlayers.getWidget(Players);

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
							sprintf(buf,"Delete player '%s'",p->sName);
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
									lv->AddItem("",p->iID,0xffff);
									lv->AddSubitem(LVS_IMAGE,"",p->bmpWorm);
									lv->AddSubitem(LVS_TEXT, p->sName, NULL);
								}
							}
						}

                        // Update the details
                        int sel = cViewPlayers.SendMessage(Players,LVM_GETCURINDEX,0,0);
	                    p = FindProfile(sel);
                        if(p) {
                            cViewPlayers.SendMessage( Name2,    TXM_SETTEXT,    (DWORD)p->sName,0);
                            cViewPlayers.SendMessage( Red,	    SLM_SETVALUE,   p->R, 0);
	                        cViewPlayers.SendMessage( Green,	SLM_SETVALUE,   p->G, 0);
	                        cViewPlayers.SendMessage( Blue,	    SLM_SETVALUE,   p->B, 0);
                            cViewPlayers.SendMessage( Type,     CBM_SETCURSEL,  p->iType, 0);
                            cViewPlayers.SendMessage( AIDiff,   SLM_SETVALUE,   p->nDifficulty, 0);
                        
                            // Hide the AI stuff if it is a human type of player
                            cViewPlayers.getWidget(AIDiffLbl)->setEnabled(p->iType == PRF_COMPUTER);
	                        cViewPlayers.getWidget(AIDiff)->setEnabled(p->iType == PRF_COMPUTER); 
                        }



						// Re-draw the buffer again
						DrawImage(tMenu->bmpBuffer,tMenu->bmpMainBack_wob,0,0);
                        Menu_DrawBox(tMenu->bmpBuffer, 15,130, 625, 465);
						Menu_DrawSubTitle(tMenu->bmpBuffer,SUB_PLAYER);
						Menu_RedrawMouse(true);

					}

					// Play a click
// TODO: implement sound system
//					BASS_SamplePlay(sfxGeneral.smpClick);
				}
				break;


            // Apply button
            case Apply:
                if( ev->iEventMsg == BTN_MOUSEUP ) {
                    int sel = cViewPlayers.SendMessage(Players, LVM_GETCURINDEX, 0,0);                    
	                profile_t *p = FindProfile(sel);
	                if(p) {
                        cViewPlayers.SendMessage(Name2, TXM_GETTEXT, (DWORD)p->sName, sizeof(p->sName));
                        p->R = cViewPlayers.SendMessage(Red,SLM_GETVALUE,0,0);
                        p->G = cViewPlayers.SendMessage(Green,SLM_GETVALUE,0,0);
                        p->B = cViewPlayers.SendMessage(Blue,SLM_GETVALUE,0,0);
                        p->iType = cViewPlayers.SendMessage(Type, CBM_GETCURINDEX,0,0);
                        p->nDifficulty = cViewPlayers.SendMessage(AIDiff, SLM_GETVALUE,0,0);
                        cViewPlayers.SendMessage(PlySkin, CBM_GETCURSINDEX, (DWORD)p->szSkin, sizeof(p->szSkin));

                        // Re-load the profile's graphics
                        LoadProfileGraphics(p);

                        // Update the item
                        lv_item_t *it = (lv_item_t *)cViewPlayers.SendMessage(Players, LVM_GETCURITEM, 0,0);
                        if(it) {
                            if(it->tSubitems) {
                                it->tSubitems->bmpImage = p->bmpWorm;                            
                                if(it->tSubitems->tNext)
                                    strcpy(it->tSubitems->tNext->sText, p->sName);
                            }
                        }

                        // Add the players to the list

                        /*CListview *lv = (CListview *)cViewPlayers.getWidget(Players);
						lv->Create();
						lv->AddColumn("Players",22);
						lv->AddColumn("",60);
						p = GetProfiles();
						for(; p; p=p->tNext) {
							lv->AddItem("",p->iID,0xffff);
							lv->AddSubitem(LVS_IMAGE,"",p->bmpWorm);
							lv->AddSubitem(LVS_TEXT, p->sName, NULL);
						}*/
                    }
                }
                break;


            // Player listbox
            case Players:
                if( ev->iEventMsg == LV_CHANGED ) {                    
                    int sel = cViewPlayers.SendMessage(Players,LVM_GETCURINDEX,0,0);
	                profile_t *p = FindProfile(sel);
                    if(p) {
                        cViewPlayers.SendMessage( Name2,    TXM_SETTEXT,    (DWORD)p->sName,0);
                        cViewPlayers.SendMessage( Red,	    SLM_SETVALUE,   p->R, 0);
	                    cViewPlayers.SendMessage( Green,	SLM_SETVALUE,   p->G, 0);
	                    cViewPlayers.SendMessage( Blue,	    SLM_SETVALUE,   p->B, 0);
                        cViewPlayers.SendMessage( Type,     CBM_SETCURSEL,  p->iType, 0);
                        cViewPlayers.SendMessage( AIDiff,   SLM_SETVALUE,   p->nDifficulty, 0);
                        cViewPlayers.SendMessage( PlySkin,  CBM_SETCURSINDEX,(DWORD)p->szSkin, 0);

                        // Load the skin
                        char buf[256];
                        sprintf(buf,"skins/%s",p->szSkin);
                        tMenu->bmpWorm = LoadImage(buf, 16);
                        
                        // Hide the AI stuff if it is a human type of player
                        cViewPlayers.getWidget(AIDiffLbl)->setEnabled(p->iType == PRF_COMPUTER);
	                    cViewPlayers.getWidget(AIDiff)->setEnabled(p->iType == PRF_COMPUTER); 
                    }
                }
                break;

            // Type
            case Type:
                if( ev->iEventMsg == CMB_CHANGED ) {

                    int type = cViewPlayers.SendMessage(Type,CBM_GETCURINDEX,0,0);

                    // Hide the AI stuff if it is a human type of player
                    cViewPlayers.getWidget(AIDiffLbl)->setEnabled(type == PRF_COMPUTER);
	                cViewPlayers.getWidget(AIDiff)->setEnabled(type == PRF_COMPUTER);                   
                }
                break;

            // Skin            
            case PlySkin:
                if(ev->iEventMsg == CMB_CHANGED) {
                    char skin[256];
                    char buf[256];
                    cViewPlayers.SendMessage(PlySkin, CBM_GETCURSINDEX, (DWORD)skin, 255);

                    // Load the skin
                    sprintf(buf,"skins/%s",skin);
                    tMenu->bmpWorm = LoadImage(buf, 16);
                }
                break;
		}
	}


	// Show info about the selected player
	CListview *lv = (CListview *)cViewPlayers.getWidget(Players);

	int sel = lv->getCurIndex();
	profile_t *p = FindProfile(sel);
	if(p) {

        Uint8 r = ((CSlider *)cViewPlayers.getWidget(Red))->getValue();
	    Uint8 g = ((CSlider *)cViewPlayers.getWidget(Green))->getValue();
	    Uint8 b = ((CSlider *)cViewPlayers.getWidget(Blue))->getValue();	

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
    int type = cViewPlayers.SendMessage(Type,CBM_GETCURINDEX,0,0);
    if( type == PRF_COMPUTER ) {
        char *difflevels[] = {"Easy", "Medium", "Hard", "Xtreme"};
        int level = cViewPlayers.SendMessage(AIDiff,SLM_GETVALUE,0,0);
        tLX->cFont.Draw(tMenu->bmpScreen, 530,313,0xffff,"%s",difflevels[level]);
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
            if( mask == MakeColour(0,0,0) ) {
                PutPixel(bmpDest, sx+dx,y+dy, pixel);
                continue;
            }

            // Pink means just ignore the pixel completely
            if( mask == MakeColour(255,0,255) )
                continue;

            // Must be white (or some over unknown colour)
			float dr, dg, db;

			dr = (float)r / 96.0f;
			dg = (float)g / 156.0f;
			db = (float)b / 252.0f;

			r2 = (float)ColR * dr;
			g2 = (float)ColG * dg;
			b2 = (float)ColB * db;

			r2 = MIN(255,r2);
			g2 = MIN(255,g2);
			b2 = MIN(255,b2);


			// Bit of a hack to make sure it isn't completey pink (see through)
			if(MakeColour((int)r2, (int)g2, (int)b2) == MakeColour(255,0,255)) {
				r2=240;
				b2=240;
			}

            // Put the colourised pixel
			PutPixel(bmpDest,sx+dx,y+dy, MakeColour((int)r2, (int)g2, (int)b2));
		}
	}
}


///////////////////
// Fill the skin combo box
void Menu_Player_FillSkinCombo(CCombobox *cb)
{
    if( !cb )
        return;

    char szFilename[256];
    char szName[256];

    cb->clear();

    if( !FindFirst("skins", "*.*", szFilename) )
        return;

    int index = 0;
    int def = -1;

    while(1) {

        // Get the extension
        char ext[8];
        strcpy(ext, szFilename+strlen(szFilename)-4);

        // Is this a image type filename?
        if( stricmp(ext,".tga")==0 ||
            stricmp(ext,".png")==0 ||
            stricmp(ext,".bmp")==0 ||
            stricmp(ext,".pcx")==0) {
            
            strcpy(szName, szFilename);
            szName[strlen(szName)-4] = '\0';

            // Remove the dir
            char *n = strrchr(szName,'\\');
            char *f = strrchr(szFilename,'\\');
            cb->addItem(index++, f+1, n+1);

            // If this is the default skin, store the index for selection later
            if( stricmp(n+1,"default") == 0 )
                def = index-1;
        }

        if( !FindNext(szFilename) )
            break;
    }

    // Select the default
    cb->setCurItem(def);
}