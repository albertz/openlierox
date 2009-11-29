#ifndef OMFG_TCP_H
#define OMFG_TCP_H

#include <ctime>

namespace TCP
{

	struct Socket
	{
		static int const bufferSize = 1024;

		enum Error
		{
		    ErrorNone,
		    ErrorConnect,
		    ErrorSend,
		    ErrorRecv,
		    ErrorDisconnect,
		    ErrorTimeout,
		};

		struct ResumeSend
		{
			ResumeSend(Socket* sock_, char const* b_, char const* e_)
					: sock(sock_), b(b_), e(e_), error(ErrorNone)
			{}

			Socket* sock;
			char const* b;
			char const* e;
			Error error;

			bool resume();
		};

		Socket(int s_, int timeOut_ = 10)
				: s(s_),
				connected(false),
				connecting(true),
				dataBegin(0),
				dataEnd(0),
				error(ErrorNone),
				timeOut(timeOut_)
		{
			resetTimer();
		}

		void checkTimeout()
		{
			if(time(0) > t + timeOut)
				error = ErrorTimeout;
		}

		void resetTimer()
		{
			t = time(0);
		}

		bool think();

		bool readChunk();

		ResumeSend* send(char const* b, char const* e);

		ResumeSend* send(ResumeSend* r, char const* b, char const* e);

		bool trySend(char const*& b, char const* e);

		Error getError()
		{
			return error;
		}

		~Socket();

		void close();

	protected:
		int s;
		bool connected;
		bool connecting;
		char staticBuffer[bufferSize];
		char const* dataBegin;
		char const* dataEnd;
		Error error;
		time_t t;
		int timeOut;
	};

}

#endif //OMFG_TCP_H
