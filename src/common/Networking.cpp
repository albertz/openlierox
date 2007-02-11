/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Common networking routines to help us
// Created 18/12/02
// Jason Boettcher

#include "defs.h"
#include "LieroX.h"

#include <map>

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
NetworkSocket	http_Socket;
bool			http_Connected;
bool			http_Requested;
bool			http_SocketReady;
char			http_url[1024];
char			http_host[1024];
char			http_content[HTTP_CONTENT_LEN];
float           http_ResolveTime = -9999;



void http_Init() {
	SetSocketStateValid(http_Socket, false);
	// TODO ...
}


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

	memset(http_content, 0, sizeof(http_content));

	
	// Open the socket
	http_Socket = OpenReliableSocket(0);
	if(!IsSocketStateValid(http_Socket))
		return false;

	// Resolve the address
	// reset the current adr; sometimes needed (hack? bug in hawknl?)
	memset(&http_RemoteAddress, 0, sizeof(http_RemoteAddress));
	SetNetAddrValid(&http_RemoteAddress, false);
    http_ResolveTime = (float)SDL_GetTicks() * 0.001f;
	if(!GetNetAddrFromNameAsync( http_host, &http_RemoteAddress )) {
		printf("ERROR: cannot start resolving DNS: ");
		printf("%s\n", GetSocketErrorStr(GetSocketErrorNr()));
	}

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
        if(f - http_ResolveTime > 10 /*HTTP_TIMEOUT*/) {
            if(szError) {
                strcpy(szError, "Could not resolve the address: ");
            }
		    http_Quit();
		    return -1;
        }
        // still waiting for dns resolution
        return 0;
	}

    
	// Make sure the socket is ready for writing
	if( !http_SocketReady && http_Connected) {
		if( WriteSocket(http_Socket, "", 0) >= 0) {
			http_SocketReady = true;
		}
		else {
			//MessageBox(0,"Socket not ready!","Debug",MB_OK);
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
			// adr was resolved; save it
			AddToDnsCache(http_host, &http_RemoteAddress);
		
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
		//MessageBox(0,"Address not yet resolved!","Debug",MB_OK);
		return 0;
	}

	
	// If we aren't ready yet, leave
	if(!http_SocketReady || !http_Connected || !http_Requested)	 {
		//char buf[64];
		//sprintf(buf,"Not ready: http_SocketReady = %i, http_Connected = %i, http_Requested = %i",http_SocketReady,http_Connected,http_Requested);
		//MessageBox(0,buf,"Debug",MB_OK);
		return 0;
	}



	// Check if we have a response
	static char data[1024];
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
		if( fix_strnlen(http_content) + count < 1020 ) {
			data[count] = '\0';
			fix_strncat( http_content, data );
		}
	}

	// Still processing
	return 0;
}


///////////////////
// Send a request
bool http_SendRequest(void)
{
	static char request[1024];

	// Build the url
	snprintf(request, sizeof(request), "GET %s HTTP/1.0\nHost: %s\n\n", http_url,http_host);
	int count = WriteSocket( http_Socket, request, fix_strnlen(request) );

	// Anything written?
	if( count < 0 )
		return false;

	return true;
}


///////////////////
// Quit the http request
void http_Quit(void)
{
	if( IsSocketStateValid(http_Socket) ) {
		CloseSocket(http_Socket);
		SetSocketStateValid(http_Socket, false);
	}

	http_RemoveHeader();
	http_content[1000] = 0;
}


///////////////////
// Convert the url into a friendly url (no spaces)
void http_ConvertUrl(char *dest, char *url)
{
	size_t i,j;
	char buffer[3];
	char c;


	dest[0] = 0;

	for( i=0,j=0; url[i]; i++) {
		c = url[i];
		
//		if(url[i] == '_')
//			c = ' ';
			

		if( isalnum(c) )
			dest[j++] = c;
		else {
			if( j<1020 ) {
				snprintf(buffer,sizeof(buffer),"%X",c);
				dest[j] = '%';
				dest[j+1] = buffer[0];
				dest[j+2] = buffer[1];
			}
			j+=3;
		}
	}
	dest[j] = '\0';
}


///////////////////
// Create the host & url strings
// host is the beginning of an URL, url is the end (to be appended)
void http_CreateHostUrl(char *host, char *url)
{
    http_host[0] = '\0';
    http_url[0] = '\0';

    // All characters up to a / goes into the host
	size_t i;
    size_t len = strnlen(host,sizeof(http_host));
    for( i=0; i<len; i++ ) {
        if( host[i] != '/' )
            http_host[i] = host[i];
        else {
            fix_strncpy(http_url, host+i);
            break;
        }
    }

    http_host[MIN(i,sizeof(http_host)-1)] = '\0';
    fix_strncat(http_url, url);
}


///////////////////
// Remove the http header from content downloaded
void http_RemoveHeader(void)
{
	short	lffound = 0;
	short	crfound = 0;
	static char	temp[1024];

	memcpy(temp, http_content, 1024);

    size_t i;
	size_t count = fix_strnlen(temp);
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
			fix_strncpy(http_content, &temp[i+1]);
			break;
		}
	}
}


