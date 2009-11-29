#include "sockets.h"

namespace TCP
{

char* copyCString(char* p)
{
	int l = strlen(p);
	char* n = new char[l + 1];
	memcpy(n, p, l + 1);
	return n;
}

char* copyString(char* p, size_t l)
{
	char* n = new char[l + 1];
	memcpy(n, p, l + 1);
	return n;
}

char** copyList(char** p)
{
	int l = 0;
	for(; p[l]; ++l)
		/* nothing */;
	
	char** n = new char*[l + 1];
	for(int i = 0; i < l; ++i)
	{
		n[i] = copyCString(p[i]);
	}
	
	n[l] = 0;
	
	return n;
}

char** copyListL(char** p, size_t len)
{
	int l = 0;
	for(; p[l]; ++l)
		/* nothing */;
	
	char** n = new char*[l + 1];
	for(int i = 0; i < l; ++i)
	{
		n[i] = copyString(p[i], len);
	}
	
	n[l] = 0;
	
	return n;
}

void deleteList(char** p)
{
	for(int i = 0; p[i]; ++i)
		delete[] p[i];
	delete[] p;
}

Hostent::Hostent(hostent const* p)
{
	h_name = copyCString(p->h_name);
	h_aliases = copyList(p->h_aliases);
	h_addrtype = p->h_addrtype;
	h_length = p->h_length;
	h_addr_list = copyListL(p->h_addr_list, h_length);
}

Hostent::~Hostent()
{
	delete[] h_name;
	deleteList(h_aliases);
	deleteList(h_addr_list);
}

Hostent* resolveHost(std::string const& name)
{
	hostent* p = gethostbyname( name.c_str() );
	
	if(!p)
		return 0; // ERROR
	
	return new Hostent(p);
}

int socketNonBlock()
{
	int s;
	
	if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0)
		return -1;

#ifdef WINDOWS
	unsigned long nonBlocking = 1;
	ioctlsocket(s, FIONBIO, &nonBlocking);
#else
	fcntl(s, F_SETFL, O_NONBLOCK);
#endif

	return s;
}

bool connect(int s, sockaddr_in& addr)
{
	int r = connect(s, reinterpret_cast<sockaddr *>(&addr), sizeof(addr));
	
	if(r == -1 &&
	#ifdef WINDOWS
		sockError() != EWOULDBLOCK)
	#else
		sockError() != EINPROGRESS)
	#endif
		return false; // ERROR
		
	return true;
}

bool createAddr(sockaddr_in& addr, hostent* hp, int port)
{
	memset((char *) &addr, 0, sizeof(addr));
    memmove((char *) &addr.sin_addr, hp->h_addr_list[0], hp->h_length);
    addr.sin_family = hp->h_addrtype;
    addr.sin_port = htons( port );
    
    return true;
}

}
