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

#ifdef _MSC_VER
// TODO: can't you put this into your IDE settings? it does not belong here
#pragma warning(disable: 4786)  // WARNING: identifier XXX was truncated to 255 characters in the debug info
#pragma warning(disable: 4503)  // WARNING: decorated name length exceeded, name was truncated
#endif

#include <iostream>
#include "StringUtils.h"
#include "FindFile.h"
#include "EndianSwap.h"
#include "FileDownload.h"

using namespace std;

//
// Download error strings
//
const std::string sDownloadErrors[] =  {
	"No error",
	"No file specified",
	"No destination specified",
	"No servers available",
	"HTTP error",
	"Error while saving"
};

//
// Single file download
//

//////////////////
// Start the transfer
void CHttpDownloader::Start(const std::string& filename, const std::string& dest_dir)
{
	// Checks
	if (filename.size() == 0)  {
		SetDlError(FILEDL_ERROR_NO_FILE);
		return;
	}

	if (dest_dir.size() == 0)  {
		SetDlError(FILEDL_ERROR_NO_DEST);
		return;
	}
		
	if (tDownloadServers == NULL)  {
		SetDlError(FILEDL_ERROR_NO_SERVER);
		return;
	}

	if (tDownloadServers->empty())  {
		SetDlError(FILEDL_ERROR_NO_SERVER);
		return;
	}

	// Stop & clear any previous download
	Stop();

	// Fill in the info
	sFileName = filename;
	if (*dest_dir.rbegin() != '/' && *dest_dir.rbegin() != '\\')
		sDestPath = dest_dir + "/" + filename;
	else
		sDestPath = dest_dir + filename;

	// Create the file
	tFile = OpenGameFile(sDestPath, "wb");
	if (tFile == NULL)  {
		SetDlError(FILEDL_ERROR_SAVING);
		return;
	}

	// Try to download from first server in the list
	iCurrentServer = 0;
	tHttp.RequestData((*tDownloadServers)[iCurrentServer] + filename);

	// Set the state to initializing
	iState = FILEDL_INITIALIZING;
}

/////////////////
// Stop the transfer and clear everything
void CHttpDownloader::Stop()
{
	// Cancel the transfer
	if (tHttp.RequestedData())
		tHttp.CancelProcessing();

	// Clear
	if (tFile)  {
		fclose(tFile);
		remove(GetFullFileName(sDestPath).c_str());
	}
	sFileName = "";
	sDestPath = "";
	tFile = NULL;
	iCurrentServer = 0;
	iState = FILEDL_NO_FILE;

	SetDlError(FILEDL_ERROR_NO_ERROR);
}

/////////////////
// Process the downloading
void CHttpDownloader::ProcessDownload()
{
	// Check that there has been no error yet
	if (tError.iError != FILEDL_ERROR_NO_ERROR)  {
		return;
	}

	// Process the HTTP
	int res = tHttp.ProcessRequest();

	switch (res)  {
	// Still processing
	case HTTP_PROC_PROCESSING:
		iState = FILEDL_RECEIVING;

		// If we already received something, put it in the file
		if (tHttp.GetData().length() > 0 && tFile)  {
			if (fwrite(tHttp.GetData().data(), tHttp.GetData().length(), 1, tFile) == 1)
				tHttp.ClearReceivedData(); // Save memory
		}
		break;

	// Error
	case HTTP_PROC_ERROR:
		// If the file could not be found, try another server
		if (tHttp.GetError().iError == HTTP_FILE_NOT_FOUND)  {
			iCurrentServer++;
			if ((size_t)iCurrentServer < tDownloadServers->size())  {
				tHttp.RequestData((*tDownloadServers)[iCurrentServer] + sFileName);  // Request the file
				iState = FILEDL_INITIALIZING;
				break;  // Don't set the error
			}
		}

		SetHttpError(tHttp.GetError());
		iState = FILEDL_ERROR;

		// Delete the file
		if (tFile)  {
			fclose(tFile);
			remove(GetFullFileName(sDestPath).c_str());
		}

		break;

	// Finished
	case HTTP_PROC_FINISHED:
		iState = FILEDL_FINISHED;

		// Save the data in the file
		if (tFile)  {
			if (fwrite(tHttp.GetData().data(), tHttp.GetData().size(), 1, tFile) != 1)
				SetDlError(FILEDL_ERROR_SAVING);
			fclose(tFile);
			tFile = NULL;
		} else {
			SetDlError(FILEDL_ERROR_SAVING);
		}

		break;
	}

}

