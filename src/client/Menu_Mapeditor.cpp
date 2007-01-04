/////////////////////////////////////////
//
//             Liero Xtreme
//
//     Copyright Auxiliary Software 2002
//
//
/////////////////////////////////////////


// Map editor
// Created 28/7/02
// Jason Boettcher


#include "defs.h"
#include "LieroX.h"
#include "Menu.h"


CGuiLayout	cMaped;

// Map editor controls
enum {
	map_new,
	map_random,
	map_load,
	map_save,
	map_quit
};

CMap		cMap;
CViewport	cMapedView;
int			grabbed = false;
int			grabX,  grabY;
int			grabWX, grabWY;

///////////////////
// Initialize the map editor
int Menu_MapEdInitialize(void)
{
	tMenu->iMenuType = MNU_MAPED;

	// Create the buffer
	/*DrawImage(tMenu->bmpBuffer,tMenu->bmpMainBack,0,0);
	Menu_DrawSubTitle(tMenu->bmpBuffer,SUB_MAPED);

	Menu_DrawBox(tMenu->bmpBuffer,20,171, 619,458);
	Menu_DrawBox(tMenu->bmpBuffer,20,135, 54,169);*/

    DrawImage(tMenu->bmpBuffer,tMenu->bmpMainBack_wob,0,0);
    Menu_DrawSubTitleAdv(tMenu->bmpBuffer,SUB_MAPED,18);
    Menu_DrawBox(tMenu->bmpBuffer, 15,100, 625, 465);
    
    Menu_DrawBox(tMenu->bmpBuffer,20,105, 54,139);      // Preview box
    Menu_DrawBox(tMenu->bmpBuffer,20,146, 619,459);     // Level box

    

	Menu_RedrawMouse(true);

	// Create a map
	cMap.New(504,350,"dirt");
	cMapedView.Setup(22,148, 596,310,0);
	cMapedView.SetWorldX(0);
	cMapedView.SetWorldY(0);

	tMenu->iEditMode = 0;
	tMenu->iCurHole = 0;
	tMenu->iCurStone = 0;
	tMenu->iCurMisc = 0;
	tMenu->iCurDirt = 0;

	cMaped.Shutdown();
	cMaped.Initialize();

	grabbed = false;

	// Add the controls
	cMaped.Add( new CButton(BUT_NEW, tMenu->bmpButtons),	map_new,	 230,110, 50,15);
	cMaped.Add( new CButton(BUT_RANDOM, tMenu->bmpButtons), map_random,  300,110, 80,15);
	cMaped.Add( new CButton(BUT_LOAD, tMenu->bmpButtons),   map_load,	 400,110, 50,15);
	cMaped.Add( new CButton(BUT_SAVE, tMenu->bmpButtons),   map_save,	 480,110, 50,15);
	cMaped.Add( new CButton(BUT_QUIT, tMenu->bmpButtons),   map_quit,	 550,110, 50,15);




	return true;
}

/////////////
// Shutdown
void Menu_MapEdShutdown(void)
{
	cMaped.Shutdown();
	cMap.Shutdown();
}


