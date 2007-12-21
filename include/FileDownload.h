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
#include <set>
#include <stdio.h>  // for FILE
#include <SDL_thread.h>
#include <SDL_mutex.h>
#include "HTTP.h"
#include "types.h"
#include "CBytestream.h"


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
		tFile(NULL),
		tDownloadServers(NULL),
		iState(FILEDL_NO_FILE),
		iID(0),
		tMutex(NULL)
		{ tMutex = SDL_CreateMutex(); }

	CFileDownload(std::vector<std::string> *download_servers, size_t id) :
		tFile(NULL),
		tDownloadServers(download_servers),
		iState(FILEDL_NO_FILE),
		iID(id)
		{ tMutex = SDL_CreateMutex(); }

	~CFileDownload()  { Stop(); SDL_DestroyMutex(tMutex); }

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
	SDL_mutex					*tMutex;

public:
	void						StartFileDownload(const std::string& filename, const std::string& dest_dir);
	void						CancelFileDownload(const std::string& filename);
	bool						IsFileDownloaded(const std::string& filename);
	DownloadError				FileDownloadError(const std::string& filename);
	byte						GetFileProgress(const std::string& filename);
	std::list<CFileDownload>	*GetDownloads()		{ return &tDownloads; }
	bool						ShouldBreakThread();
	void						Lock()		{ SDL_LockMutex(tMutex); }
	void						Unlock()	{ SDL_UnlockMutex(tMutex); }
};

// In-lobby or in-game file downloader over unreliable protocol - send packets of 256 bytes
class CFileDownloaderInGame
{
public:
	CFileDownloaderInGame() { tState = S_FINISHED; iPos = 0; bAllowFileRequest = true; };
	~CFileDownloaderInGame() { };

	enum State_t 	{ S_SEND, S_RECEIVE, S_FINISHED, S_ERROR };

	// Basic functions for file download
	// Contains garbage when download not finished yet, or when uploading a file
	const std::string & getFilename() { return sFilename; };
	const std::string & getData() { return sData; };

	// Should be called when received S2C_SENDFILE or C2S_SENDFILE msg - read needed amount of bytes from bytestream
	// Returns true if download finished or error occured
	// If file request received automatically start sending requested file if bAllowFileRequest is true
	bool		receive( CBytestream * bs );
	// Should be called to append file data to S2C_SENDFILE or C2S_SENDFILE messages when sending in a loop
	// Returns true if download finished or error occured
	bool		send( CBytestream * bs );

	bool		errorOccured() { return (tState == S_ERROR); };
	State_t		getState() { return tState; };

	void		setFileToSend( const std::string & name, const std::string & data );
	void		setFileToSend( const std::string & path );

	void		reset() { iPos = 0; tState = S_FINISHED; sFilename = ""; sData = ""; };

	// Functions that will trigger remote CFileDownloaderInGame to do something like send some file or list some dir
	void		allowFileRequest( bool allow );
	void		requestFile( const std::string & path, bool retryIfFail = true ); // Same for dir
	void		requestFileInfo( const std::string & path ); // Same for dir
	bool		requestFilesPending(); // Re-send file request if downloading fails
	static bool	isPathValid( const std::string & path );	// Check if someone tries to access /etc/shadow to get system passwords

private:
	std::string		sFilename;
	std::string		sData;
	uint			iPos;

	State_t			tState;
	
	bool			bAllowFileRequest;
	
	std::set< std::string > tRequestedFiles;
	
	void			processFileRequests();

};

#endif // __FILEDOWNLOAD_H__
