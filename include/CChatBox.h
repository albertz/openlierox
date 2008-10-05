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


#define MAX_LLENGTH		128


// Line structure
class line_t { public:
	std::string	strLine;
	Uint32	iColour;
	TXT_TYPE iTextType;
	float	fTime;
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
	void	Clear(void);
	void    AddText(const std::string& txt, int colour, TXT_TYPE TextType, float time);

    // Variables
	lines_iterator Begin()  { return Lines.begin(); }
	lines_iterator End()  { return Lines.end(); }
	lines_iterator At(int i);
	lines_riterator RBegin()  { return Lines.rbegin(); }
	lines_riterator REnd()  { return Lines.rend(); }
	line_t *GetNewLine(void);
	size_t		getNumLines(void)	{ return Lines.size(); }
};




#endif  //  __CCHATBOX_H__