///////////////////
// Map editor frame
void Menu_MapEdFrame(SDL_Surface *bmpDest, int process)
{
	gui_event_t *ev;
	mouse_t *Mouse = GetMouse();
	int mouse = 0;
	int x,y,n,i;

	// Re-draw the buffer over buttons
	//DrawImageAdv(bmpDest, tMenu->bmpBuffer, 230,140, 230,140, 410,50);
	
	ev = cMaped.Process();
	cMaped.Draw(bmpDest);

	
	// Draw the map
	cMap.Draw(bmpDest, &cMapedView);


	// Toolbar
	int down = -1;
	if(Mouse->Down || Mouse->Up) {
		if(Mouse->Y > 108 && Mouse->Y < 137) {

			for(x=58,n=0;x<155;x+=32,n++) {
				if(Mouse->X > x && Mouse->X < x+29)
					down = n;
			}
		}
		if(Mouse->Up && down >= 0) {
			PlaySoundSample(sfxGeneral.smpClick);
			
			tMenu->iEditMode = down;
		}
	}

	for(x=0,n=58,i=0;x<116;x+=29,n+=32,i++) {
		y = tMenu->iEditMode == i || down == i ? 29 : 0;
		DrawImageAdv(bmpDest, tMenu->bmpMapEdTool, x,y, n, 108, 29,29);
	}
	
	// Item clicked on?
	int Clicked = false;
	if(Mouse->Up) {
		if(Mouse->X > 20 && Mouse->X < 54)
			if(Mouse->Y > 105 && Mouse->Y < 139)
				Clicked = Mouse->Up;
	}


	DrawImageAdv(bmpDest, tMenu->bmpBuffer, 20,105, 20,105, 34,34);

	//
	// Draw item
	//
	theme_t *t = cMap.GetTheme();
	
	// Holes
	if(tMenu->iEditMode == 0) {		

		// left mouse button
		if(Clicked & SDL_BUTTON(1)) {
			tMenu->iCurHole++;
			if(tMenu->iCurHole >= 5)
				tMenu->iCurHole = 0;
		}

		// Right mouse button
		if(Clicked & SDL_BUTTON(3)) {
			tMenu->iCurHole--;
			if(tMenu->iCurHole < 0)
				tMenu->iCurHole = 4;
		}


		int w = t->bmpHoles[ tMenu->iCurHole ]->w;
		int h = t->bmpHoles[ tMenu->iCurHole ]->h;

		DrawImageStretch(bmpDest, t->bmpHoles[ tMenu->iCurHole ], 37-w, 122-h);
	}

	// Stones
	if(tMenu->iEditMode == 1) {		

		// lmb
		if(Clicked & SDL_BUTTON(1)) {
			tMenu->iCurStone++;
			if(tMenu->iCurStone >= t->NumStones)
				tMenu->iCurStone = 0;
		}

		// rmb
		if(Clicked & SDL_BUTTON(3)) {
			tMenu->iCurStone--;
			if(tMenu->iCurStone < 0)
				tMenu->iCurStone = t->NumStones-1;
		}

		
		int w = t->bmpStones[ tMenu->iCurStone ]->w;
		int h = t->bmpStones[ tMenu->iCurStone ]->h;

		if(w > 17 || h > 17) {
			w >>= 1;
			h >>= 1;
			DrawImage(bmpDest,t->bmpStones[ tMenu->iCurStone ], 37-w, 122-h);
			DrawRect(bmpDest,22,107, 52,137, 0xffff);
		} else
			DrawImageStretchKey(bmpDest, t->bmpStones[ tMenu->iCurStone ], 37-w, 122-h, (Uint16)MakeColour(255,0,255));
	}

	
	// Miscellanous
	if(tMenu->iEditMode == 2) {		

		// lmb
		if(Clicked & SDL_BUTTON(1)) {
			tMenu->iCurMisc++;
			if(tMenu->iCurMisc >= t->NumMisc)
				tMenu->iCurMisc = 0;
		}

		// rmb
		if(Clicked & SDL_BUTTON(3)) {
			tMenu->iCurMisc--;
			if(tMenu->iCurMisc < 0)
				tMenu->iCurMisc = t->NumMisc-1;
		}

		
		int w = t->bmpMisc[ tMenu->iCurMisc ]->w;
		int h = t->bmpMisc[ tMenu->iCurMisc ]->h;

		DrawImageStretchKey(bmpDest, t->bmpMisc[ tMenu->iCurMisc ], 37-w, 122-h, (Uint16)MakeColour(255,0,255));
	}

	// Dirt
	if(tMenu->iEditMode == 3) {
		
		// lmb
		if(Clicked & SDL_BUTTON(1)) {
			tMenu->iCurDirt++;
			if(tMenu->iCurDirt >= 5)
				tMenu->iCurDirt = 0;
		}

		// rmb
		if(Clicked & SDL_BUTTON(3)) {
			tMenu->iCurDirt--;
			if(tMenu->iCurDirt < 0)
				tMenu->iCurDirt = 4;
		}

		int w = t->bmpHoles[ tMenu->iCurDirt ]->w;
		int h = t->bmpHoles[ tMenu->iCurDirt ]->h;

		DrawImageStretch(bmpDest, t->bmpHoles[ tMenu->iCurDirt ], 37-w, 122-h);
	}

	if(!process)
		return;


	//
	// Place the objects on the map
	//
	SDL_Surface *MouseImg = NULL;
	if(Mouse->X >= 22 && Mouse->X <= 618) {
		if(Mouse->Y >= 148 && Mouse->Y <= 457) {

			x = (Mouse->X - 22)/2 + cMapedView.GetWorldX();
			y = (Mouse->Y - 148)/2 + cMapedView.GetWorldY();
			CVec pos = CVec((float)x, (float)y);


			switch(tMenu->iEditMode) {

				// Hole
				case 0:
					MouseImg = t->bmpHoles[tMenu->iCurHole];
					if(Mouse->Down & SDL_BUTTON(1))
						cMap.CarveHole(tMenu->iCurHole,pos);
					break;

				// Stone
				case 1:
					MouseImg = t->bmpStones[tMenu->iCurStone];
					if(Mouse->Up & SDL_BUTTON(1))
						cMap.PlaceStone(tMenu->iCurStone,pos);
					break;

				// Misc
				case 2:
					MouseImg = t->bmpMisc[tMenu->iCurMisc];
					if(Mouse->Up & SDL_BUTTON(1))
						cMap.PlaceMisc(tMenu->iCurMisc,pos);
					break;

				// Dirt
				case 3:
					MouseImg = t->bmpHoles[tMenu->iCurDirt];
					if(Mouse->Down & SDL_BUTTON(1))
						cMap.PlaceDirt(tMenu->iCurDirt,pos);
					break;

				// Delete object
				/*case 4:
					if(Mouse->Up & SDL_BUTTON(1))
						cMap.DeleteObject(pos);
					break;*/
			}
		}
	}



	//
	// Process the gui
	//

	if(ev) {

		if(ev->cWidget->getType() == wid_Button) {
			mouse = 1;
			if(ev->iEventMsg == BTN_MOUSEUP)
				PlaySoundSample(sfxGeneral.smpClick);
		}
		if(ev->cWidget->getType() == wid_Textbox)
			mouse = 2;

		switch(ev->iControlID) {

			// New
			case map_new:
				if(ev->iEventMsg == BTN_MOUSEUP) {
					Menu_MapEd_New();
				}
				break;

			// Random
			case map_random:
				if(ev->iEventMsg == BTN_MOUSEUP)
					cMap.ApplyRandom();
				break;

			// Load
			case map_load:
				if(ev->iEventMsg == BTN_MOUSEUP) {
					Menu_MapEd_LoadSave(false);
				}
				break;

			// Save
			case map_save:
				if(ev->iEventMsg == BTN_MOUSEUP) {
					Menu_MapEd_LoadSave(true);
				}
				break;

			// Quit
			case map_quit:
				if(ev->iEventMsg == BTN_MOUSEUP) {

					// Shutdown the classes
					Menu_MapEdShutdown();

					Menu_MainInitialize();
				}
				break;
		}
	}


	// Keyboard arrows
	keyboard_t *kb = GetKeyboard();
	int Scroll = 250 * (int)tLX->fDeltaTime;
	if(kb->keys[SDLK_UP])
		cMapedView.SetWorldY( cMapedView.GetWorldY() - Scroll );
	if(kb->keys[SDLK_DOWN])
		cMapedView.SetWorldY( cMapedView.GetWorldY() + Scroll );
	if(kb->keys[SDLK_LEFT])
		cMapedView.SetWorldX( cMapedView.GetWorldX() - Scroll );
	if(kb->keys[SDLK_RIGHT])
		cMapedView.SetWorldX( cMapedView.GetWorldX() + Scroll );

    // Grab the level & drag it
	if(Mouse->Down & SDL_BUTTON(2)) {
		int inmap = false;
		
		if(Mouse->X >= 22 && Mouse->X <= 618) {
			if(Mouse->Y >= 173 && Mouse->Y <= 457) {
				inmap = true;

				// Mouse button grabs and moves the map
				x = (Mouse->X - 22)/2;
				y = (Mouse->Y - 173)/2;

				// First grab
				if(!grabbed) {
					grabbed = true;
					grabX = x;
					grabY = y;
					grabWX = cMapedView.GetWorldX();
					grabWY = cMapedView.GetWorldY();
				} else {

					cMapedView.SetWorldX( grabWX + (grabX-x));
					cMapedView.SetWorldY( grabWY + (grabY-y));
				}
			}
		}
	} else
		grabbed = false;

    // When the mouse is on the edges of the screen, move the level
    int EdgeSize = 7;
    if( !Mouse->Down && !Mouse->Up ) {

        // X Axis
        if( Mouse->X < EdgeSize )
            cMapedView.SetWorldX( cMapedView.GetWorldX() - Scroll );
        if( Mouse->X > 640-EdgeSize )
            cMapedView.SetWorldX( cMapedView.GetWorldX() + Scroll );

        // Y Axis
        if( Mouse->Y < EdgeSize )
            cMapedView.SetWorldY( cMapedView.GetWorldY() - Scroll );
        if( Mouse->Y > 480-EdgeSize )
            cMapedView.SetWorldY( cMapedView.GetWorldY() + Scroll );
    }

	// Clamp the viewport
	cMapedView.Clamp(cMap.GetWidth(), cMap.GetHeight());




	// Draw the mouse
	if(MouseImg) {
		int w = MouseImg->w;
		int h = MouseImg->h;
		if(tMenu->iEditMode == 0 || tMenu->iEditMode == 3)
			DrawImageStretchKey(tMenu->bmpScreen,MouseImg, Mouse->X-w, Mouse->Y-h,0);
		else
			DrawImageStretchKey(tMenu->bmpScreen,MouseImg, Mouse->X-w, Mouse->Y-h, (Uint16)MakeColour(255,0,255));
	}
	else
		DrawImage(tMenu->bmpScreen,gfxGUI.bmpMouse[mouse], Mouse->X,Mouse->Y);
}


