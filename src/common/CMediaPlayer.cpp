// OpenLieroX Media Player
// Made by Dark Charlie and Albert Zeyer

#include "defs.h"
#include "LieroX.h"
#include "Menu.h"

/*

  Playlist

*/


//////////////////
// Clears and shutdowns the playlist
void CPlayList::Clear(void)
{
	tSongList.clear();
	iCurSong = 0;
	bRepeat = true;
	bShuffle = false;
}

//////////////////
// Loads the directory and adds all music files in the playlist
void CPlayList::Load(char *dir, bool include_subdirs, bool add_to_current_pl)
{
	if (!dir)
		return;

	// Clear me first
	if (!add_to_current_pl)  {
		iCurSong = 0;
		tSongList.clear();
	}

	//
	// Load the files
	//
	char filename[1024]=""; // TODO: !
	int done = false;
	if(!FindFirst(dir,"*",filename))
		done = true;

	char ext[4] = "";
	char *tmp = NULL;
	song_path temp = "";

	while(!done) {
		tmp = strrchr(filename,'.');
		if (tmp)  {
			strncpy(ext,tmp+1,4);
			// Only supported media
			if (!stricmp(ext,"mp3") || !stricmp(ext,"ogg") || !stricmp(ext,"mod") || /*!stricmp(ext,"wav") ||*/
				!stricmp(ext,"mid") || !stricmp(ext,"voc"))  {
				temp = filename;
				tSongList.push_back(temp);
			}
		}
	

		if(!FindNext(filename))
			break;
	}

	//
	//	Subdirectories
	//
	if (!include_subdirs)
		return;

	// TODO: change!
	char directory[1024]="";

	std::string str_temp = "";
	std::vector<std::string> dir_list;

	if(FindFirstDir(dir,directory)) {
		fix_markend(directory);
		while(1) {

			str_temp = directory;
			dir_list.push_back(str_temp);

			if(!FindNextDir(directory))
				break;
			fix_markend(directory);
		}
	}

	for (int i=0;i<dir_list.size();i++)  {
		strcpy(directory,dir_list[i].c_str());
		Load(directory,true,true);
	}

	// TODO: can be strongly optimized
	// TODO: dirty
}

/////////////////
// Defines if the playlist should be repeated
void CPlayList::setRepeat(bool _r)
{
	bRepeat = _r;
	tLXOptions->bRepeatPlaylist = _r;
}

/////////////////
// Defines if the playlist should be shuffled
void CPlayList::setShuffle(bool _s)
{
	bShuffle = _s;
	tLXOptions->bShufflePlaylist = _s;
}

///////////////////
// Get the current played song
song_path CPlayList::GetCurSong(void)
{
	static song_path result = "";
	if (tSongList.size() == 0 || iCurSong < 0)
		return result;

	if (iCurSong > tSongList.size()-1)
		iCurSong = tSongList.size()-1;
	if (iCurSong < 1) {
		iCurSong = 0;
	}

	result = tSongList[iCurSong];

	return result;
}

//////////////////
// Move to the next song in playlist
void CPlayList::GoToNextSong(void)
{
	// Shuffle, just get some random song
	if (bShuffle)  {
		iCurSong = GetRandomInt(tSongList.size()-1);
		iCurSong = abs(iCurSong);
		while (iCurSong >= tSongList.size())  {
			iCurSong = iCurSong-tSongList.size();
		}
	// Not shuffle, go to the next song
	// If repeat is enabled, go to the first song if needed, else stop playing
	} else {
		iCurSong++;
		if (iCurSong > tSongList.size()-1)  {
			if (bRepeat)
				iCurSong = 0;
			else
				iCurSong = -1;
		}
	}
}

//////////////////
// Move to the previous song in playlist
void CPlayList::GoToPrevSong(void)
{
	// Shuffle, just get some random song
	if (bShuffle)  {
		iCurSong = GetRandomInt(tSongList.size()-1);
		iCurSong = abs(iCurSong);
		while (iCurSong >= tSongList.size())  {
			iCurSong = iCurSong-tSongList.size();
		}
	// Not shuffle, go to the previous song
	// If repeat is enabled, go to the last song if needed, else stop playing
	} else {
		iCurSong--;
		if (iCurSong < 0)  {
			if (bRepeat)
				iCurSong = tSongList.size()-1;
			else
				iCurSong = -1;
		}
	}
}