/////////////////
// Set downloading error
void CHttpDownloader::SetDlError(int id)
{
	tError.iError = id;
	tError.sErrorMsg = sDownloadErrors[id];
	tError.tHttpError = tHttp.GetError();

	if (id != FILEDL_ERROR_NO_ERROR)
		std::cout << "HTTP Download Error: " << tError.sErrorMsg << std::endl;
}

/////////////////
// Set downloading error (HTTP problem)
void CHttpDownloader::SetHttpError(HttpError err)
{
	tError.iError = FILEDL_ERROR_HTTP;
	tError.sErrorMsg = "HTTP Error: " + err.sErrorMsg;
	tError.tHttpError.iError = err.iError;
	tError.tHttpError.sErrorMsg = err.sErrorMsg;

	if (err.iError != HTTP_NO_ERROR)
		std::cout << "HTTP Download Error: " << tError.sErrorMsg << std::endl;
}

////////////////
// Get the file downloading progress
byte CHttpDownloader::GetProgress()
{
	byte res = 0;
	if (tHttp.GetDataLength() != 0)
		res = (byte)MIN(100, tHttp.GetReceivedDataLen() * 100 / tHttp.GetDataLength());

	return res;
}

//
// File downloader
//

/////////////////
// Constructor
CHttpDownloadManager::CHttpDownloadManager()
{
	// Load the list of available servers
	FILE *fp = OpenGameFile("cfg/downloadservers.txt", "r");
	if (fp)  {
		while (!feof(fp))  {
			std::string server = ReadUntil(fp, '\n');

			// Check (server with less than 3 characters cannot be valid)
			if (server.size() < 3)
				continue;

			// Delete CR character if present
			if (*server.rbegin() == '\r')
				server.erase(server.size() - 1);

			// Trim & add the server
			TrimSpaces(server);
			tDownloadServers.push_back(server);

			// Skip the LF character
			fseek(fp, ftell(fp)+1, SEEK_SET);
		}
	}
}

///////////////
// Destructor
CHttpDownloadManager::~CHttpDownloadManager()
{
	// Stop the downloads
	for (std::list<CHttpDownloader *>::iterator i = tDownloads.begin(); i != tDownloads.end(); i++)  {
		(*i)->Stop();
		delete (*i);
	}
}

//////////////
// Add and start a download
void CHttpDownloadManager::StartFileDownload(const std::string& filename, const std::string& dest_dir)
{
	// Add and start the download
	CHttpDownloader *new_dl = new CHttpDownloader(&tDownloadServers, tDownloads.size());
	new_dl->Start(filename, dest_dir);

	tDownloads.push_back(new_dl);
}

/////////////
// Cancel a download
void CHttpDownloadManager::CancelFileDownload(const std::string& filename)
{
	// Find the download and remove it, the destructor will stop the download automatically
	for (std::list<CHttpDownloader *>::iterator i = tDownloads.begin(); i != tDownloads.end(); i++)
		if ((*i)->GetFileName() == filename)  {
			(*i)->Stop();
			delete (*i);
			tDownloads.erase(i);
			break;
		}
}

//////////////
// Returns true if the file has been successfully downloaded
bool CHttpDownloadManager::IsFileDownloaded(const std::string& filename)
{
	for (std::list<CHttpDownloader *>::iterator i = tDownloads.begin(); i != tDownloads.end(); i++)
		if ((*i)->GetFileName() == filename)
			return (*i)->GetState() == FILEDL_FINISHED;

	return false;
}

//////////////
// Returns the download error for the specified file
DownloadError CHttpDownloadManager::FileDownloadError(const std::string& filename)
{
	for (std::list<CHttpDownloader *>::iterator i = tDownloads.begin(); i != tDownloads.end(); i++)
		if ((*i)->GetFileName() == filename)
			return (*i)->GetError();

	// Not found, create a default "no error"
	DownloadError err;
	err.iError = FILEDL_ERROR_NO_ERROR;
	err.sErrorMsg = sDownloadErrors[FILEDL_ERROR_NO_ERROR];
	err.tHttpError.iError = HTTP_NO_ERROR;
	err.tHttpError.sErrorMsg = "No error";
	return err;
}

