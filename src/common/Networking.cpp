/////////////////////////////////////////
//
//             Liero Xtreme
//
//     Copyright Auxiliary Software 2002
//
//
/////////////////////////////////////////


// COmmon networking routines to help us
// Created 18/12/02
// Jason Boettcher


#include "defs.h"
#include "LieroX.h"


// Random Number list
#include "RandomNumberList.h"



///////////////////
// Return a fixed random number
float GetFixedRandomNum(int index)
{
	// Safety
	assert( index >= 0 && index <= 255);

	return RandomNumbers[index];
}






/*
=============================

		HTTP functions

=============================
*/



NetworkAddr		http_RemoteAddress;
NetworkSocket		http_Socket = InvalidNetworkState;
bool			http_Connected;
bool			http_Requested;
bool			http_SocketReady;
char			http_url[1024];
char			http_host[1024];
char			http_content[1024];
float           http_ResolveTime = -9999;


///////////////////
// Initialize a HTTP get request
bool http_InitializeRequest(char *host, char *url)
{
	// Make the url http friendly (get rid of spaces)	
	//http_ConvertUrl(http_url, url);
	
    // Convert the host & url into a good set
    // Ie, '/'s from host goes into url
    http_CreateHostUrl(host, url);

    d_printf("_Sending request %s %s\n",http_host, http_url);

	memset(http_content, 0, 1024);

	
	// Open the socket
	http_Socket = OpenReliableSocket(0);
	if(http_Socket == InvalidNetworkState)
		return false;

	// Resolve the address
	SetNetAddrValid(&http_RemoteAddress, false);
    http_ResolveTime = (float)SDL_GetTicks() * 0.001f;
	GetNetAddrFromNameAsync( http_host, &http_RemoteAddress );

	http_Connected = false;
	http_Requested = false;
	http_SocketReady = false;

	return true;
}


///////////////////
// Process the HTTP get request
// Returns:
// -1 : failed
// 0  : still processing
// 1  : complete
int http_ProcessRequest(char *szError)
{
    if(szError)
        szError[0] = '\0';

	// Check if the address failed resolving in n seconds
	if(!IsNetAddrValid(&http_RemoteAddress)) {
        float f = (float)SDL_GetTicks() * 0.001f;
        // Timed out?
        if(f - http_ResolveTime > 1/*HTTP_TIMEOUT*/) {
            if(szError)
                strcpy(szError, "Could not resolve the address");
		    http_Quit();
		    return -1;
        }
	}

    
	// Make sure the socket is ready for writing
	if( !http_SocketReady && http_Connected) {
		if( WriteSocket(http_Socket, "", 0) >= 0) {
			http_SocketReady = true;
		}
		else {
			return 0;
		}
	}

	// Send a request
	if( http_SocketReady && http_Connected && !http_Requested) {
		http_Requested = true;
		if( !http_SendRequest() ) {
            if(szError)
                strcpy(szError, "Could not send the request");
			http_Quit();
			return -1;
		}
	}


	// Check if the address completed resolving
	if( IsNetAddrValid(&http_RemoteAddress) ) {
		
		// Default http port (80)
		SetNetAddrPort(&http_RemoteAddress, 80);

		// Connect to the destination
		if( !http_Connected ) {
			if(!ConnectSocket( http_Socket, &http_RemoteAddress )) {
                if(szError)
                    strcpy(szError, "Could not connect to the server");
				http_Quit();
				return -1;
			}

			http_Connected = true;
		}
	} else {

		// Havn't resolved the address yet, so leave but let the 
		// caller of this function keep processing us
		return 0;
	}

	
	// If we aren't ready yet, leave
	if(!http_SocketReady || !http_Connected || !http_Requested)		
		return 0;



	// Check if we have a response
	char data[1024];
	data[0] = 0;
	int count = ReadSocket(http_Socket, data, 1023);
	
	// Error, or end of connection?
	if( count < 0 ) {
		int err = GetSocketErrorNr();
		if( IsMessageEndSocketErrorNr(err) ) {
			// End of connection
			// Complete!
			http_Quit();
			return 1;
		} else {
			// Error
            if(szError)
                sprintf(szError, "NetError \"%s\"", GetSocketErrorStr(err));
			http_Quit();
			return -1;
		}
	}

	// Got some data
	if(count > 0) {
		if( strlen(http_content) + count < 1020 ) {
			data[count] = 0;
			strncat( http_content, data, count );
		}
	}

	// Still processing
	return 0;
}


///////////////////
// Send a request
bool http_SendRequest(void)
{
	char request[1024];

	// Build the url
	sprintf(request, "GET %s HTTP/1.0\nHost: %s\n\n", http_url,http_host);
	int count = WriteSocket( http_Socket, request, strlen(request) );

	// Anything written?
	if( count < 0 )
		return false;

	return true;
}


///////////////////
// Quit the http request
void http_Quit(void)
{
	if( http_Socket != InvalidNetworkState ) {
		CloseSocket(http_Socket);
		http_Socket = InvalidNetworkState;
	}

	http_RemoveHeader();
	http_content[1000] = 0;
}


