// OpenLieroX Media Player Interface Components
// Made by Dark Charlie and Albert Zeyer

#include "defs.h"
#include "LieroX.h"
#include "CMediaPlayer.h"
#include "Menu.h"
// NOTE: listview ist included in CMediaPlayer.h


//
// Player button
//

void CPlayerButton::Draw(SDL_Surface *bmpDest)
{
	int src_y = 0;
	if (bDown)
		src_y = bmpImage->h/2;
	DrawImageAdv(bmpDest,bmpImage,0,src_y,iX,iY,bmpImage->w,bmpImage->h/2);
}

int CPlayerButton::MouseDown(mouse_t *tMouse, int nDown)
{
	bDown = InBox(tMouse->X,tMouse->Y) != 0;
	return MP_WID_NONE;
}

int CPlayerButton::MouseUp(mouse_t *tMouse, int nDown)
{
	bDown = false;
	return MP_BTN_CLICK;
}

//
// Player slider
//

void CPlayerSlider::Draw(SDL_Surface *bmpDest)
{
	// Background
	DrawImage(bmpDest,bmpBackground,iX,iY);

	// Get the width
	int x = iX+5+bmpStart->w;
	int w = bmpBackground->w - 5;
	int val = (int)( ((float)w/(float)iMax) * (float)iValue ) + x-bmpStart->w;

	// Progress start
	if (((float)iValue/(float)iMax) > 0.02)  {
		DrawImage(bmpDest,bmpStart,iX+5,iY+3);
	}

	// Progress
	int max = MIN(val,bmpBackground->w-10-bmpEnd->w+x);
	for (int i=x;i<max;i+=bmpProgress->w)  {
		DrawImage(bmpDest,bmpProgress,i,iY+3);
	}

	// Progress end
	if (((float)iValue/(float)iMax) >= 0.98)  {
		DrawImage(bmpDest,bmpEnd,iX+max,iY+3);
	}	
}

int CPlayerSlider::MouseDown(mouse_t *tMouse, int nDown)
{
	iCanLoseFocus = false;

	int x = iX+5;
	int w = iWidth - 10;

	int val = (int)( (float)iMax / ( (float)w / (float)(tMouse->X-x)) );
	iValue = val;

	if(tMouse->X > x+w)
		iValue = iMax;
	if(tMouse->X < x)
		iValue = 0;

	// Clamp the value
	iValue = MAX(0,iValue);
	iValue = MIN(iMax,iValue);

	return MP_SLD_CHANGE;
}

//
// Player toggle button
//

void CPlayerToggleBtn::Draw(SDL_Surface *bmpDest)
{
	int src_y = 0;
	if (bEnabled) 
		src_y = bmpImage->h/2;
	DrawImageAdv(bmpDest,bmpImage,0,src_y,iX,iY,bmpImage->w,bmpImage->h/2);
}

int CPlayerToggleBtn::MouseUp(mouse_t *tMouse,int nDown)
{
	bEnabled = !bEnabled;
	return MP_TOG_TOGGLE;
}

//
// Player marquee
//

void CPlayerMarquee::RedrawBuffer(void)
{
	if (bmpBuffer)
		SDL_FreeSurface(bmpBuffer);

	iTextWidth = tLX->cFont.GetWidth(szText);

	bmpBuffer = gfxCreateSurface(iTextWidth,tLX->cFont.GetHeight());
	if (!bmpBuffer)
		return;
	SDL_SetColorKey(bmpBuffer, SDL_SRCCOLORKEY, tLX->clPink);
	DrawRectFill(bmpBuffer,0,0,bmpBuffer->w,bmpBuffer->h,tLX->clPink);

	tLX->cFont.Draw(bmpBuffer,0,0,iColour,"%s",szText);
}

void CPlayerMarquee::Draw(SDL_Surface *bmpDest)
{
	if (iTextWidth <= iWidth)  {
		int x = iX + iWidth/2 - iTextWidth/2;
		DrawImage(bmpDest,bmpBuffer,x,iY);
		return;
	}

	fTime += tLX->fDeltaTime;
	// Move the text
	if (fTime > MARQUEE_TIME)  {
		fTime = 0;
		iFrame += MARQUEE_STEP*iDirection;
		if (iFrame+iWidth >= iTextWidth)  {
			fEndWait += tLX->fDeltaTime;
			if (fEndWait >= MARQUEE_ENDWAIT)  {
				iDirection = -1;
				fEndWait = 0;
				iFrame += MARQUEE_STEP*iDirection;  // Move
			// Don't move at the end
			} else  {
				iFrame -= MARQUEE_STEP*iDirection;
			}
		}
		else if (iFrame <= 0 && iDirection == -1)  {
			fEndWait += tLX->fDeltaTime;
			if (fEndWait >= MARQUEE_ENDWAIT)  {
				iFrame = 0;
				iDirection = 1;
				fEndWait = 0;
				iFrame += MARQUEE_STEP*iDirection;  // Move
			// Don't move at the end
			} else {
				iFrame = 0;
			}
		}
	}
	DrawImageAdv(bmpDest,bmpBuffer,iFrame,0,iX,iY,iWidth,iHeight);
}

