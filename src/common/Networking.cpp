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

#include "LieroX.h"
#include "Error.h"
#include "Networking.h"
#include "Utils.h"
#include "StringUtils.h"
#include <SDL_syswm.h>
#include <SDL_thread.h>


#ifdef _MSC_VER
#pragma warning(disable: 4786)
#endif

#include <map>

#include <nl.h>
// workaraound for bad named makros by nl.h
// macros are bad, esp the names (reserved/used by CBytestream)
// TODO: they seem to not work correctly!
// all use of it in CBytestream was removed
inline void nl_writeShort(char* x, int& y, NLushort z)		{ writeShort(x, y, z); }
inline void nl_writeLong(char* x, int& y, NLulong z)		{ writeLong(x, y, z); }
inline void nl_writeFloat(char* x, int& y, NLfloat z)		{ writeFloat(x, y, z); }
inline void nl_writeDouble(char* x, int& y, NLdouble z)		{ writeDouble(x, y, z); }
inline void nl_readShort(char* x, int& y, NLushort z)		{ readShort(x, y, z); }
inline void nl_readLong(char* x, int& y, NLulong z)			{ readLong(x, y, z); }
inline void nl_readFloat(char* x, int& y, NLfloat z)		{ readFloat(x, y, z); }
inline void nl_readDouble(char* x, int& y, NLdouble z)		{ readDouble(x, y, z); }
#undef writeByte
#undef writeShort
#undef writeFloat
#undef writeString
#undef readByte
#undef readShort
#undef readFloat
#undef readString


DECLARE_INTERNDATA_CLASS(NetworkAddr, NLaddress);
DECLARE_INTERNDATA_CLASS(NetworkSocket, NLsocket);




/*
=============================

		HTTP functions

=============================
*/

/*

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
	InvalidateSocketState(http_Socket);
}


///////////////////
// Initialize a HTTP get request
bool http_InitializeRequest(const std::string& host, const std::string& url)
{
	// Make the urls http friendly (get rid of spaces)
	std::string friendly_url = url;
	std::string friendly_host = host;

	http_ConvertUrl(friendly_url, url);
	http_ConvertUrl(friendly_host, host);
	
    // Convert the host & url into a good set
    // Ie, '/'s from host goes into url
    http_CreateHostUrl(friendly_host, friendly_url);

    printf("Sending HTTP request " + http_host + http_url + "\n");

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
		if( IsSocketReady(http_Socket)) {
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

		// Haven't resolved the address yet, so leave but let the 
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
	return (count > 0);
}


///////////////////
// Quit the http request
void http_Quit(void)
{
	if( IsSocketStateValid(http_Socket) ) {
		CloseSocket(http_Socket);
		InvalidateSocketState(http_Socket);
	}

	http_RemoveHeader();
	http_content = "";
}


///////////////////
// Convert the url into a friendly url (no spaces)
void http_ConvertUrl(std::string& dest, const std::string& url)
{
	dest = "";

	const std::string dont_encode = "-_.!~*'()&?/="; // Characters that won't be encoded in any way

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

	// Remove slashes at the end (looks nicer when printed to console :) )
	if (*dest.rbegin() == '/')
		dest.erase(dest.size()-1);
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
*/

/*
 *
 * HawkNL Network wrapper
 *
 */
bool bNetworkInited = false;

/////////////////////
// Initializes network
bool InitNetworkSystem() {
	bNetworkInited = false;

    if(!nlInit()) {
    	SystemError("nlInit failed");
    	return false;
    }
    
    if(!nlSelectNetwork(NL_IP)) {
        SystemError("could not select IP-based network");
		return false;
    }
	
	bNetworkInited = true;
	return true;
}

//////////////////
// Shutdowns the network system
bool QuitNetworkSystem() {
	nlShutdown();
	bNetworkInited = false;
	return true;
}

NetworkSocket OpenReliableSocket(unsigned short port) {
	NetworkSocket ret;
	*NetworkSocketData(&ret) = nlOpen(port, NL_RELIABLE);
	return ret;
}

