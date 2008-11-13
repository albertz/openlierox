/*
	OpenLieroX

	a CVS-reader
	
	code under LGPL
	created 22-05-2007
	by Albert Zeyer and Dark Charlie
*/

#ifndef __CVSREADER_H__
#define __CVSREADER_H__

#include <istream>
#include <string>


// A basic stream class for reading the stream data
class CsvStream  {
private:
	char *buffer;
	size_t bufLen;
	size_t bufPos;
public:
	CsvStream(std::istream& is)  {
		open(is);
	}

	~CsvStream()  { if (buffer) delete[] buffer; }

	void open(std::istream& is)  {
		// Get length of file
		size_t oldpos = is.tellg();
		is.seekg(0, std::ios::end);
		bufLen = is.tellg();
		is.seekg(oldpos, std::ios::beg);

		// Allocate memory
		buffer = new char[bufLen];

		// read data as a block:
		is.read(buffer, bufLen);
		bufPos = 0;		
	}

	bool is_open()  { return buffer != NULL; }
	bool eof()	{ return bufPos >= bufLen; }
	bool get(char& c)	{ 
		c = buffer[bufPos];
		++bufPos; 
		return bufPos < bufLen;
	}

	bool skipLine()  {
		while (bufPos < bufLen)  {
			if (buffer[bufPos] == '\n')  {
				++bufPos;
				return bufPos >= bufLen;
			}
			++bufPos;
		}
		return false;
	}
};

/*
	_handler is supposed to be a functor, which is compatible to:
		
	(It will be called, if I found a new token.)
		bool _handler(int index, const std::string& token);
	whereby
		index: tokenindex of current entry (line)
		token: data of token (quotes excluded if any)
		return: if false, I will continue with the next entry
	
	and:
		
	(It will be called, if the current entry is closed (e.g. on
	linebreak) and if there was at least one entry.)
		bool _handler();
	whereby
		return: if false, I will break the whole parsing
			
*/
enum ReaderState  {
	LINE_START = 0,
	READ_ENTRY,
	READ_QUOTED_ENTRY,
	READ_UNQUOTED_ENTRY,
	FINISH_ENTRY
};

template<typename _handler>
class CsvReader {
public:
	CsvStream *stream;
	_handler *handler;
	
	CsvReader()
		: stream(NULL), handler(NULL) { }

	CsvReader(std::istream* s, _handler *h)
		: handler(h) { stream = new CsvStream(*s); }

	~CsvReader()  { if (stream) delete stream; }

	void setStream(std::istream* s)  { 
		if (!stream) 
			stream = new CsvStream(*s);
		else 
			stream->open(*s); 
	}

	void setHandler(_handler* h)  { handler = h; }
	
private:
	bool endEntry(std::list<std::string>& entries)  {
		bool res = (*handler)(entries);
		entries.clear();
		return res;
	}

	void endToken(std::string& token, std::list<std::string>& entries)  {
		entries.push_back(token);
		token.clear();
	}

	// returns false, if there was a break
	bool read(size_t max_num = (size_t)-1) {
		if (!stream)
			return false;

		std::list<std::string> entries;

		ReaderState state = LINE_START;
		std::string token;

		size_t read_entries = 0;
		char nextch = '\0';
		while (stream->get(nextch) && read_entries != max_num)  {
			switch (state)  {
			case LINE_START:
				switch (nextch)  {
				case '#': // Comment
					if (stream->skipLine())
						return true;
					break;
				case '\r': // Blank line?
				case '\n':
					if (stream->skipLine())
						return true;
					break;
				case '\"': // Quoted token
					state = READ_QUOTED_ENTRY;
					break;
				default:  // Unquoted token
					token += nextch;
					state = READ_UNQUOTED_ENTRY;
				}
			break;
			case READ_QUOTED_ENTRY:
				switch (nextch)  {
				case '\"':  // End of the entry
					endToken(token, entries);
					state = FINISH_ENTRY;
					break;
				case '\r': // Newline, should not happen but who knows...
				case '\n':
					endToken(token, entries);
					++read_entries;
					if (!endEntry(entries))
						return false;
					state = LINE_START;
				break;
				default: // Add the character to the token
					token += nextch;
				}
			break;

			case READ_UNQUOTED_ENTRY:
				switch (nextch)  {
				case '\r': // End of line
				case '\n':
					endToken(token, entries);
					++read_entries;
					if (!endEntry(entries))
						return false;
					state = LINE_START;
				break;
				case ',': // End of the token
					endToken(token, entries);
					state = READ_ENTRY;
				break;
				default:
					token += nextch;
				}
			break;

			case READ_ENTRY:
				switch (nextch)  {
				case '\"':
					state = READ_QUOTED_ENTRY;
				break;
				case '\r': // End of line, should not happen
				case '\n':
					state = LINE_START;
				break;
				default:
					state = READ_UNQUOTED_ENTRY;
					token += nextch;
				}
			break;

			case FINISH_ENTRY:
				switch (nextch)  {
				case ',': // End of entry
					state = READ_ENTRY;
				break;
				case '\r': // End of line
				case '\n':
					++read_entries;
					if (!endEntry(entries))
						return false;
					state = LINE_START;
				break;
				}
			break;
			}
		}

		return true;
	}

public:
	bool readWhole()			{ return read(); }
	bool readSome(size_t count)	{ return read(count); }
	bool readingFinished()		{ return stream ? stream->eof() : true; } 

};

#endif
