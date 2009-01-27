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


// HTTP class
// Created 9/4/02
// Jason Boettcher

#ifndef __HTTP_H__
#define __HTTP_H__

#include <string>
#include "Networking.h"

// Some basic defines
#define		HTTP_TIMEOUT	5
#define		BUFFER_LEN		8192

//
// Functions
//
void AutoSetupHTTPProxy();

//
// Classes
//

// HTTP Chunk parsing states
enum  {
	CHPAR_SKIPBLANK,
	CHPAR_LENREAD,
	CHPAR_DATAREAD,
	CHPAR_FOOTERREAD
};

// HTTP Chunk parsing class
class CChunkParser  {
public:
	CChunkParser(std::string *pure_data, size_t *final_length, size_t *received); 

	CChunkParser& operator=(const CChunkParser& chunk)  {
		iState = chunk.iState;
		iNextState = chunk.iNextState;
		sChunkLenStr = chunk.sChunkLenStr;
		iCurRead = chunk.iCurRead;
		iCurLength = chunk.iCurLength;

		return *this;
	}

private:
	std::string *sPureData;
	size_t		*iFinalLength;
	size_t		*iReceived;

	// Internal
	int			iState;
	int			iNextState;  // What state should come after the current one (used when iState = CHPAR_SKIPBLANK)
	std::string sChunkLenStr;  // Current chunk length as a string (temporary variable)
	size_t		iCurRead;  // How many bytes read from the current chunk
	size_t		iCurLength;  // How many bytes does the current chunk have

public:
	bool	ParseNext(char c);
	void	Reset();
};

// HTTP Post Field structure
class HTTPPostField  {
private:
	std::string sData;
	std::string sMimeType;
	std::string sName;
	std::string sFileName;
public:
	HTTPPostField() {}
	HTTPPostField(const std::string& data, const std::string& mimetype, const std::string& name, const std::string& file) :
		sData(data), sMimeType(mimetype), sName(name), sFileName(file) {}
	HTTPPostField(const HTTPPostField& oth)  { operator= (oth); }


	HTTPPostField& operator= (const HTTPPostField& oth)  {
		if (&oth == this)
			return *this;
		sData = oth.sData;
		sMimeType = oth.sMimeType;
		sName = oth.sName;
		sFileName = oth.sFileName;

		return *this;
	}

	void setFileName(const std::string& n)	{ sFileName = n; }
	const std::string& getFileName() const	{ return sFileName; }

	void setName(const std::string& n)	{ sName = n; }
	const std::string& getName() const	{ return sName; }

	void setMimeType(const std::string& n)	{ sMimeType = n; }
	const std::string& getMimeType() const	{ return sMimeType; }

	void setData(const std::string& d)	{ sData = d; }
	const std::string& getData() const	{ return sData; }
};

// HTTP error struct
class HttpError  { public:
	std::string sErrorMsg;
	int			iError;

	HttpError& operator=(const HttpError& oth)  {
		if (&oth != this)  {
			sErrorMsg = oth.sErrorMsg;
			iError = oth.iError;
		}
		return *this;
	}
};

// HTTP errors
// NOTE: If you change this, change also error strings in HTTP.cpp!!
enum  {
	HTTP_NO_ERROR = 0,
	HTTP_NO_SOCKET_ERROR,
	HTTP_CANNOT_RESOLVE_DNS,
	HTTP_INVALID_URL,
	HTTP_DNS_TIMEOUT,
	HTTP_ERROR_SENDING_REQ,
	HTTP_NO_CONNECTION,
	HTTP_ERROR_TIMEOUT,
	HTTP_NET_ERROR,
	HTTP_BAD_RESPONSE,
	HTTP_FILE_NOT_FOUND = 404
	// HINT: Add more if you need them
};

// HTTP processing results
enum  {
	HTTP_PROC_ERROR = -1,
	HTTP_PROC_PROCESSING = 0,
	HTTP_PROC_FINISHED = 1
};

// HTTP class
class CHttp  {
public:
	CHttp();
	~CHttp();

	CHttp(const CHttp& oth)  { operator= (oth); }