NetworkSocket OpenUnreliableSocket(unsigned short port) {
	NetworkSocket ret;
	*NetworkSocketData(&ret) = nlOpen(port, NL_UNRELIABLE);
	return ret;
}

NetworkSocket OpenBroadcastSocket(unsigned short port) {
	NetworkSocket ret;
	*NetworkSocketData(&ret) = nlOpen(port, NL_BROADCAST);
	return ret;
}

bool ConnectSocket(NetworkSocket sock, const NetworkAddr* addr) {
	if(addr == NULL)
		return false;
	else  {
		return (nlConnect(*NetworkSocketData(&sock), NetworkAddrData(addr)) != NL_FALSE);
	}
}

bool ListenSocket(NetworkSocket sock) {
	return (nlListen(*NetworkSocketData(&sock)) != NL_FALSE);
}

bool CloseSocket(NetworkSocket sock) {
	return (nlClose(*NetworkSocketData(&sock)) != NL_FALSE);
}

int WriteSocket(NetworkSocket sock, const void* buffer, int nbytes) {
	NLint ret = nlWrite(*NetworkSocketData(&sock), buffer, nbytes);

#ifdef DEBUG
	// Error checking
	if (ret == NL_INVALID)  {
		if (nlGetError() == NL_SYSTEM_ERROR)
			printf("WriteSocket: " + std::string(nlGetSystemErrorStr(nlGetSystemError())) + "\n");
		else
			printf("WriteSocket: " + std::string(nlGetErrorStr(nlGetError())) + "\n");

		return NL_INVALID;
	}

	if (ret == 0) {
		printf("WriteSocket: Could not send the packet, network buffers are full.\n");
	}
#endif // DEBUG

	return ret;
}

int	WriteSocket(NetworkSocket sock, const std::string& buffer) {
	return WriteSocket(sock, buffer.data(), buffer.size());
}

int ReadSocket(NetworkSocket sock, void* buffer, int nbytes) {
	NLint ret = nlRead(*NetworkSocketData(&sock), buffer, nbytes);

#ifdef DEBUG
	// Error checking
	if (ret == NL_INVALID)  {
		if (nlGetError() == NL_SYSTEM_ERROR)
			printf("ReadSocket: SYSTEM: " + std::string(nlGetSystemErrorStr(nlGetSystemError())) + "\n");
		else
			printf("ReadSocket: " + std::string(nlGetErrorStr(nlGetError())) + "\n");

		return NL_INVALID;
	}
#endif // DEBUG

	return ret;
}

bool IsSocketStateValid(NetworkSocket sock) {
	return (*NetworkSocketData(&sock) != NL_INVALID);
}

bool IsSocketReady(NetworkSocket sock)  {
	return nlWrite(*NetworkSocketData(&sock), (void *)"", 0) >= 0;
}

