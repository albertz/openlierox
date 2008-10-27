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

#include <iostream>
#include <assert.h>
#ifdef WIN32
#include <windows.h>
#include <wininet.h>
#else
#include <stdlib.h>
#endif

#include "LieroX.h"

#include "Options.h"
#include "HTTP.h"
#include "Timer.h"
#include "StringUtils.h"
#include "types.h"
#include "Version.h"
#include "MathLib.h"


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

//
// Functions
//

/////////////////
// Automatically updates tLXOptions->sHttpProxy with proxy settings retrieved from the system
void AutoSetupHTTPProxy()
{
	// User doesn't wish an automatic proxy setup
	if (!tLXOptions->bAutoSetupHttpProxy)
		return;

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
	if(!InternetQueryOption(NULL, INTERNET_OPTION_PER_CONNECTION_OPTION, &List, &size))
		return;

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

			std::cout << "Using HTTP proxy: " << tLXOptions->sHttpProxy << std::endl;
		}
	} else {
		tLXOptions->sHttpProxy = ""; // No proxy
	}

	// Cleanup
	if(Options[0].Value.pszValue != NULL)
		GlobalFree(Options[0].Value.pszValue);

#else
#ifdef linux
	// Linux has numerous configuration of proxies for each application, but environment var seems to be the most common
	const char * c_proxy = getenv("http_proxy");
	if( c_proxy == NULL )  {
		c_proxy = getenv("HTTP_PROXY");
		if( c_proxy == NULL )
			return;
	}

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
	if( proxy.size() == 0 )  {
		tLXOptions->sHttpProxy = "";
		return;
	}

	// Remove trailing slash
	if( *proxy.rbegin() == '/')
		proxy.resize( proxy.size() - 1 );
	tLXOptions->sHttpProxy = proxy;
#else
	// TODO: similar for other systems if possible
#endif
#endif
}

//
// Chunk parser class
//

//////////////
// Contructor
CChunkParser::CChunkParser(std::string *pure_data, size_t *final_length, size_t *received)
{
	Reset();
	sPureData = pure_data;
	iFinalLength = final_length;
	iReceived = received;
}

//////////////////
// Parse a next character from the received data
// NOTE: the caller is responsible for a valid iterator
bool CChunkParser::ParseNext(char c)
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
				//std::cout << "ParseChunks - atoi failed: \"" << sChunkLenStr << "\", " << itoa((uint)c) << std::endl;
