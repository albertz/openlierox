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

#ifdef _MSC_VER
#pragma warning(disable: 4786)
#endif

#include <map>





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
std::string		http_url;
std::string		http_host;
std::string		http_content;
float           http_ResolveTime = -9999;



void http_Init() {
	SetSocketStateValid(http_Socket, false);
}


///////////////////
// Initialize a HTTP get request
bool http_InitializeRequest(const std::string& host, const std::string& url)
{
	// Make the url http friendly (get rid of spaces)	
	// TODO: why was this commented out?
	//http_ConvertUrl(http_url, url);
	
    // Convert the host & url into a good set
    // Ie, '/'s from host goes into url
    http_CreateHostUrl(host, url);

    d_printf("Sending HTTP request %s %s\n",http_host.c_str(), http_url.c_str());

	http_content = "";

	
	// Open the socket
	http_Socket = OpenReliableSocket(0);
	if(!IsSocketStateValid(http_Socket))
		return false;

	// Resolve the address
	// reset the current adr; sometimes needed (hack? bug in hawknl?)
	ResetNetAddr(&http_RemoteAddress);
    http_ResolveTime = (float)SDL_GetTicks() * 0.001f;
	if(!GetNetAddrFromNameAsync( http_host, &http_RemoteAddress )) {
		printf("ERROR: cannot start resolving DNS: ");
		printf(GetSocketErrorStr(GetSocketErrorNr()));
		printf("\n");
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
// HINT: it doesn't do http_Quit now
int http_ProcessRequest(std::string* szError)
{
    if(szError)
        *szError = "";

	// Check if the address failed resolving in n seconds
	if(!IsNetAddrValid(&http_RemoteAddress)) {
        float f = (float)SDL_GetTicks() * 0.001f;
        // Timed out?
        if(f - http_ResolveTime > DNS_TIMEOUT) {
            if(szError) {
                *szError = "DNS-timeout, could not resolve the address: ";
            }
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
			return 0;
		}
	}

	// Send a request
	if( http_SocketReady && http_Connected && !http_Requested) {
		http_Requested = true;
		if( !http_SendRequest() ) {
            if(szError)
                *szError = "Could not send the request";
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
                    *szError = "Could not connect to the server";
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
	if(!http_SocketReady || !http_Connected || !http_Requested)	 {
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
			return 1;
		} else {
			// Error
			if(szError) {
				*szError = "NetError \"";
				*szError += GetSocketErrorStr(err);
				*szError += "\"";
			}
			return -1;
		}
	}

	// Got some data
	if(count > 0) {
		http_content.append(data, count);
	}

	// Still processing
	return 0;
}


///////////////////
// Send a request
bool http_SendRequest(void)
{
	static std::string request;

	// Build the url
	request = "GET " + http_url + " HTTP/1.0\n";
	request += "Host: " + http_host + "\n\n";
	int count = WriteSocket( http_Socket, request );

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
	http_content = "";
}


///////////////////
// Convert the url into a friendly url (no spaces)
void http_ConvertUrl(std::string& dest, const std::string& url)
{
	size_t i;
	char buffer[3];
	char c;


	dest = "";

	// TODO: use iterators!
	for( i=0; i < url.size(); i++) {
		c = url[i];
		
//		if(url[i] == '_')
//			c = ' ';
			

		if( isalnum(c) )
			dest += c;
		else {
			snprintf(buffer,sizeof(buffer),"%X",c);
			dest += '%';
			dest += buffer[0]; dest += buffer[1];
		}
	}
}


///////////////////
// Create the host & url strings
// host is the beginning of an URL, url is the end (to be appended)
void http_CreateHostUrl(const std::string& host, const std::string& url)
{
    http_host = "";
    http_url = "";

    // All characters up to a / goes into the host
	size_t i;
    std::string::const_iterator it = host.begin();
    for( i=0; it != host.end(); i++, it++ ) {
        if( *it == '/' ) {
			http_host = host.substr(0,i);
			http_url = host.substr(i);
            break;
        }
    }

	http_url += url;
}


///////////////////
// Remove the http header from content downloaded
void http_RemoveHeader(void)
{
	ushort	lffound = 0;
	ushort	crfound = 0;

    size_t i=1;
    std::string::const_iterator it = http_content.begin();
	for(; it != http_content.end(); i++, it++) {

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
			http_content.erase(0, i);
			break;
		}
	}
}


///////////////////
// Get the content buffer
const std::string& http_GetContent(void)
{
	return http_content;
}


/*
 *
 * HawkNL Network wrapper
 *
 */

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

int	WriteSocket(NetworkSocket sock, const std::string& buffer) {
	return WriteSocket(sock, buffer.c_str(), buffer.size());
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

const std::string GetSocketErrorStr(int errnr) {
	return std::string(nlGetErrorStr(errnr));
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

void ResetNetAddr(NetworkAddr* addr) {
	if(!addr) return;
	// TODO: is this the best way?
	memset(addr, 0, sizeof(NetworkAddr));
	SetNetAddrValid(addr, false);
}

bool StringToNetAddr(const std::string& string, NetworkAddr* addr) {
	if(addr == NULL) {
		return false;
	} else	
		return (nlStringToAddr(string.c_str(), &addr->adr) != NL_FALSE);
}

bool NetAddrToString(const NetworkAddr* addr, std::string& string) {
	static char buf[256];
	nlAddrToString(&addr->adr, buf);
	string = buf;
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
// TODO: use hash_map
typedef map<string, NetworkAddr> dnsCacheT; 
dnsCacheT dnsCache;

void AddToDnsCache(const std::string& name, const NetworkAddr* addr) {
	dnsCache[name] = *addr;
}

bool GetFromDnsCache(const std::string& name, NetworkAddr* addr) {
	dnsCacheT::iterator it = dnsCache.find(name);
	if(it != dnsCache.end()) {
		*addr = it->second;
		return true;
	} else
		return false;
}

bool GetNetAddrFromNameAsync(const std::string& name, NetworkAddr* addr) {
	if(addr == NULL)
		return false;
	else {
		if(GetFromDnsCache(name, addr)) {
			SetNetAddrValid(addr, true);
			return true;
		}
		return (nlGetAddrFromNameAsync(name.c_str(), &addr->adr) != NL_FALSE);		
	}
}