///////////////////
// Show a 'new' dialog box
enum {
	mn_Cancel=0,
	mn_Ok,
	mn_Width,
	mn_Height,
	mn_Scheme
};

void Menu_MapEd_New(void)
{
	keyboard_t *kb = GetKeyboard();
	mouse_t *Mouse = GetMouse();
	gui_event_t *ev;
	int mouse=0;
	int i;
	int quitloop = false;
	CTextbox *t1,*t2;

	// Save the background
	Menu_MapEdFrame(tMenu->bmpBuffer,false);

	Menu_DrawBox(tMenu->bmpBuffer, 210, 170, 430, 310);
	DrawImageAdv(tMenu->bmpBuffer, tMenu->bmpMainBack_wob, 212,172, 212,172, 217,137);
    Menu_DrawBox(tMenu->bmpBuffer, 15,100, 625, 465);
	//DrawRectFill(tMenu->bmpBuffer, 212, 172, 429, 309, 0);

	Menu_RedrawMouse(true);

	CGuiLayout cg;

	cg.Initialize();

	cg.Add( new CButton(BUT_CANCEL, tMenu->bmpButtons), mn_Cancel, 220,285, 75,15);
	cg.Add( new CButton(BUT_OK, tMenu->bmpButtons),     mn_Ok, 390,285, 40,15);
	cg.Add( new CTextbox(),                             mn_Width, 280,200, 100,20);
	cg.Add( new CTextbox(),                             mn_Height, 280,230, 100,20);
	cg.Add( new CCombobox(),							mn_Scheme, 280,260, 120,17);

	cg.SendMessage(2,TXM_SETMAX,64,0);
	cg.SendMessage(3,TXM_SETMAX,64,0);

	int dirtindex = -1;

	// Find directories in the theme dir
	static char dir[512];
	char *d;
	if(FindFirstDir("data/themes",dir)) {
		fix_markend(dir);
		i=0;
		while(1) {
			d = MAX(strrchr(dir,'\\'),strrchr(dir,'/'))+1;
			cg.SendMessage(4,CBM_ADDITEM,i,(DWORD)d);

			if(stricmp(d,"dirt") == 0)
				dirtindex = i;
			i++;

			if(!FindNextDir(dir))
				break;
			fix_markend(dir);	
		}
	}

	if(dirtindex != -1)
		cg.SendMessage(4,CBM_SETCURSEL,dirtindex,0);



	// Set initial values
	t1 = (CTextbox *)cg.getWidget(2);
	t2 = (CTextbox *)cg.getWidget(3);

	static char buf[16];
	snprintf(buf,sizeof(buf),"%d",cMap.GetWidth()); fix_markend(buf);
	t1->setText(buf);
	snprintf(buf,sizeof(buf),"%d",cMap.GetHeight()); fix_markend(buf);
	t2->setText(buf);
	
	
	ProcessEvents();
	while(!kb->KeyUp[SDLK_ESCAPE] && !quitloop) {
		Menu_RedrawMouse(false);
		ProcessEvents();

		DrawImageAdv(tMenu->bmpScreen,tMenu->bmpBuffer, 210,170, 210,170, 220, 260);

		tLX->cFont.DrawCentre(tMenu->bmpScreen, 320, 175, 0xffff,"%s", "Level details");
		tLX->cFont.Draw(tMenu->bmpScreen, 220, 202, 0xffff,"%s", "Width");
		tLX->cFont.Draw(tMenu->bmpScreen, 220, 232, 0xffff,"%s", "Height");
		tLX->cFont.Draw(tMenu->bmpScreen, 220, 262, 0xffff,"%s", "Theme");

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
				case mn_Cancel:
					if(ev->iEventMsg == BTN_MOUSEUP) {
						PlaySoundSample(sfxGeneral.smpClick);
						quitloop = true;
					}
					break;

				// OK
				case mn_Ok:
					if(ev->iEventMsg == BTN_MOUSEUP) {
						PlaySoundSample(sfxGeneral.smpClick);
						int w = atoi(t1->getText());
						int h = atoi(t2->getText());
						static char theme[128];
						strcpy(theme,"dirt");
						cb_item_t *it = (cb_item_t *)cg.SendMessage(4,CBM_GETCURITEM,0,0);
						if(it)
							fix_strncpy(theme,it->sName);


						// Check for min & max sizes
						if(w >= 350 && h >= 250 &&
							w <= 4000 && h <= 4000) {
								quitloop = true;
								cMap.New(w,h,theme);
						}
					}
					break;
			}

		}


		DrawImage(tMenu->bmpScreen,gfxGUI.bmpMouse[mouse], Mouse->X,Mouse->Y);

		FlipScreen(tMenu->bmpScreen);
	}

	// Redraw back to normal
	DrawImage(tMenu->bmpBuffer,tMenu->bmpMainBack_wob,0,0);
    Menu_DrawSubTitleAdv(tMenu->bmpBuffer,SUB_MAPED,18);
    Menu_DrawBox(tMenu->bmpBuffer, 15,100, 625, 465);
    
    Menu_DrawBox(tMenu->bmpBuffer,20,105, 54,139);      // Preview box
    Menu_DrawBox(tMenu->bmpBuffer,20,146, 619,459);     // Level box

	Menu_RedrawMouse(true);

	cg.Shutdown();
}


