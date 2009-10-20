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

#include "BreakPad.h"

#ifndef NBREAKPAD

#include "Debug.h"
#include "Unicode.h"
#include "FindFile.h"

#ifndef WIN32
#include <unistd.h>

// defined in main.cpp
extern void teeStdoutQuit();

static bool
LaunchUploader( const char* dump_dir,
               const char* minidump_id,
               void* that, 
               bool succeeded )
{
    // DON'T USE THE HEAP!!!
    // So that indeed means, no QStrings, no qDebug(), no QAnything, seriously!

	printf("CrashHandler called, minidump_id: %s\n", minidump_id);
	
    if (!succeeded)
        return false;

	// NOTE: If we would want to add any further information to be added to the crash report,
	// like local variables or so, we could just write them with printf() at this point. This
	// is save. The process which writes to stdout is not affected by the crash!
	// Anyway, accessing any ingame variables could be insafe, so we should avoid any access
	// here which could lead to a crash.
	// A better/safer way would be to analyse the minidump and read some variables
	// directly from there.

	// Save it because we will quit the teeStdout and this will reset the logfilename.
	char logfile[2048]; logfile[0] = 0;
	strncpy(logfile, GetLogFilename(), sizeof(logfile));
	
	// Close the logfile, we don't want the crashreport debug info in there (too much spam right now from breakpad).
	// This should be save.
	teeStdoutQuit();
	
    pid_t pid = fork();

    if (pid == -1) // fork failed
        return false;
    if (pid == 0) { // we are the fork
		
        execl( GetBinaryFilename(),
               GetBinaryFilename(),
			   "-crashreport",
               dump_dir,
               minidump_id,
			   logfile,
               (char*) 0 );

        // execl replaces this process, so no more code will be executed
        // unless it failed. If it failed, then we should return false.

		// Return anyway true because otherwise, the process will not die.
		printf("ERROR: Cannot start %s\n", GetBinaryFilename());
		return true;
	}

    // we called fork()
    return true;
}

#else
static bool
LaunchUploader( const wchar_t* dump_dir,
               const wchar_t* minidump_id,
               void* that,
               EXCEPTION_POINTERS *exinfo,
               MDRawAssertionInfo *assertion,
               bool succeeded )
{
    if (!succeeded)
        return false;

    // DON'T USE THE HEAP!!!
    // So that indeed means, no QStrings, no qDebug(), no QAnything, seriously!

    const char* m_product_name = static_cast<BreakPad*>(that)->productName();

    // convert m_product_name to widechars, which sadly means the product name must be Latin1    
    wchar_t product_name[ 256 ];
    char* out = (char*)product_name;
    const char* in = m_product_name - 1;
    do {
        *out++ = *++in; //latin1 chars fit in first byte of each wchar
        *out++ = '\0';  //every second byte is NULL
    }
    while (*in);

    wchar_t command[MAX_PATH * 3 + 6];
    wcscpy( command, GetBinaryFilename() );
	wcscat( command, L" -crashreport \"" );
    wcscat( command, dump_dir );
    wcscat( command, L"\" \"" );
    wcscat( command, minidump_id );
    wcscat( command, L"\" \"" );
    wcscat( command, GetLogFilename() );
    wcscat( command, L"\"" );

    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_SHOWNORMAL;
    ZeroMemory( &pi, sizeof(pi) );

    if (CreateProcess( NULL, command, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
    {
        CloseHandle( pi.hProcess );
        CloseHandle( pi.hThread );
        TerminateProcess( GetCurrentProcess(), 1 );
    }

    return false;
}

#endif // WIN32


BreakPad::BreakPad( const std::string& path )
: google_breakpad::ExceptionHandler( Utf8ToSystemNative(path), 
									0, 
									LaunchUploader, 
									this, 
									true )
{}

BreakPad::~BreakPad()
{}

#endif // NBREAKPAD
