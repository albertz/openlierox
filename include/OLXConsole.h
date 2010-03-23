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
#include "olx-types.h"


// Routines
int		Con_Initialize();
bool	Con_IsInited();
void	Con_Shutdown();
void	Con_Toggle();
void	Con_Process(TimeDiff dt);
void	Con_Hide();
void	Con_Draw(SDL_Surface * bmpDest);

void	Con_AddText(int colour, const std::string& text, bool alsoToLogger = true);

bool	Con_IsVisible();

// if you want to execute something and get the output on the ingame console
void	Con_Execute(const std::string& cmd);



#endif  //  __CONSOLE_H__
