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


// HTTP class implementation
// Created 9/4/02
// Jason Boettcher


#include <cassert>
#ifdef WIN32
#include <windows.h>
#include <wininet.h>
#else
#include <stdlib.h>
#endif
#ifdef LIBCURL
#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>
#endif

#include "LieroX.h"
#include "Debug.h"
#include "Options.h"
#include "HTTP.h"
#include "Timer.h"
#include "StringUtils.h"
#include "types.h"
#include "Version.h"
#include "MathLib.h"
#include "InputEvents.h"
#include "ReadWriteLock.h"

// Some basic defines
#define		HTTP_TIMEOUT	10	// Filebase became laggy lately, so increased that from 5 seconds
#define		BUFFER_LEN		8192


// List of errors, MUST match error IDs in HTTP.h
static const std::string sHttpErrors[] = {
	"No error",
	"Could not open HTTP socket",
	"Could not resolve DNS",
	"Invalid URL",
	"DNS timeout",
	"Error sending HTTP request",
	"Could not connect to the server",
	"Connection timed out",
	"Network error: ",
	"Server sent an unsupported code: "
};

// Internal defines
#define HTTP_POST_BOUNDARY "0P3N1I3R0X" // Can be anything, this define is here only to sync BuildPostHeader and POSTEncodeData
#define HTTP_MAX_DATA_LEN 8192

//
// Functions
//

/////////////////
// Automatically updates tLXOptions->sHttpProxy with proxy settings retrieved from the system
void AutoSetupHTTPProxy()
{
	// User doesn't wish an automatic proxy setup
	if (!tLXOptions->bAutoSetupHttpProxy) {
		notes << "AutoSetupHTTPProxy is disabled, ";
		if(tLXOptions->sHttpProxy != "")
			notes << "using proxy " << tLXOptions->sHttpProxy << endl;
		else
			notes << "not using any proxy" << endl;
		return;
	}
	
	if(tLXOptions->sHttpProxy != "") {
		notes << "AutoSetupHTTPProxy: we had the proxy " << tLXOptions->sHttpProxy << " but we are trying to autodetect it now" << endl;
	}
	
// DevCpp won't compile this - too old header files
#if defined( WIN32 ) && defined( MSC_VER )
	// Create the list of options we want to retrieve
	INTERNET_PER_CONN_OPTION_LIST List;
	INTERNET_PER_CONN_OPTION Options[2];
	DWORD size = sizeof(INTERNET_PER_CONN_OPTION_LIST);

	// Options we need
	Options[0].dwOption = INTERNET_PER_CONN_PROXY_SERVER;
	Options[1].dwOption = INTERNET_PER_CONN_FLAGS;

	// Fill the list info
	List.dwSize = sizeof(INTERNET_PER_CONN_OPTION_LIST);
	List.pszConnection = NULL;
	List.dwOptionCount = 2;
	List.dwOptionError = 0;
	List.pOptions = Options;

	// Ask for proxy info
	if(InternetQueryOption(NULL, INTERNET_OPTION_PER_CONNECTION_OPTION, &List, &size)) {

		// Using proxy?
		bool using_proxy = (Options[1].Value.dwValue & PROXY_TYPE_PROXY) == PROXY_TYPE_PROXY;
		
		if (using_proxy)  {
			if (Options[0].Value.pszValue != NULL)  { // Safety check
				tLXOptions->sHttpProxy = Options[0].Value.pszValue; // Set the proxy

				// Remove http:// if present
				static const size_t httplen = 7; // length of "http://"
				if( stringcaseequal(tLXOptions->sHttpProxy.substr(0, httplen), "http://") )
					tLXOptions->sHttpProxy.erase(0, httplen);

				// Remove trailing slash
				if (*tLXOptions->sHttpProxy.rbegin() == '/')
					tLXOptions->sHttpProxy.resize(tLXOptions->sHttpProxy.size() - 1);

				notes << "Using HTTP proxy: " << tLXOptions->sHttpProxy << endl;
			}
		} else {
			tLXOptions->sHttpProxy = ""; // No proxy
		}

		// Cleanup
		if(Options[0].Value.pszValue != NULL)
			GlobalFree(Options[0].Value.pszValue);
	}
	
#else
	// Linux has numerous configuration of proxies for each application, but environment var seems to be the most common
	const char * c_proxy = getenv("http_proxy");
	if( c_proxy == NULL )  {
		c_proxy = getenv("HTTP_PROXY");
	}

	if(c_proxy) {
		// Get the value (string after '=' char)
		std::string proxy(c_proxy);
		if( proxy.find('=') == std::string::npos )  { // No proxy
			tLXOptions->sHttpProxy = "";
			return;
		}
		proxy = proxy.substr( proxy.find('=') + 1 );
		TrimSpaces(proxy);
	
		// Remove http:// if present
		static const size_t httplen = 7; // length of "http://"
		if( stringcaseequal(proxy.substr(0, httplen), "http://") )
			proxy.erase(0, httplen);

		// Blank proxy?
		if( proxy != "" )  {
			// Remove trailing slash
			if( *proxy.rbegin() == '/')
				proxy.resize( proxy.size() - 1 );
		}

		tLXOptions->sHttpProxy = proxy;

		notes << "AutoSetupHTTPProxy: " << proxy << endl;
	}	
#endif
}

#ifndef LIBCURL

//
// Chunk parser class
//

//////////////
// Contructor
CHttp::CChunkParser::CChunkParser(std::string *pure_data, size_t *final_length, size_t *received)
{
	Reset();
	sPureData = pure_data;
	iFinalLength = final_length;
	iReceived = received;
}

//////////////////
// Parse a next character from the received data
// NOTE: the caller is responsible for a valid iterator
bool CHttp::CChunkParser::ParseNext(char c)
{
	switch (iState)  {
	// Reading the chunk length
	case CHPAR_LENREAD:
		if (isspace((uchar)c) || c == ';')  {  // Ends with a whitespace or a semicolon
			// Get the length
			bool failed = false;
			iCurLength = from_string<size_t>(sChunkLenStr, std::hex, failed);
			if (failed)  {
#ifdef DEBUG
				//notes << "ParseChunks - atoi failed: \"" << sChunkLenStr << "\", " << itoa((uint)c) << endl;
#endif
				iCurLength = 0;
				return false;  // Still processing
			}
			sChunkLenStr = "";  // Not needed anymore

			// End?
			if (iCurLength == 0)  {  // 0-length chunk = end of the chunk message
				iState = CHPAR_SKIPCRLF; // Not needed, but if someone continued calling us, this is what we should do
				iNextState = CHPAR_FOOTERREAD;
				return true;  // Finished!
			}
			(*iFinalLength) += iCurLength;

			// Change the state
			iState = CHPAR_SKIPCRLF;
			iNextState = CHPAR_DATAREAD;
			iCurRead = 0;
		} else
			sChunkLenStr += c;
	return false;  // Still processing

	// Reading the data
	case CHPAR_DATAREAD:
		(*sPureData) += c;
		(*iReceived)++;
		iCurRead++;
		if (iCurRead >= iCurLength)  {
			// Reset
			iCurRead = 0;
			iCurLength = 0;
			sChunkLenStr = "";
			// Change the state
			iState = CHPAR_SKIPCRLF;
			iNextState = CHPAR_LENREAD;
		}
	return false; // Still processing

	// Skip blank characters
	case CHPAR_SKIPCRLF:
		if (c == '\n')  {  // LF, end of CRLF seq, fe'll fail to parse HTTP not conforming to RFC, but whatever - I have never seen those
			iState = iNextState; // Change the state
		}
	return false;  // Still processing

	// Process the footer
	case CHPAR_FOOTERREAD:
		// We currently have no interest in footers
		return false;

	}

	return false;  // Should not happen
}


