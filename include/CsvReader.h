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
template<typename _handler>
class CsvReader {
public:
	std::istream* stream;
	_handler *handler;
	
	// reading states
	char *buffer;
	size_t bufPos;
	size_t bufLen;
	
	CsvReader()
		: stream(NULL), handler(NULL), buffer(NULL), bufPos(0), bufLen(0) {}

	CsvReader(std::istream* s, _handler *h)
		: stream(s), handler(h), buffer(NULL), bufPos(0), bufLen(0) {}

	void setStream(std::istream* s)  { stream = s; }
	void setHandler(_handler* h)  { handler = h; }
	
private:
	// Returns true if we should skip the line
	inline bool lineStart()
	{
		if (bufPos == bufLen)
			return true;

		// Check for a comment/blank line
		if (buffer[bufPos] == '#')  {
			while (bufPos != bufLen && buffer[bufPos] != '\n')
				++bufPos;
			return true;
		}

		return false;
	}

	// Returns false if reached the end of the entry
	inline bool readEntry(std::list<std::string>& entries)
	{
		if (bufPos == bufLen)
			return false;

		bool quot = false;

		if (buffer[bufPos] == '\"') {
			++bufPos;
			quot = true;
			if (bufPos == bufLen)
				return false;
		}

		std::string res;
		while (bufPos != bufLen)  {

			// Quote
			if (buffer[bufPos] == '\"')  {
				// If not in quote or escaped, add it and move on
				if (!quot || buffer[bufPos - 1] == '\\')  {
					res += '\"';
					++bufPos;
					continue;
				} else  {
					entries.push_back(res); // End of quote, finished reading the entry
					++bufPos;
					while (bufPos != bufLen)  {
						if (buffer[bufPos] == ',')  {
							++bufPos;
							return true;
						}
						if (buffer[bufPos] == '\n')  {
							++bufPos;
							return false;
						}
						++bufPos;
					}
					return false;
				}
			}

			// Delimiter
			if (!quot && buffer[bufPos] == ',')  {
				++bufPos;
				entries.push_back(res);
				return true;
			}

			// End of line
			if (buffer[bufPos] == '\n')  {
				++bufPos;
				entries.push_back(res);
				return false;
			}

			res += buffer[bufPos];
			++bufPos;
		}

		// We are at the end of the stream
		entries.push_back(res);
		return false;
	}

public:	
	// returns false, if there was a break
	bool read() {

		// Get length of file:
		stream->seekg(0, ios::end);
		bufLen = stream->tellg();
		stream->seekg(0, ios::beg);

		// Allocate memory
		buffer = new char[bufLen];

		// read data as a block:
		stream->read(buffer, bufLen);
		bufPos = 0;

		std::list<std::string> entries;

		while (bufPos != bufLen)  {
			if (lineStart())
				continue;

			entries.clear();

			while (readEntry(entries)) {}

			if (!(*handler)(entries))  { // Critical error?
				delete[] buffer;
				buffer = NULL;
				return false;
			}
		}

		delete[] buffer;
		buffer = NULL;
		return true;
	}

};

#endif
