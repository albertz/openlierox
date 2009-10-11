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
#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>

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


CHttp::CHttp()
{
	curl = curl_easy_init();
	ThreadRunning = false;
	ThreadAborting = false;
	ProcessingResult = HTTP_PROC_FINISHED;
	DownloadStart = DownloadEnd = 0;
	curlForm = NULL;
}

CHttp::~CHttp()
{
	waitThreadFinish();
	curl_easy_cleanup(curl);
}

void CHttp::waitThreadFinish()
{
	while(true) {
		{
			Mutex::ScopedLock l(Lock);
			if(!ThreadRunning)
				return;
		}
		SDL_Delay(100);
	}
}

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
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, (long)HTTP_TIMEOUT);
	//curl_easy_setopt(curl, CURLOPT_TIMEOUT, (long)HTTP_TIMEOUT); // Do not set this if you don't want abort in the middle of large transfer

	// I can set CURLOPT_PROGRESSFUNCTION but it's eating some resources
	// Also it's required to abort download quickly, otherwise CHTTP may hang I think
}

void CHttp::RequestData(const std::string& url, const std::string& proxy)
{
	InitializeTransfer(url, proxy);
	threadPool->start(new CurlThread(this), "CHttp: " + Url, true);
}

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
}

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
}

size_t CHttp::GetDataLength() const
{
	Mutex::ScopedLock l(const_cast<Mutex &>(Lock));
	long len = 0;
	curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &len);
	if( len < 0 )
		len = 0;
	return len;
}

std::string CHttp::GetMimeType() const
{
	Mutex::ScopedLock l(const_cast<Mutex &>(Lock));
	const char * c = NULL;
	curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, c);
	std::string ret;
	if( c != NULL )
		ret = c;
	return ret;
}

float CHttp::GetDownloadSpeed() const
{
	Mutex::ScopedLock l(const_cast<Mutex &>(Lock));
	double d = 0;
	curl_easy_getinfo(curl, CURLINFO_SPEED_DOWNLOAD, &d);
	return float(d);
}

float CHttp::GetUploadSpeed() const
{
	Mutex::ScopedLock l(const_cast<Mutex &>(Lock));
	double d = 0;
	curl_easy_getinfo(curl, CURLINFO_SPEED_UPLOAD, &d);
	return float(d);
}

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
}