///////////////////
// Get the content buffer
char *http_GetContent(void)
{
	http_content[sizeof(http_content)-1] = '\0';
	return http_content;
}



/////////////////////
// Initializes network
bool InitNetworkSystem() {
    if(!nlInit()) {
    	SystemError("nlInit failed");
    	return false;
    }
    
    if(!nlSelectNetwork(NL_IP)) {
        SystemError("could not select IP-based network");
		return false;
    }
	
	http_Init();
	
	return true;
}

//////////////////
// Shutdowns the network system
bool QuitNetworkSystem() {
	nlShutdown();
	return true;
}

NetworkSocket OpenReliableSocket(unsigned short port) {
	NetworkSocket ret;
	ret.socket = nlOpen(port, NL_RELIABLE);
	return ret;
}

NetworkSocket OpenUnreliableSocket(unsigned short port) {
	NetworkSocket ret;
	ret.socket = nlOpen(port, NL_UNRELIABLE);
	return ret;
}

NetworkSocket OpenBroadcastSocket(unsigned short port) {
	NetworkSocket ret;
	ret.socket = nlOpen(port, NL_BROADCAST);
	return ret;
}

bool ConnectSocket(NetworkSocket sock, const NetworkAddr* addr) {
	if(addr == NULL)
		return false;
	else  {
		return (nlConnect(sock.socket, &addr->adr) != NL_FALSE);
	}
}

bool ListenSocket(NetworkSocket sock) {
	return (nlListen(sock.socket) != NL_FALSE);
}

bool CloseSocket(NetworkSocket sock) {
	return (nlClose(sock.socket) != NL_FALSE);
}

int WriteSocket(NetworkSocket sock, const void* buffer, int nbytes) {
	return nlWrite(sock.socket, buffer, nbytes);
}

int ReadSocket(NetworkSocket sock, void* buffer, int nbytes) {
	return nlRead(sock.socket, buffer, nbytes);
}

bool IsSocketStateValid(NetworkSocket sock) {
	return (sock.socket != NL_INVALID);
}

void SetSocketStateValid(NetworkSocket& sock, bool valid) {
	sock.socket = NL_INVALID;
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
	if(addr == NULL)
		return false;
	else
		return (nlGetLocalAddr(sock.socket, &addr->adr) != NL_FALSE);
}

bool GetRemoteNetAddr(NetworkSocket sock, NetworkAddr* addr) {
	if(addr == NULL)
		return false;
	else
		return (nlGetRemoteAddr(sock.socket, &addr->adr) != NL_FALSE);
}

bool SetRemoteNetAddr(NetworkSocket sock, const NetworkAddr* addr) {
	if(addr == NULL)
		return false;
	else
		return (nlSetRemoteAddr(sock.socket, &addr->adr) != NL_FALSE);
}

bool IsNetAddrValid(NetworkAddr* addr) {
	if(addr)
		return (addr->adr.valid != NL_FALSE);
	else
		return false;
}

bool SetNetAddrValid(NetworkAddr* addr, bool valid) {
	if(!addr) return false;
	addr->adr.valid = valid ? NL_TRUE : NL_FALSE;
	return true;
}

bool StringToNetAddr(const char* string, NetworkAddr* addr) {
	if(addr == NULL) {
		return false;
	} else	
		return (nlStringToAddr(string, &addr->adr) != NL_FALSE);
}

bool NetAddrToString(const NetworkAddr* addr, char* string) {
	if(addr == NULL) {
		string[0] = '\0';
		return false;
	}
	nlAddrToString(&addr->adr, string);
	return true; // TODO: check it
}

unsigned short GetNetAddrPort(NetworkAddr* addr) {
	if(addr == NULL)
		return 0;
	else
		return nlGetPortFromAddr(&addr->adr);
}

bool SetNetAddrPort(NetworkAddr* addr, unsigned short port) {
	if(addr == NULL)
		return false;
	else
		return (nlSetAddrPort(&addr->adr, port) != NL_FALSE);
}

bool AreNetAddrEqual(const NetworkAddr* addr1, const NetworkAddr* addr2) {
	if(addr1 == addr2)
		return true;
	else {
		if(addr1 == NULL || addr2 == NULL)
			return false;
		else
			return (nlAddrCompare(&addr1->adr, &addr2->adr) != NL_FALSE);
	}
}

using namespace std;
typedef map<string, NetworkAddr> dnsCacheT; 
dnsCacheT dnsCache;

void AddToDnsCache(string name, const NetworkAddr* addr) {
	dnsCache[name] = *addr;
}

bool GetFromDnsCache(string name, NetworkAddr* addr) {
	dnsCacheT::iterator it = dnsCache.find(name);
	if(it != dnsCache.end()) {
		*addr = it->second;
		return true;
	} else
		return false;
}

bool GetNetAddrFromNameAsync(const char* name, NetworkAddr* addr) {
	if(addr == NULL)
		return false;
	else {
		if(GetFromDnsCache(name, addr)) {
			SetNetAddrValid(addr, true);
			return true;
		}
		return (nlGetAddrFromNameAsync(name, &addr->adr) != NL_FALSE);		
	}
}
