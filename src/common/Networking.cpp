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

#include <SDL_syswm.h>
#include <SDL_thread.h>
#ifndef WIN32
#include <signal.h>
#endif

#include "LieroX.h"

#include "Options.h"
#include "Error.h"
#include "Networking.h"
#include "Utils.h"
#include "StringUtils.h"
#include "SmartPointer.h"
#include "Timer.h"
#include "Utils.h"
#include "ThreadVar.h"
#include "MathLib.h"
#include "InputEvents.h"


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

class NetAddrIniter {
public:
	void operator()(SmartPointer<NLaddress, NetAddrIniter>* addr) {
		NLaddress* addrPtr = new NLaddress;
		memset(addrPtr, 0, sizeof(NLaddress));
		*addr = addrPtr;
	}
};

typedef SmartPointer<NLaddress, NetAddrIniter> NetAddrPtr;
DECLARE_INTERNDATA_CLASS( NetworkAddr, NetAddrPtr );

DECLARE_INTERNDATA_CLASS( NetworkSocket, NLsocket );


static NLsocket* getNLsocket(NetworkSocket* socket) {
	return NetworkSocketData(socket);
}

static NLaddress* getNLaddr(NetworkAddr& addr) {
	return NetworkAddrData(&addr)->get();
}

static const NLaddress* getNLaddr(const NetworkAddr& addr) {
	return NetworkAddrData(&addr)->get();
}


// TODO: perhaps move these test-functions somewhere else?
// but it's good to keep them somewhere to easily test some code part
void test_NetworkSmartPointer() {
	for(int i = 0; i < 100; i++)
	{
		printf("creating SP\n");
		NetAddrPtr sp;
		printf("destroying SP\n");
	}


	SDL_Delay(1000);

	printf("creating data-array\n");
	char* tmp = new char[2048];

	printf("freeing data-array\n");
	delete tmp;

//	exit(-1);
}


// --------------------------------------------------------------------------
// ------------- Net events

bool SdlNetEvent_Inited = false;
bool SdlNetEventThreadExit = false;
SDL_Thread* SdlNetEventThreads[3] = {NULL, NULL, NULL};
NLint SdlNetEventGroup = 0;
int SdlNetEventSocketCount = 0;

struct NetActivityData {
	uint type;
	NLsocket* socket;
	NetActivityData(uint t, NLsocket* s) : type(t), socket(s) {}
};
Event<NetActivityData> onNetActivity;

static bool isSocketInGroup(NLint group, NetworkSocket sock) {
	NLsocket sockets[NL_MAX_GROUP_SOCKETS];
	NLint len = NL_MAX_GROUP_SOCKETS;
	nlGroupGetSockets( group, sockets, &len );
	for( int f = 0; f < len; f++ )
		if( sockets[f] == *getNLsocket(&sock) )
			return true;

	return false;
}

static bool isSocketGroupEmpty(NLint group) {
	NLsocket oneSocket;
	NLint len = 1;
	// because we set len=1 we will either get one or none sockets here
	nlGroupGetSockets( group, &oneSocket, &len );
	return len == 0;
}

