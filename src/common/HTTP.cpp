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
#include "HTTP.h"
#include "Timer.h"
#include "StringUtils.h"
#include "Types.h"

// List of errors, MUST match error IDs in HTTP.h
const std::string sHttpErrors[] = {
	"No error",
	"Could not open HTTP socket",
	"Could not resolve DNS",
	"Invalid URL",
	"DNS timeout",
	"Error sending HTTP request",
	"Could not connect to the server",
	"Network error: "
};

//////////////
// Constructor
CHttp::CHttp()
{
	Clear();
}

//////////////
// Destructor
CHttp::~CHttp()
{
	CancelProcessing();
}

//////////////
// Clear everything
void CHttp::Clear()
{
	sHost = "";
	sUrl = "";
	sRemoteAddress = "";
	sData = "";
	sHeader = "";
	sMimeType = "";
	SetHttpError(HTTP_NO_ERROR);
	bConnected = false;
	bRequested = false;
	bSocketReady = false;
	fResolveTime = -9999;
	ResetNetAddr(&tRemoteIP);
	InvalidateSocketState(tSocket);
}

///////////////
// Cancel current request
void CHttp::CancelProcessing()
{
	if (bRequested && IsSocketStateValid(tSocket))  {
		CloseSocket(tSocket);
		InvalidateSocketState(tSocket);
	}

	Clear();
}

///////////////
// Set the HTTP error
void CHttp::SetHttpError(int id)
{
	tError.iError = id;
	tError.sErrorMsg = sHttpErrors[id];
}

//////////////
// Request data from server
void CHttp::RequestData(const std::string& address)
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

	std::cout << "Sending HTTP request " << address << "\n";
	
	// Open the socket
	tSocket = OpenReliableSocket(0);
	if(!IsSocketStateValid(tSocket))  {
		SetHttpError(HTTP_NO_SOCKET_ERROR);
		return;
	}

	// Resolve the address
	// Reset the current adr; sometimes needed (hack? bug in hawknl?)
	ResetNetAddr(&tRemoteIP);
    fResolveTime = GetMilliSeconds();
	if(!GetNetAddrFromNameAsync(sHost, &tRemoteIP)) {
		SetHttpError(HTTP_CANNOT_RESOLVE_DNS);
		tError.sErrorMsg += GetSocketErrorStr(GetSocketErrorNr());
		return;
	}
}

/////////////////
// Parse the address given by user
void CHttp::ParseAddress(const std::string& addr)
{
    sHost = "";
    sUrl = "";

    // All characters up to a / goes into the host
    std::string::const_iterator it = addr.begin();
    for(size_t i=0; it != addr.end(); i++, it++ ) {
        if( *it == '/' ) {
			sHost = addr.substr(0, i);
			sUrl = addr.substr(i);
            break;
        }
    }
}

/////////////////
// Adjust the given URL
bool CHttp::AdjustUrl(std::string &dest, const std::string &url)
{
	dest = "";

	const std::string dont_encode = "-_.!~*'()&?/="; // Characters that won't be encoded in any way

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

	// Everything lowercase
	stringlwr(dest);

	// Remove double slashes
	while (replace(dest, "//", "/")) {}
	// Except for http://...
	replace(dest, "http:/", "http://");

	return true;
}

/////////////////
// Send the HTTP request
bool CHttp::SendRequest()
{
	std::string request;

	// Build the url
	request = "GET " + sUrl + " HTTP/1.0\n";
	request += "Host: " + sHost + "\n\n";
	return WriteSocket(tSocket, request) > 0;  // Anything written?
}

/////////////////
// Process the received data
void CHttp::ProcessData()
{
	ushort	lffound = 0;
	ushort	crfound = 0;

    std::string::const_iterator it = sData.begin();
	for(size_t i=1; it != sData.end(); i++, it++) {

		if( *it == 0x0D )
			crfound++;
		else {
			if( *it == 0x0A )
				lffound++;
			else
				crfound = lffound = 0;
		}

		// End of the header
		if(lffound == 2) {
			sHeader = sData.substr(0, i);
			sData.erase(0, i);
			break;
		}
	}
	
	// Process the header

	// Find the mime-type
	size_t mt_pos = sHeader.find("Content-Type:");
	if (mt_pos == std::string::npos)  {  // Mime-type not found
		printf("HTTP Warning: no mime-type specified in header, assuming text/plain\n");
		sMimeType = "text/plain";
	} else {  // Mime-type found
		mt_pos += std::string("Content-Type: ").size();

		// Check
		if (mt_pos >= sHeader.size())  {
			printf("HTTP Warning: no mime-type specified in header, assuming text/plain\n");
			sMimeType = "text/plain";
			return;
		}

		// Get the mime-type
		sMimeType = "";
		std::string::iterator i = PositionToIterator(sHeader, mt_pos);
		for (; i != sHeader.end(); i++)  {
			if (isalpha((uchar)*i) || *i == '/')
				sMimeType += *i;
			else
				break;
		}
	}
}

/////////////////
// Main processing function
int CHttp::ProcessRequest()
{
	// Check that there has been no error yet
	if (tError.iError != HTTP_NO_ERROR)
		return HTTP_PROC_ERROR;

	// Process DNS resolving
	if(!IsNetAddrValid(&tRemoteIP)) {
        float f = GetMilliSeconds();

        // Timed out?
        if(f - fResolveTime > DNS_TIMEOUT) {
            SetHttpError(HTTP_DNS_TIMEOUT);
		    return HTTP_PROC_ERROR;
        }

        // Still waiting for DNS resolution
        return HTTP_PROC_PROCESSING;
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
		if(!SendRequest()) {
            SetHttpError(HTTP_ERROR_SENDING_REQ);
			return HTTP_PROC_ERROR;
		}
	}


	// Check if the address completed resolving
	if(IsNetAddrValid(&tRemoteIP)) {
		
		// Default http port (80)
		SetNetAddrPort(&tRemoteIP, 80);

		// Connect to the destination
		if(!bConnected) {
			// Address was resolved; save it
			AddToDnsCache(sHost, &tRemoteIP);
		
			if(!ConnectSocket(tSocket, &tRemoteIP)) {
				SetHttpError(HTTP_NO_CONNECTION);
				return HTTP_PROC_ERROR;
			}

			bConnected = true;
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
	static char buffer[4096];
	buffer[0] = '\0';
	int count = ReadSocket(tSocket, buffer, sizeof(buffer));
	
	// Error, or end of connection?
	if(count < 0) {
		int err = GetSocketErrorNr();
		if(IsMessageEndSocketErrorNr(err) ) {
			// End of connection
			// Complete!
			ProcessData();
			return HTTP_PROC_FINISHED;
		} else {
			// Error
			SetHttpError(HTTP_NET_ERROR);
			tError.sErrorMsg += GetSocketErrorStr(err);
			return HTTP_PROC_ERROR;
		}
	}

	// Got some data
	if(count > 0)
		sData.append(buffer, count);

	// Still processing
	return HTTP_PROC_PROCESSING;
}