///////////////
// Get the file download progress in percents
byte CHttpDownloadManager::GetFileProgress(const std::string& filename)
{
	for (std::list<CHttpDownloader *>::iterator i = tDownloads.begin(); i != tDownloads.end(); i++)
		if ((*i)->GetFileName() == filename)
			return (*i)->GetProgress();

	return 0;
}

void CHttpDownloadManager::ProcessDownloads()
{
	// Process the downloads
	for (std::list<CHttpDownloader *>::iterator i = tDownloads.begin(); i != tDownloads.end(); i++)  {
		if ((*i)->GetState() == FILEDL_INITIALIZING || (*i)->GetState() == FILEDL_RECEIVING)
			(*i)->ProcessDownload();
	}
}


// TODO: more comments please
// TODO: rename the class, it's not only downloading but also uploading
// It might be also used not only in game in the future
// Valid name will be CFileDownloaderUdp, since it's packet oriented, comments will appear someday (I hope).

void CUdpFileDownloader::reset()
{
	iPos = 0;
	tState = S_FINISHED;
	sFilename = "";
	sData = "";
	sLastFileRequested = "";
	tRequestedFiles.clear();
	bWasAborted = false;
	bWasError = false;
};

void CUdpFileDownloader::setDataToSend( const std::string & name, const std::string & data, bool noCompress )
{
	if( name == "" )
	{
		cout << "CUdpFileDownloader::setDataToSend() empty file name" << endl;
		reset();
		bWasError = true;
		return;
	};
	tState = S_SEND;
	iPos = 0;
	sFilename = name;
	std::string data1 = sFilename;
	data1.append( 1, '\0' );
	data1.append(data);
	Compress( data1, &sData, noCompress );
	cout << "CFileDownloaderInGame::setDataToSend() filename " << sFilename << " data.size() " << data.size() << " compressed " << sData.size() << endl;
};

void CUdpFileDownloader::setFileToSend( const std::string & path )
{
	FILE * ff = OpenGameFile( path, "rb" );
	if( ff == NULL )
	{
		reset();
		bWasError = true;
		return;
	};
	char buf[16384];
	std::string data = "";
	
	while( ! feof( ff ) )
	{
		int readed = fread( buf, 1, sizeof(buf), ff );
		data.append( buf, readed );
	};
	fclose( ff );
	
	bool noCompress = false;
	if( stringcaserfind( path, ".png" ) != std::string::npos ||
		stringcaserfind( path, ".lxl" ) != std::string::npos || // LieroX levels are in .png format
		stringcaserfind( path, ".ogg" ) != std::string::npos ||
		stringcaserfind( path, ".mp3" ) != std::string::npos )
		noCompress = true;
	setDataToSend( path, data, noCompress );
};

enum { MAX_DATA_CHUNK = 254 };	// UCHAR_MAX - 1, client and server should have this equal
bool CUdpFileDownloader::receive( CBytestream * bs )
{
	uint chunkSize = bs->readByte();
	if( chunkSize == 0 )	// Ping packet with zero data - do not change downloader state
		return false;
	if( tState == S_FINISHED )
	{
		tState = S_RECEIVE;
		iPos = 0;
		sFilename = "";
		sData = "";
	};
	bool Finished = false;
	if( chunkSize != MAX_DATA_CHUNK )
	{
		Finished = true;
		if( chunkSize > MAX_DATA_CHUNK )
			chunkSize = MAX_DATA_CHUNK;
	}
	if( tState != S_RECEIVE )
	{
		reset();
		bWasError = true;
		std::string data, unpacked;
		data.append( bs->readData(chunkSize) );
		if( Decompress( data, &unpacked ) )
			if( unpacked.find( "ABORT:" ) == 0 )
			{
				cout << "CFileDownloaderInGame::receive() - abort received" << endl;
				bWasAborted = true;
			};
		return true;	// Receive finished (due to error)
	};
	//cout << "CFileDownloaderInGame::receive() chunk " << chunkSize << endl;
	sData.append( bs->readData(chunkSize) );
	if( Finished )
	{
		tState = S_FINISHED;
		iPos = 0;
		bool error = true;
		int compressedSize = sData.size();
		if( Decompress( sData, &sFilename ) )
		{
			error = false;
			std::string::size_type f = sFilename.find('\0');
			if( f == std::string::npos )
				error = true;
			else
			{
				sData.assign( sFilename, f+1, sFilename.size() - (f+1) );
				sFilename.resize( f );
				cout << "CFileDownloaderInGame::receive() filename " << sFilename << " data.size() " << sData.size() << " compressed " << compressedSize << endl;
			};
		};
		if( error )
			reset();
		bWasError = error;
		processFileRequests();
		return true;	// Receive finished
	};
	return false;
};

