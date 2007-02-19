// OpenLieroX Media Player
// Made by Dark Charlie and Alber Zeyer
// code under LGPL

#ifndef __MEDIAPLAYER_H__
#define __MEDIAPLAYER_H__

#include <vector>

typedef std::vector<std::string> song_list;


//
//	Open/add directory dialog
//

class COpenAddDir  {
public:
	COpenAddDir(void)  {
		bAdd = false;
		bIncludeSubdirs = true;
		iWidth = 300;
		iHeight = 360;
		iX = 320-iWidth/2;
		iY = 240-iHeight/2;
	}
private:
	std::string	szDir;
	bool	bAdd;
	bool	bIncludeSubdirs;
	int		iX;
	int		iY;
	int		iWidth;
	int		iHeight;
CGuiLayout cOpenGui;

	// Methods
	void		ReFillList(CListview *lv, const std::string& dir);
	bool		IsRoot(const std::string& dir);
public:
	std::string Execute(const std::string& default_dir);
	inline bool getAdd(void) { return bAdd; }
	inline bool getIncludeSubdirs(void) { return bIncludeSubdirs; }
};

//
//	Play list
//

class CPlayList {
public:
	CPlayList() {
		Clear();
	}
private:
	song_list	tSongList;
	int			iCurSong;
	bool		bRepeat;
	bool		bShuffle;
public:
	void		Clear(void);
	void		Load(const std::string& dir,bool include_subdirs, bool add_to_current_pl);
	std::string	GetCurSong(void);
	void		GoToNextSong(void);
	void		GoToPrevSong(void);
	void		setRepeat(bool _r);
	inline bool	getRepeat(void)		{return bRepeat; }
	inline bool	getPlaylistEnded(void) { return (iCurSong == -1) || (tSongList.size() == 0); }
	inline void SetCurSong(int s) { s=MIN(s,tSongList.size()); s=MAX(0,s); iCurSong = s; }
	void		setShuffle(bool _s);
	inline bool	getShuffle(void) { return bShuffle; }
	inline int	getNumSongs(void) { return tSongList.size(); }

	void		SaveToFile(const std::string& filename,bool absolute_path);
	void		LoadFromFile(const std::string& filename,bool absolute_path);
};


typedef struct player_gfx_s {
	SDL_Surface *bmpBackground;
	SDL_Surface *bmpHide;
	SDL_Surface *bmpWindow;
	SDL_Surface *bmpPlay;
	SDL_Surface *bmpPause;
	SDL_Surface *bmpStop;
	SDL_Surface *bmpNext;
	SDL_Surface *bmpPrevious;
	SDL_Surface *bmpSelectDir;
	SDL_Surface *bmpRepeat;
	SDL_Surface *bmpShuffle;
	SDL_Surface *bmpMusicVolume;
	SDL_Surface *bmpGameVolume;
	SDL_Surface *bmpProgress;
	SDL_Surface *bmpProgressStart;
	SDL_Surface *bmpProgressEnd;
} player_gfx_t;

// Widget ids
enum {
	mp_Next=0,
	mp_Pause,
	mp_Play,
	mp_Previous,
	mp_Repeat,
	mp_Shuffle,
	mp_SelectDir,
	mp_Stop,
	mp_MusicVol,
	mp_GameVol,
	mp_PlayingMarq,
	mp_Hide
};

//
//	Media player
//

class CMediaPlayer {
public:
	// Constructor
	CMediaPlayer() {
		Clear();
	}

	~CMediaPlayer() {
		Shutdown();
	}

private:
	// Attributes
	std::string		szCurSongName;
	SoundMusic		*tCurrentSong;
	CPlayList		tPlayList;

	// Drawing
	player_gfx_t	tPlayerGfx;
	bool			bGfxInitialized;
	bool			bDrawPlayer;
	CGuiLayout		cPlayerGui;
	int				iX, iY;
	int				iLastMouseX,iLastMouseY;
	bool			bGrabbed;


	// Dialogs
	COpenAddDir		cOpenDialog;

	

public:
	 void		Clear(void);
	 bool		Initialize(void);
	 void		Shutdown(void);
	 std::string	GetNameFromFile(const std::string& path);
	 void		Play(void);
	 void		PauseResume(void);
	 void		Stop(void);
	 void		Rewind(void);
	 void		Forward(void);
	 void		SetSongPosition(double pos);
	 float		GetSongTime(void);
	 void		SetVolume(byte _v);
	 byte		GetVolume(void);