	CHttp& operator=(const CHttp& http)  {
		if (&http == this)
			return *this;

		sHost = http.sHost;
		sUrl = http.sUrl;
		sRemoteAddress = http.sRemoteAddress;
		sDataToSend = http.sDataToSend;
		sData = http.sData;
		sPureData = http.sPureData;
		sHeader = http.sHeader;
		sMimeType = http.sMimeType;
		tError.sErrorMsg = http.tError.sErrorMsg;
		tError.iError = http.tError.iError;
		if (http.tBuffer && tBuffer)
			memcpy(tBuffer, http.tBuffer, BUFFER_LEN);
		if (http.tChunkParser && tChunkParser)
			(*tChunkParser) = (*http.tChunkParser); // HINT: CChunkParser has a copy operator defined
		
		iDataLength = http.iDataLength;
		iDataReceived = http.iDataReceived;
		iDataSent = http.iDataSent;
		bActive = http.bActive;
		bTransferFinished = http.bTransferFinished;
		bSentHeader = http.bSentHeader;
		bConnected = http.bConnected;
		bRequested = http.bRequested;
		bRedirecting = http.bRedirecting;
		iRedirectCode = http.iRedirectCode;
		bSocketReady = http.bSocketReady;
		bGotHttpHeader = http.bGotHttpHeader;
		bChunkedTransfer = http.bChunkedTransfer;
		bGotDataFromServer = http.bGotDataFromServer;
		fResolveTime = http.fResolveTime;
		fConnectTime = http.fConnectTime;
		fSocketActionTime = http.fSocketActionTime;
		tSocket = http.tSocket;
		tRemoteIP = http.tRemoteIP;
		sProxyUser = http.sProxyUser;
		sProxyPasswd = http.sProxyPasswd;
		sProxyHost = http.sProxyHost;
		iProxyPort = http.iProxyPort;
		iAction = http.iAction;

		fDownloadStart = http.fDownloadStart;
		fDownloadEnd = http.fDownloadEnd;
		fUploadStart = http.fUploadStart;
		fUploadEnd = http.fUploadEnd;

		return *this;
	}
private:
	enum Action  {
		htaGet = 0,
		htaHead,
		htaPost
	};

	std::string		sHost;
	std::string		sUrl;
	std::string		sProxyHost;
	int				iProxyPort;
	std::string		sProxyUser;
	std::string		sProxyPasswd;
	std::string		sRemoteAddress;
	std::string		sDataToSend; // Data to be sent to the network, usually POST encoded
	std::string		sData;  // Data received from the network, can be chunked and whatever
	std::string		sPureData;  // Pure data, without chunks or any other stuff
	std::string		sHeader;
	std::string		sMimeType;
	HttpError		tError;
	char			*tBuffer; // Internal buffer
	CChunkParser	*tChunkParser;
	Action			iAction;

	size_t			iDataLength;
	size_t			iDataReceived;
	size_t			iDataSent;
	bool			bActive;
	bool			bTransferFinished;
	bool			bSentHeader;  // Only for POST
	bool			bConnected;
	bool			bRequested;
	bool			bSocketReady;
	bool			bGotHttpHeader;
	bool			bChunkedTransfer;
	bool			bGotDataFromServer;  // True if we received some data from the server
	float			fResolveTime;
	float			fConnectTime;
	float			fSocketActionTime;
	bool			bRedirecting;
	int				iRedirectCode;
	NetworkSocket	tSocket;
	NetworkAddr		tRemoteIP;

	// Bandwidth measurement
	float			fDownloadStart;
	float			fDownloadEnd;
	float			fUploadStart;
	float			fUploadEnd;

	bool				InitTransfer(const std::string& address, const std::string & proxy);
	void				SendDataInternal(const std::string& encoded_data, const std::string url, const std::string& proxy);
	void				SetHttpError(int id);
	void				Clear();
	bool				AdjustUrl(std::string& dest, const std::string& url); // URL-encode given string, substitute all non-ASCII symbols with %XX
	bool				SendRequest();
	void				POSTEncodeData(const std::list<HTTPPostField>& data);
	std::string			BuildPOSTHeader();
	void				ProcessData();
	std::string			GetPropertyFromHeader(const std::string& prop);
	void				ParseHeader();
	void				ParseChunks();
	void				ParseAddress(const std::string& addr);
	void				ParseProxyAddress(const std::string& proxy);
	std::string			GetBasicAuthentication(const std::string &user, const std::string &passwd);
	void				FinishTransfer();
	void				HandleRedirect(int code);
	void				RetryWithNoProxy();
	int					ReadAndProcessData();

	int					ProcessGET();
	int					ProcessPOST();
	int					ProcessHEAD();

public:
	// Proxy is string "user:passwd@host:port", only host is required, "user:passwd" were not tested
	// TODO: Maybe do the proxy string global?
	void				SendSimpleData(const std::string& data, const std::string url, const std::string& proxy = "");
	void				SendData(const std::list<HTTPPostField>& data, const std::string url, const std::string& proxy = "");
	void				RequestData(const std::string& url, const std::string& proxy = "");
	int					ProcessRequest();
	void				CancelProcessing();
	void				ClearReceivedData()		{ sPureData = ""; }
	const HttpError&	GetError()				{ return tError; }
	const std::string&	GetData()				{ return sPureData; }
	const std::string&	GetMimeType()			{ return sMimeType; }
	const std::string&	GetDataToSend()			{ return sDataToSend; }
	size_t				GetDataToSendLength()	{ return sDataToSend.size(); }
	size_t				GetDataLength()			{ return iDataLength; }
	size_t				GetReceivedDataLen()	{ return iDataReceived; }
	size_t				GetSentDataLen()		{ return iDataSent; }
	bool				RequestedData()			{ return bRequested; }

	float				GetDownloadTime()		{ return fDownloadEnd - fDownloadStart; }
	float				GetUploadTime()			{ return fUploadEnd - fUploadStart; }

	float				GetDownloadSpeed();
	float				GetUploadSpeed();

	const std::string&	GetHostName()			{ return sHost; }
	const std::string&	GetUrl()				{ return sUrl; }
};

#endif  // __HTTP_H__
