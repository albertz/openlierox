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
#include "LieroX.h" // for LX_VERSION
#include "Timer.h"
#include "StringUtils.h"
#include "types.h"

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
				printf("ParseChunks - atoi failed: \"" + sChunkLenStr + "\", " + itoa((uint)c) + "\n");
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

			// TODO: i got a segfault here:
			/*
			Program terminated with signal 11, Segmentation fault.
#0  0x0817bbcf in CChunkParser::ParseNext (this=0x8b4f200, c=32 ' ')
    at /home/az/Programmierung/openlierox/src/common/HTTP.cpp:80
80                              (*iFinalLength) += iCurLength;

Thread 1 (process 1273):
#0  0x0817bbcf in CChunkParser::ParseNext (this=0x8b4f200, c=32 ' ')
    at /home/az/Programmierung/openlierox/src/common/HTTP.cpp:80
        failed = false
#1  0x0817bd32 in CHttp::ParseChunks (this=0xbfbc6f7c)
    at /home/az/Programmierung/openlierox/src/common/HTTP.cpp:421
        chunk_it = {_M_current = 0x8c04c66 " "}
#2  0x0817bf47 in CHttp::ProcessData (this=0xbfbc6f7c)
    at /home/az/Programmierung/openlierox/src/common/HTTP.cpp:366
        header_end_len = 4 '\004'
        header_end = 289
#3  0x0817c3ae in CHttp::ProcessRequest (this=0xbfbc6f7c)
    at /home/az/Programmierung/openlierox/src/common/HTTP.cpp:589
        count = 4
#4  0x08095fc9 in Menu_Net_NETUpdateList ()
    at /home/az/Programmierung/openlierox/src/client/Menu_Net_Internet.cpp:618
        cListUpdate = {cWidgets = 0x8c02de8, tEvent = 0x8b4f078, 
  cFocused = 0x0, cMouseOverWidget = 0x0, iID = -1, nMouseButtons = 0, 
  fMouseNext = {-9999, -9999, -9999}, iCanFocus = 1}
        ev = (gui_event_t *) 0x0
        updateList = true
        http_result = 0
        SvrCount = 3
        CurServer = 1
        SentRequest = true
        fp = (FILE *) 0x8c02308
        http = {sHost = {static npos = 4294967295, ......
			*/
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
	// Buffer for reading from socket, each instance has its own buffer (thread safety)
	tBuffer = new char[4096];
	tChunkParser = new CChunkParser(&sPureData, &iDataLength, &iDataReceived);
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
	sRemoteAddress = "";
	sData = "";
	sPureData = "";
	sHeader = "";
	sMimeType = "";
	SetHttpError(HTTP_NO_ERROR);
	iDataLength = 0;
	iDataReceived = 0;
	bTransferFinished = false;
	bConnected = false;
	bRequested = false;
	bGotHttpHeader = false;
	bSocketReady = false;
	bChunkedTransfer = false;
	fResolveTime = -9999;
	ResetNetAddr(&tRemoteIP);
	InvalidateSocketState(tSocket);
	if (tChunkParser)
		tChunkParser->Reset();
	ResetSocketError();  // To prevent quitting when previous request failed
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

	size_t p = addr.find("/");
	if(p == std::string::npos)
		sHost = addr;
	else {
		sHost = addr.substr(0, p);
		sUrl = addr.substr(p);
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

	// Remove http:// (not needed, we know we use http)
	if (dest.size() > 9)
		if (stringcasecmp(dest.substr(0, 9), "http%3a//") == 0)  {  // HINT - ":" is already URL-encoded
			dest.erase(0, 9);
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
	request += "Host: " + sHost + "\r\n";
	request += "User-Agent: OpenLieroX/" + std::string(LX_VERSION) + "\r\n";
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

			sPureData += sData;  // No parsing, just add them
			sData = ""; // HINT: save memory, don't keep the data twice
		}
	}

	// We only strip the header and parse it here
	// If it has been already parsed, there's no work for us
	if (bGotHttpHeader)
		return;

	// Find the end of the header
	uchar header_end_len = 2;
	size_t header_end = sData.find("\n\n");  // Two LFs...
	if (header_end == std::string::npos)  {
		header_end = sData.find("\r\n\r\n"); // ... or two CR/LFs
		header_end_len = 4;
	}
	if (header_end == std::string::npos)  {
		header_end = sData.find("\n\r\n\r"); // ... or two LF/CRs
		header_end_len = 4;
	}

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
	if (tError.iError != HTTP_NO_ERROR)
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
				if (*i2 == '\n') i2++;

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
				break;  // Finished!
			}
	}

	// Remove the parsed data
	sData = "";
	
	// There could be still some footers, but we don't care about them
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
		if (*i == ' ')  {
			i++;
			break;
		}

		code += *i;
		i++;
	}
	
	// Check the code
	if (code[0] != '2') {  // 2XX = success
		if (code == "100")  {  // 100 Continue - we should wait for a real header
			bGotHttpHeader = false;
			return;
		}

		// Other errors
		tError.iError = atoi(code);
		tError.sErrorMsg = std::string(i, PositionToIterator(sHeader, sHeader.find('\n')));  // From error code to the end of the line
		return;
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
	bChunkedTransfer = stringcasecmp(GetPropertyFromHeader("Transfer-Encoding"), "chunked") == 0;
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

		// Error
		if (GetSocketErrorNr() != 0)  {
			SetHttpError(HTTP_CANNOT_RESOLVE_DNS);
			tError.sErrorMsg += " (" + GetSocketErrorStr(GetSocketErrorNr()) + ")";
			return HTTP_PROC_ERROR;
		}

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
	int count = 0;
	if (tBuffer != NULL)  {
		while (true)  {
			count = ReadSocket(tSocket, tBuffer, sizeof(tBuffer));
			if (count <= 0)
				break;

			// Got some data
			sData.append(tBuffer, count);
			iDataReceived += count;
			ProcessData();
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
			return HTTP_PROC_FINISHED;
		} else {
			// Error
			SetHttpError(HTTP_NET_ERROR);
			tError.sErrorMsg += GetSocketErrorStr(err);
			return HTTP_PROC_ERROR;
		}
	}

	// Transfer fininshed (can get here if chunk message end happened)
	if (bTransferFinished)  {
		return HTTP_PROC_FINISHED;
	}

	// Still processing
	return HTTP_PROC_PROCESSING;
}
