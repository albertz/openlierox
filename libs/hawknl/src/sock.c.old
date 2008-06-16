/*
  HawkNL cross platform network library
  Copyright (C) 2000-2002 Phil Frisbie, Jr. (phil@hawksoft.com)

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.
  
  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.
    
  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the
  Free Software Foundation, Inc., 59 Temple Place - Suite 330,
  Boston, MA  02111-1307, USA.
      
  Or go to http://www.gnu.org/copyleft/lgpl.html
*/


#define FD_SETSIZE      8192

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

/* SGI and MacOS X do not include socklen_t */
#if defined __sgi || defined MACOSX
typedef int socklen_t;
#endif

#endif /* WINDOWS_APP*/


#include "nlinternal.h"
#include "sock.h"

#ifndef IN_MULTICAST
#define IN_MULTICAST(i) (((unsigned long)(i) & 0xF0000000) == (unsigned long)0xE0000000)
#endif

#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN      NL_MAX_STRING_LENGTH
#endif

#define NL_CONNECT_STRING   "HawkNL request connection."
#define NL_REPLY_STRING     "HawkNL connection OK."
#define NL_HEADER_LEN       4

/*
* Portable NLMAX() function macro
*/
#define NLMAX( a, b )             ( ( a ) > ( b ) ? ( a ) : ( b ) )
#define NLMIN( a, b )             ( ( a ) < ( b ) ? ( a ) : ( b ) )

static volatile NLuint ouraddress, bindaddress;
static volatile int backlog = SOMAXCONN;
static volatile int multicastTTL = 1;
static volatile NLboolean reuseaddress = NL_FALSE;
static volatile NLboolean nlTCPNoDelay = NL_FALSE;

static NLaddress *alladdr = NULL;

typedef struct
{
    NLaddress   /*@temp@*/*address;
    NLchar      /*@temp@*/*name;
    NLsocket    socket;
} NLaddress_ex_t;

extern SOCKET nlGroupGetFdset(NLint group, /*@out@*/ fd_set *fd);

/*
This is a Winsock work around to be able to bind() to more than 3976 ports
*/

#ifdef WINDOWS_APP

static NLmutex  portlock; /* In memory of my step-father, Don Portlock,
who passed away Jan 12, 2001 - Phil */

static volatile NLushort nextport = 1024;

static NLushort sock_getNextPort(void)
{
    NLlong temp;

    (void)nlMutexLock(&portlock);
    temp = (NLlong)nextport;
    if(++temp > 65535)
    {
        /* skip the well known ports */
        temp = 1024;
    }
    nextport = (NLushort)temp;
    (void)nlMutexUnlock(&portlock);
    return nextport;
}

static NLint sock_bind(SOCKET socket, const struct sockaddr *a, int len)
{
    struct sockaddr_in  *addr = (struct sockaddr_in *)a;
    int                 ntries = 500; /* this is to prevent an infinite loop */
    NLboolean           found = NL_FALSE;
    
    /* check to see if the port is already specified */
    if(addr->sin_port != 0)
    {
        /* do the normal bind */
        return bind(socket, a, len);
    }
    
    /* let's find our own port number */
    while(ntries-- > 0)
    {
        addr->sin_port = htons(sock_getNextPort());
        if(bind(socket, (struct sockaddr *)addr, len) != SOCKET_ERROR)
        {
            found = NL_TRUE;
            break;
        }
    }
    if(found == NL_TRUE)
    {
        return 0;
    }
    /* could not find a port, restore the port number back to 0 */
    addr->sin_port = 0;
    /*  return error */
    return SOCKET_ERROR;
}

static int sock_connect(SOCKET socket, const struct sockaddr* a, int len )
{
    struct sockaddr_in  addr;

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = 0;

    if(sock_bind(socket, (struct sockaddr *)&addr, (int)sizeof(addr)) == SOCKET_ERROR)
    {
        return SOCKET_ERROR;
    }
    return connect(socket, a, len);
}

#else
#define sock_bind bind
#define sock_connect connect
#endif /* WINDOWS_APP */

/*
helper functions for NL_RELIABLE_PACKETS
*/

static NLint    rpGroup;                /* the group to hold all the NL_RELIABLE_PACKETS sockets */
static NLmutex  rpMutex;                /* mutex to lock the functions */
static NLboolean needThread = NL_TRUE;  /* do we need to spawn a thread? */
static NLint    rpSocketCount = 0;      /* total count of NL_RELIABLE_PACKETS sockets */
static volatile NLint rpBufferedCount = 0;/* count of sockets that are buffering data */

static void sock_WritePacketCheckPending(NLsocket socket)
{
    nl_socket_t *sock = nlSockets[socket];
    
    /* first check for data waiting to be sent */
    if(sock->sendlen > 0)
    {
        NLint       count, size = NL_MAX_PACKET_LENGTH + NL_HEADER_LEN;

        if(size > sock->sendlen)
        {
            size = sock->sendlen;
        }
        count = send((SOCKET)sock->realsocket, (char *)sock->outbuf, size, 0);
        if(count > 0)
        {
            sock->sendlen -= count;
            if(sock->sendlen > 0)
            {
                /* move remaining data to beginning of outbuf */
                memmove(sock->outbuf, (sock->outbuf + count), (size_t)sock->sendlen);
            }
            else
            {
                rpBufferedCount--;
            }
       }
    }
}

static void *sock_rpThread(void *p)
{
    static NLsocket *sockets = NULL;
    static NLint    maxsockets = 0;

    /* allocate memory */
    if(sockets == NULL)
    {
        sockets = (NLsocket *)malloc(sizeof(NLsocket *) * 16);
        if(sockets == NULL)
        {
            needThread = NL_TRUE;
            return NULL;
        }
        maxsockets = 16;
    }
    while(rpGroup != NL_INVALID)
    {
        NLint count = maxsockets;

        /* make sure there is something to do */
        if(rpSocketCount == 0 || rpBufferedCount == 0)
        {
            goto loopend;
        }
        /* see if we need to allocate more memory */
        if(maxsockets < rpSocketCount)
        {
            NLsocket *temp;
            
            while(maxsockets < rpSocketCount)
            {
                maxsockets *= 2;
            }
            temp = (NLsocket *)realloc((void *)sockets, sizeof(NLsocket *) * maxsockets);
            if(temp == NULL)
            {
                goto cleanup;
            }
            sockets = temp;
        }
        /* we can now get the sockets */
        if(nlGroupGetSockets(rpGroup, sockets, &count) == NL_FALSE)
        {
            goto cleanup;
        }
        while(count-- > 0 && rpBufferedCount > 0)
        {
            NLsocket socket = sockets[count];

            (void)nlLockSocket(socket, NL_WRITE);
            sock_WritePacketCheckPending(socket);
            nlUnlockSocket(socket, NL_WRITE);
        }
loopend:
        nlThreadSleep(50);
    }
cleanup:
    free(sockets);
    sockets = NULL;
    maxsockets = 0;
    needThread = NL_TRUE;
    return p;
}

static void sock_AddSocket(NLsocket socket)
{
    (void)nlMutexLock(&rpMutex);
    (void)nlGroupAddSocket(rpGroup, socket);
    rpSocketCount++;
    if(needThread == NL_TRUE)
    {
        (void)nlThreadCreate(sock_rpThread, NULL, NL_FALSE);
        needThread = NL_FALSE;
    }
    (void)nlMutexUnlock(&rpMutex);
}

static void sock_DeleteSocket(NLsocket socket)
{
    (void)nlMutexLock(&rpMutex);
    (void)nlGroupDeleteSocket(rpGroup, socket);
    rpSocketCount--;
    (void)nlMutexUnlock(&rpMutex);
}

/*
handle some common connection errors so the app knows when a connection has been dropped
*/

static NLint sock_Error(void)
{
    switch (sockerrno) {
        
#ifdef WINDOWS_APP
    case WSABASEERR:
        return 0;
#endif
        
    case EWOULDBLOCK:
        return 0;
        
    case ENETRESET:
    case EHOSTUNREACH:
    case ECONNABORTED:
    case ECONNRESET:
    case ENETUNREACH:
    case ETIMEDOUT:
        nlSetError(NL_SOCK_DISCONNECT);
        break;
        
    default:
        nlSetError(NL_SYSTEM_ERROR);
        break;
    }
    
    return NL_INVALID;
}

