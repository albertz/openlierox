/////////////////////////////////////////
//
//             Liero Xtreme
//
//     Copyright Auxiliary Software 2002
//
//
/////////////////////////////////////////


// Chat Box class
// Created 10/5/01
// Jason Boettcher


#ifndef __CCHATBOX_H__
#define __CCHATBOX_H__


#define MAX_LLENGTH		128
#define MAX_CLINES		6


// Line structure
typedef struct {
	char	strLine[MAX_LLENGTH];
	int		iUsed;
	Uint32	iColour;
	float	fTime;
} line_t;




class CChatBox {
public:
	// Constructor
	CChatBox() {
		Clear();
	}

private:
	// Attributes
	line_t	Lines[MAX_CLINES];
    int     nWidth;

public:
	// Methods


	void	Clear(void);
	void    AddText(char *txt, int colour, float time);
	void    MoveUp(void);

    // Variables
	line_t *GetLine(int n)		{ return &Lines[n]; }
    void    setWidth(int w)     { nWidth = w; }
};




#endif  //  __CCHATBOX_H__