bool CUdpFileDownloader::send( CBytestream * bs )
{
	// TODO: more safety here!
	// The transferred file can be corrupted if some of the packets  gets lost,
	// create some sequece checking here
	// Don't worry about safety, we send everything zipped and it has checksum attached, missed packets -> wrong checksum.
	if( tState != S_SEND )
	{
		reset();
		bWasError = true;
		return true;	// Send finished (due to error)
	};
	uint chunkSize = MIN( sData.size() - iPos, MAX_DATA_CHUNK );
	if( sData.size() - iPos == MAX_DATA_CHUNK )
		chunkSize++;
	bs->writeByte( chunkSize );
	bs->writeData( sData.substr( iPos, MIN( chunkSize, MAX_DATA_CHUNK ) ) );
	iPos += chunkSize;
	//cout << "CFileDownloaderInGame::send() " << iPos << "/" << sData.size() << endl;
	if( chunkSize != MAX_DATA_CHUNK )
	{
		tState = S_FINISHED;
		iPos = 0;
		return true;	// Send finished
	};
	return false;
};

void CUdpFileDownloader::sendPing( CBytestream * bs ) const
{
	bs->writeByte( 0 );
};

void CUdpFileDownloader::allowFileRequest( bool allow ) 
{ 
	if( ! bAllowFileRequest )
		reset();	// Stop uploading any files
	bAllowFileRequest = allow;
};

void CUdpFileDownloader::requestFile( const std::string & path, bool retryIfFail )
{
	setDataToSend( "GET:", path, false );
	if( retryIfFail )
	{ 
		bool exist = false;
		for( uint f = 0; f < tRequestedFiles.size(); f++ )
			if( tRequestedFiles[f] == path )
				exist = true;
		if( ! exist )
			tRequestedFiles.push_back( path );
	};
	sLastFileRequested = path;
};

bool CUdpFileDownloader::requestFilesPending()
{
	if( tRequestedFiles.empty() )
		return false;
	if( ! isFinished() )
		return true;	// Receiving or sending in progress
	
	if( tRequestedFiles.back().find("STAT:") == 0 )
		requestFileInfo( tRequestedFiles.back().substr( strlen("STAT:") ), false );
	else
		requestFile( tRequestedFiles.back(), false ); // May modify tRequestedFiles array
	return true;
};

void CUdpFileDownloader::requestFileInfo( const std::string & path, bool retryIfFail )
{
	cStatInfo.clear();
	setDataToSend( "STAT:", path );
	if( retryIfFail )
	{ 
		bool exist = false;
		for( uint f = 0; f < tRequestedFiles.size(); f++ )
			if( tRequestedFiles[f] == "STAT:" + path )
				exist = true;
		if( ! exist )
			tRequestedFiles.push_back( "STAT:" + path );
	};
	sLastFileRequested = path;
};

void CUdpFileDownloader::abortDownload()
{
	reset();
	setDataToSend( "ABORT:", "" );
};

std::string getStatPacketOneFile( const std::string & path );
std::string getStatPacketRecursive( const std::string & path );

