// OpenLieroX Media Player
// Made by Dark Charlie and Albert Zeyer
// code under LGPL



#ifdef WITH_MEDIAPLAYER

#include "LieroX.h"
#include "MathLib.h"
#include "AuxLib.h"
#include "Graphics.h"
#include "Menu.h"
#include "GfxPrimitives.h"
#include "FindFile.h"
#include "StringUtils.h"

/*

  Playlist filler stack

*/


// TODO: remove all this, if we have tested it enough
#define OWN_DIRSTACK 0
#if OWN_DIRSTACK == 1

typedef struct stackitem_s  {
	std::string str;
	stackitem_s *prev;
} stackitem_t;

class CDirStack {
private:
	stackitem_t *tStackTop;  // std::vector is slower, std::list even more; std::stack not in msvc
	stackitem_t *iter;
public:
	inline CDirStack()  { tStackTop = NULL; }
	inline ~CDirStack() { Clear(); }
	inline void Clear() {
		while(tStackTop) {
			iter = tStackTop->prev;
			delete tStackTop;
			tStackTop = iter;
		}
	}
	inline void Push(const std::string& dir) {
		iter = new stackitem_t;
		if(iter) {
			iter->prev = tStackTop;
			iter->str = dir;
			tStackTop = iter;
		}
	}
	inline bool Pop(std::string& dir)  { 
		if (tStackTop == NULL) {
			return false; 
		} else {
			dir = tStackTop->str;
			iter = tStackTop->prev;
			delete tStackTop;
			tStackTop = iter;
			return true; 
		}
	}
};
#else // not OWN_DIRSTACK

class CDirStack {
private:
	// TODO: test it also with deque
	std::list<std::string> stack;
public:
	inline void Clear() { stack.clear(); }
	inline void Push(const std::string& str) { stack.push_back(str); }
	inline bool Pop(std::string& str) {
		if(stack.size() == 0) return false;
		str = stack.back(); stack.pop_back();
		return true;
	}
};

#endif

// TODO: why the hell is this a global variable?
// It has to be accessible both from the functor and from CPlayList::Load
CDirStack cStack;


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
	bLoadCancelled = false;
}



class PlaylistLoader { public:
	// TODO: why does it need a playlist ref?
	CPlayList* playlist;
	PlaylistLoader(CPlayList* pl) : playlist(pl) {}
	inline bool operator() (const std::string& dir) {
		// TODO: only add it, if it is a dir
		cStack.Push(dir);
		return true;
	}
};

class SongListFiller { public:
	CPlayList* playlist;
	SongListFiller(CPlayList* pl) : playlist(pl) {}
	inline bool operator() (const std::string& file) {
		static const std::string supported_media[] = {"mp3","ogg","mod","mid","voc"};
		std::string ext = GetFileExtension(file);
		stringlwr(ext);
		for(register unsigned short i=0; i<sizeof(supported_media)/sizeof(std::string); i++)
			if(supported_media[i] == ext) {
				playlist->tSongList.push_back(file);
				break;
			}
		return true;
	}
};

