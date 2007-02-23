/////////////////////////////////////////
//
//         OpenLieroX
//
// based on sources for Carnage Marines
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Console header file
// Created 7/4/02
// Jason Boettcher


#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#include "con_cmd.h"

// Console states
#define		CON_HIDDEN		0
#define		CON_DROPPING	1
#define		CON_DOWN		2
#define		CON_HIDING		3

#define		MAX_CONLENGTH	256
#define		MAX_CONLINES	15
#define		MAX_CONHISTORY	10

// Colours
#define		CNC_NORMAL		0
#define		CNC_NOTIFY		1//MakeColour(200,200,200)
#define		CNC_ERROR		2//MakeColour(255,0,0)
#define		CNC_WARNING		3//MakeColour(200,128,128)
#define		CNC_DEV			4//MakeColour(100,100,255)
#define		CNC_CHAT		5//MakeColour(100,255,100)



class conline_t { public:
	int			Colour;
	std::string	strText;
};


class console_t { public:

	int			iState;
	float		fPosition;
	int			iLastchar;

	int			iCurLength;
	int			iCurpos;
	conline_t	Line[MAX_CONLINES];

	int			icurHistory;
	int			iNumHistory;
	conline_t	History[MAX_CONHISTORY];

	int			iBlinkState; // 1 - displayed, 0 - hidden
	float		fBlinkTime;  

	float		fTimePushed;
	float		fLastRepeat;
	bool		bHolding;

    SDL_Surface *bmpConPic;

};



// Routines
int		Con_Initialize(void);
void	Con_Shutdown(void);
void	Con_Toggle(void);
void	Con_Process(float dt);
void	Con_ProcessCharacter(int input);
void	Con_Hide(void);
void	Con_Draw(SDL_Surface *bmpDest);

void	Con_AddText(int colour, const std::string& text);
void	Con_Printf(int colour, char *fmt, ...);
void	Con_Printf(int colour, const std::string& txt); // same as Con_AddText
void	Con_AddHistory(const std::string& text);

void	Con_Parse(void);

bool	Con_IsUsed(void);


#endif  //  __CONSOLE_H__