static NLboolean sock_SetNonBlocking(SOCKET socket)
{
    int rc;
    unsigned long i = 1;
    
    rc = ioctl(socket, FIONBIO, &i);
    if(rc == SOCKET_ERROR)
    {
        return NL_FALSE;
    }
    return NL_TRUE;
}

static NLboolean sock_SetBroadcast(SOCKET socket)
{
    int i = 1;
    
    if(setsockopt(socket, SOL_SOCKET, SO_BROADCAST, (char *)&i, (int)sizeof(i)) == SOCKET_ERROR)
    {
        nlSetError(NL_SYSTEM_ERROR);
        return NL_FALSE;
    }
    return NL_TRUE;
}

static void sock_SetReuseAddr(SOCKET socket)
{
    int i = 1;
    
    if(reuseaddress == NL_FALSE)
    {
        return;
    }
    if(setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, (char *)&i, (int)sizeof(i)) == SOCKET_ERROR)
    {
        nlSetError(NL_SYSTEM_ERROR);
    }
}

static void sock_SetTCPNoDelay(SOCKET socket)
{
    int i = 1;
    
    if(setsockopt(socket, IPPROTO_TCP, TCP_NODELAY, (char *)&i, (int)sizeof(i)) == SOCKET_ERROR)
    {
        nlSetError(NL_SYSTEM_ERROR);
    }
}

static NLboolean sock_SetMulticastTTL(SOCKET socket, NLint ttl)
{
    unsigned char   cttl;
    
    /* make sure we have a valid TTL */
    if(ttl > 255) ttl = 255;
    if(ttl < 1) ttl = 1;
    cttl = (unsigned char)ttl;
    
    /* first try setsockopt by passing a 'char', the Unix standard */
    if(setsockopt(socket, IPPROTO_IP, IP_MULTICAST_TTL,
        (char *)&cttl, (int)sizeof(cttl)) == SOCKET_ERROR)
    {
    /* if that failed, we might be on a Windows system
        that requires an 'int' */
        if(setsockopt(socket, IPPROTO_IP, IP_MULTICAST_TTL,
            (char *)&ttl, (int)sizeof(ttl)) == SOCKET_ERROR)
        {
            nlSetError(NL_SYSTEM_ERROR);
            return NL_FALSE;
        }
    }
    return NL_TRUE;
}

static NLsocket sock_SetSocketOptions(NLsocket s)
{
    nl_socket_t     *sock = nlSockets[s];
    NLenum          type = sock->type;
    SOCKET          realsocket = (SOCKET)sock->realsocket;
    
    if(type == NL_RELIABLE || type == NL_RELIABLE_PACKETS)
    {
        sock->reliable = NL_TRUE;
        if(nlTCPNoDelay == NL_TRUE)
        {
            sock_SetTCPNoDelay(realsocket);
        }
        if(type == NL_RELIABLE_PACKETS)
        {
            sock->packetsync = NL_TRUE;
        }
    }
    else
    {
        sock->reliable = NL_FALSE;
    }
    
    sock_SetReuseAddr(realsocket);
    
    if(sock->blocking == NL_FALSE)
    {
        if(sock_SetNonBlocking(realsocket) == NL_FALSE)
        {
            nlSetError(NL_SYSTEM_ERROR);
            return NL_INVALID;
        }
    }
    
    return s;
}

static NLuint sock_GetHostAddress(void)
{
    struct hostent  *local;
    char            buff[MAXHOSTNAMELEN];
    
    if(gethostname(buff, MAXHOSTNAMELEN) == SOCKET_ERROR)
    {
        return INADDR_NONE;
    }
    buff[MAXHOSTNAMELEN - 1] = '\0';
    local = gethostbyname(buff);
    if(!local)
        return (NLuint)htonl(0x7f000001);
    return *(NLuint *)local->h_addr_list[0];
}

static NLushort sock_GetPort(SOCKET socket)
{
    struct sockaddr_in   addr;
    socklen_t            len;
    
    len = (socklen_t)sizeof(struct sockaddr_in);
    if(getsockname(socket, (struct sockaddr *) &addr, &len) == SOCKET_ERROR)
    {
        return 0;
    }
    
    return ntohs(addr.sin_port);
}

NLboolean sock_Init(void)
{
#ifdef WINDOWS_APP
    WSADATA libmibWSAdata;
    
    /* first try Winsock 2.0 */
    if(WSAStartup(MAKEWORD(2, 0),&libmibWSAdata) != 0)
    {
        /* Winsock 2.0 failed, so now try 1.1 */
        if(WSAStartup(MAKEWORD(1, 1),&libmibWSAdata) != 0)
        {
            return NL_FALSE;
        }
    }
    if(nlMutexInit(&portlock) == NL_FALSE)
    {
        return NL_FALSE;
    }
#endif
    if(nlMutexInit(&rpMutex) == NL_FALSE)
    {
        return NL_FALSE;
    }
    if((rpGroup = nlGroupCreate()) == NL_INVALID)
    {
        return NL_FALSE;
    }
    ouraddress = sock_GetHostAddress();
    if(ouraddress == (NLuint)INADDR_NONE)
    {
        return NL_FALSE;
    }
    
    bindaddress = INADDR_ANY;
    return NL_TRUE;
}

void sock_Shutdown(void)
{
#ifdef WINDOWS_APP
    (void)WSACleanup();
    (void)nlMutexDestroy(&portlock);
    (void)nlMutexDestroy(&rpMutex);
    (void)nlGroupDestroy(rpGroup);
    rpGroup = NL_INVALID;
#endif
    if(alladdr != NULL)
    {
        free(alladdr);
        alladdr = NULL;
    }
}

NLboolean sock_Listen(NLsocket socket)
{
    nl_socket_t *sock = nlSockets[socket];

    if(sock->listen == NL_TRUE)
    {
        return NL_TRUE;
    }
    if(sock->reliable == NL_TRUE) /* TCP */
    {
        /* check for unbound socket */
        if(sock->localport == 0)
        {
            /* bind socket */
            ((struct sockaddr_in *)&sock->addressin)->sin_family = AF_INET;
            ((struct sockaddr_in *)&sock->addressin)->sin_addr.s_addr = bindaddress;
            ((struct sockaddr_in *)&sock->addressin)->sin_port = 0;

            if(sock_bind((SOCKET)sock->realsocket, (struct sockaddr *)&sock->addressin, (int)sizeof(struct sockaddr)) == SOCKET_ERROR)
            {
                nlSetError(NL_SYSTEM_ERROR);
                return NL_FALSE;
            }
        }
        if(listen((SOCKET)sock->realsocket, backlog) == SOCKET_ERROR)
        {
            nlSetError(NL_SYSTEM_ERROR);
            return NL_FALSE;
        }
    }

    sock->listen = NL_TRUE;
    return NL_TRUE;
}

static SOCKET sock_AcceptUDP(NLsocket nlsocket, /*@out@*/struct sockaddr_in *newaddr)
{
    nl_socket_t         *sock = nlSockets[nlsocket];
    struct sockaddr_in  ouraddr;
    SOCKET              newsocket;
    NLushort            localport;
    NLbyte              buffer[NL_MAX_STRING_LENGTH];
    socklen_t           len = (socklen_t)sizeof(struct sockaddr_in);
    NLint               slen = (NLint)sizeof(NL_CONNECT_STRING);
    NLbyte              reply = (NLbyte)0x00;
    NLint               count = 0;
    
    /* Get the packet and remote host address */
    if(recvfrom((SOCKET)sock->realsocket, buffer, (int)sizeof(buffer), 0,
        (struct sockaddr *)newaddr, &len) < (int)sizeof(NL_CONNECT_STRING))
    {
        nlSetError(NL_NO_PENDING);
        return INVALID_SOCKET;
    }
    /* Let's check for the connection string */
    buffer[slen - 1] = (NLbyte)0; /* null terminate for peace of mind */
    if(strcmp(buffer, NL_CONNECT_STRING) != 0)
    {
        nlSetError(NL_NO_PENDING);
        return INVALID_SOCKET;
    }
    /* open up a new socket on this end */
    newsocket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(newsocket == INVALID_SOCKET)
    {
        nlSetError(NL_SYSTEM_ERROR);
        (void)closesocket(newsocket);
        return INVALID_SOCKET;
    }
    
    ouraddr.sin_family = AF_INET;
    ouraddr.sin_addr.s_addr = bindaddress;
    /* system assigned port number */
    ouraddr.sin_port = 0;
    
    if(sock_bind(newsocket, (struct sockaddr *)&ouraddr, len) == SOCKET_ERROR)
    {
        nlSetError(NL_SYSTEM_ERROR);
        (void)closesocket(newsocket);
        return INVALID_SOCKET;
    }
    /* get the new port */
    localport = sock_GetPort(newsocket);
    
    /* create the return message */
    writeShort(buffer, count, localport);
    writeString(buffer, count, (NLchar *)TEXT(NL_REPLY_STRING));
    
    /* send back the reply with our new port */
    if(sendto((SOCKET)sock->realsocket, buffer, count, 0, (struct sockaddr *)newaddr,
        (int)sizeof(struct sockaddr_in)) < count)
    {
        nlSetError(NL_SYSTEM_ERROR);
        (void)closesocket(newsocket);
        return INVALID_SOCKET;
    }
    /* send back a 0 length packet from our new port, needed for firewalls */
    if(sendto(newsocket, &reply, 0, 0,
        (struct sockaddr *)newaddr,
        (int)sizeof(struct sockaddr_in)) < 0)
    {
        nlSetError(NL_SYSTEM_ERROR);
        (void)closesocket(newsocket);
        return INVALID_SOCKET;
    }
    /* connect the socket */
    if(connect(newsocket, (struct sockaddr *)newaddr,
        (int)sizeof(struct sockaddr_in)) == SOCKET_ERROR)
    {
        nlSetError(NL_SYSTEM_ERROR);
        (void)closesocket(newsocket);
        return INVALID_SOCKET;
    }
    
    return newsocket;
}

