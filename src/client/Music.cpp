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
#include <SDL_mutex.h>
#include "FindFile.h"
#include "Sounds.h"
#include "StringUtils.h"
#include "LieroX.h" // for bDedicated

bool breakThread = false;
ThreadPoolItem *musicThread = NULL;
SDL_cond *waitCond = NULL;
SDL_mutex *waitMutex = NULL;

// Load the playlist
class PlaylistFiller { public:
	std::set<std::string> *list;
	PlaylistFiller(std::set<std::string>* c) : list(c) {}
	bool operator() (const std::string& filename) {
		std::string ext = filename.substr(filename.rfind('.'));
		if (stringcaseequal(ext, ".mp3"))
			list->insert(filename);

		return true;
	}
};

///////////////////
// Called when a song finishes
void OnSongFinished()
{
	SDL_CondSignal(waitCond);
}

/////////////////////
// The player thread
int MusicMain(void *)
{
	std::set<std::string> playlist;	
	PlaylistFiller filler(&playlist);

	FindFiles(filler, "music", false, FM_REG);

	// Nothing to play, just quit
	if (!playlist.size())
		return 0;

	std::set<std::string>::iterator song = playlist.begin();
	SoundMusic *songHandle = NULL;

	while (!breakThread)  {
		// If not playing, start some song
		if (!PlayingMusic())  {
			// Free any previous song
			if (songHandle)
				FreeMusic(songHandle);

			// Load and skip to next one
			songHandle = LoadMusic(*song);
			song++;
			if (song == playlist.end())
				song = playlist.begin();

			// Could not open, move on to next one
			if (!songHandle)
				continue;

			// Play
			PlayMusic(songHandle);

			// Wait until the song finishes
			SDL_LockMutex(waitMutex);
			SDL_CondWait(waitCond, waitMutex);
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
	waitMutex = SDL_CreateMutex();
	waitCond = SDL_CreateCond();
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
		SDL_CondSignal(waitCond);
		threadPool->wait(musicThread, NULL);
		SDL_DestroyCond(waitCond);
		SDL_DestroyMutex(waitMutex);
	}
	breakThread = false;
	musicThread = NULL;
	waitCond = NULL;
	waitMutex = NULL;

	ShutdownMusic();
}

#else // DEDICATED_ONLY

// dummies
void InitializeBackgroundMusic() {}
void ShutdownBackgroundMusic() {}

#endif

