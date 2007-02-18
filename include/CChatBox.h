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

#include <vector>
using namespace std;


#define MAX_LLENGTH		128


// Line structure
typedef struct {
	std::string	strLine;
	Uint32	iColour;
	float	fTime;
	bool	bNew;
} line_t;

typedef vector<line_t> ct_lines_t;

class CChatBox {
public:
	// Constructor
	CChatBox() {
		Clear();
	}

private:
	// Attributes
	ct_lines_t		Lines;
	ct_lines_t		WrappedLines;
    int				nWidth;
	int				iNewLine;

	// Methods
	void	AddWrapped(const std::string& txt, int colour, float time);

public:
	// Methods
	void	Clear(void);
	void    AddText(const std::string& txt, int colour, float time);

    // Variables
	line_t *GetLine(int n);
	line_t *GetNewLine(void);
    void    setWidth(int w);
	int		getNumLines(void)	{ return WrappedLines.size(); }
};




#endif  //  __CCHATBOX_H__