NLsocket sock_AcceptConnection(NLsocket socket)
{
    nl_socket_t         *sock = nlSockets[socket];
    nl_socket_t         *newsock = NULL;
    NLsocket            newsocket = NL_INVALID;
    SOCKET              realsocket = INVALID_SOCKET;
    struct sockaddr_in  newaddr;
    socklen_t           len = (socklen_t)sizeof(newaddr);
    
    memset(&newaddr, 0, sizeof(newaddr));
    if(sock->listen != NL_TRUE)
    {
        nlSetError(NL_NOT_LISTEN);
        return NL_INVALID;
    }
    
    if(sock->type == NL_RELIABLE || sock->type == NL_RELIABLE_PACKETS) /* TCP */
    {
        SOCKET s = 101;
        
        /* !@#$% metrowerks compiler, try to get it to actually produce some code */
        s = accept((SOCKET)(sock->realsocket), (struct sockaddr *)&newaddr, &len);
        
        realsocket = s;
        if(realsocket == INVALID_SOCKET)
        {
            if(sockerrno == EWOULDBLOCK || errno == EAGAIN)/* yes, we need to use errno here */
            {
                nlSetError(NL_NO_PENDING);
            }
            else
            {
                nlSetError(NL_SYSTEM_ERROR);
            }
            return NL_INVALID;
        }
        
    }
    else if(sock->type == NL_UNRELIABLE)/* UDP*/
    {
        realsocket = sock_AcceptUDP(socket, &newaddr);
        
        if(realsocket == INVALID_SOCKET)
        {
            /* error is already set in sock_AcceptUDP */
            return NL_INVALID;
        }
    }
    else /* broadcast or multicast */
    {
        nlSetError(NL_WRONG_TYPE);
        return NL_INVALID;
    }
    
    newsocket = nlGetNewSocket();
    if(newsocket == NL_INVALID)
    {
        return NL_INVALID;
    }
    if(nlLockSocket(newsocket, NL_BOTH) == NL_FALSE)
    {
        return NL_INVALID;
    }
    newsock = nlSockets[newsocket];
    
    /* update the remote address */
    memcpy((char *)&newsock->addressin, (char *)&newaddr, sizeof(struct sockaddr_in));
    newsock->realsocket = (NLint)realsocket;
    newsock->localport = sock_GetPort(realsocket);
    newsock->remoteport = sock_GetPortFromAddr((NLaddress *)&newsock->addressin);
    
    newsock->type = sock->type;
    newsock->connected = NL_TRUE;
    if(newsock->type == NL_RELIABLE_PACKETS)
    {
        sock_AddSocket(newsocket);
    }
    return sock_SetSocketOptions(newsocket);
}

NLsocket sock_Open(NLushort port, NLenum type)
{
    nl_socket_t *newsock;
    NLsocket    newsocket;
    SOCKET      realsocket;
    
    switch (type) {
        
    case NL_RELIABLE: /* TCP */
    case NL_RELIABLE_PACKETS: /* TCP packets */
        realsocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
        break;
        
    case NL_UNRELIABLE: /* UDP */
    case NL_BROADCAST:  /* UDP broadcast */
    case NL_UDP_MULTICAST:  /* UDP multicast */
        realsocket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
        break;
        
    default:
        nlSetError(NL_INVALID_ENUM);
        return NL_INVALID;
    }
    
    if(realsocket == INVALID_SOCKET)
    {
        nlSetError(NL_SYSTEM_ERROR);
        return NL_INVALID;
    }
    
    newsocket = nlGetNewSocket();
    if(newsocket == NL_INVALID)
    {
        return NL_INVALID;
    }
    if(nlLockSocket(newsocket, NL_BOTH) == NL_FALSE)
    {
        return NL_INVALID;
    }
    newsock = nlSockets[newsocket];
    newsock->realsocket = (NLint)realsocket;
    newsock->type = type;
    
    if(sock_SetSocketOptions(newsocket) == NL_INVALID)
    {
        nlUnlockSocket(newsocket, NL_BOTH);
        (void)sock_Close(newsocket);
        return NL_INVALID;
    }
    /* do not bind a TCP socket here if the port is 0; let connect assign the port */
    if((type == NL_RELIABLE || type == NL_RELIABLE_PACKETS) && port == 0)
    {
        newsock->localport = 0;
    }
    else
    {
        ((struct sockaddr_in *)&newsock->addressin)->sin_family = AF_INET;
        ((struct sockaddr_in *)&newsock->addressin)->sin_addr.s_addr = bindaddress;
        ((struct sockaddr_in *)&newsock->addressin)->sin_port = htons((unsigned short)port);
        
        if(sock_bind(realsocket, (struct sockaddr *)&newsock->addressin, (int)sizeof(struct sockaddr)) == SOCKET_ERROR)
        {
            nlSetError(NL_SYSTEM_ERROR);
            nlUnlockSocket(newsocket, NL_BOTH);
            (void)sock_Close(newsocket);
            return NL_INVALID;
        }
        if(type == NL_BROADCAST)
        {
            if(sock_SetBroadcast(realsocket) == NL_FALSE)
            {
                nlSetError(NL_SYSTEM_ERROR);
                nlUnlockSocket(newsocket, NL_BOTH);
                (void)sock_Close(newsocket);
                return NL_INVALID;
            }
            ((struct sockaddr_in *)&newsock->addressout)->sin_family = AF_INET;
            ((struct sockaddr_in *)&newsock->addressout)->sin_addr.s_addr = INADDR_BROADCAST;
            ((struct sockaddr_in *)&newsock->addressout)->sin_port = htons((unsigned short)port);
        }
        newsock->localport = sock_GetPort(realsocket);
    }
    if(type == NL_RELIABLE_PACKETS)
    {
        sock_AddSocket(newsocket);
    }
    nlUnlockSocket(newsocket, NL_BOTH);
    return newsocket;
}

