// OpenLieroX Media Player
// Made by Dark Charlie and Albert Zeyer
// code under LGPL

#ifndef __MEDIAPLAYER_H__
#define __MEDIAPLAYER_H__

#include <vector>
typedef std::string song_path;
typedef std::vector<song_path> song_list;

class CPlayList {
public:
	CPlayList() {
		Clear();
	}
private:
	song_list	tSongList;
	int			iCurSong;
	bool		bRepeat;
public:
	void	Clear(void);
	void	Load(const std::string dir,bool include_subdirs=true);
	std::string GetCurSong(void);
	void	GoToNextSong(void);
	void	setRepeat(bool _r) {bRepeat = true; }
	bool	getRepeat(void)		{return bRepeat; }
};




class CMediaPlayer {
private:
	static CMediaPlayer* instance;
public:
	// Constructor
	CMediaPlayer() {
		if(instance) printf("WARNING: more than 1 instance of CMediaPlayer\n");
		instance = this;
		Clear();
	}

	~CMediaPlayer() {
		instance = NULL;
	}

private:
	// Attributes
	std::string	szCurSongName;
	SoundMusic	*tCurrentSong;
	CPlayList	tPlayList;
	

public:
	 void	Clear(void);
	 void	Shutdown(void);
	 std::string	GetNameFromFile(const std::string path);
	 void	Play(void);
	 void	PauseResume(void);
	 void	Stop(void);
	 void	Rewind(void);
	 void	Forward(void);
	 void	SetSongPosition(double pos);
	 float	GetSongTime(void);
	 void	SetVolume(byte _v);
	 byte	GetVolume(void);

	 static void	OnSongFinished(void);
};







#endif  //  __MEDAIPLAYER_H__
