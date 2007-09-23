/////////////////////////////////////////
//
//   OpenLieroX
//
//   Auxiliary Software class library
//
//   based on the work of JasonB
//   enhanced by Dark Charlie and Albert Zeyer
//
//   code under LGPL
//
/////////////////////////////////////////


// File downloading over HTTP
// Created 13/10/07
// By Karel Petranek, Albert Zeyer and Martin Griffin


#ifndef __FILEDOWNLOAD_H__
#define __FILEDOWNLOAD_H__

#include <list>
#include <vector>
#include <string>
#include <stdio.h>  // for FILE
#include <SDL/SDL_thread.h>
#include <SDL/SDL_mutex.h>
#include "HTTP.h"

// File download states
enum {
	FILEDL_NO_FILE = -1,
	FILEDL_INITIALIZING = 0,
	FILEDL_RECEIVING,
	FILEDL_FINISHED,
	FILEDL_ERROR
};

// File download errors
// NOTE: if you change this, change also error strings in FileDownload.cpp
enum  {
	FILEDL_ERROR_NO_ERROR = 0,
	FILEDL_ERROR_NO_FILE,
	FILEDL_ERROR_NO_DEST,
	FILEDL_ERROR_NO_SERVER,
	FILEDL_ERROR_HTTP,
	FILEDL_ERROR_SAVING,
};

// File download error structure
class DownloadError  { public:
	std::string	sErrorMsg;
	int			iError;
	HttpError	tHttpError;
};



// Single file download handling class
class CFileDownload  {
public:
	// Constructors and destructors
	CFileDownload() : 
		iState(FILEDL_NO_FILE),
		tFile(NULL),
		tDownloadServers(NULL),
		iID(0)
		{ tMutex = SDL_CreateMutex(); }

	CFileDownload(std::vector<std::string> *download_servers, size_t id) :
		iState(FILEDL_NO_FILE),
		tFile(NULL),
		tMutex(NULL),
		tDownloadServers(download_servers),
		iID(id)
		{ tMutex = SDL_CreateMutex(); }

	~CFileDownload()  { Stop();/* SDL_DestroyMutex(tMutex);*/ }

private:
	std::string		sFileName;
	std::string		sDestPath;
	FILE			*tFile;
	std::vector<std::string> *tDownloadServers;
	int				iCurrentServer;
	int				iState;
	size_t			iID;
	CHttp			tHttp;
	DownloadError	tError;

	SDL_mutex		*tMutex;

private:
	void			SetHttpError(HttpError err);
	void			SetDlError(int id);

public:
	void				Start(const std::string& filename, const std::string& dest_dir);
	void				Stop();
	void				ProcessDownload();
	std::string			GetFileName();
	int					GetState();
	size_t				GetID()			{ return iID; }
	byte				GetProgress();
	DownloadError		GetError();

	// Thread safety
	void				Lock()			{ SDL_LockMutex(tMutex); }
	void				Unlock()		{ SDL_UnlockMutex(tMutex); }
};


// File downloader class
class CFileDownloader  {
public:
	CFileDownloader();
	~CFileDownloader();

private:
	std::list<CFileDownload>	tDownloads;
	std::vector<std::string>	tDownloadServers;
	SDL_Thread					*tThread;
	bool						bBreakThread;

public:
	void						StartFileDownload(const std::string& filename, const std::string& dest_dir);
	void						CancelFileDownload(const std::string& filename);
	bool						IsFileDownloaded(const std::string& filename);
	DownloadError				FileDownloadError(const std::string& filename);
	byte						GetFileProgress(const std::string& filename);
	std::list<CFileDownload>	*GetDownloads()		{ return &tDownloads; }
	bool						ShouldBreakThread()	{ return bBreakThread; }
};


#endif // __FILEDOWNLOAD_H__