#include "http.h"
#include "sockets.h"

#include <cstdio>
#include <cctype>
#include <memory>

#include <iostream>
#include <iterator>
#include <string>
#include <sstream>
#include <list>
#include <utility>
#include "../util/macros.h"
#include "../util/text.h"
#include "../util/log.h"
#include <boost/lexical_cast.hpp>
using boost::lexical_cast;

using std::cout;
using std::endl;

namespace HTTP
{

void Request::addHeader(std::string const& header)
{
	if(header.find("200 OK") != std::string::npos)
		success = true;
	else
	{
		std::string::size_type p = header.find(':');
		if(p == std::string::npos) return;
		std::string::size_type b = header.find_first_not_of(" ", p + 1);;
		//static char const contentLen[] = "Content-Length";
		if(istrCmp("Content-Length", header.begin(), header.begin() + p))
		{
			if(b == std::string::npos) return;
			dataLength = lexical_cast<size_t>(header.substr(b));
		}
	}
}

char const* Request::parseHeaders(char const* b, char const* e)
{
	char const* oldb = b;
	for(; b != e; ++b)
	{
		char c = *b;
		if(c == '\012')
		{
			oldb = b + 1;
			continue;
		}
		if(c == '\015')
		{
			header.insert(header.end(), oldb, b);
			
			if(header.empty())
				return b + 2; // No more headers, return pointer to the first piece of data
				
			addHeader(header);
			header.clear();
			continue;
		}
	}
	
	header.insert(header.end(), oldb, e);
		
	return 0;
}

void Request::switchState(Request::State newState)
{
	switch(newState)
	{
		case SendingData:
		{
			state = SendingData;
		}
		break;
		
		case ReadingHeader:
		{
			state = ReadingHeader;
		}
		break;
		
		case ReadingData:
		{
			state = ReadingData;
		}
		break;
		
		case Done:
		{
			close();
			state = Done;
		}
		break;
	}
}

bool Request::think()
{
	if(TCP::Socket::think())
		return true;
	
	if(connected)
	{
		switch(state)
		{
			case SendingData:
			{
				dataSender = send(dataSender,
					outData.data(),
					outData.data() + outData.size());
					
				if(error)
				{
					success = false;
					switchState(Done);
					return true; // Error
				}
				
				if(!dataSender)
					switchState(ReadingHeader);
			}
			break;
			
			case ReadingHeader:
			{
				if(readChunk())
				{
					if(error)
					{
						success = false;
						switchState(Done);
						return true; // Error
					}
					else
						return true;
				}
				
				if(char const* d = parseHeaders(dataBegin, dataEnd))
				{
					dataBegin = d; // Cut off the end of the header
					if(dataEnd < dataBegin) // Safe-guard if the HTTP header is malformed
					{
						success = false;
						switchState(Done);
						return true; // Error
					}
					
					if(concat)
						data.insert(data.end(), dataBegin, dataEnd);
					dataRecieved += dataEnd - dataBegin;
					switchState(ReadingData);
				}
			}
			break;
			
			case ReadingData:
			{
				if(readChunk())
				{
					if(error)
					{
						success = false;
						switchState(Done);
						return true; // Error
					}
					else
						return true;
				}
				
				if(concat)
					data.insert(data.end(), dataBegin, dataEnd);
				dataRecieved += dataEnd - dataBegin;
			}
			break;
			
			case Done:
			{
				return true;
			}
			break;
		}
	}
	
	return error != ErrorNone;
}

char Host::hexDigit(int v)
{
	if(v >= 0 && v <= 9)
		return '0' + v;
	else if(v >= 10 && v <= 15)
		return 'A' + (v - 10);
	else
		return '0';
}

std::string Host::urlencode(std::string const& v)
{
	std::string ret;
	const_foreach(i, v)
	{
		char c = *i;
		if(isalnum(c))
			ret += c;
		else if(c == ' ')
			ret += '+';
		else if(c == '\n')
			ret += "%0D%0A";
		else if(c != '\r') // Ignore \r
		{
			int ci = static_cast<int>(c);
			ret += '%';
			ret += hexDigit(ci / 16);
			ret += hexDigit(ci % 16);
		}
	}
	
	return ret;
}

std::string Host::urlencode(std::list<std::pair<std::string, std::string> > const& values)
{
	std::string ret;
	bool first = true;
	const_foreach(i, values)
	{
		if(!first)
			ret += '&';
		else
			first = false;
		ret += urlencode(i->first);
		ret += '=';
		ret += urlencode(i->second);
	}
	
	return ret;
}


Request* Host::query(
	std::string const& command,
	std::string const& url,
	std::string const& addHeader,
	std::string const& data
	)
{
	sockaddr_in server;
	
	if(!hp || options.changed) // Address not resolved
	{
		options.changed = false;
		delete hp; hp = 0;
		if(!(hp = TCP::resolveHost( (options.hasProxy ? options.proxy : host) )))
			return 0;
	}
	
    TCP::createAddr(server, hp, options.hasProxy ? options.proxyPort : port);
    
    int s;
    if((s = TCP::socketNonBlock()) < 0)
    	return 0;
    	
    if(!TCP::connect(s, server))
    	return 0;
    
	std::stringstream ss;
	if (options.hasProxy)
	{
		ss
		<< command << " http://" << host << ':' << port << '/'
		<< url << " HTTP/1.0\015\012"
		"User-Agent: " << options.userAgent << "\015\012";
		
		if(data.size() > 0)
			ss << "Content-Length: " << data.size() << "\015\012";
			
		ss
		<< addHeader << "\015\012";
    }
    else
    {
    	ss
    	<< command << " /" << url << " HTTP/1.0\015\012"
    	"Host: " << host << ":" << port << "\015\012"
    	"User-Agent: " << options.userAgent << "\015\012";
    	
    	if(data.size() > 0)
			ss << "Content-Length: " << data.size() << "\015\012";
			
		ss
    	<< addHeader << "\015\012";
    }
    
    //cout << "Returning request" << endl;

    return new Request(s, ss.str(), data);
}

Request* Host::get(std::string const& url)
{
	return query("GET", url, "", "");
}

Request* Host::post(std::string const& url, std::list<std::pair<std::string, std::string> > const& values)
{
	return query("POST", url, "Content-Type: application/x-www-form-urlencoded\015\012", urlencode(values));
}

Host::~Host()
{
	delete hp;
}

}