static int SdlNetEventThreadMain( void * param )
{
	int buffer_size = MAX(2, SdlNetEventSocketCount);
	NLsocket *sock_out = new NLsocket[buffer_size];
	// save event-type (NL_READ_STATUS, NL_WRITE_STATUS or NL_ERROR_STATUS)

	// When restarting, this can happen, we wait for options to initialize
	while (tLXOptions == NULL && !SdlNetEventThreadExit)
		SDL_Delay(20);

	float max_frame_time = MAX(0.01f, (tLXOptions->nMaxFPS > 0) ? 1.0f/(float)tLXOptions->nMaxFPS : 0);
	float lastTime = GetMilliSeconds();
	while( ! SdlNetEventThreadExit )
	{
		if( ! isSocketGroupEmpty( SdlNetEventGroup ) && SdlNetEventSocketCount ) { // only when we have at least one socket
		if( nlPollGroup( SdlNetEventGroup, *(uint*)param, sock_out, buffer_size, (int)(max_frame_time * 1000.0f) ) > 0 ) // Wait max_frame_time
		{
			if(sock_out >= 0) {
				SendSDLUserEvent(&onNetActivity, NetActivityData( (long) *(uint*)param, sock_out ));
			}
			else
				printf("WARNING: net-event-system: invalid socket\n");
		} else {
			if (nlGetError() == NL_BUFFER_SIZE)  { // We should increase the buffer size
				delete[] sock_out;
				buffer_size = MAX(2, SdlNetEventSocketCount);
				sock_out = new NLsocket[buffer_size];
				continue; // Retry
			}
		}
		}
		
		float curTime = GetMilliSeconds();
		if(curTime - lastTime < max_frame_time) {
			SDL_Delay( (int)( ( max_frame_time - curTime + lastTime ) * 1000.0f ) );
		}
		lastTime = curTime;
	};

	delete (uint*)param;
	delete[] sock_out;
	return 0;
};

static bool SdlNetEvent_Init()
{
	if( SdlNetEvent_Inited )
		return false;
	SdlNetEvent_Inited = true;

	SdlNetEventGroup = nlGroupCreate();
	SdlNetEventThreads[0] = SDL_CreateThread( &SdlNetEventThreadMain, new uint(NL_READ_STATUS) );
	// TODO: this does not behave as expected
	// select() which is internally used by nlPollGroup will return true if the socket is *writeable*
	// which for UDP sockets means always (this has nothing to do with the actual writing to the socket)
	// See http://msdn2.microsoft.com/en-us/library/ms740141(VS.85).aspx or
	// http://support.sas.com/documentation/onlinedoc/sasc/doc750/html/lr2/select.htm for more details
	//SdlNetEventThreads[1] = SDL_CreateThread( &SdlNetEventThreadMain, new uint(NL_WRITE_STATUS) );
	SdlNetEventThreads[2] = SDL_CreateThread( &SdlNetEventThreadMain, new uint(NL_ERROR_STATUS) );

	return true;
};

static void SdlNetEvent_UnInit() {
	if( ! SdlNetEvent_Inited )
		return;

	SdlNetEventThreadExit = true;
	int status = 0;
	SDL_WaitThread( SdlNetEventThreads[0], &status );
	SDL_WaitThread( SdlNetEventThreads[1], &status );
	SDL_WaitThread( SdlNetEventThreads[2], &status );
	nlGroupDestroy(SdlNetEventGroup);

	SdlNetEvent_Inited = false;
};



void AddSocketToNotifierGroup( NetworkSocket sock )
{
	SdlNetEvent_Init();

	if( IsSocketStateValid(sock) && !isSocketInGroup(SdlNetEventGroup, sock) )  {
		nlGroupAddSocket( SdlNetEventGroup, *getNLsocket(&sock) );
		SdlNetEventSocketCount++;
	}
};

void RemoveSocketFromNotifierGroup( NetworkSocket sock )
{
	SdlNetEvent_Init();

	if (nlGroupDeleteSocket( SdlNetEventGroup, *getNLsocket(&sock) ) && SdlNetEventSocketCount > 0)
		SdlNetEventSocketCount--;
};

// ------------------------------------------------------------------------


#ifndef WIN32
static void sigpipe_handler(int i) {
	printf("WARNING: got SIGPIPE, ignoring...\n");
	signal(SIGPIPE, sigpipe_handler);
}
#endif


/*
 *
 * HawkNL Network wrapper
 *
 */



typedef std::map<std::string, NLaddress> dnsCacheT;
ThreadVar<dnsCacheT>* dnsCache = NULL;

void AddToDnsCache(const std::string& name, const NetworkAddr& addr) {
	ThreadVar<dnsCacheT>::Writer dns( *dnsCache );
	dns.get()[name] = *getNLaddr(addr);
}