void InvalidateSocketState(NetworkSocket& sock) {
	*NetworkSocketData(&sock) = NL_INVALID;
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

void ResetSocketError()  {
	if (!bNetworkInited)
		return;

	// TODO: make this correct
	// Init a new instance and then shut it down (a bit dirty but there's no other way to do it)
	nlInit();
	nlShutdown();
}

bool GetLocalNetAddr(NetworkSocket sock, NetworkAddr* addr) {
	if(addr == NULL)
		return false;
	else
		return (nlGetLocalAddr(*NetworkSocketData(&sock), NetworkAddrData(addr)) != NL_FALSE);
}

bool GetRemoteNetAddr(NetworkSocket sock, NetworkAddr* addr) {
	if(addr == NULL)
		return false;
	else
		return (nlGetRemoteAddr(*NetworkSocketData(&sock), NetworkAddrData(addr)) != NL_FALSE);
}

bool SetRemoteNetAddr(NetworkSocket sock, const NetworkAddr* addr) {
	if(addr == NULL)
		return false;
	else
		return (nlSetRemoteAddr(*NetworkSocketData(&sock), NetworkAddrData(addr)) != NL_FALSE);
}

bool IsNetAddrValid(NetworkAddr* addr) {
	if(addr)
		return (NetworkAddrData(addr)->valid != NL_FALSE);
	else
		return false;
}

bool SetNetAddrValid(NetworkAddr* addr, bool valid) {
	if(!addr) return false;
	NetworkAddrData(addr)->valid = valid ? NL_TRUE : NL_FALSE;
	return true;
}

void ResetNetAddr(NetworkAddr* addr) {
	if(!addr) return;
	// TODO: is this the best way?
	memset(NetworkAddrData(addr), 0, sizeof(NLaddress));
	SetNetAddrValid(addr, false);
}

bool StringToNetAddr(const std::string& string, NetworkAddr* addr) {
	if(addr == NULL) {
		return false;
	} else	
		return (nlStringToAddr(string.c_str(), NetworkAddrData(addr)) != NL_FALSE);
}

bool NetAddrToString(const NetworkAddr* addr, std::string& string) {
	static char buf[256];
	NLchar* res = nlAddrToString(NetworkAddrData(addr), buf);
	if(res) {
		fix_markend(buf);
		string = buf;
		return true;
	} else
		return false;
}

unsigned short GetNetAddrPort(NetworkAddr* addr) {
	if(addr == NULL)
		return 0;
	else
		return nlGetPortFromAddr(NetworkAddrData(addr));
}

bool SetNetAddrPort(NetworkAddr* addr, unsigned short port) {
	if(addr == NULL)
		return false;
	else
		return (nlSetAddrPort(NetworkAddrData(addr), port) != NL_FALSE);
}

bool AreNetAddrEqual(const NetworkAddr* addr1, const NetworkAddr* addr2) {
	if(addr1 == addr2)
		return true;
	else {
		if(addr1 == NULL || addr2 == NULL)
			return false;
		else
			return (nlAddrCompare(NetworkAddrData(addr1), NetworkAddrData(addr2)) != NL_FALSE);
	}
}

typedef std::map<std::string, NetworkAddr> dnsCacheT; 
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
		return (nlGetAddrFromNameAsync(name.c_str(), NetworkAddrData(addr)) != NL_FALSE);
	}
}

bool GetNetAddrFromName(const std::string& name, NetworkAddr* addr) {
	if(addr == NULL)
		return false;
	else {
		if(GetFromDnsCache( name, addr )) {
			SetNetAddrValid( addr, true );
			return true;
		}
		if( nlGetAddrFromName(name.c_str(), NetworkAddrData(addr)) != NL_FALSE ) {
			AddToDnsCache( name, addr );
			return true;
		}
		return false;
	}
}

bool isDataAvailable(NetworkSocket sock) {
	NLint group = nlGroupCreate();
	nlGroupAddSocket( group, *NetworkSocketData(&sock) );
	NLsocket sock_out[2];
	int ret = nlPollGroup( group, NL_READ_STATUS, sock_out, 1, 0 );
	nlGroupDestroy(group);
	return ret > 0;
};


enum	{ SDL_USEREVENT_NET_ACTIVITY = SDL_USEREVENT + 1 };

bool SdlNetEvent_Inited = false;
SDL_mutex * SdlNetEventMutex = NULL;
bool SdlNetEventThreadExit = false;
SDL_Thread * SdlNetEventThread = NULL;
NLint SdlNetEventGroup = 0;
NetworkSocket SdlNetEventThreadUnblockDummySocketIn;
NetworkSocket SdlNetEventThreadUnblockDummySocketOut;