void CUdpFileDownloader::processFileRequests()
{
	// Process received files
	for( std::vector< std::string > :: iterator it = tRequestedFiles.begin(); 
			it != tRequestedFiles.end(); )
	{
		bool erase = false;
		if( * it == getFilename() )
			erase = true;
		if( it->find("STAT:") == 0 && getFilename() == "STAT_ACK:" )
		{
			if( getData().find( it->substr(strlen("STAT:")) ) == 0 )
				erase = true;
		}
		if( erase )
		{
			tRequestedFiles.erase( it );
			it = tRequestedFiles.begin(); 
		}
 		else
			it ++ ;
	};
	
	// Process file sending requests
	if( ! bAllowFileRequest )
		return;
	if( getFilename() == "GET:" )
	{
		if( ! isPathValid( getData() ) )
		{
			cout << "CFileDownloaderInGame::processFileRequests(): invalid filename "<< getData() << endl;
			return;
		};
		struct stat st;
		if( ! StatFile( getData(), &st ) )
		{
			cout << "CFileDownloaderInGame::processFileRequests(): cannot stat file " << getData() << endl;
			return;
		};
		if( S_ISREG( st.st_mode ) )
		{
			setFileToSend( getData() );
			return;
		};
		if( S_ISDIR( st.st_mode ) )
		{
			cout << "CFileDownloaderInGame::processFileRequests(): cannot send dir (wrong request): " << getData() << endl;
			return;
		};
	};
	if( getFilename() == "STAT:" )
	{
		if( ! isPathValid( getData() ) )
		{
			cout << "CFileDownloaderInGame::processFileRequests(): invalid filename " << getData() << endl;
			return;
		};
		setDataToSend( "STAT_ACK:", getStatPacketRecursive( getData() ) );
		return;
	};
	if( getFilename() == "STAT_ACK:" )
	{
		for( std::string::size_type f = 0, f1 = 0; f < getData().size(); )
		{
			f1 = getData().find( '\0', f );
			if( f1 == f || f1 == std::string::npos || getData().size() < f1 + 13 ) // '\0' + 3 * sizeof(uint)
			{
				reset();
				bWasError = true;
				return;
			};
			std::string filename = getData().substr( f, f1 - f );
			f1 ++;
			Uint32 size=0, compressedSize=0, checksum=0;
			memcpy( &size, getData().data() + f1, 4 );
			f1 += 4;
			memcpy( &compressedSize, getData().data() + f1, 4 );
			f1 += 4;
			memcpy( &checksum, getData().data() + f1, 4 );
			f1 += 4;
			EndianSwap( checksum );
			EndianSwap( size );
			EndianSwap( compressedSize );
			cStatInfo.push_back( StatInfo( filename, size, compressedSize, checksum ) );
			f = f1;
		};
		for( uint ff = 0; ff < cStatInfo.size(); ff++ )
		{
			cStatInfoCache[ cStatInfo[ff].filename ] = cStatInfo[ff];
		};
	};
	if( getFilename() == "ABORT:" )
	{
		cout << "CFileDownloaderInGame::processFileRequests() - abort received" << endl;
		// Server sent us ABORT instead of file - abort only failed request (the last one), leave other requests
		if( ! tRequestedFiles.empty() )
			tRequestedFiles.pop_back();
		iPos = 0;
		tState = S_FINISHED;
		sFilename = "";
		sData = "";
		sLastFileRequested = "";
		bWasError = true;
		bWasAborted = true;
	};
};

// Valid filename symbols
// TODO: don't hardcode this, better check if the path exists/can be created
// If path already exists is checked outside of this class, 
// this func checks if someone tries to access things outside OLX dir or use forbidden symbols for some filesystems.
#define S_LETTER_UPPER "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define S_LETTER_LOWER "abcdefghijklmnopqrstuvwxyz"
#define S_LETTER S_LETTER_UPPER S_LETTER_LOWER
#define S_NUMBER "0123456789"
#define S_SYMBOL "/. -_&+"	// No "\\" symbol, no tab.
// Characters 128-255 - valid UTF8 char will consist only of these ones
#define S_UTF8_SYMBOL "\128\129\130\131\132\133\134\135\136\137\138\139\140\141\142\143\144\145\146\147\148\149\150\151\152\153\154\155\156\157\158\159\160\161\162\163\164\165\166\167\168\169\170\171\172\173\174\175\176\177\178\179\180\181\182\183\184\185\186\187\188\189\190\191\192\193\194\195\196\197\198\199\200\201\202\203\204\205\206\207\208\209\210\211\212\213\214\215\216\217\218\219\220\221\222\223\224\225\226\227\228\229\230\231\232\233\234\235\236\237\238\239\240\241\242\243\244\245\246\247\248\249\250\251\252\253\254\255"
const char * invalid_file_names [] = { "CON", "PRN", "AUX", "NUL", 
	"COM1", "COM2", "COM3", "COM4", "COM5", "COM6", "COM7", "COM8", "COM9", 
	"LPT1", "LPT2", "LPT3", "LPT4", "LPT5", "LPT6", "LPT7", "LPT8", "LPT9" };