static void AddToDnsCache(const std::string& name, const NLaddress& addr) {
	ThreadVar<dnsCacheT>::Writer dns( *dnsCache );
	dns.get()[name] = addr;
}

bool GetFromDnsCache(const std::string& name, NetworkAddr& addr) {
	ThreadVar<dnsCacheT>::Reader dns( *dnsCache );
	dnsCacheT::const_iterator it = dns.get().find(name);
	if(it != dns.get().end()) {
		*getNLaddr(addr) = it->second;
		return true;
	} else
		return false;
}



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

	if(!SdlNetEvent_Init()) {
		SystemError("SdlNetEvent_Init failed");
		return false;
	}

	dnsCache = new ThreadVar<dnsCacheT>();

#ifndef WIN32
	//sigignore(SIGPIPE);
	signal(SIGPIPE, sigpipe_handler);
#endif

	return true;
}

//////////////////
// Shutdowns the network system
bool QuitNetworkSystem() {
	SdlNetEvent_UnInit();
	nlShutdown();
	bNetworkInited = false;
	delete dnsCache; dnsCache = NULL;
	return true;
}

NetworkSocket OpenReliableSocket(unsigned short port) {
	NetworkSocket ret;
	*getNLsocket(&ret) = nlOpen(port, NL_RELIABLE);
	if (IsSocketStateValid(ret))
		AddSocketToNotifierGroup(ret);
#ifdef DEBUG
	else
		printf("OpenReliableSocket: " + GetSocketErrorStr(nlGetError()) + "\n");
#endif
	return ret;
}

NetworkSocket OpenUnreliableSocket(unsigned short port, bool events) {
	NetworkSocket ret;
	*getNLsocket(&ret) = nlOpen(port, NL_UNRELIABLE);
	if (!IsSocketStateValid(ret))  {
#ifdef DEBUG
		printf("OpenUnreliableSocket: " + GetLastErrorStr() + "\n");
#endif
		return ret;
	}
	if(events) AddSocketToNotifierGroup(ret);
	return ret;
}

NetworkSocket OpenBroadcastSocket(unsigned short port, bool events) {
	NetworkSocket ret;
	*getNLsocket(&ret) = nlOpen(port, NL_BROADCAST);
	if (!IsSocketStateValid(ret))  {
#ifdef DEBUG
		printf("OpenBroadcastSocket: " + GetLastErrorStr() + "\n");
#endif
		return ret;
	}
	if(events) AddSocketToNotifierGroup(ret);
	return ret;
}

bool ConnectSocket(NetworkSocket sock, const NetworkAddr& addr) {
	AddSocketToNotifierGroup(sock);
	return (nlConnect(*getNLsocket(&sock), getNLaddr(addr)) != NL_FALSE);
}

bool ListenSocket(NetworkSocket sock) {
	AddSocketToNotifierGroup(sock);
	return (nlListen(*getNLsocket(&sock)) != NL_FALSE);
}

bool CloseSocket(NetworkSocket sock) {
	RemoveSocketFromNotifierGroup(sock);
	return (nlClose(*getNLsocket(&sock)) != NL_FALSE);
}

int WriteSocket(NetworkSocket sock, const void* buffer, int nbytes) {
	NLint ret = nlWrite(*getNLsocket(&sock), buffer, nbytes);

#ifdef DEBUG
	// Error checking
	if (ret == NL_INVALID)  {
		printf("WriteSocket: " + GetLastErrorStr() + "\n");
		return NL_INVALID;
	}

	if (ret == 0) {
		printf("WriteSocket: Could not send the packet, network buffers are full.\n");
	}
#endif // DEBUG

	return ret;
}

int	WriteSocket(NetworkSocket sock, const std::string& buffer) {
	return WriteSocket(sock, buffer.data(), (int)buffer.size());
}

