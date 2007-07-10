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

#include <SDL/SDL.h>
#include <list>


#define MAX_LLENGTH		128


// Line structure
class line_t { public:
	std::string	strLine;
	Uint32	iColour;
	float	fTime;
	unsigned int iID;
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
	ct_lines_t		WrappedLines;
	ct_lines_t		NewLines;
    unsigned int	nWidth;

	// Methods
	void	AddWrapped(const std::string& txt, Uint32 colour, float time, ct_lines_t &lines, bool mark_as_new);

public:
	// Methods
	void	Clear(void);
	void    AddText(const std::string& txt, int colour, float time);

    // Variables
	line_t *GetLine(int n);
	inline lines_iterator Begin()  { return WrappedLines.begin(); }
	inline lines_iterator End()  { return WrappedLines.end(); }
	lines_iterator At(int i);
	inline lines_riterator RBegin()  { return WrappedLines.rbegin(); }
	inline lines_riterator REnd()  { return WrappedLines.rend(); }
	line_t *GetNewLine(void);
    void    setWidth(int w);
	inline unsigned int		getNumLines(void)	{ return WrappedLines.size(); }
};




#endif  //  __CCHATBOX_H__
