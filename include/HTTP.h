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
#include "Event.h"
#include "SmartPointer.h"

#include "ThreadPool.h"
#include <SDL_mutex.h>

// Some basic defines
#define		HTTP_TIMEOUT	10	// Filebase became laggy lately, so increased that from 5 seconds
#define		BUFFER_LEN		8192

//
// Functions
//
void AutoSetupHTTPProxy();

//
// Classes
//

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
enum HttpProc_t {
	HTTP_PROC_ERROR = -1,
	HTTP_PROC_PROCESSING = 0,
	HTTP_PROC_FINISHED = 1
};

class CHttpBase
{
	public:
	CHttpBase() { };
	~CHttpBase() { };

	enum Action  {
		htaGet = 0,
		htaHead,
		htaPost
	};

	struct HttpEventData  {
		HttpEventData(CHttpBase *h, bool succeeded) : cHttp(h), bSucceeded(succeeded) {}
		CHttpBase *cHttp;
		bool bSucceeded;
	};
	
	Event<HttpEventData>	onFinished;
	
	// Proxy is string "user:passwd@host:port", only host is required, "user:passwd" were not tested
	// TODO: Maybe do the proxy string global?
	virtual void				SendSimpleData(const std::string& data, const std::string url, const std::string& proxy = "") = 0;
	virtual void				SendData(const std::list<HTTPPostField>& data, const std::string url, const std::string& proxy = "") = 0;
	virtual void				RequestData(const std::string& url, const std::string& proxy = "") = 0;
	virtual HttpProc_t			ProcessRequest() = 0;
	virtual void				CancelProcessing() = 0;
	virtual void				ClearReceivedData() = 0;
	virtual HttpError			GetError() const = 0;
	virtual const std::string&	GetData() const = 0;
	virtual const std::string&	GetMimeType() const = 0;
	virtual const std::string&	GetDataToSend() const = 0;
	virtual size_t				GetDataToSendLength() const = 0;
	virtual size_t				GetDataLength() const = 0;
	virtual size_t				GetReceivedDataLen() const = 0;
	virtual size_t				GetSentDataLen() const = 0;
	virtual bool				RequestedData()	const = 0;

	virtual TimeDiff			GetDownloadTime() const = 0;
	virtual TimeDiff			GetUploadTime()	const = 0;

	virtual float				GetDownloadSpeed() const = 0;
	virtual float				GetUploadSpeed() const = 0;

	virtual const std::string&	GetHostName() const = 0;
	virtual const std::string&	GetUrl() const = 0;
	virtual bool				IsRedirecting() const = 0;

private:
	// they are not allowed atm
	CHttpBase(const CHttpBase& oth)  { operator= (oth); }
	CHttpBase& operator=(const CHttpBase& http);
};

struct HttpThread;
struct HttpRedirectEventData;
struct HttpRetryEventData;

// HTTP built-in class - rather buggy
class CHttp: public CHttpBase { 
	friend struct HttpThread;
public:
	CHttp();
	~CHttp();
	
private:
	// they are not allowed atm
	CHttp(const CHttp& oth)  { operator= (oth); }
	CHttp& operator=(const CHttp& http);

	// HTTP Chunk parsing states
	enum  {
		CHPAR_SKIPCRLF,
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

	// Processing thread
	HttpThread*		m_thread;
	mutable SDL_mutex	*tMutex;
	bool			bThreadRunning;

	HttpProc_t		iProcessingResult;  // Result of the last call of the ProcessInternal function

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
	AbsTime			fResolveTime;
	AbsTime			fConnectTime;
	AbsTime			fSocketActionTime;
	bool			bRedirecting;
	int				iRedirectCode;
	NetworkSocket	tSocket;
	NetworkAddr		tRemoteIP;

	// Bandwidth measurement
	AbsTime			fDownloadStart;
	AbsTime			fDownloadEnd;
	AbsTime			fUploadStart;
	AbsTime			fUploadEnd;

	std::string		sEmpty; // Returned when Data is being processed

private:
	void				Lock() const;
	void				Unlock() const;

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
	void				RetryWithNoProxy(const std::string& url, const std::string& data_to_send);
	int					ReadAndProcessData();
	bool				ProcessInternal();

	void				InitThread();
	void				ShutdownThread();
	void				HttpThread_onFinished(EventData);
	void				HttpThread_onRetry(SmartPointer<HttpRetryEventData>);
	void				HttpThread_onRedirect(SmartPointer<HttpRedirectEventData>);
	
	HttpProc_t			ProcessGET();
	HttpProc_t			ProcessPOST();
	HttpProc_t			ProcessHEAD();

public:
	
	void				SendSimpleData(const std::string& data, const std::string url, const std::string& proxy = "");
	void				SendData(const std::list<HTTPPostField>& data, const std::string url, const std::string& proxy = "");
	void				RequestData(const std::string& url, const std::string& proxy = "");
	HttpProc_t			ProcessRequest();
	void				CancelProcessing();
	void				ClearReceivedData()			{ Lock(); sPureData = ""; Unlock(); }
	HttpError			GetError() const;
	const std::string&	GetData() const				{ return bThreadRunning ? sEmpty : sPureData; }
	const std::string&	GetMimeType() const			{ return bThreadRunning ? sEmpty : sMimeType; }
	const std::string&	GetDataToSend() const		{ return sDataToSend; }
	size_t				GetDataToSendLength() const	{ Lock(); size_t r = sDataToSend.size(); Unlock(); return r; }
	size_t				GetDataLength() const		{ Lock(); size_t r = iDataLength; Unlock(); return r; }
	size_t				GetReceivedDataLen() const	{ Lock(); size_t r = iDataReceived; Unlock(); return r; }
	size_t				GetSentDataLen() const		{ Lock(); size_t r = iDataSent; Unlock(); return r; }
	bool				RequestedData()	const		{ return bRequested; }

	TimeDiff			GetDownloadTime() const		{ Lock(); TimeDiff r = fDownloadEnd - fDownloadStart; Unlock(); return r; }
	TimeDiff			GetUploadTime()	const		{ Lock(); TimeDiff r = fUploadEnd - fUploadStart; Unlock(); return r; }

	float				GetDownloadSpeed() const;
	float				GetUploadSpeed() const;

	const std::string&	GetHostName() const		{ return sHost; }
	const std::string&	GetUrl() const			{ return sUrl; }
	bool				IsRedirecting() const	{ return bRedirecting; }
};

#endif  // __HTTP_H__