int ReadSocket(NetworkSocket sock, void* buffer, int nbytes) {
	NLint ret = nlRead(*getNLsocket(&sock), buffer, nbytes);

#ifdef DEBUG
	// Error checking
	if (ret == NL_INVALID)  {
		printf("ReadSocket: " + GetLastErrorStr() + "\n");
		return NL_INVALID;
	}
#endif // DEBUG

	return ret;
}

bool IsSocketStateValid(NetworkSocket sock) {
	return (*getNLsocket(&sock) != NL_INVALID);
}


// ---------------------------------------------------
// ----------- for state checking -------------------

// copied from sock.c from HawkNL

#include <memory.h>
#include <stdio.h>
#include <string.h>

#if defined (_WIN32_WCE)
#define EAGAIN          11
#define errno GetLastError()
#else
#include <errno.h>
#endif

#if defined WIN32 || defined WIN64 || defined (_WIN32_WCE)

#include "wsock.h"

#elif defined Macintosh

#include <Types.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <sys/time.h>
#include <LowMem.h>
#define closesocket close
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define SOCKET int
#define sockerrno errno

/* define INADDR_NONE if not already */
#ifndef INADDR_NONE
#define INADDR_NONE ((unsigned long) -1)
#endif

#else

/* Unix-style systems */
#ifdef SOLARIS
#include <sys/filio.h> /* for FIONBIO */
#endif

#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/ioctl.h>
#define closesocket close
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define SOCKET int
#define sockerrno errno

/* define INADDR_NONE if not already */
#ifndef INADDR_NONE
#define INADDR_NONE ((unsigned long) -1)
#endif

/* SGI do not include socklen_t */
#if defined __sgi
typedef int socklen_t;
#endif

#endif /* WINDOWS_APP*/


// from nlinternal.h from HawkNL

/* number of buckets for average bytes/second */
#define NL_NUM_BUCKETS          8

/* number of packets stored for NL_LOOP_BACK */
#define NL_NUM_PACKETS          8
#define NL_MAX_ACCEPT           10


typedef struct
{
    NLlong      bytes;          /* bytes sent/received */
    NLlong      packets;        /* packets sent/received */
    NLlong      highest;        /* highest bytes/sec sent/received */
    NLlong      average;        /* average bytes/sec sent/received */
    time_t      stime;          /* the last time stats were updated */
    NLint       lastbucket;     /* the last bucket that was used */
    NLlong      curbytes;       /* current bytes sent/received */
    NLlong      bucket[NL_NUM_BUCKETS];/* buckets for sent/received counts */
    NLboolean   firstround;     /* is this the first round through the buckets? */
} nl_stats_t;

typedef struct
{
    /* info for NL_LOOP_BACK, NL_SERIAL, and NL_PARALLEL */
    NLbyte      *outpacket[NL_NUM_PACKETS];/* temp storage for packet data */
    NLbyte      *inpacket[NL_NUM_PACKETS];/* temp storage for packet data */
    NLint       outlen[NL_NUM_PACKETS];/* the length of each packet */
    NLint       inlen[NL_NUM_PACKETS];/* the length of each packet */
    NLint       nextoutused;    /* the next used packet */
    NLint       nextinused;     /* the next used packet */
    NLint       nextoutfree;    /* the next free packet */
    NLint       nextinfree;     /* the next free packet */
    NLsocket    accept[NL_MAX_ACCEPT];/* pending connects */
    NLsocket    consock;        /* the socket this socket is connected to */
} nl_extra_t;


