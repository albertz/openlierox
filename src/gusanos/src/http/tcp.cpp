#include "tcp.h"
#include "sockets.h"

namespace TCP
{

Socket::~Socket()
{
	close();
}

void Socket::close()
{
	if(s)
	{
#ifdef WINDOWS
		closesocket(s);
#else
		::close(s);
#endif
		s = 0;
	}
}


bool Socket::think()
{
	if(connecting)
	{
		fd_set monitor;
		timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 0;
		FD_ZERO(&monitor);
		FD_SET(s, &monitor);
		select(s+1, 0, &monitor, 0, &tv);
		
		if(FD_ISSET(s, &monitor))
		{
			int status;
			socklen_t len = sizeof(status);
			getsockopt(s, SOL_SOCKET, SO_ERROR, (char *)&status, &len);
			if(status != 0) // Error
			{
				error = ErrorConnect;
				return true;
			}
			
			connected = true;
			connecting = false;
			resetTimer();
		}
		else
			checkTimeout();
	}
	
	return error != ErrorNone;
}

bool Socket::readChunk()
{
	dataBegin = dataEnd = 0;
	
	int r = recv(s, staticBuffer, 1024, 0);
	if(r != -1)
	{
		if(r == 0)
		{
			connected = false;
			return true; // No more data
		}
		dataBegin = staticBuffer;
		dataEnd = staticBuffer + r;
		resetTimer();
	}
	else if(sockError() != EWOULDBLOCK)
	{
		error = ErrorRecv;
		return true; // Error
	}
	else
	{
		checkTimeout();
	}
	
	return error != ErrorNone;
}

bool Socket::ResumeSend::resume()
{
	if(b == e)
		return true;
	
	if(sock->trySend(b, e))
	{
		error = sock->error; // Propagate any error
		return true;
	}
	
	return false;
}

Socket::ResumeSend* Socket::send(char const* b, char const* e)
{
	if(!trySend(b, e))
		return new ResumeSend(this, b, e);
	return 0;
}

Socket::ResumeSend* Socket::send(ResumeSend* r, char const* b, char const* e)
{
	if(r)
	{
		assert(r->e == e);
		if(r->resume())
		{
			delete r;
			return 0;
		}
	}
	else
	{
		return send(b, e);
	}
	
	return r;
}

bool Socket::trySend(char const*& b, char const* e)
{
	assert(b <= e);
	
	if(b == e)
		return true;
		
	int sent = ::send(s, b, e - b, 0);
	if(sent == -1)
	{
		if(sockError() != EWOULDBLOCK) // Error
		{
			error = ErrorSend;
			return true;
		}
		else
			checkTimeout();
	}
	else
	{
		b += sent;
		if(b == e)
			return true;
	}
	
	return error != ErrorNone;
}

}