static NLboolean sock_ConnectUDP(NLsocket socket, const NLaddress *address)
{
    nl_socket_t     *sock = nlSockets[socket];
    time_t          begin, t;
    
    if(sendto((SOCKET)sock->realsocket, (char *)NL_CONNECT_STRING, (NLint)sizeof(NL_CONNECT_STRING),
        0, (struct sockaddr *)address, (int)sizeof(struct sockaddr_in))
        == SOCKET_ERROR)
    {
        if(sock->blocking == NL_TRUE)
        {
            nlSetError(NL_SYSTEM_ERROR);
        }
        else
        {
            sock->conerror = NL_TRUE;
        }
        return NL_FALSE;
    }
    
    (void)time(&begin);
    
    /* try for six seconds */
    while((time(&t) - begin) < 6)
    {
        NLbyte              buffer[NL_MAX_STRING_LENGTH];
        NLbyte              *pbuffer = buffer;
        NLushort            newport;
        NLint               slen = (NLint)(sizeof(NL_REPLY_STRING) + sizeof(newport));
        NLint               received;
        NLbyte              reply = (NLbyte)0;
        socklen_t           len = (socklen_t)sizeof(struct sockaddr_in);
        
        received = recvfrom((SOCKET)sock->realsocket, (char *)buffer, (int)sizeof(buffer), 0,
            (struct sockaddr *)&sock->addressin, &len);
        
        if(received == SOCKET_ERROR)
        {
            if(sockerrno != EWOULDBLOCK)
            {
                if(sock->blocking == NL_TRUE)
                {
                    nlSetError(NL_CON_REFUSED);
                }
                else
                {
                    sock->conerror = NL_TRUE;
                }
                return NL_FALSE;
            }
        }
        if(received >= slen)
        {
            NLint count = 0;
            
            /* retrieve the port number */
            readShort(buffer, count, newport);
            ((struct sockaddr_in *)&sock->addressin)->sin_port = htons(newport);
            /* Lets check for the reply string */
            pbuffer[slen - 1] = (NLbyte)0; /* null terminate for peace of mind */
            pbuffer += sizeof(newport);
            if(strcmp(pbuffer, NL_REPLY_STRING) == 0)
            {
                if(connect((SOCKET)sock->realsocket, (struct sockaddr *)&sock->addressin,
                    (int)sizeof(struct sockaddr_in)) == SOCKET_ERROR)
                {
                    if(sock->blocking == NL_TRUE)
                    {
                        nlSetError(NL_SYSTEM_ERROR);
                    }
                    else
                    {
                        sock->conerror = NL_TRUE;
                    }
                    return NL_FALSE;
                }
                /* send back a 0 length packet to the new port, needed for firewalls */
                if(send((SOCKET)sock->realsocket, &reply, 0, 0) == SOCKET_ERROR)
                {
                    if(sock->blocking == NL_TRUE)
                    {
                        nlSetError(NL_SYSTEM_ERROR);
                    }
                    else
                    {
                        sock->conerror = NL_TRUE;
                    }
                    return NL_FALSE;
                }
                /* success! */
                sock->localport = sock_GetPort((SOCKET)sock->realsocket);
                sock->remoteport = sock_GetPortFromAddr((NLaddress *)&sock->addressin);
                sock->connected = NL_TRUE;
                sock->connecting = NL_FALSE;
                return NL_TRUE;
            }
        }
        nlThreadSleep(NL_CONNECT_SLEEP);
    }
    
    if(sock->blocking == NL_TRUE)
    {
        nlSetError(NL_CON_REFUSED);
    }
    else
    {
        sock->conerror = NL_TRUE;
    }
    return NL_FALSE;
}

static void *sock_ConnectUDPAsynchInt(void /*@owned@*/*addr)
{
    NLaddress_ex_t *address = (NLaddress_ex_t *)addr;
    
    (void)sock_ConnectUDP(address->socket, address->address);
    free(addr);
    return NULL;
}

static NLboolean sock_ConnectUDPAsynch(NLsocket socket, const NLaddress *address)
{
    NLaddress_ex_t  /*@dependent@*/*addr;
    nl_socket_t     *sock = nlSockets[socket];
    
    addr = (NLaddress_ex_t *)malloc(sizeof(NLaddress_ex_t));
    if(addr == NULL)
    {
        nlSetError(NL_OUT_OF_MEMORY);
        return NL_FALSE;
    }
    addr->address = (NLaddress *)address;
    addr->socket = socket;
    sock->connecting = NL_TRUE;
    sock->conerror = NL_FALSE;
    if(nlThreadCreate(sock_ConnectUDPAsynchInt, (void *)addr, NL_FALSE) == (NLthreadID)NL_INVALID)
    {
        return NL_FALSE;
    }
    return NL_TRUE;
}

static NLboolean sock_ConnectMulticast(NLsocket socket, const NLaddress *address)
{
    struct ip_mreq  mreq;
    nl_socket_t     *sock = nlSockets[socket];
    
    if(sock->reliable == NL_TRUE)
    {
        nlSetError(NL_WRONG_TYPE);
        return NL_FALSE;
    }
    if(!IN_MULTICAST(ntohl(((struct sockaddr_in *)address)->sin_addr.s_addr)))
    {
        nlSetError(NL_BAD_ADDR);
        return NL_FALSE;
    }
    
    memcpy((char *)&sock->addressin, (char *)address, sizeof(struct sockaddr_in));
    memcpy((char *)&sock->addressout, (char *)address, sizeof(struct sockaddr_in));
    
    /* join the multicast group */
    mreq.imr_multiaddr.s_addr = ((struct sockaddr_in *)address)->sin_addr.s_addr;
    mreq.imr_interface.s_addr = bindaddress;
    
    if(setsockopt((SOCKET)sock->realsocket, IPPROTO_IP, IP_ADD_MEMBERSHIP,
        (char *)&mreq, (int)sizeof(mreq)) == SOCKET_ERROR)
    {
        nlSetError(NL_SYSTEM_ERROR);
        return NL_FALSE;
    }
    sock->localport = sock_GetPort((SOCKET)sock->realsocket);
    sock->remoteport = sock_GetPortFromAddr((NLaddress *)&sock->addressout);
    
    return sock_SetMulticastTTL((SOCKET)sock->realsocket, multicastTTL);
}

NLboolean sock_Connect(NLsocket socket, const NLaddress *address)
{
    nl_socket_t *sock = nlSockets[socket];
    
    memcpy((char *)&sock->addressin, (char *)address, sizeof(NLaddress));
    
    if(sock->type == NL_RELIABLE || sock->type == NL_RELIABLE_PACKETS)
    {
        
        if(sock_connect((SOCKET)sock->realsocket, (struct sockaddr *)&sock->addressin,
            (int)sizeof(struct sockaddr_in)) == SOCKET_ERROR)
        {
            if(sock->blocking == NL_FALSE &&
                (sockerrno == EWOULDBLOCK || sockerrno == EINPROGRESS))
            {
                sock->connecting = NL_TRUE;
            }
            else
            {
                nlSetError(NL_SYSTEM_ERROR);
                return NL_FALSE;
            }
        }
        sock->localport = sock_GetPort((SOCKET)sock->realsocket);
        sock->remoteport = sock_GetPortFromAddr((NLaddress *)&sock->addressin);
        sock->connected = NL_TRUE;
        return NL_TRUE;
    }
    else if(sock->type == NL_UDP_MULTICAST)
    {
        return sock_ConnectMulticast(socket, &sock->addressin);
    }
    else if(sock->type == NL_UNRELIABLE)
    {
        if(sock->blocking == NL_TRUE)
        {
            return sock_ConnectUDP(socket, &sock->addressin);
        }
        else
        {
            return sock_ConnectUDPAsynch(socket, &sock->addressin);
        }
    }
    else
    {
        nlSetError(NL_WRONG_TYPE);
    }
    return NL_FALSE;
}

void sock_Close(NLsocket socket)
{
    nl_socket_t     *sock = nlSockets[socket];
    struct ip_mreq  mreq;
    
    if(sock->type == NL_UDP_MULTICAST)
    {
        /* leave the multicast group */
        mreq.imr_multiaddr.s_addr = ((struct sockaddr_in *)&sock->addressout)->sin_addr.s_addr;
        mreq.imr_interface.s_addr = bindaddress;
        
        (void)setsockopt((SOCKET)sock->realsocket, IPPROTO_IP, IP_DROP_MEMBERSHIP,
            (char *)&mreq, (int)sizeof(mreq));
    }
    if(sock->type == NL_RELIABLE_PACKETS)
    {
        /* check for unsent data */
        if(sock->sendlen > 0)
        {
            int tries = 200;

            /* 200 * 50 ms = up to a 10 second delay to allow data to be sent */
            while(tries-- > 0 && sock->sendlen > 0)
            {
                nlThreadSleep(50);
            }
        }
        sock_DeleteSocket(socket);
    }
    if((sock->type == NL_RELIABLE_PACKETS || sock->type == NL_RELIABLE) && sock->listen == NL_FALSE)
    {
        struct linger l = {1, 10};

        (void)setsockopt((SOCKET)sock->realsocket, SOL_SOCKET, SO_LINGER, (const char *)&l, (int)sizeof(l));
    }
    (void)closesocket((SOCKET)sock->realsocket);
}