bool CUdpFileDownloader::isPathValid( const std::string & path )
{
	if( path == "" )
		return false;
	if( path.find_first_not_of( S_LETTER S_NUMBER S_SYMBOL S_UTF8_SYMBOL ) !=	std::string::npos )
		return false;
	if( path[0] == '/' || path[0] == ' ' )
		return false;
	if( path[path.size()-1] == ' ' )
		return false;
	if( path.find( ".." ) != std::string::npos ||
		path.find( "//" ) != std::string::npos ||
		path.find( "./" ) != std::string::npos ||
		path.find( "/." ) != std::string::npos )	// Okay, "~/.OpenLieroX/" is valid path, fail it anyway
		return false;
	if( stringcasefind( path, "cfg/" ) == 0 )	// Config dir is forbidden - some passwords may be stored here
		return false;
	for( uint f=0; f < path.size(); )
	{
		uint f1 = path.find_first_of(S_SYMBOL, f);
		if( f1 == std::string::npos )
			f1 = path.size();
		std::string word = path.substr( f, f1-f );
		for( uint f2=0; f2<sizeof(invalid_file_names)/sizeof(invalid_file_names[0]); f2++ )
		{
			if( stringcasecmp( word, invalid_file_names[f2] ) == 0 )
				return false;
		};
		if( f1 == path.size() )
			break;
		f = path.find_first_not_of(S_SYMBOL, f1);
		if( f == std::string::npos )	// Filename cannot end with a dot
			return false;
	};
	return true;
};

class StatFileList
{ 
	public:
   	std::string *data;
	const std::string & reqpath;	// Path as client requested it
   	int index;
	StatFileList( std::string *_data, const std::string & _reqpath ) : 
		data(_data), reqpath(_reqpath) {}
	inline bool operator() (std::string path)
	{
		size_t slash = findLastPathSep(path);
		if(slash != std::string::npos)
			path.erase(0, slash+1);
		*data += getStatPacketOneFile( reqpath + "/" + path );
		return true;
	};
};

class StatDirList
{ 
	public:
   	std::string *data;
	const std::string & reqpath;	// Path as client requested it
   	int index;
	StatDirList( std::string *_data, const std::string & _reqpath ) : 
		data(_data), reqpath(_reqpath) {}
	inline bool operator() (std::string path)
	{
		size_t slash = findLastPathSep(path);
		if(slash != std::string::npos)
			path.erase(0, slash+1);
		if( path == ".svn" )
			return true;
		*data += getStatPacketRecursive( reqpath + "/" + path );
		return true;
	};
};

std::string getStatPacketOneFile( const std::string & path )
{
	uint checksum, size, compressedSize;
	if( ! FileChecksum( path, &checksum, &size ) )
		return "";
	compressedSize = size + path.size() + 24; // Most files from disk are compressed already, so guessing size
	EndianSwap( checksum );
	EndianSwap( size );
	EndianSwap( compressedSize );
	return path + '\0' + 
			std::string( (const char *) (&size), 4 ) + // Real file size
			std::string( (const char *) (&compressedSize), 4 ) + // Zipped file size (used for download progressbar, so inexact for now)
			std::string( (const char *) (&checksum), 4 );	// Checksum
};

std::string getStatPacketRecursive( const std::string & path )
{
		struct stat st;
		if( ! StatFile( path, &st ) )
		{
			cout << "getStatPacketFileOrDir(): cannot stat file " << path << endl;
			return "";
		};
		if( S_ISREG( st.st_mode ) )
		{
			return getStatPacketOneFile( path );
		};
		if( S_ISDIR( st.st_mode ) )
		{
			std::string data;
			FindFiles( StatFileList( &data, path ), path, false, FM_REG);
			FindFiles( StatDirList( &data, path ), path, false, FM_DIR);
			return data;
		};
		return "";
};

std::string CUdpFileDownloader::getFileDownloading() const
{
	if( getState() != S_RECEIVE )
		return "";
	if( sLastFileRequested == "STAT:" )
		return "directory info";
	return sLastFileRequested;
};

float CUdpFileDownloader::getFileDownloadingProgress() const
{
	if( getState() != S_RECEIVE )
		return 0.0;
	if( cStatInfoCache.find(sLastFileRequested) == cStatInfoCache.end() )
		return 0.0;
	float ret = float(sData.size()) / float(cStatInfoCache.find(sLastFileRequested)->second.compressedSize);
	if( ret < 0.0 ) 
		ret = 0.0;
	if( ret > 1.0 ) 
		ret = 1.0;
	return ret;
};

uint CUdpFileDownloader::getFilesPendingAmount() const
{
	return tRequestedFiles.size();
};