int SdlNetEventThreadMain( void * param )
{
	NLsocket sock_out;
	SDL_Event ev;
	ev.type = SDL_USEREVENT_NET_ACTIVITY;
	ev.user.code = 0;
	ev.user.data1 = NULL;
	ev.user.data2 = NULL;
	int eventFlood = 0;
	while( ! SdlNetEventThreadExit )
	{
		SDL_LockMutex( SdlNetEventMutex );
		if( nlPollGroup( SdlNetEventGroup, NL_READ_STATUS, &sock_out, 1, 1000 ) > 0 )	// Wait 1 second
		{
			//printf("SdlNetEventThreadMain(): SDL_PushEvent()\n");
			SDL_PushEvent( &ev );
			if( sock_out == *NetworkSocketData(&SdlNetEventThreadUnblockDummySocketIn) )
			{
				char temp[256];
				ReadSocket( SdlNetEventThreadUnblockDummySocketIn, temp, sizeof(temp) );
			}
			eventFlood ++;
		}
		else
		{
			eventFlood = 0;
		}
		SDL_UnlockMutex( SdlNetEventMutex );
		if( eventFlood > 20 )	// Some socket with data but noone wants to read it
			SDL_Delay(200);		// TODO: some better solution?
		else
			SDL_Delay(10);	// Allow other threads to run to add/remove sockets to group or to process the data.
	};
	return 0;
};

void SdlNetEvent_Init()
{
	if( SdlNetEvent_Inited )
		return;
	SdlNetEvent_Inited = true;
	SdlNetEventMutex = SDL_CreateMutex();
	SdlNetEventThread = SDL_CreateThread( &SdlNetEventThreadMain, NULL );
	SdlNetEventGroup = nlGroupCreate();
	SdlNetEventThreadUnblockDummySocketIn = OpenUnreliableSocket(0);
	SdlNetEventThreadUnblockDummySocketOut = OpenUnreliableSocket(0);
	NetworkAddr localAddr;
	ResetNetAddr( &localAddr );
	GetLocalNetAddr( SdlNetEventThreadUnblockDummySocketIn, &localAddr );
	SetRemoteNetAddr( SdlNetEventThreadUnblockDummySocketOut, &localAddr );
	nlGroupAddSocket( SdlNetEventGroup, *NetworkSocketData(&SdlNetEventThreadUnblockDummySocketIn) );
	// If we send something into SdlNetEventThreadUnlockDummySocketOut the thread will unblock
};

void	SendSdlEventWhenDataAvailable( NetworkSocket sock )
{
	SdlNetEvent_Init();
	WriteSocket( SdlNetEventThreadUnblockDummySocketOut, "1", 1 );
	SDL_LockMutex( SdlNetEventMutex );
	NLsocket sockets[NL_MAX_GROUP_SOCKETS];
	NLint len = NL_MAX_GROUP_SOCKETS;
	nlGroupGetSockets( SdlNetEventGroup, sockets, &len );
	for( int f = 0; f < len; f++ )
		if( sockets[f] == *NetworkSocketData(&sock) )
		{
			SDL_UnlockMutex( SdlNetEventMutex );
			return;
		};
	nlGroupAddSocket( SdlNetEventGroup, *NetworkSocketData(&sock) );
	SDL_UnlockMutex( SdlNetEventMutex );
};

void	StopSendSdlEventWhenDataAvailable( NetworkSocket sock )
{
	SdlNetEvent_Init();
	WriteSocket( SdlNetEventThreadUnblockDummySocketOut, "1", 1 );
	SDL_LockMutex( SdlNetEventMutex );
	nlGroupDeleteSocket( SdlNetEventGroup, *NetworkSocketData(&sock) );
	SDL_UnlockMutex( SdlNetEventMutex );
};

class t_SdlNetEventMutexDelete
{
	public:
	~t_SdlNetEventMutexDelete()
	{
		if( ! SdlNetEvent_Inited )
			return;
		SdlNetEventThreadExit = true;
		int status = 0;
		//WriteSocket( SdlNetEventThreadUnblockDummySocketOut, "1", 1 );// Cannot do that, network already shut down in main()
		SDL_WaitThread( SdlNetEventThread, &status );
		SDL_DestroyMutex( SdlNetEventMutex );
		SdlNetEventMutex = NULL;
		CloseSocket( SdlNetEventThreadUnblockDummySocketIn );
		CloseSocket( SdlNetEventThreadUnblockDummySocketOut );
		nlGroupDestroy(SdlNetEventGroup);
	};
};
t_SdlNetEventMutexDelete SdlNetEventMutexDelete;