////////////
// Reset the state and all variables and prepare for a new reading
void CHttp::CChunkParser::Reset()
{
	iState = CHPAR_LENREAD;
	iNextState = CHPAR_LENREAD;
	sChunkLenStr = "";
	iCurRead = 0;
	iCurLength = 0;
}


//
// HTTP helper functions
//

struct HttpRedirectEventData  {
	HttpRedirectEventData(CHttp *h, const std::string& url, const std::string proxy, const std::string& data) :
	http(h), sUrl(url), sProxy(proxy), sData(data) {}
	
	CHttp *http;
	std::string sUrl;
	std::string sProxy;
	std::string sData;
};

struct HttpRetryEventData  {
	HttpRetryEventData(CHttp *h, const std::string& url, const std::string& data) :
	http(h), sUrl(url), sData(data) {}
	
	CHttp *http;
	std::string sUrl;
	std::string sData;
};

struct HttpThread {
	Event<> onFinished;
	Event< SmartPointer<HttpRetryEventData> > onRetry;
	Event< SmartPointer<HttpRedirectEventData> > onRedirect;
	
	CHttp* http;
	ThreadPoolItem* thread;
	volatile bool breakThreadSignal;
	
	HttpThread(CHttp* h) : http(h), thread(NULL), breakThreadSignal(false) {}
	
	void startThread() {
		breakThread(); // if a thread is already running, break it
		breakThreadSignal = false;
		thread = threadPool->start(run, this, "HTTP helper");
	}
	
	void breakThread() {
		if(thread != NULL) {
			breakThreadSignal = true;
			threadPool->wait(thread, NULL);
			thread = NULL;
		}
	}
	
	static int run(void* param) { return ((HttpThread*)param)->run(); }
	int run() {
		while (!breakThreadSignal)  {
			// Transfer finished
			if (http->ProcessInternal())
				break;
		}
		
		// Notify about thread finishing
		http->Lock();
		if (http->iProcessingResult != HTTP_PROC_PROCESSING) // Send the finished event only when not restarting (for example due to a proxy fail)
			onFinished.pushToMainQueue(EventData(http));
		http->Unlock();

		return 0;
	}
};

////////////////////
// Handle the retry event (retrying with no proxy)
void CHttp::HttpThread_onRetry(SmartPointer<HttpRetryEventData> d)
{
	ShutdownThread();
	RetryWithNoProxy(d->sUrl, d->sData);
}

////////////////////
// Redirect event
void CHttp::HttpThread_onRedirect(SmartPointer<HttpRedirectEventData> d)
{
	ShutdownThread();

	switch (iAction)  {
	case CHttp::htaGet:
		RequestData(d->sUrl, d->sProxy);
	break;
	case CHttp::htaPost:
		SendDataInternal(d->sData, d->sUrl, d->sProxy);
	break;
	case CHttp::htaHead:
		errors << "HTTP HEAD not implemented" << endl;
	break;
	}
}

//
// HTTP class
//

//////////////
// Constructor
CHttp::CHttp()
{
	tMutex = SDL_CreateMutex();
	m_thread = new HttpThread(this);
	m_thread->onFinished.handler() = getEventHandler(this, &CHttp::HttpThread_onFinished);
	m_thread->onRetry.handler() = getEventHandler(this, &CHttp::HttpThread_onRetry);
	m_thread->onRedirect.handler() = getEventHandler(this, &CHttp::HttpThread_onRedirect);

	// Buffer for reading from socket
	tBuffer = new char[BUFFER_LEN];
	tChunkParser = new CChunkParser(&sPureData, &iDataLength, &iDataReceived);
	Clear();
}

//////////////
// Destructor
CHttp::~CHttp()
{
	CancelProcessing();
	if (tBuffer) delete[] tBuffer; tBuffer = NULL;
	if (tChunkParser) delete tChunkParser; tChunkParser = NULL;
	if (m_thread) delete m_thread; m_thread = NULL;
	SDL_DestroyMutex(tMutex); tMutex = NULL;
}

///////////////
// Copy operator
CHttp& CHttp::operator =(const CHttp& http)
{
	assert(false); // not allowed atm
	
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
	tRemoteIP = http.tRemoteIP;
	sProxyUser = http.sProxyUser;
	sProxyPasswd = http.sProxyPasswd;
	sProxyHost = http.sProxyHost;
	iProxyPort = http.iProxyPort;
	iAction = http.iAction;
	iProcessingResult = http.iProcessingResult;

	fDownloadStart = http.fDownloadStart;
	fDownloadEnd = http.fDownloadEnd;
	fUploadStart = http.fUploadStart;
	fUploadEnd = http.fUploadEnd;

	if (m_thread)  {
		m_thread->breakThread();
		delete m_thread;
		m_thread = NULL;
	}
	if (tMutex)
		SDL_DestroyMutex(tMutex);
	tMutex = http.tMutex;
	bThreadRunning = http.bThreadRunning;

	return *this;
}

//////////////
// Clear everything
void CHttp::Clear()
{
	// End the processing thread first
	ShutdownThread();

	sHost = "";
	sUrl = "";
	sProxyHost = "";
	iProxyPort = 0;
	sProxyUser = "";
	sProxyPasswd = "";
	sRemoteAddress = "";
	sDataToSend = "";
	sData = "";
	sPureData = "";
	sHeader = "";
	sMimeType = "";
	SetHttpError(HTTP_NO_ERROR);
	iAction = htaGet;
	iDataLength = 0;
	iDataReceived = 0;
	iDataSent = 0;
	bActive = false;
	bSentHeader = false;
	bTransferFinished = false;
	bConnected = false;
	bRequested = false;
	bGotHttpHeader = false;
	bSocketReady = false;
	bChunkedTransfer = false;
	bRedirecting = false;
	bGotDataFromServer = false;
	iRedirectCode = 0;
	fResolveTime = AbsTime();
	fConnectTime = AbsTime();
	fSocketActionTime = AbsTime();
	fDownloadStart = AbsTime();
	fDownloadEnd = AbsTime();
	fUploadStart = AbsTime();
	fUploadEnd = AbsTime();
	iProcessingResult = HTTP_PROC_PROCESSING;

	ResetNetAddr(tRemoteIP);
	tSocket.Clear();
	if (tChunkParser)
		tChunkParser->Reset();
	ResetSocketError();  // To prevent quitting when previous request failed
}