/* the internal socket object */
typedef struct
{
    /* the current status of the socket */
    NLenum      driver;         /* the driver used with this socket */
    NLenum      type;           /* type of socket */
    NLboolean   inuse;          /* is in use */
    NLboolean   connecting;     /* a non-blocking TCP or UDP connection is in process */
    NLboolean   conerror;       /* an error occured on a UDP connect */
    NLboolean   connected;      /* is connected */
    NLboolean   reliable;       /* do we use reliable */
    NLboolean   blocking;       /* is set to blocking */
    NLboolean   listen;         /* can receive an incoming connection */
    NLint       realsocket;     /* the real socket number */
    NLushort    localport;      /* local port number */
    NLushort    remoteport;     /* remote port number */
    NLaddress   addressin;      /* address of remote system, same as the socket sockaddr_in structure */
    NLaddress   addressout;     /* the multicast address set by nlConnect or the remote address for unconnected UDP */
    NLmutex     readlock;       /* socket is locked to update data */
    NLmutex     writelock;      /* socket is locked to update data */

    /* the current read/write statistics for the socket */
    nl_stats_t  instats;        /* stats for received */
    nl_stats_t  outstats;       /* stats for sent */

    /* NL_RELIABLE_PACKETS info and storage */
    NLbyte      *outbuf;        /* temp storage for partially sent reliable packet data */
    NLint       outbuflen;      /* the length of outbuf */
    NLint       sendlen;        /* how much still needs to be sent */
    NLbyte      *inbuf;         /* temp storage for partially received reliable packet data */
    NLint       inbuflen;       /* the length of inbuf */
    NLint       reclen;         /* how much of the reliable packet we have received */
    NLboolean   readable;       /* a complete packet is in inbuf */
    NLboolean   message_end;    /* a message end error ocured but was not yet reported */
    NLboolean   packetsync;     /* is the reliable packet stream in sync */
    /* pointer to extra info needed for NL_LOOP_BACK, NL_SERIAL, and NL_PARALLEL */
    nl_extra_t   *ext;
} nl_socket_t;

typedef /*@only@*/ nl_socket_t *pnl_socket_t;

// -------------------------------------
// extern defs for HawkNL intern stuff
#ifdef _MSC_VER
extern "C"  {
	__declspec(dllimport) pnl_socket_t *nlSockets;
}
#else
#ifdef WIN32
extern "C" pnl_socket_t *nlSockets;	// For Dev-Cpp
#else
extern "C"  {
	NL_EXP pnl_socket_t *nlSockets;
}
#endif
#endif


#if defined(WIN32) && (! defined(_MSC_VER))
extern "C" void NL_APIENTRY nlSetError(NLenum err); // For Dev-Cpp
#elif (_MSC_VER == 1400) //MSVC 2005 hax
extern "C" void nlSetError(NLenum err);
#else
extern "C" {
	NL_EXP void NL_APIENTRY nlSetError(NLenum err);
}
#endif

// modified sock_Write of sock.c from HawkNL
// returns true if socket is connected and data could be send
static bool nlUpdateState(NLsocket socket)
{
    nl_socket_t *sock = nlSockets[socket];
    NLint       count = 0;

    if((sock->type == NL_RELIABLE) || (sock->type == NL_RELIABLE_PACKETS)) /* TCP */
    {
        if(sock->connecting == NL_TRUE)
        {
            fd_set          fdset;
            struct timeval  t = {0,0};
            int             serrval = -1;
            socklen_t       serrsize = (socklen_t)sizeof(serrval);


            FD_ZERO(&fdset);
            FD_SET((SOCKET)sock->realsocket, &fdset);
            if(select(sock->realsocket + 1, NULL, &fdset, NULL, &t) == 1)
            {
                /* Check the socket status */
                (void)getsockopt( sock->realsocket, SOL_SOCKET, SO_ERROR, (char *)&serrval, &serrsize );
                if(serrval != 0)
                {
                    if(serrval == ECONNREFUSED)
                    {
                        nlSetError(NL_CON_REFUSED);
                    }
                    else if(serrval == EINPROGRESS || serrval == EWOULDBLOCK)
                    {
                        nlSetError(NL_CON_PENDING);
                    }
                    return false;
                }
                /* the connect has completed */
                sock->connected = NL_TRUE;
                sock->connecting = NL_FALSE;
            }
            else
            {
                /* check for a failed connect */
                FD_ZERO(&fdset);
                FD_SET((SOCKET)sock->realsocket, &fdset);
                if(select(sock->realsocket + 1, NULL, NULL, &fdset, &t) == 1)
                {
                    nlSetError(NL_CON_REFUSED);
                }
                else
                {
                    nlSetError(NL_CON_PENDING);
                }
                return false;
            }
        }
        /* check for reliable packets */
        if(sock->type == NL_RELIABLE_PACKETS)
        {
            return true;
        }
		return true;
	}
    else /* unconnected UDP */
    {
        /* check for a non-blocking connection pending */
        if(sock->connecting == NL_TRUE)
        {
            nlSetError(NL_CON_PENDING);
            return false;
        }
        /* check for a connection error */
        if(sock->conerror == NL_TRUE)
        {
            nlSetError(NL_CON_REFUSED);
            return false;
        }
        if(sock->type == NL_BROADCAST)
        {
            ((struct sockaddr_in *)&sock->addressin)->sin_addr.s_addr = INADDR_BROADCAST;
        }
        if(sock->type == NL_UDP_MULTICAST)
        {
			return true;
		}
        else if(sock->connected == NL_TRUE)
        {
            return true;
        }
        else
        {
			return true;
		}
    }
    if(count == SOCKET_ERROR)
    {
        return false;
    }
    return false;
}