///////////////////
// File save/load dialog
enum  {
	sl_Cancel=0,
	sl_Ok,
	sl_FileList,
	sl_FileName
};

void Menu_MapEd_LoadSave(int save)
{
	keyboard_t *kb = GetKeyboard();
	mouse_t *Mouse = GetMouse();
	gui_event_t *ev;
	int mouse=0;
	int quitloop = false;
	CTextbox *t;

	// Save the background
	Menu_MapEdFrame(tMenu->bmpBuffer,false);

	Menu_DrawBox(tMenu->bmpBuffer, 170, 150, 470, 330);
	DrawImageAdv(tMenu->bmpBuffer, tMenu->bmpMainBack_wob, 172,152, 172,152, 297,177);

	Menu_RedrawMouse(true);

	CGuiLayout cg;

	cg.Initialize();

	cg.Add( new CButton(BUT_CANCEL, tMenu->bmpButtons), 0, 180,310, 75,15);
	cg.Add( new CButton(BUT_OK, tMenu->bmpButtons),     1, 430,310, 40,15);
	cg.Add( new CListview(),                            2, 180,170, 280,110);
	cg.Add( new CTextbox(),                             3, 260,285, 200,20);

	cg.SendMessage(2,		LVM_SETOLDSTYLE, 0, 0);
	
	t = (CTextbox *)cg.getWidget(3);

	// Load the level list
	static char	filename[512];
	static char	id[32], name[64];
	int		version;

	int done = false;
	if(!FindFirst("levels","*",filename))
		done = true;
	fix_markend(filename);
	CListview *lv = (CListview *)cg.getWidget(2);
	lv->AddColumn("Levels",60);


	while(!done) {

		// Liero Xtreme level
		if( stricmp(filename + fix_strnlen(filename)-4, ".lxl") == 0) {

			FILE *fp = OpenGameFile(filename,"rb");
			if(fp) {
				fread(id,		sizeof(char),	32,	fp);
				fread(&version,	sizeof(int),	1,	fp);
				EndianSwap(version);
				fread(name,		sizeof(char),	64,	fp);

				if(strcmp(id,"LieroX Level") == 0 && version == MAP_VERSION) {
					 // Remove the 'levels' bit from the filename
					char *f = MAX(strrchr(filename,'\\'),strrchr(filename,'/'));
					if(f) {
						lv->AddItem(f+1,0,tLX->clListView);
						lv->AddSubitem(LVS_TEXT,name,NULL);
					}
				}

				fclose(fp);
			}
		}


		// Liero level
		if( stricmp(filename + fix_strnlen(filename)-4, ".lev") == 0) {
			FILE *fp = OpenGameFile(filename,"rb");
			
			if(fp) {

				// Make sure it's the right size to be a liero level
				fseek(fp,0,SEEK_END);
				// 176400 is liero maps
				// 176402 is worm hole maps (same, but 2 bytes bigger)
				// 177178 is a powerlevel
				if( ftell(fp) == 176400 || ftell(fp) == 176402 || ftell(fp) == 177178) {

					char *f = MAX(strrchr(filename,'\\'),strrchr(filename,'/'));
					if(f) {
						lv->AddItem(f+1,0,tLX->clListView);
						lv->AddSubitem(LVS_TEXT,f+1,NULL);
					}
				}

				fclose(fp);
			}
		}

		if(!FindNext(filename))
			break;
		fix_markend(filename);
	}


	
	
	ProcessEvents();
	while(!kb->KeyUp[SDLK_ESCAPE] && !quitloop && tMenu->iMenuRunning) {
		Menu_RedrawMouse(false);
		ProcessEvents();

		DrawImageAdv(tMenu->bmpScreen,tMenu->bmpBuffer, 170,150, 170,150, 300, 180);

		tLX->cFont.DrawCentre(tMenu->bmpScreen, 320, 155, 0xffff,"%s", save ? "Save" : "Load");
		tLX->cFont.Draw(tMenu->bmpScreen, 180,288,0xffff,"%s","Level name");

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
				case sl_Cancel:
					if(ev->iEventMsg == BTN_MOUSEUP) {
						PlaySoundSample(sfxGeneral.smpClick);
						quitloop = true;
					}
					break;

				// OK
				case sl_Ok:
					if(ev->iEventMsg == BTN_MOUSEUP) {
						PlaySoundSample(sfxGeneral.smpClick);
						
						if(strlen(t->getText()) > 0) {

							quitloop = true;
							static char buf[256]; 
							if(save) {

								// Save								
								snprintf(buf,sizeof(buf),"levels/%s",t->getText());
								fix_markend(buf);
								
								// Check if it exists already. If so, ask user if they wanna overwrite
								if(Menu_MapEd_OkSave(buf))
									cMap.Save(t->getText(),buf);
								else
									quitloop = false;
							} else {
								
								// Load
								snprintf(buf,sizeof(buf),"levels/%s",t->getText());
								fix_markend(buf);
								cMap.Load(buf);
							}
						}					
					}
					break;

				// Level list
				case sl_FileList:
					if(ev->iEventMsg != LV_NONE) {
						t->setText( lv->getCurSIndex() );
					}
					break;
			}
		}


		DrawImage(tMenu->bmpScreen,gfxGUI.bmpMouse[mouse], Mouse->X,Mouse->Y);

		FlipScreen(tMenu->bmpScreen);
	}

	// Redraw back to normal
	DrawImage(tMenu->bmpBuffer,tMenu->bmpMainBack_wob,0,0);
    Menu_DrawSubTitleAdv(tMenu->bmpBuffer,SUB_MAPED,18);
    Menu_DrawBox(tMenu->bmpBuffer, 15,100, 625, 465);
    
    Menu_DrawBox(tMenu->bmpBuffer,20,105, 54,139);      // Preview box
    Menu_DrawBox(tMenu->bmpBuffer,20,146, 619,459);     // Level box

	Menu_RedrawMouse(true);

	cg.Shutdown();
}


///////////////////
// Check if there is a possible overwrite
int Menu_MapEd_OkSave(char *szFilename)
{
	// Adjust the filename
	if( stricmp( szFilename + strlen(szFilename) - 4, ".lxl") != 0)
		strcat(szFilename,".lxl");

	FILE *fp = OpenGameFile(szFilename,"rb");
	if( fp == NULL)
		// File doesn't exist, ok to save
		return true;

	fclose(fp);


	// The file already exists, show a message box to confirm the overwrite
	int nResult = Menu_MessageBox("Confirmation","The level already exists. Overwrite?", LMB_YESNO);
	if( nResult == MBR_YES )
		return true;

	
	// No overwrite
	// Fix the screen up
	Menu_MapEdFrame(tMenu->bmpBuffer,false);
	Menu_DrawBox(tMenu->bmpBuffer, 170, 150, 470, 330);
	DrawImageAdv(tMenu->bmpBuffer, tMenu->bmpMainBack_wob, 172,152, 172,152, 297,177);
	Menu_RedrawMouse(true);

	return false;
}
