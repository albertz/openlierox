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
	_handler& handler;
	
	// reading states
	bool inquote;
	bool waitforkomma;
	bool ignoreline;
	
	// collected data
	int tindex;
	std::string token;	
	
	CsvReader(std::istream* s, _handler& h)
		: stream(s), handler(h), inquote(false), waitforkomma(false),
		ignoreline(false), tindex(0) {}
	
private:
	// gets called at the end of line, if there was at least one entry
	// if return-value is false, reading will break
	inline bool endEntry() {
		return handler();
	}
	
	inline void endToken() {
		ignoreline = false;
		waitforkomma = false;
		inquote = false;
		
		ignoreline = !handler(tindex, token);
		
		token = "";
		tindex++;
	}

public:	
	// returns false, if there was a break
	bool read() {
		char nextch = '\0';
		while(!stream->eof()) {
			stream->get(nextch);			
			switch(nextch) {
			case 0:
				break;
				
			case ' ':
			case 9: // TAB
				if(ignoreline) break;
				if(waitforkomma) break;
				if(inquote) token += nextch;
				break;
							
			case '\"':
				if(ignoreline) break;
				if(waitforkomma) break;
				if(inquote) {
					endToken();
					waitforkomma = true;
				} else {
					inquote = true;
				}
				break;
				
			case 10: // LF (newline)
			case 13: // CR
				if(!ignoreline && !waitforkomma) endToken();
				if(tindex > 0) if(!endEntry()) return false;
				ignoreline = false;
				waitforkomma = false;
				inquote = false;
				tindex = 0;
				break;
				
			case '#': // comment marking
				if(ignoreline) break;
				if(inquote)
					token += nextch;
				else {
					if(!waitforkomma) endToken();
					ignoreline = true;
				}
				break;
				
			case ',': // new token marking
				if(ignoreline) break;				
				if(waitforkomma) {
					waitforkomma = false;
					break;
				}
				if(inquote)
					token += nextch;
				else
					endToken();
				break;
				
			default:
				if(!ignoreline && !waitforkomma) token += nextch;
			}
		}
	
		// stream->eof() here
		if(tindex > 0) if(!endEntry()) return false;
		return true;
	}

};

#endif
