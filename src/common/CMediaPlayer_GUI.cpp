// OpenLieroX Media Player Interface Components
// Made by Dark Charlie and Albert Zeyer
// code under LGPL

#ifdef WITH_MEDIAPLAYER

#include "LieroX.h"
#include "AuxLib.h"
#include "Graphics.h"
#include "Menu.h"
#include "GfxPrimitives.h"
#include "FindFile.h"
#include "StringUtils.h"


//
// Player button
//

////////////////////
// Constructor
CPlayerButton::CPlayerButton(SDL_Surface *image) {
	if (!image)
		return;

	bmpImage = image;
	bDown = false;
}

/////////////////////
// Draws the button
void CPlayerButton::Draw(SDL_Surface *bmpDest)
{
	int src_y = 0;
	if (bDown)
		src_y = bmpImage->h/2;
	DrawImageAdv(bmpDest,bmpImage,0,src_y,iX,iY,bmpImage->w,bmpImage->h/2);
}

//////////////////////
// Mouse down on Player buton
int CPlayerButton::MouseDown(mouse_t *tMouse, int nDown)
{
	bDown = InBox(tMouse->X,tMouse->Y) != 0;
	return MP_WID_MOUSEDOWN;
}

///////////////////////
// Click on Player button
int CPlayerButton::MouseUp(mouse_t *tMouse, int nDown)
{
	bDown = false;
	return MP_BTN_CLICK;
}

//
// Player slider
//

///////////////////
// Constructor
CPlayerSlider::CPlayerSlider(SDL_Surface *progress, SDL_Surface *start, SDL_Surface *end, SDL_Surface *background, int max)  {
	if (!progress || !start || !end || !background)
		return;
	iValue = 0;
	iMax = max;

	bmpProgress = progress;
	bmpStart = start;
	bmpEnd = end;
	bmpBackground = background;
}

////////////////////
// Draw the slider
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
		DrawImage(bmpDest,bmpEnd,iX+iWidth-5,iY+3);
	}
}