//
//	Open/add directory dialog
//

enum  {
	od_List,
	od_Ok,
	od_Cancel,
	od_Add,
	od_IncludeSubdirs
};

////////////////////////
// Runs the dialog, returns the directory user selected
char *COpenAddDir::Execute(char *default_dir)
{
	if (!default_dir)
		return NULL;
	fix_strncpy(szDir,default_dir);

	bool done = false;
	bool cancelled = false;

	SDL_Surface *Screen = SDL_GetVideoSurface();
	keyboard_t *Keyboard = GetKeyboard();

	// Initialize the GUI
	cOpenGui.Initialize();
	cOpenGui.Add(new CListview(),od_List,iX+5,iY+5,iWidth-10,iHeight-60);
	cOpenGui.Add(new CCheckbox(bAdd),od_Add,iX+5,iY+iHeight-59,17,17);
	cOpenGui.Add(new CLabel("Add to current playlist",tLX->clNormalLabel),-1,iX+25,iY+iHeight-57,0,0);
	cOpenGui.Add(new CCheckbox(bIncludeSubdirs),od_IncludeSubdirs,iX+5,iY+iHeight-42,17,17);
	cOpenGui.Add(new CLabel("Include subdirectories",tLX->clNormalLabel),-1,iX+25,iY+iHeight-40,0,0);
	cOpenGui.Add(new CButton(BUT_OK,tMenu->bmpButtons),od_Ok,iX+3*iWidth/4,iY+iHeight-20,30,15);
	cOpenGui.Add(new CButton(BUT_CANCEL,tMenu->bmpButtons),od_Cancel,iX+iWidth/4,iY+iHeight-20,60,15);

	((CButton *)(cOpenGui.getWidget(od_Ok)))->setRedrawMenu(false);
	((CButton *)(cOpenGui.getWidget(od_Cancel)))->setRedrawMenu(false);

	gui_event_t *ev = NULL;
	CListview *lv = (CListview *)cOpenGui.getWidget(od_List);
	if (!lv)
		return NULL;

	CCheckbox *c = NULL;

	lv->setRedrawMenu(false);

	// Fill the directory list with the default directory
	ReFillList(lv,default_dir);

	// Save the area we are going to draw on in a buffer
	SDL_Surface *bmpBuffer = gfxCreateSurface(Screen->w,Screen->h);
	if (!bmpBuffer)
		return NULL;

	DrawImage(bmpBuffer,Screen,0,0);
	float oldtime = GetMilliSeconds()-tLX->fDeltaTime;

	while (!done)  {
		oldtime = tLX->fCurTime;
		tLX->fCurTime = GetMilliSeconds();
		tLX->fDeltaTime = tLX->fCurTime-oldtime;

		ProcessEvents();

		// Restore the original screen before drawing
		DrawImage(Screen,bmpBuffer,0,0);

		// Draw the borders
		DrawRectFill(Screen,iX,iY,iX+iWidth,iY+iHeight,0);
		Menu_DrawBox(Screen,iX,iY,iX+iWidth,iY+iHeight);

		cOpenGui.Draw(Screen);
		ev = cOpenGui.Process();
		if (ev)  {
			switch(ev->iControlID)  {
			// Ok
			case od_Ok:
				if(ev->iEventMsg == BTN_MOUSEUP) {

					// Click!
					PlaySoundSample(sfxGeneral.smpClick);

					if (!lv->getCurSIndex())
						break;


					// Check that this is not the parent or current directory
					char *dir_name = MAX(strrchr(lv->getCurSIndex(),'\\'),strrchr(lv->getCurSIndex(),'/'));
					if (!dir_name)
						break;

					if (!strcmp(dir_name,"..") || !strcmp(dir_name,"."))
						break;

					// Copy the directory
					fix_strncpy(szDir,lv->getCurSIndex());

					// We're done
					done = true;
					cancelled = false;
				}
				break;

			// Cancel
			case od_Cancel:
				if(ev->iEventMsg == BTN_MOUSEUP) {
					// Click!
					PlaySoundSample(sfxGeneral.smpClick);

					done = true;
					cancelled = true;
				}
				break;


			// Player list
			case od_List:
				if(ev->iEventMsg == LV_DOUBLECLK || ev->iEventMsg == LV_ENTER) {
					// Re-fill the list with the double-clicked directory
					ReFillList(lv,lv->getCurSIndex());			
				}
				break;

			// Include subdirectories
			case od_IncludeSubdirs:
				if(ev->iEventMsg == CHK_CHANGED) {
					c = (CCheckbox *)cOpenGui.getWidget(od_IncludeSubdirs);
					bIncludeSubdirs = c->getValue() != 0;
				}
				break;

			// Add to playlist
			case od_Add:
				if(ev->iEventMsg == CHK_CHANGED) {
					c = (CCheckbox *)cOpenGui.getWidget(od_Add);
					bAdd = c->getValue() != 0;
				}
				break;
			}
		}

		// Handle the keyboard
		if (Keyboard->KeyDown[SDLK_ESCAPE])  {
			done = true;
			cancelled = true;
		}

		// Should we quit?
		if (tLX->iQuitGame)  {
			done = true;
			cancelled = true;
		}

		// Draw the mouse
		DrawImage(Screen,gfxGUI.bmpMouse[0],GetMouse()->X,GetMouse()->Y);

		FlipScreen(Screen);
	}

	// Restore and free the buffer
	DrawImage(Screen,bmpBuffer,0,0);
	SDL_FreeSurface(bmpBuffer);

	// Free the GUI
	cOpenGui.Shutdown();

	if (cancelled)
		return NULL;
	else  {
		// Adjust
		replace(szDir,"//","/",szDir);
		replace(szDir,"\\/","/",szDir);
		return szDir;
	}
}