///////////////
// Cancel current request
void CHttp::CancelProcessing()
{
	Clear();
}

///////////////
// Set the HTTP error
void CHttp::SetHttpError(int id)
{
	assert(id >= 0 && id < (int)(sizeof(sHttpErrors)/sizeof(std::string)));
	tError.iError = id;
	tError.sErrorMsg = sHttpErrors[id];
}

////////////////////
// Called when the processing thread finishes
void CHttp::HttpThread_onFinished(EventData)
{
	ShutdownThread();

	// If finished (error or success), fire the event
	if (iProcessingResult != HTTP_PROC_PROCESSING)
		onFinished.occurred(HttpEventData(this, tError.iError == HTTP_NO_ERROR));
}


/////////////////////
// Initialize the processing thread
void CHttp::InitThread()
{
	// Create and run the processing thread
	iProcessingResult = HTTP_PROC_PROCESSING;
	m_thread->startThread();
	bThreadRunning = true;
}

/////////////////////
// Shutdown the processing thread
void CHttp::ShutdownThread()
{
	m_thread->breakThread();
	bThreadRunning = false;
}

////////////////////
// Initiates the data transfer (GET/POST/HEAD)
bool CHttp::InitTransfer(const std::string& address, const std::string & proxy)
{
	// Stop any previous transfers
	CancelProcessing();

	// Lock
	Lock();

	// Make the urls http friendly (get rid of spaces)
	if (!AdjustUrl(sRemoteAddress, address))  {
		SetHttpError(HTTP_INVALID_URL);
		Unlock();
		return false;
	}

    // Convert the address to host & url
    // Ie, '/'s from host goes into url
    ParseAddress(sRemoteAddress);
	ParseProxyAddress(proxy);	// Fills sProxyHost, iProxyPort, sProxyPasswd and sProxyUser

	//notes << "Sending HTTP request " << address << "\n";

	// Open the socket
	if(!tSocket.OpenReliable(0))  {
		SetHttpError(HTTP_NO_SOCKET_ERROR);
		Unlock();
		return false;
	}

	// Resolve the address
	// Reset the current adr; sometimes needed (hack? bug in hawknl?)
	ResetNetAddr(tRemoteIP);
    fResolveTime = GetTime();
	std::string host = sHost;
	if( sProxyHost.size() != 0 )
		host = sProxyHost;

	ResetNetAddr(tRemoteIP);
	
	// Try if an IP has been entered
	if (!StringToNetAddr(host, tRemoteIP))  {
		ResetSocketError(); // Clear the BAD_ADDR error

		// Not an IP, use DNS
		GetNetAddrFromNameAsync(host, tRemoteIP);
	}

	// We're active now
	bActive = true;
	iProcessingResult = HTTP_PROC_PROCESSING;

	// Unlock
	Unlock();

	return true;
}

//////////////
// Request data from server
void CHttp::RequestData(const std::string& address, const std::string & proxy)
{
	InitTransfer(address, proxy);
	iAction = htaGet;
	InitThread();
}
/*
//////////////////
// Send data to a server (simple version)
void CHttp::SendSimpleData(const std::string& data, const std::string url, const std::string& proxy)
{
	InitTransfer(url, proxy);
	iAction = htaPost;

	std::list<HTTPPostField> temp;
	temp.push_back(HTTPPostField(data, "", "data", ""));
	POSTEncodeData(temp);

	InitThread();
}
*/

///////////////////
// Send data to a server (advanced)
void CHttp::SendData(const std::list<HTTPPostField> &data, const std::string url, const std::string &proxy)
{
	InitTransfer(url, proxy);
	iAction = htaPost;

	POSTEncodeData(data);

	InitThread();
}

//////////////////
// Send data to a server (internal, for no-proxy retrying)
void CHttp::SendDataInternal(const std::string& encoded_data, const std::string url, const std::string& proxy)
{
	Lock();
	InitTransfer(url, proxy);
	iAction = htaPost;

	sDataToSend = encoded_data;
	iDataLength = sData.size();

	InitThread();

	Unlock();
}

/////////////////
// Parse the address given by user
void CHttp::ParseAddress(const std::string& addr)
{
    sHost = "";
    sUrl = "";

	size_t p = addr.find('/');
	if(p == std::string::npos)  {
		sHost = addr;
		sUrl = "/";
	} else {
		sHost = addr.substr(0, p);
		sUrl = addr.substr(p);

		if (sUrl.size() == 0)
			sUrl = "/";
	}
}

////////////////////
// Create the authentication header contents for the `Basic' scheme.
std::string CHttp::GetBasicAuthentication(const std::string &user, const std::string &passwd)
{
	// This is done by encoding the string "USER:PASS" to base64 and
	// prepending the string "Basic " in front of it.
	return "Basic " + Base64Encode(user + ":" + passwd);
}


////////////////////
// Returns the download speed in bytes per second
float CHttp::GetDownloadSpeed() const
{
	Lock();
	TimeDiff dt = fDownloadEnd - fDownloadStart;
	Unlock();
	if (dt == TimeDiff())
		return 999999999.0f;  // Unmeasurable
	return iDataReceived / dt.seconds();
}

////////////////////
// Returns the upload speed in bytes per second
float CHttp::GetUploadSpeed() const
{
	Lock();
	TimeDiff dt = fUploadEnd - fUploadStart;
	Unlock();
	if (dt == TimeDiff())
		return 999999999.0f;  // Unmeasurable
	return iDataSent / dt.seconds();
}

////////////////////
// Returns the HTTP error
HttpError CHttp::GetError() const
{
	Lock();
	HttpError tmp = tError;
	Unlock();
	return tmp;
}

///////////////////
// Lock the HTTP class
void CHttp::Lock() const
{
	SDL_LockMutex(tMutex);
}

/////////////////
// Unlock the HTTP class
void CHttp::Unlock() const
{
	SDL_UnlockMutex(tMutex);
}

/////////////////
// Parse the given proxy address
void CHttp::ParseProxyAddress(const std::string& proxy)
{
	sProxyHost = "";
	iProxyPort = 80;
	sProxyUser = "";
	sProxyPasswd = "";
	if( proxy.length() == 0 )
		return;

	// The format is user:pass@host:port
	// or host:port

	size_t atpos = proxy.find('@');
	std::string server;
	if( atpos != std::string::npos )	{
		// Get the login info and server
		std::string proxy_login = proxy.substr( 0, atpos );
		server = proxy.substr( atpos + 1 );
		
		size_t dotpos = proxy_login.find(':');

		// Get user and password from the login info
		if( dotpos != std::string::npos )  {
			sProxyUser = proxy_login.substr( 0, dotpos );
			sProxyPasswd = proxy_login.substr( dotpos + 1 );
		} else {  // No ':' character, everything goes to user
			sProxyUser = proxy_login;
		}
	} else { // No '@' character, everything goes to server
		server = proxy;
	}

	// Get the host
	sProxyHost = server;
	if( server.find(':') != std::string::npos )  { // Is port present?
		sProxyHost = server.substr( 0, server.find(':') );
		iProxyPort = atoi( server.substr( server.find(':') + 1 ) );
	}
}

