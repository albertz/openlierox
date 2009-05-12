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
#include "types.h"


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