///////////////////
// Convert the url into a friendly url (no spaces)
void http_ConvertUrl(char *dest, char *url)
{
	int i,j;
	char buffer[33];
	char c;


	dest[0] = 0;

	for( i=0,j=0; url[i]; i++) {
		c = url[i];
		
		if(url[i] == '_')
			c = ' ';
			

		if( isalnum(c) )
			dest[j++] = c;
		else {
			if( j<1020 ) {
				sprintf(buffer,"%X", c);
				dest[j] = '%';
				dest[j+1] = buffer[0];
				dest[j+2] = buffer[1];
			}
			j+=3;
		}
	}
}


///////////////////
// Create the host & url strings
void http_CreateHostUrl(char *host, char *url)
{
    strcpy(http_host, "");
    strcpy(http_url, "");

    // All characters up to a / goes into the host
	unsigned int i;
    for( i=0; i<strlen(host); i++ ) {
        if( host[i] != '/' )
            http_host[i] = host[i];
        else {
            strcpy(http_url, host+i);
            break;
        }
    }

    http_host[i] = '\0';
    strcat(http_url, url);
}


///////////////////
// Remove the http header from content downloaded
void http_RemoveHeader(void)
{
	int		lffound = 0;
	int		crfound = 0;
	char	temp[1024];

	memcpy(temp, http_content, 1024);

    int i;
	int count = (int)strlen(temp);
	for(i=0; i< count; i++) {

		if( temp[i] == 0x0D )
			crfound++;
		else {
			if( temp[i] == 0x0A )
				lffound++;
			else
				crfound = lffound = 0;
		}

		// End of the header
		if(lffound == 2) {
			strcpy(http_content, temp+i+1);
			break;
		}
	}
}


///////////////////
// Get the content buffer
char *http_GetContent(void)
{
	return http_content;
}


bool InitNetworkSystem() {
    if(!nlInit()) {
    	SystemError("nlInit failed");
    	return false;
    }
    
    if(!nlSelectNetwork(NL_IP)) {
        SystemError("could not select IP-based network");
		return false;
    }
	
	return true;
}

bool QuitNetworkSystem() {
	nlShutdown();
	return true;
}

NetworkSocket OpenReliableSocket(unsigned short port) {
	return nlOpen(port, NL_RELIABLE);
}

NetworkSocket OpenUnreliableSocket(unsigned short port) {
	return nlOpen(port, NL_UNRELIABLE);
}

NetworkSocket OpenBroadcastSocket(unsigned short port) {
	return nlOpen(port, NL_BROADCAST);
}

bool ConnectSocket(NetworkSocket sock, const NetworkAddr* addr) {
	return (nlConnect(sock, addr) != NL_FALSE);
}

bool ListenSocket(NetworkSocket sock) {
	return (nlListen(sock) != NL_FALSE);
}

bool CloseSocket(NetworkSocket sock) {
	return (nlClose(sock) != NL_FALSE);
}

int WriteSocket(NetworkSocket sock, const void* buffer, int nbytes) {
	return nlWrite(sock, buffer, nbytes);
}

int ReadSocket(NetworkSocket sock, void* buffer, int nbytes) {
	return nlRead(sock, buffer, nbytes);
}

int GetSocketErrorNr() {
	return nlGetError();
}

const char*	GetSocketErrorStr(int errnr) {
	return nlGetErrorStr(errnr);
}

bool IsMessageEndSocketErrorNr(int errnr) {
	return (errnr == NL_MESSAGE_END);
}

bool GetLocalNetAddr(NetworkSocket sock, NetworkAddr* addr) {
	return (nlGetLocalAddr(sock, addr) != NL_FALSE);
}

bool GetRemoteNetAddr(NetworkSocket sock, NetworkAddr* addr) {
	return (nlGetRemoteAddr(sock, addr) != NL_FALSE);
}

bool SetRemoteNetAddr(NetworkSocket sock, const NetworkAddr* addr) {
	return (nlSetRemoteAddr(sock, addr) != NL_FALSE);
}

bool IsNetAddrValid(NetworkAddr* addr) {
	if(addr)
		return (addr->valid != NL_FALSE);
	else
		return false;
}

bool SetNetAddrValid(NetworkAddr* addr, bool valid) {
	if(!addr) return false;
	addr->valid == valid ? NL_TRUE : NL_FALSE;
	return true;
}

bool StringToNetAddr(const char* string, NetworkAddr* addr) {
	return (nlStringToAddr(string, addr) != NL_FALSE);
}

bool NetAddrToString(const NetworkAddr* addr, char* string) {
	nlAddrToString(addr, string);
	return true; // TODO: check it
}

unsigned short GetNetAddrPort(NetworkAddr* addr) {
	return nlGetPortFromAddr(addr);
}

bool SetNetAddrPort(NetworkAddr* addr, unsigned short port) {
	return (nlSetAddrPort(addr, port) != NL_FALSE);
}

bool AreNetAddrEqual(const NetworkAddr* addr1, const NetworkAddr* addr2) {
	return (nlAddrCompare(addr1, addr2) != NL_FALSE);
}

bool GetNetAddrFromNameAsync(const char* name, NetworkAddr* addr) {
	return (nlGetAddrFromNameAsync(name, addr) != NL_FALSE);
}