/* internal function to read reliable packets from TCP stream */
static NLint sock_ReadPacket(NLsocket socket, NLvoid /*@out@*/ *buffer, NLint nbytes,
                             NLboolean checkonly)
{
    nl_socket_t *sock = nlSockets[socket];
    
    /* skip error reporting if checkonly is TRUE */
    /* check for sync */
    if(sock->packetsync == NL_FALSE)
    {
        if(checkonly == NL_FALSE)
        {
            nlSetError(NL_PACKET_SYNC);
        }
        else
        {
            sock->readable = NL_TRUE;
        }
        return NL_INVALID;
    }
    
    if(sock->message_end == NL_TRUE && checkonly == NL_FALSE && sock->reclen == 0)
    {
        sock->message_end = NL_FALSE;
        nlSetError(NL_MESSAGE_END);
        return NL_INVALID;
    }
    /* allocate some temp storage the first time through */
    if(sock->inbuf == NULL)
    {
        sock->inbuflen = (nbytes + nbytes / 4 + NL_HEADER_LEN);
        
        if(sock->inbuflen < 1024)
        {
            sock->inbuflen = 1024;
        }
        else if(sock->inbuflen > NL_MAX_PACKET_LENGTH + NL_MAX_PACKET_LENGTH/4)
        {
            sock->inbuflen = NL_MAX_PACKET_LENGTH + NL_MAX_PACKET_LENGTH/4;
        }
        sock->inbuf = (NLbyte *)malloc((size_t)sock->inbuflen);
        if(sock->inbuf == NULL)
        {
            if(checkonly == NL_FALSE)
            {
                sock->inbuflen = 0;
                nlSetError(NL_OUT_OF_MEMORY);
            }
            return NL_INVALID;
        }
    }
    /* if inbuf is empty, get some data */
    if(sock->reclen < NL_HEADER_LEN)
    {
        NLint count;
        
        count = recv((SOCKET)sock->realsocket, (char *)(sock->inbuf + sock->reclen), (sock->inbuflen  - sock->reclen), 0);
        if(count == SOCKET_ERROR)
        {
            if(checkonly == NL_FALSE)
            {
                return sock_Error();
            }
            else
            {
                sock->readable = NL_TRUE;
                return NL_INVALID;
            }
        }
        if(count == 0)
        {
            if(checkonly == NL_FALSE)
            {
                nlSetError(NL_MESSAGE_END);
            }
            else
            {
                sock->message_end = NL_TRUE;
                sock->readable = NL_TRUE;
            }
            return NL_INVALID;
        }
        sock->reclen += count;
    }
    /* start parsing the packet */
    if(sock->reclen >= NL_HEADER_LEN)
    {
        NLboolean   done = NL_FALSE;
        NLushort    len;
        NLint       c = 2;
        
        /* check for valid packet */
        if(sock->inbuf[0] != 'N' || sock->inbuf[1] != 'L')
        {
        /* packet is not valid, we are somehow out of sync,
            or we are talking to a regular TCP stream */
            if(checkonly == NL_FALSE)
            {
                nlSetError(NL_PACKET_SYNC);
            }
            else
            {
                sock->readable = NL_TRUE;
            }
            sock->packetsync = NL_FALSE;
            return NL_INVALID;
        }
        
        /* read the length of the packet */
        readShort(sock->inbuf, c, len);
        if(len > NL_MAX_PACKET_LENGTH)
        {
            /* packet is not valid, or we are talking to a regular TCP stream */
            if(checkonly == NL_FALSE)
            {
                nlSetError(NL_PACKET_SYNC);
            }
            else
            {
                sock->readable = NL_TRUE;
            }
            sock->packetsync = NL_FALSE;
            return NL_INVALID;
        }
        /* check to see if we need to make the inbuf storage larger */
        if((NLint)len > sock->inbuflen)
        {
            NLint   newbuflen;
            NLbyte  *temp;

            newbuflen = (len + len / 4 + NL_HEADER_LEN);
            temp = (NLbyte *)realloc(sock->inbuf, (size_t)newbuflen);
            if(temp == NULL)
            {
                if(checkonly == NL_FALSE)
                {
                    nlSetError(NL_OUT_OF_MEMORY);
                }
                return NL_INVALID;
            }
            sock->inbuf = temp;
            sock->inbuflen = newbuflen;
        }
        if(checkonly == NL_FALSE)
        {
            if(len > (NLushort)nbytes)
            {
                nlSetError(NL_BUFFER_SIZE);
                return NL_INVALID;
            }
        }
        /* see if we need to get more of the packet */
        if(len > (NLushort)(sock->reclen - c))
        {
            done = NL_FALSE;
            while(done == NL_FALSE)
            {
                NLint count;
                
                if(checkonly == NL_FALSE)
                {
                    count = recv((SOCKET)sock->realsocket,
                        (char *)(sock->inbuf + sock->reclen),
                        (sock->inbuflen - sock->reclen), 0);
                }
                else
                {
                    /* we are calling this from PollGroup, so it cannot block */
                    fd_set          fdset;
                    struct timeval  t = {0,0};
                    
                    FD_ZERO(&fdset);
                    FD_SET((SOCKET)sock->realsocket, &fdset);
                    if(select(sock->realsocket + 1, &fdset, NULL, NULL, &t) == 1)
                    {
                        count = recv((SOCKET)sock->realsocket,
                            (char *)(sock->inbuf + sock->reclen),
                            (sock->inbuflen - sock->reclen), 0);
                    }
                    else
                    {
                        /* socket would block, so break */
                        break;
                    }
                }
                if(count == SOCKET_ERROR)
                {
                    if(checkonly == NL_FALSE)
                    {
                        /* check to see if we already have all the packet */
                        if(len <= (NLushort)(sock->reclen - c))
                        {
                            done = NL_TRUE;
                        }
                        else
                        {
                            /* report the error */
                            return sock_Error();
                        }
                    }
                    else
                    {
                        done = NL_TRUE;
                    }
                }
                else if(count == 0)
                {
                    sock->message_end = NL_TRUE;
                    sock->readable = NL_TRUE;
                    done = NL_TRUE;
                }
                else
                {
                    sock->reclen += count;
                    if(len <= (NLushort)(sock->reclen - c))
                    {
                        done = NL_TRUE;
                    }
                }
            }
        }
        /* see if we now have all of the packet */
        if(len <= (NLushort)(sock->reclen - c))
        {
            sock->readable = NL_TRUE;
            
            if(checkonly == NL_FALSE)
            {
                /* copy the packet */
                memcpy(buffer, (sock->inbuf + c), (size_t)len);
                
                /* check for another packet */
                sock->reclen -= (len + c);
                if(sock->reclen > 0)
                {
                    /* move it down to the beginning of inbuf */
                    memmove(sock->inbuf, (sock->inbuf + c + len), (size_t)sock->reclen);
                }
                /* quick check to see if we have another complete packet buffered */
                if(sock->reclen >= NL_HEADER_LEN)
                {
                    NLushort templen;
                    
                    /* read the length of the packet */
                    c = 2;
                    readShort(sock->inbuf, c, templen);
                    
                    /* check the length */
                    if(templen <= (NLushort)(sock->reclen - c))
                    {
                        /* we have another complete packet, so mark as readable for PollGroup */
                        sock->readable = NL_TRUE;
                    }
                    else
                    {
                        sock->readable = NL_FALSE;
                    }
                }
                else
                {
                    sock->readable = NL_FALSE;
                }
                return (NLint)len;
            }
        }
        else
        {
            return 0;
        }
    }
    return 0;
}

NLint sock_Read(NLsocket socket, NLvoid *buffer, NLint nbytes)
{
    nl_socket_t *sock = nlSockets[socket];
    NLint       count;
    
    if(nbytes < 0)
    {
        return 0;
    }
    if(sock->type == NL_RELIABLE || sock->type == NL_RELIABLE_PACKETS) /* TCP */
    {
        /* check for a non-blocking connection pending */
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
                    return NL_INVALID;
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
                return NL_INVALID;
            }
        }
        /* check for reliable packets */
        if(sock->type == NL_RELIABLE_PACKETS)
        {
            return sock_ReadPacket(socket, buffer, nbytes, NL_FALSE);
        }
        count = recv((SOCKET)sock->realsocket, (char *)buffer, nbytes, 0);
        if(count == 0)
        {
            /* end of message */
            nlSetError(NL_MESSAGE_END);
            return NL_INVALID;
        }
    }
    else /* UDP */
    {
        /* check for a non-blocking connection pending */
        if(sock->connecting == NL_TRUE)
        {
            nlSetError(NL_CON_PENDING);
            return NL_INVALID;
        }
        /* check for a connection error */
        if(sock->conerror == NL_TRUE)
        {
            nlSetError(NL_CON_REFUSED);
            return NL_INVALID;
        }
        if(sock->connected == NL_TRUE)
        {
            count = recv((SOCKET)sock->realsocket, (char *)buffer, nbytes, 0);
        }
        else
        {
            socklen_t   len = (socklen_t)sizeof(struct sockaddr_in);
            
            count = recvfrom((SOCKET)sock->realsocket, (char *)buffer, nbytes, 0,
                (struct sockaddr *)&sock->addressin, &len);
        }
    }
    if(count == SOCKET_ERROR)
    {
        return sock_Error();
    }
    return count;
}