/////////////////
// Adjust the given URL
bool CHttp::AdjustUrl(std::string &dest, const std::string &url)
{
	// We don't need that piece of code - we have UrlEncode() function which should be used by user of this class
	/*
	dest = "";
	
	static const std::string dont_encode = "-_.!~*'()&?/="; // Characters that won't be encoded in any way

	// Go through the url, encoding the characters
	std::string::const_iterator url_it;
	for( url_it = url.begin(); url_it != url.end(); url_it++) {

		if( isalnum(*url_it) || dont_encode.find(*url_it) != std::string::npos)
			dest += *url_it;
		else {
			dest += '%';
			if (*url_it <= 0xF)
				dest += '0';

			dest += itoa((int)*url_it, 16);
		}
	}

	// Remove http:// (not needed, we know we use http)
	if (dest.size() > 9)
		if (stringcasecmp(dest.substr(0, 9), "http%3a//") == 0)  {  // HINT - ":" is already URL-encoded
			dest.erase(0, 9);
		}
	*/
	
	dest = url;
	// Remove http:// (not needed, we know we use http)
	if (dest.size() > 7)
		if (stringcasecmp(dest.substr(0, 7), "http://") == 0)  {
			dest.erase(0, 7);
		}

	// Remove double slashes
	while (replace(dest, "//", "/")) {}

	return true;
}

/////////////////
// Send the HTTP request (used only for GET atm.)
bool CHttp::SendRequest()
{
	std::string request;

	// Build the url
	request = "GET " + sUrl + " HTTP/1.1\r\n";
	if (sProxyHost.size() != 0)  // Proxy servers require full URL in GET header (so they know where to forward the request)
		request = "GET http://" + sHost + sUrl + " HTTP/1.1\r\n";
	if (sProxyPasswd.size() != 0 || sProxyUser.size() != 0)
		request += "Proxy-Authorization: " + GetBasicAuthentication(sProxyUser, sProxyPasswd) + "\r\n";
	request += "Host: " + sHost + "\r\n";
	request += "User-Agent: "; request += GetFullGameName(); request += "\r\n";
	request += "Connection: close\r\n\r\n";  // We currently don't support persistent connections
	tSocket.setRemoteAddress(tRemoteIP);
	return tSocket.Write(request) > 0;  // Anything written?
}

//////////////////
// Build the POST header
// NOTE: must be called after sDataToSend has been prepared
std::string CHttp::BuildPOSTHeader()
{
	std::string header;

	// Build the url
	header = "POST " + sUrl + " HTTP/1.1\r\n";
	if( sProxyHost.size() != 0 )  // Proxy servers require full URL in POST header (so they know where to forward the request)
		header = "POST http://" + sHost + sUrl + " HTTP/1.1\r\n";
	if( sProxyPasswd.size() != 0 || sProxyUser.size() != 0 )	// Don't know their order, guess Proxy-Authorization should be first header
		header += "Proxy-Authorization: " + GetBasicAuthentication(sProxyUser, sProxyPasswd) + "\r\n";
	header += "Host: " + sHost + "\r\n";
	header += "User-Agent: "; header += GetFullGameName(); header += "\r\n";
	header += "Content-Length: " + itoa(sDataToSend.size()) + "\r\n";
	header += "Content-Type: multipart/form-data, boundary=" + std::string(HTTP_POST_BOUNDARY) + "\r\n";
	header += "Connection: close\r\n\r\n";  // We currently don't support persistent connections

	return header;
}

////////////////////
// Fills sData with the post-encoded data
void CHttp::POSTEncodeData(const std::list<HTTPPostField>& fields)
{
	// Start with the boundary
	sDataToSend = "--" + std::string(HTTP_POST_BOUNDARY) + "\r\n";

	// Add the fields
	for (std::list<HTTPPostField>::const_iterator it = fields.begin(); it != fields.end(); it++)  {
		bool binary = true;
		
		// Headers
		sDataToSend += "content-disposition: form-data";
		if (it->getName().size())
			sDataToSend += "; name=\"" + it->getName() + "\""; // TODO: slash-escape the name
		if (it->getFileName().size())
			sDataToSend += "; filename=\"" + it->getFileName() + "\"";
		sDataToSend += "\r\n";
		if (it->getMimeType().size())  {
			sDataToSend += "Content-Type: " + it->getMimeType() + "\r\n";
			binary = (it->getMimeType().find("text") != 0); // text/* - not a binary transfer
		}
		if (binary)
			sDataToSend += "Content-Transfer-Encoding: binary\r\n";
		sDataToSend += "\r\n";

		// Data itself
		sDataToSend += it->getData();

		// Ending boundary
		sDataToSend += "\r\n--" + std::string(HTTP_POST_BOUNDARY) + "\r\n";
	}

	// Make sure we generate valid data even with an empty input
	if (fields.empty())
		sDataToSend += "--" + std::string(HTTP_POST_BOUNDARY) + "\r\n";
}

/////////////////
// Process the received data
void CHttp::ProcessData()
{
	// If the data is split into chunks, we have to parse them
	if (bChunkedTransfer)
		ParseChunks();
	else  {
		if (bGotHttpHeader)  {  // We don't know if the data is chunked or not if we haven't parsed the header,
								// don't do anything if we don't know what to do
			sPureData += sData;
			sData = ""; // HINT: save memory, don't keep the data twice
		}
	}

	// We only strip the header and parse it here
	// If it has been already parsed, there's no work for us
	if (bGotHttpHeader)
		return;

	// Find the end of the header
	uchar header_end_len = 2;
	// TODO: remove variants non-conforming to RFC like \n\n or \n\r\n\r
	size_t header_end1 = sData.find("\n\n");  // Two LFs...
	size_t header_end2 = sData.find("\r\n\r\n"); // ... or two CR/LFs
	size_t header_end3 = sData.find("\n\r\n\r"); // ... or two LF/CRs

	if (header_end1 == std::string::npos) header_end1 = sData.size();
	if (header_end2 == std::string::npos) header_end2 = sData.size();
	if (header_end3 == std::string::npos) header_end3 = sData.size();
	size_t header_end = MIN(MIN(header_end1, header_end2), header_end3);

	// Found the end of the header?
	if (header_end == sData.size())
		return; // No header is present

	if (header_end == header_end2 || header_end == header_end3)
		header_end_len = 4;
	

	header_end += header_end_len; // Two LFs or two CR/LFs
	if (header_end > sData.size())  // Should not happen...
		return;

	sHeader = sData.substr(0, header_end);
	sData.erase(0, header_end);

	// The header is ok :)
	bGotHttpHeader = true;

	// Process the header
	ParseHeader();

	// Error check
	if (tError.iError < 400 && tError.iError != HTTP_NO_ERROR) // Network-related errors, 4xx and 5xx are HTTP errors which can contain a message body
		return;

	// We could just receive the header and find out the data is chunked...
	if (bChunkedTransfer)  {
		ParseChunks();
	} else  {
		sPureData += sData;  // No parsing, just add them
		sData = ""; // HINT: save memory, don't keep the data twice
	}
}