////////////////////
// Checks if the given directory is the root directory
bool COpenAddDir::IsRoot(const char *dir)
{
	static char tmp[1024];
	fix_strncpy(tmp,dir);

	// Adjust
	replace(tmp,"//","/",tmp);
	replace(tmp,"\\/","/",tmp);

	int len = strnlen(tmp,sizeof(tmp));

	// Remove the ending slash
	if (tmp[len-1] == '\\' || tmp[len-1] == '/')
		tmp[len-1] = '\0';

	// If we can't find another slash, this must be the parent directory
	char *slash = MAX(strrchr(tmp,'\\'),strrchr(tmp,'/'));
	if (!slash)
		return true;

	// If there's a slash and this is the link to the parent directory, check, if there's another slash
	if (!strcmp(slash,".."))  {
		*slash = '\0';
		slash = MAX(strrchr(tmp,'\\'),strrchr(tmp,'/'));
		// Not another slash, this is a root directory
		if (!slash)
			return true;
	}

	// Not a root directory
	return false;

}

///////////////////////
// Fills the list with the subdirectories of the "dir"
void COpenAddDir::ReFillList(CListview *lv, char *dir)
{
	if (!dir)
		return;

	static char directory[1024]="";
	static char tmp_dir[1054];
	char *dir_name = NULL;
	int index=0;
	int len = 0;

	fix_strncpy(tmp_dir,dir);
	len = strlen(dir);

	// Add the slash if needed
	if (tmp_dir[len-1] != '\\' && tmp_dir[len-1] != '/')  {
		strcat(tmp_dir,"/");
		len++;
	}

	bool root = IsRoot(tmp_dir);
	if (!root)  {
		// Handle the parent directory
		//dir_name[len-1] = '\0';
		if (dir_name = strstr(tmp_dir,"../"))  {
			*(dir_name-1) = '\0';
			dir_name = MAX(strrchr(tmp_dir,'\\'),strrchr(tmp_dir,'/'));
			if (dir_name)  {
				*(dir_name+1) = '\0';
			}
			// Check again
			root = IsRoot(tmp_dir);
		}
	}



	// Fill in the first directory 
	snprintf(directory,sizeof(directory),"%s%s",tmp_dir,"..");

	// Clear the listview
	lv->Clear();

	// Add the parent directory
	if (!root) { // Check if not root
		lv->AddItem(directory,index++,tLX->clListView);
		lv->AddSubitem(LVS_TEXT,"..",NULL);
	}

	if(FindFirstDir(tmp_dir,directory,true)) {
		fix_markend(directory);
		while(1) {
			// Extract the directory name from the path
			dir_name = MAX(strrchr(directory,'\\'),strrchr(directory,'/'));

			// Add the directory
			if (dir_name)  {
				lv->AddItem(directory,index++,tLX->clListView);
				lv->AddSubitem(LVS_TEXT,dir_name+1,NULL);
			}

			if(!FindNextDir(directory))
				break;
			fix_markend(directory);
		}
		lv->setSelectedID(0);
	}
}