static NLboolean allocateBuffer(nl_socket_t *sock, NLint nbytes)
{
    NLint size = NLMIN(NLMAX((nbytes + NL_HEADER_LEN) * 2, 1024), (NL_MAX_PACKET_LENGTH + NL_HEADER_LEN));

    /* first call */
    if(sock->outbuf == NULL)
    {
        sock->outbuf = (NLbyte *)malloc((size_t)size);
        if(sock->outbuf == NULL)
        {
            sock->outbuflen = 0;
            nlSetError(NL_OUT_OF_MEMORY);
            return NL_FALSE;
        }
        sock->outbuflen = size;
    }
    else
    {
        if(size > sock->outbuflen)
        {
            NLbyte *temp;

            temp = (NLbyte *)realloc(sock->outbuf, (size_t)size);
            if(temp == NULL)
            {
                sock->outbuflen = 0;
                nlSetError(NL_OUT_OF_MEMORY);
                return NL_FALSE;
            }
            sock->outbuf = temp;
            sock->outbuflen = size;
        }
    }
    return NL_TRUE;
}

static NLint sock_WritePacket(NLsocket socket, const NLvoid *buffer, NLint nbytes)
{
    nl_socket_t *sock = nlSockets[socket];
    NLint       count;
    NLbyte      temp[NL_HEADER_LEN];
    NLint       c = 0;
    
    
    /* allocate memory for outbuf */
    if(sock->outbuf == NULL)
    {
        if(allocateBuffer(sock, nbytes) == NL_FALSE)
        {
            return NL_INVALID;
        }
    }
    else
    {
        /* send any unsent data from last packet */
        sock_WritePacketCheckPending(socket);
    }
    
    /* check to see if we already have some pending data */
    if(sock->sendlen > 0)
    {
        /* to comply with the way UDP packets act */
        /* if the send buffer has unsent data return 0 */
        return 0;
    }

    /* ID for packets is 'NL'*/
    writeByte(temp, c, 'N');
    writeByte(temp, c, 'L');
    /* add the packet length */
    writeShort(temp, c, (NLushort)nbytes);
    
    count = send((SOCKET)sock->realsocket, (char *)temp, c, 0);

    if(count == SOCKET_ERROR)
    {
        if(sockerrno == EWOULDBLOCK)
        {
            count = 0;
        }
        else
        {
            return sock_Error();
        }
    }
    if(count < c)
    {
        int dif = c - count;
        
        /* check outbuf size */
        if(allocateBuffer(sock, nbytes) == NL_FALSE)
        {
            return NL_INVALID;
        }
        /* store it */
        memcpy((sock->outbuf + sock->sendlen), (temp + count), (size_t)(dif));
        sock->sendlen += (dif);
        memcpy((sock->outbuf + sock->sendlen), ((NLbyte *)buffer), (size_t)(nbytes));
        sock->sendlen += (nbytes);
        rpBufferedCount++;
    }
    else
    {
        count = send((SOCKET)sock->realsocket, (char *)buffer, nbytes, 0);
        if(count == SOCKET_ERROR)
        {
            if(sockerrno == EWOULDBLOCK)
            {
                count = 0;
            }
            else
            {
                return sock_Error();
            }
        }
        /* make sure all was sent */
        if(count < nbytes)
        {
            int dif = nbytes - count;
            
            /* check outbuf size */
            if(allocateBuffer(sock, nbytes) == NL_FALSE)
            {
                return NL_INVALID;
            }
            /* store it */
            memcpy((sock->outbuf + sock->sendlen), ((NLbyte *)buffer + count), (size_t)(dif));
            sock->sendlen += dif;
            rpBufferedCount++;
        }
    }
    count = nbytes;
    return count;
}

NLint sock_Write(NLsocket socket, const NLvoid *buffer, NLint nbytes)
{
    nl_socket_t *sock = nlSockets[socket];
    NLint       count;
    
    if(nbytes < 0)
    {
        return 0;
    }
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
                    return NL_INVALID;
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
                return NL_INVALID;
            }
        }
        /* check for reliable packets */
        if(sock->type == NL_RELIABLE_PACKETS)
        {
            if(nbytes > NL_MAX_PACKET_LENGTH)
            {
                nlSetError(NL_PACKET_SIZE);
                return NL_INVALID;
            }
            return sock_WritePacket(socket, buffer, nbytes);
        }
        count = send((SOCKET)sock->realsocket, (char *)buffer, nbytes, 0);
    }
    else /* unconnected UDP */
    {
        /* check for a non-blocking connection pending */
        if(sock->connecting == NL_TRUE)
        {
            nlSetError(NL_CON_PENDING);
            return NL_INVALID;
        }
        /* check for a connection error */
        if(sock->conerror == NL_TRUE)
        {
            nlSetError(NL_CON_REFUSED);
            return NL_INVALID;
        }
        if(nbytes > NL_MAX_PACKET_LENGTH)
        {
            nlSetError(NL_PACKET_SIZE);
            return NL_INVALID;
        }
        if(sock->type == NL_BROADCAST)
        {
            ((struct sockaddr_in *)&sock->addressin)->sin_addr.s_addr = INADDR_BROADCAST;
        }
        if(sock->type == NL_UDP_MULTICAST)
        {
            count = sendto((SOCKET)sock->realsocket, (char *)buffer, nbytes, 0,
                (struct sockaddr *)&sock->addressout,
                (int)sizeof(struct sockaddr_in));
        }
        else if(sock->connected == NL_TRUE)
        {
            count = send((SOCKET)sock->realsocket, (char *)buffer, nbytes, 0);
        }
        else
        {
            count = sendto((SOCKET)sock->realsocket, (char *)buffer, nbytes, 0,
                (struct sockaddr *)&sock->addressout,
                (int)sizeof(struct sockaddr_in));
        }
    }
    if(count == SOCKET_ERROR)
    {
        return sock_Error();
    }
    return count;
}

NLchar *sock_AddrToString(const NLaddress *address, NLchar *string)
{
    NLulong     addr;
    NLushort    port;
    
    addr = ntohl(((struct sockaddr_in *)address)->sin_addr.s_addr);
    port = ntohs(((struct sockaddr_in *)address)->sin_port);
    if(port == 0)
    {
        _stprintf(string, TEXT("%lu.%lu.%lu.%lu"), (addr >> 24) & 0xff, (addr >> 16)
            & 0xff, (addr >> 8) & 0xff, addr & 0xff);
    }
    else
    {
        _stprintf(string, TEXT("%lu.%lu.%lu.%lu:%u"), (addr >> 24) & 0xff, (addr >> 16)
            & 0xff, (addr >> 8) & 0xff, addr & 0xff, port);
    }
    return string;
}

NLboolean sock_StringToAddr(const NLchar *string, NLaddress *address)
{
    NLulong     a1, a2, a3, a4;
    NLulong     ipaddress, port = 0;
    int         ret;

    ret = _stscanf((const NLchar *)string, (const NLchar *)TEXT("%lu.%lu.%lu.%lu:%lu"), &a1, &a2, &a3, &a4, &port);

    if(a1 > 255 || a2 > 255 || a3 > 255 || a4 > 255 || port > 65535 || ret < 4)
    {
        /* bad address */
        ((struct sockaddr_in *)address)->sin_family = AF_INET;
        ((struct sockaddr_in *)address)->sin_addr.s_addr = INADDR_NONE;
        ((struct sockaddr_in *)address)->sin_port = 0;
        nlSetError(NL_BAD_ADDR);
        address->valid = NL_FALSE;
        return NL_FALSE;
    }
    else
    {
        ipaddress = (a1 << 24) | (a2 << 16) | (a3 << 8) | a4;
        ((struct sockaddr_in *)address)->sin_family = AF_INET;
        ((struct sockaddr_in *)address)->sin_addr.s_addr = htonl(ipaddress);
        ((struct sockaddr_in *)address)->sin_port = htons((NLushort)port);
        address->valid = NL_TRUE;
        return NL_TRUE;
    }
}