#endif
				iCurLength = 0;
				return false;  // Still processing
			}
			sChunkLenStr = "";  // Not needed anymore

			// End?
			if (iCurLength == 0)  {  // 0-length chunk = end of the chunk message
				iState = CHPAR_SKIPBLANK; // Not needed, but if someone continued calling us, this is what we should do
				iNextState = CHPAR_FOOTERREAD;
				return true;  // Finished!
			}
			(*iFinalLength) += iCurLength;

			// Change the state
			iState = CHPAR_SKIPBLANK;
			iNextState = CHPAR_DATAREAD;
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

			// Change the state
			iState = CHPAR_SKIPBLANK;
			iNextState = CHPAR_LENREAD;
		}
	return false; // Still processing

	// Skip blank characters
	case CHPAR_SKIPBLANK:
		if (!isspace((uchar)c))  {  // Not a blank character, we finished our job
			iState = iNextState; // Change the state
			ParseNext(c);  // Parse the non-blank character
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
void CChunkParser::Reset()
{
	iState = CHPAR_SKIPBLANK;
	iNextState = CHPAR_LENREAD;
	sChunkLenStr = "";
	iCurRead = 0;
	iCurLength = 0;
}


//
// HTTP class
//

//////////////
// Constructor
CHttp::CHttp()
{
	// Buffer for reading from socket
	tBuffer = new char[BUFFER_LEN];
	tChunkParser = new CChunkParser(&sPureData, &iDataLength, &iDataReceived);
	InvalidateSocketState(tSocket);
	Clear();
}

//////////////
// Destructor
CHttp::~CHttp()
{
	if (tBuffer)
		delete[] tBuffer;
	if (tChunkParser)
		delete tChunkParser;
	tBuffer = NULL;
	tChunkParser = NULL;
	CancelProcessing();
}

//////////////
// Clear everything
void CHttp::Clear()
{
	sHost = "";
	sUrl = "";
	sProxyHost = "";
	iProxyPort = 0;
	sProxyUser = "";
	sProxyPasswd = "";
	sRemoteAddress = "";
	sData = "";
	sPureData = "";
	sHeader = "";
	sMimeType = "";
	SetHttpError(HTTP_NO_ERROR);
	iDataLength = 0;
	iDataReceived = 0;
	bActive = false;
	bTransferFinished = false;
	bConnected = false;
	bRequested = false;
	bGotHttpHeader = false;
	bSocketReady = false;
	bChunkedTransfer = false;
	bRedirecting = false;
	iRedirectCode = 0;
	fResolveTime = -9999;
	fConnectTime = -9999;
	fReceiveTime = -9999;
	ResetNetAddr(tRemoteIP);
	if( IsSocketStateValid(tSocket) ) {
		CloseSocket(tSocket);
	}
	InvalidateSocketState(tSocket);
	if (tChunkParser)
		tChunkParser->Reset();
	ResetSocketError();  // To prevent quitting when previous request failed
}

///////////////
// Cancel current request
void CHttp::CancelProcessing()
{
	// Wait for reply from DNS server, else we could get in trouble with memory
	if (bActive && !IsSocketStateValid(tSocket))  {
		printf("HTTP Stop: Waiting for DNS reply...\n");
		float start = GetMilliSeconds();
		while (!IsSocketStateValid(tSocket) && (GetMilliSeconds() - start) <= 10) {
			SDL_Delay(10);
		}
	}

	// If we got DNS reply, just quit
	if ((bConnected || bRequested) && IsSocketStateValid(tSocket))  {
		CloseSocket(tSocket);
		InvalidateSocketState(tSocket);
	}



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

//////////////
// Request data from server
void CHttp::RequestData(const std::string& address, const std::string & proxy)
{
	// Stop any previous transfers
	CancelProcessing();

	// Make the urls http friendly (get rid of spaces)
	if (!AdjustUrl(sRemoteAddress, address))  {
		SetHttpError(HTTP_INVALID_URL);
		return;
	}

    // Convert the address to host & url
    // Ie, '/'s from host goes into url
    ParseAddress(sRemoteAddress);
	ParseProxyAddress(proxy);	// Fills sProxyHost, iProxyPort, sProxyPasswd and sProxyUser

	//std::cout << "Sending HTTP request " << address << "\n";

	// Open the socket
	tSocket = OpenReliableSocket(0);
	if(!IsSocketStateValid(tSocket))  {
		SetHttpError(HTTP_NO_SOCKET_ERROR);
		return;
	}

	// Resolve the address
	// Reset the current adr; sometimes needed (hack? bug in hawknl?)
	ResetNetAddr(tRemoteIP);
    fResolveTime = GetMilliSeconds();
	std::string host = sHost;
	if( sProxyHost.size() != 0 )
		host = sProxyHost;

	// Try if an IP has been entered
	if (!StringToNetAddr(host, tRemoteIP))  {
		ResetSocketError(); // Clear the BAD_ADDR error

		// Not an IP, use DNS
		if(!GetNetAddrFromNameAsync(host, tRemoteIP)) {
			SetHttpError(HTTP_CANNOT_RESOLVE_DNS);
			tError.sErrorMsg += GetSocketErrorStr(GetSocketErrorNr());
			return;
		}
	}

	// We're active now
	bActive = true;
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
// Send the HTTP request
bool CHttp::SendRequest()
{
	std::string request;

	// Build the url
	request = "GET " + sUrl + " HTTP/1.1\r\n";
	if( sProxyHost.size() != 0 )  // Proxy servers require full URL in GET header (so they know where to forward the request)
		request = "GET http://" + sHost + sUrl + " HTTP/1.1\r\n";
	if( sProxyPasswd.size() != 0 || sProxyUser.size() != 0 )	// Don't know their order, guess Proxy-Authorization should be first header
		request += "Proxy-Authorization: " + GetBasicAuthentication(sProxyUser, sProxyPasswd) + "\r\n";
	request += "Host: " + sHost + "\r\n";
	request += "User-Agent: " + GetFullGameName() + "\r\n";
	request += "Connection: close\r\n\r\n";  // We currently don't support persistent connections
	return WriteSocket(tSocket, request) > 0;  // Anything written?
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
	size_t header_end1 = sData.find("\n\n");  // Two LFs...
	size_t header_end2 = sData.find("\r\n\r\n"); // ... or two CR/LFs
	size_t header_end3 = sData.find("\n\r\n\r"); // ... or two LF/CRs

	if (header_end1 == std::string::npos) header_end1 = sData.size();
	if (header_end2 == std::string::npos) header_end2 = sData.size();
	if (header_end3 == std::string::npos) header_end3 = sData.size();
	size_t header_end = MIN(MIN(header_end1, header_end2), header_end3);

	// Found the end of the header?
	if (header_end == sData.size())
		return;

	if (header_end == header_end2 || header_end == header_end3)
		header_end_len = 4;
	

	if (header_end == std::string::npos)  {  // No header is present
		return;
	} else {
		header_end += header_end_len; // Two LFs or two CR/LFs
		if (header_end > sData.size())  // Should not happen...
			return;

		sHeader = sData.substr(0, header_end);
		sData.erase(0, header_end);

		// The header is ok :)
		bGotHttpHeader = true;
	}

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
			RequestData(location, tLXOptions->sHttpProxy);
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
			printf("HTTP notice: redirected from " + sHost + sUrl + " to " + location + "\n");
			RequestData(location, tLXOptions->sHttpProxy);
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
			printf("HTTP notice: redirected from " + sHost + sUrl + " to " + location + "\n");
			RequestData(location, tLXOptions->sHttpProxy);
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
			RequestData(sHost + sUrl, location);
			printf("HTTP notice: accessing the desired location using proxy: " + location + "\n");
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
			CloseSocket(tSocket);
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

/////////////////
// Main processing function
int CHttp::ProcessRequest()
{
	// Check that there has been no error yet
	if (tError.iError != HTTP_NO_ERROR)
		return HTTP_PROC_ERROR;

	// Process DNS resolving
	if(!IsNetAddrValid(tRemoteIP)) {
        float f = GetMilliSeconds();
		bool error = false;

		// Error
		if (GetSocketErrorNr() != 0)  {
			SetHttpError(HTTP_CANNOT_RESOLVE_DNS);
			tError.sErrorMsg += " (" + GetSocketErrorStr(GetSocketErrorNr()) + ")";
			error = true;
		}

        // Timed out?
        if(f - fResolveTime > DNS_TIMEOUT) {
			SetHttpError(HTTP_DNS_TIMEOUT);
			error = true;
        }

		// If using proxy, try direct connection
		if (error)  {
			if (sProxyHost.size() != 0)  {
				printf("HINT: proxy failed, trying a direct connection\n");
				RequestData(sHost + sUrl);
				return HTTP_PROC_PROCESSING;
			}

			return HTTP_PROC_ERROR;
		}

        // Still waiting for DNS resolution
        return HTTP_PROC_PROCESSING;
	}

	// Check for HTTP timeout
	if (GetMilliSeconds() - fConnectTime >= HTTP_TIMEOUT  && bConnected && !bRequested)  {
		// If using proxy, try direct connection
		if (sProxyHost.size() != 0)  {
			printf("HINT: proxy failed, trying a direct connection\n");
			RequestData(sHost + sUrl);
			return HTTP_PROC_PROCESSING;
		} else { // Not using proxy, there's no other possibility to obtain the data
			SetHttpError(HTTP_ERROR_TIMEOUT);
			return HTTP_PROC_ERROR;
		}
	}

	// This can happen when the server stops responding in the middle of the transfer
	if (bRequested && GetMilliSeconds() - fReceiveTime >= HTTP_TIMEOUT)  {
		// If using proxy, try direct connection
		if (sProxyHost.size() != 0)  {
			printf("HINT: proxy failed, trying a direct connection\n");
			RequestData(sHost + sUrl);
			return HTTP_PROC_PROCESSING;
		} else { // Not using proxy, there's no other possibility to obtain the data
			SetHttpError(HTTP_ERROR_TIMEOUT);
			return HTTP_PROC_ERROR;
		}
	}


	// Make sure the socket is ready for writing
	if(!bSocketReady && bConnected) {
		if(IsSocketReady(tSocket))
			bSocketReady = true;
		else
			return HTTP_PROC_PROCESSING;
	}

	// Send a request
	if(bSocketReady && bConnected && !bRequested) {
		bRequested = true;
		fReceiveTime = GetMilliSeconds();

		if(!SendRequest()) {
            SetHttpError(HTTP_ERROR_SENDING_REQ);
			return HTTP_PROC_ERROR;
		}
	}


	// Check if the address completed resolving
	if(IsNetAddrValid(tRemoteIP)) {

		// Default http port (80)
		SetNetAddrPort(tRemoteIP, 80);
		if( sProxyHost != "" )
			SetNetAddrPort(tRemoteIP, iProxyPort);

		// Connect to the destination
		if(!bConnected) {
			// Address was resolved; save it
			std::string host = sProxyHost.size() ? sProxyHost : sHost;
			NetworkAddr temp;
			if (!StringToNetAddr(host, temp))  // Add only if it is not an IP
				AddToDnsCache(host, tRemoteIP);

			if(!ConnectSocket(tSocket, tRemoteIP)) {
				if (sProxyHost.size() != 0)  { // If using proxy, try direct connection
					printf("HINT: proxy failed, trying a direct connection\n");
					RequestData(sHost + sUrl);
					return HTTP_PROC_PROCESSING;
				}

				SetHttpError(HTTP_NO_CONNECTION);
				return HTTP_PROC_ERROR;
			}

			bConnected = true;
			fConnectTime = GetMilliSeconds();
		}

	} else {

		// Haven't resolved the address yet, so leave but let the
		// caller of this function keep processing us
		return HTTP_PROC_PROCESSING;
	}


	// If we aren't ready yet, leave
	if(!bSocketReady || !bConnected || !bRequested)
		return HTTP_PROC_PROCESSING;

	// Check if we have a response
	int count = 0;
	if (tBuffer != NULL)  {
		while (true)  {
			count = ReadSocket(tSocket, tBuffer, BUFFER_LEN);
			if (count <= 0)
				break;

			// Got some data
			sData.append(tBuffer, count);
			iDataReceived += count;
			ProcessData();
			fReceiveTime = GetMilliSeconds();
		}
	}

	// Some HTTP errors?
	if (tError.iError != HTTP_NO_ERROR)
		return HTTP_PROC_ERROR;

	// Error, or end of connection?
	if(count < 0) {
		int err = GetSocketErrorNr();
		if(IsMessageEndSocketErrorNr(err) ) {
			// End of connection
			// Complete!
			bTransferFinished = true;
			bActive = false;

		} else {
			// Error
			SetHttpError(HTTP_NET_ERROR);
			tError.sErrorMsg += GetSocketErrorStr(err);
			return HTTP_PROC_ERROR;
		}
	}

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