	 inline bool	Paused(void)	{return PausedMusic(); }
	 inline bool	Playing(void)	{return PlayingMusic(); }
	 inline bool	Stopped(void)	{return GetSongStopped(); }
	 inline void	SetRepeatPlaylist(bool _r)  {tPlayList.setRepeat(_r); }
	 inline bool	GetRepeatPlaylist(void)		{return tPlayList.getRepeat(); }
	 inline void	SetShufflePlaylist(bool _s)  {tPlayList.setShuffle(_s); }
	 inline bool	GetShufflePlaylist(void)		{return tPlayList.getShuffle(); }
	 inline void	OpenDirectory(const std::string& dir,bool include_subdirs=true,bool add_to_current_pl=true)  {tPlayList.Load(dir,include_subdirs,add_to_current_pl); }
	 inline const std::string& GetCurrentSongName(void)	{return szCurSongName; }

	 void			LoadPlaylistFromFile(const std::string& filename,bool absolute_path=false);
	 inline void	SavePlaylistToFile(const std::string& filename,bool absolute_path=false)  {tPlayList.SaveToFile(filename,absolute_path); }

	 // Drawing and GUI processing
	 bool			InitializeGfx(void);
	 void			Draw(SDL_Surface *bmpDest);
	 void			Frame(void);
	 void			SetDrawPlayer(bool _d);
	 inline bool	GetDrawPlayer(void) { return bDrawPlayer; }
	 inline int		GetX(void)  { return iX; }
	 void			SetX(int x);
	 inline int		GetY(void)  { return iY; }
	 void			SetY(int y);
	 inline int		GetWidth(void)   { if(tPlayerGfx.bmpBackground) {return tPlayerGfx.bmpBackground->w; } else return -1; }
	 inline int		GetHeight(void)  { if(tPlayerGfx.bmpBackground) {return tPlayerGfx.bmpBackground->h; } else return -1; }
};

// Media player GUI
#define MP_WID_NONE -1

enum {
	MP_BTN_CLICK = 0,
	MP_SLD_CHANGE,
	MP_TOG_TOGGLE,
	MP_WID_MOUSEDOWN
};


// Button
class CPlayerButton: public CWidget  {
public:
	CPlayerButton(SDL_Surface *image);

private:
	SDL_Surface		*bmpImage;
	bool			bDown;

public:
	// Methods
	void	Create(void) { }
	void	Destroy(void) { }

	//These events return an event id, otherwise they return -1
	int		MouseOver(mouse_t *tMouse)			{ return MP_WID_NONE; }
	int		MouseUp(mouse_t *tMouse, int nDown);
	int		MouseDown(mouse_t *tMouse, int nDown);
	int		MouseWheelDown(mouse_t *tMouse)		{ return MP_WID_NONE; }
	int		MouseWheelUp(mouse_t *tMouse)		{ return MP_WID_NONE; }
	int		KeyDown(int c)						{ return MP_WID_NONE; }
	int		KeyUp(int c)						{ return MP_WID_NONE; }

	void	Draw(SDL_Surface *bmpDest);

	void	LoadStyle(void) {}
	DWORD	SendMessage(int iMsg, DWORD Param1, DWORD Param2) {return 0;}
};

// Slider
class CPlayerSlider: public CWidget  {
public:
	CPlayerSlider(SDL_Surface *progress, SDL_Surface *start, SDL_Surface *end, SDL_Surface *background, int max);

private:
	SDL_Surface *bmpProgress;
	SDL_Surface *bmpStart;
	SDL_Surface *bmpEnd;
	SDL_Surface *bmpBackground;
	int			iValue;
	int			iMax;

public:
	// Methods
	void	Create(void) { }
	void	Destroy(void) { }