//////////////////
// Loads the previously saved playlist
void CPlayList::LoadFromFile(const char *filename,bool absolute_path)
{
	// Clear first
	tSongList.clear();
	iCurSong = 0;

	// Open the file
	FILE *fp = NULL;
	if (absolute_path)
		fp = fopen(filename,"r");
	else
		fp = OpenGameFile(filename,"r");

	if (!fp)
		return;

	// Read the file line by line
	char line[1024]; // TODO !
	song_path tmp = "";
	while(fgets(line,sizeof(line)-1,fp))  {
		line[fix_strnlen(line)-1] = '\0';  // Remove the newline
		tmp = line;
		tSongList.push_back(tmp);
	}

	fclose(fp);
}

//////////////////
// Loads the previously saved playlist
// NOTE: if the file exists, it will be overwritten
void CPlayList::SaveToFile(const char *filename,bool absolute_path)
{
	// Open the file
	FILE *fp = NULL;
	if (absolute_path)
		fp = fopen(filename,"w");
	else
		fp = OpenGameFile(filename,"w");

	if (!fp)
		return;

	// Write the file
	// Each song means one line
	for (int i=0;i<tSongList.size();i++)  {
		fputs(tSongList[i].c_str(),fp);
		fputs("\n",fp);
	}

	fclose(fp);
}


/*

	Media Player

*/

/////////////////////
// Clears the media player
void CMediaPlayer::Clear(void)
{
	tPlayList.Clear();
	szCurSongName = "";
	FreeMusic(tCurrentSong);
	tCurrentSong = NULL;
	bGfxInitialized = false;
	bDrawPlayer = false;
	bGrabbed = false;
	iLastMouseX = 0;
	iLastMouseY = 0;
	memset(&tPlayerGfx,0,sizeof(player_gfx_t));
}

/////////////////////
// Initialize the whole player
bool CMediaPlayer::Initialize(void)
{
	// Clear
	Clear();

	// Load options
	tPlayList.setRepeat(tLXOptions->bRepeatPlaylist);
	tPlayList.setShuffle(tLXOptions->bShufflePlaylist);

	// Initialize gfx
	return InitializeGfx();
}

/////////////////////
// Shutdowns the media player
void CMediaPlayer::Shutdown(void)
{
	// NOTE: bitmaps are freed by the cache
	cPlayerGui.Shutdown();
	Clear();
}

/////////////////////
// Get the song name from the path
song_name CMediaPlayer::GetNameFromFile(song_path path)
{
	song_name name = "";

	// Try to get the MP3 info
	id3v1_t mp3tag = GetMP3Info(path.c_str());
	if (mp3tag.name[0])  {
		name.append(mp3tag.name);
		if (mp3tag.interpreter[0])  {
			name.append(" - ");
			name.append(mp3tag.interpreter);
		}
		return name;
	}

	// Remove directory
	int pos1 = path.find_last_of('\\');
	int pos2 = path.find_last_of('/');
	int pos = MAX(pos1 >= path.length() ? 0 : pos1,pos2 >= path.length() ? 0 : pos2);
	if (pos)  {
		name = path.substr(pos+1,path.length()-pos);
	} else {
		name = path;
	}

	// Remove extension
	pos = name.find_last_of('.');
	if (pos)  {
		name = name.substr(0,pos);
	}

	return name;
}

//////////////////////
// Loads the playlist from the specified file
void CMediaPlayer::LoadPlaylistFromFile(const char *filename, bool absolute_path)
{
	tPlayList.LoadFromFile(filename,absolute_path);
	if (tPlayList.getNumSongs() > 0)  {
		tPlayList.SetCurSong(0);
		szCurSongName = GetNameFromFile(tPlayList.GetCurSong());
		if (bGfxInitialized)
			((CPlayerMarquee *)(cPlayerGui.getWidget(mp_PlayingMarq)))->setText(szCurSongName.c_str());
	}
}

