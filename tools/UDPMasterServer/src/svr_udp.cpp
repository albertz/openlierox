
// That's the same as svr_udp.php but needs no MySQL or PHP :)

#include <string>
#include <vector>
#include <list>
#include <stdlib.h>
#include <stdio.h>

#ifdef WIN32

#define WIN32_LEAN_AND_MEAN
#include <winsock.h>
#include <time.h>

typedef int socklen_t;

#else

#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#endif

#define DEFAULT_PORT 23450
bool quit = false;	// TODO: signal here on Ctrl-C

struct HostInfo
{
	HostInfo( std::string _addr, time_t _lastping, std::string _name, int _maxworms, int _numplayers, int _state ):
		addr(_addr), lastping(_lastping), name(_name), maxworms(_maxworms), numplayers(_numplayers), state(_state) 
	{ };
	HostInfo(): lastping(0), maxworms(0), numplayers(0), state(0) {};
	std::string addr;
	time_t lastping;
	std::string name;
	int maxworms;
	int numplayers;
	int state;
};


int main(int argc, char ** argv)
{
	#ifdef WIN32
	WSADATA dummy;
	WSAStartup(MAKEWORD(2,0), &dummy );
	#endif

	int port = DEFAULT_PORT;
	if( argc > 1 )
		port = atoi( argv[1] );

	int sock = socket( PF_INET, SOCK_DGRAM, IPPROTO_UDP );
	if( sock == -1 )
	{
		printf("Error opening UDP socket\n");
		return 1;
	};
	
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = 0;
	
	if( bind( sock, (struct sockaddr *)&addr, sizeof(addr) ) != 0 )
	{
		printf("Error binding UDP socket at port %i\n", port);
		return 1;
	};
	
	std::list< HostInfo > hosts;
	
	struct sockaddr_in source;
	unsigned sourcePort;
	socklen_t sourceLen;
	char buf[1500];
	std::string data;
	
	while( ! quit )
	{
		sourceLen = sizeof(source);
		int size = recvfrom( sock, buf, sizeof(buf)-1, 0, (struct sockaddr *)&source, &sourceLen );
		if( size == -1 )
			continue;

		time_t lastping = time(NULL);
		
		for( std::list<HostInfo> :: iterator it = hosts.begin(); it != hosts.end(); it++ )
		{
			if( lastping - it->lastping > 2*60 )
			{
				printf("Host db updated: deleted: %s\n", it->name.c_str());
				hosts.erase(it);
				it = hosts.begin();
			};
		};

		data.assign( buf, size );
		
		sourcePort = ntohs(source.sin_port);
		
		std::string srcAddr = inet_ntoa( source.sin_addr );
		char sourceAddrBuf[128];
		sprintf(sourceAddrBuf, "%i", sourcePort );
		srcAddr += ":";
		srcAddr += sourceAddrBuf;
		
		printf("Got msg from %s: %s\n", srcAddr.c_str(), data.c_str() );
		
		if( data.find( "\xff\xff\xff\xfflx::traverse" ) == 0 )
		{
			struct sockaddr_in dest;
			dest.sin_family = AF_INET;
			unsigned destPort;
			int f = data.find( '\0' );
			if( f == std::string::npos )
				continue;
			f++;
			if( data.find(":", f) == std::string::npos )
				continue;
			dest.sin_addr.s_addr = inet_addr( data.substr( f, data.find(":", f) - f ).c_str() );
			f = data.find(":", f);
			f++;
			destPort = atoi( data.c_str()+f );
			dest.sin_port = htons(destPort);
			std::string send = "\xff\xff\xff\xfflx::traverse";
			send += '\0';
			send += srcAddr;
			send += '\0';
			printf("Sending lx::traverse %s to %s:%i\n", send.c_str() + send.find('\0')+1, inet_ntoa( dest.sin_addr ), destPort );
			sendto( sock, send.c_str(), send.size(), 0, (struct sockaddr *)&dest, sizeof(dest) );
			continue;
		};
		
		if( data.find( "\xff\xff\xff\xfflx::ping" ) == 0 )
		{
			std::string send = "\xff\xff\xff\xfflx::query";
			send += '\0';
			send += '\0';
			printf("Sending lx::query\n");
			sendto( sock, send.c_str(), send.size(), 0, (struct sockaddr *)&source, sizeof(source) );
			continue;
		};

		if( data.find( "\xff\xff\xff\xfflx::queryreturn" ) == 0 )
		{
			int f = data.find( '\0' );
			if( f == std::string::npos )
				continue;
			f++;
			std::string name = data.substr( f, data.find( '\0', f ) );
			f = data.find( '\0' );
			if( f == std::string::npos )
				continue;
			f++;
			int numplayers = data[f];
			int maxworms = data[f+1];
			int state = data[f+2];
	
			std::list<HostInfo> :: iterator it;
			for( it = hosts.begin(); it != hosts.end(); it++ )
			{
				if( it->addr == srcAddr )
				{
					*it = HostInfo( srcAddr, lastping, name, maxworms, numplayers, state );
					printf("Host db updated: updated: %s\n", name.c_str());
					break;
				};
			};
			if( it == hosts.end() )
			{
				hosts.push_back( HostInfo( srcAddr, lastping, name, maxworms, numplayers, state ) );
				printf("Host db updated: added: %s\n", name.c_str());
			};
			continue;
		};

		if( data.find( "\xff\xff\xff\xfflx::getserverlist" ) == 0 )
		{
			std::string send;
			unsigned amount = 0;
			for( std::list<HostInfo> :: iterator it = hosts.begin(); it != hosts.end(); it++, amount++ )
			{
				if( send.size() >= 255 || amount >= 255 )
				{
					send = std::string("\xff\xff\xff\xfflx::serverlist") + '\0' + char((unsigned char)amount) + send;
					printf("Sending lx::serverlist\n");
					sendto( sock, send.c_str(), send.size(), 0, (struct sockaddr *)&source, sizeof(source) );
					amount = 0;
					send = "";
				};
				send += it->addr + '\0' + it->name + '\0' + 
						char((unsigned char)it->numplayers) +
						char((unsigned char)it->maxworms) +
						char((unsigned char)it->state);
			};
			printf("Sending lx::serverlist\n");
			send = std::string("\xff\xff\xff\xfflx::serverlist") + '\0' + char((unsigned char)amount) + send;
			sendto( sock, send.c_str(), send.size(), 0, (struct sockaddr *)&source, sizeof(source) );
			continue;
		};

	};

	#ifdef WIN32
	closesocket(sock);
	WSACleanup();
	#else
	close(sock);
	#endif
	return 0;
};