//////////////////
// Reads a HTTP header property
std::string CHttp::GetPropertyFromHeader(const std::string& prop)
{
	size_t pos = stringcasefind(sHeader, prop + ":");
	if (pos == std::string::npos)  // Not found
		return "";
	else {
		pos += prop.size() + 1;  // 1 - ":" character

		// Check
		if (pos >= sHeader.size())
			return "";

		// Get the value
		std::string result = "";
		std::string::iterator i = PositionToIterator(sHeader, pos);
		for (; i!= sHeader.end(); i++)  {
			if (*i == '\r' || *i == '\n')  {

				// Does it continue on next line?
				std::string::iterator i2 = i;
				i2++;
				if (i2 == sHeader.end())
					break;

				if (*i2 == '\n')   {
					i2++;
					if (i2 == sHeader.end())
						break;
				}

				if (*i2 != ' ' && *i2 != '\t')  // Not beginning with space or tab, it's a real end
					break;
			}

			result += *i;
		}

		// Remove any spaces
		TrimSpaces(result);

		return result;
	}
}


//////////////////
// Gets the data from chunks
void CHttp::ParseChunks()
{
	// Parse the chunks
	if (!bTransferFinished)  {
		std::string::iterator chunk_it = sData.begin();
		for (; chunk_it != sData.end(); chunk_it++)
			if (tChunkParser->ParseNext(*chunk_it))  {
				bTransferFinished = true;
				bActive = false;
				break;  // Finished!
			}
	}

	// Remove the parsed data
	sData = "";

	// There could be still some footers, but we don't care about them
}

//////////////////
// Reads the redirect information from HTTP header
void CHttp::HandleRedirect(int code)
{
	// Check
	if (code < 300 || code >= 400)  {
		SetHttpError(HTTP_BAD_RESPONSE);
		tError.sErrorMsg += "Wrong redirect code " + itoa(code);
		return;
	}

	// If the transfer is not yet finished, wait
	if (!bTransferFinished)  {
		bRedirecting = true;
		iRedirectCode = code;
		return;
	}

	// All the 3XX messages may contain the Location field with a preferred address
	std::string location = GetPropertyFromHeader("Location");

	// Process depending on the code
	switch (code)  {
	case 300: // Multiple choices

		// First try the Location field
		if (location.size() != 0)  {
			// Starting a new request must be done in the main thread, send a notification and quit processing
			std::string proxy = sProxyUser + ":" + sProxyPasswd + "@" + sProxyHost + ":" + itoa(iProxyPort);
			m_thread->onRedirect.pushToMainQueue(SmartPointer<HttpRedirectEventData>(new HttpRedirectEventData(this, location, proxy, sDataToSend)));
			return;
		}
		
		// Multiple Choices, these are listed in the message body

		// Got any data?
		if (sPureData.size() == 0)  {
			SetHttpError(HTTP_BAD_RESPONSE);
			tError.sErrorMsg = "No redirect address specified in the redirect message";
			return;
		}

		// Read the first location (assuming that there's one location per line)
		location = ReadUntil(sPureData, '\n');
		TrimSpaces(location);

		if (location.size())  {
			notes("HTTP notice: redirected from " + sHost + sUrl + " to " + location + "\n");
			// Starting a new request must be done in the main thread, send a notification and quit processing
			std::string proxy = sProxyUser + ":" + sProxyPasswd + "@" + sProxyHost + ":" + itoa(iProxyPort);
			m_thread->onRedirect.pushToMainQueue(SmartPointer<HttpRedirectEventData>(new HttpRedirectEventData(this, location, proxy, sDataToSend)));
		} else {
			SetHttpError(HTTP_BAD_RESPONSE);
			tError.sErrorMsg = "No redirect address specified in the redirect message";
		}
	break;
	case 301: // Moved Permanently
	case 302: // Found
	case 303: // See Other
	case 307: // Temporary Redirect; TODO: according to the RFC we must ask the user if he/she allows the redirect...

		// New location should be stored in the Location field
		if (location.size() != 0)  {
			notes("HTTP notice: redirected from " + sHost + sUrl + " to " + location + "\n");
			// Starting a new request must be done in the main thread, send a notification and quit processing
			std::string proxy = sProxyUser + ":" + sProxyPasswd + "@" + sProxyHost + ":" + itoa(iProxyPort);
			m_thread->onRedirect.pushToMainQueue( 
				SmartPointer<HttpRedirectEventData>(new HttpRedirectEventData(this, location, proxy, sDataToSend)));
			return;
		} else { // No location has been given, just quit...
			SetHttpError(HTTP_BAD_RESPONSE);
			tError.sErrorMsg = "No redirect address specified in the redirect message";
		}
	break;

	case 304: // Not Modified
		// We should never get this as we don't use conditional requests
		SetHttpError(HTTP_BAD_RESPONSE);
		tError.sErrorMsg = "Got a Not Modified response on a non-conditional request";
	break;

	case 305: // Use Proxy
		// The proxy address is given in the location field
		if (location.size())  {
			// Starting a new request must be done in the main thread, send a notification and quit processing
			m_thread->onRedirect.pushToMainQueue(
				SmartPointer<HttpRedirectEventData>(new HttpRedirectEventData(this, sHost + sUrl, location, sDataToSend)));
			notes("HTTP notice: accessing the desired location using a proxy: " + location + "\n");
		} else {
			SetHttpError(HTTP_BAD_RESPONSE);
			tError.sErrorMsg = "No redirect address specified in the redirect message";
		}
	break;
	}
}