////////////////////
// Starts playing
void CMediaPlayer::Play(void)
{
	// If paused, just resume
	if (PausedMusic())
		ResumeMusic();
	else  {
		// The playlist is blank, do nothing
		if (tPlayList.getNumSongs() == 0)  {
			FreeMusic(tCurrentSong);
			tCurrentSong = NULL;
			szCurSongName = "";
			Stop();
			return;
		}


		szCurSongName = tPlayList.GetCurSong();  // Use szCurSongName as a temp
		if (szCurSongName.length() > 1)  {
			FreeMusic(tCurrentSong);  // Free the previous song (if any)
			tCurrentSong = LoadMusic(szCurSongName.c_str());
			if (tCurrentSong)  {
				PlayMusic(tCurrentSong);
			}
			szCurSongName = GetNameFromFile(szCurSongName);
			// Update the marquee
			if (bGfxInitialized)
				((CPlayerMarquee *)(cPlayerGui.getWidget(mp_PlayingMarq)))->setText(szCurSongName.c_str());
		} else {
			// Start again
			if (tPlayList.getPlaylistEnded())  {
				tPlayList.SetCurSong(0);
			} else {
				// Nothing to play
				Stop();
			}
		}
	}
}

////////////////////
// Pauses or resumes the current song
void CMediaPlayer::PauseResume(void)
{
	if (!PausedMusic())
		PauseMusic();
	else
		ResumeMusic();
}

///////////////////
// Stop playing and rewind to the beginning
void CMediaPlayer::Stop(void)
{
	if (!GetSongStopped())
		StopMusic();
}

///////////////////
// Rewind to the beginning of the song, doesn't stop playing!
void CMediaPlayer::Rewind(void)
{
	// If we're at the beginning, jump to the previous song
	if (GetCurrentMusicTime() < 2.0f)  {
		Stop();
		tPlayList.GoToPrevSong();
		Play();
	}
	// If we're somewhere further, just rewind to the beginning
	else
		RewindMusic();
}

///////////////////
// Plays the next song in playlist
void CMediaPlayer::Forward(void)
{
	Stop();
	tPlayList.GoToNextSong();
	Play();
}

////////////////////
// Sets the current position in the song (in seconds)
void CMediaPlayer::SetSongPosition(double pos)
{
	SetMusicPosition(pos);
}

////////////////////
// Get the current number of seconds played from the song
float CMediaPlayer::GetSongTime(void)
{
	return GetCurrentMusicTime();
}

//////////////////////
// Set the music volume
void CMediaPlayer::SetVolume(byte vol)
{
	SetMusicVolume(vol);
}

///////////////////////
// Get the music volume
byte CMediaPlayer::GetVolume(void)
{
	return GetMusicVolume();	
}