NLboolean sock_GetLocalAddr(NLsocket socket, NLaddress *address)
{
    nl_socket_t *sock = nlSockets[socket];
    socklen_t   len;
    
    memset(address, 0, sizeof(NLaddress));
    ((struct sockaddr_in *)address)->sin_family = AF_INET;
    address->valid = NL_TRUE;
    len = (socklen_t)sizeof(struct sockaddr_in);
    /* if the socket is connected, this will get us
    the correct address on a multihomed system*/
    if(getsockname((SOCKET)sock->realsocket, (struct sockaddr *)address, &len) == SOCKET_ERROR)
    {
        /* ignore error if socket has not been bound or connected yet */
        if(sockerrno != EINVAL)
        {
            nlSetError(NL_SYSTEM_ERROR);
            address->valid = NL_FALSE;
            return NL_FALSE;
        }
    }
    
    /* if not connected, substitute the NIC address */
    if(((struct sockaddr_in *)address)->sin_addr.s_addr == INADDR_ANY)
    {
        ((struct sockaddr_in *)address)->sin_addr.s_addr = ouraddress;
    }
    sock_SetAddrPort(address, sock->localport);
    return NL_TRUE;
}

NLaddress *sock_GetAllLocalAddr(NLint *count)
{
    struct hostent  *local;
    char            buff[MAXHOSTNAMELEN];
    int             i = 0;
    
    if(gethostname(buff, MAXHOSTNAMELEN) == SOCKET_ERROR)
    {
        nlSetError(NL_SYSTEM_ERROR);
        return NULL;
    }
    buff[MAXHOSTNAMELEN - 1] = '\0';
    local = gethostbyname(buff);
    if(local == NULL)
    {
        if(sockerrno == ENETDOWN)
        {
            nlSetError(NL_SYSTEM_ERROR);
            return NULL;
        }
    }
    /* count the number of returned IP addresses */
    *count = 0;
    if(local != NULL)
    {
        while(local->h_addr_list[i++] != NULL)
        {
            (*count)++;
        }
    }
    /* allocate storage for address */
    if(alladdr != NULL)
    {
        free(alladdr);
    }
    if(*count == 0)
    {
        *count = 1;
        alladdr = (NLaddress *)malloc(sizeof(NLaddress));
        memset(alladdr, 0, sizeof(NLaddress));
        
        /* fill in the localhost address */
        ((struct sockaddr_in *)alladdr)->sin_family = AF_INET;
        ((struct sockaddr_in *)alladdr)->sin_addr.s_addr = (NLuint)htonl(0x7f000001);
        alladdr->valid = NL_TRUE;
    }
    else
    {
        alladdr = (NLaddress *)malloc(sizeof(NLaddress) * *count);
        memset(alladdr, 0, sizeof(NLaddress) * *count);
        
        /* fill in the addresses */
        i = 0;
        while(local->h_addr_list[i] != NULL)
        {
            NLaddress *addr = &alladdr[i];
            
            ((struct sockaddr_in *)addr)->sin_family = AF_INET;
            ((struct sockaddr_in *)addr)->sin_addr.s_addr = *(NLuint *)local->h_addr_list[i];
            addr->valid = NL_TRUE;
            i++;
        }
    }
    return alladdr;
}

NLboolean sock_SetLocalAddr(const NLaddress *address)
{
    /* should we check against all the local addresses? */
    bindaddress = ouraddress = (NLuint)((struct sockaddr_in *)address)->sin_addr.s_addr;
    return NL_TRUE;
}

NLchar *sock_GetNameFromAddr(const NLaddress *address, NLchar *name)
{
    struct hostent  *hostentry;
    NLchar          tempname[MAXHOSTNAMELEN];
    
    hostentry = gethostbyaddr((char *)&((struct sockaddr_in *)address)->sin_addr,
        (int)sizeof(struct in_addr), AF_INET);
    if(hostentry != NULL)
    {
        NLushort port = sock_GetPortFromAddr(address);
#ifdef _UNICODE
        NLchar temp[MAXHOSTNAMELEN];
        /* convert from multibyte char string to wide char string */
        mbstowcs(temp, (const char *)hostentry->h_name, MAXHOSTNAMELEN);
        temp[MAXHOSTNAMELEN - 1] = '\0';
#else
        NLchar *temp = (NLchar *)hostentry->h_name;
#endif
        if(port != 0)
        {
            _sntprintf(tempname, (size_t)(NL_MAX_STRING_LENGTH), (const NLchar *)TEXT("%s:%hu"), (const NLchar *)temp, port);
        }
        else
        {
            _tcsncpy(tempname, (const NLchar *)temp, (size_t)(NL_MAX_STRING_LENGTH));
        }
        tempname[NL_MAX_STRING_LENGTH - 1] = (NLchar)'\0';
    }
    else
    {
        if(((struct sockaddr_in *)address)->sin_addr.s_addr == (unsigned long)INADDR_NONE)
        {
            _tcsncpy(tempname, (const NLchar *)TEXT("Bad address"), (size_t)(NL_MAX_STRING_LENGTH));
        }
        else
        {
            (void)sock_AddrToString(address, tempname);
        }
    }
    /* special copy in case this was called as sock_GetNameFromAddrAsync */
    name[0] = (NLchar)'\0';
    _tcsncpy(&name[1], (const NLchar *)&tempname[1], (size_t)(NL_MAX_STRING_LENGTH - 1));
    name[0] = tempname[0];
    return name;
}

static void *sock_GetNameFromAddrAsyncInt(void /*@owned@*/ * addr)
{
    NLaddress_ex_t *address = (NLaddress_ex_t *)addr;
    
    (void)sock_GetNameFromAddr(address->address, address->name);
    free(address->address);
    free(address);
    return NULL;
}

NLboolean sock_GetNameFromAddrAsync(const NLaddress *address, NLchar *name)
{
    NLaddress_ex_t  *addr;
    
    memset(name, 0, sizeof(NLchar) * NL_MAX_STRING_LENGTH);
    addr = (NLaddress_ex_t *)malloc(sizeof(NLaddress_ex_t));
    if(addr == NULL)
    {
        nlSetError(NL_OUT_OF_MEMORY);
        return NL_FALSE;
    }
    addr->address = (NLaddress *)malloc(sizeof(NLaddress));
    if(addr->address == NULL)
    {
        nlSetError(NL_OUT_OF_MEMORY);
        free(addr);
        return NL_FALSE;
    }
    memcpy(addr->address, address, sizeof(NLaddress));
    addr->name = name;
    if(nlThreadCreate(sock_GetNameFromAddrAsyncInt, (void *)addr, NL_FALSE) == (NLthreadID)NL_INVALID)
    {
        return NL_FALSE;
    }
    return NL_TRUE;
}

NLboolean sock_GetAddrFromName(const NLchar *name, NLaddress *address)
{
    struct hostent *hostentry;
    NLushort    port = 0;
    int			pos;
    NLbyte      temp[NL_MAX_STRING_LENGTH];
    
    address->valid = NL_FALSE;
    /* first check to see if we have a numeric address already */
    (void)sock_StringToAddr(name, address);
    /* clear out an NL_BAD_ADDR error */
    if(nlGetError() == NL_BAD_ADDR)
    {
        nlSetError(NL_NO_ERROR);
    }
    if(((struct sockaddr_in *)address)->sin_addr.s_addr != (unsigned long)INADDR_NONE)
    {
        /* we are already done! */
        address->valid = NL_TRUE;
        return NL_TRUE;
    }
    
#ifdef _UNICODE
    /* convert from wide char string to multibyte char string */
    (void)wcstombs(temp, (const NLchar *)name, NL_MAX_STRING_LENGTH);
#else
    strncpy(temp, name, NL_MAX_STRING_LENGTH);
#endif
    temp[NL_MAX_STRING_LENGTH - 1] = (NLbyte)'\0';
    pos = (int)strcspn(temp, (const char *)":");
    if(pos > 0)
    {
        NLbyte      *p = &temp[pos+1];

        temp[pos] = (NLbyte)'\0';
        (void)sscanf(p, "%hu", &port);
    }
    hostentry = gethostbyname((const char *)temp);

    if(hostentry != NULL)
    {
        ((struct sockaddr_in *)address)->sin_family = AF_INET;
        ((struct sockaddr_in *)address)->sin_port = htons(port);
        ((struct sockaddr_in *)address)->sin_addr.s_addr = *(NLulong *)hostentry->h_addr_list[0];
        address->valid = NL_TRUE;
    }
    else
    {
        ((struct sockaddr_in *)address)->sin_family = AF_INET;
        ((struct sockaddr_in *)address)->sin_addr.s_addr = INADDR_NONE;
        ((struct sockaddr_in *)address)->sin_port = 0;
        nlSetError(NL_SYSTEM_ERROR);
        return NL_FALSE;
    }
    return NL_TRUE;
}

