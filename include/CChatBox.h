/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Chat Box class
// Created 10/5/01
// Jason Boettcher


#ifndef __CCHATBOX_H__
#define __CCHATBOX_H__

#include <SDL.h>
#include <list>
#include "Protocol.h"
#include "olx-types.h"
#include "Color.h"

//what is this for? since there was no conflict, I renamed it to correct the typo
#define MAX_LENGTH		128 


// Line structure
struct line_t {
	std::string	strLine;
	Color	iColour;
	TXT_TYPE iTextType;
	AbsTime	fTime;
	size_t iID;
};

typedef std::list<line_t> ct_lines_t;
typedef ct_lines_t::iterator lines_iterator;
typedef ct_lines_t::reverse_iterator lines_riterator;

class CChatBox {
public:
	// Constructor
	CChatBox() {
		Clear();
	}

	~CChatBox() {
		Clear();
	}

private:
	// Attributes
	ct_lines_t		Lines;
	ct_lines_t		NewLines;

public:
	// Methods
	void	Clear();
	void    AddText(const std::string& txt, Color colour, TXT_TYPE TextType, const AbsTime& time);

    // Variables
	lines_iterator Begin()  { return Lines.begin(); }
	lines_iterator End()  { return Lines.end(); }
	lines_iterator At(int i);
	lines_riterator RBegin()  { return Lines.rbegin(); }
	lines_riterator REnd()  { return Lines.rend(); }
	bool GetNewLine(line_t& ln);
	size_t		getNumLines()	{ return Lines.size(); }
};




#endif  //  __CCHATBOX_H__
