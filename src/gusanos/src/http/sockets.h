#ifndef OMFG_HTTP_SOCKETS_H
#define OMFG_HTTP_SOCKETS_H

#ifdef WINDOWS
#define NOGDI
#include <winsock2.h>
#include <wininet.h>
#include <cstdlib>
typedef int socklen_t;

inline int sockError() { return WSAGetLastError(); }
const int EINPROGRESS = WSAEINPROGRESS;
const int EWOULDBLOCK = WSAEWOULDBLOCK;

#else //if !defined(OSK)
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

inline int sockError() { return errno; }

#endif

#include <string>
#include <cassert>

namespace TCP
{

struct Hostent : public hostent
{
	Hostent(hostent const* p);
	
	~Hostent();
};

Hostent* resolveHost(std::string const& name);

int socketNonBlock();

bool connect(int s, sockaddr_in& addr);

bool createAddr(sockaddr_in& addr, hostent* hp, int port);

}

#endif //OMFG_HTTP_SOCKETS_H