bool IsSocketReady(NetworkSocket sock)  {
	return IsSocketStateValid(sock) && nlUpdateState(*getNLsocket(&sock));
}

void InvalidateSocketState(NetworkSocket& sock) {
	*NetworkSocketData(&sock) = NL_INVALID;
}

int GetSocketErrorNr() {
	return nlGetError();
}

std::string GetSocketErrorStr(int errnr) {
	return GetLastErrorStr();
}

std::string GetLastErrorStr()  {
	if (nlGetError() != NL_SYSTEM_ERROR)
		return std::string(nlGetErrorStr(nlGetError()));
	else
		return std::string(nlGetSystemErrorStr(nlGetSystemError()));
}

bool IsMessageEndSocketErrorNr(int errnr) {
	return (errnr == NL_MESSAGE_END);
}

void ResetSocketError()  {
	if (!bNetworkInited)
		return;

	nlSetError(NL_NO_ERROR);
}

bool GetLocalNetAddr(NetworkSocket sock, NetworkAddr& addr) {
	if(getNLaddr(addr) == NULL)
		return false;
	else
		return (nlGetLocalAddr(*getNLsocket(&sock), getNLaddr(addr)) != NL_FALSE);
}

bool GetRemoteNetAddr(NetworkSocket sock, NetworkAddr& addr) {
	if(getNLaddr(addr) == NULL)
		return false;
	else
		return (nlGetRemoteAddr(*getNLsocket(&sock), getNLaddr(addr)) != NL_FALSE);
}

bool SetRemoteNetAddr(NetworkSocket sock, const NetworkAddr& addr) {
	if(getNLaddr(addr) == NULL)
		return false;
	else
		return (nlSetRemoteAddr(*getNLsocket(&sock), getNLaddr(addr)) != NL_FALSE);
}

bool IsNetAddrValid(const NetworkAddr& addr) {
	if(getNLaddr(addr))
		return (getNLaddr(addr)->valid != NL_FALSE);
	else
		return false;
}

bool SetNetAddrValid(NetworkAddr& addr, bool valid) {
	if(!getNLaddr(addr)) return false;
	getNLaddr(addr)->valid = valid ? NL_TRUE : NL_FALSE;
	return true;
}

void ResetNetAddr(NetworkAddr& addr) {
	if(!getNLaddr(addr)) return;
	// TODO: is this the best way?
	memset(getNLaddr(addr), 0, sizeof(NLaddress));
	SetNetAddrValid(addr, false);
}

// accepts "%i.%i.%i.%i[:%l]" as input
bool StringToNetAddr(const std::string& string, NetworkAddr& addr) {
	if(getNLaddr(addr) == NULL) {
		return false;
	} else
		return (nlStringToAddr(string.c_str(), getNLaddr(addr)) != NL_FALSE);
}

