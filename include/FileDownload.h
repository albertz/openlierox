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
#include <map>
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


// TODO: what is the difference between CFileDownload and CFileDownloader


// Single file download handling class
class CFileDownload  {
public:
	// Constructors and destructors
	CFileDownload() : 
		tFile(NULL),
		tDownloadServers(NULL),
		iState(FILEDL_NO_FILE),
		iID(0)
		{ }

	CFileDownload(std::vector<std::string> *download_servers, size_t id) :
		tFile(NULL),
		tDownloadServers(download_servers),
		iState(FILEDL_NO_FILE),
		iID(id)
		{ }

	~CFileDownload()  { Stop();}

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

private:
	void			SetHttpError(HttpError err);
	void			SetDlError(int id);

public:
	void				Start(const std::string& filename, const std::string& dest_dir);
	void				Stop();
	void				ProcessDownload();
	std::string			GetFileName()	{ return sFileName; }
	int					GetState()		{ return iState; }
	size_t				GetID()			{ return iID; }
	byte				GetProgress();
	DownloadError		GetError()		{ return tError; }
};


// File downloader class
class CFileDownloader  {
public:
	CFileDownloader();
	~CFileDownloader();

private:
	std::list<CFileDownload>	tDownloads;
	std::vector<std::string>	tDownloadServers;

public:
	void						ProcessDownloads();
	void						StartFileDownload(const std::string& filename, const std::string& dest_dir);
	void						CancelFileDownload(const std::string& filename);
	bool						IsFileDownloaded(const std::string& filename);
	DownloadError				FileDownloadError(const std::string& filename);
	byte						GetFileProgress(const std::string& filename);
	std::list<CFileDownload>	*GetDownloads()		{ return &tDownloads; }
};

// In-lobby or in-game file downloader over unreliable protocol - send packets of 256 bytes
class CFileDownloaderInGame
{
public:
	CFileDownloaderInGame() { reset(); };
	~CFileDownloaderInGame() { };

	enum State_t 	{ S_SEND, S_RECEIVE, S_FINISHED, S_ERROR };

	// Basic functions for file download
	// Contains garbage when download not finished yet, or when uploading a file
	const std::string & getFilename() const { return sFilename; };
	const std::string & getData() const { return sData; } ;

	// Should be called when received S2C_SENDFILE or C2S_SENDFILE msg - read needed amount of bytes from bytestream
	// Returns true if download finished or error occured
	// If file request received automatically start sending requested file if bAllowFileRequest is true
	bool		receive( CBytestream * bs );
	// Should be called to append file data to S2C_SENDFILE or C2S_SENDFILE messages when sending in a loop
	// Returns true if download finished or error occured
	bool		send( CBytestream * bs );

	// Does not change any variables, just pings server with zero-sized packet to un-freeze it, else download speed sucks
	void		sendPing( CBytestream * bs ) const;

	bool		errorOccured() const { return (tState == S_ERROR); };
	State_t		getState() const { return tState; };

	void		setDataToSend( const std::string & name, const std::string & data, bool noCompress = false );
	void		setFileToSend( const std::string & path );

	void		reset();

	// Functions that will trigger remote CFileDownloaderInGame to do something like send some file or list some dir
	void		allowFileRequest( bool allow );
	void		requestFile( const std::string & path, bool retryIfFail = true ); // Same for dir
	bool		requestFilesPending(); // Re-send file request if downloading fails
	static bool	isPathValid( const std::string & path );	// Check if someone tries to access /etc/shadow to get system passwords
	
	struct		StatInfo
	{
		std::string filename;
		uint size;
		uint compressedSize;	// Approximate! Use only for progressbars
		uint checksum;
		StatInfo( const std::string & _filename="", uint _size=0, uint _compressedSize=0, uint _checksum=0 ):
			filename(_filename), size(_size), compressedSize(_compressedSize), checksum(_checksum) {};
	};
	
	// For dir returns recursive list of all files in dir, clears previous file info
	void		requestFileInfo( const std::string & path ); 
	// File statistics from requestFileInfo() is saved in array returned by this func
	const std::vector< StatInfo > & getFileInfo() const { return cStatInfo; };
	
	// Additional functionality to show download progress - inexact, server may send any file and we should accept it,
	// yet current implementation will send only files we will request
	std::string getFileDownloading() const;
	float getFileDownloadingProgress() const;

private:
	void			processFileRequests();

	// TODO: should use intern-pointer here
	std::string		sFilename;
	std::string		sData;
	uint			iPos;

	State_t			tState;
	
	bool			bAllowFileRequest;
	
	std::vector< std::string > tRequestedFiles;
	
	std::vector< StatInfo > cStatInfo;
	
	// For progressbar and download percentage
	std::map< std::string, StatInfo > cStatInfoCache;	// TODO: Never cleared
	std::string sLastFileRequested;

};

#endif // __FILEDOWNLOAD_H__