//////////////////////
// Initialize the graphics
bool CMediaPlayer::InitializeGfx(void)
{
	// Load options
	iX = tLXOptions->iMPlayerLeft;
	iY = tLXOptions->iMPlayerTop;

	LOAD_IMAGE(tPlayerGfx.bmpBackground,	"data/frontend/mplayer/background.png");
	LOAD_IMAGE(tPlayerGfx.bmpHide,			"data/frontend/mplayer/hide.png")
	LOAD_IMAGE(tPlayerGfx.bmpWindow,		"data/frontend/mplayer/window.png");
	LOAD_IMAGE(tPlayerGfx.bmpNext,			"data/frontend/mplayer/next.png");
	LOAD_IMAGE(tPlayerGfx.bmpPause,			"data/frontend/mplayer/pause.png");
	LOAD_IMAGE(tPlayerGfx.bmpPlay,			"data/frontend/mplayer/play.png");
	LOAD_IMAGE(tPlayerGfx.bmpPrevious,		"data/frontend/mplayer/previous.png");
	LOAD_IMAGE(tPlayerGfx.bmpRepeat,		"data/frontend/mplayer/repeat.png");
	LOAD_IMAGE(tPlayerGfx.bmpShuffle,		"data/frontend/mplayer/shuffle.png");
	LOAD_IMAGE(tPlayerGfx.bmpSelectDir,		"data/frontend/mplayer/open.png");
	LOAD_IMAGE(tPlayerGfx.bmpStop,			"data/frontend/mplayer/stop.png");
	LOAD_IMAGE(tPlayerGfx.bmpMusicVolume,	"data/frontend/mplayer/musicvolume.png");
	LOAD_IMAGE(tPlayerGfx.bmpGameVolume,	"data/frontend/mplayer/gamevolume.png");
	LOAD_IMAGE(tPlayerGfx.bmpProgress,		"data/frontend/mplayer/progress.png");
	LOAD_IMAGE(tPlayerGfx.bmpProgressStart,	"data/frontend/mplayer/progress_start.png");
	LOAD_IMAGE(tPlayerGfx.bmpProgressEnd,	"data/frontend/mplayer/progress_end.png");
	bGfxInitialized = true;

	
	cPlayerGui.Initialize();
	cPlayerGui.Add(new CPlayerSlider(tPlayerGfx.bmpProgress,tPlayerGfx.bmpProgressStart,tPlayerGfx.bmpProgressEnd,tPlayerGfx.bmpMusicVolume,100),mp_MusicVol,0,0,0,0);
	cPlayerGui.Add(new CPlayerSlider(tPlayerGfx.bmpProgress,tPlayerGfx.bmpProgressStart,tPlayerGfx.bmpProgressEnd,tPlayerGfx.bmpGameVolume,100),mp_GameVol,0,0,0,0);
	cPlayerGui.Add(new CPlayerButton(tPlayerGfx.bmpNext),mp_Next,0,0,0,0);
	cPlayerGui.Add(new CPlayerButton(tPlayerGfx.bmpHide),mp_Hide,0,0,0,0);
	cPlayerGui.Add(new CPlayerButton(tPlayerGfx.bmpPause),mp_Pause,0,0,0,0);
	cPlayerGui.Add(new CPlayerButton(tPlayerGfx.bmpPlay),mp_Play,0,0,0,0);
	cPlayerGui.Add(new CPlayerButton(tPlayerGfx.bmpPrevious),mp_Previous,0,0,0,0);
	cPlayerGui.Add(new CPlayerToggleBtn(tPlayerGfx.bmpRepeat,GetRepeatPlaylist()),mp_Repeat,0,0,0,0);
	cPlayerGui.Add(new CPlayerToggleBtn(tPlayerGfx.bmpShuffle,GetShufflePlaylist()),mp_Shuffle,0,0,0,0);
	cPlayerGui.Add(new CPlayerButton(tPlayerGfx.bmpSelectDir),mp_SelectDir,0,0,0,0);
	cPlayerGui.Add(new CPlayerButton(tPlayerGfx.bmpStop),mp_Stop,0,0,0,0);
	cPlayerGui.Add(new CPlayerMarquee("",tLX->clMPlayerSong),mp_PlayingMarq,0,0,0,0);

	((CPlayerSlider *)(cPlayerGui.getWidget(mp_MusicVol)))->SetValue(GetVolume());
	((CPlayerSlider *)(cPlayerGui.getWidget(mp_GameVol)))->SetValue(GetSoundVolume());
	return true;
}

