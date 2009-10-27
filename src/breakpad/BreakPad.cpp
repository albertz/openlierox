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
#include "CrashHandler.h"

#ifndef WIN32
#include <unistd.h>
#include <cstdlib>
#include <setjmp.h>

// defined in main.cpp
extern void teeStdoutQuit(bool wait);

static sigjmp_buf restartLongjumpPoint;

static void dorestart() {
	execl( GetBinaryFilename(),
		  GetBinaryFilename(),
		  "-aftercrash",
		  (char*) 0 );
			
	// execl replaces this process, so no more code will be executed
	// unless it failed. If it failed, then we should return false.
}

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
	teeStdoutQuit(false);
	
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

	if(CrashHandler::restartAfterCrash) {
		printf("restarting game\n");
		// We must go out of the signal handler stack because we want to reset
		// it in the child process.
		siglongjmp(restartLongjumpPoint, true);
	}
	
    // we called fork()
    return true;
}

#else

#include "common/convert_UTF.h"

wchar_t* utf16fromutf8(const char* in, wchar_t* buf) {
	wchar_t* out = buf;

	buf[0] = 0;
	ConvertUTF8toUTF16(
		(const UTF8**)&in, (UTF8*)in + strlen(in),
		(UTF16**)&out, (UTF16*)out + MAX_PATH,
        lenientConversion);

	out[0] = 0;

	return buf;
}

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

	wprintf(L"CrashHandler, minidump_id %s\n", minidump_id);

	// DON'T USE THE HEAP!!!
    // So that indeed means, no QStrings, no qDebug(), no QAnything, seriously!

	wchar_t command[ 2 * MAX_PATH * 3 + 12 ];
	wchar_t buf[MAX_PATH * 3 + 6];
    wcscpy( command, L"\"" );
    wcscat( command, utf16fromutf8(GetBinaryFilename(), buf) );
	wcscat( command, L"\" -crashreport \"" );
    wcscat( command, dump_dir );
    wcscat( command, L"\" \"" );
    wcscat( command, minidump_id );
    wcscat( command, L"\" \"" );
    wcscat( command, utf16fromutf8(GetLogFilename(), buf) );
    wcscat( command, L"\"" );

    STARTUPINFOW si;
    PROCESS_INFORMATION pi;

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_SHOWNORMAL;
    ZeroMemory( &pi, sizeof(pi) );

    if (CreateProcessW( NULL, command, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
    {
        CloseHandle( pi.hProcess );
        CloseHandle( pi.hThread );

		if(CrashHandler::restartAfterCrash) {
			printf("restarting game\n");

			wchar_t command2[ 2 * MAX_PATH * 3 + 12 ];
			wcscpy( command2, L"\"" );
			wcscat( command2, utf16fromutf8(GetBinaryFilename(), buf) );
			wcscat( command2, L"\" -aftercrash" );

			STARTUPINFOW si2;
			PROCESS_INFORMATION pi2;
			
			ZeroMemory( &si2, sizeof(si2) );
			si2.cb = sizeof(si2);
			si2.dwFlags = STARTF_USESHOWWINDOW;
			si2.wShowWindow = SW_SHOWNORMAL;
			ZeroMemory( &pi2, sizeof(pi2) );
			
			if (CreateProcessW( NULL, command, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
				CloseHandle( pi2.hProcess );
				CloseHandle( pi2.hThread );				
			}
			else {
				wprintf(L"Error: could not start crash reporter, command: %s\n", command);
			}
		}
		
		TerminateProcess( GetCurrentProcess(), 1 );
    }
	else {
		wprintf(L"Error: could not start crash reporter, command: %s\n", command);
	}
	
    return false;
}

#endif // WIN32

BreakPad::BreakPad( const std::string& path )
: google_breakpad::ExceptionHandler(
									PATHFORGPB(path), 
									0, 
									LaunchUploader, 
									this, 
									true )
{
#ifndef WIN32
	if(sigsetjmp(restartLongjumpPoint, true)) {
		// This should call execl() so we replace the current process.
		dorestart();
		// in case we get here
		abort();
	}
#endif
}

BreakPad::~BreakPad()
{}

#endif // NBREAKPAD