bool CPlayList::DrawLoadingProgress(void)
{
	if (bLoadCancelled)
		return false;

	SDL_Surface *screen = SDL_GetVideoSurface();
	if (!screen) return false;

	ProcessEvents();

	mouse_t *mouse = GetMouse();
	keyboard_t *kb = GetKeyboard();
	static CButton btnCancel(BUT_CANCEL,tMenu->bmpButtons);

	bool result = true;

	// Position
	static const int w = 270;
	static const int h = 90;
	static const int x = screen->w/2-w/2;
	static const int y = screen->h/2-h/2;
	btnCancel.Setup(0,x+w/2-40,y+h-20,60,20);

	// Process events
	if (btnCancel.InBox(mouse->X,mouse->Y))  {
		if (mouse->Down)
			btnCancel.MouseDown(mouse,true);
		else if (mouse->Up)
			result = btnCancel.MouseUp(mouse,false) != BTN_MOUSEUP;
		else 
			btnCancel.MouseOver(mouse);
	}
	result = result || kb->KeyDown[SDLK_ESCAPE];

	// Draw Player
	cMediaPlayer.Draw(screen);

	// Draw the dialog
	DrawRectFill(screen,x,y,x+w,y+h,tLX->clBlack);
	Menu_DrawBox(screen,x,y,x+w,y+h);

	tLX->cFont.DrawCentre(screen,x+w/2,y+5,tLX->clNormalLabel,"Searching for songs, please wait...");
	tLX->cFont.Draw(screen,x+w/4,y+5+tLX->cFont.GetHeight()+5,tLX->clNormalLabel,"Songs found: "+itoa((int)tSongList.size()));

	btnCancel.Draw2(screen);

	// Draw mouse
	SetGameCursor(CURSOR_ARROW);
	DrawCursor(tMenu->bmpScreen);

	// Flip the screen
	FlipScreen(screen);

	// Redraw the menu
	if (tMenu->iMenuRunning)  {
		Menu_redrawBufferRect(x,y,w+1,h+1);
		Menu_redrawBufferRect(mouse->X,mouse->Y,GetCursorWidth(CURSOR_ARROW),GetCursorHeight(CURSOR_ARROW));
	}

	bLoadCancelled = !result;
	return result;
}

