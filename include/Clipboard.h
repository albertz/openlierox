/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////

/*
	Clipboard code grabbed from Battle for Wesnoth Project
	06-12-2007 albert
*/

/* $Id: clipboard.hpp 19552 2007-08-15 13:41:56Z mordante $ */
/*
   Copyright (C) 2003 - 2007 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef CLIPBOARD_HPP_INCLUDED
#define CLIPBOARD_HPP_INCLUDED

#include <string>
#include <SDL.h>

void copy_to_clipboard(const std::string& text);
std::string copy_from_clipboard();

// on X11 we will get an SDL_SYSWMEVENT when another application requests our text in clipboard
void Clipboard_handleSysWmEvent(const SDL_Event& ev);

#endif