bool NetAddrToString(const NetworkAddr& addr, std::string& string) {
	// TODO: safty here for buffer
	char buf[256];
	NLchar* res = nlAddrToString(getNLaddr(addr), buf);
	if(res) {
		fix_markend(buf);
		string = buf;
		return true;
	} else
		return false;
}

unsigned short GetNetAddrPort(const NetworkAddr& addr) {
	if(getNLaddr(addr) == NULL)
		return 0;
	else
		return nlGetPortFromAddr(getNLaddr(addr));
}

bool SetNetAddrPort(NetworkAddr& addr, unsigned short port) {
	if(getNLaddr(addr) == NULL)
		return false;
	else
		return (nlSetAddrPort(getNLaddr(addr), port) != NL_FALSE);
}

bool AreNetAddrEqual(const NetworkAddr& addr1, const NetworkAddr& addr2) {
	if(getNLaddr(addr1) == getNLaddr(addr2))
		return true;
	else {
		return (nlAddrCompare(getNLaddr(addr1), getNLaddr(addr2)) != NL_FALSE);
	}
}


struct NLaddress_ex_t {
	std::string name;
	NetAddrPtr address;
};


Event<> onDnsReady;

// copied from HawkNL sock.c and modified for usage of SmartPointer
static void* GetAddrFromNameAsync_Internal(void /*@owned@*/ *addr)
{
    NLaddress_ex_t *address = (NLaddress_ex_t *)addr;

    if(nlGetAddrFromName(address->name.c_str(), address->address.get())) {
		address->address.get()->valid = NL_TRUE;
		AddToDnsCache(address->name, *address->address.get());
	}
		// TODO: handle failures here? there should be, but we only have the valid field
		// For now, we leave it at false, so the timeout handling will just occur.

	if(SdlNetEvent_Inited) {
		// push a net event
		SendSDLUserEvent(&onDnsReady, EventData());
    }

    delete address;
    return 0;
}

bool GetNetAddrFromNameAsync(const std::string& name, NetworkAddr& addr)
{
	// We don't use nlGetAddrFromNameAsync here because we have to use SmartPointers
	// The problem is, if you call this and then delete the memory of the network address
	// while the thread isn't ready, it will write after to deleted memory.

	if(getNLaddr(addr) == NULL)
		return false;

	if (StringToNetAddr(name, addr))  {
		return true;
	}
	ResetSocketError(); // Clear the bad address error

	if(GetFromDnsCache(name, addr)) {
		SetNetAddrValid(addr, true);
		return true;
	}

    getNLaddr(addr)->valid = NL_FALSE;

    NLaddress_ex_t  *addr_ex;
    addr_ex = new NLaddress_ex_t;
    if(addr_ex == NULL)
        return false;
    addr_ex->name = name;
    addr_ex->address = *NetworkAddrData(&addr);

	NLthreadID thread = nlThreadCreate(GetAddrFromNameAsync_Internal, addr_ex, true);
    if(thread == NULL)
        return false;
    return true;
}


bool GetNetAddrFromName(const std::string& name, NetworkAddr& addr) {
	if(getNLaddr(addr) == NULL)
		return false;
	else {
		if(GetFromDnsCache( name, addr )) {
			SetNetAddrValid( addr, true );
			return true;
		}
		if( nlGetAddrFromName(name.c_str(), getNLaddr(addr)) != NL_FALSE ) {
			AddToDnsCache( name, addr );
			return true;
		}
		return false;
	}
}

bool isDataAvailable(NetworkSocket sock) {
	NLint group = nlGroupCreate();
	nlGroupAddSocket( group, *getNLsocket(&sock) );
	NLsocket sock_out[2];
	int ret = nlPollGroup( group, NL_READ_STATUS, sock_out, 1, 0 );
	nlGroupDestroy(group);
	return ret > 0;
}