////////////////
// Parses the header
void CHttp::ParseHeader()
{
	//
	// Error checking
	//
	std::string::iterator i = sHeader.begin();
	while (i != sHeader.end())  {  // Skip the "HTTP/1.1 " thingy
		if (*i == ' ')  {
			i++;
			break;
		}

		i++;
	}

	// Get the code
	std::string code;
	while (i != sHeader.end())  {
		if (*i == ' ' || *i == '\t')  {
			i++;
			break;
		}

		code += *i;
		i++;
	}

	// Check the code
	switch (code[0]) { 
	case '1': // 100 Continue - we should wait for a real header
		if (code.substr(0, 3) == "100")  {
			bGotHttpHeader = false;
			return;
		} else {
			SetHttpError(HTTP_BAD_RESPONSE);
			tError.sErrorMsg += code;
			tSocket.Close();
			return;
		}
	break;

	case '2': // 2XX = success
	break;

	case '3': // 3XX = redirect
		HandleRedirect(atoi(code));
	break;

	case '4': // 4XX = error
	case '5': // 5XX = server error

		// Other errors
		tError.iError = atoi(code);
		if (i != sHeader.end())
			tError.sErrorMsg = std::string(i, PositionToIterator(sHeader, sHeader.find('\n')));  // From error code to the end of the line
		else
			tError.sErrorMsg = "Unknown Error";

		// HINT: continue parsing the message, there could be a useful info in the message body that the client wants to know
	}


	//
	// Mime-type
	//
	sMimeType = GetPropertyFromHeader("Content-Type");

	// Remove encoding
	size_t semic_pos = sMimeType.find(';');
	if (semic_pos != std::string::npos)
		sMimeType.erase(semic_pos, std::string::npos);

	//
	// Content length
	//
	bool incorrect = false;
	iDataLength = from_string<size_t>(GetPropertyFromHeader("Content-Length"), incorrect);
	if (incorrect)
		iDataLength = 0;

	//
	// Check for a chunked connection
	//
	bChunkedTransfer = stringcaseequal(GetPropertyFromHeader("Transfer-Encoding"), "chunked");
}

//////////////////
// Re-initiates the transfer using a direct connection (no proxy)
void CHttp::RetryWithNoProxy(const std::string& url, const std::string& data_to_send)
{
	switch (iAction)  {
	case htaGet:
		RequestData(url);
	break;
	case htaPost:
		SendDataInternal(data_to_send, url, "");
	break;
	case htaHead:
		errors("HTTP HEAD not implemented\n");
	break;
	}
}

//////////////////
// Reads data from the socket and calls ProcessData if some data is present
// NOTE: the tMutex must be always locked before calling this!
int CHttp::ReadAndProcessData()
{
	// Check if we have a response
	int count = 0;
	if (tBuffer != NULL)  {
		while (true)  {
			count = tSocket.Read(tBuffer, BUFFER_LEN);
			if (count <= 0)
				break;

			// First data we got? Start the download time timer
			if (!bGotDataFromServer)
				fDownloadStart = GetTime();
			fDownloadEnd = GetTime();  // To make the download time correct even when still downloading

			bGotDataFromServer = true;

			// Got some data
			sData.append(tBuffer, count);
			iDataReceived += count;
			ProcessData();
			fSocketActionTime = GetTime();
		}
	}

	// Any HTTP errors?
	if (tError.iError != HTTP_NO_ERROR)
		return HTTP_PROC_ERROR;

	// Error, or end of connection?
	if(count < 0) {
		int err = GetSocketErrorNr();
		if(IsMessageEndSocketErrorNr(err) ) {
			// End of connection
			// Complete!
			// HINT: fDownloadEnd has already been updated above
			bTransferFinished = true;
			bActive = false;
			tSocket.Close();

		} else {
			// Error
			SetHttpError(HTTP_NET_ERROR);
			tError.sErrorMsg += GetSocketErrorStr(err);
			return HTTP_PROC_ERROR;
		}
	}

	return HTTP_PROC_PROCESSING;
}

/////////////////
// Process the GET request
// NOTE: the tMutex must be always locked before calling this!
HttpProc_t CHttp::ProcessGET()
{
	assert(iAction == htaGet);

	// Send a request
	if(bSocketReady && bConnected && !bRequested) {
		bRequested = true;
		fSocketActionTime = GetTime();

		if(!SendRequest()) {
            SetHttpError(HTTP_ERROR_SENDING_REQ);
			return HTTP_PROC_ERROR;
		}
	}

	// If we aren't ready yet, leave
	if(!bSocketReady || !bConnected || !bRequested)
		return HTTP_PROC_PROCESSING;

	// Wait until we have some data to read
	// HINT: ReadAndProcessData() is called in a separate thread so we can do this blocking call
	// TODO: this is blocking, fix that!! (it will block also the main thread if you call ShutdownThread())
	Unlock();
	tSocket.WaitForSocketRead(100);
	Lock();

	// Check for response
	int res = ReadAndProcessData();
	if (res == HTTP_PROC_ERROR)
		return HTTP_PROC_ERROR; // The error is set in ReadAndProcessData

	// Transfer fininshed
	if (bTransferFinished)  {

		// If we have been waiting for redirect, handle it now
		if (bRedirecting)  {
			HandleRedirect(iRedirectCode);

			if (tError.iError != HTTP_NO_ERROR) // Any errors?
				return HTTP_PROC_ERROR;
			else
				return HTTP_PROC_PROCESSING;
		}

		return HTTP_PROC_FINISHED; // Really finished
	}

	// Still processing
	return HTTP_PROC_PROCESSING;
}

/////////////////
// Process posting data on the server
// NOTE: the tMutex must be always locked before calling this!
HttpProc_t CHttp::ProcessPOST()
{
	assert(iAction == htaPost);

	// Send the initial packet
	if(bSocketReady && bConnected && !bSentHeader) {
		bSentHeader = true;
		fSocketActionTime = GetTime();

		// Create the header
		std::string header = BuildPOSTHeader();
		std::string buf = header;

		// Append some data if there's still space
		size_t len = 0;
		if (buf.size() < HTTP_MAX_DATA_LEN)  {
			len = MIN(sDataToSend.size(), HTTP_MAX_DATA_LEN - buf.size());
			buf.append(sDataToSend.begin(), sDataToSend.begin() + len);
		}

		// Send
		int res = tSocket.Write(buf);
		if (res < 0)  {
			SetHttpError(HTTP_ERROR_SENDING_REQ);
			return HTTP_PROC_ERROR;
		}
		else if ((size_t)res == buf.size())  {
			iDataSent += len;
			fUploadStart = GetTime();  // We're starting the upload
			fUploadEnd = fUploadStart; // To make the time counting correct even when still uploading
		} else {
			assert((size_t)res < buf.size()); // Cannot send more data than we gave to it...
			if ((size_t)res >= header.size())  {  // The header was sent whole
				iDataSent += (size_t)res - header.size();
			} else
				bSentHeader = false; // Retry
		}


		return HTTP_PROC_PROCESSING;
	}

	// If we aren't ready yet, leave
	if(!bSocketReady || !bConnected || !bSentHeader)
		return HTTP_PROC_PROCESSING;

	// Wait until we have some data to read or write
	// HINT: this function is called in a separate thread so we can do this blocking call
	// TODO: this is blocking, fix that!! (it will block also the main thread if you call ShutdownThread())
	Unlock();
	tSocket.WaitForSocketReadOrWrite(100);
	Lock();

	// Check if we have a response
	// HINT: don't wait for sending all the data, if the server wants to refuse us, it will do it immediatelly
	int res = ReadAndProcessData();
	if (res == HTTP_PROC_ERROR)
		return HTTP_PROC_ERROR;

	// Finished?
	if (bTransferFinished)  {
		// We could get a 200 OK response even when not uploaded everything, who knows...
		if (iDataSent >= sDataToSend.size())  {
			fUploadEnd = GetTime();
			return HTTP_PROC_FINISHED;
		} else {
			bTransferFinished = false;
			SetHttpError(HTTP_BAD_RESPONSE);
			return HTTP_PROC_ERROR;
		}
	}

	// Wait until the network buffers are ready
	// HINT: ProcessPOST is called in a separate thread so we can do this blocking call
	// TODO: this is blocking, fix that!! (it will block also the main thread if you call ShutdownThread())
	Unlock();
	tSocket.WaitForSocketWrite(100);
	Lock();

	// Send another chunk
	res = tSocket.Write(sDataToSend.substr(iDataSent, MIN(HTTP_MAX_DATA_LEN, sDataToSend.size() - iDataSent)));
	fSocketActionTime = GetTime();

	// Error check
	if (res < 0)  {
		SetHttpError(HTTP_ERROR_SENDING_REQ);
		return HTTP_PROC_ERROR;
	}
	iDataSent += res;

	fUploadEnd = GetTime();  // Make the upload time correct while uploading

	return HTTP_PROC_PROCESSING;
}