///////////////////////
// Draws the player to the destination surface
void CMediaPlayer::Draw(SDL_Surface *bmpDest)
{
	// Can't draw
	if (!bGfxInitialized || !bDrawPlayer)
		return;

	// Clipping
	if (iX+tPlayerGfx.bmpBackground->w >= bmpDest->w || iY+tPlayerGfx.bmpBackground->h >= bmpDest->h)
		return;
	if (iX < 0 || iY < 0)
		return;

	// Update the widget positions according to the current coordinates
	cPlayerGui.getWidget(mp_Next)->Setup(mp_Next,iX+150,iY+70,tPlayerGfx.bmpNext->w,tPlayerGfx.bmpNext->h/2);
	cPlayerGui.getWidget(mp_Pause)->Setup(mp_Pause,iX+85,iY+60,tPlayerGfx.bmpPause->w,tPlayerGfx.bmpPause->h/2);
	cPlayerGui.getWidget(mp_Play)->Setup(mp_Play,iX+85,iY+60,tPlayerGfx.bmpPlay->w,tPlayerGfx.bmpPlay->h/2);
	cPlayerGui.getWidget(mp_Previous)->Setup(mp_Previous,iX+58,iY+70,tPlayerGfx.bmpPrevious->w,tPlayerGfx.bmpPrevious->h/2);
	cPlayerGui.getWidget(mp_Repeat)->Setup(mp_Repeat,iX+208,iY+15,tPlayerGfx.bmpRepeat->w,tPlayerGfx.bmpRepeat->h/2);
	cPlayerGui.getWidget(mp_Shuffle)->Setup(mp_Shuffle,iX+208,iY+30,tPlayerGfx.bmpShuffle->w,tPlayerGfx.bmpShuffle->h/2);
	cPlayerGui.getWidget(mp_SelectDir)->Setup(mp_SelectDir,iX+10,iY+70,tPlayerGfx.bmpSelectDir->w,tPlayerGfx.bmpSelectDir->h/2);
	cPlayerGui.getWidget(mp_Stop)->Setup(mp_Stop,iX+122,iY+70,tPlayerGfx.bmpStop->w,tPlayerGfx.bmpStop->h/2);
	cPlayerGui.getWidget(mp_MusicVol)->Setup(mp_MusicVol,iX+5,iY+45,tPlayerGfx.bmpMusicVolume->w,tPlayerGfx.bmpMusicVolume->h);
	cPlayerGui.getWidget(mp_GameVol)->Setup(mp_GameVol,iX+110,iY+45,tPlayerGfx.bmpGameVolume->w,tPlayerGfx.bmpGameVolume->h);
	cPlayerGui.getWidget(mp_PlayingMarq)->Setup(mp_PlayingMarq,iX+42,iY+25,150,tLX->cFont.GetHeight());
	cPlayerGui.getWidget(mp_Hide)->Setup(mp_Hide,iX+tPlayerGfx.bmpBackground->w-tPlayerGfx.bmpHide->w,iY,tPlayerGfx.bmpHide->w,tPlayerGfx.bmpHide->h/2);

	if (Paused())  {
		cPlayerGui.getWidget(mp_Pause)->setEnabled(false);
		cPlayerGui.getWidget(mp_Play)->setEnabled(true);
	}
	else  {
		cPlayerGui.getWidget(mp_Play)->setEnabled(false);
		cPlayerGui.getWidget(mp_Pause)->setEnabled(true);
	}

	if (Stopped() || !Playing())  {
		cPlayerGui.getWidget(mp_Pause)->setEnabled(false);
		cPlayerGui.getWidget(mp_Play)->setEnabled(true);
	}


	// Draw the background
	DrawImage(bmpDest,tPlayerGfx.bmpBackground,iX,iY);

	// Draw the song info window background
	int src_x = 0;  // Playing
	if (Paused())   // Paused
		src_x = tPlayerGfx.bmpWindow->h/3;
	if (Stopped() || !Playing())  // Stopped
		src_x = 2*tPlayerGfx.bmpWindow->h/3;
	DrawImageAdv(bmpDest,tPlayerGfx.bmpWindow,0,src_x,iX+5,iY+5,tPlayerGfx.bmpWindow->w,tPlayerGfx.bmpWindow->h/3);

	// Draw the current time
	int h,m,s;
	ConvertTime(GetSongTime(), &h,&m,&s);
	tLX->cFont.Draw(bmpDest,iX+5+tPlayerGfx.bmpWindow->w/2,iY+10,tLX->clMPlayerTime,"%d:%s%d",m,s<10 ? "0" : "",s);

	// Draw all the widgets
	cPlayerGui.Draw(bmpDest);

	// Draw the mouse
	DrawImage(bmpDest,gfxGUI.bmpMouse[0],GetMouse()->X,GetMouse()->Y);
}