//////////////////
// Loads the directory and adds all music files in the playlist
void CPlayList::Load(const std::string& dir, bool include_subdirs, bool add_to_current_pl)
{
	// Reset the status when we're called for first time
/*	if (firstcall)
		bLoadCancelled = false;

	// Draw the progress
	if (!DrawLoadingProgress())
		return;

	// Clear me first
	if (!add_to_current_pl)  {
		iCurSong = 0;
		tSongList.clear();
	}

	//
	// Load the files
	//
	FindFiles(SongListFiller(this), dir, FM_REG);

	//
	//	Subdirectories
	//
	if (!include_subdirs)
		return;

	FindFiles(PlaylistLoader(this), dir, FM_DIR);*/

	// Clear me first
	if (!add_to_current_pl)  {
		iCurSong = 0;
		tSongList.clear();
	}
	bLoadCancelled = false;
	cStack.Clear();

	if(!include_subdirs)  {
		FindFiles(SongListFiller(this), dir, FM_REG);
		return;
	}

	std::string current_dir = dir;
	cStack.Push(current_dir);
	while(DrawLoadingProgress() && cStack.Pop(current_dir)) {
		// TODO: merge SongListFiller with PlaylistLoader to speed it up
		FindFiles(SongListFiller(this), current_dir, FM_REG);
		FindFiles(PlaylistLoader(this), current_dir, FM_REG);
	}
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
std::string CPlayList::GetCurSong() {
	static std::string result; result = "";
	if(tSongList.size() == 0 || iCurSong < 0)
		return result;

	if(iCurSong < 1)
		iCurSong = 0;
	else if((size_t)iCurSong >= tSongList.size())
		iCurSong = (int)tSongList.size()-1;

	result = tSongList[iCurSong];

	return result;
}

//////////////////
// Move to the next song in playlist
void CPlayList::GoToNextSong() {
	// Shuffle, just get some random song
	if (bShuffle)  {
		iCurSong = GetRandomInt((int)tSongList.size()-1);
		iCurSong = abs(iCurSong);
		iCurSong %= tSongList.size();
	// Not shuffle, go to the next song
	// If repeat is enabled, go to the first song if needed, else stop playing
	} else {
		iCurSong++;
		if((size_t)iCurSong >= tSongList.size())  {
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
		iCurSong = GetRandomInt((int)tSongList.size()-1);
		iCurSong = abs(iCurSong);
		iCurSong %= tSongList.size();
	// Not shuffle, go to the previous song
	// If repeat is enabled, go to the last song if needed, else stop playing
	} else {
		iCurSong--;
		if (iCurSong < 0)  {
			if (bRepeat)
				iCurSong = (int)tSongList.size()-1;
			else
				iCurSong = -1;
		}
	}
}

//////////////////
// Loads the previously saved playlist
void CPlayList::LoadFromFile(const std::string& filename, bool absolute_path) {
	// Clear first
	tSongList.clear();
	iCurSong = 0;

	// Open the file
	FILE *fp = NULL;
	if (absolute_path)
		fp = fopen(filename.c_str(),"r");
	else
		fp = OpenGameFile(filename,"r");

	if (!fp)
		return;

	// Read the file line by line
	static std::string line;
	while(!feof(fp))  {
		line = ReadUntil(fp); // read a line
		tSongList.push_back(line);
	}

	fclose(fp);
}

//////////////////
// Loads the previously saved playlist
// NOTE: if the file exists, it will be overwritten
void CPlayList::SaveToFile(const std::string& filename, bool absolute_path) {
	// Open the file
	FILE *fp = NULL;
	if (absolute_path)
		fp = fopen(filename.c_str(), "w");
	else
		fp = OpenGameFile(filename, "w");

	if (!fp)
		return;

	// Write the file
	// Each song means one line
	for (std::vector<std::string>::const_iterator i = tSongList.begin(); i != tSongList.end(); i++) {
		fputs(i->c_str(), fp);
		fputs("\n", fp);
	}

	fclose(fp);
}


/*

	Media Player

*/

/////////////////////
// Clears the media player
void CMediaPlayer::Clear() {
	tPlayList.Clear();
	szCurSongName = "";
	// The song is freed by the playing thread now, so no need to free it
	// When you switch to single threaded mode, uncomment it after removing the thread
	//FreeMusic(tCurrentSong);
	//tCurrentSong = NULL;
	bGfxInitialized = false;
	bDrawPlayer = false;
	bGrabbed = false;
	iLastMouseX = 0;
	iLastMouseY = 0;
	
	tPlayerGfx.Clear();
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
std::string CMediaPlayer::GetNameFromFile(const std::string& path)
{
	std::string name = "";

	// Try to get the MP3 info
	id3v1_t mp3tag = GetMP3Info(path);
	if (mp3tag.name[0])  {
		name.append(mp3tag.name);
		if (mp3tag.interpreter[0])  {
			name.append(" - ");
			name.append(mp3tag.interpreter);
		}
		return name;
	}

	// Remove directory
	size_t pos = findLastPathSep(path);
	if(pos != std::string::npos)  {
		name = path.substr(pos+1);
	} else {
		name = path;
	}

	// Remove extension
	pos = name.find_last_of('.');
	if(pos != std::string::npos)  {
		name.erase(pos);
	}

	return name;
}

//////////////////////
// Loads the playlist from the specified file
void CMediaPlayer::LoadPlaylistFromFile(const std::string& filename, bool absolute_path) {
	tPlayList.LoadFromFile(filename, absolute_path);
	if (tPlayList.getNumSongs() > 0)  {
		tPlayList.SetCurSong(0);
		szCurSongName = GetNameFromFile(tPlayList.GetCurSong());
		if (bGfxInitialized)
			((CPlayerMarquee *)(cPlayerGui.getWidget(mp_PlayingMarq)))->setText(szCurSongName);
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
			// TODO: why is this commented out? please COMMENT!
			//FreeMusic(tCurrentSong);
			//tCurrentSong = NULL;
			szCurSongName = "";
			Stop();
			return;
		}


		szCurSongName = tPlayList.GetCurSong();  // Use szCurSongName as a temp
		if (szCurSongName.length() > 1)  {
			// TODO: what is wrong here? why is this commented out?
			/*FreeMusic(tCurrentSong);  // Free the previous song (if any)
			tCurrentSong = LoadMusic(szCurSongName);
			if (tCurrentSong)  {
				PlayMusic(tCurrentSong);
			}*/
			PlayMusicAsync(szCurSongName);
			szCurSongName = GetNameFromFile(szCurSongName);
			// Update the marquee
			if (bGfxInitialized)
				((CPlayerMarquee *)(cPlayerGui.getWidget(mp_PlayingMarq)))->setText(szCurSongName);
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

	LOAD_IMAGE_WITHALPHA(tPlayerGfx.bmpBackground,	"data/frontend/mplayer/background.png");
	LOAD_IMAGE_WITHALPHA(tPlayerGfx.bmpHide,			"data/frontend/mplayer/hide.png")
	LOAD_IMAGE_WITHALPHA(tPlayerGfx.bmpWindow,		"data/frontend/mplayer/window.png");
	LOAD_IMAGE_WITHALPHA(tPlayerGfx.bmpNext,			"data/frontend/mplayer/next.png");
	LOAD_IMAGE_WITHALPHA(tPlayerGfx.bmpPause,			"data/frontend/mplayer/pause.png");
	LOAD_IMAGE_WITHALPHA(tPlayerGfx.bmpPlay,			"data/frontend/mplayer/play.png");
	LOAD_IMAGE_WITHALPHA(tPlayerGfx.bmpPrevious,		"data/frontend/mplayer/previous.png");
	LOAD_IMAGE_WITHALPHA(tPlayerGfx.bmpRepeat,		"data/frontend/mplayer/repeat.png");
	LOAD_IMAGE_WITHALPHA(tPlayerGfx.bmpShuffle,		"data/frontend/mplayer/shuffle.png");
	LOAD_IMAGE_WITHALPHA(tPlayerGfx.bmpSelectDir,		"data/frontend/mplayer/open.png");
	LOAD_IMAGE_WITHALPHA(tPlayerGfx.bmpStop,			"data/frontend/mplayer/stop.png");
	LOAD_IMAGE_WITHALPHA(tPlayerGfx.bmpMusicVolume,	"data/frontend/mplayer/musicvolume.png");
	LOAD_IMAGE_WITHALPHA(tPlayerGfx.bmpGameVolume,	"data/frontend/mplayer/gamevolume.png");
	LOAD_IMAGE_WITHALPHA(tPlayerGfx.bmpProgress,		"data/frontend/mplayer/progress.png");
	LOAD_IMAGE_WITHALPHA(tPlayerGfx.bmpProgressStart,	"data/frontend/mplayer/progress_start.png");
	LOAD_IMAGE_WITHALPHA(tPlayerGfx.bmpProgressEnd,	"data/frontend/mplayer/progress_end.png");
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
	if (IsSongLoading())  {
		static int FontHalfWidth = tLX->cFont.GetWidth("Loading...")/2;
		tLX->cFont.Draw(bmpDest,iX+5+tPlayerGfx.bmpWindow->w/2-FontHalfWidth,iY+10,tLX->clMPlayerTime,"Loading...");
	} else {
		int h,m,s;
		ConvertTime(GetSongTime(), &h,&m,&s);
		tLX->cFont.Draw(bmpDest,iX+5+tPlayerGfx.bmpWindow->w/2,iY+10,tLX->clMPlayerTime,
			itoa(m) + ":" + (s<10 ? "0" : "") + itoa(s));
	}

	// Draw all the widgets
	cPlayerGui.Draw(bmpDest);

	// Draw the mouse
	SetGameCursor(CURSOR_ARROW);
	DrawCursor(tMenu->bmpScreen);
}

//////////////////////
// True - show player, false - hide player
void CMediaPlayer::SetDrawPlayer(bool _d)
{
	bDrawPlayer = _d;

	// Redraw the menu if needed
	if (!bDrawPlayer && tMenu->iMenuRunning)  {
		Menu_redrawBufferRect(iX, iY, GetWidth(), GetHeight());
	}
}

/////////////////////
// Set the X coordinate of the Media Player
void CMediaPlayer::SetX(int x)
{
	iX = x;

	// Screen clipping
	if (iX + GetWidth() >= SDL_GetVideoSurface()->w)
		iX = SDL_GetVideoSurface()->w - GetWidth() - 1;
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
		iY = SDL_GetVideoSurface()->h - GetHeight() - 1;
	else if (iY < 0)
		iY = 0;

	tLXOptions->iMPlayerTop = iY;
}

/////////////////////
// The processing frame
void CMediaPlayer::Frame() {
	// If the song ended, go to next one in playlist
	if (GetSongFinished())  {
		Forward();
	}

	//if ((tCurrentSong = GetLoadedMusic()) != NULL)  {
	//	PlayMusic(tCurrentSong);
	//}

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

		// Next song
		case mp_Next:
			if (ev->iEventMsg == MP_BTN_CLICK)
				Forward();
			break;

		// Pause/resume
		case mp_Pause:
			if (ev->iEventMsg == MP_BTN_CLICK)
				PauseResume();
			break;

		// Play
		case mp_Play:
			if (ev->iEventMsg == MP_BTN_CLICK)
				Play();
			break;

		// Previous song or rewind
		case mp_Previous:
			if (ev->iEventMsg == MP_BTN_CLICK)
				Rewind();
			break;

		// Toggle repeat
		case mp_Repeat:
			if (ev->iEventMsg == MP_TOG_TOGGLE)
				SetRepeatPlaylist(((CPlayerToggleBtn *)(cPlayerGui.getWidget(mp_Repeat)))->isOn());
			break;

		// Toggle shuffle
		case mp_Shuffle:
			if (ev->iEventMsg == MP_TOG_TOGGLE)
				SetShufflePlaylist(((CPlayerToggleBtn *)(cPlayerGui.getWidget(mp_Shuffle)))->isOn());
			break;

		// Select directory dialog
		case mp_SelectDir:  {
			if (ev->iEventMsg == MP_BTN_CLICK)  {
				if (!Paused() && Playing())
					PauseResume();
				std::string dir = cOpenDialog.Execute("C:\\");
				if(dir.size()>0)  {
					tPlayList.Load(dir, cOpenDialog.getIncludeSubdirs(), cOpenDialog.getAdd());
					if (!cOpenDialog.getAdd())
						Stop();
				}
				if (Playing())
					Play();
				}
			}
			break;

		// Stop button
		case mp_Stop:
			if (ev->iEventMsg == MP_BTN_CLICK)
				Stop();
			break;

		// Music volume changed
		case mp_MusicVol:
			if (ev->iEventMsg == MP_SLD_CHANGE)
			SetMusicVolume((byte)((CPlayerSlider *)(cPlayerGui.getWidget(mp_MusicVol)))->GetValue());
			break;

		// Game volume changed
		case mp_GameVol:
			if (ev->iEventMsg == MP_SLD_CHANGE)
				SetSoundVolume(((CPlayerSlider *)(cPlayerGui.getWidget(mp_GameVol)))->GetValue());
			break;

		// Hide button
		case mp_Hide:
			if (ev->iEventMsg == MP_BTN_CLICK)
				SetDrawPlayer(false);
			break;
		}

	} else { // not event

		// Process the mouse dragging
		if (tMouse->Down)  {
			if (!bGrabbed)  {
				if (MouseInRect(iX, iY, GetWidth(), GetHeight()))  {
					bGrabbed = true;
					iLastMouseX = tMouse->X;
					iLastMouseY = tMouse->Y;
				}
			} else {
				// Clear the current position from the menu screen if needed
				if (tMenu->iMenuRunning)
					Menu_redrawBufferRect(iX, iY, GetWidth(), GetHeight());

				SetX(iX + tMouse->X - iLastMouseX);
				SetY(iY + tMouse->Y - iLastMouseY);

				iLastMouseX = tMouse->X;
				iLastMouseY = tMouse->Y;
			}
		} else {
			bGrabbed = false;
			iLastMouseX = 0;
			iLastMouseY = 0;
		}

		// Hide on ESC
		if (GetKeyboard()->KeyDown[SDLK_ESCAPE])  {
			bDrawPlayer = false;
			if (tMenu->iMenuRunning)
				Menu_redrawBufferRect(iX, iY, GetWidth(), GetHeight());
		}
	}
}

#endif // WITH_MEDIAPLAYER
