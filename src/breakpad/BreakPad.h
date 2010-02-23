/***************************************************************************
 *   Copyright (C) 2005 - 2007 by                                          *
 *      Last.fm Ltd <client@last.fm>                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.          *
 ***************************************************************************/

#include "BreakpadDllExportMacro.h"

#ifndef NBREAKPAD

#include <string>

#ifdef __APPLE__
#   include "client/mac/handler/exception_handler.h"
#elif defined WIN32
#   include "client/windows/handler/exception_handler.h"
#elif defined __linux__
#   include "client/linux/handler/exception_handler.h"
#else
#   define NBREAKPAD
#endif

#ifdef WIN32									
#define PATHFORGPB(p)	Utf8ToUtf16(p)
#else
#define PATHFORGPB(p)	(p)
#endif

struct BreakPad : public google_breakpad::ExceptionHandler
{
    BreakPad( const std::string &dump_write_dirpath );
    ~BreakPad();
};

#endif


#undef char