//////////////////
// Returns result of the last call of the processing function
// TODO: get rid of this, use events instead
HttpProc_t CHttp::ProcessRequest()
{
	Lock();
	HttpProc_t res = iProcessingResult;
	Unlock();
	if (res != HTTP_PROC_PROCESSING)  {
		if (bThreadRunning)
			return HTTP_PROC_PROCESSING;
	}
	return res;
}

/////////////////
// Main processing function (called from the processing thread)
// Returns true if the processing should end
// NOTE: Locking is done automatically in this function
bool CHttp::ProcessInternal()
{
	// Lock
	ScopedLock lock(tMutex);

	// Check that there has been no error yet
	if (tError.iError != HTTP_NO_ERROR)  {
		iProcessingResult = HTTP_PROC_ERROR;
		return true;
	}

	// Process DNS resolving
	if (!IsNetAddrValid(tRemoteIP)) {
        AbsTime f = GetTime();
		bool error = false;

		// Error
		// TODO: this cannot be used like this because we're running in parallel with the main thread
		// and any other socket can set the error
		/*if (GetSocketErrorNr() != 0)  {
			SetHttpError(HTTP_CANNOT_RESOLVE_DNS);
			tError.sErrorMsg += " (" + GetSocketErrorStr(GetSocketErrorNr()) + ")";
			error = true;
		}*/

        // Timed out?
        if( (f - fResolveTime).seconds() > DNS_TIMEOUT) {
			SetHttpError(HTTP_DNS_TIMEOUT);
			error = true;
        }

		// If using proxy, try direct connection
		if (error)  {
			if (sProxyHost.size() != 0)  {
				warnings << "Http: proxy " << sProxyHost << " failed, trying a direct connection" << endl;
				if(sProxyHost != tLXOptions->sHttpProxy) notes << "System proxy is: " << tLXOptions->sHttpProxy << endl;		
				// The re-requesting must be done in the main thread, send a notification and quit
				m_thread->onRetry.pushToMainQueue( 
					SmartPointer<HttpRetryEventData>(new HttpRetryEventData(this, sHost + sUrl, sDataToSend)));
				iProcessingResult = HTTP_PROC_PROCESSING;
				return true;
			}

			iProcessingResult = HTTP_PROC_ERROR;
			return true;
		}

        // Still waiting for DNS resolution
		iProcessingResult = HTTP_PROC_PROCESSING;
        return false;
	}

	// Check for HTTP timeout
	if ( (GetTime() - fConnectTime).seconds() >= HTTP_TIMEOUT  && bConnected && !bRequested && !bSentHeader)  {
		// If using proxy, try direct connection
		if (sProxyHost.size() != 0)  {
			warnings << "Http: proxy " << sProxyHost << " failed, trying a direct connection" << endl;
			if(sProxyHost != tLXOptions->sHttpProxy) notes << "System proxy is: " << tLXOptions->sHttpProxy << endl;		
			// The re-requesting must be done in the main thread, send a notification and quit
			m_thread->onRetry.pushToMainQueue( 
				SmartPointer<HttpRetryEventData>(new HttpRetryEventData(this, sHost + sUrl, sDataToSend)));
			iProcessingResult = HTTP_PROC_PROCESSING;
			return true;
		} else { // Not using proxy, there's no other possibility to obtain the data
			SetHttpError(HTTP_ERROR_TIMEOUT);
			iProcessingResult = HTTP_PROC_ERROR;
			return true;
		}
	}

	// This can happen when the server stops responding in the middle of the transfer
	if (bRequested && (GetTime() - fSocketActionTime).seconds() >= HTTP_TIMEOUT)  {
		// If using proxy, try direct connection
		if (sProxyHost.size() != 0)  {
			warnings << "Http: proxy " << sProxyHost << " failed, trying a direct connection" << endl;
			if(sProxyHost != tLXOptions->sHttpProxy) notes << "System proxy is: " << tLXOptions->sHttpProxy << endl;		
			// The re-requesting must be done in the main thread, send a notification and quit
			m_thread->onRetry.pushToMainQueue( 
				SmartPointer<HttpRetryEventData>(new HttpRetryEventData(this, sHost + sUrl, sDataToSend)));
			iProcessingResult = HTTP_PROC_PROCESSING;
			return true;
		} else { // Not using proxy, there's no other possibility to obtain the data
			SetHttpError(HTTP_ERROR_TIMEOUT);
			iProcessingResult = HTTP_PROC_ERROR;
			return true;
		}
	}

	// If redirecting, quit the processing and wait for the event (sent in HandleRedirect) to start another request
	if (bRedirecting)  {
		iProcessingResult = HTTP_PROC_PROCESSING;
		return true;
	}


	// Make sure the socket is ready for writing
	if(!bSocketReady && bConnected) {
		if(tSocket.isReady())
			bSocketReady = true;
		else  {
			iProcessingResult = HTTP_PROC_PROCESSING;
			return false;
		}
	}


	// Check if the address completed resolving
	if(IsNetAddrValid(tRemoteIP)) {

		// Connect to the destination
		if(!bConnected) {
			// Default http port (80)
			SetNetAddrPort(tRemoteIP, 80);
			if (sProxyHost.size())
				SetNetAddrPort(tRemoteIP, iProxyPort);

			// Address was resolved; save it
			std::string host = sProxyHost.size() ? sProxyHost : sHost;
			NetworkAddr temp;
			if (!StringToNetAddr(host, temp))  // Add only if it is not an IP
				AddToDnsCache(host, tRemoteIP);

			if(!tSocket.Connect(tRemoteIP)) {
				if (sProxyHost.size() != 0)  { // If using proxy, try direct connection
					warnings << "Http: proxy " << sProxyHost << " failed, trying a direct connection" << endl;
					// The re-requesting must be done in the main thread, send a notification and quit
					m_thread->onRetry.pushToMainQueue( 
						SmartPointer<HttpRetryEventData>(new HttpRetryEventData(this, sHost + sUrl, sDataToSend)));
					iProcessingResult = HTTP_PROC_PROCESSING;
					return true;
				}

				SetHttpError(HTTP_NO_CONNECTION);
				iProcessingResult = HTTP_PROC_ERROR;
				return true;
			}

			bConnected = true;
			fConnectTime = GetTime();
		}

	} else {

		// Haven't resolved the address yet, so leave but let the
		// caller of this function keep processing us
		iProcessingResult = HTTP_PROC_PROCESSING;
		return false;
	}

	// Further processing depends on the transfer type
	switch (iAction)  {
	case htaGet:
		iProcessingResult = ProcessGET();
		return iProcessingResult != HTTP_PROC_PROCESSING;
	case htaPost:
		iProcessingResult = ProcessPOST();
		return iProcessingResult != HTTP_PROC_PROCESSING;
	case htaHead:
		errors << "HTTP HEAD not yet implemented" << endl;
	}

	iProcessingResult = HTTP_PROC_ERROR; // Should not happen
	return true;
}


