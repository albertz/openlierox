// OpenLieroX Media Player
// Made by Dark Charlie and Albert Zeyer
// code under LGPL

#include "defs.h"
#include "LieroX.h"
#include "CMediaPlayer.h"


CMediaPlayer* CMediaPlayer::instance = NULL;


//////////////////
// Clears and shutdowns the playlist
void CPlayList::Clear(void)
{
	tSongList.clear();
	iCurSong = 0;
	bRepeat = false;
}

//////////////////
// Loads the directory and adds all music files in the playlist
void CPlayList::Load(const std::string dir, bool include_subidrs)
{
	// Clear me first
	Clear();

	// TODO
}

///////////////////
// Get the current played song
std::string CPlayList::GetCurSong(void)
{
	if (iCurSong > tSongList.size()-1)
		iCurSong = tSongList.size()-1;
	if (iCurSong < 0)
		iCurSong = 0;

	return tSongList[iCurSong];
}

//////////////////
// Move to the next song in playlist
void CPlayList::GoToNextSong(void)
{
	iCurSong++;
	if (iCurSong > tSongList.size()-1)  {
		if (bRepeat)
			iCurSong = 0;
		else
			iCurSong = tSongList.size()-1;
	}
}



/////////////////////
// Clears the media player
void CMediaPlayer::Clear(void)
{
	tPlayList.Clear();
	szCurSongName = "";
	SetFinishedHook(OnSongFinished);
	FreeMusic(tCurrentSong);
	tCurrentSong = NULL;
}

/////////////////////
// Shutdowns the media player
void CMediaPlayer::Shutdown(void)
{
	Clear();
	SetFinishedHook(NULL);
}

/////////////////////
// Get the song name from the path
// TODO: use ID3 tags for MP3
std::string CMediaPlayer::GetNameFromFile(const std::string path)
{
	size_t slash, slash2;
	slash = path.rfind('/'); if(slash == path.npos) slash = -1;
	slash2 = path.rfind('\\'); if(slash2 == path.npos) slash2 = -1;
	slash = MAX(slash, slash2) + 1;

	return path.substr(slash);
}

////////////////////
// Starts playing
void CMediaPlayer::Play(void)
{
	// If paused, just resume
	if (PausedMusic())
		ResumeMusic();
	else  {
		szCurSongName = tPlayList.GetCurSong();
		if(szCurSongName.size())  {
			FreeMusic(tCurrentSong);  // Free the previous song (if any)
			tCurrentSong = LoadMusic(szCurSongName.c_str());
			if (tCurrentSong)  {
				PlayMusic(tCurrentSong);
			}
			szCurSongName = GetNameFromFile(tPlayList.GetCurSong());
		} else {
			// Nothing to play
			Stop();
		}
	}
}

////////////////////
// Pauses or resumes the current song
void CMediaPlayer::PauseResume(void)
{
	if (PlayingMusic())
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
	return GetSongTime();
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
// Event that fires when a song finishes or is stopped
void CMediaPlayer::OnSongFinished(void)
{
	if (!GetSongStopped())
		instance->Forward();
}


