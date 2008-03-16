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

#include "Networking.h"

// Some basic defines
#define		HTTP_TIMEOUT	10
#define		BUFFER_LEN		8192

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

// HTTP error struct
class HttpError  { public:
	std::string sErrorMsg;
	int			iError;
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

	CHttp& operator=(const CHttp& http)  {
		sHost = http.sHost;
		sUrl = http.sUrl;
		sRemoteAddress = http.sRemoteAddress;
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
		bActive = http.bActive;
		bTransferFinished = http.bTransferFinished;
		bConnected = http.bConnected;
		bRequested = http.bRequested;
		bSocketReady = http.bSocketReady;
		bGotHttpHeader = http.bGotHttpHeader;
		bChunkedTransfer = http.bChunkedTransfer;
		fResolveTime = http.fResolveTime;
		fConnectTime = http.fConnectTime;
		tSocket = http.tSocket;
		tRemoteIP = http.tRemoteIP;
		sProxyPasswd = http.sProxyPasswd;
		sProxyHost = http.sProxyHost;
		iProxyPort = http.iProxyPort;

		return *this;
	}
private:
	std::string		sHost;
	std::string		sUrl;
	std::string		sProxyHost;
	int				iProxyPort;
	std::string		sProxyPasswd;
	std::string		sRemoteAddress;
	std::string		sData;  // Data received from the network, can be chunked and whatever
	std::string		sPureData;  // Pure data, without chunks or any other stuff
	std::string		sHeader;
	std::string		sMimeType;
	HttpError		tError;
	char			*tBuffer; // Internal buffer
	CChunkParser	*tChunkParser;

	size_t			iDataLength;
	size_t			iDataReceived;
	bool			bActive;
	bool			bTransferFinished;
	bool			bConnected;
	bool			bRequested;
	bool			bSocketReady;
	bool			bGotHttpHeader;
	bool			bChunkedTransfer;
	float			fResolveTime;
	float			fConnectTime;
	NetworkSocket	tSocket;
	NetworkAddr		tRemoteIP;

	void				SetHttpError(int id);
	void				Clear();
	bool				AdjustUrl(std::string& dest, const std::string& url); // URL-encode given string, substitute all non-ASCII symbols with %XX
	bool				SendRequest();
	void				ProcessData();
	std::string			GetPropertyFromHeader(const std::string& prop);
	void				ParseHeader();
	void				ParseChunks();
	void				ParseAddress(const std::string& addr);
	void				ParseProxyAddress(std::string proxy);
	void				FinishTransfer();

public:
	// Proxy is string "user:passwd@host:port", only host is required, "user:passwd" were not tested
	// TODO: Maybe do the proxy string global?
	void				RequestData(const std::string& url, const std::string& proxy = "");
	int					ProcessRequest();
	void				CancelProcessing();
	void				ClearReceivedData()		{ sPureData = ""; }
	const HttpError&	GetError()				{ return tError; }
	const std::string&	GetData()				{ return sPureData; }
	const std::string&	GetMimeType()			{ return sMimeType; }
	size_t				GetDataLength()			{ return iDataLength; }
	size_t				GetReceivedDataLen()	{ return iDataReceived; }
	bool				RequestedData()			{ return bRequested; }
};

#endif  // __HTTP_H__