///////////////////////
// Mouse down event on the slider
int CPlayerSlider::MouseDown(mouse_t *tMouse, int nDown)
{
	iCanLoseFocus = false;

	int x = iX+5;
	int w = iWidth - 10;

	// Find the value
	int val = (int)( (float)iMax / ( (float)w / (float)(tMouse->X-x)) );
	iValue = val;

	// Clamp it
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

/////////////////////////
// Draws the toggle button
void CPlayerToggleBtn::Draw(SDL_Surface *bmpDest)
{
	int src_y = 0;
	if (bEnabled)
		src_y = bmpImage->h/2;
	DrawImageAdv(bmpDest,bmpImage,0,src_y,iX,iY,bmpImage->w,bmpImage->h/2);
}

////////////////////////
// Click on the toggle button
int CPlayerToggleBtn::MouseUp(mouse_t *tMouse,int nDown)
{
	bEnabled = !bEnabled;
	return MP_TOG_TOGGLE;
}

//
// Player marquee
//


/////////////////////////
// Constructor
CPlayerMarquee::CPlayerMarquee(const std::string& text, Uint32 col)  {
	szText = text;
	fTime = 0;
	fEndWait = 0;
	iFrame = 0;
	iColour = col;
	iDirection = 1;
	iTextWidth = tLX->cFont.GetWidth(szText);
}

/////////////////////
// Draws the marquee
void CPlayerMarquee::Draw(SDL_Surface *bmpDest)
{
	if (iTextWidth <= iWidth)  {
		int x = iX + iWidth/2 - iTextWidth/2;
		tLX->cFont.Draw(bmpDest,x,iY,iColour,szText);
		return;
	}

	fTime += tLX->fDeltaTime;
	// Move the text
	if (fTime > MARQUEE_TIME)  {
		fTime = 0;
		iFrame += MARQUEE_STEP*iDirection;
		if (iFrame+iWidth >= iTextWidth+4)  { // +4 - let some space behind, looks better
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

	// Draw the font passed into the widget rect
	tLX->cFont.DrawInRect(bmpDest,iX-iFrame,iY,iX,iY,iWidth,iHeight,iColour,szText);
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
std::string COpenAddDir::Execute(const std::string& default_dir)
{
	szDir = default_dir;

	bool done = false;
	bool cancelled = false;

	SDL_Surface *Screen = SDL_GetVideoSurface();
	keyboard_t *Keyboard = GetKeyboard();

	// Initialize the GUI
	cOpenGui.Initialize();
	cOpenGui.Add(new CListview(),od_List,iX+5,iY+45,iWidth-10,iHeight-115);
	cOpenGui.Add(new CLabel("Add to playlist",tLX->clNormalLabel),-1,iX+iWidth/2-tLX->cFont.GetWidth("Add to playlist")/2,iY+5,0,0);
	cOpenGui.Add(new CLabel("Select the folder by single-clicking on it",tLX->clNormalLabel),-1,iX+5,iY+25,0,0);
	cOpenGui.Add(new CCheckbox(bAdd),od_Add,iX+5,iY+iHeight-64,17,17);
	cOpenGui.Add(new CLabel("Add to current playlist",tLX->clNormalLabel),-1,iX+25,iY+iHeight-64,0,0);
	cOpenGui.Add(new CCheckbox(bIncludeSubdirs),od_IncludeSubdirs,iX+5,iY+iHeight-45,17,17);
	cOpenGui.Add(new CLabel("Include subdirectories",tLX->clNormalLabel),-1,iX+25,iY+iHeight-45,0,0);
	cOpenGui.Add(new CButton(BUT_CANCEL,tMenu->bmpButtons),od_Cancel,iX+iWidth/2,iY+iHeight-25,60,15);
	cOpenGui.Add(new CButton(BUT_OK,tMenu->bmpButtons),od_Ok,iX+iWidth/2-50,iY+iHeight-25,30,15);

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

		// Background
		DrawRectFill(Screen,iX,iY,iX+iWidth,iY+iHeight,tLX->clBlack);

		// Title bar
		DrawRectFill(Screen,iX,iY,iX+iWidth,iY+22,MakeColour(0,0,64));

		// Border
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

					if (lv->getCurSIndex() == "")
						break;


					// Check that this is not the parent or current directory
					// TODO !
					size_t dir_name_pos = findLastPathSep(lv->getCurSIndex());
					if (dir_name_pos == std::string::npos)
						break;

					if(lv->getCurSIndex().substr(dir_name_pos+1) == ".." || lv->getCurSIndex().substr(dir_name_pos+1) == ".")
						break;

					// Copy the directory
					szDir = lv->getCurSIndex();

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
		SetGameCursor(CURSOR_ARROW);
		DrawCursor(tMenu->bmpScreen);

		FlipScreen(Screen);
	}

	// Restore and free the buffer
	DrawImage(Screen,bmpBuffer,0,0);
	SDL_FreeSurface(bmpBuffer);

	// Free the GUI
	cOpenGui.Shutdown();

	if (cancelled)
		return "";
	else  {
		// Adjust
		replace(szDir,"//","/",szDir);
		replace(szDir,"\\/","/",szDir);
		return szDir;
	}
}

////////////////////
// Checks if the given directory is the root directory
bool COpenAddDir::IsRoot(const std::string& dir)
{
	// TODO: what is the sense of this?


	if (dir == "")
		return true;

	std::string tmp;
	tmp = dir;

	// Adjust
	replace(tmp,"//","/",tmp);
	replace(tmp,"\\/","/",tmp);

	size_t len = tmp.size();

	// Remove the ending slash
	if (tmp[len-1] == '\\' || tmp[len-1] == '/')
		tmp.erase(len-1);

	// If we can't find another slash, this must be the parent directory
	size_t slash = findLastPathSep(tmp);
	if(slash == std::string::npos)
		return true;

	// If there's a slash and this is the link to the parent directory, check, if there's another slash
	if(tmp.compare(slash+1,std::string::npos,"..") == 0) {
		tmp.erase(slash);
		slash = findLastPathSep(tmp);
		// Not another slash, this is a root directory
		if (slash == std::string::npos)
			return true;

	}

	// Not a root directory
	return false;
}






		// TODO: this won't get even called!!!!!! Fix it!
		class addDirToList { public:
			CListview* lv;
			int* index;
			int* selected;
			const std::string& parent_dir;
			addDirToList(CListview* l, int* i, int* s, const std::string& pd) : lv(l), index(i), selected(s), parent_dir(pd) {}
			inline bool operator() (const std::string& directory) {
				// Extract the directory name from the path
				size_t dir_sep = findLastPathSep(directory);

				// Add the directory
				if (dir_sep != std::string::npos)  {
					if(parent_dir == directory.substr(dir_sep+1))
						*selected = *index;

					lv->AddItem(directory,(*index)++,tLX->clListView);
					lv->AddSubitem(LVS_TEXT,directory.substr(dir_sep+1),NULL);
				}

				return true;
			}
		};

///////////////////////
// Fills the list with the subdirectories of the "dir"
void COpenAddDir::ReFillList(CListview *lv, const std::string& dir)
{
	if (dir == "")
		return;

	std::string directory;
	std::string tmp_dir;
	std::string parent_dir;
	size_t dir_name_pos;
	int index=0;
	size_t len = 0;
	bool isroot = false;  // True if we're in root directory of the current drive
	bool goto_drive_list = false;  // True if we're going to go in the drive list

	tmp_dir = dir;
	len = dir.size();

	// Add the slash if needed
	if (tmp_dir[len-1] != '\\' && tmp_dir[len-1] != '/')  {
		tmp_dir += "/";
		len++;
	}

	parent_dir = "";  // Clear the parent directory

	// TODO: i don't completly understand the sense of this
	// (and i am realy sure that it is wrong in some cases)
	isroot = IsRoot(tmp_dir);
	if(!isroot || (isroot && ((dir_name_pos = tmp_dir.find("../")) == std::string::npos))) {
		// Handle the parent directory
		if((dir_name_pos = tmp_dir.find("../")) != std::string::npos) {
			tmp_dir.erase(dir_name_pos-1);
			dir_name_pos = findLastPathSep(tmp_dir);
			if(dir_name_pos != std::string::npos)  {
				parent_dir = tmp_dir.substr(dir_name_pos+1);
				tmp_dir.erase(dir_name_pos+1);
			}
			// Check again
			isroot = IsRoot(tmp_dir);
		}
	// Root directory and we want to go up
	} else {
		goto_drive_list = true;
	}

	// Clear the listview
	lv->Clear();

	// Going up when this is a root directory is handled in other way than the rest of the browsing
	if (goto_drive_list)  {
		int index = 0;
		drive_list drives = GetDrives();
		char cur_drive = tmp_dir[0]; // TODO: use std::string
		for (drive_list::const_iterator i=drives.begin(); i != drives.end();i++)  {
#ifdef WIN32
			if (i->type != DRV_CDROM)  {
#endif
				lv->AddItem(i->name,index,tLX->clListView);
				lv->AddSubitem(LVS_TEXT,i->name,NULL);
				if (cur_drive == i->name.at(0))
					lv->setSelectedID(index);
				index++;
#ifdef WIN32
			}
#endif
		}

	// The directory list
	} else  {

		// Fill in the first directory
		directory = tmp_dir +"..";


		// Add the parent directory
		lv->AddItem(directory,index++,tLX->clListView);
		lv->AddSubitem(LVS_TEXT,"..",NULL);

		int selected = 0;

		FindFiles(addDirToList(lv, &index, &selected, parent_dir), tmp_dir, FM_DIR);
		if(selected) lv->setSelectedID(selected);
	}

}

#endif // WITH_MEDIAPLAYER