#else // LIBCURL

CHttp::CHttp()
{
	curl = curl_easy_init();
	ThreadRunning = false;
	ThreadAborting = false;
	ProcessingResult = HTTP_PROC_FINISHED;
	DownloadStart = DownloadEnd = 0;
	curlForm = NULL;
};

CHttp::~CHttp()
{
	waitThreadFinish();
	curl_easy_cleanup(curl);
};

void CHttp::waitThreadFinish()
{
	while(true)
	{
		Mutex::ScopedLock l(Lock);
		if(!ThreadRunning)
		{
			return;
		}
		SDL_Delay(100);
	};
};

size_t CHttp::CurlReceiveCallback(void *ptr, size_t size, size_t nmemb, void *data)
{
	CHttp * parent = (CHttp *)data;
	size_t realsize = size * nmemb;
	
	Mutex::ScopedLock l(parent->Lock);
	
	parent->Data.append((const char *)ptr, realsize);
	
	if( parent->ThreadAborting )
		return 0;
	return realsize;
}

void CHttp::InitializeTransfer(const std::string& url, const std::string& proxy)
{
	waitThreadFinish();
	ThreadRunning = true;
	ThreadAborting = false;
	ProcessingResult = HTTP_PROC_PROCESSING;
	Url = url;
	Proxy = proxy;
	Useragent = GetFullGameName();
	Data = "";
	DownloadStart = DownloadEnd = tLX->currentTime;
	
	curl_easy_setopt(curl, CURLOPT_URL, Url.c_str());
	curl_easy_setopt(curl, CURLOPT_PROXY, Proxy.c_str());
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlReceiveCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)this);
	curl_easy_setopt(curl, CURLOPT_USERAGENT, Useragent.c_str());
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, (long)1);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, (long)HTTP_TIMEOUT);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, (long)HTTP_TIMEOUT);
	// I can set CURLOPT_PROGRESSFUNCTION but it's eating some resources
};

void CHttp::RequestData(const std::string& url, const std::string& proxy)
{
	InitializeTransfer(url, proxy);
	threadPool->start(new CurlThread(this), "CHttp: " + Url, true);
};

void CHttp::SendData(const std::list<HTTPPostField>& data, const std::string url, const std::string& proxy)
{
	InitializeTransfer(url, proxy);
	
	if( curlForm != NULL )
		curl_formfree(curlForm);
	curlForm = NULL;
	struct curl_httppost *lastptr=NULL;
	
	for( std::list<HTTPPostField> :: const_iterator it = data.begin(); it != data.end(); it++ )
	{
		if( it->getFileName() == "" )
			curl_formadd(	&curlForm,
							&lastptr,
							CURLFORM_COPYNAME, it->getName().c_str(),
							CURLFORM_CONTENTSLENGTH, it->getData().size(),
							CURLFORM_COPYCONTENTS, it->getData().c_str(),
							CURLFORM_END);
		else
			curl_formadd(	&curlForm,
							&lastptr,
							CURLFORM_COPYNAME, it->getName().c_str(),
							CURLFORM_FILENAME, it->getFileName().c_str(),
							CURLFORM_CONTENTTYPE, it->getMimeType().c_str(),
							CURLFORM_CONTENTSLENGTH, it->getData().size(),
							CURLFORM_COPYCONTENTS, it->getData().c_str(),
							CURLFORM_END);
	}
	
	curl_easy_setopt(curl, CURLOPT_HTTPPOST, curlForm);
	threadPool->start(new CurlThread(this), "CHttp: " + Url, true);
};

int CurlThread::handle()
{
	CURLcode res = curl_easy_perform(Parent->curl); // Blocks until processing finished

	Mutex::ScopedLock l(Parent->Lock);

	Parent->ProcessingResult = HTTP_PROC_FINISHED;
	Parent->Error.iError = HTTP_NO_ERROR;
	if( res != CURLE_OK )
	{
		Parent->ProcessingResult = HTTP_PROC_ERROR;
		Parent->Error.iError = res; // This is not HTTP error, it's libcurl error, but whatever
		Parent->Error.sErrorMsg = curl_easy_strerror(res);
	}
	Parent->ThreadRunning = false;
	Parent->DownloadEnd = tLX->currentTime;
	
	if( Parent->curlForm != NULL )
		curl_formfree(Parent->curlForm);
	Parent->curlForm = NULL;
	
	Parent->onFinished.occurred(CHttpBase::HttpEventData(Parent, Parent->ProcessingResult == HTTP_PROC_FINISHED));
	return 0;
}

void CHttp::CancelProcessing()
{
	{
		Mutex::ScopedLock l(Lock);
		ThreadAborting = true;
	}
	waitThreadFinish();
};

size_t CHttp::GetDataLength() const
{
	Mutex::ScopedLock l(const_cast<Mutex &>(Lock));
	long len = 0;
	curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &len);
	if( len < 0 )
		len = 0;
	return len;
};

std::string CHttp::GetMimeType() const
{
	Mutex::ScopedLock l(const_cast<Mutex &>(Lock));
	const char * c = NULL;
	curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, c);
	std::string ret;
	if( c != NULL )
		ret = c;
	return ret;
};

float CHttp::GetDownloadSpeed() const
{
	Mutex::ScopedLock l(const_cast<Mutex &>(Lock));
	double d = 0;
	curl_easy_getinfo(curl, CURLINFO_SPEED_DOWNLOAD, &d);
	return d;
};

float CHttp::GetUploadSpeed() const
{
	Mutex::ScopedLock l(const_cast<Mutex &>(Lock));
	double d = 0;
	curl_easy_getinfo(curl, CURLINFO_SPEED_UPLOAD, &d);
	return d;
};

std::string CHttp::GetHostName() const
{
	std::string sHost;
	size_t s = 0;
	if( Url.find("http://") != std::string::npos )
		s = Url.find("http://") + 7;

	size_t p = Url.find("/", s );
	if(p == std::string::npos) 
		sHost = Url.substr(s);
	else
		sHost = Url.substr(s, p-s);

	return sHost;
};

#endif

