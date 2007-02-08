// OpenLieroX Media Player
// Made by Dark Charlie and Albert Zeyer

#include "defs.h"
#include "LieroX.h"
#include "CMediaPlayer.h"

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
void CPlayList::Load(const char *dir, bool include_subidrs)
{
	if (!dir)
		return;

	// Clear me first
	Clear();

	// TODO
}

///////////////////
// Get the current played song
char *CPlayList::GetCurSong(void)
{
	static char result[256] = "";

	if (iCurSong > tSongList.size()-1)
		iCurSong = tSongList.size()-1;
	if (iCurSong < 1) {
		iCurSong = 0;
		return result;
	}

	fix_strncpy(result,tSongList[iCurSong]);

	return result;
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
	szCurSongName[0] = '\0';
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
void CMediaPlayer::GetNameFromFile(char *name,size_t buflen,const char *path)
{
	if (!path || !name)
		return;


	// Remove directory
	char *tmp = strrchr(path,'\\');
	if (tmp)  {
		strncpy(name,tmp+1,buflen-1);
	} else {
		strncpy(name,path,buflen-1);
	}

	// Remove extension
	tmp = strrchr(name,'.');
	if (tmp)  {
		*tmp = '\0';
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
		fix_strncpy(szCurSongName,tPlayList.GetCurSong());
		if (szCurSongName[0])  {
			FreeMusic(tCurrentSong);  // Free the previous song (if any)
			tCurrentSong = LoadMusic(szCurSongName);
			if (tCurrentSong)  {
				PlayMusic(tCurrentSong);
			}
			GetNameFromFile(szCurSongName,sizeof(szCurSongName),tPlayList.GetCurSong());
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
	if (!instance->GetSongStopped())
		instance->Forward();
}


