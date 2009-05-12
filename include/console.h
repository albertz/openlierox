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

#include <SDL.h>
#include "SmartPointer.h"
#include "Command.h"
#include "Unicode.h"
#include "InputEvents.h"

// Console states
#define		CON_HIDDEN		0
#define		CON_DROPPING	1
#define		CON_DOWN		2
#define		CON_HIDING		3

#define		MAX_CONLENGTH	256
#define		MAX_CONLINES	15
#define		MAX_CONHISTORY	10


struct conline_t {
	int			Colour;
	std::string	strText;
};


class console_t { public:

	int			iState;
	float		fPosition;
	UnicodeChar	iLastchar;

	size_t		iCurpos;
	conline_t	Line[MAX_CONLINES];

	int			icurHistory;
	int			iNumHistory;
	conline_t	History[MAX_CONHISTORY];

	int			iBlinkState; // 1 - displayed, 0 - hidden
	AbsTime		fBlinkTime;  

    SmartPointer<SDL_Surface> bmpConPic;

};



// Routines
int		Con_Initialize();
bool	Con_IsInited();
void	Con_Shutdown();
void	Con_Toggle();
void	Con_Process(TimeDiff dt);
void	Con_Hide();
void	Con_Draw(SDL_Surface * bmpDest);

void	Con_AddText(int colour, const std::string& text, bool alsoToLogger = true);
void	Con_AddHistory(const std::string& text);

void	Con_Parse();

bool	Con_IsVisible();


#endif  //  __CONSOLE_H__
