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
	HTTP_NET_ERROR,
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
private:
	std::string		sHost;
	std::string		sUrl;
	std::string		sRemoteAddress;
	std::string		sData;
	std::string		sHeader;
	std::string		sMimeType;
	HttpError		tError;

	size_t			iDataLength;
	size_t			iDataReceived;
	bool			bConnected;
	bool			bRequested;
	bool			bSocketReady;
	bool			bGotHttpHeader;
	float			fResolveTime;
	NetworkSocket	tSocket;
	NetworkAddr		tRemoteIP;

	void				SetHttpError(int id);
	void				Clear();
	bool				AdjustUrl(std::string& dest, const std::string& url);
	bool				SendRequest();
	void				ProcessData();
	std::string			GetPropertyFromHeader(const std::string& prop);
	void				ParseHeader();
	void				ParseAddress(const std::string& addr);

public:
	void				RequestData(const std::string& url);
	int					ProcessRequest();
	void				CancelProcessing();
	HttpError			GetError()				{ return tError; }
	const std::string&	GetData()				{ return sData; }
	const std::string&	GetMimeType()			{ return sMimeType; }
	size_t				GetDataLength()			{ return iDataLength; }
	size_t				GetReceivedDataLen()	{ return iDataReceived; }
};

#endif  // __HTTP_H__