static void *sock_GetAddrFromNameAsyncInt(void /*@owned@*/ *addr)
{
    NLaddress_ex_t *address = (NLaddress_ex_t *)addr;
    
    (void)sock_GetAddrFromName(address->name, address->address);
    address->address->valid = NL_TRUE;
    free(address->name);
    free(address);
    return NULL;
}

NLboolean sock_GetAddrFromNameAsync(const NLchar *name, NLaddress *address)
{
    NLaddress_ex_t  *addr;
    
    address->valid = NL_FALSE;
    addr = (NLaddress_ex_t *)malloc(sizeof(NLaddress_ex_t));
    if(addr == NULL)
    {
        nlSetError(NL_OUT_OF_MEMORY);
        return NL_FALSE;
    }
    addr->name = (NLchar *)malloc(NL_MAX_STRING_LENGTH);
    if(addr->name == NULL)
    {
        nlSetError(NL_OUT_OF_MEMORY);
        free(addr);
        return NL_FALSE;
    }
    _tcsncpy(addr->name, name, (size_t)NL_MAX_STRING_LENGTH);
    addr->name[NL_MAX_STRING_LENGTH - 1] = '\0';
    addr->address = address;
    if(nlThreadCreate(sock_GetAddrFromNameAsyncInt, (void *)addr, NL_FALSE) == (NLthreadID)NL_INVALID)
    {
        return NL_FALSE;
    }
    return NL_TRUE;
}

NLboolean sock_AddrCompare(const NLaddress *address1, const NLaddress *address2)
{
    if(((struct sockaddr_in *)address1)->sin_family != ((struct sockaddr_in *)address2)->sin_family)
        return NL_FALSE;
    
    if(((struct sockaddr_in *)address1)->sin_addr.s_addr
        != ((struct sockaddr_in *)address2)->sin_addr.s_addr)
        return NL_FALSE;
    
    if(((struct sockaddr_in *)address1)->sin_port
        != ((struct sockaddr_in *)address2)->sin_port)
        return NL_FALSE;
    
    return NL_TRUE;
}

NLushort sock_GetPortFromAddr(const NLaddress *address)
{
    return ntohs(((struct sockaddr_in *)address)->sin_port);
}

void sock_SetAddrPort(NLaddress *address, NLushort port)
{
    ((struct sockaddr_in *)address)->sin_port = htons((NLushort)port);
}

NLint sock_GetSystemError(void)
{
    NLint err = sockerrno;
    
#ifdef WINDOWS_APP
    if(err < WSABASEERR)
    {
        if(errno > 0)
        {
            err = errno;
        }
    }
#endif
    return err;
}

NLint sock_PollGroup(NLint group, NLenum name, NLsocket *sockets, NLint number, NLint timeout)
{
    NLint           numselect, count = 0;
    NLint           numsockets = NL_MAX_GROUP_SOCKETS;
    NLsocket        temp[NL_MAX_GROUP_SOCKETS];
    NLboolean       reliable[NL_MAX_GROUP_SOCKETS];
    NLboolean       result;
    NLsocket        *ptemp = temp;
    int             i, found = 0;
    fd_set          fdset;
    SOCKET          highest;
    struct timeval  t = {0,0}; /* {seconds, useconds}*/
    struct timeval  *tp = &t;
    
    nlGroupLock();
    highest = nlGroupGetFdset(group, &fdset);
    
    if(highest == INVALID_SOCKET)
    {
        /* error is set by nlGroupGetFdset */
        nlGroupUnlock();
        return NL_INVALID;
    }
    
    result = nlGroupGetSocketsINT(group, ptemp, &numsockets);
    nlGroupUnlock();
    
    if(result == NL_FALSE)
    {
        /* any error is set by nlGroupGetSockets */
        return NL_INVALID;
    }
    if(numsockets == 0)
    {
        return 0;
    }
    
    if(name == NL_READ_STATUS)
    {
        /* check for buffered reliable packets */
        for(i=0;i<numsockets;i++)
        {
            nl_socket_t *s = nlSockets[ptemp[i]];
            
            if(s->type == NL_RELIABLE_PACKETS && s->readable == NL_TRUE)
            {
                /* mark as readable */
                reliable[i] = NL_TRUE;
                found++;
                /* change the timeout to 0, or non-blocking since we */
                /* have at least one reliable packet to read */
                timeout = 0;
            }
            else
            {
                reliable[i] = NL_FALSE;
            }
        }
    }
    
    /* check for full blocking call */
    if(timeout < 0)
    {
        tp = NULL;
    }
    else /* set t values */
    {
        t.tv_sec = timeout/1000;
        t.tv_usec = (timeout%1000) * 1000;
    }
    
    /* call select to check the status */
    switch(name) {
        
    case NL_READ_STATUS:
        numselect = select((int)highest, &fdset, NULL, NULL, tp);
        break;
        
    case NL_WRITE_STATUS:
        numselect = select((int)highest, NULL, &fdset, NULL, tp);
        break;
        
    case NL_ERROR_STATUS:
        numselect = select((int)highest, NULL, NULL, &fdset, tp);
        break;

    default:
        nlSetError(NL_INVALID_ENUM);
        return NL_INVALID;
    }
    if(numselect == SOCKET_ERROR)
    {
        if(sockerrno == ENOTSOCK)
        {
            /* one of the sockets has been closed */
            nlSetError(NL_INVALID_SOCKET);
        }
        else if(sockerrno == EINTR)
        {
            /* select was interrupted by the system, maybe because the app is exiting */
            return 0;
        }
        else
        {
            nlSetError(NL_SYSTEM_ERROR);
        }
        return NL_INVALID;
    }
    
    if(numselect > number)
    {
        nlSetError(NL_BUFFER_SIZE);
        return NL_INVALID;
    }
    /* fill *sockets with a list of the sockets ready to be read */
    numselect += found;
    i = 0;
    while(numsockets-- > 0 && numselect > count)
    {
        nl_socket_t *s = nlSockets[*ptemp];
        
        if((reliable[i] == NL_TRUE ) || (FD_ISSET(s->realsocket, &fdset) != 0))
        {
            /* if checking for read status, must check for a complete packet */
            if(s->type == NL_RELIABLE_PACKETS && s->listen == NL_FALSE && name == NL_READ_STATUS)
            {
                (void)nlLockSocket(*ptemp, NL_READ);
                if(s->readable != NL_TRUE)
                {
                    if(s->inuse == NL_TRUE)
                    {
                        (void)sock_ReadPacket(*ptemp, NULL, 0, NL_TRUE);
                    }
                    else
                    {
                        s->readable = NL_FALSE;
                    }
                }
                if(s->readable == NL_TRUE)
                {
                    /* we do have a complete packet */
                    *sockets = *ptemp;
                    sockets++;
                    count++;
                }
                nlUnlockSocket(*ptemp, NL_READ);
            }
            else
            {
                *sockets = *ptemp;
                sockets++;
                count ++;
            }
        }
        i++;
        ptemp++;
    }
    return count;
}

NLboolean sock_Hint(NLenum name, NLint arg)
{
    switch(name) {
        
    case NL_LISTEN_BACKLOG:
        backlog = arg;
        break;
        
    case NL_MULTICAST_TTL:
        if(arg < 1)
        {
            arg = 1;
        }
        else if(arg > 255)
        {
            arg = 255;
        }
        multicastTTL = arg;
        break;
        
    case NL_REUSE_ADDRESS:
        reuseaddress = (NLboolean)(arg != 0 ? NL_TRUE : NL_FALSE);
        break;
        
    case NL_TCP_NO_DELAY:
        nlTCPNoDelay = (NLboolean)(arg != 0 ? NL_TRUE : NL_FALSE);
        break;
        
    default:
        nlSetError(NL_INVALID_ENUM);
        return NL_FALSE;
    }
    return NL_TRUE;
}
