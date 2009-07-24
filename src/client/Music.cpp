/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// TODO: this file was really created by Jason?
// Background music handling
// Created 29/7/02
// Jason Boettcher

#include "Music.h"

#ifndef DEDICATED_ONLY

#include <set>
#include "ThreadPool.h"
#include "FindFile.h"
#include "Sounds.h"
#include "StringUtils.h"
#include "LieroX.h" // for bDedicated
#include "Condition.h"
#include "Mutex.h"

static bool breakThread = false;
static ThreadPoolItem *musicThread = NULL;
static Condition waitCond;
static Mutex waitMutex;


///////////////////
// Called when a song finishes
void OnSongFinished()
{
	waitCond.broadcast();
}

// Load the playlist
struct PlaylistFiller {
	std::set<std::string> list;

	bool operator() (const std::string& filename) {
		std::string ext = GetFileExtension(filename);
		if (stringcaseequal(ext, ".mp3"))
			list.insert(filename);

		return true;
	}
};

/////////////////////
// The player thread
int MusicMain(void *)
{
	PlaylistFiller playlist;
	FindFiles(playlist, "music", false, FM_REG);

	// Nothing to play, just quit
	if (!playlist.list.size())
		return 0;

	std::set<std::string>::iterator song = playlist.list.begin();
	SoundMusic *songHandle = NULL;

	{
		Mutex::ScopedLock lock(waitMutex);
		while (!breakThread)  {

			// If not playing, start some song
			if (!PlayingMusic())  {
				// Free any previous song
				if (songHandle)
					FreeMusic(songHandle);

				// Load and skip to next one
				songHandle = LoadMusic(*song);
				song++;
				if (song == playlist.list.end())
					song = playlist.list.begin();

				// Could not open, move on to next one
				if (!songHandle)
					continue;

				// Play
				PlayMusic(songHandle);
			}
			
			// Wait until the song finishes
			waitCond.wait(waitMutex);
		}
	}
	
	// Stop/free the playing song
	if (songHandle)
		FreeMusic(songHandle);

	return 0;
}

//////////////////////
// Initializes the background music thread
void InitializeBackgroundMusic()
{
	if(bDedicated) return;
	if(bDisableSound) return; // sound system failed to init
	if(musicThread) return; // already running
	if(!tLXOptions->bMusicOn) return;  // music disabled
	
	InitializeMusic();

	musicThread = threadPool->start(&MusicMain, NULL, "music player");
	SetMusicFinishedHandler(&OnSongFinished);
}

///////////////////////
// Shuts down the background music playing
void ShutdownBackgroundMusic()
{
	if( ! musicThread )
		return;
		
	SetMusicFinishedHandler(NULL);
	breakThread = true;
	if (musicThread)  {
		{
			Mutex::ScopedLock lock(waitMutex); // ensure that we are in waiting state in music thread
			waitCond.broadcast();
		}
		threadPool->wait(musicThread, NULL);
	}
	breakThread = false;
	musicThread = NULL;

	ShutdownMusic();
}

#else // DEDICATED_ONLY

// dummies
void InitializeBackgroundMusic() {}
void ShutdownBackgroundMusic() {}

#endif