//////////////////////
// True - show player, false - hide player
void CMediaPlayer::SetDrawPlayer(bool _d)
{
	bDrawPlayer = _d;

	// Redraw the menu if needed
	if (!bDrawPlayer && tMenu->iMenuRunning)  {
		Menu_redrawBufferRect(iX,iY,GetWidth(),GetHeight());
	}
}

/////////////////////
// Set the X coordinate of the Media Player
void CMediaPlayer::SetX(int x)
{
	iX = x;

	// Screen clipping
	if (iX + GetWidth() >= SDL_GetVideoSurface()->w)
		iX = SDL_GetVideoSurface()->w-GetWidth()-1;
	else if (iX < 0)
		iX = 0;	

	tLXOptions->iMPlayerLeft = iX;
}

/////////////////////
// Set the Y coordinate of the Media Player
void CMediaPlayer::SetY(int y)
{
	iY = y;

	// Screen clipping
	if (iY + GetHeight() >= SDL_GetVideoSurface()->h)
		iY = SDL_GetVideoSurface()->h-GetHeight()-1;
	else if (iY < 0)
		iY = 0;	

	tLXOptions->iMPlayerTop = iY;
}

/////////////////////
// The processing frame
void CMediaPlayer::Frame(void)
{
	// If the song ended, go to next one in playlist
	if (GetSongFinished())  {
		Forward();
	}

	// Handle the toggle key
	if (cToggleMediaPlayer.isDownOnce())  {
		SetDrawPlayer(!bDrawPlayer);
	}

	// We don't need to process widgets if we don't draw them
	if (!bGfxInitialized || !bDrawPlayer)
		return;

	gui_event_t *ev = NULL;

	mouse_t *tMouse = GetMouse();

	// Process the gui
	if (!bGrabbed || tMouse->Up)  // Hack: don't process when the user is moving the window
		ev = cPlayerGui.Process();
	if (ev)  {
		switch (ev->iControlID)  {
		case mp_Next:
			Forward();
			break;
		case mp_Pause:
			PauseResume();
			break;
		case mp_Play:
			Play();
			break;
		case mp_Previous:
			Rewind();
			break;
		case mp_Repeat:
			SetRepeatPlaylist(((CPlayerToggleBtn *)(cPlayerGui.getWidget(mp_Repeat)))->isOn());
			break;
		case mp_Shuffle:
			SetShufflePlaylist(((CPlayerToggleBtn *)(cPlayerGui.getWidget(mp_Shuffle)))->isOn());
			break;
		case mp_SelectDir:  {
			if (!Paused() && Playing())
				PauseResume();
			char *dir = cOpenDialog.Execute("C:\\");
			if (dir)  {
				tPlayList.Load(dir,cOpenDialog.getIncludeSubdirs(),cOpenDialog.getAdd());
				if (!cOpenDialog.getAdd())
					Stop();
			}
			if (Playing())
				Play();
			}
			break;
		case mp_Stop:
			Stop();
			break;
		case mp_MusicVol:
			SetMusicVolume((byte)((CPlayerSlider *)(cPlayerGui.getWidget(mp_MusicVol)))->GetValue());
			break;
		case mp_GameVol:
			SetSoundVolume(((CPlayerSlider *)(cPlayerGui.getWidget(mp_GameVol)))->GetValue());
			break;
		case mp_Hide:
			SetDrawPlayer(false);
			break;
		}
	} else {

		// Process the mouse dragging
		if (tMouse->Down)  {
			if (!bGrabbed)  {
				if (MouseInRect(iX,iY,GetWidth(),GetHeight()))  {
					bGrabbed = true;
					iLastMouseX = tMouse->X;
					iLastMouseY = tMouse->Y;
				}
			} else {
				// Clear the current position from the menu screen if needed
				if (tMenu->iMenuRunning)
					Menu_redrawBufferRect(iX,iY,GetWidth(),GetHeight());

				SetX(iX+tMouse->X-iLastMouseX);
				SetY(iY+tMouse->Y-iLastMouseY);

				iLastMouseX = tMouse->X;
				iLastMouseY = tMouse->Y;
			}
		} else {
			bGrabbed = false;
			iLastMouseX = 0;
			iLastMouseY = 0;
		}
	}
}