	//These events return an event id, otherwise they return -1
	int		MouseOver(mouse_t *tMouse)			{ return MP_WID_NONE; }
	int		MouseUp(mouse_t *tMouse, int nDown) { iCanLoseFocus = true; return MP_WID_NONE; }
	int		MouseDown(mouse_t *tMouse, int nDown);
	int		MouseWheelDown(mouse_t *tMouse)		{ return MP_WID_NONE; }
	int		MouseWheelUp(mouse_t *tMouse)		{ return MP_WID_NONE; }
	int		KeyDown(int c)						{ return MP_WID_NONE; }
	int		KeyUp(int c)						{ return MP_WID_NONE; }

	inline int GetValue(void)  { return iValue; }
	inline void SetValue(int _v) {iValue = _v; }

	inline int GetMax(void)  { return iMax; }
	inline void SetMax(int _m) {iMax = _m; }

	void	Draw(SDL_Surface *bmpDest);

	void	LoadStyle(void) {}
	DWORD	SendMessage(int iMsg, DWORD Param1, DWORD Param2) {return 0;}
};

// Toggle button
class CPlayerToggleBtn: public CWidget  {
public:
	CPlayerToggleBtn(SDL_Surface *image, bool enabled)  {
		if (!image)
			return;
		
		bEnabled = enabled;
		bmpImage = image;
	}

private:
	SDL_Surface *bmpImage;
	bool		bEnabled;
public:
	// Methods
	void	Create(void) { }
	void	Destroy(void) { }

	//These events return an event id, otherwise they return -1
	int		MouseOver(mouse_t *tMouse)			{ return MP_WID_NONE; }
	int		MouseUp(mouse_t *tMouse, int nDown);
	int		MouseDown(mouse_t *tMouse, int nDown) {return MP_WID_MOUSEDOWN; }
	int		MouseWheelDown(mouse_t *tMouse)		{ return MP_WID_NONE; }
	int		MouseWheelUp(mouse_t *tMouse)		{ return MP_WID_NONE; }
	int		KeyDown(int c)						{ return MP_WID_NONE; }
	int		KeyUp(int c)						{ return MP_WID_NONE; }

	void	Draw(SDL_Surface *bmpDest);
	
	inline bool isOn(void) { return bEnabled; }

	void	LoadStyle(void) {}
	DWORD	SendMessage(int iMsg, DWORD Param1, DWORD Param2) {return 0;}
};

// Marquee
#define MARQUEE_STEP  2
#define MARQUEE_TIME  0.10f
#define MARQUEE_ENDWAIT 0.2f
class CPlayerMarquee: public CWidget  {
public:
	CPlayerMarquee(const std::string& text, Uint32 col);

private:
	std::string szText;
	float	fTime;
	float	fEndWait;
	int		iFrame;
	Uint32	iColour;
	int		iDirection;  // 1 = right, -1 = left
	int		iTextWidth;
	SDL_Surface *bmpBuffer;

public:
	// Methods
	void	Create(void) { }
	void	Destroy(void) { 
		if (bmpBuffer)  {SDL_FreeSurface(bmpBuffer); bmpBuffer = NULL;} 
	}

	//These events return an event id, otherwise they return -1
	int		MouseOver(mouse_t *tMouse)			{ return MP_WID_NONE; }
	int		MouseUp(mouse_t *tMouse, int nDown) { return MP_WID_NONE; }
	int		MouseDown(mouse_t *tMouse, int nDown) {return MP_WID_NONE; }
	int		MouseWheelDown(mouse_t *tMouse)		{ return MP_WID_NONE; }
	int		MouseWheelUp(mouse_t *tMouse)		{ return MP_WID_NONE; }
	int		KeyDown(int c)						{ return MP_WID_NONE; }
	int		KeyUp(int c)						{ return MP_WID_NONE; }

	void	RedrawBuffer(void);
	void	Draw(SDL_Surface *bmpDest);

	inline Uint32 getColour(void)  { return iColour; }
	inline void	setColour(Uint32 _c)		{ iColour = _c; }

	inline std::string getText(void)	{ return szText; }
	inline void setText(const std::string& text)	{ szText = text; iFrame = 0; fTime=0; RedrawBuffer(); }

	void	LoadStyle(void) {}
	DWORD	SendMessage(int iMsg, DWORD Param1, DWORD Param2) {return 0;}
};



extern	CMediaPlayer	cMediaPlayer;


#endif  //  __MEDAIPLAYER_H__
